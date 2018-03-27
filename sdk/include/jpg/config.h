/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author 
 * @version 0.1
 */

#ifndef JPG_CONFIG_H
#define JPG_CONFIG_H


//=============================================================================
//                              Include Files
//=============================================================================
#if defined(WIN32)
    #include <stdio.h>
#endif

#include "mmp_types.h"
#include "jpg/jpg_error.h"


//=============================================================================
//                              Compile option
//=============================================================================
#define NOT_SUPPORT_NON_INTERLEAVED

//#define MANUFACTURE_TEST

#if !defined(WIN32)
    #define RESERVED_BUF
    #define REDUCED_BSBUF
#endif

//#define JPG_ENABLE_DEC_JPROG

//=============================================================================
//                              Constant Definition
//=============================================================================
#define JPG_TIMEOUT_COUNT   1000

#define Min(a, b)           ((a) < (b) ? (a) : (b))

#if 0//defined(DEBUG_MODE)
//#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO)
#define LOG_ZONES    (MMP_BIT_ALL)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[JPEG][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[JPEG][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[JPEG][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[JPEG][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[JPEG][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[JPEG][LEAVE]"
#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
#define LOG_CMD     ((void) ((MMP_TRUE) ? (printf("[JPEG][CMD]"
#define LOG_END     )), 1 : 0));
#else
#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_CMD
#define LOG_END         ;
#endif

#endif // End of #ifndef JPG_CONFIG_H

