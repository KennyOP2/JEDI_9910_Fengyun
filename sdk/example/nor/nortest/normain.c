/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * The example do nandflash FAT test.
 * @author Awin Huang
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "mmp_rtc.h"
#include "pal/thread.h"
#include "pal/timer.h"


#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "host/ahb.h"
#endif


//#include "test/test_f.h"
//#include "../../../share/src/fat/src/ftl/src/ftldrv.h"
//#include "../../../share/src/fat/src/ftl/src/ftl_defs.h"
//#include "../../../share/src/fat/src/ftl/src/hlayer.h"
//#include "../../../share/src/fat/src/ftl/src/ftl/mlayer.h"
//#include "../../sdk/include/xd/xd_mlayer.h"
#include "mmp_nor.h"


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif
//**************************************************************************
//  Type definiation
//**************************************************************************
enum {
	LL_OK,
	LL_ERASED,
	LL_ERROR
};

//**************************************************************************
//  Global VAriable
//**************************************************************************


//**************************************************************************
//  Compile Option
//**************************************************************************


//**************************************************************************
//  Function prototype
//**************************************************************************
static void RWRateTest();

//  Function Implamentation
//**************************************************************************
void NOR_RWRateTest()
{
    unsigned char   result;
    unsigned long   i,m, j;
    unsigned long   NOR_Capacity,TestSize=65536,TestLoop;
    unsigned char*  TempBuffer= (unsigned char*)malloc(TestSize);
	unsigned char*  TempBuffer2= (unsigned char*)malloc(TestSize);
	MMP_UINT32		clock = 0;
	MMP_UINT16      regVal = 0;
	MMP_UINT32		time = 0;
    printf("NOR TEST!!\n");
    mmpDmaInitialize();
    result = NorInitial();

    if(result)
    {
        printf("NOR Initial fail!!!\n");
    }
    else
    {
        printf("NOR Initial success!!\n");
		j = 0;
		printf("2.NOR Write/Read/Comp test!!\n");
        //2.write test
		//for(TestSize = 4; TestSize < 1024; TestSize += 7)
		for(TestSize = 25; TestSize < 1024; TestSize += 7)
		{
			printf("TestSize = %d\n", TestSize);
			for(i=0;i<TestSize;i++)
			{
				TempBuffer[i]=(i&0xFF);
				//printf("0x%02X ", TempBuffer[i]);
			}
			//printf("\n");

			printf("  2-1.Writing...");
			result=NorWrite(TempBuffer,0,TestSize);
			if(result)
			{
				printf("====================FAIL:01!!\n");
				goto    NorEnd;
			}
			else
			{
				printf("====================PASS\n");
			}

			for(i=0;i<TestSize;i++)
			{
				TempBuffer[i]=0xFF;
			}
			printf("  2-2.Reading...");
			result=NorRead(TempBuffer,0,TestSize);
			if(result)
			{
				printf("====================FAIL:02!!\n");
				goto    NorEnd;
			}
			else
			{
				printf("====================PASS\n");
			}

			printf("  2-3.Comaparing...\n");
			for(i=0;i<TestSize;i++)
			{
				if(TempBuffer[i]!=(i&0xFF))
				{
					unsigned long   j,k;
					printf("[%d]FAIL:03!! at %d\n", __LINE__, i);
					printf("0000:");
					for(j=0;j<TestSize;j++)
					{
						printf("%02x ",TempBuffer[j]);
						if( (j&0x0F)==0x07 )    printf(" ");
						if( (j&0x0F)==0x0F )    printf("\n%04x:",(j>>4)+1);
					}
					goto    NorEnd;
				}
			}
		}
		for(TestSize = 1024; TestSize < 0x10000; TestSize += 101)
		{
			printf("TestSize = %d\n", TestSize);
			for(i=0;i<TestSize;i++)
			{
				TempBuffer[i]=(i&0xFF);
			}
			//clock = PalGetClock();

			printf("  2-1.Writing...");

			result=NorWrite(TempBuffer,0,TestSize);
			//printf("(%d),Write Time: %u ms\n", __LINE__, PalGetDuration(clock));
			
			if(result)
			{
				printf("====================FAIL:01!!\n");
				goto    NorEnd;
			}
			else
			{
				printf("====================PASS\n");
			}

		
			for(i=0;i<TestSize;i++)
			{
				TempBuffer[i]=0xFF;
			}
			printf("  2-2.Reading...");
			result=NorRead(TempBuffer,0,TestSize);

			if(result)
			{
				printf("====================FAIL:02!!\n");
				goto    NorEnd;
			}
			else
			{
				printf("====================PASS\n");
			}

			printf("  2-3.Comaparing...\n");
			for(i=0;i<TestSize;i++)
			{
				if(TempBuffer[i]!=(i&0xFF))
				{
					unsigned long   j,k;
					printf("[%d]FAIL:03!!\n", __LINE__);
					printf("0000:");
					for(j=0;j<TestSize;j++)
					{
						printf("%02x ",TempBuffer[j]);
						if( (j&0x0F)==0x07 )    printf(" ");
						if( (j&0x0F)==0x0F )    printf("\n%04x:",(j>>4)+1);
					}
					printf("    ");
					goto    NorEnd;
				}
			}
		}
		printf("Compare PASS\n");
		TestSize = 65536;
        //3.erase test
        printf("3.NOR Erase test!!\n");
        printf("  3-1.Erasing...");
		clock = PalGetClock();
        result=NorBulkErase();
		printf("(%d),Write Time: %u ms\n", __LINE__, PalGetDuration(clock));
        if(result)
        {
            printf("====================FAIL:01!!\n");
            goto    NorEnd;
        }
        else
        {
            printf("====================PASS!!\n");
        }

        for(i=0;i<TestSize;i++)
        {
            TempBuffer[i]=0xAA;
        }
        printf("  3-2.Reading...");
		clock = PalGetClock();
        result=NorRead(TempBuffer,0,TestSize);
		printf("(%d),3-2.Reading... Read Time: %u ms\n", __LINE__, PalGetDuration(clock));

        if(result)
        {
            printf("====================FAIL:02!!\n");
            goto    NorEnd;
        }
        else
        {
            printf("====================PASS!!\n");
        }

        printf("  3-3.Comaparing...");
        for(i=0;i<TestSize;i++)
        {
            if(TempBuffer[i]!=0xFF)
            {
                printf("  NOR Erase test FAIL:03!!\n");
                goto    NorEnd;
            }
        }
        printf("====================PASS!!\n");
        //4.burn-in test
#ifndef  __CODE__
            printf("4.Read/Write/Erase/burn In test!!\n");
            NOR_Capacity=NorCapacity();
            printf("NOR Total SIZE = [%x]\n",NOR_Capacity);

            TestLoop=NOR_Capacity/TestSize;

            printf("  4-1.Erasing...");
            result=NorBulkErase();
			PalSleep(2 * 60 * 1000);

            if(result)
            {
                printf("====================FAIL:01!!\n");
                goto    NorEnd;
            }
            else
            {
                printf("====================PASS!!\n");
            }

            for(m=0;m<TestLoop;m++)
            {
                printf("  4-2.Reading data,%04xth Loop:",m);
                for(i=0;i<TestSize;i++)
                {
                    TempBuffer[i]=0x33;
                }
                result=NorRead(TempBuffer,m*TestSize,TestSize);

                if(result)
                {
                    printf("  4-2.Reading data,%04xth Loop:FAIL!!.01\n",m);
                    goto    NorEnd;
                }
                else
                {
                    printf("  4-2.Reading data,%04xth Loop:PASS!!",m);
                }

                printf("  4-2.Reading data,%04xth Loop:PASS,then Comparing...",m);

                for(i=0;i<TestSize;i++)
                {
                    if(TempBuffer[i]!=0xFF)
                    {
                        unsigned long   j,k;
                        printf("[%d]FAIL:03!!\n", __LINE__);
                        printf("0000:");
                        for(j=0;j<TestSize;j++)
                        {
                            printf("%02x ",TempBuffer[j]);
                            if( (j&0x0F)==0x07 )    printf(" ");
                            if( (j&0x0F)==0x0F )    printf("\n%04x:",(j>>4)+1);
                        }
                        printf("    ");
                        goto    NorEnd;
                    }
                }
                printf("  4-2.Reading data,%04xth Loop:PASS,then Comparing...PASS\n",m);
            }

            for(i=0;i<TestSize;i++)
            {
                TempBuffer[i]=0xAA;
            }

            //for(m=0;m<TestLoop;m++)

            //result=NorWrite(TempBuffer,0,TestSize);

            //result=NorWrite(TempBuffer,TestSize,TestSize);

            for(m=0;m<TestLoop;m++)
            {
                printf("  4-3.Writing data,%04xth Loop:",m);
                result=NorWrite(TempBuffer,m*TestSize,TestSize);
                printf("DONE!\n");
            }

            printf("    Writing data,%04xth Loop:Finished.\n",m);

            for(m=0;m<TestLoop;m++)
            {
                printf("  4-4.Reading data,%04xth Loop:",m);
                for(i=0;i<TestSize;i++)
                {
                    TempBuffer[i]=0xFF;
                }
                result=NorRead(TempBuffer,m*TestSize,TestSize);

                if(result)
                {
                    printf("  4-4.Reading data,%04xth Loop:FAIL!!.01\r",m);
                    goto    NorEnd;
                }
                else
                {
                    printf("  4-4.Reading data,%04xth Loop:PASS!!",m);
                }

                printf("  4-4.Reading data,%04xth Loop:PASS,then Comparing...",m);

                for(i=0;i<TestSize;i++)
                {
                    if(TempBuffer[i]!=0xAA)
                    {
                        unsigned long   j,k;
                        printf("[%d]FAIL:03!!\n", __LINE__);
                        printf("0000:");
                        for(j=0;j<TestSize;j++)
                        {
                            printf("%02x ",TempBuffer[j]);
                            if( (j&0x0F)==0x07 )    printf(" ");
                            if( (j&0x0F)==0x0F )    printf("\n",(j>>4)+1);
							if( (j&0x0F)==0x00 )    printf("%04x:",(j>>4)+1);
                        }
                        printf("\n");
                        goto    NorEnd;
                    }
                }
                printf("  4-4.Reading data,%04xth Loop:PASS,then Comparing...PASS\n",m);
            }

#endif//__CODE__
    }//else if(result)

NorEnd:
    printf("NOR R/W test finished!!\n");
}


#if defined (__FREERTOS__)
portTASK_FUNCTION(task_Nortest, params)
{
    //printf("\r\n%s: Main(): \n\r", __FILE__);
    printf("\nNOR_TEST: Main(): \n");

    //HccFatTest();
    NOR_RWRateTest();

    //RWRateTest();
    printf("\n NOR flash Read/Write test finished........\n");
    while(1);
}
#endif

int main(void)
{
    unsigned int    k;
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    ret = xTaskCreate(task_Nortest, "task_norflash",
        configMINIMAL_STACK_SIZE * 8,
        NULL, tskIDLE_PRIORITY + 1, NULL );
    if (pdFAIL == ret)
    {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#endif

#if defined (WIN32)
    printf("%s: Main(): \n", __FILE__);

    //HccFatTest();

    NOR_RWRateTest();
    //RWRateTest();
    printf("Main() test finished:");
    while(1);
#endif
    return 0;
}
