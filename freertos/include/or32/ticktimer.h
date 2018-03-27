/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for tick timer
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __TICKTIMER_H__
#define __TICKTIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "spr_defs.h"
static int bEnableAudioProcessor=1;
extern int tick_incr;
extern int tick_init;
extern int tick_mode;
static long long ticks = 0;

/////////////////////////////////////////////////////////////////
// Timer Functions
//
// The timer function is use to count the ticks in 2.68 seconds
// period maximun (count to 0x0fffffff in 100MHz). It dose not
// raise the tick timer exception to count the larger number
// of ticks.
//
// Usage:
//
//    int ticks;
//    start_timer();
//
//    ..... blah blah .....
//
//    ticks = get_timer();
//
/////////////////////////////////////////////////////////////////
static __inline int get_timer(void)
{
    int current_ticks = 0;
    if (!bEnableAudioProcessor)
    {
        current_ticks = mfspr(SPR_TTCR4);
        /* reset timer */
        mtspr(SPR_TTCR4, 0);
        return current_ticks;    
    }
    else
    {
        current_ticks = mfspr(SPR_TTCR);
        /* reset timer */
        mtspr(SPR_TTCR, 0);
        return current_ticks;    
    }
}

static __inline void reset_timer(void)
{
    if (!bEnableAudioProcessor)
    {
        mtspr(SPR_TTCR4, 0);    
    }
    else
    {
        mtspr(SPR_TTCR, 0);    
    }
}

static __inline void stop_timer(void) 
{
    if (!bEnableAudioProcessor)
    {    
        /* Stop Timer */
        mtspr(SPR_TTMR4, 0);    
    }
    else
    {
        /* Stop Timer */
        mtspr(SPR_TTMR, 0);
    }
}
static __inline void start_timer(void) 
{
    if (!bEnableAudioProcessor)
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR4, 0);
        /* Reset counter */
        mtspr(SPR_TTCR4, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR4, SPR_TTMR_SR | SPR_TTMR_PERIOD);    
    }
    else
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR, 0);
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_SR | SPR_TTMR_PERIOD);
    
    }
}

static __inline void set_audio_processor_timer(int nEnable) 
{
    if (nEnable)
    {
        bEnableAudioProcessor =1;
    }
    else
    {
        bEnableAudioProcessor =0;
    }        
}


#if 0
/////////////////////////////////////////////////////////////////
// Ticks Functions
//
// It's use the tick timer exception to count the 64bits ticks.
//
// Functions: void init_tick(int long_period);
//
//            long_perido       0: Do not use the tick timer exception,
//                                 the period between two get_ticks()
//                                 must less than the 2 seconds.
//                              1: Use the tick timer exception to count
//                                 the long period between two get_ticks()
//                                 functions.
//
// Usage:
//
//    long long old_ticks, new_tikcs;
//
//    init_ticks(1);
//    old_tikcs = get_ticks();
//
//    ..... blah blah .....
//
//    new_ticks = get_ticks();  // the totaly ticks is new_ticks-old_ticks
//
//    ..... blah blah .....
//
//    new_ticks = get_ticks();
//    stop_ticks();
//
/////////////////////////////////////////////////////////////////
static __inline void init_ticks (int long_period) {
    ticks     = 0;
    tick_init = 1;
    tick_mode = long_period;

    if (long_period) {
        /* Enable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);
    
        /* stop timer */
        mtspr(SPR_TTMR, 0);
    
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
    
        /* single run mode and enable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_SR | SPR_TTMR_IE | SPR_TTMR_PERIOD);
    } else {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
    
        /* stop timer */
        mtspr(SPR_TTMR, 0);
    
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
    
        /* Continuous run mode and disable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_CR | SPR_TTMR_PERIOD);
    }
}

static __inline long long get_ticks (void) {
    int current_ticks;
    static int last_ticks = 0;

    if (!tick_init) init_ticks(1);
    current_ticks = mfspr(SPR_TTCR);

    if (tick_mode == 1) {
        // Use tick timer exception to count the ticks. It should restart
        // the timer manually.
        ticks += ((long long)current_ticks + ((long long)tick_incr << 28));

        /* reset timer */
        mtspr(SPR_TTCR, 0);
    } else {
        // Use the countinuous timer to count the ticks.
        if (current_ticks < last_ticks) { // Timer is wrapped.
            tick_incr++;
        }
        last_ticks = current_ticks;
        ticks = ((long long)tick_incr << 32) + current_ticks;
    }

    return ticks;
}

static __inline void stop_ticks(void) {
    /* Disable tick timer exception recognition */
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

    /* Stop Timer */
    mtspr(SPR_TTMR, 0);
}
#endif

#ifdef __cplusplus
}
#endif

#endif // __TICKTIMER_H__
