#ifndef TASK_AVI_MUX_H
#define TASK_AVI_MUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "queue_mgr.h"

void
taskAviMux_Init(
    MMP_UINT32 argc);

void
taskAviMux_Terminate(
    MMP_UINT32 argc);

QUEUE_MGR_ERROR_CODE
taskAviMux_SetFree(
    QUEUE_ID queueId,
    void** pptSample);

#ifdef __cplusplus
}
#endif

#endif
