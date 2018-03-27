#include "config.h"
#include "pal/pal.h"
#include <windows.h>
#include <mmsystem.h>

PAL_CLOCK_T
PalGetClock(
    void)
{
    return timeGetTime();
}

MMP_ULONG
PalGetDuration(
    PAL_CLOCK_T clock)
{
    PAL_CLOCK_T currClock = timeGetTime();
    MMP_ULONG duration;

    if (currClock >= clock)
    {
        duration = currClock - clock;
    }
    else
    {
        // Overflow
        duration = ((PAL_CLOCK_T) -1) - clock + currClock;
    }

    return duration * 1000 / PAL_CLOCKS_PER_SEC;
}
