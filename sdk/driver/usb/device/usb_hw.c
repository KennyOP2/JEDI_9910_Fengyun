/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */


//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================
#define TIME_OUT    500

//=============================================================================
//                              Macro Definition
//=============================================================================
#if defined(DUMP_REG)
void DumpAllUsbRegister(void)
{
    MMP_UINT32  value = 0;
	
    AHB_ReadRegister(USB_HOST_REG_CAPABILITY, &value);
    LOG_DATA "R 00 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_HOST_REG_HCSPARAMS, &value);
    LOG_DATA "R 04 = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_HOST_REG_HCCPARAMS, &value);
    LOG_DATA "R 08 = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_HOST_REG_USBCMD, &value);
    LOG_DATA "R 10 = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_HOST_REG_USBSTS, &value);
    LOG_DATA "R 14 = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_HOST_REG_USBINTR, &value);
    LOG_DATA "R 18 = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_HOST_REG_FRINDEX, &value);
    LOG_DATA "R 1c = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_HOST_REG_PERIODIC_BASE_ADDR, &value);
    LOG_DATA "R 24 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_HOST_REG_ASYNCHRONOUS_BASE_ADDR, &value);
    LOG_DATA "R 28 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_HOST_REG_PORTSC, &value);
    LOG_DATA "R 30 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_HOST_REG_MISC, &value);
    LOG_DATA "R 40 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    LOG_DATA "R 80 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_OTG_REG_INTERRUPT_STATUS, &value);
    LOG_DATA "R 84 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_OTG_REG_INTERRUPT_ENABLE, &value);
    LOG_DATA "R 88 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_MAIN_CONTROL, &value);
    LOG_DATA "R 100 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_PHY_TEST_MODE, &value);
    LOG_DATA "R 114 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_VENDER_SPECIFIC_IO_CONTROL_REGISTER, &value);
    LOG_DATA "R 118 = 0x%8x\n",value LOG_END
		
    AHB_ReadRegister(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, &value);
    LOG_DATA "R 120 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_FIFO_CONFIG, &value);
    LOG_DATA "R 1AC = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_FIFO0_INSTRUCTION_BYTE_CNT, &value);
    LOG_DATA "R 1B0 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_DMA_FIFO_NUM, &value);
    LOG_DATA "R 1C0 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_DMA_SETTING1, &value);
    LOG_DATA "R 1C8 = 0x%8x\n",value LOG_END

    AHB_ReadRegister(USB_DEVICE_REG_DMA_ADDR, &value);
    LOG_DATA "R 1CC = 0x%8x\n",value LOG_END

}
#endif

//===================================
//    On-The-Go Controller
//===================================

/** 0x080h */
MMP_UINT8 USB_OTG_GetHostSpeedTypeReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_UINT8)((value & USB_OTG_MSK_HOST_SPEED_TYPE) >> USB_OTG_SHT_HOST_SPEED_TYPE);
}

MMP_BOOL USB_OTG_IsDeviceSessionEndReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_BOOL)((value & USB_OTG_MSK_DEVICE_SESSION_END) ? MMP_TRUE : MMP_FALSE);
}

void USB_OTG_SetSpeedReg(MMP_UINT32 speed)
{
    MMP_UINT32 mask = USB_OTG_MSK_FORCE_HIGH_SPEED | USB_OTG_MSK_FORCE_FULL_SPEED;
    switch(speed)
    {
    case USB_OTG_HIGH_SPEED_TYPE:
        AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, USB_OTG_MSK_FORCE_HIGH_SPEED, mask);
        break;
    case USB_OTG_FULL_SPEED_TYPE:
        AHB_WriteRegisterMask(USB_OTG_REG_CONTROL_STATUS, USB_OTG_MSK_FORCE_FULL_SPEED, mask);
        break;
    };
}
    
MMP_BOOL USB_OTG_IsDeviceSessionValidReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_BOOL)((value & USB_OTG_MSK_DEVICE_SESSION_VALID) ? MMP_TRUE : MMP_FALSE);
}

MMP_BOOL USB_OTG_IsHostSessionValidReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_BOOL)((value & USB_OTG_MSK_HOST_SESSION_VALID) ? MMP_TRUE : MMP_FALSE);
}

MMP_BOOL USB_OTG_IsHostVbusValidReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_BOOL)((value & USB_OTG_MSK_HOST_VBUS_VALID) ? MMP_TRUE : MMP_FALSE);
}

MMP_INT USB_OTG_WaitHostVbusValidReg(void)
{
    MMP_INT  result = 0;
    MMP_UINT timeOut = 0;

    while(!USB_OTG_IsHostVbusValidReg())
    {
        MMP_Sleep(1);
        if(++timeOut > TIME_OUT)
        {
            result = ERROR_USB_OTG_WAIT_HOST_VBUS_VALID_TIMEOUT;
            goto end;
        }
    }

end:
    if(result)
        LOG_ERROR "USB_OTG_WaitHostVbusValidReg() return error code 0x%08X \n", result LOG_END
    if(timeOut)
        LOG_INFO "USB_OTG_WaitHostVbusValidReg() timeOut = %d \n", timeOut LOG_END
    return result;
}

MMP_INT USB_OTG_WaitHostVbusInvalidReg(void)
{
    MMP_INT  result = 0;
    MMP_UINT timeOut = 0;

    while(USB_OTG_IsHostVbusValidReg())
    {
        MMP_Sleep(1);
        if(++timeOut > TIME_OUT)
        {
            result = ERROR_USB_OTG_WAIT_HOST_VBUS_INVALID_TIMEOUT;
            goto end;
        }
    }

end:
    if(result)
        LOG_ERROR "USB_OTG_WaitHostVbusInvalidReg() return error code 0x%08X \n", result LOG_END
    if(timeOut)
        LOG_INFO "USB_OTG_WaitHostVbusInvalidReg() timeOut = %d \n", timeOut LOG_END
    return result;
}

MMP_UINT USB_OTG_GetCurrentIdReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_UINT)((value & USB_OTG_MSK_CURRENT_ID) ? USB_OTG_ID_B : USB_OTG_ID_A);
}

MMP_BOOL USB_OTG_CurrentRoleIsDevice(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &value);
    return (MMP_BOOL)((value & USB_OTG_MSK_CURRENT_ROLE) ? MMP_TRUE : MMP_FALSE);
}

MMP_INT USB_OTG_WaitChangeToDeviceRoleReg(void)
{
    MMP_INT  result = 0;
    MMP_UINT timeOut = 0;

    while(!USB_OTG_CurrentRoleIsDevice())
    {
        MMP_Sleep(1);
        if(++timeOut > TIME_OUT)
        {
            result = ERROR_USB_OTG_WAIT_CHANGE_DEVICE_ROLE_TIMEOUT;
            goto end;
        }
    }

end:
    if(result)
        LOG_ERROR "USB_OTG_WaitChangeToDeviceRoleReg() return error code 0x%08X \n", result LOG_END
    if(timeOut)
        LOG_INFO "USB_OTG_WaitChangeToDeviceRoleReg() timeOut = %d \n", timeOut LOG_END
    return result;
}

MMP_UINT32 USB_OTG_GetInterruptStatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_OTG_REG_INTERRUPT_STATUS, &value);
    return value;
}




//===================================
//    Device Controller Register  (0x100 ~ 0x1FF)
//===================================

/** 0x100h */
void USB_DEVICE_EnableGlobalInterruptReg(MMP_BOOL enable)
{
    MMP_UINT32 value = enable ? USB_DEVICE_MSK_GLB_INTERRUPT_EN : 0x0;
    AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, value, USB_DEVICE_MSK_GLB_INTERRUPT_EN);
}

MMP_BOOL USB_DEVICE_IsGlobalInterruptEnabledReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MAIN_CONTROL, &value);
    return (MMP_BOOL)( (value & USB_DEVICE_MSK_GLB_INTERRUPT_EN) ? MMP_TRUE : MMP_FALSE );
}

void USB_DEVICE_EnableRemoteWakeupReg(MMP_BOOL enable)
{
    MMP_UINT32 value = enable ? USB_DEVICE_MSK_REMOTE_WAKEUP_CAP : 0x0;
    AHB_WriteRegisterMask(USB_DEVICE_REG_MAIN_CONTROL, value, USB_DEVICE_MSK_REMOTE_WAKEUP_CAP);
}
  
MMP_BOOL USB_DEVICE_GetRemoteWakeupCapsReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MAIN_CONTROL, &value);
    return (MMP_BOOL)( (value & USB_DEVICE_MSK_REMOTE_WAKEUP_CAP) ? MMP_TRUE : MMP_FALSE );
}

MMP_BOOL USB_DEVICE_IsHighSpeedModeReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MAIN_CONTROL, &value);
    return (MMP_BOOL)( (value & USB_DEVICE_MSK_HIGH_SPEED_STATUS) ? MMP_TRUE : MMP_FALSE );
}


/** 0x104h */
void USB_DEVICE_EnableAfterSetConfigurationReg(MMP_BOOL enable)
{
    MMP_UINT32 value = enable ? USB_DEVICE_MSK_AFTER_SET_CONFIG : 0x0;
    AHB_WriteRegisterMask(USB_DEVICE_REG_DEVICE_ADDRESS, value, USB_DEVICE_MSK_AFTER_SET_CONFIG);
}

MMP_BOOL USB_DEVICE_IsAfterSetConfigurationReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_DEVICE_ADDRESS, &value);
    return (MMP_BOOL)( (value & USB_DEVICE_MSK_AFTER_SET_CONFIG) ? MMP_TRUE : MMP_FALSE );
}


/** 0x10Ch */
MMP_UINT32 USB_DEVICE_GetFrameNumReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_SOF_FRAME_NUMBER, &value);
    return ((value & USB_DEVICE_MSK_SOF_FRAME_NUM) >> USB_DEVICE_SHT_SOF_FRAME_NUM);
}



/** 0x120h */
MMP_BOOL USB_DEVICE_IsCxFifoFullReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, &value);
    return (value & USB_DEVICE_MSK_CX_FIFO_FULL) ? MMP_TRUE : MMP_FALSE;
}

MMP_BOOL USB_DEVICE_IsCxFifoEmptyReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, &value);
    return (value & USB_DEVICE_MSK_CX_FIFO_EMPTY) ? MMP_TRUE : MMP_FALSE;
}

MMP_UINT32 USB_DEVICE_GetCxFifoByteCountReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS, &value);
    return (value & USB_DEVICE_MSK_CX_FIFO_BYTE_CNT) >> USB_DEVICE_SHT_CX_FIFO_BYTE_CNT;
}


/** 0x130h */
MMP_UINT32 USB_DEVICE_GetInterruptGroupMaskReg(
    void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MASK_INT_GROUP, &value);
    return value;
}


/** 0x134h */
MMP_UINT32 USB_DEVICE_GetInterruptGroup0MaskReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MASK_INT_GROUP0, &value);
    return value;
}


/** 0x138h */
MMP_UINT32 USB_DEVICE_GetInterruptGroup1MaskReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MASK_INT_GROUP1, &value);
    return value;
}


/** 0x13Ch */
MMP_UINT32 USB_DEVICE_GetInterruptGroup2MaskReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_MASK_INT_GROUP2, &value);
    return value;
}


/** 0x140h */
MMP_UINT32 USB_DEVICE_GetInterruptGroupStatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_INT_GROUP_STATUS, &value);
    return value;
}


/** 0x144h */
MMP_UINT32 USB_DEVICE_GetInterruptGroup0StatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_INT_GROUP0_STATUS, &value);
    return value;
}


/** 0x148h */
MMP_UINT32 USB_DEVICE_GetInterruptGroup1StatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_INT_GROUP1_STATUS, &value);
    return value;
}


/** 0x14Ch */
MMP_UINT32 USB_DEVICE_GetInterruptGroup2StatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_INT_GROUP2_STATUS, &value);
    return value;
}


/** 0x150h */
MMP_UINT32 USB_DEVICE_GetRxZeroPacketStatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_RX_ZERO_PACKET_STATUS, &value);
    return value;
}


/** 0x154h */
MMP_UINT32 USB_DEVICE_GetTxZeroPacketStatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_TX_ZERO_PACKET_STATUS, &value);
    return value;
}


/** 0x158h */
MMP_UINT32 USB_DEVICE_GetIsoSequentialErrorStatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_ISO_SEQ_ABORT_ERROR, &value);
    return (MMP_UINT32)((value & USB_DEVICE_MSK_ISO_SEQ_ERROR) >> USB_DEVICE_SHT_ISO_SEQ_ERROR);
}

MMP_UINT32 USB_DEVICE_GetIsoAbortErrorStatusReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(USB_DEVICE_REG_ISO_SEQ_ABORT_ERROR, &value);
    return (MMP_UINT32)((value & USB_DEVICE_MSK_ISO_ABORT_ERROR) >> USB_DEVICE_SHT_ISO_ABORT_ERROR);
}


//=============================================================================
/** 0x160h
 * IN Endpoint x MaxPacketSize Register
 * (One per Endpoint, x=1~8) (Address=160+4(x-1)h)
 */
//=============================================================================

MMP_BOOL USB_DEVICE_IsInEndpointStallReg(MMP_UINT32 endpoint)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister((USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE + ((endpoint - 1) << 2)), &value);
    return (value & USB_DEVICE_MSK_STALL_IEPX) ? MMP_TRUE : MMP_FALSE;
}


//=============================================================================
/** 0x180h
 * OUT Endpoint x MaxPacketSize Register
 * (One per Endpoint, x=1~8) (Address=180+4(x-1)h)
 */
//=============================================================================
MMP_BOOL USB_DEVICE_IsOutEndpointStallReg(MMP_UINT32 endpoint)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister((USB_DEVICE_REG_OUT_EPX_MAX_PACKETSIZE + ((endpoint - 1) << 2)), &value);
    return (value & USB_DEVICE_MSK_STALL_OEPX) ? MMP_TRUE : MMP_FALSE;
}


/** 0x1A0h 0x1A4 */
void USB_DEVICE_SetEndpointMapReg(MMP_UINT32 endpoint, MMP_UINT8 map)
{
    MMP_UINT32 mask = 0;
    MMP_UINT32 shift = 0;
    switch((endpoint-1)%4)
    {
        case 0:  mask = 0xFF;         shift = 0;  break;
        case 1:  mask = 0xFF00;       shift = 8;  break;
        case 2:  mask = 0xFF0000;     shift = 16; break;
        case 3:  mask = 0xFF000000;   shift = 24; break;
    	default : break;
    }
    AHB_WriteRegisterMask(USB_DEVICE_REG_ENDPOINT1_4_MAP+(((endpoint-1)>>2)<<2), (map << shift), mask);
}

MMP_UINT8 USB_DEVICE_GetEndpointMapReg(MMP_UINT32 endpoint, MMP_BOOL dirIn) /** dirIn: 1 In, 0 Out */
{
    MMP_UINT32 mask = 0;
    MMP_UINT32 shift = 0;
    MMP_UINT32 value = 0;

    switch((endpoint-1)%4)
    {
        case 0:  mask = 0xFF;         shift = 0;  break;
        case 1:  mask = 0xFF00;       shift = 8;  break;
        case 2:  mask = 0xFF0000;     shift = 16; break;
        case 3:  mask = 0xFF000000;   shift = 24; break;
        default : break;
    }

    AHB_ReadRegister(USB_DEVICE_REG_ENDPOINT1_4_MAP+(((endpoint-1)>>2)<<2), &value);
    value = (value & mask) >> shift;
    if(dirIn)
        value &= 0x0F;
    else
        value >>= 4;

    return (MMP_UINT8)value;
}

/** FIFO map */
void USB_DEVICE_SetFifoMapReg(MMP_UINT32 fifoNum, MMP_UINT8 map)
{
    MMP_UINT32 mask = 0;
    MMP_UINT32 shift = 0;
    switch(fifoNum)
    {
        case 0:  mask = 0xFF;         shift = 0;  break;
        case 1:  mask = 0xFF00;       shift = 8;  break;
        case 2:  mask = 0xFF0000;     shift = 16; break;
        case 3:  mask = 0xFF000000;   shift = 24; break;
    	default : break;
    }
    AHB_WriteRegisterMask(USB_DEVICE_REG_FIFO_MAP, (MMP_UINT32)(map << shift), mask);
}

MMP_UINT8 USB_DEVICE_GetFifoMapReg(MMP_UINT32 fifoNum)
{
    MMP_UINT32 mask = 0;
    MMP_UINT32 shift = 0;
    MMP_UINT32 value = 0;
    switch(fifoNum)
    {
        case 0:  mask = 0xFF;         shift = 0;  break;
        case 1:  mask = 0xFF00;       shift = 8;  break;
        case 2:  mask = 0xFF0000;     shift = 16; break;
        case 3:  mask = 0xFF000000;   shift = 24; break;
    	default : break;
    }
    AHB_ReadRegister(USB_DEVICE_REG_FIFO_MAP, &value);
    return (MMP_UINT8)((value & mask) >> shift);
}


/** 0x1ACh */
void USB_DEVICE_SetFifoConfigReg(MMP_UINT32 fifoNum, MMP_UINT8 config)
{
    MMP_UINT32 mask = 0;
    MMP_UINT32 shift = 0;
    switch(fifoNum)
    {
        case 0:  mask = 0xFF;         shift = 0;  break;
        case 1:  mask = 0xFF00;       shift = 8;  break;
        case 2:  mask = 0xFF0000;     shift = 16; break;
        case 3:  mask = 0xFF000000;   shift = 24; break;
    	default : break;
    }
    AHB_WriteRegisterMask(USB_DEVICE_REG_FIFO_CONFIG, (MMP_UINT32)(config << shift), mask);
}

MMP_UINT8 USB_DEVICE_GetFifoConfigReg(MMP_UINT32 fifoNum)
{
    MMP_UINT32 mask  = 0;
    MMP_UINT32 shift = 0;
    MMP_UINT32 value = 0;
    switch(fifoNum)
    {
        case 0:  mask = 0xFF;         shift = 0;  break;
        case 1:  mask = 0xFF00;       shift = 8;  break;
        case 2:  mask = 0xFF0000;     shift = 16; break;
        case 3:  mask = 0xFF000000;   shift = 24; break;
    	default : break;
    }
    AHB_ReadRegister(USB_DEVICE_REG_FIFO_CONFIG, &value);

    return (MMP_UINT8)((value & mask) >> shift);
}


/** 0x1B0h */
MMP_UINT16 USB_DEVICE_GetFifoOutByteCountReg(MMP_UINT8 fifoNum)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister((USB_DEVICE_REG_FIFO0_INSTRUCTION_BYTE_CNT + fifoNum*4), &value);
    return (MMP_UINT16)(value & USB_DEVICE_MSK_FIFO_OUT_BYTE_CNT);
}


/** 0x1C8h */
void USB_DEVICE_GetCxSetupCmdReg(MMP_UINT8* command)
{
#if defined(__FREERTOS__)

    MMP_UINT32 value=0;
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_CX);
    AHB_ReadRegister(USB_DEVICE_REG_SETUP_CMD_READ_PORT, &value);
    value = MMP_SWAP_ENDIAN32(value);
	
    *(command + 3) = (MMP_UINT8)(value & 0xFF);
    *(command + 2) = (MMP_UINT8)((value >>  8) & 0xFF);
    *(command + 1) = (MMP_UINT8)((value >>  16) & 0xFF);
    *(command) = (MMP_UINT8)((value >>  24) & 0xFF);

    AHB_ReadRegister(USB_DEVICE_REG_SETUP_CMD_READ_PORT, &value);
    value = MMP_SWAP_ENDIAN32(value);
	
    *(command + 7) = (MMP_UINT8)(value & 0xFF);
    *(command + 6) = (MMP_UINT8)((value >>  8) & 0xFF);
    *(command + 5) = (MMP_UINT8)((value >>  16) & 0xFF);
    *(command + 4) = (MMP_UINT8)((value >>  24) & 0xFF);

    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);

#else
    // Irene : It's so strange!!! But it can work.
    MMP_UINT16 value = 0;
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_CX);
    HOST_ReadRegister(USB_DEVICE_REG_SETUP_CMD_READ_PORT, &value);
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);
    *(command) = (MMP_UINT8)(value & 0xFF);
    *(command + 1) = (MMP_UINT8)((value >>  8) & 0xFF);
	
    HOST_ReadRegister(USB_DEVICE_REG_SETUP_CMD_READ_PORT, &value);
    *(command + 4) = (MMP_UINT8)(value & 0xFF);
    *(command + 5) = (MMP_UINT8)((value >> 8) & 0xFF);

    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_CX);
    HOST_ReadRegister(USB_DEVICE_REG_SETUP_CMD_READ_PORT+2, &value);
    USB_DEVICE_SetDmaFifoNumReg(DEVICE_DMA_2_FIFO_NON);
    *(command + 6) = (MMP_UINT8)(value & 0xFF);
    *(command + 7) = (MMP_UINT8)((value >>  8) & 0xFF);

    HOST_ReadRegister(USB_DEVICE_REG_SETUP_CMD_READ_PORT+2, &value);
    *(command + 2) = (MMP_UINT8)(value & 0xFF);
    *(command + 3) = (MMP_UINT8)((value >> 8) & 0xFF);
#endif
}

MMP_INT USB_EHCI_Host20_Suspend(void)
{
    MMP_INT result = 0;
    MMP_UINT32  tmp_value = 0;

    AHB_ReadRegister(USB_HOST_REG_USBCMD, &tmp_value);
    if(!(tmp_value & USB_HOST_MSK_RUN_STOP))    
        return (ERROR_USB_EHCI_HOST_SUSPEND_FAIL);

    //<1>.Make sure PortEnable
    AHB_ReadRegister(USB_HOST_REG_PORTSC, &tmp_value);
    if(!(tmp_value & USB_HOST_MSK_PORT_ENABLE))    
        return (ERROR_USB_EHCI_HOST_SUSPEND_FAIL);

#if 0
    //<3>.Write PORTSC->Suspend=1     
    result = USB_HOST_PortSuspendReg();
    if(result)
        goto end;
#endif

    if(result)
        LOG_ERROR "USB_EHCI_Host20_Suspend() return error code 0x%08X \n", result LOG_END
    return result;
}

MMP_INT USB_EHCI_Host20_Close(MMP_UINT8 bForDeviceB)
{
    MMP_INT result = 0;
    MMP_UINT32  tmp_value = 0;

    AHB_ReadRegister(USB_HOST_REG_USBINTR, &tmp_value);
    if(tmp_value & USB_HOST_MSK_INTERRUPT) 
    {
        //<1>.Suspend Host (full speed)
        if(bForDeviceB==0)
        {
            result = USB_EHCI_Host20_Suspend();
            if(result) 
                goto end;
        }
        else 
        {
            #if 0
            // Disable the Asynchronous Schedule
            if(USB_HOST_IsAsynchronousScheduleEnableReg()) 
            {
                result = USB_HOST_EnableAsynchronousScheduleReg(MMP_FALSE);
                if(result)
                    goto end;
            }

            // Disable the Periodic Schedule
            if(USB_HOST_IsPeriodicScheduleEnableReg()) 
            {  
                result = USB_HOST_EnablePeriodicScheduleReg(MMP_FALSE);  
                if(result)
                    goto end;
            }

            result = USB_EHCI_Host20_StopRun_Setting(HOST20_Disable);   
            if(result)
                goto end;
            #endif
        }

        //<2>.Disable the interrupt
        AHB_WriteRegisterMask(USB_HOST_REG_USBINTR, USB_HOST_MSK_INTERRUPT_NONE, USB_HOST_MSK_INTERRUPT);

        //<3>.Clear Interrupt Status
        AHB_WriteRegisterMask(USB_HOST_REG_USBSTS, USB_HOST_MSK_INTERRUPT, USB_HOST_MSK_INTERRUPT);
   } 
   
end:
    if(result)
        LOG_ERROR "USB_EHCI_Host20_Close() return error code 0x%08X \n", result LOG_END
    return result;
}

void USB_OTGC_A_Bus_Drop(void)
{
    USB_EHCI_Host20_Close(0);

    USB_OTG_HostBusRequestReg(MMP_FALSE);
    /**
     * 2008-09-22 Irene: Faraday suggestion
     * Because we don't have OTG controller, so A-device Bus Drop set to zeor.
     */
    //USB_OTG_HostBusDropReg(MMP_TRUE); //Exit when Current Role = Host 
    USB_OTG_HostBusDropReg(MMP_FALSE);
}

void USB_OTGC_A_Bus_Drive(void)
{
   USB_OTG_HostBusDropReg(MMP_FALSE); //Exit when Current Role = Host
   USB_OTG_HostBusRequestReg(MMP_TRUE);
}

	
#if defined(USB_LOGO_TEST)

/** 0x014h */
MMP_BOOL USB_HOST_IsHostControllerHated(void)
{
    MMP_UINT32  value = 0;
    AHB_ReadRegister(USB_HOST_REG_USBSTS, &value);
    return (MMP_BOOL)( (value & USB_HOST_MSK_HALTED) ? MMP_TRUE : MMP_FALSE );
}

#endif // #if defined(USB_LOGO_TEST)


