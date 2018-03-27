/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as USB HCD error code.
 *
 * @author Irene Lin
 */

#ifndef USBEX_ERROR_H
#define USBEX_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "errorno.h"

//=============================================================================
//                              Type Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================
/**
 *  USB HCD error code
 */
#if defined(ALPHA_SDK)
#define ERROR_USBEX_BASE                                 0xC000
#else
#define ERROR_USBEX_BASE                                 (0xC << MMP_ERROR_OFFSET)
#endif

#define ERROR_USB_ALLOC_HCD_FAIL                           (ERROR_USBEX_BASE + 0x0081)
#define ERROR_USB_ALLOC_BUS_FAIL                           (ERROR_USBEX_BASE + 0x0082)
#define ERROR_USB_ALLOC_QH_FAIL                            (ERROR_USBEX_BASE + 0x0083)
#define ERROR_USB_ALLOC_QTD_FAIL                           (ERROR_USBEX_BASE + 0x0084)
#define ERROR_USB_ALLOC_4KBUF_FAIL                         (ERROR_USBEX_BASE + 0x0085)
#define ERROR_USB_HC_RESET_TIMEOUT                         (ERROR_USBEX_BASE + 0x0086)
#define ERROR_USB_ALLOC_DEVICE_FAIL                        (ERROR_USBEX_BASE + 0x0087)
#define ERROR_USB_ALLOC_HCD_DEV_FAIL                       (ERROR_USBEX_BASE + 0x0088)
#define ERROR_USB_FREE_NON_EMPTY_URB_DEVICE                (ERROR_USBEX_BASE + 0x0089)
#define ERROR_USB_ALLOC_SYS_MEM_FAIL                       (ERROR_USBEX_BASE + 0x008A)
#define ERROR_USB_GET_URB_FAIL                             (ERROR_USBEX_BASE + 0x008B)
#define ERROR_USB_FREE_NULL_URB                            (ERROR_USBEX_BASE + 0x008C)
#define ERROR_USB_URB_COOKIE_ERROR                         (ERROR_USBEX_BASE + 0x008D)
#define ERROR_USB_URB_DOUBLE_FREE                          (ERROR_USBEX_BASE + 0x008E)
#define ERROR_USB_NOT_FIND_DEVCE_ID                        (ERROR_USBEX_BASE + 0x008F)
#define ERROR_USB_PORT_RESET_FAIL                          (ERROR_USBEX_BASE + 0x0090)
#define ERROR_USB_ALLOC_ONE_QTD_FAIL                       (ERROR_USBEX_BASE + 0x0091)
#define ERROR_USB_ALLOC_ONE_QTD_FAIL2                      (ERROR_USBEX_BASE + 0x0092)
#define ERROR_USB_ALLOC_ONE_4KBUF_FAIL                     (ERROR_USBEX_BASE + 0x0093)
#define ERROR_USB_FREE_ONE_QTD_FAIL                        (ERROR_USBEX_BASE + 0x0094)
#define ERROR_USB_FREE_ONE_20KBUF_FAIL                     (ERROR_USBEX_BASE + 0x0095)
#define ERROR_USB_CREATE_EHCI_MUTEX_FAIL                   (ERROR_USBEX_BASE + 0x0096)
#define ERROR_USB_ALLOC_ONE_QH_FAIL                        (ERROR_USBEX_BASE + 0x0097)
#define ERROR_USB_ALLOC_ONE_QH_FAIL1                       (ERROR_USBEX_BASE + 0x0098)
#define ERROR_USB_QH_LINK_FAIL1                            (ERROR_USBEX_BASE + 0x0099)
#define ERROR_USB_CREATE_HOST_TASK_FAIL                    (ERROR_USBEX_BASE + 0x009A)
#define ERROR_USB_GET_DEVICE_DESCRIPTOR_FAIL               (ERROR_USBEX_BASE + 0x009B)
#define ERROR_USB_GET_CONGIGURATION_FAIL                   (ERROR_USBEX_BASE + 0x009C)
#define ERROR_USB_TOO_MANY_INTERFACES                      (ERROR_USBEX_BASE + 0x009D)
#define ERROR_USB_INVALID_DESCRIPTOR_LEN                   (ERROR_USBEX_BASE + 0x009E)
#define ERROR_USB_TOO_MANY_ALTERNATE_SETTING               (ERROR_USBEX_BASE + 0x009F)
#define ERROR_USB_TOO_MANY_ENDPOINT                        (ERROR_USBEX_BASE + 0x00A0)
#define ERROR_USB_BAD_PARAMS                               (ERROR_USBEX_BASE + 0x00A1)
#define ERROR_USB_WAIT_ASS_PSS_READY_TIMEOUT               (ERROR_USBEX_BASE + 0x00A2)
#define ERROR_USB_NO_USB_DRIVER_SUPPORT                    (ERROR_USBEX_BASE + 0x00A3)
#define ERROR_USB_SYNC_UNLINK_TIMEOUT                      (ERROR_USBEX_BASE + 0x00A4)
#define ERROR_USB_CREATE_DEVICE_TASK_FAIL                  (ERROR_USBEX_BASE + 0x00A5)
#define ERROR_USB_ALLOC_PERIODIC_TABLE_FAIL                (ERROR_USBEX_BASE + 0x00A6)
#define ERROR_USB_ALLOC_PT_SHADOW_FAIL                     (ERROR_USBEX_BASE + 0x00A7)
#define ERROR_USB_PORT_NOT_ENABLE                          (ERROR_USBEX_BASE + 0x00A8)
#define ERROR_USB_PORT_NOT_SUSPEND                         (ERROR_USBEX_BASE + 0x00A9)
#define ERROR_USB_SERIOUS_ERROR                             (ERROR_USBEX_BASE + 0x00AA)


#ifdef __cplusplus
}
#endif


#endif // #ifndef USBEX_ERROR_H
