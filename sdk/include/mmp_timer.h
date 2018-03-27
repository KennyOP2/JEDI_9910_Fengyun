/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia Timer Driver API header file.
 *
 * @author Sammy Chen
 */

#ifndef MMP_TIMER_H
#define MMP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	#if defined(TIMER_EXPORTS)
		#define TIMER_API __declspec(dllexport)
	#else
		#define TIMER_API __declspec(dllimport)
	#endif
#else
	#define TIMER_API extern
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
//ERROR CODE
#define MMP_BL_STEP_OUT_OF_RANGE        0x424C0001

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================
typedef enum MMP_TIMER_NUM_TAG
{
    MMP_TIMER_1 = 0,
    MMP_TIMER_2 = 1,
    MMP_TIMER_3 = 2,
    MMP_TIMER_4 = 3,
    MMP_TIMER_5 = 4,
    MMP_TIMER_6 = 5,
    MMP_TIMER_COUNT = 6
}MMP_TIMER_NUM;

typedef enum
{
    MMP_TIMER_EN        = 0,
    MMP_TIMER_EXTCLK    = 1,
    MMP_TIMER_UPCOUNT   = 2,
    MMP_TIMER_ONESHOT   = 3,
    MMP_TIMER_PERIODIC  = 4,
    MMP_TIMER_PWM       = 5,
    MMP_TIMER_EN64      = 6
} MMPTIMERCTRL;



//=============================================================================
//                              Structure Definition
//=============================================================================


//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group11 SMedia Timer Driver API
 *  The Timer module API.
 *  @{
 */

//=============================================================================
//                             Timer Mode Functions
//=============================================================================
/**
 * Rest Timer module.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
TIMER_API MMP_RESULT
mmpTimerReset(void);

/**
 * Rest Timer module.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */    
TIMER_API MMP_RESULT
mmpTimerResetTimer(MMP_TIMER_NUM timer);

/**
 * Enable Timer Control Option.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
TIMER_API MMP_RESULT
mmpTimerCtrlEnable(MMP_TIMER_NUM timer, MMPTIMERCTRL ctrl);

/**
 * Disnable Timer Control Option.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
TIMER_API MMP_RESULT
mmpTimerCtrlDisable(MMP_TIMER_NUM timer, MMPTIMERCTRL ctrl);

TIMER_API MMP_RESULT
mmpTimerSetTimeOut(MMP_TIMER_NUM timer, MMP_INT us);

TIMER_API MMP_UINT32
mmpTimerReadIntr(void);

TIMER_API MMP_UINT32
mmpTimerReadCounter(MMP_TIMER_NUM timer);

TIMER_API MMP_RESULT
mmpTimerResetCounter(MMP_TIMER_NUM timer);
//@}
#ifdef __cplusplus
}
#endif

#endif
