#include "pal/pal.h"
#include "task_avi_mux.h"
#include "mencoder/mencoder.h"
#include "mps_system.h"
#include "mps_control.h"
#include "mmp_aud.h"
#include "core_interface.h"
#include "mmio.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef enum RECORD_STATE_TAG
{
    RECORD_STATE_NO_RECORD = 0,
    RECORD_STATE_IN_RECORD
} RECORD_STATE;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct TASK_AVI_MUX_HANDLE_TAG
{
    MPS_ELEMENT      *ptElement;
    MPS_STATE        mpsState;
    STREAM_HANDLE    tVideoEncoderHandle;
    STREAM_HANDLE    tAudioInHandle;
    PAL_THREAD       ptStreamMuxThread;
    MPS_CMD_OBJ      tReadCmdObj;
    MPS_CMD_OBJ      tWriteCmdObj;
    MEContext        *ptMEctx;
    MMP_UINT32       videoTimeStamp;
    MMP_UINT32       audioTimeStamp;
    MMP_UINT32       baseTimeStamp;
    MMP_AUDIO_ENGINE audioEncoderType;

    // for record+
    RECORD_STATE     recordState;
    MMP_BOOL         bVideoInfoReady;
    MMP_BOOL         bDropVideoSample;
    RECORD_MODE      recordMode;
    // for record-
} TASK_AVI_MUX_HANDLE;

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_EVENT elementThreadToMpsThread;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static TASK_AVI_MUX_HANDLE gtAviMuxHandle = {0};

// 20140424, Vincent add
// two dirty flags to indicate start and finish of mencoder process.
extern MMP_BOOL              gbmencoder_OpenProcess;
extern MMP_BOOL              gbmencoder_CloseProcess;
MMP_BOOL                     gbJPEGEncode   = MMP_FALSE;

static JPEG_ENCODER_CALLBACK gpfJpgCallback = MMP_NULL;
//=============================================================================
//                              Private Function Declaration
//=============================================================================
//AVI_MUX
static void
_AVI_MUX_Init(
    void);

static void
_AVI_MUX_DoPlay(
    void);

static void
_AVI_MUX_DoStop(
    void);

static MMP_BOOL
_AVI_MUX_DoOpen(
    MMP_WCHAR *ptFilePath);

static MMP_BOOL
_AVI_MUX_DoClose(
    void);

static void
_AVI_MUX_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data);

static void *
_AVI_MUX_ThreadFunction(
    void *arg);

//JPG MUX
static void
_JPG_MUX_DoPlay(
    void);

static void
_JPG_MUX_DoStop(
    void);

static MMP_BOOL
_JPG_MUX_DoOpen(
    JPEG_CONTEXT *ptJpgCtx);

static MMP_BOOL
_JPG_MUX_DoClose(
    void);

static void
_JPG_MUX_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data);

static void *
_JPG_MUX_ThreadFunction(
    void *arg);

//TOP MUX
static void
_MUX_DoPlay(
    void);

static void
_MUX_DoStop(
    void);

static MMP_BOOL
_MUX_DoClose(
    void);

static void
_MUX_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data);

static void *
_MUX_ThreadFunction(
    void *arg);
//=============================================================================
//                              Public Function Definition
//=============================================================================

void
taskAviMux_Init(
    MMP_UINT32 arg)
{
    MPS_CONNECTOR_LIST_OBJ *ptListObj   = MMP_NULL;
    MPS_CONNECTOR          *ptConnector = MMP_NULL;

    if (arg) { } // avoid compiler warning

    if (gtAviMuxHandle.mpsState != MPS_STATE_ZERO)
        return;

    _AVI_MUX_Init();

    gtAviMuxHandle.mpsState  = MPS_STATE_STOP;
    gtAviMuxHandle.ptElement = mpsSys_GetElement(MPS_AVI_MUX);

    if ((ptListObj = (gtAviMuxHandle.ptElement)->ptSrcList) != MMP_NULL)
    {
        do
        {
            ptConnector = ptListObj->ptConnector;

            switch (ptConnector->ptSrcElement->id)
            {
            case MPS_VIDEO_ENCODER:
                gtAviMuxHandle.tVideoEncoderHandle.queueId       =
                    ptListObj->ptConnector->queueId;
                gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle =
                    queueMgr_GetCtrlHandle(gtAviMuxHandle.tVideoEncoderHandle.queueId);
                dbg_msg(DBG_MSG_TYPE_INFO, "video queue_id: %u\n", ptListObj->ptConnector->queueId);
                break;

            case MPS_AUDIO_IN:
                gtAviMuxHandle.tAudioInHandle.queueId       =
                    ptListObj->ptConnector->queueId;
                gtAviMuxHandle.tAudioInHandle.ptQueueHandle =
                    queueMgr_GetCtrlHandle(gtAviMuxHandle.tAudioInHandle.queueId);
                dbg_msg(DBG_MSG_TYPE_INFO, "audio queue_id: %u\n", ptListObj->ptConnector->queueId);
                break;

            default:
                break;
            }
        } while ((ptListObj = ptListObj->ptNext) != MMP_NULL);
    }

    if (MMP_NULL == gtAviMuxHandle.ptStreamMuxThread)
    {
        gtAviMuxHandle.ptStreamMuxThread = PalCreateThread(PAL_THREAD_STREAM_MUX,
                                                           _AVI_MUX_ThreadFunction,
                                                           MMP_NULL, 0, 0);
    }
}

void
taskAviMux_Terminate(
    MMP_UINT32 arg)
{
    if (arg) { } // avoid compiler warning

    if (gtAviMuxHandle.ptStreamMuxThread != MMP_NULL)
        PalDestroyThread(gtAviMuxHandle.ptStreamMuxThread);

    mencoder_Terminate(gtAviMuxHandle.ptMEctx);
    PalMemset(&gtAviMuxHandle, 0x0, sizeof(TASK_AVI_MUX_HANDLE));
}

//QUEUE_MGR_ERROR_CODE
//taskAviMux_SetFree(
//    QUEUE_ID queueId,
//    void** pptSample)
//{
//
//}

//=============================================================================
//                              Private Function Definition
//=============================================================================

static void
_AVI_MUX_Init(
    void)
{
    gtAviMuxHandle.ptMEctx = mencoder_Init();
    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d)\n", __FUNCTION__, __LINE__);
}

static void
_AVI_MUX_DoPlay(
    void)
{
    MMP_UINT32      return_state              = 0;
    VIDEO_SAMPLE    *ptVideoSample            = MMP_NULL;
    AUDIO_SAMPLE    *ptAudioSample            = MMP_NULL;
    QUEUE_ID        video_queueId             = gtAviMuxHandle.tVideoEncoderHandle.queueId;
    QUEUE_OPERATION pfVideoInputQueueGetReady = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfVideoInputQueueSetFree  = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfSetFree;
    QUEUE_ID        audio_queueId             = gtAviMuxHandle.tAudioInHandle.queueId;
    QUEUE_OPERATION pfAudioInputQueueGetReady = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfAudioInputQueueSetFree  = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfSetFree;
    MMP_UINT32      timeGap                   = 0;
    MMP_BOOL        bDiscontinuity            = MMP_FALSE;
    PAL_CLOCK_T     clock                     = PalGetClock();
    MMP_RESULT      result                    = MMP_SUCCESS;

    if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState
        && gtAviMuxHandle.bDropVideoSample)
    {
        while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void **)&ptVideoSample))
        {
            pfVideoInputQueueSetFree(video_queueId, (void **) &ptVideoSample);
        }
        gtAviMuxHandle.bDropVideoSample = MMP_FALSE;
    }

    while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void **)&ptVideoSample))
    {
        //dbg_msg(DBG_MSG_TYPE_INFO, "v: %u, c: %u\n", ptVideoSample->timeStamp, ptVideoSample->frameCount);
        if (gtAviMuxHandle.videoTimeStamp <= ptVideoSample->timeStamp)
        {
            timeGap = ptVideoSample->timeStamp - gtAviMuxHandle.videoTimeStamp;
        }
        else
        {
            timeGap = (0xFFFFFFFF - gtAviMuxHandle.videoTimeStamp) + ptVideoSample->timeStamp;
        }

        if (timeGap > 100)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "v time gap: %u is larger than 100 ms, prev: %u, curr: %u\n", timeGap, gtAviMuxHandle.videoTimeStamp, ptVideoSample->timeStamp);
            bDiscontinuity = MMP_TRUE;
        }
        else
        {
            bDiscontinuity = MMP_FALSE;
        }

        gtAviMuxHandle.videoTimeStamp = ptVideoSample->timeStamp;
        avSyncSetCurrentVideoMuxTime(ptVideoSample->timeStamp);
        //dbg_msg(DBG_MSG_TYPE_INFO, "v size(%d)\n", ptVideoSample->dataSize);
        if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState)
        {
            //PAL_CLOCK_T clock = PalGetClock();

            if (!gtAviMuxHandle.bVideoInfoReady
                && (VIDEO_FRAME_TYPE_I == ptVideoSample->frameType))
            {
                mencoder_SetVideoInfo(
                    gtAviMuxHandle.ptMEctx,
                    (VIDEO_INFO *)ptVideoSample);
                gtAviMuxHandle.bVideoInfoReady = MMP_TRUE;
                gtAviMuxHandle.baseTimeStamp   = ptVideoSample->timeStamp;
            }
            if (gtAviMuxHandle.bVideoInfoReady)
            {
                result = mencoder_WriteVideoChunk(
                    gtAviMuxHandle.ptMEctx,
                    ptVideoSample->pData,
                    ptVideoSample->dataSize,
                    (double)(ptVideoSample->timeStamp - gtAviMuxHandle.baseTimeStamp) / 1000,
                    (VIDEO_FRAME_TYPE_I == ptVideoSample->frameType));
            }

            //dbg_msg(DBG_MSG_TYPE_INFO, "(%d)-%d\n", ptVideoSample->frameCount, PalGetDuration(clock));
        }

        //if ((!gtAviMuxHandle.bVideoInfoReady) || (MMP_SUCCESS == result))
        pfVideoInputQueueSetFree(video_queueId, (void **) &ptVideoSample);
        //else
        //    break;
    }

    result = MMP_SUCCESS;
    while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void **)&ptAudioSample))
    {
        if (gtAviMuxHandle.audioTimeStamp <= ptAudioSample->timeStamp)
        {
            timeGap = ptAudioSample->timeStamp - gtAviMuxHandle.audioTimeStamp;
        }
        else
        {
            timeGap = (0xFFFFFFFF - gtAviMuxHandle.audioTimeStamp) + ptAudioSample->timeStamp;
        }

        if (timeGap > 100)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "a time gap: %u is larger than 100 ms, prev: %u, curr: %u\n", timeGap, gtAviMuxHandle.audioTimeStamp, ptAudioSample->timeStamp);
            bDiscontinuity = MMP_TRUE;
        }
        else
        {
            bDiscontinuity = MMP_FALSE;
        }

        gtAviMuxHandle.audioTimeStamp = ptAudioSample->timeStamp;
        //dbg_msg(DBG_MSG_TYPE_INFO, "a: %u, v: %u, gap: %d\n",
        //        gtAviMuxHandle.audioTimeStamp,
        //        gtAviMuxHandle.videoTimeStamp,
        //        gtAviMuxHandle.videoTimeStamp - gtAviMuxHandle.audioTimeStamp);
        //dbg_msg(DBG_MSG_TYPE_INFO, "a size(%d)\n", ptAudioSample->dataSize);
        if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState)
        {
            if (gtAviMuxHandle.bVideoInfoReady
                && ptAudioSample->timeStamp >= gtAviMuxHandle.baseTimeStamp)
            {
                result = mencoder_WriteAudioChunk(
                    gtAviMuxHandle.ptMEctx,
                    ptAudioSample->pData,
                    ptAudioSample->dataSize,
                    (double)(ptAudioSample->timeStamp - gtAviMuxHandle.baseTimeStamp) / 1000);
            }
        }
        //if ((!gtAviMuxHandle.bVideoInfoReady) || (MMP_SUCCESS == result))
        pfAudioInputQueueSetFree(audio_queueId, (void **) &ptAudioSample);
        //else
        //    break;
    }
}

static void
_AVI_MUX_DoStop(
    void)
{
    VIDEO_SAMPLE    *ptVideoSample            = MMP_NULL;
    AUDIO_SAMPLE    *ptAudioSample            = MMP_NULL;
    QUEUE_ID        video_queueId             = gtAviMuxHandle.tVideoEncoderHandle.queueId;
    QUEUE_OPERATION pfVideoInputQueueGetReady = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfVideoInputQueueSetFree  = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfSetFree;
    QUEUE_ID        audio_queueId             = gtAviMuxHandle.tAudioInHandle.queueId;
    QUEUE_OPERATION pfAudioInputQueueGetReady = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfAudioInputQueueSetFree  = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfSetFree;

    while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void **)&ptVideoSample))
    {
        pfVideoInputQueueSetFree(video_queueId, (void **)&ptVideoSample);
    }
    while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void **)&ptAudioSample))
    {
        pfAudioInputQueueSetFree(audio_queueId, (void **)&ptAudioSample);
    }
    gtAviMuxHandle.audioTimeStamp = 0;
    gtAviMuxHandle.baseTimeStamp  = 0;
}

static MMP_BOOL
_AVI_MUX_DoOpen(
    MMP_WCHAR *ptFilePath)
{
    MMP_RESULT result;
    MMP_UINT32 value         = 0;
    MMP_UINT32 timeout_count = 0;
    AUDIO_INFO tAudioInfo    = {0};
    tAudioInfo.audioEncoderType = gtAviMuxHandle.audioEncoderType;

    // 20140424, Vincent note
    // I don't know why just doubt that the cause of system crash is
    // running mencoder open during audio engine start.
    // so wait audio engine start finish before mencoder open to
    // avoid this case.
    do
    {
        PalSleep(1);
        HOST_ReadRegister(AUDIO_DECODER_START_FALG, &value);
        timeout_count++;
    } while (value < 2 && timeout_count < 2000);
    if (timeout_count == 2000)
        dbg_msg(DBG_MSG_TYPE_ERROR, "wait audio engine fire timeout before mencoder open\n");

    //printf("mencoder_Open +, timeout_count: %d\n", timeout_count);
    gbmencoder_OpenProcess = MMP_TRUE;
    result                 = mencoder_SetAudioInfo(gtAviMuxHandle.ptMEctx, &tAudioInfo);
    result                 = mencoder_Open(gtAviMuxHandle.ptMEctx, ptFilePath, &gtAviMuxHandle.recordMode);
    gbmencoder_OpenProcess = MMP_FALSE;
    //printf("mencoder_Open -\n");
    return (MMP_SUCCESS == result);
}

static MMP_BOOL
_AVI_MUX_DoClose(
    void)
{
    MMP_RESULT result;
    //printf("mencoder_Close +\n");
    gbmencoder_CloseProcess = MMP_TRUE;
    result                  = mencoder_Close(gtAviMuxHandle.ptMEctx);
    gbmencoder_CloseProcess = MMP_FALSE;
    //printf("mencoder_Close -\n");
    return (MMP_SUCCESS == result);
}

static void
_AVI_MUX_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data)
{
    MPS_NOTIFY_REASON_DATA *ptNotifyData = MMP_NULL;

    gtAviMuxHandle.tWriteCmdObj.cmd = MPS_COMMAND_EVENT_NOTIFY;
    if (reason)
    {
        ptNotifyData                          = (MPS_NOTIFY_REASON_DATA *)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                                       sizeof(MPS_NOTIFY_REASON_DATA));
        ptNotifyData->notifyReason            = reason;
        ptNotifyData->bDatatNeedFree          = MMP_FALSE;
        ptNotifyData->data                    = data;
        gtAviMuxHandle.tWriteCmdObj.extraData = (void *)ptNotifyData;
        mpsCmdQ_SendCommand(gtAviMuxHandle.ptElement->specialEventId, &gtAviMuxHandle.tWriteCmdObj);
    }
    else
        PalSetEvent(elementThreadToMpsThread);
}

static void *
_AVI_MUX_ThreadFunction(
    void *arg)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;

    if (arg) { } // avoid compiler warning

    for (;;)
    {
        if (MPS_STATE_RUN == gtAviMuxHandle.mpsState)
            _MUX_DoPlay();

        mpsCmdQ_ReceiveCommand(gtAviMuxHandle.ptElement->cmdQueueId, &gtAviMuxHandle.tReadCmdObj);

        if ((gtAviMuxHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK) != MPS_COMMAND_NULL)
        {
            switch (gtAviMuxHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK)
            {
            case MPS_COMMAND_PLAY:
                gtAviMuxHandle.mpsState = MPS_STATE_RUN;
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP:
                gtAviMuxHandle.mpsState = MPS_STATE_STOP;
                _MUX_DoStop();
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_TERMINATE:
                gtAviMuxHandle.mpsState = MPS_STATE_ZERO;
                goto end;

            case MPS_COMMAND_START_JPEG_RECORD:
                gbJPEGEncode = MMP_TRUE;
                ptData       = (MPS_PROPERITY_DATA *)gtAviMuxHandle.tReadCmdObj.extraData;
                if (ptData)
                {
                    if (_JPG_MUX_DoOpen((JPEG_CONTEXT *)ptData->data))
                    {
                        gtAviMuxHandle.recordState      = RECORD_STATE_IN_RECORD;
                        gtAviMuxHandle.bDropVideoSample = MMP_TRUE;
                        gtAviMuxHandle.bVideoInfoReady  = MMP_FALSE;
                        dbg_msg(DBG_MSG_TYPE_INFO, "_JPG_MUX_OPEN_FILE_SUCCESS....\n");
                        _MUX_EventNotify(MPS_NOTIFY_REASON_OPEN_FILE_SUCCESS, 0);
                    }
                    else
                    {
                        dbg_msg(DBG_MSG_TYPE_ERROR, "_JPG_MUX_DoOpen() fail !! \n");
                        _MUX_EventNotify(MPS_NOTIFY_REASON_OPEN_FILE_FAIL, 0);
                    }
                }
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_START_RECORD:
                ptData = (MPS_PROPERITY_DATA *)gtAviMuxHandle.tReadCmdObj.extraData;
                if (ptData)
                {
                    if (_AVI_MUX_DoOpen((MMP_WCHAR *)ptData->data))
                    {
                        gtAviMuxHandle.recordState      = RECORD_STATE_IN_RECORD;
                        gtAviMuxHandle.bDropVideoSample = MMP_TRUE;
                        gtAviMuxHandle.bVideoInfoReady  = MMP_FALSE;
                        dbg_msg(DBG_MSG_TYPE_INFO, "_AVI_MUX_OPEN_FILE_SUCCESS....\n");
                        _MUX_EventNotify(MPS_NOTIFY_REASON_OPEN_FILE_SUCCESS, 0);
                    }
                    else
                    {
                        dbg_msg(DBG_MSG_TYPE_ERROR, "_AVI_MUX_DoOpen() fail !! \n");
                        _MUX_EventNotify(MPS_NOTIFY_REASON_OPEN_FILE_FAIL, 0);
                    }
                }
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP_RECORD:
                if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState)
                {
                    if (_MUX_DoClose())
                    {
                        gtAviMuxHandle.recordState     = RECORD_STATE_NO_RECORD;
                        gtAviMuxHandle.bVideoInfoReady = MMP_FALSE;
                        _MUX_EventNotify(MPS_NOTIFY_REASON_FILE_CLOSE_SUCCESS, 0);
                    }
                    else
                    {
                        dbg_msg(DBG_MSG_TYPE_ERROR, "_AVI_MUX_DoClose() fail !! \n");
                        _MUX_EventNotify(MPS_NOTIFY_REASON_FILE_CLOSE_FAIL, 0);
                    }
                }
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP_JPEG_RECORD:
                if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState)
                {
                    if (_MUX_DoClose())
                    {
                        gtAviMuxHandle.recordState     = RECORD_STATE_NO_RECORD;
                        gtAviMuxHandle.bVideoInfoReady = MMP_FALSE;
                        _MUX_EventNotify(MPS_NOTIFY_REASON_FILE_CLOSE_SUCCESS, 0);
                    }
                    else
                    {
                        dbg_msg(DBG_MSG_TYPE_ERROR, "_AVI_MUX_DoClose() fail !! \n");
                        _MUX_EventNotify(MPS_NOTIFY_REASON_FILE_CLOSE_FAIL, 0);
                    }
                }
                gbJPEGEncode = MMP_FALSE;
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_CLOSE:
            //case MPS_COMMAND_SET_ENCODE_PARAMETER:
            //case MPS_COMMAND_SET_CAPTURE_DEVICE:
            //case MPS_COMMAND_SET_ISP_MODE:
            //case MPS_COMMAND_SET_ENABLE_AV_ENGINE:
            case MPS_COMMAND_GET_PROPERTY:
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_SET_PROPERTY:
                ptData = (MPS_PROPERITY_DATA *)gtAviMuxHandle.tReadCmdObj.extraData;
                switch (ptData->properityId)
                {
                case MPS_PROPERITY_SET_MUXER_PARAMETER:
                    {
                        TS_MUXER_PARAMETER *ptMuxerParam = (TS_MUXER_PARAMETER *) ptData->data;
                        switch (ptMuxerParam->audioEncoderType)
                        {
                        case MPEG_AUDIO_ENCODER:
                            gtAviMuxHandle.audioEncoderType = MMP_MP2_ENCODE;
                            break;

                        case AAC_AUDIO_ENCODER:
                        default:
                            gtAviMuxHandle.audioEncoderType = MMP_AAC_ENCODE;
                            break;
                        }
                    }
                    break;

                case MPS_PROPERITY_SET_RECORD_MODE:
                    {
                        RECORD_MODE *ptRecMode = (RECORD_MODE *) ptData->data;
                        PalMemcpy(&gtAviMuxHandle.recordMode, ptRecMode, sizeof(RECORD_MODE));
                    }
                    break;

                default:
                    break;
                }
                _MUX_EventNotify(MMP_NULL, 0);
                break;

            default:
                break;
            }
        }

        PalSleep(5);
    }

end:
    _MUX_EventNotify(MMP_NULL, 0);
    return MMP_NULL;
}

static void
_JPG_MUX_DoPlay(
    void)
{
    MMP_UINT32      return_state              = 0;
    VIDEO_SAMPLE    *ptVideoSample            = MMP_NULL;
    AUDIO_SAMPLE    *ptAudioSample            = MMP_NULL;
    QUEUE_ID        video_queueId             = gtAviMuxHandle.tVideoEncoderHandle.queueId;
    QUEUE_OPERATION pfVideoInputQueueGetReady = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfVideoInputQueueSetFree  = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfSetFree;
    QUEUE_ID        audio_queueId             = gtAviMuxHandle.tAudioInHandle.queueId;
    QUEUE_OPERATION pfAudioInputQueueGetReady = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfAudioInputQueueSetFree  = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfSetFree;
    MMP_UINT32      timeGap                   = 0;
    MMP_BOOL        bDiscontinuity            = MMP_FALSE;
    PAL_CLOCK_T     clock                     = PalGetClock();
    MMP_RESULT      result                    = MMP_SUCCESS;
    JPEG_STREAM     jpgStream;

    if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState
        && gtAviMuxHandle.bDropVideoSample)
    {
        while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void **)&ptVideoSample))
        {
            pfVideoInputQueueSetFree(video_queueId, (void **) &ptVideoSample);
        }
        gtAviMuxHandle.bDropVideoSample = MMP_FALSE;
    }

    while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void **)&ptVideoSample))
    {
        if (RECORD_STATE_IN_RECORD == gtAviMuxHandle.recordState && !ptVideoSample->bAVCVideo)
        {
            PAL_CLOCK_T clock = PalGetClock();

            jpgStream.pHdrBuf    = ptVideoSample->AVCDecoderConfigurationRecord;
            jpgStream.headerSize = ptVideoSample->AVCDecoderConfigurationRecordSize;
            jpgStream.pStreamBuf = ptVideoSample->pData;
            jpgStream.streamSize = ptVideoSample->dataSize;
            if (gpfJpgCallback != MMP_NULL)
                (*gpfJpgCallback)(&jpgStream);

            //printf("JPEG DOPlay %x %x\n", jpgStream.pHdrBuf, ptVideoSample->AVCDecoderConfigurationRecord);
            //if (gpfJpgCallback != MMP_NULL)
            //    (*gpfJpgCallback)(ptVideoSample->pData, ptVideoSample->dataSize);

            //printf("JPEG Sample %x %d Time %d\n", ptVideoSample->pData, ptVideoSample->dataSize, PalGetDuration(clock));
        }

        pfVideoInputQueueSetFree(video_queueId, (void **) &ptVideoSample);
    }

    result = MMP_SUCCESS;
    while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void **)&ptAudioSample))
    {
        pfAudioInputQueueSetFree(audio_queueId, (void **) &ptAudioSample);
    }
}

static void
_JPG_MUX_DoStop(
    void)
{
    VIDEO_SAMPLE    *ptVideoSample            = MMP_NULL;
    AUDIO_SAMPLE    *ptAudioSample            = MMP_NULL;
    QUEUE_ID        video_queueId             = gtAviMuxHandle.tVideoEncoderHandle.queueId;
    QUEUE_OPERATION pfVideoInputQueueGetReady = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfVideoInputQueueSetFree  = gtAviMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfSetFree;
    QUEUE_ID        audio_queueId             = gtAviMuxHandle.tAudioInHandle.queueId;
    QUEUE_OPERATION pfAudioInputQueueGetReady = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfAudioInputQueueSetFree  = gtAviMuxHandle.tAudioInHandle.ptQueueHandle->pfSetFree;

    while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void **)&ptVideoSample))
    {
        pfVideoInputQueueSetFree(video_queueId, (void **)&ptVideoSample);
    }
    while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void **)&ptAudioSample))
    {
        pfAudioInputQueueSetFree(audio_queueId, (void **)&ptAudioSample);
    }
    gtAviMuxHandle.audioTimeStamp = 0;
    gtAviMuxHandle.baseTimeStamp  = 0;
}

static MMP_BOOL
_JPG_MUX_DoOpen(
    JPEG_CONTEXT *ptJpgCtx)
{
    if (ptJpgCtx->pfJPEGCallback)
        gpfJpgCallback = ptJpgCtx->pfJPEGCallback;

    //printf("_JPG_MUX_DoOpen %d %d %x\n", ptJpgCtx->bOneShot, ptJpgCtx->encodingPeriod, ptJpgCtx->pfJPEGCallback);

    return MMP_TRUE;
}

static MMP_BOOL
_JPG_MUX_DoClose(
    void)
{
    MMP_RESULT result;

    result = MMP_SUCCESS;

    return (MMP_SUCCESS == result);
}

static void
_MUX_DoPlay(
    void)
{
    if (gbJPEGEncode)
        _JPG_MUX_DoPlay();
    else
        _AVI_MUX_DoPlay();
}

static void
_MUX_DoStop(
    void)
{
    if (gbJPEGEncode)
        _JPG_MUX_DoStop();
    else
        _AVI_MUX_DoStop();
}

static MMP_BOOL
_MUX_DoClose(
    void)
{
    MMP_RESULT result;

    if (gbJPEGEncode)
        result = _JPG_MUX_DoClose();
    else
        result = _AVI_MUX_DoClose();

    return result;
}

static void
_MUX_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data)
{
    _AVI_MUX_EventNotify(reason, data);
}