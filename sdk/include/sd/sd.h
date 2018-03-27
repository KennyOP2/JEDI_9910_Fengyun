/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as SD/MMC related function header file.
 *
 * @author Irene Lin
 */
#ifndef	SD_H
#define	SD_H


//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum SD_FLAGS_TAG
{
    SD_FLAGS_DMA_ENABLE          = (0x00000001 << 0),
    SD_FLAGS_WRITE_PROTECT       = (0x00000001 << 1),
    SD_FLAGS_SUPPORT_CMD8        = (0x00000001 << 2),
    SD_FLAGS_CARD_SDHC           = (0x00000001 << 3),
    SD_FLAGS_CARD_SD             = (0x00000001 << 4),
    SD_FLAGS_CARD_MMC            = (0x00000001 << 5),
    SD_FLAGS_CARD_MMC4           = (0x00000001 << 6),
    SD_FLAGS_SUPPORT_SWITCH_FUNC = (0x00000001 << 7),
    SD_FLAGS_CARD_MMC_HC         = (0x00000001 << 8),
    SD_FLAGS_MMC_8_BIT_BUS       = (0x00000001 << 9),
    SD_FLAGS_MMC_4_BIT_BUS       = (0x00000001 << 10),
    SD_FLAGS_SD_HIGH_SPEED       = (0x00000001 << 11),
    SD_FLAGS_INIT_FAIL           = (0x00000001 << 12),
    SD_FLAGS_INIT_READY          = (0x00000001 << 13),
    SD_FLAGS_SD2                 = (0x00000001 << 14),
    SD_FLAGS_LAST_LEAVE          = (0x00000001 << 15),
    SD_FLAGS_WRAP                = (0x00000001 << 16),
    SD_FLAGS_DATA_IN             = (0x00000001 << 17),
    SD_FLAGS_DATA_DMA            = (0x00000001 << 18),
    #if defined(SD_IRQ_ENABLE)
    SD_FLAGS_WRAP_INTR           = (0x00000001 << 24),
    SD_FLAGS_IRQ_SD              = (0x00000001 << 25),
    SD_FLAGS_IRQ_DMA_END         = (0x00000001 << 26),
    SD_FLAGS_IRQ_SD_END          = (0x00000001 << 27),
    SD_FLAGS_IRQ_DMA_ERROR       = (0x00000001 << 28),
    SD_FLAGS_IRQ_SD_ERROR        = (0x00000001 << 29),
    SD_FLAGS_IRQ_WRAP_END        = (0x00000001 << 30),
    #endif
    SD_FLAGS_MAX                 = (0x00000001 << 31),
} SD_FLAGS;

#if defined(SD_IRQ_ENABLE)
#define SD_IRQ_FLAGS     (SD_FLAGS_IRQ_SD|\
                          SD_FLAGS_WRAP|\
                          SD_FLAGS_WRAP_INTR|\
                          SD_FLAGS_DATA_DMA|\
                          SD_FLAGS_IRQ_DMA_END|\
                          SD_FLAGS_IRQ_SD_END|\
                          SD_FLAGS_IRQ_DMA_ERROR|\
                          SD_FLAGS_IRQ_SD_ERROR|\
                          SD_FLAGS_IRQ_WRAP_END)
#endif

typedef enum SD_DMA_TYPE_TAG
{
    SD_DMA_NORMAL,
    SD_DMA_HW_HANDSHAKING
} SD_DMA_TYPE;


//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct SD_CTXT_TAG
{
    volatile MMP_UINT32 flags;
#if defined(SD_IRQ_ENABLE)
    MMP_UINT32          intr_flags; // for interrupt safe
#endif
    MMP_UINT8           dmaType;
    MMP_UINT8           cardBusWidth;
    MMP_UINT8           rcaByte3;
    MMP_UINT8           rcaByte2;
    MMP_UINT32          totalSectors;
    MMP_UINT32          timeout;
    MMP_INT             index;
    MMP_UINT8           clockDiv;
    MMP_UINT8           cmd;
    volatile MMP_UINT8  intrErr;
    MMP_UINT8           reserved[1];

    MMP_DMA_CONTEXT     dmaCtxt;
    MMP_UINT8           cid[16];
} SD_CTXT;


//=============================================================================
//							Funtion Declaration
//=============================================================================

/** common.c */
MMP_INT SDMMC_StartUp(SD_CTXT* ctxt);

MMP_INT SDMMC_GetCapacity(SD_CTXT* ctxt);

MMP_INT SDMMC_TransferState(SD_CTXT* ctxt);

MMP_INT SDMMC_Switch(SD_CTXT* ctxt);

MMP_INT SDMMC_DmaRead(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 size);

MMP_INT SDMMC_DmaWrite(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 size);

MMP_INT SDMMC_ReadData(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 totalSize);

MMP_INT SDMMC_WriteData(SD_CTXT* ctxt, MMP_UINT8* data, MMP_UINT32 totalSize);

MMP_INT SDMMC_ReadMultiSector(SD_CTXT* ctxt, MMP_UINT32 sector, MMP_UINT32 count, void* data);

MMP_INT SDMMC_WriteMultiSector(SD_CTXT* ctxt, MMP_UINT32 sector, MMP_UINT32 count, void* data);


/** sd.c */
MMP_INT SD_StartUp(SD_CTXT* ctxt);

MMP_INT SD_GetRca(SD_CTXT* ctxt);

MMP_INT SD_Switch(SD_CTXT* ctxt);


/** mmc.c */
MMP_INT MMC_StartUp(SD_CTXT* ctxt);

MMP_INT MMC_SetRca(SD_CTXT* ctxt);

MMP_INT MMC_Switch(SD_CTXT* ctxt);

#endif
