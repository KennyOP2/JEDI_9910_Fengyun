/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia RTC Driver API header file.
 *
 * @author Jim Tan
 */

#ifndef MMP_RTC_H
#define MMP_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
	#if defined(RTC_EXPORTS)
		#define RTC_API __declspec(dllexport)
	#else
		#define RTC_API __declspec(dllimport)
	#endif
#else
	#define RTC_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================
typedef enum MMP_RTC_INTR_TYPE_TAG
{
    MMP_RTC_INTR_EVERY_SEC  = 1,
    MMP_RTC_INTR_EVERY_MIN  = 2,
    MMP_RTC_INTR_EVERY_HOUR = 3,
    MMP_RTC_INTR_EVERY_DAY  = 4,
    MMP_RTC_INTR_ALARM      = 5
} MMP_RTC_INTR_TYPE;

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group10 SMedia RTC Driver API
 *  The RTC module API.
 *  @{
 */

//=============================================================================
//                             RTC Mode Functions
//=============================================================================
/**
 * Initialize RTC module. The date and time will start at 2000/01/01 00:00:00.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
RTC_API MMP_RESULT
mmpRtcInitialize(
    void);

/**
 * Terminate RTC module.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
RTC_API MMP_RESULT
mmpRtcTerminate(
    void);

/**
 * Enable RTC.
 */
RTC_API void
mmpRtcEnable(
    void);

/**
 * Disable RTC.
 */
RTC_API void
mmpRtcDisable(
    void);

/**
 * Enable RTC interrupt.
 *
 * @param type interrupt type.
 */
RTC_API void
mmpRtcEnableInterrupt(
    MMP_RTC_INTR_TYPE type);

/**
 * Disable RTC interrupt.
 *
 * @param type interrupt type.
 */
RTC_API void
mmpRtcDisableInterrupt(
    MMP_RTC_INTR_TYPE type);

/**
 * Indicate RTC interrupt is occured.
 *
 * @param type interrupt type.
 */
RTC_API MMP_BOOL
mmpRtcIsInterruptOccured(
    MMP_RTC_INTR_TYPE type);

/**
 * Set RTC alarm.
 *
 * @param hour alarm hour.
 * @param min alarm minute.
 * @param sec alarm second.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcSetAlarm(
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec);

/**
 * Get RTC alarm.
 *
 * @param hour alarm hour.
 * @param min alarm minute.
 * @param sec alarm second.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcGetAlarm(
    MMP_UINT32* pHour,
    MMP_UINT32* pMin,
    MMP_UINT32* pSec);

/**
 * Set RTC date.
 *
 * @param year the year.
 * @param month the month.
 * @param day the day.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcSetDate(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day);

/**
 * Get RTC date.
 *
 * @param year current year to retrieve. Can be MMP_NULL.
 * @param month current month. Can be MMP_NULL.
 * @param day current day. Can be MMP_NULL.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcGetDate(
    MMP_UINT* year,
    MMP_UINT* month,
    MMP_UINT* day);

/**
 * Set RTC time.
 *
 * @param hour the hour.
 * @param min the minute.
 * @param sec the second.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcSetTime(
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec);

/**
 * Set RTC date and time.
 *
 * @param year the year.
 * @param month the month.
 * @param day the day.
 * @param hour the hour.
 * @param min the minute.
 * @param sec the second.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */

MMP_RESULT
mmpRtcSetDateTime(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day,
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec);    

/**
 * Get RTC time.
 *
 * @param hour current hour to retrieve. Can be MMP_NULL.
 * @param min current minute. Can be MMP_NULL.
 * @param sec current second. Can be MMP_NULL.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcGetTime(
    MMP_UINT* hour,
    MMP_UINT* min,
    MMP_UINT* sec);

/**
 * Get RTC day & sec counts since 1970/01/01 00:00:00.
 *
 * @param day the day count to retrieve. Can be MMP_NULL.
 * @param sec the second count. Can be MMP_NULL.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise. 
 */
RTC_API MMP_RESULT
mmpRtcGetDateTimeOfDay(
    MMP_UINT* day,
    MMP_UINT* sec);

/**
 * Get RTC days since 1970/01/01
 *
 * @param hour the hour.
 * @param min the minute.
 * @param sec the second.
 * @return days. 
 */

RTC_API MMP_UINT32
mmpRtcConvert2Days(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day);    

//@}
#ifdef __cplusplus
}
#endif

#endif
