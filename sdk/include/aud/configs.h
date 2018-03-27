/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
#ifndef _AUDIO_CONFIGS_H_
#define _AUDIO_CONFIGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mmp_types.h"

//#if !defined(MM360) && !defined(MM365) && !defined(MM370) && !defined(MM680)
//  #error "CHIP ID IS NOT DEFINED!"
//#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
//#ifdef DEBUG_MODE
//#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG)
////#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE)
////#define LOG_ZONES    (MMP_BIT_ALL)
//
//#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][AUD][ERROR]"
//#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][AUD][WARNING]"
//#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][AUD][INFO]"
//#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][AUD][DEBUG]"
//#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][AUD][ENTER]"
//#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][AUD][LEAVE]"
//#define LOG_DATA    ((void) ((MMP_FALSE) ? (printf(
//#define LOG_END     )), 1 : 0));
//#else
#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END         ;
//#endif

#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_CONFIGS_H_ */
