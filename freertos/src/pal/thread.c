// [S069@20070720]: Add global config
//#include "config/config.h"
// ~[S069@20070720]~

#include "config.h"
#include "pal/pal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timer.h"

// move from sdk\include\config\config.h
// [S069@20070720]: Performance Tuning Options
//#define S069_PERFORMANCE_STATISTIC      0
//#define S069_PERFORMANCE_FILEIO         0
//#define S069_PERFORMANCE_DUMPAVINFO     0
//#define S069_PERFORMANCE_CODE           1
//#define S069_PERFORMANCE_MSG            0
// ~[S069@20070720]: Performance Tuning Options~

#define SMTK_MAX(a, b) (((a) > (b)) ? (a) : (b))

//#if     (S069_PERFORMANCE_STATISTIC)
//int             gnInMJS     = 0;
//int             gnInMJD     = 0;
//int             gnInMJW     = 0;
//int             gnMJWST     = 0;
//PAL_CLOCK_T     gt0;
//#endif  /* (S069_PERFORMANCE_STATISTIC) */

//#ifdef ENABLE_USB_DEVICE
//#define MAX_TASK_COUNT 15
//#else
//#define MAX_TASK_COUNT 14
//#endif

#if defined(__FREERTOS__)
/* FIXME: Can not link FREERTOS include files because queue.h is conflict */
void dc_invalidate(void);
#endif

typedef struct PAL_TASK_TAG
{
    xTaskHandle     *handle;
    MMP_UINT        name;
    PAL_THREAD_PROC proc;
    void            *arg;
    MMP_BOOL        active;
} PAL_TASK;

// main task
#define MAIN_TASK_NAME              "MAIN"

// pal file task
#define PALFILE_TASK_NAME           "PALFILE"
#if (CONFIG_HAVE_NTFS)
    #define PALFILE_TASK_STACK_SIZE 10 * 1024
#else /* (CONFIG_HAVE_NTFS) */
    #define PALFILE_TASK_STACK_SIZE 4 * 1024
#endif /* (CONFIG_HAVE_NTFS) */
#define PALFILE_TASK_PRIORITY       (tskIDLE_PRIORITY + 3)
static xTaskHandle palfileTask;

// mps task
#define MPS_TASK_NAME               "MPS"
#define MPS_TASK_STACK_SIZE         2 * 1024
#define MPS_TASK_PRIORITY           (tskIDLE_PRIORITY + 2)
static xTaskHandle mpsTask;

// stream mux task
#define STREAM_MUX_TASK_NAME        "STREAM_MUX"
#define STREAM_MUX_TASK_STACK_SIZE  8 * 1024
#define STREAM_MUX_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
static xTaskHandle streamMuxTask;

// audio in task
#define AUDIO_IN_TASK_NAME          "AUDIO_IN"
#define AUDIO_IN_TASK_STACK_SIZE    8 * 1024
#define AUDIO_IN_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
static xTaskHandle audioInTask;

// video capture task
//#define VIDEO_CAPTURE_TASK_NAME         "VIDEO_CAPTURE"
//#define VIDEO_CAPTURE_TASK_STACK_SIZE   8*1024
//#define VIDEO_CAPTURE_TASK_PRIORITY     (tskIDLE_PRIORITY+2)
//static xTaskHandle  videoCaptureTask;

// video capture task
#define VIDEO_ENCODER_TASK_NAME       "VIDEO_ENCODER"
#define VIDEO_ENCODER_TASK_STACK_SIZE 8 * 1024
#define VIDEO_ENCODER_TASK_PRIORITY   (tskIDLE_PRIORITY + 2)
static xTaskHandle videoEncoderTask;

// storage task
#define STORAGE_TASK_NAME             "STORAGE"
#define STORAGE_TASK_STACK_SIZE       8 * 1024
#define STORAGE_TASK_PRIORITY         (tskIDLE_PRIORITY + 2)
static xTaskHandle storageTask;

// HDMI Loop Through task
#define HDMILOOPTHROUGH_TASK_NAME     "HDMI_LoopThrough"
#define HDMILOOPTHROUGH_STACK_SIZE    8 * 1024
#define HDMILOOPTHROUGH_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
static xTaskHandle HDMILoopThroughTask;

// USBEX task
#define USBEX_TASK_NAME               "USBEX"
#define USBEX_TASK_STACK_SIZE         8 * 1024
#define USBEX_TASK_PRIORITY           (tskIDLE_PRIORITY + 4)
static xTaskHandle usbexTask;

#ifdef ENABLE_USB_DEVICE
// usb device task
    #define USBDEVICE_TASK_NAME       "USBDEVICE"
    #define USBDEVICE_TASK_STACK_SIZE 8 * 1024
    #define USBDEVICE_TASK_PRIORITY   (tskIDLE_PRIORITY + 4)
static xTaskHandle usbDeviceTask;
#endif

// KEY MANAGER task
#define KEY_MANAGER_TASK_NAME         "KEY_MANAGER"
#define KEY_MANAGER_TASK_STACK_SIZE   8 * 1024
#define KEY_MANAGER_TASK_PRIORITY     (tskIDLE_PRIORITY + 2)
static xTaskHandle keyManagerTask;

// Static variables, all pre-created tasks
static PAL_TASK    palTasks[PAL_THREAD_COUNT];

void
PalSleep(
    MMP_ULONG ms)
{
    //#if (S069_PERFORMANCE_STATISTIC)
    //if (gnInMJS)
    //{
    //    gnInMJS++;
    //}
    //if (gnInMJW)
    //{
    //    gnInMJW++;
    //}
    //if (gnInMJD)
    //{
    //    gnInMJD++;
    //}
    //
    //if (gnInMJW)
    //{
    //    gt0 = PalGetClock();
    //}
    //#endif  /* (S069_PERFORMANCE_STATISTIC) */

    if (ms == 0)
    {
        taskYIELD();
    }
    else
    {
        while (ms >= 500)
        {
            vTaskDelay(timer_ms_to_xticks(500));
            ms -= 500;
        }
        vTaskDelay(timer_ms_to_xticks(ms));
    }

    //#if     (S069_PERFORMANCE_STATISTIC)
    //if (gnInMJW)
    //{
    //    gnMJWST = PalGetDuration(gt0);
    //}
    //#endif  /* (S069_PERFORMANCE_STATISTIC) */
}

static portTASK_FUNCTION(ThreadFunc, param)
{
    PAL_TASK *task;

    //LOG_ENTER "ThreadFunc(param=0x%X)\r\n", param LOG_END
    PalAssert(param);

    task = (PAL_TASK *) param;

    // to workaround hcc fat malloc TLS to cause memory fragment bug
    switch (task->name)
    {
    case PAL_THREAD_MAIN:
    case PAL_THREAD_PALFILE:
    //    case PAL_THREAD_MP3:
    //    case PAL_THREAD_TS_DEMUX:
    case PAL_THREAD_STORAGE:
    case PAL_THREAD_STREAM_MUX:
        //    case PAL_THREAD_SRC_READ:
        //    case PAL_THREAD_JPEG:
        //    case PAL_THREAD_TS_TTX:
        //    case PAL_THREAD_TS_SUB://for External subtitle
        //    case PAL_THREAD_TS_VIDEO:  // for video capture
        //        //PalDiskGetFreeSpace(0,MMP_NULL);
#if defined(HAVE_FAT) && !defined(BUILDING_BOOTLOADER)
        f_enterFS();
#endif
        break;
        //
        //    //case PAL_THREAD_STORAGE:
        //    //    f_enterFS();
        //    //    break;
    }

    for (;;)
    {
        PalAssert(task);
        PalAssert(task->handle);
        PalAssert(task->active == MMP_FALSE);

        vTaskSuspend(MMP_NULL);

        task->active = MMP_TRUE;

        PalAssert(task->proc);

        task->proc(task->arg);

        task->active = MMP_FALSE;
    }

    //LOG_LEAVE "ThreadFunc()\r\n" LOG_END
}

MMP_INT
PalThreadInitialize(
    void)
{
    signed portBASE_TYPE ret    = pdFAIL;
    PAL_TASK             *palTask;
    MMP_UINT             i      = 0;
    MMP_INT              result = 0;
    //LOG_ENTER "PalThreadInitialize()\r\n" LOG_END

    // Clear variables
    PalMemset(palTasks, 0, sizeof(palTasks));

    // main task
    palTask         = &palTasks[i++];
    palTask->handle = MMP_NULL;
    palTask->name   = PAL_THREAD_MAIN;
    palTask->active = MMP_TRUE;

    // pal file task
    PalMemset(&palfileTask, 0, sizeof(palfileTask));

    palTask         = &palTasks[i++];
    palTask->handle = &palfileTask;
    palTask->name   = PAL_THREAD_PALFILE;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const) PALFILE_TASK_NAME,
        PALFILE_TASK_STACK_SIZE,
        palTask,
        PALFILE_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        LOG_ERROR "xTaskCreate FAIL: %d\r\n", ret LOG_END
        PalAssert(!"xTaskCreate FAIL");
        goto end;
    }

    // mps task
    PalMemset(&mpsTask, 0, sizeof(mpsTask));

    palTask         = &palTasks[i++];
    palTask->handle = &mpsTask;
    palTask->name   = PAL_THREAD_MPS;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)MPS_TASK_NAME,
        MPS_TASK_STACK_SIZE,
        palTask,
        MPS_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate mps task fail");
        goto end;
    }

    // video capture task
    //    PalMemset(&videoCaptureTask, 0, sizeof(videoCaptureTask));
    //
    //    palTask         = &palTasks[i++];
    //    palTask->handle = &videoCaptureTask;
    //    palTask->name   = PAL_THREAD_VIDEO_CAPTURE;
    //    palTask->active = MMP_FALSE;
    //
    //    ret = xTaskCreate(
    //        ThreadFunc,
    //        (const signed portCHAR* const)VIDEO_CAPTURE_TASK_NAME,
    //        VIDEO_CAPTURE_TASK_STACK_SIZE,
    //        palTask,
    //        VIDEO_CAPTURE_TASK_PRIORITY,
    //        palTask->handle);
    //    if (pdFAIL == ret)
    //    {
    //        PalAssert(!"xTaskCreate video capture task fail");
    //        goto end;
    //    }

    // audio in task
    PalMemset(&audioInTask, 0, sizeof(audioInTask));

    palTask         = &palTasks[i++];
    palTask->handle = &audioInTask;
    palTask->name   = PAL_THREAD_AUDIO_IN;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)AUDIO_IN_TASK_NAME,
        AUDIO_IN_TASK_STACK_SIZE,
        palTask,
        AUDIO_IN_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate audio in task fail");
        goto end;
    }

    // video encoder task
    PalMemset(&videoEncoderTask, 0, sizeof(videoEncoderTask));

    palTask         = &palTasks[i++];
    palTask->handle = &videoEncoderTask;
    palTask->name   = PAL_THREAD_VIDEO_ENCODER;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)VIDEO_ENCODER_TASK_NAME,
        VIDEO_ENCODER_TASK_STACK_SIZE,
        palTask,
        VIDEO_ENCODER_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate video encoder task fail");
        goto end;
    }

    // stream mux task
    PalMemset(&streamMuxTask, 0, sizeof(streamMuxTask));

    palTask         = &palTasks[i++];
    palTask->handle = &streamMuxTask;
    palTask->name   = PAL_THREAD_STREAM_MUX;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)STREAM_MUX_TASK_NAME,
        STREAM_MUX_TASK_STACK_SIZE,
        palTask,
        STREAM_MUX_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate stream mux task fail");
        goto end;
    }

    // storage task
    PalMemset(&storageTask, 0, sizeof(storageTask));

    palTask         = &palTasks[i++];
    palTask->handle = &storageTask;
    palTask->name   = PAL_THREAD_STORAGE;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)STORAGE_TASK_NAME,
        STORAGE_TASK_STACK_SIZE,
        palTask,
        STORAGE_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate FAIL");
        goto end;
    }

    // HDMI Loop Through task
    PalMemset(&HDMILoopThroughTask, 0, sizeof(HDMILoopThroughTask));

    palTask         = &palTasks[i++];
    palTask->handle = &HDMILoopThroughTask;
    palTask->name   = PAL_THREAD_HDMILooPThrough;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)HDMILOOPTHROUGH_TASK_NAME,
        HDMILOOPTHROUGH_STACK_SIZE,
        palTask,
        HDMILOOPTHROUGH_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate FAIL");
        goto end;
    }

    // USBEX task
    PalMemset(&usbexTask, 0, sizeof(usbexTask));

    palTask         = &palTasks[i++];
    palTask->handle = &usbexTask;
    palTask->name   = PAL_THREAD_USBEX;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const) USBEX_TASK_NAME,
        USBEX_TASK_STACK_SIZE,
        palTask,
        USBEX_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        LOG_ERROR "xTaskCreate FAIL: %d\r\n", ret LOG_END
        PalAssert(!"xTaskCreate FAIL");
        goto end;
    }

#ifdef ENABLE_USB_DEVICE
    // usb device task
    PalMemset(&usbDeviceTask, 0, sizeof(usbDeviceTask));

    palTask         = &palTasks[i++];
    palTask->handle = &usbDeviceTask;
    palTask->name   = PAL_THREAD_USB_DEVICE;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)USBDEVICE_TASK_NAME,
        USBDEVICE_TASK_STACK_SIZE,
        palTask,
        USBDEVICE_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate usb device task fail");
        goto end;
    }
#endif

    // key manager task
    PalMemset(&keyManagerTask, 0, sizeof(keyManagerTask));

    palTask         = &palTasks[i++];
    palTask->handle = &keyManagerTask;
    palTask->name   = PAL_THREAD_KEY_MANAGER;
    palTask->active = MMP_FALSE;

    ret             = xTaskCreate(
        ThreadFunc,
        (const signed portCHAR *const)KEY_MANAGER_TASK_NAME,
        KEY_MANAGER_TASK_STACK_SIZE,
        palTask,
        KEY_MANAGER_TASK_PRIORITY,
        palTask->handle);
    if (pdFAIL == ret)
    {
        PalAssert(!"xTaskCreate usb device task fail");
        goto end;
    }

end:
    //LOG_LEAVE "PalThreadInitialize()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalThreadTerminate(
    void)
{
    MMP_UINT i;
    MMP_INT  result = 0;
    //LOG_ENTER "PalThreadTerminate()\r\n" LOG_END

    // Destroy all pre-created threads except main thread
    for (i = 1; i < PAL_THREAD_COUNT; i++)
    {
        PalAssert(palTasks[i].handle);

        //vTaskDelete(palTasks[i].handle);  // BUG: UNKNOWN WHY THIS FUNCTION IS NOT EXIST
    }

    //LOG_LEAVE "PalThreadTerminate()=%d\r\n", result LOG_END
    return result;
}

PAL_THREAD
PalCreateThread(
    MMP_INT name,
    PAL_THREAD_PROC proc,
    void *arg,
    MMP_ULONG stackSize,
    MMP_UINT priority)
{
    MMP_UINT i;
    PAL_TASK *task;

    //LOG_ENTER "CreateThread(proc=0x%X,arg=0x%X,stackSize=%d,priority=%d)\r\n",
    //    proc, arg, stackSize, priority LOG_END

    PalAssert(name != PAL_THREAD_MAIN);
    PalAssert(proc);

#if defined(__FREERTOS__)
    /* FIXME: use or32_invalidate_cache instead */
    dc_invalidate();
#endif

    // Find out the pre-created task by name except main task
    for (i = 1; i < PAL_THREAD_COUNT; i++)
    {
        if (name == palTasks[i].name)
        {
            task       = &palTasks[i];

            // Set the task's procedure and parameter
            task->proc = proc;
            task->arg  = arg;

            // Resume the pre-created task
            vTaskResume(*task->handle);
            goto end;
        }
    }

    // Out of task
    PalAssert(!"OUT OF TASK");
    task = MMP_NULL;

end:
    //LOG_LEAVE "PalCreateThread()=0x%X\r\n", task LOG_END
    return task;
}

MMP_INT
PalDestroyThread(
    PAL_THREAD thread)
{
    MMP_INT  result = 0;
    PAL_TASK *task;

    //LOG_ENTER "PalDestroyThread(thread=0x%X)\r\n", thread LOG_END

    task = (PAL_TASK *) thread;
    PalAssert(task);
    PalAssert(task->handle);
    //PalAssert(task->active == MMP_FALSE);

    //LOG_LEAVE "DestroyThread()=%d\r\n", result LOG_END
    return result;
}