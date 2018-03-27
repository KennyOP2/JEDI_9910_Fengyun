/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef	SD_CONFIG_H
#define	SD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"
#include "mmp_util.h"
#include "mmp_dma.h"
#include "host/host.h"
#include "host/ahb.h"
#include "sys/sys.h"

#include "mmp_sd.h"
#include "sd/sd_error.h"

#if !defined(FPGA)
    //#define FPGA
#endif

//=============================================================================
//                              Compile Option
//=============================================================================
#define SD_WIN32_DMA
#define SD_SHARE_SEMAPHORE
#define ASYNC_RESET
//#define MMC_WR_TIMING
#define SD_READ_FLIP_FLOP

#if defined(__FREERTOS__) || defined(__OPENRTOS__)
    #define SD_IRQ_ENABLE
    #undef SD_WIN32_DMA
#endif

//#define SD_DUMP_CSD
//#define SD_DUMP_CID
//#define SD_DUMP_SWITCH_FUN
//#define SD_DUMP_EXT_CSD


//=============================================================================
//                              Constant Definition
//=============================================================================
#if 0
#define SD_CLK_NORMAL_INIT      4   // Default mode : 100/5 = 20 MHz
#define SD_CLK_NORMAL           4   // Default mode : 100/5 = 20 MHz
#define SD_CLK_HIGH_SPEED       2   // High Speed : 100/3 = 33.33 MHz
#endif

#if 1 
#define SD_CLK_NORMAL_INIT      4   // Default mode : 80/5 = 16 MHz
#define SD_CLK_NORMAL           3   // Default mode : 80/4 = 20 MHz
#define SD_CLK_HIGH_SPEED       1   // High Speed   : 80/2 = 40 MHz
#endif

//=============================================================================
//                              LOG definition
//=============================================================================
#if 1//defined(DEBUG_MODE)
//#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO & ~MMP_ZONE_WARNING)
#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG &~MMP_ZONE_INFO)
//#define LOG_ZONES     0

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[SD][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[SD][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[SD][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[SD][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[SD][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[SD][LEAVE]"
#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
#define LOG_INFO2   ((void) ((MMP_TRUE) ? (printf(
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
#define LOG_INFO2
#define LOG_END         ;
#endif


#ifdef __cplusplus
}
#endif

#endif
