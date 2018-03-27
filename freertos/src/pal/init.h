/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * The initialize functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef INIT_H
#define INIT_H

#include "pal/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Thread */
MMP_INT
PalThreadInitialize(
    void);

MMP_INT
PalThreadTerminate(
    void);

/* File */
MMP_INT
PalFileInitialize(
    void);

MMP_INT
PalFileTerminate(
    void);

#ifdef __cplusplus
}
#endif

#endif /* INIT_H */
