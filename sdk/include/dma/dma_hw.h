/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as DMA controller configure header file.
 *
 * @author Irene Lin
 */

#ifndef DMA_HW_H
#define DMA_HW_H

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

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//							Funtion Declaration
//=============================================================================
void DMA_DumpFeatureReg(void);

void DMA_EnableControllerReg(void);


static MMP_INLINE
void DMA_SetCsrReg(
    MMP_UINT8 channel,
    MMP_UINT32 csr)
{
    //AHB_WriteRegister( (DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), csr);
}

static MMP_INLINE
void DMA_SetCfgReg(
    MMP_UINT8 channel,
    MMP_UINT32 cfg)
{
	AHB_WriteRegister(DMA_REG_INT_TC_CLEAR, (0x1<<channel)); /** clear interrupt, before enable interrupt */
    AHB_WriteRegister( (DMA_REG_C0_CFG + (channel*DMA_CHANNEL_OFFSET)), cfg);
}

static MMP_INLINE
void DMA_FireReg(
    MMP_UINT8 channel,
    MMP_UINT32 csr)
{
    //AHB_WriteRegisterMask( (DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), DMA_MSK_CHANNEL_EN, DMA_MSK_CHANNEL_EN);
    AHB_WriteRegister( (DMA_REG_C0_CSR + (channel*DMA_CHANNEL_OFFSET)), (csr|DMA_MSK_CHANNEL_EN));
}

static MMP_INLINE
void DMA_SetSrcAddressReg(
    MMP_UINT8 channel,
    MMP_UINT32 addr)
{
    AHB_WriteRegister( (DMA_REG_C0_SRC_ADDR + (channel*DMA_CHANNEL_OFFSET)), addr);
}

static MMP_INLINE
void DMA_SetDstAddressReg(
    MMP_UINT8 channel,
    MMP_UINT32 addr)
{
    AHB_WriteRegister( (DMA_REG_C0_DST_ADDR + (channel*DMA_CHANNEL_OFFSET)), addr);
}

static MMP_INLINE
void DMA_SetLLPReg(
    MMP_UINT8 channel,
    MMP_UINT32 llp)
{
	/** D[31:2] DWORD align */
    AHB_WriteRegister( (DMA_REG_C0_LINKED_LIST_POINTER + (channel*DMA_CHANNEL_OFFSET)), llp);
#if 0
	{
		AHB_ReadRegister( (DMA_REG_C0_LINKED_LIST_POINTER + (channel*DMA_CHANNEL_OFFSET)), &llp);
		printf(" set Reg 0x%08X = 0x%08X \n", (DMA_REG_C0_LINKED_LIST_POINTER + (channel*DMA_CHANNEL_OFFSET)), llp);
		{
			MMP_INT i;
			MMP_UINT8* lld = (MMP_UINT8*)llp;
			for(i=0; i<20; i++)
				printf(" %02X", lld[i]);
			printf(" \n");
		}
	}
#endif
}

static MMP_INLINE
void DMA_SetTxSizeReg(
    MMP_UINT8 channel,
    MMP_UINT32 size)
{
    AHB_WriteRegister( (DMA_REG_C0_TX_SIZE + (channel*DMA_CHANNEL_OFFSET)), size);
}

MMP_INT DMA_WaitTxSizeEmptyReg(MMP_UINT8 channel);

MMP_INT DMA_WaitChannelReadyReg(DMA_CTXT* ctxt);

MMP_INT DMA_BusyWaitChannelReadyReg(DMA_CTXT* ctxt);

static MMP_INLINE MMP_BOOL DMA_IsChannelReadyReg(
    DMA_CTXT* ctxt)
{
    MMP_UINT32 reg = 0;
    AHB_ReadRegister((DMA_REG_C0_CSR + (ctxt->channel*DMA_CHANNEL_OFFSET)), &reg);
    return (reg & DMA_MSK_CHANNEL_EN) ? MMP_FALSE : MMP_TRUE;
}


#ifdef __cplusplus
}
#endif

#endif //DMA_HW_H
