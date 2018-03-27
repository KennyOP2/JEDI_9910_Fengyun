#include <stdio.h>
#include "mmp_dpu.h"

#if defined(WIN32)
//#include "dpu/ahb.h"
//#include "dpu/wishbone.h"
#endif

#include "mem/mem.h"
#include "host/ahb.h"		//for AHB_ReadRegister
#include "sys/sys.h"		//for memcpy() function
#include "host/host.h"

#if defined(__FREERTOS__)
  #if defined(__OPENRTOS__)
  #include "ite/ith.h"
  #else
  #include "intr/intr.h"
  #include "or32.h"
  #endif
#endif

/*
#define INVALID_SIZE            (-1)
#define SSP_POLLING_COUNT       (0x100000)
#define WRITE_DATA_COUNTS       8 
#define WRITE_DATA_BYTES        (WRITE_DATA_COUNTS * 4) //according to fifo length
*/

#if defined(__FREERTOS__)
  //#define DPU_IRQ_ENABLE
#endif

#ifdef __FREERTOS__
    #define WriteDpuReg    AHB_WriteRegister
    #define ReadDpuReg     AHB_ReadRegister
#else
    #define WriteDpuReg    HOST_WriteRegister
    #define ReadDpuReg     HOST_ReadRegister
#endif

/*****************************************************************/
//function protocol type
/*****************************************************************/
MMP_RESULT	DpuEcbEncode( MMP_UINT8 *src, MMP_UINT8 *dst);

static MMP_RESULT	CheckDpuContext(DPU_CONTEXT *DpuContext);
static MMP_UINT8	WaitDpuDone();
static void	InitialDpuEnviroment(DPU_MODE DpuMode);
static void	SetDstAddress(MMP_UINT32 DstAddress);
static void	SetSrcAddress(MMP_UINT32 SrcAddress);
static void	SetAllKeyToReg(MMP_UINT32 *keyCtxt, MMP_UINT8 KeyNum);
static void	SetScrRefToReg(MMP_UINT32 *RefCtxt);
static void SetAllVectorToReg(MMP_UINT32 *VectorCtxt, MMP_UINT8 VectorNum);
static void	SetSrcSize(MMP_UINT32 SourceSize);
static void	DpuFire();

static void	SetDpuCrcMode(MMP_UINT32 CrcRegSelect);
static void	SetDpuCrcProcess(MMP_UINT32 CrcProcType);
static void	SetDpuEndian(MMP_UINT32 DpuEndianSelect);
static void	SetDpuBurstSize(MMP_UINT32 BurstSize);
static void	SetDpuBurstWidth(MMP_UINT32 BurstWidth);

static void	SetDpuMode( MMP_UINT32 DpuMode);
static void	SetDpuCipherMode( MMP_UINT32 CipherMode);

static void	SetDpuEncodeMode(void);
static void	SetDpuDecodeMode(void);

static void TransBig2LittleDw(MMP_UINT8 *pBuff1, MMP_UINT8 *pBuff2);
static void SetSrcDataToShaReg(MMP_UINT8 *pBuff);
void Sha2ResetDpu(void);
/*****************************************************************/
//globol variable
/*****************************************************************/

DPU_MODE CtrlRegInitValueArray[MAX_DPU_MODE_NUM] = //MAX_DPU_MODE_NUM = 20
{
	AES_ECB_CTRL_INIT_VALUE,
	AES_CBC_CTRL_INIT_VALUE,
	AES_OFB_CTRL_INIT_VALUE,
	AES_CFB_CTRL_INIT_VALUE,
	AES_CTR_CTRL_INIT_VALUE,

	DES_ECB_CTRL_INIT_VALUE,
	DES_CBC_CTRL_INIT_VALUE,
	DES_OFB_CTRL_INIT_VALUE,
	DES_CFB_CTRL_INIT_VALUE,
	DES_CTR_CTRL_INIT_VALUE,

	DES3_ECB_CTRL_INIT_VALUE,
	DES3_CBC_CTRL_INIT_VALUE,
	DES3_OFB_CTRL_INIT_VALUE,
	DES3_CFB_CTRL_INIT_VALUE,
	DES3_CTR_CTRL_INIT_VALUE,

	CSA_CTRL_INIT_VALUE,
	CRC_CTRL_INIT_VALUE,
	SHA2_CTRL_INIT_VALUE,
	SCRAMBLE_CTRL_INIT_VALUE,
	0
};


#if defined(DPU_IRQ_ENABLE)
static MMP_EVENT DpuIsrEvent            = MMP_NULL;
#endif

#ifdef ENABLE_HW_REVERSE
static unsigned int 	g_DpuEndianSetting = DPU_SET_BIG_ENDIAN;
#else
static unsigned int 	g_DpuEndianSetting = DPU_SET_LITTLE_ENDIAN;
#endif
/*****************************************************************/
//function implementation
/*****************************************************************/
#ifdef	DPU_IRQ_ENABLE
void dpu_isr(void* data)
{
	MMP_UINT32	tmp=0;
	printf("di.i\n");

	AHB_ReadRegister( DPU_STATUS_REG, &tmp);    //check DPU done bit

	if( (tmp&0x00000001)== 0x00000001 )
    {
		SYS_SetEventFromIsr(DpuIsrEvent);

		AHB_WriteRegisterMask(DPU_INT_CONTROLL_REG, DPU_INTR_CLEAR_MASK, DPU_INTR_CLEAR_MASK); /** clear dpu interrupt **/
		//AHB_WriteRegisterMask(DPU_INT_CONTROLL_REG, 0, DPU_INTR_CLEAR_MASK); /** clear dpu interrupt **/
		
		AHB_WriteRegister( DPU_CTRL_REG, 0 );			//clear DPU fire bit
	}
	printf("di.o\n");
	//printf("di.o,r=%x\n",tmp);
}

void DpuIntrEnable(void)
{
	// Initialize Timer IRQ
	printf("Enable DPU IRQ~~1\n");
	ithIntrDisableIrq(ITH_INTR_DPU);
	ithIntrClearIrq(ITH_INTR_DPU);

	#if defined (__FREERTOS__)
	// register NAND Handler to IRQ
	ithIntrRegisterHandlerIrq(ITH_INTR_DPU, dpu_isr, MMP_NULL);
	#endif // defined (__FREERTOS__)

	// set IRQ to edge trigger
	ithIntrSetTriggerModeIrq(ITH_INTR_DPU, ITH_INTR_EDGE);

	// set IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(ITH_INTR_DPU, ITH_INTR_HIGH_RISING);

	// Enable IRQ
	ithIntrEnableIrq(ITH_INTR_DPU);

	AHB_WriteRegisterMask(DPU_INT_CONTROLL_REG, DPU_INTR_ENABLE_MASK, DPU_INTR_ENABLE_MASK); /** enable nand interrupt **/

	if(!DpuIsrEvent)
	    DpuIsrEvent = SYS_CreateEvent();

	printf("DpuIsrEvent=%x\n",DpuIsrEvent);

	printf("Enable DPU IRQ~~leave\n");
}

void DpuIntrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_DPU);
}
#endif


void SetSrcDataToShaReg(MMP_UINT8 *pBuff)
{
	MMP_UINT16 i;
	MMP_UINT32 *pSrc=(MMP_UINT32 *)pBuff;	
	
	#if defined(WIN32)
	MMP_UINT32 TmpSrc,TemSrc2;
	#endif	
	
	//printf("Dec[12.1][%x]\n",pBuff);

	for(i=0;i<16;i++)
	{
		#if defined(WIN32)
		HOST_ReadBlockMemory((MMP_UINT8*)&TmpSrc, (MMP_UINT8*)&pSrc[15-i], 4);
		//HOST_WriteBlockMemory(&pDst[cnt], &TmpDst, 4);	//set initial value @ Dst			
		TransBig2LittleDw(&TmpSrc, &TemSrc2);    //tranform endian format
		AHB_WriteRegister(DPU_SHA_INIT0_REG+(i*4), TemSrc2);
		#else
		//printf("Dec[05][%x]\n",pSrc[i]);    
		//printf("Dec[12.3][%x]\n",DPU_SHA_INIT0_REG); 
		//printf("Dec[12.4][%x]\n",pBuff);
		//printf("Dec[12.5][%x]\n",&pSrc[0]);
		//printf("Dec[12.2][%x,%x,%x]\n", DPU_SHA_INIT0_REG+(i*4), &pSrc[15-i], pSrc[15-i]);
		AHB_WriteRegister(DPU_SHA_INIT0_REG+(i*4),pSrc[15-i]);
		//printf("Dec[06]\n");
		#endif
	}
	//printf("Dec[12.3]\n");
}

static void dpuReverse(unsigned char *buff, MMP_UINT32 size)
{
	int i;
	unsigned char tmpByte;

	for(i=0; i<(size/2); i++)
	{
		tmpByte = buff[size-i-1];
		buff[size-i-1] = buff[i];
		buff[i] = tmpByte;
	}
}

static void dpuReverseData(MMP_UINT8 *buff, MMP_UINT32 size)
{
	MMP_UINT32 i;	
	MMP_UINT8 tmpByte;
	MMP_UINT8 *ptr8=(unsigned char *)( ((unsigned int)buff+3) & ~3);
	MMP_UINT32 *ptr32;
	MMP_UINT32 tmpDW;

	if(buff==ptr8)
	{
		ptr32 = (MMP_UINT32 *)buff;
	
		if(size == 8)
		{
			tmpDW = ptr32[1];	ptr32[1] = ptr32[0];	ptr32[0] = tmpDW;
		}
		else
		{
			tmpDW = ptr32[3];	ptr32[3] = ptr32[0];	ptr32[0] = tmpDW;			
			tmpDW = ptr32[2];	ptr32[2] = ptr32[1];	ptr32[1] = tmpDW;
		}      
	}
	else
	{
		ptr8 = (MMP_UINT8 *)buff;
	    if(size == 8)
	    {
	    	for(i=0; i<4; i++)
	    	{
				tmpByte = buff[i+4];	buff[i+4] = buff[i];	buff[i] = tmpByte;
			}
	    }
	    else
	    {
	    	for(i=0; i<4; i++)
	    	{
				tmpByte = buff[i+12];	buff[i+12] = buff[i];	buff[i] = tmpByte;
				tmpByte = buff[i+8];	buff[i+8] = buff[i+4];	buff[i+4] = tmpByte;
			}
	    }  
	}
}

void dpuReverseSrcData(MMP_UINT8 *buff, MMP_UINT32 size, MMP_UINT32 interval)
{
	MMP_UINT32 i;
	
	for(i=0; i<size; i+=interval)
	{
		#ifdef ENABLE_HW_REVERSE
		dpuReverseData(&buff[i], interval);
		#else
		dpuReverse(&buff[i], interval);
		#endif
	}
}

void DpuReverseAllData(MMP_UINT8 *tem_sbuf, MMP_UINT8 *srcbuf, MMP_UINT32 tmpDpuLen, DPU_MODE DpuMode, MMP_UINT32 cipher_len)
{
	MMP_UINT32 i;
/*
	printf("after reverse!!\n");	
	for(i=0; i<64; i++)
	{
		printf("%02x ",srcbuf[i]);
		if( (i&0x0F)==0x0F )	printf("\n");
	}
*/
	#if defined(__FREERTOS__)
	memcpy(tem_sbuf, srcbuf, tmpDpuLen);
	if( DpuMode!=CRC_MODE && DpuMode!=CSA_MODE )
	{
		dpuReverseSrcData(tem_sbuf, tmpDpuLen, cipher_len); 
	}
	#else
	if( DpuMode!=CRC_MODE && DpuMode!=CSA_MODE )
	{
		MMP_UINT8     *buf = malloc(tmpDpuLen);
		memcpy(buf, srcbuf,tmpDpuLen);
		dpuReverseSrcData(buf, tmpDpuLen, cipher_len); 
		HOST_WriteBlockMemory((MMP_UINT8 *)tem_sbuf, buf, tmpDpuLen);
		free(buf);
	}
	else
	{
		HOST_WriteBlockMemory((MMP_UINT8 *)tem_sbuf, srcbuf, tmpDpuLen);
	}
	#endif	
/*
	printf("after reverse!!\n");	
	for(i=0; i<64; i++)
	{
		printf("%02x ",tem_sbuf[i]);
		if( (i&0x0F)==0x0F )	printf("\n");
	}
*/
}

void TransBig2LittleDw(MMP_UINT8 *pBuff1, MMP_UINT8 *pBuff2)
{
	MMP_UINT8 i;

	for(i=0;i<4;i++)
	{
		pBuff2[i]=pBuff1[3-i];
	}
}

void Sha2ResetDpu(void)
{
	AHB_WriteRegister(DPU_CTRL_REG, 0x00000000);	//clear control register value		
	AHB_WriteRegister(DPU_CTRL_REG, 0x00000001);	//reset dpu
	
	#ifdef	DPU_IRQ_ENABLE
	AHB_WriteRegisterMask(DPU_INT_CONTROLL_REG, DPU_INTR_CLEAR_MASK, DPU_INTR_CLEAR_MASK); 		/** clear dpu interrupt **/
	AHB_WriteRegisterMask(DPU_INT_CONTROLL_REG, 0x00000000, DPU_INTR_CLEAR_MASK); 				/** clear dpu interrupt **/
	AHB_WriteRegisterMask(DPU_INT_CONTROLL_REG, DPU_INTR_ENABLE_MASK, DPU_INTR_ENABLE_MASK); 	/** enable nand interrupt **/
	#endif	
	
}

void SetDpuMode( MMP_UINT32 DpuMode )
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, DpuMode, DPU_MODE_MASK); 
}

void SetDpuCipherMode( MMP_UINT32 DpuCipherMode )
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, DpuCipherMode, DPU_CIPHER_MODE_MASK); 
}

void SetDpuBurstWidth(MMP_UINT32 BurstWidth)
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, BurstWidth, DPU_BURST_WIDTH_MASK); 
}

void SetDpuBurstSize(MMP_UINT32 BurstSize)
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, BurstSize, DPU_BURST_SIZE_MASK); 
}

void SetDpuEndian(MMP_UINT32 DpuEndianSelect)
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, DpuEndianSelect, DPU_SET_ENDIAN_MASK); 
}

void SetDpuCrcMode(MMP_UINT32 CrcRegSelect)
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, CrcRegSelect, CRC_MODE_MASK); 
}


void SetDpuCrcProcess(MMP_UINT32 CrcProcType)
{
	AHB_WriteRegisterMask(DPU_CTRL_REG, CrcProcType, CRC_PROC_MASK); 
}


MMP_RESULT	CheckDpuContext(DPU_CONTEXT *DpuContext)
{
	MMP_UINT8	result=MMP_TRUE;

	//1.Key length
	switch(DpuContext->DpuMode)
	{
	case AES_ECB_MODE:
	case AES_CBC_MODE:
	case AES_OFB_MODE:
	case AES_CFB_MODE:
	case AES_CTR_MODE:
		if(DpuContext->Keylength!=4)
		{
			printf("[DPU][ERROR] incorrect key length of AES, keyLength=[%x]\n",DpuContext->Keylength);
			result=MMP_FALSE;
		}

		break;

	case CSA_MODE:
		if(DpuContext->Keylength!=2)
		{
			printf("[DPU][ERROR] incorrect key length of CSA, keyLength=[%x]\n",DpuContext->Keylength);
			result=MMP_FALSE;
		}
		switch(DpuContext->CipherMode)
		{
		case NO_CIPHER_MODE:
			break;

		case ECB_MODE:
		case CBC_MODE:
		case OFB_MODE:
		case CFB_MODE:
		case CTR_MODE:
			printf("[DPU][ERROR] incorrect cipher mode setting in CSA, CipherMode=[%x]\n",DpuContext->CipherMode);
			DpuContext->CipherMode = NO_CIPHER_MODE;
			break;

		default:
			//printf("[DPU][ERROR] incorrect cipher mode setting in CSA default, CipherMode=[%x]\n",DpuContext->CipherMode);
			DpuContext->CipherMode = NO_CIPHER_MODE;
			break;
		}
		break;

	case DES_ECB_MODE:
	case DES_CBC_MODE:
	case DES_OFB_MODE:
	case DES_CFB_MODE:
	case DES_CTR_MODE:
		if(DpuContext->Keylength!=2)
		{
			printf("[DPU][ERROR] incorrect key length of DES, keyLength=[%x]\n",DpuContext->Keylength);
			result=MMP_FALSE;
		}
		break;
		
	case DES3_ECB_MODE:
	case DES3_CBC_MODE:
	case DES3_OFB_MODE:
	case DES3_CFB_MODE:
	case DES3_CTR_MODE:
		if(DpuContext->Keylength!=6)
		{
			printf("[DPU][ERROR] incorrect key length of DES3, keyLength=[%x]\n",DpuContext->Keylength);
			result=MMP_FALSE;
		}
		break;
	case CRC_MODE:
		break;
	case SCRAMBLE_MODE:
		break;
	default:
		printf("[DPU][ERROR] incorrect DPU mode setting, DpuMode=[%x]\n",DpuContext->DpuMode);
		result=MMP_FALSE;
		break;
	}

	//3.cehck key data (key=0x00000000 ??)

	//4.check address
	if(DpuContext->SrcAddress==0x00000000)	
	{
		printf("[DPU][ERROR] incorrect Source address setting, SrcAddr=[%x]\n",DpuContext->SrcAddress);
		result=MMP_FALSE;
	}

	if(DpuContext->DstAddress==0x00000000)	
	{
		printf("[DPU][ERROR] incorrect Distination address setting, DisAddr=[%x]\n",DpuContext->DstAddress);
		result=MMP_FALSE;
	}

	return	result;
}

void InitialDpuEnviroment(DPU_MODE DpuMode)
{
	MMP_UINT32	CtrlRegMask = CTRL_REG_INIT_MASK | DPU_MODE_MASK | DPU_SRC_OF_INIT_VCTR_MASK | DPU_RAND_KEY_SOURCE_MASK | DPU_CIPHER_MODE_MASK | DPU_MODE_MASK | DPU_RAND_KEY_FREERUN_MASK;

	AHB_WriteRegisterMask( DPU_CTRL_REG, CtrlRegInitValueArray[DpuMode], CtrlRegMask );
}


void SetSrcAddress(MMP_UINT32 SrcAddress)
{
	AHB_WriteRegister(DPU_SRC_ADDR_REG, SrcAddress); 
}


void SetDstAddress(MMP_UINT32 DstAddress)
{
	AHB_WriteRegister(DPU_DST_ADDR_REG, DstAddress); 
}

void SetSrcSize(MMP_UINT32 SourceSize)
{
	AHB_WriteRegister(DPU_SRC_SIZE_REG, SourceSize); 

}

void SetAllKeyToReg(MMP_UINT32 *keyCtxt, MMP_UINT8 KeyNum)
{
	MMP_UINT8 KeyIndex;
#if defined(WIN32) || defined(__OPENRTOS__)
	for(KeyIndex=0; KeyIndex<KeyNum; KeyIndex++)
	{
		AHB_WriteRegister( ((DPU_KEY0_REG)+(KeyIndex*4)), keyCtxt[KeyIndex] ); 
	}
#else
	MMP_UINT8  cKey2[4];
	MMP_UINT32 *pKey=(MMP_UINT32*)cKey2;
	for(KeyIndex=0; KeyIndex<KeyNum; KeyIndex++)
	{
		TransBig2LittleDw((MMP_UINT8*)&keyCtxt[KeyIndex],&cKey2[0]);
		AHB_WriteRegister( ((DPU_KEY0_REG)+(KeyIndex*4)), pKey[0] );
	}
#endif
}

void SetAllVectorToReg(MMP_UINT32 *VectorCtxt, MMP_UINT8 VectorNum)
{
	MMP_UINT8 VectorIndex;
#if defined(WIN32) || defined(__OPENRTOS__)
	for(VectorIndex=0; VectorIndex<VectorNum; VectorIndex++)
	{
		AHB_WriteRegister( ((DPU_INIT_VECTOR0_REG)+(VectorIndex*4)), VectorCtxt[VectorIndex] ); 
	}
#else
	MMP_UINT8  Vector[4];
	MMP_UINT32 *pVector=(MMP_UINT32*)Vector;
	for(VectorIndex=0; VectorIndex<VectorNum; VectorIndex++)
	{
		TransBig2LittleDw((MMP_UINT8*)&VectorCtxt[VectorIndex],&Vector[0]);
		AHB_WriteRegister( ((DPU_INIT_VECTOR0_REG)+(VectorIndex*4)), pVector[0] );
	}
#endif
}

static void	SetScrRefToReg(MMP_UINT32 *RefCtxt)
{
#if defined(WIN32) || defined(__OPENRTOS__)
	AHB_WriteRegister( DPU_INIT_SCRAMBLE_REG, RefCtxt[0] ); 
#else
	MMP_UINT8  Vector[4];
	MMP_UINT32 *pVector=(MMP_UINT32 *)Vector;

	TransBig2LittleDw((MMP_UINT8 *)&RefCtxt[0],(MMP_UINT8 *)&Vector[0]);
	AHB_WriteRegister( DPU_INIT_SCRAMBLE_REG, pVector[0] ); 
#endif
}

void SetDpuEncodeMode(void)
{
	AHB_WriteRegisterMask( DPU_CTRL_REG, DPU_SET_ENCODE, DPU_SET_ENCODE_DECODE_MASK );
}

void SetDpuDecodeMode(void)
{
	AHB_WriteRegisterMask( DPU_CTRL_REG, DPU_SET_DECODE, DPU_SET_ENCODE_DECODE_MASK );
}

void DpuFire()
{
	AHB_WriteRegisterMask( DPU_CTRL_REG, DPU_FIRE_BIT, DPU_FIRE_BIT );			//fire DPU engine 
}

MMP_UINT8 WaitDpuDone()
{
	MMP_UINT32 StatusReg32,TomeOutCnt=0x100000;
	MMP_UINT8	Result = MMP_TRUE;
	
	//printf("Chk Sts=%x!!\n",DPU_STATUS_REG);

#ifndef	ENABLE_NEW_CHECK_STATUS
	do
	{
		AHB_ReadRegister( DPU_STATUS_REG, &StatusReg32);    //check DPU done bit
		//printf("Chk Sts!!\n");
		MMP_Sleep(1);

		if(TomeOutCnt==0)
		{
			Result = MMP_FALSE;
			break;
		}
	} while((StatusReg32&0x00000001) == 0x00000000);
	//printf("Chk Sts out!!\n");

	AHB_WriteRegister( DPU_CTRL_REG, 0x00000000 );			//clear DPU fire bit
	
	//printf("clear DPU fire bit!!\n");
#else
	do
	{
		AHB_ReadRegister( DPU_CTRL_REG, &StatusReg32);    //check DPU done bit
		if(TomeOutCnt==0)
		{
			Result = MMP_FALSE;
			break;
		}
	} while((StatusReg32&0x00000002) != 0x00000000);
#endif
	return	Result;
}


MMP_RESULT DpuEncode(DPU_CONTEXT *DpuContext)
{
	MMP_RESULT result = MMP_FALSE;
	MMP_UINT32 Reg32;
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	MMP_UINT32 cipher_len = 8;
	#if defined(__OPENRTOS__)
	MMP_UINT8     *tem_sbuf=malloc(DpuContext->DataSize+4);
	#else
	MMP_UINT8     *tem_sbuf=MEM_Allocate(DpuContext->DataSize+4, MEM_USER_DPU);
	#endif
#endif
	#ifdef	DPU_IRQ_ENABLE
	MMP_INT    EventRst;
	#endif

#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	if( (DpuContext->DpuMode==AES_ECB_MODE) ||
		(DpuContext->DpuMode==AES_CBC_MODE) ||
		(DpuContext->DpuMode==AES_OFB_MODE) ||
		(DpuContext->DpuMode==AES_CFB_MODE) ||
		(DpuContext->DpuMode==AES_CTR_MODE) )
	{				
			cipher_len = 16;	
	}
	
	DpuReverseAllData(tem_sbuf, (MMP_UINT8 *)DpuContext->SrcAddress, DpuContext->DataSize, DpuContext->DpuMode, cipher_len);
#endif

    if(DpuContext->DpuMode==SHA2_MODE)
    {
		printf("SHA2 Eecode mode is incorrect(SHA2_MODE)!\n");
    }
    else if(DpuContext->DpuMode==CRC_MODE)
    {
		AHB_WriteRegister(DPU_CTRL_REG, 0x00000000);	//clear control register value("NECESSARY" under Alpha)
	    AHB_WriteRegister(DPU_CRC32_REG0, 0xffffffff);	//set key0
	    AHB_WriteRegister(DPU_CRC32_REG1, 0xffffffff);	//set key1
        InitialDpuEnviroment(DpuContext->DpuMode);

        SetSrcAddress(DpuContext->SrcAddress);
        SetSrcSize(DpuContext->DataSize);

        //printf("drv.eSrc=%x,dst=%x\n", DpuContext->SrcAddress, DpuContext->DstAddress);

        SetDpuEncodeMode();
	    DpuFire();	    
		
		#ifdef	DPU_IRQ_ENABLE
		EventRst = SYS_WaitEvent(DpuIsrEvent, 100);
		if(EventRst)
		{
			//printf("CRC Wait Evevt Time Out, rst=%x\n",EventRst);
		}
		#else
		if(WaitDpuDone()==MMP_TRUE)		result = MMP_SUCCESS;	//polling DPU status done bit
		#endif

        #if defined(__FREERTOS__)
        //ithFlushAhbWrap();
	    AHB_ReadRegister( DPU_CRC32_REG0, (MMP_UINT32*)DpuContext->DstAddress);
        #else
	    AHB_ReadRegister( DPU_CRC32_REG0, &Reg32);
        HOST_WriteBlockMemory((MMP_UINT32*)DpuContext->DstAddress, &Reg32, 4);
        #endif
    }
	else if( CheckDpuContext(DpuContext) )
	{
		AHB_WriteRegister(DPU_CTRL_REG, 0x00000000);	//clear control register value
		InitialDpuEnviroment(DpuContext->DpuMode);
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		SetSrcAddress((MMP_UINT32)tem_sbuf);
#else
		SetSrcAddress(DpuContext->SrcAddress);
#endif
		SetDstAddress(DpuContext->DstAddress);
		SetSrcSize(DpuContext->DataSize);

		if(DpuContext->DpuMode==SCRAMBLE_MODE)
		{
			SetScrRefToReg(DpuContext->Vectors);
		}
		else
		{
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
			MMP_UINT8     *buf = malloc(32);
			
			if(DpuContext->Keylength)
			{
				memcpy(buf, DpuContext->RefKeyAddress, DpuContext->Keylength*4);
				if( (DpuContext->DpuMode==DES3_ECB_MODE) ||
					(DpuContext->DpuMode==DES3_CBC_MODE) ||
					(DpuContext->DpuMode==DES3_OFB_MODE) ||
					(DpuContext->DpuMode==DES3_CFB_MODE) ||
					(DpuContext->DpuMode==DES3_CTR_MODE) )
				{
					dpuReverse(&buf[0], 8);
					dpuReverse(&buf[8], 8);
					dpuReverse(&buf[16], 8);
				}
				else
				{
					if(DpuContext->DpuMode!=CSA_MODE)
					{
						dpuReverse(buf, DpuContext->Keylength*4);
					}
				}
				SetAllKeyToReg((MMP_UINT32 *)buf, DpuContext->Keylength);
			}
			
			if(DpuContext->Vectorlength)
			{
				memcpy(buf, DpuContext->Vectors, DpuContext->Vectorlength*4);
				dpuReverse(buf, DpuContext->Vectorlength*4);
				SetAllVectorToReg( (MMP_UINT32 *)buf, DpuContext->Vectorlength);
			}

			free(buf);
#else
		    SetAllKeyToReg(DpuContext->RefKeyAddress, DpuContext->Keylength);
		    SetAllVectorToReg(DpuContext->Vectors, DpuContext->Vectorlength);
#endif
		}

		SetDpuEncodeMode();

#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		if(DpuContext->DpuMode==CSA_MODE)
		{
			SetDpuEndian(DPU_SET_LITTLE_ENDIAN);
		}
		else
		{
			SetDpuEndian(g_DpuEndianSetting);
		}
#endif

		DpuFire();

		#ifdef	DPU_IRQ_ENABLE
		EventRst = SYS_WaitEvent(DpuIsrEvent, 100);
		if(EventRst)
		{
			//printf("Encrypt Wait Evevt Time Out, rst=%x\n",EventRst);
		}
		#else
		if(WaitDpuDone()==MMP_TRUE)		result = MMP_SUCCESS;	//polling DPU status done bit
		#endif

		#if defined(__FREERTOS__)
		  #if defined(__OPENRTOS__)
		  ithInvalidateDCacheRange((MMP_UINT8*)DpuContext->DstAddress, DpuContext->DataSize);
		  #else
		  or32_invalidate_cache((MMP_UINT8*)DpuContext->DstAddress, DpuContext->DataSize);
		#endif
#endif

#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		if(DpuContext->DpuMode!=CSA_MODE)
		{
			dpuReverseSrcData((MMP_UINT8 *)DpuContext->DstAddress, DpuContext->DataSize, cipher_len);
		}
#endif
	}
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	MEM_Release(tem_sbuf);
#endif
	return result;
}

MMP_RESULT DpuDecode(DPU_CONTEXT *DpuContext)
{
	MMP_RESULT result = MMP_FALSE;
	MMP_UINT32 Reg32;
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	MMP_UINT32 cipher_len = 8;
	#if defined(__OPENRTOS__)
	MMP_UINT8     *tem_sbuf=malloc(DpuContext->DataSize+4);
	#else
	MMP_UINT8     *tem_sbuf=MEM_Allocate(DpuContext->DataSize+4, MEM_USER_DPU);
	#endif
#endif
	#ifdef	DPU_IRQ_ENABLE
	MMP_INT    EventRst;
	#endif
	
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	if( (DpuContext->DpuMode==AES_ECB_MODE) ||
		(DpuContext->DpuMode==AES_CBC_MODE) ||
		(DpuContext->DpuMode==AES_OFB_MODE) ||
		(DpuContext->DpuMode==AES_CFB_MODE) ||
		(DpuContext->DpuMode==AES_CTR_MODE) )
	{				
			cipher_len = 16;	
	}

	DpuReverseAllData(tem_sbuf, (MMP_UINT8 *)DpuContext->SrcAddress, DpuContext->DataSize, DpuContext->DpuMode, cipher_len);
#endif

    if(DpuContext->DpuMode==SHA2_MODE)
    {
        MMP_UINT32 cnt;
        MMP_UINT32 *pSrc=(MMP_UINT32*)DpuContext->SrcAddress;
        MMP_UINT32 *pDst=(MMP_UINT32*)DpuContext->DstAddress;
		MMP_UINT8  B2L_SrcData[4];
		MMP_UINT32 ShaLen=DpuContext->DataSize;
		MMP_UINT32 OriShaLen=ShaLen;
#if defined(WIN32)
		MMP_UINT32 TmpSrc=0;
#endif

		while(ShaLen)
		{
		    MMP_UINT32 TemAddr=0;
		    
		    AHB_WriteRegisterMask(DPU_CTRL_REG, 0x00000000, DPU_FIRE_BIT);	//to workaround SHA2 reset issue
			AHB_WriteRegister(DPU_CTRL_REG, 0x00000000);	//clear control register value

			if(ShaLen==DpuContext->DataSize)
			{
				AHB_WriteRegisterMask(DPU_CTRL_REG, 0x00000800, 0x00000800);	//reset SHA2 initial value
			}
			else if(ShaLen<64)
			{
				//less than 64 bytes, need fill to 64 bytes....
				printf("less than 64 bytes, need fill to 64 bytes\n");
			}

			TemAddr = (MMP_UINT32)((OriShaLen/4)-(ShaLen/4));
			SetSrcDataToShaReg((MMP_UINT8*)&pSrc[(DpuContext->DataSize/4)-(ShaLen/4)]);
			InitialDpuEnviroment(DpuContext->DpuMode);
			SetDpuDecodeMode();
			
			DpuFire();

			#ifdef	DPU_IRQ_ENABLE
			//EventRst = SYS_WaitEvent(DpuIsrEvent, 100);	//interrupt mode does not work in sha2 mode @ JEDI_9910
			if(WaitDpuDone()==MMP_TRUE)		result = MMP_SUCCESS;	//polling DPU status done bit
			#else
			if(WaitDpuDone()==MMP_TRUE)		result = MMP_SUCCESS;	//polling DPU status done bit
			#endif

			ShaLen-=64;
		}
		
		for(cnt=0;cnt<8;cnt++)
		{
			#if defined(WIN32)
			AHB_ReadRegister(DPU_SHA_DST0_REG+cnt*4,&TmpSrc); 
			HOST_WriteBlockMemory(&pDst[7-cnt], &TmpSrc, 4);
			#else
		    AHB_ReadRegister(DPU_SHA_DST0_REG+cnt*4,&pDst[7-cnt]); 		    
			#endif
		}		
		
		Sha2ResetDpu();
    }
    else if(DpuContext->DpuMode==CRC_MODE)
    {
        printf("DPU Decode mode is incorrect(CRC_MODE)!\n");
    }
    else if( CheckDpuContext(DpuContext) )
	{
		AHB_WriteRegister(DPU_CTRL_REG, 0x00000000);	//clear control register value
		InitialDpuEnviroment(DpuContext->DpuMode);
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		SetSrcAddress((MMP_UINT32)tem_sbuf);
#else
		SetSrcAddress(DpuContext->SrcAddress);
#endif
		SetDstAddress(DpuContext->DstAddress);

		SetSrcSize(DpuContext->DataSize);

#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		{
			MMP_UINT8     *buf = malloc(32);
			
			if(DpuContext->Keylength)
			{
				memcpy(buf, DpuContext->RefKeyAddress, DpuContext->Keylength*4);
				if( (DpuContext->DpuMode==DES3_ECB_MODE) ||
					(DpuContext->DpuMode==DES3_CBC_MODE) ||
					(DpuContext->DpuMode==DES3_OFB_MODE) ||
					(DpuContext->DpuMode==DES3_CFB_MODE) ||
					(DpuContext->DpuMode==DES3_CTR_MODE) )
				{
					dpuReverse(&buf[0], 8);
					dpuReverse(&buf[8], 8);
					dpuReverse(&buf[16], 8);
				}
				else
				{
					if(DpuContext->DpuMode!=CSA_MODE)
					{
						dpuReverse(buf, DpuContext->Keylength*4);
					}					
				}
				SetAllKeyToReg( (MMP_UINT32 *)buf, DpuContext->Keylength);
			}
			
			if(DpuContext->Vectorlength)
			{
				memcpy(buf, DpuContext->Vectors, DpuContext->Vectorlength*4);
				dpuReverse(buf, DpuContext->Vectorlength*4);
				SetAllVectorToReg( (MMP_UINT32 *)buf, DpuContext->Vectorlength);
			}

			free(buf);
		}
#else
		SetAllKeyToReg(DpuContext->RefKeyAddress, DpuContext->Keylength);
		SetAllVectorToReg(DpuContext->Vectors, DpuContext->Vectorlength);
#endif
		SetDpuDecodeMode();

#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		if(DpuContext->DpuMode==CSA_MODE)
		{
			SetDpuEndian(DPU_SET_LITTLE_ENDIAN);
		}
		else
		{
			SetDpuEndian(g_DpuEndianSetting);
		}
#endif
		DpuFire();

		#ifdef	DPU_IRQ_ENABLE
		EventRst = SYS_WaitEvent(DpuIsrEvent, 100);
		if(EventRst)
		{
			//printf("Decrypt Wait Evevt Time Out, rst=%x\n",EventRst);
		}
		#else
		if(WaitDpuDone()==MMP_TRUE)		result = MMP_SUCCESS;	//polling DPU status done bit
		#endif

		#if defined(__FREERTOS__)
		  #if defined(__OPENRTOS__)
		  ithInvalidateDCacheRange((MMP_UINT8*)DpuContext->DstAddress, DpuContext->DataSize);
		  #else
		  //ithFlushAhbWrap();
		  or32_invalidate_cache((MMP_UINT8*)DpuContext->DstAddress, DpuContext->DataSize);		  
		  #endif
		#endif

#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		if(DpuContext->DpuMode!=CSA_MODE)
		{
			dpuReverseSrcData((MMP_UINT8 *)DpuContext->DstAddress, DpuContext->DataSize, cipher_len);
		}
#endif

	}
#ifdef  ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	MEM_Release(tem_sbuf);
#endif
	return result;
}

MMP_RESULT mmpDpuInitialize(void)
{
    MMP_RESULT result = MMP_SUCCESS;
	MMP_UINT32 padSelReg = 0;
	MMP_UINT32 pinDirReg = 0;

    LOG_ENTER "[mmpDpuInitialize] Enter\n" LOG_END

	#ifdef	DPU_IRQ_ENABLE
	DpuIntrEnable();
	#endif

    LOG_LEAVE "[mmpDpuInitialize] Leave\n" LOG_END

    return result;
}

#if defined(MM9910) && defined(WIN32)
MMP_RESULT mmpLoadAhbCode(void)
{
	MMP_UINT8  *MemPt=0;

	//HOST_WriteBlockMemory((MMP_UINT32)MemPt, (MMP_UINT32)DPU_AHB_CODE, 0x16D4);	

	return MMP_SUCCESS;
}

MMP_RESULT	mmpLoadWishoneCode(void)
	{
	MMP_UINT8  *MemPt=0;

	//HOST_WriteBlockMemory((MMP_UINT32)MemPt, (MMP_UINT32)DPU_WBB_CODE, 0x16D4);	

	return MMP_SUCCESS;
}
#endif

MMP_RESULT	mmpDpuEncode(DPU_CONTEXT *DpuContext)
{
	MMP_RESULT result = MMP_SUCCESS;

	LOG_ENTER "[mmpDpuEncode] Enter\n" LOG_END

	result = DpuEncode(DpuContext);
	
	LOG_LEAVE "[mmpDpuEncode] Leave\n" LOG_END

	return result;
}

MMP_RESULT	mmpDpuDecode(DPU_CONTEXT *DpuContext)
{
	MMP_RESULT result = MMP_SUCCESS;

	LOG_ENTER "[mmpDpuDecode] Enter\n" LOG_END

	result = DpuDecode(DpuContext);
	
	LOG_LEAVE "[mmpDpuDecode] Leave\n" LOG_END

	return result;
}

MMP_RESULT mmpDpuTerminate(void)
{
    LOG_ENTER "[mmpDpuTerminate] Enter\n" LOG_END

//    MMP_RESULT result = MMP_SUCCESS;
    #ifdef	DPU_IRQ_ENABLE
	DpuIntrDisable();
    #endif

    LOG_LEAVE "[mmpDpuTerminate] Leave\n" LOG_END

    return MMP_RESULT_SUCCESS;
}

void mmpDpuSetRandomKeyGenerateRule(
    MMP_BOOL bFreerun)
{
    if (bFreerun)
    {
        AHB_WriteRegisterMask(DPU_CTRL_REG, 0x1 << 20, 0x1 << 20);
    }
    else
    {
        AHB_WriteRegisterMask(DPU_CTRL_REG, 0x0 << 20, 0x1 << 20);
    }
}
void mmpDpuGenerateRandomData(
    MMP_UINT8* pBuffer,
    MMP_UINT32 dataSize)
{
    MMP_UINT32 alginOffset = ((4 - ((MMP_UINT32) pBuffer & 0x3)) & 0x3);
    MMP_UINT32 loopRun = ((dataSize - alginOffset) >> 2);
    MMP_UINT32 remainSize = ((dataSize - alginOffset) & 0x3);
    MMP_UINT32 i = 0;
    MMP_UINT32 regVal = 0;

    if (alginOffset)
    {
        AHB_ReadRegister(DPU_RANKEY_NUM_REG, &regVal);
        for (i = 0; i < alginOffset; i++)
        {
            pBuffer[i] = (MMP_UINT8)regVal;
        }
    }

    for (i = 0; i < loopRun; i++)
    {
        AHB_ReadRegister(DPU_RANKEY_NUM_REG, &regVal);
        *((MMP_UINT32*) &pBuffer[alginOffset + (i << 2)]) = regVal;        
    }

    if (remainSize)
    {
        AHB_ReadRegister(DPU_RANKEY_NUM_REG, &regVal);

        for (i = 0; i < remainSize; i++)
        {
            pBuffer[dataSize - remainSize + i] = (MMP_UINT8)regVal;
        }
    }
}    

