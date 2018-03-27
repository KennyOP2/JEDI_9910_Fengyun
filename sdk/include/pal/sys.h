/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * The system dependent functions.
 *
 * @author Kuoping Hsu
 * @version 1.0
 */
#ifndef PAL_SYS_H
#define PAL_SYS_H

#include "pal/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Setting watch point.
 *
 * @param top           top address.
 * @param buttom        buttom address.
 * @return MMP_TRUE if succed, MMP_FALSE if fail.
 */
MMP_BOOL
PalSetWatchPoint(
    void   *top,
    void   *buttom);

/**
 * Flush the data cache by address range.
 *
 * @param ptr           the starting address to clear.
 * @param len           the length to clean.
 * @return              No.
 */
void
PalInvalidateCache(
    void   *ptr,
    MMP_INT len);

/**
 * Flush all of data cache entry.
 *
 * @return              No.
 */
void
PalFlushCache(
    void);

/**
 * Get the bus clock speed.
 *
 * @return              the clock speed in Hz.
 */
MMP_INT
PalGetBusClock(
    void);

/**
 * Get the system clock speed.
 *
 * @return              the clock speed in Hz.
 */
MMP_INT
PalGetSysClock(
    void);

/**
 * Get the memory clock speed.
 *
 * @return              the clock speed in Hz.
 */
MMP_INT
PalGetMemClock(
    void);

/**
 * CPU enter doze mode.
 *
 * @return              none.
 */
void
PalDoze(
    void);

/**
 * Disable Interrupt
 *
 * @return              none.
 */
void
PalDisableInterrupts(
    void);

/**
 * Enable Interrupt
 *
 * @return              none.
 */
void
PalEnableInterrupts(
    void);

#ifdef __cplusplus
}
#endif

#endif /* PAL_SYS_H */
