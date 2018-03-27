/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file usb.h
 * Driver for USB Tuner compliant devices.
 * Main Header File
 *
 * @author Barry Wu
 */
#ifndef USB_MOD_H
#define USB_MOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mmp_types.h"
//#include "usb_mod_cfg.h"
//=============================================================================
//                              Constant Definition
//=============================================================================
#define USB_TUNER_STRING_LEN 32

//=============================================================================
//                              LOG definition
//=============================================================================
#if 1//def DEBUG_MODE
//#define LOG_ZONES    (MMP_BIT_ALL /*& ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO*/)

#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO & ~MMP_ZONE_WARNING)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][USBMOD][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][USBMOD][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][USBMOD][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][USBMOD][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][USBMOD][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][USBMOD][LEAVE]"
#define LOG_CMD     ((void) ((MMP_FALSE) ? (printf("[SMEDIA][USBMOD][CMD]"
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

//=============================================================================
//                              Structure Definition
//=============================================================================
struct usbtuner_data;

typedef int (*trans_cmnd)(struct usbtuner_data*);
typedef int (*trans_reset)(struct usbtuner_data*);

/** every device has one */
struct usbtuner_data 
{
    /* the device we're working with */
    struct usb_device*	pusb_dev;	 /* this usb_device */

    /* information about the device -- always good */
    MMP_UINT8			vendor[32];
    MMP_UINT8			product[32];
    MMP_UINT8			serial[32];
    MMP_UINT8			*transport_name;
    MMP_UINT8			*protocol_name;
    MMP_UINT8			subclass;
    MMP_UINT8			protocol;
    MMP_UINT8			max_lun;
    MMP_UINT8         use_mode_sense6;

    /* information about the device  */
    MMP_UINT8			ifnum;		 /* interface number   */
    MMP_UINT8			ep_in;		 /* bulk in endpoint   */
    MMP_UINT8			ep_out;		 /* bulk out endpoint  */
    struct usb_endpoint_descriptor *ep_int;	 /* interrupt endpoint */ 

    /* function pointers for this device */
    trans_cmnd		transport;	 /* transport function	   */
    trans_reset		transport_reset; /* transport device reset */

    void*           semaphore;
    MMP_UINT8       in_rw_access;
    MMP_UINT8       reserved[3];
};


#include "usb/usb/usb.h"
#include "usb/usb/usbex_error.h"

//=============================================================================
//                              Function Declaration
//=============================================================================

#ifdef __cplusplus
}
#endif


#endif
