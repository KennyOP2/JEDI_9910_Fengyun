/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * The thread functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_THREAD_H
#define PAL_THREAD_H

#include "pal/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Thread definition */
typedef void* PAL_THREAD;

/** Thread name */
//#define PAL_THREAD_MAIN             0
//#define PAL_THREAD_MPS              1
//#define PAL_THREAD_VIDEO_CAPTURE    2
//#define PAL_THREAD_AUDIO_IN         3
//#define PAL_THREAD_VIDEO_ENCODER    4
//#define PAL_THREAD_STREAM_MUX       5
//#define PAL_THREAD_STORAGE          6
//#define PAL_THREAD_PALFILE          7
//#define PAL_THREAD_USBEX            8
//#ifdef ENABLE_USB_DEVICE
//#define PAL_THREAD_USB_DEVICE       9
//#endif
//#define PAL_THREAD_HDMILooPThrough  10
//#define PAL_THREAD_KEY_MANAGER      11

typedef enum PAL_THREAD_N_TAG
{
    PAL_THREAD_MAIN = 0,
    PAL_THREAD_MPS,
    PAL_THREAD_AUDIO_IN,
    PAL_THREAD_VIDEO_ENCODER,
    PAL_THREAD_STREAM_MUX,
    PAL_THREAD_STORAGE,
    PAL_THREAD_PALFILE,
    PAL_THREAD_USBEX,
#ifdef ENABLE_USB_DEVICE
    PAL_THREAD_USB_DEVICE,
#endif
    PAL_THREAD_HDMILooPThrough,
    PAL_THREAD_KEY_MANAGER,
    PAL_THREAD_COUNT
} PAL_THREAD_N;

/** Thread priority */
#define PAL_THREAD_PRIORITY_HIGHEST 60
#define PAL_THREAD_PRIORITY_HIGHER  80
#define PAL_THREAD_PRIORITY_NORMAL  100
#define PAL_THREAD_PRIORITY_LOWER   120
#define PAL_THREAD_PRIORITY_LOWEST  140

void
PalSleep(
    MMP_ULONG ms);

void
PalUSleep(
    MMP_ULONG us);

typedef void*
(*PAL_THREAD_PROC)(
    void* arg);

PAL_THREAD
PalCreateThread(
    MMP_INT name,
    PAL_THREAD_PROC proc,
    void* arg,
    MMP_ULONG stackSize,
    MMP_UINT priority);

MMP_INT
PalDestroyThread(
    PAL_THREAD thread);

#ifdef __cplusplus
}
#endif

#endif /* PAL_THREAD_H */
