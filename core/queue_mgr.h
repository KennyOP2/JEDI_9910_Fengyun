/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file stream_mgr.h
 * A simple message queue and queue buffer implementation.
 *
 * @author Steven Hsiao
 * @version 0.01
 */
#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H

#include "mmp_types.h"
#include "mmp_encoder.h"
#include "mmp_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
//#define CAPTURE_SAMPLE_COUNT    8
//#define CAPTURE_SAMPLE_SIZE     2073600 // 1920*1080
#ifdef MULTIPLE_INSTANCES
#    define VIDEO_SAMPLE_COUNT      16
#else
//#    ifndef ENABLE_MENCODER
#        define VIDEO_SAMPLE_COUNT  100
//#    else
//#        define VIDEO_SAMPLE_COUNT  256
//#    endif
#endif
//#define VIDEO_SAMPLE_SIZE       262144  // 256*1024
#ifndef ENABLE_MENCODER
#    define AUDIO_SAMPLE_COUNT      16
#else
#    define AUDIO_SAMPLE_COUNT      256
#endif
//#define AUDIO_SAMPLE_SIZE       2048    // 2*1024

//=============================================================================
//                              Macro Definition
//=============================================================================
typedef enum QUEUE_ID_TAG
{
    CAPTURE_QUEUE_ID = 0,
    AUDIO_QUEUE_ID,                         // 1
    VIDEO_QUEUE_ID,                         // 2
    LAST_DATA_QUEUE_ID = VIDEO_QUEUE_ID,    // 2
    CMD_QUEUE1_ID,                          // 3
    CMD_QUEUE2_ID,                          // 4
    CMD_QUEUE3_ID,                          // 5
    CMD_QUEUE4_ID,                          // 6
    CMD_QUEUE5_ID,                          // 7
    CMD_QUEUE6_ID,                          // 8
    CMD_QUEUE7_ID,                          // 9
    CMD_QUEUE8_ID,                          // 10
    EVENT_QUEUE_ID,                         // 11
    SPECIAL_EVENT_QUEUE_ID,                 // 12
    TOTAL_QUEUE_COUNT                       // 13
} QUEUE_ID;

typedef enum QUEUE_MGR_ERROR_CODE_TAG
{
    QUEUE_MGR_NOT_INIT  = -6,
    QUEUE_INVALID_INPUT = -5,
    QUEUE_IS_EMPTY      = -4,
    QUEUE_IS_FULL       = -3,
    QUEUE_EXIST         = -2,
    QUEUE_NOT_EXIST     = -1,
    QUEUE_NO_ERROR      = 0
} QUEUE_MGR_ERROR_CODE;

typedef enum VIDEO_FRAME_TYPE_TAG
{
    VIDEO_FRAME_TYPE_I,
    VIDEO_FRAME_TYPE_P
} VIDEO_FRAME_TYPE;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef QUEUE_MGR_ERROR_CODE (*QUEUE_OPERATION)(QUEUE_ID queueId, void** pptSample);

typedef struct QUEUE_CTRL_HANDLE_TAG
{
    QUEUE_OPERATION pfGetFree;
    QUEUE_OPERATION pfSetFree;
    QUEUE_OPERATION pfGetReady;
    QUEUE_OPERATION pfSetReady;
} QUEUE_CTRL_HANDLE;

//typedef struct CAPTURE_SAMPLE_TAG
//{
//    MMP_UINT8   pData[CAPTURE_SAMPLE_SIZE];
//    MMP_UINT32  dataSize;
//} CAPTURE_SAMPLE;

typedef struct VIDEO_SAMPLE_TAG
{
    AVC_FRAME_RATE  frameRate;
    MMP_UINT32      EnFrameRate;
    MMP_UINT32      width;
    MMP_UINT32      height;
#ifdef ENABLE_MENCODER
    MMP_UINT8*      AVCDecoderConfigurationRecord;
    MMP_UINT32      AVCDecoderConfigurationRecordSize;
#endif
    MMP_UINT32      frameType;  // VIDEO_FRAME_TYPE

    MMP_UINT8*      pData;
    MMP_UINT32      dataSize;
    MMP_UINT32      timeStamp;
    MMP_UINT32      InstanceNum;
    MMP_UINT32      frameCount;
    MMP_BOOL        binterlaced_frame;
    MMP_UINT32      bitRate;
    MMP_UINT16      encoderInfo;
    MMP_BOOL        bSkipFrame;
    MMP_BOOL        bAVCVideo;
} VIDEO_SAMPLE;

typedef struct AUDIO_SAMPLE_TAG
{
    MMP_UINT8*  pData;
    MMP_UINT32  dataSize;
    MMP_UINT32  timeStamp;
} AUDIO_SAMPLE;

typedef struct VIDEO_INFO_TAG
{
    AVC_FRAME_RATE  frameRate;
    MMP_UINT32      EnFrameRate;
    MMP_UINT32      width;
    MMP_UINT32      height;
#ifdef ENABLE_MENCODER
    MMP_UINT8*      AVCDecoderConfigurationRecord;
    MMP_UINT32      AVCDecoderConfigurationRecordSize;
#endif
} VIDEO_INFO;

typedef struct AUDIO_INFO_TAG
{
    MMP_AUDIO_ENGINE audioEncoderType;
} AUDIO_INFO;

//typedef struct AUDIO_SAMPLE_TAG
//{
//    MMP_UINT8   pData[AUDIO_SAMPLE_SIZE];
//    MMP_UINT32  dataSize;
//} AUDIO_SAMPLE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
//=============================================================================
/**
 * Used to initialize the simple queue manager.
 * @return  none
 */
//=============================================================================
void
queueMgr_InitManager(
    void);

//=============================================================================
/**
 * Used to destroy the simple queue manager.
 * @return  none
 */
//=============================================================================
void
queueMgr_DestroyManager(
    void);

//=============================================================================
/**
 * Used to Create a simple queue.
 * @param queueId       An identifier of the queue.
 * @param sampleSize    The size of data sample of the queue.
 * @param sampleCount   The total sample counts of the queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the creation is success
 *                      or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_CreateQueue(
    QUEUE_ID    queueId,
    MMP_UINT32  sampleSize,
    MMP_UINT32  sampleCount);

//=============================================================================
/**
 * Used to Delete a simple queue
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_MGR_ERROR_CODE whether the deletion is success or
 *                  not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_DestroyQueue(
    QUEUE_ID    queueId);

//=============================================================================
/**
 * Set the queue control operation handle
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_MGR_ERROR_CODE whether the set operation is success
 *                  or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetCtrlHandle(
    QUEUE_ID            queueId,
    QUEUE_CTRL_HANDLE*  ptHandle);

//=============================================================================
/**
 * Get the queue control operation handle
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_CTRL_HANDLE* the control operation handle of the
 *                  specific queue, However, if it is a null pointer, indicate
 *                  the get operation is failed.
 */
//=============================================================================
QUEUE_CTRL_HANDLE*
queueMgr_GetCtrlHandle(
    QUEUE_ID    queueId);

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
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetFree(
    QUEUE_ID    queueId,
    void**      pptSample);

//=============================================================================
/**
 * To release a used sample and update the readIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a used sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetFree(
    QUEUE_ID    queueId,
    void**      pptSample);

//=============================================================================
/**
 * To get a ready sample from queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a ready sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetReady(
    QUEUE_ID    queueId,
    void**      pptSample);

//=============================================================================
/**
 * To flagged a sample to ready state and update the writeIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a prepared ready sample pointer of queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetReady(
    QUEUE_ID    queueId,
    void**      pptSample);

//=============================================================================
/**
 * To get a the current used entry count of queue
 * @param queueId       An identifier of the queue.
 * @return              Number of available entries.
 */
//=============================================================================
MMP_INT32
queueMgr_GetUsedCount(
    QUEUE_ID    queueId);

//=============================================================================
/**
 * To get a the current available entry count of queue
 * @param queueId       An identifier of the queue.
 * @return              Number of available entries.
 */
//=============================================================================
MMP_INT32
queueMgr_GetAvailableCount(
    QUEUE_ID    queueId);

#ifdef __cplusplus
}
#endif

#endif
