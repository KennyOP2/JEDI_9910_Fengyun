/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

/****** windows file system ******/
#ifdef  WIN32
#include <Windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef  WIN32
#include <io.h>
#endif
/****** windows file system ******/

#include "../include/mmp_decompress.h"
#include "pal/pal.h"

#include "../share/ucl/include/ucl/uclconf.h"
#include "../share/ucl/include/ucl/ucl.h"


#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "mem/mem.h"
#endif

#include "../../include/host/ahb.h"
#include "../../include/mmp_types.h"
#include "../../include/host/host.h"
//#include "../../include/cmq/cmd_queue.h"

#define		CALCULATE_CRC_SW
#define		fprintf		//
#define		__COMPRESS_BUFFER__
#define		ENABLE_MEM_ALLOC_FUNCTION

#define SAFE
#if defined(SAFE)
# if defined(__DEBUG__)
#  define fail(x,r)   if (x) { printf("%s #%d\n", __FILE__, __LINE__); *dst_len = olen; return r; }
# else
#  define fail(x,r)   if (x) { *dst_len = olen; return r; }
# endif // __DEBUG__
#else
# define fail(x,r)
#endif // SAFE

# define GETBYTE(src)  (src[ilen++])
# define GETBIT(bb, src)  (((bb = ((bb & 0x7f) ? (bb*2) : ((unsigned)GETBYTE(src)*2+1))) >> 8) & 1)


#define		LEVEL				(10)
#define		MAX_READ_SIZE		(0x800000)	//0x800000 = 8MB
#define		CPU_SRC_DATA_SIZE	(0x8000)

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/////////////////////////////////////////////////////////////////
//                      Constant Decleration
/////////////////////////////////////////////////////////////////
//MMP_UINT32 result[2048];
MMP_UINT32 i=0;
MMP_UINT32 err=0;

static unsigned long total_in = 0;
static unsigned long total_out = 0;
static int block_size = 1024*16;
static unsigned char decompress_pt[100000];
static unsigned char result[2000000];

#ifdef	ENABLE_MEM_ALLOC_FUNCTION
static MMP_UINT8	*SrcCompressData	= MMP_NULL;
static MMP_UINT8	*DpuDecompressData	= MMP_NULL;
static MMP_UINT8	*CpuDecompressData	= MMP_NULL;
static MMP_UINT8	*CpuSrcData 		= MMP_NULL;
#else
static MMP_UINT8	SrcCompressData[CPU_SRC_DATA_SIZE+1024];
static MMP_UINT8	DpuDecompressData[CPU_SRC_DATA_SIZE+1024];
static MMP_UINT8	CpuDecompressData[CPU_SRC_DATA_SIZE+1024];
static MMP_UINT8	CpuSrcData[CPU_SRC_DATA_SIZE+1024];
#endif

typedef struct ErrTable
{
	MMP_UINT16   address;
	MMP_UINT8    SrcByte;
	MMP_UINT8    DstByte;
}ERROR_TABLE;

static ERROR_TABLE	ErrorTable[1024];
static MMP_UINT32 ErrCnt=0;

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

/////////////////////////////////////////////////////////////////
//                      function Decleration
/////////////////////////////////////////////////////////////////
static void SW_CRC_CALCULATE(MMP_UINT32* crc_src_pt,MMP_UINT32* result_pt);
static MMP_UINT8 CompressionTest(void);
static MMP_UINT8 EcbEncodeTest(void);
static void CPU_Compress(MMP_UINT8	argc, char* argv[]);
//static void CPU_DecompressBuff(MMP_UINT8* SrcCompressData, MMP_UINT8* CpuDecompressData);
static void CPU_Decompress(MMP_UINT8 argc, char* argv[]);
static void DoTest(void);

static int decompress( const unsigned char* src, unsigned int  src_len,
                unsigned char* dst, unsigned int* dst_len,FILE* information);

static int conv_num(char *str);
static ucl_uint xread(FILE * f, ucl_voidp buf, ucl_uint len);
static ucl_uint xwrite(FILE * f, const ucl_voidp buf, ucl_uint len);
static void xwrite32(FILE *f, ucl_uint32 v);
static ucl_uint xreadbuff(FILE * f, ucl_voidp buf, ucl_uint len);
static ucl_uint xwritebuff(char *buff, char *buf, ucl_uint len);
static void xwrite32buff(char *buff, ucl_uint32 v);

static MMP_UINT8 compare2buffer(MMP_UINT8 * SrcBuffer, MMP_UINT8 * DstBuffer,MMP_UINT32 count );
static MMP_UINT8 compare2File(char* File1[], char* File2[]);

#if defined(__COMPRESS_BUFFER__)
static void AutoGenPatternBuff(MMP_UINT8 *srcbuff, MMP_UINT32 size);
static MMP_UINT32 CPU_CompressBuff(MMP_UINT8 *CpuSrcData, MMP_UINT8 *CpuDstData);
static MMP_UINT32 do_compress(MMP_UINT8 *bi, MMP_UINT8 *bo);
#else
static void AutoGenPatternFile(MMP_UINT8	argc, char* argv[]);
static MMP_UINT8 do_compress(FILE *fi, FILE *fo);
#endif

/////////////////////////////////////////////////////////////////
//                      function implement
/////////////////////////////////////////////////////////////////
MMP_UINT8 CompressionTest(void)
{
	MMP_UINT8	argc=3;
	#if defined(__COMPRESS_BUFFER__)
	#else
	char* argv[]={"-b","16384","D:\\in.bin","D:\\out.bin"};
	char* dpu_in[] = {"D:\\dpu_in.bin"};
	char* cpu_result[] = {"D:\\result.bin"};
	#endif
	
	MMP_UINT32	TotalSrcFileSize=0,length,TempRdSz=1024*128;
	FILE *fin = NULL, *fout = NULL;
	MMP_UINT8	CprResult=MMP_TRUE;

	#if defined(__COMPRESS_BUFFER__)
	//AutoGenPatternBuff(CpuSrcData, CPU_SRC_DATA_SIZE);
	AutoGenPatternBuff(SrcCompressData, CPU_SRC_DATA_SIZE);
	//printf("SrcBuff=%x,Size=%x\n",SrcCompressData,CPU_SRC_DATA_SIZE);
	TotalSrcFileSize = CPU_CompressBuff(SrcCompressData, CpuDecompressData);
	//CPU_DecompressBuff(CpuDecompressData, argv);
	//CprResult = compare2File(&argv[2],cpu_result);
	#else
	AutoGenPatternFile(argc, argv);
	CPU_Compress(argc, argv);
	CPU_Decompress(argc, argv);
	CprResult = compare2File( &argv[2], cpu_result);
	#endif

	if(CprResult==MMP_TRUE)//SrcCompressData && CpuDecompressData)
	{
		#if defined(__COMPRESS_BUFFER__)
		#else
		//open a file as the CPU source data
		if ((fout = fopen(argv[3], "rb")) == NULL) 
		{
			printf("Open %s fails\n", argv[3]);
			//exit(-1);
			CprResult = MMP_FALSE;
		}
		else
		{
			//fread data into a buffer
			memset(CpuDecompressData, 0xFF, 1024*1024);

			while(1)
			{
				length = (MMP_UINT32)fread(&CpuDecompressData[TotalSrcFileSize], sizeof(char), TempRdSz, fout);
				TotalSrcFileSize += length;
				if(length<TempRdSz)	break;
			}
		}
		fclose(fout);
		#endif
		
		#if defined(__COMPRESS_BUFFER__)
        #else
		{
			unsigned long	i=0;
			unsigned long	in_len ;
			unsigned long	out_len;
			unsigned char	*src=(unsigned char	*)CpuDecompressData;
			for (;;)
			{
				out_len = ((src[i+3]) | (src[i+2]<<8) | (src[i+1]<<16) | (src[i]<<24));
				i=i+4;
				in_len = ((src[i+3]) | (src[i+2]<<8) | (src[i+1]<<16) | (src[i]<<24));
				i=i+4;
				if (out_len == 0)
				{
					break;
				}
				
				if(in_len==out_len)
				{
					//skip
					return MMP_TRUE;
				}
				i += in_len;
			}
		}
		#endif


		if(TotalSrcFileSize>CPU_SRC_DATA_SIZE)
		{
			printf("ERROR, file size large than 1MB\n");
		}

		//open file "d:\in.bin"
		#if defined(__COMPRESS_BUFFER__)
        #else
		TotalSrcFileSize = 0;
		if ((fin = fopen(argv[2], "rb")) == NULL) 
		{
			printf("Open %s fails\n", argv[2]);
			CprResult = MMP_FALSE;
		}
		else
		{
			//fread data into a buffer
			while(1)
			{
				length = (MMP_UINT32)fread(&SrcCompressData[TotalSrcFileSize], sizeof(char), TempRdSz, fin);
				TotalSrcFileSize += length;			
				if(length<TempRdSz)	
				{
					break;
				}
			}
		}
		fclose(fin);
		#endif

		if(CprResult == MMP_TRUE)
		{
		    DECOMPRESS_INFO   DecInfo;
		    static MMP_ULONG	starttime=0;
		    MMP_ULONG			duration=0;
		    MMP_UINT32			Preidx=0;
		    DECOMPRESS_STATUS 	DecStatus;

		    duration = PalGetDuration(starttime);
		    printf("around dur=%d\n",duration);

			DecInfo.srcbuf = CpuDecompressData;
			DecInfo.dstbuf = DpuDecompressData;
			DecInfo.srcLen = TotalSrcFileSize;     //buffer size or real file size?
			DecInfo.dstLen = CPU_SRC_DATA_SIZE;     //buffer size or
			DecInfo.IsEnableComQ = 0;    			//control the decompress engine with CmdQ or not

			if(DecInfo.IsEnableComQ)
			{
				mmpGetDecompressStatus(&DecStatus);
				Preidx = DecStatus.DecompressIndex;
			}

			starttime = PalGetClock();
			mmpDecompress(&DecInfo);
			duration = PalGetDuration(starttime);
			printf("HW decompress duration=%d\n",duration);

			if(DecInfo.IsEnableComQ)
			{
				MMP_UINT32 TimeOutCnt=0;

				while(1)
				{
					DECOMPRESS_STATUS DecStatus;

					mmpGetDecompressStatus(&DecStatus);
					TimeOutCnt++;
					if( !(TimeOutCnt%100) )
					{
						printf("Preidx=%u,HwIdx=%u,TimeOutCn=%u,CmdQc[%u,%u]\n", Preidx, DecStatus.DecompressIndex, TimeOutCnt, DecStatus.CmdQueDoneIndex, DecStatus.CmdQueIndex);
					}
					if(TimeOutCnt>10000)
					{
						printf("Error, TimeOutCnt>10000!!\n");
						while(1);
					}

					if(DecStatus.DecompressStatus&0x10000000)
					{
						break;
					}

					if(DecStatus.DecompressIndex>Preidx)
					{
						printf("exit while(1) got status[%x,%x]!!\n",Preidx,DecStatus.DecompressIndex);
						break;
					}
				}
			}
/*
			if ((fout = fopen(dpu_in[0], "wb")) == NULL) 
			{
				printf("Open %s fails\n", dpu_in[0]);
				CprResult = MMP_FALSE;
			}            
			else
			{
				length = (MMP_UINT32)fwrite(DpuDecompressData, sizeof(char), TotalSrcFileSize , fout);
			}
			fclose(fout);
*/

			if(CprResult == MMP_TRUE)//if(compare2buffer(SrcCompressData, DpuDecompressData, 16384 )!=MMP_TRUE)
			{
				//CprResult = compare2buffer(SrcCompressData, DpuDecompressData, TotalSrcFileSize );
				CprResult = compare2buffer(SrcCompressData, DpuDecompressData, CPU_SRC_DATA_SIZE );//unfinished
			}
/*
			if(CprResult == MMP_TRUE)
			{
				CprResult = compare2File(&argv[2],dpu_in);//unfinished
			}
*/
		}
	}
	else
	{
		printf("CPU decompress fail\n");
	}

	return CprResult;
}

MMP_UINT8 compare2File(char* File1[], char* File2[] )
{
	FILE *f1 = NULL, *f2 = NULL;
	MMP_UINT8	CompareResult=MMP_TRUE;
	MMP_UINT32	BUFFER_SIZE=65536;
	//char* argv[]={"-b","16384","D:\\in.bin","C:\\out.bin"};
	MMP_UINT8	argi=0;	
	MMP_UINT8*  file1buffer= (MMP_UINT8*)SYS_Malloc(BUFFER_SIZE);
	MMP_UINT8*  file2buffer= (MMP_UINT8*)SYS_Malloc(BUFFER_SIZE);

	//check buffer
	if(!file1buffer)
	{
		printf("file1buffer==NULL\n");
		CompareResult=MMP_FALSE;
	}
	if(!file2buffer)
	{
		printf("file2buffer==NULL\n");
		CompareResult=MMP_FALSE;
	}

	//1.open file1 & file2
	if ((f1 = fopen(File1[0], "rb")) == NULL) 
	{
		printf("Open %s fails\n", File1[0]);
		CompareResult=MMP_FALSE;
		//goto end
	}

	if ((f2 = fopen(File2[0], "rb")) == NULL) 
	{
		printf("Open %s fails\n", File2[0]);
		CompareResult=MMP_FALSE;
		//goto end
	}

	//2.read file into a buffer & compare buffer
	if(CompareResult==MMP_TRUE)
	{
		MMP_UINT32 length1,length2;
		while(1)
		{
			length1 = (MMP_UINT32)fread(file1buffer, sizeof(char), BUFFER_SIZE, f1);
			//TotalSrcFileSize += length1;
			length2 = (MMP_UINT32)fread(file2buffer, sizeof(char), BUFFER_SIZE, f2);

			if(length1!=length2)
			{
				CompareResult=MMP_FALSE;
				break;
			}

			if(compare2buffer(file1buffer, file2buffer, length1 )!=MMP_TRUE)
			{
				CompareResult=MMP_FALSE;
				break;
			}
		
			if(length1<BUFFER_SIZE)
			{
				//CompareResult=MMP_FALSE;
				break;
			}
		}
	}

	//close 2 files
	fclose(f1);
	fclose(f2);
	SYS_Free(file1buffer);
	SYS_Free(file2buffer);

	return CompareResult;

}

int main(void)
{
	// ********* read ID command *******
	printf("Decompress TEST !!!!\n");

#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    ret = xTaskCreate(main_task_func, "decompresstest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#else
	DoTest();
#endif

	printf("DeCompression TEST end !!\n");
    while(1);
}

void DoTest(void)
{
	MMP_UINT32 Reg32;
	MMP_RESULT	ret;

	printf("DoTest[0]\n");

	//CmdQ_Initialize();
	printf("DoTest[1]\n");

	ret = mmpDecompessInitial();
	printf("DoTest[2]\n");

	#ifdef	ENABLE_MEM_ALLOC_FUNCTION
	if(SrcCompressData==MMP_NULL)
	{
		SrcCompressData = malloc(CPU_SRC_DATA_SIZE+1024);
	}
	if(DpuDecompressData==MMP_NULL)
	{
		DpuDecompressData = malloc(CPU_SRC_DATA_SIZE+1024);
	}
	if(CpuDecompressData==MMP_NULL)
	{
		CpuDecompressData = malloc(CPU_SRC_DATA_SIZE+1024);
	}
	printf("DoTest[3],Src=%x,Dpu=%x,Cpu=%x\n",SrcCompressData,DpuDecompressData,CpuDecompressData);
	#endif

//compression test//
	Reg32=1;
	while(1)
	{
		//initial mem
		memset(SrcCompressData, 0xFF, CPU_SRC_DATA_SIZE);
		memset(DpuDecompressData, 0xFF, CPU_SRC_DATA_SIZE);
		memset(CpuDecompressData, 0xFF, CPU_SRC_DATA_SIZE);

		//printf("AddrOfCpuCompressData=%x,AddrOfDpuCompressData=%x,[DpuDecompressAddess=%08x]\n",SrcCompressData,CpuDecompressData,CpuDecompressData);
		//printf("Error table, ErrCnt=%d,ErrAddr=%x, Src,Dst=[%x,%x]\n", ErrCnt, ErrorTable[ErrCnt].address, ErrorTable[ErrCnt].SrcByte, ErrorTable[ErrCnt].DstByte);

		if( CompressionTest()==MMP_FALSE)
		{
			printf("DeCompression fail, count = %d !!\n\n",Reg32++);
			//break;
		}
		else
		{
			printf("Error table, ErrCnt=%d,ErrAddr=%x, Src,Dst=[%x,%x]\n", ErrCnt, ErrorTable[ErrCnt].address, ErrorTable[ErrCnt].SrcByte, ErrorTable[ErrCnt].DstByte);
			printf("Total check count = %d !!\n\n",Reg32++);
		}
	}

	#ifdef	ENABLE_MEM_ALLOC_FUNCTION
	if(SrcCompressData)
	{
		free(SrcCompressData);
		SrcCompressData=MMP_NULL;
	}
	if(DpuDecompressData)
	{
		free(DpuDecompressData);
		DpuDecompressData=MMP_NULL;
	}
	if(CpuDecompressData)
	{
		free(CpuDecompressData);
		CpuDecompressData=MMP_NULL;
	}
	#endif

	printf("DeCompression TEST end !!\n");
    while(1);
}
/*************************************************************************
// file IO
**************************************************************************/
ucl_uint xread(FILE * f, ucl_voidp buf, ucl_uint len)
{
    ucl_uint l;

    l = (ucl_uint) fread(buf, sizeof(char), len, f);
    total_in += l;
    return l;
}

ucl_uint xwrite(FILE * f, const ucl_voidp buf, ucl_uint len)
{
    ucl_uint l;

    if (f != NULL && len != 0) {
        l = (ucl_uint) fwrite(buf, sizeof(char), len, f);
        if (l != len) {
            printf("Data write error!!\n");
            exit(1);
        }
    }
    total_out += len;
    return len;
}

void xwrite32(FILE *f, ucl_uint32 v)
{
    unsigned char b[4];

    b[3] = (unsigned char) (v >>  0);
    b[2] = (unsigned char) (v >>  8);
    b[1] = (unsigned char) (v >> 16);
    b[0] = (unsigned char) (v >> 24);
    xwrite(f,b,4);
}

int conv_num(char *str) {
    int len = strlen(str);

    if (len <= 1) return 0;

    if (str[len-1] == 'k' || str[len-1] == 'K') {
        str[len-1] = 0;
        return atoi(str) * 1024;
    } else if (str[len-1] == 'm' || str[len-1] == 'M') {
        str[len-1] = 0;
        return atoi(str) * 1024 * 1024;
    } else {
        return atoi(str);
    }
}

/*************************************************************************
// buffer IO
**************************************************************************/
/*
ucl_uint xreadbuff(char *oribuff, ucl_uint32 *idx, char *inbuf, ucl_uint len)
{
    ucl_uint l;

    //l = (ucl_uint) fread(buf, sizeof(char), len, f);
    memcpy(&oribuff[idx], inbuf, len);
    l = len;
    total_in += l;
    return l;
}
*/

ucl_uint xwritebuff(char *outbuff, char *inbuf, ucl_uint len)
{
    ucl_uint l;

    if (len != 0)
    {
        memcpy(outbuff, inbuf, len);
        l=len;
        if (l != len)
        {
            printf("Data write error!!\n");
            exit(1);
        }
    }
    total_out += len;
    return len;
}

void xwrite32buff(char *buff, ucl_uint32 v)
{
    unsigned char b[4];

    b[3] = (unsigned char) (v >>  0);
    b[2] = (unsigned char) (v >>  8);
    b[1] = (unsigned char) (v >> 16);
    b[0] = (unsigned char) (v >> 24);
    memcpy(buff, b, 4);
}


#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
	printf("Encryption/descryption FreeRtos task TEST:\n");

	DoTest();

	printf("The END of Encryption/descryption FreeRtos task TEST:(while(1))\n");
	while(1);
}
#endif




/*
int DPU_Decompress(SrcCompressData, DpuDecompressData)
{
    //unsigned int DPU_BASE = 0xE0900000; 
	unsigned long	j = 0;
    unsigned int	in_len ;
    unsigned int	out_len;
	unsigned long	Reg32;

    for (;;)
    {    
        int      r = 0;

        out_len = ((decompress_pt[i+3]) | (decompress_pt[i+2]<<8) | (decompress_pt[i+1]<<16) | (decompress_pt[i]<<24));
        i=i+4;
        in_len = ((decompress_pt[i+3]) | (decompress_pt[i+2]<<8) | (decompress_pt[i+1]<<16) | (decompress_pt[i]<<24));
        i=i+4;

        if (out_len == 0)	{break;}

        if (in_len < out_len)
        {
            AHB_WriteRegister(DPU_BASE+0x00,0x21115054);			//DPU enviroment setting
            AHB_WriteRegister(DPU_BASE+0x04,&decompress_pt[i]);		//source start address in byte
            AHB_WriteRegister(DPU_BASE+0x08,&result[j]);			//distination start address
            AHB_WriteRegister(DPU_BASE+0x54,out_len);				//decompress desitination size in byte
            AHB_WriteRegister(DPU_BASE+0x0c,in_len);				//source size in byte
            AHB_WriteRegister(DPU_BASE+0x00,0x21115056);			//fire DPU

			do
			{
				AHB_ReadRegister( DPU_BASE+0x14,&Reg32);    //check DPU done bit
			} while((Reg32&0x00000001) != 0x00000001);

			AHB_ReadRegister( DPU_BASE+0x54,&Reg32);    //check DPU done bit

            out_len <=  Reg32;							//get compressed size
            i += in_len;
            j += out_len;
        }
        else 
        {
            memcpy(&result[j], &decompress_pt[i], in_len);
            i += in_len;
            j += in_len;
        }

		AHB_ReadRegister( DPU_BASE+0x14,&Reg32);    //check DPU done bit
		if( (Reg32&0x00000004) != 0x00000000 )
		{
	            break;
        }
    }
    return 0;
}
*/
MMP_UINT32 CPU_CompressBuff(MMP_UINT8 *CpuSrcBuf, MMP_UINT8 *CpuDstBuf)
{
	MMP_UINT8   ret;
	MMP_UINT32	TotalOutSize=0;

    TotalOutSize = do_compress(CpuSrcBuf, CpuDstBuf);
	return TotalOutSize;
}

void CPU_Compress(MMP_UINT8	argc, char* argv[])
{
	//FILE*       pfile = NULL;
    FILE *fin = NULL, *fout = NULL;
	//MMP_UINT8	argc=3;
	//MMP_UINT32	block_size;
	//char* argv[]={"-b","16384","D:\\in.bin","C:\\out.bin"};
	MMP_UINT8	argi=0;	
	MMP_UINT8   ret;	


	//printf("argv[0]~[3]=[%s,%s,%s,%s]\n",argv[0],argv[1],argv[2],argv[3]);
	//while(1);

    if (argc < 3) 
	{
        printf("Usage: compress [-b size] in.bin out.bin\n\n");
        printf("       -b size  block size (default 1024)\n");
        printf("                the size must power of 2, and the size should >= 1K\n");
        printf("                and < 8M bytes\n\n");
        printf("Example:\n\n");
        printf("    compress -b 2048 in.bin out.bin\n");
        printf("    compress -b 16K in.bin out.bin\n");
        printf("    compress -b 2M in.bin out.bin\n");
        //exit(-1); 
		//break;
    }

    //argv++;
	//argi++;
	//argc--;

    while(argc) 
	{
        //if (!strcmp(argv[0], "-b")) 
		if (!strcmp(argv[argi], "-b")) 
		{
            argc--; 
			argi++;
			//argv++;

            block_size = conv_num(argv[argi]);    //this block_size is fixed in 0x16KB
            if (block_size < 1024 || block_size > 8*1024*1024L) {
                printf("The block size should >= 1K and < 8M bytes\n");
                //exit(-1);
				break;
            }
        } 
		else	//sprintf(pszBuffer, "C:\\tsi_%d.ts", index);
		{
            if ((fin = fopen(argv[argi], "rb")) == NULL) 
			{
                printf("Open %s fails\n", argv[argi]);
                //exit(-1);
            }
            argc--; 
			argi++;
			//argv++;
            if (argc >= 0) 
			{
                if ((fout = fopen(argv[argi], "wb")) == NULL) 
				{
                    printf("Open %s fails\n", argv[argi]);
                    //exit(-1);
					break;
                }
            } 
			else 
			{
                printf("Please specify the output file\n");
                //exit(-1);
				break;
            }
        }
		if(argc)
		{
			argc--; 
			argi++;
		}
		//argv++;
    }

    if (fin == NULL) 
	{
        printf("Please specify the input file\n");
        //exit(-1);
		//break;
    }

    if (fout == NULL) 
	{
        printf("Please specify the input file\n");
        //exit(-1);
    }

    if (ucl_init() != UCL_E_OK) 
	{
        printf("Abnormal error!!\n");
        //exit(1);
    }

	#if defined (__COMPRESS_BUFFER__)
	#else
    ret = do_compress(fin, fout);
	#endif

    fclose(fin);
    fclose(fout);

    //return 0;
}


/*************************************************************************
// compress
**************************************************************************/
#if defined (__COMPRESS_BUFFER__)
MMP_UINT32 do_compress(MMP_UINT8 *bi, MMP_UINT8 *bo)
#else
MMP_UINT8 do_compress(FILE *fi, FILE *fo)
#endif
{
    int r = 0;
    ucl_bytep in = NULL;
    ucl_bytep out = NULL;
    ucl_uint in_len;
    ucl_uint out_len;
    ucl_uint32 CurrWtIdx=0;

    total_in = total_out = 0;

    //printf("@Risc.00[%x,%x]\n",bi,bo);
/*
 * Step 1: write magic header, flags & block size
 */
//    xwrite(fo, magic, sizeof(magic));
    //xwrite32(fo,0);           /* no checksum */
    //xputc(fo,METHOD);         /* compression method */
    //xputc(fo,LEVEL);          /* compression level */
//    xwrite32(fo, block_size);

/*
 * Step 2: allocate compression buffers and work-memory
 */
    #if defined (__COMPRESS_BUFFER__)
      #if defined (__OPENRTOS__) || (WIN32)
    in  = (ucl_bytep)malloc(block_size);
    out = (ucl_bytep)malloc(block_size + (block_size / 8 + 256));
    #else
      //printf("@Risc.01[%x,%x,%x]\n",CPU_SRC_DATA_SIZE,block_size,block_size + (block_size / 8 + 256));
      in  = (ucl_bytep)malloc(block_size);
      out = (ucl_bytep)malloc(block_size + (block_size / 8 + 256));
      //in  = (ucl_bytep)MEM_Allocate(block_size, MEM_USER_DECOMPRESS);
      //out = (ucl_bytep)MEM_Allocate(block_size + (block_size / 8 + 256), MEM_USER_DECOMPRESS);
      #endif
    #else
    in  = (ucl_bytep)SYS_Malloc(block_size);
    out = (ucl_bytep)SYS_Malloc(block_size + (block_size / 8 + 256));
    #endif

    if (in == NULL || out == NULL) 
	{
        printf("Out of memory!!\n");
        //exit(-1);
    }

/*
 * Step 3: process blocks
 */
    //printf("@Risc.02\n");
    for (;;) 
	{
        /* read block */
        #if defined (__COMPRESS_BUFFER__)
        if( (CPU_SRC_DATA_SIZE-total_in)<block_size )
        {
        	in_len = (CPU_SRC_DATA_SIZE-total_in);
        }
        else
        {
        	in_len = block_size;
        }
        memcpy(in, &bi[total_in], in_len);
        total_in += in_len;
        #else
        in_len = xread(fi, in, block_size);
        #endif

        if (in_len <= 0)
            break;

        /* compress block */
        out_len = 0;
        //printf("@Risc.03[%x,%x,%x,%x,%x]\n",in, in_len, out, &out_len, &bi[total_in]);
        r = ucl_nrv2e_99_compress(in, in_len, out, &out_len, 0, LEVEL, NULL, NULL);
        //printf("@Risc.03.1[%x,%x,%x,%x,%x]\n",in, in_len, out, &out_len, &bi[total_in]);

        if (r == UCL_E_OUT_OF_MEMORY) 
		{
            printf("Out of memory in compress.[%x,%x,%x,%x,%x]\n",in,in_len,out,&out_len,out_len);
            exit(-1);
        }

        if (r != UCL_E_OK || out_len > in_len + (in_len / 8 + 256)) 
		{
            /* this should NEVER happen */
            printf("Abnormal error!!\n");
            exit(-1);
        }

        /* write uncompressed block size */
        #if defined (__COMPRESS_BUFFER__)
        //printf("@Risc.031[%x,%x]\n",&bo[CurrWtIdx],in_len);
        xwrite32buff(&bo[CurrWtIdx], in_len);	CurrWtIdx+=4;
        #else
        xwrite32(fo, in_len);
        #endif

        if (out_len < in_len) 
		{
            /* write compressed block */
            #if defined (__COMPRESS_BUFFER__)
            //printf("@Risc.032[%x,%x]\n",&bo[CurrWtIdx],out_len);
            xwrite32buff(&bo[CurrWtIdx], out_len);	CurrWtIdx+=4;
            //printf("@Risc.033[%x,%x]\n",&bo[CurrWtIdx],out_len);
            xwritebuff(&bo[CurrWtIdx], out, out_len);CurrWtIdx+=out_len;
            #else
            xwrite32(fo, out_len);
            xwrite(fo, out, out_len);
            #endif
        } 
		else 
		{
            /* not compressible - write uncompressed block */
            #if defined (__COMPRESS_BUFFER__)
            //printf("@Risc.034[%x,%x]\n",&bo[CurrWtIdx],out_len);
            xwrite32buff(&bo[CurrWtIdx], in_len);	CurrWtIdx+=4;
            //printf("@Risc.035[%x,%x]\n",&bo[CurrWtIdx],out_len);
            xwritebuff(&bo[CurrWtIdx], in, in_len);	CurrWtIdx+=in_len;
            #else
            xwrite32(fo, in_len);
            xwrite(fo, in, in_len);
            #endif
        }
    }
    //printf("@Risc.04\n");

    /* write EOF marker */
    #if defined (__COMPRESS_BUFFER__)
    xwrite32buff(&bo[CurrWtIdx], 0);
    #else
    xwrite32(fo, 0);
    #endif

    //printf("@Risc.05\n");

    {
        unsigned char b[4] = {0,0,0,0};
        #if defined (__COMPRESS_BUFFER__)
        //printf("@Risc.06\n");
        xwritebuff(&bo[CurrWtIdx], b, CurrWtIdx&0x3);
        #else
        xwrite(fo, b, ftell(fo)&0x3);
        #endif
    }

    //printf("@Risc.07\n");
    #if defined (__OPENRTOS__) || (WIN32)
    free(in);
    free(out);
    #else
    free(in);
    free(out);
    //MEM_Release(in);
    //MEM_Release(out);
    #endif
    //printf("@Risc.08\n");

    //printf("ROM compress ratio %6.2f%%\n", total_out * 100.0 / total_in);
    return CurrWtIdx;
}

void CPU_Decompress(MMP_UINT8 argc, char* argv[])
{
    int i = 0;
    int j = 0;
    unsigned int in_len = 0xd3;
    unsigned int out_len; 
    FILE *fp_in;
	FILE *fp_out;
	FILE *information;

    fp_in = fopen(argv[3], "rb");
	fp_out = fopen("D:\\result.bin", "wb"); 
    information = fopen("D:\\information.txt","wb");
    fprintf(information,"ilen	olen	mlen	m_off	  last_m_off 	bb\n");

	if(!fp_in || !fp_out || !information)
	{
		printf("open file error,fp_in=%x,fp_out=%x,information=%x\n", fp_in, fp_out, information);
	}

	//read all out.bin to decompress_pt
	out_len = 0;
	while(1)
	{
		in_len = (ucl_uint) fread(&decompress_pt[out_len], sizeof(char), 1024, fp_in);
		out_len += in_len;
		if(in_len<1024)
		{
			//printf("Total file size=%d(%x)!!\n",out_len, out_len);
			break;
		}
	}
	//printf("2.Total file size=%d(%x)!!\n",out_len,out_len);
	//while(1);

    for (;;)
    {    
        int      r = 0;
//        unsigned int out_len = 0x17e; 

        out_len = ((decompress_pt[i+3]) | (decompress_pt[i+2]<<8) | (decompress_pt[i+1]<<16) | (decompress_pt[i]<<24));
        i=i+4;
        //printf("i =%d\n",i);
        in_len = ((decompress_pt[i+3]) | (decompress_pt[i+2]<<8) | (decompress_pt[i+1]<<16) | (decompress_pt[i]<<24));
        i=i+4;
            //printf("in_len = %x ,out_len = %x \n",in_len,out_len);
        /* exit if last block (EOF marker) */
        if (out_len == 0)
            break;
        if (in_len < out_len)
        {
            //printf("decompress_pt[%d] = %x ,in_len = %x ,result[%d] = %x ,out_len = %x \n",i,decompress_pt[i],in_len,j,result[i],out_len);
            r = decompress(&decompress_pt[i],in_len,&result[j],&out_len,information);
            fwrite(&result[j],sizeof(char),out_len,fp_out);
            i += r;
            j += out_len;
        }
        else 
        {
            fwrite(&decompress_pt[i],sizeof(char),in_len,fp_out);
            i += in_len;
            j += in_len;
        }
    }
	if(!fp_in || !fp_out || !information)
	{
		printf("open file error,fp_in=%x,fp_out=%x,information=%x\n", fp_in, fp_out, information);
	}

	fclose(fp_in);
    fclose(fp_out);
    fclose(information);      
}

int decompress( const unsigned char* src, unsigned int  src_len,
                unsigned char* dst, unsigned int* dst_len,FILE* information)
{
    unsigned int bb = 0;
    unsigned int ilen = 0, olen = 0, last_m_off = 1;

#if defined(SAFE)
    const unsigned int oend = *dst_len;
#endif

    for (;;) {
    unsigned int m_off = 0, m_len = 0;
    fprintf(information,"step0 \n");
         while (GETBIT(bb,src)) {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(olen >= oend, UCL_E_OUTPUT_OVERRUN);
            dst[olen++] = GETBYTE(src);
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x  src[ilen-1]=%x src[ilen-2]=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1],src[ilen-1],src[ilen-2]);
        }
        m_off = 1;
    fprintf(information,"step1 \n");
        for (;;) {
            m_off = m_off*2 + GETBIT(bb,src);
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(m_off > 0xfffffful + 3, UCL_E_LOOKBEHIND_OVERRUN);
            if (GETBIT(bb,src)) {
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);
                break;
            }
            m_off = (m_off-1)*2 + GETBIT(bb,src);
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);
        }
    fprintf(information,"step2 \n");
        if (m_off == 2) {
    fprintf(information,"step2-1 \n");
            m_off = last_m_off;
            m_len = GETBIT(bb,src);
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);
        } else {
    fprintf(information,"step2-2 \n");
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            m_off = (m_off-3)*256 + GETBYTE(src);
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);	
            if (m_off == 0xfffffffful) {
    fprintf(information,"dec time add\n");
                break;
            }
            m_len = (m_off ^ 0xfffffffful) & 1;
            m_off >>= 1;
            last_m_off = ++m_off;
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);	
        }
    fprintf(information,"step3 \n");
        if (m_len) {
            m_len = 1 + GETBIT(bb,src);
    fprintf(information,"step3-1 \n");
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);
        } else if (GETBIT(bb,src)) {
            m_len = 3 + GETBIT(bb,src);
    fprintf(information,"step3-2 \n");
        } else {
            m_len++;
            do {
    fprintf(information,"step3-3-1 \n");
                m_len = m_len*2 + GETBIT(bb,src);
                fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
                fail(m_len >= oend, UCL_E_OUTPUT_OVERRUN);
    fprintf(information,"step3-3-2 \n");
            } while (!GETBIT(bb,src));
            m_len += 3;
        }

    fprintf(information,"step4 \n");
        m_len += (m_off > 0x500);
        fail(olen + m_len > oend, UCL_E_OUTPUT_OVERRUN);
        fail(m_off > olen, UCL_E_LOOKBEHIND_OVERRUN);
        {
            const unsigned char* m_pos;
            m_pos = dst + olen - m_off;
            dst[olen++] = *m_pos++;
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);
            do {	
                dst[olen++] = *m_pos++; 
    fprintf(information,"%x	%x	%x	%x		%x 	%x	dst=%x\n",ilen,olen,m_len,m_off,last_m_off,bb,dst[olen-1]);
            } while (--m_len > 0);
        }
    }

    *dst_len = olen;

//    return (ilen == src_len) ? UCL_E_OK 
//                             : (ilen < src_len ? UCL_E_INPUT_NOT_CONSUMED 
//                                               : UCL_E_INPUT_OVERRUN);
    return ilen;
}
#if defined(__COMPRESS_BUFFER__)
void AutoGenPatternBuff(MMP_UINT8 *srcbuff, MMP_UINT32 size)
{
	MMP_UINT8  *PatternData;
	MMP_UINT32	PkgFileSize=0;
	MMP_UINT32	idx=0,DataSize=size;
	MMP_UINT32  lastSec1,lastSec2,LoopCounter,TempRdSz=512*128;
	MMP_UINT8	GenResult = MMP_TRUE;
	MMP_UINT8 	RndMask=0;

	//set_rand();

	//pick 4 data block randomly PalRand()
	//printf("gen random value by PalGetClock()!! \n");
	lastSec2 = PalGetClock();
	//lastSec2 = 0x5e2;
	//printf("got!! lastSec2=%x \n",lastSec2);

	PalSrand(lastSec2);

	//MMP_Sleep(7);

	LoopCounter = PalRand();
	//LoopCounter = 0x2DD8748E;
	//printf("LC=%x\n",LoopCounter);

	//RndMask = 0x03 | ((LoopCounter>>2)&0x7C);
	RndMask = (0x01<<(((LoopCounter>>3)&0x07))-1)*2-1;	//0x01 0x03 0x07 0x0F 0x1F 0x3F 0x7F 0xFF
	//RndMask = ((LoopCounter>>3)&0xFF);
	if(RndMask==0x01)	RndMask=0x03;    //don't use RndMask=0x01 to cause the 100% compress rate.
	printf("RndMask=%02x,%x\n",RndMask,LoopCounter);

	while(1)
	{
		MMP_UINT8 RndCnt=0;
		MMP_UINT8 loop=0;

		LoopCounter = PalRand();
		//RndCnt = (MMP_UINT8)(LoopCounter>>8)&RndMask;
		RndCnt = (MMP_UINT8)(LoopCounter>>8)%RndMask;   //this code will make more random pattern
		//printf("got!! lc=%04x,RndCnt=%02x,size=[%x]\n",LoopCounter,RndCnt,RndMask,DataSize);

		for(loop=0;loop<RndCnt;loop++)
		{
			srcbuff[idx] = (MMP_UINT8)(LoopCounter & 0xFF);
			if( idx>=(DataSize) )	break;
			idx++;
		}

		if(idx>=DataSize)
		{
			//printf("STOP of whiel(1),[%x]!!\n",idx);
			//while(1);
			break;
		}
	}

	/*  printf input pattern for compressing
	if(idx)
	{
		MMP_UINT32 k=0;

		while(1)
		{
			if( (k&0x3FF)==0x0000 )	printf("[%04X]\n",k);
			printf("%02X ", PatternData[k]);
			if( (k&0x0F)==0x0F )	printf("\n");
			if( (k&0xFF)==0xFF )	printf("\n");
			if( k >= (DataSize-1) )	break;
			k++;
		}
	}
	*/
}
#else
void AutoGenPatternFile(MMP_UINT8 argc, char* argv[])
{
	FILE *finbin = NULL;
	FILE *fpkg   = NULL;
	char* dpu_pkg[] = {"D:\\Smedia_dpf.pkg"};
	MMP_UINT8  *PatternData;
	MMP_UINT32	PkgFileSize=0;
	MMP_UINT32	idx=0,DataSize=1024*70;//1024*16;//16384*32;//1024*1024;	
	MMP_UINT32 lastSec1,lastSec2,LoopCounter,TempRdSz=512*128;
	MMP_UINT8	GenResult = MMP_TRUE;

	printf("AutoGenPatternFile: argv[0]~[3]=[%s,%s,%s,%s]\n",argv[0],argv[1],argv[2],argv[3]);
	//while(1);

	//open the in.bin

	//open a 1MB buffer for the file "in.bin"
	PatternData = (MMP_UINT8 *)SYS_Malloc(DataSize);
	if(!PatternData)
	{
		printf("[ERROR], sys_alloc() of PatternData error!!\n");
	}
	//set_rand();

	//pick 4 data block randomly PalRand()
	lastSec2 = PalGetClock();
	PalSrand(lastSec2);

#ifndef _AUTO_GEN_FILE_
	//OPEN pkg file
	if ((fpkg = fopen(dpu_pkg[0], "rb")) == NULL) 
	{
		printf("Open %s fails\n", dpu_pkg[0]);
		GenResult = MMP_FALSE;
	}
	else
	{
		//random seeking address
		fseek(fpkg, 0, SEEK_END);
		PkgFileSize = (unsigned long)ftell(fpkg);
		while(1)
		{
			MMP_UINT32 Rand_L=0;
			MMP_UINT32 Rand_H=0;

			Rand_L = PalRand();
			Rand_H = PalRand();
			LoopCounter = (Rand_H<<16) + Rand_L;
			LoopCounter = LoopCounter % (PkgFileSize-DataSize);
			//LoopCounter = 0x1f2cfb1;
			//LoopCounter = 0x45600be;
			printf("FileSize=%x, FileSeek=%x\n", PkgFileSize, LoopCounter);
			if( (LoopCounter>0) && (LoopCounter < (PkgFileSize-DataSize)) )
			{
				printf("Get a seek address!!\n");
				fseek(fpkg, LoopCounter, SEEK_SET);
				break;
			}
		}

		//get data from pkg file
		LoopCounter = (MMP_UINT32)fread(SrcCompressData, sizeof(char), DataSize , fpkg);
	}
	//close pkg file
	fclose(fpkg);
#else
	while(1)
	{
		LoopCounter = PalRand();
		SrcCompressData[idx] = (MMP_UINT8)(LoopCounter & 0xFF);
		if( idx>=(DataSize) )	break;
		idx++;
	}
#endif

	//save file
	if ((finbin = fopen(argv[2], "wb")) == NULL) 
	{
		printf("Open %s fails\n", argv[3]);
		//exit(-1);
		GenResult = MMP_FALSE;
	}
	else
	{
		LoopCounter = (MMP_UINT32)fwrite(SrcCompressData, sizeof(char), DataSize , finbin);
	}
	fclose(finbin);

	/*  printf input pattern for compressing
	if(idx)
	{
		MMP_UINT32 k=0;

		while(1)
		{
			if( (k&0x3FF)==0x0000 )	printf("[%04X]\n",k);
			printf("%02X ", PatternData[k]);
			if( (k&0x0F)==0x0F )	printf("\n");
			if( (k&0xFF)==0xFF )	printf("\n");
			if( k >= (DataSize-1) )	break;
			k++;
		}
	}
	*/

	SYS_Free(PatternData);

	//return GenResult;
}
#endif

MMP_UINT8 compare2buffer(MMP_UINT8 * SrcBuffer, MMP_UINT8 * DstBuffer,MMP_UINT32 count )
{
	MMP_UINT8	CprResult=MMP_TRUE;
	MMP_UINT32	i=0;

	for(i=0;i<count;i++)
	{
		if(SrcBuffer[i]!=DstBuffer[i])
		{
			printf("Comapre Error,i=%x,Src,Dst=[%x,%x]\n",i,SrcBuffer[i],DstBuffer[i]);
			ErrorTable[ErrCnt].address = i;
			ErrorTable[ErrCnt].SrcByte = SrcBuffer[i];
			ErrorTable[ErrCnt].DstByte = DstBuffer[i];
			printf("Error table,ErrCnt=%d,i=%x,Src,Dst=[%x,%x]\n", ErrCnt, ErrorTable[ErrCnt].address, ErrorTable[ErrCnt].SrcByte, ErrorTable[ErrCnt].DstByte);

			ErrCnt++;
			if(ErrCnt>=1024)
			{
				printf("End of error table recode\n");
			}
			CprResult = MMP_FALSE;
			//i=0xF4000;
			break;
		}
	}

	return CprResult;
}
