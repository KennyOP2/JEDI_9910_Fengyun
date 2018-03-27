/*
 * RISC porting by SMedia Tech. Corp. 2008
 */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spr_defs.h"
#include "proj_defs.h"
#include "mmio.h"
#include "timer.h"

#define USE_CPU_CLK_AS_TICKS
#define MAX_U32 0xFFFFFFFFu

static unsigned g_timer_period      = 0;
static unsigned g_timer_init        = 0;

/* handle system time resolution */
static unsigned g_max32_to_sec      = 0;
static unsigned g_max32_to_ms       = 0;
static unsigned g_max32_to_us       = 0;
static unsigned g_max32_remain_tick = 0;
static unsigned g_ticks_per_sec     = 0;
static unsigned g_ticks_per_ms      = 0;
static unsigned g_ticks_per_us      = 0;
static unsigned g_sys_overflow_sec  = 0;
static unsigned g_sys_overflow_ms   = 0;
static unsigned g_sys_overflow_us   = 0;
static unsigned g_sys_overflow_tick = 0;
static unsigned g_tickCount         = 0;
static unsigned g_sys_time          = 0;

static int timer_get_sys_clk(void);

#define CHECK_TICK_OVERFLOW()                                   \
    if (tickCount < g_tickCount) {                              \
        g_sys_overflow_sec  += g_max32_to_sec;                  \
        g_sys_overflow_ms   += g_max32_to_ms;                   \
        g_sys_overflow_us   += g_max32_to_us;                   \
        g_sys_overflow_tick += g_max32_remain_tick;             \
                                                                \
        if (g_sys_overflow_tick >= g_ticks_per_us) {            \
            g_sys_overflow_us++;                                \
            g_sys_overflow_tick -= g_ticks_per_us;              \
        }                                                       \
                                                                \
        if (g_sys_overflow_us >= 1000) {                        \
            g_sys_overflow_ms += (g_sys_overflow_us / 1000);    \
            g_sys_overflow_us %= 1000;                          \
        }                                                       \
                                                                \
        if (g_sys_overflow_ms >= 1000) {                        \
            g_sys_overflow_sec += (g_sys_overflow_ms / 1000);   \
            g_sys_overflow_ms  %= 1000;                         \
        }                                                       \
                                                                \
        if (g_sys_overflow_tick >= g_ticks_per_us) {            \
            taskSOFTWARE_BREAKPOINT();                          \
        }                                                       \
    }                                                           \
    g_tickCount = tickCount;

#if 0
PROFILE_FUNC
void timer_disable_exception(void)
{
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
}

PROFILE_FUNC
void timer_enable_exception(void)
{
    mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);
}
#endif

PROFILE_FUNC
void timer_init(void)
{
    // timer_enable_exception();
    mtspr(SPR_TTMR, 0);
}

PROFILE_FUNC
int timer_start(unsigned period)
{
    // Check if initial is necessary
    if (period == g_timer_period && g_timer_init == 1) {
        return 1;
    }

    if (period > SPR_TTMR_PERIOD) {
        period = SPR_TTMR_PERIOD;
    }

    g_sys_overflow_sec  = 0;
    g_sys_overflow_ms   = 0;
    g_sys_overflow_us   = 0;
    g_sys_overflow_tick = 0;
    g_sys_time          = 0;
    g_tickCount         = 0;
    g_timer_init        = 1;
    g_timer_period      = period;

    // timer_disable_exception();
    mtspr(SPR_TTCR, 0);
    mtspr(SPR_TTMR, SPR_TTMR_CR | period); /* continus run */
    // timer_enable_exception();

    g_ticks_per_sec     = timer_get_sys_clk();
    g_ticks_per_ms      = (g_ticks_per_sec / 1000);
    g_ticks_per_us      = (g_ticks_per_sec / 1000000);
    g_max32_to_sec      = (MAX_U32 / g_ticks_per_sec);
    g_max32_to_ms       = (MAX_U32 % g_ticks_per_sec) / g_ticks_per_ms;
    g_max32_to_us       = (MAX_U32 % g_ticks_per_ms) / g_ticks_per_us;
    g_max32_remain_tick = (MAX_U32 % g_ticks_per_us);

    return 1;
}

PROFILE_FUNC
void timer_update(void)
{
    unsigned long tickCount = mfspr(SPR_TTCR);
    mtspr(SPR_TTCR, 0); /* restart */

    // update the accumulated seconds and ms.
    if (tickCount < g_tickCount) {
        g_sys_overflow_sec  += g_max32_to_sec;
        g_sys_overflow_ms   += g_max32_to_ms;
        g_sys_overflow_us   += g_max32_to_us;
        g_sys_overflow_tick += g_max32_remain_tick;
    }

    g_sys_overflow_tick += tickCount;
    g_sys_overflow_tick += (g_ticks_per_us / 2); // +0.5us for rounding
    if (g_sys_overflow_tick >= g_ticks_per_us) {
        g_sys_overflow_us += (g_sys_overflow_tick / g_ticks_per_us);
    }
    g_sys_overflow_tick = 0; // reset accumulated ticks

    if (g_sys_overflow_us >= 1000) {
        g_sys_overflow_ms += (g_sys_overflow_us / 1000);
        g_sys_overflow_us %= 1000;
    }

    if (g_sys_overflow_ms >= 1000) {
        g_sys_overflow_sec += (g_sys_overflow_ms / 1000);
        g_sys_overflow_ms  %= 1000;
    }

    // update g_ticks_per_sec and g_ticks_per_ms
    g_ticks_per_sec     = timer_get_sys_clk();
    g_ticks_per_ms      = (g_ticks_per_sec / 1000);
    g_ticks_per_us      = (g_ticks_per_sec / 1000000);
    g_max32_to_sec      = (MAX_U32 / g_ticks_per_sec);
    g_max32_to_ms       = (MAX_U32 % g_ticks_per_sec) / g_ticks_per_ms;
    g_max32_to_us       = (MAX_U32 % g_ticks_per_ms) / g_ticks_per_us;
    g_max32_remain_tick = (MAX_U32 % g_ticks_per_us);
    g_tickCount         = mfspr(SPR_TTCR);
}

PROFILE_FUNC
unsigned timer_get_ticks(void)
{
    unsigned tickCount;
    tickCount = (unsigned)mfspr(SPR_TTCR);

    CHECK_TICK_OVERFLOW();

    #if defined(USE_CPU_CLK_AS_TICKS)
    return tickCount;
    #else
    if (g_ticks_per_ms) {
        unsigned t;
        unsigned sec;
        unsigned ms;

        sec = (tickCount / g_ticks_per_sec);
        ms  = (tickCount % g_ticks_per_sec) / g_ticks_per_ms;
        t   = (g_sys_overflow_sec + sec) * 1000 + (g_sys_overflow_ms + ms);
        if (t < g_sys_time) { // Overflow check
            /* TODO */
            g_sys_overflow_sec += (MAX_U32 / 1000);
            g_sys_overflow_ms  += (MAX_U32 % 1000);
            g_sys_overflow_tick = 0;

            if (g_sys_overflow_ms >= 1000) {
                g_sys_overflow_sec += (g_sys_overflow_ms / 1000);
                g_sys_overflow_ms  %= 1000;
            }
        }
        g_sys_time = t;
        return t;
    } else {
        return 0;
    }
    #endif
}

PROFILE_FUNC
unsigned timer_get_milliseconds(void)
{
    unsigned tickCount;
    tickCount = (unsigned)mfspr(SPR_TTCR);

    CHECK_TICK_OVERFLOW();

    if (g_ticks_per_ms) {
        unsigned sec;
        unsigned ms;
        unsigned us;
        unsigned t;
        sec = (tickCount / g_ticks_per_sec);
        ms  = (tickCount % g_ticks_per_sec) / g_ticks_per_ms;
        t   = (g_sys_overflow_sec + sec) * 1000 + (g_sys_overflow_ms + ms);
        return t;
    } else {
        return 0;
    }
}

PROFILE_FUNC
unsigned timer_get_time(unsigned *sec, unsigned *ms, unsigned *us)
{
    unsigned tickCount;
    tickCount = (unsigned)mfspr(SPR_TTCR);

    CHECK_TICK_OVERFLOW();

    if (g_ticks_per_ms) {
        *sec = (tickCount / g_ticks_per_sec);
        *ms  = (tickCount % g_ticks_per_sec) / g_ticks_per_ms;
        *us  = (tickCount % g_ticks_per_ms) / g_ticks_per_us;
    } else {
        *sec = *ms = *us = 0;
    }
}

PROFILE_FUNC
void timer_except_handler(void)
{
    vTaskIncrementTick();

    /* clear the interrupt */
    mtspr(SPR_TTMR,  mfspr(SPR_TTMR) & ~SPR_TTMR_IP);
}

/*
 * Get System clock in Hz
 */
static int timer_get_sys_clk(void) {
    return or32_getSysCLK();
}

/*
    ms
    -------------------- = xTickCount required
    period * tick_per_ms

    The global variable of FreeRTOS xTickCount is incremented
    every 'period' ticks of tick timer.
*/
PROFILE_FUNC
unsigned timer_ms_to_xticks(unsigned ms)
{
    #if defined(USE_CPU_CLK_AS_TICKS)
    return (g_ticks_per_sec / 1000 * ms);
    #else
    return ms;
    #endif
}

PROFILE_FUNC
unsigned timer_us_to_xticks(unsigned us)
{
    #if defined(USE_CPU_CLK_AS_TICKS)
    return (g_ticks_per_sec / 1000000 * us);
    #else
    return ((us + 999) / 1000);
    #endif
}

PROFILE_FUNC
unsigned timer_xticks_to_ms(unsigned xticks)
{
    #if defined(USE_CPU_CLK_AS_TICKS)
    return xticks / (g_ticks_per_sec / 1000);
    #else
    return xticks;
    #endif
}

PROFILE_FUNC
unsigned timer_xticks_to_us(unsigned xticks)
{
    #if defined(USE_CPU_CLK_AS_TICKS)
    return xticks / (g_ticks_per_sec / 1000000);
    #else
    return xticks * 1000;
    #endif
}

PROFILE_FUNC
void timer_sleep(unsigned ticks)
{
    unsigned tick0, tick1;

    if (g_timer_init == 0) {
        timer_start(SPR_TTMR_PERIOD);
    }

    unsigned i = 0;

    tick0 = xTaskGetTickCount();
    for (i=0; i<ticks; ) {
        taskYIELD();
        tick1 = xTaskGetTickCount();
        if (tick0 < tick1) {
            i += (tick1 - tick0);
        }
        else {
            i += (0xffffffff - (tick0 - tick1));
        }
        tick0 = tick1;
    }
}

#if ( configGENERATE_RUN_TIME_STATS == 1 )
static unsigned int ulLastRunTimeStatsTimer;
static unsigned int ulRunTimeStatsTimer;
PROFILE_FUNC
void vConfigureTimerForRunTimeStats( void )
{
    mtspr(SPR_TTMR3, 0);
    mtspr(SPR_TTCR3, 0);
    mtspr(SPR_TTMR3, SPR_TTMR_CR);

    ulLastRunTimeStatsTimer = 0;
    ulRunTimeStatsTimer = 0;
}

PROFILE_FUNC
unsigned int vGetRunTimeCounterValue(void)
{
    int timer;
    int ticks = (g_ticks_per_sec / 1000000);
    unsigned int ulDiffTime;

    timer = mfspr(SPR_TTCR3);

    if (ulLastRunTimeStatsTimer == 0xffffffff)
        return 0xffffffff;

    if (timer > ulLastRunTimeStatsTimer) {
        ulDiffTime = ((timer - ulLastRunTimeStatsTimer) / ticks);
        if (ulRunTimeStatsTimer < 0xffffffff - ulDiffTime) { // Overflow protection
            ulRunTimeStatsTimer += ulDiffTime;
            ulDiffTime = (timer - ulLastRunTimeStatsTimer) - ulDiffTime * ticks;
        } else {
            ulRunTimeStatsTimer = 0xffffffff;
            ulDiffTime = 0;
            timer = 0xffffffff;
        }
    } else {
        ulDiffTime = ((0xffffffff - (ulLastRunTimeStatsTimer - timer) + 1) / ticks);
        if (ulRunTimeStatsTimer < 0xffffffff - ulDiffTime) { // Overflow protection
            ulRunTimeStatsTimer += ulDiffTime;
            ulDiffTime = (0xffffffff - (ulLastRunTimeStatsTimer - timer) + 1) - ulDiffTime * ticks;
        } else {
            ulRunTimeStatsTimer = 0xffffffff;
            ulDiffTime = 0;
            timer = 0xffffffff;
        }
    }

    ulLastRunTimeStatsTimer = timer - ulDiffTime;

    return ulRunTimeStatsTimer;
}
#endif
