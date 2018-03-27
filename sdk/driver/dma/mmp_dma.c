/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  DMA Controller extern API implementation.
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "dma/config.h"
#if defined(DMA_IRQ_ENABLE)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#else
#include "intr/intr.h"
#endif
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_BOOL channel[DMA_MAX_CHANNEL] = {0};
static DMA_CTXT context[DMA_MAX_CHANNEL] = {0};

#if defined(WIN32)
    #pragma data_seg(".MYSEC")
    static void	*DMA_Semaphore = MMP_NULL;
    #pragma data_seg()
#endif

#if defined (__FREERTOS__)
  static void *DMA_Semaphore = MMP_NULL;
#endif // __FREERTOS__


//=============================================================================
//                              Public Function Definition
//=============================================================================

#if defined(DMA_IRQ_ENABLE)
typedef struct DMA_ISR_CTXT_TAG {
    dmaIntrHandler isr;
    void* data;
} DMA_ISR_CTXT;

static DMA_ISR_CTXT dmaIsr[DMA_MAX_CHANNEL] = {0};

MMP_INT 
mmpDmaRegisterIsr(MMP_DMA_CONTEXT dmaCtxt, dmaIntrHandler handler, void* arg)
{
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
    dmaIsr[ctxt->channel].isr = handler;
    dmaIsr[ctxt->channel].data = arg;
    ctxt->flags |= DMA_FLAGS_USE_IRQ;
    return 0;
}

static void dma_isr(void* data)
{
    DMA_CTXT* ctx = (DMA_CTXT*)data;
    MMP_UINT32 status, tc, err_abt, intrStatus;
    MMP_UINT32 channel;
#if 0
    AHB_ReadRegister(DMA_REG_INT_STATUS, &status);
    if(!status)
    {
        LOG_DEBUG " strange dma interrupt! \n" LOG_END
        return;
    }
#endif
    AHB_ReadRegister(DMA_REG_INT_TC_STATUS, &tc);
    AHB_ReadRegister(DMA_REG_INT_ERR_ABT_STATUS, &err_abt);
    LOG_DEBUG " status: 0x%08X, tc: 0x%08X, arr_abt: 0x%08X \n", status, tc, err_abt LOG_END
    AHB_WriteRegister(DMA_REG_INT_TC_CLEAR, tc);
    AHB_WriteRegister(DMA_REG_INT_ERR_ABT_CLEAR, err_abt);
        
    for(channel=0; channel<DMA_MAX_CHANNEL; channel++)
    {
	    //if((status>>channel) & 0x1)
        if(((tc>>channel) & 0x1) || (((err_abt>>channel) & 0x1)))
        {
            intrStatus = 0;
            if((tc>>channel) & 0x1)
                intrStatus |= DMA_IRQ_FLAGS_TC;
            if((err_abt>>channel) & 0x1)
                intrStatus |= DMA_IRQ_FLAGS_ERR;
            if((err_abt>>(16+channel)) & 0x1)
                intrStatus |= DMA_IRQ_FLAGS_ABT;

            if(dmaIsr[channel].isr)
            {
                dmaIsr[channel].isr(dmaIsr[channel].data , intrStatus);
            }
        }
    }
    if(err_abt)
        LOG_ERROR " dma error 0x%08X \n\n", err_abt LOG_END

    return;
}

#else
#define mmpDmaRegisterIsr(a,b,c)    
#endif


//=============================================================================
/**
 * DAM Controller Initialization.
 */
//=============================================================================
MMP_INT mmpDmaInitialize(
    void)
{
    MMP_INT result = 0;
    LOG_ENTER "[mmpDmaInitialize] Enter \n" LOG_END

    DMA_DumpFeatureReg();
    DMA_EnableControllerReg();

    if(!DMA_Semaphore)
    {
        DMA_Semaphore = SYS_CreateSemaphore(1, "MMP_DMA");
        if(!DMA_Semaphore)
        {
            result = ERROR_DMA_CREATE_SEMAPHORE_FAIL;
            goto end;
        }
    }

    #if defined(DMA_IRQ_ENABLE)
    {
        MMP_UINT32 value;
        AHB_ReadRegister(DMA_REG_INT_TC_STATUS, &value);
        AHB_WriteRegister(DMA_REG_INT_TC_CLEAR, value);
        AHB_ReadRegister(DMA_REG_INT_ERR_ABT_STATUS, &value);
        AHB_WriteRegister(DMA_REG_INT_ERR_ABT_CLEAR, value);
    }
    /** register interrupt handler to interrupt mgr */
    ithIntrRegisterHandlerIrq(ITH_INTR_DMA, dma_isr, MMP_NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_DMA, ITH_INTR_LEVEL);
    ithIntrSetTriggerLevelIrq(ITH_INTR_DMA, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_DMA);
    #endif

end:
    LOG_LEAVE "[mmpDmaInitialize] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpDmaInitialize() return error code 0x%08X \n", result LOG_END

    return result;
}

//=============================================================================
/**
 * DAM Controller Create channel context.
 */
//=============================================================================
MMP_INT 
mmpDmaCreateContext(
    MMP_DMA_CONTEXT* dmaCtxt)
{
    MMP_INT result = 0;
    MMP_INT i = 0;
    DMA_CTXT* ctxt = MMP_NULL;
    LOG_ENTER "[mmpDmaCreateContext] Enter \n" LOG_END

    SYS_WaitSemaphore(DMA_Semaphore);

    for(i=0; i<DMA_MAX_CHANNEL; i++)
    {
        if(channel[i] == MMP_FALSE)
            break;
    }

    if(i==DMA_MAX_CHANNEL)
    {
        result = ERROR_DMA_NO_AVAILABLE_CHANNEL;
        goto end;
    }

    ctxt = &context[i];
    SYS_MemorySet((void*)ctxt, 0x0, sizeof(DMA_CTXT));
    channel[i] = MMP_TRUE;
    ctxt->channel = i;
    ctxt->csr = DMA_FIFO_TH_8; // TODO

end:
    SYS_ReleaseSemaphore(DMA_Semaphore);
    (*dmaCtxt) = ctxt;

    LOG_INFO " Get channel %d !!! \n", ctxt->channel LOG_END

    LOG_LEAVE "[mmpDmaCreateContext] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpDmaCreateContext() return error code 0x%08X \n", result LOG_END

    return result;
}

MMP_INT 
mmpDmaResetContext(MMP_DMA_CONTEXT dmaCtxt)
{
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
    MMP_UINT8 channel = ctxt->channel;
    SYS_MemorySet((void*)ctxt, 0x0, sizeof(DMA_CTXT));
    ctxt->channel = channel;
    return 0;
}

//=============================================================================
/**
 * DAM Controller Destroy channel context.
 */
//=============================================================================
MMP_INT 
mmpDmaDestroyContext(
    MMP_DMA_CONTEXT dmaCtxt)
{
    MMP_INT result = 0;
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
    LOG_ENTER "[mmpDmaDestroyContext] Enter \n" LOG_END

    SYS_WaitSemaphore(DMA_Semaphore);
    if(!ctxt)
        goto end;

    #if defined(DMA_IRQ_ENABLE)
    dmaIsr[ctxt->channel].isr = MMP_NULL;
    dmaIsr[ctxt->channel].data = MMP_NULL;
    #endif
    channel[ctxt->channel] = MMP_FALSE;
    //SYS_MemorySet((void*)ctxt, 0x0, sizeof(DMA_CTXT));

    AHB_WriteRegisterMask((DMA_REG_C0_CSR + (ctxt->channel*DMA_CHANNEL_OFFSET)), 0x0, DMA_MSK_CHANNEL_EN);

end:
    SYS_ReleaseSemaphore(DMA_Semaphore);

    LOG_LEAVE "[mmpDmaDestroyContext] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpDmaDestroyContext() return error code 0x%08X \n", result LOG_END

    return result;
}

//=============================================================================
/**
 * Set channel attribute.
 */
//=============================================================================
MMP_INT
mmpDmaSetAttrib(
    MMP_DMA_CONTEXT   dmaCtxt,
    const MMP_UINT32*  attribList)
{
    MMP_INT result = 0;
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
    LOG_ENTER "[mmpDmaSetAttrib] Enter \n" LOG_END

    while(*attribList != MMP_DMA_ATTRIB_NONE)
    {
        switch(*attribList++)
        {
        case MMP_DMA_ATTRIB_DMA_TYPE:
            ctxt->dmaType = (MMP_UINT8)*attribList++;
            break;
        case MMP_DMA_ATTRIB_SRC_ADDR:
            ctxt->srcAddr = (MMP_UINT32)*attribList++;
            break;
        case MMP_DMA_ATTRIB_DST_ADDR:
            ctxt->dstAddr = (MMP_UINT32)*attribList++;
            break;
        case MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE:
            ctxt->totalSize = (MMP_UINT32)*attribList++;
            break;
        case MMP_DMA_ATTRIB_SIZE_SET_AGAIN: /** Just for MS card performance issue */
            ctxt->onlyUpdateSize = (MMP_UINT8)*attribList++;
            break;
        case MMP_DMA_ATTRIB_HW_HANDSHAKING:
            if(*attribList++)
                ctxt->flags |= DMA_FLAGS_HW_HANDSHAKING;
            break;
        case MMP_DMA_ATTRIB_SRC_TX_WIDTH:
            ctxt->srcWidth = (MMP_UINT8)*attribList++;
            break;
        case MMP_DMA_ATTRIB_DST_TX_WIDTH:
            ctxt->dstWidth = (MMP_UINT8)*attribList++;
            break;
        case MMP_DMA_ATTRIB_SRC_BURST_SIZE:
            ctxt->burstSize = (MMP_UINT32)*attribList++;
            break;
        case MMP_DMA_ATTRIB_LLD_ADDR:
            ctxt->lldAddr = (MMP_UINT32)*attribList++;
            if(ctxt->lldAddr & 0x3)
            {
                result = ERROR_DMA_LLD_ADDR_NOT_ALIGN;
                goto end;
            }
            break;
        case MMP_DMA_ATTRIB_PRIORITY:
            ctxt->priority = (MMP_UINT8)*attribList++;
            ctxt->csr &= ~DMA_MSK_CHANNEL_PRIORITY;
            ctxt->csr |= (ctxt->priority << DMA_SHT_CHANNEL_PRIORITY);
            break;

        case MMP_DMA_ATTRIB_FIFO_TH:
            ctxt->priority = (MMP_UINT8)*attribList++;
            ctxt->csr &= ~DMA_MSK_FIFO_TH;
            ctxt->csr |= (ctxt->priority << DMA_SHT_FIFO_TH);
            break;
        default:
            result = ERROR_DMA_UNKONWN_ATTRIB;
            goto end;
        }
    };

    result = DMA_Update(ctxt);
    if(result)
        goto end;

end:
    LOG_LEAVE "[mmpDmaSetAttrib] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpDmaSetAttrib() return error code 0x%08X \n", result LOG_END

    return result;
}

//=============================================================================
/**
 * Fire related channel.
 */
//=============================================================================
MMP_INT
mmpDmaFire(
    MMP_DMA_CONTEXT   dmaCtxt)
{
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
    #if !defined(WIN32) 
    #if IT9070
    if ((ctxt->flags & DMA_FLUSH_SRC_FLAG) == DMA_FLUSH_SRC_FLAG)
    {
    ithFlushDCacheRange((void*) ctxt->srcAddr, ctxt->totalSize);
        ithFlushMemBuffer();
    }
    if ((ctxt->flags & DMA_FLUSH_DST_FLAG) == DMA_FLUSH_DST_FLAG)
    {
    ithFlushDCacheRange((void*) ctxt->dstAddr, ctxt->totalSize);
        ithFlushMemBuffer();
    }
    #endif
    #endif
    DMA_FireReg(ctxt->channel, ctxt->csr);

    return 0;
}

//=============================================================================
/**
 * Wait channel idle.
 */
//=============================================================================
MMP_INT
mmpDmaWaitIdle(
    MMP_DMA_CONTEXT   dmaCtxt)
{
    MMP_INT result = 0;
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
    LOG_ENTER "[mmpDmaWaitIdle] Enter \n" LOG_END

    result = DMA_WaitChannelReadyReg(ctxt);
    if(result)
        goto end;

end:
    LOG_LEAVE "[mmpDmaWaitIdle] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpDmaWaitIdle() return error code 0x%08X \n", result LOG_END

    return result;
}

MMP_BOOL
mmpDmaIsIdle(
    MMP_DMA_CONTEXT   dmaCtxt)
{
    return DMA_IsChannelReadyReg((DMA_CTXT*)dmaCtxt);
}

MMP_INT
mmpDmaBusyWaitIdle(
    MMP_DMA_CONTEXT   dmaCtxt)
{
    MMP_INT result = 0;
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;

    LOG_ENTER "[mmpDmaBusyWaitIdle] Enter \n" LOG_END

    result = DMA_BusyWaitChannelReadyReg(ctxt);
    if(result)
        goto end;

end:
    LOG_LEAVE "[mmpDmaBusyWaitIdle] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpDmaWaitIdle() return error code 0x%08X \n", result LOG_END

    return result;

}


void
mmpDmaDumpReg(
    MMP_DMA_CONTEXT   dmaCtxt)
{
    DMA_CTXT* ctxt = (DMA_CTXT*)dmaCtxt;
	MMP_UINT8 ch = ctxt->channel;
	MMP_UINT32 reg=0, value=0;

	reg = DMA_REG_INT_TC_STATUS+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => tc\n", reg, value LOG_END
	reg = DMA_REG_INT_ERR_ABT_STATUS+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => err\n", reg, value LOG_END

	reg = DMA_REG_C0_CSR+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => csr\n", reg, value LOG_END
	reg = DMA_REG_C0_CFG+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => cfg\n", reg, value LOG_END

	reg = DMA_REG_C0_SRC_ADDR+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => src addr\n", reg, value LOG_END
	reg = DMA_REG_C0_DST_ADDR+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => dst addr\n", reg, value LOG_END
	reg = DMA_REG_C0_TX_SIZE+(ch*DMA_CHANNEL_OFFSET);
	AHB_ReadRegister(reg, &value);
	LOG_INFO " Reg 0x%08X = 0x%08X => tx size\n", reg, value LOG_END
}
