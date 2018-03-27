/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file queue_mgr.c
 * A simple message queue and queue buffer implementation.
 *
 * @author Steven Hsiao
 * @version 0.01
 */

#include "pal/pal.h"
#include "queue_mgr.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define INVALID_QUEUE_ID (-1)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct QUEUE_DATA_BLOCK_TAG
{
    QUEUE_ID   queueId;
    MMP_UINT8  *pQueueStart;
    MMP_UINT32 sampleSize;
    MMP_UINT32 sampleCount;
    MMP_UINT32 writeIndex;
    MMP_UINT32 readIndex;
    MMP_UINT32 writeCount;
    MMP_UINT32 releaseCount;
} QUEUE_DATA_BLOCK;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_BOOL          gbManagerInit                   = MMP_FALSE;
static QUEUE_DATA_BLOCK  gptDataArray[TOTAL_QUEUE_COUNT] = { 0 };
static QUEUE_CTRL_HANDLE gptHandleArray[TOTAL_QUEUE_COUNT] = { 0 };

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Used to Init the simple queue manager
 * @return  none
 */
//=============================================================================
void
queueMgr_InitManager(
    void)
{
    MMP_UINT32 i;

    if (MMP_TRUE == gbManagerInit)
        return;

    PalMemset(gptDataArray, 0x0, sizeof(gptDataArray));
    PalMemset(gptHandleArray, 0x0, sizeof(gptHandleArray));

    for (i = 0; i < TOTAL_QUEUE_COUNT; i++)
        gptDataArray[i].queueId = INVALID_QUEUE_ID;

    gbManagerInit = MMP_TRUE;
}

//=============================================================================
/**
 * Used to Destory the simple queue manager
 * @return  none
 */
//=============================================================================
void
queueMgr_DestroyManager(
    void)
{
    MMP_UINT32 i;

    if (MMP_FALSE == gbManagerInit)
        return;

    for (i = 0; i < TOTAL_QUEUE_COUNT; i++)
    {
        if (gptDataArray[i].queueId != INVALID_QUEUE_ID)
        {
            if (gptDataArray[i].pQueueStart != MMP_NULL)
                PalHeapFree(PAL_HEAP_DEFAULT, gptDataArray[i].pQueueStart);
        }
    }
    gbManagerInit = MMP_FALSE;
}

//=============================================================================
/**
 * Used to Create a simple queue
 * @param queueId       An identifier of the queue.
 * @param sampleSize    The size of data sample of the queue.
 * @param sampleCount   The total sample counts of the queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the creation is successed
 *                      or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_CreateQueue(
    QUEUE_ID   queueId,
    MMP_UINT32 sampleSize,
    MMP_UINT32 sampleCount)
{
    if (MMP_FALSE == gbManagerInit)
        return QUEUE_MGR_NOT_INIT;

    if (queueId == gptDataArray[queueId].queueId)
        return QUEUE_EXIST;

    gptDataArray[queueId].queueId      = queueId;
    gptDataArray[queueId].sampleSize   = sampleSize;
    gptDataArray[queueId].sampleCount  = sampleCount;
    gptDataArray[queueId].writeCount   = 0;
    gptDataArray[queueId].releaseCount = 0;

    gptDataArray[queueId].pQueueStart  = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                    (sampleSize * sampleCount));

    gptHandleArray[queueId].pfGetFree  = queueMgr_GetFree;
    gptHandleArray[queueId].pfSetFree  = queueMgr_SetFree;
    gptHandleArray[queueId].pfGetReady = queueMgr_GetReady;
    gptHandleArray[queueId].pfSetReady = queueMgr_SetReady;

    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * Used to Delete a simple queue
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_MGR_ERROR_CODE wheter the deletion is successed or
 *                  not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_DestroyQueue(
    QUEUE_ID queueId)
{
    if (MMP_FALSE == gbManagerInit)
        return QUEUE_MGR_NOT_INIT;

    if (INVALID_QUEUE_ID == gptDataArray[queueId].queueId)
        return QUEUE_NOT_EXIST;

    if (gptDataArray[queueId].pQueueStart)
        PalHeapFree(PAL_HEAP_DEFAULT, gptDataArray[queueId].pQueueStart);

    PalMemset(&gptDataArray[queueId], 0x0, sizeof(QUEUE_DATA_BLOCK));
    gptDataArray[queueId].queueId = INVALID_QUEUE_ID;

    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * Set the queue control operation handle
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_MGR_ERROR_CODE wheter the set operation is successed
 *                  or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetCtrlHandle(
    QUEUE_ID          queueId,
    QUEUE_CTRL_HANDLE *ptHandle)
{
    if (MMP_FALSE == gbManagerInit)
        return QUEUE_MGR_NOT_INIT;

    if (INVALID_QUEUE_ID == gptDataArray[queueId].queueId)
        return QUEUE_NOT_EXIST;

    PalMemcpy(&gptHandleArray[queueId], ptHandle, sizeof(QUEUE_CTRL_HANDLE));
    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * Get the queue control operation handle
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_CTRL_HANDLE* the control operation handle of the
 *                  specific queue, However, if it is a null pointer, indicate
 *                  the get operation is failed.
 */
//=============================================================================
QUEUE_CTRL_HANDLE *
queueMgr_GetCtrlHandle(
    QUEUE_ID queueId)
{
    if ((MMP_FALSE == gbManagerInit)
        || (INVALID_QUEUE_ID == gptDataArray[queueId].queueId))
        return MMP_NULL;

    return &gptHandleArray[queueId];
}

// Note1: If a user create a queue as a queue buffer, these API can be
//        hanged on CTRL_HANDLE block of queue then be used directly.
// Note2: However, if a user use the queue as a message queue, sometimes,
//        the user module must be notified to free or clear some buffer or
//        data pointer. Under such a case, the user has to implement proper
//        queue operation functions. A simple example code is shown below for
//        reference.
//        QUEUE_MGR_ERROR_CODE_TAG
//        testExample_SetFree(
//            QUEUE_ID            queueId,
//            void**              pptSample)
//        {
//            QUEUE_MGR_ERROR_CODE_TAG result = QUEUE_NO_ERROR;
//            if ((result = queueMgr_SetFree(queueId, pptSample))
//             != QUEUE_NO_ERROR)
//            {
//                return result;
//            }
//            PalHeapFree(PAL_HEAP_DEAFULT, ((XXX_SAMPLE*)*pptSample)->pData);
//        }
//
//=============================================================================
/**
 * To get a free sample from queue
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a free sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetFree(
    QUEUE_ID queueId,
    void     **pptSample)
{
    if (MMP_NULL == pptSample)
        return QUEUE_INVALID_INPUT;

    if (gptDataArray[queueId].sampleCount == queueMgr_GetUsedCount(queueId))
        return QUEUE_IS_FULL;

    *pptSample = (void *)(gptDataArray[queueId].pQueueStart
                          + (gptDataArray[queueId].writeIndex * gptDataArray[queueId].sampleSize));
    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * To relese a used sample and update the readIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a used sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetFree(
    QUEUE_ID queueId,
    void     **pptSample)
{
    if (MMP_NULL == pptSample || MMP_NULL == *pptSample)
        return QUEUE_INVALID_INPUT;

    if (*pptSample
        == (void *)(gptDataArray[queueId].pQueueStart
                    + (gptDataArray[queueId].readIndex * gptDataArray[queueId].sampleSize)))
    {
        if ((gptDataArray[queueId].readIndex + 1) == gptDataArray[queueId].sampleCount)
            gptDataArray[queueId].readIndex = 0;
        else
            ++gptDataArray[queueId].readIndex;

        ++gptDataArray[queueId].releaseCount;

        return QUEUE_NO_ERROR;
    }
    return QUEUE_INVALID_INPUT;
}

//=============================================================================
/**
 * To get a ready sample from queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a ready sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetReady(
    QUEUE_ID queueId,
    void     **pptSample)
{
    if (MMP_NULL == pptSample)
        return QUEUE_INVALID_INPUT;

    if (0 == queueMgr_GetUsedCount(queueId))
    {
        return QUEUE_IS_EMPTY;
    }

    *pptSample = (void *)(gptDataArray[queueId].pQueueStart
                          + (gptDataArray[queueId].readIndex * gptDataArray[queueId].sampleSize));
    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * To flagged a sample to ready state and update the writeIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a prepared ready sample pointer of queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetReady(
    QUEUE_ID queueId,
    void     **pptSample)
{
    if (MMP_NULL == pptSample || MMP_NULL == *pptSample)
        return QUEUE_INVALID_INPUT;

    if (*pptSample
        == (void *)(gptDataArray[queueId].pQueueStart
                    + (gptDataArray[queueId].writeIndex * gptDataArray[queueId].sampleSize)))
    {
        if ((gptDataArray[queueId].writeIndex + 1) == gptDataArray[queueId].sampleCount)
            gptDataArray[queueId].writeIndex = 0;
        else
            ++gptDataArray[queueId].writeIndex;

        gptDataArray[queueId].writeCount++;

        return QUEUE_NO_ERROR;
    }
    return QUEUE_INVALID_INPUT;
}

MMP_INT32
queueMgr_GetUsedCount(
    QUEUE_ID queueId)
{
    MMP_INT32  currentSampleCount = 0;
    MMP_UINT32 writeCount         = gptDataArray[queueId].writeCount;
    MMP_UINT32 releaseCount       = gptDataArray[queueId].releaseCount;

    if (writeCount >= releaseCount)
    {
        currentSampleCount = (MMP_INT32) writeCount - releaseCount;
    }
    else
    {
        currentSampleCount = (MMP_INT32) (writeCount + (0xFFFFFFFF - releaseCount));
    }

    if (currentSampleCount > gptDataArray[queueId].sampleCount || currentSampleCount < 0)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), currentSampleCount: %d\n", __FILE__, __LINE__, currentSampleCount);
        while (1);
    }
    return currentSampleCount;
}

//=============================================================================
/**
 * To get a the current available entry count of queue
 * @param queueId       An identifier of the queue.
 * @return              Number of available entries.
 */
//=============================================================================
MMP_INT32
queueMgr_GetAvailableCount(
    QUEUE_ID queueId)
{
    MMP_INT32 avaCount = (MMP_INT32)(gptDataArray[queueId].sampleCount - queueMgr_GetUsedCount(queueId));
    if (avaCount > gptDataArray[queueId].sampleCount || avaCount < 0)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), availableCount: %d, max: %u\n", __FILE__, __LINE__, avaCount, gptDataArray[queueId].sampleCount);
        while (1);
    }
    return avaCount;
}