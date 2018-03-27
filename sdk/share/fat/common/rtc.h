/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * RTC manager.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef RTC_H
#define RTC_H

#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

MMP_INT
smtkRtcMgrInitialize(
    void);

MMP_INT
smtkRtcMgrTerminate(
    void);

MMP_INT
smtkRtcMgrGetDate(
    MMP_UINT* year,
    MMP_UINT* month,
    MMP_UINT* day);

MMP_INT
smtkRtcMgrGetTime(
    MMP_UINT* hour,
    MMP_UINT* min,
    MMP_UINT* sec);

MMP_INT
smtkRtcMgrSetDate(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day);

MMP_INT
smtkRtcMgrSetTime(
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec);

#ifdef __cplusplus
}
#endif

#endif /* RTC_H */
