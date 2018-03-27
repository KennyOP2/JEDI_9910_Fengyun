/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file mps_cmdq.c
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#include "pal/pal.h"
#include "mps_cmdq.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MAX_QUEUE_SIZE  (32)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

#ifdef WIN32
static MMP_MUTEX gtCmdMutex = MMP_NULL;
#endif

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
//                              Private Function Definition
//=============================================================================

void
mpsCmdQ_Init(
    QUEUE_ID queueId)
{
    queueMgr_CreateQueue(queueId, sizeof(MPS_CMD_OBJ), MAX_QUEUE_SIZE);
#ifdef WIN32
    gtCmdMutex = PalCreateMutex(MMP_NULL);
#endif
}

void
mpsCmdQ_SendCommand(
    QUEUE_ID        queueId,
    MPS_CMD_OBJ*    ptInputCmd)
{
    MPS_CMD_OBJ* ptCmd;
    QUEUE_CTRL_HANDLE* hQueue = queueMgr_GetCtrlHandle(queueId);

#ifdef WIN32
    PalWaitMutex(gtCmdMutex, PAL_MUTEX_INFINITE);
#endif
    if (QUEUE_NO_ERROR == hQueue->pfGetFree(queueId, (void**)&ptCmd))
    {
        PalMemcpy(ptCmd, ptInputCmd, sizeof(MPS_CMD_OBJ));
        hQueue->pfSetReady(queueId, (void**)&ptCmd);
    }
#ifdef WIN32
    PalReleaseMutex(gtCmdMutex);
#endif
}

void
mpsCmdQ_ReceiveCommand(
    QUEUE_ID        queueId,
    MPS_CMD_OBJ*    ptOutputCmd)
{
    MPS_CMD_OBJ* ptCmd;
    QUEUE_CTRL_HANDLE* hQueue = queueMgr_GetCtrlHandle(queueId);

#ifdef WIN32
    PalWaitMutex(gtCmdMutex, PAL_MUTEX_INFINITE);
#endif
    if (QUEUE_NO_ERROR == hQueue->pfGetReady(queueId, (void**)&ptCmd))
    {
        PalMemcpy(ptOutputCmd, ptCmd, sizeof(MPS_CMD_OBJ));
        hQueue->pfSetFree(queueId, (void**)&ptCmd);
    }
    else
        ptOutputCmd->cmd = MPS_COMMAND_NULL;
#ifdef WIN32
    PalReleaseMutex(gtCmdMutex);
#endif
}

void
mpsCmdQ_Terminate(
    QUEUE_ID queueId)
{
    queueMgr_DestroyQueue(queueId);
}
