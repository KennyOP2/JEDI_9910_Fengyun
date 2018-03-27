/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia USB Ex Driver API header file.
 *
 * @author Irene Lin
 */

#ifndef MMP_USB_EX_H
#define MMP_USB_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
	#if defined(USBEX_EXPORTS)
		#define USBEX_API __declspec(dllexport)
	#else
		#define USBEX_API __declspec(dllimport)
	#endif
#else
	#define USBEX_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
//=========================
/**
 * For HOST Driver
 */
//=========================
enum
{
    USB0 = 0,
    USB1
};

/**
 * Irene: 2010_1216 
 * example:
 * speed = mmpUsbExGetSpeed(USB0);
 *
 * For connected device speed.
 */
enum 
{
    USBEX_SPEED_UNKNOWN = 0,			/* not connected */
    USBEX_SPEED_LOW, USBEX_SPEED_FULL,	/* usb 1.1 */
    USBEX_SPEED_HIGH				    /* usb 2.0 */
};

/**
 * example: 
 * state = mmpUsbExCheckDeviceState();
 */
enum
{
    USB_DEVICE_TYPE_MSC         = 0x0001,
    USB_DEVICE_TYPE_SICD        = 0x0002,
    USB_DEVICE_TYPE_WIFI        = 0x0004,
    USB_DEVICE_TYPE_KBD         = 0x0008,
    USB_DEVICE_TYPE_MOUSE       = 0x0010,
    USB_DEVICE_TYPE_NOT_MATCH   = 0
};
enum
{
    USB_DEVICE_STATE_NOCHAGNE,
    USB_DEVICE_STATE_CONNECT,
    USB_DEVICE_STATE_DISCONNECT
};
#define USB_DEVICE_CONNECT(x)       ((x)==USB_DEVICE_STATE_CONNECT)
#define USB_DEVICE_DISCONNECT(x)    ((x)==USB_DEVICE_STATE_DISCONNECT)
#define USB_DEVICE_WIFI(x)          ((x)==USB_DEVICE_TYPE_WIFI)
#define USB_DEVICE_PTP(x)           ((x)==USB_DEVICE_TYPE_SICD)
#define USB_DEVICE_MSC(x)           ((x)==USB_DEVICE_TYPE_MSC)
#define USB_DEVICE_KBD(x)           ((x)==USB_DEVICE_TYPE_KBD)
#define USB_DEVICE_MOUSE(x)         ((x)==USB_DEVICE_TYPE_MOUSE)


//=========================
/**
 * For Device Driver
 */
//=========================
#define MMP_OTG_MAX_LUN_NUM 8

typedef enum MMP_OTG_INIT_ATTRIB_TAG
{
    MMP_OTG_ATTRIB_DEVICE_INITIALIZE      = 0,
    MMP_OTG_ATTRIB_DEVICE_TERMINATE       = 1,
    MMP_OTG_ATTRIB_DEVICE_GET_CAPACITY    = 2,
    MMP_OTG_ATTRIB_DEVICE_READ_SECTOR     = 3,
    MMP_OTG_ATTRIB_DEVICE_WRITE_SECTOR    = 4,
    MMP_OTG_ATTRIB_DEVICE_GET_MAX_LUN_NUM = 5,
    MMP_OTG_ATTRIB_DEVICE_RESPONSE        = 6,
    MMP_OTG_ATTRIB_DEVICE_INQUIRY         = 7,
    MMP_OTG_ATTRIB_DEVICE_IS_LOCK         = 8,
    MMP_OTG_ATTRIB_DEVICE_EJECT           = 9,
    MMP_OTG_ATTRIB_NONE                   = 0xFFFFFFFF
} MMP_OTG_INIT_ATTRIB;

typedef enum MMP_OTG_MEDIA_STATUS_TAG
{
    MMP_OTG_MEDIA_NOT_PRESENT,
    MMP_OTG_MEDIA_READY,
    MMP_OTG_MEDIA_CHANGE
} MMP_OTG_DEVICE_STATUS;

typedef MMP_INT
(*OTG_DEVICE_INITIALIZE)(
    MMP_UINT8 lun);

typedef MMP_INT
(*OTG_DEVICE_TERMINATE)(
    MMP_UINT8 lun);

typedef MMP_INT
(*OTG_DEVICE_GET_CAPACITY)(
    MMP_UINT8   lun,
    MMP_UINT32* lastBlockId,
    MMP_UINT32* blockLength);

typedef MMP_INT
(*OTG_DEVICE_READ_SECTOR)(
    MMP_UINT8   lun,
    MMP_UINT32  blockId, 
    MMP_UINT32  sizeInSector, 
    MMP_UINT16* srcBuffer);

typedef MMP_INT
(*OTG_DEVICE_WRITE_SECTOR)(
    MMP_UINT8   lun,
    MMP_UINT32  blockId, 
    MMP_UINT32  sizeInSector, 
    MMP_UINT16* dstBuffer);

typedef MMP_INT
(*OTG_DEVICE_GET_MAX_LUN_NUM)(
    MMP_UINT8* lunNum);

typedef MMP_OTG_DEVICE_STATUS
(*OTG_DEVICE_RESPONSE)(
    MMP_UINT8 lun);

typedef MMP_INT
(*OTG_DEVICE_INQUIRY)(
    MMP_UINT8 lun,
    MMP_UINT8** inquiryData);

typedef MMP_BOOL
(*OTG_DEVICE_IS_LOCK)(
    MMP_UINT8 lun);

typedef MMP_INT
(*OTG_DEVICE_EJECT)(
    MMP_UINT8 lun);

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    MMP_INT type;
    void*   ctxt;
} USB_DEVICE_INFO;

//=============================================================================
//                              Enumeration Type Definition 
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group14 SMedia USB Driver API
 *  The USB module API.
 *  @{
 */
USBEX_API void* USBEX_ThreadFunc(void* data);
USBEX_API void* DEVICE_ThreadFunc(void* data);


//=========================
/**
 * For HOST Driver
 */
//=========================
USBEX_API MMP_INT mmpUsbExInitialize(MMP_INT);

//=============================================================================   
/**
 * Get newly USB device state.
 *
 * @param  usb      choose USB0 or USB1
 * @param  state    usb device is first connect or disconnect or no change.
 * @param  device_info      get this device information
 * @return MMP_RESULT_SUCCESS if succeed.
 *
 * @see USB_DEVICE_INFO
 */
//=============================================================================   
USBEX_API MMP_UINT32 
mmpUsbExCheckDeviceState(
    MMP_INT usb, 
    MMP_INT* state, 
    USB_DEVICE_INFO* device_info
);

USBEX_API MMP_BOOL
mmpUsbExUsb0IsOtg(void);

/** 
 * Irene: 2010_1216 
 * example:
 * speed = mmpUsbExGetSpeed(USB0);
 *
 * For connected device speed.
 */
USBEX_API MMP_INT
mmpUsbExGetSpeed(MMP_INT usb);


//=========================
/**
 * For Device Driver
 */
//=========================
//=============================================================================
/**
 * Return OTG Device mode device is connect or not
 *
 * @return MMP_TRUE if device is connect, return MMP_FALSE if device is not connect.
 * @see mmpOtgDeviceModeOpen()
 */
//=============================================================================
USBEX_API MMP_BOOL mmpOtgIsDeviceMode(void);

//=============================================================================
/**
 * Setup OTG Device mode at open device connect
 *
 * @param attribList        OTG device mode need call back function pointers.
 *
 * @return 0 if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpOtgDeviceModeClose()
 */
//=============================================================================
USBEX_API MMP_INT mmpOtgDeviceModeOpen(const MMP_ULONG*  attribList);

//=============================================================================
/**
 * Setup OTG Device mode at device dis connect or error
 *
 * @return 0 if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpOtgDeviceModeOpen()
 */
//=============================================================================
USBEX_API MMP_INT mmpOtgDeviceModeClose(void);

USBEX_API MMP_BOOL
mmpOtgisUSBChargeState(
    void);    


#if defined(USB_LOGO_TEST)

#define HUB_ERROR       -9999

/** @see mmpUsbExPortControl() param ctrl */
enum 
{
    USB_PORT_TEST_J_STATE  = 0x1,	/* Test J_STATE */
    USB_PORT_TEST_K_STATE  = 0x2,	/* Test K_STATE */
    USB_PORT_TEST_SE0_NAK  = 0x3,	/* Test SE0_NAK */
    USB_PORT_TEST_PACKET   = 0x4,	/* Test Packet */
    USB_PORT_TEST_FORCE_EN = 0x5 	/* Test FORCE_ENABLE */
};

/** @see mmpUsbExDeviceControl() */
enum 
{
    USB_DEV_CTRL_SINGLE_STEP_GET_DEV      = 0x1,
    USB_DEV_CTRL_SINGLE_STEP_SET_FEATURE  = 0x2
};

/** For USB logo test "Host Port Control". */
USBEX_API MMP_INT 
mmpUsbExPortControl(
    MMP_INT     usb,
    MMP_UINT32  ctrl);

USBEX_API MMP_INT 
mmpUsbExDeviceControl(
    void* usb_dev,
    MMP_UINT32  ctrl,
    MMP_UINT32  step,
    MMP_UINT8*  data);

USBEX_API MMP_BOOL
mmpUsbExIsDeviceConnect(MMP_INT usb);

USBEX_API MMP_INT
mmpUsbExSuspend(MMP_INT usb);

USBEX_API MMP_INT
mmpUsbExResume(MMP_INT usb);


#endif // #if defined(USB_LOGO_TEST)

//@}


#ifdef __cplusplus
}
#endif

#endif 
