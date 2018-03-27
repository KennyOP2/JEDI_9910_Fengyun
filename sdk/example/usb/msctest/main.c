/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "mmp_usbex.h"
#include "mmp_msc.h"
#include "stdio.h"
#include "common/fat.h"
#include "common/common.h"
#include "chkdsk.h"
#include "config.h"
#include "pal/pal.h"

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

//#define TEST_PERFORMANCE
#define TEST_READ_WRITE_FILE
//#define TEST_FILE_SYSTEM

void* ctxt;
MMP_UINT8 testLun = 0;

extern void f_dotest(void);
extern F_DRIVER *msc_initfunc(unsigned long driver_param);

int _f_poweron(void) 
{
    int ret;
    ret=f_initvolume(0, msc_initfunc, F_AUTO_ASSIGN);

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

void TestReadWriteFile(void)
{
#define BUFSIZE        (256*1024)
#define FILE_SIZE_MB   20*4
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
    for(i=0; i<FILE_SIZE_MB; i++)
    {
        LOG_INFO " write data .. \n" LOG_END
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
    for(i=0; i<FILE_SIZE_MB; i++)
    {
        memset(bufferR1, 0x0, BUFSIZE);
        LOG_INFO " read data from USB.. \n" LOG_END
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
            //while(1);
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
        //while(1);
    }
    LOG_INFO "End Test!!\n" LOG_END
}

static MMP_UINT8 buf_8[512*8];
static MMP_UINT8 buf_16[512*16];
static MMP_UINT8 buf_32[512*32];
static MMP_UINT8 buf_64[512*64];
static MMP_UINT8 buf_128[512*128];
static MMP_UINT8 buf_256[512*256];

void TestPerformance(void)
{
#define TEST_SIZE   2*1024*1024

    MMP_INT result = 0;
    MMP_ULONG startTime = 0;
    MMP_ULONG duration = 0;
    MMP_INT i, loop;

    result = mmpMscInitialize(ctxt, testLun);
    if(result)
        goto end;

printf(" =========== Read ============== \n");
    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*8));
    for(i=0; i<loop; i++)
    {
        result = mmpMscReadMultipleSector(ctxt, testLun, 512, 8, buf_8);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "8 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*16));
    for(i=0; i<loop; i++)
    {
        result = mmpMscReadMultipleSector(ctxt, testLun, 512, 16, buf_16);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "16 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*32));
    for(i=0; i<loop; i++)
    {
        result = mmpMscReadMultipleSector(ctxt, testLun, 512, 32, buf_32);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "32 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*64));
    for(i=0; i<loop; i++)
    {
        result = mmpMscReadMultipleSector(ctxt, testLun, 512, 64, buf_64);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "64 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*128));
    for(i=0; i<loop; i++)
    {
        result = mmpMscReadMultipleSector(ctxt, testLun, 512, 128, buf_128);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "128 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*256));
    for(i=0; i<loop; i++)
    {
        result = mmpMscReadMultipleSector(ctxt, testLun, 512, 256, buf_256);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "256 Read rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

printf(" =========== Write ============== \n");
    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*8));
    for(i=0; i<loop; i++)
    {
        result = mmpMscWriteMultipleSector(ctxt, testLun, 512, 8, buf_8);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "8 Write rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*16));
    for(i=0; i<loop; i++)
    {
        result = mmpMscWriteMultipleSector(ctxt, testLun, 512, 16, buf_16);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "16 Write rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*32));
    for(i=0; i<loop; i++)
    {
        result = mmpMscWriteMultipleSector(ctxt, testLun, 512, 32, buf_32);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "32 Write rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*64));
    for(i=0; i<loop; i++)
    {
        result = mmpMscWriteMultipleSector(ctxt, testLun, 512, 64, buf_64);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "64 Write rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*128));
    for(i=0; i<loop; i++)
    {
        result = mmpMscWriteMultipleSector(ctxt, testLun, 512, 128, buf_128);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "128 Write rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

    startTime = PalGetClock();
    loop = (TEST_SIZE/(512*256));
    for(i=0; i<loop; i++)
    {
        result = mmpMscWriteMultipleSector(ctxt, testLun, 512, 256, buf_256);
        if(result)
            goto end;
    }
    duration = PalGetDuration(startTime);
    LOG_INFO "256 Write rate: %f K bytes/sec, startTime = %d, duration = %d \n", (((double)(TEST_SIZE/1024))/duration)*1000, startTime, duration LOG_END

end:
    mmpMscTerminate(ctxt, testLun);
    while(1);
}



static MMP_INT
Initialize(void)
{
    signed portBASE_TYPE ret = pdFAIL;
    xTaskHandle usb_task = NULL;

	MMP_INT result = 0;

    //<<=============== These codes are in main.c Initialize() =================
    /** initialize usb HC driver, and register mass storage driver */
    result = mmpUsbExInitialize();
    if(result)
    {
        printf(" mmpUsbExInitialize() error 0x%08X \n", result);
        while(1);
    }

    /** This is HC driver task, and it will never be destroyed. */
    ret = xTaskCreate(USBEX_ThreadFunc, "usbex",
                        10*1024,
                        NULL, 
                        tskIDLE_PRIORITY + 3, 
                        &usb_task );
    if (pdFAIL == ret) 
    {
        printf(" Create USB host task fail~~ \n", result);
        while(1);
    }

    //================= Should add this in Initialize() ======================
    result = mmpMscDriverRegister();
    if(result)
    {
        LOG_ERROR " mmpMscDriverRegister() return 0x%08X \n", result LOG_END
        while(1);
    }

    return result;
}

static MMP_INT
MainLoop(
    void)
{
	MMP_INT result = 0;
    USB_DEVICE_INFO device_info = {0};
    MMP_INT usb_state = 0;
    MMP_BOOL connected = MMP_FALSE;
    MMP_UINT32 loopCnt = 0;

    f_init();
	f_enterFS();

    /** wait usb device connect */
    do 
    {
//next:
        result = mmpUsbExCheckDeviceState(USB0, &usb_state, &device_info);
        if(!result)
        {
            if(USB_DEVICE_CONNECT(usb_state))
            {
                if(USB_DEVICE_MSC(device_info.type))
                {
                    LOG_INFO " MSC is interted!!\n" LOG_END
                    ctxt = device_info.ctxt;
                    connected = MMP_TRUE;
                }
            }
            if(USB_DEVICE_DISCONNECT(usb_state))
            {
                if(USB_DEVICE_MSC(device_info.type))
                {
                    LOG_INFO " MSC device is disconnected!\n" LOG_END
                    connected = MMP_FALSE;
                    ctxt = MMP_NULL;
                }
            }
        }

        if(connected)
        {
            /** This is for testing DPF AP beheaver */
            MMP_UINT retries = 5;
find_lun:
            retries = 40;
            do {
                result = mmpMscGetStatus(ctxt, testLun);
                if(!result)
                    break;
                retries--;
            } while(retries);
        
            if(retries == 0)
            {
                printf(" Lun %d not found! \n\n", testLun);
                testLun++;
                if(testLun > 7)
                    testLun = 0;
                goto find_lun;
            }
            printf(" Found lun %d \n\n", testLun);
        
            #if defined(TEST_FILE_SYSTEM)
            TestFS();
            #elif defined(TEST_PERFORMANCE)
            TestPerformance();

            #if 0 // for test device removed
            do
            {
                result = mmpMscGetStatus(ctxt, testLun);
                if(result)
                    goto next;
            } while(1);
            #endif
            #elif defined(TEST_READ_WRITE_FILE)
            printf(" loopCnt %d \n", loopCnt++);
            TestReadWriteFile();
            #endif
        }
        PalSleep(1);
    } while(1);


	return;
}


static MMP_INT
Terminate(void)
{
    MMP_INT result = 0;

    return result;
}

void DoTest(void)
{
    MMP_INT result;

	ithIntrInit();

    result = Initialize();
    if (result)
        goto end;

    result = MainLoop();
    if (result)
        goto end;

    result = Terminate();
    if (result)
        goto end;

end:
    return;
}

int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    MMP_INT result=0;
    signed portBASE_TYPE ret = pdFAIL;

    HOST_GetChipVersion();

    ret = xTaskCreate(main_task_func, "sdtest_main",
        10*1024,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    //=======================Important!!!=========================================
    // For RISC version.
    // NOTE: Should mark freertos/src/pal/thread.c ThreadFunc()'s f_enterFS();
    //
    // Solution: Call f_init() before PalThreadInitialize()
    // Call file system functions can fix link error. =___=|||
    //============================================================================
/*
    f_init();
    if(PalThreadInitialize())
    {
        printf(" PalInitialize()() fail \n");
        while(1);
    }
*/
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
