/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  usb_device_bulk.c usb device bulk module Control
 *	Date: 2007/10/17
 *
 * @author Jack Chain
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/device/device.h"

#include "usb_device_scsi.c"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define USB_MAX_DMA_LEN     (127*1024)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MassStorageState eUsbOTGMassStorageState;
static MMP_UINT8*   dmaBuffer  = MMP_NULL;
static MMP_UINT8*   dmaBuffer1 = MMP_NULL;

static MMP_UINT32	u32OTGFIFOUseDMA;
static MMP_UINT32   dmaTotalTxSize;
static MMP_UINT8    dmaMultipleTimes;

static CSW tOTGCSW;
CBW tOTGCBW;

extern MMP_UINT8   bOTGDMARunning;

//#define DUMP_CBW_CSW

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static MassStorageState USB_DEVICE_ProcessCBW(void);
static MassStorageState USB_DEVICE_DataOut(MMP_UINT16 u16FIFOByteCount);
static MassStorageState USB_DEVICE_DataIn(void);
static void USB_DEVICE_SendCSW(void);
static void USB_DEVICE_FifoInterruptAction(MassStorageState eState);

#if defined(DUMP_CBW_CSW)
void USB_DEVICE_ShowCBW(void);
void USB_DEVICE_ShowCSW(void);
#endif


//=============================================================================
//                              Public Function Definition
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_ApInitialize()
//		Description: User specified circuit (AP) init
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_MSC_ApInitialize(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[USB_DEVICE_MSC_ApInitialize]  Enter \n" LOG_END
	
    ////// initial Bulk //////
    eUsbOTGMassStorageState = STATE_CBW;	// Init State

    result = USB_DEVICE_SCSI_Initialize();
    //if(result)
    //    goto end;
	
    tOTGCSW.u32Signature = CSW_SIGNATE;	// Init u32Signature
    ////// initial INT and ISO test ////////////
    u8OTGInterruptCount = 0;
    u32OTGInterrupt_TX_COUNT = 1;	
    u8OTGInterruptOutCount = 0;
    u32OTGInterrupt_RX_COUNT = 1;
	
    u32ISOOTGOutTransferCount = 0;
	
//end:
    LOG_LEAVE "[USB_DEVICE_MSC_ApInitialize] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_MSC_ApInitialize() return error code 0x%08X \n", result LOG_END
    return 0;//result;
}

MMP_INT USB_DEVICE_MSC_ApTerminate(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[USB_DEVICE_MSC_ApTerminate]  Enter \n" LOG_END

    result = USB_DEVICE_SCSI_Terminate();

    LOG_LEAVE "[USB_DEVICE_MSC_ApTerminate] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_MSC_ApTerminate() return error code 0x%08X \n", result LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_BulkOut()
//		Description: USB FIFO2 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_MSC_BulkOut(MMP_UINT16 u16FIFOByteCount)
{
    LOG_ENTER "[USB_DEVICE_MSC_BulkOut]  Enter u16FIFOByteCount = 0x%4x\n",u16FIFOByteCount LOG_END

    switch(eUsbOTGMassStorageState)
    {
        case STATE_CBW:
            if(u16FIFOByteCount == 31)
            {
                eUsbOTGMassStorageState = USB_DEVICE_ProcessCBW();
            }
            else
            {
                /** MSC ErrorRecovery Test ==> CSW should stall in every case. */
                LOG_ERROR "[USB_DEVICE_MSC_BulkOut] CMD Size not 31 Bytes!! STALL this END Point!! Size = %d \n", u16FIFOByteCount LOG_END
                USB_DEVICE_ClearFifoReg();
                USB_DEVICE_StallOutEndpointReg(EP1, MMP_TRUE);
            }
            break;
			
        case STATE_CB_DATA_OUT:
            eUsbOTGMassStorageState = USB_DEVICE_DataOut(u16FIFOByteCount);
            break;
			
        case STATE_CSW:
            LOG_ERROR "[USB_DEVICE_MSC_BulkOut] STATE ERROR!! STATE_CSW!! TODO: Check Why!!!!!!! \n" LOG_END
            break;
			
        case STATE_CB_DATA_IN:
            LOG_ERROR "[USB_DEVICE_MSC_BulkOut] STATE ERROR!! STATE_CB_DATA_IN!! TODO: Check Why!!!!!!! \n" LOG_END
            break;
			
        default:
            LOG_ERROR "[USB_DEVICE_MSC_BulkOut] Can't match STATE!! \n" LOG_END
            break;
    }
	
    USB_DEVICE_FifoInterruptAction(eUsbOTGMassStorageState);
    LOG_LEAVE "[USB_DEVICE_MSC_BulkOut] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_BulkIn()
//		Description: USB FIFO0 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_MSC_BulkIn(void)
{
    LOG_ENTER "[USB_DEVICE_MSC_BulkIn]  Enter \n" LOG_END

    switch(eUsbOTGMassStorageState)
    {
    case STATE_CSW:
        USB_DEVICE_SendCSW();
        eUsbOTGMassStorageState = STATE_CBW;
        USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
        break;
		
    case STATE_CB_DATA_IN:
        eUsbOTGMassStorageState = USB_DEVICE_DataIn();
        break;
		
    case STATE_CBW:
        LOG_ERROR "[USB_DEVICE_MSC_BulkIn] STATE ERROR!! STATE_CBW!! TODO: Check Why!!!!!!! \n" LOG_END
        break;

    case STATE_CB_DATA_OUT:
        LOG_ERROR "[USB_DEVICE_MSC_BulkIn] STATE ERROR!! STATE_CB_DATA_OUT!! TODO: Check Why!!!!!!! \n" LOG_END
        break;
		
    default:
        LOG_ERROR "[USB_DEVICE_MSC_BulkIn] Can't match STATE!! \n" LOG_END
        break;
    }
	
    if(!bOTGDMARunning)
        USB_DEVICE_FifoInterruptAction(eUsbOTGMassStorageState);
	
    LOG_LEAVE "[USB_DEVICE_MSC_BulkIn] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//		USB_DEVICE_MSC_CheckDMA()
//		Description: Check OTG DMA finish or error
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_MSC_CheckDMA(void)
{
    MMP_INT result = 0;
    MMP_UINT8 transferFlag = 0;

    LOG_ENTER "[USB_DEVICE_MSC_CheckDMA]  Enter \n" LOG_END
   	
    if(dmaMultipleTimes)
    {
        switch(eUsbOTGMassStorageState)
        {
        case STATE_CB_DATA_OUT:
            USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_FIFO0_OUT);
            break;
        case STATE_CB_DATA_IN:
            USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
            break;
        };
        return;
    }
   	
    if(tOTGCBW.u8Flags == 0x00)  // bulk_out
    {
        transferFlag = USB_TO_SCSI;
        #if !defined(__FREERTOS__)
        if(dmaBuffer)
        {
            SYS_Free(dmaBuffer);
            dmaBuffer = MMP_NULL;
        }
        dmaBuffer = (MMP_UINT8*)SYS_Malloc(tOTGCSW.u32DataResidue);
        if(!dmaBuffer)
        {
            result = ERROR_USB_ALLOCATE_DMA_BUFFER_FAIL;
            goto end;
        }
        HOST_ReadBlockMemory((MMP_UINT32)dmaBuffer, (MMP_UINT32)dmaBuffer1, tOTGCSW.u32DataResidue);
        USB_DEVICE_ReleaseDmaAddr();
        dmaBuffer1 = MMP_NULL;
        #endif
    }
    else
    {
        transferFlag = SCSI_TO_USB;
    }
    
    #if defined(__FREERTOS__)
    result = USB_DEVICE_SCSI_Receive((MMP_UINT8*)tOTGCBW.u8CB, (MMP_UINT8*)dmaBuffer1, &(tOTGCSW.u32DataResidue), transferFlag); 
    #else
    result = USB_DEVICE_SCSI_Receive((MMP_UINT8*)tOTGCBW.u8CB, (MMP_UINT8*)dmaBuffer, &(tOTGCSW.u32DataResidue), transferFlag); 
    #endif
    if(result)
    {
        tOTGCSW.u8Status = CSW_STATUS_CMD_FAIL;
        LOG_ERROR "[USB_DEVICE_MSC_CheckDMA] USB_DEVICE_SCSI_Receive return ERROR Code !!result = 0x%08X => %d LUN \n",result, tOTGCBW.u8LUN LOG_END
    }

    tOTGCSW.u32DataResidue = tOTGCBW.u32DataTransferLength - tOTGCSW.u32DataResidue;	
    if(tOTGCSW.u32DataResidue)
    {
        /**
         * It will remove USBCV MCS test 6 warnings, but will cause LOCK function fail.
         * So we choose to keep warnings for LOCK function work.
         */
        //tOTGCSW.u8Status = CSW_STATUS_CMD_FAIL;
        LOG_ERROR "[USB_DEVICE_MSC_CheckDMA] Data transfer has residue 0x%X \n", tOTGCSW.u32DataResidue LOG_END
    }

    if(tOTGCSW.u8Status == CSW_STATUS_CMD_FAIL)
    {
        if(tOTGCBW.u8Flags == 0x00)  // bulk_out
            USB_DEVICE_StallInEndpointReg(EP2, MMP_TRUE);
        else
            USB_DEVICE_StallInEndpointReg(EP1, MMP_TRUE);
    }

    bOTGDMARunning = MMP_FALSE;
    eUsbOTGMassStorageState = STATE_CSW;

    USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_FIFO0_OUT);
    USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);

    USB_DEVICE_EnableInterruptGroup0Reg(DEV_INT_G0_CX_SETUP);
    USB_DEVICE_EnableInterruptGroup0Reg(DEV_INT_G0_CX_IN);
    USB_DEVICE_EnableInterruptGroup0Reg(DEV_INT_G0_CX_OUT);
    USB_DEVICE_EnableInterruptGroup0Reg(DEV_INT_G0_CX_END);
	
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);
	
end:
    if(tOTGCBW.u8Flags == 0x00)  // bulk_out
    {
        #if defined(__FREERTOS__)
        USB_DEVICE_ReleaseDmaAddr();
        dmaBuffer1 = MMP_NULL;
        #else
        if(dmaBuffer)
        {
            SYS_Free(dmaBuffer);
            dmaBuffer = MMP_NULL;
        }
        #endif
    }
    else
    {
        #if defined(__FREERTOS__)
        if(dmaBuffer)
        {
            SYS_Free(dmaBuffer);
            dmaBuffer = MMP_NULL;
        }
        #else
        USB_DEVICE_ReleaseDmaAddr();
        dmaBuffer1 = MMP_NULL;
        #endif
    }
	
    LOG_LEAVE "[USB_DEVICE_MSC_CheckDMA] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_MSC_CheckDMA() has error code 0x%08X \n", result LOG_END
    return;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//		eOTGProessCBW()
//		Description: Process the CBW
//		input: none
//		output: MassStorageState
///////////////////////////////////////////////////////////////////////////////
static MassStorageState USB_DEVICE_ProcessCBW(void)
{
    MassStorageState eState;
    MMP_UINT8 bTrans;
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_ProcessCBW]  Enter \n" LOG_END

    USB_DEVICE_DxFRd(FIFO0, (MMP_UINT8 *)(&tOTGCBW), 31);
    #if defined(__FREERTOS__)
    tOTGCBW.u32DataTransferLength = le32_to_cpu(tOTGCBW.u32DataTransferLength);
    tOTGCBW.u32Signature = le32_to_cpu(tOTGCBW.u32Signature);
    tOTGCBW.u32Tag = le32_to_cpu(tOTGCBW.u32Tag);
    #endif

    #if defined(DUMP_CBW_CSW)
    USB_DEVICE_ShowCBW();
    #endif
	
    if(tOTGCBW.u32Signature != CBW_SIGNATE)
    {
        result = ERROR_USB_DEVICE_CBW_SIGNATURE_INVALID;
        eState = STATE_CBW;
    }
    else
    {
        // pass u32DataTransferLength to u32DataResidue
        tOTGCSW.u32DataResidue = tOTGCBW.u32DataTransferLength;
        // pass Tag from CBW to CSW
        tOTGCSW.u32Tag = tOTGCBW.u32Tag;
        // Assume Status is CMD_PASS
        tOTGCSW.u8Status = CSW_STATUS_CMD_PASS;
        // Get Virtual Memory start address.

        if(tOTGCSW.u32DataResidue == 0)
        {
            eState = STATE_CSW;
            bTrans = NO_TRANSMIT;
        }
        else if(tOTGCBW.u8Flags == 0x00)
        {
            eState = STATE_CB_DATA_OUT;
            bTrans = USB_TO_SCSI;
        }
        else
        {
            eState = STATE_CB_DATA_IN;
            bTrans = SCSI_TO_USB;

            if(dmaBuffer)
            {
                SYS_Free(dmaBuffer);
                dmaBuffer = MMP_NULL;
            }
            dmaBuffer = (MMP_UINT8*)SYS_Malloc(tOTGCSW.u32DataResidue);
            if(!dmaBuffer)
            {
                result = ERROR_USB_ALLOCATE_DMA_BUFFER_FAIL;
                goto end;
            }
        }

        result = USB_DEVICE_SCSI_Command((MMP_UINT8*)tOTGCBW.u8CB, (MMP_UINT8*)dmaBuffer, &(tOTGCSW.u32DataResidue), bTrans); 
        if(result)
        {
            tOTGCSW.u8Status = CSW_STATUS_CMD_FAIL;
            LOG_ERROR "[USB_DEVICE_ProcessCBW] USB_DEVICE_SCSI_Command return ERROR Code !!result = 0x%08X => %d LUN \n",result, tOTGCBW.u8LUN LOG_END
        }
        dmaTotalTxSize = 0;
        dmaMultipleTimes = MMP_FALSE;
    }

end:
    LOG_LEAVE "[USB_DEVICE_ProcessCBW] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_ProcessCBW() has error code 0x%08X \n", result LOG_END
    return eState;
}

///////////////////////////////////////////////////////////////////////////////
//		eOTGDataOut()
//		Description: Process the data output stage
//		input: none
//		output: MassStorageState
///////////////////////////////////////////////////////////////////////////////
static MassStorageState USB_DEVICE_DataOut(MMP_UINT16 u16FIFOByteCount)
{
    MMP_INT result = 0;
    MMP_UINT32 dmaLength = 0;
    LOG_ENTER "[USB_DEVICE_DataOut]  Enter => u16FIFOByteCount = %d \n", u16FIFOByteCount LOG_END

    if(tOTGCSW.u8Status)
    {
        if(u16FIFOByteCount)
        {
            USB_DEVICE_ClearFifoReg();
        }
        USB_DEVICE_StallInEndpointReg(EP2, MMP_TRUE); 
        return STATE_CSW;
    }

    USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
    USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_FIFO0_OUT);
    u32OTGFIFOUseDMA = FIFO0;
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_0);

    if((tOTGCBW.u32DataTransferLength-dmaTotalTxSize) > USB_MAX_DMA_LEN)
    {
        dmaLength = USB_MAX_DMA_LEN;
        dmaMultipleTimes = MMP_TRUE;
    }
    else
    {
        dmaLength = tOTGCBW.u32DataTransferLength-dmaTotalTxSize;
        dmaMultipleTimes = MMP_FALSE;
    }

    USB_DEVICE_SetDmaLengthDirReg(dmaLength, DIRECTION_OUT);

    if(!dmaTotalTxSize)
    {
        result = USB_DEVICE_AllocateDmaAddr(&dmaBuffer1, tOTGCSW.u32DataResidue);
        if(result)
            goto end;
    }
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)(dmaBuffer1+dmaTotalTxSize));

    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_SETUP);
    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_IN);
    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_OUT);
    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_END);
    bOTGDMARunning = MMP_TRUE;
 
    USB_DEVICE_DmaStartReg(MMP_TRUE);
 
    dmaTotalTxSize += dmaLength;
    tOTGCSW.u32DataResidue = dmaTotalTxSize;

end:
    LOG_LEAVE "[USB_DEVICE_DataOut] Leave \n" LOG_END 
    if(result)
        LOG_ERROR "USB_DEVICE_DataOut() has error code 0x%08X \n", result LOG_END
    return STATE_CB_DATA_OUT;
}

///////////////////////////////////////////////////////////////////////////////
//		eOTGDataIn()
//		Description: Process the data intput stage
//		input: none
//		output: MassStorageState
///////////////////////////////////////////////////////////////////////////////
static MassStorageState USB_DEVICE_DataIn(void)
{
    MMP_INT result = 0;
    MMP_UINT32 dmaLength = 0;
    LOG_ENTER "[USB_DEVICE_DataIn]  Enter \n" LOG_END

    if(tOTGCSW.u8Status)
    {
        USB_DEVICE_StallInEndpointReg(EP1, MMP_TRUE); 
        return STATE_CSW;
    }

    USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
    USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_FIFO0_OUT);
    u32OTGFIFOUseDMA = FIFO0;
 
    if((tOTGCBW.u32DataTransferLength-dmaTotalTxSize) > USB_MAX_DMA_LEN)
    {
        dmaLength = USB_MAX_DMA_LEN;
        dmaMultipleTimes = MMP_TRUE;
    }
    else
    {
        dmaLength = tOTGCBW.u32DataTransferLength-dmaTotalTxSize;
        dmaMultipleTimes = MMP_FALSE;
    }

    USB_DEVICE_SetDmaLengthDirReg(dmaLength, DIRECTION_IN);
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_0);
	
    #if defined(__FREERTOS__)
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)(dmaBuffer+dmaTotalTxSize));
    #else
    result = USB_DEVICE_AllocateDmaAddr(&dmaBuffer1, dmaLength);
    if(result)
        goto end;
    HOST_WriteBlockMemory((MMP_UINT32)dmaBuffer1, (MMP_UINT32)(dmaBuffer+dmaTotalTxSize), dmaLength);
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)dmaBuffer1);
    #endif

    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_SETUP);
    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_IN);
    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_OUT);
    USB_DEVICE_DisableInterruptGroup0Reg(DEV_INT_G0_CX_END);
    bOTGDMARunning = MMP_TRUE;
    u32OTGFIFOUseDMA = FIFO0;
    USB_DEVICE_DmaStartReg(MMP_TRUE);
 
    dmaTotalTxSize += dmaLength;
    tOTGCSW.u32DataResidue = dmaTotalTxSize;
 
end:
    LOG_LEAVE "[USB_DEVICE_DataIn] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_DataIn() has error code 0x%08X \n", result LOG_END
    return STATE_CB_DATA_IN;
}

#if defined(DUMP_CBW_CSW)
///////////////////////////////////////////////////////////////////////////////
//		vShowCBW()
//		Description: show the whole CBW structure
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_ShowCBW(void)
{
    MMP_UINT8 i;
    MMP_UINT8* pp;

    LOG_ENTER "[USB_DEVICE_ShowCBW]  Enter \n" LOG_END

    pp = (MMP_UINT8 *)&tOTGCBW;
    LOG_DATA "tOTGCBW: \n" LOG_END
    for (i = 0; i < 16; i ++)
        LOG_DATA "%02x ", *(pp ++) LOG_END
    LOG_DATA "\n" LOG_END
    for (i = 16; i < 31; i ++)
        LOG_DATA "%02x ", *(pp ++) LOG_END
    LOG_DATA "\n" LOG_END

    LOG_LEAVE "[USB_DEVICE_ShowCBW] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//		vShowCSW()
//		Description: show the whole CSW structure
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_ShowCSW(void)
{
    MMP_UINT8 i;
    MMP_UINT8* pp;

    LOG_ENTER "[USB_DEVICE_ShowCSW]  Enter \n" LOG_END

    pp = (MMP_UINT8 *)&tOTGCSW;
    LOG_DATA "tOTGCSW: \n" LOG_END
    for (i = 0; i < 13; i ++)
        LOG_DATA "%02x ", *(pp ++) LOG_END
    LOG_DATA "\n" LOG_END

    LOG_LEAVE "[USB_DEVICE_ShowCSW] Leave \n" LOG_END
}
#endif

///////////////////////////////////////////////////////////////////////////////
//		vOTGSendCSW()
//		Description: Send out the CSW structure to PC
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void USB_DEVICE_SendCSW()
{
    LOG_ENTER "[USB_DEVICE_SendCSW]  Enter \n" LOG_END

    // Send CSW to F0 via DBUS;
    #if defined(DUMP_CBW_CSW)
    USB_DEVICE_ShowCSW();
    #endif

    #if defined(__FREERTOS__)
    {
        CSW tmpCsw = {0};
        tmpCsw.u32DataResidue = cpu_to_le32(tOTGCSW.u32DataResidue);
        tmpCsw.u32Signature = cpu_to_le32(tOTGCSW.u32Signature);
        tmpCsw.u32Tag = cpu_to_le32(tOTGCSW.u32Tag);
        tmpCsw.u8Status = tOTGCSW.u8Status;
        USB_DEVICE_DxFWr(FIFO0, (MMP_UINT8 *)(&tmpCsw) , 13);
    }
    #else
    USB_DEVICE_DxFWr(FIFO0, (MMP_UINT8 *)(&tOTGCSW) , 13);
    #endif
    USB_DEVICE_SetFifoDoneReg(FIFO0);

    LOG_LEAVE "[USB_DEVICE_SendCSW] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_FIFO_INT_action()
//		Description: FIFO interrupt enable or not
//					 depend on the STORAGE state
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void USB_DEVICE_FifoInterruptAction(MassStorageState eState)
{
    LOG_ENTER "[USB_DEVICE_FifoInterruptAction]  Enter \n" LOG_END

    switch(eState)
    {
        case STATE_CBW:
        case STATE_CB_DATA_OUT:
            USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
            break;
			
        case STATE_CSW:
        case STATE_CB_DATA_IN:
            USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
            break;
			
        default:
            break;
    }
    LOG_LEAVE "[USB_DEVICE_FifoInterruptAction] Leave \n" LOG_END
}


