/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for Audio engine
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#if 0

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "spr_defs.h"
#include "proj_defs.h"
#include "sys.h"
#include "or32.h"

#ifndef VOLATILE
//#define VOLATILE(A) {asm volatile(""); {A}; asm volatile(""); }
#define VOLATILE(A) { A }
#endif

/* Constan Declaration for DMA Engine */

/* MMIO Mapping (engine parameter) */
#define MISC     0x80000024L
#define ALU_OP0  0x80000028L
#define RQ_TYPE  0x80000030L
#define INIT_ACC 0x80000034L
#define UsrDefC0 0x80000038L
#define UsrDefC1 0x8000003CL
#define Src0Base 0x80000040L
#define Src1Base 0x80000044L
#define Src2Base 0x80000048L
#define Src3Base 0x8000004CL
#define Dst0Base 0x80000050L
#define Dst1Base 0x80000054L
#define Dst2Base 0x80000058L
#define Dst3Base 0x8000005CL
#define Rom0Type 0x80000060L
#define Rom1Type 0x80000064L
#define Rom2Type 0x80000068L
#define Rom3Type 0x8000006CL

/* MMIO Mapping (engine status, read only) */
#define ALU_P0   0x80000070L
#define ALU_P1   0x80000074L
#define ALU_P2   0x80000078L
#define ALU_P3   0x8000007CL
#define STATUS   0x80000080L

#define TRUE     1
#define FALSE    0

/* Bit declaration for ALU VLIW Instruction */
#define P_Reserved_0    31
#define P_BP            30
#define P_SAdd2         27
#define P_SAdd3         24
#define P_Mul0L         20
#define P_Mul0R         16
#define P_Mul1L         12
#define P_Mul1R          8
#define P_Mul2L          4
#define P_Mul2R          0

#define P_Mul3L         28
#define P_Mul3R         24
#define P_SAdd0         20
#define P_SAdd1         16
#define P_Reserved_1    12
#define P_Dst0           9
#define P_Dst1           6
#define P_Dst2           3
#define P_Dst3           0

/* Bit declaration for PARAM */
#define P_DMAClrInt      17
#define P_SoftInt        16
#define P_EnINT          1
#define P_EnWrCache      0

/* Bit declaration for RQ type */
#define P_Fire          31
#define P_EnInitAcc     30
#define P_Sat           29
#define P_RdDT          28
#define P_WrDT          27
#define P_RdGranSize    26
#define P_RdGranule     24
#define P_WrGranSize    23
#define P_WrGranule     21
#define P_Len           10
#define P_RdIncr         5
#define P_WrIncr         0

/* Bit declaration for Rd/Wr Decrement */
#define P_RdDec         31
#define P_WrDec         31

/* Bit declaration for RQ type */
#define P_ROMINCR       17
#define P_ROMSEL        11
#define P_ROMBASE        0

/* Constant for Data Type */
#define DATA16           0
#define DATA32           1

/* Contant for Shift */
#define NoSHIFT          0
#define R_SHIFT1         1
#define R_SHIFT2         2
#define R_SHIFT3         3
#define R_SHIFT6         4
#define L_SHIFT1         5
#define L_SHIFT10        6
#define R_SHIFT12        7
#define R_SHIFT14        8
#define R_SHIFT15        9
#define R_SHIFT16       10
#define R_SHIFT28       11
#define R_SHIFT29       12
#define R_SHIFT30       13
#define R_SHIFT31       14
#define R_SHIFT32       15

/* Constant for Source Type */
#define SRC_RomA         0
#define SRC_RomB         1
#define SRC_RomC         2
#define SRC_RomD         3
#define SRC_FIFOA        4
#define SRC_FIFOB        5
#define SRC_FIFOC        6
#define SRC_FIFOD        7
#define SRC_C0           8
#define SRC_C1           9
#define SRC_C_1         10
#define SRC_UDC0        14
#define SRC_UDC1        15

/* Dest FIFO */
#define DST_NoWr        0
#define DST_P0          4
#define DST_P1          5
#define DST_P2          6
#define DST_P3          7

/* Granule size */
#define GRANULE_1       0
#define GRANULE_2       1
#define GRANULE_3       2
#define GRANULE_4       3
#define NONE            2

#define SMALL           0
#define LARGE           1
#define NoSrc           0
#define NoDst           0

/* Constant for ROM table */
/* -- ROM0 -- */
#define ROM0_MP3_costab36_0             0
#define ROM0_MP3_dctiitab_0             1
#define ROM0_MP3_dtab_0                 2
#define ROM0_MP3_costab12_0             3
#define ROM0_MP3_cs                     4
#define ROM0_MP3_freqinv_a              5
#define ROM0_AAC_kbd_short_128          6

/* -- ROM1 -- */
#define ROM1_MP3_costab36_1             0
#define ROM1_MP3_dctiitab_1             1
#define ROM1_MP3_dtab_1                 2
#define ROM1_MP3_costab12_1             3
#define ROM1_MP3_window_ma              4
#define ROM1_MP3_window_ma_freqinv      5
#define ROM1_MP3_window_la              6
#define ROM1_MP3_window_la_freqinv      7

/* -- ROM2 -- */
#define ROM2_MP3_costab36_2             0
#define ROM2_MP3_dctiitab_2             1
#define ROM2_MP3_dtab_2                 2
#define ROM2_MP3_costab12_2             3
#define ROM2_MP3_ca                     4
#define ROM2_MP3_freqinv_b              5
#define ROM2_AAC_sine_short_128         6

/* -- ROM3 -- */
#define ROM3_MP3_costab36_3             0
#define ROM3_MP3_dctiitab_3             1
#define ROM3_MP3_dtab_3                 2
#define ROM3_MP3_costab12_3             3
#define ROM3_MP3_window_mb              4
#define ROM3_MP3_window_mb_freqinv      5
#define ROM3_MP3_window_lb              6
#define ROM3_MP3_window_lb_freqinv      7

/* MMIO 0x8000_0080 Status */
#ifndef DMA_IDLE
#define DMA_IDLE         0x00000001L
#define I2S_InEmpty      0x00000002L
#define I2S_OutEmpty     0x00000004L
#endif

#if !defined(PowerSaving)
#error "No define the power saving mode"
#elif PowerSaving == 0
#warning "Disable the power saving mode"
#endif

/*
  On the MM365A0, it can not enable gating clock when it uses the tick timer
  to calculate cycle time.
*/
#if defined (ENABLE_PERFORMANCE_MEASURE)
#   if defined (MM365A0)
#       undef PowerSaving
#       define PowerSaving 0
#   endif
#endif

#if defined(HAVE_INTISR)
#define ClrInt() (*(volatile int *)MISC = (*(volatile int *)MISC | (1<<P_DMAClrInt)))
#else
#define ClrInt()
#endif

#define WaitEngineIdle() while((*(volatile int *)STATUS & DMA_IDLE) == 0)

#if PowerSaving == 1
#   if defined (MM365A0)
#       define WaitEngineDone() mtspr(SPR_PMR, SPR_PMR_DME); asm volatile ("l.nop\t0x168"); WaitEngineIdle(); ClrInt(); asm volatile ("l.nop\t0x188\nl.nop\t0x188\nl.nop\t0x188")
#   else
#       define WaitEngineDone() mtspr(SPR_PMR, SPR_PMR_DME); asm volatile ("l.nop\t0x168"); WaitEngineIdle(); ClrInt()
#   endif
#else
#   if defined (MM365A0)
#       define WaitEngineDone() WaitEngineIdle(); asm volatile ("l.nop\t0x188\nl.nop\t0x188\nl.nop\t0x188\nl.nop\t0x188\nl.nop\t0x188")
#   else
#       define WaitEngineDone() WaitEngineIdle()
#   endif
#endif

/////////////////////////////////////////////////////////////////
//                      Inline Function
/////////////////////////////////////////////////////////////////
static __inline void *memmov32 (void *dst, const void *src, unsigned int num) {
  char *newdst = (char *) dst + ((num-1)<<2);
  char *newsrc = (char *) src + ((num-1)<<2);

  if (num == 0) return dst;

  if (num >= (1<<11)) {
    asm volatile("l.trap 15");
    return 0;
  }

  VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (TRUE      << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (NoSHIFT   << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Src0Base) = (TRUE << P_RdDec) | ((int) newsrc);
  *(volatile int *)(Dst0Base) = (TRUE << P_RdDec) | ((int) newdst);

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA32    << P_RdDT      ) |
                                (DATA32    << P_WrDT      ) |
                                (LARGE     << P_RdGranSize) |
                                (GRANULE_1 << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (num       << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;

  WaitEngineDone();
  );

  return dst;
}

static __inline void *memmov16 (void *dst, const void *src, unsigned int num) {
  char *newdst = (char *) dst + ((num-1)<<1);
  char *newsrc = (char *) src + ((num-1)<<1);

  if (num == 0) return dst;

  if (num >= (1<<11)) {
    asm volatile("l.trap 15");
    return 0;
  }

  VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (TRUE      << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (NoSHIFT   << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Src0Base) = (TRUE << P_RdDec) | ((int) newsrc);
  *(volatile int *)(Dst0Base) = (TRUE << P_RdDec) | ((int) newdst);

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA16    << P_RdDT      ) |
                                (DATA16    << P_WrDT      ) |
                                (LARGE     << P_RdGranSize) |
                                (GRANULE_1 << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (num       << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;

  WaitEngineDone();
  );
}

static __inline void *memcpy32 (void *dst, const void *src, unsigned int num) {

  if (num == 0) return dst;

  if (num >= (1<<11)) {
    asm volatile("l.trap 15");
    return 0;
  }

  VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (TRUE      << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (NoSHIFT   << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) src);
  *(volatile int *)(Dst0Base) = (FALSE << P_RdDec) | ((int) dst);

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA32    << P_RdDT      ) |
                                (DATA32    << P_WrDT      ) |
                                (LARGE     << P_RdGranSize) |
                                (GRANULE_1 << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (num       << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;

  WaitEngineDone();
  );

  return dst;
}

static __inline void *memcpy16 (void *dst, const void *src, unsigned int num) {

  if (num == 0) return dst;

  if (num >= (1<<11)) {
    asm volatile("l.trap 15");
    return 0;
  }

  VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (TRUE      << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (NoSHIFT   << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) src);
  *(volatile int *)(Dst0Base) = (FALSE << P_RdDec) | ((int) dst);

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA16    << P_RdDT      ) |
                                (DATA16    << P_WrDT      ) |
                                (LARGE     << P_RdGranSize) |
                                (GRANULE_1 << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (num       << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;

  WaitEngineDone();
  );

  return dst;
}

static __inline void *memset32 (void *dst, const int c, unsigned int num) {
  int i;
  int len = 0;
  void *new_dst = dst;

  for(i = 0; i<num; i+=2047) {

  len = i+2047;
  len = (len > num) ? (num - i) : 2047;

  VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (TRUE      << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_UDC0  << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (NoSHIFT   << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Dst0Base) = (FALSE << P_RdDec) | ((int) new_dst);

  *(volatile int *)(UsrDefC0) = c;

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA32    << P_RdDT      ) |
                                (DATA32    << P_WrDT      ) |
                                (NoSrc     << P_RdGranSize) |
                                (NONE      << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (len       << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;
  WaitEngineDone();
  );

  new_dst += len * 4;
  }

  return dst;
}

static __inline void *memset16 (void *dst, const int c, unsigned int num) {
  int i;
  int len = 0;
  void *new_dst = dst;

  for(i = 0; i<num; i+=2047) {

  len = i+2047;
  len = (len > num) ? (num - i) : 2047;

  VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (TRUE      << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_UDC0  << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (NoSHIFT   << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Dst0Base) = (FALSE << P_RdDec) | ((int) new_dst);

  *(volatile int *)(UsrDefC0) = c;

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA16    << P_RdDT      ) |
                                (DATA16    << P_WrDT      ) |
                                (NoSrc     << P_RdGranSize) |
                                (NONE      << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (len       << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;
  WaitEngineDone();
  );

  new_dst += len * 2;
  }

  return dst;
}

#endif /* __ENGINE_H__ */

#endif

