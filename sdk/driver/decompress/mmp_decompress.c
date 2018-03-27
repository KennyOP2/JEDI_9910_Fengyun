/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Decompress Controller extern API implementation.
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include "mem/mem.h"
#include "mmp_decompress.h"
#include "host/ahb.h"		//for AHB_ReadRegister
#include "sys/sys.h"		//for memcpy() function
#include "host/host.h"

//=============================================================================
//                              macro defination
//=============================================================================
#define DPU_AHB_BASE	0xD0900000

#define DECOMPRESS_SW_ERR_DONEINDEX_OVER    0x00000001
#define DECOMPRESS_HW_FAIL                  0x00000004

/*****************************************************************/
//globol variable
/*****************************************************************/

static MMP_UINT32 g_DecompressIndex;
static MMP_UINT32 g_CmdQueIndex;
static MMP_UINT32 g_CmdQueDoneIndex;
static MMP_UINT32 g_DecompressStatus;
static MMP_UINT32 g_CmdCntIsZero;
static MMP_UINT32 g_IsrCounter;
static MMP_UINT32 g_SwCmdQueDone;


#if defined(DECOMPRESS_IRQ_ENABLE)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#else
#include "intr/intr.h"
#endif
static MMP_EVENT DecompressIsrEvent	= MMP_NULL;
#endif
/*****************************************************************/
//function protocol type
/*****************************************************************/

#if defined(DECOMPRESS_IRQ_ENABLE)
void decompress_isr(void* CmdCnt);
#endif

static MMP_RESULT Decompress(DECOMPRESS_INFO *DecompressInfo);

/*****************************************************************/
//function implement
/*****************************************************************/
#if defined(DECOMPRESS_IRQ_ENABLE)
void decompress_isr(void* CmdCnt)
{
	MMP_UINT32	tmp=0;
	MMP_UINT32	QueCnt=0;
	
	g_IsrCounter++;
	
	//AHB_WriteRegisterMask( DPU_BASE+0x318, 0x0002, 0x0002);	
	AHB_ReadRegister( DPU_BASE+0x31C,&tmp);
	AHB_ReadRegister( DPU_BASE+0x320,&QueCnt);    //read current command queue counter
	
	//printf("$in\n");	
	if(tmp&0x04)
	{
	   //decompress fail
	   printf("decompress fail!!\n");
	   g_DecompressStatus |= DECOMPRESS_HW_FAIL;   //DECOMPRESS_FAIL  0x0004       
    }

	if(tmp&0xF0)
	{
	   //decompress fail
	   g_DecompressStatus |= (tmp&0xF0);   //DECOMPRESS_FAIL  0x0004       
    }

	g_CmdQueDoneIndex = QueCnt;
	
	if(g_CmdQueDoneIndex > g_CmdQueIndex)
	{
        g_DecompressStatus |= DECOMPRESS_SW_ERR_DONEINDEX_OVER;   //DECOMPRESS_FAIL  0x0004 
    }
    if(g_CmdQueDoneIndex == g_CmdQueIndex)
    {
    	//printf("DecIdx++\n");
    	g_DecompressIndex++;
    }
    
    AHB_WriteRegister( 0x7C08, g_IsrCounter);
    
    AHB_WriteRegisterMask( DPU_BASE+0x318, 0x0002, 0x0002);
    AHB_WriteRegisterMask( DPU_BASE+0x318, 0x0000, 0x0002);
    
    SYS_SetEventFromIsr(DecompressIsrEvent);   
    	
	//printf("$[%d,%d]\n",g_CmdQueDoneIndex,g_CmdQueIndex);
}

void DecompressIntrEnable(void)
{
	MMP_UINT32 Reg32;
    //initial Decompress interrupt
	//printf("Enable NAND IRQ~in\n");
	
	ithIntrDisableIrq(ITH_INTR_DECOMPRESS);
	ithIntrClearIrq(ITH_INTR_DECOMPRESS);

	#if defined (__FREERTOS__)
	// register NAND Handler to IRQ
	ithIntrRegisterHandlerIrq(ITH_INTR_DECOMPRESS, decompress_isr, MMP_NULL);
	#endif // defined (__FREERTOS__)

	// set IRQ to edge trigger
	ithIntrSetTriggerModeIrq(ITH_INTR_DECOMPRESS, ITH_INTR_EDGE);

	// set IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(ITH_INTR_DECOMPRESS, ITH_INTR_HIGH_RISING);
	
	if(!DecompressIsrEvent)
	    DecompressIsrEvent = SYS_CreateEvent();
	    
	// Enable IRQ
	ithIntrEnableIrq(ITH_INTR_DECOMPRESS);
	//AHB_WriteRegisterMask( DPU_BASE+0x318, 0x01, 0x01);	
	//AHB_ReadRegister( DPU_BASE+0x318, &Reg32);
	//printf("Reg7918=[%x]\n",Reg32);
	
	//printf("Enable Decompress IRQ~~leave\n");     
}

void DecompressIntrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_DECOMPRESS);
}
#endif //DECOMPRESS_IRQ_ENABLE

MMP_RESULT	DecompressInitial()
{
    //initial Decompress register
    printf("DecompressInitial()!!\n");
    g_DecompressIndex   = 0;
    g_CmdQueIndex       = 0;
    g_CmdQueDoneIndex   = 0;
    g_DecompressStatus  = 0;
    g_CmdCntIsZero      = 0;
    
    g_IsrCounter		= 0;//for test
    g_SwCmdQueDone		= 0;//for test
    //AHB_WriteRegister( DPU_BASE+0x304,0x01);    //clear CmdQue counter
    
    #if defined(DECOMPRESS_IRQ_ENABLE) 
    DecompressIntrEnable();
    #endif
    return MMP_TRUE;
}

MMP_RESULT Decompress(DECOMPRESS_INFO *DecInfo)
{
	unsigned long	i=0, j = 0;
    unsigned long	in_len ;
    unsigned long	out_len;
	unsigned long	Reg32;
	MMP_UINT32 		CmdQueCnt=0;
	static MMP_UINT8 AddrCnt=0;	
	MMP_UINT8		IsEnanleDpuCmdq = DecInfo->IsEnableComQ;
	MMP_UINT8      *SrcBuff = DecInfo->srcbuf;
	MMP_UINT8      *DstData = 0;	
	MMP_INT    		EventRst;
	
    #if defined(__FREERTOS__)
    MMP_UINT8      *DstDataAddr = malloc(DecInfo->dstLen+16);
    MMP_UINT8      *SrcData = DecInfo->srcbuf;
	#else
	MMP_UINT8      *SrcData = malloc(DecInfo->srcLen+16);
	MMP_UINT8      *DstDataAddr = malloc(DecInfo->dstLen+16);	
	MMP_UINT8      *MemData = SYS_Malloc(DecInfo->dstLen+16);	
	#endif

	#ifdef TestCode	
	AddrCnt=(++AddrCnt)%0x04;
	DstData = (MMP_UINT8 *)(DstDataAddr+AddrCnt);
	#else
	DstData = DecInfo->dstbuf;
	#endif
	
    #if defined(__FREERTOS__)
    //memset(DstData, 0xAA, DecInfo->dstLen);
    #else
    //memset(MemData, 0xAA, DecInfo->dstLen);
	HOST_WriteBlockMemory((MMP_UINT32)SrcData, (MMP_UINT32)SrcBuff, DecInfo->srcLen);
	HOST_WriteBlockMemory((MMP_UINT32)DstData, (MMP_UINT32)MemData , DecInfo->dstLen);
    #endif

	//AHB_WriteRegister( DPU_BASE+0x304,0x01);    //clear CmdQue counter

    for (;;)
    {    
        int      r = 0;

        //get src data length & get output data length
		out_len = ((SrcBuff[i+3]) | (SrcBuff[i+2]<<8) | (SrcBuff[i+1]<<16) | (SrcBuff[i]<<24));
        i=i+4;
        in_len = ((SrcBuff[i+3]) | (SrcBuff[i+2]<<8) | (SrcBuff[i+1]<<16) | (SrcBuff[i]<<24));
        i=i+4;

        if (out_len == 0)
		{
			//printf("End of DecompressData, InLen=%08x, OutLen=%08x!\n", in_len, out_len);
			break;
		}
		if ( (out_len > 0x4000) || (in_len > 0x4000) )
		{
			printf("in out length is error, InLen=%08x, OutLen=%08x!\n", in_len, out_len);
		}
		//printf("in,out=[%u,%u]\n",in_len,out_len);

//		CmdQueCnt = 1;
        if (in_len < out_len)
        {
			if(IsEnanleDpuCmdq)
			{
				/*
				CmdQ_Lock();
				CmdQ_WaitSize(8*5);
				CmdQ_PackCommand(DPU_AHB_BASE+0x108, (MMP_UINT32)&SrcData[i]);
				CmdQ_PackCommand(DPU_AHB_BASE+0x10C, (MMP_UINT32)&DstData[j]);
				CmdQ_PackCommand(DPU_AHB_BASE+0x114, out_len);
				CmdQ_PackCommand(DPU_AHB_BASE+0x110, in_len);
				CmdQ_PackCommand(DPU_AHB_BASE+0x100,0x00000012);			//fire DPU
				CmdQ_Fire();
				CmdQ_Unlock();
				CmdQueCnt++;
				*/
				//printf("CmdQueCnt=%d\n",CmdQueCnt);
			}
			else
			{
			//	AHB_WriteRegister(DPU_BASE+0x300,0x00000000);			//DPU enviroment setting
				AHB_WriteRegister(DPU_BASE+0x308,(MMP_UINT32)&SrcData[i]);	//source start address in byte
				AHB_WriteRegister(DPU_BASE+0x30C,(MMP_UINT32)&DstData[j]);	//distination start address
				//printf("SrcData = %x \n",&SrcData[j]);     
				//printf("Dsraddr = %x \n",&DstData[j]);  
				AHB_WriteRegister(DPU_BASE+0x314,out_len);				//decompress desitination size in byte
				AHB_WriteRegister(DPU_BASE+0x310,in_len);				//source size in byte

/*
				AHB_ReadRegister( DPU_BASE+0x308,&Reg32);		
				printf("0x7908 = %x \n",Reg32);
				AHB_ReadRegister( DPU_BASE+0x30c,&Reg32);
				printf("0x790c = %x \n",Reg32);
				AHB_ReadRegister( DPU_BASE+0x31C,&Reg32);			
				printf("0x791c = %x \n",Reg32);
				AHB_ReadRegister( DPU_BASE+0x314,&Reg32);
				printf("0x7914 = %x \n",Reg32);
*/

				AHB_WriteRegister(DPU_BASE+0x300,0x00000012);			//fire DPU
				
				//printf("fired~~\n");
				#if defined(DECOMPRESS_IRQ_ENABLE)
				EventRst = SYS_WaitEvent(DecompressIsrEvent, 100);
				#endif

				do
				{
					AHB_ReadRegister( DPU_BASE+0x31C,&Reg32);			//check DPU done bit
					//printf("w..\n");
					MMP_Sleep(0);
				} while((Reg32&0x00000001) != 0x00000001);

/*
				if( (MMP_UINT32)(&SrcData[i]) % 0x04)
				{
					printf("Warning, address alignment issue!! s=%x,d=%x\n",&SrcData[i],&DstData[j]);
				}
*/
				if(Reg32&0x04)
				{
					printf("Error status hasppened, StatusReg=[%x]; i,j=[%x,%x][%x,%x] add=[%x,%x]!!\n", 
															Reg32, i, j,in_len,out_len,&SrcData[i],&DstData[j]);
					//HOST_ReadBlockMemory((MMP_UINT32)dst, (MMP_UINT32)SrcData, DataSize);
					break;
				}
				AHB_ReadRegister( DPU_BASE+0x314,&Reg32);				//check DPU decompress desitination size in byte
				out_len =  Reg32;										//get decompressed size
			}
            i += in_len;
            j += out_len;	
        }
        else 
        {
            #if defined(__FREERTOS__)
            memcpy(&DstData[j], &SrcData[i], in_len);
            #else
			HOST_WriteBlockMemory((MMP_UINT32)&DstData[j], (MMP_UINT32)&SrcBuff[i], in_len);
			#endif
            i += in_len;
            j += in_len;
        }
    }

	if(IsEnanleDpuCmdq)
	{
	    #if defined(DECOMPRESS_IRQ_ENABLE)	 
		//MMP_INT    EventRst;	  		     
	    //EventRst = SYS_WaitEvent(DecompressIsrEvent, 100);
	    //printf("got event !!\n");
	    //while(1);
	    //do nothing

		g_CmdQueIndex += CmdQueCnt; 
		if(CmdQueCnt)
		{
			g_CmdCntIsZero=0;
		}
		else
		{
			g_CmdCntIsZero=1;
		}
		printf("CurrCmdQueIdx=%d,gCmdQueIdx=%d\n",CmdQueCnt,g_CmdQueIndex);	

	    #else
		do 
		{
			AHB_ReadRegister( DPU_BASE+0x320,&Reg32);
		} while( Reg32!=CmdQueCnt );
		g_CmdQueIndex += CmdQueCnt; 
		CmdQueCnt=0;
		printf("CmdQue finished!!\n");		
		#endif
	}
	
	#if defined(__FREERTOS__)
	    #if defined(DECOMPRESS_IRQ_ENABLE)	    
	    //EventRst = SYS_WaitEvent(DecompressIsrEvent, 20);
	    //printf("waiting ISR !!\n");
	    //while(1);
	    //do nothing
	    #endif
	if(((MMP_UINT32)DstData%4)!=0)
	{
		memcpy( DecInfo->srcbuf, DstData, DecInfo->dstLen);
	}	
	#else
	if(((MMP_UINT32)DstData%4)!=0)
	{
		MMP_UINT8 offset=(MMP_UINT8)((MMP_UINT32)DstData%4);
		HOST_ReadBlockMemory((MMP_UINT32)MemData, (MMP_UINT32)(DstData-offset), DecInfo->dstLen+4);
		memcpy(DecInfo->srcbuf, MemData+offset, DecInfo->dstLen);
	}	
	else
	{
		HOST_ReadBlockMemory((MMP_UINT32)DecInfo->dstbuf, (MMP_UINT32)DstData, DecInfo->dstLen);
	}
    #endif

    #if defined(__FREERTOS__)
    free(DstDataAddr);
    #else
    SYS_Free(MemData);
	free(SrcData);
	free(DstDataAddr);
	#endif

	//g_DecompressIndex++;

    return 0;
}

MMP_RESULT mmpDecompessInitial(void)
{
	MMP_RESULT result;
	
    LOG_ENTER "[mmpDpuTerminate] Enter\n" LOG_END

    result = DecompressInitial();

    LOG_LEAVE "[mmpDpuTerminate] Leave\n" LOG_END

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT mmpDecompress(DECOMPRESS_INFO *DecInfo)
{
	MMP_RESULT result = MMP_SUCCESS;

	LOG_ENTER "[mmpDecompress] Enter\n" LOG_END

	result = Decompress(DecInfo);

	LOG_LEAVE "[mmpDecompress] Leave\n" LOG_END

	return result;
}

MMP_RESULT	mmpDpuGetDecompressSize( MMP_UINT8 *Src, MMP_UINT8 *dst)
{

	return	0;
}

MMP_RESULT	mmpGetDecompressStatus(DECOMPRESS_STATUS *DecompressStatus)
{
	if(g_CmdCntIsZero)
	{
		g_DecompressStatus |= 0x10000000;
	}
	else
	{
		g_DecompressStatus &= ~0x10000000;
	}
	
	if( g_CmdQueDoneIndex==(g_CmdQueIndex-1) )
	{
		MMP_UINT32 QueCnt=0; 
		AHB_ReadRegister( DPU_BASE+0x320,&QueCnt);
		if( QueCnt==g_CmdQueIndex )
		{
			printf("error, ISR not done!!\n");
			g_CmdQueDoneIndex++;
			g_DecompressIndex++;
		}
	}

	DecompressStatus->DecompressIndex  = g_DecompressIndex; 
	DecompressStatus->CmdQueIndex      = g_CmdQueIndex;
	DecompressStatus->CmdQueDoneIndex  = g_CmdQueDoneIndex;
	DecompressStatus->DecompressStatus = g_DecompressStatus;

	return	0;
}

MMP_RESULT mmpDecompessTerminate(void)
{
    LOG_ENTER "[mmpDpuTerminate] Enter\n" LOG_END

    g_DecompressIndex   = 0;
    g_CmdQueIndex       = 0;
    g_CmdQueDoneIndex   = 0;
    g_DecompressStatus  = 0;

    LOG_LEAVE "[mmpDpuTerminate] Leave\n" LOG_END

    return MMP_RESULT_SUCCESS;
}
