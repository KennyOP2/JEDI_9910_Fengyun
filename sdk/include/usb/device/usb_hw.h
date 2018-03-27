/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as USB device HW configure header file.
 *
 * @author Irene Lin
 */

#ifndef USB_HW_H
#define USB_HW_H

#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#if defined(DUMP_REG)
void DumpAllUsbRegister(void);
#endif

//===================================
//    On-The-Go Controller
//===================================

/** 0x080h */
#define USB_OTG_PhyResetReg() do{\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, USB_OTG_MSK_PHY_RESET, USB_OTG_MSK_PHY_RESET);\
    MMP_Sleep(1);\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, 0x0, USB_OTG_MSK_PHY_RESET);\
}while(0)

MMP_UINT8 USB_OTG_GetHostSpeedTypeReg(void);

#define USB_OTG_DeviceBusRequestReg(request) do{\
    MMP_UINT32 value = (request) ? USB_OTG_MSK_DEVICE_BUS_REQUEST : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_DEVICE_BUS_REQUEST);\
}while(0)

#define USB_OTG_DeviceEanbleHnpReg(enable) do{\
    MMP_UINT32 value = (enable) ? USB_OTG_MSK_DEVICE_HNP_EN : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_DEVICE_HNP_EN);\
}while(0)

#define USB_OTG_DeviceDischargeVbusReg(enable) do{\
    MMP_UINT32 value = (enable) ? USB_OTG_MSK_DEVICE_DISCHARGE_VBUS : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_DEVICE_DISCHARGE_VBUS);\
}while(0)

#define USB_OTG_HostBusRequestReg(request) do{\
    MMP_UINT32 value = (request) ? USB_OTG_MSK_HOST_BUS_REQUEST : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_HOST_BUS_REQUEST);\
}while(0)

#define USB_OTG_HostBusDropReg(enable) do{\
    MMP_UINT32 value = (enable) ? USB_OTG_MSK_HOST_BUS_DROP : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_HOST_BUS_DROP);\
}while(0)

#define USB_OTG_HostEnableDeviceHnpReg(enable) do{\
    MMP_UINT32 value = (enable) ? USB_OTG_MSK_HOST_SET_DEVICE_HNP_EN : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_HOST_SET_DEVICE_HNP_EN);\
}while(0)

#define USB_OTG_HostEnableSrpDetectionReg(enable) do{\
    MMP_UINT32 value = (enable) ? USB_OTG_MSK_HOST_SRP_DETECTION_EN : 0x0;\
    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, value, USB_OTG_MSK_HOST_SRP_DETECTION_EN);\
}while(0)

#define USB_OTG_FullSpeedPhyResetWrReg()    AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, USB_OTG_MSK_FULL_SPEED_PHY_WR, USB_OTG_MSK_HOST_SRP_DETECTION_EN)

MMP_BOOL USB_OTG_IsDeviceSessionEndReg(void);

void USB_OTG_SetSpeedReg(MMP_UINT32 speed);
    
MMP_BOOL USB_OTG_IsDeviceSessionValidReg(void);

MMP_BOOL USB_OTG_IsHostSessionValidReg(void);

MMP_BOOL USB_OTG_IsHostVbusValidReg(void);

MMP_INT USB_OTG_WaitHostVbusValidReg(void);

MMP_INT USB_OTG_WaitHostVbusInvalidReg(void);

MMP_UINT USB_OTG_GetCurrentIdReg(void);

MMP_BOOL USB_OTG_CurrentRoleIsDevice(void);

MMP_INT USB_OTG_WaitChangeToDeviceRoleReg(void);

MMP_UINT32 USB_OTG_GetInterruptStatusReg(void);

/** 0x084h */
#define USB_OTG_ClearInterruptReg(interrup) AHB_WriteRegisterMask(USB_OTG_REG_INTERRUPT_STATUS, (interrup), (interrup))


/** 0x088h */
#define USB_OTG_EnableInterruptReg(interrup)    AHB_WriteRegisterMask(USB_OTG_REG_INTERRUPT_ENABLE, (interrup), (interrup))
#define USB_OTG_DisableInterruptReg(interrup)   AHB_WriteRegisterMask(USB_OTG_REG_INTERRUPT_ENABLE, 0x0, (interrup))




//===================================
//    Device Controller Register  (0x100 ~ 0x1FF)
//===================================

#define USB_DEVICE_ChipEnableFifoReg()  AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, USB_DEVICE_MSK_CHIP_EN, USB_DEVICE_MSK_CHIP_EN)

/** 0x100h */
void USB_DEVICE_EnableGlobalInterruptReg(MMP_BOOL enable);

MMP_BOOL USB_DEVICE_IsGlobalInterruptEnabledReg(void);

/* Half speed enable - 1: The FIFO controller asserts ACK to DMA for every 2 clock cycles. */
#define USB_DEVICE_EnableHalfSpeedReg() AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, USB_DEVICE_MSK_HALF_SPEED_EN, USB_DEVICE_MSK_HALF_SPEED_EN)

#define USB_DEVICE_SoftwareResetReg()   do{\
    AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, USB_DEVICE_MSK_SW_RESET, USB_DEVICE_MSK_SW_RESET); \
    AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, 0x0, USB_DEVICE_MSK_SW_RESET); \
} while(0)

void USB_DEVICE_EnableRemoteWakeupReg(MMP_BOOL enable);
  
#define USB_DEVICE_ForceFullSpeedReg()  AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, USB_DEVICE_MSK_FORCE_FULL_SPEED, USB_DEVICE_MSK_FORCE_FULL_SPEED)
  
MMP_BOOL USB_DEVICE_GetRemoteWakeupCapsReg(void);

#define USB_DEVICE_SetPhyGoSuspendReg()     AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, USB_DEVICE_MSK_PHY_GO_SUSPEND, USB_DEVICE_MSK_PHY_GO_SUSPEND)
#define USB_DEVICE_ClearPhyGoSuspendReg()   AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, 0x0, USB_DEVICE_MSK_PHY_GO_SUSPEND)

MMP_BOOL USB_DEVICE_IsHighSpeedModeReg(void);

#define USB_DEVICE_DmaResetReg()    AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, USB_DEVICE_MSK_DMA_RESET, USB_DEVICE_MSK_DMA_RESET)
 

/** 0x104h */
#define USB_DEVICE_SetDeviceAddressReg(addr)    AHB_WriteRegisterMask(USB_DEVICE_REG_DEVICE_ADDRESS, (addr), USB_DEVICE_MSK_DEVICE_ADDRESS)

void USB_DEVICE_EnableAfterSetConfigurationReg(MMP_BOOL enable);

MMP_BOOL USB_DEVICE_IsAfterSetConfigurationReg(void);


/** 0x108h */
#define USB_DEVICE_ClearFifoReg()   AHB_WriteRegisterMask(USB_DEVICE_REG_DEVICE_TEST, USB_DEVICE_MSK_CLEAR_FIFO, USB_DEVICE_MSK_CLEAR_FIFO)


/** 0x10Ch */
MMP_UINT32 USB_DEVICE_GetFrameNumReg(void);

/** 0x110h */
#define USB_DEVICE_SetSofMaskTimerReg(timeCount)    AHB_WriteRegisterMask(USB_DEVICE_REG_DEVICE_TEST, (timeCount), USB_DEVICE_MSK_CLEAR_FIFO)


//=============================================================================
/** 0x114h
 * Enable/disable unplug.
 * In order to let PHY drive D+ and D-, the AP should clear UNPLUG after the hardware reset.
 * If the AP does not clear UNPLUG bit, the device will be always soft-detached and the USB
 * host will never detect the attachment of the device.
 */
//=============================================================================
#define USB_DEVICE_EnableUnplugReg(enable)    do{\
    MMP_UINT32 value = enable ? USB_DEVICE_MSK_UNPLUG : 0x0; \
    AHB_WriteRegisterMask(USB_DEVICE_REG_PHY_TEST_MODE, value, USB_DEVICE_MSK_UNPLUG); \
}while(0)

#define USB_DEVICE_SelectPhyTestModeReg(mode)   AHB_WriteRegisterMask(USB_DEVICE_REG_PHY_TEST_MODE, (mode), USB_DEVICE_MSK_TEST_MODE)


/** 0x120h */
#define USB_DEVICE_SetCxDoneReg()   AHB_WriteRegisterMask(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, USB_DEVICE_MSK_CX_DONE, USB_DEVICE_MSK_CX_DONE)
#define USB_DEVICE_SetTestPacketDoneReg()   AHB_WriteRegisterMask(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, USB_DEVICE_MSK_TST_PKDONE, USB_DEVICE_MSK_TST_PKDONE)
#define USB_DEVICE_StallCxReg()     AHB_WriteRegisterMask(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, USB_DEVICE_MSK_STALL_CX, USB_DEVICE_MSK_STALL_CX)
#define USB_DEVICE_ClearCxFifoReg() AHB_WriteRegisterMask(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, USB_DEVICE_MSK_CLEAR_CX_FIFO_DATA, USB_DEVICE_MSK_CLEAR_CX_FIFO_DATA)

MMP_BOOL USB_DEVICE_IsCxFifoFullReg(void);

MMP_BOOL USB_DEVICE_IsCxFifoEmptyReg(void);

MMP_UINT32 USB_DEVICE_GetCxFifoByteCountReg(void);

//=============================================================================
/** 0x124h
 * Control the timing delay from the time indicated in GO-SUSPEND to the time the
 * device enters a suspend mode.
 */
//=============================================================================
#define USB_DEVICE_SetIdleCounterReg(cnt)   \
    AHB_WriteRegisterMask(USB_DEVICE_REG_IDLE_COUNTER, \
                          ((cnt) << USB_DEVICE_SHT_IDLE_COUNTER) & USB_DEVICE_MSK_IDLE_COUNTER, \
                          USB_DEVICE_MSK_IDLE_COUNTER)


/** 0x130h */
MMP_UINT32 USB_DEVICE_GetInterruptGroupMaskReg(void);


/** 0x134h */
#define USB_DEVICE_EnableInterruptGroup0Reg(interrupt)  AHB_WriteRegisterMask(USB_DEVICE_REG_MASK_INT_GROUP0, 0x0, (interrupt))
#define USB_DEVICE_DisableInterruptGroup0Reg(interrupt) AHB_WriteRegisterMask(USB_DEVICE_REG_MASK_INT_GROUP0, (interrupt), (interrupt))
MMP_UINT32 USB_DEVICE_GetInterruptGroup0MaskReg(void);


/** 0x138h */
#define USB_DEVICE_EnableInterruptGroup1Reg(interrupt)  AHB_WriteRegisterMask(USB_DEVICE_REG_MASK_INT_GROUP1, 0x0, (interrupt))
#define USB_DEVICE_DisableInterruptGroup1Reg(interrupt) AHB_WriteRegisterMask(USB_DEVICE_REG_MASK_INT_GROUP1, (interrupt), (interrupt))
MMP_UINT32 USB_DEVICE_GetInterruptGroup1MaskReg(void);


/** 0x13Ch */
#define USB_DEVICE_EnableInterruptGroup2Reg(interrupt)  AHB_WriteRegisterMask(USB_DEVICE_REG_MASK_INT_GROUP2, 0x0, (interrupt))
#define USB_DEVICE_DisableInterruptGroup2Reg(interrupt) AHB_WriteRegisterMask(USB_DEVICE_REG_MASK_INT_GROUP2, (interrupt), (interrupt))
MMP_UINT32 USB_DEVICE_GetInterruptGroup2MaskReg(void);


/** 0x140h */
MMP_UINT32 USB_DEVICE_GetInterruptGroupStatusReg(void);
#define USB_DEVICE_ClearInterruptGroupStatusReg(interrupt)  AHB_WriteRegisterMask(USB_DEVICE_REG_INT_GROUP_STATUS, 0x0, (interrupt))


/** 0x144h */
MMP_UINT32 USB_DEVICE_GetInterruptGroup0StatusReg(void);
#define USB_DEVICE_ClearInterruptGroup0StatusReg(interrupt) AHB_WriteRegisterMask(USB_DEVICE_REG_INT_GROUP0_STATUS, 0x0, (interrupt))


/** 0x148h */
MMP_UINT32 USB_DEVICE_GetInterruptGroup1StatusReg(void);

#define USB_DEVICE_ClearInterruptGroup1StatusReg(interrupt) AHB_WriteRegisterMask(USB_DEVICE_REG_INT_GROUP1_STATUS, 0x0, (interrupt))


/** 0x14Ch */
MMP_UINT32 USB_DEVICE_GetInterruptGroup2StatusReg(void);
#define USB_DEVICE_ClearInterruptGroup2StatusReg(interrupt) AHB_WriteRegisterMask(USB_DEVICE_REG_INT_GROUP2_STATUS, 0x0, (interrupt))


/** 0x150h */
MMP_UINT32 USB_DEVICE_GetRxZeroPacketStatusReg(void);
#define USB_DEVICE_ClearRxZeroPacketStatusReg(endpoint) AHB_WriteRegisterMask(USB_DEVICE_REG_RX_ZERO_PACKET_STATUS, 0x0, ((endpoint)&0xF))


/** 0x154h */
MMP_UINT32 USB_DEVICE_GetTxZeroPacketStatusReg(void);
#define USB_DEVICE_ClearTxZeroPacketStatusReg(endpoint) AHB_WriteRegisterMask(USB_DEVICE_REG_TX_ZERO_PACKET_STATUS, 0x0, ((endpoint)&0xF))


/** 0x158h */
MMP_UINT32 USB_DEVICE_GetIsoSequentialErrorStatusReg(void);

MMP_UINT32 USB_DEVICE_GetIsoAbortErrorStatusReg(void);
#define USB_DEVICE_ClearIsoSequentialErrorStatusReg(data) \
    AHB_WriteRegisterMask(USB_DEVICE_REG_ISO_SEQ_ABORT_ERROR, \
                          ((data) << USB_DEVICE_SHT_ISO_SEQ_ERROR),\
                          USB_DEVICE_MSK_ISO_SEQ_ERROR);

#define USB_DEVICE_ClearIsoAbortErrorStatusReg(data) \
    AHB_WriteRegisterMask(USB_DEVICE_REG_ISO_SEQ_ABORT_ERROR, \
                          ((data) << USB_DEVICE_SHT_ISO_ABORT_ERROR),\
                          USB_DEVICE_MSK_ISO_ABORT_ERROR);


//=============================================================================
/** 0x160h
 * IN Endpoint x MaxPacketSize Register
 * (One per Endpoint, x=1~8) (Address=160+4(x-1)h)
 */
//=============================================================================
#define USB_DEVICE_SetTxNumHighBandwidthReg(endpoint, dir, size) \
    AHB_WriteRegisterMask((USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), \
                          ((((MMP_UINT8)((size) >> 11)+1) << 13)*(1 - (dir))),\
                          USB_DEVICE_MSK_TX_NUM_HBW_IEPX);

/** for both In/Out endpoint use */
#define USB_DEVICE_SetMaxPacketSizeReg(endpoint, dir, size)\
    AHB_WriteRegisterMask((USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE + ((dir) * 0x20) + (((endpoint) - 1) << 2)), \
                          (size),\
                          USB_DEVICE_MSK_MAXPS_IEPX);

#define USB_DEVICE_StallInEndpointReg(endpoint, stall) do{  \
    MMP_UINT32 value = (stall) ? USB_DEVICE_MSK_STALL_IEPX : 0x0;\
    AHB_WriteRegisterMask((USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), \
                          value,\
                          USB_DEVICE_MSK_STALL_IEPX);\
}while(0)

MMP_BOOL USB_DEVICE_IsInEndpointStallReg(MMP_UINT32 endpoint);

#define USB_DEVICE_ResetToggleInEndpointReg(endpoint) do{\
    AHB_WriteRegisterMask((USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), USB_DEVICE_MSK_RSTG_IEPX, USB_DEVICE_MSK_RSTG_IEPX);\
    AHB_WriteRegisterMask((USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), 0x0, USB_DEVICE_MSK_RSTG_IEPX);\
}while(0)


//=============================================================================
/** 0x180h
 * OUT Endpoint x MaxPacketSize Register
 * (One per Endpoint, x=1~8) (Address=180+4(x-1)h)
 */
//=============================================================================
#define USB_DEVICE_StallOutEndpointReg(endpoint, stall) do{\
    MMP_UINT32 value = (stall) ? USB_DEVICE_MSK_STALL_OEPX : 0x0;\
    AHB_WriteRegisterMask((USB_DEVICE_REG_OUT_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), \
                          value,\
                          USB_DEVICE_MSK_STALL_OEPX);\
}while(0)

MMP_BOOL USB_DEVICE_IsOutEndpointStallReg(MMP_UINT32 endpoint);

#define USB_DEVICE_ResetToggleOutEndpointReg(endpoint) do{\
    AHB_WriteRegisterMask((USB_DEVICE_REG_OUT_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), USB_DEVICE_MSK_RSTG_OEPX, USB_DEVICE_MSK_RSTG_OEPX);\
    AHB_WriteRegisterMask((USB_DEVICE_REG_OUT_EPX_MAX_PACKETSIZE + (((endpoint) - 1) << 2)), 0x0, USB_DEVICE_MSK_RSTG_OEPX);\
}while(0)


/** 0x1A0h 0x1A4 */
#define USB_DEVICE_ClearAllEndpointMapReg() do{\
    AHB_WriteRegister(USB_DEVICE_REG_ENDPOINT1_4_MAP, 0x0); \
    AHB_WriteRegister(USB_DEVICE_REG_ENDPOINT5_8_MAP, 0x0); \
}while(0)

void USB_DEVICE_SetEndpointMapReg(MMP_UINT32 endpoint, MMP_UINT8 map);

MMP_UINT8 USB_DEVICE_GetEndpointMapReg(MMP_UINT32 endpoint, MMP_BOOL dirIn); /** dirIn: 1 In, 0 Out */

/** FIFO map */
#define USB_DEVICE_ClearAllFifoMapReg() AHB_WriteRegister(USB_DEVICE_REG_FIFO_MAP, 0x0)

void USB_DEVICE_SetFifoMapReg(MMP_UINT32 fifoNum, MMP_UINT8 map);

MMP_UINT8 USB_DEVICE_GetFifoMapReg(MMP_UINT32 fifoNum);


/** 0x1ACh */
#define USB_DEVICE_ClearAllFifoConfigReg() AHB_WriteRegister(USB_DEVICE_REG_FIFO_CONFIG, 0x0)

void USB_DEVICE_SetFifoConfigReg(MMP_UINT32 fifoNum, MMP_UINT8 config);

MMP_UINT8 USB_DEVICE_GetFifoConfigReg(MMP_UINT32 fifoNum);


/** 0x1B0h */
MMP_UINT16 USB_DEVICE_GetFifoOutByteCountReg(MMP_UINT8 fifoNum);

#define USB_DEVICE_SetFifoDoneReg(fifoNum) AHB_WriteRegisterMask((USB_DEVICE_REG_FIFO0_INSTRUCTION_BYTE_CNT + (fifoNum)*4), USB_DEVICE_MSK_FIFO_DONE, USB_DEVICE_MSK_FIFO_DONE)
#define USB_DEVICE_ResetFifoReg(fifoNum)   AHB_WriteRegisterMask((USB_DEVICE_REG_FIFO0_INSTRUCTION_BYTE_CNT + (fifoNum)*4), USB_DEVICE_MSK_FIFO_RESET, USB_DEVICE_MSK_FIFO_RESET)


/** 0x1C0h */
#define USB_DEVICE_SetDmaFifoNumReg(select) AHB_WriteRegisterMask(USB_DEVICE_REG_DMA_FIFO_NUM, (select), USB_DEVICE_MSK_DMA_2_FIFO_NUM)


/** 0x1C8h */
#define USB_DEVICE_SetDmaLengthDirReg(length, dir) do{\
    AHB_WriteRegisterMask(USB_DEVICE_REG_DMA_SETTING1, ((length) << USB_DEVICE_SHT_DMA_LENGTH), USB_DEVICE_MSK_DMA_LENGTH);\
    AHB_WriteRegisterMask(USB_DEVICE_REG_DMA_SETTING1, ((1-(dir))<<1), USB_DEVICE_MSK_DMA_DIRECTION);\
}while(0)

#define USB_DEVICE_DmaStartReg(start) do{\
    MMP_UINT32 value = start ? USB_DEVICE_MSK_DMA_START : 0x0;\
    AHB_WriteRegisterMask(USB_DEVICE_REG_DMA_SETTING1, value, USB_DEVICE_MSK_DMA_START);\
}while(0)

#define USB_DEVICE_SetDmaAddrReg(addr)  AHB_WriteRegister(USB_DEVICE_REG_DMA_ADDR, (addr) - (MMP_UINT32)HOST_GetVramBaseAddress())

void USB_DEVICE_GetCxSetupCmdReg(MMP_UINT8* command);

MMP_INT USB_EHCI_Host20_Close(MMP_UINT8 bForDeviceB);
void USB_OTGC_A_Bus_Drop(void);
void USB_OTGC_A_Bus_Drive(void);


#define mLowByte(u16)	((MMP_UINT8)(u16	 ))
#define mHighByte(u16)	((MMP_UINT8)(u16 >> 8))


//<4>.0x010(USBCMD - USB Command Register)
#define HOST20_Enable                                      0x01
#define HOST20_Disable                                     0x00

	
#if defined(USB_LOGO_TEST)

/** 0x014h */
MMP_BOOL USB_HOST_IsHostControllerHated(void);

/** 0x030h */
#define USB_HOST_SetPortTestMode(testmode)  AHB_WriteRegisterMask(USB_HOST_REG_PORTSC, (testmode), USB_HOST_MSK_PORT_TEST)
#define USB_HOST_SetDataTransferIsDone()    AHB_WriteRegisterMask(USB_HOST_REG_PORTSC, 0x100000, 0x100000)

#endif // #if defined(USB_LOGO_TEST)




#ifdef __cplusplus
}
#endif

#endif // USB_HW_H
