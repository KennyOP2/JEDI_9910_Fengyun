//#pragma GCC optimize ("O0")
#define WL_REFINED

#ifdef WL_REFINED

// check compiler option
    #if defined(CONFIG_HAVE_NTFS)
        #if !defined(HAVE_FAT)
            #error MUST enable FAT
        #endif
    #endif

    #if defined(CONFIG_HAVE_NTFS)
        #include "sxa/sxa_blkdev.h"
        #include "sxa_ntfs.h"
    #endif /* CONFIG_HAVE_NTFS */

    #define MK4CC(c0, c1, c2, c3)       \
    ((((unsigned int)c0) << 0) |   \
     (((unsigned int)c1) << 8) |   \
     (((unsigned int)c2) << 16) |   \
     (((unsigned int)c3) << 24) )

typedef enum _PAL_FSTAG
{
    PAL_FSTAG_UNKNOW = 0,
    PAL_FSTAG_HCC    = MK4CC('H', 'F', 'A', 'T'),
    PAL_FSTAG_NTFS   = MK4CC('N', 'T', 'F', 'S'),
} PAL_FSTAG;

typedef struct PAL_HANDLE_TAG
{
    PAL_FSTAG tag;
    void      *fp;
} PAL_HANDLE;

#else

    #ifdef CONFIG_HAVE_NTFS
        #include "sxa/sxa_blkdev.h"
        #include "sxa_ntfs.h"

        #define MK4CC(c0, c1, c2, c3)               \
    (                                       \
        (((unsigned int) (c0)) << 0) |   \
        (((unsigned int) (c1)) << 8) |   \
        (((unsigned int) (c2)) << 16) |   \
        (((unsigned int) (c3)) << 24)           \
    )

enum
{
    PAL_FSTAG_HCC  = MK4CC('H', 'F', 'A', 'T'),
    PAL_FSTAG_NTFS = MK4CC('N', 'T', 'F', 'S'),
};

struct PAL_HANDLE
{
    unsigned int tag;
    void         *fp;
};
    #endif /* CONFIG_HAVE_NTFS */
#endif

#include "config.h"
#include "pal/pal.h"

#ifdef HAVE_FAT
    #include "common/fat.h"
    #include "mmc/single/mmc_smedia.h"
#endif

#include "host/ahb.h"
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define TIMEOUT                      1000
#define THREAD_STACK_SIZE            4096
#define FILE_MSGQ_COUNT              1000

#ifdef __FREERTOS__
    #define APB_GPIO_OUTPUT_DATA_REG 0x68000000
    #define APB_GPIO_INPUT_DATA_REG  0x68000004
    #define APB_GPIO_DIRECT_REG      0x68000008
#else
    #define APB_GPIO_OUTPUT_DATA_REG 0x7800
    #define APB_GPIO_INPUT_DATA_REG  0x7804
    #define APB_GPIO_DIRECT_REG      0x7808
#endif

#define REG_BIT_XD_CARD_DETECT       (1u << 17)

#define REG_BIT_MMC_CARD_DETECT      (1u << 19)
#define REG_BIT_MMC_CARD_POWER       (1u << 21)

#define REG_BIT_MS_CARD_DETECT       (1u << 20)
#define REG_BIT_MS_CARD_POWER        (1u << 21)

/** measure Pal I/O duration **/
//#define FILE_OPERATION_TIME_MEASURE

//=============================================================================
//                              Macro Definition
//=============================================================================
#if defined(FILE_OPERATION_TIME_MEASURE)
    #define _timeMeasure(string, args ...) dbg_msg(1, string, ## args)
#else
    #define _timeMeasure(string, args ...)
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef void
(*CALLBACK)(
    void *file,
    MMP_ULONG result,
    void *arg);

typedef struct MSG_TAG
{
    void     *func;
    CALLBACK callback;
    void     *callback_arg;
    void     *arg1;
    void     *arg2;
    void     *arg3;
    void     *arg4;
} MSG;

//=============================================================================
//                              Global Data Definition
//=============================================================================
// static MSG msgBuf;
static PAL_MSGQ          queue;
static MMP_MUTEX         mutex;
static PAL_THREAD        thread;
static volatile MMP_BOOL destroyed;

#ifdef HAVE_FAT
    #define USE_HCC
F_DRIVER *m_pNORDriver        = 0;
F_DRIVER *m_pNORPRIVATEDriver = 0;
F_DRIVER *m_pNANDDriver       = 0;
F_DRIVER *m_pSDDriver         = 0;
F_DRIVER *m_pSD2Driver        = 0;
F_DRIVER *m_pCFDriver         = 0;
F_DRIVER *m_pMSDriver         = 0;
F_DRIVER *m_pMMCDriver        = 0;
F_DRIVER *m_pxDDriver         = 0;
F_DRIVER *m_pUSB0Driver       = 0;
F_DRIVER *m_pUSB1Driver       = 0;
F_DRIVER *m_pUSB2Driver       = 0;
F_DRIVER *m_pUSB3Driver       = 0;
F_DRIVER *m_pUSB4Driver       = 0;
F_DRIVER *m_pUSB5Driver       = 0;
F_DRIVER *m_pUSB6Driver       = 0;
F_DRIVER *m_pUSB7Driver       = 0;
#endif

#if !defined(ENABLE_DEBUG_MSG_OUT)
    #define XPrint(msg0, msg1, wmsg, newline)
    #define _printfBitStream(a, b, c)
    #define _printfString(a, b)

#else
void
XPrint(
    char *msg0,
    char *msg1,
    wchar_t *wmsg,
    int newline)
{
    wchar_t c;

    printf("%s%s ", msg0, msg1);
    if (wmsg)
    {
        while (c = *wmsg++)
        {
            printf("%c", c);
        }
    }
    if (newline)
    {
        printf("\r\n");
    }
}

// for debug
static MMP_INLINE void
_printfBitStream(
    MMP_UINT8 *inData,
    MMP_INT len,
    char      *type)
{
    do
    {
        printf(type, *inData++);
    } while (--len);
    printf("\n");
}

static MMP_INLINE void
_printfString(
    MMP_CHAR    *prefix,
    MMP_WCHAR   *str)
{
    int i;
    int len = PalWcslen(str);

    if (prefix)
        printf("%s", prefix);

    for (i = 0; i < len; i++)
        printf("%c", (char)str[i]);
    printf("\n");
}

#endif

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void *
FileThreadFunc(
    void *arg)
{
    MMP_ULONG  result = 0;
    MMP_UINT32 clock  = 0;

    LOG_ENTER "FileThreadFunc(arg=0x%X)\r\n", arg LOG_END

    for (;;)
    {
        MSG  msg;
        void *handle       = MMP_NULL;
        void *callback_arg = MMP_NULL;

begin:
        result = PalReadMsgQ(queue, &msg, PAL_MSGQ_INFINITE);
        if (result != 0)
        {
            LOG_ERROR "READ MSGQ FAIL:%d\r\n", result LOG_END
            goto begin;
        }

        if (msg.func == MMP_NULL) // Exit message
        {
            result    = 0;
            destroyed = MMP_TRUE;
            goto end;
        }

        callback_arg = msg.callback_arg;
#ifdef HAVE_FAT
        // Invoke the real function
        if (msg.func == (void *) PalFileRead)
        {
            handle = msg.arg4; // stream

            _timeMeasure("", clock = PalGetClock());

            result = PalFileRead(
                msg.arg1,               // buffer
                (MMP_SIZE_T) msg.arg2,  // size
                (MMP_SIZE_T) msg.arg3,  // count
                handle,
                MMP_NULL, MMP_NULL);

            _timeMeasure("read done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileWrite)
        {
            handle = msg.arg4; // stream
            _timeMeasure("", clock = PalGetClock());

            result = PalFileWrite(
                msg.arg1,               // buffer
                (MMP_SIZE_T) msg.arg2,  // size
                (MMP_SIZE_T) msg.arg3,  // count
                handle,
                MMP_NULL, MMP_NULL);

            _timeMeasure("write done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileFlush)
        {
            handle = msg.arg4; // stream
            result = PalFileFlush(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void *) PalFileSeekEx)
        {
            handle = msg.arg1; // stream
            _timeMeasure("", clock = PalGetClock());

            result = PalFileSeekEx(
                handle,
                (((unsigned long long) (unsigned int) msg.arg2) | ((((unsigned long long) (unsigned int) msg.arg3)) << 32)), // offset
                (MMP_INT) msg.arg4,                                                                                          // origin
                MMP_NULL, MMP_NULL);

            _timeMeasure("seekEx done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileSeek)
        {
            handle = msg.arg1; // stream
            _timeMeasure("", clock = PalGetClock());

            result = PalFileSeek(
                handle,
                (MMP_LONG) msg.arg2,    // offset
                (MMP_INT) msg.arg3,     // origin
                MMP_NULL, MMP_NULL);

            _timeMeasure("seek done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileTell)
        {
            handle = msg.arg1; // stream
            _timeMeasure("", clock = PalGetClock());

            result = PalFileTell(
                handle,
                MMP_NULL, MMP_NULL);

            _timeMeasure("file tell done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileEof)
        {
            handle = msg.arg1; // stream
            result = PalFileEof(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void *) PalFileOpen)
        {
            handle = PalFileOpen(
                (const MMP_CHAR *) msg.arg1, // filename
                (MMP_UINT) msg.arg2,         // mode
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void *) PalWFileOpen)
        {
            _timeMeasure("", clock = PalGetClock());

            handle = PalWFileOpen(
                (const MMP_WCHAR *) msg.arg1,   // filename
                (MMP_UINT) msg.arg2,            // mode
                MMP_NULL, MMP_NULL);

            _timeMeasure("open done: %u ms\n", PalGetDuration(clock));
            result = (MMP_INT) handle;
        }
        else if (msg.func == (void *) PalFileClose)
        {
            handle = msg.arg1; // stream
            _timeMeasure("", clock = PalGetClock());

            result = PalFileClose(
                handle,
                MMP_NULL, MMP_NULL);

            _timeMeasure("close done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileDelete)
        {
            handle = MMP_NULL;
            result = PalFileDelete(
                (const MMP_CHAR *) msg.arg1, // filename
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void *) PalWFileDelete)
        {
            handle = MMP_NULL;
            _timeMeasure("", clock = PalGetClock());

            result = PalWFileDelete(
                (const MMP_WCHAR *) msg.arg1, // filename
                MMP_NULL, MMP_NULL);

            _timeMeasure("delete done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalDiskGetFreeSpace)
        {
            handle = MMP_NULL;
            result = PalDiskGetFreeSpace(
                (const MMP_INT) msg.arg1, // dirnum
                (MMP_UINT32 *)   msg.arg2,
                (MMP_UINT32 *)   msg.arg3,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void *) PalDiskGetTotalSpace)
        {
            handle = MMP_NULL;
            _timeMeasure("", clock = PalGetClock());

            result = PalDiskGetTotalSpace(
                (const MMP_INT) msg.arg1,  // dirnum
                (MMP_UINT32 *)   msg.arg2,
                (MMP_UINT32 *)   msg.arg3,
                MMP_NULL, MMP_NULL);

            _timeMeasure("get total space done: %u ms\n", PalGetDuration(clock));
        }
        else if (msg.func == (void *) PalFileFindFirst)
        {
            handle = PalFileFindFirst(
                (const MMP_CHAR *) msg.arg1, // filename
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void *) PalWFileFindFirst)
        {
            handle = PalWFileFindFirst(
                (const MMP_WCHAR *) msg.arg1, // filename
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void *) PalFileFindNext)
        {
            handle = msg.arg1; // find
            result = PalFileFindNext(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void *) PalWFileFindNext)
        {
            handle = msg.arg1; // find
            result = PalWFileFindNext(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void *) PalFileFindClose)
        {
            handle = msg.arg1; // find
            result = PalFileFindClose(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "UNKNOWN MSG:0x%X\r\n", msg.func);
            PalAssert(!"UNKNOWN MSG");
            goto begin;
        }
        msg.callback(handle, result, callback_arg);
        PalSleep(1);
#else
        handle = MMP_NULL;
        result = 0;
        msg.callback(handle, result, callback_arg);
        PalSleep(1000);
#endif
        // Invoke the callback function
    }
end:
    LOG_LEAVE "FileThreadFunc()=%d\r\n", result LOG_END
    return (void *) result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_INT
PalFileInitialize(
    void)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s()\r\n", __FUNCTION__);

    // Crate mutex
    mutex = PalCreateMutex(PAL_MUTEX_FILE);
    if (!mutex)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "CREATE FILE MUTEX FAIL\r\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    // Crate message queue
    queue = PalCreateMsgQ(PAL_MSGQ_FILE, sizeof(MSG), FILE_MSGQ_COUNT);
    if (!queue)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "CREATE FILE MSGQ FAIL\r\n");
        PalDestroyMutex(mutex);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    // Create thread
    thread = PalCreateThread(PAL_THREAD_PALFILE,
                             FileThreadFunc,
                             MMP_NULL,
                             THREAD_STACK_SIZE,
                             PAL_THREAD_PRIORITY_NORMAL);
    if (!queue)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "CREATE FILE THREAD FAIL\r\n");
        PalDestroyMsgQ(queue);
        PalDestroyMutex(mutex);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    destroyed = MMP_FALSE;

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else
    MMP_INT result = MMP_SUCCESS;
    LOG_ENTER "PalFileInitialize()\r\n" LOG_END

    // Crate mutex
    mutex = PalCreateMutex(PAL_MUTEX_FILE);
    if (!mutex)
    {
        LOG_ERROR "CREATE FILE MUTEX FAIL\r\n" LOG_END
        result = MMP_RESULT_ERROR;
        goto end;
    }

    // Crate message queue
    queue = PalCreateMsgQ(PAL_MSGQ_FILE);
    if (!queue)
    {
        LOG_ERROR "CREATE FILE MSGQ FAIL\r\n" LOG_END
        PalDestroyMutex(mutex);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    // Create thread
    thread = PalCreateThread(PAL_THREAD_PALFILE, FileThreadFunc, MMP_NULL,
                             THREAD_STACK_SIZE, PAL_THREAD_PRIORITY_NORMAL);
    if (!queue)
    {
        LOG_ERROR "CREATE FILE THREAD FAIL\r\n" LOG_END
        PalDestroyMsgQ(queue);
        PalDestroyMutex(mutex);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    destroyed = MMP_FALSE;

end:
    LOG_LEAVE "PalFileInitialize()=%d\r\n", result LOG_END
    return result;
#endif
}

MMP_INT
PalFileTerminate(
    void)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s()\r\n", __FUNCTION__);

    // Write quit message to queue
    msgBuf.func = MMP_NULL;
    result      = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "WRITE EXIT MSG FAIL\r\n");
        goto end;
    }

    // Wait file thread finished
    while (destroyed == MMP_FALSE)
        PalSleep(5);

    result = PalDestroyThread(thread);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "DESTROY FILE THREAD FAIL\r\n");
        goto end;
    }

    result = PalDestroyMsgQ(queue);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "DESTROY FILE MSGQ FAIL\r\n");
        goto end;
    }

    result = PalDestroyMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "DESTROY FILE MUTEX FAIL\r\n");
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else
    MMP_INT result;
    LOG_ENTER "PalFileTerminate()\r\n" LOG_END

    // Write quit message to queue
    msgBuf.func = MMP_NULL;
    result      = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        LOG_ERROR "WRITE EXIT MSG FAIL\r\n" LOG_END
        goto end;
    }

    // Wait file thread finished
    while (destroyed == MMP_FALSE)
        PalSleep(1);

    result = PalDestroyThread(thread);
    if (result != 0)
    {
        LOG_ERROR "DESTROY FILE THREAD FAIL\r\n" LOG_END
        goto end;
    }

    result = PalDestroyMsgQ(queue);
    if (result != 0)
    {
        LOG_ERROR "DESTROY FILE MSGQ FAIL\r\n" LOG_END
        goto end;
    }

    result = PalDestroyMutex(mutex);
    if (result != 0)
    {
        LOG_ERROR "DESTROY FILE MUTEX FAIL\r\n" LOG_END
        goto end;
    }

end:
    LOG_LEAVE "PalFileTerminate()=%d\r\n", result LOG_END
    return result;
#endif
}

PAL_FILE *
PalFileOpen(
    const MMP_CHAR *filename,
    MMP_UINT mode,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;
    F_FILE  *file  = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(fname=%s,mode=%d,cb=0x%X)\r\n", __FUNCTION__, filename, mode, callback);

    if (!callback) // Sync mode
    {
        switch (mode)
        {
        case PAL_FILE_RB:   file = f_open(filename, "r");   break;
        case PAL_FILE_WB:   file = f_open(filename, "w");   break;
        case PAL_FILE_AB:   file = f_open(filename, "a");   break;
        case PAL_FILE_RBP:  file = f_open(filename, "r+");  break;
        case PAL_FILE_WBP:  file = f_open(filename, "w+");  break;
        case PAL_FILE_ABP:  file = f_open(filename, "a+");  break;
        default:
            break;
        }
        //PalAssert(file);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        file = (PAL_FILE *) result;
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileOpen;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;
    msgBuf.arg2         = (void *) mode;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        //file = (PAL_FILE*) result;
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() error (%d)!! ", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        //file = (PAL_FILE*) result;
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() error (%d)!! ", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return file;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return MMP_NULL;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result;
    F_FILE  *file;
    LOG_ENTER "PalFileOpen(filename=%s,mode=%d,callback=0x%X)\r\n",
    filename, mode, callback LOG_END

    if (!callback) // Sync mode
    {
        switch (mode)
        {
        case PAL_FILE_RB:
            file = f_open(filename, "r");
            break;

        case PAL_FILE_WB:
            file = f_open(filename, "w");
            break;

        case PAL_FILE_AB:
            file = f_open(filename, "a");
            break;

        case PAL_FILE_RBP:
            file = f_open(filename, "r+");
            break;

        case PAL_FILE_WBP:
            file = f_open(filename, "w+");
            break;

        case PAL_FILE_ABP:
            file = f_open(filename, "a+");
            break;

        default:
            file = MMP_NULL;
        }
        //PalAssert(file);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileOpen;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;
    msgBuf.arg2         = (void *) mode;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        goto end;
    }

    file = MMP_NULL;

end:
    LOG_LEAVE "PalFileOpen()=0x%X\r\n", file LOG_END
    return file;
    #else
    return MMP_NULL;
    #endif
#endif
}

PAL_FILE *
PalWFileOpen(
    const MMP_WCHAR *filename,
    MMP_UINT mode,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    F_FILE     *file          = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(mode=%d,cb=0x%X)\r\n", __FUNCTION__, mode, callback);

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        if (!(pt_pal_handle = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PAL_HANDLE))))
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Alloc fail !! ");
            result = MMP_RESULT_ERROR;
            goto end;
        }

        // initial handle
        pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
        pt_pal_handle->fp  = MMP_NULL;

        if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
        {
            switch (mode)
            {
            default:
                result               = MMP_RESULT_ERROR;
                break;

            case PAL_FILE_RB:   mode = SXANTFS_OPENMODE_R;          break;
            case PAL_FILE_RBP:  mode = SXANTFS_OPENMODE_RPLUS;      break;
            case PAL_FILE_WB:   mode = SXANTFS_OPENMODE_W;          break;
            case PAL_FILE_WBP:  mode = SXANTFS_OPENMODE_WPLUS;      break;
            case PAL_FILE_AB:   mode = SXANTFS_OPENMODE_A;          break;
            case PAL_FILE_ABP:  mode = SXANTFS_OPENMODE_APLUS;      break;
            }

            pt_pal_handle->tag = PAL_FSTAG_NTFS;

            if ((pt_pal_handle->fp = sxa_ntfs_fopen(filename, mode)) == MMP_NULL)
                result = MMP_RESULT_ERROR;
        }
        else
        {
            switch (mode)
            {
            case PAL_FILE_RB:   pt_pal_handle->fp = f_wopen(filename, L"r");     break;
            case PAL_FILE_WB:   pt_pal_handle->fp = f_wopen(filename, L"w");     break;
            case PAL_FILE_AB:   pt_pal_handle->fp = f_wopen(filename, L"a");     break;
            case PAL_FILE_RBP:  pt_pal_handle->fp = f_wopen(filename, L"r+");    break;
            case PAL_FILE_WBP:  pt_pal_handle->fp = f_wopen(filename, L"w+");    break;
            case PAL_FILE_ABP:  pt_pal_handle->fp = f_wopen(filename, L"a+");    break;

            default:
                result                            = MMP_RESULT_ERROR;
                break;
            }

            pt_pal_handle->tag = PAL_FSTAG_HCC;

            if (pt_pal_handle->fp == MMP_NULL)
                result = MMP_RESULT_ERROR;
        }
        #else
        switch (mode)
        {
        case PAL_FILE_RB:   file = f_wopen(filename, L"r");     break;
        case PAL_FILE_WB:   file = f_wopen(filename, L"w");     break;
        case PAL_FILE_AB:   file = f_wopen(filename, L"a");     break;
        case PAL_FILE_RBP:  file = f_wopen(filename, L"r+");    break;
        case PAL_FILE_WBP:  file = f_wopen(filename, L"w+");    break;
        case PAL_FILE_ABP:  file = f_wopen(filename, L"a+");    break;

        default:
            result               = MMP_RESULT_ERROR;
            break;
        }
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileOpen;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;
    msgBuf.arg2         = (void *) mode;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        //file = (PAL_FILE*) result;
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() error (%d)!! ", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        //file = (PAL_FILE*) result;
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() error (%d)!! ", result);
        goto end;
    }

end:

        #if defined(CONFIG_HAVE_NTFS)
    if (result != MMP_SUCCESS)
    {
        if (pt_pal_handle)
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
        pt_pal_handle = MMP_NULL;

        _printfString("\topen fail: ", (MMP_WCHAR *)filename);
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    }

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return pt_pal_handle;
        #else
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return file;
        #endif

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return MMP_NULL;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT           result;
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle = 0;
        #endif /* (CONFIG_HAVE_NTFS) */
    F_FILE            *file;
    LOG_ENTER "PalWFileOpen(filename=0x%X,mode=%d,callback=0x%X)\r\n",
    filename, mode, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        if (!(pt_pal_handle = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(*pt_pal_handle))))
        {
            goto end;
        }
        memset(pt_pal_handle, 0, sizeof(*pt_pal_handle));

        if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
        {
            switch (mode)
            {
            default:
                goto l_err;

            case PAL_FILE_RB:
                mode = SXANTFS_OPENMODE_R;
                break;

            case PAL_FILE_RBP:
                mode = SXANTFS_OPENMODE_RPLUS;
                break;

            case PAL_FILE_WB:
                mode = SXANTFS_OPENMODE_W;
                break;

            case PAL_FILE_WBP:
                mode = SXANTFS_OPENMODE_WPLUS;
                break;

            case PAL_FILE_AB:
                mode = SXANTFS_OPENMODE_A;
                break;

            case PAL_FILE_ABP:
                mode = SXANTFS_OPENMODE_APLUS;
                break;
            }

            if (!(pt_pal_handle->fp = sxa_ntfs_fopen(filename, mode)))
            {
                goto l_err;
            }

            pt_pal_handle->tag = PAL_FSTAG_NTFS;
            goto end;

l_err:
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
            pt_pal_handle = 0;

            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */

        switch (mode)
        {
        case PAL_FILE_RB:
            file = f_wopen(filename, L"r");
            break;

        case PAL_FILE_WB:
            file = f_wopen(filename, L"w");
            break;

        case PAL_FILE_AB:
            file = f_wopen(filename, L"a");
            break;

        case PAL_FILE_RBP:
            file = f_wopen(filename, L"r+");
            break;

        case PAL_FILE_WBP:
            file = f_wopen(filename, L"w+");
            break;

        case PAL_FILE_ABP:
            file = f_wopen(filename, L"a+");
            break;

        default:
            file = MMP_NULL;
        }

        #if (CONFIG_HAVE_NTFS)
        if (file)
        {
            pt_pal_handle->tag = PAL_FSTAG_HCC;
            pt_pal_handle->fp  = file;
        }
        else
        {
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
            pt_pal_handle = 0;
        }
        #endif /* (CONFIG_HAVE_NTFS) */

        //PalAssert(file);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileOpen;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;
    msgBuf.arg2         = (void *) mode;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        file = (PAL_FILE *) result;
        goto end;
    }

    file = MMP_NULL;

end:

        #if (CONFIG_HAVE_NTFS)
    file = pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */

    LOG_LEAVE "PalWFileOpen()=0x%X\r\n", file LOG_END

    return file;
    #else
    return MMP_NULL;
    #endif
#endif
}

MMP_INT
PalFileClose(
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(stream=0x%X,cnb=0x%X)\r\n", __FUNCTION__, stream, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback) // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            result = sxa_ntfs_fclose((SXA_NTFS_HANDLE)pt_pal_handle->fp);
            break;

        case PAL_FSTAG_HCC:
            result = f_close((PAL_FILE *)pt_pal_handle->fp);
            if (result != F_NO_ERROR)
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE CLOSE ERROR: %d\r\n", result);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }

        pt_pal_handle->tag = PAL_FSTAG_UNKNOW;

        PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
        pt_pal_handle      = MMP_NULL;
        #else
        result             = f_close(stream);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE CLOSE ERROR: %d\r\n", result);
            //PalAssert(!"FILE CLOSE ERROR");
        }
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail !!");
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileClose;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail !!");
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail !!");
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
    unsigned int      tag;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT           result;

    LOG_ENTER "PalFileClose(stream=0x%X,callback=0x%X)\r\n",
    stream, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;
        tag           = pt_pal_handle->tag;
            #if (0)
        if ((tag != PAL_FSTAG_NTFS) && (tag != PAL_FSTAG_HCC))
        {
            printf("[X] tag=%d, halted!!!\r\n", tag);
            while (1);
            result = 0;
            goto end;
        }
        pt_pal_handle->tag = 0;
            #endif
        PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
        if (tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_fclose(stream);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */

        result = f_close(stream);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FILE CLOSE ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FILE CLOSE ERROR");
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileClose;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileClose()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_SIZE_T
PalFileRead(
    void *buffer,
    MMP_SIZE_T size,
    MMP_SIZE_T count,
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    MMP_SIZE_T realSize       = 0;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(buf=0x%X,size=%d,count=%d,stream=0x%X,cb=0x%X)\r\n",
            __FUNCTION__, buffer, size, count, stream, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            realSize = sxa_ntfs_fread(buffer, size, count, (SXA_NTFS_HANDLE)pt_pal_handle->fp);
            break;

        case PAL_FSTAG_HCC:
            realSize = f_read(buffer, size, count, (PAL_FILE *)pt_pal_handle->fp);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }
        #else
        realSize = f_read(buffer, size, count, stream);
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileRead;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) buffer;
    msgBuf.arg2         = (void *) size;
    msgBuf.arg3         = (void *) count;
    msgBuf.arg4         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return realSize;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_SIZE_T        result;
    LOG_ENTER "PalFileRead(buffer=0x%X,size=%d,count=%d,stream=0x%X,callback=0x%X)\r\n",
    buffer, size, count, stream, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;

        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_fread(buffer, size, count, stream);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */

        result = f_read(buffer, size, count, stream);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileRead;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) buffer;
    msgBuf.arg2         = (void *) size;
    msgBuf.arg3         = (void *) count;
    msgBuf.arg4         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileRead()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_SIZE_T
PalFileWrite(
    const void *buffer,
    MMP_SIZE_T size,
    MMP_SIZE_T count,
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    MMP_SIZE_T realSize       = 0;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(buf=0x%X,size=%d,count=%d,stream=0x%X,cb=0x%X)\r\n",
            __FUNCTION__, buffer, size, count, stream, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            realSize = sxa_ntfs_fwrite(buffer, size, count, (SXA_NTFS_HANDLE)pt_pal_handle->fp);
            break;

        case PAL_FSTAG_HCC:
            realSize = f_write(buffer, size, count, (PAL_FILE *)pt_pal_handle->fp);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }
        #else
        realSize = f_write(buffer, size, count, stream);
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileWrite;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) buffer;
    msgBuf.arg2         = (void *) size;
    msgBuf.arg3         = (void *) count;
    msgBuf.arg4         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return realSize;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_SIZE_T        result;
    LOG_ENTER "PalFileWrite(buffer=0x%X,size=%d,count=%d,stream=0x%X,callback=0x%X)\r\n",
    buffer, size, count, stream, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;

        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_fwrite(buffer, size, count, stream);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_write(buffer, size, count, stream);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        printf("file.c(%d), file write prepare faile\n", __LINE__);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileWrite;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) buffer;
    msgBuf.arg2         = (void *) size;
    msgBuf.arg3         = (void *) count;
    msgBuf.arg4         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        printf("file.c(%d), file write prepare is fail, error: %u\n", __LINE__, result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        printf("file.c(%d), file write prepare faile\n", __LINE__);
        goto end;
    }

end:
    LOG_LEAVE "PalFileWrite()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalFileFlush(
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(stream=0x%X, cb=0x%X)\r\n", __FUNCTION__, stream, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            result = sxa_ntfs_fflush((SXA_NTFS_HANDLE)pt_pal_handle->fp);
            break;

        case PAL_FSTAG_HCC:
            result = f_flush((PAL_FILE *)pt_pal_handle->fp);
            if (result != F_NO_ERROR)
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE FLUSH ERROR: %d\r\n", result);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }
        #else
        result = f_flush(stream);
        if (result != F_NO_ERROR)
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE FLUSH ERROR: %d\r\n", result);
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFlush;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = MMP_NULL;
    msgBuf.arg2         = MMP_NULL;
    msgBuf.arg3         = MMP_NULL;
    msgBuf.arg4         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT32         result;

    LOG_ENTER "PalFileFlush(stream=0x%X, callback=0x%X)\r\n",
    stream, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;

        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_fflush(stream);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_flush(stream);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFlush;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = MMP_NULL;
    msgBuf.arg2         = MMP_NULL;
    msgBuf.arg3         = MMP_NULL;
    msgBuf.arg4         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileFlush()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalFileSeekEx(
    PAL_FILE *stream,
    MMP_INT64 offset,
    MMP_INT origin,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(stream=0x%X,offset=%lld,org=%d,cb=0x%X)\r\n",
            __FUNCTION__, stream, offset, origin, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            result = sxa_ntfs_fseek((SXA_NTFS_HANDLE)pt_pal_handle->fp, offset, origin);
            break;

        case PAL_FSTAG_HCC:
            result = f_seek((PAL_FILE *)pt_pal_handle->fp, (MMP_LONG)offset, origin);
            if (result != F_NO_ERROR)
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE SEEK ERROR: %d\r\n", result);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }
        #else
        result = f_seek(stream, (MMP_LONG)offset, origin);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE SEEK ERROR: %d\r\n", result);
            //PalAssert(!"FILE SEEK ERROR");
        }
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileSeekEx;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) stream;
    msgBuf.arg2         = (void *) (unsigned int) (((unsigned long long) offset) >> 0);
    msgBuf.arg3         = (void *) (unsigned int) (((unsigned long long) offset) >> 32);
    msgBuf.arg4         = (void *) origin;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT           result;
    LOG_ENTER "PalFileSeek(stream=0x%X,offset=%lld,origin=%d,callback=0x%X)\r\n",
    stream, offset, origin, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;

        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_fseek(stream, offset, origin);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_seek(stream, offset, origin);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FILE SEEK ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FILE SEEK ERROR");
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileSeekEx;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) stream;
    msgBuf.arg2         = (void *) (unsigned int) (((unsigned long long) offset) >> 0);
    msgBuf.arg3         = (void *) (unsigned int) (((unsigned long long) offset) >> 32);
    msgBuf.arg4         = (void *) origin;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileSeek()=%lld\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalFileSeek(
    PAL_FILE *stream,
    MMP_LONG offset,
    MMP_INT origin,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    return (MMP_INT) PalFileSeekEx(stream, offset, origin, callback, callback_arg);
}

MMP_INT64
PalFileTellEx(
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    MMP_INT64  realPos        = 0LL;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(stream=0x%X,cb=0x%X)\r\n", __FUNCTION__, stream, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback) // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            realPos = sxa_ntfs_ftell((SXA_NTFS_HANDLE)pt_pal_handle->fp);
            if (realPos < 0)
            {
                realPos = 0;
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "sxa_ntfs_ftell() fail !!");
            }
            break;

        case PAL_FSTAG_HCC:
            realPos = f_tell((PAL_FILE *)pt_pal_handle->fp);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }

        #else
        realPos = f_tell(stream);
        #endif
        goto end;
    }

    result = MMP_RESULT_ERROR;
    PalAssert(!"Async. mode not supported for PalFileTellEx()");

    // Async mode not supported yet!!!
    realPos = 0;

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return realPos;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT64         result;
    LOG_ENTER "PalFileTell(stream=0x%X,callback=0x%X)\r\n",
    stream, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;

        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_ftell(stream);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_tell(stream);
        goto end;
    }
    else
    {
        PalAssert(!"Async. mode not supported for PalFileTellEx()");
    }

    // Async mode not supported yet!!!
    result = -1;

end:
    LOG_LEAVE "PalFileTell()=%lld\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_LONG
PalFileTell(
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    return (MMP_LONG) PalFileTellEx(stream, callback, callback_arg);
}

MMP_INT
PalFileEof(
    PAL_FILE *stream,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(stream=0x%X,cb=0x%X)\r\n", __FUNCTION__, stream, callback);

    if (stream == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback) // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)stream;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            result = sxa_ntfs_feof((SXA_NTFS_HANDLE)pt_pal_handle->fp);
            break;

        case PAL_FSTAG_HCC:
            result = f_eof((PAL_FILE *)pt_pal_handle->fp);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }

        #else
        result = f_eof(stream);
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileEof;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!", result);
        goto end;
    }

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT           result;
    LOG_ENTER "PalFileEof(stream=0x%X,callback=0x%X)\r\n",
    stream, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) stream;
        stream        = (PAL_FILE *) pt_pal_handle->fp;

        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_feof(stream);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_eof(stream);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileEof;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) stream;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileEof()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalFileDelete(
    const MMP_CHAR *filename,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(filename=%s,cb=0x%X)\r\n", __FUNCTION__, filename, callback);

    if (!callback) // Sync mode
    {
        result = f_delete(filename);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileDelete;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result;
    LOG_ENTER "PalFileDelete(filename=%s,callback=0x%X)\r\n",
    filename, callback LOG_END

    if (!callback) // Sync mode
    {
        result = f_delete(filename);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileDelete;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileDelete()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalWFileDelete(
    const MMP_WCHAR *filename,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(cb=0x%X)\r\n", __FUNCTION__, callback);

    if (!callback) // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
        {
            result = sxa_ntfs_unlink(filename);
        }
        else
        {
            result = f_wdelete(filename);
            if (result != F_NO_ERROR)
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE DELETE FAIL (%d)!!\n", result);
        }
        #else
        result = f_wdelete(filename);
        if (result != F_NO_ERROR)
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FILE DELETE FAIL (%d)!!\n", result);
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileDelete;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result;
    LOG_ENTER "PalWFileDelete(filename=0x%X,callback=0x%X)\r\n",
    filename, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
        {
            result = sxa_ntfs_unlink(filename);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_wdelete(filename);
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileDelete;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalWFileDelete()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_ULONG
PalDiskGetFreeSpace(
    const MMP_INT dirnum,
    MMP_UINT32              *free_h,
    MMP_UINT32              *free_l,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_ULONG result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(dirnum=%d,cb=0x%X)\r\n", __FUNCTION__, dirnum, callback);

    if (!callback) // Sync mode
    {
        F_SPACE       space = {0};
        #if defined(CONFIG_HAVE_NTFS)
        struct statfs buf;

        if (sxa_ntfs_statfs(dirnum, &buf) == 0)
        {
            unsigned long long size = ((unsigned long long) buf.bavail) * buf.bsize;

            if (free_h)
                *free_h = (MMP_UINT32) (size >> 32);

            if (free_l)
                *free_l = (MMP_UINT32) (size & 0x00000000FFFFFFFFULL);
        }
        else
        {
            result = f_getfreespace(dirnum, &space);
            if (result != F_NO_ERROR)
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "GET STORE INFO ERROR: %d\r\n", result);
                //PalAssert(!"GET STORE INFO ERROR");
                goto end;
            }

            if (free_h) *free_h = space.free_high;
            if (free_l) *free_l = space.free;
        }
        #else
        result = f_getfreespace(dirnum, &space);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "GET STORE INFO ERROR: %d\r\n", result);
            //PalAssert(!"GET STORE INFO ERROR");
            goto end;
        }

        if (free_h) *free_h = space.free_high;
        if (free_l) *free_l = space.free;
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalDiskGetFreeSpace;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) dirnum;
    msgBuf.arg2         = (void *) free_h;
    msgBuf.arg3         = (void *) free_l;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_ULONG result;
    LOG_ENTER "PalDiskGetFreeSpace(dirnum=%d,callback=0x%X)\r\n",
    dirnum, callback LOG_END

    if (!callback) // Sync mode
    {
        F_SPACE       space;
        #if (CONFIG_HAVE_NTFS)
        struct statfs buf;
        if (sxa_ntfs_statfs(dirnum, &buf) == 0)
        {
            unsigned long long size = ((unsigned long long) buf.bavail) * buf.bsize;
            *free_h = (MMP_UINT32) (size >> 32);
            *free_l = (MMP_UINT32) (size & 0x00000000FFFFFFFFULL);
            #if (0)
            printf("[i] free=%08X%08X\r\n", *free_h, *free_l);
            #endif
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_getfreespace(dirnum, &space);
        if (result != 0)
        {
            LOG_ERROR "GET STORE INFO ERROR: %d\r\n", result LOG_END
            //PalAssert(!"GET STORE INFO ERROR");
            result = (MMP_ULONG) -1;
            goto end;
        }

        *free_h = space.free_high;
        *free_l = space.free;
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalDiskGetFreeSpace;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) dirnum;
    msgBuf.arg2         = (void *) free_h;
    msgBuf.arg3         = (void *) free_l;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalDiskGetFreeSpace()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_ULONG
PalDiskGetTotalSpace(
    const MMP_INT dirnum,
    MMP_UINT32              *total_h,
    MMP_UINT32              *total_l,
    PAL_FILE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_ULONG result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(dirnum=%d,cb=0x%X)\r\n", __FUNCTION__, dirnum, callback);

    if (!callback)  // Sync mode
    {
        F_SPACE       space = {0};
        #if defined(CONFIG_HAVE_NTFS)
        struct statfs buf;
        if (sxa_ntfs_statfs(dirnum, &buf) == 0)
        {
            unsigned long long size = ((unsigned long long) buf.blocks) * buf.bsize;

            if (total_h) *total_h = (MMP_UINT32) (size >> 32);
            if (total_l) *total_l = (MMP_UINT32) (size & 0x00000000FFFFFFFFULL);
        }
        else
        {
            result = f_getfreespace(dirnum, &space);
            if (result != F_NO_ERROR)
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "GET STORE INFO ERROR: %d\r\n", result);
                goto end;
            }

            if (total_h) *total_h = space.total_high;
            if (total_l) *total_l = space.total;
        }
        #else
        result = f_getfreespace(dirnum, &space);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "GET STORE INFO ERROR: %d\r\n", result);
            goto end;
        }

        if (total_h) *total_h = space.total_high;
        if (total_l) *total_l = space.total;
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalDiskGetTotalSpace;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) dirnum;
    msgBuf.arg2         = (void *) total_h;
    msgBuf.arg3         = (void *) total_l;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_ULONG result;
    LOG_ENTER "PalDiskGetTotalSpace(dirnum=%d,callback=0x%X)\r\n",
    dirnum, callback LOG_END

    if (!callback) // Sync mode
    {
        F_SPACE       space;
        #if (CONFIG_HAVE_NTFS)
        struct statfs buf;
        if (sxa_ntfs_statfs(dirnum, &buf) == 0)
        {
            unsigned long long size = ((unsigned long long) buf.blocks) * buf.bsize;
            *total_h = (MMP_UINT32) (size >> 32);
            *total_l = (MMP_UINT32) (size & 0x00000000FFFFFFFFULL);
            #if (0)
            printf("[i] total=%08X%08X\r\n", *total_h, *total_l);
            #endif
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_getfreespace(dirnum, &space);
        if (result != 0)
        {
            LOG_ERROR "GET STORE INFO ERROR: %d\r\n", result LOG_END
//            PalAssert(!"GET STORE INFO ERROR");
            result = (MMP_ULONG) -1;
            goto end;
        }

        *total_h = space.total_high;
        *total_l = space.total;
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalDiskGetTotalSpace;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) dirnum;
    msgBuf.arg2         = (void *) total_h;
    msgBuf.arg3         = (void *) total_l;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalDiskGetTotalSpace()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

PAL_FILE_FIND
PalFileFindFirst(
    const MMP_CHAR *filename,
    PAL_FILE_FIND_FIRST_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;
    F_FIND  *find  = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(filename=%s,cb=0x%X)\r\n", __FUNCTION__, filename, callback);

    if (!callback) // Sync mode
    {
        if ((find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(F_FIND))) == MMP_NULL)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Allocat fail !!");
            goto end;
        }

        result = f_findfirst(filename, find);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FIND FIRST ERROR: %d\r\n", result);
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFindFirst;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return find;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result;
    F_FIND  *find = MMP_NULL;
    LOG_ENTER "PalFileFindFirst(filename=%s,callback=0x%X)\r\n",
    filename, callback LOG_END

    if (!callback) // Sync mode
    {
        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(F_FIND));
        if (!find)
        {
            goto end;
        }
        result = f_findfirst(filename, find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND FIRST ERROR: %d\r\n", result LOG_END
//            PalAssert(!"FIND FIRST ERROR");
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFindFirst;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileFindFirst()=0x%X\r\n", find LOG_END
    return find;
    #else
    return MMP_NULL;
    #endif
#endif
}

PAL_FILE_FIND
PalWFileFindFirst(
    const MMP_WCHAR *filename,
    PAL_FILE_FIND_FIRST_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    FN_WFIND   *find          = MMP_NULL;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(cb=0x%X)\r\n", __FUNCTION__, callback);

    if (!callback) // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        if ((pt_pal_handle = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PAL_HANDLE))) == MMP_NULL)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Alloc fail !! ");
            result = MMP_RESULT_ERROR;
            goto end;
        }

        // initial handle
        pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
        pt_pal_handle->fp  = MMP_NULL;

        if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
        {
            pt_pal_handle->tag = PAL_FSTAG_NTFS;

            pt_pal_handle->fp  = sxa_ntfs_find_file_open(filename);
            if (pt_pal_handle->fp == MMP_NULL)
            {
                pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
                result             = MMP_RESULT_ERROR;
            }
        }
        else
        {
            if ((find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(FN_WFIND))) == MMP_NULL)
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Alloc fail !! ");
                goto end;
            }

            pt_pal_handle->tag = PAL_FSTAG_HCC;

            result             = f_wfindfirst(filename, find);
            if (result != F_NO_ERROR)
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FIND FIRST ERROR: %d\r\n", result);
                PalHeapFree(PAL_HEAP_DEFAULT, find);
                find               = MMP_NULL;
                pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
            }

            pt_pal_handle->fp = find;
        }
        #else
        if ((find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(FN_WFIND))) == MMP_NULL)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Allocat fail !!");
            goto end;
        }

        result = f_wfindfirst(filename, find);
        if (result != F_NO_ERROR)
        {
            _printfString("findFirst fail: ", (MMP_WCHAR *)filename);
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FIND FIRST ERROR: %d\r\n", result);
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }

        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileFindFirst;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:

        #if defined(CONFIG_HAVE_NTFS)
    if (result != MMP_SUCCESS)
    {
        if (pt_pal_handle)
        {
            if (pt_pal_handle->fp)
            {
                PalHeapFree(0, pt_pal_handle->fp);
                pt_pal_handle->fp = MMP_NULL;
            }
            PalHeapFree(0, pt_pal_handle);
            pt_pal_handle = MMP_NULL;
        }

        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    }
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return (PAL_FILE_FIND)pt_pal_handle;
        #else
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return find;
        #endif

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle = 0;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT           result         = 0;
    FN_WFIND          *find          = MMP_NULL;
    LOG_ENTER "PalFileFindFirst(filename=%s,callback=0x%X)\r\n",
    filename, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        XPrint(__FUNCTION__, ":", filename, 1);
        if (!(pt_pal_handle = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(*pt_pal_handle))))
        {
            goto l_err;
        }
        memset(pt_pal_handle, 0, sizeof(*pt_pal_handle));

        if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
        {
            if (!(pt_pal_handle->fp = sxa_ntfs_find_file_open(filename)))
            {
                goto l_err;
            }

            pt_pal_handle->tag = PAL_FSTAG_NTFS;
            goto end;

l_err:
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
            pt_pal_handle = 0;
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(FN_WFIND));
        if (!find)
        {
            goto end;
        }
        result = f_wfindfirst(filename, find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND FIRST ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FIND FIRST ERROR");
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }

        #if (CONFIG_HAVE_NTFS)
        if (find)
        {
            pt_pal_handle->tag = PAL_FSTAG_HCC;
            pt_pal_handle->fp  = find;
        }
        else
        {
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
            pt_pal_handle = NULL;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileFindFirst;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) filename;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
        #if (CONFIG_HAVE_NTFS)
    find = pt_pal_handle;

    if (!callback && pt_pal_handle)
    {
        XPrint(__FUNCTION__, ":", PalWFileFindGetName(pt_pal_handle), 1);
    }
        #endif /* (CONFIG_HAVE_NTFS) */

    LOG_LEAVE "PalFileFindFirst()=0x%X\r\n", find LOG_END
    return find;

    #else
    return MMP_NULL;
    #endif
#endif
}

MMP_INT
PalFileFindNext(
    PAL_FILE_FIND find,
    PAL_FILE_FIND_NEXT_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X,cb=0x%X)\r\n", __FUNCTION__, find, callback);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        result = f_findnext((F_FIND *)find);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FIND NEXT ERROR: %d\r\n", result);
            result = MMP_RESULT_ERROR;
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFindNext;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) find;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result;
    LOG_ENTER "PalFileFindNext(find=0x%X,callback=0x%X)\r\n",
    find, callback LOG_END

    if (!callback) // Sync mode
    {
        result = f_findnext((F_FIND *) find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND NEXT ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FIND NEXT ERROR");
            result = 1;
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFindNext;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) find;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileFindNext()=%d\r\n", result LOG_END
    return result;
    #else
    return MMP_NULL;
    #endif
#endif
}

MMP_INT
PalWFileFindNext(
    PAL_FILE_FIND find,
    PAL_FILE_FIND_NEXT_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X,cb=0x%X)\r\n", __FUNCTION__, find, callback);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)find;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            result = sxa_ntfs_find_file_next((SXA_NTFS_HANDLE)pt_pal_handle->fp);
            break;

        case PAL_FSTAG_HCC:
            result = f_wfindnext((FN_WFIND *)pt_pal_handle->fp);
            if (result != F_NO_ERROR)
                sdk_msg_ex(SDK_MSG_TYPE_PAL_INFO, "FIND NEXT ERROR: %d\r\n", result);
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }

        #else
        result = f_wfindnext((FN_WFIND *)find);
        if (result != F_NO_ERROR)
            sdk_msg_ex(SDK_MSG_TYPE_PAL_INFO, "FIND NEXT ERROR: %d\r\n", result);

        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileFindNext;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) find;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS && result != F_ERR_NOTFOUND)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT           result;
    LOG_ENTER "PalWFileFindNext(find=0x%X,callback=0x%X)\r\n",
    find, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) find;
        find          = pt_pal_handle->fp;
        if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_find_file_next(find);

            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        result = f_wfindnext((FN_WFIND *) find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND NEXT ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FIND NEXT ERROR");
            result = 1;
        }
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalWFileFindNext;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) find;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
        #if (CONFIG_HAVE_NTFS)
    if (!callback && !result && find)
    {
        XPrint(__FUNCTION__, ":", PalWFileFindGetName(pt_pal_handle), 1);
    }
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalWFileFindNext()=%d\r\n", result LOG_END
    return result;
    #else
    return MMP_NULL;
    #endif
#endif
}

MMP_INT
PalFileFindClose(
    PAL_FILE_FIND find,
    PAL_FILE_FIND_CLOSE_CALLBACK callback,
    void *callback_arg)
{
    MSG msgBuf = {0};
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT    result         = MMP_SUCCESS;
    PAL_HANDLE *pt_pal_handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X,cb=0x%X)\r\n", __FUNCTION__, find, callback);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

    if (!callback)  // Sync mode
    {
        #if defined(CONFIG_HAVE_NTFS)
        pt_pal_handle = (PAL_HANDLE *)find;

        switch (pt_pal_handle->tag)
        {
        case PAL_FSTAG_NTFS:
            result             = sxa_ntfs_find_file_close((SXA_NTFS_HANDLE)pt_pal_handle->fp);
            pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
            pt_pal_handle      = MMP_NULL;
            break;

        case PAL_FSTAG_HCC:
            if (pt_pal_handle->fp)
                PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle->fp);
            pt_pal_handle->fp  = MMP_NULL;
            pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
            PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
            pt_pal_handle      = MMP_NULL;
            break;

        default:
            result = MMP_RESULT_ERROR;
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
            break;
        }
        #else
        PalHeapFree(PAL_HEAP_DEFAULT, find);
        find = MMP_NULL;
        #endif
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFindClose;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) find;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != MMP_SUCCESS)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
    unsigned int      tag;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT           result;
    LOG_ENTER "PalFileFindClose(find=0x%X,callback=0x%X)\r\n",
    find, callback LOG_END

    if (!callback) // Sync mode
    {
        #if (CONFIG_HAVE_NTFS)
        pt_pal_handle = (struct PAL_HANDLE *) find;
        find          = pt_pal_handle->fp;
        tag           = pt_pal_handle->tag;
            #if (0)
        if ((tag != PAL_FSTAG_NTFS) && (tag != PAL_FSTAG_HCC))
        {
            printf("[X] tag=%d, halted!!!\r\n", tag);
            while (1);
            result = 0;
            goto end;
        }
        pt_pal_handle->tag = 0;
            #endif
        PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
        if (tag == PAL_FSTAG_NTFS)
        {
            result = sxa_ntfs_find_file_close(find);
            goto end;
        }
        #endif /* (CONFIG_HAVE_NTFS) */
        PalHeapFree(PAL_HEAP_DEFAULT, find);
        result = 0;
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void *) PalFileFindClose;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void *) find;

    // Write message to queue
    result              = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        goto end;
    }

end:
    LOG_LEAVE "PalFileFindClose()=%d\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

const MMP_CHAR *
PalFileFindGetName(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_CHAR *result = MMP_NULL;
    F_FIND   *handle = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    PalAssert(find);

    handle = (F_FIND *) find;
    result = handle->filename;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_CHAR *result;
    F_FIND   *handle;
    LOG_ENTER "PalFileFindGetName(find=0x%X)\r\n", find LOG_END

    PalAssert(find);

    handle = (F_FIND *) find;
    result = handle->filename;

    LOG_LEAVE "PalFileFindGetName()=0x%X\r\n", result LOG_END
    return result;
    #else
    return MMP_NULL;
    #endif
#endif
}

const MMP_WCHAR *
PalWFileFindGetName(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    MMP_WCHAR  *name          = MMP_NULL;
    FN_WFIND   *handle        = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

        #if defined(CONFIG_HAVE_NTFS)
    pt_pal_handle = (PAL_HANDLE *)find;

    switch (pt_pal_handle->tag)
    {
    case PAL_FSTAG_NTFS:
        name = (MMP_WCHAR *)sxa_ntfs_find_file_get_name((SXA_NTFS_HANDLE)pt_pal_handle->fp);
        break;

    case PAL_FSTAG_HCC:
        handle = (FN_WFIND *)pt_pal_handle->fp;
        name   = handle->filename;
        break;

    default:
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
        break;
    }

        #else
    handle = (FN_WFIND *) find;
    name   = handle->filename;
        #endif

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return name;
    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef  HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_WCHAR         *result;
    FN_WFIND          *handle;
    LOG_ENTER "PalWFileFindGetName(find=0x%X)\r\n", find LOG_END

    PalAssert(find);

        #if (CONFIG_HAVE_NTFS)
    pt_pal_handle = (struct PAL_HANDLE *) find;
    find          = pt_pal_handle->fp;
    if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
    {
        result = sxa_ntfs_find_file_get_name(find);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */

    handle = (FN_WFIND *) find;
    result = handle->filename;

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalWFileFindGetName()=0x%X\r\n", result LOG_END
    return result;
    #else
    return MMP_NULL;
    #endif
#endif
}

const MMP_UINT64
PalFileFindGetSizeEx(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    MMP_UINT64 fileSize       = 0LL;
    FN_WFIND   *handle        = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

        #if defined(CONFIG_HAVE_NTFS)
    pt_pal_handle = (PAL_HANDLE *)find;

    switch (pt_pal_handle->tag)
    {
    case PAL_FSTAG_NTFS:
        fileSize = sxa_ntfs_find_file_get_size((SXA_NTFS_HANDLE)pt_pal_handle->fp);
        break;

    case PAL_FSTAG_HCC:
        handle   = (FN_WFIND *)pt_pal_handle->fp;
        fileSize = handle->filesize;
        break;

    default:
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
        break;
    }

        #else
    handle   = (FN_WFIND *) find;
    fileSize = handle->filesize;
        #endif

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return fileSize;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef  HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_UINT64        result;
    FN_WFIND          *handle;
    LOG_ENTER "PalFileFindGetSize(find=0x%X)\r\n", find LOG_END

    PalAssert(find);

        #if (CONFIG_HAVE_NTFS)
    pt_pal_handle = (struct PAL_HANDLE *) find;
    find          = pt_pal_handle->fp;
    if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
    {
        result = sxa_ntfs_find_file_get_size(find);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */
    handle = (FN_WFIND *) find;
    result = handle->filesize;

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalFileFindGetSize()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

const MMP_ULONG
PalFileFindGetSize(
    PAL_FILE_FIND find)
{
    return (MMP_ULONG) PalFileFindGetSizeEx(find);
}

const MMP_UINT16
PalFileFindGetTime(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    MMP_ULONG  fileTime       = 0;
    FN_WFIND   *handle        = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

        #if defined(CONFIG_HAVE_NTFS)
    pt_pal_handle = (PAL_HANDLE *) find;

    switch (pt_pal_handle->tag)
    {
    case PAL_FSTAG_NTFS:
        fileTime = (MMP_ULONG)sxa_ntfs_find_file_get_time((SXA_NTFS_HANDLE)pt_pal_handle->fp);
        break;

    case PAL_FSTAG_HCC:
        handle   = (FN_WFIND *)pt_pal_handle->fp;
        fileTime = handle->ctime;
        break;

    default:
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
        break;
    }

        #else
    handle   = (FN_WFIND *) find;
    fileTime = handle->ctime;
        #endif

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return fileTime;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_ULONG         result;
    FN_WFIND          *handle;
    LOG_ENTER "PalFileFindGetTime(find=0x%X)\r\n", find LOG_END

    PalAssert(find);

        #if (CONFIG_HAVE_NTFS)
    pt_pal_handle = (struct PAL_HANDLE *) find;
    find          = pt_pal_handle->fp;
    if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
    {
        result = sxa_ntfs_find_file_get_time(find);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */
    handle = (FN_WFIND *) find;
    result = handle->ctime;

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalFileFindGetTime()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

const MMP_UINT16
PalFileFindGetDate(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    MMP_ULONG  fileDate       = 0;
    FN_WFIND   *handle        = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

        #if defined(CONFIG_HAVE_NTFS)
    pt_pal_handle = (PAL_HANDLE *)find;

    switch (pt_pal_handle->tag)
    {
    case PAL_FSTAG_NTFS:
        fileDate = (MMP_ULONG)sxa_ntfs_find_file_get_date((SXA_NTFS_HANDLE)pt_pal_handle->fp);
        break;

    case PAL_FSTAG_HCC:
        handle   = (FN_WFIND *)pt_pal_handle->fp;
        fileDate = handle->cdate;
        break;

    default:
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
        break;
    }

        #else
    handle   = (FN_WFIND *) find;
    fileDate = handle->cdate;
        #endif

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return fileDate;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_ULONG         result;
    FN_WFIND          *handle;
    LOG_ENTER "PalFileFindGetDate(find=0x%X)\r\n", find LOG_END

    PalAssert(find);

        #if (CONFIG_HAVE_NTFS)
    pt_pal_handle = (struct PAL_HANDLE *) find;
    find          = pt_pal_handle->fp;
    if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
    {
        result = sxa_ntfs_find_file_get_date(find);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */
    handle = (FN_WFIND *) find;
    result = handle->cdate;

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalFileFindGetDate()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalFindAttrIsDirectory(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    MMP_INT    result         = 0;
    FN_WFIND   *handle        = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

        #if defined(CONFIG_HAVE_NTFS)
    pt_pal_handle = (PAL_HANDLE *) find;

    switch (pt_pal_handle->tag)
    {
    case PAL_FSTAG_NTFS:
        result = sxa_ntfs_find_file_is_directory((SXA_NTFS_HANDLE)pt_pal_handle->fp);
        break;

    case PAL_FSTAG_HCC:
        handle = (FN_WFIND *)pt_pal_handle->fp;
        result = (MMP_INT)(handle->attr & F_ATTR_DIR);
        break;

    default:
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
        break;
    }

        #else
    handle = (FN_WFIND *) find;
    result = (MMP_INT)(handle->attr & F_ATTR_DIR);
        #endif

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_UINT          result;
    FN_WFIND          *handle;
    LOG_ENTER "PalFindAttrIsDirectory(find=0x%X)\r\n", find LOG_END

    PalAssert(find);

        #if (CONFIG_HAVE_NTFS)
    pt_pal_handle = (struct PAL_HANDLE *) find;
    find          = pt_pal_handle->fp;
    if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
    {
        result = sxa_ntfs_find_file_is_directory(find);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */

    handle = (FN_WFIND *) find;
    result = (MMP_INT)(handle->attr & F_ATTR_DIR);

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalFindAttrIsDirectory()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

void *
PalWFileFindGetPos(
    PAL_FILE_FIND find)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    void       *result        = MMP_NULL;
    FN_WFIND   *handle        = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(find=0x%X)\r\n", __FUNCTION__, find);

    if (find == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Null pointer !!");
        goto end;
    }

        #if defined(CONFIG_HAVE_NTFS)
    pt_pal_handle = (PAL_HANDLE *)find;

    switch (pt_pal_handle->tag)
    {
    case PAL_FSTAG_NTFS:
        result = (void *)sxa_ntfs_find_file_get_pos((SXA_NTFS_HANDLE)pt_pal_handle->fp);
        break;

    case PAL_FSTAG_HCC:
        handle = (FN_WFIND *)pt_pal_handle->fp;
        result = &handle->oripos;
        break;

    default:
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Un-support FS type, Something wrong !!");
        break;
    }

        #else
    handle = (FN_WFIND *) find;
    result = &handle->oripos;
        #endif

end:
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */
    void              *result;
        #ifdef _WIN32
            #ifdef WIN32_USE_HCC
    FN_WFIND          *handle;
            #else
    FILE_FIND         *fileFind;
            #endif
        #else
    FN_WFIND          *handle;
        #endif

    LOG_ENTER "PalWFileFindGetPos(find=0x%X)\r\n", find LOG_END

        #if (CONFIG_HAVE_NTFS)
    pt_pal_handle = (struct PAL_HANDLE *) find;
    find          = pt_pal_handle->fp;
    if (pt_pal_handle->tag == PAL_FSTAG_NTFS)
    {
        result = sxa_ntfs_find_file_get_pos(find);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */

        #ifdef USE_HCC

    handle = (FN_WFIND *) find;
    result = &handle->oripos;

        #else

    fileFind = (FILE_FIND *) find;
    result   = MMP_NULL;

        #endif

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */

    LOG_LEAVE "PalWFileFindGetPos()=0x%X\r\n", result LOG_END
    return result;

    #else
    return 0;
    #endif
#endif
}

PAL_FILE_FIND
PalWFileFindByPos(
    int drivenum,
    MMP_WCHAR *path,
    void *pos)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    PAL_HANDLE *pt_pal_handle = MMP_NULL;
    MMP_INT    result         = MMP_SUCCESS;
    MMP_WCHAR  volumename[16] = {0};
    FN_WFIND   *find          = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(drivenum=%d,pos=0x%X)\r\n", __FUNCTION__, drivenum, pos);

        #if defined(CONFIG_HAVE_NTFS)
    if ((pt_pal_handle = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PAL_HANDLE))) == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Allocate fail !!");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    pt_pal_handle->tag = PAL_FSTAG_UNKNOW;
    pt_pal_handle->fp  = MMP_NULL;

    if (sxa_ntfs_mounted(drivenum))     // NTFS
    {
        pt_pal_handle->tag = PAL_FSTAG_NTFS;
        pt_pal_handle->fp  = sxa_ntfs_find_file_open_pos(path, pos);
        if (pt_pal_handle->fp == MMP_NULL)
            result = MMP_RESULT_ERROR;
    }
    else
    {
        pt_pal_handle->tag = PAL_FSTAG_HCC;

        if ((find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(FN_WFIND))) == MMP_NULL)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Allocate fail !!");
            result = MMP_RESULT_ERROR;
            goto end;
        }
        PalMemset(find, 0, sizeof(FN_WFIND));

        PalWcscpy(volumename, L"A:/*.*");
        volumename[0] = L'A' + drivenum;

        result        = f_wfindfirst(volumename, find);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FIND POS ERROR (%d)!!", result);
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
            goto end;
        }

        find->findfsname.drivenum = drivenum;
        PalMemset(&find->findfsname.lname, 0, sizeof(MMP_WCHAR) * F_MAXLNAME);
        memcpy(&find->findfsname.lname, L"*.*", sizeof(MMP_WCHAR) * 3);
        PalMemset(&find->findfsname.path, 0, sizeof(MMP_WCHAR) * FN_MAXPATH);
        memcpy(&find->findfsname.path, path, sizeof(MMP_WCHAR) * PalWcslen(path));
        memcpy(&find->pos, pos, sizeof(F_POS));

        result = f_wfindbypos(find);
        if (result != F_NO_ERROR)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "NOT FOUND %d-th POS (%d)!!\r\n", find->pos.pos, result);
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }

        pt_pal_handle->fp = find;
    }

end:
    if (result != MMP_SUCCESS)
    {
        if (pt_pal_handle)
        {
            if (pt_pal_handle->fp)
            {
                PalHeapFree(0, pt_pal_handle->fp);
                pt_pal_handle->fp = MMP_NULL;
            }

            PalHeapFree(0, pt_pal_handle);
            pt_pal_handle = MMP_NULL;
        }
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    }
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return (PAL_FILE_FIND)pt_pal_handle;

        #else
    if ((find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(FN_WFIND))) == MMP_NULL)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Allocate fail !!");
        goto end;
    }

    PalMemset(find, 0, sizeof(FN_WFIND));

    PalWcscpy(volumename, L"A:/*.*");
    volumename[0] = L'A' + drivenum;

    result        = f_wfindfirst(volumename, find);
    if (result != F_NO_ERROR)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "FIND POS ERROR (%d)!!", result);
        PalHeapFree(PAL_HEAP_DEFAULT, find);
        find = MMP_NULL;
        goto end;
    }

    find->findfsname.drivenum = drivenum;
    PalMemset(&find->findfsname.lname, 0, sizeof(MMP_WCHAR) * 256);
    memcpy(&find->findfsname.lname, L"*.*", sizeof(MMP_WCHAR) * 3);
    PalMemset(&find->findfsname.path, 0, sizeof(MMP_WCHAR) * 256);
    memcpy(&find->findfsname.path, path, sizeof(MMP_WCHAR) * 256);
    memcpy(&find->pos, pos, sizeof(F_POS));

    result = f_wfindbypos(find);
    if (result != F_NO_ERROR)
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "NOT FOUND %d-th POS !!\r\n", find->pos.pos);
        PalHeapFree(PAL_HEAP_DEFAULT, find);
        find = MMP_NULL;
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return find;
        #endif

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct PAL_HANDLE *pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */

    MMP_INT           result         = 0;
    MMP_WCHAR         volumename[16] = {0};
        #ifdef _WIN32
            #ifdef WIN32_USE_HCC
    FN_WFIND          *find          = MMP_NULL;
            #else
    FILE_FIND         *fileFind;
            #endif
        #else
    FN_WFIND          *find = MMP_NULL;
        #endif

    LOG_ENTER "PalWFileFindByPos(drivenum=%d,pos=0x%X,path=%s)\r\n",
    drivenum, pos, path LOG_END

        #if (CONFIG_HAVE_NTFS)
    if (!(pt_pal_handle = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(*pt_pal_handle))))
    {
        goto end;
    }
    memset(pt_pal_handle, 0, sizeof(*pt_pal_handle));

    if (!sxa_ntfs_mounted(drivenum))     // unsupported in NTFS
    {
        goto l_hcc;
    }

    pt_pal_handle->tag = PAL_FSTAG_NTFS;
    find               = sxa_ntfs_find_file_open_pos(path, pos);
    goto end;

l_hcc:
    pt_pal_handle->tag = PAL_FSTAG_HCC;
        #endif /* (CONFIG_HAVE_NTFS) */

        #ifdef USE_HCC
    {
        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(FN_WFIND));
        PalMemset(find, 0, sizeof(FN_WFIND));

        if (!find)
        {
            goto end;
        }

        PalWcscpy(volumename, L"A:/*.*");
        volumename[0] = L'A' + drivenum;
        result        = f_wfindfirst(volumename, find);
        if (result != F_NO_ERROR)
        {
            printf("FIND POS ERROR: %d, %s [#%d]\r\n", result, __FILE__, __LINE__);
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
            goto end;
        }

        find->findfsname.drivenum = drivenum;
        PalMemset(&find->findfsname.lname, 0, sizeof(MMP_WCHAR) * 256);
        memcpy(&find->findfsname.lname, L"*.*", sizeof(MMP_WCHAR) * 3);
        PalMemset(&find->findfsname.path, 0, sizeof(MMP_WCHAR) * 256);
        memcpy(&find->findfsname.path, path, 256 * sizeof(MMP_WCHAR));
        memcpy(&find->pos, pos, sizeof(F_POS));

        result = f_wfindbypos(find);
        if (result != F_NO_ERROR)
        {
            printf("NOT FOUND %d-th POS !! %s [#%d]\r\n", find->pos.pos, __FILE__, __LINE__);
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }
    }
        #endif

end:
        #if (CONFIG_HAVE_NTFS)
    if (find)
    {
        pt_pal_handle->fp = find;
    }
    else
    {
        PalHeapFree(PAL_HEAP_DEFAULT, pt_pal_handle);
        pt_pal_handle = NULL;
    }

    find = pt_pal_handle;
        #endif /* (CONFIG_HAVE_NTFS) */

    LOG_LEAVE "PalWFileFindByPos()=0x%X\r\n", find LOG_END
    return find;

    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalInitVolume(
    MMP_UINT drvNumber,
    void *drvInit,
    MMP_UINT cardType,
    MMP_UINT partitionNum)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT   result      = MMP_SUCCESS;
    MMP_UINT8 hccResult   = 0;
    F_DRIVER  *tmp_pDrive = MMP_NULL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(drvnumber=0x%X,drvinit=0x%X,cardtype=0x%X,par=%d)\r\n",
            __FUNCTION__, drvNumber, drvInit, cardType, partitionNum);

        #if defined(CONFIG_HAVE_NTFS)
    {
        struct SXA_BLKDEV_INFO blkdev_mbr = {0};

        result = sxa_blkdev_get_info(SXA_NTFS_MKDEVNODE(cardType, 0), &blkdev_mbr);
        if (result < 0)
            sdk_msg(SDK_MSG_TYPE_ERROR, "sxa_blkdev_get_info() error return 0x%x !!\n", result);
        else
        {
            // TODO: check if ntfs partition here !!!
            if (blkdev_mbr.partitions[partitionNum].type == SXA_BLKDEV_PARTITION_TYPE_NTFS)
            {
                result  = sxa_ntfs_mount(SXA_NTFS_MKDEVNODE(cardType, partitionNum + 1), drvNumber, 0);
                if (result != 0)
                    sdk_msg(SDK_MSG_TYPE_ERROR, "sxa_ntfs_mount() error return 0x%x !!\n", result);
                result |= NTFS_RESULT_FLAG;
                goto end;
            }
            else
                sdk_msg(SDK_MSG_TYPE_ERROR, "blkdev_mbr.partitions.type=0x%x\n", blkdev_mbr.partitions[partitionNum].type);
        }
    }
        #endif

    switch (cardType)
    {
    case PAL_DEVICE_TYPE_SD:
        if (!m_pSDDriver)
            hccResult = f_createdriver(&m_pSDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pSDDriver;
        break;

    case PAL_DEVICE_TYPE_SD2:
        if (!m_pSD2Driver)
            hccResult = f_createdriver(&m_pSD2Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pSD2Driver;
        break;

    case PAL_DEVICE_TYPE_MS:
        if (!m_pMSDriver)
            hccResult = f_createdriver(&m_pMSDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pMSDriver;
        break;

    case PAL_DEVICE_TYPE_MMC:
        if (!m_pMMCDriver)
            hccResult = f_createdriver(&m_pMMCDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pMMCDriver;
        break;

    case PAL_DEVICE_TYPE_USB0:
        if (!m_pUSB0Driver)
            hccResult = f_createdriver(&m_pUSB0Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB0Driver;
        break;

    case PAL_DEVICE_TYPE_USB1:
        if (!m_pUSB1Driver)
            hccResult = f_createdriver(&m_pUSB1Driver, drvInit, F_AUTO_ASSIGN); \

        tmp_pDrive = m_pUSB1Driver;
        break;

    case PAL_DEVICE_TYPE_USB2:
        if (!m_pUSB2Driver)
            hccResult = f_createdriver(&m_pUSB2Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB2Driver;
        break;

    case PAL_DEVICE_TYPE_USB3:
        if (!m_pUSB3Driver)
            hccResult = f_createdriver(&m_pUSB3Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB3Driver;
        break;

    case PAL_DEVICE_TYPE_USB4:
        if (!m_pUSB4Driver)
            hccResult = f_createdriver(&m_pUSB4Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB4Driver;
        break;

    case PAL_DEVICE_TYPE_USB5:
        if (!m_pUSB5Driver)
            hccResult = f_createdriver(&m_pUSB5Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB5Driver;
        break;

    case PAL_DEVICE_TYPE_USB6:
        if (!m_pUSB6Driver)
            hccResult = f_createdriver(&m_pUSB6Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB6Driver;
        break;

    case PAL_DEVICE_TYPE_USB7:
        if (!m_pUSB7Driver)
            hccResult = f_createdriver(&m_pUSB7Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB7Driver;
        break;

    case PAL_DEVICE_TYPE_PTP:
        //result = PtpInitVolume();
        return result;
        break;

    case PAL_DEVICE_TYPE_NOR:
        if (!m_pNORDriver)
            hccResult = f_createdriver(&m_pNORDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNORDriver;
        break;

    case PAL_DEVICE_TYPE_NORPRIVATE:
        if (!m_pNORPRIVATEDriver)
            hccResult = f_createdriver(&m_pNORPRIVATEDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNORPRIVATEDriver;
        break;

    case PAL_DEVICE_TYPE_NAND:
        if (!m_pNANDDriver)
            hccResult = f_createdriver(&m_pNANDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNANDDriver;
        break;

    case PAL_DEVICE_TYPE_CF:
        if (!m_pCFDriver)
            hccResult = f_createdriver(&m_pCFDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pCFDriver;
        break;

    case PAL_DEVICE_TYPE_xD:
        if (!m_pxDDriver)
            hccResult = f_createdriver(&m_pxDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pxDDriver;
        break;
    }

    if (hccResult == F_NO_ERROR)
        result = f_initvolumepartition(drvNumber, tmp_pDrive, partitionNum);
    else
    {
        // driver initial fail
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "cardType=%d, f_createdriver() faill (%d)!!", cardType, hccResult);
    }

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #if (CONFIG_HAVE_NTFS)
    struct SXA_BLKDEV_INFO blkdev_mbr;
    #endif /* (CONFIG_HAVE_NTFS) */

    #ifdef HAVE_FAT
    MMP_INT   result      = 0;
    MMP_UINT8 hccResult   = 0;
    F_DRIVER  *tmp_pDrive = MMP_NULL;

    LOG_ENTER "PalInitVolume(drvnumber=0x%X,drvinit=0x%X,cardtype=0x%X)\r\n", drvNumber, drvInit, cardType LOG_END

        #if (CONFIG_HAVE_NTFS)
    if (sxa_blkdev_get_info(SXA_NTFS_MKDEVNODE(cardType, 0), &blkdev_mbr) < 0)
    {
        goto l_hcc;
    }

    // TODO: check if ntfs partition here!!!
    if (blkdev_mbr.partitions[partitionNum].type != SXA_BLKDEV_PARTITION_TYPE_NTFS)
    {
        goto l_hcc;
    }

    result = sxa_ntfs_mount(SXA_NTFS_MKDEVNODE(cardType, partitionNum + 1), drvNumber, 0);
    goto end;

l_hcc:

        #endif /* (CONFIG_HAVE_NTFS) */

    switch (cardType)
    {
    case PAL_DEVICE_TYPE_SD:
        if (!m_pSDDriver)
            hccResult = f_createdriver(&m_pSDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pSDDriver;
        break;

    case PAL_DEVICE_TYPE_SD2:
        if (!m_pSD2Driver)
            hccResult = f_createdriver(&m_pSD2Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pSD2Driver;
        break;

    case PAL_DEVICE_TYPE_MS:
        if (!m_pMSDriver)
            hccResult = f_createdriver(&m_pMSDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pMSDriver;
        break;

    case PAL_DEVICE_TYPE_MMC:
        if (!m_pMMCDriver)
            hccResult = f_createdriver(&m_pMMCDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pMMCDriver;
        break;

    case PAL_DEVICE_TYPE_USB0:
        if (!m_pUSB0Driver)
            hccResult = f_createdriver(&m_pUSB0Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB0Driver;
        break;

    case PAL_DEVICE_TYPE_USB1:
        if (!m_pUSB1Driver)
            hccResult = f_createdriver(&m_pUSB1Driver, drvInit, F_AUTO_ASSIGN); \

        tmp_pDrive = m_pUSB1Driver;
        break;

    case PAL_DEVICE_TYPE_USB2:
        if (!m_pUSB2Driver)
            hccResult = f_createdriver(&m_pUSB2Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB2Driver;
        break;

    case PAL_DEVICE_TYPE_USB3:
        if (!m_pUSB3Driver)
            hccResult = f_createdriver(&m_pUSB3Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB3Driver;
        break;

    case PAL_DEVICE_TYPE_USB4:
        if (!m_pUSB4Driver)
            hccResult = f_createdriver(&m_pUSB4Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB4Driver;
        break;

    case PAL_DEVICE_TYPE_USB5:
        if (!m_pUSB5Driver)
            hccResult = f_createdriver(&m_pUSB5Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB5Driver;
        break;

    case PAL_DEVICE_TYPE_USB6:
        if (!m_pUSB6Driver)
            hccResult = f_createdriver(&m_pUSB6Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB6Driver;
        break;

    case PAL_DEVICE_TYPE_USB7:
        if (!m_pUSB7Driver)
            hccResult = f_createdriver(&m_pUSB7Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB7Driver;
        break;

    case PAL_DEVICE_TYPE_PTP:
        //result = PtpInitVolume();
        return result;
        break;

    case PAL_DEVICE_TYPE_NOR:
        if (!m_pNORDriver)
            hccResult = f_createdriver(&m_pNORDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNORDriver;
        break;

    case PAL_DEVICE_TYPE_NORPRIVATE:
        if (!m_pNORPRIVATEDriver)
            hccResult = f_createdriver(&m_pNORPRIVATEDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNORPRIVATEDriver;
        break;

    case PAL_DEVICE_TYPE_NAND:
        if (!m_pNANDDriver)
            hccResult = f_createdriver(&m_pNANDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNANDDriver;
        break;

    case PAL_DEVICE_TYPE_CF:
        if (!m_pCFDriver)
            hccResult = f_createdriver(&m_pCFDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pCFDriver;
        break;

    case PAL_DEVICE_TYPE_xD:
        if (!m_pxDDriver)
            hccResult = f_createdriver(&m_pxDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pxDDriver;
        break;
    }

    if (hccResult == F_NO_ERROR)
    {
        //printf("check (drvNumber, tmp_pDrive, partitionNum) = (%d, 0x%x, %d) %s [#%d]\n",
        //               drvNumber, tmp_pDrive, partitionNum, __FILE__, __LINE__);
        result = f_initvolumepartition(drvNumber, tmp_pDrive, partitionNum);
        //printf("check card Type = %d (return = %d) %s [#%d]\n", cardType, result, __FILE__, __LINE__);
    }
    else
    {
        printf("card Type = %d, f_createdriver() faill (return %d) !! %s [%d]\n", cardType, hccResult, __FILE__, __LINE__);
    }

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */

    LOG_LEAVE "PalInitVolume()=0x%X\r\n", result LOG_END
    return result;

    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalDelVolume(
    MMP_UINT drvnumber)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(drvnumber=%d)\r\n", __FUNCTION__, drvnumber);

        #if defined(CONFIG_HAVE_NTFS)
    if (sxa_ntfs_mounted(drvnumber))
    {
        result = sxa_ntfs_umount(drvnumber);
    }
    else
        #endif
    result = f_delvolume(drvnumber);

    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result = 0;

        #if (CONFIG_HAVE_NTFS)
    if (sxa_ntfs_mounted(drvnumber))
    {
        return sxa_ntfs_umount(drvnumber);
    }
        #endif /* (CONFIG_HAVE_NTFS) */
    result = f_delvolume(drvnumber);

    return result;

    #else

    return 0;
    #endif
#endif
}

MMP_INT
PalDelDriver(
    MMP_UINT cardtype)
{
#if defined(HAVE_FAT)

    MMP_INT result = 0;

    switch (cardtype)
    {
    case PAL_DEVICE_TYPE_SD:
        if (m_pSDDriver)
            f_releasedriver(m_pSDDriver);

        m_pSDDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_SD2:
        if (m_pSD2Driver)
            f_releasedriver(m_pSD2Driver);

        m_pSD2Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_MS:
        if (m_pMSDriver)
            f_releasedriver(m_pMSDriver);

        m_pMSDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_MMC:
        if (m_pMMCDriver)
            f_releasedriver(m_pMMCDriver);

        m_pMMCDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB0:
        if (m_pUSB0Driver)
            f_releasedriver(m_pUSB0Driver);

        m_pUSB0Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB1:
        if (m_pUSB1Driver)
            f_releasedriver(m_pUSB1Driver);

        m_pUSB1Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB2:
        if (m_pUSB2Driver)
            f_releasedriver(m_pUSB2Driver);

        m_pUSB2Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB3:
        if (m_pUSB3Driver)
            f_releasedriver(m_pUSB3Driver);

        m_pUSB3Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB4:
        if (m_pUSB4Driver)
            f_releasedriver(m_pUSB4Driver);

        m_pUSB4Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB5:
        if (m_pUSB5Driver)
            f_releasedriver(m_pUSB5Driver);

        m_pUSB5Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB6:
        if (m_pUSB6Driver)
            f_releasedriver(m_pUSB6Driver);

        m_pUSB6Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_USB7:
        if (m_pUSB7Driver)
            f_releasedriver(m_pUSB7Driver);

        m_pUSB7Driver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_PTP:
        //PtpDelVolume();
        break;

    case PAL_DEVICE_TYPE_NOR:
        if (m_pNORDriver)
            f_releasedriver(m_pNORDriver);

        m_pNORDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_NORPRIVATE:
        if (m_pNORPRIVATEDriver)
            f_releasedriver(m_pNORPRIVATEDriver);

        m_pNORPRIVATEDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_NAND:
        if (m_pNANDDriver)
            f_releasedriver(m_pNANDDriver);

        m_pNANDDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_CF:
        if (m_pCFDriver)
            f_releasedriver(m_pCFDriver);

        m_pCFDriver = MMP_NULL;
        break;

    case PAL_DEVICE_TYPE_xD:
        if (m_pxDDriver)
            f_releasedriver(m_pxDDriver);

        m_pxDDriver = MMP_NULL;
        break;
    }

    return result;
#else
    return 0;
#endif
}

MMP_INT
PalGetPartitionCount(
    MMP_UINT cardType,
    void *drvInit)
{
#if defined(HAVE_FAT)
    MMP_INT   result         = 0;
    MMP_UINT8 hccResult      = 0;
    F_DRIVER  *tmp_pDrive    = MMP_NULL;
    MMP_INT   partitionCount = 0;

    switch (cardType)
    {
    case PAL_DEVICE_TYPE_SD:
        if (!m_pSDDriver)
            hccResult = f_createdriver(&m_pSDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pSDDriver;
        break;

    case PAL_DEVICE_TYPE_USB0:
        if (!m_pUSB0Driver)
            hccResult = f_createdriver(&m_pUSB0Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB0Driver;
        break;

    case PAL_DEVICE_TYPE_USB1:
        if (!m_pUSB1Driver)
            hccResult = f_createdriver(&m_pUSB1Driver, drvInit, F_AUTO_ASSIGN); \

        tmp_pDrive = m_pUSB1Driver;
        break;

    case PAL_DEVICE_TYPE_USB2:
        if (!m_pUSB2Driver)
            hccResult = f_createdriver(&m_pUSB2Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB2Driver;
        break;

    case PAL_DEVICE_TYPE_USB3:
        if (!m_pUSB3Driver)
            hccResult = f_createdriver(&m_pUSB3Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB3Driver;
        break;

    case PAL_DEVICE_TYPE_USB4:
        if (!m_pUSB4Driver)
            hccResult = f_createdriver(&m_pUSB4Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB4Driver;
        break;

    case PAL_DEVICE_TYPE_USB5:
        if (!m_pUSB5Driver)
            hccResult = f_createdriver(&m_pUSB5Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB5Driver;
        break;

    case PAL_DEVICE_TYPE_USB6:
        if (!m_pUSB6Driver)
            hccResult = f_createdriver(&m_pUSB6Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB6Driver;
        break;

    case PAL_DEVICE_TYPE_USB7:
        if (!m_pUSB7Driver)
            hccResult = f_createdriver(&m_pUSB7Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pUSB7Driver;
        break;

    case PAL_DEVICE_TYPE_PTP:
        //result = PtpInitVolume();
        //return result;
        break;

    case PAL_DEVICE_TYPE_NOR:
        if (!m_pNORDriver)
            hccResult = f_createdriver(&m_pNORDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNORDriver;
        break;

    case PAL_DEVICE_TYPE_NORPRIVATE:
        if (!m_pNORPRIVATEDriver)
            hccResult = f_createdriver(&m_pNORPRIVATEDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNORPRIVATEDriver;
        break;

    case PAL_DEVICE_TYPE_NAND:
        if (!m_pNANDDriver)
            hccResult = f_createdriver(&m_pNANDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pNANDDriver;
        break;

    case PAL_DEVICE_TYPE_SD2:
        if (!m_pSD2Driver)
            hccResult = f_createdriver(&m_pSD2Driver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pSD2Driver;
        break;

    case PAL_DEVICE_TYPE_CF:
        if (!m_pCFDriver)
            hccResult = f_createdriver(&m_pCFDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pCFDriver;
        break;

    case PAL_DEVICE_TYPE_MS:
        if (!m_pMSDriver)
            hccResult = f_createdriver(&m_pMSDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pMSDriver;
        break;

    case PAL_DEVICE_TYPE_xD:
        if (!m_pxDDriver)
            hccResult = f_createdriver(&m_pxDDriver, drvInit, F_AUTO_ASSIGN);

        tmp_pDrive = m_pxDDriver;
        break;
    }

    if (hccResult == F_NO_ERROR)
    {
        F_PARTITION partitionInfo[12] = {0};
        MMP_INT     parNum            = sizeof(partitionInfo) / sizeof(partitionInfo[0]);

        partitionCount = 0;
        result         = f_getpartition(tmp_pDrive, parNum, partitionInfo);
        if (result == F_NO_ERROR)
        {
            MMP_INT i = 0;

            printf("\n");
            for (i = 0; i < parNum; i++)
            {
                if (partitionInfo[i].secnum != 0)
                {
                    partitionCount++;
                    printf("Partition %02d-th: (secnum, system_indicator) = (%d, %d)\n\n", i, partitionInfo[i].secnum, partitionInfo[i].system_indicator);
                }
            }
        }
        else
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Get partition error (%d) !!\n", result);
            partitionCount = 0;
        }
    }
    else
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "f_createdriver() fail (%d)!!\n", hccResult);
        partitionCount = 0;
    }

    return partitionCount;
#else

    return 0;
#endif
}

MMP_INT
PalGetTimeDate(
    const MMP_CHAR *filename,
    MMP_UINT16 *sec,
    MMP_UINT16 *minute,
    MMP_UINT16 *hour,
    MMP_UINT16 *day,
    MMP_UINT16 *month,
    MMP_UINT16 *year)
{
#if defined(HAVE_FAT)
    MMP_INT    result;
    MMP_UINT16 t, d;
    LOG_ENTER "PalGetTimeDate(filename=0x%X,sec=0x%X,minute=0x%X,hour=0x%X,day=0x%X,month=0x%X,year=0x%X)\r\n",
    filename, sec, minute, hour, day, month, year LOG_END

    if (!f_gettimedate(filename, &t, &d))
    {
        sec[0]    = ((t & 0x001f) << 1);
        minute[0] = ((t & 0x07e0) >> 5);
        hour[0]   = ((t & 0xf800) >> 11);
        day[0]    = (d & 0x001f);
        month[0]  = ((d & 0x01e0) >> 5);
        year[0]   = 1980 + ((d & 0xf800) >> 9);
        result    = 0;
    }
    else
    {
        result = 1;
    }

    LOG_LEAVE "PalGetTimeDate()=0x%X\r\n", result LOG_END
    return result;
#else
    sec[0]    = 0;
    minute[0] = 0;
    hour[0]   = 0;
    day[0]    = 0;
    month[0]  = 0;
    year[0]   = 0;
    return 0;
#endif
}

MMP_INT
PalWGetTimeDate(
    const MMP_WCHAR *filename,
    MMP_UINT16 *sec,
    MMP_UINT16 *minute,
    MMP_UINT16 *hour,
    MMP_UINT16 *day,
    MMP_UINT16 *month,
    MMP_UINT16 *year)
{
    MMP_INT    result = MMP_SUCCESS;
    MMP_UINT16 t, d;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s()\r\n", __FUNCTION__);

#if defined(HAVE_FAT)
    if (!f_wgettimedate(filename, &t, &d) )
    {
        if (sec) *sec = t & F_CTIME_SEC_MASK;
        if (minute) *minute = (t & F_CTIME_MIN_MASK) >> F_CTIME_MIN_SHIFT;
        if (hour) *hour = (t & F_CTIME_HOUR_MASK) >> F_CTIME_HOUR_SHIFT;
        if (day) *day = d & F_CDATE_DAY_MASK;
        if (month) *month = (d & F_CDATE_MONTH_MASK) >> F_CDATE_MONTH_SHIFT;
        if (year) *year = 1980 + ((d & F_CDATE_YEAR_MASK) >> F_CDATE_YEAR_SHIFT);
    }
    else
    {
        result = MMP_RESULT_ERROR;
    }
#else
    if (sec) *sec = 0;
    if (minute) *minute = 0;
    if (hour) *hour = 0;
    if (day) *day = 0;
    if (month) *month = 0;
    if (year) *year = 0;
#endif
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;
}

MMP_INT
PalWSetTimeDate(
    const MMP_WCHAR *filename,
    MMP_UINT16 sec,
    MMP_UINT16 minute,
    MMP_UINT16 hour,
    MMP_UINT16 day,
    MMP_UINT16 month,
    MMP_UINT16 year)
{
    MMP_INT    result = MMP_SUCCESS;
    MMP_UINT16 t      = 0, d = 0;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(sec=%d,min=%d,hour=%d,day=%d,month=%d,year=%d)\r\n",
            __FUNCTION__, sec, minute, hour, day, month, year);

#if defined(HAVE_FAT)
    d = (((year - 1980) << F_CDATE_YEAR_SHIFT) & F_CDATE_YEAR_MASK) |
        (((month ) << F_CDATE_MONTH_SHIFT) & F_CDATE_MONTH_MASK) |
        (day & F_CDATE_DAY_MASK);

    t = ((hour << F_CTIME_HOUR_SHIFT) & F_CTIME_HOUR_MASK) |
        ((minute << F_CTIME_MIN_SHIFT) & F_CTIME_MIN_MASK) |
        (sec & F_CTIME_SEC_MASK);

    result = f_wsettimedate(filename, t, d);
#endif

    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;
}

MMP_INT64
PalGetFileLengthEx(
    const MMP_CHAR *filename)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = 0;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(filename=%s)\r\n", __FUNCTION__, filename);

    result = f_filelength(filename);
    if (result == -1)
        result = 0;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
        #if (CONFIG_HAVE_NTFS)
    struct stat buf;
        #endif /* (CONFIG_HAVE_NTFS) */
    MMP_INT64   result;

    LOG_ENTER "PalGetFileLength(filename=0x%X)\r\n",
    filename LOG_END

        #if (CONFIG_HAVE_NTFS)
    if (sxa_ntfs_stat(filename, &buf))
    {
        result = 0;
        goto end;
    }
    result = buf.st_size;
    goto end;
        #endif /* (CONFIG_HAVE_NTFS) */
    result = f_filelength(filename);
    if (result == -1)
        result = 0;

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalGetFileLength()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT32
PalGetFileLength(
    const MMP_CHAR *filename)
{
    return (MMP_INT32) PalGetFileLengthEx(filename);
}

MMP_INT32
PalWGetFileLength(
    const MMP_WCHAR *filename)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT64 result = 0LL;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(filename=%s)\r\n", __FUNCTION__, filename);
    {
        #if defined(CONFIG_HAVE_NTFS)
        struct stat buf = {0};

        if (!sxa_ntfs_stat(filename, &buf))
        {
            result = buf.st_size;
        }
        else
        #endif
        {
            result = f_wfilelength(filename);
            if (result == -1)
                result = 0;
        }
    }

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return (MMP_INT32)result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT32 result;

    LOG_ENTER "PalWGetFileLength(filename=0x%X)\r\n",
    filename LOG_END

    result = f_wfilelength(filename);
    if (result == -1)
        result = 0;

    LOG_LEAVE "PalWGetFileLength()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_UINT
PalGetFindAttr(
    PAL_FILE_FIND find)
{
#if defined(HAVE_FAT)

    MMP_UINT result;
    FN_WFIND *handle;

    PalAssert(find);

    handle = (FN_WFIND *) find;
    result = handle->attr;

    return result;
#else
    return 0;
#endif
}

MMP_INT32
PalMakeDir(
    const MMP_CHAR *filename)
{
#if defined(HAVE_FAT)

    MMP_INT32 result;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(filename=%s)\r\n", __FUNCTION__, filename);

    result = f_mkdir(filename);

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);
    return result;
#else
    return 0;
#endif
}

MMP_INT32
PalWMakeDir(
    const MMP_WCHAR *filename)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT32 result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s()\r\n", __FUNCTION__);

        #if defined(CONFIG_HAVE_NTFS)
    if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)) )
    {
        result = sxa_ntfs_mkdir(filename, 0755);
    }
    else
        #endif
    result = f_wmkdir(filename);

    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT32 result;

    LOG_ENTER "PalWMakeDir(filename=0x%X)\r\n",
    filename LOG_END

        #if (CONFIG_HAVE_NTFS)
    if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*filename)))
    {
        result = sxa_ntfs_mkdir(filename, 0755);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */
    result = f_wmkdir(filename);

        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    LOG_LEAVE "PalWMakeDir()=0x%X\r\n", result LOG_END
    return result;
    #else
    return 0;
    #endif
#endif
}

MMP_INT
PalRemoveDir(
    const MMP_CHAR *dirname,
    PAL_FILE_DELETE_CALLBACK callback,
    void *callback_arg)
{
#if defined(HAVE_FAT)
    MMP_INT result = 0;

    result = f_rmdir(dirname);

    return result;
#else
    return 0;
#endif
}

MMP_INT
PalWRemoveDir(
    const MMP_WCHAR *dirname,
    PAL_FILE_DELETE_CALLBACK callback,
    void *callback_arg)
{
#ifdef WL_REFINED

    #if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_INT result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s()\r\n", __FUNCTION__);

        #if defined(CONFIG_HAVE_NTFS)
    if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*dirname)))
    {
        result = sxa_ntfs_rmdir(dirname);
    }
    else
        #endif
    result = f_wrmdir(dirname);

end:
    if (result != MMP_SUCCESS)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\t%s() errCode = %d\n", __FUNCTION__, result);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return result;

    #else /** defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS) **/
    return 0;
    #endif

#else

    #ifdef HAVE_FAT
    MMP_INT result = 0;
        #if (CONFIG_HAVE_NTFS)
    if (sxa_ntfs_mounted(sxa_ntfs_get_mount_point(*dirname)))
    {
        result = sxa_ntfs_rmdir(dirname);
        goto end;
    }
        #endif /* (CONFIG_HAVE_NTFS) */
        #ifdef SMTK_FILE_PAL
    result = f_wrmdir(dirname);
        #else
    result = _wrmdir(dirname);
        #endif
        #if (CONFIG_HAVE_NTFS)
end:
        #endif /* (CONFIG_HAVE_NTFS) */
    return result;
    #else
    return 0;
    #endif
#endif
}

//TODO: open card polling function
PalFormat(
    MMP_UINT drvnumber,
    MMP_UINT cardType,
    MMP_INT partitionIdx,
    void *drvInit)
{
#if defined(HAVE_FAT)
    MMP_INT   result    = 0;
    MMP_UINT8 hccResult = 0;
    F_DRIVER  *pDrive   = MMP_NULL;

    switch (cardType)
    {
    case PAL_DEVICE_TYPE_SD:
        if (!m_pSDDriver)
            hccResult = f_createdriver(&m_pSDDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pSDDriver;
        break;

    case PAL_DEVICE_TYPE_SD2:
        if (!m_pSD2Driver)
            hccResult = f_createdriver(&m_pSD2Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pSD2Driver;
        break;

    case PAL_DEVICE_TYPE_MS:
        if (!m_pMSDriver)
            hccResult = f_createdriver(&m_pMSDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pMSDriver;
        break;

    case PAL_DEVICE_TYPE_MMC:
        if (!m_pMMCDriver)
            hccResult = f_createdriver(&m_pMMCDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pMMCDriver;
        break;

    case PAL_DEVICE_TYPE_USB0:
        if (!m_pUSB0Driver)
            hccResult = f_createdriver(&m_pUSB0Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB0Driver;
        break;

    case PAL_DEVICE_TYPE_USB1:
        if (!m_pUSB1Driver)
            hccResult = f_createdriver(&m_pUSB1Driver, drvInit, F_AUTO_ASSIGN); \

        pDrive = m_pUSB1Driver;
        break;

    case PAL_DEVICE_TYPE_USB2:
        if (!m_pUSB2Driver)
            hccResult = f_createdriver(&m_pUSB2Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB2Driver;
        break;

    case PAL_DEVICE_TYPE_USB3:
        if (!m_pUSB3Driver)
            hccResult = f_createdriver(&m_pUSB3Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB3Driver;
        break;

    case PAL_DEVICE_TYPE_USB4:
        if (!m_pUSB4Driver)
            hccResult = f_createdriver(&m_pUSB4Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB4Driver;
        break;

    case PAL_DEVICE_TYPE_USB5:
        if (!m_pUSB5Driver)
            hccResult = f_createdriver(&m_pUSB5Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB5Driver;
        break;

    case PAL_DEVICE_TYPE_USB6:
        if (!m_pUSB6Driver)
            hccResult = f_createdriver(&m_pUSB6Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB6Driver;
        break;

    case PAL_DEVICE_TYPE_USB7:
        if (!m_pUSB7Driver)
            hccResult = f_createdriver(&m_pUSB7Driver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pUSB7Driver;
        break;

    case PAL_DEVICE_TYPE_PTP:
        //result = PtpInitVolume();
        return result;
        break;

    case PAL_DEVICE_TYPE_NOR:
        if (!m_pNORDriver)
            hccResult = f_createdriver(&m_pNORDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pNORDriver;
        break;

    case PAL_DEVICE_TYPE_NORPRIVATE:
        if (!m_pNORPRIVATEDriver)
            hccResult = f_createdriver(&m_pNORPRIVATEDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pNORPRIVATEDriver;
        break;

    case PAL_DEVICE_TYPE_NAND:
        if (!m_pNANDDriver)
            hccResult = f_createdriver(&m_pNANDDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pNANDDriver;
        break;

    case PAL_DEVICE_TYPE_CF:
        if (!m_pCFDriver)
            hccResult = f_createdriver(&m_pCFDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pCFDriver;
        break;

    case PAL_DEVICE_TYPE_xD:
        if (!m_pxDDriver)
            hccResult = f_createdriver(&m_pxDDriver, drvInit, F_AUTO_ASSIGN);

        pDrive = m_pxDDriver;
        break;
    }

    if (hccResult == F_NO_ERROR)
    {
        F_PARTITION partitionInfo[4] = {0};
        MMP_LONG    formatType       = 0;

        hccResult = f_getpartition(pDrive, 4, partitionInfo);
        if (hccResult == F_NO_ERROR)
        {
            switch (partitionInfo[partitionIdx].system_indicator)
            {
            case F_SYSIND_DOSFAT12:
                formatType = F_FAT12_MEDIA;
                break;

            case F_SYSIND_DOSFAT16UPTO32MB:
            case F_SYSIND_DOSFAT16OVER32MB:
                formatType = F_FAT16_MEDIA;
                break;

            case F_SYSIND_DOSFAT32:
            default:
                formatType = F_FAT32_MEDIA;
                break;
            }

            hccResult = f_format(drvnumber, formatType);
            if (hccResult)
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Format fial (%d) !!\n", hccResult);
                goto end;
            }

            f_delvolume(drvnumber);

            hccResult = f_initvolumepartition(drvnumber, pDrive, partitionIdx);
            if (hccResult)
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "Init volume fial (%d) !!", hccResult);
                goto end;
            }
        }
        else
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "f_getpartition() error (%d) !!", hccResult);
        }
    }
    else
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "f_createdriver() fail (%d)!!", hccResult);
    }

end:
    if (hccResult != F_NO_ERROR)
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\terrCode = %d\n", hccResult);
    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Leave] %s()\r\n", __FUNCTION__);

    return hccResult;
#else

    return 0;
#endif
}

MMP_INT
PalGetAttr(
    const MMP_CHAR *filename,
    MMP_UINT8 *attr)
{
#if defined(HAVE_FAT)
    MMP_INT result = 0;

    result = f_getattr(filename, attr);

    return result;
#else
    return 0;
#endif
}

MMP_INT
PalWGetAttr(
    const MMP_WCHAR *filename,
    MMP_UINT8 *attr)
{
#if defined(HAVE_FAT)
    MMP_INT result = 0;

    result = f_wgetattr(filename, attr);

    return result;
#else
    return 0;
#endif
}

MMP_INT
PalSetAttr(
    const MMP_CHAR *filename,
    MMP_UINT8 attr)
{
#if defined(HAVE_FAT)
    MMP_INT result = 0;

    result = f_setattr(filename, attr);

    return result;
#else
    return 0;
#endif
}

MMP_INT
PalWSetAttr(
    const MMP_WCHAR *filename,
    MMP_UINT8 attr)
{
#if defined(HAVE_FAT)
    MMP_INT result = 0;

    result = f_wsetattr(filename, attr);

    return result;
#else
    return 0;
#endif
}