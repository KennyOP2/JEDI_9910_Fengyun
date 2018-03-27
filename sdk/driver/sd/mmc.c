/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  MMC related function.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "sd/config.h"
#include "sd/sd_hw.h"
#include "sd/sd.h"
#if defined (__OPENRTOS__)
#include "ite/ith.h"
#endif
#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
extern MMP_UINT8 sdResp[17];


//=============================================================================
//                              Private Function Definition
//=============================================================================
//=============================================================================
/**
 * MMC4 switch.
 *
 * Send CMD8: send EXT_CSD
 *            From EXT_CSD the host gets the power and speed class of the card.
 *
 * Send CMD6: Switch the mode of operation of the selected card or modifies the EXT_CSD register.
 *
 * Send CMD19: host sends a data pattern with CMD19, card sends a response and latches received 
 *             data pattern internally.
 * Send CMD14: host sends CMD14, the card would sned the reverse pattern of received data.
 */
//=============================================================================
static MMP_INLINE MMP_INT
MMC4_Switch(SD_CTXT* ctxt)
{
    MMP_INT result = 0;
    MMP_UINT8 extCSD[SD_SECTOR_SIZE] = {0};
    MMP_UINT8* pattern = extCSD;

    //==============================
    // Send CMD8: send EXT_CSD
    //            From EXT_CSD the host gets the power and speed class of the card.
    //==============================
    SD_SetSectorLengthReg(SD_SECTOR_SIZE);
    SD_SetSectorCountReg(1);
#if defined(MMC_WR_TIMING) || defined(SD_WIN32_DMA)
    if(0)
#else
    if((ctxt->dmaType == SD_DMA_HW_HANDSHAKING) && ctxt->dmaCtxt)
#endif
    {
        result = SDMMC_DmaRead(ctxt, extCSD, SD_SECTOR_SIZE);
        if(result)
            goto end;
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_DMA); /** set smedia wrap control */
        result = SD_SendCmdReg(ctxt, MMC_ADTC_SEND_EXT_CSD, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)); // Irene: Need swap??
        if(result)
            goto end;
        #if defined (__FREERTOS__)
        ithInvalidateDCacheRange(extCSD, SD_SECTOR_SIZE);
        #endif
    }
    else
    {
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        SD_SendCmdNoWaitReg(ctxt, MMC_ADTC_SEND_EXT_CSD, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)); // Irene: Need swap??

        result = SDMMC_ReadData(ctxt, extCSD, SD_SECTOR_SIZE);
        if(result)
            goto end;
    }

    if((extCSD[196] & 0x03) == 0x03)
    {
        if(!(ctxt->flags & SD_FLAGS_CARD_MMC4))
            LOG_WARNING " It is NOT MMC v4.0 but it is MMC4 card type!! \n" LOG_END
        ctxt->flags |= SD_FLAGS_CARD_MMC4;
    }
    else
    {
        if(ctxt->flags & SD_FLAGS_CARD_MMC4)
            LOG_WARNING " It is MMC v4.0 but NOT MMC4 card type!! \n" LOG_END
        ctxt->flags &= ~SD_FLAGS_CARD_MMC4;
    }
    LOG_INFO " MMC4 card type 0x%02X \n", extCSD[196] LOG_END

    #if defined(SD_DUMP_EXT_CSD)
    {
        MMP_UINT i = 0;
        LOG_DATA " EXT_CSD content: (H->L) \n" LOG_END
        for(i=0; i<512; i+=16)
        {
            LOG_DATA " Byte %02d: %02X %02X %02X %02X %02X %02X %02X %02X -  %02X %02X %02X %02X %02X %02X %02X %02X \n",
                    i, extCSD[i+0], extCSD[i+1], extCSD[i+2], extCSD[i+3], extCSD[i+4], extCSD[i+5], extCSD[i+6], extCSD[i+7]
                     , extCSD[i+8], extCSD[i+9], extCSD[i+10], extCSD[i+11], extCSD[i+12], extCSD[i+13], extCSD[i+14], extCSD[i+15] LOG_END
        }
    }
    #endif

    ctxt->clockDiv = SD_CLK_NORMAL; 
    SD_SetClockDivReg(ctxt->clockDiv);
    SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
    if(ctxt->flags & SD_FLAGS_CARD_MMC_HC)
    {
        ctxt->totalSectors = (extCSD[212] |
                               (extCSD[213]<<8) |
                               (extCSD[214]<<16) |
                               (extCSD[215]<<24) );
        LOG_INFO " MMC HC new total sector number = 0x%08X \n", ctxt->totalSectors LOG_END
        if(!ctxt->totalSectors)
        {
            result = ERROR_MMC_HC_TOTAL_SECTORS_ERROR;
            goto end;
        }
    }

    //==============================
    // MMC4 card switch to High speed
    // Send CMD6: Switch the mode of operation of the selected card or modifies the EXT_CSD register.
    //==============================
    if(ctxt->flags & SD_FLAGS_CARD_MMC4)
    {
        /** arg 0x03B90100 */
        //if(SD_SendCmdReg(MMC_AC_SWITCH, ACCESS_WRITE_BYTE, HS_TIMING_INDEX, MMC_SPEED, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
        if(SD_SendCmdReg(ctxt, MMC_AC_SWITCH, ACCESS_WRITE_BYTE, HS_TIMING_INDEX, MMC_SPEED, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA)))
        {
            ctxt->flags &= ~SD_FLAGS_CARD_MMC4;
            LOG_WARNING " MMC4 change high speed fail! => disable MMC4 \n" LOG_END
        }
        else
        {
            ctxt->flags |= SD_FLAGS_SD_HIGH_SPEED;
            //LOG_INFO2 " Run with High Speed! \n" LOG_END
        }
    }
    if(ctxt->flags & SD_FLAGS_CARD_MMC4)
    {
        ctxt->clockDiv = SD_CLK_HIGH_SPEED;
        SD_SetHighSpeedReg();
        LOG_INFO2 " Run with High Speed! \n" LOG_END
    }
    else
    {
        ctxt->clockDiv = SD_CLK_NORMAL;
        LOG_INFO2 " Run with Default Mode! \n" LOG_END
    }
    SD_SetClockDivReg(ctxt->clockDiv);

    //==============================
    // MMC4 card Change bus width procedure
    // Send CMD19: test write
    // Send CMD14: test read
    //==============================
    /** set host as 8-bits */
    ctxt->cardBusWidth = 8;
    SD_SetBusWidthReg(ctxt->cardBusWidth);
    SD_CrcBypassReg();
	#if defined(SD_READ_FLIP_FLOP)
    SD_CrcNonFixReg(MMP_FALSE);
	#endif

    /** test write: CMD19, test read: CMD14 */
    pattern[0] = 0x55;
    pattern[1] = 0xAA;
    SD_SetSectorLengthReg(SD_SECTOR_SIZE);
    SD_SetSectorCountReg(1);
#if defined (__FREERTOS__)
    if((ctxt->dmaType == SD_DMA_HW_HANDSHAKING) && ctxt->dmaCtxt)
    {
        /** test write: CMD19 */
        result = SDMMC_DmaWrite(ctxt, pattern, SD_SECTOR_SIZE);
        if(result)
            goto end;
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_OUT, SDW_DMA);  /** set smedia wrap control */
        result = SD_SendCmdReg(ctxt, MMC_ADTC_BUSTEST_W, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_OUT|SD_MSK_AUTO_SWAP)); // Irene: Need swap??
        if(result)
            goto end;
        /** test read: CMD14 */
        pattern[0] = 0x0;
        pattern[1] = 0x0;
        result = SDMMC_DmaRead(ctxt, pattern, SD_SECTOR_SIZE);
        if(result)
            goto end;
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_DMA); /** set smedia wrap control */
        result = SD_SendCmdReg(ctxt, MMC_ADTC_BUSTEST_R, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)); // Irene: Need swap??
        if(result)
            goto end;
        #if defined (__FREERTOS__)
        ithInvalidateDCacheRange(pattern, SD_SECTOR_SIZE);
        #endif
    }
    else
#endif
    {
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_OUT, SDW_NON_DMA);  /** set smedia wrap control */
        /** test write: CMD19 */
        SD_SendCmdNoWaitReg(ctxt, MMC_ADTC_BUSTEST_W, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_OUT|SD_MSK_AUTO_SWAP)); // Irene: Need swap??

        result = SDMMC_WriteData(ctxt, pattern, SD_SECTOR_SIZE);
        if(result)
            goto end;
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        /** test read: CMD14 */
        SD_SendCmdNoWaitReg(ctxt, MMC_ADTC_BUSTEST_R, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP)); // Irene: Need swap??

        result = SDMMC_ReadData(ctxt, pattern, SD_SECTOR_SIZE);
        if(result)
            goto end;
    }
    #if defined(SD_READ_FLIP_FLOP)
    SD_CrcNonFixReg(MMP_TRUE);
    #endif

    //==============================
    // Determines the bus line connection or the number of the card I/O through CMD6
    //==============================
    ctxt->cardBusWidth = 1;
    SD_SetBusWidthReg(ctxt->cardBusWidth);
    if((pattern[0]==0xAA) && (pattern[1]==0x55))
    {
        if(SD_SendCmdReg(ctxt, MMC_AC_SWITCH, ACCESS_WRITE_BYTE, BUS_WIDTH_INDEX, MMC_BUS_WIDTH_8, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA)) == 0)
        {
            ctxt->cardBusWidth = 8;
            LOG_INFO2 " This MMC card run with 8-bits! \n" LOG_END
        }
        else
            LOG_WARNING " MMC 8-bits bus, but set 8 bits bus fail! \n" LOG_END

        ctxt->flags |= SD_FLAGS_MMC_8_BIT_BUS;
    }
    else if(((pattern[0]&0x0F)==0x0A) && ((pattern[1]&0x0F)==0x05))
    {
        if(SD_SendCmdReg(ctxt, MMC_AC_SWITCH, ACCESS_WRITE_BYTE, BUS_WIDTH_INDEX, MMC_BUS_WIDTH_4, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA)) == 0)
        {
            ctxt->cardBusWidth = 4;
            LOG_INFO2 " This MMC card run with 4-bits! \n" LOG_END
            LOG_INFO2 " pattern[0] = %X, pattern[1] = %X\n", pattern[0], pattern[1] LOG_END
        }
        else
            LOG_WARNING " MMC 4-bits bus, but set 4 bits bus fail! \n" LOG_END

        ctxt->flags |= SD_FLAGS_MMC_4_BIT_BUS;
    }
    else
    {
        LOG_ERROR " pattern[0] = %X, pattern[1] = %X\n", pattern[0], pattern[1] LOG_END
        result = ERROR_MMC_TEST_BUS_FAIL;
        goto end;
    }
    SD_SetBusWidthReg(ctxt->cardBusWidth);

    //SD_ResetIFReg();

end:
    if(result)
    {
        //SD_SetBusWidthReg(1);
        LOG_ERROR " MMC4_Switch() return error code 0x%08X \n", result LOG_END
    }
    return result;
}


//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * MMC start up function.
 *
 * Send CMD1: asks all cards in idle state to send their operation conditions regsiter contents
 *            in the response on the CMD line.
 */
//=============================================================================
MMP_INT
MMC_StartUp(SD_CTXT* ctxt)
{
    MMP_INT result = 0;
    SYS_CLOCK_T lastTime, sleepTime;

    sleepTime = lastTime = SYS_GetClock();
    do
    {
        if(SYS_GetDuration(lastTime) > 600)
        {
            result = ERROR_MMC_SEND_CMD1_TIMEOUT;
            goto end;
        }
        if(SYS_GetDuration(sleepTime) > 3)
        {
            sleepTime = SYS_GetClock();
            MMP_USleep(200);
        }
        if(!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }

        if(SD_SendCmdReg(ctxt, MMC_BCR_SEND_OP_COND, MMC_HOST_HC, 0xFF, 0x80, 0x00, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
            continue;

        SD_ReadResponseReg(0x1E);
        LOG_INFO " OCR register: 0x%02X%02X%02X%02X \n", sdResp[4], sdResp[3], sdResp[2], sdResp[1] LOG_END
    } while(!(sdResp[4] & 0x80)); /** HOST repeatedly issues CMD1 until the busy bit is set to 1. */

    if((sdResp[4] & 0x60) == MMC_HOST_HC)
    {
        ctxt->flags |= SD_FLAGS_CARD_MMC_HC;
        LOG_INFO " MMC => SD_FLAGS_CARD_MMC_HC \n" LOG_END
    }

    ctxt->flags |= SD_FLAGS_CARD_MMC;
    LOG_INFO " This card is MMC! \n" LOG_END

end:
    if(result)
        LOG_ERROR " MMC_StartUp() return error code 0x%08X\n", result LOG_END
    return result;
}

//=============================================================================
/**
 * MMC card go into Stand-by State.
 *
 * Send CMD3: Assigns relative address to the card.
 */
//=============================================================================
MMP_INT
MMC_SetRca(SD_CTXT* ctxt)
{
    MMP_INT result = 0;

    do
    {
        ctxt->rcaByte2++;
        if(ctxt->rcaByte2 > 10)
        {
            result = ERROR_MMC_SET_RCA_MORE_THAN_10;
            goto end;
        }
        if(SD_SendCmdReg(ctxt, MMC_BCR_SET_RELATIVE_ADDR, 0, ctxt->rcaByte2, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
            continue;

        SD_ReadResponseReg(0x1E);
        LOG_DEBUG " MMC Set RCA Status = 0x%08X \n", sdResp[4], sdResp[3], sdResp[2], sdResp[1] LOG_END
        break;
    } while(1);

end:
    if(result)
        LOG_ERROR " MMC_SetRca() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * MMC set bus width.
 *
 */
//=============================================================================
MMP_INT
MMC_Switch(SD_CTXT* ctxt)
{
    MMP_INT result = 0;

    if(ctxt->flags & SD_FLAGS_CARD_MMC4)
    {
        result = MMC4_Switch(ctxt);
        if(result)
            goto end;
    }
    if(!(ctxt->flags & SD_FLAGS_CARD_MMC4))
    {
        ctxt->clockDiv = SD_CLK_NORMAL;
        SD_SetClockDivReg(ctxt->clockDiv);
        SDMMC_WriteDelay(ctxt->flags & SD_FLAGS_SD_HIGH_SPEED);
    }

end:
    if(result)
        LOG_ERROR " MMC_Switch() return error code 0x%08X \n", result LOG_END
    return result;
}

