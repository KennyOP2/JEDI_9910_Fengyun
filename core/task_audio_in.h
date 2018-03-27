#ifndef TASK_AUDIO_IN_H
#define TASK_AUDIO_IN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "queue_mgr.h"

void
taskAudioIn_Init(
    MMP_UINT32 argc);

void
taskAudioIn_Terminate(
    MMP_UINT32 argc);

QUEUE_MGR_ERROR_CODE
taskAudioIn_SetFree(
    QUEUE_ID queueId,
    void** pptSample);

#ifdef __cplusplus
}
#endif

#endif
