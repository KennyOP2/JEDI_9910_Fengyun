/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef	MSC_CONFIG_H
#define	MSC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"
#include "msc_error.h"


//=============================================================================
//                              Compile Option
//=============================================================================
//#define DUMP_CMD
//#define US_CBW_BUSY_WAIT_EN

//=============================================================================
//                              Constant Definition
//=============================================================================
#define US_BULK_TIMEOUT    10000
#define US_ADSC_TIMEOUT    2000

//=============================================================================
//                              LOG definition
//=============================================================================
#if 0//def DEBUG_MODE
//#define LOG_ZONES    (MMP_BIT_ALL /*& ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO*/)

#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO & ~MMP_ZONE_WARNING)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][MSC][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][MSC][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][MSC][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][MSC][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][MSC][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][MSC][LEAVE]"
#define LOG_CMD     ((void) ((MMP_FALSE) ? (printf("[SMEDIA][MSC][CMD]"
#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
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

#include "mmp_util.h"
#include "sys/sys.h"
#include "usb/usb/usb.h"
#include "usb/usb/usbex_error.h"

#include "mmp_msc.h"
#include "msc/scsi.h"
#include "msc/usb.h"
#include "msc/protocol.h"
#include "msc/transport.h"


#ifdef __cplusplus
}
#endif

#endif
