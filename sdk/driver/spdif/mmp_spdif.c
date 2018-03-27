/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file mmp_spdif.c
 * API to control SPDIF module.
 * @author Steven Hsiao.
 * @version 0.1
 */

#include "pal/pal.h"

#include "host/ahb.h"
#include "host/gpio.h"
#include "mmp_i2s.h"
#include "mmp_spdif.h"
#include "mmp_dma.h"

//#define ENABLE_SPDIF_HW_BUG_WORK_AROUND
#define AHB_SPI_DATA_BASE	    0xDE900018  //TODO
#define AHB_SPI_DATA_BASE_COUNT	0xDE900064

#define SSP_BASE SSP2_BASE

//=============================================================================
//                              Extern Reference
//=============================================================================

#ifdef _WIN32
static MMP_UINT32 gpio_spdif = DGPIO(9); // DGPIO(5)
#else
extern MMP_UINT32 gpio_iic;
#endif

typedef struct SPDIF_CTXT_TAG
{
    MMP_BOOL            bActive;
    SPDIF_MODE          mode;
    MMP_UINT32          samplingReate;
    MMP_UINT32          divider;
    MMP_UINT32          inputSampleSize;
    MMP_UINT8*          pDmaBuffer;
    MMP_UINT32          perDmaBufferSize;
    MMP_UINT32          dmaBufferCount;
    MMP_UINT32          dmaBufferIndex;
    SPDIF_DMA_CALLBACK  pfApDmaCallback;
    MMP_DMA_CONTEXT     dmaCtxt;
} SPDIF_CTXT;

static SPDIF_CTXT               gtSpdifContext = { 0 };

MMP_INT32 result = 0;
static MMP_UINT32 gAttribList[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE,            (MMP_UINT32)MMP_DMA_TYPE_SPI2_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR,            (MMP_UINT32)(AHB_SPI_DATA_BASE),
    MMP_DMA_ATTRIB_DST_ADDR,            (MMP_UINT32)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE,  0,
    MMP_DMA_ATTRIB_HW_HANDSHAKING,      (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH,        2,//for 32bit (20,24)
    MMP_DMA_ATTRIB_DST_TX_WIDTH,        2,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE,      8,
    MMP_DMA_ATTRIB_NONE
};
//=============================================================================
//                              Private Function
//=============================================================================

static void SetPadSel(void)
{
	MMP_UINT32 data;
	
	//TODO ssp clock select
	AHB_ReadRegister(GPIO_BASE + 0xD0, &data);
	data |= (1 << 7);
	AHB_WriteRegister(GPIO_BASE + 0xD0, data);

	AHB_ReadRegister(GPIO_BASE + 0x90, &data);
	data &= ~(3 << 18);
	data |= (0 << 19) | (1 << 18);
	AHB_WriteRegister(GPIO_BASE + 0x90, data);
}

static void
SpdifRxDmaIsr(
	void*      arg,
	MMP_UINT32 status)
{
    MMP_INT32 result = 0;
    MMP_UINT32 i = 0;
    MMP_UINT32 j = 0;
    MMP_UINT8* pNewWritePos;
    gtSpdifContext.dmaBufferIndex = (gtSpdifContext.dmaBufferIndex + 1) % gtSpdifContext.dmaBufferCount;
    
    if (gtSpdifContext.pfApDmaCallback)
    {
        (*gtSpdifContext.pfApDmaCallback)(gtSpdifContext.dmaBufferIndex * gtSpdifContext.perDmaBufferSize);
    }
    pNewWritePos = gtSpdifContext.pDmaBuffer + (gtSpdifContext.dmaBufferIndex * gtSpdifContext.perDmaBufferSize);
    gAttribList[5] = (MMP_UINT32) pNewWritePos;
    result = mmpDmaSetAttrib(gtSpdifContext.dmaCtxt, gAttribList);
    mmpDmaFire(gtSpdifContext.dmaCtxt);
}
//=============================================================================
/**
 * Initialize the SPDIF module.
 * @param none.
 * @return none
 */
//=============================================================================
void
mmpSpdifInitialize(
    void)
{
    MMP_UINT32 reg = 0;
    MMP_INT32 result = 0;
    MMP_UINT8* pBuffer = MMP_NULL;
    MMP_UINT32 i = 0;
    MMP_DMA_CONTEXT tDmaContext = { 0 };

    gtSpdifContext.bActive = MMP_FALSE;
    gtSpdifContext.mode = SPDIF_LINEAR_PCM;
    gtSpdifContext.samplingReate = 48000;

    //padSel
	SetPadSel();

    AHB_WriteRegisterMask(SSP_BASE + SSP_CONTROL_REG0, (0x1 << 16) | (0x5 << 12) | (0x0 << 2) | (0x0 << 3), (0x1 << 16) | (0x5 << 12) | (0x1 << 2) | (0x1 << 3));   
    AHB_WriteRegisterMask(SSP_BASE + SSP_INT_CONTROL_REG, (0x1 << 4), (0x1 << 4));
    AHB_WriteRegisterMask(SSP_BASE + SPDIF_EXTRA_CONTROL_REG, (0x1 << 0), (0x1 << 0));
    AHB_WriteRegisterMask(SSP_BASE + SPDIF_IO_CONTROL_REG, (0x0 << 0), (0x1 << 0));

    mmpDmaCreateContext(&gtSpdifContext.dmaCtxt);
    mmpDmaRegisterIsr(gtSpdifContext.dmaCtxt, SpdifRxDmaIsr, NULL);
#if 0
    result = mmpDmaSetAttrib(gtSpdifContext.dmaCtxt, gAttribList);

    		{
		STRC_I2S_SPEC spec;
		memset((void*)&spec, 0, sizeof(STRC_I2S_SPEC));
		spec.use_hdmirx         = 0;
		spec.internal_hdmirx    = 1;
		spec.slave_mode         = 0;
		spec.channels           = 2;
		spec.sample_rate        = 48000;
		spec.buffer_size        = 65536;
		spec.is_big_endian      = 0;
		spec.sample_size        = 16;
		spec.base_out_i2s_spdif     = pBuffer;
		spec.postpone_audio_output  = 0;
		i2s_init_DAC(&spec);
    }
	I2S_DA32_SET_WP(0);
    mmpDmaFire(gtSpdifContext.dmaCtxt);
#endif
}

//=============================================================================
/**
 * Terminate the SPDIF module.
 *
 * @param none.
 * @return none
 */
//=============================================================================
void
mmpSpdifTerminate(
    void)
{
    MMP_UINT32 busyLoop = 0;
    //while (!mmpSpdifWaitEngineIdle())
    //    for (busyLoop = 0; busyLoop < 2048; busyLoop++) asm("");
    mmpSpdifSetEngineState(MMP_FALSE);
    
    //dma terminate
    if(gtSpdifContext.dmaCtxt)
    {
        mmpDmaDestroyContext(gtSpdifContext.dmaCtxt);
    }

    memset(&gtSpdifContext, 0x0, sizeof(SPDIF_CTXT));
}

//=============================================================================
/**
 * Setup the SPDIF attribute.
 *
 * @param mode SPDIF mode - Linear PCM or Compression Data (Non Linear PCM).
   @param samplingRate The audio data sampling rate.
 * @return none
 */
//=============================================================================
MMP_UINT32
mmpSpdifSetAttribute(
    const MMP_UINT32*  attribList)
{
    MMP_UINT32 result = 0;
    MMP_UINT16 regVal = 0;
    MMP_UINT32 allowBit = 0;

    while(*attribList != MMP_SPDIF_ATTRIB_NONE)
    {
        //printf("type: %u\n", *attribList);
        switch(*attribList++)
        {
            case MMP_SPDIF_PER_DMA_BLOCK_BUFFER_SIZE:
                gtSpdifContext.perDmaBufferSize = *attribList++;
                //printf("%s(%d): %u\n", __FILE__, __LINE__, gtSpdifContext.perDmaBufferSize);
                break;
            case MMP_SPDIF_BUFFER_COUNT:
                gtSpdifContext.dmaBufferCount = *attribList++;
                //printf("%s(%d): %u\n", __FILE__, __LINE__, gtSpdifContext.dmaBufferCount);
                break;
            case MMP_AP_DMA_FUNCTION_PTR:
                gtSpdifContext.pfApDmaCallback = (SPDIF_DMA_CALLBACK) *attribList++;
                //printf("%s(%d): 0x%X\n", __FILE__, __LINE__, gtSpdifContext.pfApDmaCallback);
                break;
            case MMP_SPDIF_INPUT_IO:
                switch(*attribList)
                {
                    case Z2_SPDIF:
                    case Z3_SPDIF:
                    case PMCLK_SPDIF:
                    case IO_SPDIF:
                        AHB_WriteRegisterMask(SSP_BASE + SPDIF_EXTRA_CONTROL_REG, (*attribList << 8), (*attribList << 8));                        
                        break;
                    default:                
                        result = SPDIF_WRONG_IO_INPUT;
                        goto end;
                }
                //printf("%s(%d): %u\n", __FILE__, __LINE__, *attribList);                
                attribList++;
                break;
            case MMP_SPDIF_INPUT_SAMPLE_SIZE:
                gtSpdifContext.inputSampleSize = *attribList++;
                //printf("%s(%d): %u\n", __FILE__, __LINE__, gtSpdifContext.inputSampleSize);   
                if (gtSpdifContext.inputSampleSize > 24)
                {
                    result = SPDIF_SAMPLE_SIZE_ERR;
                    goto end;
                }
                break;
            case MMP_SPDIF_FORCE_16BIT:
                AHB_WriteRegisterMask(SSP_BASE + SSP_CONTROL_REG1, (0x0F << 16), (0x1F << 16));
                attribList++;
                break;
            case MMP_SPDIF_BIT_ENDIAN_SWAP:
                AHB_WriteRegisterMask(SSP_BASE + SPDIF_EXTRA_CONTROL_REG, (0x1 << 5), (0x1 << 5));
                attribList++;
                break;
            default:
                result = SPDIF_WRONG_ATTRIBUTE;
                goto end;
        }
    };
    
    if (0 == gtSpdifContext.perDmaBufferSize)
    {
        result = SPDIF_BUFFER_SIZE_ERR;
        goto end;
    }
    if (0 == gtSpdifContext.dmaBufferCount)
    {
        result = SPDIF_BUFFER_COUNT_ERR;
        goto end;
    }
    
    if (gtSpdifContext.pDmaBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, gtSpdifContext.pDmaBuffer);
        gtSpdifContext.pDmaBuffer = MMP_NULL;
    }

    gtSpdifContext.pDmaBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, gtSpdifContext.perDmaBufferSize * gtSpdifContext.dmaBufferCount);
    if (gtSpdifContext.pDmaBuffer)
    {
        PalMemset(gtSpdifContext.pDmaBuffer, 0x0, gtSpdifContext.perDmaBufferSize * gtSpdifContext.dmaBufferCount);
        allowBit = (regVal >> 16) & 0x1F;
        if (allowBit == 16 || gtSpdifContext.inputSampleSize == 16)
        {
            gAttribList[11] = gAttribList[13] = 2;
        }
        else
        {
            gAttribList[11] = gAttribList[13] = 4; 
        }
        gAttribList[5] = (MMP_UINT32) gtSpdifContext.pDmaBuffer;
        gAttribList[7] = gtSpdifContext.perDmaBufferSize;
        AHB_WriteRegisterMask(SSP_BASE + SSP_CONTROL_REG2, (0x1 << 0), (0x1 << 0));
        mmpDmaSetAttrib(gtSpdifContext.dmaCtxt, gAttribList);
        mmpDmaFire(gtSpdifContext.dmaCtxt);
    }
    else
    {
        result = SPDIF_ALLOC_FAIL;
    }
end:
    //printf("result: 0x%X\n", result);
    return result;
}

//=============================================================================
/**
 * Fire or Disable the SPDIF engine.
 *
 * @param bEnable Whether fire the SPDIF Engine.
 * @return none
 */
//=============================================================================
void
mmpSpdifSetEngineState(
    MMP_BOOL bEnable)
{
    //while (!mmpSpdifWaitEngineIdle())
    //{
    //    PalSleep(1);
    //}
    MMP_UINT32 regVal = 0;
    MMP_BOOL   bEngineEnable = MMP_FALSE;
    AHB_ReadRegister(SSP_BASE + SSP_CONTROL_REG2, &regVal);
    if (regVal & 0x1)
    {
        bEngineEnable = MMP_TRUE;
    }
    else
    {
        bEngineEnable = MMP_FALSE;
    }

    if (bEnable == bEngineEnable)
    {
        return;
    }
    AHB_WriteRegisterMask(SSP_BASE + SSP_CONTROL_REG2, (0x0 << 0), (0x1 << 0));
    
    if (bEnable)
    {
        AHB_WriteRegisterMask(SSP_BASE + SSP_CONTROL_REG2, (0x1 << 0), (0x1 << 0));
    }
}

//=============================================================================
/**
 * Whether SPDIF Engine Idle or not.
 * @param none.
 * @return Whether the SPDIF Engine Idle.
 */
//=============================================================================   
MMP_BOOL
mmpSpdifWaitEngineIdle(
    void)
{
    MMP_UINT32 reg = 0;
    AHB_ReadRegister(SSP_BASE + SSP_STATUS_REG, &reg);
    // Busy bit 2
    if ((reg & SSP_BUSY_BIT))
        return MMP_FALSE;
    else
        return MMP_TRUE;
}

MMP_UINT32
mmpSpdifGetCurrentWriteIndex(
    void)
{
    return (gtSpdifContext.perDmaBufferSize * gtSpdifContext.dmaBufferIndex);
}

MMP_UINT8*
mmpSpdifGetStartBuffer(
    void)
{
    return gtSpdifContext.pDmaBuffer;
}
