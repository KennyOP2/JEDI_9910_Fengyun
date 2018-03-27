/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "mmp_dma.h"
#include "mmp_cf.h"
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
extern F_DRIVER *mspro_initfunc(unsigned long driver_param);

int _f_poweron(void) 
{
    int ret;
    ret=f_initvolume(0, mspro_initfunc, F_AUTO_ASSIGN);

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
#define TEST_SIZE   2*1024//*1024

    MMP_INT result = 0;
    MMP_ULONG startTime = 0;
    MMP_ULONG duration = 0;
    MMP_INT i;
    static MMP_UINT8 buffer[256*512];

    result = mmpCfInitialize();
    if(result)
        goto end;

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*1)); i++)
    {
        result = mmpCfReadMultipleSector(512, 1, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO " Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*2)); i++)
    {
        result = mmpCfReadMultipleSector(512, 2, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "2 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*4)); i++)
    {
        result = mmpCfReadMultipleSector(512, 4, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "4 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*8)); i++)
    {
        result = mmpCfReadMultipleSector(512, 8, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "8 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*16)); i++)
    {
        result = mmpCfReadMultipleSector(512, 16, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "16 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*32)); i++)
    {
        result = mmpCfReadMultipleSector(512, 32, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "32 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*64)); i++)
    {
        result = mmpCfReadMultipleSector(512, 64, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "64 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*128)); i++)
    {
        result = mmpCfReadMultipleSector(512, 128, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "128 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    for(i=0; i<(TEST_SIZE/(512*256)); i++)
    {
        result = mmpCfReadMultipleSector(512, 256, buffer);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "256 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

end:
    mmpCfTerminate();
    while(1);
}

void TestReadWriteFile(void)
{
#define BUFSIZE        (512*1024)
#define FILE_SIZE_MB   4*2
#define FILE_SIZE_BYTE (FILE_SIZE_MB * BUFSIZE)

    MMP_INT result = 0;
    FN_FILE* file1 = MMP_NULL;
    MMP_UINT8* bufferW1 = MMP_NULL;
    MMP_UINT8* bufferR1 = MMP_NULL;
    MMP_UINT i = 0;
    MMP_UINT diff = 0;

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
#if 1
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
        printf("@ \n");
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
#endif

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
        printf("@ \n");
        memset(bufferR1, 0x0, BUFSIZE);
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
    LOG_INFO "Compare Data End!!\n" LOG_END
    if(file1)
    {
        f_close(file1);
        file1 = MMP_NULL;
    }

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
    LOG_INFO "End Test!!\n\n\n\n" LOG_END
}

static MMP_UINT8 buf1[516];
static MMP_UINT8 buf2[516];

void TestLowLevel(void)
{
    MMP_INT result=0;
#if 0 // ok
    MMP_UINT8* r1 = buf1;
    MMP_UINT8* r2 = buf2;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1;
    MMP_UINT8* r2 = buf2+1;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1;
    MMP_UINT8* r2 = buf2+2;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1+1;
    MMP_UINT8* r2 = buf2;
#endif
#if 1 // ok
    MMP_UINT8* r1 = buf1+1;
    MMP_UINT8* r2 = buf2+1;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1+1;
    MMP_UINT8* r2 = buf2+2;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1+2;
    MMP_UINT8* r2 = buf2;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1+2;
    MMP_UINT8* r2 = buf2+1;
#endif
#if 0 // ok
    MMP_UINT8* r1 = buf1+2;
    MMP_UINT8* r2 = buf2+2;
#endif
    MMP_INT diff;

    result = mmpCfInitialize();
    if(result)
        goto end;

    result = mmpCfReadMultipleSector(512, 1, r1);
    if(result)
        goto end;

    result = mmpCfWriteMultipleSector(512, 1, r1);
    if(result)
        goto end;

    result = mmpCfReadMultipleSector(512, 1, r2);
    if(result)
        goto end;

    diff = memcmp((void*)r1, (void*)r2, 512);
    if(diff)
        LOG_ERROR " compare data fail! diff %d \n", diff LOG_END

//===========
    result = mmpCfWriteMultipleSector(512, 1, r2);
    if(result)
        goto end;

    result = mmpCfReadMultipleSector(512, 1, r1);
    if(result)
        goto end;

    diff = memcmp((void*)r1, (void*)r2, 512);
    if(diff)
        LOG_ERROR " compare data fail! diff %d \n", diff LOG_END

	printf(" end...........\n");
	while(1);

end:
    mmpCfTerminate();
}

void DoTest(void)
{
	MMP_INT result = 0;
MEM_Allocate(4, 0); // FPGA test
	result = mmpDmaInitialize();
	if(result)
		goto end;

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

int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    ret = xTaskCreate(main_task_func, "sdtest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

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
#endif
