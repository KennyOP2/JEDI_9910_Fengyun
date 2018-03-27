/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as OTG USB Device header file.
 * Date: 2007/10/17
 *
 * @author Jack Chain
 * @version 0.01
 */

#ifndef OTG_DEVICE_H
#define OTG_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================


#if !defined(CONFIG_HAVE_USBD)
//=============================================================================
//          Constant Definition - for Interrupt
//=============================================================================
#define IntTransSizeFix 		0
	
MMP_UINT8 u8OTGInterruptCount;
MMP_UINT32 u32OTGInterrupt_TX_COUNT ;
	
MMP_UINT8 u8OTGInterruptOutCount ;
MMP_UINT32 u32OTGInterrupt_RX_COUNT ;


//=============================================================================
//          Constant Definition - for Isochronous
//=============================================================================
#define ISO_Wrap 	        254
	
//MMP_UINT32 *u32OTGISOArray;
MMP_UINT32 u32ISOOTGOutTransferCount;
//MMP_UINT32 u32ISOOTGInTest[4096];
//MMP_UINT32 *u32ISOOTGOutArray;
//MMP_UINT8 u8ISOOTGOutCount ;
#endif


//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum 
{
	CMD_VOID,						// No command
	CMD_GET_DESCRIPTOR,			// Get_Descriptor command
	CMD_SET_DESCRIPTOR,			// Set_Descriptor command
	CMD_CxOUT_Vendor,				//Cx OUT Vandor command test
	CMD_CxIN_Vendor				//Cx IN Vandor command test
} CommandType;

typedef enum 
{
	ACT_IDLE,
	ACT_DONE,
	ACT_STALL
} Action;


//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct Setup_Packet
{
	MMP_UINT8 Direction;		/*Data transfer direction: IN, OUT*/
	MMP_UINT8 Type;			/*Request Type: Standard, Class, Vendor*/
	MMP_UINT8 Object;		/*Recipient: Device, Interface, Endpoint,other*/
	MMP_UINT16 Request;		/*Refer to Table 9-3*/
	MMP_UINT16 Value;
	MMP_UINT16 Index;
	MMP_UINT16 Length;
} SetupPacket;

extern MMP_UINT32 OTG_interrupt_level1;
extern MMP_UINT32 OTG_interrupt_level1_Save;
extern MMP_UINT32 OTG_interrupt_level1_Mask;
	


//=============================================================================
//                              Function Declaration
//=============================================================================	
//============================================================================= 
//		OTG_OTGP_main()
//		Description: Leave when the VBus is not valid 
//			
//		input: bDoNotInit   =0: Init
//                          =1: Do not init
//             bWaitForVBUS =0: Do not wait for VBUS
//                          =1: wait for VBUS 
//		output: 
//=============================================================================
MMP_INT OTG_OTGP_main(MMP_UINT8 bDoNotInit,MMP_UINT8 bWaitForVBUS,MMP_UINT8 bExitMode);

//=============================================================================
//		OTG_DEVICE_init()
//		Description:
//		input: Reserved
//		output: Reserved
//=============================================================================
MMP_INT OTG_DEVICE_init(MMP_UINT8 bInitAP);

//=============================================================================
//		OTG_OTGP_Close()
//		Description:
//		input: Reserved
//		output: Reserved
//=============================================================================
void OTG_OTGP_Close(void);

//============================================================================= 
//		OTG_OTGP_HNP_Enable()
//		Description:
//			
//		input: none
//		output: none
//=============================================================================
void OTG_OTGP_HNP_Enable(void);

///////////////////////////////////////////////////////////////////////////////
//		OTG_vFOTG200_Dev_Init()
//		Description:
//			1. Reset all interrupt and clear all fifo data of FOTG200 Device
//			2. Turn on the "Global Interrupt Enable" bit of FOTG200 Device
//			3. Turn on the "Chip Enable" bit of FOTG200 Device
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT OTG_vFOTG200_Dev_Init(void);

///////////////////////////////////////////////////////////////////////////////
//		OTG_vOTGInit()
//		Description:
//			1. Configure the FIFO and EPx map.
//			2. Initiate FOTG200 Device.
//			3. Set the usb interrupt source as edge-trigger.
//			4. Enable Usb interrupt.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT OTG_vOTGInit(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Handler()
//		Description:
//			1. Service all Usb events
//			2. ReEnable Usb interrupt.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Handler(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0Setup()
//		Description:
//			1. Read 8-byte setup packet.
//			2. Decode command as Standard, Class, Vendor or NOT support command
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Setup(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0Tx()
//		Description:
//			1. Transmit data to EP0 FIFO.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Tx(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0Rx()
//		Description:
//			1. Receive data from EP0 FIFO.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_Ep0Rx(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0End()
//		Description:
//			1. End this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0End(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0Fail()
//		Description:
//			1. Stall this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Fail(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0Abort()
//		Description:
//			1. Stall this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Abort(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_BusReset()
//		Description:
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_BusReset(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Suspend()
//		Description:
//			1. Clear suspend interrupt, and set suspend register.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Suspend(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Resume()
//		Description:
//			1. Clear resume interrupt status and leave supend mode.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Resume(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_StandardCommand()
//		Description:
//			1. Process standard Cx 8 bytes command.
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_StandardCommand(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxGetStatus()
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxGetStatus(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxClearFeature()
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxClearFeature(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxSetFeature()
//		Description:
//			1. Process Cx Set feature command.
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetFeature(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxSetAddress()
//		Description:
//			1. Set USB bus addr to FOTG200 Device register.
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetAddress(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxGetDescriptor()
//		Description:
//			1. Point to the start location of the correct descriptor.
//			2. set the transfer length and return descriptor information back to host
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxGetDescriptor(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxSetDescriptor()
//		Description:
//			1. Point to the start location of the correct descriptor.
//			2. Set the transfer length, and we will save data into sdram when Rx interrupt occure
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetDescriptor(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxGetConfiguration()
//		Description:
//			1. Send 1 bytes configuration value to host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_CxGetConfiguration(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxSetConfiguration()
//		Description:
//			1. Get 1 bytes configuration value from host.
//			2-1. if(value == 0) then device return to address state
//			2-2. if(value match descriptor table)
//					then config success & Clear all EP toggle bit
//			2-3	 else stall this command
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetConfiguration(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxGetInterface()
//		Description:
//			Getting interface
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxGetInterface(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxSetInterface()
//		Description:
//			1-1. If (the device stays in Configured state)
//					&(command match the alternate setting)
//						then change the interface
//			1-2. else stall it
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetInterface(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxSynchFrame()
//		Description:
//			1. If the EP is a Iso EP, then return the 2 bytes Frame number.
//				 else stall this command
//		input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSynchFrame(void);

MMP_INT USB_DEVICE_ClassCommand(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0TxData()
//		Description:
//			1. Send data(max or short packet) to host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0TxData(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Ep0RxData()
//		Description:
//			1. Receive data(max or short packet) from host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_Ep0RxData(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_ClearEpX()
//		Description:
//			1. Clear all endpoint Toggle Bit
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_ClearEpX(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_ClearEpXFifoX()
//		Description:
//			1. Clear all endpoint & FIFO register setting
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_ClearEpXFifoX(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_IsoSequentialError()
//		Description:
//			1. FOTG200 Device Detects High bandwidth isochronous Data PID sequential error.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_IsoSequentialError(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_IsoSequentialAbort()
//		Description:
//			1. FOTG200 Device Detects High bandwidth isochronous Data PID sequential abort.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_IsoSequentialAbort(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Tx0Byte()
//		Description:
//			1. Send 0 byte data to host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Tx0Byte(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_Rx0Byte()
//		Description:
//			1. Receive 0 byte data from host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Rx0Byte(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_DxFWr()
//		Description: Write the buffer content to USB220 Data FIFO
//		input: Buffer pointer, byte number to write, Change Endian type or NOT
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_DxFWr(MMP_UINT8 FIFONum, MMP_UINT8   * pu8Buffer, MMP_UINT16 u16Num);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_DxFRd()
//		Description: Fill the buffer from USB220 Data FIFO
//		input: Buffer pointer, byte number to write, Change Endian type or NOT
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_DxFRd(MMP_UINT8 FIFONum, MMP_UINT8 * pu8Buffer, MMP_UINT16 u16Num);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxFWr()
//		Description: Write the buffer content to USB220 Cx FIFO
//		input: Buffer pointer, byte number to write, Change Endian type or NOT
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_CxFWr( MMP_UINT8 *pu8Buffer, MMP_UINT16 u16Num);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_CxFRd()
//		Description: Fill the buffer from USB220 Cx FIFO
//		input: Buffer pointer, byte number to write, Change Endian type or NOT
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxFRd(MMP_UINT8 *pu8Buffer, MMP_UINT16 u16Num);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_InterruptInitial() 
//		Description: Initial interrupt transfer test variable
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_InterruptInitial(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_InterruptIn() 
//		Description: FIFO4 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_InterruptIn(void);

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_InterruptOut() 
//		Description: FIFO5 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_InterruptOut(void);

//2008.12.23 Jack add, for USB Charge use
#ifdef USB_CHARGE_ENABLE
MMP_BOOL USB_DEVICE_GetIsUSBCharge(void);
void USB_DEVICE_SetIsUSBCharge(MMP_BOOL flag);
#endif


#ifdef __cplusplus
}
#endif

#endif
