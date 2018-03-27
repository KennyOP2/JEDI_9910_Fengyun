/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "dma/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_INLINE MMP_INT
DMA_UpdateTxSize(
    DMA_CTXT* ctxt)
{
    MMP_INT result = 0;
    MMP_UINT8 srcWidth = ctxt->srcWidth;
    MMP_UINT8 dstWidth = ctxt->dstWidth;

    ctxt->csr &= ~(DMA_MSK_SRC_TX_WIDTH|DMA_MSK_DST_TX_WIDTH);

    if(!srcWidth)
    {
        if(ctxt->totalSize & 0x1)  /** size is 8-bits */
        {
            srcWidth = 1;
        }
        else if(ctxt->totalSize & 0x2) /** size is 16-bits */
        {
            if(ctxt->srcAddr & 0x1)
                srcWidth = 1;
            else
                srcWidth = 2;
        }
        else
        {
            if(ctxt->srcAddr & 0x1)
                srcWidth = 1;
            else if(ctxt->srcAddr & 0x2)
                srcWidth = 2;
            else
                srcWidth = 4;
        }
    }
    if(!dstWidth)
    {
        if(ctxt->totalSize & 0x1)  /** size is 8-bits */
        {
            dstWidth = 1;
        }
        else if(ctxt->totalSize & 0x2) /** size is 16-bits */
        {
            if(ctxt->dstAddr & 0x1)
                dstWidth = 1;
            else
                dstWidth = 2;
        }
        else
        {
            if(ctxt->dstAddr & 0x1)
                dstWidth = 1;
            else if(ctxt->dstAddr & 0x2)
                dstWidth = 2;
            else
                dstWidth = 4;
        }
    }


    switch(srcWidth)
    {
    case 1:
        ctxt->csr |= DMA_SRC_TX_WIDTH_8_BITS;
        break;
    case 2:
        ctxt->csr |= DMA_SRC_TX_WIDTH_16_BITS;
        break;
    case 4:
        ctxt->csr |= DMA_SRC_TX_WIDTH_32_BITS;
        break;
    default:
        break;
    };

    switch(dstWidth)
    {
    case 1:
        ctxt->csr |= DMA_DST_TX_WIDTH_8_BITS;
        break;
    case 2:
        ctxt->csr |= DMA_DST_TX_WIDTH_16_BITS;
        break;
    case 4:
        ctxt->csr |= DMA_DST_TX_WIDTH_32_BITS;
        break;
    default:
        break;
    };

	ctxt->srcW = srcWidth;
	ctxt->dstW = dstWidth;
    ctxt->txSize = ctxt->totalSize/ctxt->srcW;

    LOG_INFO " TxSize=0x%X, srcWidth=%d, dstWidth=%d \n", ctxt->txSize, srcWidth, dstWidth LOG_END

    return result;
}


static MMP_INLINE MMP_INT
DMA_UpdateAddrCtrl(
    DMA_CTXT* ctxt)
{
    MMP_INT result = 0;

    ctxt->csr &= ~(DMA_MSK_SRC_ADDR_CTRL|DMA_MSK_DST_ADDR_CTRL|DMA_MSK_SRC_AHB1|DMA_MSK_DST_AHB1);

    switch(ctxt->dmaType)
    {
    case MMP_DMA_TYPE_MEM_TO_MEM:
        ctxt->csr |= (DMA_SRC_ADDR_CTRL_INC|DMA_DST_ADDR_CTRL_INC);
        ctxt->flags |= (DMA_FLUSH_SRC_FLAG | DMA_FLUSH_DST_FLAG);
        break;
    case MMP_DMA_TYPE_MEM_SET:
        ctxt->csr |= (DMA_SRC_ADDR_CTRL_FIX|DMA_DST_ADDR_CTRL_INC);
        ctxt->flags |= (DMA_FLUSH_SRC_FLAG | DMA_FLUSH_DST_FLAG);
        break;
    case MMP_DMA_TYPE_MEM_TO_MS:
    case MMP_DMA_TYPE_MEM_TO_CF:
    case MMP_DMA_TYPE_MEM_TO_SD:
    case MMP_DMA_TYPE_MEM_TO_XD:
	case MMP_DMA_TYPE_MEM_TO_SPDIF:
	case MMP_DMA_TYPE_MEM_TO_SPI:   
	case MMP_DMA_TYPE_MEM_TO_SPI2:   
    case MMP_DMA_TYPE_MEM_TO_UART: 
	case MMP_DMA_TYPE_MEM_TO_UART2: 
	case MMP_DMA_TYPE_MEM_TO_IRDA:
    case MMP_DMA_TYPE_MEM_TO_IR:
        ctxt->csr |= (DMA_SRC_ADDR_CTRL_INC|DMA_DST_ADDR_CTRL_FIX);
        ctxt->csr |= DMA_MSK_DST_AHB1;
        ctxt->flags |= DMA_FLUSH_SRC_FLAG;
        break;
    case MMP_DMA_TYPE_MS_TO_MEM:
    case MMP_DMA_TYPE_CF_TO_MEM:
    case MMP_DMA_TYPE_SD_TO_MEM:
    case MMP_DMA_TYPE_XD_TO_MEM:
    case MMP_DMA_TYPE_SPI_TO_MEM:  
	case MMP_DMA_TYPE_SPI2_TO_MEM:  
    case MMP_DMA_TYPE_UART_TO_MEM:  
	case MMP_DMA_TYPE_UART2_TO_MEM:
	case MMP_DMA_TYPE_IRDA_TO_MEM:
    case MMP_DMA_TYPE_IR_TO_MEM:
        ctxt->csr |= (DMA_SRC_ADDR_CTRL_FIX|DMA_DST_ADDR_CTRL_INC);
        ctxt->csr |= DMA_MSK_SRC_AHB1;
        ctxt->flags |= DMA_FLUSH_DST_FLAG;
        break;
        
    case MMP_DMA_TYPE_APB_TO_SPI:
        ctxt->csr |= (DMA_SRC_ADDR_CTRL_FIX|DMA_DST_ADDR_CTRL_FIX);
        ctxt->csr |= DMA_MSK_SRC_AHB1;
        ctxt->csr |= DMA_MSK_DST_AHB1;
        break;
    };

    if(result)
        LOG_ERROR "DMA_UpdateSize() return error code 0x%08X \n", result LOG_END

    return result;
}


static MMP_INLINE MMP_INT
DMA_UpdateBurstSize(
    DMA_CTXT* ctxt)
{
    MMP_INT result = 0;
    MMP_UINT32 burstSize = ctxt->burstSize;

    ctxt->csr &= ~DMA_MSK_SRC_BURST_SIZE;

    if(!burstSize)
    {
        if(ctxt->txSize >= 256)
            burstSize = 256;
        else if(ctxt->txSize >= 128)
            burstSize = 128;
        else if(ctxt->txSize >= 64)
            burstSize = 64;
        else if(ctxt->txSize >= 32)
            burstSize = 32;
        else if(ctxt->txSize >= 16)
            burstSize = 16;
        else if(ctxt->txSize >= 8)
            burstSize = 8;
        else if(ctxt->txSize >= 4)
            burstSize = 4;
        else
            burstSize = 1;
    }

    switch(burstSize)
    {
    case 1:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_1;
        break;
    case 4:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_4;
        break;
    case 8:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_8;
        break;
    case 16:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_16;
        break;
    case 32:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_32;
        break;
    case 64:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_64;
        break;
    case 128:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_128;
        break;
    case 256:
        ctxt->csr |= DMA_SRC_BURSET_SIZE_256;
        break;
    default:
        result = ERROR_DMA_BURST_SIZE_ERROR;
        goto end;
        break;
    };

end:
    LOG_INFO " burstSize = %d \n", burstSize LOG_END
    //printf(" burstSize = %d \n", burstSize);
    if(result)
        LOG_ERROR "DMA_UpdateBurstSize() return error code 0x%08X \n", result LOG_END

    return result;
}


static MMP_INLINE MMP_INT
DMA_UpdateCfg(
    DMA_CTXT* ctxt)
{
    MMP_INT result = 0;

    if(ctxt->flags & DMA_FLAGS_USE_IRQ)
		ctxt->cfg = 0;
	else
		ctxt->cfg = DMA_INTR_MSK;

    if(!(ctxt->flags & DMA_FLAGS_HW_HANDSHAKING))
        goto end;


    switch(ctxt->dmaType)
    {
    case MMP_DMA_TYPE_MEM_TO_MS:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_MS << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_MS_TO_MEM:
        ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_MS << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_MEM_TO_CF:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_CF << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_CF_TO_MEM:
        ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_CF << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_MEM_TO_SD:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_CF << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_SD_TO_MEM:
        ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_CF << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_MEM_TO_XD:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_CF << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;
    case MMP_DMA_TYPE_XD_TO_MEM:
        ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_HANDSHAKING_CF << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
        break;
	case MMP_DMA_TYPE_MEM_TO_SPDIF:
		ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_HANDSHAKING_SPDIF << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
		break;
 	case MMP_DMA_TYPE_MEM_TO_SPI:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_SSP_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;
	case MMP_DMA_TYPE_MEM_TO_SPI2:
		ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_SSP2_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
		break;
        
    case MMP_DMA_TYPE_SPI_TO_MEM:
        ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_SSP_RX << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
        break;  

	case MMP_DMA_TYPE_SPI2_TO_MEM:
		ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_SSP2_RX << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
		break;    


    case MMP_DMA_TYPE_APB_TO_SPI:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_SSP_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;     

    case MMP_DMA_TYPE_UART_TO_MEM:
        ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_UART_RX << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
        break;    

    case MMP_DMA_TYPE_MEM_TO_UART:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
        ctxt->cfg |= ((DMA_HW_UART_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
        break;     
	
	case MMP_DMA_TYPE_UART2_TO_MEM:
		ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_UART2_RX << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
		break; 
	
	case MMP_DMA_TYPE_MEM_TO_UART2:
		ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_UART2_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
		break;

	case MMP_DMA_TYPE_IRDA_TO_MEM:
		ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_UART_FIR << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
		break; 

	case MMP_DMA_TYPE_MEM_TO_IRDA:
		ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_UART_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
		break;

    case MMP_DMA_TYPE_IR_TO_MEM:
		ctxt->cfg |= DMA_MSK_SRC_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_IR_Cap << DMA_SHT_SRC_REQUEST_SEL) & DMA_MSK_SRC_REQUEST_SEL);
		break; 

    case MMP_DMA_TYPE_MEM_TO_IR:
        ctxt->cfg |= DMA_MSK_DST_HW_HANDSHAKING_EN;
		ctxt->cfg |= ((DMA_HW_IR_Cap_TX << DMA_SHT_DST_REQUEST_SEL) & DMA_MSK_DST_REQUEST_SEL);
		break;

    default:
        result = ERROR_DMA_UNSUPPORT_HH_HANDSHAKING_ENGINE;
        goto end;
        break;
    };

    ctxt->csr |= DMA_MSK_HW_HANDSHAKE_MODE;

end:
    if(result)
        LOG_ERROR "DMA_UpdateCfg() return error code 0x%08X \n", result LOG_END

    return result;
}


static MMP_INLINE MMP_INT
DMA_UpdateHw(
    DMA_CTXT* ctxt)
{
    MMP_INT result = 0;

    DMA_SetSrcAddressReg(ctxt->channel, ctxt->srcAddr);
    DMA_SetDstAddressReg(ctxt->channel, ctxt->dstAddr);
    DMA_SetTxSizeReg(ctxt->channel, ctxt->txSize);
    //DMA_SetCsrReg(ctxt->channel, ctxt->csr); /** set with fire */
    //if(ctxt->flags & DMA_FLAGS_HW_HANDSHAKING)
    {
        DMA_SetCfgReg(ctxt->channel, ctxt->cfg);
    }
    DMA_SetLLPReg(ctxt->channel, ctxt->lldAddr);
	if((ctxt->flags & DMA_FLAGS_USE_IRQ) && ctxt->lldAddr)
		ctxt->csr |= DMA_MSK_TC;

    return result;
}



//=============================================================================
//                              Public Function Definition
//=============================================================================

MMP_INT
DMA_Update(DMA_CTXT* ctxt)
{
    MMP_INT result = 0;

    /**
     * Input check.
     */
    if(!ctxt->dmaType)
    {
        result = ERROR_DMA_TYPE_NOT_SET;
        goto end;
    }
    if(!ctxt->srcAddr)
    {
        result = ERROR_DMA_SRC_ADDR_NOT_SET;
        goto end;
    }
    if(!ctxt->dstAddr)
    {
        result = ERROR_DMA_DST_ADDR_NOT_SET;
        goto end;
    }
    if(!ctxt->totalSize)
    {
        result = ERROR_DMA_TOTAL_SIZE_NOT_SET;
        goto end;
    }
    if(ctxt->onlyUpdateSize) /** Just for MS card performance issue */
    {
        DMA_SetTxSizeReg(ctxt->channel, ctxt->txSize);
        ctxt->onlyUpdateSize = 0;
        goto end;
    }

    /** Update src/dst width and transfer size */
    result = DMA_UpdateTxSize(ctxt);
    if(result)
        goto end;

    result = DMA_UpdateAddrCtrl(ctxt);
    if(result)
        goto end;
    
    result = DMA_UpdateBurstSize(ctxt);
    if(result)
        goto end;

    result = DMA_UpdateCfg(ctxt);
    if(result)
        goto end;

    result = DMA_UpdateHw(ctxt);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "DMA_Update() return error code 0x%08X \n", result LOG_END

    return result;
}
