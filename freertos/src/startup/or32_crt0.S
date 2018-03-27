
#define __CRT0__

#include "or32.h"
#include "spr_defs.h"
//#include "version.h"

//James mark for build example
//#define HAVE_AUDIO_PROCESSOR

#define MMIO_BASE           0x50000000
#define APB_SET             0x007c
#define AHB_BASE            0x60300000
#define AHB_REMAP           0x0088
#define FLAG_3RD_NANDBOOT   0x500016b2
#define CODEC_ENTRY_OFFSET  0x10

        .extern main
        .extern _stack
        .extern or32_init
        .extern _isr_root_handle

#if defined(HAVE_AUDIO_PROCESSOR)
        .extern codec_api
        .extern codec_start_addr
#endif

#if defined(HAVE_NANDBOOT)
        .extern _NAND_1st_BOOT
        .extern _NAND_2nd_BOOT
#endif

        .extern _exception_stack
        .extern pxCurrentTCB
        .extern timer_init
        .extern uxCriticalNesting

        .global _exit_halt
        .global _bus_error_halt
        .global _data_page_fault_halt
        .global _inst_page_fault_halt
        .global _tick_halt
        .global _alignment_halt
        .global _illegal_inst_halt
        .global _int_halt
        .global _dtlb_miss_halt
        .global _itlb_miss_halt
        .global _range_halt
        .global _sys_call_halt
        .global _trap_halt

###########################################################################
## Macro
###########################################################################

#define ENTRY(symbol) \
        .global symbol ; \
symbol:

#define FUNC_BEGIN(symbol) \
        .global symbol; \
        .proc symbol; \
        .type symbol, @function ; \
symbol:

#define FUNC_END(symbol) \
        .size   symbol, .-symbol

        #############################################
        # Install Exception Handler
        #############################################
        .macro  INSTALL_EXCEPTION isr_n
        l.addi  r1,r1, CTXT_BYTES_OFFSET_FROM_SP
        l.sw    0x14(r1), r4
        l.sw    0x2C(r1), r10
        l.movhi r10,hi(_isr_root_handle)
        l.ori   r10,r10,lo(_isr_root_handle)
        l.addi  r4,r0,(\isr_n-1)*8
        l.add   r10,r10,r4
        l.lwz   r4,0(r10)
        l.j     exception_handler
        l.nop
        .endm

        #############################################
        # Save context
        #############################################
        .macro  SAVE_CONTEXT
        l.mfspr r10, r0, SPR_EPCR_BASE
#if 0
        # EPCR points to the l.trap instruction, but
        # the task should be resumed from the next
        # insruction slot.

        # If this macro is used in l.sys or ISR, EPCR points
        # to the next non executed instruction. This
        # increment is not necessary.

        l.addi  r10, r10, 4
#endif

        l.sw    CTXT_OFFSET_PC(r1), r10

        l.mfspr r10, r0, SPR_ESR_BASE
        l.sw    CTXT_OFFSET_SR(r1), r10

        l.sw    0x0c(r1), r2
        l.sw    0x10(r1), r3
        # r4 was saved.
        l.sw    0x18(r1), r5
        l.sw    0x1c(r1), r6
        l.sw    0x20(r1), r7
        l.sw    0x24(r1), r8
        l.sw    0x28(r1), r9
        # r10 was saved.
        l.sw    0x30(r1), r11
        l.sw    0x34(r1), r12
        l.sw    0x38(r1), r13
        l.sw    0x3c(r1), r14
        l.sw    0x40(r1), r15
        l.sw    0x44(r1), r16
        l.sw    0x48(r1), r17
        l.sw    0x4c(r1), r18
        l.sw    0x50(r1), r19
        l.sw    0x54(r1), r20
        l.sw    0x58(r1), r21
        l.sw    0x5c(r1), r22
        l.sw    0x60(r1), r23
        l.sw    0x64(r1), r24
        l.sw    0x68(r1), r25
        l.sw    0x6c(r1), r26
        l.sw    0x70(r1), r27
        l.sw    0x74(r1), r28
        l.sw    0x78(r1), r29
        l.sw    0x7c(r1), r30
        l.sw    0x80(r1), r31

        l.movhi r10, hi(uxCriticalNesting)
        l.ori   r10, r10, lo(uxCriticalNesting)
        l.lwz   r10, 0x0(r10)
        l.sw    0x84(r1), r10

        l.addi  r1, r1, -CTXT_BYTES_OFFSET_FROM_SP

        l.movhi r10, hi(pxCurrentTCB)
        l.ori   r10, r10, lo(pxCurrentTCB)
        l.lwz   r10, 0x0(r10)
        l.sw    0(r10), r1
        .endm

        #############################################
        # Restore context
        #############################################
        .macro  RESTORE_CONTEXT
        l.movhi r3, hi(pxCurrentTCB)
        l.ori   r3, r3, lo(pxCurrentTCB)
        l.lwz   r3, 0x0(r3)
        l.lwz   r1, 0x0(r3)

        l.addi  r1, r1, CTXT_BYTES_OFFSET_FROM_SP

        l.movhi r3, hi(uxCriticalNesting)
        l.ori   r3, r3, lo(uxCriticalNesting)
        l.lwz   r4, 0x84(r1)
        l.sw    0(r3), r4

        l.lwz   r3, CTXT_OFFSET_PC(r1)
        l.mtspr r0, r3, SPR_EPCR_BASE
        l.lwz   r3, CTXT_OFFSET_SR(r1)
        l.mtspr r0, r3, SPR_ESR_BASE

        l.lwz   r2,0x0c(r1)
        l.lwz   r3,0x10(r1)
        l.lwz   r4,0x14(r1)
        l.lwz   r5,0x18(r1)
        l.lwz   r6,0x1c(r1)
        l.lwz   r7,0x20(r1)
        l.lwz   r8,0x24(r1)
        l.lwz   r9,0x28(r1)
        l.lwz   r10,0x2c(r1)
        l.lwz   r11,0x30(r1)
        l.lwz   r12,0x34(r1)
        l.lwz   r13,0x38(r1)
        l.lwz   r14,0x3c(r1)
        l.lwz   r15,0x40(r1)
        l.lwz   r16,0x44(r1)
        l.lwz   r17,0x48(r1)
        l.lwz   r18,0x4c(r1)
        l.lwz   r19,0x50(r1)
        l.lwz   r20,0x54(r1)
        l.lwz   r21,0x58(r1)
        l.lwz   r22,0x5c(r1)
        l.lwz   r23,0x60(r1)
        l.lwz   r24,0x64(r1)
        l.lwz   r25,0x68(r1)
        l.lwz   r26,0x6c(r1)
        l.lwz   r27,0x70(r1)
        l.lwz   r28,0x74(r1)
        l.lwz   r29,0x78(r1)
        l.lwz   r30,0x7c(r1)
        l.lwz   r31,0x80(r1)
        l.addi  r1, r1, -CTXT_BYTES_OFFSET_FROM_SP
        .endm

###########################################################################
## Begin of RAM
###########################################################################
        .section .reset, "ax"
        .org    0x000

1:      l.j     1b      # forever loop
        l.nop   0x02

FUNC_BEGIN(_remap)
        /* Disable IC */
        l.addi  r1,r0,SPR_SR
        l.mfspr r3,r1,0
        l.addi  r4,r0,~SPR_SR_ICE
        l.and   r2,r3,r4
        l.mtspr r1,r2,0

        #Remap Memory
        l.movhi r1,hi(AHB_BASE)
        l.ori   r1,r1,lo(AHB_REMAP)
        l.addi  r2,r0,0x1
        l.sw    0x0(r1),r2
        l.nop   0x168           # 3 delay slots to remap memory.
        l.nop   0x168           # Fetch+decode+execute pipeline statge.
        l.nop   0x168
        l.nop   0x168           # one more delay slot for safty.

        l.j     _reset
        l.nop   0x0
FUNC_END(_remap)


########################################
# Pointer of Print Buffer
########################################
        .org    0xcc
        .global __dbgmsg_buf_ptr
__dbgmsg_buf_ptr:
        .long   0
        .global __dbgmsg_buf_len
__dbgmsg_buf_len:
        .long   0
        .global __dbgmsg_idx_ptr
__dbgmsg_idx_ptr:
        .long   0

########################################
# Pointer of Version Information
########################################

#        .org    0xe0
#        .global __dpf_version_num
#__dpf_version_num:
#        .long   DPF_VERSION_NUM

#        .global __dtv_res_noraddr
#__dtv_res_noraddr:
#        .long   0

#        .global __dtv_rom_size
#__dtv_rom_size:
#        .long   0
        
#        .global __dtv_boot_noraddr
#__dtv_boot_noraddr:
#        .long   0

###########################################################################
## Reset section
###########################################################################
        .org    0x100
ENTRY(_start)
ENTRY(_reset)
# The default setting of RISC is big endian, use following object code
# (in big endian) to set the RISC as little endian.
#if defined(__SM32_LITTLE_ENDIAN)
        .byte   0x18,0x00,0x00,0x00     # l.movhi r0,0x0
        .byte   0xa8,0x00,0x00,0x00     # l.ori   r0,r0,0x0
        .byte   0x9c,0x20,0x00,0x80     # l.addi  r1,r0,0x80
        .byte   0xc0,0x00,0x08,0x11     # l.mtspr r0,r1,0x11
        .byte   0x15,0x00,0x00,0x15     # l.nop   0x15
        .byte   0x15,0x00,0x00,0x15     # l.nop   0x15
        .byte   0x15,0x00,0x00,0x15     # l.nop   0x15
        .byte   0x15,0x00,0x00,0x15     # l.nop   0x15
#endif
        l.movhi r0, 0x0
        l.ori   r0,r0,0x0
        l.addi  r1,r0,0x0
        l.addi  r2,r0,0x0
        l.addi  r3,r0,0x0
        l.addi  r4,r0,0x0
        l.addi  r5,r0,0x0
        l.addi  r6,r0,0x0
        l.addi  r7,r0,0x0
        l.addi  r8,r0,0x0
        l.addi  r9,r0,0x0
        l.addi  r10,r0,0x0
        l.addi  r11,r0,0x0
        l.addi  r12,r0,0x0
        l.addi  r13,r0,0x0
        l.addi  r14,r0,0x0
        l.addi  r15,r0,0x0
        l.addi  r16,r0,0x0
        l.addi  r17,r0,0x0
        l.addi  r18,r0,0x0
        l.addi  r19,r0,0x0
        l.addi  r20,r0,0x0
        l.addi  r21,r0,0x0
        l.addi  r22,r0,0x0
        l.addi  r23,r0,0x0
        l.addi  r24,r0,0x0
        l.addi  r25,r0,0x0
        l.addi  r26,r0,0x0
        l.addi  r27,r0,0x0
        l.addi  r28,r0,0x0
        l.addi  r29,r0,0x0
        l.addi  r30,r0,0x0
        l.addi  r31,r0,0x0

        # Normal Function
        l.movhi r3,hi(_start_proc)
        l.ori   r3,r3,lo(_start_proc)
        l.jr    r3
        l.addi  r3, r0, 0x0

###########################################################################
## Exit Procedure
###########################################################################
.global _exit
FUNC_BEGIN(_exit)
        l.trap  0xf     # Software break point

        # SHOULD NOT REACH HERE
_exit_halt:
        l.j     _exit_halt
        l.nop   0x01
FUNC_END(_exit)

###########################################################################
## Bus Error
###########################################################################
        .org    0x200
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(bus_error)

        INSTALL_EXCEPTION (0x2)

_bus_error_halt:
        RESTORE_CONTEXT
1:      l.j     1b
        l.nop   0x02
FUNC_END(bus_error)

###########################################################################
## Data Page Fault
###########################################################################
        .org    0x300
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(data_page_fault)
_data_page_fault_halt:
        l.j     _data_page_fault_halt
        l.nop   0x03
FUNC_END(data_page_fault)

###########################################################################
## Instructions Page Fault
###########################################################################
        .org    0x400
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(inst_page_fault)
_inst_page_fault_halt:
        l.j     _inst_page_fault_halt
        l.nop   0x04
FUNC_END(inst_page_fault)

###########################################################################
## Tick Timer Interrupt
###########################################################################
        .org    0x500
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(tick_timer)

        INSTALL_EXCEPTION (0x5)

        # SHOULD NOT REACH HERE
_tick_halt:
        l.j     _tick_halt
        l.nop   0x05
FUNC_END(tick_timer)

###########################################################################
## Alignment
###########################################################################
        .org    0x600
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(alignment)

        INSTALL_EXCEPTION (0x6)

_alignment_halt:
        RESTORE_CONTEXT
1:      l.j     1b
        l.nop   0x02
FUNC_END(alignment)

###########################################################################
## Illegal instructions
###########################################################################
        .org    0x700
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(illegal_inst)

        INSTALL_EXCEPTION (0x7)

_illegal_inst_halt:
        RESTORE_CONTEXT
1:      l.j     1b
        l.nop   0x02
FUNC_END(illegal_inst)

###########################################################################
## External Interrupt handler
###########################################################################
        .org    0x800
        l.nop   0x0         # Two of NOPs is required, it can not be removed.
        l.nop   0x0
FUNC_BEGIN(_int)

        INSTALL_EXCEPTION (0x8)

        # SHOULD NOT REACH HERE
_int_halt:
        l.j     _int_halt
        l.nop   0x08
FUNC_END(_int)

###########################################################################
## D-TLB Miss
###########################################################################
        .org    0x900
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(dtlb_miss)
_dtlb_miss_halt:
        l.j     _dtlb_miss_halt
        l.nop   0x09
FUNC_END(dtlb_miss)

###########################################################################
## I-TLB Miss
###########################################################################
        .org    0xa00
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(itlb_miss)
_itlb_miss_halt:
        l.j     _itlb_miss_halt
        l.nop   0x0a
FUNC_END(itlb_miss)

###########################################################################
## Arithmetic overflow
###########################################################################
        .org    0xb00
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_range)
_range_halt:
        l.j     _range_halt
        l.nop   0x0b
FUNC_END(_range)

###########################################################################
## System call
###########################################################################
        .org    0xc00
        # HACK: These two nop's are required. So far nobody knows why,
        # keep them here!
        l.nop
        l.nop
FUNC_BEGIN(_sys_call)

        INSTALL_EXCEPTION (0xC)

        # SHOULD NOT REACH HERE
_sys_call_halt:
        l.j     _sys_call_halt
        l.nop   0x0c
FUNC_END(_sys_call)

###########################################################################
## Instruction l.trap or debug unit exeception
###########################################################################
        .org    0xe00
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_trap)

        INSTALL_EXCEPTION (0xe)

_trap_halt:
        RESTORE_CONTEXT
1:      l.j     1b
        l.nop   0x02
FUNC_END(_trap)

#if defined(HAVE_NANDBOOT)
        .section .boot
#else
        .section .text
#endif
###########################################################################
## Exception Handler
###########################################################################
exception_handler:
        SAVE_CONTEXT

        l.movhi r1, hi(_exception_stack)
        l.ori   r1, r1, lo(_exception_stack)

        # JUMP_TO_EXCEPTION_HANDLER
        l.jalr  r4
        l.nop

        RESTORE_CONTEXT

        l.rfe
        l.nop 0

###########################################################################
## Start Procedure
###########################################################################
ENTRY(_start_proc)
#if defined(HAVE_AUDIO_PROCESSOR)
#
# Check the processor number
#
        l.addi  r5,r0,SPR_VR
        l.mfspr r4,r5,0
        l.andi  r3,r4,SPR_VER_N
        l.sfeqi r3,0
        l.bf    .proc1
        l.nop

#
# Processor 2, for audio codec
# _header->entry_point(&codec_api));
#
.proc2:
        # Enable Cache
        l.jal   ic_enable
        l.nop
        l.jal   dc_enable
        l.nop

        # Set the return address to _exit
        l.movhi r9,hi(exit)
        l.ori   r9,r9,lo(exit)

        l.movhi r1,hi(codec_stack)
        l.ori   r1,r1,lo(codec_stack)
        l.addi  r2, r1, 0x0

        l.movhi r5,hi(codec_start_addr)
        l.ori   r5,r5,lo(codec_start_addr)
        l.movhi r3,hi(codec_api)
        l.ori   r3,r3,lo(codec_api)
        l.lwz   r4,CODEC_ENTRY_OFFSET(r5)

        l.jalr  r4
        l.nop

# never reach here
1:      l.j     1b
        l.nop

#
# Processor 1
#
.proc1:
#endif

#if defined(HAVE_NANDBOOT)
        # Test remap, ((*(volatile int*)0x60300000 & 0x1) == 0x0)
        l.movhi r6,hi(AHB_BASE)
        l.ori   r6,r6,lo(AHB_REMAP)
        l.lwz   r5,0(r6)
        l.xori  r4,r5,1
        l.andi  r3,r4,1
        l.sfeqi r3,0
        l.bf    1f
        l.nop   # nop delay slot

        # Initial the Tick Timer 2 with signal run mode
        # for benchmarking the performance of booting.
        l.movhi r3,hi(SPR_TTMR_SR | SPR_TTMR_PERIOD)
        l.ori   r3,r3,lo(SPR_TTMR_SR | SPR_TTMR_PERIOD)
        l.addi  r4,r0,lo(SPR_TTMR2)
        l.mtspr r4,r3,0

        # Test BOOT_CFG, ((*(volatile short int*)0x50000000 & 0x3) == 0x2)
        l.movhi r6,hi(MMIO_BASE)
        l.lhz   r5,0(r6)
        l.andi  r4,r5,3
        l.sfnei r4,2
        l.bf    1f
        l.nop

        # Enable Cache
        l.jal   ic_enable
        l.nop
        l.jal   dc_enable
        l.nop

        # Nand BOOT
        l.jal   _NAND_1st_BOOT
        l.nop
        l.j     _remap
        l.nop
#endif

1:      # Stack pointer
        l.movhi r1,hi(_stack)
        l.ori   r1,r1,lo(_stack)
        l.addi  r2, r1, 0x0

        l.movhi r3,hi(ic_enable)
        l.ori   r3,r3,lo(ic_enable)
        l.jalr  r3
        l.nop

        l.movhi r3,hi(dc_enable)
        l.ori   r3,r3,lo(dc_enable)
        l.jalr  r3
        l.nop

#if defined(HAVE_NANDBOOT)
        # Test BOOT_CFG, ((*(volatile short int*)0x50000000 & 0x3) == 0x2)
        l.movhi r6,hi(MMIO_BASE)
        l.lhz   r5,0(r6)
        l.andi  r4,r5,3
        l.sfnei r4,2
        l.bf    2f
        l.nop   # nop delay slot

        # Skip the 2nd NAND booting if ((*(volatile short int*)0x50001698) != 0)
        l.movhi r6,hi(FLAG_3RD_NANDBOOT)
        l.ori   r6,r6,lo(FLAG_3RD_NANDBOOT)
        l.lhz   r5,0(r6)
        l.sfnei r5,0
        l.bf    2f
        l.nop

        # set the flag (*(volatile short int*)0x50001698) = 1 for 3rd NAND booting
        l.movhi r6,hi(FLAG_3RD_NANDBOOT)
        l.ori   r6,r6,lo(FLAG_3RD_NANDBOOT)
        l.addi  r4,r0,1
        l.sh    0(r6),r4

        l.jal   _NAND_2nd_BOOT
        l.nop
        l.j     3f
2:
        # Clear flag (*(volatile short int*)0x50001698) = 0 for 3rd NAND booting
        l.movhi r6,hi(FLAG_3RD_NANDBOOT)
        l.ori   r6,r6,lo(FLAG_3RD_NANDBOOT)
        l.sh    0(r6),r0
3:
#endif

        # Zero the BSS section
        l.movhi r3,hi(_bss_start)
        l.ori   r3,r3,lo(_bss_start)
        l.movhi r4,hi(_bss_end-4)
        l.ori   r4,r4,lo(_bss_end-4)
4:
        l.sw    0(r3),r0
        l.sfltu r3, r4
        l.bf    4b
        l.addi  r3,r3,4

        # OR32 Init
        l.jal   or32_init
        l.nop

        # Set the return address to _exit
        l.movhi r9,hi(exit)
        l.ori   r9,r9,lo(exit)

        # Jump to _main procedure
        l.movhi r2,hi(main)
        l.ori   r2,r2,lo(main)
        l.jr    r2
        l.addi  r2, r1, 0x0

###########################################################################
## Enable I-Cache, and invalidate all of I-Cache entry.
##
## Code generate by GCC 3.4.4. with -O3 -fomit-frame-pointer options.
##
## void or32_ic_invalidate(void) {
##     int ncs, bs;
##     int cache_size, cache_line_size;
##     int i = 0;
##
##     // Disable IC
##     mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_ICE);
##
##     // Number of cache sets
##     ncs = (mfspr(SPR_ICCFGR) >> 3) & 0xf;
##
##     // Number of cache block size
##     bs  = ((mfspr(SPR_ICCFGR) >> 7) & 0x1) + 4;
##
##     // Calc Cache size
##     cache_line_size = 1 << bs;
##     cache_size      = 1 << (ncs+bs);
##
##     // Flush IC
##     do {
##         mtspr(SPR_ICBIR, i);
##         i += cache_line_size;
##     } while (i < cache_size);
##
##     // Enable IC
##     mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_ICE);
##     asm volatile("l.nop 0x0\nl.nop 0x0\nl.nop 0x0\nl.nop 0x0\n");
## }
###########################################################################
.global ic_invalidate
.global ic_enable
.align  0x100
FUNC_BEGIN(ic_invalidate)
FUNC_BEGIN(ic_enable)
        l.addi  r3,r0,0

        /* Disable IC */
        l.addi  r21,r0,SPR_SR
        l.mfspr r25,r21,0
        l.addi  r27,r0,~SPR_SR_ICE
        l.and   r23,r25,r27
        l.mtspr r21,r23,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        /* Calc Cache size */
        l.addi  r15,r0,SPR_ICCFGR
        l.mfspr r19,r15,0
        l.srli  r17,r19,3
        l.andi  r5,r17,0xf
        l.mfspr r6,r15,0
        l.srli  r13,r6,7
        l.addi  r8,r0,1
        l.and   r11,r13,r8
        l.addi  r6,r0,SPR_ICBIR
        l.addi  r4,r11,4
        l.add   r7,r5,r4
        l.sll   r5,r8,r4
        l.sll   r4,r8,r7
        l.j     1f
        l.nop

        /* For the NAND booting, the NAND controller prefetch 64 bytes as buffering. */
        /* So it would be 64 bytes boundary on the loop to prevent multi-access the  */
        /* NAND data if it cross 64 bytes boundary                                   */
        .align  0x40

        /* Flush IC */
1:
        l.mtspr r6,r3,0
        l.add   r3,r3,r5
        l.sfges r3,r4
        l.bnf   1b

        /* Enable IC */
        l.addi  r29,r0,SPR_SR
        l.mfspr r31,r29,0
        l.ori   r3,r31,SPR_SR_ICE
        l.mtspr r29,r3,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(ic_enable)

###########################################################################
## Disable I Cache
###########################################################################
.global ic_disable
FUNC_BEGIN(ic_disable)
        /* Disable IC */
        l.mfspr r4,r0,SPR_SR
        l.addi  r3,r0,-1
        l.xori  r3,r3,SPR_SR_ICE
        l.and   r3,r4,r3
        l.mtspr r0,r3,SPR_SR
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(ic_disable)

###########################################################################
## Enable D-Cache, and invalidate all of D-Cache entry.
##
## Code generate by GCC 3.4.4. with -O3 -fomit-frame-pointer options.
##
## void or32_dc_invalidate(void) {
##     int ncs, bs;
##     int cache_size, cache_line_size;
##     int i = 0;
##
##     // Disable DC
##     mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_DCE);
##
##     // Number of cache sets
##     ncs = ((mfspr(SPR_DCCFGR) >> 3) & 0xf);
##
##     // Number of cache block size
##     bs  = ((mfspr(SPR_DCCFGR) >> 7) & 0x1) + 4;
##
##     // Calc Cache size
##     cache_line_size = 1 << bs;
##     cache_size      = 1 << (ncs+bs);
##
##     // Flush DC
##     do {
##         mtspr(SPR_DCBIR, i);
##         i += cache_line_size;
##     } while(i < cache_size);
##
##     // Enable DC
##     mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_DCE);
##     asm volatile("l.nop 0x0\nl.nop 0x0\nl.nop 0x0\nl.nop 0x0\n");
## }
###########################################################################
.global dc_invalidate
.global dc_enable
FUNC_BEGIN(dc_invalidate)
FUNC_BEGIN(dc_enable)
        l.addi  r3,r0,0

        /* Disable DC */
        l.addi  r21,r0,SPR_SR
        l.mfspr r25,r21,0
        l.addi  r27,r0,~SPR_SR_DCE
        l.and   r23,r25,r27
        l.mtspr r21,r23,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        /* Calc Cache size */
        l.addi  r15,r0,SPR_DCCFGR
        l.mfspr r19,r15,0
        l.srli  r17,r19,3
        l.andi  r5,r17,0xf
        l.mfspr r6,r15,0
        l.srli  r13,r6,7
        l.addi  r8,r0,1
        l.and   r11,r13,r8
        l.addi  r6,r0,SPR_DCBIR
        l.addi  r4,r11,4
        l.add   r7,r5,r4
        l.sll   r5,r8,r4
        l.sll   r4,r8,r7

        /* Flush IC */
1:
        l.mtspr r6,r3,0
        l.add   r3,r3,r5
        l.sfges r3,r4
        l.bnf   1b

        /* Enable DC */
        l.addi  r29,r0,SPR_SR
        l.mfspr r31,r29,0
        l.ori   r3,r31,SPR_SR_DCE
        l.mtspr r29,r3,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(dc_invalidate)
FUNC_END(dc_enable)

###########################################################################
## Disable D Cache
###########################################################################
.global dc_disable
FUNC_BEGIN(dc_disable)
        /* Disable DC */
        l.mfspr r4,r0,SPR_SR
        l.addi  r3,r0,-1
        l.xori  r3,r3,SPR_SR_DCE
        l.and   r3,r4,r3
        l.mtspr r0,r3,SPR_SR
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(dc_disable)

###########################################################################
## void crt0_memcpy32(char* dst, char* src, unsigned int length_in_byte)
###########################################################################
FUNC_BEGIN(crt0_memcpy32)
        l.sflesi    r5,0
        l.bf        2f
        l.nop
1:
        l.lwz       r6,0(r4)
        l.addi      r5,r5,-4
        l.sw        0(r3),r6
        l.addi      r4,r4,4
        l.sflesi    r5,0
        l.bnf       1b
        l.addi      r3,r3,4

2:
        l.movhi     r3,hi(_start)
        l.ori       r3,r3,lo(_start)
        l.jr        r3
        l.nop
FUNC_END(crt0_memcpy32)

.global crt0_memcpy32_end
crt0_memcpy32_end:
