/*
 * Copyright (c) 2008 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */

#ifndef USB_EX_CONFIG_H
#define USB_EX_CONFIG_H

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


//#define RUN_FPGA
#define DUMP_DEVICE_INFO
//#define DUMP_QH
//#define DUMP_REG
//#define DUMP_SETUP_CMD

#if defined(USB_LOGO_TEST)
    #define USB_PM
#endif

#define UNLINK_TIMEOUT      1000

#define WR_MAX_PACKET_LENGTH
/** 
 * Microsoft enumerate flow 
 */
//#define MS_ENUMERATE

#define QUICK_REMOVE

#if defined(__FREERTOS__)
    #define USB_IRQ_ENABLE
    #define POLLING_REMOVE
    #define INTR_THRESHOLD  1
#endif

#define HANDLE_HW_HANG

#define WR_USB_AHB_WRAP
//#define USB0_OTG_USB1_HOST

//=============================================================================
//                              Constant Definition
//=============================================================================
#if 0//defined(DEBUG_MODE)
//#define LOG_ZONES    0
#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[USBEX][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[USBEX][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[USBEX][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[USBEX][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[USBEX][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[USBEX][LEAVE]"
#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
#define LOG_CMD     ((void) ((MMP_FALSE) ? (printf("[USBEX][CMD]"
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

#define USBEX_URB_NUM       32

//2008.12.23 Jack add, For USB Chargeing
//#define USB_CHARGE_ENABLE

/**
 * Porting for MM9070 || MM9910
 */
#if defined(MM9070) || defined(MM9910)
#define MEM_USAGE_DMA           0
#define MEM_USAGE_QH_EX         0
#define MEM_USAGE_QTD_EX        0
#define MEM_USAGE_PERIODIC_EX   0
#define MEM_USAGE_4KBUF_EX      0
#define MEM_Allocate(a,b,c)     MEM_Allocate(a,b)
#endif


#ifdef __cplusplus
}
#endif


#endif // End of #ifndef USB_EX_CONFIG_H


