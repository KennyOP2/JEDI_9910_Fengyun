/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *    sdmmc_hw.c SD/MMC controller basic hardware setting
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "sd/config.h"
#include "sd/sd_reg.h"
#include "sd/sd_hw.h"
#include "sd/sd.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define SD_WAIT_FIFO_TIMEOUT (1000)

//=============================================================================
//                              Global Data Definition
//=============================================================================
MMP_UINT8 sdResp[17];

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Power up procedure
 */
//=============================================================================
void SD_PowerOnReg(MMP_INT index)
{
    MMP_INT    result = 0;
    MMP_UINT32 card = (index == SD_1) ? MMP_CARD_SD_I : MMP_CARD_SD2;

    HOST_CardPowerReset(card, 30);
    SD_SelectIo(index);

    /** clock delay */
    SDMMC_DefaultDelay();

    /** Reset smedia wrap */
    SDW_SwResetReg();

    /** Reset SD IF */
    SD_ResetIFReg();
    SD_SelectIo(index); // should select sd index again after reset

    /** card power up */
    MMP_Sleep(250);

    /** Init clock: Status 0 D[4:3], clock divisor */
    SD_SetClockDivReg(0x200); // 80/512 = 156
    //SD_SetClockDivReg(0x87); // 27/135 = 200

    SD_GenClockReg();
    SD_SetSectorLengthReg(SD_SECTOR_SIZE);
    SD_SetRespTimeoutReg(64 * 2);
}

//=============================================================================
/**
 * Power down procedure
 */
//=============================================================================
void SD_PowerDownReg(SD_CTXT *ctxt)
{
    MMP_UINT32 card = (ctxt->index == SD_1) ? MMP_CARD_SD_I : MMP_CARD_SD2;

    /** card power down */
    HOST_CardPowerReset(card, 30);

    /** Reset smedia wrap */
    SDW_SwResetReg();

    /** reset SD host controller */
    SD_ResetIFReg();
}

//=============================================================================
/**
 * Send command related setting without wait ready.
 */
//=============================================================================
void SD_SendCmdNoWaitReg(
    SD_CTXT *ctxt,
    MMP_UINT8 command,
    MMP_UINT8 arg3,
    MMP_UINT8 arg2,
    MMP_UINT8 arg1,
    MMP_UINT8 arg0,
    MMP_UINT8 condition)
{
#if defined(MM9910)
    MMP_UINT32 errBypass = 0;
    if (!(command & RESP_CRC))
        errBypass |= SD_MSK_RESP_CRC_BYPASS;
    if (command & RESP_NON)
        errBypass |= SD_MSK_RESP_TIMEOUT_BYPASS;
    AHB_WriteRegister(SD_REG_STS1, errBypass);
#endif

    /** clock/command delay */
    if ((condition & SD_CMD_DATA_IN) == SD_CMD_DATA_IN)
    {
        ctxt->flags |= SD_FLAGS_DATA_IN;
        SDMMC_ReadDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
    }

    ctxt->cmd = SD_CMD(command);
    AHB_WriteRegister(SD_REG_ARG0, (MMP_UINT32)arg0);
    AHB_WriteRegister(SD_REG_ARG1, (MMP_UINT32)arg1);
    AHB_WriteRegister(SD_REG_ARG2, (MMP_UINT32)arg2);
    AHB_WriteRegister(SD_REG_ARG3, (MMP_UINT32)arg3);
    AHB_WriteRegister(SD_REG_COMMAND, (MMP_UINT32)ctxt->cmd);
#if defined(SD_IRQ_ENABLE)
    ctxt->flags |= SD_FLAGS_IRQ_SD;
#endif
    AHB_WriteRegister(SD_REG_CTL, (MMP_UINT32)(SD_MSK_CMD_TRIGGER | condition));
}

//=============================================================================
/**
 * Send command related setting and wait ready.
 */
//=============================================================================
MMP_INT SD_SendCmdReg(
    SD_CTXT *ctxt,
    MMP_UINT8 command,
    MMP_UINT8 arg3,
    MMP_UINT8 arg2,
    MMP_UINT8 arg1,
    MMP_UINT8 arg0,
    MMP_UINT8 condition)
{
    MMP_INT result = 0;
    ctxt->timeout = (condition & SD_MSK_CMD_TYPE) ? 200 : 50 /*30*/;

    if (((SD_CMD(command) & SD_MSK_CMD) == SD_CMD(COM_ADTC_READ_SINGLE_BLOCK)) ||
        ((SD_CMD(command) & SD_MSK_CMD) == SD_CMD(COM_ADTC_READ_MULTIPLE_BLOCK)) ||
        ((SD_CMD(command) & SD_MSK_CMD) == SD_CMD(COM_ADTC_WRITE_SINGLE_BLOCK)) ||
        ((SD_CMD(command) & SD_MSK_CMD) == SD_CMD(COM_ADTC_WRITE_MULTIPLE_BLOCK)) ||
        ((SD_CMD(command) & SD_MSK_CMD) == SD_CMD(COM_AC_STOP_TRANSMISSION)))
    {
        ctxt->timeout = 2500;
    }

    SD_SendCmdNoWaitReg(ctxt, command, arg3, arg2, arg1, arg0, condition);
    result = SD_WaitSendCmdReadyReg(ctxt);
    if (result)
    {
        if (ctxt->cmd != 8)
        {
            MMP_UINT32 reg1, reg2, reg3;
            AHB_ReadRegister(SD_REG_CTL, &reg1);
            AHB_ReadRegister(SD_REG_STS1, &reg2);
            AHB_ReadRegister(SDW_REG_WRAP_STATUS, &reg3);
            LOG_ERROR " SD_SendCmdReg(0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X) return error code 0x%08X => \nreg 0x%08X = 0x%08X \nreg 0x%08X = 0x%08X \nreg 0x%08X = 0x%08X \n",
                ctxt->cmd, arg3, arg2, arg1, arg0, result, SD_REG_CTL, reg1, SD_REG_STS1, reg2, SDW_REG_WRAP_STATUS, reg3 LOG_END
        }
    }
    return result;
}

#if defined(SD_IRQ_ENABLE)
//#include "ite/ith.h"

MMP_EVENT sd_isr_event = MMP_NULL;

static MMP_INLINE void irq_status(SD_CTXT *ctxt)
{
    MMP_UINT32 mask = SD_FLAGS_IRQ_SD_END;

    if (ctxt->flags & SD_FLAGS_DATA_DMA)
        mask |= SD_FLAGS_IRQ_DMA_END;

#if defined(MM9910)
    if (ctxt->flags & SD_FLAGS_WRAP_INTR)
        mask |= SD_FLAGS_IRQ_WRAP_END;
#endif

    if ((ctxt->intr_flags & mask) == mask)
        SYS_SetEventFromIsr(sd_isr_event);

    return;
}

void dma_isr(void *arg, MMP_UINT32 status)
{
    SD_CTXT *ctxt = (SD_CTXT *)arg;
    if ((status & DMA_IRQ_SUCCESS) != DMA_IRQ_SUCCESS)
    {
        ctxt->intr_flags |= SD_FLAGS_IRQ_DMA_ERROR;
        //LOG_ERROR " dma irq error 0x%X \n", status LOG_END
    }

    ctxt->intr_flags |= SD_FLAGS_IRQ_DMA_END;
    irq_status(ctxt);
}

void sd_isr(void *arg)
{
#if defined(MM9910)
    SD_CTXT    *ctxt = (SD_CTXT *)arg;
    MMP_UINT32 intr;

    if (!(ctxt->flags & SD_FLAGS_IRQ_SD))
        ctxt++;
    if (!(ctxt->flags & SD_FLAGS_IRQ_SD))
    {
        printf("\n\n unknown sd intr !!!! \n\n");
        AHB_WriteRegisterMask(0xDE200000 + 0x104, 0 << (37 - 32), 0x1 << (37 - 32));//kenny
        return;
    }

    /** check sd controller interrupt */
    AHB_ReadRegister(SD_REG_INTR, &intr);
    AHB_WriteRegister(SD_REG_INTR, intr);
    intr = (intr & SD_INTR_ALL);
    if (intr)
    {
        ctxt->intrErr = (MMP_UINT8)(intr & SD_INTR_ERR);
        if (ctxt->intrErr)
        {
            ctxt->intr_flags |= SD_FLAGS_IRQ_SD_ERROR;
            LOG_ERROR " sd irq error reg 0x%08X = 0x%08X \n", SD_REG_INTR, intr LOG_END
        }
        ctxt->intr_flags |= SD_FLAGS_IRQ_SD_END;
    }

    /** check wrap interrupt */
    AHB_ReadRegister(SDW_REG_WRAP_STATUS, &intr);
    if (intr & SDW_INTR_WRAP_END)
    {
        ctxt->intr_flags |= SD_FLAGS_IRQ_WRAP_END;
        AHB_WriteRegister(SDW_REG_WRAP_STATUS, intr);
    }

#else
    SD_CTXT    *ctxt = (SD_CTXT *)arg;
    MMP_UINT32 status;

    if (!(ctxt->flags & SD_FLAGS_IRQ_SD))
        ctxt++;
    if (!(ctxt->flags & SD_FLAGS_IRQ_SD))
    {
        AHB_ReadRegister(SD_REG_STS1, &status);
        //ithPrintf("\n\n unknown sd intr, sts1 0x%08X !!!! \n\n", status);
        return;
    }

    AHB_ReadRegister(SD_REG_STS1, &status);
    AHB_WriteRegisterMask(SD_REG_STS1, SD_MSK_INTR_CLR, SD_MSK_INTR_CLR); /** clear sd interrupt */
    if (status & SD_ERROR)
    {
        ctxt->intr_flags |= SD_FLAGS_IRQ_SD_ERROR;
        //LOG_ERROR " sd irq error reg 0x%08X = 0x%08X \n", SD_REG_STS1, status LOG_END
    }
    ctxt->intr_flags |= SD_FLAGS_IRQ_SD_END;
#endif

    irq_status(ctxt);
}

MMP_INT SD_WaitSendCmdReadyReg(SD_CTXT *ctxt)
{
    MMP_INT result;

    /** check sd ready and dma ready */

    result = SYS_WaitEvent(sd_isr_event, ctxt->timeout);
    if (result)
    {
        if (ctxt->intr_flags & SD_FLAGS_IRQ_SD_END)
            LOG_ERROR " SD end! \n" LOG_END
        if (ctxt->intr_flags & SD_FLAGS_IRQ_DMA_END)
            LOG_ERROR " DMA end! \n" LOG_END
        if (ctxt->intr_flags & SD_FLAGS_IRQ_WRAP_END)
            LOG_ERROR " WRAP end! \n" LOG_END
        result = ERROR_SD_SEND_CMD_TIMEOUT;
        LOG_ERROR " cmd = %d (timeout=%d) \n", ctxt->cmd, ctxt->timeout LOG_END
        LOG_ERROR " need check: " LOG_END
        if (ctxt->flags & SD_FLAGS_IRQ_SD)
            printf("SD ");
        if (ctxt->flags & SD_FLAGS_DATA_DMA)
            printf("DMA ");
        if (ctxt->flags & SD_FLAGS_WRAP_INTR)
            printf("WRAP ");
        printf("\n");
        mmpDmaDumpReg(ctxt->dmaCtxt);
        goto end;
    }
    else
    {
#if defined(MM9910)
        if (ctxt->intr_flags & SD_FLAGS_IRQ_SD_ERROR)
        {
            if (ctxt->intrErr & SD_MSK_RESP_CRC)
            {
                LOG_ERROR " Command CRC Error!! command %d \n", ctxt->cmd LOG_END
                result = ERROR_SD_CRC_ERROR;
                goto end;
            }
            if (ctxt->intrErr & SD_MSK_RESP_TIMEOUT)
            {
                LOG_ERROR " Response timeout!! command %d \n", ctxt->cmd LOG_END
                result = ERROR_SD_CMD_RESP_TIMEOUT;
                goto end;
            }
            if (ctxt->intrErr & (SD_MSK_CRC_WRITE | SD_MSK_CRC_READ))
            {
                LOG_ERROR " Data CRC Error!! command %d \n", ctxt->cmd LOG_END
                if ((ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_W)) &&
                    (ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_R)))
                {
                    result = ERROR_SD_CRC_ERROR;
                    goto end;
                }
                if ((ctxt->cmd == MMC_ADTC_BUSTEST_W) || (ctxt->cmd == MMC_ADTC_BUSTEST_R))
                    LOG_ERROR " Bus Test CRC error!! %d \n", ctxt->cmd LOG_END
            }
        }
#else
        if (ctxt->intr_flags & SD_FLAGS_IRQ_SD_ERROR)
        {
            LOG_ERROR " CRC Error!! command %d \n", ctxt->cmd LOG_END
            if ((ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_W)) &&
                (ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_R)))
            {
                result = ERROR_SD_CRC_ERROR;
                goto end;
            }
            if ((ctxt->cmd == SD_CMD(MMC_ADTC_BUSTEST_W)) || (ctxt->cmd == SD_CMD(MMC_ADTC_BUSTEST_R)))
                LOG_ERROR " Bus Test CRC error!! %d \n", ctxt->cmd LOG_END
        }
#endif
        if (!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }
    }

    /** check warp ready!  NOTE: 9070 warp doesn't have interrupt !!*/
    if (ctxt->flags & SD_FLAGS_WRAP)
    {
        result = SDW_WaitWrapReadyReg(ctxt);
        if (result)
            goto end;
    }

end:
    /** for clock/command delay */
    if (ctxt->flags & SD_FLAGS_DATA_IN)
    {
        SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
        ctxt->flags &= ~SD_FLAGS_DATA_IN;
    }

    ctxt->flags &= ~SD_IRQ_FLAGS;
    ctxt->intr_flags &= ~SD_IRQ_FLAGS;

    return result;
}

#else  // #if defined(SD_IRQ_ENABLE)

MMP_INT SD_WaitSendCmdReadyReg(SD_CTXT *ctxt)
{
    MMP_INT     result = 0;
    SYS_CLOCK_T lastTime, sleepTime;
    MMP_UINT32  reg = 0;
    MMP_UINT32  error;

    /** check sd ready */
    sleepTime = lastTime = SYS_GetClock();
    do
    {
        if (SYS_GetDuration(lastTime) > ctxt->timeout)
        {
            result = ERROR_SD_SEND_CMD_TIMEOUT;
            goto end;
        }
        if (SYS_GetDuration(sleepTime) > 3)
        {
            sleepTime = SYS_GetClock();
            MMP_USleep(200);
        }
        if (!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }
        if (error = SD_IsErrorReg())
        {
#if defined(MM9910)
            if (error & SD_MSK_RESP_CRC)
            {
                LOG_ERROR " Command CRC Error!! command %d \n", ctxt->cmd LOG_END
                result = ERROR_SD_CRC_ERROR;
                goto end;
            }
            if (error & SD_MSK_RESP_TIMEOUT)
            {
                LOG_ERROR " Response timeout!! command %d \n", ctxt->cmd LOG_END
                result = ERROR_SD_CMD_RESP_TIMEOUT;
                goto end;
            }
            if (error & (SD_MSK_CRC_WRITE | SD_MSK_CRC_READ))
            {
                LOG_ERROR " Data CRC Error!! command %d \n", ctxt->cmd LOG_END
                if ((ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_W)) &&
                    (ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_R)))
                {
                    result = ERROR_SD_CRC_ERROR;
                    goto end;
                }
                if ((ctxt->cmd == MMC_ADTC_BUSTEST_W) || (ctxt->cmd == MMC_ADTC_BUSTEST_R))
                    LOG_ERROR " Bus Test CRC error!! %d \n", ctxt->cmd LOG_END
            }
#else
            if (error & (SD_MSK_CRC_ERROR))
            {
                LOG_ERROR " CRC Error!! command %d \n", ctxt->cmd LOG_END
                if ((ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_W)) &&
                    (ctxt->cmd != SD_CMD(MMC_ADTC_BUSTEST_R)))
                {
                    LOG_ERROR " CRC Error!! command %d \n", ctxt->cmd LOG_END
                        result = ERROR_SD_CRC_ERROR;
                    goto end;
                }
                if ((ctxt->cmd == SD_CMD(MMC_ADTC_BUSTEST_W)) || (ctxt->cmd == SD_CMD(MMC_ADTC_BUSTEST_R)))
                    LOG_ERROR " Bus Test CRC error!! %d \n", ctxt->cmd LOG_END
            }
#endif
        }
        AHB_ReadRegister(SD_REG_CTL, &reg);
    } while (reg & SD_MSK_CMD_TRIGGER);

    /** check dma ready */
    if (ctxt->flags & SD_FLAGS_DATA_DMA)
    {
        result = mmpDmaWaitIdle(ctxt->dmaCtxt);
        if (result)
            goto end;
    }

    /** check warp ready */
    if (ctxt->flags & SD_FLAGS_WRAP)
    {
        result = SDW_WaitWrapReadyReg(ctxt);
        if (result)
            goto end;
    }

end:
    /** for clock/command delay */
    if (ctxt->flags & SD_FLAGS_DATA_IN)
    {
        SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
        ctxt->flags &= ~SD_FLAGS_DATA_IN;
    }
    if (result)
        LOG_ERROR " SD_WaitSendCmdReadyReg() return 0x%08X, reg 0x%08X = 0x%08X \n", result, SD_REG_CTL, reg LOG_END

    return result;
}
#endif  // #if defined(SD_IRQ_ENABLE)

MMP_INT SDW_WaitWrapReadyReg(SD_CTXT *ctxt)
{
    MMP_INT     result = 0;
    SYS_CLOCK_T lastTime, sleepTime;
    MMP_UINT32  reg;
    MMP_INT     retries = 50;

    //<< for better performance
    do
    {
        AHB_ReadRegister(SDW_REG_WRAP_CTRL, &reg);
    } while ((reg & SDW_MSK_WRAP_FIRE) && --retries);

    if (retries)
        goto end;
    //>> for better performance

    /** for potential error */
    sleepTime = lastTime = SYS_GetClock();
    do
    {
        AHB_ReadRegister(SDW_REG_WRAP_CTRL, &reg);
        if (SYS_GetDuration(lastTime) > 300)
        {
            result = ERROR_SD_WRAP_TIMEOUT;
            goto end;
        }
        if (SYS_GetDuration(sleepTime) > 3)
        {
            sleepTime = SYS_GetClock();
            MMP_USleep(200);
        }
        if (!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }
    } while (reg & SDW_MSK_WRAP_FIRE);

end:
    ctxt->flags &= ~SD_FLAGS_WRAP;
    if (result)
        LOG_ERROR " SDW_WaitWrapReadyReg() return error code 0x%08X, reg 0x%08X = 0x%08X \n", result, SDW_REG_WRAP_CTRL, reg LOG_END

    return result;
}

//=============================================================================
/**
 * Read the SD card response value.
 * @param readByte  D[i]=1 means read the i byte
 */
//=============================================================================
void SD_ReadResponseReg(MMP_UINT16 readByte)
{
    MMP_UINT16 i = 0;
    MMP_UINT32 value = 0;

    SYS_MemorySet((void *)sdResp, 0x0, sizeof(sdResp));
    for (i = 0; i < 17; i++)
    {
#if 0
        if ((readByte >> i) & 0x1)
        {
            AHB_ReadRegister((SD_REG_RESP_7_0 + i * 4), &value);
            sdResp[i] = (MMP_UINT8)value;
        }
#else   // read all response value
        AHB_ReadRegister((SD_REG_RESP_7_0 + i * 4), &value);
        sdResp[i] = (MMP_UINT8)value;
#endif
    }
}

//=============================================================================
/**
 * Wait FIFO full.
 */
//=============================================================================
MMP_INT SD_WaitFifoFullReg(void)
{
    MMP_INT     result = 0;
    MMP_UINT32  status = 0;
    SYS_CLOCK_T lastTime = SYS_GetClock();

    do
    {
        AHB_ReadRegister(SDW_REG_WRAP_STATUS, &status);
        if (SYS_GetDuration(lastTime) > SD_WAIT_FIFO_TIMEOUT)
        {
            result = ERROR_SD_WAIT_FIFO_FULL_TIMEOUT;
            goto end;
        }
    } while (!(status & SDW_MSK_FIFO_FULL));

end:
    if (result)
        LOG_ERROR "SD_WaitFifoFullReg() return error code 0x%08X, reg 0x%08X = 0x%08X \n", result, SDW_REG_WRAP_STATUS, status LOG_END
    return result;
}

//=============================================================================
/**
 * Wait FIFO Empty.
 */
//=============================================================================
MMP_INT SD_WaitFifoEmptyReg(void)
{
    MMP_INT     result = 0;
    MMP_UINT32  status = 0;
    SYS_CLOCK_T lastTime = SYS_GetClock();

    do
    {
        AHB_ReadRegister(SDW_REG_WRAP_STATUS, &status);
        if (SYS_GetDuration(lastTime) > SD_WAIT_FIFO_TIMEOUT)
        {
            result = ERROR_SD_WAIT_FIFO_EMPTY_TIMEOUT;
            goto end;
        }
    } while (!(status & SDW_MSK_FIFO_EMPTY));

end:
    if (result)
        LOG_ERROR "SD_WaitFifoEmptyReg() return error code 0x%08X, reg 0x%08X = 0x%08X \n", result, SDW_REG_WRAP_STATUS, status LOG_END
    return result;
}

void SD_DumpReg(void)
{
    MMP_UINT32 i = 0;
    MMP_UINT32 value0 = 0;
    MMP_UINT32 value1 = 0;
    MMP_UINT32 value2 = 0;
    MMP_UINT32 value3 = 0;

    LOG_DATA " SD Register: \n" LOG_END
    for (i = SD_REG_STS0; i <= SDW_REG_WRAP_CTRL; i += 16)
    {
        value0 = 0;
        value1 = 0;
        value2 = 0;
        value3 = 0;
        if (i != SDW_REG_DATA_PORT)
            AHB_ReadRegister(i, &value0);
        AHB_ReadRegister(i + 0x4, &value1);
        AHB_ReadRegister(i + 0x8, &value2);
        AHB_ReadRegister(i + 0xc, &value3);
        LOG_DATA " reg 0x%08X   %08X %08X %08X %08X \n", i, value0, value1, value2, value3 LOG_END
    }
}