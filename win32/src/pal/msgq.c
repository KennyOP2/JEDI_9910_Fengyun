#include "config.h"
#include "pal/pal.h"
#include <windows.h>
#include <stdio.h>

#define MAX_MSGQ_COUNT      1

typedef struct MSGQ_TAG
{
    MMP_UINT    name;
    HANDLE      mutex;
    HANDLE      waitMsgEvent;
    HANDLE      msgReadyEvent;
    HANDLE      thread;
    DWORD       threadId;
    DWORD       timeout;
    DWORD*      msg;
    DWORD       msgSize;
} MSGQ;

static DWORD WINAPI
ThreadFunc(
    LPVOID param)
{
    BOOL result = TRUE;
    MSGQ* queue;
    MSG msg;
    LOG_ENTER "ThreadFunc(param=0x%X)\r\n", param LOG_END

    PalAssert(param);

    queue = (MSGQ*) param;

    // Force the system to create the message queue
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    for (;;)
    {
        DWORD beginTime;
begin:

        // Wait reading message event
        result = WaitForSingleObject(queue->waitMsgEvent, INFINITE);
        if (result == WAIT_FAILED)
        {
            LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
            PalAssert(!"WaitForSingleObject FAIL");
            continue;
        }
        else
        {
            result = 0;
        }

        beginTime = timeGetTime();

        // Try to get message from queue
        while (!PeekMessage(&msg, NULL, WM_USER + queue->name, WM_USER + queue->name + 1, PM_REMOVE))
        {
            if (timeGetTime() - beginTime > queue->timeout)
                goto begin; // Timeout

            Sleep(1);
        }

        // If receive exit message
        if (msg.message == WM_USER + queue->name + 1)
        {
            goto end;
        }

        PalAssert(msg.message == WM_USER + queue->name);

        // Set message
        queue->msg      = (DWORD*) msg.lParam;
        queue->msgSize  = msg.wParam;

        // Set message ready event
        result = SetEvent(queue->msgReadyEvent);
        if (result == FALSE)
        {
            LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
            PalAssert(!"SetEvent FAIL");
            result = 1;
        }
        else
        {
            result = 0;
        }
    }

end:
    LOG_LEAVE "ThreadFunc()=0x%X\r\n", !result LOG_END
    return (DWORD) !result;
}

PAL_MSGQ
PalCreateMsgQ(
    MMP_INT name)
{
    MSGQ* queue;
    CHAR buf[32];
    LOG_ENTER "PalCreateMsgQ(name=%d)\r\n", name LOG_END

    // Allocate message queue structure
    queue = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof (MSGQ));
    if (!queue)
    {
        PalAssert(!"Out of memory");
        goto end;
    }

    // Clear variables
    PalMemset(queue, 0, sizeof (MSGQ));

    // Create mutex
    sprintf(buf, "PALMSGQMUTEX%d", name);

    queue->mutex = CreateMutex(NULL, FALSE, buf);
    if (queue->mutex == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateMutex FAIL");

        PalHeapFree(PAL_HEAP_DEFAULT, queue);
        queue = MMP_NULL;
        goto end;
    }

    // Create events
    sprintf(buf, "PALMSGQWAITEVENT%d", name);

    queue->waitMsgEvent = CreateEvent(NULL, FALSE, FALSE, buf);
    if (queue->waitMsgEvent == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateEvent FAIL");

        CloseHandle(queue->mutex);
        PalHeapFree(PAL_HEAP_DEFAULT, queue);
        queue = MMP_NULL;
        goto end;    
    }

    sprintf(buf, "PALMSGQREADYEVENT%d", name);

    queue->msgReadyEvent = CreateEvent(NULL, FALSE, FALSE, buf);
    if (queue->msgReadyEvent == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateEvent FAIL");

        CloseHandle(queue->waitMsgEvent);
        CloseHandle(queue->mutex);
        PalHeapFree(PAL_HEAP_DEFAULT, queue);
        queue = MMP_NULL;
        goto end;    
    }
    
    // Create thread
    queue->thread = CreateThread(NULL, 0, ThreadFunc, queue, 0, &queue->threadId);
    if (queue->thread == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateThread FAIL");

        CloseHandle(queue->msgReadyEvent);
        CloseHandle(queue->waitMsgEvent);
        CloseHandle(queue->mutex);
        PalHeapFree(PAL_HEAP_DEFAULT, queue);
        queue = MMP_NULL;
        goto end;    
    }

    // Wait system queue to be created
    Sleep(100);

    queue->name = name;

end:
    LOG_LEAVE "PalCreateMsgQ()=0x%X\r\n", queue LOG_END
    return queue;
}

MMP_INT
PalDestroyMsgQ(
    PAL_MSGQ queue)
{
    BOOL result;
    MSGQ* palQueue;
    LOG_ENTER "PalDestroyMsgQ(queue=0x%X)\r\n", queue LOG_END

    PalAssert(queue);

    palQueue = (MSGQ*) queue;

    // Post exit message
    result = PostThreadMessage(palQueue->threadId, WM_USER + palQueue->name + 1, 0, 0);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"PostThreadMessage FAIL");
        goto end;
    }

    // Set event to read message
    result = SetEvent(palQueue->waitMsgEvent);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"SetEvent FAIL");
        goto end;
    }

    // Wait thread exit
    result = WaitForSingleObject(palQueue->thread, INFINITE);
    if (result == WAIT_FAILED)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"WaitForSingleObject FAIL");
        goto end;
    }

    // Destroy thread
    result = CloseHandle(palQueue->thread);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
        goto end;
    }
    
    // Destroy events
    result = CloseHandle(palQueue->waitMsgEvent);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
        goto end;
    }

    result = CloseHandle(palQueue->msgReadyEvent);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
        goto end;
    }

    // Destroy mutex
    result = CloseHandle(palQueue->mutex);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
        goto end;
    }

    // Free buffer
    PalHeapFree(PAL_HEAP_DEFAULT, palQueue);

end:
    LOG_LEAVE "PalDestroyMsgQ()=%d\r\n", !result LOG_END
    return !result;
}

MMP_INT
PalReadMsgQ(
    PAL_MSGQ queue,
    void* buf,
    MMP_ULONG bufSize,
    MMP_ULONG* msgSize,
    MMP_ULONG timeout)
{
    DWORD result;
    MSGQ* palQueue;
    LOG_ENTER "PalReadMsgQ(queue=0x%X,buf=0x%X,bufSize=%d,msgSize=0x%X,timeout=%lu)\r\n",
        queue, buf, bufSize, msgSize, timeout LOG_END

    PalAssert(queue);
    PalAssert(buf);
    PalAssert(bufSize > 0);
    PalAssert(msgSize);

    palQueue = (MSGQ*) queue;

    // Wait mutex
    result = WaitForSingleObject(palQueue->mutex, timeout);
    if (result == WAIT_TIMEOUT)
    {
       	result = PAL_MSGQ_EMPTY;
        goto end_no_mutex;
    }
    else if (result == WAIT_FAILED)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"WaitForSingleObject FAIL");
        goto end_no_mutex;
    }

    palQueue->timeout = timeout;

    // Set event to read message
    result = SetEvent(palQueue->waitMsgEvent);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"SetEvent FAIL");
        result = 1;
        goto end;
    }

    // Wait message ready event
    result = WaitForSingleObject(palQueue->msgReadyEvent, timeout);
    if (result == WAIT_TIMEOUT)
    {
       	LOG_WARNING "WAIT EVENT TIMEOUT\r\n" LOG_END
        goto end;
    }
    else if (result == WAIT_FAILED)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"WaitForSingleObject FAIL");
        goto end;
    }
    else
    {
        result = 0;
    }

    // Copy message
    *msgSize = bufSize <= palQueue->msgSize ? bufSize : palQueue->msgSize;

    PalMemcpy(buf, palQueue->msg, *msgSize);

    // Free buffer
    PalHeapFree(PAL_HEAP_DEFAULT, palQueue->msg);

end:
    // Release mutex
    ReleaseMutex(palQueue->mutex);

end_no_mutex:
    LOG_LEAVE "PalReadMsgQ(*msgSize=%d)=%d\r\n", *msgSize, result LOG_END
    return result;
}

MMP_INT
PalWriteMsgQ(
    PAL_MSGQ queue,
    void* msg,
    MMP_ULONG size,
    MMP_ULONG timeout)
{
    BOOL result;
    MSGQ* palQueue;
    DWORD* buf;
    LOG_ENTER "PalWriteMsgQ(queue=0x%X,msg=0x%X,size=%d,timeout=%lu)\r\n",
        queue, msg, size, timeout LOG_END

    PalAssert(queue);
    PalAssert(msg);
    PalAssert(size > 0);

    palQueue = (MSGQ*) queue;

    // Allocate message buffer
    buf = PalHeapAlloc(PAL_HEAP_DEFAULT, size);
    if (!buf)
    {
        PalAssert(!"Out of memory");
        result = TRUE;
        goto end;
    }

    // Copy message to buffer
    PalMemcpy(buf, msg, size);

    // Send message buffer to queue
    result = PostThreadMessage(palQueue->threadId, WM_USER + palQueue->name, (WPARAM) size, (LPARAM) buf);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"PostThreadMessage FAIL");
    }

end:
    LOG_LEAVE "PalWriteMsgQ()=%d\r\n", !result LOG_END
    return !result;
}
