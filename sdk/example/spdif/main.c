/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "pal/pal.h"
#include "mmp_spdif.h"



#include "host/ahb.h"
#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "host/ahb.h"
#endif

#ifdef __OPENRTOS__
#include "ite/ith.h"
#endif


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

#define SSP_BASE SSP2_BASE

MMP_RESULT DoTest(void)
{
    MMP_RESULT result = 0;
    MMP_UINT16 *buf = MMP_NULL;
	MMP_UINT16 *rxBuf = MMP_NULL;
    MMP_UINT32 size = 0xFF;

    MMP_UINT32 reg = 0;
    MMP_UINT32 value = 0;
    MMP_UINT32 i = 0;
    MMP_UINT16 base = 0;
    MMP_UINT32 index = 0;

    MMP_UINT32 count = 0;
    MMP_UINT16 *countBuf = MMP_NULL;
	MMP_UINT16 *countRxBuf = MMP_NULL;

    printf("[SPDIF] Test start ...\n");

    //for (index = 0; index < 10000; index++)
    {
        buf = malloc(size * sizeof(MMP_UINT16));
        rxBuf = malloc(size * sizeof(MMP_UINT16));
	    if(rxBuf == MMP_NULL || buf == MMP_NULL)
	    {
            printf("alloc fail\n");
		    result = -1;
		    goto end;
	    }
        countBuf = malloc(size * sizeof(MMP_UINT16));
        countRxBuf = malloc(size * sizeof(MMP_UINT16));
	    if(countRxBuf == MMP_NULL || countBuf == MMP_NULL)
	    {
            printf("alloc count fail\n");
		    result = -1;
		    goto end;
	    }

	    mmpDmaInitialize();
        mmpSpdifInitialize();

	    mmpSpdifSetAttribute(SPDIF_NON_LINEAR_DATA, 48000);
	    mmpSpdifSetEngineState(MMP_TRUE);
	    mmpSpdifDmaSetUp(buf, size*2);
        mmpSpdifDmaCountSetUp(countBuf, size*2);

        usb2spi_ReadMemory(rxBuf, buf, size * sizeof(MMP_UINT16));
        usb2spi_ReadMemory(countRxBuf, countBuf, size * sizeof(MMP_UINT16));

#if 0 // for 16 bits
        base = rxBuf[32];
        //printf("base:0x%x rxBuf[32]:0x%x\n",base,rxBuf[32]);
        
        for(i = 32; i < size; i++)
        {    
            if (base != rxBuf[i])
            {                
                printf("i=%d\n",i);
                printf("base:0x%x  rxBuf[i]:0x%x\n",base,rxBuf[i]);
                printf("\n");

                for(i = 0; i < size; i++)
                {
                    printf("rxBuf[%d]:0x%x\n",i,rxBuf[i]);
                }

                printf("[Data different!!!!!]\n");
                while(1);
            }

            if (base == 0x7F)
                base = 0;
            else
                base += 1;
        }

        if (i == size)
            printf("[Data the same!] ");
#else
        base = (rxBuf[65] << 8) | rxBuf[64];
        //printf("base:0x%x\n",base);

        for(i = 64; i < size-1; i++)
        {    
            if (base != ((rxBuf[i+1] << 8) | rxBuf[i]))
            {                
                printf("i=%d\n",i);
                printf("base:0x%x  rxBuf[i]:0x%x\n",base,((rxBuf[i+1] << 8) | rxBuf[i]));
                printf("\n");

                for(i = 0; i < size; i++)
                {
                    printf("rxBuf[%d]:0x%x\n",i,rxBuf[i]);
                }

                printf("[Data different!!!!!]\n");
                while(1);
            }

            i += 1;

            if (base == 0x7F)
                base = 0;
            else
                base += 1;
        }

        if (i == size - 1)
            printf("[Data the same!] ");

        for(i = 0; i < size; i++)
        {
            printf("countRxBuf[%d]:0x%x\n",i,countRxBuf[i]);
        }

#endif
        /*for(i = 32; i < size; i++)
        {    
            if (base != (rxBuf[i] & 0xFF) 
             || (base + 1) != (rxBuf[i] >> 8))
            {
                for(i = 0; i < size; i++)
                {
                    printf("rxBuf[%d]:0x%x\n",i,rxBuf[i]);
                }
                printf("\n");

                printf("i=%d\n",i);
                printf("base:0x%x  rxBuf[i] & 0xFF :0x%x\n",base,rxBuf[i] & 0xFF);
                printf("base+1:0x%x  rxBuf[i] >> 8 :0x%x\n",base+1,rxBuf[i] >> 8);
                printf("[Data different!!!!!]\n");
                break;
            }

            if (base == 0xFF || (base+1) == 0xFF)
                base = 0;
            else
                base += 2;
        }

        if (i == size)
            printf("[Data the same!] ");*/

end:
        if(buf)
        {
            buf = MMP_NULL;
            free(buf);
        }
        if(rxBuf)
        {
            rxBuf = MMP_NULL;
            free(rxBuf);
        }
        if(countBuf)
        {
            countBuf = MMP_NULL;
            free(countBuf);
        }
        if(countRxBuf)
        {
            countRxBuf = MMP_NULL;
            free(countRxBuf);
        }
    }
    printf("[End!!!!!]\n");
    while(1);
    return result;
}

int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "spdiftest_main",
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
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;
    
    result = DoTest();
}
#endif

