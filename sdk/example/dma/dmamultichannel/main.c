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
#endif


#if defined(__FREERTOS__)
xTaskHandle task0_handle = NULL;
portTASK_FUNCTION_PROTO(task0_func, params);

xTaskHandle task1_handle = NULL;
portTASK_FUNCTION_PROTO(task1_func, params);
#endif


MMP_INT DMA_Vram2Vram(MMP_UINT taskIndex, MMP_UINT32 srcOffset, MMP_UINT32 dstOffset)
{
    MMP_INT result = 0;
    MMP_UINT32 srcAddr = MMP_NULL;
    MMP_UINT32 dstAddr = MMP_NULL;
    MMP_UINT32 size = 1024*1024;
    MMP_DMA_CONTEXT dmaCtxt = 0;
    MMP_INT i = 0;
    MMP_UINT8* tmpSrcAddr = MMP_NULL;
    MMP_INT diff = 0;

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
    printf(" [Task %d] srcAddr = 0x%08X \n", taskIndex, srcAddr+srcOffset);
    printf(" [Task %d] dstAddr = 0x%08X \n", taskIndex, dstAddr+dstOffset);

    MMP_Sleep(1);
    // Process DMA.
    {
        const MMP_UINT32 attribList[] =
        {
            MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
            MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)(srcAddr+srcOffset),
            MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)(dstAddr+dstOffset),
            MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)size,
            MMP_DMA_ATTRIB_NONE
        };

        result = mmpDmaCreateContext(&dmaCtxt);
        if(result)
        {
            result = mmpDmaDestroyContext(dmaCtxt);
            if(result)
                goto end;
        }
        result = mmpDmaSetAttrib(dmaCtxt, attribList);
        if(result)
            goto end;
        result = mmpDmaFire(dmaCtxt);
        if(result)
            goto end;
        MMP_Sleep(1);
        result = mmpDmaWaitIdle(dmaCtxt);
        if(result)
            goto end;
        result = mmpDmaDestroyContext(dmaCtxt);
        if(result)
            goto end;
    }
    #if defined(__FREERTOS__)
    ithInvalidateDCacheRange((void*)(dstAddr+dstOffset), size);
    #endif

    // Compare data
    diff = memcmp((void*)(srcAddr+srcOffset), (void*)(dstAddr+dstOffset), size);
    printf(" [Task %d] memory compare diff = %d \n", taskIndex, diff);

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

void DoTest(MMP_UINT taskIndex)
{

#if 1
    printf("\n Case 4: source is 8-bits, destination is 32-bits algin! \n");
    DMA_Vram2Vram(taskIndex, 1, 0);

    printf("\n Case 5: source is 8-bits, destination is 8-bits algin! \n");
    DMA_Vram2Vram(taskIndex, 1, 1);

    printf("\n Case 6: source is 8-bits, destination is 16-bits algin! \n");
    DMA_Vram2Vram(taskIndex, 1, 2);

    printf("\n Case 7: source is 16-bits, destination is 32-bits algin! \n");
    DMA_Vram2Vram(taskIndex, 2, 0);

    printf("\n Case 8: source is 16-bits, destination is 8-bits algin! \n");
    DMA_Vram2Vram(taskIndex, 2, 1);

    printf("\n Case 9: source is 16-bits, destination is 16-bits algin! \n");
    DMA_Vram2Vram(taskIndex, 2, 2);
#endif
    printf("\n [Task %d] Case 1: source is 32-bits, destination is 32-bits algin! \n", taskIndex);
    DMA_Vram2Vram(taskIndex, 0, 0);

    printf("\n [Task %d] Case 2: source is 32-bits, destination is 8-bits algin! \n", taskIndex);
    DMA_Vram2Vram(taskIndex, 0, 1);

    printf("\n [Task %d] Case 3: source is 32-bits, destination is 16-bits algin! \n", taskIndex);
    DMA_Vram2Vram(taskIndex, 0, 2);
}


int main(int argc, char** argv)
{	
    MMP_INT result = 0;

#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

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

    vTaskStartScheduler();
#endif
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(task0_func, params)
{
    MMP_INT result = 0;

    printf("[DMA VRAM] Start Test!!\n");
    DoTest(0);

    while(1)
       MMP_Sleep(1);
}

portTASK_FUNCTION(task1_func, params)
{
    MMP_INT result = 0;

    printf("[DMA VRAM] Start Test!!\n");
    DoTest(1);

    while(1)
       MMP_Sleep(1);
}
#endif


