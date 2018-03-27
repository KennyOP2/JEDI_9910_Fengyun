/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 */
#ifndef	CONFIG_H
#define	CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Compile Option
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              LOG definition
//=============================================================================
//#define LOG_ZONES    (MMP_BIT_ALL /*& ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO*/)
#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE /*& ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO*/)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[USBEX][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[USBEX][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[USBEX][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[USBEX][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[USBEX][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[USBEX][LEAVE]"
#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
#define LOG_END     )), 1 : 0));



#ifdef __cplusplus
}
#endif

#endif
