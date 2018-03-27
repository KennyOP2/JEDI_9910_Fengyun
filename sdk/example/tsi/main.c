/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef WIN32
#include <io.h>
#endif
#include "pal/pal.h"
#include "host/host.h"

#include "mmp_tsi.h"
#include "sys/sys.h"
#include "../../../core/demod_control.h"
#include "mem/mem.h"

/****************************************************************************/
/*                          MACRO for testing                               */
/****************************************************************************/
#define   ENABLE_TSI_IRQ_FUNCTION

#define   TSI_SELECT        0
#define   TEST_TSI_BASE     0x00

/****************************************************************************/
/*                          MACRO                                           */
/****************************************************************************/
#define   MAX_READ_SIZE   0x7FFFF0 //0x1FFF FF0//0x1FFFFF8//0x3FFFE8//0x1FFFFF8//0x7FFFFC//0x3FFFFC


#define   TTV_PID_4001  0x0FA1
#define   TTV_PID_4002  0x0FA2
#define   TTV_PID_4003  0x0FA3
#define   TTV_PID_4011  0x0FAB
#define   TTV_PID_4012  0x0FAC
#define   TTV_PID_4021  0x0FB5
#define   TTV_PID_4022  0x0FB6

#define   CTV_PID_1001  0x03E9
#define   CTV_PID_1002  0x03EA
#define   CTV_PID_1003  0x03EB
#define   CTV_PID_1011  0x03F3
#define   CTV_PID_1012  0x03F4
#define   CTV_PID_1021  0x03FD
#define   CTV_PID_1022  0x03FE

#define   CTS_PID_5011  0x1393

//static HWND hWnd;
//static UINT_PTR timer;

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

static void
_Verification(
    void);

static void
_Check_INT_PID_PCR(
    void);

static void
_Check_INT_PCR_CNT(
    void);

static void SearchLastPid(MMP_UINT8 *SearchBuffer,MMP_UINT32 TotalSize,MMP_UINT32 *LastFrameAddress );
static void SearchFirstPidAfterErr(MMP_UINT8 *SearchBuffer, MMP_UINT32 TotalSize, MMP_UINT32 *SndAddress);

static MMP_INT
Initialize(
    void)
{
    //WNDCLASS wc;
    MMP_BOOL result;
    MMP_UINT32 error = 0;

end:
    return error;
}

static MMP_INT
Terminate(
    void)
{
    MMP_INT result = 0;

    return result;
}

static MMP_INT
MainLoop(
    void)
{
    return 1;
}

void DoTest(void)
{
    MMP_INT result;

    //result = Initialize();

    //if (result)
        //goto testend;

    //_Verification();  //seems here has already fail? maybe the default setting has been changed
    //_Check_INT_PID_PCR();
    _Check_INT_PCR_CNT();

}

#if defined(__FREERTOS__)
int  main(int argc, char** argv)
{
    signed portBASE_TYPE ret = pdFAIL;

    ret = xTaskCreate(main_task_func, "tsitest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();

    DoTest();
}

portTASK_FUNCTION(main_task_func, params)
{
    DoTest();
}
#else
int
main(
    void)
{
    MMP_INT result;

    result = Initialize();
    if (result)
        goto end;

    //_Verification();  //seems here has already fail? maybe the default setting has been changed
    _Check_INT_PID_PCR();
    //_Check_INT_PCR_CNT();

    result = MainLoop();
    if (result)
        goto end;

    result = Terminate();
    if (result)
        goto end;

end:
    // Debug memory leaks
    //_CrtDumpMemoryLeaks();

    return result;
}
#endif


MMP_WCHAR*
smtkItow(
    MMP_UINT16 value,
    MMP_WCHAR* string)
{
    MMP_INT i, d;
    MMP_INT flag = 0;
    MMP_WCHAR* ptr = string;

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;

        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = PAL_T('-');

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 100000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (MMP_WCHAR)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;
}

static void
_Verification(
    void)
{

    MMP_UINT16  index = 0;
    MMP_UINT8*  sysBuf_0  = MMP_NULL;
    MMP_UINT8*  sysBuf_1  = MMP_NULL;
    MMP_UINT16  TestTsiBase=TEST_TSI_BASE;

    printf("TSI init..\n");
    mmpTsiInitialize(TSI_SELECT);   //Tsi0 base address = 0x1000
    //mmpTsiInitialize(1);          //Tsi0 base address = 0x1000
    //HOST_WriteRegister(0x102A, 0x0400); //falling sample ?

    for (index = 0; index < 1; index++)
    {
        FILE*       pfile = NULL;
        FILE*       pfile2 = NULL;
        FILE*       pfileTsi2 = NULL;
        MMP_RESULT  result;
        MMP_RESULT  result_0;
        MMP_RESULT  result_1;
        MMP_UINT32  size_0 = 0;
        MMP_UINT32  size_1 = 0;
        MMP_UINT32  totalReadSize_0 = 0;
        MMP_UINT32  totalReadSize_1 = 0;
        MMP_UINT16  Reg16,Wpt_L,Wpt_H;
        MMP_UINT16  PCR_H,PCR_L;
        MMP_UINT32  PCR1,PCR2,PCR3;
        MMP_UINT32  Address32=0,SndAddress=0;
        MMP_UINT32  i;
        MMP_UINT32  Timmer=0;


        char pszBuffer[256] = "";
        char pszBuffer2[256] = "";

        //set PCR INT enable
        {
            HOST_WriteRegisterMask(TestTsiBase+0x100C, 0x0000,0x0004);
            MMP_Sleep(1);
            HOST_ReadRegister(TestTsiBase+0x100C,&Reg16);
            printf("Set PCR INT enable,0x100C=[%x].\n",Reg16);

            HOST_ReadRegister(TestTsiBase+0x1022,&Reg16);
            printf("sync byte & PCR setting,0x1022=[%x].\n",Reg16);
        }

        //check PID of TTV
        while(0)
        {
            HOST_ReadRegister(0x1018+TestTsiBase,&Reg16);
            HOST_ReadRegister(0x1016+TestTsiBase,&Reg16);
            Reg16&=0x1FFF;

            if( (Reg16!=4001) || (Reg16!=4011) || (Reg16!=4021) )
            {
                break;
            }
        }

        //set 32 sets of PID filter
        {
            HOST_WriteRegister(0x102C+TestTsiBase, (TTV_PID_4001<<1)|0x0001 );
            //HOST_WriteRegister(0x102E, (TTV_PID_4011<<1)|0x0001 );
            //HOST_WriteRegister(0x1030, (TTV_PID_4021<<1)|0x0001 );
            //HOST_WriteRegister(0x1032, (4042<<1)|0x0001 );
            //HOST_WriteRegister(0x102C, (256<<1)|0x0001 ); //only recieve the frame with PID=256
            /*
            for(i=0;i<32;i++)
            {
                HOST_WriteRegister(0x102C+((MMP_UINT16)i*2), ((2001+(MMP_UINT16)i)<<1)|0x0001 );
            }
            */
            //HOST_WriteRegister( 0x102C+((MMP_UINT16)(index%32)*2), ((TTV_PID_4001)<<1)|0x0001 );
            //HOST_WriteRegister( 0x102C, (TTV_PID_4001<<1)|0x0001 );
            //HOST_WriteRegister( 0x102C, (256<<1)|0x0001 );
            //HOST_WriteRegister( 0x102C+0x12, (256<<1)|0x0001 );
            //HOST_WriteRegister( 0x102C+0x08, (CTV_PID_1001<<1)|0x0001 );
            //HOST_WriteRegister( 0x102C+0x12, (CTS_PID_5011<<1)|0x0001 );
            /*
            for(i=0;i<32;i++)
            {
                HOST_ReadRegister(0x102C+((MMP_UINT16)i*2), &Wpt_H);
                printf("Reg[0x%04x]=0x%04x\n",0x102C+((MMP_UINT16)i*2), Wpt_H);
            }
            */
        }

        //set TSI1 register
        #ifdef  ENABLE_DUAL_TSI_RECORD_TEST
            //HOST_WriteRegister(0x1080, 0x0000 );
            //HOST_WriteRegister(0x1082, 0x0080 );
            //HOST_WriteRegister(0x1084, 0xfff8 );
            //HOST_WriteRegister(0x1086, 0x006f );
              //HOST_WriteRegister(0x108e, 0x1f43 ); //set PID int of TSI1
            //HOST_WriteRegister(0x10A2, 0x4706 );
              //HOST_WriteRegister( 0x10AC+0x08, (TTV_PID_4001<<1)|0x0001 ); //set TSI1 PID filter 4001

            //HOST_WriteRegister(0x0006,0x0060);    //??
            //HOST_ReadRegister(0x0006,&Wpt_H);
            //HOST_ReadRegister(0x100C,&Wpt_L);

            //mmpTsiEnable(1);
            //HOST_WriteRegister(0x10A8, (27 << 2));        //27M

            //HOST_WriteRegister(0x108c, 0x0041 );          //Enable TSI1
            //HOST_WriteRegister(0x102A, 0x0400);
        #eniif

        if(index)
        {
            //set read pointer
            //_TSI_SetReadPtr(TSI_SELECT, 0x10);

            //HOST_WriteRegister(0x1008, 0x0000);
            //HOST_WriteRegister(0x100A, 0x00f0);

            //HOST_ReadRegister(TestTsiBase+0x1012,&Wpt_L);
            //HOST_ReadRegister(TestTsiBase+0x1014,&Wpt_H);
        }

        printf("TSI[%u] enable..\n",TSI_SELECT);
        mmpTsiEnable(TSI_SELECT);

        //HOST_ReadRegister(0x1012+TestTsiBase,&Wpt_L);
        //HOST_ReadRegister(0x1014+TestTsiBase,&Wpt_H);

        if(index)
        {
            MMP_ULONG   ReadSize;
            MMP_RESULT  ret;

            HOST_WriteRegister(TestTsiBase+0x1008, 0x0008);
            HOST_WriteRegister(TestTsiBase+0x100A, 0x0000);
            while(1)
            {
                HOST_ReadRegister(0x1012+TestTsiBase,&Wpt_L);
                HOST_ReadRegister(0x1014+TestTsiBase,&Wpt_H);
                if( (Wpt_H<=0x0000) && (Wpt_L<=0x00F0) ) {  break;  }
            }

            #ifdef WIN32
            ret = mmpTsiReceive(TSI_SELECT,
                (void*)(sysBuf_0),
                16,
                &ReadSize);
            #else
            {
                MMP_UINT8 *CurrSysBuf;

                printf("Recieve 16 bytes!!\n");

                CurrSysBuf = (MMP_UINT8 *)(sysBuf_0 + totalReadSize_0);
                size_0 = 16;

                result_0 = mmpTsiReceive(TSI_SELECT, &CurrSysBuf, &size_0);
            }
            #endif

            printf("read size = %x.\n",ReadSize);
            HOST_WriteRegister(0x1008+TestTsiBase, 0x0000);
            HOST_WriteRegister(0x100A+TestTsiBase, 0x0000);
        }

        HOST_ReadRegister(TestTsiBase+0x1012,&Wpt_L);
        HOST_ReadRegister(TestTsiBase+0x1014,&Wpt_H);
        printf("idx=%u,Wpt_H=%x,Wpt_L=%x\n",index,Wpt_H,Wpt_L);

        //HOST_WriteRegister(0x1008+TestTsiBase, 0x0000);
        //HOST_WriteRegister(0x100A+TestTsiBase, 0x0000);
        //HOST_WriteRegister(0x0006,0x0060);
        //HOST_ReadRegister(0x0006,&Wpt_H);
        //HOST_ReadRegister(0x100C+TestTsiBase,&Wpt_L);

        //polling TSI INT -->2/18 change to check the register 0x1018[3]
        while(0)
        {
            HOST_ReadRegister( TestTsiBase + 0x101E, &Reg16);
            if(Reg16&0x0010)    {   break;  }
        }
        //printf("v.polling TSI INT..got it!!\n");

        //check PID & PCR
        HOST_ReadRegister(TestTsiBase+0x1018,&Reg16);
        printf("PCR=[%x]\n",Reg16);

        HOST_ReadRegister(TestTsiBase+0x1016,&Reg16);
        printf("PID=[%x]\n",Reg16);

        HOST_ReadRegister(TestTsiBase+0x1000,&Reg16);
        printf("BaseAddrL=[%x]\n",Reg16);

        HOST_ReadRegister(TestTsiBase+0x1002,&Reg16);
        printf("BaseAddrH=[%x]\n",Reg16);

        HOST_ReadRegister(TestTsiBase+0x1012,&Wpt_L);
        HOST_ReadRegister(TestTsiBase+0x1014,&Wpt_H);
        printf("3.Wpt=%x,[%x,%x].\n",((MMP_UINT32)Wpt_H<<16)+Wpt_L,Wpt_H,Wpt_L);

        printf("alloc mem..\n");
        if(sysBuf_0==MMP_NULL)
        {
            #ifdef WIN32
            sysBuf_0 = (MMP_UINT8*)SYS_Malloc(MAX_READ_SIZE);
            #else
            //sysBuf_0 = (MMP_UINT8*)malloc(MAX_READ_SIZE);
            sysBuf_0 = (MMP_UINT8*)MEM_Allocate(MAX_READ_SIZE, MEM_USER_TSI);
            //sysBuf_0 = (MMP_UINT8*)MEM_Allocate(0x7FFFF8, MEM_USER_TSI);
            #endif
        }
        //sysBuf = (MMP_UINT8*)MEM_Allocate(MAX_READ_SIZE, MEM_USER_TSI);

        sprintf(pszBuffer, "C:\\tsi_%d.ts", index);
        printf("[0]alloc mem..OK\n");

        #ifdef WIN32
        pfile  = fopen(pszBuffer, "wb");

        if(pfile)
        {
            printf("open file suc :%s\n",pszBuffer);
        }

        if(sysBuf_0)
        {
            printf("sysBuf_0=%x.\n",sysBuf_0);
        }

        if(sysBuf_1==MMP_NULL)
        {
            sysBuf_1 = (MMP_UINT8*)SYS_Malloc(MAX_READ_SIZE);
        }
        if(sysBuf_1)
        {
            printf("sysBuf_1=%x.\n",sysBuf_1);
        }
/*
        sprintf(pszBuffer, "C:\\tsix_%d.ts", index);
        pfileTsi2 = fopen(pszBuffer, "wb");
        if(pfileTsi2)
        {
            printf("open pfileTsi2 suc :%s\n",pszBuffer);
        }
*/
        #else
        if(sysBuf_0)
        {
            printf("sysBuf_0=%x.\n",sysBuf_0);
        }
        else
        {
            printf("Err, sysBuf_0=%x.\n",sysBuf_0);
            while(1);
        }
        if(sysBuf_1==MMP_NULL)
        {
            sysBuf_1 = (MMP_UINT8*)malloc(MAX_READ_SIZE);
        }
        if(sysBuf_1)
        {
            printf("sysBuf_1=%x.\n",sysBuf_1);
        }
        #endif

        Wpt_L = 0x00;

        {
            MMP_UINT32 RegLoop;
            MMP_UINT16 wReg16;

            for(RegLoop=0;RegLoop<0x10;RegLoop++)
            {
                HOST_ReadRegister(0x1000+TestTsiBase+RegLoop*2,&wReg16);
                printf("Reg[%04x]=[%04x]\n",0x1000+TestTsiBase+RegLoop*2,wReg16);
            }
        }


        //waiting for write-pointer stop
        while(0)
        {
            MMP_UINT16 REG16;
            MMP_UINT16 OccurIndex=0;

            HOST_ReadRegister(0x1012,&Wpt_L);
            HOST_ReadRegister(0x1014,&Wpt_H);
            HOST_ReadRegister(0x101E,&REG16);

            if( REG16&0x8000 )
            {
                if(!OccurIndex)
                {
                    printf("[ERROR]non-188 case happened,WtPtr=[%x,%x]\n",Wpt_H,Wpt_L);
                    OccurIndex=1;
                }
            }
            else
            {
                OccurIndex=0;
            }
            //if( (Wpt_L>=0xFF00) && (Wpt_H>=0x00FF) ) {    break;  }
            if( (Wpt_L>=0xFFF0) ) { break;  }
        }

        {
            MMP_UINT32 RegLoop;
            MMP_UINT16 wReg16;

            for(RegLoop=0;RegLoop<0x30;RegLoop++)
            {
                HOST_ReadRegister(0x1000+TestTsiBase+RegLoop*2,&wReg16);
                printf("Reg[%04x]=[%04x]\n",0x1000+TestTsiBase+RegLoop*2,wReg16);
            }
        }
        //printf("STOP DTV AP!!\n");
        //while(1);

        //set FIFOrequest acc size
        //HOST_WriteRegisterMask(0x100c,0x1C00,0x1F00);//Enable bit and size=7(8)
        //HOST_ReadRegister(0x100C,&Wpt_L);

        //HOST_WriteRegister(0x102A, 0x0400); //falling sample

        while(1)
        {
            MMP_UINT32 RegLoop;
            MMP_UINT16 wReg16=0;

            while( (wReg16&0x0018)!=0x0018 )
            {
                HOST_ReadRegister(0x101E,&wReg16);//need to check this reg, too
                if( (wReg16&0x8000) == 0x8000 )
                    printf("[TSI][error][non-188]\n");
                if( (wReg16&0x4000) == 0x4000 )
                    printf("[TSI][error][fifo-full]\n");
                //printf("wait interrupt,reg15:= 0x%x\n",Reg_15);
                break;
            }
            break;
        }


        do
        {
            #ifdef WIN32
            result_0 = mmpTsiReceive(TSI_SELECT,
                (void*)(sysBuf_0 + totalReadSize_0),
                MAX_READ_SIZE - totalReadSize_0,
                &size_0);
            #else
            MMP_UINT8 *CurrBuf=MMP_NULL;
            MMP_UINT32 k;

            //size_0 = MAX_READ_SIZE - totalReadSize_0;
            result_0 = mmpTsiReceive(TSI_SELECT, &CurrBuf, &size_0);
            if(totalReadSize_0+size_0<=MAX_READ_SIZE)
            {
                //ithInvalidateDCacheRange(CurrBuf, size_0);
                memcpy( &sysBuf_0[totalReadSize_0], CurrBuf, size_0);
            }
            else
            {
                size_0 = MAX_READ_SIZE - totalReadSize_0;
                //ithInvalidateDCacheRange(CurrBuf, size_0);
                memcpy( &sysBuf_0[totalReadSize_0], CurrBuf, size_0);
                printf("last data,size=[%x,%x]!!\n",totalReadSize_0,size_0);
            }
            /*
            printf("2.CurrSysBuf=[%x,%x,%x][%x,%x]\n",CurrBuf,sysBuf_0,&sysBuf_0[totalReadSize_0],totalReadSize_0,size_0);
            for(k=0;k<16;k++)
            {
                printf("%02X ",sysBuf_0[totalReadSize_0+k]);
                if( (k&0x0F)==0x0F )    printf("\n");
            }

            printf("3.TsiBuf=%x\n",CurrBuf);
            for(k=0;k<16;k++)
            {
                printf("%02X ",CurrBuf[k]);
                if( (k&0x0F)==0x0F )    printf("\n");
            }
            */
            #endif

/*
            #ifdef WIN32
            result_1 = mmpTsiReceive(1,
                (void*)(sysBuf_1 + totalReadSize_1),
                MAX_READ_SIZE - totalReadSize_1,
                &size_1);
            #else
            size_1 = MAX_READ_SIZE - totalReadSize_1;
            result_1 = mmpTsiReceive(1, &sysBuf_1, &size_1);
            #endif
*/
            //printf("result:[0x%x, 0x%x]\n",result_0, result_1);
            //printf("totalReadSize:[0x%x, 0x%x]\n",totalReadSize_0, totalReadSize_1);
            if (result_0 == MMP_SUCCESS && size_0 > 0)
            {
                totalReadSize_0 += size_0;
                printf("receive 0x%x bytes\n", size_0);
            }
            /*
            if (result_1 == MMP_SUCCESS && size_1 > 0)
            {
                totalReadSize_1 += size_1;
                printf("receive 0x%x bytes\n", size_1);
            }
            */

        //} while( (totalReadSize_0 < MAX_READ_SIZE) || (totalReadSize_1 < MAX_READ_SIZE) );
        } while( (totalReadSize_0 < MAX_READ_SIZE) );

        //SearchLastPid( sysBuf, totalReadSize, &Address32);

        if(Address32)
        {
            SearchFirstPidAfterErr(&sysBuf_0[Address32], totalReadSize_0-Address32, &SndAddress);
        }

        #ifdef WIN32
        fwrite(sysBuf_0, totalReadSize_0, 1, pfile);
        fclose(pfile);


        if(Address32 && SndAddress)
        {
            sprintf(pszBuffer2, "C:\\tsi_%da.ts", index);
            pfile2  = fopen(pszBuffer2, "wb");
            if(pfile2)
            {
                printf("open file suc :%s\n",pszBuffer2);
            }
            fwrite(&sysBuf_0[Address32+SndAddress], totalReadSize_0-Address32-SndAddress, 1, pfile2);
            fclose(pfile2);
        }
        #endif

        //fwrite(sysBuf_1, totalReadSize_1, 1, pfileTsi2);
        //fclose(pfileTsi2);



        printf("END of TSI save data...\n");
        while(1);
        Timmer++;
        if(Timmer>=1)
        {
            MMP_UINT16 rReg16;
            //for checking TSI reset bug only
            Timmer=0;
            while(1)
            {
                MMP_UINT16 wReg16;

                Timmer++;

                HOST_ReadRegister(0x101E,&wReg16);

                if( (wReg16&0x0002)==0x0000 )
                {
                    printf("Wait not Idle, Timmer=%d!!\n",Timmer);
                    break;
                }
            }

            //1.disable TSI controller
            //HOST_WriteRegisterMask(0x100C, 0x0000,0x0001);
            mmpTsiDisable(TSI_SELECT);

            //2.check TSI idle bit
            Timmer=0;

            while(1)
            {
                MMP_UINT16 wReg16;

                Timmer++;
                HOST_ReadRegister(0x100C, &wReg16);
                HOST_ReadRegister(0x101E, &wReg16);

                if(wReg16 & 0x0002)
                {
                    printf("Wait Idle, Timmer=%d!!\n",Timmer);
                    break;
                }
            }

            HOST_ReadRegister(0x1012+TestTsiBase, &rReg16);
            HOST_ReadRegister(0x1014+TestTsiBase, &rReg16);

            //3.enable TSI controller again
            //HOST_WriteRegisterMask(0x100C, 0x0001,0x0001);
            //mmpTsiEnable(TSI_SELECT);

            //4.check if PID PCR are still normal
            Timmer=0;
        }

        //mmpTsiDisable(TSI_SELECT);
    }
    SYS_Free(sysBuf_0);
    SYS_Free(sysBuf_1);

    mmpTsiTerminate(TSI_SELECT);
    printf("END of TSItest...\n");
    while(1);
}

static void
_Check_INT_PID_PCR(
    void)
{
    MMP_UINT16  Reg_6,Reg_15;
    MMP_UINT16  TV_PID;
    MMP_UINT16  PID,PCR1,PCR2,PCR3,PCR_H=0;
    MMP_UINT32  PCR,Last_PCR[8];
    MMP_UINT32  Counter=1000000;

    MMP_RESULT  result;
    MMP_UINT8*  sysBuf  = MMP_NULL;
    MMP_UINT32  size = 0;
    MMP_UINT32  totalReadSize = 0;
    MMP_UINT16  PcrCnt_L,PcrCnt_H;
    MMP_UINT32  PcrCnt;
    MMP_UINT32  PCR_Diff;
    MMP_UINT32  Timmer=0;
    MMP_UINT8   TsiSelect;
    MMP_UINT16  TmpTsiBase;
    int i=0; int t;

    printf("_Check_INT_PID_PCR.in\n");

    //initial FIQ of TSI
    printf("initial FIQ of TSI\n");
    //HOST_WriteRegisterMask(0x8400+0x010E, 0x0000, 0x0030);    //Tsi0
    //HOST_WriteRegisterMask(0x8400+0x0112, 0x0000, 0x0030);    //Tsi1
    //HOST_WriteRegisterMask(0x8400+0x0106, 0x0030, 0x0030);    //enable FIQ
    TsiSelect=0;

    if(TsiSelect)
    {
        TmpTsiBase=0x80;
    }
    else
    {
        TmpTsiBase=0x00;
    }

    printf("mmpTsiInitialize\n");
    mmpTsiInitialize(TsiSelect);

    printf("set PCLK enable, for temp\n");
    HOST_WriteRegisterMask(0x0042, 0x0001, 0x0001); //set PCLK enable, for temp

    //enable PCR INT
    printf("enable PCR INT\n");
    HOST_WriteRegisterMask(TmpTsiBase+0x100C, 0x0000, 0x0004);

    //disable the mem size threshold INT()
    HOST_WriteRegisterMask(TmpTsiBase+0x100C, 0x0002, 0x0002);

    HOST_ReadRegister(TmpTsiBase+0x100C,&Reg_6);
    printf("Set PCR INT enable,0x100C=[%x].\n",Reg_6);

    //set PID intr value, 4001(0x1F43=0x0FA1<<1 + 1)
    TV_PID = 4001;
    //TV_PID = 256;
    HOST_WriteRegister( (TmpTsiBase + 0x100E), TV_PID<<1 | 1); //set PID intr value

    printf("alloc mem..\n");
    sysBuf = (MMP_UINT8*)SYS_Malloc(MAX_READ_SIZE);//SYS_Malloc() is for PC memory allocation
    //sysBuf = (MMP_UINT8*)MEM_Allocate(MAX_READ_SIZE, MEM_USER_TSI);
    //sprintf(pszBuffer, "C:\\tsi_%d.ts", index);
    printf("[1]alloc mem..OK\n");

    mmpTsiEnable(TsiSelect);
    //HOST_WriteRegisterMask(0x100C, 0x0000,0x0001);
    //HOST_WriteRegister(0x100C, 0x0000);
    //HOST_WriteRegisterMask(0x106C, 0x0001,0x0001);
    //HOST_WriteRegister(0x100C, 0x4069); //0x0049  //0x4061 for serial
    //HOST_WriteRegisterMask(0x106C, 0x0001,0x0001);

    //set FIFOrequest acc size
    //HOST_WriteRegisterMask(0x100c,0x1100,0x1F00);//Enable bit and size=7(8)
    //HOST_ReadRegister(0x100C,&Reg_6);

    //initial Last_PCR[8]
    for(i=0;i<8;i++)
    {
        Last_PCR[i]=0;
    }

    t = PalGetClock();
    while (Counter--)
    //while (1)
    {
        MMP_UINT16 Reg16=0;
        MMP_UINT32 diff=0;

        //check default value
        //printf("[TSI]get tx %d ms\n", PalGetDuration(t));

        //polling TSI INT
        while(0)
        {
            HOST_ReadRegister(0x8400+0x116, &Reg16);    //polling FIQ
            if(Reg16&0x0010)    {   break;  }//TSI0
            //if(Reg16&0x0020)  {   break;  }//TSI1
        }
        t = PalGetClock();
        //printf("polling TSI INT.");

        //polling TSI0 INT
        #ifdef  ENABLE_TSI_IRQ_FUNCTION
        while(1)
        {
            static MMP_UINT32 loop=0;
            MMP_UINT32 diff=0;

            if( mmpTsiIsPcrInterruptTriggered(TsiSelect)==MMP_TRUE )
            {
                //TsiMsg = mmpTsiGetMsg(TsiSelect);
                //if(TsiMsg&)
                //mmpTsiClearInterrupt(TsiSelect);
                //printf("mmp got intr!![%x,%x]\n",mmpTsiIsPcrInterruptTriggered(TsiSelect),MMP_TRUE);
                break;
            }
            /*
            loop++;
            if(loop>1024)
            {
                printf(".");
                loop=0;
            }
            */
            //printf(".");
        }
        PCR=mmpTsiGetPcrValue((TsiSelect);
        PID=mmpTsiGetPcrPid(TsiSelect);
        #else
        Reg_15=0;
        while( (Reg_15&0x0018)!=0x0018 )
        {
            HOST_ReadRegister(TmpTsiBase+0x101E,&Reg_15);//need to check this reg, too
            //if( (Reg_15&0x8000) == 0x8000 )
                //printf("[TSI][error][non-188]\n");
            if( (Reg_15&0x4000) == 0x4000 )
                printf("[TSI][error][fifo-full]\n");
            printf("wait interrupt,reg15:= 0x%x\n",Reg_15);
        }
        HOST_ReadRegister(TmpTsiBase+0x100C,&Reg_6);

        //check PID & PCR
        {
            HOST_ReadRegister(0x1018,&PCR1);    //have to read PCR1 first before check PID & PCR2 & PCR3
            HOST_ReadRegister(0x101A,&PCR2);
            HOST_ReadRegister(0x101C,&PCR3);
            HOST_ReadRegister(0x1016,&PID);
        }

        //Get the PCRcnt
        HOST_ReadRegister(0x1024,&PcrCnt_L);
        HOST_ReadRegister(0x1026,&PcrCnt_H);

        PID&=0x1FFF;    //Mask the PID value
        PCR=((MMP_UINT32)PCR2<<16) + PCR1;  //merge PCR value
        #endif

        //check PID & PCR
        {
            //HOST_ReadRegister(0x1018,&PCR1);  //have to read PCR1 first before check PID & PCR2 & PCR3
            //HOST_ReadRegister(0x101A,&PCR2);
            //HOST_ReadRegister(0x101C,&PCR3);
            //HOST_ReadRegister(0x1016,&PID);
        }
        //mmpTsiClearInterrupt(TsiSelect);
        //HOST_WriteRegisterMask(0x106C, 0x0001,0x0001);

        //check PID
        switch(PID)
        {

            case TTV_PID_4001:
            //case 256:
                if(PCR<=Last_PCR[0])
                {
                    diff=Last_PCR[0]-PCR;
                }
                else
                {
                    diff=PCR - Last_PCR[0];
                }
                printf("[TSI][%x]:PCR > last PCR of PID=%x!![%08x][%08x][%x]\n",Counter,PID,PCR,Last_PCR[0],diff);
                //printf("[TSI][%x]:PCR3 := [%08x]\n",PCR3);
                //printf("[TSI][%x]:PIDDuration := [%08x]\n",Counter,PCR-Last_PCR[0]);
                //printf("[TSI][error][%x]:wrong PID value!!PID=%d,PCR/Cnt=[%x,%x],PCRcnt[%x,%x]\n",Counter,PID,PCR_Diff,PcrCnt,PcrCnt_H,PcrCnt_L);
                if(PCR<=Last_PCR[0])
                {
                    printf("[TSI][error][2][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[0]);
                    //HOST_WriteRegister(TmpTsiBase+0x1020,0x0001); //Enable BIST mode to stop DEMOD?
                    //HOST_WriteRegister(TmpTsiBase+0x1020,0x0000); //Disable BIST mode to continue DEMOD?
                    if(Last_PCR[0])
                    {
                        if( (PCR>0x000FFFFF) && (Last_PCR[0]<0xFF000000) )
                            while(1);
                    }
                }
                else
                {
                    if( (PCR-Last_PCR[0]) >0x2000 )
                    {
                        printf("[TSI][%x]:PCR-last PCR of PID=%x!![%08x][%08x]\n", Counter, PID, PCR, Last_PCR[0]);
                        if(Last_PCR[0])
                        {
                            if( (PCR>0x000FFFFF) && (Last_PCR[0]<0xFF000000) )
                                while(1);
                        }
                    }
                }

                Last_PCR[0]=PCR;
                break;

            case TTV_PID_4011:
                printf("[TSI][%x]:PCR > last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[1]);
                if(PCR<=Last_PCR[1])
                {
                    printf("[TSI][error][3][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[1]);
                    HOST_WriteRegister(TmpTsiBase+0x1020,0x0001);   //Enable BIST mode to stop DEMOD?
                    //HOST_WriteRegister(TmpTsiBase+0x1020,0x0000); //Disable BIST mode to continue DEMOD?
                }

                Last_PCR[1]=PCR;
                break;

            case TTV_PID_4021:
                printf("[TSI][%x]:PCR > last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[2]);
                if(PCR<=Last_PCR[2])
                {
                    printf("[TSI][error][4][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[2]);
                    HOST_WriteRegister(TmpTsiBase+0x1020,0x0001);   //Enable BIST mode to stop DEMOD?
                    HOST_WriteRegister(TmpTsiBase+0x1020,0x0001);   //Enable BIST mode to stop DEMOD?
                }

                Last_PCR[2]=PCR;
                break;

            case 4042:
                printf("[TSI][%x]:PCR > last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[3]);
                if(PCR<=Last_PCR[3])
                {
                    printf("[TSI][error][5][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[3]);
                    HOST_WriteRegister(TmpTsiBase+0x1020,0x0001);   //Enable BIST mode to stop DEMOD?
                    HOST_WriteRegister(TmpTsiBase+0x1020,0x0001);   //Enable BIST mode to stop DEMOD?
                }

                Last_PCR[3]=PCR;
                break;

            default:
                printf("[TSI][error][%x]:wrong PID value!!PID=%d\n",Counter,PID);
                break;
        }

        //Timmer++;

        if(Timmer>1)
        {
            //for checking TSI reset bug only
            Timmer=0;
            while(1)
            {
                MMP_UINT16 wReg16;

                Timmer++;

                HOST_ReadRegister(TmpTsiBase+0x101E,&wReg16);

                if( (wReg16&0x0002)==0x0000 )
                {
                    printf("Wait not Idle, Timmer=%d!!\n",Timmer);
                    break;
                }
            }

            //1.disable TSI controller
            HOST_WriteRegisterMask(TmpTsiBase+0x100C, 0x0000,0x0001);
            Timmer=0;

            //2.check TSI idle bit


            while(1)
            {
                MMP_UINT16 wReg16;

                Timmer++;

                HOST_ReadRegister(TmpTsiBase + 0x101E,&wReg16);

                if(wReg16 & 0x0002)
                {
                    printf("Wait Idle, Timmer=%d!!\n",Timmer);
                    break;
                }
            }

            //3.enable TSI controller again
            HOST_WriteRegisterMask(TmpTsiBase+0x100C, 0x0001,0x0001);

            //4.check if PID PCR are still normal
            Timmer=0;
        }

        //PCR_H=PCR3;
        //clear B0TSI0Int
        //HOST_WriteRegisterMask(TmpTsiBase+0x106C, 0x0001,0x0001);
        //HOST_WriteRegisterMask(0x8400+0x10A, 0x0030,0x0030);//clear FIQ_TSI0
        #ifdef  ENABLE_TSI_IRQ_FUNCTION
        mmpTsiClearInterrupt(TsiSelect);
        #else
        //clear B0TSI0Int
        HOST_WriteRegisterMask(TmpTsiBase+0x106C, 0x0001,0x0001);
        //HOST_WriteRegisterMask(0x8400+0x10A, 0x0030,0x0030);//clear FIQ_TSI0
        #endif

    }
    //Disable
    mmpTsiDisable(TsiSelect);

    mmpTsiTerminate(TsiSelect);
    printf("END of TSI INT/PCR/PID test...\n");
    while(1);
}


static void
_Check_INT_PCR_CNT(
    void)
{
    MMP_UINT16  Reg_6,Reg_15;
    MMP_UINT16  TV_PID;
    MMP_UINT16  PID,PCR1,PCR2,PCR3,PCR_H=0;
    MMP_UINT32  PCR,Last_PCR[8];
    MMP_UINT32  Counter=100000;
    MMP_UINT16  tmp;

    MMP_RESULT  result;
    MMP_UINT8*  sysBuf  = MMP_NULL;
    MMP_UINT32  size = 0;
    MMP_UINT32  totalReadSize = 0;
    MMP_UINT16  PcrCnt_L,PcrCnt_H;
    MMP_UINT32  PcrCnt;
    MMP_UINT32  PCR_Diff;
    MMP_UINT8   Non188Index=0;
    MMP_UINT8   FifoFullIndex=0;
    MMP_UINT8   TsiSelect;
    MMP_UINT8   TsiBaseAddr;

    MMP_UINT32 i=0;
    MMP_UINT32 TotalPCRcntIdx=0;
    MMP_UINT32 TotalSumPCR=0;
    MMP_UINT32 AvgPCRcnt=0;

    printf("_Check_INT_PCR_CNT[0]\n");

    TsiSelect=1;
    if(TsiSelect)
    {
        TsiBaseAddr=0x80;
    }
    else
    {
        TsiBaseAddr=0x00;
    }

    mmpTsiInitialize(TsiSelect);
    printf("mmpInit[1]\n");

    //enable PCR INT
    HOST_WriteRegisterMask(TsiBaseAddr+0x100C, 0x0000, 0x0004);

    //disable the mem size threshold INT()
    HOST_WriteRegisterMask(TsiBaseAddr+0x100C, 0x0002, 0x0002);

    HOST_ReadRegister(TsiBaseAddr+0x100C,&Reg_15);
    printf("Set PCR INT enable,0x100C=[%x].\n",Reg_15);

    //set PID filter 4001(0x1F43=0x0FA1<<1 + 1)
    TV_PID = 4001;
    HOST_WriteRegister(TsiBaseAddr+0x100E, TV_PID<<1 | 1); //set PID filter

    //SET pcr COUNT THRESHOLD REG = (VALUE << 1)
    HOST_WriteRegister(TsiBaseAddr+0x1010, (0X000E<<1)|0x0001 );

    //set 32 sets of PID filter
    //HOST_WriteRegister(TsiBaseAddr+0x102C, (TTV_PID_4001<<1)|0x0001 );
    //HOST_WriteRegister(TsiBaseAddr+0x102E, (TTV_PID_4011<<1)|0x0001 );
    //HOST_WriteRegister(TsiBaseAddr+0x1030, (TTV_PID_4021<<1)|0x0001 );
    //HOST_WriteRegister(TsiBaseAddr+0x1030, (TTV_PID_4042<<1)|0x0001 );

    printf("alloc mem..\n");
    sysBuf = (MMP_UINT8*)SYS_Malloc(MAX_READ_SIZE);
    //sysBuf = (MMP_UINT8*)MEM_Allocate(MAX_READ_SIZE, MEM_USER_TSI);
    //sprintf(pszBuffer, "C:\\tsi_%d.ts", index);
    printf("[2]alloc mem..OK\n");
    printf("[0]init tsi..OK\n");

    mmpTsiEnable(TsiSelect);
    printf("[1]init tsi..OK\n");

    //set PCR count refference clock PCLK
    HOST_WriteRegisterMask(TsiBaseAddr+0x1028, 0x006C, 0x03FF); //for clock=27Mhz
    //HOST_WriteRegisterMask(TsiBaseAddr+0x1028, 0x0090, 0x03FF); //for clock=36Mhz
    HOST_WriteRegisterMask(0x0040, 0x0000, 0x0002);     //for FPGA platform,

    //initial Last_PCR[8]
    for(i=0;i<8;i++)
    {
        Last_PCR[i]=0;
    }

    //while (Counter--)
    while (1)
    {
        //check default value

        //polling TSI0 INT
        #ifdef  ENABLE_TSI_IRQ_FUNCTION
        //printf("chk IRQ intr\n");
        while(1)
        {
            static MMP_UINT32 loop=0;
            MMP_UINT32 diff=0;

            if( mmpTsiIsPcrInterruptTriggered(TsiSelect)==MMP_TRUE )
            {
                //TsiMsg = mmpTsiGetMsg(TsiSelect);
                //if(TsiMsg&)
                //mmpTsiClearInterrupt(TsiSelect);
                //printf("mmp got intr!![%x,%x]\n",mmpTsiIsPcrInterruptTriggered(TsiSelect),MMP_TRUE);
                break;
            }
            /*
            loop++;
            if(loop>1024)
            {
                printf(".");
                loop=0;
            }
            */
            //printf(".");
        }
        #else
        Reg_15=0;
        while( (Reg_15&0x0018)!=0x0018 )
        {
            HOST_ReadRegister(TsiBaseAddr+0x101E,&Reg_15);//need to check this reg, too
            if( (Reg_15&0x8000) == 0x8000 )
                printf("[TSI][error][non-188]\n");
            if( (Reg_15&0x4000) == 0x4000 )
                printf("[TSI][error][fifo-full]\n");
            printf("wait interrupt,reg15:= 0x%x\n",Reg_15);
        }
        #endif

        /*
        if(Non188Index || FifoFullIndex)
        {
            MMP_UINT16  WrPtrReg16;
            HOST_ReadRegister(TsiBaseAddr+0x1012, &WrPtrReg16);//read WrPtr
            printf("b0.WrPtr.L=[%04x]\n",WrPtrReg16);

            HOST_WriteRegisterMask(TsiBaseAddr+0x100C, 0x0000, 0x0001);//clear INTr

            HOST_ReadRegister(TsiBaseAddr+0x1012, &WrPtrReg16);//read WrPtr
            printf("b1.WrPtr.L=[%04x]\n",WrPtrReg16);


            FifoFullIndex = 0;
            Non188Index = 0;

            HOST_ReadRegister(TsiBaseAddr+0x1014, &WrPtrReg16);//read WrPtr
            printf("a0.WrPtr.H=[%04x]\n",WrPtrReg16);
            HOST_ReadRegister(TsiBaseAddr+0x1012, &WrPtrReg16);//read WrPtr
            printf("a0.WrPtr.L=[%04x]\n",WrPtrReg16);

            HOST_WriteRegisterMask(TsiBaseAddr+0x100C, 0x0001, 0x0001);//clear INTr (disable TSI)

            HOST_ReadRegister(TsiBaseAddr+0x1014, &WrPtrReg16);//read WrPtr
            printf("a0.WrPtr.H=[%04x]\n",WrPtrReg16);
            HOST_ReadRegister(TsiBaseAddr+0x1012, &WrPtrReg16);//read WrPtr
            printf("a.WrPtr.L=[%04x]\n",WrPtrReg16);
        }
        */
        //check PID & PCR
        #ifdef  ENABLE_TSI_IRQ_FUNCTION
        PCR=mmpTsiGetPcrValue((TsiSelect);
        PID=mmpTsiGetPcrPid(TsiSelect);
        PcrCnt=mmpTsiGetPCRCnt(TsiSelect);
        if(PCR<Last_PCR[0])
        {
            printf("[TSI][error][2][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[0]);
        }
        else
        {
            PCR_Diff = PCR - Last_PCR[0];
        }
        #else
        HOST_ReadRegister(TsiBaseAddr+0x1018,&PCR1);    //have to read PCR1 first before check PID & PCR2 & PCR3
        HOST_ReadRegister(TsiBaseAddr+0x101A,&PCR2);
        HOST_ReadRegister(TsiBaseAddr+0x101C,&PCR3);
        HOST_ReadRegister(TsiBaseAddr+0x1016,&PID);


        //Get the PCRcnt
        HOST_ReadRegister(TsiBaseAddr+0x1024,&PcrCnt_L);
        HOST_ReadRegister(TsiBaseAddr+0x1026,&PcrCnt_H);

        PID&=0x1FFF;    //Mask the PID value
        PCR=((MMP_UINT32)PCR2<<16) + PCR1;  //merge PCR value
        //if(PCR<=Last_PCR[0])
        if(PCR<Last_PCR[0])
        {
            printf("[TSI][error][2][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[0]);
        }
        else
        {
            PCR_Diff = PCR - Last_PCR[0];
        }

        PcrCnt = ((MMP_UINT32)(PcrCnt_H & 0x3FF)<<16) + PcrCnt_L;   //merge PCR count value
        #endif

        //check PID
        switch(PID)
        {
            case 4001://256:
                //printf("[TSI]:PCR=[%08x],LastPCR=[%08x],[%x,%x][%x,%04x]\n",PCR,Last_PCR[0],PCR_Diff,PcrCnt,PcrCnt_H,PcrCnt_L);
                //printf("[TSI][%x]:PID=%d,PCR=[%08x],LastPCR=[%08x],[%x,%x][%x,%04x]\n",Counter,PID,PCR,Last_PCR[0],PCR_Diff,PcrCnt,PcrCnt_H,PcrCnt_L);
                //printf("[TSI][%x]:PID=%d,PCR=[%08x],[%x,%x]\n",Counter,PID,PCR,PCR_Diff,PcrCnt);
                //printf("[TSI][%x]:wrong PID value!!PID=%d,PCR/Cnt=[%x,%x],PCRcnt[%x,%x]\n",Counter,PID,PCR_Diff,PcrCnt,PcrCnt_H,PcrCnt_L);
                {
                    MMP_UINT32 k=0;

                    //i = (PCR_Diff/90)*(1000000/37);
                    i = ((PCR_Diff*3003)/10);//i is the converted value of PCRcnt
                    //i = ((PCR_Diff*1001)/10);//for pclk = 27Mhz(real chip is 80Mhz)
                    //printf("PCR_Diff=0x%x, PcrCnt=0x%x, [%x,%x]\n ",PCR_Diff,PcrCnt,i,k);
                }
                if(PCR_Diff<0x3500)
                {
                    //calculate the average of PCR_Diff to avoid from stopping for PCR_Diff*2 case
                    TotalSumPCR+=PCR_Diff;
                    TotalPCRcntIdx++;
                    AvgPCRcnt = (TotalSumPCR)/TotalPCRcntIdx;
                }
                printf("[TSI]:PCR=[%08x],LastPCR=[%08x],[%x,%x,%x][%x,%x]\n",PCR,Last_PCR[0], PCR_Diff,i,PcrCnt, TotalPCRcntIdx,AvgPCRcnt);

                if( (i>PcrCnt) && ((i-PcrCnt)>0x20000) && (PcrCnt>0x20000))
                //if( (i>PcrCnt) && ((i-PcrCnt)>0x10000) )
                {
                    printf("  i=0x%x, PCR_Diff=0x%x, PcrCnt=0x%x\n ",i, PCR_Diff, PcrCnt);
                    if(Last_PCR[0])
                    {
                        if( (PCR_Diff%AvgPCRcnt) > (AvgPCRcnt>>2) )
                        {
                            if( (AvgPCRcnt-(PCR_Diff%AvgPCRcnt)) > (AvgPCRcnt>>2) )
                            {
                                while(1);
                            }
                        }
                    }
                }
                else if( (i<PcrCnt) && ((PcrCnt-i)>0x20000) )
                {
                    printf("  i=0x%x, PCR_Diff=0x%x, PcrCnt=0x%x\n ",i, PCR_Diff, PcrCnt);
                    if(Last_PCR[0])
                    {
                        if( (PCR_Diff%AvgPCRcnt)>(AvgPCRcnt>>2) )
                        {
                            if( (AvgPCRcnt-(PCR_Diff%AvgPCRcnt)) > (AvgPCRcnt>>2) )
                            {
                                while(1);
                            }
                        }
                    }
                }

                //if(PCR<=Last_PCR[0])
                if (PcrCnt<0x10000)
                {
                    printf("i=0x%x, PCR_Diff=0x%x, PcrCnt=0x%x,[H=%x,L=%x]\n ",i, PCR_Diff,PcrCnt,PcrCnt_H,PcrCnt_L);
                    if(Last_PCR[0])
                        while(1);
                }

                if(PCR<Last_PCR[0])
                {
                    MMP_UINT16 Reg_16;
                    HOST_ReadRegister(TsiBaseAddr+0x101E,&Reg_16);//need to check this reg, too
                    if( (Reg_16&0x8000) == 0x8000 )
                        printf("[TSI][error][non-188] Reg(101E)=[%x]!!\n",Reg_16);
                    printf("[TSI][error][3.1][%x]:PCR <= last PCR of PID=%x!![%08x][%08x]\n",Counter,PID,PCR,Last_PCR[0]);
                    //HOST_WriteRegister(0x1020,0x0001);    //Enable BIST mode to stop DEMOD?
                    //HOST_WriteRegister(0x1020,0x0000);    //Enable BIST mode to stop DEMOD?
                    if(Last_PCR[0])
                    {
                        if( (PCR>0x00002000) && (Last_PCR[0]<0xFF000000) )
                            while(1);
                    }
                }
                Last_PCR[0]=PCR;
                break;

            default:
                printf("[TSI][error][%x]:wrong PID value!!PID=%d,PCR/Cnt=[%x,%x]\n",Counter,PID,PCR_Diff,PcrCnt);
                break;
        }

        //PCR_H=PCR3;

        //clear B0TSI0Int
        #ifdef  ENABLE_TSI_IRQ_FUNCTION
        mmpTsiClearInterrupt(TsiSelect);
        #else
        {
            MMP_UINT16 Reg_w;

            HOST_WriteRegisterMask(TsiBaseAddr+0x106C, 0x0001, 0x0001);
            HOST_ReadRegister(TsiBaseAddr+0x101E,&Reg_w);//need to check this reg, too
            printf("Do int clear!![%04x]\n",Reg_w);
        }
        #endif

    }
    //Disable
    mmpTsiDisable(TsiSelect);

    mmpTsiTerminate(TsiSelect);
    printf("END of TSI INT/PCR/PID test...\n");
    while(1);
}

void SearchLastPid(MMP_UINT8 *SearchBuffer,MMP_UINT32 TotalSize, MMP_UINT32 *LastFrameAddress )
{
    MMP_UINT32 i=0,j,k=0;

    while(1)
    {
        if(i>=TotalSize)
        {
            printf("out of buffer range!!\n");
            *LastFrameAddress = 0;
            break;
        }

        if(SearchBuffer[i]==0x47)
        {
            //result=check_PID_PCR();
            if( (i+188)>=TotalSize )
            {
                printf("does not Get NON-188 frame!!\n");
                *LastFrameAddress = 0;
                break;
            }
            if(SearchBuffer[i+188]!=0x47)
            {
                printf("[Err] Not 188,address=%08x!!\n",i);
                *LastFrameAddress = i;
                /*
                if(i>=188*5)
                {
                    k=i-(188*5);
                }
                for(j=0;j<1880;j++)
                {
                    printf("%02x ",SearchBuffer[k+j]);
                    if( (j&0x0F)==0x0F )    printf("\n");
                    //if( (j%188)==187 )    printf("\n");
                }
                printf("\n");
                */
                //search_next_47
                break;
            }
        }
        i+=188;
    }
}

void SearchFirstPidAfterErr(MMP_UINT8 *SearchBuffer, MMP_UINT32 TotalSize, MMP_UINT32 *SndAddress)
{
    MMP_UINT32 i=0,j,k=0;

    while(1)
    {
        if(i>=TotalSize)
        {
            printf("out of buffer range!!\n");
            *SndAddress = 0;
            break;
        }

        if(SearchBuffer[i]==0x47)
        {
            MMP_UINT8   *bData=&SearchBuffer[i+1];
            MMP_UINT8   getTsiFrame=0;
            MMP_UINT16  wData16=((MMP_UINT16)bData[0]<<8)+(MMP_UINT16)bData[1];

            switch(wData16)
            {
                case 1001:
                case 1002:
                case 1003:
                case 1011:
                case 1012:
                case 1021:
                case 1022:

                case 4001:
                case 4002:
                case 4003:
                case 4011:
                case 4012:
                case 4021:
                case 4022:
                    if(SearchBuffer[i+188]==0x47)
                    {
                        getTsiFrame=1;
                    }
                    break;
                default:
                    break;
            }

            if(getTsiFrame)
            {
                printf("Got the first 188 frame,address=%08x!!\n",i);
                *SndAddress = i;
                /*
                if(i>=188*5)
                {
                    k=i-(188*5);
                }
                for(j=0;j<1880;j++)
                {
                    printf("%02x ",SearchBuffer[k+j]);
                    if( (j&0x0F)==0x0F )    printf("\n");
                    //if( (j%188)==187 )    printf("\n");
                }
                printf("\n");
                */

                break;
            }
            //result=check_PID_PCR();
            if( (i+188)>=TotalSize )
            {
                printf("does not Get 188 frame!!\n");
                *SndAddress = 0;
                break;
            }
        }
        i++;
    }
}
