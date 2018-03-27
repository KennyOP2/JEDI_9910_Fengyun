/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as DMA controller configure header file.
 *
 * @author Irene Lin
 */

#ifndef DMA_H
#define DMA_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "dma/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum DMA_FLAGS_TAG
{
    DMA_FLAGS_HW_HANDSHAKING             = (0x00000001 << 0),
    DMA_FLAGS_USE_IRQ                    = (0x00000001 << 1),
    DMA_FLUSH_SRC_FLAG                   = (0x00000001 << 2),
    DMA_FLUSH_DST_FLAG                   = (0x00000001 << 3)
} DMA_FLAGS;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct DMA_CTXT_TAG
{
    MMP_UINT32 flags;
    MMP_UINT8  channel;
    MMP_UINT8  dmaType;
    MMP_UINT8  onlyUpdateSize;
    MMP_UINT8  priority;

    MMP_UINT32 srcAddr;
    MMP_UINT32 dstAddr;
    MMP_UINT32 totalSize; /** byte unit */
    MMP_UINT32 txSize;    /** source width unit */
    MMP_UINT8  srcWidth;  /** source transfer width in bytes, set from outside */
    MMP_UINT8  dstWidth;  /** destination width in bytes, set from outside */
	MMP_UINT8  srcW;  /** source transfer width in bytes, final setting */
	MMP_UINT8  dstW;  /** destination width in bytes, final setting */
    MMP_UINT32 burstSize; /** srcWidth unit */

    MMP_UINT32 csr;
    MMP_UINT32 cfg;
    MMP_UINT32 lldAddr;
	MMP_UINT8  fifoThreshold;
	MMP_UINT8  reserved[3];
} DMA_CTXT;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//							Funtion Declaration
//=============================================================================
MMP_INT DMA_Update(DMA_CTXT* ctxt);


#ifdef __cplusplus
}
#endif

#endif //DMA_H
