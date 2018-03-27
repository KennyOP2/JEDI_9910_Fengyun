/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef	DMA_CONFIG_H
#define	DMA_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"
#include "host/host.h"
#include "host/ahb.h"
#include "sys/sys.h"
#include "mem/mem.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define FPGA

//=============================================================================
//                              Constant Definition
//=============================================================================
#define DMA_MAX_CHANNEL     8

#if defined(__FREERTOS__)
    #define DMA_IRQ_ENABLE
#endif


//=============================================================================
//                              LOG definition
//=============================================================================
#if 1//defined(DEBUG_MODE)
#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO)
//#define LOG_ZONES    (0)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][DMA][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][DMA][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][DMA][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][DMA][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][DMA][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][DMA][LEAVE]"
#define LOG_DATA    ((void) ((MMP_FALSE) ? (printf(
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
#define LOG_END         ;
#endif


#include "mmp_dma.h"
#include "dma/dma_error.h"
#include "dma/dma_reg.h"
#include "dma/dma.h"
#include "dma/dma_hw.h"





#ifdef __cplusplus
}
#endif

#endif
