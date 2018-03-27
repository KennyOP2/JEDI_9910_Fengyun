/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  SD/MMC common function.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_dma.h"
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
//                              Global Data Definition
//=============================================================================
static MMP_UINT32 dmaReadAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_SD_TO_MEM,
   MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)SD_AHB_DATA_PORT,
    MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)SD_SECTOR_SIZE,
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, SD_FIFO_WIDTH,
    MMP_DMA_ATTRIB_DST_TX_WIDTH, 0,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 128,
    MMP_DMA_ATTRIB_NONE
};

static MMP_UINT32 dmaWriteAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_SD,
    MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)SD_AHB_DATA_PORT,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)SD_SECTOR_SIZE,
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_DST_TX_WIDTH, SD_FIFO_WIDTH,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 0,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 128,
    MMP_DMA_ATTRIB_NONE
};

extern MMP_UINT8 sdResp[17];


//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_INLINE void SD_DumpCID(SD_CTXT* ctxt)
{
    SD_ReadResponseReg(0xFFFF);
    memcpy((void*)ctxt->cid, (void*)sdResp, sizeof(ctxt->cid));
#if defined(SD_DUMP_CID)
    {
        MMP_UINT8 tmp8 = 0;
        MMP_UINT16 tmp16 = 0;

        LOG_DATA " CID Register: \n" LOG_END
        LOG_DATA " D[7:1]     CRC7 checksum : 0x%02X \n", (sdResp[0]>>1) LOG_END
        if(ctxt->flags & SD_FLAGS_CARD_SD)
        {
            tmp8  = sdResp[1] & 0x0F;
            tmp16 = (((sdResp[2]&0xF) << 4) | ((sdResp[1]&0xF0) >> 4)) + 2000;
            LOG_DATA " D[19:8]    Manufacturing date   : %d-%d \n", tmp16, tmp8 LOG_END
            LOG_DATA " D[55:24]   Product serial number: %02X%02X%02X%02X \n", sdResp[6], sdResp[5], sdResp[4], sdResp[3] LOG_END
            LOG_DATA " D[63:56]   Product revision     : %01X.%01X \n", ((sdResp[7]&0xF0)>>4), (sdResp[7]&0x0F) LOG_END
            LOG_DATA " D[103:64]  Product name         : %c%c%c%c%c \n", sdResp[12], sdResp[11], sdResp[10], sdResp[9], sdResp[8] LOG_END
        }
        else
        {
            LOG_DATA " D[15:8]    Manufacturing date   : %d-%d \n", (1997+(sdResp[1]&0xF)), ((sdResp[1]&0xF0)>>4) LOG_END
            LOG_DATA " D[55:24]   Product serial number: %02X%02X%02X%02X \n", sdResp[5], sdResp[4], sdResp[3], sdResp[2] LOG_END
            LOG_DATA " D[63:56]   Product revision     : %01X.%01X \n", ((sdResp[6]&0xF0)>>4), (sdResp[6]&0x0F) LOG_END
            LOG_DATA " D[103:64]  Product name         : %c%c%c%c%c%c \n", sdResp[12], sdResp[11], sdResp[10], sdResp[9], sdResp[8], sdResp[7] LOG_END
        }
        LOG_DATA " D[119:104] OEM/Application ID   : %c%c \n", sdResp[14], sdResp[13] LOG_END
        LOG_DATA " D[127:120] Manufacturer ID      : %02X \n", sdResp[15] LOG_END
    }
#endif
}



//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * SD/MMC start up function.
 *
 * Step 1: Go into Ready State
 * Step 2: Go into Identification State. 
 * Step 3: Go into Stand-by State. 
 */
//=============================================================================
MMP_INT SDMMC_StartUp(SD_CTXT* ctxt)
{
    MMP_INT     result = 0;
    MMP_UINT8   retry = 3;
    SYS_CLOCK_T lastTime, sleepTime;

    ctxt->cardBusWidth = 1;

    //============================
    // CMD 0: Go Idle State
    //============================
    #if 1//!defined(SD_HW_POWER_RESET) /** FPGA need this command to replace power reset for card reset */
    result = SD_SendCmdReg(ctxt, COM_BC_GO_IDLE_STATE, 0, 0, 0, 0, (SD_RESP_TYPE_NON|SD_CMD_NO_DATA));
    if(result)
    {
        result = ERROR_SD_GO_IDLE_STATE_FAIL;
        goto end;
    }
    #endif

    //============================
    // Go into Ready State. 
    //
    // MMC: Send CMD1
    // SD : Send ACMD41
    //============================
    while(1)
    {
        if(SD_SendCmdReg(ctxt, SD_AC_APP_CMD, 0, 0, 0, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA)))
        {
            retry--;
            if(retry)
                continue;

            LOG_INFO " MMC Start Up! \n" LOG_END
            result = MMC_StartUp(ctxt);
            if(result)
                goto end;
            break;
        }
        else
        {
            #if 1
            result = SD_SendCmdReg(ctxt, COM_BC_GO_IDLE_STATE, 0, 0, 0, 0, (SD_RESP_TYPE_NON|SD_CMD_NO_DATA));
            if(result)
            {
                result = ERROR_SD_GO_IDLE_STATE_FAIL;
                goto end;
            }
            #endif
            LOG_INFO " SD Start Up! \n" LOG_END
            result = SD_StartUp(ctxt);
            if(result)
                goto end;
            break;
        }
    }

    sleepTime = lastTime = SYS_GetClock();
    do
    {
        //============================
        // Go into Identification State. 
        // Send CMD2 to get CID.
        //============================
        if(SYS_GetDuration(lastTime) > 1000) /** MMC may send multiple times */
        {
            result = ERROR_MMC_SEND_CMD2_TIMEOUT;
            goto end;
        }
        if(SYS_GetDuration(sleepTime) > 15)
        {
            sleepTime = SYS_GetClock();
            MMP_Sleep(0);
        }
        if(!SD_IsCardInserted(ctxt->index))
        {
            result = ERROR_SD_NO_CARD_INSERTED;
            goto end;
        }
        if(SD_SendCmdReg(ctxt, COM_BCR_ALL_SEND_CID, 0, 0, 0, 0, (SD_RESP_TYPE_136|SD_CMD_NO_DATA)))
            continue;
        SD_DumpCID(ctxt);

        //============================
        // Go into Stand-by State. 
        // Send CMD3 to get/set RCA.
        //============================
        if(ctxt->flags & SD_FLAGS_CARD_SD)
        {
            result = SD_GetRca(ctxt);
            if(result)
                goto end;
        }
        else if(ctxt->flags & SD_FLAGS_CARD_MMC)
        {
            result = MMC_SetRca(ctxt);
            if(result)
                goto end;
        }
        break;
    } while(1);

end:
    if(result)
        LOG_ERROR " SDMMC_StartUp() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * SD/MMC Get Capacity function.
 *
 * Send CMD9 to get CSD register.
 */
//=============================================================================
MMP_INT SDMMC_GetCapacity(SD_CTXT* ctxt)
{
    MMP_INT result = 0;
    MMP_UINT8 retry = 3;

    while(SD_SendCmdReg(ctxt, COM_AC_SEND_CSD, ctxt->rcaByte3, ctxt->rcaByte2, 0, 0, (SD_RESP_TYPE_136|SD_CMD_NO_DATA)))
    {
        retry--;
        if(!retry)
        {
            result = ERROR_SD_SEND_CSD_FAIL;
            goto end;
        }
    }
    SD_ReadResponseReg(0xFFFF);
#if defined(SD_DUMP_CSD)
    {
        MMP_UINT i;
        LOG_DATA " CSD:" LOG_END
        for(i=0; i<16; i++)
            LOG_DATA " %02X", sdResp[i] LOG_END
        LOG_DATA " \n" LOG_END
    }
#endif

    if( (ctxt->flags & SD_FLAGS_CARD_SD) && /** SD card */
        (sdResp[15] & 0xC0)                && /** CSD V2.0 */
        ((sdResp[8] & 0x2F)==0) )            /** The max. capacity of the Physical Layer Spec. V2.0 is 32GB. */
    {
        MMP_UINT32 c_size = 0;
        c_size = (sdResp[6]        |
                  (sdResp[7] << 8) |
                  ((sdResp[8]&0x2F) << 16));
        ctxt->totalSectors = (c_size+1)*1024;
        LOG_INFO " SD with CSD V2.0 \n" LOG_END
        LOG_INFO " TAAC = 0x%02X,  NSAC = 0x%02X, TRAN_SPEED = 0x%02X \n", sdResp[15], sdResp[14], sdResp[13] LOG_END
    }
    else /** CSD V1.0 or MMC */
    {
        MMP_UINT16 c_size=0, c_size_mult=0, mult=0, block_len=0, i;

        c_size = (MMP_UINT16)(((sdResp[7] & 0xC0) >> 6) |
                              (sdResp[8] << 2) |
                              ((sdResp[9] & 0x03) << 10));
        c_size_mult = (MMP_UINT16)(((sdResp[5] & 0x80) >> 7) |
                                   ((sdResp[6] & 0x03) << 1));

        mult = 1;
        for(i=0; i<(c_size_mult+2); i++)
            mult <<= 1;

        block_len = 1;
        for(i=0; i<(sdResp[10]&0x0F); i++)
            block_len <<= 1;
        block_len /= SD_SECTOR_SIZE;  /** secotr unit: 512 bytes */

        ctxt->totalSectors = (MMP_UINT32)((c_size+1) * mult * block_len);
        LOG_INFO " CSD V1.0 or MMC \n" LOG_END
        LOG_INFO " TAAC = 0x%02X,  NSAC = 0x%02X, TRAN_SPEED = 0x%02X \n", sdResp[14], sdResp[13], sdResp[12] LOG_END
    }
    LOG_INFO " Total secotor number = 0x%08X \n", ctxt->totalSectors LOG_END

    if( (ctxt->flags & SD_FLAGS_CARD_MMC) &&  /** MMC card */
        ((sdResp[15] & 0x3C) >= 0x10) )         /** >= MMC V4.0 */
    {
        ctxt->flags |= SD_FLAGS_CARD_MMC4;
    }

end:
    if(result)
        LOG_ERROR " SDMMC_GetCapacity() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * Put SD/MMC card into Transfer State.
 *
 * Send CMD7
 */
//=============================================================================
MMP_INT SDMMC_TransferState(SD_CTXT* ctxt)
{
    MMP_INT result = 0;

    result = SD_SendCmdReg(ctxt, COM_AC_SELECT_DESELECT_CARD, ctxt->rcaByte3, ctxt->rcaByte2, 0, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA));
    if(result)
    {
        result = ERROR_SD_SEND_CMD7_FAIL;
        goto end;
    }
    SD_ReadResponseReg(0x4);
    if( ((sdResp[2] & 0x1E) >> 1) != 3 )
    {
        result = ERROR_SD_PUT_INTO_TRANSFER_STATE_FAIL;
        goto end;
    }

end:
    if(result)
        LOG_ERROR " SDMMC_TransferState() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * Set SD/MMC bus width.
 */
//=============================================================================
MMP_INT SDMMC_Switch(SD_CTXT* ctxt)
{
    MMP_INT result = 0;

    if(ctxt->flags & SD_FLAGS_CARD_MMC)
    {
        result = MMC_Switch(ctxt);
        if(result)
            goto end;
    }
    else
    {
        result = SD_Switch(ctxt);
        if(result)
            goto end;
    }

    /** Set sector size */
    result = SD_SendCmdReg(ctxt, COM_AC_SET_BLOCK_LEN, 0, 0, 0x02, 0, (SD_RESP_TYPE_48|SD_CMD_NO_DATA));
    if(result)
        goto end;
    SD_SetSectorLengthReg(SD_SECTOR_SIZE);

end:
    if(result)
        LOG_ERROR " SDMMC_Switch() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * Programing DMA engine for SD read.
 *
 * @param data   the destination system memory address
 * @param size   the total transfer size
 */
//=============================================================================
MMP_INT SDMMC_DmaRead(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 size)
{
    MMP_INT result = 0;

    if( (ctxt->dmaType == SD_DMA_HW_HANDSHAKING) && ctxt->dmaCtxt )
    {
        dmaReadAttrib[5] = (MMP_UINT32)data; /** destination address */
        dmaReadAttrib[7] = size;             /** total size */
        result = mmpDmaSetAttrib(ctxt->dmaCtxt, dmaReadAttrib);
        if(result)
            goto end;
        result = mmpDmaFire(ctxt->dmaCtxt);
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR "SDMMC_DmaRead() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=============================================================================
/**
 * Programing DMA engine for SD write.
 *
 * @param data  the source system memory address
 * @param size  the total transfer size
 */
//=============================================================================
MMP_INT SDMMC_DmaWrite(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 size)
{
    MMP_INT result = 0;

    if( (ctxt->dmaType == SD_DMA_HW_HANDSHAKING) && ctxt->dmaCtxt )
    {
        dmaWriteAttrib[3] = (MMP_UINT32)data;
        dmaWriteAttrib[7] = size;             /** total size */
        result = mmpDmaSetAttrib(ctxt->dmaCtxt, dmaWriteAttrib);
        if(result)
            goto end;
        result = mmpDmaFire(ctxt->dmaCtxt);
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR "SDMMC_DmaWrite() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=============================================================================
/**
 * Read data from card. From host controller's register by CPU. (without DMA)
 *
 * @param data  the destination system memory address
 * @param size  the total size to be read
 */
//=============================================================================
MMP_INT SDMMC_ReadData(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 totalSize)
{
    MMP_INT result = 0;
    MMP_UINT32 i,j;
    MMP_UINT32 tmpData = 0;
    MMP_UINT32 fifoSizeInbyte = SD_FIFO_DEPTH * SD_FIFO_WIDTH;

    if(totalSize % fifoSizeInbyte)
        LOG_WARNING " FIFO size not match! \n" LOG_END

    for(i=0; i<(totalSize/fifoSizeInbyte); i++)
    {
        result = SD_WaitFifoFullReg();
        if(result)
            goto end;
        
        for(j=0; j<SD_FIFO_DEPTH; j++, data+=4)
        {
        	  tmpData = SD_ReadDataReg();
            *(MMP_UINT32*)data = cpu_to_le32(tmpData);
        }
    }
    result = SD_WaitSendCmdReadyReg(ctxt);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR " SDMMC_ReadData() return error code 0x%08X, i = %d \n", result, i LOG_END
    return result;
}

//=============================================================================
/**
 * Write data to card. Write to host controller's register by CPU. (without DMA)
 *
 * @param data  the source system memory address
 * @param size  the total size to be write
 */
//=============================================================================
MMP_INT SDMMC_WriteData(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 totalSize)
{
    MMP_INT result = 0;
    MMP_UINT32 i,j;
    MMP_UINT32 tmpData = 0;
    MMP_UINT32 fifoSizeInbyte = SD_FIFO_DEPTH * SD_FIFO_WIDTH;
  
    if(totalSize % fifoSizeInbyte)
        LOG_WARNING " FIFO size not match! \n" LOG_END

    for(i=0; i<(totalSize/fifoSizeInbyte); i++)
    {
        result = SD_WaitFifoEmptyReg();
        if(result)
            goto end;
        
        for(j=0; j<SD_FIFO_DEPTH; j++, data+=4)
        {
            tmpData = (*(data+3) << 24 |
                       *(data+2) << 16 |
                       *(data+1) << 8  |
                       *(data));
            SD_WriteDataReg(tmpData);
        }
    }
    result = SD_WaitSendCmdReadyReg(ctxt);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR " SDMMC_WriteData() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * SD/MMC read multi-sector process.
 *
 * @param sector    start sector to be read
 * @param count     total sectors to be read
 * @param data      the buffer to receive data
 */
//=============================================================================
MMP_INT
SDMMC_ReadMultiSector(SD_CTXT* ctxt, MMP_UINT32 sector, MMP_UINT32 count, void* data)
{
    MMP_INT    result = 0;
    //MMP_UINT8  command = (count == 1) ? COM_ADTC_READ_SINGLE_BLOCK : COM_ADTC_READ_MULTIPLE_BLOCK;
    MMP_UINT8  command = COM_ADTC_READ_MULTIPLE_BLOCK;

    /** Data address is in byte units in a Standard Capacity SD Memory Card. */
    if(!(ctxt->flags & SD_FLAGS_CARD_SDHC) && !(ctxt->flags & SD_FLAGS_CARD_MMC_HC))
    {
        sector *= SD_SECTOR_SIZE; 
    }

    SD_SetSectorCountReg(count);
    if((ctxt->dmaType == SD_DMA_HW_HANDSHAKING) && ctxt->dmaCtxt)
    {
        result = SDMMC_DmaRead(ctxt, data, SD_SECTOR_SIZE*count);
        if(result)
            goto end;
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_DMA); /** set smedia wrap control */
        result = SD_SendCmdReg( ctxt, command, 
                                (MMP_UINT8)((sector & 0xFF000000) >> 24), 
                                (MMP_UINT8)((sector & 0x00FF0000) >> 16), 
                                (MMP_UINT8)((sector & 0x0000FF00) >> 8), 
                                (MMP_UINT8)((sector & 0x000000FF)), 
                                (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP));
        if(result)
            goto end;
        #if defined (__FREERTOS__)
        ithInvalidateDCacheRange(data, SD_SECTOR_SIZE*count);
        #endif
    }
    else
    {
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_IN, SDW_NON_DMA); /** set smedia wrap control */
        SD_SendCmdNoWaitReg( ctxt, command,
                            (MMP_UINT8)((sector & 0xFF000000) >> 24), 
                            (MMP_UINT8)((sector & 0x00FF0000) >> 16), 
                            (MMP_UINT8)((sector & 0x0000FF00) >> 8), 
                            (MMP_UINT8)((sector & 0x000000FF)), 
                            (SD_RESP_TYPE_48|SD_CMD_DATA_IN|SD_MSK_AUTO_SWAP));

        result = SDMMC_ReadData(ctxt, data, SD_SECTOR_SIZE*count);
        if(result)
            goto end;
    }

    if(command == COM_ADTC_READ_MULTIPLE_BLOCK)
    {
        result = SD_SendCmdReg(ctxt, COM_AC_STOP_TRANSMISSION, 0, 0, 0, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA));
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR " SDMMC_ReadMultiSector() return error code 0x%08X \n", result LOG_END
    return result;
}

//=============================================================================
/**
 * SD/MMC write multi-sector process.
 *
 * @param sector    start sector to be write
 * @param count     total sectors to be write
 * @param data      the buffer contain the data to be written
 */
//=============================================================================
MMP_INT
SDMMC_WriteMultiSector(SD_CTXT* ctxt, MMP_UINT32 sector, MMP_UINT32 count, void* data)
{
    MMP_INT    result = 0;
    //MMP_UINT8  command = (count==1) ? COM_ADTC_WRITE_SINGLE_BLOCK : COM_ADTC_WRITE_MULTIPLE_BLOCK;
    MMP_UINT8 command = COM_ADTC_WRITE_MULTIPLE_BLOCK;

    if(!(ctxt->flags & SD_FLAGS_CARD_SDHC) & !(ctxt->flags & SD_FLAGS_CARD_MMC_HC))
    {
        sector *= SD_SECTOR_SIZE; 
    }

    SD_SetSectorCountReg(count);
    if((ctxt->dmaType == SD_DMA_HW_HANDSHAKING) && ctxt->dmaCtxt)
    {
        result = SDMMC_DmaWrite(ctxt, data, SD_SECTOR_SIZE*count);
        if(result)
            goto end;
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_OUT, SDW_DMA); /** set smedia wrap control */
        result = SD_SendCmdReg( ctxt, command, 
                                (MMP_UINT8)((sector & 0xFF000000) >> 24), 
                                (MMP_UINT8)((sector & 0x00FF0000) >> 16), 
                                (MMP_UINT8)((sector & 0x0000FF00) >> 8), 
                                (MMP_UINT8)((sector & 0x000000FF)), 
                                (SD_RESP_TYPE_48|SD_CMD_DATA_OUT|SD_MSK_AUTO_SWAP));
        if(result)
            goto end;
    }
    else
    {
        SDW_SetWrapCtrlReg(ctxt, SDW_DATA_OUT, SDW_NON_DMA);  /** set smedia wrap control */
        SD_SendCmdNoWaitReg(ctxt, command, 
                            (MMP_UINT8)((sector & 0xFF000000) >> 24), 
                            (MMP_UINT8)((sector & 0x00FF0000) >> 16), 
                            (MMP_UINT8)((sector & 0x0000FF00) >> 8), 
                            (MMP_UINT8)((sector & 0x000000FF)), 
                            (SD_RESP_TYPE_48|SD_CMD_DATA_OUT|SD_MSK_AUTO_SWAP));

        result = SDMMC_WriteData(ctxt, data, SD_SECTOR_SIZE*count);
        if(result)
            goto end;
    }

    if(command == COM_ADTC_WRITE_MULTIPLE_BLOCK)
    {
        result = SD_SendCmdReg(ctxt, COM_AC_STOP_TRANSMISSION, 0, 0, 0, 0, (SD_RESP_TYPE_48_BUSY|SD_CMD_NO_DATA));
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR " SDMMC_WriteMultiSector() return error code 0x%08X \n", result LOG_END
    return result;
}



