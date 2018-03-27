/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * The rand functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_RAND_H
#define PAL_RAND_H

#include "pal/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Random definition */
#define PAL_RAND_MAX    0x7FFF

MMP_INT
PalRand(
    void);

void
PalSrand(
    MMP_UINT seed);

#ifdef __cplusplus
}
#endif

#endif /* PAL_RAND_H */

