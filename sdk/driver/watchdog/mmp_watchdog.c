/*
 * (c) SMedia Tech. Corp. 2008
 *
 *  Driver for WATCHDOG
 *
 */
#include "mmp_watchdog.h"
#include "host/ahb.h"

//=============================================================================
//                              LOG definition
//=============================================================================
#define LOG_ZONES   (MMP_ZONE_ERROR | MMP_ZONE_WARNING) // | (MMP_ZONE_ENTER | MMP_ZONE_LEAVE | MMP_ZONE_INFO)

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR   & (LOG_ZONES)) ? (printf("[SMEDIA][WATCHDOG][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & (LOG_ZONES)) ? (printf("[SMEDIA][WATCHDOG][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO    & (LOG_ZONES)) ? (printf("[SMEDIA][WATCHDOG][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG   & (LOG_ZONES)) ? (printf("[SMEDIA][WATCHDOG][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER   & (LOG_ZONES)) ? (printf("[SMEDIA][WATCHDOG][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE   & (LOG_ZONES)) ? (printf("[SMEDIA][WATCHDOG][LEAVE]"
#define LOG_DATA    ((void) ((MMP_FALSE) ? (printf(
#define LOG_INFO2   ((void) ((MMP_TRUE) ? (printf(
#define LOG_END     )), 1 : 0));

//=============================================================================
//                             WATCHDOG Mode Functions
//=============================================================================
/**
 * Enable WATCHDOG module.
 */
WATCHDOG_API void
mmpWatchDogEnable(
    MMP_UINT watchdogtimeout)
{
    MMP_UINT32 wdCounter, clk;

    LOG_ENTER "[mmpWatchDogEnable] Enter\n" LOG_END

    // Disable watch dog
    AHB_WriteRegister(WATCH_DOG_BASE+WDG_CTRL_REG, 0);

    #ifdef _WIN32
        clk = 41943040;
    #else
        #if defined(__OPENRTOS__)
			clk = ithGetBusClock();
        #elif defined(__FREERTOS__)
			clk = or32_getBusCLK();
        #endif
    #endif

    // Check the timeout counter is overflow or not.
    if (watchdogtimeout == 0)           // Reset immediately
    {
        wdCounter = 4096;               // set 4096 cycles, minimun APB clock / 32768 = 66MHz/32768 = 2014
    }
    else if (watchdogtimeout > (MAX_WDG_COUNTER/clk)) // Overflow
    {
        wdCounter = MAX_WDG_COUNTER;    // set maximun cycles
        LOG_WARNING "The watchdog timer setting is out of range.\n" LOG_END
    }
    else
    {
        wdCounter = clk * watchdogtimeout;
    }

    // Set watch dog time out timer
    AHB_WriteRegister(WATCH_DOG_BASE+WDG_LOAD_REG, wdCounter);

    // Reload the watch dog timer
    AHB_WriteRegister(WATCH_DOG_BASE+WDG_REST_REG, WDG_REFRESH_ID);

    // Enable watch dog
    AHB_WriteRegister(WATCH_DOG_BASE+WDG_CTRL_REG, 0x00000003);
    // Open this if using interrupt
    //AHB_WriteRegister(WATCH_DOG_BASE+WDG_CTRL_REG, 0x00000005);

    LOG_LEAVE "[mmpWatchDogEnable] Leave\n" LOG_END
}

/**
 * Disable WATCHDOG modul
 */
WATCHDOG_API void
mmpWatchDogDisable(
    void)
{
    LOG_ENTER "[mmpWatchDogDisable] Enter\n" LOG_END

    // Disable watch dog
    AHB_WriteRegister(WATCH_DOG_BASE+WDG_CTRL_REG, 0x00000000);

    LOG_LEAVE "[mmpWatchDogDisable] Leave\n" LOG_END
}

/**
 * Refresh WATCHDOG timer
 */
WATCHDOG_API void
mmpWatchDogRefreshTimer(
    void)
{
    LOG_ENTER "[mmpWatchDogRefreshTimer] Enter\n" LOG_END

    // refresh watch dog timer
    AHB_WriteRegister(WATCH_DOG_BASE+WDG_REST_REG, WDG_REFRESH_ID);

    LOG_LEAVE "[mmpWatchDogRefreshTimer] Leave\n" LOG_END
}
