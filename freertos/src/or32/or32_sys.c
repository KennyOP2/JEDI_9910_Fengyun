/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Function body for OR32 related functions.
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#include <stdlib.h>

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include "proj_defs.h"
#include "spr_defs.h"
#include "or32.h"
#include "mmio.h"

#define MHz 1000000
#define KHz 1000
#define PLL_STEP 4

//James add for FPGA test -- must removed
// FPGA
//#define RISC_FPGA
//#define RISC_FPGA_CLK_CPU   200000000
//#define RISC_FPGA_CLK_MEM   230000000
//#define RISC_FPGA_CLK_BUS    80000000

/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////
static unsigned int tick_in_ms = 0;
static unsigned int tick_in_us = 0;

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
static char _except_str[128];
static char *_except_ptr;

//static int clkMappingTbl[] =
//{
//    12000000,
//    16380000,
//    20480000,
//    26000000,
//};

//static int pllPreDivTbl[] =
//{
//    0x0C,
//    0x10,
//    0x14,
//    0x1A,
//};

/////////////////////////////////////////////////////////////////
//                      Public Function
/////////////////////////////////////////////////////////////////

#if defined(__FREERTOS__) && defined(ENABLE_PROFILING)
#  include "FreeRTOS.h"
#  include "task.h"

#  define MAX_TASK_SIZE 32
extern unsigned int _stack_start;
extern unsigned int _stack_end;

// global variable
int taskStackCount[MAX_TASK_SIZE];

typedef struct tskTaskControlBlock
{
    portSTACK_TYPE          *pxTopOfStack;      /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE STRUCT. */
    portSTACK_TYPE          *pxStack;           /*< Points to the start of the stack. */
    unsigned portBASE_TYPE  uxTCBNumber;        /*< This is used for tracing the scheduler and making debugging easier only. */
    unsigned portBASE_TYPE  uxPriority;         /*< The priority of the task where 0 is the lowest priority. */
    xListItem               xGenericListItem;   /*< List item used to place the TCB in ready and blocked queues. */
    xListItem               xEventListItem;     /*< List item used to place the TCB in event lists. */
    signed portCHAR         pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */
    unsigned portSHORT      usStackDepth;       /*< Total depth of the stack (when empty).  This is defined as the number of variables the stack can hold, not the number of bytes. */
} tskTCB;

extern signed portBASE_TYPE xSchedulerRunning;

void print_stack_size(void)
{
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", taskStackCount[0], taskStackCount[1], taskStackCount[2],
        taskStackCount[3], taskStackCount[4], taskStackCount[5], taskStackCount[6], taskStackCount[7], taskStackCount[8],
        taskStackCount[9], taskStackCount[10], taskStackCount[11], taskStackCount[12], taskStackCount[13], taskStackCount[14],
        taskStackCount[15], taskStackCount[16]);
}

/*
 * Profiler
 */
void PROFILE_FUNC or32_profiler_start(int call_id)
{
    xTaskHandle taskHandle;
    unsigned int framePointer;
    int stackSize, n;

    if (xSchedulerRunning == pdFALSE) return;

    // Get frame pointer
    asm volatile("l.add %0, r0, r2" : "=r"(framePointer) : );

    // Skip the profiling when it in the OS kernel.
    if ((unsigned)framePointer >= (unsigned)_stack_start && (unsigned)framePointer <= (unsigned)_stack_end) return;

    // Check task count
    n = uxTaskGetNumberOfTasks();
    if (n > MAX_TASK_SIZE) while(1);

    // Update stack count
    taskHandle = xTaskGetCurrentTaskHandle();
    stackSize = (int)(((tskTCB*)taskHandle)->pxStack) + (int)(((tskTCB*)taskHandle)->usStackDepth)*sizeof(portSTACK_TYPE) - framePointer;
    if (taskStackCount[((tskTCB*)taskHandle)->uxTCBNumber] < stackSize) {
        taskStackCount[((tskTCB*)taskHandle)->uxTCBNumber] = stackSize;
    }

    //printf("CALL ID: %08x, Task ID: %d, Stack Size: (%d,%d)\n",
    //        call_id, ((tskTCB*)taskHandle)->uxTCBNumber, stackSize, taskStackCount[((tskTCB*)taskHandle)->uxTCBNumber]);
}

void PROFILE_FUNC or32_profiler_stop(int call_id)
{
    // printf("end:%d\r\n", call_id);
}
#endif // __FREERTOS__ && ENABLE_PROFILING

/*
 * Get Processor ID
 */
int or32_getCpuID(void)
{
    return ((mfspr(SPR_VR) & SPR_VER_N) >> 6) + 1;
}

static int get_pll_clk(int n)
{
    int pll = 0;
    int reg1 = MMIO_Read(n*0x10 + 0x90);
    int SDM_Sel    = (reg1 & (1<<15)) ? 1 : 0;
    int SDM_Bypass = (reg1 & (1<<14)) ? 1 : 0;
    int SDM_Fix    = ((reg1 & (3<<12)) >> 12) + 3;
    int Pre_Div    = (reg1 & 0x1f);
    
    if (SDM_Sel) { // SDM divider
        int reg4   = MMIO_Read(n*0x10 + 0x96);
        int sdm    = (reg4 & 0x7ff);
        int sdm_dv = (reg4 & (3<<12)) >> 12;
        if (sdm & (1<<10)) sdm = sdm | 0xfffff800;
        switch(sdm_dv) {
            case 0: sdm += (int)(16 * 1024); break;
            case 1: sdm += (int)(17 * 1024); break;
            case 2: sdm += (int)(18 * 1024); break;
            case 3: sdm += (int)(16.5 * 2048); break;
        }
        if (sdm == 3)
        	pll = (int)((float)(12000000 / Pre_Div) * (SDM_Fix * sdm / 2048.0));
   		else     	
        pll = (int)((float)(12000000 / Pre_Div) * (SDM_Fix * sdm / 1024.0));
    } else { // fix divider
        int reg3 = MMIO_Read(n*0x10 + 0x94);
        int Num = reg3 & 0x3ff;
        pll = (int)((float)(12000000 / Pre_Div) * Num);
    }
    
    return pll;
}

static int get_pll_clk_output(int pll_src)
{
    int n = 0;
    int output;
    int srcclk;
    int clk;

    switch (pll_src)
    {
        case 0x0: // From PLL1 output1 (default)
        default:
            n = 1;
            output = 1;
            break;
        case 0x1: // From PLL1 output2
            n = 1;
            output = 2;
            break;
        case 0x2: // From PLL2 output1
            n = 2;
            output = 1;
            break;
        case 0x3: // From PLL2 output2
            n = 2;
            output = 2;
            break;
        case 0x4: // From PLL3 output1
            n = 3;
            output = 1;
            break;
        case 0x5: // From PLL3 output2
            n = 3;
            output = 2;
            break;
        case 0x6: // From CKSYS (12MHz)
            clk = CFG_OSC_CLK;
            break;
        case 0x7: // From Ring OSC (200KHz)
            clk = 200000;
            break;
    }

    if (n)
    {
        srcclk = get_pll_clk(n);
        if (output == 1)
            clk = srcclk / (MMIO_Read(n*0x10 + 0x92) & 0x7f);
        else if (output == 2)    
            clk = srcclk / ((MMIO_Read(n*0x10 + 0x92) >> 8) & 0x7f);
    }

    return clk;
}

/*
 * Get System clock in Hz
 */
int or32_getSysCLK(void) {
    unsigned int pll_src;
    unsigned int pll_clk;
    unsigned int div;
    unsigned int sys_clk;
    
    pll_src = (MMIO_Read(0x18) >> 11) & 0x7;
    pll_clk = get_pll_clk_output(pll_src);
    div = (MMIO_Read(0x18) & 0x3FF) + 1; // NCLK_Ratio[9:0]    
    sys_clk = pll_clk / div;

end:     
    //printf("sys_clk: %d Hz\n", sys_clk);
    return sys_clk;
}

/*
 * Get bus clock in Hz
 */
int or32_getBusCLK(void) {
    unsigned int pll_src;
    unsigned int pll_clk;
    unsigned int div;
    unsigned int bus_clk;
    
    pll_src = (MMIO_Read(0x1C) >> 11) & 0x7;
    pll_clk = get_pll_clk_output(pll_src);
    div = (MMIO_Read(0x1C) & 0x3FF) + 1; // WCLK_Ratio[9:0]    
    bus_clk = pll_clk / div;

end:
    //printf("bus_clk: %d Hz\n", bus_clk);
    return bus_clk;
}

/*
 * Set System clock in Hz
 */
void or32_setSysCLK(int clk) {
    int i;
    int target_numerator, current_numerator;
    unsigned int pll_clk;
    unsigned short int mmio_pll      = 0;
    unsigned short int mmio_enReLock = 0;
    unsigned int enRelockPLL_Bits    = 0;

    #if !(defined(MM365) && defined(MM365A0))
    short int pll_setting;
    #endif

    /* Get PLL Clock Source */

#if defined(MM9070) || defined(MM9910)
    unsigned int pll_src = (MMIO_Read(MMIO_NCLK_SRC) & NCLK_SRC) >> NCLK_SRC_BIT;
    unsigned int pll_cfg = (MMIO_Read(MMIO_TRAP) & PLL_CFG) >> PLL_CFG_BIT;

    // IIS PLL Source
    switch(pll_src) {
        case 0x00: // PLL1
            mmio_pll         = MMIO_PLL1;
            mmio_enReLock    = EnRelockPLL1_MMIO;
            enRelockPLL_Bits = EnRelockPLL1_Bits;
            break;
        case 0x01: // PLL2
            mmio_pll         = MMIO_PLL2;
            mmio_enReLock    = EnRelockPLL2_MMIO;
            enRelockPLL_Bits = EnRelockPLL2_Bits;
            break;
        case 0x02: // PLL3
            mmio_pll         = MMIO_PLL3;
            mmio_enReLock    = EnRelockPLL3_MMIO;
            enRelockPLL_Bits = EnRelockPLL3_Bits;
            break;
    }

    switch(pll_cfg) {
        case 0x00:
            #if CHIP_CLOCK_13M
            pll_clk = (13000000/512);
            #elif CHIP_CLOCK_24M
            pll_clk = (24000000/512);
            #else
            pll_clk = (13000000/512);
            #endif
            break;
        case 0x01:
            #if CHIP_CLOCK_13M
            pll_clk = 13000000;
            #elif CHIP_CLOCK_24M
            pll_clk = 24000000;
            #else
            pll_clk = 13000000;
            #endif
            break;
        case 0x02:
        case 0x03:
            pll_clk = 32768;
            break;
        default:
            pll_clk = 0;
    }
    #elif !defined(MM365A0)

    if (!(MMIO_Read(0x168c) & (1<<7))) {
        mmio_pll         = MMIO_PLL2;
        mmio_enReLock    = EnRelockPLL2_MMIO;
        enRelockPLL_Bits = EnRelockPLL2_Bits;
    } else {
        mmio_pll         = MMIO_PLL1;
        mmio_enReLock    = EnRelockPLL1_MMIO;
        enRelockPLL_Bits = EnRelockPLL1_Bits;
    }

    // REG 0x0000 D[15]: 0 -- PLL 32K input
    //                   1 -- PLL 13M input
    // Notes: Only for MM365A1 or later project
    if ((MMIO_Read(MMIO_TRAP) & PLL_CFG) != 0) {
        #if CHIP_CLOCK_13M
        pll_clk = (13000000/512);
        #elif CHIP_CLOCK_24M
        pll_clk = (24000000/512);
        #else
        pll_clk = (13000000/512);
        #endif
    } else {
        pll_clk = (32768);
    }
    #else
    if (!(MMIO_Read(0x168c) & (1<<7))) {
        mmio_pll         = MMIO_PLL2;
        mmio_enReLock    = EnRelockPLL2_MMIO;
        enRelockPLL_Bits = EnRelockPLL2_Bits;
    } else {
        mmio_pll         = MMIO_PLL1;
        mmio_enReLock    = EnRelockPLL1_MMIO;
        enRelockPLL_Bits = EnRelockPLL1_Bits;
    }

    if (0) { /* 13MHz */
        #if CHIP_CLOCK_13M
        pll_clk = (13000000/512);
        #elif CHIP_CLOCK_24M
        pll_clk = (24000000/512);
        #else
        pll_clk = (13000000/512);
        #endif
    } else { /* 32KHz */
        pll_clk = (32768);
    }
    #endif // !defined(P365A0)

    target_numerator = (((clk << 1) / (pll_clk) + 1) >> 1) - 1;
    current_numerator = MMIO_Read(MMIO_PLL2) & MMIO_PLL_MASK;

    // Disable PLL2 relock
    #if !(defined(MM365) && defined(MM365A0))
    pll_setting = MMIO_Read(EnRelockPLL2_MMIO);
    MMIO_Write(EnRelockPLL2_MMIO, (pll_setting | (1<<EnRelockPLL2_Bits)));
    #endif

    // Adjust PLL Clock
    if (current_numerator < target_numerator) {
        while(current_numerator < target_numerator) {
            current_numerator += PLL_STEP;
            if (current_numerator > target_numerator) {
                current_numerator = target_numerator;
            }
            MMIO_Write(MMIO_PLL2, (short int)current_numerator);
            for(i=0; i<20000; i++) /* dummy loop */ ;
        }
    } else {
        while(current_numerator > target_numerator) {
            current_numerator -= PLL_STEP;
            if (current_numerator < target_numerator) {
                current_numerator = target_numerator;
            }
            MMIO_Write(MMIO_PLL2, (short int)current_numerator);
            for(i=0; i<20000; i++) /* dummy loop */ ;
        }
    }

    // restore the relock setting for PLL2
    #if !(defined(MM365) && defined(MM365A0))
    MMIO_Write(EnRelockPLL2_MMIO, pll_setting);
    #endif // !(defined(MM365) && defined(MM365A0))
}

/*
 * Get memory clock in Hz
 */
int or32_getMemCLK(void) {
    unsigned int pll_src;
    unsigned int pll_clk;
    unsigned int div;
    unsigned int mem_clk;
    
    pll_src = (MMIO_Read(0x14) >> 11) & 0x7;
    pll_clk = get_pll_clk_output(pll_src);
    div = (MMIO_Read(0x14) & 0x3FF) + 1; // MCLK_Ratio[9:0]    
    mem_clk = pll_clk / div;

end:
    //printf("mem_clk: %d Hz\n", mem_clk);
    return mem_clk;
}

/*
 * sleep in ticks.
 */
#if PowerSaving == 0
// Sleeping in for loop.
void or32_sleep(unsigned int ticks)
{
    unsigned int i = 0;
    asm volatile("\
1:      l.addi  %0, %0, 0x1\n\
        l.sfleu %1, %0\n\
        l.bnf   1b\n\
        l.nop" : : "r" (i), "r" (ticks/4));
}
#else
// Sleeping in gating clock.
void or32_sleep(unsigned int ticks)
{

    if (ticks == 0) return;

#if defined(MM9070) || defined(MM9910)
    /* switch TTMR2 raise to the external interrupt */
    mtspr(SPR_TTSR, mfspr(SPR_TTSR) | SPR_TTSR_INTS);
#endif

    if (ticks > SPR_TTMR_PERIOD) ticks = SPR_TTMR_PERIOD;

    /* stop timer */
    mtspr(SPR_TTMR2, 0);
    mtspr(SPR_TTCR2, 0);

    /* continus run mode */
    /* do not use the single run mode, else the interrupt ISR
       will stop the timer */
    mtspr(SPR_TTMR2, SPR_TTMR_CR | SPR_TTMR_IE | ((ticks) & SPR_TTMR_PERIOD));

    /* Set the flag to enter the doze mode (for driver) */
    MMIO_Write(DrvAudioStatus, MMIO_Read(DrvAudioStatus) | DrvOR32_Sleep);

    /* enter doze mode */
    or32_doze();

    /* Clear the flag to leave the doze mode (for driver)*/
    MMIO_Write(DrvAudioStatus, MMIO_Read(DrvAudioStatus) & ~DrvOR32_Sleep);

    /* stop timer */
    mtspr(SPR_TTMR2, 0);
    mtspr(SPR_TTCR2, 0);
}
#endif

/*
 * get the ticks in millisecond.
 */
int or32_getTicks_ms (void) {
    if (!tick_in_ms) or32_calcTicks();
    return tick_in_ms;
}

/*
 * get the ticks in microsecond.
 */
int or32_getTicks_us (void) {
    if (!tick_in_us) or32_calcTicks();
    return tick_in_us;
}

/*
 * Delay in microsecond.
 */
void or32_delay_us(unsigned short int us)
{
    or32_sleep(us * or32_getTicks_us());
}

/*
 * Delay in millisecond.
 */
void or32_delay_ms(unsigned short int ms)
{
    or32_sleep(ms * or32_getTicks_ms());
}

/*
 * Caculate the ticks for one millisecond.
 */
void or32_calcTicks(void)
{
    int sysclk = or32_getSysCLK();

    tick_in_ms = sysclk / 1000;
    tick_in_us = tick_in_ms / 1000;
}

/*
 * put the message to the except_str, and raise the system call.
 */
void report_error(const char *errstr) {
    int i;

    _except_ptr = (char *)errstr;

    for(i=0; i<sizeof(_except_str) && errstr[i]!=0; i++) {
        _except_str[i] = errstr[i];
    }
    _except_str[i] = 0;

    asm volatile ("l.trap 15");
}
