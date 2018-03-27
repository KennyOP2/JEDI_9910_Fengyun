/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "sys/sys.h"
#include "host/host.h"
#include "mem/mem.h"
#include "mmp_dma.h"
#include "mmp_util.h"
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
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

static MMP_UINT32 lldAddrAlloc;
static MMP_UINT32 lldAddr;

static MMP_INLINE MMP_INT DMA_CreateLLD(
    MMP_UINT32 srcAddr, 
    MMP_UINT32 dstAddr, 
    MMP_INT size)
{
#define PAGE_SIZE   (4*1024)
#define DMA_LLD_MAX_SIZE    300

    MMP_INT result = 0;
    MMP_UINT32 lldSize=0, curSize=0, srcWidth, dstWidth;
    MMP_INT totalSize = size;
    static MMP_DMA_LLD sysLLD[DMA_LLD_MAX_SIZE];

    memset((void*)sysLLD, 0, sizeof(sysLLD));
#if defined(__FREERTOS__)
    lldAddr = (MMP_UINT32)sysLLD+sizeof(sysLLD[0]);
#else
    if(!lldAddrAlloc)
    {
        lldAddrAlloc = (MMP_UINT32)MEM_Allocate(sizeof(sysLLD), 0);
        lldAddr = (lldAddrAlloc+3) & ~3;
    }
#endif
    printf(" lldAddr = 0x%08X \n", lldAddr);
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
        sysLLD[lldSize].srcAddr = sysLLD[lldSize-1].srcAddr+curSize;
        sysLLD[lldSize].dstAddr = sysLLD[lldSize-1].dstAddr+curSize;
        /** current size in byte */
        if(totalSize >= PAGE_SIZE)
            curSize = PAGE_SIZE;
        else
            curSize = totalSize;
        /** src/dst width */
        if(curSize & 0x1)  /** size is 8-bits */
        {
            srcWidth = 1;
            dstWidth = 1;
        }
        else if(curSize & 0x2) /** size is 16-bits */
        {
            if(sysLLD[lldSize].srcAddr & 0x1)
                srcWidth = 1;
            else
                srcWidth = 2;

            if(sysLLD[lldSize].dstAddr & 0x1)
                dstWidth = 1;
            else
                dstWidth = 2;
        }
        else  /** size is 32-bits */
        {
            if(sysLLD[lldSize].srcAddr & 0x1)
                srcWidth = 1;
            else if(sysLLD[lldSize].srcAddr & 0x2)
                srcWidth = 2;
            else
                srcWidth = 4;

            if(sysLLD[lldSize].dstAddr & 0x1)
                dstWidth = 1;
            else if(sysLLD[lldSize].dstAddr & 0x2)
                dstWidth = 2;
            else
                dstWidth = 4;
        }
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
        sysLLD[lldSize].control |= (DMA_ADDR_CTRL_INC<<LLD_SRC_ADDR_CTRL_SHT)|(DMA_ADDR_CTRL_INC<<LLD_DST_ADDR_CTRL_SHT);
        /** current tx size */
        sysLLD[lldSize].txSize = curSize/srcWidth;
        /** next LLD pointer */
        sysLLD[lldSize].llp = lldAddr + lldSize*sizeof(sysLLD[0]);
        /** update totalSize */
        totalSize -= curSize;
    }
    sysLLD[lldSize].llp = 0;

#if 0
    {
        MMP_INT i=1;
        //for(i=1; i<=lldSize; i++)
        {
            printf(" srcAddr=0x%08X, dstAddr=0x%08X, llp=0x%08X, ctrl=0x%08X, txSize=0x%08X \n",
                sysLLD[i].srcAddr, sysLLD[i].dstAddr, sysLLD[i].llp, sysLLD[i].control, sysLLD[i].txSize);
        }
    }
#endif

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

MMP_INT DMA_Vram2Vram(MMP_UINT32 srcOffset, MMP_UINT32 dstOffset, MMP_UINT32 burstSize)
{
    MMP_INT result = 0;
    MMP_UINT32 srcAddr = MMP_NULL;
    MMP_UINT32 dstAddr = MMP_NULL;
    MMP_UINT32 size = 1024*1024;
    MMP_DMA_CONTEXT dmaCtxt = 0;
    MMP_UINT32 i = 0, firstTxSize, lldTxSize;
#if defined(__FREERTOS__)
    MMP_UINT8* tmpSrcAddr = MMP_NULL;

    srcAddr = (MMP_UINT32)MEM_Allocate((size+4), MEM_USER_SD);
    dstAddr = (MMP_UINT32)MEM_Allocate((size+4), MEM_USER_SD);
    tmpSrcAddr = (MMP_UINT8*)srcAddr;
    for(i=0; i<(size+4); i++)
        tmpSrcAddr[i] = (i & 0xFF);
#else
    MMP_UINT8* srcData = malloc(size+4);

    srcAddr = (MMP_UINT32)MEM_Allocate(4, MEM_USER_SD); // FPGA test
    srcAddr = (MMP_UINT32)MEM_Allocate((size+4), MEM_USER_SD);
    dstAddr = (MMP_UINT32)MEM_Allocate((size+4), MEM_USER_SD);
    for(i=0; i<(size+4); i++)
        srcData[i] = (MMP_UINT8)(i % 0x100);
    HOST_WriteBlockMemory(srcAddr, (MMP_UINT32)srcData, (size+4));
    free(srcData);  srcData = MMP_NULL;
#endif
    HOST_SetBlockMemory(dstAddr, 0x55, size);
    printf(" srcAddr = 0x%08X \n", srcAddr+srcOffset);
    printf(" dstAddr = 0x%08X \n", dstAddr+dstOffset);
    firstTxSize = PAGE_SIZE - ((srcAddr+srcOffset) % PAGE_SIZE);
    printf(" first TxSize = 0x%X \n", firstTxSize);
    lldTxSize = size - firstTxSize;
    DMA_CreateLLD((srcAddr+srcOffset+firstTxSize), (dstAddr+dstOffset+firstTxSize), lldTxSize);
 
    // Process DMA.
    {
        const MMP_UINT32 attribList[] =
        {
            MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
            MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)(srcAddr+srcOffset),
            MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)(dstAddr+dstOffset),
            MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)firstTxSize,
            MMP_DMA_ATTRIB_SRC_BURST_SIZE, (MMP_UINT32)burstSize,
            MMP_DMA_ATTRIB_LLD_ADDR, (MMP_UINT32)lldAddr,
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
    {
        MMP_UINT8* sysSrcAddr = malloc(size+4);
        MMP_UINT8* sysDstAddr = malloc(size+4);
        MMP_INT diff = 0;

        HOST_ReadBlockMemory((MMP_UINT32)sysSrcAddr, srcAddr, (size+4));
        HOST_ReadBlockMemory((MMP_UINT32)sysDstAddr, dstAddr, (size+4));
        diff = memcmp((void*)(sysSrcAddr+srcOffset), (void*)(sysDstAddr+dstOffset), size);
        printf(" memory compare diff = %d \n", diff);
#if 1
        if(diff)
        {
            MMP_UINT32 j = 0;
            MMP_UINT8* srcD = sysSrcAddr+srcOffset;
            MMP_UINT8* dstD = sysDstAddr+dstOffset;
            for(j=0; j<size; j++)
            {
                if(srcD[j] != dstD[j])
                    printf(" src[%X] = %02X, dst[%X] = %02X \n", j, srcD[j], j, dstD[j]);
            }
            while(1);
        }
#endif
#if 0
        {
            MMP_UINT8* data = sysSrcAddr+srcOffset;
            printf("src data:\n");
            for(i=0; i<512; i+=16)
            {
                printf(" 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n",
                    data[i],data[i+1],data[i+2],data[i+3],data[i+4],
                    data[i+5],data[i+6],data[i+7],data[i+8],data[i+9],
                    data[i+10],data[i+11],data[i+12],data[i+13],data[i+14],data[i+15]);
            }
            data = sysDstAddr+dstOffset;
            printf("\ndst data:\n");
                for(i=0; i<512; i+=16)
                {
                    printf(" 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n",
                        data[i],data[i+1],data[i+2],data[i+3],data[i+4],
                        data[i+5],data[i+6],data[i+7],data[i+8],data[i+9],
                        data[i+10],data[i+11],data[i+12],data[i+13],data[i+14],data[i+15]);
                }
        }
#endif

        free(sysSrcAddr);
        free(sysDstAddr);
    }

end:
    DMA_DestroyLLD();
    if(srcAddr)
        MEM_Release((void*)srcAddr);
    if(dstAddr)
        MEM_Release((void*)dstAddr);
    if(result)
    {
        printf(" DMA_Vram2Vram() return error code 0x%08X \n", result);
    }

    return result;
}
MMP_INT DMA_Memset(void)
{
    MMP_INT result = 0;
    MMP_UINT32 srcPattern = 0x01020304;
    MMP_UINT32* dstAddr = MMP_NULL;
    MMP_UINT32 size = 256;
    MMP_DMA_CONTEXT dmaCtxt = 0;
#if defined(__FREERTOS__)

    dstAddr = (MMP_UINT32*)malloc(size);
    if(!dstAddr)
    {
        printf("Allocate memory fail! \n");
        while(1);
    }
    memset((void*)dstAddr, 0x0, size);

    // Process DMA.
    {
        const MMP_UINT32 attribList[] =
        {
            MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_SET,
            MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)(&srcPattern),
            MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)dstAddr,
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
        result = mmpDmaWaitIdle(dmaCtxt);
        if(result)
            goto end;
        result = mmpDmaDestroyContext(dmaCtxt);
        if(result)
            goto end;
    }
    ithInvalidateDCacheRange((void*)dstAddr, size);

    // Compare data
    {
        MMP_UINT i = 0;
        for(i=0; i<(size/(sizeof(MMP_UINT32))); i++)
        {
            if(dstAddr[i] != srcPattern)
                printf(" dstAddr[%d] != 0x08X \n", i, srcPattern);
        }
        printf("Memset End! \n");
    }

end:
    if(dstAddr)
        free((void*)dstAddr);
    if(result)
    {
        printf(" DMA_Memset() return error code 0x%08X \n", result);
    }
#endif

    return result;
}

static MMP_UINT16 nclkDiv = 0;
static MMP_UINT32 loopCnt;
void DoTest(void)
{
    MMP_UINT32 burstSize[8] = {256,128,64,32,16,8,4,1};
    MMP_UINT32 i;

    loopCnt++;
    printf("loopCnt = %d \n", loopCnt);
#if 0 // nclk
    HOST_WriteRegisterMask(0x18, 0, 0x400);
    HOST_WriteRegisterMask(0x18, nclkDiv, 0x3FF);
    HOST_WriteRegisterMask(0x18, 1, 0x400);
    MMP_Sleep(1000);
    HOST_ReadRegister(0x18, &tmp);
    printf("reg 0x18 = 0x%04X \n", tmp);
    nclkDiv++;
    if(nclkDiv==10) nclkDiv = 0;
#endif
    for(i=0;i<8;i++)
    {
        printf(" burst size: %d \n", burstSize[i]);
        if(burstSize[i]==1)
        {
            printf("\n Case 1: source is 32-bits, destination is 32-bits align! \n");
            DMA_Vram2Vram(0, 0, burstSize[i]);

            printf("\n Case 2: source is 32-bits, destination is 8-bits align! \n");
            DMA_Vram2Vram(0, 1, burstSize[i]);

            printf("\n Case 3: source is 32-bits, destination is 16-bits align! \n");
            DMA_Vram2Vram(0, 2, burstSize[i]);
        }
        else
        {
            printf("\n Case 1: source is 32-bits, destination is 32-bits align! \n");
            DMA_Vram2Vram(0, 0, burstSize[i]);

            printf("\n Case 2: source is 32-bits, destination is 8-bits align! \n");
            DMA_Vram2Vram(0, 1, burstSize[i]);

            printf("\n Case 3: source is 32-bits, destination is 16-bits align! \n");
            DMA_Vram2Vram(0, 2, burstSize[i]);

            printf("\n Case 4: source is 8-bits, destination is 32-bits align! \n");
            DMA_Vram2Vram(1, 0, burstSize[i]);

            printf("\n Case 5: source is 8-bits, destination is 8-bits align! \n");
            DMA_Vram2Vram(1, 1, burstSize[i]);

            printf("\n Case 6: source is 8-bits, destination is 16-bits align! \n");
            DMA_Vram2Vram(1, 2, burstSize[i]);

            printf("\n Case 7: source is 16-bits, destination is 32-bits align! \n");
            DMA_Vram2Vram(2, 0, burstSize[i]);

            printf("\n Case 8: source is 16-bits, destination is 8-bits align! \n");
            DMA_Vram2Vram(2, 1, burstSize[i]);

            printf("\n Case 9: source is 16-bits, destination is 16-bits align! \n");
            DMA_Vram2Vram(2, 2, burstSize[i]);
        }
    }

    //DMA_Memset();


    //while(1);
}

int main(int argc, char** argv)
{	
    MMP_INT result = 0;

    mmpDmaInitialize();
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    HOST_GetChipVersion();

    ret = xTaskCreate(main_task_func, "dmavram_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#endif

    while(1)
        DoTest();
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;

    //HOST_WriteRegisterMask(0x168C, (0x1<<9), (0x1<<9));
    printf("[DMA VRAM] Start Test!!\n");
    while(1)
        DoTest();
}
#endif


