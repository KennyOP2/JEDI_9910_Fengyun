/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file av_sync.h
 * @author
 * @version 0.1
 */

#ifndef AV_SYNC_H
#define AV_SYNC_H

#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function  Definition
//=============================================================================

void
avSyncSetVideoStable(
    MMP_BOOL bStable);

void
avSyncSetAudioInitFinished(
    MMP_BOOL bStable);
    
void
avSyncSetCurrentTime(
    MMP_UINT32 currentTime);

MMP_UINT32
avSyncGetCurrentTime(
    void);

MMP_BOOL
avSyncIsVideoStable(
    void);

MMP_BOOL
avSyncIsAudioInitFinished(
    void);

void
avSyncSetAudioSampleRate(
    MMP_UINT32 sampleRate);
        
MMP_UINT32
avSyncGetAudioSampleRate(
    void);

void
avSyncSetDeviceInitFinished(
    MMP_BOOL bDone);

MMP_BOOL
avSyncGetDeviceInitFinished(
    void);

void
avSyncSetCurrentVideoMuxTime(
    MMP_UINT32 videoMuxTime);

MMP_UINT32
avSyncGetCurrentVideoMuxTime(
    void);

#ifdef __cplusplus
}
#endif

#endif
