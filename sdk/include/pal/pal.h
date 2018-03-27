/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * The platform adaptation layer functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_PAL_H
#define PAL_PAL_H

#include "pal/def.h"
#include "pal/error.h"
#include "pal/event.h"
#include "pal/file.h"
#include "pal/heap.h"
#include "pal/keypad.h"
#include "pal/msgq.h"
#include "pal/mutex.h"
#include "pal/print.h"
#include "pal/rand.h"
#include "pal/stat.h"
#include "pal/string.h"
#include "pal/thread.h"
#include "pal/timer.h"
#include "pal/sys.h"

#define SMTK_MAX(a,b)   (((a) > (b)) ? (a) : (b))
#define SMTK_MIN(a,b)   (((a) < (b)) ? (a) : (b))
#define SMTK_ABS(x)     (((x) >= 0)  ? (x) : -(x))

#define SMTK_COUNT_OF(array) (sizeof (array) / sizeof (array[0]))

#endif /* PAL_PAL_H */
