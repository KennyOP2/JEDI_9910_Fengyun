/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for OpenRISC function
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __OR32_H__
#define __OR32_H__

//#include "debug.h"
#include "spr_defs.h"
#include "proj_defs.h"
#include "or32_host.h"

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//                      Global Configuration
/////////////////////////////////////////////////////////////////
//#define PowerSaving 0
#define PowerSaving 1

#if defined(__GDB_DEBUG__)
#undef  PowerSaving
#define PowerSaving 0
#endif

#if !defined(__CRT0__)
/////////////////////////////////////////////////////////////////
//                      Function Decleration
/////////////////////////////////////////////////////////////////
int  or32_getCpuID(void);                  // get CPU ID
int  or32_getSysCLK(void);                 // get system clock in Hz
void or32_setSysCLK(int clk);              // set system clock in Hz
int  or32_getBusCLK(void);                 // get bus clock in Hz
int  or32_getMemCLK(void);                 // get memory clock in Hz
void or32_sleep(unsigned int ticks);       // Delay in 1 ticks
void or32_delay_us(unsigned short int us); // Delay in 1 us
void or32_delay_ms(unsigned short int ms); // Delay in 1 ms
void or32_calcTicks(void);                 // Caculate the ticks for one millisecond.
int  or32_getTicks_ms(void);               // Return the number of ticks in one millisecond. 10^-3 second.
int  or32_getTicks_us(void);               // Return the number of ticks in one microsecond. 10^-6 second.

#define or32_delay(ms) or32_delay_ms(ms)

/////////////////////////////////////////////////////////////////
//                      Function Definiton
/////////////////////////////////////////////////////////////////
static __inline void or32_doze(void)
{
    asm volatile("l.mtspr\t\t%0,%1,0": : "r" (SPR_PMR), "r" (SPR_PMR_DME));
    asm volatile("l.nop\t\t0x3388"); // One delay slot to enter doze mode
    asm volatile("l.nop\t\t0x3388"); // Another delay slot to avoid the
                                     // delay interrupt
}

static __inline void or32_halt(void)
{
    asm volatile("l.mtspr\t\t%0,%1,0": : "r" (SPR_PMR), "r" (SPR_PMR_SME));
    asm volatile("l.nop\t\t0x3388"); // One delay slot to enter doze mode
    while(1); // never reach here
}

/* Invalidate the cache line corresponds to the memory address [start,
   start + bytes)
*/
static __inline void or32_invalidate_cache(void *ptr, int bytes)
{
    unsigned int line_size;
    unsigned int cache_size;
    unsigned int start;
    unsigned int end;
    int ncs, bs;

    // Number of cache sets
    ncs = ((mfspr(SPR_DCCFGR) >> 3) & 0xf);

    // Number of cache block size
    bs  = ((mfspr(SPR_DCCFGR) >> 7) & 0x1) + 4;

    cache_size = (1 << (ncs+bs));
    line_size  = (1 << bs);

    if (bytes < 0)
        bytes = cache_size;

    start = ((unsigned int)ptr) & ~(line_size-1);
    end   = ((unsigned int)ptr) + bytes - 1;
    end   = ((unsigned int)end) & ~(line_size-1);
    if (end > start + cache_size - line_size) {
        end = start + cache_size - line_size;
    }

    do {
        mtspr(SPR_DCBIR, start);
        start += line_size;
    } while (start <= end);
}
#endif // defined(__CRT0__)

#define ENABLE_SR_INTERRUPT_EXCEPTION()     mtspr(SPR_SR,    mfspr(SPR_SR)    |   SPR_SR_IEE)
#define DISABLE_SR_INTERRUPT_EXCEPTION()    mtspr(SPR_SR,    mfspr(SPR_SR)    & ~(SPR_SR_IEE))
#define ENABLE_PICMR_HOST_INTERRUPT()       mtspr(SPR_PICMR, mfspr(SPR_PICMR) |   SPR_PICMR_HOST)
#define DISABLE_PICMR_HOST_INTERRUPT()      mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(SPR_PICMR_HOST))
#define ENABLE_PICMR_TT_INTERRUPT()         mtspr(SPR_PICMR, mfspr(SPR_PICMR) |   SPR_PICMR_TT)
#define DISABLE_PICMR_TT_INTERRUPT()        mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(SPR_PICMR_TT))
#define CLEAR_PICSR_HOST_INTERRUPT()        mtspr(SPR_PICSR, mfspr(SPR_PICSR) & ~(SPR_PICSR_HOST))
#define CLEAR_PICSR_TT_INTERRUPT()          mtspr(SPR_PICSR, mfspr(SPR_PICSR) & ~(SPR_PICSR_TT))
#define GET_PICSR()                         mfspr(SPR_PICSR)


#ifdef __cplusplus
}
#endif

#endif // __OR32_H__

