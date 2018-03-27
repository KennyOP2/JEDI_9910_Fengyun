#include "config.h"
#include "pal/pal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timer.h"

/**
 * Get milliseconds
 *
 * @param               No.
 * @return              milliseconds
 */
PAL_CLOCK_T
PalGetClock(
    void)
{
    return xTaskGetTimerCount();
}

/**
 * Return the duration with milliseconds.
 * It about 49 days on 4G milliseconds for maximun value.
 *
 * @param clock         last time
 * @return              milliseconds
 */
MMP_ULONG
PalGetDuration(
    PAL_CLOCK_T clock)
{
    return (xTaskGetTimerCount() - clock);
}

/**
 * Get microseconds
 *
 * @param               No.
 * @return              microseconds
 */
PAL_UCLOCK_T
PalGetUClock(
    void)
{
    static struct _pal_uclock t; // Only for non-preemptive OS
    xTaskGetTime(&t.sec, &t.ms, &t.us);
    return &t;
}

/**
 * Return the duration with microseconds.
 * It about 71 minutes on 4G microseconds for maximun value.
 * The overhead to call this function about 10 ~ 60 us.
 *
 * @param clock         last time
 * @return              microseconds
 */
MMP_ULONG
PalGetUDuration(
    PAL_UCLOCK_T clock)
{
    struct _pal_uclock t;

    xTaskGetTime(&t.sec, &t.ms, &t.us);

    t.sec -= clock->sec;

    if (t.ms < clock->ms)
    {
        t.sec--;
        t.ms = 1000 + t.ms - clock->ms;
    }
    else
    {
        t.ms -= clock->ms;
    }

    if (t.us < clock->us)
    {
        t.ms--;
        t.us = 1000 + t.us - clock->us;
    }
    else
    {
        t.us -= clock->us;
    }

    return (t.sec * 1000 * 1000 + t.ms * 1000 + t.us);
}

#ifdef ENABLE_DEBUG_MSG_OUT
void
dbg_measureTime(
    MMP_INT     id,
    MMP_CHAR    *prefix)
{
    static MMP_UINT32   st[10] = {0};

    if( id >= 10 ) 
    {
        printf("Wrong id !!!!!! %s [%d]\n", __FILE__, __LINE__);
        return;
    }
    
    if( prefix )
    {
        printf("%s %d ms\n", prefix, PalGetDuration(st[id]));
    }
    else
    {
        st[id] = PalGetClock();
    }
}
#endif

