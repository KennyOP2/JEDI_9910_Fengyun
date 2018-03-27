/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Function body for or32_init and the ISR function.
 *
 * @author Kuoping Hsu
 * @date 2006.11.06.
 * @version 1.0
 *
 */

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "or32.h"
#include "mmio.h"
#include "spr_defs.h"
#include "engine.h"
#include "isr.h"
#include "FreeRTOS.h"
#include "timer.h"

#define HOOK_EXCEPTION

/////////////////////////////////////////////////////////////////
//                      Function Declare
/////////////////////////////////////////////////////////////////
static void isr_dispatch(void);
static struct ISR_HANDLE isr_int_handle;

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
static struct ISR_HANDLE isr_tt_handle;
static struct ISR_HANDLE isr_sys_handle;

#if defined(HOOK_EXCEPTION)
static struct ISR_HANDLE isr_illinst_handle;
static struct ISR_HANDLE isr_align_handle;
static struct ISR_HANDLE isr_buserr_handle;
static struct ISR_HANDLE isr_trap_handle;

static void ill_except_handler(void);
static void align_except_handler(void);
static void buserr_except_handler(void);
static void trap_except_handler(void);

#endif // HOOK_EXCEPTION

/////////////////////////////////////////////////////////////////
//                      Public Function
/////////////////////////////////////////////////////////////////

typedef void (*FlipCallback)(void);
static FlipCallback gpfFlipCallback;
static FlipCallback gpfTimerCallback;

void
SetFlipCallback(
    FlipCallback callback)
{
    gpfFlipCallback = callback;
}

#ifdef ENABLE_ONFLY
void Set_Flip_Timer(
    FlipCallback callback,
    int period)
{
    int hz = or32_getSysCLK();
    int ms = period * (hz / 1000);

    gpfTimerCallback = callback;

    /* Enable tick timer exception recognition */
    ENABLE_SR_INTERRUPT_EXCEPTION();

    /* stop timer */
    mtspr(SPR_TTMR4, 0);

    /* Reset counter */
    mtspr(SPR_TTCR4, 0);

    /* Set Tick timer 2 raise to external interrupt */
    mtspr(SPR_TTSR, mfspr(SPR_TTSR) | SPR_TTSR_INTS);

    /* timer single run mode and enable interrupt */
    mtspr(SPR_TTMR4, SPR_TTMR_SR | SPR_TTMR_IE | ms);
}
#endif

/*
 * OpenRISC initialization
 */
void or32_init(void) {

    /* Install interrupt ISR (0x800) */
    or32_installISR(OR32_ISR_INT, &isr_int_handle, isr_dispatch);
    or32_installISR(OR32_ISR_TT, &isr_tt_handle, timer_except_handler);
    or32_installISR(OR32_ISR_SYSCALL, &isr_sys_handle, vTaskSwitchContext);

#if defined(HOOK_EXCEPTION)
    or32_installISR(OR32_ISR_ILL,    &isr_illinst_handle, ill_except_handler);
    or32_installISR(OR32_ISR_ALIGN,  &isr_align_handle,   align_except_handler);
    or32_installISR(OR32_ISR_BUSERR, &isr_buserr_handle,  buserr_except_handler);
    or32_installISR(OR32_ISR_TRAP,   &isr_trap_handle,    trap_except_handler);
#endif // HOOK_EXCEPTION

#if defined(MM9070) || defined(MM9910)
    //MMIO_Write(GIR_SELECT, GIR_OPENRISC|GIR_MMC);
#else
    #error "interrupt direction is not defined"
#endif

    /* Clear the DrvEnable bit to indicate the code is initialized
       (the BSS section has cleared)
    */
    MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvEnable);

    /* Enable Interrupt */
    ENABLE_SR_INTERRUPT_EXCEPTION();

    timer_init();

    /* Mask the interrupt from host               */
    /* D[31:6] -- None                            */
    /* D[5]    -- UART interrupt (P370)           */
    /* D[4]    -- Interrupt from Tick Timer 2     */
    /* D[3]    -- Interrupt from host             */
    /* D[2]    -- DMA Interrupt                   */
    /* D[1:0]  -- Reserved (Non-Maskable)         */
    ENABLE_PICMR_HOST_INTERRUPT();
    ENABLE_PICMR_TT_INTERRUPT();

    /* Update the Ticks */
    //or32_calcTicks();
}

/////////////////////////////////////////////////////////////////
//                      Private Function
/////////////////////////////////////////////////////////////////
volatile unsigned int __host_int_raised = 0;
volatile int __apb_int_raised = 0;

void isr_dispatch(void)
{
    unsigned long picsr = mfspr(SPR_PICSR);
    unsigned int host_who = 0;
    int apb_who = 0;

#ifdef ENABLE_ONFLY
    if (picsr & TICK_INT_MASK) {
        if (mfspr(SPR_TTMR4) & SPR_TTMR_IP)
        {
            // stop timer
            mtspr(SPR_TTMR4, 0);

            if (gpfTimerCallback)
                gpfTimerCallback();
        }
        #if 0
        if (mfspr(SPR_TTMR) & SPR_TTMR_IP)
        {
            // stop timer
            mtspr(SPR_TTMR, mfspr(SPR_TTMR) & (~SPR_TTMR_IP));
        }
        if (mfspr(SPR_TTMR2) & SPR_TTMR_IP)
        {
            // stop timer
            mtspr(SPR_TTMR2, mfspr(SPR_TTMR2) & (~SPR_TTMR_IP));
        }
        if (mfspr(SPR_TTMR3) & SPR_TTMR_IP)
        {
            // stop timer
            mtspr(SPR_TTMR3, mfspr(SPR_TTMR3) & (~SPR_TTMR_IP));
        }
        #endif
        CLEAR_PICSR_TT_INTERRUPT();
    }

    if (picsr & HOST_INT_MASK) {
        //__apb_int_raised = AHB_MMIO_Read(0x68200040);
        //if (__apb_int_raised & 0x00000002) {
            __host_int_raised = MMIO_Read(GIR_STATUS);
        //}
        if (gpfFlipCallback)
            gpfFlipCallback();
        CLEAR_PICSR_HOST_INTERRUPT();
    }
#endif
}

#if defined(HOOK_EXCEPTION)
extern volatile int *pxCurrentTCB;
extern int _bus_error_halt;
extern int _alignment_halt;
extern int _illegal_inst_halt;
extern int _trap_halt;

static __inline void dump_reg(void)
{
    int *stack;

    // Prevent non-alignment access
    if ((((int ) pxCurrentTCB) & 3) != 0) return;
    if ((*((int*)pxCurrentTCB) & 3) != 0) return;

    stack  = (int*)(*pxCurrentTCB);
    stack += (CTXT_BYTES_OFFSET_FROM_SP / sizeof(int));

    printf("Exception occurs at address 0x%08x.\n\n", stack[1]);
    printf("Register dump:\n");
    printf("R0 = 0x%08x, R1 = 0x%08x, R2 = 0x%08x, R3 = 0x%08x\n", 0, &stack, stack[3], stack[4]);
    printf("R4 = 0x%08x, R5 = 0x%08x, R6 = 0x%08x, R7 = 0x%08x\n", stack[ 5], stack[ 6], stack[ 7], stack[ 8]);
    printf("R8 = 0x%08x, R9 = 0x%08x, R10= 0x%08x, R11= 0x%08x\n", stack[ 9], stack[10], stack[11], stack[12]);
    printf("R12= 0x%08x, R13= 0x%08x, R14= 0x%08x, R15= 0x%08x\n", stack[13], stack[14], stack[15], stack[16]);
    printf("R16= 0x%08x, R17= 0x%08x, R18= 0x%08x, R19= 0x%08x\n", stack[17], stack[18], stack[19], stack[20]);
    printf("R20= 0x%08x, R21= 0x%08x, R22= 0x%08x, R23= 0x%08x\n", stack[21], stack[22], stack[23], stack[24]);
    printf("R24= 0x%08x, R25= 0x%08x, R26= 0x%08x, R27= 0x%08x\n", stack[25], stack[26], stack[27], stack[28]);
    printf("R28= 0x%08x, R29= 0x%08x, R30= 0x%08x, R31= 0x%08x\n", stack[29], stack[30], stack[31], stack[32]);
//  printf("EPCR = 0x%08x, ESR = 0x%08x\n", stack[1], stack[2]);
    printf("\n");
}

__attribute__ ((naked))
void ill_except_handler(void)
{
    printf("\n[EXCEPTION] CPU%d illegal instruction...\n", or32_getCpuID());
    dump_reg();
    printf("** system halt **\n");
    asm volatile("l.jr %0\nl.nop" : : "r"(&_illegal_inst_halt));
}

__attribute__ ((naked))
void align_except_handler(void)
{
    printf("\n[EXCEPTION] CPU%d memory alignment access error...\n", or32_getCpuID());
    dump_reg();
    printf("** system halt **\n");
    asm volatile("l.jr %0\nl.nop" : : "r"(&_alignment_halt));
}

__attribute__ ((naked))
void buserr_except_handler(void)
{
    printf("\n[EXCEPTION] CPU%d bus error...\n", or32_getCpuID());
    dump_reg();
    printf("** system halt **\n");
    asm volatile("l.jr %0\nl.nop" : : "r"(&_bus_error_halt));
}

#define MEM_DEBUG_REG1  0x03d8
#define MEM_DEBUG_REG2  0x03da
#define MEM_DEBUG_REG3  0x03dc
#define MEM_DEBUG_REG4  0x03de

#define MEM_DEBUG_REG5  0x03f6
#define MEM_DEBUG_REG6  0x03f8
#define MEM_DEBUG_REG7  0x03fa
#define MEM_DEBUG_REG8  0x03fc

static const char *dbg_module[] = {
    "Demod Wr", "Demod Rd", "IIS Rd", "IIS Wr", "TSI Wr", "TSI Wr", "HOST Wr", "HOST Rd",
    "LCD Rd", "CQ Rd", "IQ Rd", "USB Wr", "USB Rd", "RISC Rd", "RISC Wr", "AHB Rd",
    "AHB Wr", "ISP Wr", "ISP Rd", "MPEG Wr", "MPEG Rd0", "MPEG Rd1", "Video Wr", "Video Rd0",
    "Video Rd1", "2D Rd", "2D Wr", "BIST Wr", "BIST Rd", "Unknown"
};

__attribute__ ((naked))
void trap_except_handler(void)
{
    int *stack = (int*)(*pxCurrentTCB);
    stack += (CTXT_BYTES_OFFSET_FROM_SP / sizeof(int));
    int *inst = (int*)stack[1];
    int cpu_id = or32_getCpuID();

    if ((stack[1] & 0xf0000000) == 0)
    {
        if (*inst == 0x2100000a)
            printf("\n[EXCEPTION] CPU%d divide by zero...\n", cpu_id);
        else if (*inst == 0x2100000f)
            printf("\n[EXCEPTION] CPU%d software breakpoint...\n", cpu_id);
        else if ((*inst & 0xff000000) == 0x21000000)
            printf("\n[EXCEPTION] CPU%d trap(%d)...\n", cpu_id, (*inst & 0xf));
        else
        {
            short regdata = MMIO_Read(MEM_DEBUG_REG2);
            if (regdata & 0x8000) // if watch point match
            {
                int type = (regdata >> 9)&0x1f;
                if (type >= sizeof(dbg_module) / sizeof(dbg_module[0]))
                    type = sizeof(dbg_module) / sizeof(dbg_module[0]) - 1;

                printf("\n[EXCEPTION] CPU%d trap by memory protection (%s)...\n", cpu_id, dbg_module[type]);

                if (MMIO_Read(MEM_DEBUG_REG4) & 0x8000) {
                    printf("Try to access memory region 0x%08x ~ 0x%08x ...\n",
                           ((MMIO_Read(MEM_DEBUG_REG2) & 0xff) << 18) + (MMIO_Read(MEM_DEBUG_REG1) << 2),
                           ((MMIO_Read(MEM_DEBUG_REG4) & 0xff) << 18) + (MMIO_Read(MEM_DEBUG_REG3) << 2));
                }

                if (MMIO_Read(MEM_DEBUG_REG8) & 0x8000) {
                    printf("Try to access memory region 0x%08x ~ 0x%08x ...\n",
                           ((MMIO_Read(MEM_DEBUG_REG6) & 0xff) << 18) + (MMIO_Read(MEM_DEBUG_REG5) << 2),
                           ((MMIO_Read(MEM_DEBUG_REG8) & 0xff) << 18) + (MMIO_Read(MEM_DEBUG_REG7) << 2));
                }
            }
            else
            {
                printf("\n[EXCEPTION] CPU%d unknown trap type...\n", cpu_id);
            }
        }
    }
    else
    {
        printf("\n[EXCEPTION] CPU%d unknown trap, can not trace stack...\n", cpu_id);
    }

    dump_reg();
    printf("** system halt **\n");
    asm volatile("l.jr %0\nl.nop" : : "r"(&_trap_halt));
}
#endif // HOOK_EXCEPTION

