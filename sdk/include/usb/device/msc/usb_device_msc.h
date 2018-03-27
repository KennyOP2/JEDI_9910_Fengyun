/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  usb_device_msc.h usb Mass Storage Class heder file.
 *
 * @author Irene Lin
 * @version 0.1
 */
#ifndef USB_DEVICE_MSC_H
#define USB_DEVICE_MSC_H



//=============================================================================
//                              Constant Definition
//=============================================================================
#define CBW_SIGNATE		            0x43425355
#define CSW_SIGNATE			        0x53425355
#define CSW_STATUS_CMD_PASS			0x00
#define CSW_STATUS_CMD_FAIL			0x01
#define CSW_STATUS_PHASE_ERROR		0x02
 

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum 
{
	IDLE,
	STATE_CBW,
	STATE_CB_DATA_IN,
	STATE_CB_DATA_OUT,
	STATE_CSW
} MassStorageState;

typedef struct CommandBlockWrapper
{
	MMP_UINT32 u32Signature;
	MMP_UINT32 u32Tag;
	MMP_UINT32 u32DataTransferLength;
	MMP_UINT8  u8Flags;
	MMP_UINT8  u8LUN;
	MMP_UINT8  u8CBLength;
	MMP_UINT8  u8CB[16];
} CBW;

typedef struct CommandStatusWrapper
{
	MMP_UINT32 u32Signature;
	MMP_UINT32 u32Tag;
	MMP_UINT32 u32DataResidue;
	MMP_UINT8  u8Status;
} CSW;


//=============================================================================
//                              Function Declaration
//=============================================================================	
///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_ApInitialize()
//		Description: User specified circuit (AP) init
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_MSC_ApInitialize(void);

MMP_INT USB_DEVICE_MSC_ApTerminate(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_BulkOut()
//		Description: USB FIFO2 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_MSC_BulkOut(MMP_UINT16 u16FIFOByteCount);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_BulkIn()
//		Description: USB FIFO0 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_MSC_BulkIn(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_CheckDMA()
//		Description: Check OTG DMA finish or error
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_MSC_CheckDMA(void);




#endif /* USB_DEVICE_MSC_H */
