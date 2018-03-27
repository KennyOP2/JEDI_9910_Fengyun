#ifndef MPS_CMDQ_H
#define MPS_CMDQ_H

#include "mmp_types.h"
#include "pal/pal.h"
#include "queue_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

// The bit31 of the command is whether the command is synchronous or asynchronous.
#define MPS_COMMAND_SYNC_BIT_MASK       (0x80000000)
#define MPS_COMMAND_MASK                (0x7FFFFFFF)

typedef enum MPS_COMMAND_TAG
{
    MPS_COMMAND_NULL = 0,
    MPS_COMMAND_INIT,
    MPS_COMMAND_TERMINATE,
    MPS_COMMAND_CLOSE,
    MPS_COMMAND_PLAY,
    MPS_COMMAND_STOP,
    MPS_COMMAND_START_RECORD,
    MPS_COMMAND_STOP_RECORD,
    MPS_COMMAND_START_JPEG_RECORD,
    MPS_COMMAND_STOP_JPEG_RECORD,
    MPS_COMMAND_SET_PROPERTY,
    MPS_COMMAND_GET_PROPERTY,
    //MPS_COMMAND_SET_ENCODE_PARAMETER,
    //MPS_COMMAND_SET_CAPTURE_DEVICE,
    //MPS_COMMAND_SET_ISP_MODE,
    //MPS_COMMAND_SET_ENABLE_AV_ENGINE,
    MPS_COMMAND_EVENT_NOTIFY,
    MPS_COMMAND_TOTAL
} MPS_COMMAND;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MPS_CMD_OBJ_TAG
{
    MPS_COMMAND     cmd;
    void*           extraData;
} MPS_CMD_OBJ;

//=============================================================================
//                              Function  Definition
//=============================================================================
void
mpsCmdQ_Init(
    QUEUE_ID queueId);

void
mpsCmdQ_SendCommand(
    QUEUE_ID        queueId,
    MPS_CMD_OBJ*    ptInputCmd);

void
mpsCmdQ_ReceiveCommand(
    QUEUE_ID        queueId,
    MPS_CMD_OBJ*    ptOutputCmd);

void
mpsCmdQ_Terminate(
    QUEUE_ID queueId);

#ifdef __cplusplus
}
#endif

#endif
