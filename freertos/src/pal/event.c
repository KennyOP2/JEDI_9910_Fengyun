#include "config.h"
#include "pal/pal.h"			
#include "sys/sys.h"

MMP_EVENT
PalCreateEvent(
    MMP_INT name)
{
    void* event;
    LOG_ENTER "PalCreateEvent(name=%d)\r\n", name LOG_END


    event = SYS_CreateEvent();
    PalAssert(event);

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

    result = SYS_DelEvent(event);
    PalAssert(result == MMP_TRUE);

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

    result = SYS_SetEvent(event);
    PalAssert(result == MMP_TRUE);

    LOG_LEAVE "PalSetEvent()=%d\r\n", !result LOG_END
    return !result;
}

MMP_INT
PalWaitEvent(
    MMP_EVENT event,
    MMP_ULONG timeout)
{
    MMP_INT result = 0;
    LOG_ENTER "PalWaitEvent(event=0x%X,timeout=%d)\r\n", event, timeout LOG_END

    PalAssert(event);

    SYS_WaitForEventForever(event);

    LOG_LEAVE "PalWaitEvent()=%d\r\n", result LOG_END
    return result;
}
