//=============================================================================
//                              Include Files
//=============================================================================

#include <stdio.h>
#include "ssp/ssp_reg.h"
#include "ssp/ssp_error.h"
#include "mem/mem.h"
#include "host/host.h"
#include "host/ahb.h"
#include "sys/sys.h"
#include "mmp_dma.h"
#if defined (__OPENRTOS__)
#elif defined (__FREERTOS__)
#include "or32.h"
#endif
#include "mmp_spi.h"
#include "mmp_util.h"

//=============================================================================
//                              Macro
//=============================================================================
//#define SPI_DMA_ISR

//=============================================================================
//                              Macro
//=============================================================================
#define SWAP_ENDIAN16  MMP_SWAP_ENDIAN16
   
//=============================================================================
//                              Constant Definition
//=============================================================================
#define SSP_DMA_TIMEOUT			5000
#define SSP_POLLING_COUNT       (0x10000)
#define SSP_DMA_TEMPBUF         2060

#define AHB_SPI1_DATA_BASE	0xDE800018
#define AHB_SPI2_DATA_BASE	0xDE900018
#define AHB_SPI_DATA_BASE	AHB_SPI1_DATA_BASE
#define FREERUN_SHIFT 16
#define SSP_DEFAULT_DIV     0
#define SSP_DEFAULT_FIFO_LENGTH 31
#define SSP_DEFAULT_RX_THRESHOLD    8
#define SSP_DEFAULT_TX_THRESHOLD    8

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_UINT8            g_initCount[SPI_PORT_MAX];
static SPI_CONTEXT          g_SpiContext[SPI_PORT_MAX];
static MMP_DMA_CONTEXT      g_spiTxDmaCtxt;
static MMP_DMA_CONTEXT      g_spiRxDmaCtxt;
#ifdef SPI_DMA_ISR
static MMP_EVENT			SpiTxIsrEvent = MMP_NULL;
static MMP_EVENT			SpiRxIsrEvent = MMP_NULL;
#endif

static MMP_UINT32 spidmaReadAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE,			(MMP_UINT32)MMP_DMA_TYPE_SPI2_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR,			AHB_SPI_DATA_BASE,        //FOR WIN32 
    MMP_DMA_ATTRIB_DST_ADDR,			(MMP_UINT32)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE,	(MMP_UINT32)1024,//TODO
    MMP_DMA_ATTRIB_HW_HANDSHAKING,		(MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH,		1, //TODO
	MMP_DMA_ATTRIB_DST_TX_WIDTH,		1,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE,		1,
	MMP_DMA_ATTRIB_FIFO_TH,				4,
    MMP_DMA_ATTRIB_NONE
};

static MMP_UINT32 spidmaWriteAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_SPI2,
    MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_DST_ADDR, AHB_SPI_DATA_BASE,    //FOR WIN32 
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)1024,//TODO
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 1, //TODO
	MMP_DMA_ATTRIB_DST_TX_WIDTH, 1,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 8,
    MMP_DMA_ATTRIB_NONE
};

//=============================================================================
//                              Private Function Definition
//=============================================================================
#ifdef SPI_DMA_ISR
static void
SpiTxDmaIsr(
	void*      arg,
	MMP_UINT32 status)
{
	SYS_SetEventFromIsr(SpiTxIsrEvent); 
}

static void
SpiRxDmaIsr(
	void*      arg,
	MMP_UINT32 status)
{
	SYS_SetEventFromIsr(SpiRxIsrEvent); 
}
#endif

static void
InitRegs(
    SPI_PORT port)
{

    MMP_INT format = 0;
    
    switch(g_SpiContext[port].format)
    {
        case CPO_0_CPH_0:
            format = REG_BIT_CPOL_LO | REG_BIT_CPHA_ACTIVE;
            break;
        case CPO_1_CPH_0:
            format = REG_BIT_CPOL_HI | REG_BIT_CPHA_ACTIVE;
            break;
        case CPO_0_CPH_1:
            format = REG_BIT_CPOL_LO | REG_BIT_CPHA_HALF_ACTIVE;
            break;
        case CPO_1_CPH_1:
            format = REG_BIT_CPOL_HI | REG_BIT_CPHA_HALF_ACTIVE;
            break;
    }


	AHB_WriteRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, REG_BIT_CS_ACTIVE_LOW | REG_BIT_FFMT_SPI | REG_BIT_MASTER_MODE | format);
    AHB_WriteRegister(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, SSP_DEFAULT_DIV | (SSP_DEFAULT_FIFO_LENGTH << REG_SHIFT_SERIAL_DATA_LEN));
	AHB_WriteRegisterMask(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3));
    AHB_WriteRegister(REG_SSP_INTR_CONTROL + port*REG_SSP_PORT_OFFSET, 
		(SSP_DEFAULT_TX_THRESHOLD << REG_SHIFT_TX_THRESHOLD) | 
        (SSP_DEFAULT_RX_THRESHOLD << REG_SHIFT_RX_THRESHOLD) | 
        REG_BIT_RX_INTR_OR_EN |
        REG_BIT_TX_INTR_UR_EN |
        REG_BIT_RX_INTR_TH_EN | 
        REG_BIT_TX_INTR_TH_EN);
        
    // Test Master/Salve
    //HOST_WriteRegister(0xA000, 0x1020);
    //HOST_WriteRegister(0x9C00, 0x1028);

}


static MMP_INT
WaitContollerReady(
    SPI_PORT port)
{

    MMP_UINT32 temp;
    MMP_INT result = ERROR_SSP_TRANSMIT_TIMEOUT;
   	MMP_UINT j = SSP_POLLING_COUNT;
  
    while(--j)
    {
        AHB_ReadRegister(REG_SSP_STATUS + port*REG_SSP_PORT_OFFSET, &temp);
        if((temp & REG_BIT_IS_BUSY) == 0)
        {
            result = 0;
            break;
        }
    }
   
    return result;
}


static MMP_INT
WaitFifoReady(
    SPI_PORT port,
    MMP_UINT32 count)
{

    MMP_UINT32 temp;
    MMP_INT result = ERROR_SSP_FIFO_READY_TIMEOUT;
   	MMP_UINT j = SSP_POLLING_COUNT;
    
    while(--j)
    {
        AHB_ReadRegister(REG_SSP_STATUS + port*REG_SSP_PORT_OFFSET, &temp);
        if(((temp & REG_MASK_RX_FIFO_VALID_ENTRIES) >> REG_SHIFT_RX_FIFO_VALID_ENTRIES) >= count)
        {
            result = 0;
            break;
        }
    }
    
    return result;
}

static void
SetPadSel(
	SPI_PORT port)
{
    MMP_UINT32 data;

    switch(port)
    {
        case SPI_1:
            AHB_ReadRegister(GPIO_BASE + 0xD0,&data);
        	//SSP CLK SEL
        	data |= 0x0080;
        	AHB_WriteRegister(GPIO_BASE + 0xD0,data);
            break;
        case SPI_2:
        	AHB_ReadRegister(GPIO_BASE + 0xD0,&data);
        	//SSP CLK SEL
        	data |= 0x0080;
        	AHB_WriteRegister(GPIO_BASE + 0xD0,data);
            break;    
    }
}

static MMP_INT
SPI_Initial(
    SPI_PORT port)
{

    MMP_INT result = 0;
    
    switch(port)
    {
        case SPI_1:
		case SPI_2:	//TODO
            g_SpiContext[port].format = CPO_0_CPH_0;
            g_SpiContext[port].mode = SPI_MODE_0;
            
            //spi 1/2 share dma channel
        	if(g_spiTxDmaCtxt == MMP_NULL)
        	{
        		result = mmpDmaCreateContext(&g_spiTxDmaCtxt);
        		if(result || g_spiTxDmaCtxt == MMP_NULL)
        		{
        			printf("%s[%d]\n", __FUNCTION__, __LINE__);
        			result = ERROR_SSP_CREATE_DMA_FAIL;
        			goto end;
        		}
#ifdef SPI_DMA_ISR
        		mmpDmaRegisterIsr(g_spiTxDmaCtxt, SpiTxDmaIsr, NULL);
        		if( SpiTxIsrEvent == MMP_NULL )
        		{
				    SpiTxIsrEvent = SYS_CreateEvent();
				}
#endif
        	}
        
        	if(g_spiRxDmaCtxt == MMP_NULL)
        	{
        		result = mmpDmaCreateContext(&g_spiRxDmaCtxt);
        		if(result || g_spiRxDmaCtxt == MMP_NULL)
        		{
        			result = ERROR_SSP_CREATE_DMA_FAIL;
        			goto end;
        		}
#ifdef SPI_DMA_ISR
        		mmpDmaRegisterIsr(g_spiRxDmaCtxt, SpiRxDmaIsr, NULL);
        		if( SpiRxIsrEvent == MMP_NULL )
        		{
				    SpiRxIsrEvent = SYS_CreateEvent();
				}
#endif
        	}
                
            InitRegs(port);
            break;
    }
  
end:
    return result;
}


MMP_INT
mmpSpiInitialize(
    SPI_PORT port)
{
    MMP_INT result = 0;
    
    switch(port)
    {
        case SPI_1:
            if(g_initCount[SPI_1] == 0)
        	{
        		SetPadSel(SPI_1);
                result = SPI_Initial(SPI_1);
        	}
        
            g_initCount[SPI_1]++;

            //HOST_WriteRegister(0x7C92, 0x0055);
            //HOST_WriteRegister(0x7C94, 0x5500);

			printf("%s[%d]\n", __FUNCTION__, __LINE__);
            //HOST_WriteRegister( 0x7c90, 0x4000);
			//HOST_WriteRegister( 0x7c92, 0x0054);

			AHB_WriteRegisterMask(0xDE000090, 0x00544000, 0x00FCC000);
            break;
        case SPI_2:
            if(g_initCount[SPI_2] == 0)
        	{
        		SetPadSel(SPI_2);
                result = SPI_Initial(SPI_2);
        	}
            g_initCount[SPI_2]++;
            //HOST_WriteRegister(0x7C92, 0x0055);
            //HOST_WriteRegister(0x7C94, 0x5500);
			printf("%s[%d]\n", __FUNCTION__, __LINE__);
            //HOST_WriteRegister( 0x7c90, 0x4000);
			//HOST_WriteRegister( 0x7c92, 0x0054);
			AHB_WriteRegisterMask(0xDE000090, 0x00544000, 0x00FCC000);
            break;    
            
    }

    // Frequency Divider
	AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, 2, REG_MASK_SCLK_DIV);

    return result;
}

MMP_INT
mmpSpiTerminate(
    SPI_PORT port)
{
    MMP_INT result = 0;
    
    switch(port)
    {
    case SPI_1:
        if((--g_initCount[SPI_1]) == 0)
        {
            
            MMP_RESULT result = 0;
            //STOP SSP
            AHB_WriteRegister(REG_SSP_CONTROL_2, REG_BIT_TXFCLR_EN);
        }
        break;
        
    case SPI_2:
        if((--g_initCount[SPI_2]) == 0)
        {
            
            MMP_RESULT result = 0;
            //STOP SSP
            AHB_WriteRegister(REG_SSP_CONTROL_2 + REG_SSP_PORT_OFFSET, REG_BIT_TXFCLR_EN);
        }
        break;    
        
    }

#ifdef SPI_DMA_ISR
    if (   g_initCount[SPI_1] == 0
        && g_initCount[SPI_2] == 0 )
    {
    	if ( SpiTxIsrEvent ) { SYS_DelEvent(SpiTxIsrEvent); }
	    if ( SpiRxIsrEvent ) { SYS_DelEvent(SpiRxIsrEvent); }
	}
#endif
    
    return result;
}

MMP_INT
mmpSpiDmaWrite(
    SPI_PORT port,
    MMP_UINT8 *inputData,
    MMP_INT   inputSize,
    MMP_UINT8* psrc,
    MMP_INT size,
    MMP_UINT8 dataLength)
{
    MMP_INT result = -1;
    MMP_UINT8 *tempBuf = MMP_NULL;
    MMP_UINT32 totalSize = size + inputSize;
    MMP_UINT8 dataLen = dataLength - 1;
	MMP_UINT32 data, srcWidth, dstWidth,burstSize;
   	MMP_UINT32 tx_threshold, rx_threshold;

    SetPadSel(port);
	AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET, 0);
    
    tempBuf = (MMP_UINT8 *)MEM_Allocate(2060 , MEM_USER_NOR);
	if(tempBuf == MMP_NULL)
	{
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}
    if(g_spiTxDmaCtxt)
    {
        //TX channel
        //write control data
#if defined(_WIN32)
        HOST_WriteBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)inputData, inputSize);
        HOST_WriteBlockMemory((MMP_UINT32)(tempBuf + inputSize), (MMP_UINT32)psrc, size);
#else
		{
			unsigned int i = 0;
			
        	for(i = 0; i < inputSize; ++i)
			{
            	*(tempBuf+ i) = *(inputData + i);
			}
		}

        memcpy(tempBuf + inputSize, psrc, size);
#endif
		//32bits
		if((totalSize & 0x3) == 0)
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 4;  
			dstWidth = 4;
			burstSize = 4;
			dataLen = 31;
			//big enddian
			AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &data);
			data |= REG_BIT_TXFIFO_BIG_END;
			AHB_WriteRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET,data);			
		}
		else if((totalSize & 0x1) == 0) //16bits
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 2;  
			dstWidth = 2;
			burstSize = 4;
			dataLen = 15;
			//big enddian
			AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &data);
			data |= REG_BIT_TXFIFO_BIG_END;
			AHB_WriteRegister(REG_SSP_CONTROL_0+ port*REG_SSP_PORT_OFFSET,data);

		}
		else					//8bits
		{
			tx_threshold = 8;
			rx_threshold = 8;
			srcWidth = 1; 
			dstWidth = 1;
			burstSize = 8;
			dataLen = 7;
		}
		
		switch(port)
		{
		    case SPI_1:
		        spidmaWriteAttrib[1] = MMP_DMA_TYPE_MEM_TO_SPI;	//TODO
		        break;
            case SPI_2:
                spidmaWriteAttrib[1] = MMP_DMA_TYPE_MEM_TO_SPI2;//MMP_DMA_TYPE_MEM_TO_SPI2;
		        break;
		}
		
        spidmaWriteAttrib[3] = (MMP_UINT32)tempBuf;  /** source address */

		switch(port)
		{
		    case SPI_1:
		        spidmaWriteAttrib[5] = AHB_SPI1_DATA_BASE;
		        break;
            case SPI_2:
                spidmaWriteAttrib[5] = AHB_SPI2_DATA_BASE;
		        break;
		}
		
        spidmaWriteAttrib[7] = totalSize;             /** total size */
		spidmaWriteAttrib[11] = srcWidth;             /** source width */
		spidmaWriteAttrib[13] = dstWidth;             /** destination width */
		spidmaWriteAttrib[15] = burstSize;             /** burst width */
        
        result = mmpDmaSetAttrib(g_spiTxDmaCtxt, spidmaWriteAttrib);
        if(result)
            goto end;
            
        //clear tx FIFO
        AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
		
		//Free run
		AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,REG_BIT_FR_CSN_CLR | REG_BIT_FR_RX_LOCK);

        result = mmpDmaFire(g_spiTxDmaCtxt);
        if(result)
           goto end;
           

	    AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

        //SSP dma enable
        AHB_WriteRegister(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET, 
        	((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
            (rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_INTR_OR_EN |
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_TH_EN |
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_TX_DMA_EN));

        //Fire SSP
        AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
#ifdef SPI_DMA_ISR
		result = SYS_WaitEvent(SpiTxIsrEvent, SSP_DMA_TIMEOUT);
		if(result)
		{
            goto end;
        }
#else
        result = mmpDmaWaitIdle(g_spiTxDmaCtxt);
        if(result)
       	{
            goto end;
        }
#endif

        //wait ssp ready
        result = WaitContollerReady(port);

    }
    
end:
    MEM_Release(tempBuf);

    //SSP dma disable
    AHB_WriteRegister(
		REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET, 
		((SSP_DEFAULT_TX_THRESHOLD << REG_SHIFT_TX_THRESHOLD) |
		(SSP_DEFAULT_RX_THRESHOLD << REG_SHIFT_RX_THRESHOLD) |
		REG_BIT_RX_INTR_OR_EN |
		REG_BIT_TX_INTR_UR_EN |
		REG_BIT_RX_INTR_TH_EN | 
		REG_BIT_TX_INTR_TH_EN));

	//reset enddian
	AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &data);
	data &= ~REG_BIT_TXFIFO_BIG_END;
	AHB_WriteRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET,data);

       

	//Free run
	AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,0x0);

    return result;

}

MMP_INT
mmpSpiDmaRead(
    SPI_PORT port,
    MMP_UINT8 *inputData,
    MMP_INT   inputSize,
    void* pdes,
    MMP_INT size,
    MMP_UINT8 dataLength)

{
    MMP_INT result = -1;
    MMP_UINT8 *tempBuf = MMP_NULL;
    MMP_UINT8 *tempRxBuf = MMP_NULL;
    MMP_UINT32 totalSize = size + inputSize;
    MMP_UINT8 dataLen = dataLength - 1;
	MMP_UINT32 data, i;
	MMP_UINT32 srcWidth, dstWidth,burstSize;
   	MMP_UINT32 tx_threshold, rx_threshold;

    MMP_UINT32 c = SSP_POLLING_COUNT;

    tx_threshold = 1;
    rx_threshold = 1;


    SetPadSel(port);

#if defined(_WIN32)
    tempRxBuf = (MMP_UINT8 *)MEM_Allocate(SSP_DMA_TEMPBUF , MEM_USER_NOR);
    if(tempRxBuf == MMP_NULL)
	{
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}
#elif defined(__OPENRTOS__) || defined(__FREERTOS__)
	tempRxBuf = (MMP_UINT8 *)malloc(SSP_DMA_TEMPBUF);
    if(tempRxBuf == MMP_NULL)
	{
		printf("%s[%d]ERROR!!!\n", __func__, __LINE__);
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}
#endif
    if(g_spiRxDmaCtxt)
    {
		//32bits
		if((totalSize&0x3) == 0)
		{
			MMP_UINT32 value;
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 4;  
			dstWidth = 4;
			burstSize = 1;
			dataLen = 31;
			
			//TX channel
			AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,0);
			AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET,  REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
            AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
			//big enddian
			AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &data);
#if defined(_WIN32) || defined(__OPENRTOS__)
			data |= REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END;
#elif defined(__FREERTOS__)
			data |= REG_BIT_RXFIFO_BIG_END;
#endif
			AHB_WriteRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET,data);			

			//free run
			{
				MMP_INT freerunCount = 0;
				
				freerunCount = size / ((dataLen + 1) / 8) + 31;
				data = (freerunCount << FREERUN_SHIFT) | 0x06; 
				AHB_WriteRegister(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);

				for(i=0;i<inputSize/4; ++i)
				{
					value = *((MMP_UINT32*)inputData + i);
					AHB_WriteRegister(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, value);
				}

				data = (freerunCount << FREERUN_SHIFT) | 0x07; 
				AHB_WriteRegister(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data); //awin
			}
		}
		else if((totalSize&0x1) == 0)		//16bits
		{
			MMP_UINT32 value;
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 2;  
			dstWidth = 2;
			burstSize = 1;
			dataLen = 15;

			//TX channel
			AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET, 0);
			AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET,  REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
            AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
			//big enddian
			AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &data);
			//data |= REG_BIT_RXFIFO_BIG_END;
#if defined(_WIN32) || defined(__OPENRTOS__)
			data |= REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END;
#elif defined(__FREERTOS__)
			data |= REG_BIT_RXFIFO_BIG_END;
#endif
			AHB_WriteRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET,data);

			for(i=0;i<inputSize/2; ++i)
			{
				value = *((MMP_UINT16*)inputData + i);
				AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, value);
			}
			
			//free run
			{
				MMP_INT freerunCount = 0;

				freerunCount = size / ((dataLen + 1) / 8) + 31;
				data = (freerunCount << FREERUN_SHIFT) | 0x06; 
				AHB_WriteRegister(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);

				data = (freerunCount << FREERUN_SHIFT) | 0x07; 
				AHB_WriteRegister(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);
			}
		}
		else				//8bits
		{
			tx_threshold = 1;
			rx_threshold = 1;
			srcWidth = 1; 
			dstWidth = 1;
			burstSize = 1;
			dataLen = 7;
			
			//TX channel
			//clear rx tx FIFO
			AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,0);
			AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET,  REG_BIT_TXFCLR_EN | REG_BIT_RXFCLR_EN);
            AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

			data = ((size + 31)<< FREERUN_SHIFT)  | 0x06; 
			AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,data);

			for(i=0;i<inputSize; ++i)
				AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(inputData + i));

			data = ((size + 31)<<FREERUN_SHIFT) | 0x07; 
			AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,data);

		}

		
        //RX channel
        switch(port)
		{
		    case SPI_1:
		        spidmaReadAttrib[1] = MMP_DMA_TYPE_SPI_TO_MEM;
		        break;
            case SPI_2:
                spidmaReadAttrib[1] = MMP_DMA_TYPE_SPI2_TO_MEM;//;MMP_DMA_TYPE_SPI2_TO_MEM;
		        break;
		}

		switch(port)
		{
		    case SPI_1:
		        spidmaReadAttrib[3] = AHB_SPI1_DATA_BASE;
		        break;
            case SPI_2:
                spidmaReadAttrib[3] = AHB_SPI2_DATA_BASE;
		        break;
		}
		
		//spidmaReadAttrib[3] = (MMP_UINT32)0xDE900018;  /** source address */
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        spidmaReadAttrib[5] = (MMP_UINT32)tempRxBuf; /** destination address */
#else
        spidmaReadAttrib[5] = (MMP_UINT32)pdes; /** destination address */
#endif
        spidmaReadAttrib[7] = totalSize;             /** total size */
		spidmaReadAttrib[11] = srcWidth;             /** source width */
        spidmaReadAttrib[13] = dstWidth;             /** destination width */
		spidmaReadAttrib[15] = burstSize;             /** burst size */

        result = mmpDmaSetAttrib(g_spiRxDmaCtxt, spidmaReadAttrib);
        if(result)
           goto end;

        result = mmpDmaFire(g_spiRxDmaCtxt);
        if(result)
            goto end;
		
        //SSP dma enable
        AHB_WriteRegister(
			REG_SSP_INTR_CONTROL + port*REG_SSP_PORT_OFFSET, 
			((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
			(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_DMA_EN | 
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_RX_INTR_TH_EN | 
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_OR_EN));

		//Fire SSP
		AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

#ifdef SPI_DMA_ISR
		result = SYS_WaitEvent(SpiRxIsrEvent, SSP_DMA_TIMEOUT);
		if(result)
		{
            goto end;
        }
#else
        result = mmpDmaWaitIdle(g_spiRxDmaCtxt);
        if(result)
        {
            goto end;
        }
#endif
        //wait ssp ready
        result = WaitContollerReady(port);
		if(result)
            goto end;
       
        AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_DIS);
#if defined(_WIN32)
        HOST_ReadBlockMemory((MMP_UINT32)pdes, (MMP_UINT32)(tempRxBuf + 4), totalSize - 4);
#elif defined (__OPENRTOS__)
        // Clear cache
        ithInvalidateDCacheRange(tempRxBuf + 4, totalSize - 4);
		memcpy(pdes, tempRxBuf + 4, totalSize - 4);
#elif defined (__FREERTOS__)
		or32_invalidate_cache(tempRxBuf + 4, totalSize - 4);
		memcpy(pdes, tempRxBuf + 4, totalSize - 4);
#endif
    }
end:
#if defined(_WIN32) || defined (__FREERTOS__) || defined (__OPENRTOS__)
    MEM_Release(tempRxBuf);
#endif
   
    //SSP dma disable
    AHB_WriteRegister(REG_SSP_INTR_CONTROL + port*REG_SSP_PORT_OFFSET, 
    	((SSP_DEFAULT_TX_THRESHOLD << REG_SHIFT_TX_THRESHOLD) |
		(SSP_DEFAULT_RX_THRESHOLD << REG_SHIFT_RX_THRESHOLD) |
                                        REG_BIT_RX_INTR_OR_EN |REG_BIT_TX_INTR_UR_EN |
                                        REG_BIT_RX_INTR_TH_EN | REG_BIT_TX_INTR_TH_EN));

	AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET, 0);
	//reset enddian
	AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &data);
	data &= ~(REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END);
	AHB_WriteRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET,data);

    return result;
}

MMP_INT
mmpSpiDmaTriggerRead(
    SPI_PORT	port,
    void*		pdes,
    MMP_INT		size)
{
    MMP_INT    result       = -1;
    MMP_UINT8  dataLen      = size - 1;
	MMP_UINT32 srcWidth     = 0; 
	MMP_UINT32 dstWidth     = 0;
	MMP_UINT32 burstSize    = 0;
   	MMP_UINT32 tx_threshold = 1;
	MMP_UINT32 rx_threshold = 1;

	{
		MMP_UINT32 data;

        /*
		AHB_ReadRegister(GPIO_BASE + 0x90,&data);
		//GPIO9
		data &= (3<<18);
		data |= (2<<18);
		AHB_WriteRegister(GPIO_BASE + 0x90,data);
		*/
		//MUDEX
		AHB_ReadRegister(GPIO_BASE + 0xD0,&data);
		//GPIO9
		data &= 0x7;
		data |= 0x0086;
		AHB_WriteRegister(GPIO_BASE + 0xD0,data);
	}

    SetPadSel(port);

    if(g_spiRxDmaCtxt)
    {
    	//32bits
    	/*
		if((size & 0x3) == 0)
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 4;  
			dstWidth = 4;
			burstSize = 1;
			dataLen = 31;
		}
		if((size & 0x1) == 0)	// 16bits
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 2;  
			dstWidth = 2;
			burstSize = 1;
			dataLen = 15;
		}
		else					// 8bits
		*/
		{
			tx_threshold = 1;
			rx_threshold = 1;
			srcWidth = 1; 
			dstWidth = 1;
			burstSize = 1;
			dataLen = 7;
		}

		/* TXDOE */
		AHB_WriteRegisterMask(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_TXDO_EN, REG_BIT_TXDO_EN);
		/* Set serial data length */
		AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
		/* SSP dma enable */
        AHB_WriteRegister(
			REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET, 
			((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
			(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_DMA_EN | 
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_RX_INTR_TH_EN | 
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_OR_EN));
		
        //RX channel
        switch(port)
		{
	    case SPI_1:
	        spidmaReadAttrib[1] = MMP_DMA_TYPE_SPI_TO_MEM;
			spidmaReadAttrib[3] = AHB_SPI1_DATA_BASE;
	        break;
        case SPI_2:
            spidmaReadAttrib[1] = MMP_DMA_TYPE_SPI2_TO_MEM;
			spidmaReadAttrib[3] = AHB_SPI2_DATA_BASE;
	        break;
		}

        spidmaReadAttrib[5]  = (MMP_UINT32)pdes;	/** destination address */
        spidmaReadAttrib[7]  = size;				/** total size */
		spidmaReadAttrib[11] = srcWidth;				/** source width */
        spidmaReadAttrib[13] = dstWidth;				/** destination width */
		spidmaReadAttrib[15] = burstSize;				/** burst size */

        result = mmpDmaSetAttrib(g_spiRxDmaCtxt, spidmaReadAttrib);
        if(result)
        {
			goto end;
        }

        result = mmpDmaFire(g_spiRxDmaCtxt);
        if(result)
        {
        	goto end;
        }

		//Fire SSP
		AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
    }
	
end:

    return result;
}

void
mmpSpiDmaTriggerReadWiatIdle()
{
    mmpDmaWaitIdle(g_spiRxDmaCtxt);
}


MMP_INT
mmpSpiPioWrite(
    SPI_PORT port,
    void *inputData,
    MMP_UINT32  inputSize,
    void *pbuf,
    MMP_UINT32 size,
    MMP_UINT8 dataLength)

{
    MMP_UINT32 i;

    MMP_UINT8 dataLen = dataLength - 1;
    MMP_INT result = 0;

	SetPadSel(port);

    //clear rx tx FIFO
	AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET, 4);
    AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);

    AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

    if(dataLength == 8) //datalen 8bits
    {
        MMP_UINT8 *binput = (MMP_UINT8*)(inputData);
        MMP_UINT8 *pbBuf = (MMP_UINT8*)(pbuf);

		if((inputSize + size) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        //write data to fifo
        for(i = 0; i < inputSize; i++)
        {
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(binput + i));
        }
            
        for(i = 0; i < size; i++)
        {
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(pbBuf + i));
        }
    }
    else if(dataLength == 16) //datalen 16bits
    {
        MMP_UINT16 *winput = (MMP_UINT16*)(inputData);
        MMP_UINT16 *pwbuf = (MMP_UINT16*)(pbuf);
        MMP_UINT32 winputSize = inputSize/2;
        MMP_UINT32 wsize = size/2;
        MMP_UINT16 temp;

		if((winputSize + wsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        //write data to fifo
        for(i = 0; i < winputSize; i++)
        {
#if defined(_WIN32) || defined(__OPENRTOS__)
            temp = *(winput + i);
            temp = SWAP_ENDIAN16(temp);
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__)
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(winput + i));
#endif
        }

		for(i = 0; i < wsize; i++)
        {
#if defined(_WIN32) || defined(__OPENRTOS__)
            temp = *(pwbuf + i);
            temp = SWAP_ENDIAN16(temp);
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__)
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(pwbuf + i));
#endif
        }
    }    
    else if(dataLength == 32) //datalen 32bits
    {
        MMP_UINT32 *dwinput = (MMP_UINT32*)(inputData);
        MMP_UINT32 *pdwbuf = (MMP_UINT32*)(pbuf);
        MMP_UINT32 dwinputSize = inputSize/4;
        MMP_UINT32 dwsize = size/4;

		if((dwinputSize + dwsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}

        //write data to fifo
        for(i = 0; i < dwinputSize; i++)
        {
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(dwinput + i));
        }
            
        for(i = 0; i < dwinputSize; i++)
        {
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(pdwbuf + i));
        }
    }
    else
    {
        //TODO
        result = ERROR_SSP_FIFO_LENGTH_UNSUPPORT;
		goto end;
    }

    //fire
    AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
    
    result = WaitContollerReady(port);
    WaitFifoReady(port,0);

end:
	AHB_WriteRegister(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);
    return result;
}

MMP_INT
mmpSpiPioRead(
    SPI_PORT port,
    void *inputData,
    MMP_UINT32 inputSize,
    void *outputBuf,
    MMP_UINT32 size,
    MMP_UINT8 dataLength)

{
    MMP_UINT32 temp;
    MMP_UINT32 dummy = -1;
    MMP_UINT32 i;
    MMP_UINT8 dataLen = dataLength - 1;
    MMP_INT result = 0;


    SetPadSel(port);
    //clear rx tx FIFO
    AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);

    AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
	AHB_WriteRegister(REG_SSP_FREERUN + port*REG_SSP_PORT_OFFSET,4);

    if(dataLength == 8) //datalen 8bits
    {
        MMP_UINT8 *binput = (MMP_UINT8*)(inputData);
        MMP_UINT8 *boutput = (MMP_UINT8*)(outputBuf);

		if((inputSize + size) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}

        //write data to fifo
        for(i = 0; i < inputSize; i++)
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(binput + i));
    
        for(i = 0; i < size; i++)
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, dummy);
    
        //fire
        AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
    
        //wait ssp ready
        result = WaitContollerReady(port);
        result = WaitFifoReady(port, inputSize+ size);
    
        //read dummy
        for(i = 0; i < inputSize; i++)
        {
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
            //printf("input r 0x%x\n", temp);
        }
        for(i = 0; i < size; i++)
        {
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
            *(boutput+i) = (MMP_UINT8)temp;
        }
    }
    else if(dataLength == 16) //datalen 16bits
    {
        MMP_UINT16 *winput = (MMP_UINT16*)(inputData);
        MMP_UINT16 *woutput = (MMP_UINT16*)(outputBuf);
        MMP_UINT32 winputSize = inputSize/2;
        MMP_UINT32 wsize = size/2;
		
		if((winputSize + wsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        //write data to fifo
		for(i = 0; i < winputSize; i++)
        {
#if defined(_WIN32) || defined(__OPENRTOS__)
            temp = *(winput + i);
            temp = SWAP_ENDIAN16(temp);
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__)
            AHB_WriteRegister(REG_SSP_DATA+ port*REG_SSP_PORT_OFFSET, *(winput + i));
#endif
        }
    
        for(i = 0; i < wsize; i++)
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, dummy);
    
        //fire
        AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
    
        //wait ssp ready
        result = WaitContollerReady(port);
        result = WaitFifoReady(port,winputSize+ wsize);

        //read dummy
        for(i = 0; i < winputSize; i++)
        {
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
            //printf("input r 0x%x\n", temp);
        }

		for(i = 0; i < wsize; i++)
        {
#if defined(_WIN32) || defined(__OPENRTOS__)
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
            temp = SWAP_ENDIAN16(temp);
            *(woutput+i) = (MMP_UINT16)temp;
#elif defined(__FREERTOS__)
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
            *(woutput+i) = (MMP_UINT16)temp;
#endif
        }
    }
    else if(dataLength == 32) //datalen 32bits
    {
        MMP_UINT32 *dwinput = (MMP_UINT32*)(inputData);
        MMP_UINT32 *dwoutput = (MMP_UINT32*)(outputBuf);
        MMP_UINT32 dwinputSize = inputSize/4;
        MMP_UINT32 dwsize = size/4;

		if((dwinputSize + dwsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        //write data to fifo
        for(i = 0; i < dwinputSize; i++)
        {
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, *(dwinput + i));
        }
    
        for(i = 0; i < dwsize; i++)
            AHB_WriteRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, dummy);
    
        //fire
        AHB_WriteRegister(REG_SSP_CONTROL_2 + port*REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
    
        //wait ssp ready
        result = WaitContollerReady(port);
        result = WaitFifoReady(port,dwinputSize+ dwsize);
        //read dummy
        for(i = 0; i < dwinputSize; i++)
        {
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
        }
        for(i = 0; i < dwsize; i++)
        {
            AHB_ReadRegister(REG_SSP_DATA + port*REG_SSP_PORT_OFFSET, &temp);
            *(dwoutput+i) = (MMP_UINT32)temp;
        }
    }
    else
    {
        //TODO
		result = ERROR_SSP_FIFO_LENGTH_UNSUPPORT;
    }

end:
	AHB_WriteRegister(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

    return result;
}

void
mmpSpiSetDiv(
    SPI_PORT port,
    MMP_INT16 div)
{
    AHB_WriteRegisterMask(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, div, REG_MASK_SCLK_DIV);
}
    
void
mmpSpiSetMode(
    SPI_PORT port,
    SPI_MODE mode)
{
    MMP_INT32 temp;

    AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &temp);
    temp &= ~(0x3);
    temp |= mode;
    AHB_WriteRegister(REG_SSP_CONTROL_0+ port*REG_SSP_PORT_OFFSET, temp);
}

void
mmpSpiSetMaster(
    SPI_PORT port)
{
    MMP_INT32 temp;

    AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &temp);
    temp &= ~(0xC);
    temp |= 0x8;
    AHB_WriteRegister(REG_SSP_CONTROL_0+ port*REG_SSP_PORT_OFFSET, temp);
}

void
mmpSpiSetSlave(
    SPI_PORT port)
{
    MMP_INT32 temp;

    AHB_ReadRegister(REG_SSP_CONTROL_0 + port*REG_SSP_PORT_OFFSET, &temp);
    temp &= ~(0xC);
    AHB_WriteRegister(REG_SSP_CONTROL_0+ port*REG_SSP_PORT_OFFSET, temp);
}

