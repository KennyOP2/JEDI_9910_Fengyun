/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as Timer Register Definition.
 *
 * @author Sammy Chen
 */
#ifndef	TIMER_REG_H
#define	TIMER_REG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "host/ahb.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Register Definition
//=============================================================================
#define TIMER_BASE_REG          TIMER_BASE

#define TIMER_MAP_OFFSET        0x10
#define TIMER_REG_OFFSET        0x04

#define TIMR_T1COUT_REG         0x0
#define TIMR_T1LOAD_REG         0x4
#define TIMR_T1MAT1_REG         0x8
#define TIMR_T1MAT2_REG         0xc

#define TIMR_TM1CR_REG          0x60
#define TIMR_TM2CR_REG          0x64
#define TIMR_TM3CR_REG          0x68
#define TIMR_TM4CR_REG          0x6C
#define TIMR_TM5CR_REG          0x70
#define TIMR_TM6CR_REG          0x74

#define TIMR_INTR_RAW_REG       0x78
#define TIMR_INTR_STATE_REG     0x7C
#define TIMR_INTR_MASK_REG      0x80

/*Timer Control Reg*/
#define TIMER_BIT_ENABLE          (1<<0)
#define TIMER_BIT_CLK_EXT         (1<<1)
#define TIMER_BIT_UP_COUNT        (1<<2)
#define TIMER_BIT_ONE_SHOT_STOP   (1<<3)
#define TIMER_BIT_PERIODIC_MODE   (1<<4)
#define TIMER_BIT_PWM_ENABLE      (1<<5)
#define TIMER_BIT_MERGE           (1<<6)

#define TIMER_BIT_DISABLE         (0<<0)
#define TIMER_BIT_CLK_PCLK        (0<<1)
#define TIMER_BIT_DOWN_COUNT      (0<<2)
#define TIMER_BIT_WRAPPING        (0<<3)
#define TIMER_BIT_FREE_MODE       (0<<4)
#define TIMER_BIT_PWM_DISABLE     (0<<5)

#ifdef __cplusplus
}
#endif

#endif
