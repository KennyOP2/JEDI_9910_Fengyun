/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  SD/MMC extern API implementation.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_sd.h"
#include "sd/config.h"
#include "sd/sd_hw.h"
#include "sd/sd.h"
#if defined(SD_WIN32_DMA)
    #include "mem/mem.h"
#endif
#include "host/gpio.h"

#if defined(SD_IRQ_ENABLE)
    #if defined(__OPENRTOS__)
        #include "ite/ith.h"
    #elif defined(__FREERTOS__)
        #include "intr/intr.h"
    #endif
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
enum
{
    MODE_RW,
    MODE_NON_RW
};

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static MMP_INT SD_ContextInitialize(SD_CTXT *ctxt);

//=============================================================================
//                              Global Data Definition
//=============================================================================
static void       *SD_Semaphore    = MMP_NULL;
SD_CTXT           g_sdCtxt[SD_NUM] = {0};

#if defined(_WIN32)
static MMP_UINT32 gpio_2sd_switch = 0;
#else
//extern MMP_UINT32 gpio_2sd_switch;
#endif

//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_INT SD_ContextInitialize(SD_CTXT *ctxt)
{
    MMP_INT result = 0;
    LOG_ENTER "[SD_ContextInitialize] Enter \n" LOG_END

    memset((void *)ctxt, 0x0, sizeof(SD_CTXT));

#if defined (__FREERTOS__) || defined(SD_WIN32_DMA)
    ctxt->flags  |= SD_FLAGS_DMA_ENABLE;
    //ctxt->dmaType = SD_DMA_NORMAL;
    ctxt->dmaType = SD_DMA_HW_HANDSHAKING;
    LOG_INFO " Run with DMA! \n" LOG_END
#endif

    if (ctxt->flags & SD_FLAGS_DMA_ENABLE)
    {
        result = mmpDmaCreateContext(&ctxt->dmaCtxt);
        if (result)
            goto end;
    }

end:
    LOG_LEAVE "[SD_ContextInitialize] Leave \n" LOG_END
    if (result)
        LOG_ERROR "SD_ContextInitialize() return error code 0x%08X \n", result LOG_END

        return result;
}

static MMP_INLINE void SD_SwitchLun(MMP_INT index, MMP_INT mode)
{
    SD_CTXT *ctxt = &g_sdCtxt[index];

    if ((mode == MODE_RW) && !(ctxt->flags & SD_FLAGS_LAST_LEAVE))
    {
        SD_SetClockDivReg(ctxt->clockDiv);
        SD_SetBusWidthReg(ctxt->cardBusWidth);
    }

    switch (index)
    {
    case SD_1:
        /** switch clock to SD1 */
        g_sdCtxt[SD_2].flags &= ~SD_FLAGS_LAST_LEAVE;
        g_sdCtxt[SD_1].flags |= SD_FLAGS_LAST_LEAVE;
        break;

    case SD_2:
        /** switch clock to SD2 */
        g_sdCtxt[SD_1].flags &= ~SD_FLAGS_LAST_LEAVE;
        g_sdCtxt[SD_2].flags |= SD_FLAGS_LAST_LEAVE;
        break;
    }
}

#if defined(SD_IRQ_ENABLE)

extern MMP_EVENT sd_isr_event;

static MMP_INLINE void intrEnable(SD_CTXT *ctxt)
{
    /** create event for isr */
    if (!sd_isr_event)
        sd_isr_event = SYS_CreateEvent();

    /** register interrupt handler to interrupt mgr */
    ithIntrRegisterHandlerIrq(ITH_INTR_SD, sd_isr, (void *)g_sdCtxt);
    #if defined(MM9910)
    ithIntrSetTriggerModeIrq(ITH_INTR_SD, ITH_INTR_LEVEL);
    #else
    ithIntrSetTriggerModeIrq(ITH_INTR_SD, ITH_INTR_EDGE);
    #endif
    ithIntrSetTriggerLevelIrq(ITH_INTR_SD, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_SD);

    /** register isr to dma */
    mmpDmaRegisterIsr(ctxt->dmaCtxt, dma_isr, (void *)ctxt);

    #if defined(MM9910)
    AHB_WriteRegister(SD_REG_INTR, ~(SD_INTR_ALL << SD_SHT_INTR_MSK) | SD_INTR_ALL);
    SD_WrapIntrEnable();
    #else
    AHB_WriteRegisterMask(SD_REG_STS1, 0, SD_MSK_INTR_MSK);     /** enable sd interrupt, only INTRMSK is writable */
    #endif
}

static MMP_INLINE void intrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_SD);
    if (sd_isr_event)
    {
        SYS_DelEvent(sd_isr_event);
        sd_isr_event = MMP_NULL;
    }
    #if defined(MM9910)
    AHB_WriteRegister(SD_REG_INTR, (SD_INTR_ALL << SD_SHT_INTR_MSK) | SD_INTR_ALL);
    SD_WrapIntrDisable();
    #endif
}
#else
    #define intrEnable(a)
    #define intrDisable()
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * SD Initialization.
 */
//=============================================================================
MMP_INT mmpSdInitializeEx(MMP_INT index)
{
    MMP_INT  result           = 0;
    SD_CTXT  *ctxt            = &g_sdCtxt[index];
    MMP_BOOL releaseSemaphore = MMP_FALSE;
    LOG_ENTER "[mmpSd%dInitialize] Enter \n", (index + 1)LOG_END

    if (ctxt->flags & SD_FLAGS_INIT_READY)
        return result;

    /** Check card is inserted. */
    if (!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end;
    }

    result      = SD_ContextInitialize(ctxt);
    if (result)
        goto end;
    ctxt->index = index;

    if (!SD_Semaphore)
    {
#if defined(SD_SHARE_SEMAPHORE)
        SD_Semaphore = (void *)HOST_GetStorageSemaphore();
#else
        SD_Semaphore = SYS_CreateSemaphore(1, "MMP_SD");
#endif
        if (!SD_Semaphore)
        {
            result = ERROR_SD_CREATE_SEMAPHORE_FAIL;
            goto end;
        }
    }

    SYS_WaitSemaphore(SD_Semaphore);
    releaseSemaphore = MMP_TRUE;
    SD_SwitchLun(index, MODE_NON_RW);

    SD_PowerOnReg(index);
    intrEnable(ctxt);

    result = SDMMC_StartUp(ctxt);
    if (result)
        goto end;

    result = SDMMC_GetCapacity(ctxt);
    if (result)
        goto end;

    result = SDMMC_TransferState(ctxt);
    if (result)
        goto end;

    result = SDMMC_Switch(ctxt);
    if (result)
        goto end;

    if (HOST_IsCardLocked((index == 0) ? MMP_CARD_SD_I : MMP_CARD_SD2))
        ctxt->flags |= SD_FLAGS_WRITE_PROTECT;

    ctxt->flags |= SD_FLAGS_INIT_READY;

end:
    if (SD_Semaphore && releaseSemaphore)
        SYS_ReleaseSemaphore(SD_Semaphore);

    LOG_LEAVE "[mmpSdInitialize] Leave \n" LOG_END
    if (result)
    {
        ctxt->flags |= SD_FLAGS_INIT_FAIL;
        mmpSdTerminateEx(index);
        LOG_ERROR "mmpSd%dInitialize() return error code 0x%08X \n", (index + 1), result LOG_END
    }

    return result;
}

//=============================================================================
/**
 * SD Terminate.
 */
//=============================================================================
MMP_INT
mmpSdTerminateEx(MMP_INT index)
{
    MMP_INT result = 0;
    SD_CTXT *ctxt  = &g_sdCtxt[index];
    LOG_ENTER "[mmpSd%dTerminate] Enter \n", (index + 1)LOG_END

    if (ctxt->dmaCtxt)
    {
        result        = mmpDmaDestroyContext(ctxt->dmaCtxt);
        ctxt->dmaCtxt = MMP_NULL;
    }

    if (SD_Semaphore)
    {
        SYS_WaitSemaphore(SD_Semaphore);
        SD_SwitchLun(index, MODE_NON_RW);

        if (!(g_sdCtxt[!index].flags & SD_FLAGS_INIT_READY)) // for two sd
            intrDisable();
        else
        {
#if defined(MM9910)
            AHB_WriteRegister(SD_REG_INTR, ~(SD_INTR_ALL << SD_SHT_INTR_MSK) | SD_INTR_ALL);
            SD_WrapIntrEnable();
#else
            AHB_WriteRegisterMask(SD_REG_STS1, 0, SD_MSK_INTR_MSK);     /** enable sd interrupt, only INTRMSK is writable */
#endif
        }
        SD_PowerDownReg(ctxt);    // iclai(2015-06-09): fix sd error when frequently plug in/out the sd card.
        SYS_ReleaseSemaphore(SD_Semaphore);
    }

#if !defined(SD_SHARE_SEMAPHORE)
    #if 0
    if (SD_Semaphore)
    {
        SYS_DeleteSemaphore(SD_Semaphore);
        SD_Semaphore = MMP_NULL;
    }
    #endif
#endif

    ctxt->flags &= ~SD_FLAGS_INIT_READY;

    LOG_LEAVE "[mmpSdTerminate] Leave \n" LOG_END
    if (result)
        LOG_ERROR "mmpSd%dTerminate() return error code 0x%08X \n", (index + 1), result LOG_END

        return result;
}

//=============================================================================
/**
 * SD read multisector function.
 */
//=============================================================================
MMP_INT
mmpSdReadMultipleSectorEx(MMP_INT index, MMP_UINT32 sector, MMP_INT count, void *data)
{
    MMP_INT   result    = 0;
    SD_CTXT   *ctxt     = &g_sdCtxt[index];
    MMP_UINT  i         = 0;
    MMP_UINT8 *tmpAddr1 = MMP_NULL;
    LOG_ENTER "[mmpSd%dReadMultipleSector] Enter sector = %d, count = %d, dataAddr = 0x%08X \n", (index + 1), sector, count, data LOG_END
    //printf(" R(%d,%d) \n", sector, count);

    SYS_WaitSemaphore(SD_Semaphore);
    SD_SwitchLun(index, MODE_RW);
    SD_SelectIo(index);

    if (!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end;
    }

#if defined(SD_WIN32_DMA)
    if (ctxt->dmaCtxt)
    {
        MMP_UINT8 *tmpAddr2 = MMP_NULL;

        tmpAddr1 = (MMP_UINT8 *)MEM_Allocate((SD_SECTOR_SIZE * count + 4), MEM_USER_SD);
        if (!tmpAddr1)
        {
            result = ERROR_SD_ALLOC_TMP_VRAM_FAIL;
            goto end;
        }
        tmpAddr2 = (MMP_UINT8 *)(((MMP_UINT32)tmpAddr1 + 3) & ~3);

        result   = SDMMC_ReadMultiSector(ctxt, sector, (MMP_UINT32)count, tmpAddr2);
        if (result)
            goto end;

        HOST_ReadBlockMemory((MMP_UINT32)data, (MMP_UINT32)tmpAddr2, SD_SECTOR_SIZE * count);
    }
#else
    if (((MMP_UINT32)data & 0x3) && !ctxt->dmaCtxt)
    {
        MMP_UINT8 *tmpAddr2 = MMP_NULL;

        LOG_WARNING " mmpSdReadMultipleSector() input data address doesn't dword align! addr = 0x%08X \n", data LOG_END
        tmpAddr1 = SYS_Malloc(SD_SECTOR_SIZE * count + 4);
        if (!tmpAddr1)
        {
            result = ERROR_SD_ALLOC_TMP_SYSTEM_MEMORY_FAIL;
            goto end;
        }
        tmpAddr2 = (MMP_UINT8 *)(((MMP_UINT32)tmpAddr1 + 3) & ~3);

        result   = SDMMC_ReadMultiSector(ctxt, sector, (MMP_UINT32)count, tmpAddr2);
        if (result)
            goto end;

        SYS_Memcpy(data, tmpAddr2, SD_SECTOR_SIZE * count);
    }
#endif
    else
    {
        result = SDMMC_ReadMultiSector(ctxt, sector, (MMP_UINT32)count, data);
        if (result)
            goto end;
    }

#if 0
    if ((sector == 0) && (count == 1))
    {
        MMP_UINT32 i       = 0;
        MMP_UINT8  *status = data;
        for (i = 0; i < 512; i += 16)
        {
            LOG_DATA " Byte %04X: %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X \n",
            i, status[i + 0], status[i + 1], status[i + 2], status[i + 3], status[i + 4], status[i + 5], status[i + 6], status[i + 7]
            , status[i + 8], status[i + 9], status[i + 10], status[i + 11], status[i + 12], status[i + 13], status[i + 14], status[i + 15] LOG_END
        }
    }
#endif

end:
    SYS_ReleaseSemaphore(SD_Semaphore);
    if (tmpAddr1)
    {
#if defined(SD_WIN32_DMA)
        MEM_Release((void *)tmpAddr1);
#else
        SYS_Free(tmpAddr1);
#endif
        tmpAddr1 = MMP_NULL;
    }
    LOG_LEAVE "[mmpSdReadMultipleSector] Leave \n" LOG_END
    if (result)
        LOG_ERROR "mmpSd%dReadMultipleSector(%d, %d) return error code 0x%08X \n", (index + 1), sector, count, result LOG_END

        return result;
}

//=============================================================================
/**
 * SD read multisector function.
 */
//=============================================================================
MMP_INT
mmpSdWriteMultipleSectorEx(MMP_INT index, MMP_UINT32 sector, MMP_INT count, void *data)
{
    MMP_INT   result    = 0;
    SD_CTXT   *ctxt     = &g_sdCtxt[index];
    MMP_UINT  i         = 0;
    MMP_UINT8 *tmpAddr1 = MMP_NULL;
    LOG_ENTER "[mmpSd%dWriteMultipleSector] Enter sector = %d, count = %d, dataAddr = 0x%08X \n", (index + 1), sector, count, data LOG_END
    //printf(" W(%d,%d) \n", sector, count);

    SYS_WaitSemaphore(SD_Semaphore);
    SD_SwitchLun(index, MODE_RW);
    SD_SelectIo(index);

    if (!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end;
    }

    if (ctxt->flags & SD_FLAGS_WRITE_PROTECT)
    {
        result = ERROR_SD_IS_WRITE_PROTECT;
        goto end;
    }

#if defined(SD_WIN32_DMA)
    if (ctxt->dmaCtxt)
    {
        MMP_UINT8 *tmpAddr2 = MMP_NULL;

        tmpAddr1 = (MMP_UINT8 *)MEM_Allocate((SD_SECTOR_SIZE * count + 4), MEM_USER_SD);
        if (!tmpAddr1)
        {
            result = ERROR_SD_ALLOC_TMP_VRAM_FAIL;
            goto end;
        }
        tmpAddr2 = (MMP_UINT8 *)(((MMP_UINT32)tmpAddr1 + 3) & ~3);
        HOST_WriteBlockMemory((MMP_UINT32)tmpAddr2, (MMP_UINT32)data, SD_SECTOR_SIZE * count);

        result   = SDMMC_WriteMultiSector(ctxt, sector, (MMP_UINT16)count, tmpAddr2);
        if (result)
            goto end;
    }
#else
    if (((MMP_UINT32)data & 0x3) && !ctxt->dmaCtxt)
    {
        MMP_UINT8 *tmpAddr2 = MMP_NULL;

        LOG_WARNING " mmpSdWriteMultipleSector() input data address doesn't dword align! addr = 0x%08X \n", data LOG_END
        tmpAddr1 = SYS_Malloc(SD_SECTOR_SIZE * count + 4);
        if (!tmpAddr1)
        {
            result = ERROR_SD_ALLOC_TMP_SYSTEM_MEMORY_FAIL;
            goto end;
        }
        tmpAddr2 = (MMP_UINT8 *)(((MMP_UINT32)tmpAddr1 + 3) & ~3);
        SYS_Memcpy(tmpAddr2, data, SD_SECTOR_SIZE * count);

        result   = SDMMC_WriteMultiSector(ctxt, sector, (MMP_UINT16)count, tmpAddr2);
        if (result)
            goto end;
    }
#endif
    else
    {
        result = SDMMC_WriteMultiSector(ctxt, sector, (MMP_UINT16)count, data);
        if (result)
            goto end;
    }

end:
    SYS_ReleaseSemaphore(SD_Semaphore);
    if (tmpAddr1)
    {
#if defined(SD_WIN32_DMA)
        MEM_Release((void *)tmpAddr1);
#else
        SYS_Free(tmpAddr1);
#endif
        tmpAddr1 = MMP_NULL;
    }
    LOG_LEAVE "[mmpSdWriteMultipleSector] Leave \n" LOG_END
    if (result)
        LOG_ERROR "mmpSd%dWriteMultipleSector(%d, %d) return error code 0x%08X \n", (index + 1), sector, count, result LOG_END

        return result;
}

//=============================================================================
/**
 * SD card status API.
 */
//=============================================================================
MMP_BOOL
mmpSdGetCardStateEx(MMP_INT index, SD_CARD_STATE state)
{
    MMP_BOOL result = MMP_FALSE;
    LOG_ENTER "[mmpSdGetCardState] Enter \n" LOG_END

    switch (state)
    {
    case SD_INIT_OK:
        result = (g_sdCtxt[index].flags & SD_FLAGS_INIT_READY) ? MMP_TRUE : MMP_FALSE;
        break;

    case SD_INSERTED:
        result = SD_IsCardInserted(index);
        break;

    default:
        break;
    }

    LOG_LEAVE "[mmpSdGetCardState] Leave result = %d \n", result LOG_END

    return result;
}

//=============================================================================
/**
 * SD card capacity API.
 */
//=============================================================================
MMP_INT
mmpSdGetCapacityEx(MMP_INT index,
                   MMP_UINT32 *sectorNum,
                   MMP_UINT32 *blockLength)
{
    MMP_INT result = 0;
    SD_CTXT *ctxt  = &g_sdCtxt[index];
    LOG_ENTER "[mmpSd%dGetCapacity] Enter \n", (index + 1)LOG_END

    if (!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end;
    }

    *sectorNum   = ctxt->totalSectors; /** this value is sector numbers, not last block ID. */
    *blockLength = SD_SECTOR_SIZE;

end:
    LOG_LEAVE "[mmpSdGetCapacity] Leave \n" LOG_END
    if (result)
        LOG_ERROR "mmpSd%dGetCapacity() return error code 0x%08X \n", (index + 1), result LOG_END

        return result;
}

//=============================================================================
/**
 * SD card Is in write-protected status?
 */
//=============================================================================
MMP_BOOL
mmpSdIsLockEx(MMP_INT index)
{
    MMP_BOOL isLock = MMP_FALSE;
    SD_CTXT  *ctxt  = &g_sdCtxt[index];
    LOG_ENTER "[mmpSd%IsLock] Enter \n", (index + 1) LOG_END

    isLock = (ctxt->flags & SD_FLAGS_WRITE_PROTECT) ? MMP_TRUE : MMP_FALSE;

    LOG_LEAVE "[mmpSdIsLock] Leave \n" LOG_END

    return isLock;
}

/** Get CID information */
void
mmpSdGetCidEx(MMP_INT index, MMP_UINT8 *buf)
{
    SD_CTXT *ctxt = &g_sdCtxt[index];
    memcpy((void *)buf, ctxt->cid, sizeof(ctxt->cid));
}