#include "config.h"
#include "pal/pal.h"
#include "host/ahb.h"

#ifndef UNICODE
    #define UNICODE
#endif

#ifdef SMTK_FILE_PAL
#include "common/fat.h"
#include "mmc/single/mmc_smedia.h"
#include "ftl/src/ftl.h"
#include "nor/nordrv_f.h"
#else
#include <windows.h>
#include <direct.h>
#include <stdio.h>
#endif

#include <windows.h>
#include <stdio.h>

//=============================================================================
//                              Constant Definition
//=============================================================================
#define TIMEOUT             1000
#define THREAD_STACK_SIZE   4096

#ifdef __FREERTOS__
#define APB_GPIO_OUTPUT_DATA_REG    0x68000000
#define APB_GPIO_INPUT_DATA_REG     0x68000004
#define APB_GPIO_DIRECT_REG         0x68000008
#else
#define APB_GPIO_OUTPUT_DATA_REG    0x7800
#define APB_GPIO_INPUT_DATA_REG     0x7804
#define APB_GPIO_DIRECT_REG         0x7808
#endif

#define REG_BIT_XD_CARD_DETECT          (1u << 17)

#define REG_BIT_MMC_CARD_DETECT         (1u << 19)
#define REG_BIT_MMC_CARD_POWER          (1u << 21)

#define REG_BIT_MS_CARD_DETECT          (1u << 20)
#define REG_BIT_MS_CARD_POWER           (1u << 21)

/** measure Pal I/O duration **/
//#define FILE_OPERATION_TIME_MEASURE

//=============================================================================
//                              Macro Definition
//=============================================================================
#if defined(FILE_OPERATION_TIME_MEASURE)
    #define _timeMeasure(string, args...)       dbg_msg(1, string, ## args)
#else
    #define _timeMeasure(string, args...)
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct MESSAGE_TAG
{
    void*               func;
    PAL_FILE_CALLBACK   callback;
    void*               callback_arg;
    void*               arg1;
    void*               arg2;
    void*               arg3;
    void*               arg4;
} MESSAGE;

typedef struct FILE_FIND_TAG
{
    HANDLE          handle;
    WIN32_FIND_DATA data;
    MMP_CHAR        filename[MAX_PATH];
} FILE_FIND;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MESSAGE msgBuf;
static PAL_MSGQ queue;
static MMP_MUTEX mutex;
static PAL_THREAD thread;
static volatile MMP_BOOL destroyed;
static MMP_BOOL inited = MMP_FALSE;

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void*
FileThreadFunc(
    void* arg)
{
    MMP_ULONG result = 0;
    MMP_UINT32 clock = 0;
    LOG_ENTER "FileThreadFunc(arg=0x%X)\r\n", arg LOG_END

    for (;;)
    {
        MESSAGE msg;
        MMP_ULONG msgSize   = 0;
        void* handle        = MMP_NULL;
        void* callback_arg  = MMP_NULL;

begin:
        result = PalReadMsgQ(queue, &msg, sizeof (msg), &msgSize, PAL_MSGQ_INFINITE);
        if (result != 0)
        {
            LOG_ERROR "READ MSGQ FAIL:%d\r\n", result LOG_END
            goto begin;
        }

        //PalAssert(msgSize == sizeof (msg));
        if (msg.func == MMP_NULL) // Exit message
        {
            result      = 0;
            destroyed   = MMP_TRUE;
            goto end;
        }

        callback_arg = msg.callback_arg;
        // Invoke the real function
        if (msg.func == (void*) PalFileRead)
        {
            handle = msg.arg4; // stream
            result = PalFileRead(
                msg.arg1,               // buffer
                (MMP_SIZE_T) msg.arg2,  // size
                (MMP_SIZE_T) msg.arg3,  // count
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileWrite)
        {
            handle = msg.arg4; // stream
            result = PalFileWrite(
                msg.arg1,               // buffer
                (MMP_SIZE_T) msg.arg2,  // size
                (MMP_SIZE_T) msg.arg3,  // count
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileSeek)
        {
            handle = msg.arg1; // stream
            result = PalFileSeek(
                handle,
                (MMP_LONG) msg.arg2,    // offset
                (MMP_INT) msg.arg3,     // origin
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileTell)
        {
            handle = msg.arg1; // stream
            result = PalFileTell(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileEof)
        {
            handle = msg.arg1; // stream
            result = PalFileEof(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileOpen)
        {
            handle = PalFileOpen(
                (const MMP_CHAR*) msg.arg1, // filename
                (MMP_UINT) msg.arg2,        // mode
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void*) PalWFileOpen)
        {
            handle = PalWFileOpen(
                (const MMP_WCHAR*) msg.arg1,    // filename
                (MMP_UINT) msg.arg2,            // mode
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void*) PalFileClose)
        {
            handle = msg.arg1; // stream
            result = PalFileClose(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileDelete)
        {
            handle = MMP_NULL;
            result = PalFileDelete(
                (const MMP_CHAR*) msg.arg1, // filename
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalWFileDelete)
        {
            handle = MMP_NULL;
            result = PalWFileDelete(
                (const MMP_WCHAR*) msg.arg1, // filename
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalDiskGetFreeSpace)
        {
            handle = MMP_NULL;
            result = PalDiskGetFreeSpace(
                (const MMP_INT) msg.arg1, // dirnum
                (MMP_UINT32*)   msg.arg2,
                (MMP_UINT32*)   msg.arg3,   
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileFindFirst)
        {
            handle = PalFileFindFirst(
                (const MMP_CHAR*) msg.arg1, // filename
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void*) PalWFileFindFirst)
        {
            handle = PalWFileFindFirst(
                (const MMP_WCHAR*) msg.arg1, // filename
                MMP_NULL, MMP_NULL);

            result = (MMP_INT) handle;
        }
        else if (msg.func == (void*) PalFileFindNext)
        {
            handle = msg.arg1; // find
            result = PalFileFindNext(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalWFileFindNext)
        {
            handle = msg.arg1; // find
            result = PalWFileFindNext(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else if (msg.func == (void*) PalFileFindClose)
        {
            handle = msg.arg1; // find
            result = PalFileFindClose(
                handle,
                MMP_NULL, MMP_NULL);
        }
        else
        {
            LOG_ERROR "UNKNOWN MESSAGE:0x%X\r\n", msg.func LOG_END
            //PalAssert(!"UNKNOWN MESSAGE");
            goto begin;
        }

        // Invoke the callback function
        msg.callback(handle, result, callback_arg);
    }

end:
    LOG_LEAVE "FileThreadFunc()=%d\r\n", result LOG_END
    return (void*) result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_INT
PalFileInitialize(
    void)
{
    MMP_INT result;
    LOG_ENTER "PalFileInitialize()\r\n" LOG_END

    // Crate mutex
    mutex = PalCreateMutex(PAL_MUTEX_FILE);
    if (!mutex)
    {
        LOG_ERROR "CREATE FILE MUTEX FAIL\r\n" LOG_END
        result = 1;
        goto end;
    }

    // Crate message queue
    queue = PalCreateMsgQ(PAL_MSGQ_FILE);
    if (!queue)
    {
        LOG_ERROR "CREATE FILE MSGQ FAIL\r\n" LOG_END
        PalDestroyMutex(mutex);
        result = 1;
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
        result = 1;
        goto end;
    }

    destroyed   = MMP_FALSE;
    result      = 0;

end:
    LOG_LEAVE "PalFileInitialize()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalFileTerminate(
    void)
{
    MMP_INT result;
    LOG_ENTER "PalFileTerminate()\r\n" LOG_END

    // Write quit message to queue
    msgBuf.func = MMP_NULL;
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        LOG_ERROR "WRITE EXIT MESSAGE FAIL\r\n" LOG_END
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
}

PAL_FILE*
PalFileOpen(
    const MMP_CHAR*         filename,
    MMP_UINT                mode,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;
#ifdef SMTK_FILE_PAL
    F_FILE* file;
#else
    FILE* file;
#endif
    LOG_ENTER "PalFileOpen(filename=%s,mode=%d,callback=0x%X)\r\n",
        filename, mode, callback LOG_END

    if (!callback) // Sync mode
    {
        switch (mode)
        {
        case PAL_FILE_RB:
#ifdef SMTK_FILE_PAL
            file = f_open(filename, "r");
#else
            file = fopen(filename, "rb");
#endif
            break;

        case PAL_FILE_WB:
#ifdef SMTK_FILE_PAL
            file = f_open(filename, "w");
#else
            file = fopen(filename, "wb");
#endif
            break;

        case PAL_FILE_AB:
#ifdef SMTK_FILE_PAL
            file = f_open(filename, "a");
#else
            file = fopen(filename, "ab");
#endif
            break;

        case PAL_FILE_RBP:
#ifdef SMTK_FILE_PAL
            file = f_open(filename, "r+");
#else
            file = fopen(filename, "rb+");
#endif
            break;

        case PAL_FILE_WBP:
#ifdef SMTK_FILE_PAL
            file = f_open(filename, "w+");
#else
            file = fopen(filename, "wb+");
#endif
            break;

        case PAL_FILE_ABP:
#ifdef SMTK_FILE_PAL
            file = f_open(filename, "a+");
#else
            file = fopen(filename, "ab+");
#endif
            break;

        default:
            file = MMP_NULL;
        }
        //PalAssert(file);
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
        {
            file = (PAL_FILE*) result;
            goto end;
        }
        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE*) result;
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileOpen;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) filename;
    msgBuf.arg2         = (void*) mode;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE*) result;
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        file = (PAL_FILE*) result;
        goto end;
    }

    file = MMP_NULL;

end:
    LOG_LEAVE "PalFileOpen()=0x%X\r\n", file LOG_END
    return file;
}

PAL_FILE*
PalWFileOpen(
    const MMP_WCHAR*        filename,
    MMP_UINT                mode,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;
#ifdef SMTK_FILE_PAL
    F_FILE* file;
#else
    FILE* file;
#endif
    LOG_ENTER "PalWFileOpen(filename=0x%X,mode=%d,callback=0x%X)\r\n",
        filename, mode, callback LOG_END

    if (!callback) // Sync mode
    {
        switch (mode)
        {
        case PAL_FILE_RB:
#ifdef SMTK_FILE_PAL
            file = f_wopen(filename, L"r");
#else
            file = _wfopen(filename, L"rb");
#endif
            break;

        case PAL_FILE_WB:
#ifdef SMTK_FILE_PAL
            file = f_wopen(filename, L"w");
#else
            file = _wfopen(filename, L"wb");
#endif
            break;

        case PAL_FILE_AB:
#ifdef SMTK_FILE_PAL
            file = f_wopen(filename, L"a");
#else
            file = _wfopen(filename, L"ab");
#endif
            break;

        case PAL_FILE_RBP:
#ifdef SMTK_FILE_PAL
            file = f_wopen(filename, L"r+");
#else
            file = _wfopen(filename, L"rb+");
#endif
            break;

        case PAL_FILE_WBP:
#ifdef SMTK_FILE_PAL
            file = f_wopen(filename, L"w+");
#else
            file = _wfopen(filename, L"wb+");
#endif
            break;

        case PAL_FILE_ABP:
#ifdef SMTK_FILE_PAL
            file = f_wopen(filename, L"a+");
#else
            file = _wfopen(filename, L"ab+");
#endif
            break;

        default:
            file = MMP_NULL;
        }
        //PalAssert(file);
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
        {
            file = (PAL_FILE*) result;
            goto end;
        }
        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE*) result;
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalWFileOpen;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) filename;
    msgBuf.arg2         = (void*) mode;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if (result != 0)
    {
        file = (PAL_FILE*) result;
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if (result != 0)
    {
        file = (PAL_FILE*) result;
        goto end;
    }

    file = MMP_NULL;

end:
    LOG_LEAVE "PalWFileOpen()=0x%X\r\n", file LOG_END
    return file;
}

MMP_INT
PalFileClose(
    PAL_FILE*               stream,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;

    LOG_ENTER "PalFileClose(stream=0x%X,callback=0x%X)\r\n",
        stream, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_close(stream);
#else
        result = fclose(stream);
#endif

        if (result != 0)
        {
            LOG_ERROR "FILE CLOSE ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FILE CLOSE ERROR");
        }
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileClose;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) stream;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

MMP_SIZE_T
PalFileRead(
    void*                   buffer,
    MMP_SIZE_T              size,
    MMP_SIZE_T              count,
    PAL_FILE*               stream,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_SIZE_T result;
    LOG_ENTER "PalFileRead(buffer=0x%X,size=%d,count=%d,stream=0x%X,callback=0x%X)\r\n",
        buffer, size, count, stream, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_read(buffer, size, count, stream);
#else
        result = fread(buffer, size, count, stream);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileRead;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) buffer;
    msgBuf.arg2         = (void*) size;
    msgBuf.arg3         = (void*) count;
    msgBuf.arg4         = (void*) stream;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

MMP_SIZE_T
PalFileWrite(
    const void*             buffer,
    MMP_SIZE_T              size,
    MMP_SIZE_T              count,
    PAL_FILE*               stream,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_SIZE_T result;
    LOG_ENTER "PalFileWrite(buffer=0x%X,size=%d,count=%d,stream=0x%X,callback=0x%X)\r\n",
        buffer, size, count, stream, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_write(buffer, size, count, stream);
#else
        result = fwrite(buffer, size, count, stream);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileWrite;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) buffer;
    msgBuf.arg2         = (void*) size;
    msgBuf.arg3         = (void*) count;
    msgBuf.arg4         = (void*) stream;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
    LOG_LEAVE "PalFileWrite()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalFileSeek(
    PAL_FILE*               stream,
    MMP_LONG                offset,
    MMP_INT                 origin,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalFileSeek(stream=0x%X,offset=%d,origin=%d,callback=0x%X)\r\n",
        stream, offset, origin, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_seek(stream, offset, origin);
#else
        result = fseek(stream, offset, origin);
#endif
        if (result != 0)
        {
            LOG_ERROR "FILE SEEK ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FILE SEEK ERROR");
        }
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileSeek;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) stream;
    msgBuf.arg2         = (void*) offset;
    msgBuf.arg3         = (void*) origin;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
    LOG_LEAVE "PalFileSeek()=%d\r\n", result LOG_END
    return result;
}

MMP_LONG
PalFileTell(
    PAL_FILE*               stream,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_LONG result;
    LOG_ENTER "PalFileTell(stream=0x%X,callback=0x%X)\r\n",
        stream, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_tell(stream);
#else
        result = ftell(stream);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileTell;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) stream;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
    LOG_LEAVE "PalFileTell()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalFileEof(
    PAL_FILE*               stream,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalFileEof(stream=0x%X,callback=0x%X)\r\n",
        stream, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_eof(stream);
#else
        result = feof((FILE*) stream);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileEof;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) stream;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

MMP_INT
PalFileDelete(
    const MMP_CHAR*         filename,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalFileDelete(filename=%s,callback=0x%X)\r\n",
        filename, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_delete(filename);
#else
        TCHAR buf[MAX_PATH];

        mbstowcs(buf, filename, MAX_PATH);

        result = DeleteFile(buf);
        if (result == 0)
        {
            result = GetLastError();
            LOG_ERROR "%s\r\n", PalGetErrorString(result) LOG_END
            //PalAssert(!"DeleteFile FAIL");
        }
        else
        {
            result = 0;
        }
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileDelete;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) filename;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

MMP_INT
PalWFileDelete(
    const MMP_WCHAR*        filename,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalWFileDelete(filename=0x%X,callback=0x%X)\r\n",
        filename, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_wdelete(filename);
#else
        result = DeleteFile(filename);
        if (result == 0)
        {
            result = GetLastError();
            LOG_ERROR "%s\r\n", PalGetErrorString(result) LOG_END
            //PalAssert(!"DeleteFile FAIL");
        }
        else
        {
            result = 0;
        }
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalWFileDelete;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) filename;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

MMP_ULONG
PalDiskGetFreeSpace(
    const MMP_INT           dirnum,
    MMP_UINT32              *free_h,
    MMP_UINT32              *free_l,
    PAL_FILE_CALLBACK       callback,
    void*                   callback_arg)
{
#ifdef WL_REFINED 
        
#if defined(HAVE_FAT) || defined(CONFIG_HAVE_NTFS)
    MMP_ULONG     result = MMP_SUCCESS;

    sdk_msg(SDK_MSG_TYPE_PAL_TRACE, "[Enter] %s(dirnum=%d,cb=0x%X)\r\n", __FUNCTION__, dirnum, callback);

    if( !callback ) // Sync mode
    {
        F_SPACE     space = {0};
#if defined(CONFIG_HAVE_NTFS)
        struct statfs   buf;

        if( sxa_ntfs_statfs(dirnum, &buf) == 0 )
        {
            unsigned long long      size = ((unsigned long long) buf.bavail) * buf.bsize;

            if( free_h )
                *free_h = (MMP_UINT32) (size >> 32);
                
            if( free_l )
                *free_l = (MMP_UINT32) (size & 0x00000000FFFFFFFFULL);           
        }
        else
        {
            result = f_getfreespace(dirnum, &space);
            if( result != F_NO_ERROR )
            {
                sdk_msg_ex(SDK_MSG_TYPE_ERROR, "GET STORE INFO ERROR: %d\r\n", result);
                //PalAssert(!"GET STORE INFO ERROR");
                goto end;
            }

            if( free_h )    *free_h = space.free_high;
            if( free_l )    *free_l = space.free;        
        }
#else
        result = f_getfreespace(dirnum, &space);
        if( result != F_NO_ERROR )
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "GET STORE INFO ERROR: %d\r\n", result);
            //PalAssert(!"GET STORE INFO ERROR");
            goto end;
        }

        if( free_h )    *free_h = space.free_high;
        if( free_l )    *free_l = space.free;
#endif        
        goto end;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if( result != MMP_SUCCESS )
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWaitMutex() fail (%d)!!\n", result);
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalDiskGetFreeSpace;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) dirnum;
    msgBuf.arg2         = (void*) free_h;
    msgBuf.arg3         = (void*) free_l;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
    if( result != MMP_SUCCESS )
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalWriteMsgQ() fail (%d)!!\n", result);
        PalReleaseMutex(mutex);
        goto end;
    }

    result = PalReleaseMutex(mutex);
    if( result != MMP_SUCCESS )
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "PalReleaseMutex() fail (%d)!!\n", result);
        goto end;
    }

end:
    if( result != MMP_SUCCESS )
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
        F_SPACE space;
#if (CONFIG_HAVE_NTFS)
        struct statfs           buf;
        if (sxa_ntfs_statfs(dirnum, &buf) == 0)
        {
            unsigned long long      size = ((unsigned long long) buf.bavail) * buf.bsize;
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
    msgBuf.func         = (void*) PalDiskGetFreeSpace;
    msgBuf.callback     = (CALLBACK) callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) dirnum;
    msgBuf.arg2         = (void*) free_h;
    msgBuf.arg3         = (void*) free_l;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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

PAL_FILE_FIND
PalFileFindFirst(
    const MMP_CHAR*              filename,
    PAL_FILE_FIND_FIRST_CALLBACK callback,
    void*                        callback_arg)
{
    MMP_INT result;
#ifdef SMTK_FILE_PAL
    F_FIND* find;
#else
    FILE_FIND* find = MMP_NULL;
#endif
    LOG_ENTER "PalFileFindFirst(filename=%s,callback=0x%X)\r\n",
        filename, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof (F_FIND));
        if (!find)
        {
            goto end;
        }
        result = f_findfirst(filename, find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND FIRST ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FIND FIRST ERROR");
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
        }
#else
        TCHAR buf[MAX_PATH];

        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof (FILE_FIND));
        if (!find)
        {
            goto end;
        }

        mbstowcs(buf, filename, MAX_PATH);

        find->handle = FindFirstFile(buf, &find->data);
        if (find->handle == INVALID_HANDLE_VALUE)
        {
            LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
            goto end;
        }
        //PalAssert(find->handle);

        wcstombs(find->filename, find->data.cFileName, MAX_PATH);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileFindFirst;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) filename;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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

    find = MMP_NULL;

end:
    LOG_LEAVE "PalFileFindFirst()=0x%X\r\n", find LOG_END
    return find;
}

PAL_FILE_FIND
PalWFileFindFirst(
    const MMP_WCHAR*             filename,
    PAL_FILE_FIND_FIRST_CALLBACK callback,
    void*                        callback_arg)
{
    MMP_INT result;
#ifdef SMTK_FILE_PAL
    FN_WFIND* find = MMP_NULL;
#else
    FILE_FIND* find;
#endif
    LOG_ENTER "PalFileFindFirst(filename=%s,callback=0x%X)\r\n",
        filename, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof (FN_WFIND));
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
#else
        find = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof (FILE_FIND));
        if (!find)
        {
            goto end;
        }

        find->handle = FindFirstFile(filename, &find->data);
        if (find->handle == INVALID_HANDLE_VALUE)
        {
            LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
            PalHeapFree(PAL_HEAP_DEFAULT, find);
            find = MMP_NULL;
            goto end;
        }
        //PalAssert(find->handle);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalWFileFindFirst;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) filename;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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

    find = MMP_NULL;

end:
    LOG_LEAVE "PalFileFindFirst()=0x%X\r\n", find LOG_END
    return find;
}

MMP_INT
PalFileFindNext(
    PAL_FILE_FIND               find,
    PAL_FILE_FIND_NEXT_CALLBACK callback,
    void*                       callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalFileFindNext(find=0x%X,callback=0x%X)\r\n",
        find, callback LOG_END

    //PalAssert(find);

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_findnext((F_FIND*) find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND NEXT ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FIND NEXT ERROR");
            result = 1;
        }
#else
        FILE_FIND* fileFind = (FILE_FIND*) find;

        result = !FindNextFile(fileFind->handle, &fileFind->data);

        if (!result)
            wcstombs(fileFind->filename, fileFind->data.cFileName, MAX_PATH);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileFindNext;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) find;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

MMP_INT
PalWFileFindNext(
    PAL_FILE_FIND               find,
    PAL_FILE_FIND_NEXT_CALLBACK callback,
    void*                       callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalWFileFindNext(find=0x%X,callback=0x%X)\r\n",
        find, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        result = f_wfindnext((FN_WFIND*) find);
        if (result != F_NO_ERROR)
        {
            LOG_ERROR "FIND NEXT ERROR: %d\r\n", result LOG_END
            //PalAssert(!"FIND NEXT ERROR");
            result = 1;
        }
#else
        FILE_FIND* fileFind = (FILE_FIND*) find;

        result = !FindNextFile(fileFind->handle, &fileFind->data);
#endif
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalWFileFindNext;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) find;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
    LOG_LEAVE "PalWFileFindNext()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalFileFindClose(
    PAL_FILE_FIND                find,
    PAL_FILE_FIND_CLOSE_CALLBACK callback,
    void*                        callback_arg)
{
    MMP_INT result;
    LOG_ENTER "PalFileFindClose(find=0x%X,callback=0x%X)\r\n",
        find, callback LOG_END

    if (!callback) // Sync mode
    {
#ifdef SMTK_FILE_PAL
        /*
        FILE_FIND* fileFind = (FILE_FIND*) find;

        result = !FindClose(fileFind->handle);
        if (result)
        {
            LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
            //PalAssert(!"FindClose FAIL");
        }
        */
#endif
        PalHeapFree(PAL_HEAP_DEFAULT, find);
        result = 0;
        goto end;
    }

    if (inited == MMP_FALSE)
    {
        result = PalFileInitialize();
        if (result != 0)
            goto end;

        inited = MMP_TRUE;
    }

    // Async mode
    result = PalWaitMutex(mutex, TIMEOUT);
    if (result != 0)
    {
        goto end;
    }

    // Prepare message
    msgBuf.func         = (void*) PalFileFindClose;
    msgBuf.callback     = callback;
    msgBuf.callback_arg = callback_arg;
    msgBuf.arg1         = (void*) find;

    // Write message to queue
    result = PalWriteMsgQ(queue, &msgBuf, TIMEOUT);
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
}

const MMP_CHAR*
PalFileFindGetName(
    PAL_FILE_FIND find)
{
    MMP_CHAR* result;
#ifdef SMTK_FILE_PAL
    F_FIND* handle;
#else
    FILE_FIND* fileFind;
#endif
    LOG_ENTER "PalFileFindGetName(find=0x%X)\r\n", find LOG_END

    //PalAssert(find);
#ifdef SMTK_FILE_PAL
    handle = (F_FIND*) find;
    result = handle->filename;
#else
    fileFind = (FILE_FIND*) find;
    result = fileFind->filename;
#endif
    LOG_LEAVE "PalFileFindGetName()=0x%X\r\n", result LOG_END
    return result;
}

const MMP_WCHAR*
PalWFileFindGetName(
    PAL_FILE_FIND find)
{
    MMP_WCHAR* result;
#ifdef SMTK_FILE_PAL
    FN_WFIND* handle;
#else
    FILE_FIND* fileFind;
#endif
    LOG_ENTER "PalWFileFindGetName(find=0x%X)\r\n", find LOG_END

    //PalAssert(find);

#ifdef SMTK_FILE_PAL
    handle = (FN_WFIND*) find;
    result = handle->filename;
#else
    fileFind = (FILE_FIND*) find;
    result = fileFind->data.cFileName;
#endif

    LOG_LEAVE "PalWFileFindGetName()=0x%X\r\n", result LOG_END
    return result;
}

const MMP_ULONG
PalFileFindGetSize(
    PAL_FILE_FIND find)
{
    MMP_ULONG result;
#ifdef SMTK_FILE_PAL
    FN_WFIND* handle;
#else
    FILE_FIND* fileFind;
#endif
    LOG_ENTER "PalFileFindGetSize(find=0x%X)\r\n", find LOG_END

    //PalAssert(find);
#ifdef SMTK_FILE_PAL
    handle = (FN_WFIND*) find;
    result = handle->filesize;
#else
    fileFind = (FILE_FIND*) find;
    result = fileFind->data.nFileSizeLow;
#endif
    LOG_LEAVE "PalFileFindGetSize()=0x%X\r\n", result LOG_END
    return result;
}

const MMP_UINT16
PalFileFindGetDate(
    PAL_FILE_FIND find)
{
    MMP_ULONG result;
#ifdef SMTK_FILE_PAL
    FN_WFIND* handle;
#else
    FILE_FIND* fileFind;
#endif
    LOG_ENTER "PalFileFindGetDate(find=0x%X)\r\n", find LOG_END

    //PalAssert(find);
#ifdef SMTK_FILE_PAL
    handle = (FN_WFIND*) find;
    result = handle->cdate;
#else
    fileFind = (FILE_FIND*) find;
    result = fileFind->data.ftCreationTime.dwLowDateTime;
#endif
    LOG_LEAVE "PalFileFindGetDate()=0x%X\r\n", result LOG_END
    return result;
}

const MMP_UINT16
PalFileFindGetTime(
    PAL_FILE_FIND find)
{
    MMP_ULONG result;
#ifdef SMTK_FILE_PAL
    FN_WFIND* handle;
#else
    FILE_FIND* fileFind;
#endif
    LOG_ENTER "PalFileFindGetTime(find=0x%X)\r\n", find LOG_END

    //PalAssert(find);
#ifdef SMTK_FILE_PAL
    handle = (FN_WFIND*) find;
    result = handle->cdate;
#else
    fileFind = (FILE_FIND*) find;
    result = fileFind->data.ftCreationTime.dwHighDateTime;
#endif
    LOG_LEAVE "PalFileFindGetTime()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT
PalFindAttrIsDirectory(
    PAL_FILE_FIND find)
{
    MMP_UINT result;
#ifdef SMTK_FILE_PAL
    FN_WFIND* handle;
#else
    FILE_FIND* fileFind;
#endif
    LOG_ENTER "PalFindAttrIsDirectory(find=0x%X)\r\n", find LOG_END

    //PalAssert(find);
#ifdef SMTK_FILE_PAL
    handle = (FN_WFIND*) find;
    result = (MMP_INT)(handle->attr&F_ATTR_DIR);
#else
    fileFind = (FILE_FIND*) find;
    result = (MMP_INT)(fileFind->data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
#endif
    LOG_LEAVE "PalFindAttrIsDirectory()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT
PalInitVolume(
    MMP_UINT drvnumber,
    MMP_UINT cardtype)
{
   MMP_INT result;
   MMP_UINT32 user_ptr = cardtype;
   LOG_ENTER "PalInitVolume(drvnumber=0x%X,cardtype=0x%X)\r\n", drvnumber,cardtype LOG_END

#ifdef SMTK_FILE_PAL
   switch(drvnumber)
   {
   case PAL_NAND:
       {
           MMP_UINT8 hccResult = 0;
           F_DRIVER* m_pNfDriver = 0;

           hccResult = f_createdriver(&m_pNfDriver, f_ftlinit, F_AUTO_ASSIGN);
           if ( hccResult == F_NO_ERROR )
           {
               result = f_initvolumepartition(drvnumber,m_pNfDriver,0);
               result = f_initvolumepartition(6,m_pNfDriver,1);
           }
       }
       break;

   case PAL_MMC:
       result = f_initvolume(drvnumber,mmp_mmc_initfunc,user_ptr);
       break;

   case 2:
       //result = f_initvolume(drvnumber,mmc_initfunc,user_ptr);
       break;

   case 3:
       //result = f_initvolume(drvnumber,cf_initfunc,user_ptr);
       break;

   case PAL_MS:
#ifdef DTV_MS_ENABLE
       result = f_initvolume(drvnumber,ms_initfunc,user_ptr);
#endif
       break;

   case 5:
       //result = f_initvolume(drvnumber,xd_initfunc,user_ptr);
       break;

   case 6:
       //result = f_initvolume(drvnumber,temp1_initfunc,user_ptr);
       break;

   case 7:
       result = f_initvolume(drvnumber,f_nordrvinit,F_AUTO_ASSIGN);
       break;

   case PAL_USB_0:
       result = f_initvolume(drvnumber,usb0_initfunc,user_ptr);
       break;

   case PAL_USB_1:
       result = f_initvolume(drvnumber,usb1_initfunc,user_ptr);
       break;

   case PAL_USB_2:
       result = f_initvolume(drvnumber,usb2_initfunc,user_ptr);
       break;

   case PAL_USB_3:
       result = f_initvolume(drvnumber,usb3_initfunc,user_ptr);
       break;

   case PAL_USB_4:
       result = f_initvolume(drvnumber,usb4_initfunc,user_ptr);
       break;

   case PAL_USB_5:
       result = f_initvolume(drvnumber,usb5_initfunc,user_ptr);
       break;

   case PAL_USB_6:
       result = f_initvolume(drvnumber,usb6_initfunc,user_ptr);
       break;

   case PAL_USB_7:
       result = f_initvolume(drvnumber,usb7_initfunc,user_ptr);
       break;
   }
#else
    result = 0;
#endif

   LOG_LEAVE "PalInitVolume()=0x%X\r\n", result LOG_END
   return result;
}

MMP_INT
PalDelVolume(
    MMP_UINT drvnumber)
{
   MMP_INT result;
#ifdef SMTK_FILE_PAL
   result = f_delvolume(drvnumber);
#else
   result = 0;
#endif
   return result;
}

MMP_INT
PalGetTimeDate(
    const MMP_WCHAR* filename,
    MMP_UINT16*      sec,
    MMP_UINT16*      minute,
    MMP_UINT16*      hour,
    MMP_UINT16*      day,
    MMP_UINT16*      month,
    MMP_UINT16*      year)
{
    MMP_INT result;
    MMP_UINT16 t =0;
    MMP_UINT16 d =0;
    LOG_ENTER "PalGetTimeDate(filename=0x%X,sec=0x%X,minute=0x%X,hour=0x%X,day=0x%X,month=0x%X,year=0x%X)\r\n",
        filename, sec, minute, hour, day, month, year LOG_END
#ifdef SMTK_FILE_PAL   
    if(!f_gettimedate(filename,&t,&d))
    {
        sec[0] = ((t & 0x001f) << 1);
        minute[0] = ((t & 0x07e0) >> 5);
        hour[0] = ((t & 0xf800) >> 11);
        day[0] = (d & 0x001f);
        month[0] = ((d & 0x01e0) >> 5);
        year[0] = 1980 + ((d & 0xf800) >> 9);
        result = 0;
    }
    else
    {
        result = 1;
    }
#else
    result = 1;
#endif    

    LOG_LEAVE "PalGetTimeDate()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT32
PalGetFileLength(
    const MMP_CHAR* filename)
{
    MMP_INT32 result;
    
    LOG_ENTER "PalGetFileLength(filename=0x%X)\r\n",
        filename LOG_END
        
#ifdef SMTK_FILE_PAL        
    result = f_filelength(filename);
#else
{
    MMP_UINT    dwFileERRCode = 0;
    MMP_UINT32  dwFileSize = 0;
    FILE* file;
    
    file = fopen(filename, "rb");

    /*--- get file size ---*/
    dwFileERRCode = fseek(file, 0L, SEEK_END);
    if (dwFileERRCode < 0) {
        dwFileSize = 0;
    }
    dwFileSize = ftell(file);
    dwFileERRCode = fseek(file, 0L, SEEK_SET);
    if (dwFileERRCode < 0) {
        dwFileSize = 0;
    }
    dwFileERRCode = fclose(file);
    
    result = dwFileSize;
}
#endif
    
    LOG_LEAVE "PalGetTimeDate()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT32
PalWGetFileLength(
    const MMP_WCHAR* filename)
{
    MMP_INT32 result;
    
    LOG_ENTER "PalWGetFileLength(filename=0x%X)\r\n",
        filename LOG_END
        
#ifdef SMTK_FILE_PAL        
        result = f_wfilelength(filename);
#else
    {
        MMP_UINT    dwFileERRCode = 0;
        MMP_UINT32  dwFileSize = 0;
        FILE* file;

        file = _wfopen(filename, L"rb");

        if (file)
        {
            /*--- get file size ---*/
            dwFileERRCode = fseek(file, 0L, SEEK_END);
            if (dwFileERRCode < 0) {
                dwFileSize = 0;
            }
            dwFileSize = ftell(file);
            dwFileERRCode = fseek(file, 0L, SEEK_SET);
            if (dwFileERRCode < 0) {
                dwFileSize = 0;
            }
            dwFileERRCode = fclose(file);

            result = dwFileSize;
        }
        else
            result = 0;
    }
#endif
    
    LOG_LEAVE "PalWGetTimeDate()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT32
PalMakeDir(
    const MMP_CHAR* filename)
{
    MMP_INT32 result;

    LOG_ENTER "PalMakeDir(filename=0x%X)\r\n",
        filename LOG_END

#ifdef SMTK_FILE_PAL 
    result = f_mkdir(filename);
#else
    result = _mkdir(filename);
#endif

    LOG_LEAVE "PalMakeDir()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT32
PalWMakeDir(
    const MMP_WCHAR* filename)
{
    MMP_INT32 result;

    LOG_ENTER "PalWMakeDir(filename=0x%X)\r\n",
        filename LOG_END

#ifdef SMTK_FILE_PAL 
    result = f_wmkdir(filename);
#else
    result = _wmkdir(filename);
#endif

    LOG_LEAVE "PalWMakeDir()=0x%X\r\n", result LOG_END
    return result;
}

MMP_INT
PalRemoveDir(
    const MMP_CHAR*                     dirname,
    PAL_FILE_DELETE_CALLBACK            callback,
    void*                               callback_arg)
{
	MMP_INT result = 0;
#ifdef SMTK_FILE_PAL
    result = f_rmdir(dirname);
#else
    result = _rmdir(dirname);
#endif
	return result;
}

MMP_INT
PalWRemoveDir(
    const MMP_WCHAR*         dirname,
    PAL_FILE_DELETE_CALLBACK callback,
    void*                    callback_arg)
{
	MMP_INT result = 0;
#ifdef SMTK_FILE_PAL
    result = f_wrmdir(dirname);
#else
    result = _wrmdir(dirname);
#endif
	return result;
}

MMP_BOOL
PalPollingMMCSD(
    void)
{
    MMP_BOOL result = MMP_FALSE;
    MMP_UINT32 data;

    LOG_ENTER "PalPollingMMCSD()\r\n" LOG_END
    AHB_ReadRegister(APB_GPIO_INPUT_DATA_REG, &data);
	if (!(data & REG_BIT_MMC_CARD_DETECT))
    {
        result = MMP_TRUE;
    }
    LOG_LEAVE "PalPollingMMCSD()\r\n" LOG_END
    return result;
}

MMP_BOOL
PalPollingMS(
    void)
{
#if 0
    MMP_BOOL result = MMP_FALSE;
    MMP_UINT32 data;

    LOG_ENTER "PalPollingMS()\r\n" LOG_END
    AHB_ReadRegister(APB_GPIO_INPUT_DATA_REG, &data);
	if (!(data & REG_BIT_MS_CARD_DETECT))
    {
        result = MMP_TRUE;
    }
    LOG_LEAVE "PalPollingMS()\r\n" LOG_END
    return result;
#else
    return MMP_FALSE;
#endif
}

void
PalPollingUSB(
    void* cards)
{
#if 0
	MMP_INT result = 0;
    MMP_UINT32 data;
    MMP_UINT8  i= 0;

    MMP_BOOL* tempcards = (MMP_BOOL*)cards;

    LOG_ENTER "PalPollingUSB(cards=0x%X)\r\n",
        cards LOG_END

    for (i=0; i<8; i++)
    {
        result = mmpOtgHost_CheckDeviceIsReady(i);
        if(!result)
        {
            tempcards[i] = MMP_FALSE;
        }
        else
        {
            tempcards[i] = MMP_TRUE;
        }
    }

    LOG_LEAVE "PalPollingUSB()\r\n" LOG_END
#endif
    return;
}

MMP_BOOL
PalPollingXD(
    void)
{
#if 0
    MMP_BOOL result = MMP_FALSE;
    MMP_UINT32 data;

    LOG_ENTER "PalPollingXD()\r\n" LOG_END
    AHB_ReadRegister(APB_GPIO_INPUT_DATA_REG, &data);
	if (!(data & REG_BIT_XD_CARD_DETECT))
    {
        result = MMP_TRUE;
    }
    LOG_LEAVE "PalPollingXD()\r\n" LOG_END
    return result;
#else
    return MMP_FALSE;
#endif
}

MMP_INT
PalGetAttr(
    const MMP_CHAR* filename,
    MMP_UINT8* attr)
{
	MMP_INT result = 0;
#ifdef SMTK_FILE_PAL
    result = f_getattr(filename,attr);
#endif
	return result;
}

MMP_INT
PalWGetAttr(
    const MMP_WCHAR* filename,
    MMP_UINT8* attr)
{
	MMP_INT result = 0;
#ifdef SMTK_FILE_PAL
    result = f_wgetattr(filename,attr);
#endif
	return result;
}

MMP_INT
PalSetAttr(
    const MMP_CHAR* filename,
    MMP_UINT8 attr)
{
	MMP_INT result = 0;
#ifdef SMTK_FILE_PAL
    result = f_setattr(filename,attr);
#endif
	return result;
}

MMP_INT
PalWSetAttr(
    const MMP_WCHAR* filename,
    MMP_UINT8 attr)
{
	MMP_INT result = 0;
#ifdef SMTK_FILE_PAL
    result = f_wsetattr(filename,attr);
#endif
	return result;
}