#include "config.h"
#include "pal/pal.h"
#include "sys/sys.h"

MMP_MUTEX
PalCreateMutex(
    MMP_INT name)
{
    MMP_MUTEX mutex;
    LOG_ENTER "PalCreateMutex(name=%d)\r\n", name LOG_END

    mutex = SYS_CreateSemaphore(1, MMP_NULL);
    PalAssert(mutex);

    LOG_LEAVE "PalCreateMutex()=0x%X\r\n", mutex LOG_END
    return mutex;
}

MMP_INT
PalDestroyMutex(
    MMP_MUTEX mutex)
{
    MMP_INT result = 0;
    LOG_ENTER "PalDestroyMutex(mutex=0x%X)\r\n", mutex LOG_END

    PalAssert(mutex);

    SYS_DeleteSemaphore(mutex);

    LOG_LEAVE "PalDestroyMutex()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalWaitMutex(
    MMP_MUTEX mutex,
    MMP_ULONG timeout)
{
    MMP_INT result = 0;
    LOG_ENTER "PalWaitMutex(mutex=0x%X,timeout=%d)\r\n", mutex, timeout LOG_END

    PalAssert(mutex);

    SYS_WaitSemaphore(mutex);
    
    LOG_LEAVE "PalWaitMutex()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalReleaseMutex(
    MMP_MUTEX mutex)
{
    MMP_INT result = 0;
    LOG_ENTER "PalReleaseMutex(mutex=0x%X)\r\n", mutex LOG_END

    PalAssert(mutex);

    SYS_ReleaseSemaphore(mutex);

    LOG_LEAVE "PalReleaseMutex()=%d\r\n", result LOG_END
    return result;
}
