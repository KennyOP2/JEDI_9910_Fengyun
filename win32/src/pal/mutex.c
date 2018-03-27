#include "config.h"
#include "pal/pal.h"
#include <windows.h>
#include <stdio.h>

MMP_MUTEX
PalCreateMutex(
    MMP_INT name)
{
    static MMP_UINT count = 0;
    HANDLE mutex;
    CHAR buf[16];
    LOG_ENTER "PalCreateMutex(name=%d)\r\n", name LOG_END

    // sprintf(buf, "PALMUTEX%d", name);
    sprintf(buf, "%d", count++);

    mutex = CreateMutex(NULL, 0, buf);
    if (mutex == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateMutex FAIL");
    }

    LOG_LEAVE "PalCreateMutex()=0x%X\r\n", mutex LOG_END
    return mutex;
}

MMP_INT
PalDestroyMutex(
    MMP_MUTEX mutex)
{
    MMP_BOOL result;
    LOG_ENTER "PalDestroyMutex(mutex=0x%X)\r\n", mutex LOG_END

    PalAssert(mutex);

    result = CloseHandle(mutex);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
    }

    LOG_LEAVE "PalDestroyMutex()=%d\r\n", !result LOG_END
    return !result;
}

MMP_INT
PalWaitMutex(
    MMP_MUTEX mutex,
    MMP_ULONG timeout)
{
    DWORD result;

    LOG_ENTER "PalWaitMutex(mutex=0x%X,timeout=%d)\r\n", mutex, timeout LOG_END

    PalAssert(mutex);

    result = WaitForSingleObject(mutex, timeout);
    if (result == WAIT_TIMEOUT)
    {
        LOG_WARNING "WAIT MUTEX TIMEOUT\r\n" LOG_END
    }
    else if (result == WAIT_FAILED)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"WaitForSingleObject FAIL");
    }
    else
    {
        result = 0;
    }

    LOG_LEAVE "PalWaitMutex()=%d\r\n", result LOG_END
    return (MMP_INT) result;
}

MMP_INT
PalReleaseMutex(
    MMP_MUTEX mutex)
{
    MMP_BOOL result;

    LOG_ENTER "PalReleaseMutex(mutex=0x%X)\r\n", mutex LOG_END

    PalAssert(mutex);

    result = ReleaseMutex(mutex);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"ReleaseMutex FAIL");
    }

    LOG_LEAVE "PalReleaseMutex()=%d\r\n", !result LOG_END
    return !result;
}
