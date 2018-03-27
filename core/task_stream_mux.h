#ifndef TASK_STREAM_MUX_H
#define TASK_STREAM_MUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "queue_mgr.h"

void
taskStreamMux_Init(
    MMP_UINT32 argc);

void
taskStreamMux_Terminate(
    MMP_UINT32 argc);

QUEUE_MGR_ERROR_CODE
taskStreamMux_SetFree(
    QUEUE_ID queueId,
    void** pptSample);

void
taskStreamMux_GetBufferInfo(
    MMP_UINT8** pBufferStart,
    MMP_UINT32* bufferSize);
    
MMP_UINT32
taskStreamMux_GetWriteIndex(
    void);

void 
taskStreamMux_UndoSeal(
    MMP_BOOL enable);

#ifdef __cplusplus
}
#endif

#endif
