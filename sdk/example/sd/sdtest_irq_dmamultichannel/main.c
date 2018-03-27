/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "mmp_dma.h"
#include "mmp_sd.h"
#include "stdio.h"
#include "common/fat.h"
#include "common/common.h"
#include "chkdsk.h"
#include "config.h"
#include "pal/timer.h"

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

//#define TEST_PERFORMANCE
//#define TEST_LOW_LEVEL
#define TEST_READ_WRITE_FILE

extern void f_dotest(void);
extern F_DRIVER *sd_initfunc(unsigned long driver_param);

int _f_poweron(void)
{
    int ret;
    ret=f_initvolume(0, sd_initfunc, F_AUTO_ASSIGN);

    return ret;
}

int _f_poweroff(void)
{
    int ret;
    ret = f_delvolume(0);

    return ret;
}

void _f_dump(char *s) {
    printf("%s\n", s);
}

long _f_result(long testnum,long result) {
    printf("test number %d failed with error %d\n", testnum, result);
    return(testnum);
}


void dir_test(void) {
    FN_FIND find;
    if (!f_findfirst("A:/*.mp3",&find)) {
        do {
            printf ("filename:%s",find.filename);
            if (find.attr&F_ATTR_DIR) {
                printf (" directory\n");
            }
            else {
                printf (" size %d\n",find.filesize);
            }
        } while (!f_findnext(&find));
    }
}

void TestFS(void)
{
    MMP_INT result = 0;

    LOG_INFO "Start File System Test!!\n" LOG_END
    f_init();
    f_enterFS();
    result = _f_poweron();
    if(result)
    {
        LOG_ERROR "_f_poweron() return error code 0x%08X \n", result LOG_END
    }
    if(f_chdrive(0) != F_NO_ERROR)
    {
        printf("ERROR: Change drive failed.\n");
        return;
    }
    f_dotest();
    LOG_INFO "End File System Test!!\n" LOG_END
}

void TestPerformance(void)
{
#define TEST_SIZE   2*1024*1024

    MMP_INT result = 0;
    MMP_ULONG startTime = 0;
    MMP_ULONG duration = 0;
    MMP_INT i;
    static MMP_UINT8 buffer[256*512];

    result = mmpSdInitialize();
    if(result)
        goto end;

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*1)); i++)
    {
        result = mmpSdReadMultipleSector(512, 1, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO " Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*2)); i++)
    {
        result = mmpSdReadMultipleSector(512, 2, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "2 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*4)); i++)
    {
        result = mmpSdReadMultipleSector(512, 4, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "4 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*8)); i++)
    {
        result = mmpSdReadMultipleSector(512, 8, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "8 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*16)); i++)
    {
        result = mmpSdReadMultipleSector(512, 16, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "16 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*32)); i++)
    {
        result = mmpSdReadMultipleSector(512, 32, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "32 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*64)); i++)
    {
        result = mmpSdReadMultipleSector(512, 64, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "64 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*128)); i++)
    {
        result = mmpSdReadMultipleSector(512, 128, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "128 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*256)); i++)
    {
        result = mmpSdReadMultipleSector(512, 256, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "256 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

end:
    mmpSdTerminate();
    while(1);
}

void TestReadWriteFile(void)
{
#define BUFSIZE        (512*1024)
#define FILE_SIZE_MB   (4*2)
#define FILE_SIZE_BYTE (FILE_SIZE_MB * BUFSIZE)

    MMP_INT result = 0;
    FN_FILE* file1 = MMP_NULL;
    MMP_UINT8* bufferW1 = MMP_NULL;
    MMP_UINT8* bufferR1 = MMP_NULL;
    MMP_UINT i = 0;
    MMP_UINT diff = 0;

    LOG_INFO "\n" LOG_END
    LOG_INFO "Start R/W Test!!\n" LOG_END

    f_init();
	f_enterFS();

    result = _f_poweron();
    if(result)
    {
        LOG_ERROR "_f_poweron() return error code 0x%08X \n", result LOG_END
        goto end;
    }

    bufferW1 = (MMP_UINT8*)malloc(BUFSIZE);
    if(!bufferW1)
    {
        LOG_ERROR " Allocate memory for WRITE1 fail! \n" LOG_END
        goto end;
    }
    for(i=0; i<BUFSIZE; i++) /** fill pattern */
    {
        bufferW1[i] = i % 0x100;
    }

    //============================================
    /** write data */
    LOG_DATA "\n Open A:/test.dat for write!!\n" LOG_END
    file1 = f_open("A:/test.dat", "w");
    if(!file1)
    {
        LOG_ERROR "f_open() fail!! A:/test.dat \n" LOG_END
        goto end;
    }
	LOG_INFO " write data .. \n" LOG_END
    for(i=0; i<FILE_SIZE_MB; i++)
    {
		printf("@\n");
        result = f_write(bufferW1, 1, BUFSIZE, file1);
        if(result != BUFSIZE)
        {
            LOG_ERROR " real write size 0x%X != write size 0x%X \n", result, BUFSIZE LOG_END
            goto end;
        }
    }
    result = 0;
    f_flush(file1);
    LOG_INFO " Write finish!\n" LOG_END
    if(file1)
    {
        f_close(file1);
        file1 = MMP_NULL;
    }

only_read:
    //============================================
    /** Read and compare data from USB */
    LOG_INFO "\n" LOG_END
    LOG_INFO "Compare Data!!\n" LOG_END
    bufferR1 = (MMP_UINT8*)malloc(BUFSIZE);
    if(!bufferR1)
    {
        LOG_ERROR " Allocate memory for READ1 fail! \n" LOG_END
        goto end;
    }
    LOG_DATA "\n Open A:/test.dat for read!!\n" LOG_END
    file1 = f_open("A:/test.dat", "r");
    if(!file1)
    {
        LOG_ERROR "f_open() fail!! \n" LOG_END
        goto end;
    }
	LOG_INFO " read data from card.. \n" LOG_END
    for(i=0; i<FILE_SIZE_MB; i++)
    {
		printf("@\n");
        memset(bufferR1, 0x55, BUFSIZE);
        result = f_read(bufferR1, 1, BUFSIZE, file1);
        if(result != BUFSIZE)
        {
            LOG_ERROR " read back size 0x%X != read size 0x%X \n", result, BUFSIZE LOG_END
            goto end;
        }
        diff = memcmp((void*)bufferW1, (void*)bufferR1, BUFSIZE);
        if(diff)
            LOG_ERROR " diff = %d \n", diff LOG_END

        if(diff)
		{
			MMP_UINT32 j = 0;
			LOG_ERROR " i = %d \n", i LOG_END
			for(j=0; j<BUFSIZE; j++)
			{
				if(bufferW1[j] != bufferR1[j])
					LOG_ERROR " write buffer[%X] = %02X, read buffer1[%X] = %02X \n", j, bufferW1[j], j, bufferR1[j] LOG_END
			}
            while(1);
		}
    }
    result = 0;
    LOG_INFO "Compare Data End!!\n\n\n" LOG_END
    if(file1)
    {
        f_close(file1);
        file1 = MMP_NULL;
    }
	//goto only_read;

end:
    if(bufferW1)
    {
        free(bufferW1);
        bufferW1 = MMP_NULL;
    }
    if(bufferR1)
    {
        free(bufferR1);
        bufferR1 = MMP_NULL;
    }
    if(file1)
    {
        f_close(file1);
        file1 = MMP_NULL;
    }
    _f_poweroff();
    if(result)
    {
        LOG_ERROR " Error code = 0x%08X \n", result LOG_END
        while(1);
    }
    //while(1);
    LOG_INFO "End Test!!\n" LOG_END
}

void TestLowLevel(void)
{
    MMP_INT result=0;
    static MMP_UINT8 sd_r1[512];
    static MMP_UINT8 sd_r2[512];
    MMP_INT diff;

    result = mmpSdInitialize();
    if(result)
        goto end;

    result = mmpSdReadMultipleSector(512, 1, sd_r1);
    if(result)
        goto end;

    result = mmpSdWriteMultipleSector(512, 1, sd_r1);
    if(result)
        goto end;

    result = mmpSdReadMultipleSector(512, 1, sd_r2);
    if(result)
        goto end;

    diff = memcmp((void*)sd_r1, (void*)sd_r2, 512);
    if(diff)
        LOG_ERROR " sd compare data fail! diff %d \n", diff LOG_END

end:
    mmpSdTerminate();
}

#include "mem/mem.h"
void DoTest(void)
{
	MMP_INT result = 0;
MEM_Allocate(4, MEM_USER_SD); // FPGA test

    #if defined(TEST_PERFORMANCE)
    while(1)
        TestPerformance();
    #elif defined(TEST_LOW_LEVEL)
    while(1)
        TestLowLevel();
    #elif defined(TEST_READ_WRITE_FILE)
    while(1)
        TestReadWriteFile();
    #endif

end:
	return;
}

//============================================================================
//============== DMA =========================================================
#include "mmp_dma.h"
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#endif
#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

#define TEST_CHANNLE	7

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

#define USE_IRQ
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

//#define Kbytes  1024
#define Kbytes  256
static MMP_UINT32 testSize[8] = {1024*Kbytes, 128*Kbytes, 512*Kbytes, 64*Kbytes, 1024*Kbytes, 256*Kbytes, 128*Kbytes, 640*Kbytes};

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

void DmaDoTest(MMP_UINT taskIndex, MMP_UINT32 loopCnt)
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


int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;
    MMP_UINT32 i;

    HOST_GetChipVersion();
	mmpDmaInitialize();

    ret = xTaskCreate(main_task_func, "sdtest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 2, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

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

	for(i=0; i<TEST_CHANNLE; i++)
		isrEvent[i] = SYS_CreateEvent();

    vTaskStartScheduler();
#endif

    DoTest();

    return 1;
}


#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;

    DoTest();
}

portTASK_FUNCTION(task0_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(0, loopCnt++);
}

portTASK_FUNCTION(task1_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(1, loopCnt++);
}

portTASK_FUNCTION(task2_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(2, loopCnt++);
}

portTASK_FUNCTION(task3_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(3, loopCnt++);
}

portTASK_FUNCTION(task4_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(4, loopCnt++);
}

portTASK_FUNCTION(task5_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(5, loopCnt++);
}

portTASK_FUNCTION(task6_func, params)
{
    MMP_INT result = 0;
    MMP_UINT32 loopCnt = 0;

    printf("[DMA VRAM] Start Test!!\n");
    while(1)
	    DmaDoTest(6, loopCnt++);
}
#endif
