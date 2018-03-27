#include "config.h"
#include "pal/pal.h"
#include <windows.h>

void
PalSleep(
    MMP_ULONG ms)
{
    Sleep(ms);
}

static DWORD WINAPI
ThreadFunc(
    LPVOID param)
{
    PAL_THREAD_PROC proc;
    DWORD* params;
    void* result;

    LOG_ENTER "ThreadFunc(param=0x%X)\r\n", param LOG_END
    PalAssert(param);

    params = (DWORD*) param;

    proc = (PAL_THREAD_PROC) params[0];
    result = proc((void*) params[1]);

    PalHeapFree(PAL_HEAP_DEFAULT, params);

    LOG_LEAVE "ThreadFunc()=0x%X\r\n", result LOG_END
    return (DWORD) result;
}

PAL_THREAD
PalCreateThread(
    MMP_INT name,
    PAL_THREAD_PROC proc,
    void* arg,
    MMP_ULONG stackSize,
    MMP_UINT priority)
{
    HANDLE thread;
    DWORD* params;

    LOG_ENTER "CreateThread(proc=0x%X,arg=0x%X,stackSize=%d,priority=%d)\r\n",
        proc, arg, stackSize, priority LOG_END

    PalAssert(proc);
    PalAssert(stackSize);

    params = PalHeapAlloc(PAL_HEAP_DEFAULT, 2 * sizeof (DWORD));
    if (!params)
    {
        PalAssert(!"Out of memory");
        thread = NULL;
        goto end;
    }

    params[0] = (DWORD) proc;
    params[1] = (DWORD) arg;

    thread = CreateThread(NULL, stackSize, ThreadFunc, params, 0, NULL);
    if (thread == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateThread FAIL");
    }

end:
    LOG_LEAVE "CreateThread()=0x%X\r\n", thread LOG_END
    return thread;
}

MMP_INT
PalDestroyThread(
    PAL_THREAD thread)
{
    MMP_BOOL result;

    LOG_ENTER "DestroyThread(thread=0x%X)\r\n", thread LOG_END

    PalAssert(thread);

    // Wait thread exit
    result = WaitForSingleObject(thread, INFINITE);
    if (result == WAIT_FAILED)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"WaitForSingleObject FAIL");
        goto end;
    }

    result = CloseHandle(thread);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
        goto end;
    }

end:
    LOG_LEAVE "DestroyThread()=%d\r\n", !result LOG_END
    return !result;
}
