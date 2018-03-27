/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file config.h
 *
 * @author
 */
 
#ifndef _VIDEO_CONFIG_H
#define _VIDEO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#if defined(WIN32)
    #include <stdio.h>
#endif
#include "mmp_types.h"
#include "mmp_util.h"
#include "sys/sys.h"
#include "host/ahb.h"
#include "host/host.h"
#include "mem/mem.h"

//#define RC_ENABLE
#define TILED_MODE_MAPPING
//#define MULTIPLE_INSTANCES
//#define NULL_CMD

//#define USING_HW_SPS_PPS
//#define FULL_HD
#ifndef USING_HW_SPS_PPS
#define VUI_ON
#define SEI_ON   
#endif

//#define ITURNE_TRICK
#define ISP_ONFLY_CACHE_ON

//=============================================================================
//                              Constant Definition
//=============================================================================
#if 0//defined(DEBUG_MODE)
//#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO)
#define LOG_ZONES    (MMP_BIT_ALL)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[ENCODER][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[ENCODER][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[ENCODER][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[ENCODER][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[ENCODER][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[ENCODER][LEAVE]"
#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
#define LOG_CMD     ((void) ((MMP_TRUE) ? (printf("[ENCODER][CMD]"
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

#ifdef __cplusplus
}
#endif


#endif // End of #ifndef _VIDEO_CONFIG_H
