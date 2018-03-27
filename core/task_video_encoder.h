#ifndef TASK_VIDEO_ENCODER_H
#define TASK_VIDEO_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "queue_mgr.h"

void
taskVideoEncoder_Init(
    MMP_UINT32 argc);

void
taskVideoEncoder_Terminate(
    MMP_UINT32 argc);

QUEUE_MGR_ERROR_CODE
taskVideoEncoder_SetFree(
    QUEUE_ID queueId,
    void** pptSample);

#ifdef __cplusplus
}
#endif

#endif
