#include "av_sync.h"
#include "mmp_aud.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct AV_SYNC_STATUS_TAG
{
    MMP_BOOL   bVideoStable;
    MMP_BOOL   bAudioInitFinished;
    MMP_UINT32 timeStamp;
    MMP_UINT32 audioSampleRate;
    MMP_BOOL   bDeviceInitFinished;
    MMP_UINT32 videoMuxTime;
} AV_SYNC_STATUS;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

static AV_SYNC_STATUS gtAvSyncStatus = { 0 };

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

void
avSyncSetVideoStable(
    MMP_BOOL bStable)
{
    gtAvSyncStatus.bVideoStable = bStable;
}

void
avSyncSetAudioInitFinished(
    MMP_BOOL bStable)
{
    gtAvSyncStatus.bAudioInitFinished = bStable;
}

void
avSyncSetCurrentTime(
    MMP_UINT32 currentTime)
{
    gtAvSyncStatus.timeStamp = currentTime;
    mmpAudioSetAttrib(MMP_AUDIO_ENCODE_START_TIME, (void *)&currentTime);
}

MMP_UINT32
avSyncGetCurrentTime(
    void)
{
    return gtAvSyncStatus.timeStamp;
}

MMP_BOOL
avSyncIsVideoStable(
    void)
{
    return gtAvSyncStatus.bVideoStable;
}

MMP_BOOL
avSyncIsAudioInitFinished(
    void)
{
    return gtAvSyncStatus.bAudioInitFinished;
}

void
avSyncSetAudioSampleRate(
    MMP_UINT32 sampleRate)
{
    gtAvSyncStatus.audioSampleRate = sampleRate;
}

MMP_UINT32
avSyncGetAudioSampleRate(
    void)
{
    return gtAvSyncStatus.audioSampleRate;
}

void
avSyncSetDeviceInitFinished(
    MMP_BOOL bDone)
{
    gtAvSyncStatus.bDeviceInitFinished = bDone;
}

MMP_BOOL
avSyncGetDeviceInitFinished(
    void)
{
    return gtAvSyncStatus.bDeviceInitFinished;
}

void
avSyncSetCurrentVideoMuxTime(
    MMP_UINT32 videoMuxTime)
{
    gtAvSyncStatus.videoMuxTime = videoMuxTime;
}

MMP_UINT32
avSyncGetCurrentVideoMuxTime(
    void)
{
    return gtAvSyncStatus.videoMuxTime;
}