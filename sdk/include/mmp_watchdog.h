/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia WATCHDOG Driver API header file.
 *
 * @author James Lin
 */

#ifndef MMP_WATCHDOG_H
#define MMP_WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
	#if defined(WATCHDOG_EXPORTS)
		#define WATCHDOG_API __declspec(dllexport)
	#else
		#define WATCHDOG_API __declspec(dllimport)
	#endif
#else
	#define WATCHDOG_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_WDG_COUNTER     0xffffffff

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group11 SMedia WATCHDOG Driver API
 *  The WATCHDOG module API.
 *  @{
 */

//=============================================================================
//                             WATCHDOG Mode Functions
//=============================================================================
/**
 * Enable WATCHDOG module.
 */
WATCHDOG_API void
mmpWatchDogEnable(
    MMP_UINT watchdogtimeout);

/**
 * Disable WATCHDOG module.
 */
WATCHDOG_API void
mmpWatchDogDisable(
    void);

/**
 * Refresh WATCHDOG timer
 */
WATCHDOG_API void
mmpWatchDogRefreshTimer(
    void);

//@}
#ifdef __cplusplus
}
#endif

#endif
