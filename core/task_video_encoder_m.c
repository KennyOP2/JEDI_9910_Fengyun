#ifdef __FREERTOS__
    #ifdef __OPENRTOS__
        #include "ite/ith.h"
    #else
        #include "intr/intr.h"
        #include "or32.h"
    #endif
#endif

#include "av_sync.h"
#include "task_video_encoder.h"
#include "mps_control.h"
#include "mps_system.h"
#include "mmp_encoder.h"
#include "mmp_capture.h"
#include "mmp_isp.h"
#include "mmp_timer.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define CAPTURE_RESULT_QUEUE_SIZE 9
#define ENCODED_RESULT_QUEUE_SIZE CAPTURE_RESULT_QUEUE_SIZE

#define QUEUE_EMPTY               (-1)
#define QUEUE_NOT_EMPTY           (0)

#define TIMER_NUM                 4
#define ISP_TRANSFORM_NUM         3
#define ENCODE_STREAM_NUM         3
//#define _DEBUG_ISP_RUN_TIME

typedef enum CAPTURE_STATE_TAG
{
    CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR,
    CAPTURE_STATE_POLLING_DEVICE_STATUS,
    CAPTURE_STATE_FIRE_CAPTURE,
    CAPTURE_STATE_WAIT_CAPTURE_SYNC,
    CAPTURE_STATE_RETRY_CAPTURE_POLARITY,
    CAPTURE_STATE_RESET_CAPTURE_AND_ISP
} CAPTURE_STATE;

typedef enum FRAME_RATE_SETTING_STATE_TAG
{
    FRAME_RATE_SETTING_STATE_INIT,
    FRAME_RATE_SETTING_STATE_DETECTED,
    FRAME_RATE_SETTING_STATE_SET
} FRAME_RATE_SETTING_STATE;

//=============================================================================
//                Macro Definition
//=============================================================================
typedef MMP_UINT (*frameCount2TimeStamp)(MMP_UINT framecount);

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct TASK_VIDEO_ENCODER_HANDLE_TAG
{
    MPS_ELEMENT   *ptElement;
    MPS_STATE     mpsState;
    STREAM_HANDLE tVideoCaptureHandle;
    STREAM_HANDLE tStreamMuxHandle;
    PAL_THREAD    ptVideoEncoderThread;
    MPS_CMD_OBJ   tReadCmdObj;
    MPS_CMD_OBJ   tWriteCmdObj;

    MMP_UINT width;
    MMP_UINT height;
} TASK_VIDEO_ENCODER_HANDLE;

typedef struct AVC_ENCODER_FLOW_TAG
{
    MMP_UINT                 baseTimeStamp;
    MMP_UINT                 currTimeStamp;
    MMP_UINT                 captureFrameCount;
    MMP_UINT                 encodedFrameCount;
    MMP_UINT                 encodedTimeStamp;
    MMP_CAP_FRAMERATE        frameRate;
    MMP_CAP_FRAMERATE        captureFrameRate;
    MMP_CAP_FRAMERATE        detectFrameRate;
    FRAME_RATE_SETTING_STATE frameRateState;
    frameCount2TimeStamp     pfToTimeStamp;
} AVC_ENCODER_FLOW;

typedef struct CAPTURE_RESULT_TAG
{
    MMP_UINT resIdx;
    MMP_UINT frameCount;
    MMP_UINT timeStamp;
    MMP_UINT frameIndex;
} CAPTURE_RESULT;

typedef struct CAPTURE_RESULT_QUEUE_TAG
{
    CAPTURE_RESULT entry[CAPTURE_RESULT_QUEUE_SIZE];
    MMP_UINT       wIdx;
    MMP_UINT       rIdx;
} CAPTURE_RESULT_QUEUE;

typedef struct ENCODED_RESULT_TAG
{
    MMP_UINT  frameCount;
    MMP_UINT  timeStamp;
    MMP_UINT8 *pData;
    MMP_UINT  dataSize;
    MMP_UINT  InstanceNum;
} ENCODED_RESULT;

typedef struct ENCODED_RESULT_QUEUE_TAG
{
    ENCODED_RESULT entry[ENCODED_RESULT_QUEUE_SIZE];
    MMP_UINT       wIdx;
    MMP_UINT       rIdx;
} ENCODED_RESULT_QUEUE;

typedef struct ISP_TRANSFORM_PARAMETER_TAG
{
    MMP_UINT16 inWidth;
    MMP_UINT16 inHeight;
    MMP_UINT32 inAddrY[3];
    MMP_UINT32 inAddrUV[3];
    MMP_UINT16 outWidth;
    MMP_UINT16 outHeight;
    MMP_UINT32 outAddrY[3];
    MMP_UINT32 outAddrUV[3];
} ISP_TRANSFORM_PARAMETER;

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_EVENT elementThreadToMpsThread;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static TASK_VIDEO_ENCODER_HANDLE gtVideoEncoderHandle = {0};
static AVC_ENCODER               *gptAVCEncoder[ENCODE_STREAM_NUM];
static AVC_ENCODER_FLOW          gtAVCEncoderFlow;

static CAPTURE_RESULT_QUEUE      gtCaptureResultQ;
static CAPTURE_RESULT_QUEUE      gtIspResultQ;
static ENCODED_RESULT_QUEUE      gtEncodedResultQ;

static CAPTURE_DEVICE            gtDevice_ID    = CAPTURE_DEV_UNKNOW;
static CAPTURE_DEVICE            gtPreDevice_ID = CAPTURE_DEV_UNKNOW;
static MMP_BOOL                  gtHDMIRxStable;

static ISP_TRANSFORM_PARAMETER   gtIspTransformParm[ISP_TRANSFORM_NUM];
static CAPTURE_RESULT            gtIspFireEntry;

#ifdef _DEBUG_ISP_RUN_TIME
static MMP_UINT32                initClockIsp = 0;
static MMP_UINT32                durIsp       = 0;
#endif

static MMP_UINT32                gEncodeIdx   = 0;
static MMP_BOOL                  gbEncodeFire = MMP_FALSE;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_VIDEO_ENCODER_Init(
    void);

static void
_VIDEO_ENCODER_DoPlay(
    void);

static void
_VIDEO_ENCODER_DoStop(
    void);

static void
_VIDEO_ENCODER_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data);

static void *
_VIDEO_ENCODER_ThreadFunction(
    void *arg);

MMP_RESULT
_CaptureAndIsp_Init(
    void);

MMP_RESULT
_Capture_Fire(
    void);

MMP_RESULT
_CaptureAndIsp_Terminate(
    void);

static void
_DoAVSync(
    void);

static MMP_INLINE void
_CaptureResultQ_Reset(
    void)
{
    gtCaptureResultQ.wIdx = gtCaptureResultQ.rIdx = 0;
}

static MMP_INLINE MMP_RESULT
_CaptureResultQ_Dequeue(
    CAPTURE_RESULT *ptEntry)
{
    if (gtCaptureResultQ.wIdx == gtCaptureResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;
    ptEntry->resIdx     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].resIdx;
    ptEntry->frameCount = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].frameCount;
    ptEntry->timeStamp  = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].timeStamp;
    ptEntry->frameIndex = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].frameIndex;
    gtCaptureResultQ.rIdx++;
    if (gtCaptureResultQ.rIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtCaptureResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_CaptureResultQ_Enqueue(
    CAPTURE_RESULT *ptEntry)
{
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].resIdx     = ptEntry->resIdx;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].frameCount = ptEntry->frameCount;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].timeStamp  = ptEntry->timeStamp;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].frameIndex = ptEntry->frameIndex;

    gtCaptureResultQ.wIdx++;
    if (gtCaptureResultQ.wIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtCaptureResultQ.wIdx = 0;
}

static MMP_INLINE void
_IspResultQ_Reset(
    void)
{
    gtIspResultQ.wIdx = gtIspResultQ.rIdx = 0;
}

static MMP_INLINE MMP_RESULT
_IspResultQ_Dequeue(
    CAPTURE_RESULT *ptEntry)
{
    if (gtIspResultQ.wIdx == gtIspResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;

    ptEntry->resIdx     = gtIspResultQ.entry[gtIspResultQ.rIdx].resIdx;
    ptEntry->frameCount = gtIspResultQ.entry[gtIspResultQ.rIdx].frameCount;
    ptEntry->timeStamp  = gtIspResultQ.entry[gtIspResultQ.rIdx].timeStamp;
    ptEntry->frameIndex = gtIspResultQ.entry[gtIspResultQ.rIdx].frameIndex;
    gtIspResultQ.rIdx++;
    if (gtIspResultQ.rIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtIspResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_IspResultQ_Enqueue(
    CAPTURE_RESULT *ptEntry)
{
    gtIspResultQ.entry[gtIspResultQ.wIdx].resIdx     = ptEntry->resIdx;
    gtIspResultQ.entry[gtIspResultQ.wIdx].frameCount = ptEntry->frameCount;
    gtIspResultQ.entry[gtIspResultQ.wIdx].timeStamp  = ptEntry->timeStamp;
    gtIspResultQ.entry[gtIspResultQ.wIdx].frameIndex = ptEntry->frameIndex;

    gtIspResultQ.wIdx++;
    if (gtIspResultQ.wIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtIspResultQ.wIdx = 0;
}

static MMP_INLINE void
_EncodedResultQ_Reset(
    void)
{
    gtEncodedResultQ.wIdx = gtEncodedResultQ.rIdx = 0;
}

static MMP_INLINE MMP_RESULT
_EncodedResultQ_Dequeue(
    ENCODED_RESULT *ptEntry)
{
    MMP_INT result;

    if (gtEncodedResultQ.wIdx == gtEncodedResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;

    ptEntry->frameCount  = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].frameCount;
    ptEntry->timeStamp   = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].timeStamp;
    ptEntry->pData       = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].pData;
    ptEntry->dataSize    = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].dataSize;
    ptEntry->InstanceNum = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].InstanceNum;
    gtEncodedResultQ.rIdx++;
    if (gtEncodedResultQ.rIdx >= ENCODED_RESULT_QUEUE_SIZE)
        gtEncodedResultQ.rIdx = 0;

    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_EncodedResultQ_Enqueue(
    ENCODED_RESULT *ptEntry)
{
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].frameCount  = ptEntry->frameCount;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].timeStamp   = ptEntry->timeStamp;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].pData       = ptEntry->pData;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].dataSize    = ptEntry->dataSize;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].InstanceNum = ptEntry->InstanceNum;
    gtEncodedResultQ.wIdx++;
    if (gtEncodedResultQ.wIdx >= ENCODED_RESULT_QUEUE_SIZE)
        gtEncodedResultQ.wIdx = 0;
}

static void
_WaitAllQueue_Empty(
    void)
{
    CAPTURE_RESULT tEntry        = {0};
    ENCODED_RESULT tEncodedEntry = {0};
    MMP_UINT16     timeOut       = 0;

    while (QUEUE_NOT_EMPTY == _CaptureResultQ_Dequeue(&tEntry))
    {
        PalSleep(30);
        if (++timeOut > 100)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "wait QUEUE_NOT_EMPTY timeout %s() #%d\n", __FUNCTION__, __LINE__);
            return;
        }
    }

    while (QUEUE_NOT_EMPTY == _IspResultQ_Dequeue(&tEntry) && !gbEncodeFire)
    {
        PalSleep(30);
        if (++timeOut > 100)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "wait QUEUE_NOT_EMPTY timeout %s() #%d\n", __FUNCTION__, __LINE__);
            return;
        }
    }
}

static void
_DoIsp(
    void)
{
    CAPTURE_RESULT              tEntry  = {0};
    static MMP_ISP_SINGLE_SHARE ispctxt = {0};
    MMP_UINT16                  resIdx;
    MMP_UINT16                  inBufferIdx;
    MMP_UINT16                  outBufferIdx;

    if (QUEUE_NOT_EMPTY == _CaptureResultQ_Dequeue(&tEntry))
    {
#ifdef _DEBUG_ISP_RUN_TIME
        if (tEntry.resIdx == 0)
            initClockIsp = mmpTimerReadCounter(TIMER_NUM) + 1;
#endif

        resIdx              = tEntry.resIdx;
        inBufferIdx         = tEntry.frameIndex;
        outBufferIdx        = tEntry.resIdx;

        //Signal Process Input Parameter
        ispctxt.In_AddrY    = gtIspTransformParm[resIdx].inAddrY[inBufferIdx];
        ispctxt.In_AddrUV   = gtIspTransformParm[resIdx].inAddrUV[inBufferIdx];
        ispctxt.In_Width    = gtIspTransformParm[resIdx].inWidth;
        ispctxt.In_Height   = gtIspTransformParm[resIdx].inHeight;
        ispctxt.In_PitchY   = CAP_MEM_BUF_PITCH;
        ispctxt.In_PitchUV  = CAP_MEM_BUF_PITCH;
        ispctxt.In_Format   = MMP_ISP_IN_NV12;

        //Signal Process Output Parameter
        ispctxt.Out_AddrY   = gtIspTransformParm[resIdx].outAddrY[outBufferIdx];
        ispctxt.Out_AddrU   = gtIspTransformParm[resIdx].outAddrUV[outBufferIdx];
        ispctxt.Out_Width   = gtIspTransformParm[resIdx].outWidth;
        ispctxt.Out_Height  = gtIspTransformParm[resIdx].outHeight;
        ispctxt.Out_PitchY  = 2048;
        ispctxt.Out_PitchUV = 2048;
        ispctxt.Out_Format  = MMP_ISP_OUT_NV12;

        mmpIspEnable(MMP_ISP_INTERRUPT);
        mmpIspEnable(MMP_ISP_REMAP_ADDRESS);

        mmpIspSingleProcess(&ispctxt);

        memcpy(&gtIspFireEntry, &tEntry, sizeof(CAPTURE_RESULT));
    }
}

static void
_DoEncode(
    void)
{
    CAPTURE_RESULT tEntry = {0};

    if (QUEUE_NOT_EMPTY == _IspResultQ_Dequeue(&tEntry) && !gbEncodeFire)
    {
        gEncodeIdx                                 = tEntry.resIdx;
        //gptAVCEncoder[gEncodeIdx]->streamBufSelect = (gptAVCEncoder[gEncodeIdx]->streamBufSelect + 1)
        //                                                % (gptAVCEncoder[gEncodeIdx]->streamBufCount);
        gtAVCEncoderFlow.encodedFrameCount         = tEntry.frameCount;
        gtAVCEncoderFlow.encodedTimeStamp          = tEntry.timeStamp;
        gptAVCEncoder[gEncodeIdx]->sourceBufSelect = tEntry.resIdx;
        gbEncodeFire                               = MMP_TRUE;
        mmpAVCEncodeFire(gptAVCEncoder[gEncodeIdx]);
    }
}

static void
cap_isr(
    void *arg);

static void
isp_isr(
    void *arg);

static void
encoder_isr(
    void *arg);

static MMP_INLINE MMP_UINT
frameRate25Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return frameCount * 40;
}

static MMP_INLINE MMP_UINT
frameRate29_97Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    static MMP_UINT timeStamp[30] = {
        0,  33,  66,
        100, 133, 166,
        200, 233, 266,
        300, 333, 366,
        400, 433, 466,
        500, 533, 566,
        600, 633, 666,
        700, 733, 766,
        800, 833, 866,
        900, 933, 967
    };
    return (frameCount / 30) * 1001 + timeStamp[(frameCount % 30)];
}

static MMP_INLINE MMP_UINT
frameRate30Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return (frameCount / 3) * 100 + (frameCount % 3) * 33;
}

static frameCount2TimeStamp _frameCount2TimeStamp_TABLE[7] =
{
    MMP_NULL,
    frameRate25Hz_frameCount2TimeStamp,
    frameRate25Hz_frameCount2TimeStamp,
    frameRate30Hz_frameCount2TimeStamp,
    frameRate30Hz_frameCount2TimeStamp,
    frameRate29_97Hz_frameCount2TimeStamp,
    frameRate29_97Hz_frameCount2TimeStamp,
};

static MMP_INLINE frameCount2TimeStamp
_VIDEO_ENCODER_GetTimeStampConverter(
    MMP_CAP_FRAMERATE frameRate)
{
    frameCount2TimeStamp pf = MMP_NULL;

    if (frameRate <= 6)
        pf = _frameCount2TimeStamp_TABLE[frameRate];

    return pf;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
taskVideoEncoder_Init(
    MMP_UINT32 arg)
{
    if (arg) { } // avoid compiler warning

    if (gtVideoEncoderHandle.mpsState != MPS_STATE_ZERO)
        return;

    //ithIntrInit();
    _VIDEO_ENCODER_Init();
    mmpCapMemoryInitialize();

    mmpTimerCtrlEnable(TIMER_NUM, MMP_TIMER_UPCOUNT);

    gtVideoEncoderHandle.mpsState  = MPS_STATE_STOP;
    gtVideoEncoderHandle.ptElement = mpsSys_GetElement(MPS_VIDEO_ENCODER);

    //gtVideoEncoderHandle.tVideoCaptureHandle.queueId =
    //    gtVideoEncoderHandle.ptElement->ptSrcList->ptConnector->queueId;
    //gtVideoEncoderHandle.tVideoCaptureHandle.ptQueueHandle =
    //    queueMgr_GetCtrlHandle(gtVideoEncoderHandle.tVideoCaptureHandle.queueId);

    gtVideoEncoderHandle.tStreamMuxHandle.queueId       =
        gtVideoEncoderHandle.ptElement->ptDestList->ptConnector->queueId;
    gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle =
        queueMgr_GetCtrlHandle(gtVideoEncoderHandle.tStreamMuxHandle.queueId);

    if (MMP_NULL == gtVideoEncoderHandle.ptVideoEncoderThread)
    {
        gtVideoEncoderHandle.ptVideoEncoderThread = PalCreateThread(PAL_THREAD_VIDEO_ENCODER,
                                                                    _VIDEO_ENCODER_ThreadFunction,
                                                                    MMP_NULL, 0, 0);
    }
}

void
taskVideoEncoder_Terminate(
    MMP_UINT32 arg)
{
    if (arg) { } // avoid compiler warning

    _CaptureAndIsp_Terminate();

    if (gtVideoEncoderHandle.ptVideoEncoderThread != MMP_NULL)
        PalDestroyThread(gtVideoEncoderHandle.ptVideoEncoderThread);

    PalMemset(&gtVideoEncoderHandle, 0x0, sizeof(TASK_VIDEO_ENCODER_HANDLE));
}

QUEUE_MGR_ERROR_CODE
taskVideoEncoder_SetFree(
    QUEUE_ID queueId,
    void **pptSample)
{
    return queueMgr_SetFree(queueId, pptSample);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_VIDEO_ENCODER_Init(
    void)
{
    MMP_UINT16 i, j;

    if (mmpAVCEncodeInit() != 0)
        dbg_msg(DBG_MSG_TYPE_INFO, "[264 TEST] mmpAVCEncodeInit Fail\n");

    for (i = 0; i < ENCODE_STREAM_NUM; i++)
    {
        if (MMP_NULL == gptAVCEncoder[i])
            gptAVCEncoder[i] = (AVC_ENCODER *)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(AVC_ENCODER));

        if (MMP_NULL != gptAVCEncoder[i])
            PalMemset(gptAVCEncoder[i], 0, sizeof(AVC_ENCODER));
    }

    //Instance 0
    gptAVCEncoder[0]->frameWidth               = 1280;
    gptAVCEncoder[0]->frameHeight              = 720;
    gptAVCEncoder[0]->frameCropTop             = 0;
    gptAVCEncoder[0]->frameCropBottom          = 0;
    gptAVCEncoder[0]->frameCropLeft            = 0;
    gptAVCEncoder[0]->frameCropRight           = 0;
    gptAVCEncoder[0]->frameRate                = AVC_FRAME_RATE_29_97HZ;
    gptAVCEncoder[0]->gopSize                  = 30;
    gptAVCEncoder[0]->bitRate                  = 8000; // VBR
    gptAVCEncoder[0]->enableAutoSkip           = 0;
    gptAVCEncoder[0]->initialDelay             = 0;
    gptAVCEncoder[0]->chromaQpOffset           = 0x1E; //-12~+12 need to use 5 bits 2's complemment, default -2
    gptAVCEncoder[0]->constrainedIntraPredFlag = 0;
    gptAVCEncoder[0]->disableDeblk             = 0;
    gptAVCEncoder[0]->deblkFilterOffsetAlpha   = 0;
    gptAVCEncoder[0]->deblkFilterOffsetBeta    = 0;
    gptAVCEncoder[0]->vbvBufferSize            = 0;
    gptAVCEncoder[0]->intraRefresh             = 0;
    gptAVCEncoder[0]->rcIntraQp                = 20;
    gptAVCEncoder[0]->userQpMax                = 51;
    gptAVCEncoder[0]->userGamma                = 24576; // default 0.75*32768
    gptAVCEncoder[0]->RcIntervalMode           = 0;     // 0:MB, 1:frame, 2:slice, 3:MB_NUM
    gptAVCEncoder[0]->MbInterval               = 0;
    gptAVCEncoder[0]->MEUseZeroPmv             = 0;     // 0:PMV, 1:ZMV
    gptAVCEncoder[0]->MESearchRange            = 1;     // 0:(128, 64), 1:(64:32), 2:(32:16), 3:(16,16)
    gptAVCEncoder[0]->IntraCostWeight          = 100;
    gptAVCEncoder[0]->PicQS                    = 25;
    gptAVCEncoder[0]->forceIPicture            = 0;

    gptAVCEncoder[0]->streamBufSelect          = 0;
    gptAVCEncoder[0]->sourceBufSelect          = 0;
    gptAVCEncoder[0]->framecount               = 0;

    //Instance 1
    gptAVCEncoder[1]->frameWidth               = 1280;
    gptAVCEncoder[1]->frameHeight              = 720;
    gptAVCEncoder[1]->frameCropTop             = 0;
    gptAVCEncoder[1]->frameCropBottom          = 0;
    gptAVCEncoder[1]->frameCropLeft            = 0;
    gptAVCEncoder[1]->frameCropRight           = 0;
    gptAVCEncoder[1]->frameRate                = AVC_FRAME_RATE_29_97HZ;
    gptAVCEncoder[1]->gopSize                  = 30;
    gptAVCEncoder[1]->bitRate                  = 8000; // VBR
    gptAVCEncoder[1]->enableAutoSkip           = 0;
    gptAVCEncoder[1]->initialDelay             = 0;
    gptAVCEncoder[1]->chromaQpOffset           = 0x1E; //-12~+12 need to use 5 bits 2's complemment, default -2
    gptAVCEncoder[1]->constrainedIntraPredFlag = 0;
    gptAVCEncoder[1]->disableDeblk             = 0;
    gptAVCEncoder[1]->deblkFilterOffsetAlpha   = 0;
    gptAVCEncoder[1]->deblkFilterOffsetBeta    = 0;
    gptAVCEncoder[1]->vbvBufferSize            = 0;
    gptAVCEncoder[1]->intraRefresh             = 0;
    gptAVCEncoder[1]->rcIntraQp                = 20;
    gptAVCEncoder[1]->userQpMax                = 51;
    gptAVCEncoder[1]->userGamma                = 24576; // default 0.75*32768
    gptAVCEncoder[1]->RcIntervalMode           = 0;     // 0:MB, 1:frame, 2:slice, 3:MB_NUM
    gptAVCEncoder[1]->MbInterval               = 0;
    gptAVCEncoder[1]->MEUseZeroPmv             = 0;     // 0:PMV, 1:ZMV
    gptAVCEncoder[1]->MESearchRange            = 1;     // 0:(128, 64), 1:(64:32), 2:(32:16), 3:(16,16)
    gptAVCEncoder[1]->IntraCostWeight          = 100;
    gptAVCEncoder[1]->PicQS                    = 25;
    gptAVCEncoder[1]->forceIPicture            = 0;

    gptAVCEncoder[1]->streamBufSelect          = 0;
    gptAVCEncoder[1]->sourceBufSelect          = 0;
    gptAVCEncoder[1]->framecount               = 0;

    //Instance 2
    gptAVCEncoder[2]->frameWidth               = 1280;
    gptAVCEncoder[2]->frameHeight              = 720;
    gptAVCEncoder[2]->frameCropTop             = 0;
    gptAVCEncoder[2]->frameCropBottom          = 0;
    gptAVCEncoder[2]->frameCropLeft            = 0;
    gptAVCEncoder[2]->frameCropRight           = 0;
    gptAVCEncoder[2]->frameRate                = AVC_FRAME_RATE_29_97HZ;
    gptAVCEncoder[2]->gopSize                  = 30;
    gptAVCEncoder[2]->bitRate                  = 8000; // VBR
    gptAVCEncoder[2]->enableAutoSkip           = 0;
    gptAVCEncoder[2]->initialDelay             = 0;
    gptAVCEncoder[2]->chromaQpOffset           = 0x1E; //-12~+12 need to use 5 bits 2's complemment, default -2
    gptAVCEncoder[2]->constrainedIntraPredFlag = 0;
    gptAVCEncoder[2]->disableDeblk             = 0;
    gptAVCEncoder[2]->deblkFilterOffsetAlpha   = 0;
    gptAVCEncoder[2]->deblkFilterOffsetBeta    = 0;
    gptAVCEncoder[2]->vbvBufferSize            = 0;
    gptAVCEncoder[2]->intraRefresh             = 0;
    gptAVCEncoder[2]->rcIntraQp                = 20;
    gptAVCEncoder[2]->userQpMax                = 51;
    gptAVCEncoder[2]->userGamma                = 24576; // default 0.75*32768
    gptAVCEncoder[2]->RcIntervalMode           = 0;     // 0:MB, 1:frame, 2:slice, 3:MB_NUM
    gptAVCEncoder[2]->MbInterval               = 0;
    gptAVCEncoder[2]->MEUseZeroPmv             = 0;     // 0:PMV, 1:ZMV
    gptAVCEncoder[2]->MESearchRange            = 1;     // 0:(128, 64), 1:(64:32), 2:(32:16), 3:(16,16)
    gptAVCEncoder[2]->IntraCostWeight          = 100;
    gptAVCEncoder[2]->PicQS                    = 25;
    gptAVCEncoder[2]->forceIPicture            = 0;

    gptAVCEncoder[2]->streamBufSelect          = 0;
    gptAVCEncoder[2]->sourceBufSelect          = 0;
    gptAVCEncoder[2]->framecount               = 0;

    _CaptureResultQ_Reset();
    _IspResultQ_Reset();
    _EncodedResultQ_Reset();
    //if (mmpAVCEncodeOpen(gptAVCEncoder) != 0)
    //    dbg_msg(DBG_MSG_TYPE_INFO, "[264 TEST] mmpAVCEncodeOpen Fail\n");
}

static void
_VIDEO_ENCODER_DoPlay(
    void)
{
    static CAPTURE_STATE state           = CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR;
    static PAL_CLOCK_T   captureStartTime;
    static PAL_CLOCK_T   queryStableTime;
    static MMP_UINT16    reTryCounter    = 0;
    MMP_BOOL             doBreak         = MMP_FALSE;
    MMP_BOOL             bSignalStable   = MMP_TRUE;
    MMP_UINT16           captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);
    MMP_UINT16           i;

    do
    {
        //dbg_msg(DBG_MSG_TYPE_INFO, "state(%d)\n", state);
        switch (state)
        {
        case CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR:
            if (captureErrState & 0x0F00)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "[Debug] Capture Error State Code = 0x%x\n", captureErrState);
                state = CAPTURE_STATE_RESET_CAPTURE_AND_ISP;
            }
            else
            {
                state = CAPTURE_STATE_POLLING_DEVICE_STATUS;
            }
            break;

        case CAPTURE_STATE_POLLING_DEVICE_STATUS:
            if (!mmpCapIsFire() || PalGetDuration(queryStableTime) >= 500)
            {
                gtHDMIRxStable  = bSignalStable = mmpCapDeviceIsSignalStable();
                queryStableTime = PalGetClock();
            }

            if (bSignalStable && !mmpCapIsFire())
            {
                state = CAPTURE_STATE_FIRE_CAPTURE;
            }
            else
            {
                state = CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR;
                if (!bSignalStable)
                {
                    avSyncSetVideoStable(MMP_FALSE);
                }
                doBreak = MMP_TRUE;
            }
            break;

        case CAPTURE_STATE_FIRE_CAPTURE:
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
                if (mmpAVCEncodeOpen(gptAVCEncoder[i]) != 0)
                    dbg_msg(DBG_MSG_TYPE_INFO, "[264 TEST] mmpAVCEncodeOpen Fail\n");
            gbEncodeFire     = MMP_FALSE;
            _Capture_Fire();
            dbg_msg(DBG_MSG_TYPE_INFO, "Fire Capture and ISP Engine Device(%d)!\n", mmpCapGetCaptureDevice());
            state            = CAPTURE_STATE_WAIT_CAPTURE_SYNC;
            captureStartTime = PalGetClock();
            doBreak          = MMP_TRUE;
            break;

        case CAPTURE_STATE_WAIT_CAPTURE_SYNC:
            if (mmpCapIsFire() && !(captureErrState & 0x000F))
            {
                state = CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR;
            }
            else
            {
                if (PalGetDuration(captureStartTime) > 200)
                    state = CAPTURE_STATE_RESET_CAPTURE_AND_ISP;
            }
            doBreak = MMP_TRUE;
            break;

        case CAPTURE_STATE_RETRY_CAPTURE_POLARITY:
            if (reTryCounter == 0)
                mmpCapSetPolarity(1, 0);
            else if (reTryCounter == 1)
                mmpCapSetPolarity(0, 0);
            else if (reTryCounter == 2)
                mmpCapSetPolarity(1, 0);

            reTryCounter++;
            state            = CAPTURE_STATE_WAIT_CAPTURE_SYNC;
            captureStartTime = PalGetClock();
            doBreak          = MMP_TRUE;
            break;

        case CAPTURE_STATE_RESET_CAPTURE_AND_ISP:
            _WaitAllQueue_Empty();
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
            {
                MMP_RESULT result;
                if (MMP_SUCCESS != mmpAVCEncodeClose(gptAVCEncoder[i]))
                    dbg_msg(DBG_MSG_TYPE_INFO, "mmpAVCEncodeClose time out\n");
            }

            mmpCapResetEngine();
            mmpCapEnableInterrupt(MMP_TRUE);
            dbg_msg(DBG_MSG_TYPE_INFO, "Reset Capture and ISP Engine Device(%d)!\n", mmpCapGetCaptureDevice());
            state = CAPTURE_STATE_POLLING_DEVICE_STATUS;
            break;
        }
    } while (!doBreak);

    if (gtDevice_ID != CAPTURE_DEV_HDMIRX)
        _DoAVSync();

    {
        static MMP_BOOL bInit;
        if (!bInit)
        {
#if 0
            if ((fp = PalFileOpen("c:/test.264", PAL_FILE_WB, MMP_NULL)) == NULL)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "Open write plane date fail..\n");
                exit(-1);
            }
#endif

            for (i = 0; i < ENCODE_STREAM_NUM; i++)
                gptAVCEncoder[i]->sourceBufSelect = 0;

            bInit = MMP_TRUE;
        }
    }

    {
        ENCODED_RESULT tEntry = {0};

        if (FRAME_RATE_SETTING_STATE_DETECTED == gtAVCEncoderFlow.frameRateState)
        {
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
                mmpAVCEncodeCreateHdr(gptAVCEncoder[i]);
            gtAVCEncoderFlow.frameRateState = FRAME_RATE_SETTING_STATE_SET;
        }

        while (QUEUE_NOT_EMPTY == _EncodedResultQ_Dequeue(&tEntry))
        {
            QUEUE_MGR_ERROR_CODE errorCode      = QUEUE_NO_ERROR;
            VIDEO_SAMPLE         *ptVideoSample = MMP_NULL;

            errorCode = (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfGetFree(
                gtVideoEncoderHandle.tStreamMuxHandle.queueId,
                (void **) &ptVideoSample);

            if (QUEUE_NO_ERROR == errorCode)
            {
                or32_invalidate_cache(tEntry.pData, tEntry.dataSize);
                ptVideoSample->pData       = tEntry.pData;
                ptVideoSample->dataSize    = tEntry.dataSize;
                ptVideoSample->timeStamp   = tEntry.timeStamp;
                ptVideoSample->InstanceNum = tEntry.InstanceNum;
#if 0
                if (gptAVCEncoder->framecount < 1000 && fp)
                {
                    or32_invalidate_cache(pData, dataSize);
                    PalFileWrite(pData, dataSize, 1, fp, MMP_NULL);
                    PalFileFlush(fp, MMP_NULL);
                }
                else
                {
                    if (fp)
                    {
                        dbg_msg(DBG_MSG_TYPE_INFO, "=========================FUCK====================\n");
                        PalFileClose(fp, MMP_NULL);
                        fp = MMP_NULL;
                    }
                }
#endif

                (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfSetReady(
                    gtVideoEncoderHandle.tStreamMuxHandle.queueId,
                    (void **) &ptVideoSample);
            }
        }
    }
}

static void
_VIDEO_ENCODER_DoStop(
    void)
{
    MMP_UINT16 i;

    avSyncSetVideoStable(MMP_FALSE);
    gtHDMIRxStable = MMP_FALSE;
    _CaptureAndIsp_Terminate();

    mmpAVCEncodeDisableInterrupt();
    for (i = 0; i < ENCODE_STREAM_NUM; i++)
        mmpAVCEncodeClose(gptAVCEncoder[i]);
    PalMemset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    avSyncSetCurrentTime(0);
    _CaptureResultQ_Reset();
    _IspResultQ_Reset();
    _EncodedResultQ_Reset();
}

static void
_VIDEO_ENCODER_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data)
{
    MPS_NOTIFY_REASON_DATA *ptNotifyData = MMP_NULL;

    gtVideoEncoderHandle.tWriteCmdObj.cmd = MPS_COMMAND_EVENT_NOTIFY;
    if (reason)
    {
        ptNotifyData                                = (MPS_NOTIFY_REASON_DATA *)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                                             sizeof(MPS_NOTIFY_REASON_DATA));
        ptNotifyData->notifyReason                  = reason;
        ptNotifyData->bDatatNeedFree                = MMP_FALSE;
        ptNotifyData->data                          = data;
        gtVideoEncoderHandle.tWriteCmdObj.extraData = (void *)ptNotifyData;
        mpsCmdQ_SendCommand(gtVideoEncoderHandle.ptElement->specialEventId, &gtVideoEncoderHandle.tWriteCmdObj);
    }
    else
        PalSetEvent(elementThreadToMpsThread);
}

static void *
_VIDEO_ENCODER_ThreadFunction(
    void *arg)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;

    if (arg) { } // avoid compiler warning

    for (;;)
    {
        if (MPS_STATE_RUN == gtVideoEncoderHandle.mpsState)
            _VIDEO_ENCODER_DoPlay();

        mpsCmdQ_ReceiveCommand(gtVideoEncoderHandle.ptElement->cmdQueueId, &gtVideoEncoderHandle.tReadCmdObj);

        if ((gtVideoEncoderHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK) != MPS_COMMAND_NULL)
        {
            switch (gtVideoEncoderHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK)
            {
            case MPS_COMMAND_PLAY:
                if (MPS_STATE_RUN != gtVideoEncoderHandle.mpsState)
                {
                    _CaptureAndIsp_Terminate();
                    _CaptureAndIsp_Init();
                    gtPreDevice_ID = gtDevice_ID;
                    dbg_msg(DBG_MSG_TYPE_INFO, "change device ID(%d)\n", gtDevice_ID);
                    mmpAVCEncodeEnableInterrupt(encoder_isr);
                }
                gtVideoEncoderHandle.mpsState = MPS_STATE_RUN;
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP:
                gtVideoEncoderHandle.mpsState = MPS_STATE_STOP;
                _VIDEO_ENCODER_DoStop();
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_TERMINATE:
                gtVideoEncoderHandle.mpsState = MPS_STATE_ZERO;
                goto end;

            //case MPS_COMMAND_SET_ENCODE_PARAMETER:
            //    {
            //        ENCODE_PARAMETER* para;
            //        ptData  = (MPS_PROPERITY_DATA*)gtVideoEncoderHandle.tReadCmdObj.extraData;
            //        para    = (ENCODE_PARAMETER*)ptData->data;
            //
            //        gptAVCEncoder[para->InstanceNum]->frameWidth   = para->width;
            //        gptAVCEncoder[para->InstanceNum]->frameHeight  = para->height;
            //        gptAVCEncoder[para->InstanceNum]->bitRate      = para->bitRate;
            //
            //        gtDevice_ID = para->captureDevice;
            //        dbg_msg(DBG_MSG_TYPE_INFO, "device(%d) w(%d) h(%d) b(%d)\n", para->captureDevice, para->width, para->height, para->bitRate);
            //        _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
            //    }
            //    break;

            case MPS_COMMAND_GET_PROPERTY:
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_SET_PROPERTY:
                ptData = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                switch (ptData->properityId)
                {
                case MPS_PROPERITY_SET_ENCODE_PARAMETER:
                    {
                        ENCODE_PARAMETER *para;
                        ptData                                        = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                        para                                          = (ENCODE_PARAMETER *)ptData->data;

                        gptAVCEncoder[para->InstanceNum]->frameWidth  = para->width;
                        gptAVCEncoder[para->InstanceNum]->frameHeight = para->height;
                        gptAVCEncoder[para->InstanceNum]->bitRate     = para->bitRate;

                        gtDevice_ID                                   = para->captureDevice;
                        dbg_msg(DBG_MSG_TYPE_INFO, "device(%d) w(%d) h(%d) b(%d)\n", para->captureDevice, para->width, para->height, para->bitRate);
                        // _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                    }
                    break;

                default:
                    break;
                }
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            default:
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;
            }
        }

        PalSleep(30);
    }

end:
    _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
    return MMP_NULL;
}

MMP_RESULT
_CaptureAndIsp_Init(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    //Select Capture Device
    mmpCapSetCaptureDevice(gtDevice_ID);

    //Device Init
    mmpCapDeviceInitialize();

    //Capture Init
    mmpCapInitialize();

    //ISP Init
    mmpIspInitialize();

    //Register IRQ
    mmpIspRegisterIRQ(isp_isr);

    //Register IRQ
    mmpCapRegisterIRQ(cap_isr);

    return result;
}

MMP_RESULT
_Capture_Fire(
    void)
{
    MMP_RESULT    result   = MMP_SUCCESS;
    MMP_CAP_SHARE *capctxt = MMP_NULL;
    MMP_UINT16    i, j;
    MMP_UINT      oldTimeStamp;

    //Capture setting
    capctxt = calloc(1, sizeof(MMP_CAP_SHARE));
    if (capctxt == MMP_NULL)
        dbg_msg(DBG_MSG_TYPE_INFO, "capctxt alloc Fail ! %s [#%d]\n", __FILE__, __LINE__);

    //Enable function
    mmpCapFunEnable(MMP_CAP_INTERRUPT);

    //get device information
    mmpCapGetDeviceInfo(capctxt);

    //set capture parameter
    mmpCapParameterSetting(capctxt);

    oldTimeStamp                   = gtAVCEncoderFlow.currTimeStamp;
    PalMemset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    gtAVCEncoderFlow.baseTimeStamp = oldTimeStamp;
    gtAVCEncoderFlow.frameRate     = capctxt->FrameRate;
    gtAVCEncoderFlow.pfToTimeStamp = _VIDEO_ENCODER_GetTimeStampConverter(gtAVCEncoderFlow.frameRate);

    for (i = 0; i < ISP_TRANSFORM_NUM; i++)
    {
        gtIspTransformParm[i].inWidth  = capctxt->OutWidth;
        gtIspTransformParm[i].inHeight = capctxt->OutHeight;
        for (j = 0; j < 3; j++)
        {
            gtIspTransformParm[i].inAddrY[j]  = capctxt->OutAddrY[j];
            gtIspTransformParm[i].inAddrUV[j] = capctxt->OutAddrUV[j];
        }
    }

    for (i = 0; i < ISP_TRANSFORM_NUM; i++)
    {
        for (j = 0; j < gptAVCEncoder[i]->sourceBufCount; j++)
        {
            gtIspTransformParm[i].outAddrY[j]  = (MMP_UINT32) gptAVCEncoder[i]->pSourceBufAdrY[j];
            gtIspTransformParm[i].outAddrUV[j] = (MMP_UINT32) gptAVCEncoder[i]->pSourceBufAdrU[j];
        }

        gtIspTransformParm[i].outWidth  = gptAVCEncoder[i]->frameWidth;
        gtIspTransformParm[i].outHeight = gptAVCEncoder[i]->frameHeight;
    }

    mmpCapSetSkipMode(MMP_CAPTURE_NO_DROP);

    //Capture Fire
    mmpCapFire();

    if (capctxt)
        free(capctxt);

    return result;
}

MMP_RESULT
_CaptureAndIsp_Terminate(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    mmpCapDeviceTerminate();
    mmpIspTerminate();
    mmpCapTerminate();

    return result;
}

static void
cap_isr(
    void *arg)
{
    CAPTURE_RESULT    tEntry          = {0};
    MMP_BOOL          bIsIdle         = MMP_FALSE;
    MMP_UINT32        dur             = 0;
    static MMP_UINT32 initClock       = 0;
    static MMP_UINT32 totalDur        = 0;
    MMP_UINT16        captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);
    MMP_UINT16        i;

    //disable Interrupt
    mmpCapEnableInterrupt(MMP_FALSE);

    //Error frame not encode
    if ((captureErrState & 0x0F00) && gtAVCEncoderFlow.captureFrameCount == 0)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "---Error frame not encode---\n");
        return;
    }

    tEntry.frameIndex = mmpCapReturnWrBufIndex();
    tEntry.frameCount = ++gtAVCEncoderFlow.captureFrameCount;

    if (initClock)
    {
        dur = (mmpTimerReadCounter(TIMER_NUM) - initClock);
    }

    mmpTimerCtrlDisable(TIMER_NUM, MMP_TIMER_EN);
    mmpTimerResetCounter(TIMER_NUM);
    mmpTimerCtrlEnable(TIMER_NUM, MMP_TIMER_EN);
    initClock = mmpTimerReadCounter(TIMER_NUM) + 1;

    if (tEntry.frameCount <= 5)
    {
        if (1 == tEntry.frameCount)
        {
            totalDur = 0;

            if (gtAVCEncoderFlow.baseTimeStamp != 0)
            {
                gtAVCEncoderFlow.currTimeStamp     =
                    gtAVCEncoderFlow.baseTimeStamp = gtAVCEncoderFlow.baseTimeStamp
                                                     + (dur / 80000);
            }
            goto end;
        }
        else if (2 == tEntry.frameCount)
            goto end;
        else if (5 == tEntry.frameCount)
        {
            totalDur += dur;
            dur       = totalDur / 8 / 3;
            if ((402414 > dur) && (dur > 397614))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_25HZ;
            else if ((333444 > dur) && (dur > 331675))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_30HZ;
            else if ((333778 > dur) && (dur > 333444))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_29_97HZ;
            else if ((359066 > dur) && (dur > 355240))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_56HZ;      // 28
            else if ((431034 > dur) && (dur > 425532))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_70HZ;      // 23.33
            else if ((419287 > dur) && (dur > 414079))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_72HZ;      // 24
            else if ((354610 > dur) && (dur > 350877))
                gtAVCEncoderFlow.detectFrameRate = MMP_CAP_FRAMERATE_85HZ;      // 28.33
            else
                gtAVCEncoderFlow.detectFrameRate = mmpCapGetInputFrameRate();

            dbg_msg(DBG_MSG_TYPE_INFO, "dur(%d) frame rate(%d)\n", dur, gtAVCEncoderFlow.detectFrameRate);
            if (MMP_CAP_FRAMERATE_UNKNOW != gtAVCEncoderFlow.detectFrameRate)
            {
                gtAVCEncoderFlow.frameRate     = gtAVCEncoderFlow.detectFrameRate;
                gtAVCEncoderFlow.pfToTimeStamp = _VIDEO_ENCODER_GetTimeStampConverter(gtAVCEncoderFlow.frameRate);
                switch (gtAVCEncoderFlow.frameRate)
                {
                case MMP_CAP_FRAMERATE_25HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_25HZ;
                    break;

                case MMP_CAP_FRAMERATE_30HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_30HZ;
                    break;

                case MMP_CAP_FRAMERATE_29_97HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_29_97HZ;
                    break;
                }
                gtAVCEncoderFlow.frameRateState = FRAME_RATE_SETTING_STATE_DETECTED;
                //mmpAVCEncodeCreateHdr(gptAVCEncoder);
            }

            avSyncSetVideoStable(MMP_TRUE);
        }
        else
        {
            totalDur += dur;
            goto end;
        }
    }

    if (gtAVCEncoderFlow.pfToTimeStamp)
    {
        tEntry.timeStamp             = gtAVCEncoderFlow.currTimeStamp
                                     = gtAVCEncoderFlow.baseTimeStamp
                                       + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount);
        avSyncSetCurrentTime(tEntry.timeStamp);
    }

    //Error frame not encode
    if (captureErrState & 0x0F00)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "---Error frame not encode---\n");
        return;
    }

    if (FRAME_RATE_SETTING_STATE_SET == gtAVCEncoderFlow.frameRateState)
    {
        for (tEntry.resIdx = 0; tEntry.resIdx < ISP_TRANSFORM_NUM; tEntry.resIdx++)
            _CaptureResultQ_Enqueue(&tEntry);
    }

    if (mmpIspIsEngineIdle())
    {
        _DoIsp();
    }

end:
    mmpCapEnableInterrupt(MMP_TRUE);
}

static void
isp_isr(
    void *arg)
{
    MMP_BOOL bIsIdle = MMP_FALSE;

    mmpIspClearInterrupt();

#ifdef _DEBUG_ISP_RUN_TIME
    if (gtIspFireEntry.resIdx == (ISP_TRANSFORM_NUM - 1))
    {
        durIsp = (mmpTimerReadCounter(TIMER_NUM) - initClockIsp);
        dbg_msg(DBG_MSG_TYPE_INFO, "isp run time = %d ms\n", (durIsp / 80000));
    }
#endif

    _IspResultQ_Enqueue(&gtIspFireEntry);

    if (MMP_SUCCESS == mmpAVCEncodeQuery(AVC_ENCODER_ENGINE_IDLE, (MMP_UINT32 *)&bIsIdle)
        && bIsIdle)
    {
        _DoEncode();
    }

    _DoIsp();
}

static void
encoder_isr(
    void *arg)
{
    MMP_UINT32 streamLen;
    MMP_BOOL   bFrmEnd;

    mmpAVCEncodeClearInterrupt();
    gbEncodeFire = MMP_FALSE;

    if (mmpAVCEncodeGetStream(gptAVCEncoder[gEncodeIdx], &streamLen, &bFrmEnd) == 0)
    {
        ENCODED_RESULT tEntry = {
            gtAVCEncoderFlow.encodedFrameCount,
            gtAVCEncoderFlow.encodedTimeStamp,
            gptAVCEncoder[gEncodeIdx]->pStreamBufAdr[gptAVCEncoder[gEncodeIdx]->streamBufSelect],
            streamLen,
            gEncodeIdx
        };

        //if (gEncodeIdx == 0)
        {
            _EncodedResultQ_Enqueue(&tEntry);
        }
    }

    _DoEncode();
}

MMP_BOOL GetHDMIRx_Stable_Status(void)
{
    return gtHDMIRxStable;
}

static void
_DoAVSync(
    void)
{
    MMP_UINT32        frameCnt          = gtAVCEncoderFlow.captureFrameCount;
    static MMP_UINT32 initAVSync        = 1;
    MMP_INT32         AUDIO_SAMPLE_RATE = 48, AV_DELAY_MS = 3;
    static MMP_UINT32 sysTime           = 0, sysDiff = 0;
    static MMP_INT32  avDiff            = 0;
    MMP_UINT32        audioCounter      = 0;
    static MMP_UINT32 preAudioCounter   = 0, readAudioCntFlag = 0;
    static MMP_UINT32 initClkMax        = 0, initClkMin = 0;
    static MMP_UINT16 pllSDMValue       = 0;

    if (readAudioCntFlag)
    {
        audioCounter = mmpAVSyncCounterRead(AUDIO_COUNTER_SEL);
        if (preAudioCounter != audioCounter)
        {
            avDiff = (sysDiff * AUDIO_SAMPLE_RATE) - audioCounter;

            //dbg_msg(DBG_MSG_TYPE_INFO, "V(%f) A(%f) D(%d)\n", (MMP_FLOAT)sysDiff, ((MMP_FLOAT)audioCounter / AUDIO_SAMPLE_RATE), avDiff);

            if (avDiff > (AV_DELAY_MS * AUDIO_SAMPLE_RATE)) //1(ms)*AUDIO_SAMPLE_RATE
            {
                if (pllSDMValue < initClkMax)
                {
                    pllSDMValue      = (pllSDMValue & 0x07FF) + 1;
                    HOST_WriteRegisterMask(0xC6, pllSDMValue, 0x07FF);
                    mmpAVSyncCounterReset(AUDIO_COUNTER_CLEAR);
                    sysTime          = PalGetClock();
                    readAudioCntFlag = 0;
                    dbg_msg(DBG_MSG_TYPE_INFO, "adjust +1 %x\n", pllSDMValue);
                    return;
                }
            }
            else if (avDiff < (-1 * AV_DELAY_MS * AUDIO_SAMPLE_RATE)) //1(ms)*AUDIO_SAMPLE_RATE
            {
                if (pllSDMValue > initClkMin)
                {
                    pllSDMValue      = (pllSDMValue & 0x07FF) - 1;
                    HOST_WriteRegisterMask(0xC6, pllSDMValue, 0x07FF);
                    mmpAVSyncCounterReset(AUDIO_COUNTER_CLEAR);
                    sysTime          = PalGetClock();
                    readAudioCntFlag = 0;
                    dbg_msg(DBG_MSG_TYPE_INFO, "adjust -1 %x\n", pllSDMValue);
                    return;
                }
            }
            else
                avDiff = 0;
        }
        preAudioCounter  = audioCounter;
        readAudioCntFlag = 0;
    }

    if (avSyncIsAudioInitFinished() && avSyncIsVideoStable() && frameCnt >= 20)
    {
        if (initAVSync)
        {
            //Enable Audio/I2S Counter
            mmpAVSyncCounterCtrl((AUDIO_COUNTER_SEL | I2S_SOURCE_SEL), 0);
            mmpAVSyncCounterReset(AUDIO_COUNTER_CLEAR);
            sysTime     = PalGetClock();
            HOST_ReadRegister(0xC6, (MMP_UINT16 *)&pllSDMValue);
            pllSDMValue = (pllSDMValue & 0x07FF);
            initClkMax  = pllSDMValue + 2;
            initClkMin  = pllSDMValue - 2;
            initAVSync  = 0;
        }
        else
        {
            mmpAVSyncCounterLatch(AUDIO_COUNTER_LATCH);
            sysDiff          = PalGetDuration(sysTime);
            readAudioCntFlag = 1;
        }
    }
    else
        initAVSync = 1;
}