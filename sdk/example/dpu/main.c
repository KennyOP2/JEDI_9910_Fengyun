/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

/****** windows file system ******/
//#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
//#include <io.h>
/****** windows file system ******/

#include "../include/mmp_dpu.h"
#include "pal/pal.h"


#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "../../include/host/ahb.h"
#include "../../include/mmp_types.h"
#include "../../include/host/host.h"
#include "polarssl/aes.h"
#include "polarssl/des.h"

#define ENABLE_SHA2

#ifdef ENABLE_SHA2
#include "polarssl/test_suite_shax.h"
#endif

#if !defined(MM9910)
#include "../../include/cmq/cmd_queue.h"
#endif

#define		CALCULATE_CRC_SW
#define		ENABLE_CSA_SW_DECODE

#if defined(__FREERTOS__)
  //#define		ADDRESS_OFFSET
#endif

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

#define		MAX_TEST_DATA_SIZE  (512)

#define		AES_MODE_DATA_SIZE  (MAX_TEST_DATA_SIZE)
#define		DES_MODE_DATA_SIZE  (MAX_TEST_DATA_SIZE)
#define		CSA_MODE_DATA_SIZE  (184)
#define		CRC_MODE_DATA_SIZE  (MAX_TEST_DATA_SIZE)

#if defined(MM9910)
#define MAX_SPRS_PER_GRP_BITS (11)
#define SPRGROUP_MAC    (5<< MAX_SPRS_PER_GRP_BITS)
#define SPR_SECURITY    (SPRGROUP_MAC + 0)
#endif
/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#  include "crc_pattern.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__OR32__)
#  include "../../freertos/include/or32/mmio.h"
#else

#endif


/////////////////////////////////////////////////////////////////
//                      Constant Decleration
/////////////////////////////////////////////////////////////////
MMP_UINT32 i=0;
MMP_UINT32 err=0;

static unsigned int total_in = 0;
static unsigned int total_out = 0;
static int block_size = 1024;
static unsigned char decompress_pt[100000];
static unsigned char result[2000000];

static MMP_UINT8	*SrcData=MMP_NULL;
static MMP_UINT8	*DstData=MMP_NULL;
static MMP_UINT8	*NewSrcData=MMP_NULL;
static MMP_UINT8	*NewDstData=MMP_NULL;
static MMP_UINT8	*gSwResult=MMP_NULL;
static MMP_UINT8	*gSysBuf=MMP_NULL;
static MMP_UINT8	*sysbuf=MMP_NULL;
static MMP_UINT8	*SwResult=MMP_NULL;

static MMP_UINT8	SrcOffSet=0;
static MMP_UINT32	gMaxDataSize=MAX_TEST_DATA_SIZE;

static DPU_CONTEXT  *DpuContext;

static MMP_UINT8	Csa_Src_Data[184] =
{
0x6c,0xbf,0xa0,0xe5,0xa9,0x6d,0xdc,0x60,0x3a,0xdc,0xad,0x22,0xa7,0x95,0x24,0xfc,
0x4e,0xb5,0xf9,0x15,0xca,0xa1,0x87,0x17,0xf2,0x27,0x13,0x97,0x1b,0x5b,0x6d,0x70,
0xff,0x3c,0xcd,0x75,0x63,0x85,0x60,0xa8,0xb5,0x4e,0x0f,0x40,0x42,0x8f,0xef,0x00,
0xe5,0xd5,0x76,0x47,0xf0,0x2a,0x64,0x8e,0x24,0x55,0xbf,0x09,0x29,0x45,0x95,0xd2,
0x78,0xbe,0x82,0xfd,0x07,0xb6,0x03,0xf4,0x30,0xab,0x42,0x8d,0x2b,0xb9,0x70,0x66,
0x60,0x81,0x86,0x15,0x93,0xbc,0xd3,0xf9,0xdf,0xb7,0xb2,0x97,0x71,0x0a,0x36,0xa1,
0xda,0x7a,0x5e,0x0e,0x4b,0x4c,0xf9,0xeb,0xa8,0xcc,0xfe,0xe1,0x69,0x11,0xba,0x34,
0xe2,0x8a,0xc1,0x3a,0x3d,0x5c,0x50,0xbb,0xb7,0x57,0x08,0x5a,0x89,0x91,0x8c,0xd7,
0x30,0xdd,0x1a,0xb9,0x89,0xa7,0xbf,0x55,0x98,0x61,0x2b,0xed,0x73,0x2e,0x62,0xb6,
0x0b,0x7e,0x30,0x88,0x0c,0x6e,0xcb,0x9b,0x72,0x99,0xb2,0x7c,0xc9,0x58,0x7d,0xed,
0xc0,0x3d,0x12,0xaa,0x3c,0xf3,0x5e,0x1e,0x27,0xb2,0x15,0x79,0xda,0xb3,0x25,0xd6,
0xf6,0x7f,0x81,0xee,0x18,0xa1,0x05,0x38};

static MMP_UINT8	Sha2_Random_Data[512];
static MMP_UINT8	Scramble_Random_Data[512];

static MMP_UINT8	Aes_Random_Data[AES_MODE_DATA_SIZE+16];
static MMP_UINT8	Des_Random_Data[DES_MODE_DATA_SIZE+16];
static MMP_UINT8	Csa_Random_Data[CSA_MODE_DATA_SIZE+16];
static MMP_UINT8	CsaDecodeData[CSA_MODE_DATA_SIZE+16];
static MMP_UINT8	Csa_Result_Data[CSA_MODE_DATA_SIZE+16];

#ifdef	ENABLE_CSA_SW_DECODE

#define NUM_OF_SBOXES       7
#define SBOX_INPUT_SIZE     5
#define SBOX_OUTPUT_SIZE    2

//typedef unsigned char byte_t;
// stream cipher
typedef struct shiftreg
{
    unsigned int    len;
    unsigned int    cell[10];
} SHIFTREG;

// Declare and initiate tables specifying various bit mappings:
// -from A to sboxes, -from sboxes to C, -from B to D (via T3)
static unsigned int bit_from_a[7][5] =
{ {16,  2, 23, 25, 36},
  { 7, 10, 21, 28, 35},
  { 1,  8, 19, 17, 22},
  { 9,  3,  5, 14, 32},
  {18, 13, 24, 31, 34},
  {11, 15, 20, 26, 33},
  { 6, 12, 27, 30, 29}
};

static unsigned int bit_to_c [7][2] =
{ {4,10}, {3, 9}, {8, 2}, {7, 1}, {12, 6}, {11, 5}, {13, 14} };

static unsigned int bit_from_b [4][4] =
{ {12, 23, 26, 33}, {9, 14, 24, 31}, {16, 19, 17, 30}, {11, 21, 32, 34} };

// Declare and initiate the sboxes
static unsigned int s_Sbox[7][32] =
{ {2,0,1,1,2,3,3,0,3,2,2,0,1,1,0,3,0,3,3,0,2,2,1,1,2,2,0,3,1,1,3,0},
  {3,1,0,2,2,3,3,0,1,3,2,1,0,0,1,2,3,1,0,3,3,2,0,2,0,0,1,2,2,1,3,1},
  {2,0,1,2,2,3,3,1,1,1,0,3,3,0,2,0,1,3,0,1,3,0,2,2,2,0,1,2,0,3,3,1},
  {3,1,2,3,0,2,1,2,1,2,0,1,3,0,0,3,1,0,3,1,2,3,0,3,0,3,2,0,1,2,2,1},
  {2,0,0,1,3,2,3,2,0,1,3,3,1,0,2,1,2,3,2,0,0,3,1,1,1,0,3,2,3,1,0,2},
  {0,1,2,3,1,2,2,0,0,1,3,0,2,3,1,3,2,3,0,2,3,0,1,1,2,1,1,2,0,3,3,0},
  {0,3,2,2,3,0,0,1,3,0,1,3,1,2,2,1,1,0,3,3,0,1,1,2,2,3,1,0,2,3,0,2}
};

// block cipher
static MMP_UINT8 b_Sbox[256] =
{
     58, 234, 104, 254,  51, 233, 136,  26, 131, 207, 225, 127, 186, 226,  56,  18,
    232,  39,  97, 149,  12,  54, 229, 112, 162,   6, 130, 124,  23, 163,  38,  73,
    190, 122, 109,  71, 193,  81, 143, 243, 204,  91, 103, 189, 205,  24,   8, 201,
    255, 105, 239,   3,  78,  72,  74, 132,  63, 180,  16,   4, 220, 245,  92, 198,
     22, 171, 172,  76, 241, 106,  47,  60,  59, 212, 213, 148, 208, 196,  99,  98,
    113, 161, 249,  79,  46, 170, 197,  86, 227,  57, 147, 206, 101, 100, 228,  88,
    108,  25,  66, 121, 221, 238, 150, 246, 138, 236,  30, 133,  83,  69, 222, 187,
    126,  10, 154,  19,  42, 157, 194,  94,  90,  31,  50,  53, 156, 168, 115,  48,
     41,  61, 231, 146, 135,  27,  43,  75, 165,  87, 151,  64,  21, 230, 188,  14,
    235, 195,  52,  45, 184,  68,  37, 164,  28, 199,  35, 237, 144, 110,  80,   0,
    153, 158,  77, 217, 218, 141, 111,  95,  62, 215,  33, 116, 134, 223, 107,   5,
    142,  93,  55,  17, 210,  40, 117, 214, 167, 119,  36, 191, 240, 176,   2, 183,
    248, 252, 129,   9, 177,   1, 118, 145, 125,  15, 200, 160, 242, 203, 120,  96,
    209, 247, 224, 181, 152,  34, 179,  32,  29, 166, 219, 123,  89, 159, 174,  49,
    251, 211, 182, 202,  67, 114,   7, 244, 216,  65,  20,  85,  13,  84, 139, 185,
    173,  70,  11, 175, 128,  82,  44, 250, 140, 137, 102, 253, 178, 169, 155, 192
};

// Key permutation for key schedule
static MMP_UINT8 kd_perm[64] =
{   18, 36,  9,  7, 42, 49, 29, 21, 28, 54, 62, 50, 19, 33, 59, 64,
    24, 20, 37, 39,  2, 53, 27,  1, 34,  4, 13, 14, 57, 40, 26, 41,
    51, 35, 52, 12, 22, 48, 30, 58, 45, 31,  8, 25, 23, 47, 61, 17,
    60,  5, 56, 43, 11,  6, 10, 44, 32, 63, 46, 15,  3, 38, 16, 55
};

static MMP_UINT8 b_mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};

SHIFTREG    sr_a, sr_b;
unsigned int s_c, s_d, s_e, s_f, s_r, s_bib;
MMP_UINT8 b_kb[7][8];
#if defined(WIN32)
FILE *ex, exb, exs;
#endif
#endif	//ENABLE_CSA_SW_DECODE

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

/////////////////////////////////////////////////////////////////
//                      function Decleration
/////////////////////////////////////////////////////////////////
static void SW_CRC_CALCULATE(MMP_UINT32* crc_src_pt,MMP_UINT32* result_pt, MMP_UINT32 CrcSize);
static MMP_UINT8 CompressionTest(void);
static void Do_Test(void);

static MMP_UINT8 EncodeTest(MMP_UINT8 *InPtnPtr, MMP_UINT8 *key, MMP_UINT8 *iv, DPU_MODE DpuMode);
static MMP_UINT8 DecodeTest(MMP_UINT8 *InPtnPtr, MMP_UINT8 *key, MMP_UINT8 *iv, DPU_MODE DpuMode);

static MMP_UINT8 AES_Test(void);
static MMP_UINT8 CRC_Test(void);
static MMP_UINT8 CSA_Test(void);
static MMP_UINT8 DES_Test(void);
static MMP_UINT8 DES3_Test(void);

static MMP_UINT8 SHA2_Test(void);
static MMP_UINT8 SCRAMBLE_Test(void);

static MMP_UINT8 CsaDecodeTest(MMP_UINT8 *Src, MMP_UINT8 *Dst, MMP_UINT32 CsaSize);

static MMP_UINT8 CrcMasterModeTest(MMP_UINT8 *Src, MMP_UINT32 DataSize);
static MMP_UINT8 CrcSlaveModeTest(MMP_UINT32 *Src, MMP_UINT32 DataSize);

static void CsaDecodeBySoftware(MMP_UINT8 *scrambled_field, MMP_UINT8 *Csa_Result, MMP_UINT8 *Csa_Key, MMP_UINT32 CsaSize);

static void ShowBufferData(MMP_UINT8 *DataBuffer, MMP_UINT32 Datalen);
static MMP_UINT8 compare2buffer(MMP_UINT8 * SrcBuffer, MMP_UINT8 * DstBuffer,MMP_UINT32 count );
static void AutoGenPatternInBuffer(MMP_UINT8 *buffer1, MMP_UINT32 totalBytes);
static MMP_UINT8 GetParameter(MMP_UINT32 *size, MMP_UINT8 *keyLength, MMP_UINT8 *VtLength, DPU_MODE DpuMode);
static void ExOrOneByte(unsigned char *Buf1, unsigned char *Buf2, unsigned char *OutBuf);
static void ExOr(unsigned char *Buf1, unsigned char *Buf2, unsigned char *OutBuf, unsigned int DataLength);

static void SwAesEcbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *AesKey);
static void SwAesEcbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *AesKey);
static void SwAesCbcEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector);
static void SwAesCbcDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector);
static void SwAesCfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector);
static void SwAesOfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector);
static void SwAesCtrEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector);

static void SwDesEcbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *DesKey);
static void SwDesEcbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *DesKey);
static void SwDesCbcEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv);
static void SwDesCbcDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv);
static void SwDesCfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv);
static void SwDesOfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv);
static void SwDesCtrEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv);

static void SwDes3EcbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key);
static void SwDes3EcbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key);
static void SwDes3CbcEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);
static void SwDes3CbcDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);
static void SwDes3CfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);
static void SwDes3OfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);
static void SwDes3CtrEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);
static void SwDes3CfbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);
static void SwDes3OfbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv);

static void SwEncryption(SW_ENC_INFO *SwEncInfo);
static void SwDecryption(SW_ENC_INFO *SwEncInfo);

#if defined(MM9910) && defined(WIN32)
static void LoadDpuDrvToMem(void);
#endif

#if defined(MM9910)
static MMP_RESULT JediAhbModeTest(void);
static void FireRisc3AndSwitch2WbMode(void);
static void Wait_Risc1_Ready(void);
static void Wait_Risc3_Ready(void);
#endif
/////////////////////////////////////////////////////////////////
//                      function implement
/////////////////////////////////////////////////////////////////
/* For writing into SPR. */
#if defined(MM9910) && defined(__FREERTOS__)
__attribute__((no_instrument_function))
static __inline void mtspr_sec(unsigned long spr, unsigned long value) {
    asm volatile("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
}
#endif

int main(int argc, char** argv)
{
	MMP_UINT32	TestCount=1;
	// ********* read ID command *******

#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    ret = xTaskCreate(main_task_func, "dputest_main",
        configMINIMAL_STACK_SIZE * 13,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret)
    {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#else
    Do_Test();
    //Do_Test2();
#endif

	printf("DPU main() end !!\n");
    while(1);
}

#if defined(MM9910) && defined(__FREERTOS__)
int security(void) 
{
    signed portBASE_TYPE ret = pdFAIL;
    MMP_UINT16  Reg16;
    
    //PalEnableUartPrint(MMP_TRUE,0,7,115200);
    //printf("DPU3 main() start !!\n"); 
    
    //mtspr(SPR_SECURITY, 0x00000000); // security mode
	#if defined(MM9910) && defined(__FREERTOS__)
    mtspr_sec(SPR_SECURITY, 0x00000001); // AHB mode
	#endif

    //MIO_Write(0x1698, 0x5A5A);
    HOST_WriteRegister(0x1698, 0x5A5A);
    
    //printf("DPU3 main() end !!\n");
    //while(1);
    //Wait_Risc1_Ready();
    
	//wait for 0x1698 ==0x5A5A
	
	HOST_ReadRegister(0x1698, &Reg16);	
	while(Reg16!=0x01A0)
	{	    
	    HOST_ReadRegister(0x1698, &Reg16);
	    //printf("GOT the [1698]=[%x]!!\n",Reg16);
	}	
	
	//printf("GOT the 1698=0x1A0!!\n");
	HOST_WriteRegister(0x1698, 0x1234); 
	while(1);

#if defined(__FREERTOS__)
    ret = xTaskCreate(main_task_func, "dputest_main",
        configMINIMAL_STACK_SIZE * 8,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret)
    {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#else
    Do_Test();
#endif

    return 1;
}
#endif

unsigned char checkIfUsed( unsigned char *ptr, unsigned char num)
{
	for(i=0; i<num; i++)
	{
		if(ptr[num]==ptr[i])
		{
			return 1;
		}
	}
	
	return 0;
}

void Do_Test2()
{
	unsigned char N[10];
	unsigned int TotalNum;
	
	for(N[0]=1; N[0]<10; N[0]++)
	{
		for(N[1]=0; N[1]<10; N[1]++)
		{
			if(checkIfUsed( (unsigned char*)&N, 1)==1)
			{
				continue;
			}
			for(N[2]=0; N[2]<10; N[2]++)
			{
				if(checkIfUsed( (unsigned char*)&N, 2)==1)
				{
					continue;
				}
				for(N[3]=0; N[3]<10; N[3]++)
				{
					if(checkIfUsed( (unsigned char*)&N, 3)==1)
					{
						continue;
					}
					for(N[4]=0; N[4]<10; N[4]++)
					{
						if(checkIfUsed( (unsigned char*)&N, 4)==1)
						{
							continue;
						}
							
						for(N[5]=0; N[5]<10; N[5]++)
						{
							if(checkIfUsed( (unsigned char*)&N, 5)==1)
							{
								continue;
							}
							for(N[6]=0; N[6]<10; N[6]++)
							{
								if(checkIfUsed( (unsigned char*)&N, 6)==1)
								{
									continue;
								}
								for(N[7]=0; N[7]<10; N[7]++)
								{
									if(checkIfUsed( (unsigned char*)&N, 7)==1)
									{
										continue;
									}
									for(N[8]=0; N[8]<10; N[8]++)
									{
										if(checkIfUsed( (unsigned char*)&N, 8)==1)
										{
											continue;
										}
										for(N[9]=0; N[9]<10; N[9]++)
										{
											if(checkIfUsed( (unsigned char*)&N, 9)==1)
											{
												continue;
											}
											TotalNum = N[0]*1000 + (N[1]+N[4])*100 + (N[2]+N[5]+N[7])*10 + (N[3]+N[6]+N[8]-N[9]);
											if(TotalNum==2012)
											{
												printf("%d%d%d%d + %d%d%d + %d%d - %d = %d\n",N[0],N[1],N[2],N[3],N[4],N[5],N[6],N[7],N[8],N[9],TotalNum);
												//printf("Got the Ans=%d\n",TotalNum);
												while(1);
											}
											/*
											else
											{
												printf("%d%d%d%d + %d%d%d + %d%d - %d = %d\n",N[0],N[1],N[2],N[3],N[4],N[5],N[6],N[7],N[8],N[9],TotalNum);
											}
											*/
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
	}
	printf("calcution is over\n");
	while(1);
	
}
/*
void Do_Test3()
{
	unsigned char N[10];
	unsigned int TotalNum;
	
	for(N[0]=1; N[0]<10; N[0]++)
	{
		for(N[1]=0; N[1]<10; N[1]++)
		{
			if(checkIfUsed( &N, 1)==1)
			{
				continue;
			}
			for(N[2]=0; N[2]<10; N[2]++)
			{
				if(checkIfUsed( &N, 2)==1)
				{
					continue;
				}
				for(N[3]=0; N[3]<10; N[3]++)
				{
					unsigned int tempSum;
					if(checkIfUsed( &N, 3)==1)
					{
						continue;
					}
					tempSum = N[0]+N[1]+N[2]-N[3];
					if(tempSum==12)
					{
						printf("Got d,g,i,j=[%d,%d,%d,%d]\n",N[0],N[1],N[2],N[3]);
						for(N[4]=1; N[4]<10; N[4]++)
						{
							if(checkIfUsed( &N, 4)==1)	continue;
							for(N[5]=1; N[5]<10; N[5]++)
							{
								if(checkIfUsed( &N, 5)==1)	continue;
								for(N[6]=1; N[6]<10; N[6]++)
								{
									if(checkIfUsed( &N, 6)==1)	continue;
									tempSum = N[4]+N[5]+N[6];
									if(tempSum==10)
									{
										for(N[7]=1; N[7]<10; N[7]++)
										{
											
										
								}
					}
					
}
*/
void Do_Test()
{
	MMP_UINT32	TestCount=1;
	MMP_RESULT  result;
	MMP_UINT32  Reg32;
	MMP_UINT32  DPU_WBB_ADDR = 0xE0000000;
	MMP_UINT32  DPU_AHB_ADDR = 0xD0900000;
	//MMP_UINT32  DPU_REG_SRC_ADDR = DPU_WBB_ADDR + 4;
	MMP_UINT32  DPU_REG_SRC_ADDR = DPU_AHB_ADDR + 4;
	// ********* read ID command *******

#if defined(__FREERTOS__)
    //RISC hw uart test
	//PalEnableUartPrint(MMP_TRUE,0,7,115200);
	
    //RISC printbuffer
    //PalEnablePrintBuffer(MMP_TRUE,4*1024); 
#endif

	#if defined(MM9910) && (WIN32)
	//if MM9910, then load DPU driver code to RISC3's MEM address
	//mmpLoadAhbCode();
	if(0)
	{
	    //printf("Error, DPU-[01]\n");
	    LoadDpuDrvToMem();
	    //SwitchDpu2AhbMode();
	    //switchDpu2WbbMode();
	    //TurnOnRisc3AndChkPc();
	    //SetDpuReg();	    
	}
	#endif

	// ********* initial *******
	result = mmpDpuInitialize();
	if(result)
	{
		printf("DPU initial fail!!\n");
		goto end;
	}

	//aes_self_test(1);
	//des_self_test(1);
	//sha2_self_test(1);
	//sha2_self_test2(1);
	//while(1);

	if(gSwResult==MMP_NULL)
	{
		#if defined(__FREERTOS__)
		gSwResult = (MMP_UINT8*)MEM_Allocate(gMaxDataSize);
		#else
		gSwResult = (MMP_UINT8*)SYS_Malloc(gMaxDataSize);
		#endif
		if(gSwResult==MMP_NULL)
		{
			//Error
			goto end;
		}
	}

	if(gSysBuf==MMP_NULL)
	{
		#if defined(__FREERTOS__)
		gSysBuf = (MMP_UINT8*)MEM_Allocate(gMaxDataSize);
		#else
		gSysBuf = (MMP_UINT8*)SYS_Malloc(gMaxDataSize);
		#endif

		if(gSysBuf==MMP_NULL)
		{
			//Error
			goto end;
		}
	}
	if(DpuContext==MMP_NULL)
	{
		#if defined(__FREERTOS__)
		DpuContext = (DPU_CONTEXT *)MEM_Allocate(sizeof(DPU_CONTEXT));
		#else
		DpuContext = (DPU_CONTEXT *)SYS_Malloc(sizeof(DPU_CONTEXT));
		#endif

		if(DpuContext==MMP_NULL)
		{
			//Error
			goto end;
		}
	}

	SrcData = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	DstData = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	NewSrcData = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	NewDstData = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);

	#if defined(__FREERTOS__)
	sysbuf  = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	SwResult = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	DpuContext = (DPU_CONTEXT *)MEM_Allocate(sizeof(DPU_CONTEXT));
	#else
	sysbuf  = (MMP_UINT8 *)SYS_Malloc(gMaxDataSize+16);
	SwResult = (MMP_UINT8 *)SYS_Malloc(gMaxDataSize+16);
	DpuContext = (DPU_CONTEXT *)SYS_Malloc(sizeof(DPU_CONTEXT));
	#endif

	while(1)
	{
		printf("\nTest loop = %d !!\n",TestCount++);

		if( AES_Test()!=MMP_TRUE )
		{
			printf("AES_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("AES_Test Success!!\n");
		}

		if( DES_Test()!=MMP_TRUE )
		{
			printf("DES_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("DES_Test Success!!\n");
		}

		if( DES3_Test()!=MMP_TRUE )
		{
			printf("DES3_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("DES3_Test Success!!\n");
		}

		if( CSA_Test()!=MMP_TRUE )
		{
			printf("CSA_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("CSA_Test Success!!\n");
		}

		if( CRC_Test()!=MMP_TRUE )
		{
			printf("CRC_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("CRC_Test Success!!\n");
		}

#if defined(MM9910)

		if( SCRAMBLE_Test()!=MMP_TRUE )
		{
			printf("SCRAMBLE_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("SCRAMBLE_Test Success!!\n");
		}

#ifdef ENABLE_SHA2

		if( SHA2_Test()!=MMP_TRUE )
		{
			printf("SHA2_Test fail!!\n");
			goto end;
		}
		else
		{
			printf("SHA2_Test Success!!\n");
		}

#endif

#endif
    }//while(1) loop
//--------------------------------------------------------------//
end:
	if(NewSrcData)
	{
		free(NewSrcData);
		NewSrcData=MMP_NULL;
	}
	if(NewDstData)
	{
		free(NewDstData);
		NewDstData=MMP_NULL;
	}
	#if defined(__FREERTOS__)
	if(gSwResult)
	{
		MEM_Release(gSwResult);
		gSwResult=MMP_NULL;
	}
	if(gSysBuf)
	{
		MEM_Release(gSysBuf);
		gSysBuf=MMP_NULL;
	}
	if(DpuContext)
	{
		MEM_Release(DpuContext);
		DpuContext=MMP_NULL;
	}
	#else
	if(gSwResult)
	{
		SYS_Free(gSwResult);
		gSwResult=MMP_NULL;
	}
	if(gSysBuf)
	{
		SYS_Free(gSysBuf);
		gSysBuf=MMP_NULL;
	}
	if(DpuContext)
	{
		SYS_Free(DpuContext);
		DpuContext=MMP_NULL;
	}
	#endif

	printf("End of DPU test!!\n");
	while(1);
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
	printf("DPU FreeRtos task TEST:\n");

    Do_Test();
    //Do_Test2();

	printf("The END of DPU FreeRtos task TEST:(while(1))\n");
	while(1);
}
#endif

void SW_CRC_CALCULATE(MMP_UINT32* crc_src_pt,MMP_UINT32* result_pt, MMP_UINT32 CrcSize)
{
	unsigned char *ptr_crc,i,j;
	MMP_UINT32	crc32val = 0xffffffff;
	ptr_crc = (unsigned char *) crc_src_pt;

	for (i = 0; i < (CrcSize/4); i++)
	{
		for (j = 0; j < 4; j++)
		{
			crc32val = crc32_tab[(crc32val ^ *ptr_crc) & 0xff] ^ (crc32val >> 8);
			ptr_crc++;
            if( (MMP_UINT32)ptr_crc > ((MMP_UINT32)crc_src_pt+0x200) )
            {
                ptr_crc--;
            }
		}
		//result_pt[i] = crc32val;  //get the result step by step
        result_pt[0] = crc32val;    //only get the final result
	}

	//return 0;
}

MMP_UINT8 GetParameter(MMP_UINT32 *size, MMP_UINT8 *keyLength, MMP_UINT8 *VtLength, DPU_MODE DpuMode)
{
	switch(DpuMode)
	{
	case AES_ECB_MODE:
		*size = AES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_128_BIT;
		*VtLength = VECTOR_LENTGH_NO_VECTOR;
		break;
	case AES_CBC_MODE:
		*size = AES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_128_BIT;
		*VtLength = VECTOR_LENTGH_4_VECTORS;
		break;
	case AES_OFB_MODE:
		*size = AES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_128_BIT;
		*VtLength = VECTOR_LENTGH_4_VECTORS;
		break;
	case AES_CFB_MODE:
		*size = AES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_128_BIT;
		*VtLength = VECTOR_LENTGH_4_VECTORS;
		break;
	case AES_CTR_MODE:
		*size = AES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_128_BIT;
		*VtLength = VECTOR_LENTGH_4_VECTORS;
		break;

	case DES_ECB_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_64_BIT;
		*VtLength = VECTOR_LENTGH_NO_VECTOR;
		break;
	case DES_CBC_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;
	case DES_OFB_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;
	case DES_CFB_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;
	case DES_CTR_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;

	case DES3_ECB_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_3X64_BIT;
		*VtLength = VECTOR_LENTGH_NO_VECTOR;
		break;

	case DES3_CBC_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_3X64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;
	case DES3_OFB_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_3X64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;
	case DES3_CFB_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_3X64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;
	case DES3_CTR_MODE:
		*size = DES_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_3X64_BIT;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
		break;

	case CSA_MODE:
		*size = CSA_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_64_BIT;
		*VtLength = VECTOR_LENTGH_NO_VECTOR;
        break;

	case CRC_MODE:
		*size = CRC_MODE_DATA_SIZE;
		*keyLength = KEY_LENTGH_NO_KEY;
		*VtLength = VECTOR_LENTGH_NO_VECTOR;
        break;

	case SHA2_MODE:
		*size = 256;
		*keyLength = KEY_LENTGH_NO_KEY;
		*VtLength = VECTOR_LENTGH_NO_VECTOR;
        break;

	case SCRAMBLE_MODE:
		*size = 512;
		*keyLength = KEY_LENTGH_NO_KEY;
		*VtLength = VECTOR_LENTGH_2_VECTORS;
        break;

	default:
		//error
		return MMP_FALSE;
		break;
	}
	return MMP_TRUE;
}

MMP_UINT8 EncodeTest(MMP_UINT8 *InPtnPtr, MMP_UINT8 *key, MMP_UINT8 *iv, DPU_MODE DpuMode)
{
	MMP_UINT32	DataSize;
	MMP_UINT32	Reg32;
	MMP_UINT8	KeyLen=0;
	MMP_UINT8	VectorLen=0;
	MMP_UINT8	TestResult=MMP_FALSE;
	MMP_UINT8	rst=MMP_FALSE;
	SW_ENC_INFO SwEncodeInfo;

	rst = GetParameter(&DataSize, &KeyLen, &VectorLen, DpuMode);

	SwEncodeInfo.DpuMode=DpuMode;
	SwEncodeInfo.InBuf=InPtnPtr;
	SwEncodeInfo.OutBuf=gSwResult;
	SwEncodeInfo.Key=key;
	SwEncodeInfo.SwEncSize=DataSize;
	SwEncodeInfo.iv=iv;
    memset( gSwResult, 0x00, DataSize);

	SwEncryption(&SwEncodeInfo);

	memset(DpuContext, 0x00, sizeof(DpuContext));

#ifdef ADDRESS_OFFSET
	memset( sysbuf, 0x5A, SrcOffSet);
	memcpy( sysbuf+SrcOffSet, InPtnPtr, DataSize);
	memset( sysbuf+SrcOffSet+DataSize, 0x5A, 4-SrcOffSet);
	#if defined(__FREERTOS__)
	memcpy( NewSrcData, sysbuf, DataSize+4);
	#else
	HOST_WriteBlockMemory((MMP_UINT32)NewSrcData, (MMP_UINT32)sysbuf, DataSize+4);
	#endif
#else
    #if defined(__FREERTOS__)
    memcpy( NewSrcData, InPtnPtr, DataSize);
    #else
	HOST_WriteBlockMemory((MMP_UINT32)NewSrcData, (MMP_UINT32)InPtnPtr, DataSize);
    #endif
#endif
	//ShowBufferData(InPtnPtr, 32);
	memset( gSysBuf, 0x5A, SrcOffSet);

	DpuContext->DpuMode = DpuMode;
	DpuContext->SrcAddress = (MMP_UINT32)NewSrcData+SrcOffSet;
	DpuContext->DstAddress = (MMP_UINT32)NewDstData+SrcOffSet;
	DpuContext->DataSize = DataSize;

	DpuContext->Keylength = KeyLen;
	DpuContext->Vectorlength = VectorLen;

	if(KeyLen)
		DpuContext->RefKeyAddress = (MMP_UINT32 *)key;

	if(VectorLen)
		DpuContext->Vectors = (MMP_UINT32 *)iv;

	mmpDpuEncode(DpuContext);

    if(DpuMode==CRC_MODE)
    {
        DataSize = 4;
    }
#ifdef ADDRESS_OFFSET
	#if defined(__FREERTOS__)
	memcpy( sysbuf, NewDstData, DataSize+4);
	#else
	HOST_ReadBlockMemory((MMP_UINT32)sysbuf, (MMP_UINT32)NewDstData, DataSize+4);//(MMP_UINT32)
	#endif
	memcpy( gSysBuf, sysbuf+SrcOffSet, DataSize);
#else
    #if defined(__FREERTOS__)
    memcpy( gSysBuf, NewDstData, DataSize);
    #else

	HOST_ReadBlockMemory((MMP_UINT32)gSysBuf, (MMP_UINT32)NewDstData, DataSize);
#endif
#endif
	//ShowBufferData(gSwResult, 64);
	//ShowBufferData(gSysBuf, 64);
	//ShowBufferData(aes_ecb_ct, 32);
	if(DpuMode==SCRAMBLE_MODE)
	{
		MMP_UINT32	fakeiv=0x11111111;
		MMP_UINT32	*ori_iv=(MMP_UINT32*)iv;

		memcpy( gSwResult, gSysBuf, DataSize);

#ifdef ADDRESS_OFFSET
	memset( sysbuf, 0x5A, SrcOffSet);
	memcpy( sysbuf+SrcOffSet, gSwResult, DataSize);
	memset( sysbuf+SrcOffSet+DataSize, 0x5A, 4-SrcOffSet);
	#if defined(__FREERTOS__)
	memcpy( NewSrcData, sysbuf, DataSize+4);
	#else
	HOST_WriteBlockMemory((MMP_UINT32)NewSrcData, (MMP_UINT32)sysbuf, DataSize+4);
	#endif
#else
    #if defined(__FREERTOS__)
    memcpy( NewSrcData, gSwResult, DataSize);
    #else
	HOST_WriteBlockMemory((MMP_UINT32)NewSrcData, (MMP_UINT32)gSwResult, DataSize);
    #endif
#endif
		if(fakeiv==ori_iv[0])
		{
			fakeiv = 0x22222222;
		}
		if(ori_iv[0]&0x10)
		{
			DpuContext->Vectors = (MMP_UINT32 *)&fakeiv;
		}

		mmpDpuEncode(DpuContext);

#ifdef ADDRESS_OFFSET
	#if defined(__FREERTOS__)
	    memcpy( sysbuf, NewDstData, DataSize+4);
	#else
	    HOST_ReadBlockMemory((MMP_UINT32)sysbuf, (MMP_UINT32)NewDstData, DataSize+4);//(MMP_UINT32)
	#endif
	memcpy( gSysBuf, sysbuf+SrcOffSet, DataSize);
#else
    #if defined(__FREERTOS__)
        memcpy( gSysBuf, NewDstData, DataSize);
    #else
	    HOST_ReadBlockMemory((MMP_UINT32)gSysBuf, (MMP_UINT32)NewDstData, DataSize);
    #endif
#endif
		TestResult=compare2buffer(gSysBuf,InPtnPtr,DataSize);
		if(ori_iv[0]&0x10)
		{
			printf("Compare error is correct if initial value(%x) is different in SRAMBLE mode!!\n",ori_iv[0]);
			if(TestResult)
			{
				TestResult=0;
			}
			else
			{
				TestResult=1;
			}
		}
	}
	else
	{
		TestResult=compare2buffer(gSysBuf,gSwResult,DataSize);
		//TestResult=compare2buffer(gSysBuf,aes_ecb_ct,DataSize);
	}

end:

	return TestResult;
}

MMP_UINT8 DecodeTest(MMP_UINT8 *InPtnPtr, MMP_UINT8 *key, MMP_UINT8 *iv, DPU_MODE DpuMode)
{
	MMP_UINT32	DataSize;
	MMP_UINT32	Reg32;
	MMP_UINT8	KeyLen=0;
	MMP_UINT8	VectorLen=0;
	MMP_UINT8	rst=MMP_FALSE;
	MMP_UINT8	TestResult=MMP_FALSE;
	SW_ENC_INFO SwEncodeInfo;
	MMP_UINT32	CmprDataSize=0;
	
	rst = GetParameter(&DataSize, &KeyLen, &VectorLen, DpuMode);

	SwEncodeInfo.DpuMode=DpuMode;
	SwEncodeInfo.InBuf=InPtnPtr;
	SwEncodeInfo.OutBuf=gSwResult;
	SwEncodeInfo.Key=key;
	SwEncodeInfo.SwEncSize=DataSize;
	SwEncodeInfo.iv=iv;

	SwDecryption(&SwEncodeInfo);

	memset(DpuContext, 0x00, sizeof(DpuContext));

#ifdef ADDRESS_OFFSET
	memset( sysbuf, 0x5A, SrcOffSet);
	memcpy( sysbuf+SrcOffSet, InPtnPtr, DataSize);
	memset( sysbuf+SrcOffSet+DataSize, 0x5A, 4-SrcOffSet);
    #if defined(__FREERTOS__)
    memcpy( NewSrcData, sysbuf, DataSize+4);
    #else
	HOST_WriteBlockMemory((MMP_UINT32)NewSrcData, (MMP_UINT32)sysbuf, DataSize+4);
	#endif
#else
    #if defined(__FREERTOS__)
    memcpy( NewSrcData, InPtnPtr, DataSize);
    #else
	HOST_WriteBlockMemory((MMP_UINT32)NewSrcData, (MMP_UINT32)InPtnPtr, DataSize);
    #endif
#endif
    //ShowBufferData(InPtnPtr, 32);
	memset(gSysBuf, 55, DataSize);

	DpuContext->DpuMode = DpuMode;
	DpuContext->SrcAddress = (MMP_UINT32)NewSrcData+SrcOffSet;
	DpuContext->DstAddress = (MMP_UINT32)NewDstData+SrcOffSet;
	DpuContext->DataSize = DataSize;

	DpuContext->Keylength = KeyLen;
	DpuContext->Vectorlength = VectorLen;

	if(KeyLen)
		DpuContext->RefKeyAddress = (MMP_UINT32 *)key;

	if(VectorLen)
		DpuContext->Vectors = (MMP_UINT32 *)iv;

	mmpDpuDecode(DpuContext);
	//ShowBufferData(InPtnPtr, 32);

	if(DpuMode==CRC_MODE)
	{
		CmprDataSize = 4;
	}
	else if (DpuMode==SHA2_MODE)
	{
	    //printf("Dec[sha2]\n");
		CmprDataSize = 32;
	}
	else
	{
		CmprDataSize = DataSize;
	}
#ifdef ADDRESS_OFFSET
	#if defined(__FREERTOS__)
	memcpy( sysbuf, NewDstData, DataSize+4);
	#else
	HOST_ReadBlockMemory((MMP_UINT32)sysbuf, (MMP_UINT32)NewDstData, DataSize+4);//(MMP_UINT32)
	#endif
	memcpy( gSysBuf, sysbuf+SrcOffSet, DataSize);
#else
	#if defined(__FREERTOS__)
	memcpy( gSysBuf, NewDstData, CmprDataSize);
	#else
	HOST_ReadBlockMemory((MMP_UINT32)gSysBuf, (MMP_UINT32)NewDstData, CmprDataSize);
    #endif
#endif
	//ShowBufferData(gSysBuf, 32);
	//ShowBufferData(gSwResult, 32);

	//TestResult=compare2buffer(gSysBuf,gSwResult,DataSize);
	TestResult=compare2buffer(gSysBuf,gSwResult,CmprDataSize);

//end:
#ifdef ADDRESS_OFFSET
	SrcOffSet=(++SrcOffSet)%4;
#endif

	return TestResult;
}

MMP_UINT8 CsaDecodeTest(MMP_UINT8 *Src, MMP_UINT8 *Dst, MMP_UINT32 CsaSize)
{
	MMP_UINT32	DataSize = 184;
	MMP_UINT32	Reg32;
	MMP_UINT8	*InPtr=MMP_NULL;
	MMP_UINT8	*OutPtr=MMP_NULL;
	MMP_UINT32	KeyData[4]={0x20bd9d64,0x91cff00c,0,0};
	MMP_UINT8	TestResult=MMP_FALSE;

#ifndef TEST

	//HOST_WriteBlockMemory((MMP_UINT32)SrcData, (MMP_UINT32)csa_pt, DataSize);
	HOST_WriteBlockMemory((MMP_UINT32)SrcData, (MMP_UINT32)Src, DataSize);

	AHB_WriteRegister(DPU_CTRL_REG, 0x00000001);	//reset DPU controller
	AHB_WriteRegister(DPU_CTRL_REG, 0x00000000);	//clear control register value
	AHB_WriteRegister(DPU_CTRL_REG, 0x22005020);							//
	AHB_WriteRegister(DPU_SRC_ADDR_REG, (MMP_UINT32)SrcData);
	AHB_WriteRegister(DPU_DST_ADDR_REG, (MMP_UINT32)DstData);
	AHB_WriteRegister(DPU_SRC_SIZE_REG, DataSize);

	AHB_WriteRegister(DPU_KEY0_REG, KeyData[0]);							//set key0
	AHB_WriteRegister(DPU_KEY1_REG, KeyData[1]);							//set key1

	AHB_WriteRegister(DPU_CTRL_REG, 0x22005022);
/*
	do
	{
		AHB_ReadRegister( DPU_STATUS_REG, &Reg32);
		AHB_WriteRegister(DPU_CTRL_REG, 0x00000002);
		AHB_ReadRegister( DPU_STATUS_REG, &Reg32);
	} while( (Reg32&0x00000001) != 0x00000001);
*/
	do
	{
		AHB_ReadRegister( DPU_CTRL_REG, &Reg32);
	} while( (Reg32&0x00000002) == 0x00000002);

	HOST_ReadBlockMemory((MMP_UINT32)CsaDecodeData, (MMP_UINT32)DstData, DataSize);

	//if( compare2buffer(DstData,csa_ct,DataSize)!=MMP_TRUE )
	//TestResult= compare2buffer(DstData,csa_ct,DataSize);
	TestResult= compare2buffer(CsaDecodeData, Dst, DataSize);

	return TestResult;
#else
	#if defined(__FREERTOS__)
	sysbuf  = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	SwResult = (MMP_UINT8 *)MEM_Allocate(gMaxDataSize+16);
	DpuContext = (DPU_CONTEXT *)MEM_Allocate(sizeof(DPU_CONTEXT));
	#else
	sysbuf  = (MMP_UINT8 *)SYS_Malloc(gMaxDataSize+16);
	SwResult = (MMP_UINT8 *)SYS_Malloc(gMaxDataSize+16);
	DpuContext = (DPU_CONTEXT *)SYS_Malloc(sizeof(DPU_CONTEXT));
	#endif

	SrcData = (MMP_UINT8 *)SYS_Malloc(DataSize);
	DstData = (MMP_UINT8 *)SYS_Malloc(DataSize);
	DpuContext = (DPU_CONTEXT *)SYS_Malloc(sizeof(DPU_CONTEXT));

	//HOST_WriteBlockMemory((MMP_UINT32)SrcData, (MMP_UINT32)ctr_pt, DataSize);

	DpuContext->DpuMode = CSA_MODE;
	DpuContext->SrcAddress = SrcData;
	DpuContext->DstAddress = ResultData;
	DpuContext->DataSize = DataSize;

	DpuContext->Keylength = KEY_LENTGH_64_BIT;
	DpuContext->Vectorlength = VECTOR_LENTGH_NO_VECTOR;

	DpuContext->RefKeyAddress = KeyData;
	//DpuContext->Vectors = VectorData;

	mmpDpuDecode(DpuContext);

	HOST_ReadBlockMemory((MMP_UINT32)DstData, (MMP_UINT32)ResultData, DataSize);

	TestResult= compare2buffer(DstData,Dst,DataSize);

	free(SrcData);
	free(DstData);
	#if defined(__FREERTOS__)
	SYS_free(DpuContext)
	#else

	#endif

	return TestResult;
#endif
}

MMP_UINT8 CrcMasterModeTest(MMP_UINT8 *Src, MMP_UINT32 DataSize)
{
	MMP_UINT32	*dwSrcData;
	MMP_UINT32	*HwCrcData;
	MMP_UINT32	*SwCrcData;
	DPU_CONTEXT *DpuContext;
	MMP_UINT32	Reg32;
	MMP_UINT8	TestResult=MMP_FALSE;

	dwSrcData = (MMP_UINT32 *)MEM_Allocate(DataSize);
	#if defined(__FREERTOS__)
	HwCrcData = (MMP_UINT32 *)MEM_Allocate(DataSize);
	SwCrcData = (MMP_UINT32 *)MEM_Allocate(DataSize);
	#else
	HwCrcData = (MMP_UINT32 *)SYS_Malloc(DataSize);
	SwCrcData = (MMP_UINT32 *)SYS_Malloc(DataSize);
	#endif

	AHB_WriteRegister(DPU_CRC32_REG0, 0xffffffff);							//set key0
	AHB_WriteRegister(DPU_CRC32_REG1, 0xffffffff);							//set key1

	AHB_WriteRegister(DPU_CTRL_REG, 0x20005004);

	//HOST_WriteBlockMemory((MMP_UINT32)dwSrcData, (MMP_UINT32)crc_pt, DataSize);
	HOST_WriteBlockMemory((MMP_UINT32)dwSrcData, (MMP_UINT32)Src, DataSize);

	AHB_WriteRegister(DPU_SRC_ADDR_REG, (MMP_UINT32)dwSrcData);
	AHB_WriteRegister(DPU_SRC_SIZE_REG, DataSize); //do 1 loop

	AHB_WriteRegister(DPU_CTRL_REG, 0x20005006);

	do
	{
		AHB_ReadRegister( DPU_CTRL_REG, &Reg32);
	} while( (Reg32&0x00000002) == 0x00000002);

	AHB_ReadRegister( DPU_CRC32_REG0, &Reg32);

	SW_CRC_CALCULATE((MMP_UINT32*)Src, (MMP_UINT32*)SwCrcData, DataSize);
	//TestResult= compare2buffer((MMP_UINT8*)&Reg32, (MMP_UINT8*)&SwCrcData[127], 4);
    TestResult= compare2buffer((MMP_UINT8*)&Reg32, (MMP_UINT8*)&SwCrcData[0], 4);

	MEM_Release(dwSrcData);
	#if defined(__FREERTOS__)
	free(HwCrcData);
	free(SwCrcData);
	#else
	SYS_Free(HwCrcData);
	SYS_Free(SwCrcData);
	#endif

	return TestResult;
}

MMP_UINT8 CrcSlaveModeTest(MMP_UINT32 *Src, MMP_UINT32 DataSize)
{
	//MMP_UINT32	*dwSrcData;
	MMP_UINT32	*SwCrcData;
	MMP_UINT32	*HwCrcData;
	//MMP_UINT32	DataSize = 512;
	MMP_UINT32	Reg32,i;
	MMP_UINT8	TestResult=MMP_FALSE;

	#if defined(__FREERTOS__)
	HwCrcData = (MMP_UINT32 *)MEM_Allocate(DataSize);
	SwCrcData = (MMP_UINT32 *)MEM_Allocate(DataSize);
	#else
	HwCrcData = (MMP_UINT32 *)SYS_Malloc(DataSize);
	SwCrcData = (MMP_UINT32 *)SYS_Malloc(DataSize);
	#endif

	AHB_WriteRegister(DPU_CTRL_REG,0x20000000);
	AHB_WriteRegister(DPU_CRC_SLAVE_MODE_DATA_REG,0x00000000);
	AHB_WriteRegister(DPU_CRC32_REG0,0xffffffff);
	AHB_WriteRegister(DPU_CRC32_REG1,0xffffffff);

    for(i=0;i<(DataSize/4);i++)
    {
        #ifdef  WIN32
        AHB_WriteRegister(DPU_CTRL_REG,0x20000000);
        HOST_WriteRegister(DPU_CRC_SLAVE_MODE_DATA_REG+0x02,(Src[i]>>16)&0xFFFF);
        AHB_WriteRegister(DPU_CTRL_REG,0x20080000);
        HOST_WriteRegister(DPU_CRC_SLAVE_MODE_DATA_REG,Src[i]&0xFFFF);	//not verified yet 20111004
        #else
        //AHB_WriteRegister(DPU_CTRL_REG,0x20000000);
        AHB_WriteRegister(DPU_CTRL_REG,0x20080000);
          #if defined(__OPENRTOS__) || defined(WIN32)
        AHB_WriteRegister(DPU_CRC_SLAVE_MODE_DATA_REG,Src[i]);
          #else
          {
              MMP_UINT8   Temp[4];
              MMP_UINT8  *pSrc=(MMP_UINT8*)(&Src[i]);
              MMP_UINT32  *pData=(MMP_UINT32 *)Temp;
              Temp[0]=pSrc[3];Temp[1]=pSrc[2];Temp[2]=pSrc[1];Temp[3]=pSrc[0];
              //printf("src=%08x, tmp=%08x\n",Src[i],pData[0]);
              AHB_WriteRegister(DPU_CRC_SLAVE_MODE_DATA_REG,pData[0]);
          }
          #endif
        #endif
        AHB_ReadRegister( DPU_CRC32_REG0,&Reg32);
        //HwCrcData[i] = Reg32;
        HwCrcData[0] = Reg32; //calculate the final CRC result
    }

	SW_CRC_CALCULATE((MMP_UINT32*)Src, (MMP_UINT32*)SwCrcData, DataSize);
	//TestResult= compare2buffer((MMP_UINT8*)HwCrcData, (MMP_UINT8*)SwCrcData, DataSize);
    TestResult= compare2buffer((MMP_UINT8*)HwCrcData, (MMP_UINT8*)SwCrcData, 4);

	#if defined(__FREERTOS__)
	free(HwCrcData);
	free(SwCrcData);
	#else
	SYS_Free(HwCrcData);
	SYS_Free(SwCrcData);
	#endif

	return TestResult;
}

MMP_UINT8 AES_Test(void)
{
	MMP_UINT8	AesResult=MMP_TRUE;
	MMP_UINT8	Aes_Random_key[32];
	MMP_UINT8	Aes_iv[32];
	MMP_UINT8	ModeIdx;
	DPU_MODE	DesMode[5]={AES_ECB_MODE,AES_CBC_MODE,AES_CFB_MODE,AES_OFB_MODE,AES_CTR_MODE};
	//MMP_UINT8	FixedData[64]={0x5d, 0xeb, 0x40, 0x73, 0xb0, 0x2a, 0x1c, 0x5a, 0xd1, 0x84, 0x16, 0x92, 0xf9, 0xde, 0xd0, 0x44,
    //                           0x7c, 0xcb, 0xe4, 0xb9, 0x4f, 0x00, 0x81, 0x6f, 0xc7, 0x09, 0x55, 0x6c, 0xa3, 0x3a, 0xb0, 0xd2,
    //                           0x4d, 0xe7, 0xe6, 0xb4, 0x5d, 0xd8, 0xf5, 0x28, 0x5f, 0xf7, 0x67, 0x9d, 0xd6, 0x4c, 0x7a, 0x6f,
    //                           0x81, 0x2b, 0x7f, 0xb9, 0xdc, 0xac, 0x80, 0x6b, 0xea, 0x5c, 0x25, 0x9c, 0x32, 0x32, 0xd8, 0x21};
	

	AutoGenPatternInBuffer(Aes_Random_Data, AES_MODE_DATA_SIZE);
	//memcpy(Aes_Random_Data,aes_ecb_pt,DES_MODE_DATA_SIZE);
	MMP_Sleep(5);
	//ShowBufferData(Aes_Random_Data, 32);

	AutoGenPatternInBuffer(Aes_Random_key, 32);
	//memset(Aes_Random_key, 0, 32);
	MMP_Sleep(5);
	//ShowBufferData(Aes_Random_key, 16);

	AutoGenPatternInBuffer(Aes_iv, 32);
	//ShowBufferData(Aes_iv, 8);

	for(ModeIdx=0;ModeIdx<5;ModeIdx++)
	{
		//aes encode test
		if(EncodeTest(Aes_Random_Data, Aes_Random_key, Aes_iv, DesMode[ModeIdx])!=MMP_TRUE)
		{
			printf("Aes EncodeTest fail,mode=[%x]!!\n",DesMode[ModeIdx]);
			AesResult=MMP_FALSE;
		}
		else
		{
			//printf("Aes EncodeTest Success!!,mode=[%x]!!\n",DesMode[ModeIdx]);
		}

		//aes decode test//
		if(DecodeTest(Aes_Random_Data, Aes_Random_key, Aes_iv, DesMode[ModeIdx])!=MMP_TRUE)
		{
			printf("Aes DecodeTest fail,mode=[%x]!!\n",DesMode[ModeIdx]);
			AesResult=MMP_FALSE;
		}
		else
		{
			//printf("Aes DecodeTest Success!!,mode=[%x]!!\n",DesMode[ModeIdx]);
		}
	}

	return AesResult;
}

MMP_UINT8 CRC_Test(void)
{
	MMP_UINT8	CrcResult=MMP_TRUE;
	MMP_UINT8	*SrcAddr;

	//crc master mode test
#ifdef	ENABLE_CRC_FIX_PATTERN
	if( CrcMasterModeTest((MMP_UINT8 *)crc_pt, 512)!=MMP_TRUE )
	{
		printf("	CrcMasterModeTest fail!!\n");
		CrcResult=MMP_FALSE;
	}
	else
	{
		//printf("	CrcMasterModeTest Success!!\n");
	}

	//crc slave mode test
	if( CrcSlaveModeTest(crc_pt, 512)!=MMP_TRUE )
	{
		printf("	CrcSlaveModeTest fail!!\n");
		CrcResult=MMP_FALSE;
	}
	else
	{
		//printf("	CrcSlaveModeTest Success!!\n");
	}
#else
	//auto gen CRC pattern
	#if defined(__FREERTOS__)
	SrcAddr = (MMP_UINT8 *)MEM_Allocate(CRC_MODE_DATA_SIZE+16);
	#else
	SrcAddr = (MMP_UINT8 *)SYS_Malloc(CRC_MODE_DATA_SIZE+16);
	#endif

	AutoGenPatternInBuffer(SrcAddr, CRC_MODE_DATA_SIZE);
	//AutoGenPatternInBuffer(Csa_Random_Data, CRC_MODE_DATA_SIZE);

	//if( CrcMasterModeTest(SrcAddr, 512)!=MMP_TRUE )
    if(EncodeTest(SrcAddr, MMP_NULL, MMP_NULL, CRC_MODE)!=MMP_TRUE)
	//if(EncodeTest(Csa_Random_Data, MMP_NULL, MMP_NULL, CRC_MODE)!=MMP_TRUE)
	{
		printf("	CrcMasterModeTest fail!!\n");
		CrcResult=MMP_FALSE;
	}
	else
	{
		//printf("	CrcMasterModeTest Success!!\n");
	}

	//crc slave mode test
	if( CrcSlaveModeTest( (MMP_UINT32*)SrcAddr, CRC_MODE_DATA_SIZE)!=MMP_TRUE )
	{
		printf("	CrcSlaveModeTest fail!!\n");
		CrcResult=MMP_FALSE;
	}
	else
	{
		//printf("	CrcSlaveModeTest Success!!\n");
	}
	#if defined(__FREERTOS__)
	free(SrcAddr);
	#else
	SYS_Free(SrcAddr);
#endif
#endif

	return CrcResult;
}


MMP_UINT8 CSA_Test(void)
{
	MMP_UINT8	CsaResult=MMP_TRUE;
    MMP_UINT8	Csa_Random_Key[16];

#ifdef	ENABLE_CSA_FIX_PATTERN
	CsaDecodeBySoftware(Csa_Src_Data, Csa_Result_Data, Csa_Random_Key, 184);
	if( CsaDecodeTest(Csa_Src_Data, Csa_Result_Data, 184)!=MMP_TRUE )
#else
	AutoGenPatternInBuffer(Csa_Random_Data, 184);
	AutoGenPatternInBuffer(Csa_Random_Key, 8);
	//CsaDecodeBySoftware(Csa_Random_Data, Csa_Result_Data, Csa_Random_Key, 184);
	//if( CsaDecodeTest(Csa_Random_Data, Csa_Result_Data, 184)!=MMP_TRUE )
	if(DecodeTest(Csa_Random_Data, Csa_Random_Key, MMP_NULL, CSA_MODE)!=MMP_TRUE)
#endif
	{
		printf("	CsaDecodeTest fail!!\n");
		CsaResult=MMP_FALSE;
	}
	else
	{
		//printf("	CsaDecodeTest Success!!\n");
	}

	return CsaResult;
}

MMP_UINT8 DES_Test(void)
{
	MMP_UINT8	DesResult=MMP_TRUE;
	MMP_UINT8	*pAutoPtn=MMP_NULL;
	MMP_UINT8	Des_Random_Key[24];
	MMP_UINT8	Des_Random_Iv[24];
	MMP_UINT8	ModeIdx;
	DPU_MODE	DesMode[5]={DES_ECB_MODE,DES_CBC_MODE,DES_CFB_MODE,DES_OFB_MODE,DES_CTR_MODE};

	//AutoGenPatternInBuffer(pAutoPtn, 512);
	AutoGenPatternInBuffer(Des_Random_Data, DES_MODE_DATA_SIZE);
	MMP_Sleep(5);
	AutoGenPatternInBuffer(Des_Random_Key, 24);
	MMP_Sleep(5);
	AutoGenPatternInBuffer(Des_Random_Iv, 24);

	for(ModeIdx=0;ModeIdx<5;ModeIdx++)
	{
		//des3 encode test
		if(ModeIdx<5)
		{
			if(EncodeTest(Des_Random_Data, Des_Random_Key, Des_Random_Iv, DesMode[ModeIdx])!=MMP_TRUE)
			{
				printf("Des EncodeTest fail,mode=[%x,%x]!!\n",ModeIdx,DesMode[ModeIdx]);
				DesResult=MMP_FALSE;
			}
			else
			{
				//printf("Des Ecb EncodeTest Success!!,mode=[%x]!!\n",DesMode[ModeIdx]);
			}
		}

		//des3 decode test//
		if(ModeIdx<5)
		{
			if(DecodeTest(Des_Random_Data, Des_Random_Key, Des_Random_Iv, DesMode[ModeIdx])!=MMP_TRUE)
			{
				printf("Des DecodeTest fail,mode=[%x,%x]!!\n",ModeIdx,DesMode[ModeIdx]);
				DesResult=MMP_FALSE;
			}
			else
			{
				//printf("Des Ecb DecodeTest Success!!,mode=[%x]!!\n",DesMode[ModeIdx]);
			}
		}
	}

	return DesResult;
}

MMP_UINT8 DES3_Test(void)
{
	MMP_UINT8	Des3Result=MMP_TRUE;
	MMP_UINT8	Des3_Key[24];
	MMP_UINT8	Des3_IV[24];
	MMP_UINT8	ModeIdx;
	DPU_MODE	Des3Mode[5]={DES3_ECB_MODE,DES3_CBC_MODE,DES3_CFB_MODE,DES3_OFB_MODE,DES3_CTR_MODE};

	AutoGenPatternInBuffer(Des_Random_Data, DES_MODE_DATA_SIZE);
	MMP_Sleep(5);
	AutoGenPatternInBuffer(Des3_Key, 24);
	MMP_Sleep(5);
	AutoGenPatternInBuffer(Des3_IV, 8);

	for(ModeIdx=0;ModeIdx<5;ModeIdx++)
	{
		//des3 encode test
		if(ModeIdx<5)
		{
			if(EncodeTest(Des_Random_Data, Des3_Key, Des3_IV, Des3Mode[ModeIdx])!=MMP_TRUE)
			{
				printf("Des3 EncodeTest fail,mode=[%x]!!\n",Des3Mode[ModeIdx]);
				Des3Result=MMP_FALSE;
			}
			else
			{
				//printf("Des3EcbEncodeTest Success!!\n");
			}
		}

		//des3 decode test//
		if(ModeIdx<5)
		{
			if(DecodeTest(Des_Random_Data, Des3_Key, Des3_IV, Des3Mode[ModeIdx])!=MMP_TRUE)
			{
				printf("Des3 DecodeTest fail,mode=[%x]!!\n",Des3Mode[ModeIdx]);
				Des3Result=MMP_FALSE;
			}
			else
			{
				//printf("Des3EcbDecodeTest Success!!\n");
			}
		}

	}

	return Des3Result;
}

#if defined(MM9910)
#ifdef ENABLE_SHA2
MMP_UINT8 SHA2_Test(void)
{
	MMP_UINT8	ShaResult=MMP_TRUE;
    MMP_UINT8	Sha2_Random_Key[16];
    //MMP_UINT32	Csa_Random_Key_1[4]={0x20bd9d64,0x91cff00c,0,0};
#ifdef ENABLE_SHA2
#ifdef	ENABLE_SHA2_FIX_PATTERN
	Sha2EncodeBySoftware(Csa_Src_Data, Csa_Result_Data, Csa_Random_Key, 184);
	if( CsaDecodeTest(Csa_Src_Data, Csa_Result_Data, 184)!=MMP_TRUE )
#else
	AutoGenPatternInBuffer(Sha2_Random_Data, 512);
	AutoGenPatternInBuffer(Sha2_Random_Key, 16);
	//memset(Sha2_Random_Data, 0x11, 512);
/*
	memset(Sha2_Random_Data, 0x11, 4);
	memset(Sha2_Random_Data+4, 0x22, 4);
	memset(Sha2_Random_Data+8, 0x33, 4);
	memset(Sha2_Random_Data+12, 0x44, 4);
	
	memset(Sha2_Random_Data+16, 0x55, 4);
	memset(Sha2_Random_Data+20, 0x66, 4);
	memset(Sha2_Random_Data+24, 0x77, 4);
	memset(Sha2_Random_Data+28, 0x88, 4);
	
	memset(Sha2_Random_Data+32, 0x99, 4);
	memset(Sha2_Random_Data+36, 0xAA, 4);
	memset(Sha2_Random_Data+40, 0xBB, 4);
	memset(Sha2_Random_Data+44, 0xCC, 4);
	
	memset(Sha2_Random_Data+48, 0xDD, 4);
	memset(Sha2_Random_Data+52, 0xEE, 4);
	memset(Sha2_Random_Data+56, 0xFF, 4);
	memset(Sha2_Random_Data+60, 0x5A, 4);
*/
	if(DecodeTest(Sha2_Random_Data, Sha2_Random_Key, MMP_NULL, SHA2_MODE)!=MMP_TRUE)
#endif
	{
		printf("	CsaDecodeTest fail!!\n");
		ShaResult=MMP_FALSE;
	}
	else
	{
		//printf("	CsaDecodeTest Success!!\n");
	}
#endif
	return ShaResult;
}
#endif

MMP_UINT8 SCRAMBLE_Test(void)
{
	MMP_UINT8	ScrResult=MMP_TRUE;
	MMP_UINT8	Scramble_Key[24];
	MMP_UINT8	Scramble_IV[24];
	MMP_UINT8	ModeIdx;

	AutoGenPatternInBuffer(Scramble_Random_Data, 512);
	//MMP_Sleep(5);
	//AutoGenPatternInBuffer(Scramble_Key, 24);
	//MMP_Sleep(5);
	AutoGenPatternInBuffer(Scramble_IV, 4);

	//scramble encode/decode test
	if(EncodeTest(Scramble_Random_Data, MMP_NULL, Scramble_IV, SCRAMBLE_MODE)!=MMP_TRUE)
	{
		printf("Scramble EncodeTest fail!!\n");
		ScrResult=MMP_FALSE;
	}
	else
	{
		//printf("SCRAMBLE EncodeTest Success!!\n");
	}

	return ScrResult;
}
#endif

MMP_UINT8 compare2buffer(MMP_UINT8 * SrcBuffer, MMP_UINT8 * DstBuffer, MMP_UINT32 count )
{
	MMP_UINT8	CprResult=MMP_TRUE;

	for(i=0;i<count;i++)
	{
		if(SrcBuffer[i]!=DstBuffer[i])
		{
			printf("Comapre Error,i=%x,Src,Dst=[%x,%x]\n",i,SrcBuffer[i],DstBuffer[i]);
			CprResult = MMP_FALSE;
			break;
		}
	}

	return CprResult;
}

void ShowBufferData(MMP_UINT8 *DataBuffer, MMP_UINT32 Datalen)
{
	MMP_UINT32	k;

	printf("Show Buffer data from [%x] to [%x]\n",DataBuffer, DataBuffer+Datalen );
	for(k=0;k<Datalen;k++)
	{
		printf("0x%02x,",DataBuffer[k]);
		if( (k&0x3FF)==0x3FF )	printf("\n[%x]",k+1);
		if( (k&0x0F)==0x0F )	printf("\n");
	}
	printf("\n");
}

#ifdef	ENABLE_CSA_SW_DECODE


//=================================
// Get bit_num from sr
static unsigned int get_sr_bit(SHIFTREG sr, unsigned int bit_num)
{
    return ((sr.cell[bit_num/4] >> (3-(bit_num % 4))) & 1);
}

//==========================================
// Shift SR by one nibble and push in the feedback nibble
static void shift_register (SHIFTREG *sr, unsigned int feedback_nibble)
{
    int i;
    for(i=sr->len-1;i>0;i--)
        sr->cell[i] = sr->cell[i-1];
    sr->cell[0] = feedback_nibble;
}

//static int s_idx = 0;
//============================================
static void do_nibble(MMP_UINT8 in, MMP_UINT8 bb)
{
    unsigned int k,j,x,y,z,p,q, local, sav_d, sav_e;
    sav_d = s_d; sav_e = s_e;
    x = (s_c>>10) & 0x0f;
    y = (s_c>>6) & 0x0f;
    z = (s_c>>2) & 0x0f;
    p = (s_c>>1) & 1;
    q = s_c & 1;

    // Update C according to S
    for(k=0,s_c=0;k<NUM_OF_SBOXES;k++)
    {
        for(j=0, local=0; j<SBOX_INPUT_SIZE;j++)
            local |= get_sr_bit(sr_a, bit_from_a[k][j]-1) << (4-j);
        s_c |= (s_Sbox[k][local] >> 1) << (14-bit_to_c[k][0]);
        s_c |= (s_Sbox[k][local] & 1) << (14-bit_to_c[k][1]);
    }

    // Update D, according to T3
    for(k=0,local = 0;k<4;k++)
        for(j=0;j<4;j++)
            local ^= get_sr_bit(sr_b, bit_from_b[3-k][j]-1) << k;
    s_d = local ^ sav_e ^ z;
    s_bib  = ((s_d & 8) >> 2) ^ ((s_d & 4) >> 1);
    s_bib |= ((s_d & 2) >> 1) ^ (s_d & 1);

    // Update E, and then F and r, accroding to T4
    s_e = s_f;
    if(q == 1)
    {
        s_f = (local = (sav_e + z + s_r)) & 0x0f;
        s_r = (local > 0x0f) ? 1: 0;
    }
    else
        s_f = sav_e;

    // Update A according to T1
    x ^= (sr_a.cell[10-1]);
    if(bb==0)
        x ^= ((in>>4) & 0x0f) ^ sav_d;
    shift_register (&sr_a, x);

    // Update B, according to T2
    y ^= sr_b.cell[10-1] ^ sr_b.cell[7-1];
    if(bb==0)
        y ^= (in & 0x0f);
    if(p==1)
        y = ((y & 7) << 1) | ((y & 8) >> 3);
    shift_register (&sr_b, y);
}

//===================================
// Reset of the stream cipher
static void reset_stream(MMP_UINT8 *cw)
{
    int j;
    sr_a.len = sr_b.len = 10;
    sr_a.cell[8] = sr_b.cell[8] = 0;
    sr_a.cell[9] = sr_b.cell[9] = 0;
    s_c = s_d = s_e = s_f = s_r = 0;
    for(j=0;j<4;j++)
    {
        sr_a.cell[j+j] = (cw[j] >> 4) & 0x0f;
        sr_a.cell[j+j+1] = cw[j] & 0x0f;
        sr_b.cell[j+j] = (cw[j+4] >> 4) & 0x0f;
        sr_b.cell[j+j+1] = cw[j+4] & 0x0f;
    }
}

//==================================
// Initialization of the stream cipher
static void init_stream(MMP_UINT8 in)
{
    MMP_UINT8 ni;
    ni = ((in >> 4) & 0x0f) | ((in & 0x0f) <<4);
    do_nibble(in, 0);
    do_nibble(ni, 0);
    do_nibble(in, 0);
    do_nibble(ni, 0);
}

//================================
// Generation of one scrambling byte
static MMP_UINT8 scrambling_byte()
{
    unsigned char sbyte;
    do_nibble(0,1);
    sbyte = s_bib << 6;
    do_nibble(0,1);
    sbyte |= s_bib << 4;
    do_nibble(0,1);
    sbyte |= s_bib << 2;
    do_nibble(0,1);
    sbyte |= s_bib;
    return sbyte;
}

//==============================
static void key_deperm(MMP_UINT8 k[8])
{
    int         j,dst;
    MMP_UINT8      work[8];
    memset(work, '\0', 8);
//  printf("\n\n");
    for(j=0;j<64;j++)
    {
        dst = kd_perm[j]-1;
        if(k[j>>3] & b_mask[j&7])
            work[dst>>3] |= b_mask[dst&7];

//    printf("(%d.%d) <= (%d.%d), %d \n",dst>>3, 7-(dst&7), j>>3, 7-(j&7), j/8*8+7-(j&7));
    }
//  printf("\n\n");
    memcpy(k, work, 8);
}

//==================================
// The permuted byte is built up, starting from the msb on the left
static MMP_UINT8 c_perm(MMP_UINT8 x)
{
    static MMP_UINT8 perm[8] = {5, 8, 2, 6, 4, 3, 1, 7};
 //   static MMP_UINT8 perm[8] = {4, 1, 7, 3, 5, 6, 8, 2};
    int j;
    MMP_UINT8 ans = 0;
    for(j=0;j<8;j++)
    {
        if(x&b_mask[j])
            ans |= b_mask[perm[j]-1];
    }
    return ans;
}

//===================================
// Block cipher, key schedule
static void key_schedule(MMP_UINT8 key[8])
{
    int j,i;
    MMP_UINT8 work[8];
    memcpy(work, key, 8);
    memcpy(&b_kb[6][0], &work, 8);
    for(j=5;j>-1;j--)
    {
        key_deperm(work);
        memcpy(&b_kb[j][0], &work, 8);
    }
}

//==================================
// Block cipher, decipherment
static void decipher(MMP_UINT8 src[8])
{
    int j, k ,i;
    MMP_UINT8 kv, idx, fb, tmp, sr[8];
    //ex=fopen("test.txt", "wt");
    // Copy message into scratch RAM
    memcpy(sr, src, 8);
    for(j=6;j>-1;j--)
    {
        for(k=7;k>-1;k--)
        {
            for(i=7;i>-1;i--)
            {
                #if defined(WIN32)
                fprintf(ex, "%02x ", sr[i]);
                #endif
            }
            // Get the key byte for this step
            kv = b_kb[j][k];
            idx = sr[6]^kv^j;
            // Output of the sbox
            fb = b_Sbox[idx];

            tmp = sr[7]^fb;
            sr[7] = sr[6];
            sr[6] = sr[5]^c_perm(fb);
            sr[5] = sr[4];
            sr[4] = sr[3]^tmp;
            sr[3] = sr[2]^tmp;
            sr[2] = sr[1]^tmp;
            sr[1] = sr[0];
            sr[0] = tmp;
            #if defined(WIN32)
            fprintf(ex, "fb=%02x \n",fb);
            #endif
        }
    }
    memcpy(src, sr, 8);
}

#if defined(WIN32)
void CsaDecodeBySoftware(MMP_UINT8 *scrambled_field, MMP_UINT8 *Csa_Result, MMP_UINT8 *Csa_Key, MMP_UINT32 CsaSize)
#else
void CsaDecodeBySoftware(MMP_UINT8 *scrambled_field, MMP_UINT8 *Csa_Result, MMP_UINT8 *control_word, MMP_UINT32 CsaSize)
#endif
{
    int         i,j,k,r;
    MMP_UINT8	common_key[8];
    MMP_UINT8	reg0[8], reg1[8], reg2[8];
    MMP_UINT8	sort[8] = {0,0,0,0,0,0,0,0};
#if defined(WIN32)
    FILE *ans;
    MMP_UINT8	control_word[8] = {Csa_Key[0], Csa_Key[1], Csa_Key[2], Csa_Key[3],
                                   Csa_Key[4], Csa_Key[5], Csa_Key[6], Csa_Key[7]};
#endif
    int         mode = 1;
    int         p = 184;
	MMP_UINT8	RsIdx=0;

#if defined(WIN32)
    ans = fopen("result.bin", "wt");
#endif
	memset(Csa_Result,0xFF,184);

    // Conformace mechainism
    for(i=0;i<8;i++)
        common_key[i] = control_word[i];

    // 0: without conformace mechanism
    // 1: with conformance mechanism
    if(mode==1)
    {
        for(i=0;i<2;i++)
        {
            k=0;
            for(j=0;j<3;j++)
                k += common_key[4*i+j];
            common_key[4*i+3] = k&0xff;
        }
    }

    // Initialization of the 56 key bytes for the block cipher
    key_schedule(common_key);

    // Reset of the stream cipher
    reset_stream(common_key);
    #if defined(WIN32)
	ex=fopen("test.txt", "wt");
    #endif

    j=0;
    for(i=0;i<p+8-(p&7);i++)
    {
        reg1[j] = reg0[j];
        if(i<8)
            init_stream(scrambled_field[i]);

        if(i<p)
        {
            //printf( "%d: s_b(%3d)= %02x; ",j, i+1, scrambled_field[i]);
            reg0[j] = scrambled_field[i];
            if(i>7)
            {
                //printf( "%02x^", reg0[j]);
                reg0[j] ^= (k = scrambling_byte());
                //printf( "%02x = %02x; ", k, reg0[j]);
            }
        }

        if((i>15)&(i<p+8))
        {
            //printf( "%02x^%02x ", reg1[j], reg2[j]);
            reg2[j] ^= reg1[j];
            //printf( "= %02x =d_b(%3d)\n", reg2[j], i-15);
            #if defined(WIN32)
			fprintf(ans, " %02x \n",reg2[j]);//SOL
            #endif
			Csa_Result[RsIdx]=reg2[j];
			RsIdx++;
        }

        j+=1;
        if((j==8)&(i<p+8))
        {
            //printf( "\n");
            if(i>14)
            {
                //printf( "Deciphered block:              ");
                for(k=0;k<8;k++)
                    sort[k] = reg1[k];
                decipher(sort);
                for(k=0;k<8;k++)
                {
					/*
                    if(k==4)
                        printf( " ");
						*/
                    //printf( "%02x", sort[k]);
                    reg2[k] = sort[k];
                }
                //printf( "\n");
            }
            j=0;
        }
    }
    #if defined(WIN32)
    fclose(ex);
    #endif
    r = p&7;

    if(p>7)
    {
        for(i=0;i<8;i++)
        {
            //printf( "                                  ");
            //printf( "%02x =d_b(%3d)\n", reg2[i], p-r+i-7);
            #if defined(WIN32)
			fprintf(ans, " %02x \n",reg2[i]);//SOL
            #endif
			Csa_Result[RsIdx++]=reg2[i];
        }
        //printf( "\n");
    }
    for(i=0;i<r;i++)
    {
        //printf( "                                  ");
        //printf( "%02x =d_b(%3d)\n", reg0[i], p-r+i+1);
        #if defined(WIN32)
		fprintf(ans, " %02x \n",reg0[i]);//SOL
        #endif
		Csa_Result[RsIdx++]=reg0[i];
    }
    //fclose(ex);
    #if defined(WIN32)
    fclose(ans);
    #endif
    return;

}

void AutoGenPatternInBuffer(MMP_UINT8 *buffer1, MMP_UINT32 totalBytes)
{
	MMP_UINT32	idx=0;
	MMP_UINT32	lastSec1,lastSec2,LoopCounter;
	MMP_UINT8	GenResult = MMP_TRUE;

	//printf("AutoGenPatternInBuffer: argv[0]~[3]=[%x,%d]\n",buffer1, totalBytes);

	//set_rand();
	lastSec2 = PalGetClock();
	PalSrand(lastSec2);

	while(1)
	{
		LoopCounter = PalRand();
		buffer1[idx] = (MMP_UINT8)(LoopCounter & 0xFF);
		idx++;
		if( idx>=(totalBytes) )	break;
	}

	//return GenResult;
}
void ExOrOneByte(unsigned char *Buf1, unsigned char *Buf2, unsigned char *OutBuf)
{
	unsigned char	A=Buf1[0];
	unsigned char	B=Buf2[0];

	OutBuf[0]=((A&(~B)) | ((~A)&B));
}

void ExOr(unsigned char *Buf1, unsigned char *Buf2, unsigned char *OutBuf, unsigned int DataLength)
{
	unsigned int i;

	for(i=0; i<DataLength; i++)
	{
		ExOrOneByte(&Buf1[i], &Buf2[i], &OutBuf[i]);
	}
}

void SwDesEcbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *DesKey)
{
	des_context ctx;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des_key[24];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des_key, DesKey, 8 );
	memcpy( buf, InBuf, 8 );

	des_setkey_enc( &ctx, (unsigned char *) des_key );
	des_crypt_ecb( &ctx, buf, buf );

	memcpy( OutBuf, buf, 8 );
#else
	memset( des_key, 0, 24 );

	//for(i=0;i<24;i++)
	for(i=0;i<8;i++)
	{
		//des_key[i]=DesKey[23-i];
		des_key[i]=DesKey[7-i];
	}

	for(i=0;i<8;i++)
	{
		buf[i]=InBuf[7-i];
	}
	//memcpy( buf, InBuf, 8 );
	des_setkey_enc( &ctx, (unsigned char *) des_key );
	des_crypt_ecb( &ctx, buf, buf );

	for(i=0;i<8;i++)
	{
		OutBuf[i]=buf[7-i];
	}
#endif
}

void SwDesEcbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *DesKey)
{
	des_context ctx;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des_key[24];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des_key, DesKey, 8 );
	memcpy( buf, InBuf, 8 );

	des_setkey_dec( &ctx, (unsigned char *) des_key );
	des_crypt_ecb( &ctx, buf, buf );

	memcpy( OutBuf, buf, 8 );
#else
	memset( des_key, 0, 24 );

	//for(i=0;i<24;i++)
	for(i=0;i<8;i++)
	{
		//des_key[i]=DesKey[23-i];
		des_key[i]=DesKey[7-i];
	}

	for(i=0;i<8;i++)
	{
		buf[i]=InBuf[7-i];
	}
	des_setkey_dec( &ctx, (unsigned char *) des_key );
	des_crypt_ecb( &ctx, buf, buf );
	for(i=0;i<8;i++)
	{
		OutBuf[i]=buf[7-i];
	}
#endif
}


void SwAesEcbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *AesKey)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char aes_key[32];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( aes_key, AesKey, 16 );
	memcpy( buf, InBuf, 16 );	

	aes_setkey_enc( &ctx, (unsigned char *) aes_key, 128 );
	aes_crypt_ecb( &ctx, 1, buf, buf );	//1 means encrypt

	memcpy( OutBuf, buf, 16 );
#else
	memset( aes_key, 0, 32 );

	for(i=0;i<16;i++)
	{
		aes_key[i]=AesKey[15-i];
	}

	for(i=0;i<16;i++)
	{
		buf[i]=InBuf[15-i];
	}
	//memcpy( buf, InBuf, 8 );
	aes_setkey_enc( &ctx, (unsigned char *) aes_key, 128 );
	aes_crypt_ecb( &ctx, 1, buf, buf );	//1 means encrypt

	for(i=0;i<16;i++)
	{
		OutBuf[i]=buf[15-i];
	}
#endif
}

void SwAesEcbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *AesKey)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char aes_key[32];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( aes_key, AesKey, 16 );
	memcpy( buf, InBuf, 16 );	

	aes_setkey_dec( &ctx, (unsigned char *) aes_key, 128 );
	aes_crypt_ecb( &ctx, 0, buf, buf );	//1 means encrypt
	memcpy( OutBuf, buf, 16 );
#else
	memset( aes_key, 0, 32 );

	for(i=0;i<16;i++)
	{
		aes_key[i]=AesKey[15-i];
	}

	for(i=0;i<16;i++)
	{
		buf[i]=InBuf[15-i];
	}
	aes_setkey_dec( &ctx, (unsigned char *) aes_key, 128 );
	aes_crypt_ecb( &ctx, 0, buf, buf );		//0 means decrypt
	for(i=0;i<16;i++)
	{
		OutBuf[i]=buf[15-i];
	}
#endif
}

void SwAesCbcEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char aes_key[16];
	unsigned char aes_vector[16];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( aes_key, Key, 16 );
	memcpy( aes_vector, vector, 16 );
	memcpy( buf, InBuf, 16 );

	aes_setkey_enc( &ctx, aes_key, 128 );
	aes_crypt_cbc( &ctx, 1, 16, aes_vector, buf, buf );

	memcpy( OutBuf, buf, 16 );
#else
	memset( aes_key, 0, 16 );
	memset( aes_vector, 0, 16 );

	for(i=0;i<16;i++)
	{
		aes_key[i]=Key[15-i];
	}

	for(i=0;i<16;i++)
	{
		aes_vector[i]=vector[15-i];
	}

	for(i=0;i<16;i++)
	{
		buf[i]=InBuf[15-i];
	}
	aes_setkey_enc( &ctx, aes_key, 128 );
	aes_crypt_cbc( &ctx, 1, 16, aes_vector, buf, buf );

	for(i=0;i<16;i++)
	{
		OutBuf[i]=buf[15-i];
	}
#endif
}

void SwAesCfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char buf2[16];
	unsigned char aes_key[16];
	unsigned char aes_vector[16];

	SwAesEcbEnc(vector, buf, Key);

	ExOr(buf,InBuf, OutBuf, 16);
}

void SwAesOfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char aes_key[16];
	unsigned char aes_vector[16];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( aes_key, Key, 16 );
	memcpy( buf, vector, 16 );

	SwAesEcbEnc(vector, buf, Key);
	ExOr(buf,InBuf, OutBuf, 16);

	memcpy( vector, buf, 16 );
#else
	memset( aes_key, 0, 16 );

	for(i=0;i<16;i++)
	{
		aes_key[i]=Key[15-i];
	}

	for(i=0;i<16;i++)
	{
		buf[i]=vector[15-i];
	}

	SwAesEcbEnc(vector, buf, Key);
	ExOr(buf,InBuf, OutBuf, 16);

	for(i=0;i<16;i++)
	{
		vector[i]=buf[i];
	}
#endif
}


void SwAesCtrEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char aes_key[16];
	unsigned char aes_vector[16];
	unsigned int *ivPtr;

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( aes_key, Key, 16 );
	memcpy( buf, vector, 16 );

	SwAesEcbEnc(vector, buf, Key);
	ExOr(buf,InBuf, OutBuf, 16);

	for(i=0;i<16;i++)
	{
		vector[15-i]++;
		if(vector[15-i])	break;
	}
#else
	memset( aes_key, 0, 16 );

	for(i=0;i<16;i++)
	{
		aes_key[i]=Key[15-i];
	}

	for(i=0;i<16;i++)
	{
		buf[i]=vector[15-i];
	}

	SwAesEcbEnc(vector, buf, Key);
	ExOr(buf,InBuf, OutBuf, 16);

	#ifndef  NEW_CODE
	for(i=0;i<16;i++)
	{
	    if(vector[i]==0xFF)
	    {
	        vector[i]=0;	        
	    }
	    else
	    {
	        vector[i]++;
	        break;
	    }
    }
	#else

	ivPtr=(unsigned int *)vector;

	ivPtr[0]++;

	if(!ivPtr[0])
	{
		ivPtr[1]++;
		if(!ivPtr[1])
		{
			ivPtr[2]++;
			if(!ivPtr[2])
			{
				ivPtr[3]++;
			}
		}
	}
	#endif
#endif
}

void SwAesCbcDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *vector)
{
	aes_context ctx;
	unsigned char i;
	unsigned char buf[16];
	unsigned char aes_key[16];
	unsigned char aes_vector[16];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( aes_key, Key, 16 );
	memcpy( aes_vector, vector, 16 );
	memcpy( buf, InBuf, 16 );

	aes_setkey_dec( &ctx, aes_key, 128 );
	aes_crypt_cbc( &ctx, 0, 16, aes_vector, buf, buf );

	memcpy( OutBuf, buf, 16 );
#else
	memset( aes_key, 0, 16 );
	memset( aes_vector, 0, 16 );

	for(i=0;i<16;i++)
	{
		aes_key[i]=Key[15-i];
	}

	for(i=0;i<16;i++)
	{
		aes_vector[i]=vector[15-i];
	}

	for(i=0;i<16;i++)
	{
		buf[i]=InBuf[15-i];
	}

	aes_setkey_dec( &ctx, aes_key, 128 );
	aes_crypt_cbc( &ctx, 0, 16, aes_vector, buf, buf );

	for(i=0;i<16;i++)
	{
		OutBuf[i]=buf[15-i];
	}
#endif
}

void SwDesCbcEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv)
{
	des_context ctx;
	unsigned char i;
	unsigned char datalength=8;
	unsigned char buf[8];
	unsigned char des_key[8];
	unsigned char des_vector[8];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des_key, Key, datalength );
	memcpy( des_vector, iv, datalength );
	memcpy( buf, InBuf, datalength );

	des_setkey_enc( &ctx, (unsigned char *) des_key );
	des_crypt_cbc( &ctx, 1, datalength, des_vector, buf, buf );
	memcpy( OutBuf, buf, datalength );
#else
	for(i=0;i<datalength;i++)
	{
		des_key[i]=Key[datalength-i-1];
	}

	for(i=0;i<datalength;i++)
	{
		des_vector[i]=iv[datalength-i-1];
	}

	for(i=0;i<datalength;i++)
	{
		buf[i]=InBuf[datalength-i-1];
	}

	des_setkey_enc( &ctx, (unsigned char *) des_key );
	des_crypt_cbc( &ctx, 1, datalength, des_vector, buf, buf );

	for(i=0;i<datalength;i++)
	{
		OutBuf[i]=buf[datalength-i-1];
	}
#endif
}

void SwDesCbcDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv)
{
	des_context ctx;
	unsigned char i;
	unsigned char datalength=8;
	unsigned char buf[8];
	unsigned char des_key[8];
	unsigned char des_vector[8];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des_key, Key, datalength );
	memcpy( des_vector, iv, datalength );
	memcpy( buf, InBuf, datalength );

	des_setkey_dec( &ctx, (unsigned char *) des_key );
	des_crypt_cbc( &ctx, 0, datalength, des_vector, buf, buf );
	memcpy( OutBuf, buf, datalength );
#else
	for(i=0;i<datalength;i++)
	{
		des_key[i]=Key[datalength-i-1];
	}

	for(i=0;i<datalength;i++)
	{
		des_vector[i]=iv[datalength-i-1];
	}

	for(i=0;i<datalength;i++)
	{
		buf[i]=InBuf[datalength-i-1];
	}

	des_setkey_dec( &ctx, (unsigned char *) des_key );
	des_crypt_cbc( &ctx, 0, datalength, des_vector, buf, buf );

	for(i=0;i<datalength;i++)
	{
		OutBuf[i]=buf[datalength-i-1];
	}
#endif
}

void SwDesCfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv)
{
	des_context ctx;
	unsigned char buf[8];

	SwDesEcbEnc(iv, buf, Key);
	ExOr(buf,InBuf, OutBuf, 8);
}

void SwDesOfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv)
{
	des_context ctx;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des_key[8];
	unsigned char des_vector[8];

	SwDesEcbEnc(iv, buf, Key);
	ExOr(buf,InBuf, OutBuf, 8);
	memcpy( iv, buf, 8 );
}

void SwDesCtrEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Key, unsigned char *iv)
{
	des_context ctx;
	unsigned char i;
	unsigned char buf[8];
	unsigned int *ivPtr;

	SwDesEcbEnc(iv, buf, Key);
	ExOr(buf,InBuf, OutBuf, 8);

	for(i=0;i<8;i++)
	{
#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		iv[7-i]++;
		if(iv[7-i])	break;
#else
		iv[i]++;
		if(iv[i])	break;
#endif
    }
}

void SwDes3EcbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key)
{
	des3_context ctx3;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des3_key, Des3Key, 24 );	
	memcpy( buf, InBuf, 8 );

	des3_set3key_enc( &ctx3, (unsigned char *) des3_key );
	des3_crypt_ecb( &ctx3, buf, buf );

	memcpy( OutBuf, buf, 8 );
#else
	memset( des3_key, 0, 24 );

	for(i=0;i<8;i++)
	{
		des3_key[i]=Des3Key[7-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[8+i]=Des3Key[15-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[16+i]=Des3Key[23-i];
	}

	for(i=0;i<8;i++)
	{
		buf[i]=InBuf[7-i];
	}

	des3_set3key_enc( &ctx3, (unsigned char *) des3_key );
	des3_crypt_ecb( &ctx3, buf, buf );

	for(i=0;i<8;i++)
	{
		OutBuf[i]=buf[7-i];
	}
#endif
}

void SwDes3EcbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key)
{
	des3_context ctx3;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des3_key, Des3Key, 24 );	
	memcpy( buf, InBuf, 8 );

	des3_set3key_dec( &ctx3, (unsigned char *) des3_key );
	des3_crypt_ecb( &ctx3, buf, buf );

	memcpy( OutBuf, buf, 8 );
#else
	memset( des3_key, 0, 24 );

	for(i=0;i<8;i++)
	{
		des3_key[i]=Des3Key[7-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[8+i]=Des3Key[15-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[16+i]=Des3Key[23-i];
	}

	for(i=0;i<8;i++)
	{
		buf[i]=InBuf[7-i];
	}
	//memcpy( buf, InBuf, 8 );
	des3_set3key_dec( &ctx3, (unsigned char *) des3_key );
	des3_crypt_ecb( &ctx3, buf, buf );

	for(i=0;i<8;i++)
	{
		OutBuf[i]=buf[7-i];
	}
#endif
}

void SwDes3CbcEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	des3_context ctx3;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];
	unsigned char des3_vector[8];
	unsigned char datalength=8;

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des3_key, Des3Key, datalength*3 );
	memcpy( des3_vector, iv, datalength );
	memcpy( buf, InBuf, datalength );

	des3_set3key_enc( &ctx3, (unsigned char *) des3_key );
	des3_crypt_cbc( &ctx3, 1, datalength, des3_vector, buf, buf );

	memcpy( OutBuf, buf, 8 );
#else
	memset( des3_key, 0, 24 );

	//for(i=0;i<24;i++)
	for(i=0;i<8;i++)
	{
		des3_key[i]=Des3Key[7-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[8+i]=Des3Key[15-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[16+i]=Des3Key[23-i];
	}

	for(i=0;i<8;i++)
	{
		des3_vector[i]=iv[7-i];
	}

	for(i=0;i<8;i++)
	{
		buf[i]=InBuf[7-i];
	}

	des3_set3key_enc( &ctx3, (unsigned char *) des3_key );
	des3_crypt_cbc( &ctx3, 1, datalength, des3_vector, buf, buf );

	for(i=0;i<8;i++)
	{
		OutBuf[i]=buf[7-i];
	}
#endif
}

void SwDes3CbcDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	des3_context ctx3;
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];
	unsigned char des3_vector[24];
	unsigned char datalength=8;

#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
	memcpy( des3_key, Des3Key, datalength*3 );
	memcpy( des3_vector, iv, datalength );
	memcpy( buf, InBuf, datalength );

	des3_set3key_dec( &ctx3, (unsigned char *) des3_key );
	des3_crypt_cbc( &ctx3, 0, datalength, des3_vector, buf, buf );

	memcpy( OutBuf, buf, 8 );
#else
	memset( des3_key, 0, 24 );

	for(i=0;i<8;i++)
	{
		des3_key[i]=Des3Key[7-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[8+i]=Des3Key[15-i];
	}

	for(i=0;i<8;i++)
	{
		des3_key[16+i]=Des3Key[23-i];
	}

	for(i=0;i<8;i++)
	{
		des3_vector[i]=iv[7-i];
	}

	for(i=0;i<8;i++)
	{
		buf[i]=InBuf[7-i];
	}

	des3_set3key_dec( &ctx3, (unsigned char *) des3_key );
	des3_crypt_cbc( &ctx3, 0, datalength, des3_vector, buf, buf );

	for(i=0;i<8;i++)
	{
		OutBuf[i]=buf[7-i];
	}
#endif
}

void SwDes3CfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	unsigned char buf[8];

	SwDes3EcbEnc(iv, buf, Des3Key);
	ExOr(buf,InBuf, OutBuf, 8);
}

void SwDes3CfbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];

	SwDes3EcbEnc(iv, buf, Des3Key);
	ExOr(buf,InBuf, OutBuf, 8);
}

void SwDes3OfbEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	unsigned char i;
	unsigned char buf[8];

	SwDes3EcbEnc(iv, buf, Des3Key);
	ExOr(buf,InBuf, OutBuf, 8);
	memcpy( iv, buf, 8 );
}

void SwDes3OfbDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];

	SwDes3EcbEnc(iv, buf, Des3Key);
	ExOr(buf,InBuf, OutBuf, 8);
	memcpy( iv, buf, 8 );
}

void SwDes3CtrEnc(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	unsigned char buf[8];
	unsigned int *ivPtr;

	SwDes3EcbEnc(iv, buf, Des3Key);
	ExOr(buf,InBuf, OutBuf, 8);

	for(i=0;i<8;i++)
	{
#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		iv[7-i]++;
		if(iv[7-i])	break;
#else
		iv[i]++;
		if(iv[i])	break;
#endif
	}
}

void SwDes3CtrDec(unsigned char *InBuf, unsigned char *OutBuf, unsigned char *Des3Key, unsigned char *iv)
{
	unsigned char i;
	unsigned char buf[8];
	unsigned char des3_key[24];
	unsigned int *ivPtr;

	SwDes3EcbEnc(iv, buf, Des3Key);
	ExOr(buf,InBuf, OutBuf, 8);

	for(i=0;i<8;i++)
	{
#ifdef ENABLE_PATCH_DPU_DATA_ENDIAN_ISSUE
		iv[7-i]++;
		if(iv[7-i])	break;
#else
		iv[i]++;
		if(iv[i])	break;
#endif
	}
}

void SwEncryption(SW_ENC_INFO *SwEncInfo)
{
	MMP_UINT32 Reg32;
	MMP_UINT8  *SrcBuf=SwEncInfo->InBuf;
	MMP_UINT8  *DstBuf=SwEncInfo->OutBuf;
	MMP_UINT8  *TemIv[16];

	switch(SwEncInfo->DpuMode)
	{
		case AES_ECB_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
			{
				SwAesEcbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key);
				SrcBuf  +=16;
				DstBuf  +=16;
			}
			break;
		case AES_CBC_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
			{
				if(Reg32)
				{
					SwAesCbcEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(DstBuf-16) );
				}
				else
				{
					SwAesCbcEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
				}
				SrcBuf  +=16;
				DstBuf  +=16;
			}
			break;
		case AES_OFB_MODE:
			memcpy(TemIv, SwEncInfo->iv, 16);
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
			{
				SwAesOfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
				SrcBuf  +=16;
				DstBuf  +=16;
			}
			break;
		case AES_CFB_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
			{
				if(Reg32)
				{
					SwAesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(DstBuf-16) );
				}
				else
				{
					SwAesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
				}
				SrcBuf  +=16;
				DstBuf  +=16;
			}
			break;
		case AES_CTR_MODE:
			memcpy(TemIv, SwEncInfo->iv, 16);
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
			{
				SwAesCtrEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
				SrcBuf  +=16;
				DstBuf  +=16;
			}
			break;

		case DES_ECB_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				SwDesEcbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key);
				DstBuf  +=8;
				SrcBuf  +=8;
			}
			break;
		case DES_CBC_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				if(Reg32)
				{
					SwDesCbcEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)DstBuf-8);
				}
				else
				{
					SwDesCbcEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
				}
				DstBuf  +=8;
				SrcBuf  +=8;
			}
			break;
		case DES_OFB_MODE:
			memcpy(TemIv, SwEncInfo->iv, 8);
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				SwDesOfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
				DstBuf  +=8;
				SrcBuf  +=8;
			}
			break;
		case DES_CFB_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				if(Reg32)
				{
					SwDesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(DstBuf-8) );
				}
				else
				{
					SwDesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
				}
				SrcBuf  +=8;
				DstBuf  +=8;
			}
			break;
		case DES_CTR_MODE:
			memcpy(TemIv, SwEncInfo->iv, 8);
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
			    //ShowBufferData(TemIv, 8);
				SwDesCtrEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
				SrcBuf  +=8;
				DstBuf  +=8;
			}
			break;

		case DES3_ECB_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				SwDes3EcbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key);
				DstBuf  +=8;
				SrcBuf  +=8;
			}
			break;
		case DES3_CBC_MODE:
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				if(Reg32)
				{
					SwDes3CbcEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, SwEncInfo->Key, (unsigned char *)DstBuf-8);
				}
				else
				{
					SwDes3CbcEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
				}
				DstBuf  +=8;
				SrcBuf  +=8;
			}
			break;
		case DES3_OFB_MODE:
			memcpy(TemIv, SwEncInfo->iv, 8);
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				SwDes3OfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
				DstBuf  +=8;
				SrcBuf  +=8;
			}
			break;
		case DES3_CFB_MODE:
		   for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		   {
			   if(Reg32)
			   {
				   SwDes3CfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, ((unsigned char *)DstBuf-8) );
			   }
			   else
			   {
				   SwDes3CfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			   }
			   SrcBuf  +=8;
			   DstBuf  +=8;
		   }
		   break;
		case DES3_CTR_MODE:
			memcpy(TemIv, SwEncInfo->iv, 8);
			for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
			{
				SwDes3CtrEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
				SrcBuf  +=8;
				DstBuf  +=8;
			}
            break;

		case CSA_MODE:
		case CRC_MODE:
            SW_CRC_CALCULATE((MMP_UINT32*)SrcBuf, (MMP_UINT32*)DstBuf, SwEncInfo->SwEncSize);
			break;

		case SHA2_MODE:
            SwSha2Caculate((MMP_UINT32*)SrcBuf, (MMP_UINT32*)DstBuf, SwEncInfo->SwEncSize);
			break;

		default:
			//error
			//goto end;
			break;
	}
}

void SwDecryption(SW_ENC_INFO *SwEncInfo)
{
	MMP_UINT32 Reg32;
	MMP_UINT8  *SrcBuf=SwEncInfo->InBuf;
	MMP_UINT8  *DstBuf=SwEncInfo->OutBuf;
	MMP_UINT8  *TemIv[16];

	switch(SwEncInfo->DpuMode)
	{
	case AES_ECB_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
		{
			SwAesEcbDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key);
			SrcBuf  +=16;
			DstBuf  +=16;
		}
		break;
	case AES_CBC_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
		{
			if(Reg32)
			{
				SwAesCbcDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(SrcBuf-16) );
			}
			else
			{
				SwAesCbcDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			}

			DstBuf  +=16;
			SrcBuf  +=16;
		}
		break;
	case AES_OFB_MODE:
		memcpy(TemIv, SwEncInfo->iv, 16);
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
		{
			SwAesOfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
			DstBuf  +=16;
			SrcBuf  +=16;
		}
		break;
	case AES_CFB_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
		{
			if(Reg32)
			{
				SwAesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(SrcBuf-16) );
			}
			else
			{
				SwAesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			}
			DstBuf  +=16;
			SrcBuf  +=16;
		}
		break;
	case AES_CTR_MODE:
		memcpy(TemIv, SwEncInfo->iv, 16);
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/16);Reg32++)
		{
			SwAesCtrEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
			DstBuf  +=16;
			SrcBuf  +=16;
		}
		break;

	case DES_ECB_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			SwDesEcbDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key);
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES_CBC_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			if(Reg32)
			{
				SwDesCbcDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(SrcBuf-8) );
			}
			else
			{
				SwDesCbcDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			}

			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES_OFB_MODE:
		memcpy(TemIv, SwEncInfo->iv, 8);
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			SwDesOfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES_CFB_MODE://HW has bug in this mode?
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			if(Reg32)
			{
				SwDesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(SrcBuf-8) );
			}
			else
			{
				SwDesCfbEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			}
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES_CTR_MODE:
		memcpy(TemIv, SwEncInfo->iv, 8);
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			SwDesCtrEnc((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;

	case DES3_ECB_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			SwDes3EcbDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key);
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES3_CBC_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			if(Reg32)
			{
				SwDes3CbcDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(SrcBuf-8) );
			}
			else
			{
				SwDes3CbcDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			}

			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES3_OFB_MODE:
		memcpy(TemIv, SwEncInfo->iv, 8);
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			SwDes3OfbDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES3_CFB_MODE:
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			if(Reg32)
			{
				SwDes3CfbDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)(SrcBuf-8) );
			}
			else
			{
				SwDes3CfbDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)SwEncInfo->iv);
			}
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;
	case DES3_CTR_MODE:
		memcpy(TemIv, SwEncInfo->iv, 8);
		for(Reg32=0;Reg32<(SwEncInfo->SwEncSize/8);Reg32++)
		{
			SwDes3CtrDec((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, (unsigned char *)TemIv);
			DstBuf  +=8;
			SrcBuf  +=8;
		}
		break;

	case CSA_MODE:
		CsaDecodeBySoftware((unsigned char *)SrcBuf, (unsigned char *)DstBuf, (unsigned char *)SwEncInfo->Key, SwEncInfo->SwEncSize);
        break;

	case CRC_MODE:
		break;

	case SHA2_MODE:
        SwSha2Caculate((MMP_UINT32*)SrcBuf, (MMP_UINT32*)DstBuf, SwEncInfo->SwEncSize);
		break;

	default:
		//error
		//goto end;
		break;
	}
}

#if defined(MM9910)
void LoadDpuDrvToMem(void)
{
	FILE *fin   = NULL;
	MMP_UINT8  *DrvData;
	MMP_UINT8  *MemPt=0;
	MMP_UINT8	GenResult = MMP_TRUE;
	MMP_UINT32	Reg32=0;
	MMP_UINT32	BinFileSize=0;
	MMP_UINT32	LoopCounter=0;
	MMP_UINT32	DataSize  = 16*1024;    //16kB
	//char* dpu_drv[] = {"J:\\DTV_Caster3_Joseph\MM9910_DpuDrv\security_1\security.bin"};
	char* dpu_drv[] = {"J:\\security.bin"};

    //open and read bin file
    DrvData = (MMP_UINT8 *)SYS_Malloc(DataSize);
	if(!DrvData)
	{
		printf("[ERROR], sys_alloc() of DrvData error!!\n");
	}
	#if defined(WIN32)
	//mmpLoadAhbCode();
	#endif

	//HOST_WriteBlockMemory((MMP_UINT32)MemPt, (MMP_UINT32)DPU_AHB_CODE, 0x16D4);	


	//OPEN pkg file
	if ((fin = fopen(dpu_drv[0], "rb")) == NULL) 
	{
		printf("Open %s fails\n", dpu_drv[0]);
		GenResult = MMP_FALSE;
	}
	else
	{
		fseek(fin, 0, SEEK_END);
		BinFileSize = (unsigned long)ftell(fin);
		if(BinFileSize>DataSize)
		{
		    printf("[ERROR], file size is larger than allocated size!!\n");		    
		}
		else
		{
		    fseek(fin, 0, SEEK_SET);
		    LoopCounter = (MMP_UINT32)fread(DrvData, sizeof(char), BinFileSize , fin);
		    if(LoopCounter==BinFileSize)
		    {
		        HOST_WriteBlockMemory((MMP_UINT32)MemPt, (MMP_UINT32)DrvData, LoopCounter);	    
		    }
		    else
		    {
		        printf("[ERROR], Read length of bin file is less than previous one!!\n");
		    }
		}
}
	//close pkg file
	fclose(fin);
    
    //FireRisc3On()
    //AHB_WriteRegisterMask(0x168C, 0x00000001,0x00000001);	//Fire Risc1
    //AHB_WriteRegisterMask(0x16CA, 0x00000001,0x00000001);	//Fire Risc2
	AHB_WriteRegisterMask(0x170C, 0x00000001,0x00000001);	//Fire Risc3

	AHB_ReadRegister(0x1716, &Reg32);	//The Program Counter of RISC3
	while(Reg32!=0x1A0)
	{
		AHB_ReadRegister(0x1716, &Reg32);	//reset DPU controller
}

	AHB_ReadRegister(DPU_SRC_ADDR_REG, &Reg32);	//reset DPU controller
	if(Reg32==0x0000)
	{
		AHB_WriteRegister(DPU_SRC_ADDR_REG, 0x12345678);	//reset DPU controller
		AHB_ReadRegister(DPU_SRC_ADDR_REG, &Reg32);	//reset DPU controller
		if(Reg32!=0x12345678)
		{
			printf("[DPU ERROR3], initial DPU drver FAIL!!\n");
			while(1);
		}
		else
		{
			printf("[DPU], initial DPU drver success!!\n");
		}
	}
    
}

MMP_RESULT JediAhbModeTest(void)
{
    MMP_UINT8  result=MMP_TRUE;
    MMP_UINT32 Reg32;
    MMP_UINT32 TemReg32;
    MMP_UINT32 DPU_AHB_REG_BASE = 0xD0900000;
    MMP_UINT32 DPU_WBB_REG_BASE = 0xE0000000;
    MMP_UINT32 DPU_AHB_REG_SRC_ADDR = DPU_AHB_REG_BASE+04;
    
    printf("[DPU.02]\n");    
    
	AHB_ReadRegister(DPU_AHB_REG_SRC_ADDR, &Reg32);	//
	if(Reg32==0x0000)
	{
		AHB_WriteRegister(DPU_AHB_REG_SRC_ADDR, 0x12345678);	//
		AHB_ReadRegister(DPU_AHB_REG_SRC_ADDR, &Reg32);	//
		if(Reg32!=0x12345678)
		{
			printf("[DPU ERROR1], initial DPU drver FAIL!!\n");
			result = MMP_FALSE;
		}
		else
		{
			printf("[DPU], initial DPU drver success!!\n");
		}
	}
	else
	{
	    AHB_WriteRegister(DPU_AHB_REG_SRC_ADDR, 0x00000000);	//
	    AHB_ReadRegister(DPU_AHB_REG_SRC_ADDR, &Reg32);	//
		if(Reg32==0x00000000)
		{
			printf("[DPU], initial DPU drver success!!\n");		    

		}
		else
		{
			printf("[DPU ERROR2], initial DPU drver FAIL!!\n");
			result = MMP_FALSE;
		}
	}
	
	printf("End of DPU AHB mode Test\n");
	return result;
}

void FireRisc3AndSwitch2WbMode(void)
{
	MMP_UINT32	TestCount=1;
	MMP_RESULT  result;
	MMP_UINT32  Reg32;
	//MMP_UINT32  DPU_WBB_ADDR = 0xE0000000;
	MMP_UINT32  DPU_AHB_ADDR = 0xD0900000;
	//MMP_UINT32  DPU_REG_SRC_ADDR = DPU_WBB_ADDR + 4;
	MMP_UINT32  DPU_REG_SRC_ADDR = DPU_AHB_ADDR + 4;
	
	//PalEnablePrintBuffer(MMP_TRUE,4*1024); 
    printf("[DPU.01]\n");
    //HOST_WriteRegister(0x1698, 0x0000);
    
    //JediSecurityModeTest();
    //JediAhbModeTest();
    if(JediAhbModeTest()==MMP_TRUE)
    {
        MMP_UINT16  Reg16;
        
        HOST_WriteRegisterMask(0x170C, 0x0001,0x0001);	//Fire Risc3
		
		HOST_ReadRegister(0x170C, &Reg16);  //check fire bit of RISC3
		printf("[DPU3], fire RISC3 success,[%x]!!\n",Reg16);
		
		HOST_ReadRegister(0x1716, &Reg16);  //check PC of RISC3
		printf("[DPU3], fire RISC3 success2,[%x]!!\n",Reg16);
		
		//wait for 0x1698 ==0x5A5A
	    HOST_ReadRegister(0x1698, &Reg16);	//
	
	    while(Reg16!=0x5A5A)
	    {
	        HOST_ReadRegister(0x1698, &Reg16);
	    }
	    printf("Risc1 Got reg[1698]=0x5A5A(means Risc3 has been ready.)\n");
		
		//check AHB bus of DPU register
		if(JediAhbModeTest()==MMP_TRUE)
		{
		    printf("[Check ERROR]: it must be fail in Wish Bone mode");
		}
		else
		{
		    printf("[Check CORRECT]: it must be fail in Wish Bone mode");
		}
    }  
}

void Wait_Risc1_Ready(void)
{
    MMP_UINT16  Reg16;
	//wait for 0x1698 ==0x01A0;
	HOST_ReadRegister(0x1698, &Reg16);	//
	
	while(Reg16!=0x01A0)
	{
	    HOST_ReadRegister(0x1698, &Reg16);
}
	printf("Risc3 Got reg[1698]=0x01A0(means Risc1 has been ready.)\n");
}

void Wait_Risc3_Ready(void)
{
    MMP_UINT16  Reg16;
	//wait for 0x1698 ==0x5A5A;
	HOST_ReadRegister(0x1698, &Reg16);	//
	
	while(Reg16!=0x5A5A)
	{
	    HOST_ReadRegister(0x1698, &Reg16);
	}
	printf("Risc3 Got reg[1698]=0x5A5A(means Risc3 has been ready.)\n");
}
#endif

#endif	//ENABLE_CSA_SW_DECODE
