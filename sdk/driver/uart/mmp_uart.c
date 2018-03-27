#include "sys/sys.h"
#include "host/ahb.h"
#include "mem/mem.h"
#include "host/host.h"
#include "host/gpio.h"
#include "uart/uart.h"
#include "mmp_uart.h"
#include "mmp_dma.h"
#include "mmp_util.h"

//for interrupt
#if defined(__FREERTOS__) && defined(ENABLE_INTR)
    #define UART_IRQ_ENABLE
#endif

#if defined(UART_IRQ_ENABLE)
#if defined(__FREERTOS__)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#else
#include "intr/intr.h"
#endif
#endif
#endif

//=============================================================================
//                              Extern Reference
//=============================================================================
#ifdef _WIN32
static MMP_UINT32 gpio_uart = DGPIO(6); // DGPIO(11)
#else
extern MMP_UINT32 gpio_uart;
#endif


#define ON 1
#define UART_CLOCK   27000000 //TODO
#define UART_GET_MAX_COUNT  200
#define PAGE_SIZE   (32)
#define DMA_LLD_MAX_SIZE    32
#define UART1_AHB_BASE	0xDE600000
#define UART2_AHB_BASE	0xDE700000
#define UART_AHB_BASE	UART2_AHB_BASE
//for interrupt
#define UART_RINGBUFFER_SIZE    256
typedef struct RING_CONTEXT_TAG
{
    MMP_UINT wt_ix;
    MMP_UINT rd_ix;
}RING_CONTEXT;

#if defined(UART_IRQ_ENABLE)
static MMP_UINT8*       pRing = MMP_NULL;
static RING_CONTEXT     ringCtx;

static MMP_INT
WriteRing(void *buf, MMP_UINT32 size)
{
    MMP_INT result = 0;
    MMP_UINT8 *pbuf = (MMP_UINT8*)(buf);
    MMP_UINT32 *p32buf = (MMP_UINT32*)(buf);
    MMP_INT items;
    
    MMP_INT i;
    for(i= 0 ; i< size; ++i)
    {
        //full check
        items = (ringCtx.rd_ix <= ringCtx.wt_ix ) ? (ringCtx.wt_ix - ringCtx.rd_ix) \
                : (UART_RINGBUFFER_SIZE - (ringCtx.rd_ix - ringCtx.wt_ix));
                    
        if(items >= (UART_RINGBUFFER_SIZE - 1))
        {
            result = -1;
            printf("RING FULL\n");
            goto end;
        }    
        *(pRing + ringCtx.wt_ix) = *(pbuf + i);
        if(++ringCtx.wt_ix >= UART_RINGBUFFER_SIZE)
            ringCtx.wt_ix = 0;
    }
	
	/* Turn on the Tx interrupt so the ISR will remove the character from the
	queue and send it.   This does not need to be in a critical section as
	if the interrupt has already removed the character the next interrupt
	will simply turn off the Tx interrupt again. */
    //ithUartEnableIntr(port, ITH_UART_TX_READY);

end:
    return result;            
}


static MMP_INT
ReadRing(void *buf, MMP_UINT32 maxSize)
{
    MMP_INT result = 0;
    MMP_UINT8 *pbuf = (MMP_UINT8*)(buf);
    MMP_INT i = 0;
    
    if(ringCtx.wt_ix == ringCtx.rd_ix)
    {
        result = 0;
        goto end;
    }
    for(i= 0 ; i< maxSize; ++i)
    {
        //empty check
        if(ringCtx.wt_ix == ringCtx.rd_ix)  
        {
            goto end;
        }  
        *(pbuf + i) = *(pRing + ringCtx.rd_ix);
        if(++ringCtx.rd_ix >= UART_RINGBUFFER_SIZE)
            ringCtx.rd_ix = 0;
    }
end:
    result = i;

    return result;            
} 


MMP_INT uart1_isr(void* data)
{
    MMP_UINT32 temp;
    MMP_INT result;
    MMP_UINT32 ch;
    MMP_UINT8 chlsb;

    MMP_UINT32 status;
    MMP_INT i;
    
	AHB_ReadRegister(UART1_BASE + SERIAL_IIR, &temp);
	if(SERIAL_IIR_DR & temp)
	{
	    //data count >= fifo level
	    for(;;)
	    {
   	        AHB_ReadRegister(UART1_BASE+SERIAL_LSR, &status);
    	 
	        if((status & SERIAL_LSR_DR)==SERIAL_LSR_DR)
	        {
                AHB_ReadRegister(UART1_BASE + SERIAL_RBR, &ch);
           	    chlsb = (MMP_UINT8)ch;
                WriteRing(&chlsb, 1);

            }
            else
                break;
        }
	    
	}
	if(SERIAL_IIR_RLS & temp)
    {
        	    
        AHB_ReadRegister(UART1_BASE + SERIAL_LSR, &ch);
	    //printf("SERIAL_IIR_RLS 0x%x\n", ch);
	    
    }
    if(SERIAL_IIR_TIMEOUT & temp)
    {
	    //data count < fifo level
	    for(;;)
	    {

   	        AHB_ReadRegister(UART1_BASE+SERIAL_LSR, &status);
    	 
	        if((status & SERIAL_LSR_DR)==SERIAL_LSR_DR)
	        {
                AHB_ReadRegister(UART1_BASE + SERIAL_RBR, &ch);
           	    chlsb = (MMP_UINT8)ch;
                WriteRing(&chlsb, 1);
            }
            else
                break;
        }
    }
    if(SERIAL_IIR_TE & temp)
    {
	    //printf("SERIAL_IIR_TE\n");
    }
	

    return result;
}

MMP_INT uart2_isr(void* data)
{
    MMP_UINT32 temp;
    MMP_INT result;
    MMP_UINT32 ch;
    MMP_UINT8 chlsb;

    MMP_UINT32 status;
    MMP_INT i;
    
	AHB_ReadRegister(UART2_BASE + SERIAL_IIR, &temp);
	if(SERIAL_IIR_DR & temp)
	{
	    //data count >= fifo level
	    for(;;)
	    {
   	        AHB_ReadRegister(UART2_BASE+SERIAL_LSR, &status);
    	 
	        if((status & SERIAL_LSR_DR)==SERIAL_LSR_DR)
	        {
                AHB_ReadRegister(UART2_BASE + SERIAL_RBR, &ch);
           	    chlsb = (MMP_UINT8)ch;
                WriteRing(&chlsb, 1);

            }
            else
                break;
        }
	    
	}
	if(SERIAL_IIR_RLS & temp)
    {
        	    
        AHB_ReadRegister(UART2_BASE + SERIAL_LSR, &ch);
	    //printf("SERIAL_IIR_RLS 0x%x\n", ch);
	    
    }
    if(SERIAL_IIR_TIMEOUT & temp)
    {
	    //data count < fifo level
	    for(;;)
	    {

   	        AHB_ReadRegister(UART2_BASE+SERIAL_LSR, &status);
    	 
	        if((status & SERIAL_LSR_DR)==SERIAL_LSR_DR)
	        {
                AHB_ReadRegister(UART2_BASE + SERIAL_RBR, &ch);
           	    chlsb = (MMP_UINT8)ch;
                WriteRing(&chlsb, 1);
            }
            else
                break;
        }
    }
    if(SERIAL_IIR_TE & temp)
    {
	    //printf("SERIAL_IIR_TE\n");
    }
	

    return result;
}
 
#endif

MMP_UINT32 DebugPort = UART1_BASE;
MMP_UINT32 SystemPort = UART2_BASE;

static MMP_DMA_CONTEXT      g_uartTxDmaCtxt;
static MMP_DMA_CONTEXT      g_uartRxDmaCtxt;
static MMP_UINT32 lldAddrAlloc;
static MMP_UINT32 lldAddr;

static MMP_UINT32 uartDmaRxttrib[] =
{
    //TODO
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_UART2_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR, UART_AHB_BASE,        //FOR WIN32 
    MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)1024,
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 1, 
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 1,//32,
	MMP_DMA_ATTRIB_DST_TX_WIDTH, 1,
	MMP_DMA_ATTRIB_LLD_ADDR, 0,
    MMP_DMA_ATTRIB_NONE
 
};

static MMP_UINT32 uartDmaTxttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_UART2,
    MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_DST_ADDR, UART_AHB_BASE,    //FOR WIN32 
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)1024,
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 1,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 1,//8,
	MMP_DMA_ATTRIB_DST_TX_WIDTH, 1,
	MMP_DMA_ATTRIB_LLD_ADDR, 0,
    MMP_DMA_ATTRIB_NONE
};

static MMP_INLINE MMP_INT DMA_Rx_CreateLLD(
										MMP_UINT32 srcAddr, 
										MMP_UINT32 dstAddr, 
										MMP_INT size)

{
	MMP_INT result = 0;
	MMP_UINT32 lldSize=0, curSize=0, srcWidth, dstWidth;
	MMP_INT totalSize = size;
	MMP_INT i;
	static MMP_DMA_LLD sysLLD[DMA_LLD_MAX_SIZE];

	memset((void*)sysLLD, 0, sizeof(sysLLD));
#if defined(__FREERTOS__)
	lldAddr = (MMP_UINT32)sysLLD;
#else
	if(!lldAddrAlloc)
	{
		lldAddrAlloc = (MMP_UINT32)MEM_Allocate(sizeof(sysLLD), 0);
		lldAddr = (lldAddrAlloc+3) & ~3;
	}
#endif
	//printf(" lldAddr = 0x%08X \n", lldAddr);
	if(lldAddr & 0x3)
	{
		printf(" LLD VRAM Addr not align....!!\n");
		while(1);
	}

	sysLLD[0].srcAddr = srcAddr;
	sysLLD[0].dstAddr = dstAddr;
	while(totalSize > 0)
	{
		lldSize++;

		sysLLD[lldSize].srcAddr = srcAddr;
		sysLLD[lldSize].dstAddr = sysLLD[lldSize-1].dstAddr+curSize;

		/** current size in byte */
		if(totalSize >= PAGE_SIZE)
			curSize = PAGE_SIZE;
		else
			curSize = totalSize;
		/** src/dst width */
		srcWidth = 1;
		if(curSize & 0x1)  /** size is 8-bits */
		{
			dstWidth = 1;
		}

		dstWidth = 1;
#if 0
		else if(curSize & 0x2) /** size is 16-bits */
		{
			if(sysLLD[lldSize].dstAddr & 0x1)
				dstWidth = 1;
			else
				dstWidth = 2;
		}
		else  /** size is 32-bits */
		{
			if(sysLLD[lldSize].dstAddr & 0x1)
				dstWidth = 1;
			else if(sysLLD[lldSize].dstAddr & 0x2)
				dstWidth = 2;
			else
				dstWidth = 4;
		}
#endif
		switch(srcWidth)
		{
		case 1:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_8<<LLD_SRC_WIDTH_SHT);
			break;
		case 2:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_16<<LLD_SRC_WIDTH_SHT);
			break;
		case 4:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_32<<LLD_SRC_WIDTH_SHT);
			break;
		default:
			break;
		};
		switch(dstWidth)
		{
		case 1:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_8<<LLD_DST_WIDTH_SHT);
			break;
		case 2:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_16<<LLD_DST_WIDTH_SHT);
			break;
		case 4:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_32<<LLD_DST_WIDTH_SHT);
			break;
		default:
			break;
		};
		/** address control */
		sysLLD[lldSize].control |= (DMA_ADDR_CTRL_FIX<<LLD_SRC_ADDR_CTRL_SHT)|(DMA_ADDR_CTRL_INC<<LLD_DST_ADDR_CTRL_SHT) | (1<<LLD_SRC_SEL);
		/** current tx size */
		sysLLD[lldSize].txSize = curSize/srcWidth;
		/** next LLD pointer */
		sysLLD[lldSize].llp = lldAddr + lldSize*sizeof(sysLLD[0]);
		/** update totalSize */
		totalSize -= curSize;
	}
	sysLLD[lldSize].llp = 0;
#if defined(__FREERTOS__)
    {   /** for risc endian issue */
        MMP_INT i=1;
        for(i=1; i<=lldSize; i++)
        {
            sysLLD[i].srcAddr = cpu_to_le32(sysLLD[i].srcAddr);
            sysLLD[i].dstAddr = cpu_to_le32(sysLLD[i].dstAddr);
            sysLLD[i].llp = cpu_to_le32(sysLLD[i].llp);
            sysLLD[i].control = cpu_to_le32(sysLLD[i].control);
            sysLLD[i].txSize = cpu_to_le32(sysLLD[i].txSize);
        }
    }	
#else
    HOST_WriteBlockMemory(lldAddr, (MMP_UINT32)&sysLLD[1], lldSize*sizeof(sysLLD[0]));
#endif

#if 0
	for(i=1; i<=lldSize; i++)
	{
		printf(" srcAddr=0x%08X, dstAddr=0x%08X, llp=0x%08X, ctrl=0x%08X, txSize=0x%08X \n",
			sysLLD[i].srcAddr, sysLLD[i].dstAddr, sysLLD[i].llp, sysLLD[i].control, sysLLD[i].txSize);
	}
#endif

	return result;
}


static MMP_INLINE MMP_INT DMA_Tx_CreateLLD(
	MMP_UINT32 srcAddr, 
	MMP_UINT32 dstAddr, 
	MMP_INT size)

{
	MMP_INT result = 0;
	MMP_UINT32 lldSize=0, curSize=0, srcWidth, dstWidth;
	MMP_INT totalSize = size;
	MMP_INT i;
	static MMP_DMA_LLD sysLLD[DMA_LLD_MAX_SIZE];

	memset((void*)sysLLD, 0, sizeof(sysLLD));
#if defined(__FREERTOS__)
	lldAddr = (MMP_UINT32)sysLLD;
#else
	if(!lldAddrAlloc)
	{
		lldAddrAlloc = (MMP_UINT32)MEM_Allocate(sizeof(sysLLD), 0);
		lldAddr = (lldAddrAlloc+3) & ~3;
	}
#endif
	//printf(" lldAddr = 0x%08X \n", lldAddr);
	if(lldAddr & 0x3)
	{
		printf(" LLD VRAM Addr not align....!!\n");
		while(1);
	}

	sysLLD[0].srcAddr = srcAddr;
	sysLLD[0].dstAddr = dstAddr;
	while(totalSize > 0)
	{
		lldSize++;

		sysLLD[lldSize].srcAddr = sysLLD[lldSize-1].srcAddr+curSize;;
		sysLLD[lldSize].dstAddr = dstAddr;

		/** current size in byte */
		if(totalSize >= PAGE_SIZE)
			curSize = PAGE_SIZE;
		else
			curSize = totalSize;
		/** src/dst width */
		if(curSize & 0x1)  /** size is 8-bits */
		{
			srcWidth = 1;
		}
		else if(curSize & 0x2) /** size is 16-bits */
		{
			if(sysLLD[lldSize].srcAddr & 0x1)
				srcWidth = 1;
			else
				srcWidth = 2;
		}
		else  /** size is 32-bits */
		{
			if(sysLLD[lldSize].srcAddr & 0x1)
				srcWidth = 1;
			else if(sysLLD[lldSize].srcAddr & 0x2)
				srcWidth = 2;
			else
				srcWidth = 4;
		}
		dstWidth = 1;
		switch(srcWidth)
		{
		case 1:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_8<<LLD_SRC_WIDTH_SHT);
			break;
		case 2:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_16<<LLD_SRC_WIDTH_SHT);
			break;
		case 4:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_32<<LLD_SRC_WIDTH_SHT);
			break;
		default:
			break;
		};
		switch(dstWidth)
		{
		case 1:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_8<<LLD_DST_WIDTH_SHT);
			break;
		case 2:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_16<<LLD_DST_WIDTH_SHT);
			break;
		case 4:
			sysLLD[lldSize].control |= (DMA_TX_WIDTH_32<<LLD_DST_WIDTH_SHT);
			break;
		default:
			break;
		};
		/** address control */
		sysLLD[lldSize].control |= (DMA_ADDR_CTRL_INC<<LLD_SRC_ADDR_CTRL_SHT)|(DMA_ADDR_CTRL_FIX<<LLD_DST_ADDR_CTRL_SHT) | (1<<LLD_DST_SEL);
		/** current tx size */
		sysLLD[lldSize].txSize = curSize/srcWidth;
		/** next LLD pointer */
		sysLLD[lldSize].llp = lldAddr + lldSize*sizeof(sysLLD[0]);
		/** update totalSize */
		totalSize -= curSize;
	}
	sysLLD[lldSize].llp = 0;
#if defined(__FREERTOS__)
    {   /** for risc endian issue */
        MMP_INT i=1;
        for(i=1; i<=lldSize; i++)
        {
            sysLLD[i].srcAddr = cpu_to_le32(sysLLD[i].srcAddr);
            sysLLD[i].dstAddr = cpu_to_le32(sysLLD[i].dstAddr);
            sysLLD[i].llp = cpu_to_le32(sysLLD[i].llp);
            sysLLD[i].control = cpu_to_le32(sysLLD[i].control);
            sysLLD[i].txSize = cpu_to_le32(sysLLD[i].txSize);
        }
    }
#else    
	HOST_WriteBlockMemory(lldAddr, (MMP_UINT32)&sysLLD[1], lldSize*sizeof(sysLLD[0]));
#endif

#if 0
	for(i=1; i<=lldSize; i++)
	{
		printf(" srcAddr=0x%08X, dstAddr=0x%08X, llp=0x%08X, ctrl=0x%08X, txSize=0x%08X \n",
			sysLLD[i].srcAddr, sysLLD[i].dstAddr, sysLLD[i].llp, sysLLD[i].control, sysLLD[i].txSize);
	}
#endif

	return result;
}

static MMP_INLINE void DMA_DestroyLLD(void)
{
	if(!lldAddrAlloc)
	{
		MEM_Release((void*)lldAddrAlloc);
		lldAddrAlloc = 0;
	}
}

static void SetPadSel(MMP_UINT32 port)
{
	MMP_UINT32 data;
	if(port == UART1)
	{
#if 1
		AHB_WriteRegisterMask(GPIO_BASE + 0xD0,4<<4,7<<4);//Host_sel	
		AHB_ReadRegister(GPIO_BASE + 0x90,&data);
		data |= (1<<0);
		data |= 0xFFFD;
		AHB_WriteRegister(GPIO_BASE + 0x90,data);	
#else
		AHB_ReadRegister(GPIO_BASE + 0x90,&data);
		data |= (3<<0) | (3<<1);
		AHB_WriteRegister(GPIO_BASE + 0x90,data);
#endif	
	}
	else if(port == UART2)
	{
		AHB_ReadRegister(GPIO_BASE + 0x90,&data);
		data = 0;
        AHB_WriteRegister(GPIO_BASE + 0x90,data);
		AHB_WriteRegister(GPIO_BASE + 0xD8,1<<14); // TX 
		AHB_WriteRegister(GPIO_BASE + 0xE0,1<<13); // Rx
		AHB_WriteRegister(GPIO_BASE + 0x8,1<<14);  // TX 
	}
}

void UART_SetMode(MMP_UINT32 port, MMP_UINT32 mode)
{
	MMP_UINT32 mdr;

    AHB_ReadRegister(port + SERIAL_MDR, &mdr);
    mdr &= ~SERIAL_MDR_MODE_SEL;
    AHB_WriteRegister(port + SERIAL_MDR, mdr | mode);
}


void UART_EnableIRMode(MMP_UINT32 port, MMP_UINT32 TxEnable, MMP_UINT32 RxEnable)
{
	MMP_UINT32 acr;

    AHB_ReadRegister(port + SERIAL_ACR, &acr);
    acr &= ~(SERIAL_ACR_TXENABLE | SERIAL_ACR_RXENABLE);
    if(TxEnable)
    	acr |= SERIAL_ACR_TXENABLE;
    if(RxEnable)
    	acr |= SERIAL_ACR_RXENABLE;
    AHB_WriteRegister(port + SERIAL_ACR, acr);
}

/*****************************************************************************/

void UART_Init(MMP_UINT32 port, MMP_UINT32 baudrate, MMP_UINT32 parity,MMP_UINT32 num,MMP_UINT32 len, MMP_UINT32 f_baudrate)
{
	MMP_UINT32 lcr;

    AHB_ReadRegister(port + SERIAL_LCR, &lcr);
    lcr &= ~SERIAL_LCR_DLAB;
	/* Set DLAB=1 */
    AHB_WriteRegister(port + SERIAL_LCR,SERIAL_LCR_DLAB);
    /* Set baud rate */
    AHB_WriteRegister(port + SERIAL_DLM, ((baudrate & 0xf00) >> 8));
    AHB_WriteRegister(port + SERIAL_DLL, (baudrate & 0xff));

    /*Set fraction rate*/
    AHB_WriteRegister(port + SERIAL_DLH, (f_baudrate & 0xf));

	//clear orignal parity setting
	lcr &= 0xc0;

	switch (parity)
	{
		case UART_PARITY_NONE:
			//do nothing
    		break;
    	case UART_PARITY_ODD:
		    lcr|=SERIAL_LCR_ODD;
   		 	break;
    	case UART_PARITY_EVEN:
    		lcr|=SERIAL_LCR_EVEN;
    		break;
    	case UART_PARITY_MARK:
    		lcr|=(SERIAL_LCR_STICKPARITY|SERIAL_LCR_ODD);
    		break;
    	case UART_PARITY_SPACE:
    		lcr|=(SERIAL_LCR_STICKPARITY|SERIAL_LCR_EVEN);
    		break;

    	default:
    		break;
    }

    if(num==2)
		lcr|=SERIAL_LCR_STOP;

	len-=5;

	lcr|=len;

    AHB_WriteRegister(port+SERIAL_LCR,lcr);
}


void UART_SetLoopback(MMP_UINT32 port, MMP_UINT32 onoff)
{
	MMP_UINT32 temp;

	AHB_ReadRegister(port+SERIAL_MCR, &temp);
	if(onoff==ON)
		temp|=SERIAL_MCR_LPBK;
	else
		temp&=~(SERIAL_MCR_LPBK);

	AHB_WriteRegister(port+SERIAL_MCR,temp);
}

void UART_SetFifoCtrl(MMP_UINT32 port, MMP_UINT32 level_tx, MMP_UINT32 level_rx, MMP_UINT32 resettx, MMP_UINT32 resetrx)  //V1.20//ADA10022002
{
	MMP_UINT8 fcr = 0;

 	fcr |= SERIAL_FCR_FE;

 	switch(level_rx)
 	{
 		case SERIAL_FIFO_TRIGGER_LEVEL_32:
 			fcr|=0x01<<6;
 			break;
 		case SERIAL_FIFO_TRIGGER_LEVEL_64:
 			fcr|=0x02<<6;
 			break;
 		case SERIAL_FIFO_TRIGGER_LEVEL_120:
 			fcr|=0x03<<6;
 			break;
 		default:
 			break;
 	}
 	switch(level_tx)
 	{
 		case SERIAL_FIFO_TRIGGER_LEVEL_32:
 			fcr|=0x01<<4;
 			break;
 		case SERIAL_FIFO_TRIGGER_LEVEL_64:
 			fcr|=0x02<<4;
 			break;
 		case SERIAL_FIFO_TRIGGER_LEVEL_120:
 			fcr|=0x03<<4;
 			break;
 		default:
 			break;
 	}
	if(resettx)
		fcr|=SERIAL_FCR_TXFR;

	if(resetrx)
		fcr|=SERIAL_FCR_RXFR;

	AHB_WriteRegister(port+SERIAL_FCR,fcr);
}


void UART_DisableFifo(MMP_UINT32 port)
{
	AHB_WriteRegister(port+SERIAL_FCR,0);
}


void UART_SetInt(MMP_UINT32 port, MMP_UINT32 IntMask)
{
	AHB_WriteRegister(port + SERIAL_IER, IntMask);
}

MMP_BOOL UART_GetChar(MMP_UINT32 port, char *pout)
{
	MMP_UINT32 status;
	MMP_UINT32 count = 0;
	MMP_UINT32 test = 0;
	MMP_BOOL state = MMP_TRUE;

   	do
	{
	 	AHB_ReadRegister(port+SERIAL_LSR, &status);
	 	if(count++ > UART_GET_MAX_COUNT)
	 	{
	 	    state = MMP_FALSE;
	 	    break;
	 	}
	}
	while (!((status & SERIAL_LSR_DR)==SERIAL_LSR_DR));	// wait until Rx ready
	if(state == MMP_TRUE)
	{
#ifdef _WIN32
	    MMP_UINT16 ch;
		HOST_ReadRegister((MMP_UINT16)(port + SERIAL_RBR), &ch);
#else
        MMP_UINT32 ch;

		AHB_ReadRegister(port + SERIAL_RBR, &ch);
#endif
		*pout =  (char)(ch);

    }
    return state;
}

void UART_PutChar(MMP_UINT32 port, char ch)
{
  	MMP_UINT32 status;
	MMP_UINT16 data;

    do
	{
	 	AHB_ReadRegister(port+SERIAL_LSR, &status);
	}while (!((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));	// wait until Tx ready

#ifdef _WIN32
	HOST_WriteRegister((MMP_UINT16)(port + SERIAL_THR), ch);
#else
	AHB_WriteRegister(port + SERIAL_THR, ch);
#endif
}

//void Uart_PutStr(MMP_UINT32 port, char *Str)
//{
//  	char *cp;
//
// 	for(cp = Str; *cp != 0; cp++)
//   		UART_PutChar(port, *cp);
//}

MMP_BOOL UART_DmaGetChars(MMP_UINT32 port, char *pout, MMP_UINT32 size)
{   
	MMP_BOOL state = MMP_TRUE;
    MMP_UINT result = 0;
    MMP_UINT8 *tempBuf;
    MMP_UINT32 temp;
	MMP_UINT32 i = 0, firstSize, lldSize, dstWidth;
	MMP_UINT32 uart_ahb_base =  UART_AHB_BASE;
	
	if(port == UART1_BASE)
		uart_ahb_base = UART1_AHB_BASE;
	else
	    uart_ahb_base = UART2_AHB_BASE;

	firstSize = (size > PAGE_SIZE) ? PAGE_SIZE : size; 
	//printf(" first Size = 0x%X \n", firstSize);
	lldSize = size - firstSize;

    tempBuf = (MMP_UINT8 *)MEM_Allocate(1024 , MEM_USER_MMP);
    if(!tempBuf)
    {
        result = -1;//TODO ERROR CODE
        goto end;
    }
    HOST_SetBlockMemory((MMP_UINT32)tempBuf, 0xFF, 1024);
	if(lldSize)
		DMA_Rx_CreateLLD(uart_ahb_base, (MMP_UINT32)tempBuf + firstSize, lldSize);

	if(firstSize & 0x1)  /** size is 8-bits */
	{
		dstWidth = 1;
	}
	dstWidth = 1;
#if 0
	else if(firstSize & 0x2) /** size is 16-bits */
	{
		if((MMP_UINT32)tempBuf & 0x1)
			dstWidth = 1;
		else
			dstWidth = 2;
	}
	else  /** size is 32-bits */
	{
		if((MMP_UINT32)tempBuf & 0x1)
			dstWidth = 1;
		else if((MMP_UINT32)tempBuf & 0x2)
			dstWidth = 2;
		else
			dstWidth = 4;
	}
#endif
    
	if(port == UART1_BASE)
		uartDmaRxttrib[1] = MMP_DMA_TYPE_UART_TO_MEM;
	else
	    uartDmaRxttrib[1] = MMP_DMA_TYPE_UART2_TO_MEM;
		
	uartDmaRxttrib[3] = uart_ahb_base;	
    uartDmaRxttrib[5] = (MMP_UINT32)tempBuf;  /** destination address */
    uartDmaRxttrib[7] = firstSize;            /** size */
	uartDmaRxttrib[15] = dstWidth;
	uartDmaRxttrib[17] = lldAddr;
    result = mmpDmaSetAttrib(g_uartRxDmaCtxt, uartDmaRxttrib);
    if(result)
        goto end;

	AHB_ReadRegister(port+SERIAL_MCR, &temp);
	temp |= SERIAL_MCR_DMAMODE2;
	AHB_WriteRegister(port+SERIAL_MCR,temp);

    result = mmpDmaFire(g_uartRxDmaCtxt);
    if(result)
        goto end;

    result = mmpDmaWaitIdle(g_uartRxDmaCtxt);
    if(result)
        goto end;

#if defined(__FREERTOS__)
#if defined(__OPENRTOS__)
    ithInvalidateDCacheRange((void*)(tempBuf), size);
#else
    dc_invalidate();
#endif
#endif
            
#ifdef _WIN32
        HOST_ReadBlockMemory((MMP_UINT32)pout, (MMP_UINT32)tempBuf, size);
#else
        memcpy(pout,tempBuf,size);
#endif
    
end:    
    MEM_Release(tempBuf);
    AHB_ReadRegister(port+SERIAL_MCR, &temp);
	temp &= ~SERIAL_MCR_DMAMODE2;
	AHB_WriteRegister(port+SERIAL_MCR,temp);	

    if(result)
        state = MMP_FALSE;
    return state;
}				

MMP_BOOL UART_DmaPutChars(MMP_UINT32 port, char *pin, MMP_UINT32 size)
{   
	MMP_BOOL state = MMP_TRUE;
    MMP_UINT result = 0;
    MMP_UINT8 *tempBuf;
    MMP_UINT32 temp = 0;
	MMP_UINT32 i = 0, firstSize, lldSize, srcWidth;
	MMP_UINT32 uart_ahb_base =  UART_AHB_BASE;
	
	if(port == UART1_BASE)
		uart_ahb_base = UART1_AHB_BASE;
	else
	    uart_ahb_base = UART2_AHB_BASE;

	firstSize = (size > PAGE_SIZE) ? PAGE_SIZE : size; 
	//printf(" first Size = 0x%X \n", firstSize);
	lldSize = size - firstSize;
	
    tempBuf = (MMP_UINT8 *)MEM_Allocate(1024 , MEM_USER_MMP);
    if(!tempBuf)
    {
        result = -1;//TODO ERROR CODE
        goto end;
    }
	
#ifdef _WIN32
    HOST_WriteBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)pin, size);
#else
    memcpy(tempBuf, pin, size);
#endif
	if(lldSize)
		DMA_Tx_CreateLLD((MMP_UINT32)tempBuf + firstSize,uart_ahb_base, lldSize);

	if(firstSize & 0x1)  /** size is 8-bits */
	{
		srcWidth = 1;
	}
	else if(firstSize & 0x2) /** size is 16-bits */
	{
		if((MMP_UINT32)tempBuf & 0x1)
			srcWidth = 1;
		else
			srcWidth = 2;
	}
	else  /** size is 32-bits */
	{
		if((MMP_UINT32)tempBuf & 0x1)
			srcWidth = 1;
		else if((MMP_UINT32)tempBuf & 0x2)
			srcWidth = 2;
		else
			srcWidth = 4;
	}


	if(port == UART1_BASE)
		uartDmaTxttrib[1] = MMP_DMA_TYPE_MEM_TO_UART;
	else
	    uartDmaTxttrib[1] = MMP_DMA_TYPE_MEM_TO_UART2;
		
	uartDmaTxttrib[5] = uart_ahb_base;	
    uartDmaTxttrib[3] = (MMP_UINT32)tempBuf;  /** source address */
    uartDmaTxttrib[7] = firstSize;             /** total size */
	uartDmaTxttrib[11] = srcWidth;
	uartDmaTxttrib[17] = lldAddr;
    result = mmpDmaSetAttrib(g_uartTxDmaCtxt, uartDmaTxttrib);
    if(result)
        goto end;
	
    result = mmpDmaFire(g_uartTxDmaCtxt);
    if(result)
        goto end;
	
    AHB_ReadRegister(port+SERIAL_MCR, &temp);
	temp |= SERIAL_MCR_DMAMODE2;
	AHB_WriteRegister(port+SERIAL_MCR,temp);	

    result = mmpDmaWaitIdle(g_uartTxDmaCtxt);
    if(result)
        goto end;
    
end:    
    MEM_Release(tempBuf);

    AHB_ReadRegister(port+SERIAL_MCR, &temp);
	temp &= ~SERIAL_MCR_DMAMODE2;
	AHB_WriteRegister(port+SERIAL_MCR,temp);	

    if(result)
        state = MMP_FALSE;
    return state;
}				

void UART_EnableInt(MMP_UINT32 port, MMP_UINT32 mode)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_IER, &data);
	AHB_WriteRegister(port + SERIAL_IER, data | mode);
}


void UART_DisableInt(MMP_UINT32 port, MMP_UINT32 mode)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_IER, &data);
	mode = data & (~mode);
	AHB_WriteRegister(port + SERIAL_IER, mode);
}

MMP_UINT32 UART_IntIdentification(MMP_UINT32 port)
{
    MMP_UINT32 data;
	AHB_ReadRegister(port + SERIAL_IIR, &data);
	return data;
}

void UART_SetLineBreak(MMP_UINT32 port)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_LCR, &data);
	AHB_WriteRegister(port + SERIAL_LCR, data | SERIAL_LCR_SETBREAK);
}

void UART_SetLoopBack(MMP_UINT32 port,MMP_UINT32 onoff)
{
    MMP_UINT32 temp;

	AHB_ReadRegister(port+SERIAL_MCR, &temp);
	if(onoff == ON)
		temp |= SERIAL_MCR_LPBK;
	else
		temp &= ~(SERIAL_MCR_LPBK);

	AHB_WriteRegister(port+SERIAL_MCR,temp);
}

void UART_RequestToSend(MMP_UINT32 port)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_MCR, &data);
	AHB_WriteRegister(port + SERIAL_MCR, data | SERIAL_MCR_RTS);
}

void UART_StopToSend(MMP_UINT32 port)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_MCR, &data);
	data &= ~(SERIAL_MCR_RTS);
	AHB_WriteRegister(port + SERIAL_MCR, data);
}

void UART_DataTerminalReady(MMP_UINT32 port)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_MCR, &data);
	AHB_WriteRegister(port + SERIAL_MCR, data | SERIAL_MCR_DTR);
}

void UART_DataTerminalNotReady(MMP_UINT32 port)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_MCR, &data);
	data &= ~(SERIAL_MCR_DTR);
	AHB_WriteRegister(port + SERIAL_MCR, data);
}

MMP_UINT8 UART_ReadLineStatus(MMP_UINT32 port)
{
    MMP_UINT32 data;

	AHB_ReadRegister(port + SERIAL_LSR, &data);
	return (MMP_UINT8)data;
}


MMP_RESULT mmpUartInitialize(URAT_PORT port, MMP_UINT32 baudrate, MMP_UINT32 parity,MMP_UINT32 numofStop,MMP_UINT32 len)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    MMP_UINT32 int_div = 0;
    MMP_UINT32 total_div = 0;
    MMP_UINT32 f_div = 0;
    MMP_UINT16  data;

#if defined(__FREERTOS__)
#if defined(__OPENRTOS__)
    total_div = (ithGetBusClock()/baudrate);
#else
    total_div = (or32_getBusCLK()/baudrate);
#endif
#else
    total_div = (UART_CLOCK/baudrate);
#endif
    int_div = (total_div >> 4);
    f_div = total_div & 0xF;

    //pad sel
	SetPadSel(port);
    //power on clk
    HOST_ReadRegister(MMP_APB_CLOCK_REG_22, &data);
    data |= MMP_APB_EN_CLK48M;
    HOST_WriteRegister(MMP_APB_CLOCK_REG_22, data);
 
	//TODO
    if(port == UART1)
    {
        UART_SetMode(UART1_BASE, SERIAL_MDR_UART);
    #if defined(UART_IRQ_ENABLE)
        UART_EnableInt(UART1_BASE, SERIAL_IER_DR/*|SERIAL_IER_TE|SERIAL_IER_RLS*/);
		#endif
        UART_Init(UART1_BASE, int_div, parity, numofStop, len, f_div);
		    UART_SetFifoCtrl(UART1_BASE, SERIAL_FIFO_TRIGGER_LEVEL_1, SERIAL_FIFO_TRIGGER_LEVEL_1 ,1 ,1);
        //UART_EnableIRMode(UART1_BASE, 1, 1);

    }
    else if (port == UART2)
    {
        UART_SetMode(UART2_BASE, SERIAL_MDR_UART);
		#if defined(UART_IRQ_ENABLE)
        UART_EnableInt(UART2_BASE, SERIAL_IER_DR/*|SERIAL_IER_TE|SERIAL_IER_RLS*/);
		#endif
        UART_Init(UART2_BASE, int_div, parity, numofStop, len, f_div);
        UART_SetFifoCtrl(UART2_BASE, SERIAL_FIFO_TRIGGER_LEVEL_1, SERIAL_FIFO_TRIGGER_LEVEL_1 ,1 ,1);
        //UART_EnableIRMode(UART2_BASE, 1, 1);
    }
    else
        result = MMP_RESULT_ERROR;
		
#if defined(UART_IRQ_ENABLE)
    if(port == UART1)
    {	
        /* Enable the Rx interrupts.  The Tx interrupts are not enabled
        until there are characters to be transmitted. */
        ithIntrDisableIrq(ITH_INTR_UART0_I0);
        //ithUartClearIntr(ITH_UART0);
        UART_IntIdentification(UART1_BASE);
		    ithIntrClearIrq(ITH_INTR_UART0_I0);

        ithIntrRegisterHandlerIrq(ITH_INTR_UART0_I0, uart1_isr, (void*)UART1_BASE);
        //ithUartEnableIntr(ITH_UART0, ITH_UART_RX_READY);
        

        /* Enable the interrupts. */
        ithIntrEnableIrq(ITH_INTR_UART0_I0);		
        
    	//alocate ring buffer
    	pRing = SYS_Malloc(UART_RINGBUFFER_SIZE);
    	if(!pRing)
    	{
        	result = UART_ALLOCATE_MEM_FAIL;
    		goto end;
    	}
    }
    else if(port == UART2)
    {		
        /* Enable the Rx interrupts.  The Tx interrupts are not enabled
        until there are characters to be transmitted. */
        ithIntrDisableIrq(ITH_INTR_UART1_I0);
        //ithUartClearIntr(ITH_UART1);
        UART_IntIdentification(UART2_BASE);
        ithIntrClearIrq(ITH_INTR_UART1_I0);

        ithIntrRegisterHandlerIrq(ITH_INTR_UART1_I0, uart2_isr, (void*)UART2_BASE);
        //ithUartEnableIntr(ITH_UART1, ITH_UART_RX_READY);

        /* Enable the interrupts. */
        ithIntrEnableIrq(ITH_INTR_UART1_I0);		
        
    	//alocate ring buffer
    	pRing = SYS_Malloc(UART_RINGBUFFER_SIZE);
    	if(!pRing)
    	{
        	result = UART_ALLOCATE_MEM_FAIL;
    		goto end;
    	}
    }
	
	ringCtx.wt_ix = 0;
	ringCtx.rd_ix = 0;

#endif    	

     //create dma context
    result = mmpDmaCreateContext(&g_uartTxDmaCtxt);
    if(result)
        goto end;
    result = mmpDmaCreateContext(&g_uartRxDmaCtxt);
    if(result)
        goto end;

end:

    return result;

}

UART_API MMP_RESULT mmpUartTerminate(URAT_PORT port)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
#if defined(UART_IRQ_ENABLE)
    if(port == UART1)
    {
    	ithIntrDisableIrq(ITH_INTR_UART0_I0);
    	if(pRing)
    	    SYS_Free(pRing);
    }
	else
	{
	    ithIntrDisableIrq(ITH_INTR_UART1_I0);
    	if(pRing)
    	    SYS_Free(pRing);
	}
	ringCtx.wt_ix = 0;
	ringCtx.rd_ix = 0;    
#endif	
	
    if(g_uartTxDmaCtxt)
        result = mmpDmaDestroyContext(g_uartTxDmaCtxt);
    
    if(g_uartRxDmaCtxt)
        result = mmpDmaDestroyContext(g_uartRxDmaCtxt);
    //TODO
    return result;
}

UART_API MMP_BOOL mmpUartGetChar(URAT_PORT port, char *pout)
{
    MMP_BOOL state;

    if(port == UART1)
    {
        state = UART_GetChar(UART1_BASE, pout);
    }
    else if(port == UART2)
    {
        state = UART_GetChar(UART2_BASE, pout);
    }
	return state;
}

UART_API MMP_BOOL mmpUartGetChars(URAT_PORT port, char *pout, MMP_UINT size)
{
    MMP_BOOL state;

    if(port == UART1)
    {
        state = UART_DmaGetChars(UART1_BASE, pout, size);
    }
    else if(port == UART2)
    {
        state = UART_DmaGetChars(UART2_BASE, pout, size);
    }
    
    return state;
}


UART_API void mmpUartPutChar(URAT_PORT port, char ch)
{
    if(port == UART1)
    {
        UART_PutChar(UART1_BASE, ch);
    }
    else if(port == UART2)
    {
        UART_PutChar(UART2_BASE, ch);
    }
}

UART_API void mmpUartPutChars(URAT_PORT port, char *chs, MMP_UINT size)
{
    if(port == UART1)
    {
        UART_DmaPutChars(UART1_BASE, chs, size);
    }
    else if(port == UART2)
    {
        UART_DmaPutChars(UART2_BASE, chs, size);
    }
}

UART_API void mmpUartSetLoopback(URAT_PORT port, MMP_UINT32 onoff)
{
	if(port == UART1)
	{
		UART_SetLoopback(UART1_BASE, onoff);
	}
	else if(port == UART2)
	{
		UART_SetLoopback(UART2_BASE, onoff);
	}
}

UART_API MMP_INT mmpUartInterruptGetChars(URAT_PORT port, char *pout, MMP_UINT32 MaxSize)
{
    MMP_INT c = 0;
    
#if defined(UART_IRQ_ENABLE)
    c = ReadRing(pout, MaxSize);
#endif    
    return c;    
}
