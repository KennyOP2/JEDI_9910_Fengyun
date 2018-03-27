/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  DMA Controller internal function implementation.
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "dma/config.h"
#include "sys/sys.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
void DMA_DumpFeatureReg(void)
{
    MMP_UINT32 reg = 0;
    AHB_ReadRegister(DMA_REG_FEATURE, &reg);
    LOG_INFO " DMA controller feature: \n" LOG_END
    LOG_INFO " FIFO ram address width: %d\n", (reg & 0xF) LOG_END

    if(reg & 0x100)
        LOG_INFO " DMA supports link list\n" LOG_END
    else
        LOG_INFO " DMA does not support link list \n" LOG_END

    if(reg & 0x200)
        LOG_INFO " DMA has AHB0 and AHB1 \n" LOG_END
    else
        LOG_INFO " DMA only has AHB0 \n" LOG_END

    if(reg & 0x400)
        LOG_INFO " DMA has built in a simple bridge \n" LOG_END
    else
        LOG_INFO " DMA has not built in a simple bridge \n" LOG_END

    LOG_INFO " DMA maximum channel number: %d \n", ((reg&0xE000)>>12) LOG_END
}

void DMA_EnableControllerReg(void)
{
    AHB_WriteRegisterMask(DMA_REG_MAIN_CSR, DMA_MSK_CTRL_EN, DMA_MSK_CTRL_EN);

    /** open channel synchronization logic */
    AHB_WriteRegister(DMA_REG_SYNC, 0xFF);
}

MMP_INT DMA_WaitChannelReadyReg(
    DMA_CTXT* ctxt)
{
    MMP_INT    result = 0;
    MMP_UINT32 reg = 0;
    SYS_CLOCK_T lastTime = 0;
    SYS_CLOCK_T sleepTime = 0;
    MMP_INT    retries = 50;
	MMP_UINT8  channel = ctxt->channel;

    do
    {
        AHB_ReadRegister((DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), &reg);
    } while((reg & DMA_MSK_CHANNEL_EN) && --retries);
    if(retries)
        goto end;

    lastTime = sleepTime = SYS_GetClock();
    do
    {
        AHB_ReadRegister((DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), &reg);
#if defined(FPGA)
        if(SYS_GetDuration(lastTime) > 100000)
        {
            result = ERROR_DMA_WAIT_CHANNEL_READY_TIMEOUT;
            goto end;
        }
		if(SYS_GetDuration(sleepTime) > 2000) // test
        {
			printf(" reg 0x%08X = 0x%08X \n", (DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), reg);
            sleepTime = SYS_GetClock();
            MMP_Sleep(0);
        }
#else
        if(SYS_GetDuration(lastTime) > 1600)
        {
            result = ERROR_DMA_WAIT_CHANNEL_READY_TIMEOUT;
            goto end;
        }
        if(SYS_GetDuration(sleepTime) > 1)
        {
            sleepTime = SYS_GetClock();
            MMP_Sleep(0);
        }
#endif
    } while(reg & DMA_MSK_CHANNEL_EN);

end:
    if(result)
    {
        MMP_UINT32 reg = 0;
        LOG_ERROR "DMA_WaitChannelReadyReg() return error code 0x%08X, reg 0x%08X = 0x%08X \n", result, (DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), reg LOG_END

		AHB_ReadRegister(DMA_REG_C0_SRC_ADDR, &reg);
		LOG_INFO " Reg 0x%08X = 0x%08X => src addr\n", DMA_REG_C0_SRC_ADDR, reg LOG_END
		AHB_ReadRegister(DMA_REG_C0_DST_ADDR, &reg);
		LOG_INFO " Reg 0x%08X = 0x%08X => dst addr\n", DMA_REG_C0_DST_ADDR, reg LOG_END
		AHB_ReadRegister(DMA_REG_C0_TX_SIZE, &reg);
		LOG_INFO " Reg 0x%08X = 0x%08X => tx size\n", DMA_REG_C0_TX_SIZE, reg LOG_END
		AHB_ReadRegister( (DMA_REG_C0_LINKED_LIST_POINTER + (channel*DMA_CHANNEL_OFFSET)), &reg);
		LOG_INFO " Reg 0x%08X = 0x%08X => llp\n", (DMA_REG_C0_LINKED_LIST_POINTER + (channel*DMA_CHANNEL_OFFSET)), reg LOG_END

        AHB_ReadRegister(DMA_REG_TC_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_TC_STATUS, reg LOG_END
        AHB_ReadRegister(DMA_REG_ERR_ABT_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_ERR_ABT_STATUS, reg LOG_END
        AHB_ReadRegister(DMA_REG_CHANNEL_EN_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_CHANNEL_EN_STATUS, reg LOG_END
        AHB_ReadRegister(DMA_REG_CHANNEL_BUSY_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_CHANNEL_BUSY_STATUS, reg LOG_END
        LOG_ERROR " Disable channal %d !! \n", channel LOG_END
        AHB_WriteRegisterMask((DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), 0x0, DMA_MSK_CHANNEL_EN);
		//while(1);
    }
    return result;
}


MMP_INT DMA_BusyWaitChannelReadyReg(
    DMA_CTXT* ctxt)
{
    MMP_INT    result = 0;
    MMP_UINT32 reg = 0;
    SYS_CLOCK_T lastTime = 0;
    SYS_CLOCK_T sleepTime = 0;
    MMP_INT    retries = 10;
	MMP_UINT8  channel = ctxt->channel;

    do
    {
        AHB_ReadRegister((DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), &reg);
    } while((reg & DMA_MSK_CHANNEL_EN) && --retries);
    if(retries)
        goto end;
/*
    lastTime = SYS_GetClock();
    do
    {
        AHB_ReadRegister((DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), &reg);
        if(SYS_GetDuration(lastTime) > 1600)
        {
            result = ERROR_DMA_WAIT_CHANNEL_READY_TIMEOUT;
            goto end;
        }
    } while(reg & DMA_MSK_CHANNEL_EN);
//*/
end:
    if(result)
    {
        MMP_UINT32 reg = 0;
        LOG_ERROR "DMA_BusyWaitChannelReadyReg() return error code 0x%08X, reg 0x%08X = 0x%08X \n", result, (DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), reg LOG_END

        AHB_ReadRegister(DMA_REG_TC_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_TC_STATUS, reg LOG_END
        AHB_ReadRegister(DMA_REG_ERR_ABT_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_ERR_ABT_STATUS, reg LOG_END
        AHB_ReadRegister(DMA_REG_CHANNEL_EN_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_CHANNEL_EN_STATUS, reg LOG_END
        AHB_ReadRegister(DMA_REG_CHANNEL_BUSY_STATUS, &reg);
        LOG_INFO " Reg 0x%08X = 0x%08X \n", DMA_REG_CHANNEL_BUSY_STATUS, reg LOG_END
        LOG_ERROR " Disable channal %d !! \n", channel LOG_END
        AHB_WriteRegisterMask((DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), 0x0, DMA_MSK_CHANNEL_EN);
    }

    return result;
}

