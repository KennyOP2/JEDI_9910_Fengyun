#include "config.h"
#include "pal/pal.h"
#include <windows.h>
#include <stdio.h>

MMP_EVENT
PalCreateEvent(
    MMP_INT name)
{
    HANDLE event;
    CHAR buf[16];
    LOG_ENTER "PalCreateEvent(name=%d)\r\n", name LOG_END

    sprintf(buf, "PALEVENT%d", name);

    event = CreateEvent(NULL, FALSE, FALSE, buf);
    if (event == NULL)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CreateEvent FAIL");
    }

    LOG_LEAVE "PalCreateEvent()=0x%X\r\n", event LOG_END
    return event;
}

MMP_INT
PalDestroyEvent(
    MMP_EVENT event)
{
    MMP_BOOL result;
    LOG_ENTER "PalDestroyEvent(event=0x%X)\r\n", event LOG_END

    PalAssert(event);

    result = CloseHandle(event);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"CloseHandle FAIL");
    }

    LOG_LEAVE "PalDestroyEvent()=%d\r\n", !result LOG_END
    return !result;
}

MMP_INT
PalSetEvent(
    MMP_EVENT event)
{
    MMP_BOOL result;

    LOG_ENTER "PalSetEvent(event=0x%X)\r\n", event LOG_END

    PalAssert(event);

    result = SetEvent(event);
    if (result == FALSE)
    {
        LOG_ERROR "%s\r\n", PalGetErrorString(GetLastError()) LOG_END
        PalAssert(!"SetEvent FAIL");
    }

    LOG_LEAVE "PalSetEvent()=%d\r\n", !result LOG_END
    return !result;
}

MMP_INT
PalWaitEvent(
    MMP_EVENT event,
    MMP_ULONG timeout)
{
    DWORD result;

    LOG_ENTER "PalWaitEvent(event=0x%X,timeout=%d)\r\n", event, timeout LOG_END

    PalAssert(event);

    result = WaitForSingleObject(event, timeout);
    if (result == WAIT_TIMEOUT)
    {
       	LOG_WARNING "WAIT EVENT TIMEOUT\r\n" LOG_END
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

    LOG_LEAVE "PalWaitEvent()=%d\r\n", result LOG_END
    return (MMP_INT) result;
}
