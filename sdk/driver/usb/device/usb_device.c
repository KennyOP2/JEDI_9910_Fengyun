/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  usb_device.c usb device module Control
 *
 */
#if !defined(DISABLE_USB_DEVICE)
//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/device/device.h"
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#endif
#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

#include "usb_hw.c"
#include "usb_mem.c"
#if defined(CONFIG_HAVE_USBD)
#include "usbd/inc/it_usbd.h"
#else
#include "msc/usb_device_msc.c"
#endif



//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Private Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
/**   */
MMP_UINT8 u8OTGConfigValue;
MMP_UINT8 u8OTGInterfaceValue;
MMP_UINT8 u8OTGInterfaceAlternateSetting;

/** for DMA use */
MMP_UINT8   bOTGDMARunning;

MMP_UINT8* u8InterruptOTGArray = MMP_NULL;

SetupPacket ControlOTGCmd;
MMP_UINT8   * pu8OTGDescriptorEX;

MMP_UINT16 u16OTGTxRxCounter;
MMP_UINT8 bOTGEP0HaltSt;
CommandType eOTGCxCommand;
Action eOTGCxFinishAction;

MMP_UINT32 OTG_interrupt_level1;
MMP_UINT32 OTG_interrupt_level1_Save;
MMP_UINT32 OTG_interrupt_level1_Mask;

MMP_UINT8 bOTGHighSpeed;
MMP_UINT8 bOTGChirpFinish;

MMP_UINT8 u8OTGInterruptCount;
MMP_UINT32 u32OTGInterrupt_TX_COUNT ;
MMP_UINT32 u32OTGInterrupt_RX_COUNT ;
MMP_UINT8 u8OTGInterruptOutCount ;


#ifdef USB_LOGO_TEST
//2008.5.5 Jack add, for USB_LOGO_TEST USE, To Record Suspend Status
MMP_UINT8 status_suspend = 0;
MMP_UINT8 timeout_times = 0;
#endif


#if !defined(CONFIG_HAVE_USBD)
#include "usb_device_table.c"
#include "config/usb_device_string.c"
#include "config/usb_device_fifoepx.c"
#endif

//2008.12.23 Jack add, for USB Charge use
#ifdef USB_CHARGE_ENABLE
static MMP_BOOL isUSBCharge = MMP_TRUE;
static MMP_UINT32 usbCmdCount = 0;
#endif


//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Public Function Definition
//=============================================================================

//============================================================================= 
//      OTG_OTGP_main()
//      Description: Leave when the VBus is not valid 
//          
//      input: bDoNotInit   =0: Init
//                          =1: Do not init
//             bWaitForVBUS =0: Do not wait for VBUS
//                          =1: wait for VBUS 
//      output: 
//=============================================================================
MMP_INT OTG_OTGP_main(MMP_UINT8 bDoNotInit,MMP_UINT8 bWaitForVBUS,MMP_UINT8 bExitMode)
{
    MMP_INT result = 0;
    LOG_ENTER "[OTG_OTGP_main]  Enter bDoNotInit = 0x%4x bWaitForVBUS = 0x%4x bExitMode = 0x%4x\n",bDoNotInit ,bWaitForVBUS ,bExitMode LOG_END

    //2007.9.10 Jack add, This is to do OTG Mode init, may not need to do.
    //<1>.Waiting for the VBus high
    if(bWaitForVBUS>0)
    {
        result = USB_OTG_WaitHostVbusValidReg();
        if(result)
            goto end;
    }

    //2007.9.10 Jack add, This is to do Device Mode Initial, Need to do!!
    //<2>.Enable the Peripheral Interrupt
    if(bDoNotInit==1) 
    {
        result = OTG_DEVICE_init(1);//0:Do not init AP
        if(result)
            goto end;
    }
#if !defined(CONFIG_HAVE_USBD)
    else 
    {
        USB_DEVICE_InterruptInitial();
    }
#endif

    //<4>.Turn on D+
    USB_DEVICE_EnableUnplugReg(MMP_FALSE);//unplug issue;;
    
    #if defined(RUN_FPGA)
    /** if AHB>30Mhz, doesn't need to set */
    USB_DEVICE_EnableHalfSpeedReg();        
    #endif

end:
    LOG_LEAVE "[OTG_OTGP_main] Leave \n" LOG_END
    if(result)
        LOG_ERROR "OTG_OTGP_main() return error code 0x%08X \n", result LOG_END
    return result;
}

//============================================================================= ok
//      OTG_OTGP_init()
//      Description:
//      input: Reserved
//      output: Reserved
//=============================================================================
MMP_INT OTG_DEVICE_init(MMP_UINT8 bInitAP)
{
    MMP_INT result = 0;
    LOG_ENTER "[OTG_OTGP_init]  Enter bInitAP = 0x%4x \n",bInitAP LOG_END

#if !defined(CONFIG_HAVE_USBD)
    // Initial Interrupt and ISO transfer buffer
    if(bInitAP==1)
    {
        USB_DEVICE_InterruptInitial();
        result = USB_DEVICE_MSC_ApInitialize();
        if(result)
            goto end;
    }
#endif

    // Initial global variable
    bOTGDMARunning = MMP_FALSE;

    // Initial FUSB220 Reg
    result = OTG_vOTGInit();
    if(result)
        goto end;

end:
    LOG_LEAVE "[OTG_OTGP_init] Leave \n" LOG_END
    if(result)
        LOG_ERROR "OTG_OTGP_init() return error code 0x%08X \n", result LOG_END
    return result;
}

//============================================================================= ok
//      OTG_OTGP_Close()
//      Description:
//      input: Reserved
//      output: Reserved
//=============================================================================
void OTG_OTGP_Close(void)
{
    MMP_INT result = 0;
    MMP_UINT32 wTemp;

    LOG_ENTER "[OTG_OTGP_Close]  Enter  \n" LOG_END

#if !defined(CONFIG_HAVE_USBD)
    result = USB_DEVICE_MSC_ApTerminate();
#endif

    /** Clear All the Interrupt */
    USB_DEVICE_EnableGlobalInterruptReg(MMP_FALSE);

    //<2>.Clear all the Interrupt Status
    wTemp = USB_DEVICE_GetInterruptGroupStatusReg();  
    USB_DEVICE_ClearInterruptGroupStatusReg(0xFFFFFFFF); // Irene : so strange!!!!! It's read only????
    
    // Interrupt source group 0 (0x144)
    wTemp=USB_DEVICE_GetInterruptGroup0StatusReg();
    USB_DEVICE_ClearInterruptGroup0StatusReg(0xFFFFFFFF); // Irene : so strange!!!!! It's read only????
    
    // Interrupt source group 1 (0x148)          
    wTemp=USB_DEVICE_GetInterruptGroup1StatusReg();
    USB_DEVICE_ClearInterruptGroup1StatusReg(0xFFFFFFFF); // Irene : so strange!!!!! It's read only????
    
    // Interrupt source group 2 (0x14C)          
    wTemp=USB_DEVICE_GetInterruptGroup2StatusReg();
    USB_DEVICE_ClearInterruptGroup2StatusReg(0xFFFFFFFF); // Irene : so strange!!!!! It's read only????

    //<3>.Turn off D+  
    if(USB_OTG_CurrentRoleIsDevice())//For Current Role = Peripheral
        USB_DEVICE_EnableUnplugReg(MMP_TRUE);

    LOG_LEAVE "[OTG_OTGP_Close] Leave \n" LOG_END
    if(result)
        LOG_ERROR "OTG_OTGP_Close() has error code 0x%08X \n", result LOG_END
}

//============================================================================= 
//      OTG_OTGP_HNP_Enable()
//      Description:
//          
//      input: none
//      output: none
//=============================================================================
void OTG_OTGP_HNP_Enable(void)
{
    LOG_ENTER "[OTG_OTGP_HNP_Enable]  Enter  \n" LOG_END

    //<1>.Set b_Bus_Request
    USB_OTG_DeviceBusRequestReg(MMP_TRUE);

    //<2>.Set the HNP enable
    USB_OTG_DeviceEanbleHnpReg(MMP_TRUE);

    LOG_LEAVE "[OTG_OTGP_HNP_Enable] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//      OTG_vFOTG200_Dev_Init()
//      Description:
//          1. Reset all interrupt and clear all fifo data of FOTG200 Device
//          2. Turn on the "Global Interrupt Enable" bit of FOTG200 Device
//          3. Turn on the "Chip Enable" bit of FOTG200 Device
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT OTG_vFOTG200_Dev_Init(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[OTG_vFOTG200_Dev_Init]  Enter  \n" LOG_END

    // suspend counter
    USB_DEVICE_SetIdleCounterReg(7);

    // Clear interrupt
    USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_BUS_RESET |
                                             DEV_INT_G2_SUSPEND   |
                                             DEV_INT_G2_RESUME);
    
    // Disable all fifo interrupt
    USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_FIFO_OUT_ALL|DEV_INT_FIFO_IN_ALL);
        
    /** Soft Reset  */
    USB_DEVICE_SoftwareResetReg();
    
    // Clear all fifo
    USB_DEVICE_ClearFifoReg();  // will be cleared after one cycle.
    
    // Enable usb200 global interrupt
    USB_DEVICE_EnableGlobalInterruptReg(MMP_TRUE);
    USB_DEVICE_ChipEnableFifoReg();

    LOG_LEAVE "[OTG_vFOTG200_Dev_Init] Leave \n" LOG_END
    if(result)
        LOG_ERROR "OTG_vFOTG200_Dev_Init() return error code 0x%08X \n", result LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//      OTG_vOTGInit()
//      Description:
//          1. Configure the FIFO and EPx map.
//          2. Initiate FOTG200 Device.
//          3. Set the usb interrupt source as edge-trigger.
//          4. Enable Usb interrupt.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT OTG_vOTGInit(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[OTG_vOTGInit]  Enter  \n" LOG_END

    // init variables
    u16OTGTxRxCounter = 0;
    eOTGCxCommand = CMD_VOID;
    u8OTGConfigValue = 0;
    u8OTGInterfaceValue = 0;
    u8OTGInterfaceAlternateSetting = 0;
    bOTGEP0HaltSt = MMP_FALSE;
    
    eOTGCxFinishAction = ACT_IDLE;
    OTG_interrupt_level1 = 0;

    // Initiate FUSB220
    result = OTG_vFOTG200_Dev_Init();
    if(result)
        goto end;

end:
    LOG_LEAVE "[OTG_vOTGInit] Leave \n" LOG_END
    if(result)
        LOG_ERROR "OTG_vOTGInit() return error code 0x%08X \n", result LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Handler()
//      Description:
//          1. Service all Usb events
//          2. ReEnable Usb interrupt.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Handler(void)
{
    MMP_INT    result = 0;
    MMP_UINT32 OTG_Interrupt_level2 = 0;
    MMP_UINT32 OTG_Interrupt_Mask = 0;
    MMP_UINT32 OTG_Interrupt_Origan = 0;

    LOG_ENTER "[USB_DEVICE_Handler]  Enter  \n" LOG_END

    if(USB_DEVICE_IsGlobalInterruptEnabledReg()>0) 
    {
        OTG_interrupt_level1_Save = USB_DEVICE_GetInterruptGroupStatusReg();
        OTG_interrupt_level1_Mask = USB_DEVICE_GetInterruptGroupMaskReg();
              
        OTG_interrupt_level1 = OTG_interrupt_level1_Save & ~OTG_interrupt_level1_Mask;

        LOG_DEBUG "[USB_DEVICE_Handler]  OTG_interrupt_level1_Save = 0x%4x  OTG_interrupt_level1_Mask = 0x%4x OTG_interrupt_level1 = 0x%4x\n",OTG_interrupt_level1_Save,OTG_interrupt_level1_Mask,OTG_interrupt_level1 LOG_END

        /** No interrupt enabled */
        if(OTG_interrupt_level1 == 0) 
            goto end;
    }

//2008.12.23 Jack add, for USB Charge use
#ifdef USB_CHARGE_ENABLE
    if(usbCmdCount > 1)
        USB_DEVICE_SetIsUSBCharge(MMP_FALSE);
    usbCmdCount++;
#endif



    //=====================================
    // Source Group 2
    //=====================================
    if(OTG_interrupt_level1 & USB_DEVICE_MSK_INT_GROUP2_MASK)               //Group Source 2
    {
        OTG_Interrupt_Origan = USB_DEVICE_GetInterruptGroup2StatusReg();
        OTG_Interrupt_Mask   = USB_DEVICE_GetInterruptGroup2MaskReg();
        OTG_Interrupt_level2 = OTG_Interrupt_Origan & ~OTG_Interrupt_Mask;

        LOG_DEBUG "[USB_DEVICE_Handler]  OTG_Interrupt_Origan = 0x%4x  OTG_Interrupt_Mask = 0x%4x OTG_Interrupt_level2 = 0x%4x\n",OTG_Interrupt_Origan,OTG_Interrupt_Mask,OTG_Interrupt_level2 LOG_END

        /** USB Reset Interrupt (Bus reset interrupt bit.) */
        if(OTG_Interrupt_level2 & DEV_INT_G2_BUS_RESET)
        {
            LOG_CMD "USB_DEVICE_Handler() G2: Bus reset! \n" LOG_END
            result = USB_DEVICE_BusReset();
            if(result)
                LOG_ERROR "USB_DEVICE_BusReset@USB_DEVICE_Handler() has error code 0x%08X \n", result LOG_END
        }

        /** Suspend Interrupt (Suspend-state-change interrupt bit.)  */
        if(OTG_Interrupt_level2 & DEV_INT_G2_SUSPEND)
        {
            LOG_CMD "USB_DEVICE_Handler() G2: Supsend! \n" LOG_END
            USB_DEVICE_Suspend();
            /**
             * Irene 2010_01_15
             * To aviod SUSPEND and G1 group appear at the same time.
             * It may cause DMA dead....
             */
            if(OTG_interrupt_level1 & USB_DEVICE_MSK_INT_GROUP1_MASK)
            {
                OTG_Interrupt_Origan = USB_DEVICE_GetInterruptGroup1StatusReg();
                OTG_Interrupt_Mask   = USB_DEVICE_GetInterruptGroup1MaskReg();
                OTG_Interrupt_level2 = OTG_Interrupt_Origan & ~OTG_Interrupt_Mask;
                USB_DEVICE_DisableInterruptGroup1Reg(OTG_Interrupt_level2);
            }
        }

        /** Resume Interrupt (Resume-state-change interrupt bit.) */
        if(OTG_Interrupt_level2 & DEV_INT_G2_RESUME)
        {
            LOG_CMD "USB_DEVICE_Handler() G2: Resume! \n" LOG_END
            USB_DEVICE_Resume();
        }
        
        /** ISO Sequential Error Interrupt  */
        if(OTG_Interrupt_level2 & DEV_INT_G2_ISO_SEQUENTIAL_ERROR)
        {
            LOG_INFO "USB_DEVICE_Handler() G2: Sequence error! \n" LOG_END
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_ISO_SEQUENTIAL_ERROR);
            USB_DEVICE_IsoSequentialError();
        }
        
        /** ISO Sequential Abort Interrupt  */
        if(OTG_Interrupt_level2 & DEV_INT_G2_ISO_SEQUENTIAL_ABORT)
        {
            LOG_INFO "USB_DEVICE_Handler() G2: Sequence abort! \n" LOG_END
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_ISO_SEQUENTIAL_ABORT);
            USB_DEVICE_IsoSequentialAbort();
        }
        
        /** Transferred Zero-length Data Packet Interrupt   */
        if(OTG_Interrupt_level2 & DEV_INT_G2_TX_ZERO_BTYE)
        {
            LOG_INFO "USB_DEVICE_Handler() G2: Tx zero byte! \n" LOG_END
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_TX_ZERO_BTYE);
            USB_DEVICE_Tx0Byte();
        }   
        
        /** Received Zero-length Data Packet Interrupt  */  
        if(OTG_Interrupt_level2 & DEV_INT_G2_RX_ZERO_BYTE)
        {
            LOG_INFO "USB_DEVICE_Handler() G2: Rx zero byte! \n" LOG_END
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_RX_ZERO_BYTE);  
            USB_DEVICE_Rx0Byte();     
        }
        
        /** DMA Completion Interrupt (The DMA operation has finished normally.) */
        if(OTG_Interrupt_level2 & DEV_INT_G2_DMA_COMPLETION)
        {
            LOG_INFO "USB_DEVICE_Handler() G2: DMA completion! \n" LOG_END
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            if(bOTGDMARunning)
                USB_DEVICE_MSC_CheckDMA();
            else
                LOG_ERROR "[USB_DEVICE_Handler]  DMA finish Interrupt error!!\n" LOG_END
        }   
        
        /** DMA Error Interrupt */
        if(OTG_Interrupt_level2 & DEV_INT_G2_DMA_ERROR)
        {
            LOG_ERROR "USB_DEVICE_Handler() G2: DMA error! \n" LOG_END
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_ERROR);
            if (bOTGDMARunning)
            {
                USB_DEVICE_MSC_CheckDMA();
                LOG_ERROR "[USB_DEVICE_Handler] DMA error 2!!\n" LOG_END           
            }
            else
                LOG_ERROR "[USB_DEVICE_Handler] DMA error 3!!\n" LOG_END
        }
    }

    //=====================================
    // Source Group 0
    //=====================================
    if(OTG_interrupt_level1 & USB_DEVICE_MSK_INT_GROUP0_MASK)               //Group Source 0
    {
        OTG_Interrupt_Origan = USB_DEVICE_GetInterruptGroup0StatusReg();
        OTG_Interrupt_Mask = USB_DEVICE_GetInterruptGroup0MaskReg();
        OTG_Interrupt_level2 = OTG_Interrupt_Origan & ~OTG_Interrupt_Mask;

        /** This bit indicates a command abort event happened. */
        if(OTG_Interrupt_level2 & DEV_INT_G0_CX_CMD_ABORT)
        {
            LOG_INFO "USB_DEVICE_Handler() G0: Cx command abort! \n" LOG_END
            USB_DEVICE_Ep0Abort();
        }
        
        /** This bit indicates the control transfer contains valid data for control-write transfers. */
        if(OTG_Interrupt_level2 & DEV_INT_G0_CX_OUT)
        {
            LOG_INFO "USB_DEVICE_Handler() G0: Cx out! \n" LOG_END
            result = USB_DEVICE_Ep0Rx();
        }
        
        /** This bit will remain asserted until the firmware starts to read data from control transfer FIFO of device. */
        if(OTG_Interrupt_level2 & DEV_INT_G0_CX_SETUP)
        {
            LOG_INFO "USB_DEVICE_Handler() G0: Cx setup! \n" LOG_END
            USB_DEVICE_Ep0Setup();
        }
        
        /** This bit indicates the control transfer has entered status stage. */
        if(OTG_Interrupt_level2 & DEV_INT_G0_CX_END)
        {
            /** 
             * Because we use polling mechanism, so SETUP and STATUS stages may be asserted at the same time.
             * For this case we should process STALL first, or it will casuse STALL handshake fail.
             */
            if(OTG_Interrupt_level2 & DEV_INT_G0_CX_SETUP)
            {
                if(eOTGCxFinishAction == ACT_STALL)
                {
                    LOG_ERROR "[USB_DEVICE_Handler] Unsuported EP0 command...Return Cx Stall...!!\n" LOG_END           
                    USB_DEVICE_StallCxReg();
                }
                else if(eOTGCxFinishAction == ACT_DONE)
                {
                    LOG_CMD "USB_DEVICE_Handler() DEVICE set Cx done! \n" LOG_END
                    USB_DEVICE_SetCxDoneReg();
                }
                // Clear Action
                eOTGCxFinishAction = ACT_IDLE;
            }
            LOG_CMD "USB_DEVICE_Handler() G0: Cx end! \n" LOG_END
            USB_DEVICE_Ep0End();
        }
        
        /** This bit indicates the firmware should write data for control-read transfer to control transfer FIFO. */
        if(OTG_Interrupt_level2 & DEV_INT_G0_CX_IN)
        {
            LOG_INFO "USB_DEVICE_Handler() G0: Cx in! \n" LOG_END
            USB_DEVICE_Ep0Tx();
        }
        
        /** This bit indicates the control transfer has abnormally terminated. */
        if(OTG_Interrupt_level2 & DEV_INT_G0_CX_CMD_FAIL)
        {
            LOG_ERROR "USB_DEVICE_Handler() G0: Cx command fail! \n" LOG_END
            USB_DEVICE_Ep0Fail();
        }

        if(eOTGCxFinishAction == ACT_STALL)
        {
            LOG_ERROR "[USB_DEVICE_Handler] Unsuported EP0 command...Return Cx Stall...!!\n" LOG_END           
            USB_DEVICE_StallCxReg();
        }
        else if(eOTGCxFinishAction == ACT_DONE)
        {
            USB_DEVICE_SetCxDoneReg();
        }

        // Clear Action
        eOTGCxFinishAction = ACT_IDLE;
    }

#if !defined(CONFIG_HAVE_USBD)
    //=====================================
    // Source Group 1
    //=====================================
    if(OTG_interrupt_level1 & USB_DEVICE_MSK_INT_GROUP1_MASK)               //Group Source 1
    {
        OTG_Interrupt_Origan = USB_DEVICE_GetInterruptGroup1StatusReg();
        OTG_Interrupt_Mask   = USB_DEVICE_GetInterruptGroup1MaskReg();//0x138
        OTG_Interrupt_level2 = OTG_Interrupt_Origan & ~OTG_Interrupt_Mask;
        
        /** use FIFO0 for ep2 (bulk out) */
        if(OTG_Interrupt_level2 & DEV_INT_G1_FIFO0_SPK) /* short packet */
        {
            LOG_INFO "USB_DEVICE_Handler() G1: FIFO 0 short packet bulk out! \n" LOG_END
            USB_DEVICE_MSC_BulkOut(USB_DEVICE_GetFifoOutByteCountReg(FIFO0));
        }
        else if(OTG_Interrupt_level2 & DEV_INT_G1_FIFO0_OUT) /* full packet */
        {
            LOG_INFO "USB_DEVICE_Handler() G1: FIFO 0 bulk out! \n" LOG_END
            USB_DEVICE_MSC_BulkOut(USB_DEVICE_GetFifoOutByteCountReg(FIFO0));
        }
        
        /** use FIFO0 for ep1 (bulk in) */
        if(OTG_Interrupt_level2 & DEV_INT_G1_FIFO0_IN)
        {
            LOG_INFO "USB_DEVICE_Handler() G1: FIFO 0 bulk in! \n" LOG_END
            USB_DEVICE_MSC_BulkIn();
        }
            
        /** use FIFO3 for ep4 (Interrupt out) */
        if(OTG_Interrupt_level2 & DEV_INT_G1_FIFO3_SPK) /* short packet */
        {
            LOG_CMD "USB_DEVICE_Handler() G1: FIFO 3 interrupt short packet out! \n" LOG_END
            USB_DEVICE_InterruptOut();
        }
        else if(OTG_Interrupt_level2 & DEV_INT_G1_FIFO3_OUT) /* full packet */
        {
            LOG_CMD "USB_DEVICE_Handler() G1: FIFO 3 interrupt out! \n" LOG_END
            USB_DEVICE_InterruptOut();
        }
        
        /** use FIFO2 for ep3 (Interrupt In) */
        if(OTG_Interrupt_level2 & DEV_INT_G1_FIFO2_IN)
        {
            LOG_CMD "USB_DEVICE_Handler() G1: FIFO 2 interrupt in! \n" LOG_END
            USB_DEVICE_InterruptIn();
        }
    }
#else
    it_usbd_do_isr_g1();
#endif

end:
    // Clear usb interrupt flags
    OTG_interrupt_level1 = 0;
    LOG_LEAVE "[USB_DEVICE_Handler] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0Setup()
//      Description:
//          1. Read 8-byte setup packet.
//          2. Decode command as Standard, Class, Vendor or NOT support command
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Setup(void)
{
    MMP_INT     result = 0;
    MMP_UINT8   c = 0;
    MMP_UINT8*  u8UsbCmd = MMP_NULL;
    MMP_UINT8   u8index = 0;

    LOG_ENTER "[USB_DEVICE_Ep0Setup]  Enter  \n" LOG_END

    //====================================================
    /**
     * First we must check if this is the first Cx 8 byte command after USB reset.
     * If this is the first Cx 8 byte command, we can check USB High/Full speed right now.
     */
    //====================================================
    if(!bOTGChirpFinish)
    {
        // first ep0 command after usb reset, means we can check usb speed right now.
        bOTGChirpFinish = MMP_TRUE;

        if(USB_DEVICE_IsHighSpeedModeReg())
        {
            LOG_INFO "[USB_DEVICE_Ep0Setup]  HIGH speed mode!!\n" LOG_END
            //==============================
            // Device stays in High Speed
            //==============================
            bOTGHighSpeed = MMP_TRUE;
            
            /**
             * <1> copy Device descriptors (Table 9.8)
             */
            memcpy((void*)u8OTGDeviceDescriptorEX, (void*)u8OTGHSDeviceDescriptor, sizeof(u8OTGHSDeviceDescriptor));

            /**
             * <2> copy Device Qualifier descriptors (Table 9.9)  Irene : why use full speed????
             */
            /** bLength */
            u8OTGDeviceQualifierDescriptorEX[0] = DEVICE_QUALIFIER_LENGTH;
            /** bDescriptorType Device_Qualifier */
            u8OTGDeviceQualifierDescriptorEX[1] = DT_DEVICE_QUALIFIER;
            for(u8index=2 ; u8index<8; u8index++)
                u8OTGDeviceQualifierDescriptorEX[u8index] = u8OTGFSDeviceDescriptor[u8index];
            /** Number of Other-speed Configurations */
            u8OTGDeviceQualifierDescriptorEX[8] = u8OTGFSDeviceDescriptor[17];
            /** Reserved for future use, must be zero  */
            u8OTGDeviceQualifierDescriptorEX[9] = 0x00;

            /**
             * <3> copy Device Config descriptors (Table 9.10)
             */
            memcpy((void*)u8ConfigOTGDescriptorEX, (void*)u8HSConfigOTGDescriptor01, sizeof(u8HSConfigOTGDescriptor01));

            /**
             * <4> copy Other speed Config descriptors (Table 9.11)
             */
            memcpy((void*)u8OtherSpeedConfigOTGDescriptorEX, (void*)u8FSConfigOTGDescriptor01, sizeof(u8FSConfigOTGDescriptor01));
            /** Change Descriptor type "DT_OTHER_SPEED_CONFIGURATION" */
            u8OtherSpeedConfigOTGDescriptorEX[1] = DT_OTHER_SPEED_CONFIGURATION;
        }
        else
        {
            LOG_INFO "[USB_DEVICE_Ep0Setup]  FULL speed mode!!\n" LOG_END
            //==============================
            // Device stays in Full Speed
            //==============================
            bOTGHighSpeed = MMP_FALSE;

            /**
             * <1> copy Device descriptors (Table 9.8)
             */
            memcpy((void*)u8OTGDeviceDescriptorEX, (void*)u8OTGFSDeviceDescriptor, sizeof(u8OTGFSDeviceDescriptor));

            /**
             * <2> copy Device Qualifierdescriptors (Table 9.9)  Irene : why use high speed????
             */
            /** bLength */
            u8OTGDeviceQualifierDescriptorEX[0] = DEVICE_QUALIFIER_LENGTH;
            /** bDescriptorType Device_Qualifier */
            u8OTGDeviceQualifierDescriptorEX[1] = DT_DEVICE_QUALIFIER;
            for(u8index=2 ; u8index<8; u8index++)
                u8OTGDeviceQualifierDescriptorEX[u8index] = u8OTGHSDeviceDescriptor[u8index];
            /** Number of Other-speed Configurations */
            u8OTGDeviceQualifierDescriptorEX[8] = u8OTGHSDeviceDescriptor[17];
            /** Reserved for future use, must be zero  */
            u8OTGDeviceQualifierDescriptorEX[9] = 0x00;

            /**
             * <3> copy Device Config descriptors (Table 9.10)
             */
            memcpy((void*)u8ConfigOTGDescriptorEX, (void*)u8FSConfigOTGDescriptor01, sizeof(u8FSConfigOTGDescriptor01));

            /**
             * <4> copy Other speed Config descriptors (Table 9.11)
             */
            memcpy((void*)u8OtherSpeedConfigOTGDescriptorEX, (void*)u8HSConfigOTGDescriptor01, sizeof(u8HSConfigOTGDescriptor01));
            /** Change Descriptor type "DT_OTHER_SPEED_CONFIGURATION" */
            u8OtherSpeedConfigOTGDescriptorEX[1] = DT_OTHER_SPEED_CONFIGURATION;
        }

        /**
         * copy String descriptors
         */

#if !defined(CONFIG_HAVE_USBD)
        memcpy((void*)u8OTGString00DescriptorEX, (void*)u8OTGString00Descriptor, sizeof(u8OTGString00Descriptor));
        memcpy((void*)u8OTGString10DescriptorEX, (void*)u8OTGString10Descriptor, sizeof(u8OTGString10Descriptor));
        memcpy((void*)u8OTGString20DescriptorEX, (void*)u8OTGString20Descriptor, sizeof(u8OTGString20Descriptor));
        memcpy((void*)u8OTGString30DescriptorEX, (void*)u8OTGString30Descriptor, sizeof(u8OTGString30Descriptor));
        memcpy((void*)u8OTGString40DescriptorEX, (void*)u8OTGString40Descriptor, sizeof(u8OTGString40Descriptor));
        memcpy((void*)u8OTGString50DescriptorEX, (void*)u8OTGString50Descriptor, sizeof(u8OTGString50Descriptor));
#else
        it_usbd_init_string_descriptors();
#endif
    }

    //==========================================
    // Read 8-byte setup packet from FIFO
    //==========================================
    u8UsbCmd = (MMP_UINT8*)SYS_Malloc(8);   
    USB_DEVICE_GetCxSetupCmdReg(u8UsbCmd);

    //==========================================
    // Save to setup command data structure.
    //==========================================
    c = u8UsbCmd[0];                                            // get 1st byte
    ControlOTGCmd.Direction = (MMP_UINT8)(c & 0x80);            // xfer Direction(IN, OUT)
    ControlOTGCmd.Type = (MMP_UINT8)(c & REQUEST_TYPE_MASK);    // type(Standard, Class, Vendor)
    ControlOTGCmd.Object = (MMP_UINT8)(c & RECIPIENT_TYPE_MASK);// Device, Interface, Endpoint
    ControlOTGCmd.Request = u8UsbCmd[1];                        // get 2nd byte
    ControlOTGCmd.Value = u8UsbCmd[2];                          // get 3rd byte      
    c = u8UsbCmd[3];                                            // get 4th byte
    ControlOTGCmd.Value |= (c<<8);

    ControlOTGCmd.Index = u8UsbCmd[4];                      // get 5th byte
    c = u8UsbCmd[5];                                        // get 6th byte
    ControlOTGCmd.Index |= (c<<8);

    ControlOTGCmd.Length = u8UsbCmd[6];                     // get 7th byte
    c = u8UsbCmd[7];                                        // get 8th byte
    ControlOTGCmd.Length |= (c<<8);  

    if(!(((u8UsbCmd[0] == 0x40)&&(u8UsbCmd[1] == 0x00)&&(u8UsbCmd[2] == 0x00)&&(u8UsbCmd[3] == 0x00)&&(u8UsbCmd[4] == 0x00)&&(u8UsbCmd[5] == 0x00))||
        ((u8UsbCmd[0] == 0xC0)&&(u8UsbCmd[1] == 0x00)&&(u8UsbCmd[2] == 0x00)&&(u8UsbCmd[3] == 0x00)&&(u8UsbCmd[4] == 0x00)&&(u8UsbCmd[5] == 0x00))))
    {       
        // do not print test vendor command
        LOG_DATA "[USB_DEVICE_Ep0Setup] EP0Cmd: " LOG_END
        LOG_DATA "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n",
                  u8UsbCmd[0], u8UsbCmd[1], u8UsbCmd[2], u8UsbCmd[3], 
                  u8UsbCmd[4], u8UsbCmd[5], u8UsbCmd[6], u8UsbCmd[7] LOG_END
    }

    //==========================================
    // Command Decode
    //==========================================
    if((ControlOTGCmd.Type & REQUEST_TYPE_MASK) == REQUEST_TYPE_STANDARD)     // standard command
    {
        result = USB_DEVICE_StandardCommand();
        if(result)
            goto end;
    }
#if !defined(CONFIG_HAVE_USBD)
    else if((ControlOTGCmd.Type & REQUEST_TYPE_MASK) == REQUEST_TYPE_CLASS)   // class command
    {
        result = USB_DEVICE_ClassCommand();
        if(result)
            goto end;
    }
#endif
    else if((ControlOTGCmd.Type & REQUEST_TYPE_MASK) == REQUEST_TYPE_VENDOR)  // vendor command
    {   
        LOG_ERROR " Unsupported REQUEST_TYPE_VENDOR 0x%02X \n", ControlOTGCmd.Type LOG_END
        result = ERROR_USB_DEVICE_INVALID_REQUEST_TYPE;
        goto end;
    }
    else
    {
        result = ERROR_USB_DEVICE_INVALID_REQUEST_TYPE;
        goto end;
    }
    
end:
    if(u8UsbCmd)
        SYS_Free(u8UsbCmd);

    LOG_LEAVE "[USB_DEVICE_Ep0Setup] Leave \n" LOG_END
    if(result)
    {
        eOTGCxFinishAction = ACT_STALL;
        LOG_ERROR "USB_DEVICE_Ep0Setup() has error code 0x%08X \n", result LOG_END
    }
    return;
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0Tx()
//      Description:
//          1. Transmit data to EP0 FIFO.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Tx(void)
{
    LOG_ENTER "[USB_DEVICE_Ep0Tx]  Enter  \n" LOG_END

    switch(eOTGCxCommand)
    {
        case CMD_GET_DESCRIPTOR:
            USB_DEVICE_Ep0TxData();
            break;
        case CMD_CxIN_Vendor:
            /** Cx In Vendor Tx data */
            break;
        default:
            USB_DEVICE_StallCxReg();    
            break;
    }
    LOG_LEAVE "[USB_DEVICE_Ep0Tx] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0Rx()
//      Description:
//          1. Receive data from EP0 FIFO.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_Ep0Rx(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[USB_DEVICE_Ep0Rx]  Enter  \n" LOG_END

    switch(eOTGCxCommand)
    {
    case CMD_SET_DESCRIPTOR:
        result = USB_DEVICE_Ep0RxData();
        break;
    case CMD_CxOUT_Vendor:
        /** Cx Out Vendor Rx data */
        break;
    default:
        USB_DEVICE_StallCxReg();
        break;
    }

    LOG_LEAVE "[USB_DEVICE_Ep0Rx] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_Ep0Rx() return error code 0x%08X \n", result LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0End()
//      Description:
//          1. End this transfer.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0End(void)
{
    LOG_ENTER "[USB_DEVICE_Ep0End]  Enter  \n" LOG_END

    eOTGCxCommand = CMD_VOID;
    USB_DEVICE_SetCxDoneReg();                              // Return EP0_Done flag
    LOG_LEAVE "[USB_DEVICE_Ep0End] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0Fail()
//      Description:
//          1. Stall this transfer.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Fail(void)
{
    LOG_ENTER "[USB_DEVICE_Ep0Fail]  Enter  \n" LOG_END

    LOG_DEBUG "[USB_DEVICE_Ep0Fail] EP0 fail !!\n" LOG_END

    USB_DEVICE_StallCxReg();                                // Return EP0_Stall

    LOG_LEAVE "[USB_DEVICE_Ep0Fail] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0Abort()
//      Description:
//          1. Stall this transfer.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0Abort(void)
{
    LOG_ENTER "[USB_DEVICE_Ep0Abort]  Enter  \n" LOG_END

    USB_DEVICE_ClearInterruptGroup0StatusReg(DEV_INT_G0_CX_CMD_ABORT);  // Irene : so stange!!! It's read only??

    LOG_LEAVE "[USB_DEVICE_Ep0Abort] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_BusReset()
//      Description:
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_BusReset(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[USB_DEVICE_BusReset]  Enter  \n" LOG_END


#ifdef USB_LOGO_TEST
    //2008.5.5 Jack add, for USB_LOGO_TEST USE, To Record Suspend Status
    status_suspend = 0;
    timeout_times = 0;
#endif


    USB_DEVICE_SetDeviceAddressReg(0); 
#if !defined(CONFIG_HAVE_USBD)
    // Init AP
    result = USB_DEVICE_MSC_ApInitialize(); 
    if(result)
        goto end;
    
    // start
    USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_FIFO0_OUT);
    USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_G1_FIFO2_IN);
    USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_FIFO3_OUT);     
    USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
#else
    it_usbd_init_fifo_interrupt_mask();
#endif

    USB_DEVICE_EnableInterruptGroup0Reg(DEV_INT_G0_CX_SETUP|DEV_INT_G0_CX_IN|DEV_INT_G0_CX_OUT|DEV_INT_G0_CX_END);
    USB_DEVICE_DisableInterruptGroup2Reg(DEV_INT_G2_WAKEUP_BY_VBUS|DEV_INT_G2_IDLE); // Irene 2010/01/15

    USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_BUS_RESET);
    USB_DEVICE_ClearFifoReg();
    bOTGChirpFinish = MMP_FALSE;

end:
    LOG_LEAVE "[USB_DEVICE_BusReset] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_BusReset() return error code 0x%08X \n", result LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Suspend()
//      Description:
//          1. Clear suspend interrupt, and set suspend register.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Suspend(void)
{
    LOG_ENTER "[USB_DEVICE_Suspend]  Enter  \n" LOG_END
    
    USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_SUSPEND);

#ifdef USB_LOGO_TEST
    //2008.5.5 Jack add, for USB_LOGO_TEST USE, To Record Suspend Status
    status_suspend = 1;
    timeout_times = 90;
    //2008.3.13 Jack add, Need to make phy enter suspend mode and turm off u_clk
    USB_DEVICE_SetPhyGoSuspendReg();
#endif
    
    LOG_LEAVE "[USB_DEVICE_Suspend] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Resume()
//      Description:
//          1. Clear resume interrupt status and leave supend mode.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Resume(void)
{
    LOG_ENTER "[USB_DEVICE_Resume]  Enter  \n" LOG_END

    USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_RESUME);

#ifdef USB_LOGO_TEST
    //2008.5.5 Jack add, for USB_LOGO_TEST USE, To Record Suspend Status
    status_suspend = 0;
    timeout_times = 0;
    //2008.4.14 Jack add, Need to clear phy go to suspend mode bit and return to normal mode
    USB_DEVICE_ClearPhyGoSuspendReg();
#endif

    LOG_LEAVE "[USB_DEVICE_Resume] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_StandardCommand()
//      Description:
//          1. Process standard Cx 8 bytes command.
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_StandardCommand(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_StandardCommand]  Enter  \n" LOG_END

    // by Standard Request codes
    switch(ControlOTGCmd.Request) 
    {
    case REQUEST_CODE_GET_STATUS:
        LOG_CMD " Standard Command: REQUEST_CODE_GET_STATUS \n" LOG_END
        result = USB_DEVICE_CxGetStatus();
        break;
        
    case REQUEST_CODE_CLEAR_FEATURE:
        LOG_CMD " Standard Command: REQUEST_CODE_CLEAR_FEATURE \n" LOG_END
        result = USB_DEVICE_CxClearFeature();
        break;

    case REQUEST_CODE_RESERVED:
        LOG_CMD " Standard Command: REQUEST_CODE_RESERVED \n" LOG_END
        result = ERROR_USB_DEVICE_INVALID_REQUEST_CODE;
        break;

    case REQUEST_CODE_SET_FEATURE:
        LOG_CMD " Standard Command: REQUEST_CODE_SET_FEATURE \n" LOG_END
        result = USB_DEVICE_CxSetFeature();
        break;

    case REQUEST_CODE_RESERVED1:
        LOG_CMD " Standard Command: REQUEST_CODE_RESERVED1 \n" LOG_END
        result = ERROR_USB_DEVICE_INVALID_REQUEST_CODE;
        break;

    case REQUEST_CODE_SET_ADDRESS:
        LOG_CMD " Standard Command: REQUEST_CODE_SET_ADDRESS \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxSetAddress();
        break;

    case REQUEST_CODE_GET_DESCRIPTOR:
        LOG_CMD " Standard Command: REQUEST_CODE_GET_DESCRIPTOR \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxGetDescriptor();
        break;

    case REQUEST_CODE_SET_DESCRIPTOR:
        LOG_CMD " Standard Command: REQUEST_CODE_SET_DESCRIPTOR \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxSetDescriptor();
        break;

    case REQUEST_CODE_GET_CONFIGURATION:
        LOG_CMD " Standard Command: REQUEST_CODE_GET_CONFIGURATION \n" LOG_END
        if(!bOTGEP0HaltSt)
            USB_DEVICE_CxGetConfiguration();
        break;

    case REQUEST_CODE_SET_CONFIGURATION:
        LOG_CMD " Standard Command: REQUEST_CODE_SET_CONFIGURATION \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxSetConfiguration();
        break;

    case REQUEST_CODE_GET_INTERFACE:
        LOG_CMD " Standard Command: REQUEST_CODE_GET_INTERFACE \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxGetInterface();
        break;

    case REQUEST_CODE_SET_INTERFACE:
        LOG_CMD " Standard Command: REQUEST_CODE_SET_INTERFACE \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxSetInterface();
        break;

    case REQUEST_CODE_SYNCH_FRAME:
        LOG_CMD " Standard Command: REQUEST_CODE_SYNCH_FRAME \n" LOG_END
        if(!bOTGEP0HaltSt)
            result = USB_DEVICE_CxSynchFrame();
        break;

    default:
        result = ERROR_USB_DEVICE_INVALID_REQUEST_CODE; 
        break;
    }
    
    LOG_LEAVE "[USB_DEVICE_StandardCommand] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_StandardCommand() return error code 0x%08X, request code = %d \n", result, ControlOTGCmd.Request LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxGetStatus()
//      Description:
//          1. Send 2 bytes status to host.
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxGetStatus(void)
{
    MMP_INT   result = 0;
    MMP_UINT8 endpointIndex = 0;
    MMP_UINT8 fifoIndex = 0;
    MMP_BOOL  direction = 0;
    MMP_UINT8 RecipientStatusLow  = 0;
    MMP_UINT8 RecipientStatusHigh = 0;
    MMP_UINT8 u8Tmp[2] = {0};

    LOG_ENTER "[USB_DEVICE_CxGetStatus]  Enter  \n" LOG_END
    
    switch(ControlOTGCmd.Object) // Judge which recipient type is at first
    {
    case RECIPIENT_TYPE_DEVICE:
        /**
         * Return 2-byte's Device status (Bit1:Remote_Wakeup, Bit0:Self_Powered) to Host
         * Notice that the programe sequence of RecipientStatus
         */
        RecipientStatusLow = (USB_DEVICE_GetRemoteWakeupCapsReg() == MMP_TRUE) ? BIT1: 0;
        // Bit0: Self_Powered--> DescriptorTable[0x23], D6(Bit 6)
        RecipientStatusLow |= ((u8ConfigOTGDescriptorEX[0x07] >> 6) & 0x01);
        break;
        
    case RECIPIENT_TYPE_INTERFACE:
        // Return 2-byte ZEROs Interface status to Host
        break;

    case RECIPIENT_TYPE_ENDPOINT:
        if(ControlOTGCmd.Index == 0x00)
        {
            RecipientStatusLow = (MMP_UINT8)bOTGEP0HaltSt;
        }
        else
        {
            endpointIndex = ControlOTGCmd.Index & 0x7F;      // which ep will be clear
            direction = ControlOTGCmd.Index >> 7;            // the direction of this ep
            if(endpointIndex > DEVICE_MAX_ENDPOINT_NUM)      // over the Max. ep count ?
            {
                result = ERROR_USB_DEVICE_INVALID_ENDPOINT_NUM;
                LOG_ERROR "endpointIndex = %d \n", endpointIndex LOG_END
                goto end;
            }

            fifoIndex = USB_DEVICE_GetEndpointMapReg(endpointIndex, direction);      // get the relatived FIFO number
            if(fifoIndex >= DEVICE_MAX_FIFO_NUM) // over the Max. fifo count ?
            {
                result = ERROR_USB_DEVICE_INVALID_FIFO_NUM;
                LOG_ERROR "fifoIndex = %d \n", fifoIndex LOG_END
                goto end;
            }

            // Check the FIFO had been enable ?
            if((USB_DEVICE_GetFifoConfigReg(fifoIndex) & USB_DEVICE_MSK_FIFO_ENABLE) == 0)
            {
                result = ERROR_USB_DEVICE_FIFO_NOT_ENABLED;
                goto end;
            }
            
            if(direction == INDEX_DIRECTION_IN)
                RecipientStatusLow = (MMP_UINT8)USB_DEVICE_IsInEndpointStallReg(endpointIndex);
            else
                RecipientStatusLow = (MMP_UINT8)USB_DEVICE_IsOutEndpointStallReg(endpointIndex);
        }
        break;
        
    default :
        result = ERROR_USB_DEVICE_INVALID_RECIPIENT;
        goto end;
    }

    // return RecipientStatus;
    u8Tmp[0] = RecipientStatusLow;
    u8Tmp[1] = RecipientStatusHigh;
    USB_DEVICE_CxFWr(u8Tmp, 2);
    
    eOTGCxFinishAction = ACT_DONE;

end:
    LOG_LEAVE "[USB_DEVICE_CxGetStatus] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxGetStatus() return error code 0x%08X \n", result LOG_END
    return result;
}



///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxClearFeature()
//      Description:
//          1. Send 2 bytes status to host.
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxClearFeature(void)
{
    MMP_INT   result = 0;
    MMP_UINT8 endpointIndex = 0;
    MMP_UINT8 fifoIndex = 0;
    MMP_BOOL  direction = 0;

    LOG_ENTER "[USB_DEVICE_CxClearFeature]  Enter  \n" LOG_END

    // FeatureSelector
    switch(ControlOTGCmd.Value)        
    {
    case FEATURE_SEL_ENDPOINT_HALT:
        // Clear "Endpoint_Halt", Turn off the "STALL" bit in Endpoint Control Function Register
        if(ControlOTGCmd.Index == 0x00)
            bOTGEP0HaltSt = MMP_FALSE;
        else
        {
            endpointIndex = ControlOTGCmd.Index & 0x7F;        // which ep will be clear
            direction = ControlOTGCmd.Index >> 7;            // the direction of this ep
            if(endpointIndex > DEVICE_MAX_ENDPOINT_NUM)           // over the Max. ep count ?
            {
                result = ERROR_USB_DEVICE_INVALID_ENDPOINT_NUM;
                goto end;
            }

            fifoIndex = USB_DEVICE_GetEndpointMapReg(endpointIndex, direction);      // get the relatived FIFO number
            if(fifoIndex >= DEVICE_MAX_FIFO_NUM) // over the Max. fifo count ?
            {
                result = ERROR_USB_DEVICE_INVALID_FIFO_NUM;
                goto end;
            }
            
            // Check the FIFO had been enable ?
            if((USB_DEVICE_GetFifoConfigReg(fifoIndex) & USB_DEVICE_MSK_FIFO_ENABLE) == 0)
            {
                result = ERROR_USB_DEVICE_FIFO_NOT_ENABLED;
                goto end;
            }
            
            if(direction == INDEX_DIRECTION_IN)
            {
                USB_DEVICE_ResetToggleInEndpointReg(endpointIndex);
                USB_DEVICE_StallInEndpointReg(endpointIndex, MMP_FALSE);
            }
            else
            {
                USB_DEVICE_ResetToggleOutEndpointReg(endpointIndex);
                USB_DEVICE_StallOutEndpointReg(endpointIndex, MMP_FALSE);
            }
        }
        break;
                
    case FEATURE_SEL_DEVICE_REMOTE_WAKEUP:
        /**
         * Clear "Device_Remote_Wakeup", Turn off the"RMWKUP" bit in Main Control Register
         */
        USB_DEVICE_EnableRemoteWakeupReg(MMP_FALSE);
        break;

    case FEATURE_SEL_DEVICE_TEST_MODE:
        result = ERROR_USB_DEVICE_UNSUPPORT_FEATURE_SEL;
        goto end;
            
    default :
        result = ERROR_USB_DEVICE_INVALID_FEATURE_SEL;
        goto end;
    }
    
    eOTGCxFinishAction = ACT_DONE;
    
end:
    LOG_LEAVE "[USB_DEVICE_CxClearFeature] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxClearFeature() return error code 0x%08X \n", result LOG_END
    return result;
}



///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxSetFeature()
//      Description:
//          1. Process Cx Set feature command.
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetFeature(void)
{
    MMP_INT   result = 0;
    MMP_UINT8 i = 0;
    MMP_UINT8 endpointIndex = 0;
    MMP_UINT8 fifoIndex = 0;
    MMP_UINT8 u8Tmp[60] = {0};
    MMP_UINT8 *tp = MMP_NULL;
    MMP_BOOL direction = 0;

    LOG_ENTER "[USB_DEVICE_CxSetFeature]  Enter  \n" LOG_END
    
    // FeatureSelector
    switch(ControlOTGCmd.Value)        
    {
    case FEATURE_SEL_ENDPOINT_HALT:
        // Set "Endpoint_Halt", Turn on the "STALL" bit in Endpoint Control Function Register
        if(ControlOTGCmd.Index == 0x00)
            bOTGEP0HaltSt = MMP_TRUE;
        else
        {
            endpointIndex = ControlOTGCmd.Index & 0x7F;        // which ep will be clear
            direction = ControlOTGCmd.Index >> 7;            // the direction of this ep
            if(endpointIndex > DEVICE_MAX_ENDPOINT_NUM)           // over the Max. ep count ?
            {
                result = ERROR_USB_DEVICE_INVALID_ENDPOINT_NUM;
                goto end;
            }

            fifoIndex = USB_DEVICE_GetEndpointMapReg(endpointIndex, direction);      // get the relatived FIFO number
            if(fifoIndex >= DEVICE_MAX_FIFO_NUM) // over the Max. fifo count ?
            {
                result = ERROR_USB_DEVICE_INVALID_FIFO_NUM;
                goto end;
            }

            // Check the FIFO had been enable ?
            if((USB_DEVICE_GetFifoConfigReg(fifoIndex) & USB_DEVICE_MSK_FIFO_ENABLE) == 0)
            {
                result = ERROR_USB_DEVICE_FIFO_NOT_ENABLED;
                goto end;
            }
            
            if(direction == INDEX_DIRECTION_IN)
                USB_DEVICE_StallInEndpointReg(endpointIndex, MMP_TRUE);        // Clear Stall Bit
            else
                USB_DEVICE_StallOutEndpointReg(endpointIndex, MMP_TRUE);       // Set Stall Bit
        }
        break;
            
    case FEATURE_SEL_DEVICE_REMOTE_WAKEUP:
        // Set "Device_Remote_Wakeup", Turn on the"RMWKUP" bit in Mode Register
        USB_DEVICE_EnableRemoteWakeupReg(MMP_TRUE);
        eOTGCxFinishAction = ACT_DONE;
        break;

    case FEATURE_SEL_DEVICE_TEST_MODE:
        switch((ControlOTGCmd.Index >> 8)) // TestSelector
        {
            case TEST_MODE_TEST_J:   // Test_J
                USB_DEVICE_SelectPhyTestModeReg(PHY_TEST_MODE_J_STATE);
                break;
            
            case TEST_MODE_TEST_K:   // Test_K
                USB_DEVICE_SelectPhyTestModeReg(PHY_TEST_MODE_K_STATE);
                break;
                    
            case TEST_MODE_TEST_SE0_NAK:   // TEST_SE0_NAK
                USB_DEVICE_SelectPhyTestModeReg(PHY_TEST_MODE_SE0_NAK);
                break;
                    
            case TEST_MODE_TEST_PACKET:   // Test_Packet
                USB_DEVICE_SelectPhyTestModeReg(PHY_TEST_MODE_PACKET);
                USB_DEVICE_SetCxDoneReg();          // special case: follow the test sequence
                //////////////////////////////////////////////
                // Jay ask to modify, 91-6-5 (Begin)        //
                //////////////////////////////////////////////
                #if 0
                pp = u8Tmp;
                for (i=0; i<9; i++)         // JKJKJKJK x 9
                {
                    (*pp) = (0x00);
                    pp ++;
                }

                //(*pp) = (0xAA);
                //pp ++;
                //(*pp) = (0x00);
                //pp ++;      
                
                for (i=0; i<8; i++)         // 8*AA
                {
                    (*pp) = (0xAA);
                    pp ++;
                }
                
                for (i=0; i<8; i++)         // 8*EE
                {
                    (*pp) = (0xEE);
                    pp ++;
                }
                (*pp) = (0xFE);
                pp ++;  
                
                for (i=0; i<11; i++)        // 11*FF
                {
                    (*pp) = (0xFF);
                    pp ++;
                }
                
                (*pp) = (0x7F);
                pp ++;
                (*pp) = (0xBF);
                pp ++;
                (*pp) = (0xDF);
                pp ++;
                (*pp) = (0xEF);
                pp ++;
                (*pp) = (0xF7);
                pp ++;
                (*pp) = (0xFB);
                pp ++;
                (*pp) = (0xFD);
                pp ++;
                (*pp) = (0xFC);
                pp ++;
                (*pp) = (0x7E);
                pp ++;
                (*pp) = (0xBF);
                pp ++;
                (*pp) = (0xDF);
                pp ++;
                (*pp) = (0xFB);
                pp ++;
                (*pp) = (0xFD);
                pp ++;
                (*pp) = (0xFB);
                pp ++;
                (*pp) = (0xFD);
                pp ++;
                (*pp) = (0x7E);
                #else // #if 0
                // Irene 2010_1116
                tp = u8Tmp;
                for (i = 0; i < 9; i++)/*JKJKJKJK x 9*/
                  *tp++ = 0x00;
                     
                 for (i = 0; i < 8; i++) /* 8*AA */
                  *tp++ = 0xAA;
                     
                 for (i = 0; i < 8; i++) /* 8*EE */  
                  *tp++ = 0xEE;
                
                 
                
                 *tp++ = 0xFE;
                     
                 for (i = 0; i < 11; i++) /* 11*FF */
                  *tp++ = 0xFF;
                     
                 *tp++ = 0x7F;
                 *tp++ = 0xBF;
                 *tp++ = 0xDF;
                 *tp++ = 0xEF;
                 *tp++ = 0xF7;
                 *tp++ = 0xFB;
                 *tp++ = 0xFD;
                 *tp++ = 0xFC;
                 *tp++ = 0x7E;
                 *tp++ = 0xBF;
                 *tp++ = 0xDF;
                 *tp++ = 0xEF;
                 *tp++ = 0xF7;
                 *tp++ = 0xFB;
                 *tp++ = 0xFD;
                 *tp++ = 0x7E;
                #endif
                
                //USB_DEVICE_CxFWr( u8Tmp, 50);
                USB_DEVICE_CxFWr( u8Tmp, 53); // Irene 2010_1116
    
                //////////////////////////////////////////////
                // Jay ask to modify, 91-6-5 (End)          //
                //////////////////////////////////////////////

                // Turn on "r_test_packet_done" bit(flag) (Bit 5)
                USB_DEVICE_SetTestPacketDoneReg();
                break;
            
            case TEST_MODE_TEST_FORCE_ENABLE:   // Test_Force_Enable
                //FUSBPort[0x08] = 0x20;    //Start Test_Force_Enable
                break;
        
            default:
                result = ERROR_USB_DEVICE_INVALID_TEST_SEL;
                goto end;
        }
        break;

    case 3 :        //For OTG => b_hnp_enable
        OTG_OTGP_HNP_Enable();
        break;          

        eOTGCxFinishAction = ACT_DONE;
        break;  
        
    case 5 :        //For OTG => b_hnp_enable
        LOG_ERROR "[USB_DEVICE_CxSetFeature] >>> Please Connect to an alternate port on the A-device for HNP...\n\n" LOG_END
        break;      

    default :
        result = ERROR_USB_DEVICE_INVALID_FEATURE_SEL;
        goto end;
    }

    eOTGCxFinishAction = ACT_DONE;
    
end:
    LOG_LEAVE "[USB_DEVICE_CxSetFeature] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxSetFeature() return error code 0x%08X \n", result LOG_END
    return result;
}



///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxSetAddress()
//      Description:
//          1. Set USB bus addr to FOTG200 Device register.
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetAddress(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_CxSetAddress]  Enter  \n" LOG_END

    if(ControlOTGCmd.Value >= 0x0100)
    {
        result = ERROR_USB_DEVICE_INVALID_ADDRESS;
        goto end;
    }
    USB_DEVICE_SetDeviceAddressReg(ControlOTGCmd.Value);
    eOTGCxFinishAction = ACT_DONE;

end:
    LOG_LEAVE "[USB_DEVICE_CxSetAddress] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxSetAddress() return error code 0x%08X \n", result LOG_END
    return result;
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxGetDescriptor()
//      Description:
//          1. Point to the start location of the correct descriptor.
//          2. set the transfer length and return descriptor information back to host
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxGetDescriptor(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_CxGetDescriptor]  Enter  \n" LOG_END

    switch((MMP_UINT8)(ControlOTGCmd.Value >> 8))
    {
    case DT_DEVICE:
        LOG_CMD " ==> DT_DEVICE \n" LOG_END
        pu8OTGDescriptorEX = &u8OTGDeviceDescriptorEX[0];
        u16OTGTxRxCounter = u8OTGDeviceDescriptorEX[0];
        break;

    case DT_CONFIGURATION:
        LOG_CMD " ==> DT_CONFIGURATION \n" LOG_END
        // It includes Configuration, Interface and Endpoint Table
        switch((MMP_UINT8)ControlOTGCmd.Value)
        {
        case 0x00:      // configuration no: 0
            pu8OTGDescriptorEX = &u8ConfigOTGDescriptorEX[0];
            u16OTGTxRxCounter = u8ConfigOTGDescriptorEX[2] + (u8ConfigOTGDescriptorEX[3] << 8);
            break;
        default:
            result = ERROR_USB_DEVICE_INVALID_CONFIGURATION_NUM;
            goto end;
        }
        break;  
                
    case DT_STRING:
        LOG_CMD " ==> DT_STRING \n" LOG_END
#if !defined(CONFIG_HAVE_USBD)
        // DescriptorIndex = low_byte of wValue
        switch((MMP_UINT8)ControlOTGCmd.Value)
        {
        case 0x00:
            pu8OTGDescriptorEX = &u8OTGString00DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString00DescriptorEX[0];
            break;
        
        case 0x01:
            pu8OTGDescriptorEX = &u8OTGString10DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString10DescriptorEX[0];
            break;
        
        case 0x02:
            pu8OTGDescriptorEX = &u8OTGString20DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString20DescriptorEX[0];
            break;
        
        case 0x30:
            pu8OTGDescriptorEX = &u8OTGString30DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString30DescriptorEX[0];
            break;
            
        case 0x40:
            pu8OTGDescriptorEX = &u8OTGString40DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString40DescriptorEX[0];
            break;
            
        case 0x50:
            pu8OTGDescriptorEX = &u8OTGString50DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString50DescriptorEX[0];
            break;
            
        case 0x03:
            pu8OTGDescriptorEX = &u8OTGStringSerialNum[0];
            u16OTGTxRxCounter = u8OTGStringSerialNum[0];
            break;
            
        default:
            result = ERROR_USB_DEVICE_INVALID_STRING_NUM;
            goto end;
        }
#else
        result = it_usbd_cx_get_string_descriptor();
        if (result)
        {
            goto end;
        }
#endif
        break;  
                
    case DT_INTERFACE:
        // It cannot be accessed individually, it must follow "Configuraton"
        break;
        
    case DT_ENDPOINT:
        // It cannot be accessed individually, it must follow "Configuraton"
        break;

    case DT_DEVICE_QUALIFIER:
        LOG_CMD " ==> DT_DEVICE_QUALIFIER \n" LOG_END
        pu8OTGDescriptorEX = &u8OTGDeviceQualifierDescriptorEX[0];
        u16OTGTxRxCounter = u8OTGDeviceQualifierDescriptorEX[0];
        break;
    
    case DT_OTHER_SPEED_CONFIGURATION:
        LOG_CMD " ==> DT_OTHER_SPEED_CONFIGURATION \n" LOG_END
        // It includes Configuration, Interface and Endpoint Table
        pu8OTGDescriptorEX = &u8OtherSpeedConfigOTGDescriptorEX[0];
        u16OTGTxRxCounter = u8OtherSpeedConfigOTGDescriptorEX[2] + (u8OtherSpeedConfigOTGDescriptorEX[3] << 8);
        break;

    default:
        result = ERROR_USB_DEVICE_INVALID_DESCRIPTOR;
        goto end;
    }

    if(u16OTGTxRxCounter > ControlOTGCmd.Length)
        u16OTGTxRxCounter = ControlOTGCmd.Length;

    eOTGCxCommand = CMD_GET_DESCRIPTOR;
    USB_DEVICE_Ep0TxData();

end:
    LOG_LEAVE "[USB_DEVICE_CxGetDescriptor] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxGetDescriptor() return error code 0x%08X \n", result LOG_END
    return result;
}           

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxSetDescriptor()
//      Description:
//          1. Point to the start location of the correct descriptor.
//          2. Set the transfer length, and we will save data into sdram when Rx interrupt occure
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetDescriptor(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_CxSetDescriptor]  Enter  \n" LOG_END

    switch((MMP_UINT8)(ControlOTGCmd.Value >> 8))
    {
    case DT_DEVICE:
        pu8OTGDescriptorEX = &u8OTGDeviceDescriptorEX[0];
        u16OTGTxRxCounter = u8OTGDeviceDescriptorEX[0];
        break;

    case DT_CONFIGURATION:
        // It includes Configuration, Interface and Endpoint Table
        // DescriptorIndex = low_byte of wValue
        switch ((MMP_UINT8)ControlOTGCmd.Value)
        {
        case 0x00:      // configuration no: 0
            pu8OTGDescriptorEX = &u8ConfigOTGDescriptorEX[0];
            u16OTGTxRxCounter = u8ConfigOTGDescriptorEX[0];
            break;
            
        default:
            result = ERROR_USB_DEVICE_INVALID_CONFIGURATION_NUM;
            goto end;
        }
        break;  
                
    case DT_STRING:
#if !defined(CONFIG_HAVE_USBD)
        // DescriptorIndex = low_byte of wValue
        switch ((MMP_UINT8)ControlOTGCmd.Value)
        {
        case 0x00:
            pu8OTGDescriptorEX = &u8OTGString00DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString00DescriptorEX[0];
            break;
        
        case 0x01:
            pu8OTGDescriptorEX = &u8OTGString10DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString10DescriptorEX[0];
            break;
        
        case 0x02:
            pu8OTGDescriptorEX = &u8OTGString20DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString20DescriptorEX[0];
            break;
        
        case 0x30:
            pu8OTGDescriptorEX = &u8OTGString30DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString30DescriptorEX[0];
            break;
            
        case 0x40:
            pu8OTGDescriptorEX = &u8OTGString40DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString40DescriptorEX[0];
            break;
            
        case 0x50:
            pu8OTGDescriptorEX = &u8OTGString50DescriptorEX[0];
            u16OTGTxRxCounter = u8OTGString50DescriptorEX[0];
            break;
            
        default:
            result = ERROR_USB_DEVICE_INVALID_STRING_NUM;
            goto end;
        }
#else
        result = it_usbd_cx_set_string_descriptor();
#endif
        break;
        
    default:
        result = ERROR_USB_DEVICE_INVALID_DESCRIPTOR;
        goto end;
    }

    if(u16OTGTxRxCounter > ControlOTGCmd.Length)
        u16OTGTxRxCounter = ControlOTGCmd.Length;

    eOTGCxCommand = CMD_SET_DESCRIPTOR;
    
end:
    LOG_LEAVE "[USB_DEVICE_CxSetDescriptor] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxSetDescriptor() return error code 0x%08X \n", result LOG_END
    return result;
}           


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxGetConfiguration()
//      Description:
//          1. Send 1 bytes configuration value to host.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_CxGetConfiguration(void)
{
    MMP_UINT8 u8Tmp[2] = {0};

    LOG_ENTER "[USB_DEVICE_CxGetConfiguration]  Enter  \n" LOG_END

    u8Tmp[0] = u8OTGConfigValue;
    USB_DEVICE_CxFWr(u8Tmp, 1);

    eOTGCxFinishAction = ACT_DONE;
    
    LOG_LEAVE "[USB_DEVICE_CxGetConfiguration] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxSetConfiguration()
//      Description:
//          1. Get 1 bytes configuration value from host.
//          2-1. if(value == 0) then device return to address state
//          2-2. if(value match descriptor table)
//                  then config success & Clear all EP toggle bit
//          2-3  else stall this command
//      input: none
//      output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetConfiguration(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_CxSetConfiguration]  Enter  \n" LOG_END

    if((MMP_UINT8)ControlOTGCmd.Value == 0)
    {
        u8OTGConfigValue = 0;
        USB_DEVICE_EnableAfterSetConfigurationReg(MMP_FALSE);
    }
    else
    {
        if(USB_DEVICE_IsHighSpeedModeReg())
        {
            if((MMP_UINT8)ControlOTGCmd.Value > HS_CONFIGURATION_NUMBER)
            {
                result = ERROR_USB_DEVICE_INVALID_CONFIGURATION_NUM;
                goto end;
            }
            u8OTGConfigValue = (MMP_UINT8)ControlOTGCmd.Value;
            USB_DEVICE_ClearEpXFifoX();
            OTG_DEV_vOTGFIFO_EPxCfg_HS();
            USB_DEVICE_SetSofMaskTimerReg(0x44C);
        }
        else
        {
            if ((MMP_UINT8)ControlOTGCmd.Value > FS_CONFIGURATION_NUMBER)
            {
                result = ERROR_USB_DEVICE_INVALID_CONFIGURATION_NUM;
                goto end;
            }
            u8OTGConfigValue = (MMP_UINT8)ControlOTGCmd.Value;
            OTG_DEV_vOTGFIFO_EPxCfg_FS();
            USB_DEVICE_SetSofMaskTimerReg(0x2710);
        }
        USB_DEVICE_EnableAfterSetConfigurationReg(MMP_TRUE);
        USB_DEVICE_ClearEpX();
    }

    if((MMP_UINT8)ControlOTGCmd.Value == 1)            // Card Reader App.
    {
#if !defined(CONFIG_HAVE_USBD)
        USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_FIFO0_OUT);
        USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_G1_FIFO2_IN);
        USB_DEVICE_EnableInterruptGroup1Reg(DEV_INT_FIFO3_OUT);
        USB_DEVICE_DisableInterruptGroup1Reg(DEV_INT_G1_FIFO0_IN);
#else
        it_usbd_init_fifo_interrupt_mask();
#endif
    }

    eOTGCxFinishAction = ACT_DONE;

end:
    LOG_LEAVE "[USB_DEVICE_CxSetConfiguration] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxSetConfiguration() return error code 0x%08X \n", result LOG_END
    return result;
}


#if !defined(CONFIG_HAVE_USBD)
///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxGetInterface()
//      Description:
//          Getting interface
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxGetInterface(void)
{
    MMP_INT result = 0;
    MMP_UINT8 u8Tmp[2] = {0};

    LOG_ENTER "[USB_DEVICE_CxGetInterface]  Enter  \n" LOG_END

    if(!USB_DEVICE_IsAfterSetConfigurationReg())
    {
         result = ERROR_USB_DEVICE_NOT_IN_CONFIGURATION_STATE;
         goto end;
    }

    // If there exists many interfaces, Interface0,1,2,...N,
    // You must check & select the specific one
    switch(u8OTGConfigValue)
    {
    // Configuration 1
    #if (HS_CONFIGURATION_NUMBER >= 1)
    case 1:
        if(ControlOTGCmd.Index > HS_C1_INTERFACE_NUMBER)
        {
             result = ERROR_USB_DEVICE_INVALID_INTERFACE_NUM;
             goto end;
        }
        break;
    #endif
    
    // Configuration 2
    #if (HS_CONFIGURATION_NUMBER >= 2)
    case 2:
        if(ControlOTGCmd.Index > HS_C2_INTERFACE_NUMBER)
        {
             result = ERROR_USB_DEVICE_INVALID_INTERFACE_NUM;
             goto end;
        }
        break;
    #endif
    
    default:
        return MMP_FALSE;
    }

    u8Tmp[0] = u8OTGInterfaceAlternateSetting;
    USB_DEVICE_CxFWr(u8Tmp, 1);

    eOTGCxFinishAction = ACT_DONE;

end:
    LOG_LEAVE "[USB_DEVICE_CxGetInterface] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxGetInterface() return error code 0x%08X \n", result LOG_END
    return result;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxSetInterface()
//      Description:
//          1-1. If (the device stays in Configured state)
//                  &(command match the alternate setting)
//                      then change the interface
//          1-2. else stall it
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSetInterface(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_CxSetInterface]  Enter  \n" LOG_END

    if(!USB_DEVICE_IsAfterSetConfigurationReg())
    {
         result = ERROR_USB_DEVICE_NOT_IN_CONFIGURATION_STATE;
         goto end;
    }

    // If there exists many interfaces, Interface0,1,2,...N,
    // You must check & select the specific one
    switch(ControlOTGCmd.Index)
    {
    case 0: // Interface0
        if((MMP_UINT8)ControlOTGCmd.Value == u8ConfigOTGDescriptorEX[12])
        {
            u8OTGInterfaceValue = (MMP_UINT8)ControlOTGCmd.Index;
            u8OTGInterfaceAlternateSetting = (MMP_UINT8)ControlOTGCmd.Value;
            USB_DEVICE_ClearEpXFifoX();
            if(USB_DEVICE_IsHighSpeedModeReg())
                OTG_DEV_vOTGFIFO_EPxCfg_HS();
            else
                OTG_DEV_vOTGFIFO_EPxCfg_FS();
            USB_DEVICE_ClearEpX();
        }
        
    case 1: // Interface1
    case 2: // Interface2
    default:
        goto end;
    }
    eOTGCxFinishAction = ACT_DONE;

end:
    LOG_LEAVE "[USB_DEVICE_CxSetInterface] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxSetInterface() return error code 0x%08X \n", result LOG_END
    return result;
}



#if !defined(CONFIG_HAVE_USBD)
///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxSynchFrame()
//      Description:
//          1. If the EP is a Iso EP, then return the 2 bytes Frame number.
//               else stall this command
//      input: none
//      output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxSynchFrame(void)
{
    MMP_INT result = 0;
    MMP_INT8 TransferType = 0;
    MMP_UINT16 u16Tmp = 0;

    LOG_ENTER "[USB_DEVICE_CxSynchFrame]  Enter  \n" LOG_END

    TransferType = -1;
    // Does the Endpoint support Isochronous transfer type? 
    switch(ControlOTGCmd.Index)
    {
    case 1:     // EP1
        TransferType = u8ConfigOTGDescriptorEX[22] & 0x03;
        break;
    default:
        break;
    }

    if(TransferType == 1)  // Isochronous
    {
        u16Tmp = (MMP_UINT16)USB_DEVICE_GetFrameNumReg();
        USB_DEVICE_CxFWr((MMP_UINT8*)&u16Tmp, 2);
        eOTGCxFinishAction = ACT_DONE;
    }
    else
    {
        result = ERROR_USB_DEVICE_NOT_ISOCHRONOUS_TX_TYPE;
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_CxSynchFrame] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxSynchFrame() return error code 0x%08X \n", result LOG_END
    return result;
}

extern OTG_DEVICE_GET_MAX_LUN_NUM   OTG_DEVICE_GetMaxLunNum;

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_ClassCommand()
//      Description:
//          1. Process class command of Cx 8 bytes command.
//      input: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_ClassCommand(void)
{
    MMP_INT result = 0;
    MMP_UINT8 u8Tmp[2] = {0};

    LOG_ENTER "[USB_DEVICE_ClassCommand]  Enter  \n" LOG_END

    // by Standard Request codes
    switch(ControlOTGCmd.Request) 
    {
    case 0xFE:  // Get max lun number.
        LOG_CMD " Class Command : Get Max Lun Num \n" LOG_END
        if(ControlOTGCmd.Index > HS_C1_INTERFACE_NUMBER)
        {
             result = ERROR_USB_DEVICE_INVALID_INTERFACE_NUM;
             goto end;
        }
        if(ControlOTGCmd.Value)
        {
             result = ERROR_USB_DEVICE_INVALID_VALUE;
             goto end;
        }
        if(ControlOTGCmd.Length != 1)
        {
             result = ERROR_USB_DEVICE_INVALID_LENGTH;
             goto end;
        }

        result = OTG_DEVICE_GetMaxLunNum(&u8Tmp[0]);
        //u8Tmp[0] -= 1;
        if(result)
            goto end;
        USB_DEVICE_CxFWr(u8Tmp, 1);

        eOTGCxFinishAction = ACT_DONE;
        break;

    case 0xFF:  // Bulk-Only Mass Storage Reset
        LOG_CMD " Class Command : Bulk-Only Mass Storage Reset \n" LOG_END
        if(ControlOTGCmd.Index > HS_C1_INTERFACE_NUMBER)
        {
             result = ERROR_USB_DEVICE_INVALID_INTERFACE_NUM;
             goto end;
        }
        if(ControlOTGCmd.Value)
        {
             result = ERROR_USB_DEVICE_INVALID_VALUE;
             goto end;
        }
        if(ControlOTGCmd.Length)
        {
             result = ERROR_USB_DEVICE_INVALID_LENGTH;
             goto end;
        }

        result = USB_DEVICE_MSC_ApInitialize();
        if(result)
            goto end;

        eOTGCxFinishAction = ACT_DONE;
        break;

    default:
        result = ERROR_USB_DEVICE_INVALID_REQUEST_CODE; 
        break;
    }
    
end:
    LOG_LEAVE "[USB_DEVICE_ClassCommand] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_ClassCommand() return error code 0x%08X, request code = %d \n", result, ControlOTGCmd.Request LOG_END
    return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0TxData()
//      Description:
//          1. Send data(max or short packet) to host.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Ep0TxData(void)
{
    MMP_UINT8 u8temp = 0;

    LOG_ENTER "[USB_DEVICE_Ep0TxData]  Enter  \n" LOG_END

    if(u16OTGTxRxCounter < EP0MAXPACKETSIZE)
        u8temp = (MMP_UINT8)u16OTGTxRxCounter;
    else
        u8temp = EP0MAXPACKETSIZE;

    u16OTGTxRxCounter -= (MMP_UINT16)u8temp;

    // Transmit u8Temp bytes data
    USB_DEVICE_CxFWr(pu8OTGDescriptorEX, u8temp);
    pu8OTGDescriptorEX = pu8OTGDescriptorEX + u8temp;
    
    eOTGCxFinishAction = ACT_DONE;
    // end of the data stage
    if(u16OTGTxRxCounter == 0)
    {
        eOTGCxCommand = CMD_VOID;
    }

    LOG_LEAVE "[USB_DEVICE_Ep0TxData] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Ep0RxData()
//      Description:
//          1. Receive data(max or short packet) from host.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_Ep0RxData(void)
{
    MMP_INT   result = 0;
    MMP_UINT8 u8temp = 0;

    LOG_ENTER "[USB_DEVICE_Ep0RxData]  Enter  \n" LOG_END

    if(u16OTGTxRxCounter < EP0MAXPACKETSIZE)
         u8temp = (MMP_UINT8)u16OTGTxRxCounter;
    else
         u8temp = EP0MAXPACKETSIZE;

    u16OTGTxRxCounter -= (MMP_UINT16)u8temp;
    result = USB_DEVICE_CxFRd(pu8OTGDescriptorEX , u8temp); 
    if(result)
        goto end;
    pu8OTGDescriptorEX = pu8OTGDescriptorEX + u8temp;

    // end of the data stage
    if (u16OTGTxRxCounter == 0)
    {
        eOTGCxCommand = CMD_VOID;
        eOTGCxFinishAction = ACT_DONE;
    }

end:
    LOG_LEAVE "[USB_DEVICE_Ep0RxData] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_Ep0RxData() return error code 0x%08X \n", result LOG_END
    return result;
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_ClearEpX()
//      Description:
//          1. Clear all endpoint Toggle Bit
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_ClearEpX(void)
{
    MMP_UINT8 u8ep;

    LOG_ENTER "[USB_DEVICE_ClearEpX]  Enter  \n" LOG_END

    // Clear All EPx Toggle Bit
    for(u8ep = 1; u8ep <= DEVICE_MAX_ENDPOINT_NUM; u8ep ++)
    {
        USB_DEVICE_ResetToggleInEndpointReg(u8ep);
    }
    for(u8ep = 1; u8ep <= DEVICE_MAX_ENDPOINT_NUM; u8ep ++)
    {
        USB_DEVICE_ResetToggleOutEndpointReg(u8ep);
    }

    LOG_LEAVE "[USB_DEVICE_ClearEpX] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_ClearEpXFifoX()
//      Description:
//          1. Clear all endpoint & FIFO register setting
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_ClearEpXFifoX(void)
{
    MMP_UINT8 u8ep;

    LOG_ENTER "[USB_DEVICE_ClearEpXFifoX]  Enter  \n" LOG_END

    for(u8ep = 1; u8ep <= DEVICE_MAX_ENDPOINT_NUM; u8ep ++)
    {
        USB_DEVICE_SetMaxPacketSizeReg(u8ep, DIRECTION_IN, 0);  
        USB_DEVICE_SetMaxPacketSizeReg(u8ep, DIRECTION_OUT, 0); 
    }
    USB_DEVICE_ClearAllEndpointMapReg();
    USB_DEVICE_ClearAllFifoMapReg();
    USB_DEVICE_ClearAllFifoConfigReg();

    LOG_LEAVE "[USB_DEVICE_ClearEpXFifoX] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_IsoSequentialError()
//      Description:
//          1. FOTG200 Device Detects High bandwidth isochronous Data PID sequential error.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_IsoSequentialError(void)
{
    MMP_UINT32 u32Tmp = USB_DEVICE_GetIsoSequentialErrorStatusReg();

    LOG_ENTER "[USB_DEVICE_IsoSequentialError]  Enter  \n" LOG_END

    USB_DEVICE_ClearIsoSequentialErrorStatusReg(u32Tmp);

    LOG_LEAVE "[USB_DEVICE_IsoSequentialError] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_IsoSequentialAbort()
//      Description:
//          1. FOTG200 Device Detects High bandwidth isochronous Data PID sequential abort.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_IsoSequentialAbort(void)
{
    MMP_UINT32 u32Tmp = USB_DEVICE_GetIsoAbortErrorStatusReg();     

    LOG_ENTER "[USB_DEVICE_IsoSequentialAbort]  Enter  \n" LOG_END

    USB_DEVICE_ClearIsoAbortErrorStatusReg(u32Tmp);

    LOG_LEAVE "[USB_DEVICE_IsoSequentialAbort] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Tx0Byte()
//      Description:
//          1. Send 0 byte data to host.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Tx0Byte(void)
{
    MMP_UINT32 u32Tmp = USB_DEVICE_GetTxZeroPacketStatusReg();
    MMP_UINT32 i;

    LOG_ENTER "[USB_DEVICE_Tx0Byte]  Enter  \n" LOG_END

    USB_DEVICE_ClearTxZeroPacketStatusReg(u32Tmp);

    for(i = 1; i < 8; i ++)
    {
        if(u32Tmp & (BIT0 << i))
        {
            LOG_DEBUG "[USB_DEVICE_Tx0Byte] Error Code = 0x%4x EP%x IN data 0 byte to host!!\n",u32Tmp ,i LOG_END
         }
    }

    LOG_LEAVE "[USB_DEVICE_Tx0Byte] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_Rx0Byte()
//      Description:
//          1. Receive 0 byte data from host.
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_Rx0Byte(void)
{
    MMP_UINT32 u32Tmp = USB_DEVICE_GetRxZeroPacketStatusReg();
    MMP_UINT32 i;

    LOG_ENTER "[USB_DEVICE_Rx0Byte]  Enter  \n" LOG_END

    USB_DEVICE_ClearRxZeroPacketStatusReg(u32Tmp);

    for(i = 1; i < 8; i ++)
    {
        if(u32Tmp & (BIT0 << i))
        {
            LOG_DEBUG "[USB_DEVICE_Rx0Byte] Error Code = 0x%4x EP%x OUT data 0 byte to Device!!\n",u32Tmp ,i LOG_END
        }
    }
    
    LOG_LEAVE "[USB_DEVICE_Rx0Byte] Leave \n" LOG_END
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_DxFWr()
//      Description: Write the buffer content to USB220 Data FIFO
//      input: Buffer pointer, byte number to write, Change Endian type or NOT
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_DxFWr(MMP_UINT8 FIFONum, MMP_UINT8* pu8Buffer, MMP_UINT16 u16Num)
{
    MMP_INT    result = 0;
    MMP_UINT32 wTemp = 0;
    MMP_UINT8* dmaAddr = MMP_NULL;
    MMP_UINT32 FIFO_Sel = 0;
    MMP_UINT32 timeout = 10000;
    SYS_CLOCK_T lastTime;
    
    LOG_ENTER "[USB_DEVICE_DxFWr]  Enter  FIFONum = %d pu8Buffer = 0x%4x u16Num = %d \n",FIFONum ,pu8Buffer ,u16Num  LOG_END

    if(u16Num==0)
        return;

    if(FIFONum<4)
    {
        FIFO_Sel = 1<<FIFONum;
    }
    else
    {
        result = ERROR_USB_DEVICE_INVALID_FIFO_NUM;
        goto end;
    }
    
    USB_DEVICE_SetDmaLengthDirReg(u16Num, DIRECTION_IN);
    USB_DEVICE_SetDmaFifoNumReg(FIFO_Sel);

    #if defined(__FREERTOS__)
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)pu8Buffer);
    #else
    result = USB_DEVICE_AllocateDmaAddr(&dmaAddr, u16Num);
    if(result)
        goto end;
    HOST_WriteBlockMemory((MMP_UINT32)dmaAddr, (MMP_UINT32)pu8Buffer, u16Num);
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)dmaAddr);
    #endif

    USB_DEVICE_DmaStartReg(MMP_TRUE);
    while(1)
    {
        wTemp = USB_DEVICE_GetInterruptGroup2StatusReg();
        if(wTemp & DEV_INT_G2_DMA_ERROR)
        {
            USB_DEVICE_ResetFifoReg(FIFONum);
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_ERROR);
            LOG_ERROR "[USB_DEVICE_DxFWr] dx OUT DMA error..!!\n" LOG_END
            goto end;
        }
        
        if(wTemp & DEV_INT_G2_DMA_COMPLETION)
        {
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            break;
        }

        if((wTemp & (DEV_INT_G2_RESUME|DEV_INT_G2_SUSPEND|DEV_INT_G2_BUS_RESET)))
        {
            USB_DEVICE_DmaResetReg();
            USB_DEVICE_ResetFifoReg(FIFONum);
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            LOG_ERROR "[USB_DEVICE_DxFWr]  dx OUT DMA stop because USB Resume/Suspend/Reset..!!\n" LOG_END
            goto end;
        }
        if(timeout)
        {
            timeout--;
            if(!timeout)
            {
                lastTime = SYS_GetClock();
                MMP_Sleep(0);
            }
        }
        else
        {
            if(SYS_GetDuration(lastTime) > 50)
            {
                LOG_ERROR " USB_DEVICE_DxFWr() timeout! u16Num 0x%X\n", u16Num LOG_END
                USB_DEVICE_DmaResetReg();
                USB_DEVICE_ResetFifoReg(FIFONum);
                USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
                break;
            }
        }
    }

end:
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);
    #if !defined(__FREERTOS__)
    USB_DEVICE_ReleaseDmaAddr();
    #endif

    LOG_LEAVE "[USB_DEVICE_DxFWr] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_DxFWr() has error code 0x%08X \n", result LOG_END
    return;
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_DxFRd()
//      Description: Fill the buffer from USB220 Data FIFO
//      input: Buffer pointer, byte number to write, Change Endian type or NOT
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_DxFRd(MMP_UINT8 FIFONum, MMP_UINT8 * pu8Buffer, MMP_UINT16 u16Num)
{
    MMP_INT    result = 0;
    MMP_UINT32 wTemp = 0;
    MMP_UINT32 FIFO_Sel = 0;
    MMP_UINT8* dmaAddr = MMP_NULL;
    MMP_UINT32 timeout = 10000;
    SYS_CLOCK_T lastTime;

    LOG_ENTER "[USB_DEVICE_DxFRd]  Enter FIFONum = %d pu8Buffer = 0x%4x u16Num = %d \n",FIFONum ,pu8Buffer ,u16Num  LOG_END
    
    if(u16Num==0)
        return;
        
    if(FIFONum<4)
        FIFO_Sel = 1<<FIFONum;
    else
    {
        result = ERROR_USB_DEVICE_INVALID_FIFO_NUM;
        goto end;
    }

    USB_DEVICE_SetDmaLengthDirReg(u16Num, DIRECTION_OUT);
    USB_DEVICE_SetDmaFifoNumReg(FIFO_Sel);

    #if defined(__FREERTOS__)
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)pu8Buffer);
    #else
    result = USB_DEVICE_AllocateDmaAddr(&dmaAddr, u16Num);
    if(result)
        goto end;
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)dmaAddr);
    #endif

    USB_DEVICE_DmaStartReg(MMP_TRUE);
    while(1)
    {
        wTemp = USB_DEVICE_GetInterruptGroup2StatusReg();
        if(wTemp & DEV_INT_G2_DMA_ERROR)
        {
            USB_DEVICE_ResetFifoReg(FIFONum);
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_ERROR);
            LOG_ERROR "[USB_DEVICE_DxFRd]FIFO%d OUT transfer DMA error..!!\n",FIFONum LOG_END
            goto end;
        }
        
        if(wTemp & DEV_INT_G2_DMA_COMPLETION)
        {
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            break;
        }
        
        if((wTemp & (DEV_INT_G2_RESUME|DEV_INT_G2_SUSPEND|DEV_INT_G2_BUS_RESET)))
        {
            USB_DEVICE_DmaResetReg();
            USB_DEVICE_ResetFifoReg(FIFONum);
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            LOG_ERROR "[USB_DEVICE_DxFRd]  Dx OUT DMA stop because USB Resume/Suspend/Reset..!!\n" LOG_END
            goto end;
        }
        if(timeout)
        {
            timeout--;
            if(!timeout)
            {
                lastTime = SYS_GetClock();
                MMP_Sleep(0);
            }
        }
        else
        {
            if(SYS_GetDuration(lastTime) > 50)
            {
                LOG_ERROR " USB_DEVICE_DxFRd() timeout!\n" LOG_END
                USB_DEVICE_DmaResetReg();
                USB_DEVICE_ResetFifoReg(FIFONum);
                USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
                break;
            }
        }
    }

    #if defined(__FREERTOS__)
#if defined(WR_USB_AHB_WRAP)
    {
        MMP_UINT16 tmp;
        HOST_WriteRegisterMask(0x914, (0x1<<3), (0x1<<3)); // flush wrap
        HOST_WriteRegister(0x912, 0x3); // wrap fire and end
        do{
           HOST_ReadRegister(0x912, &tmp);
           LOG_DATA "@" LOG_END
        } while(tmp & 0x3);
        HOST_WriteRegisterMask(0x914, (0x0<<3), (0x1<<3)); // disable flush
        LOG_DATA "\n\n" LOG_END
    }
#endif
    ithInvalidateDCacheRange(pu8Buffer, u16Num);
    #else
    HOST_ReadBlockMemory((MMP_UINT32)pu8Buffer, (MMP_UINT32)dmaAddr, u16Num);
    #endif

end:
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);
    #if !defined(__FREERTOS__)
    USB_DEVICE_ReleaseDmaAddr();
    #endif

    LOG_LEAVE "[USB_DEVICE_DxFRd] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_DxFRd() has error code 0x%08X \n", result LOG_END

    return;
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxFWr()
//      Description: Write the buffer content to USB220 Cx FIFO
//      input: Buffer pointer, byte number to write, Change Endian type or NOT
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_CxFWr( MMP_UINT8 *pu8Buffer, MMP_UINT16 u16Num)
{
    MMP_INT    result = 0;
    MMP_UINT32 wTemp = 0;
    MMP_UINT8* dmaAddr = 0;
    MMP_UINT32 timeout = 10000;
    SYS_CLOCK_T lastTime;

    LOG_ENTER "[USB_DEVICE_CxFWr]  Enter pu8Buffer = 0x%4x u16Num = %d \n",pu8Buffer ,u16Num  LOG_END

    if(u16Num==0)
        return;

    USB_DEVICE_SetDmaLengthDirReg(u16Num, DIRECTION_IN);
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_CX);

    #if defined(__FREERTOS__)
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)pu8Buffer);
    #else
    result = USB_DEVICE_AllocateDmaAddr(&dmaAddr, u16Num);
    if(result)
        goto end;
    HOST_WriteBlockMemory((MMP_UINT32)dmaAddr, (MMP_UINT32)pu8Buffer, u16Num);
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)dmaAddr);
    #endif

    USB_DEVICE_DmaStartReg(MMP_TRUE);
    while(1)
    {
        wTemp = USB_DEVICE_GetInterruptGroup2StatusReg();
        if(wTemp & DEV_INT_G2_DMA_ERROR)
        {
            USB_DEVICE_ClearCxFifoReg();
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_ERROR);
            LOG_ERROR "[USB_DEVICE_CxFWr] Cx OUT DMA error..!!\n" LOG_END
            goto end;
        }
        
        if(wTemp & DEV_INT_G2_DMA_COMPLETION)
        {
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            break;
        }
        
        if((wTemp & (DEV_INT_G2_RESUME|DEV_INT_G2_SUSPEND|DEV_INT_G2_BUS_RESET)))
        {
            USB_DEVICE_DmaResetReg();
            USB_DEVICE_ClearCxFifoReg();
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            LOG_ERROR "[USB_DEVICE_CxFWr]  Cx OUT DMA stop because USB Resume/Suspend/Reset..!!\n" LOG_END
            goto end;
        }
        if(timeout)
        {
            timeout--;
            if(!timeout)
            {
                lastTime = SYS_GetClock();
                MMP_Sleep(0);
            }
        }
        else
        {
            if(SYS_GetDuration(lastTime) > 50)
            {
                LOG_ERROR " USB_DEVICE_CxFWr() timeout!\n" LOG_END
                USB_DEVICE_DmaResetReg();
                USB_DEVICE_ClearCxFifoReg();
                USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
                break;
            }
        }
    }

end:
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);     
    #if !defined(__FREERTOS__)
    USB_DEVICE_ReleaseDmaAddr();
    #endif

    LOG_LEAVE "[USB_DEVICE_CxFWr] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxFWr() has error code 0x%08X \n", result LOG_END
    return;
}


///////////////////////////////////////////////////////////////////////////////
//      USB_DEVICE_CxFRd()
//      Description: Fill the buffer from USB220 Cx FIFO
//      input: Buffer pointer, byte number to write, Change Endian type or NOT
//      output: none
///////////////////////////////////////////////////////////////////////////////
MMP_INT USB_DEVICE_CxFRd(MMP_UINT8 *pu8Buffer, MMP_UINT16 u16Num)
{
    MMP_INT    result = 0;
    MMP_UINT32 wTemp = 0;
    MMP_UINT8* dmaAddr = MMP_NULL;
    MMP_UINT32 timeout = 10000;
    SYS_CLOCK_T lastTime;

    LOG_ENTER "[USB_DEVICE_CxFRd]  Enter  pu8Buffer = 0x%4x u16Num = %d \n",pu8Buffer ,u16Num  LOG_END

    if(u16Num==0)
        return result;

    USB_DEVICE_SetDmaLengthDirReg(u16Num,DIRECTION_OUT);
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_CX);

    #if defined(__FREERTOS__)
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)pu8Buffer);
    #else
    result = USB_DEVICE_AllocateDmaAddr(&dmaAddr, u16Num);
    if(result)
        goto end;
    USB_DEVICE_SetDmaAddrReg((MMP_UINT32)dmaAddr);
    #endif

    USB_DEVICE_DmaStartReg(MMP_TRUE);
    
    while(1)
    {
        wTemp = USB_DEVICE_GetInterruptGroup2StatusReg();
        if(wTemp & DEV_INT_G2_DMA_ERROR)
        {
            USB_DEVICE_ClearCxFifoReg();
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_ERROR);
            LOG_ERROR "[USB_DEVICE_CxFRd] Cx OUT DMA error..!!\n" LOG_END
            goto end;
        }
        
        if(wTemp & DEV_INT_G2_DMA_COMPLETION)
        {
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            break;
        }
        
        if((wTemp & (DEV_INT_G2_RESUME|DEV_INT_G2_SUSPEND|DEV_INT_G2_BUS_RESET)))
        {
            USB_DEVICE_DmaResetReg();
            USB_DEVICE_ClearCxFifoReg();
            USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
            LOG_ERROR "[USB_DEVICE_CxFRd]  Cx OUT DMA stop because USB Resume/Suspend/Reset..!!\n" LOG_END
            goto end;
        }
        if(timeout)
        {
            timeout--;
            if(!timeout)
            {
                lastTime = SYS_GetClock();
                MMP_Sleep(0);
            }
        }
        else
        {
            if(SYS_GetDuration(lastTime) > 50)
            {
                LOG_ERROR " USB_DEVICE_CxFRd() timeout!\n" LOG_END
                USB_DEVICE_DmaResetReg();
                USB_DEVICE_ClearCxFifoReg();
                USB_DEVICE_ClearInterruptGroup2StatusReg(DEV_INT_G2_DMA_COMPLETION);
                break;
            }
        }
    }

    #if !defined(__FREERTOS__)
    HOST_ReadBlockMemory((MMP_UINT32)pu8Buffer, (MMP_UINT32)dmaAddr, u16Num);
    #endif

end:
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);
    #if !defined(__FREERTOS__)
    USB_DEVICE_ReleaseDmaAddr();
    #endif

    LOG_LEAVE "[USB_DEVICE_CxFRd] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_CxFRd() has error code 0x%08X \n", result LOG_END
    return result;
}


///////////////////////////////////////////////////////////////////////////////
//      vOTG_Interrupt_Initial() 
//      Description: Initial interrupt transfer test variable
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_InterruptInitial(void)
{
    MMP_UINT8 u8i = 0;
    MMP_UINT16 u16i = 0;
    static MMP_BOOL u8IntTransInit = MMP_FALSE;
    
    LOG_ENTER "[USB_DEVICE_InterruptInitial]  Enter \n" LOG_END

    if(u8IntTransInit == MMP_FALSE)
    {
        u8InterruptOTGArray =  (MMP_UINT8 *)SYS_Malloc(2048);
        for(u16i=0; u16i<2048; u16i++)
        {
            u8InterruptOTGArray[u16i] = u8i++;
        }
        u8IntTransInit = MMP_TRUE;
    }
    LOG_LEAVE "[USB_DEVICE_InterruptInitial] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      vOTG_Interrupt_In() 
//      Description: FIFO4 interrupt service process
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_InterruptIn(void)
{
    MMP_UINT32 u32i;
    MMP_UINT32 u32PacketSize ;
    MMP_UINT8 *u8Array;

    LOG_ENTER "[USB_DEVICE_InterruptIn]  Enter \n" LOG_END
    
    if(bOTGHighSpeed)
        u32PacketSize = HS_C1_I0_A0_EP1_MAX_PACKET ;
    else
        u32PacketSize = FS_C1_I0_A0_EP1_MAX_PACKET ;
    
    u8Array = SYS_Malloc(u32OTGInterrupt_TX_COUNT);

    for(u32i = 0; u32i < u32OTGInterrupt_TX_COUNT; u32i ++)
    {
        u8Array[u32i] = (u8OTGInterruptCount); 
        u8OTGInterruptCount ++;
    }

    USB_DEVICE_DxFWr(FIFO2, u8Array, (MMP_UINT16)u32OTGInterrupt_TX_COUNT);
    USB_DEVICE_SetFifoDoneReg(FIFO2);

    u32OTGInterrupt_TX_COUNT++;
    if(u32OTGInterrupt_TX_COUNT > u32PacketSize)
        u32OTGInterrupt_TX_COUNT = 1;

    SYS_Free(u8Array);

    LOG_LEAVE "[USB_DEVICE_InterruptIn] Leave \n" LOG_END
}

///////////////////////////////////////////////////////////////////////////////
//      vOTG_Interrupt_Out() 
//      Description: FIFO5 interrupt service process
//      input: none
//      output: none
///////////////////////////////////////////////////////////////////////////////
void USB_DEVICE_InterruptOut(void)
{
    MMP_UINT32 u32IntOutFifoUse = FIFO3;
    MMP_UINT16 u16Interrupt_RX_Count = USB_DEVICE_GetFifoOutByteCountReg((MMP_UINT8)u32IntOutFifoUse);
    MMP_UINT32 u32PacketSize ;
    MMP_UINT8 *u8Array;

    LOG_ENTER "[USB_DEVICE_InterruptOut]  Enter \n" LOG_END

    if(bOTGHighSpeed)
        u32PacketSize = HS_C1_I0_A0_EP2_MAX_PACKET ;
    else
        u32PacketSize = FS_C1_I0_A0_EP2_MAX_PACKET ;

    u8Array = SYS_Malloc(u32OTGInterrupt_RX_COUNT);

    if(u16Interrupt_RX_Count != u32OTGInterrupt_RX_COUNT)
    {        
        LOG_ERROR "[USB_DEVICE_InterruptOut] Interrupt_Out Byte Count Error = %x...(Correct = %x) \n",u16Interrupt_RX_Count, u32OTGInterrupt_RX_COUNT LOG_END
    }
    
    // Read Interrupt Data from FIFO..
    USB_DEVICE_DxFRd((MMP_UINT8)u32IntOutFifoUse, u8Array, u16Interrupt_RX_Count );
    //u8Test = OTG_Reg_Rd(USBBASE, 0x1c) & 0xFF;
    
    if(memcmp(u8InterruptOTGArray+u8OTGInterruptOutCount, u8Array, u32OTGInterrupt_RX_COUNT) != 0)
    {
        // for Bulk Bir-direction test
        USB_DEVICE_StallInEndpointReg(EP4, MMP_TRUE);
        LOG_ERROR "[USB_DEVICE_InterruptOut]  Interrupt_Out Data error...\n" LOG_END
    }
    
    u8OTGInterruptOutCount = u8OTGInterruptOutCount + u32OTGInterrupt_RX_COUNT;

    u32OTGInterrupt_RX_COUNT++;
    if(u32OTGInterrupt_RX_COUNT > u32PacketSize)
        u32OTGInterrupt_RX_COUNT = 1;   
    
    SYS_Free(u8Array);
    
    LOG_LEAVE "[USB_DEVICE_InterruptOut] Leave \n" LOG_END
}

//2008.12.23 Jack add, for USB Charge use
#ifdef USB_CHARGE_ENABLE
MMP_BOOL USB_DEVICE_GetIsUSBCharge(void)
{
    return isUSBCharge;
}

void USB_DEVICE_SetIsUSBCharge(MMP_BOOL flag)
{
    isUSBCharge = flag;
    if(isUSBCharge == MMP_TRUE)
        usbCmdCount = 0;
}
#endif


#endif // #if !defined(DISABLE_USB_DEVICE)
