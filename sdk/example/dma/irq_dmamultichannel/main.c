/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "sys/sys.h"
#include "host/host.h"
#include "mem/mem.h"
#include "mmp_dma.h"
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#endif
#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"

#define USE_IRQ
#endif

#define TEST_CHANNLE	8

#if defined(__FREERTOS__)
xTaskHandle task0_handle = NULL;
portTASK_FUNCTION_PROTO(task0_func, params);

xTaskHandle task1_handle = NULL;
portTASK_FUNCTION_PROTO(task1_func, params);

xTaskHandle task2_handle = NULL;
portTASK_FUNCTION_PROTO(task2_func, params);

xTaskHandle task3_handle = NULL;
portTASK_FUNCTION_PROTO(task3_func, params);

xTaskHandle task4_handle = NULL;
portTASK_FUNCTION_PROTO(task4_func, params);

xTaskHandle task5_handle = NULL;
portTASK_FUNCTION_PROTO(task5_func, params);

xTaskHandle task6_handle = NULL;
portTASK_FUNCTION_PROTO(task6_func, params);

xTaskHandle task7_handle = NULL;
portTASK_FUNCTION_PROTO(task7_func, params);
#endif


#if defined(USE_IRQ)

#define IRQ_TIMEOUT		8000
static MMP_EVENT isrEvent[TEST_CHANNLE];

void isr(void* arg, MMP_UINT32 status)
{
	MMP_EVENT isrEvent = (MMP_EVENT)arg;
	if((status & DMA_IRQ_SUCCESS) != DMA_IRQ_SUCCESS)
		printf(" dma error 0x%08X, isrEvent 0x%08X \n", status, isrEvent);

	SYS_SetEventFromIsr(isrEvent);
}
#endif

static MMP_UINT32 testSize[8] = {1024*1024, 128*1024, 512*1024, 64*1024, 1024*1024, 256*1024, 128*1024, 640*1024};

MMP_INT DMA_Vram2Vram(MMP_UINT taskIndex, MMP_UINT32 srcOffset, MMP_UINT32 dstOffset, MMP_UINT32 burstSize)
{
    MMP_INT result = 0;
    MMP_UINT32 srcAddr = MMP_NULL;
    MMP_UINT32 dstAddr = MMP_NULL;
    MMP_UINT32 size = testSize[taskIndex];
    MMP_DMA_CONTEXT dmaCtxt = 0;
    MMP_INT i = 0;
    MMP_UINT8* tmpSrcAddr = MMP_NULL;
    MMP_INT diff = 0;

	#if defined(USE_IRQ)
	MMP_EVENT event = isrEvent[taskIndex];
	#endif

    srcAddr = (MMP_UINT32)malloc(size+4);
    if(!srcAddr)
    {
        printf(" Allocate source address fail! \n");
        while(1);
    }
    dstAddr = (MMP_UINT32)malloc(size+4);
    if(!dstAddr)
    {
        printf(" Allocate destination address fail! \n");
        while(1);
    }
    tmpSrcAddr = (MMP_UINT8*)srcAddr;
    for(i=0; i<(size+4); i++)
        tmpSrcAddr[i] = (i & 0xFF);

    memset((void*)dstAddr, 0x0, size);
    //printf(" [Task %d] srcAddr = 0x%08X \n", taskIndex, srcAddr+srcOffset);
    //printf(" [Task %d] dstAddr = 0x%08X \n", taskIndex, dstAddr+dstOffset);

    //MMP_Sleep(1);
    // Process DMA.
    {
        const MMP_UINT32 attribList[] =
        {
            MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
            MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)(srcAddr+srcOffset),
            MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)(dstAddr+dstOffset),
            MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)size,
            MMP_DMA_ATTRIB_SRC_BURST_SIZE, (MMP_UINT32)burstSize,
            MMP_DMA_ATTRIB_NONE
        };

        result = mmpDmaCreateContext(&dmaCtxt);
        if(result)
        {
            result = mmpDmaDestroyContext(dmaCtxt);
            if(result)
                goto end;
        }
		#if defined(USE_IRQ)
		mmpDmaRegisterIsr(dmaCtxt, isr, (void*)event);
		#endif
        result = mmpDmaSetAttrib(dmaCtxt, attribList);
        if(result)
            goto end;
		printf(" T%d S! \n", taskIndex);
        result = mmpDmaFire(dmaCtxt);
        if(result)
            goto end;
		#if defined(USE_IRQ)
	    result = SYS_WaitEvent(event, IRQ_TIMEOUT);
		if(result)
		{
			printf(" wait irq timeout... \n");
			while(1);
			goto end;
		}
		else
			printf(" T%d E! \n", taskIndex);
		#else
        result = mmpDmaWaitIdle(dmaCtxt);
        if(result)
            goto end;
		#endif
        result = mmpDmaDestroyContext(dmaCtxt);
        if(result)
            goto end;
    }
    #if defined(__FREERTOS__)
    ithInvalidateDCacheRange((void*)(dstAddr+dstOffset), size);
    #endif

    // Compare data
    diff = memcmp((void*)(srcAddr+srcOffset), (void*)(dstAddr+dstOffset), size);
	if(diff)
	{
	    printf(" [T%d] diff = %d \n", taskIndex, diff);
		while(1);
	}


end:
    if(srcAddr)
        free((void*)srcAddr);
    if(dstAddr)
        free((void*)dstAddr);
    if(result)
    {
        printf(" DMA_Vram2Vram() return error code 0x%08X \n", result);
    }

    return result;
}

void DoTest(MMP_UINT taskIndex, MMP_UINT32 loopCnt)
{
    MMP_UINT32 burstSize[8] = {256,128,64,32,16,8,4,1};
    MMP_UINT32 i;

    printf("loopCnt = %d \n", loopCnt);

    for(i=0;i<8;i++)
    {
        if(burstSize[i]==1)
        {
            printf("\n Case 1: (32, 32, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 0, 0, burstSize[i]);

            printf("\n Case 2: (32, 8, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 0, 1, burstSize[i]);

            printf("\n Case 3: (32, 16, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 0, 2, burstSize[i]);
        }
        else
        {
            printf("\n Case 1: (32, 32, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 0, 0, burstSize[i]);

            printf("\n Case 2: (32, 8, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 0, 1, burstSize[i]);

            printf("\n Case 3: (32, 16, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 0, 2, burstSize[i]);

            printf("\n Case 4: (8, 32, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 1, 0, burstSize[i]);

            printf("\n Case 5: (8, 8, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 1, 1, burstSize[i]);

            printf("\n Case 6: (8, 16, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 1, 2, burstSize[i]);

            printf("\n Case 7: (16, 32, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 2, 0, burstSize[i]);

            printf("\n Case 8: (16, 8, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 2, 1, burstSize[i]);

            printf("\n Case 9: (16, 16, %d) -%d \n", burstSize[i], taskIndex);
            DMA_Vram2Vram(taskIndex, 2, 2, burstSize[i]);
        }
    }
}


int main(int argc, char** argv)
{	
    MMP_INT result = 0;

#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;
	MMP_INT i=0;

    HOST_GetChipVersion();
    mmpDmaInitialize();

    ret = xTaskCreate(task0_func, "dma0",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task0_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 0!!\n");
        return 1;
    }

    ret = xTaskCreate(task1_func, "dma1",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task1_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

    ret = xTaskCreate(task2_func, "dma2",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task2_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

    ret = xTaskCreate(task3_func, "dma3",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task3_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

    ret = xTaskCreate(task4_func, "dma4",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task4_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

    ret = xTaskCreate(task5_func, "dma5",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task5_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

    ret = xTaskCreate(task6_func, "dma6",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task6_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

    ret = xTaskCreate(task7_func, "dma7",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &task7_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create task 1!!\n");
        return 1;
    }

	for(i=0; i<TEST_CHANNLE; i++)
		isrEvent[i] = SYS_CreateEvent();

    vTaskStartScheduler();
#endif
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(task0_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(0, loopCnt++);
}

portTASK_FUNCTION(task1_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(1, loopCnt++);
}

portTASK_FUNCTION(task2_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(2, loopCnt++);
}

portTASK_FUNCTION(task3_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(3, loopCnt++);
}

portTASK_FUNCTION(task4_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(4, loopCnt++);
}

portTASK_FUNCTION(task5_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(5, loopCnt++);
}

portTASK_FUNCTION(task6_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(6, loopCnt++);
}

portTASK_FUNCTION(task7_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DoTest(7, loopCnt++);
}
#endif


