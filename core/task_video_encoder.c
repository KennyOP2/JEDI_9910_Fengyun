#ifdef __FREERTOS__
    #ifdef __OPENRTOS__
        #include "ite/ith.h"
    #else
        #include "intr/intr.h"
        #include "or32.h"
    #endif
#endif
#include "host/ahb.h"
#include "host/host.h"
#include "host/gpio.h"

#include "av_sync.h"
#include "task_video_encoder.h"
#include "mps_control.h"
#include "mps_system.h"
#include "mmp_encoder.h"
#include "mmp_capture.h"
#include "mmp_isp.h"
#include "mmp_timer.h"
#include "mps_control.h"
#include "mmp_hdmirx.h"
#include "grabber_control.h"
#include "mps_control.h"
#include "mmp_jpg.h"

//=============================================================================
//                Constant Definition
//=============================================================================
//#define AUTO_ONFLY_MODE
#define CAPTURE_RESULT_QUEUE_SIZE 9
#define ENCODED_RESULT_QUEUE_SIZE CAPTURE_RESULT_QUEUE_SIZE

#define QUEUE_EMPTY               (-1)
#define QUEUE_NOT_EMPTY           (0)

#define TIMER_NUM                 4

#if defined(MULTIPLE_INSTANCES)
    #define ISP_TRANSFORM_NUM     3
    #define ENCODE_STREAM_NUM     3
#else
    #define ISP_TRANSFORM_NUM     1
    #define ENCODE_STREAM_NUM     1
#endif

//#define MAX_OUTPUT_BUFFER_SIZE      (16 * 1024 * 1024)
#define MAX_OUTPUT_BUFFER_SIZE    (16)

#define FRAME_RATE_CHECK_COUNT    (3)

#define HDMIRX_REINT_GPIO         48

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

typedef enum AV_SYNC_STATE_TAG
{
    AV_SYNC_TIMER_INIT,
    AV_SYNC_TIMER_GET,
    AV_SYNC_DO_SYNC_INIT,
    AV_SYNC_DO_SYNC
} AV_SYNC_STATE;

//=============================================================================
//                Macro Definition
//=============================================================================
typedef MMP_UINT (*frameCount2TimeStamp)(MMP_UINT framecount);

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct TASK_VIDEO_ENCODER_HANDLE_TAG
{
    MPS_ELEMENT     *ptElement;
    MPS_STATE       mpsState;
    STREAM_HANDLE   tStreamMuxHandle;
    PAL_THREAD      ptVideoEncoderThread;
    MPS_CMD_OBJ     tReadCmdObj;
    MPS_CMD_OBJ     tWriteCmdObj;
    MMP_BOOL        bInRecord;
    MPS_SYSTEM_TYPE systemType;
    CAPTURE_STATE   captureState;
} TASK_VIDEO_ENCODER_HANDLE;

typedef struct CAPTURE_RESULT_TAG
{
    MMP_UINT resIdx;                // index of resolution table
    MMP_UINT frameCount;            // how many frames have been captured from the time of start record
    MMP_UINT timeStamp;             // the predicted PTS of the latest captured frame
    MMP_UINT frameIndex;            // the index of buffer stores the latest captured frame
    MMP_BOOL bTopField;
    MMP_BOOL bAVCVideo;
} CAPTURE_RESULT;

typedef MMP_RESULT (*doEncodeDequeue)(CAPTURE_RESULT *ptEntry);

typedef struct AVC_ENCODER_FLOW_TAG
{
    MMP_UINT                 baseTimeStamp;
    MMP_UINT                 currTimeStamp;
    MMP_UINT                 captureFrameCount;
    MMP_UINT                 encodedFrameCount;
    MMP_UINT                 encodedTimeStamp;
    MMP_CAP_FRAMERATE        frameRate;
    MMP_CAP_FRAMERATE        detectFrameRate;
    FRAME_RATE_SETTING_STATE frameRateState;
    frameCount2TimeStamp     pfToTimeStamp;
    doEncodeDequeue          pfEnDequeue;
    MMP_UINT                 ispFrameRate;
    MMP_UINT                 frameRateDiff;
    MMP_UINT                 frameCount;
    MMP_UINT                 skipCount;
    MMP_BOOL                 bOneShot;
    MMP_UINT                 encodingPeriod;
    MMP_BOOL                 bJPGFirstFrm;
    MMP_BOOL                 bJPPGFirstFire;
    MMP_BOOL                 bSkipAVCVideo;
} AVC_ENCODER_FLOW;

typedef struct AVC_SYNC_TIMER_TAG
{
    MMP_UINT32 curAudTime;
    MMP_UINT32 frameDuration;
    MMP_UINT32 fieldPeriod;
    MMP_UINT32 framePeriod;
    MMP_UINT32 frameRate;
    MMP_UINT32 audSampleRate;
} AVC_SYNC_TIMER;

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
    MMP_BOOL   deinterlaceOn;
    MMP_BOOL   bframeDouble;
} ISP_TRANSFORM_PARAMETER;

typedef struct ASPECT_RATIO_PARAMETER_TAG
{
    MMP_UINT16   inWidth;
    MMP_UINT16   inHeight;
    MMP_UINT16   outWidth;
    MMP_UINT16   outHeight;
    ASPECT_RATIO aspectRatioMode;

    MMP_UINT16   srcARPosX;
    MMP_UINT16   srcARPosY;
    MMP_UINT16   srcARWidth;
    MMP_UINT16   srcARHeight;

    MMP_UINT16   dstARPosX;
    MMP_UINT16   dstARPosY;
    MMP_UINT16   dstARWidth;
    MMP_UINT16   dstARHeight;
} ASPECT_RATIO_PARAMETER;

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
static AVC_SYNC_TIMER            gtAVSyncTimer;

static CAPTURE_RESULT_QUEUE      gtCaptureResultQ;
static ENCODED_RESULT_QUEUE      gtEncodedResultQ;

static CAPTURE_DEVICE            gtDevice_ID      = CAPTURE_DEV_UNKNOW;
static CAPTURE_DEVICE            gtPreDevice_ID   = CAPTURE_DEV_UNKNOW;
static CAPTURE_VIDEO_SOURCE      gtDevice_VIDMode = CAPTURE_VIDEO_SOURCE_UNKNOW;

static MMP_BOOL                  gbAVCEncodeFire  = MMP_FALSE;
static MMP_BOOL                  gbJPGEncodeFire  = MMP_FALSE;

static CAPTURE_RESULT_QUEUE      gtIspResultQ;
static ISP_TRANSFORM_PARAMETER   gtIspTransformParm[ISP_TRANSFORM_NUM];
static CAPTURE_RESULT            gtIspFireEntry;
static MMP_BOOL                  gbEnableOnfly = MMP_FALSE;
static MMP_BOOL                  gtEnAVEngine  = MMP_TRUE;

#ifdef _DEBUG_ISP_RUN_TIME
static MMP_UINT32                initClockIsp  = 0;
static MMP_UINT32                durIsp        = 0;
#endif

static MMP_UINT32                gEncodeIdx    = 0;
static ASPECT_RATIO_PARAMETER    gtARParm      = {0};

static JPG_ENCODER               *gptJPGEncoder;
static MMP_BOOL                  gbJPEGInRECORD = MMP_FALSE;

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

static void
jpeg_encoder_isr(
    void);

MMP_RESULT
_CaptureAndIsp_Init(
    void);

MMP_RESULT
_CaptureAndIsp_Fire(
    void);

MMP_RESULT
_CaptureAndIsp_Terminate(
    void);

static void
_DoAVSync(
    void);

static void
_DO_AV_SYNC(
    AV_SYNC_STATE state);

static MMP_BOOL
_Chk_Skip_Frame(
    void);

static void
_OnFly_PowerSaving(
    MMP_BOOL enable);

void
_HDMIRXDiableHPD(
    MMP_BOOL enable);

void
_CalcAspectRatio(
    ASPECT_RATIO_PARAMETER *pARParm);

static void
CopyJPEGInfo(void);

static void
_JPEG_ENCODER_Open(JPEG_CONTEXT *ptJpgCtx);

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

    PalMemcpy(ptEntry, &gtCaptureResultQ.entry[gtCaptureResultQ.rIdx], sizeof(*ptEntry));

    gtCaptureResultQ.rIdx++;
    if (gtCaptureResultQ.rIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtCaptureResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_CaptureResultQ_Enqueue(
    CAPTURE_RESULT *ptEntry)
{
    PalMemcpy(&gtCaptureResultQ.entry[gtCaptureResultQ.wIdx], ptEntry, sizeof(*ptEntry));

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

    PalMemcpy(ptEntry, &gtIspResultQ.entry[gtIspResultQ.rIdx], sizeof(*ptEntry));

    gtIspResultQ.rIdx++;
    if (gtIspResultQ.rIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtIspResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_IspResultQ_Enqueue(
    CAPTURE_RESULT *ptEntry)
{
    PalMemcpy(&gtIspResultQ.entry[gtIspResultQ.wIdx], ptEntry, sizeof(*ptEntry));

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

    PalMemcpy(ptEntry, &gtEncodedResultQ.entry[gtEncodedResultQ.rIdx], sizeof(*ptEntry));

    gtEncodedResultQ.rIdx++;
    if (gtEncodedResultQ.rIdx >= ENCODED_RESULT_QUEUE_SIZE)
        gtEncodedResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_EncodedResultQ_Enqueue(
    ENCODED_RESULT *ptEntry)
{
    PalMemcpy(&gtEncodedResultQ.entry[gtEncodedResultQ.wIdx], ptEntry, sizeof(*ptEntry));

    gtEncodedResultQ.wIdx++;
    if (gtEncodedResultQ.wIdx >= ENCODED_RESULT_QUEUE_SIZE)
        gtEncodedResultQ.wIdx = 0;
}

static void
_WaitAllQueue_Empty(
    void)
{
    CAPTURE_RESULT tEntry  = {0};
    MMP_UINT32     timeOut = 0;

    while (QUEUE_NOT_EMPTY == _CaptureResultQ_Dequeue(&tEntry) && !gbAVCEncodeFire && !gbJPGEncodeFire)
    {
        PalSleep(30);
        if (++timeOut > 100)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "wait QUEUE_NOT_EMPTY timeout %s() #%d\n", __FUNCTION__, __LINE__);
            break;
        }
    }
}

static void
_DoIsp(
    void)
{
    CAPTURE_RESULT              tEntry   = {0};
    static MMP_ISP_SINGLE_SHARE ispctxt  = {0};
    MMP_UINT16                  inBufferIdx;
    MMP_UINT16                  outBufferIdx;
    static MMP_UINT16           srcCount = 0;
    MMP_UINT16                  resIdx;

    if (QUEUE_NOT_EMPTY == _CaptureResultQ_Dequeue(&tEntry))
    {
#ifdef _DEBUG_ISP_RUN_TIME
        initClockIsp      = mmpTimerReadCounter(TIMER_NUM) + 1;
#endif
        resIdx            = tEntry.resIdx;
        inBufferIdx       = tEntry.frameIndex;
#if defined(MULTIPLE_INSTANCES)
        outBufferIdx      = resIdx;
#else
        outBufferIdx      = srcCount;   //tEntry.frameIndex;
#endif
        tEntry.frameIndex = outBufferIdx;

        srcCount          = (srcCount + 1) % gptAVCEncoder[0]->sourceBufCount;

        //Signal Process Input Parameter
        ispctxt.In_AddrY  = gtIspTransformParm[resIdx].inAddrY[inBufferIdx];
        ispctxt.In_AddrUV = gtIspTransformParm[resIdx].inAddrUV[inBufferIdx];

        if (inBufferIdx == 0)
            ispctxt.In_AddrYp = gtIspTransformParm[resIdx].inAddrY[2];
        else
            ispctxt.In_AddrYp = gtIspTransformParm[resIdx].inAddrY[inBufferIdx - 1];

        ispctxt.In_Width    = gtIspTransformParm[resIdx].inWidth;
        ispctxt.In_Height   = gtIspTransformParm[resIdx].inHeight;
        ispctxt.In_PitchY   = CAP_MEM_BUF_PITCH;
        ispctxt.In_PitchUV  = CAP_MEM_BUF_PITCH;
        ispctxt.In_Format   = MMP_ISP_IN_NV12;

        //Signal Process Output Parameter
        ispctxt.Out_AddrY   = gtIspTransformParm[resIdx].outAddrY[outBufferIdx];
        ispctxt.Out_AddrU   = gtIspTransformParm[resIdx].outAddrUV[outBufferIdx];
        ispctxt.Out_Width   = gptAVCEncoder[resIdx]->frameWidth;
        ispctxt.Out_Height  = gptAVCEncoder[resIdx]->frameHeight;
        ispctxt.Out_PitchY  = gptAVCEncoder[resIdx]->framePitchY;
        ispctxt.Out_PitchUV = gptAVCEncoder[resIdx]->framePitchY;
        ispctxt.Out_Format  = MMP_ISP_OUT_NV12;

        mmpIspEnable(MMP_ISP_INTERRUPT);
        mmpIspEnable(MMP_ISP_REMAP_ADDRESS);

        if (gtIspTransformParm[resIdx].deinterlaceOn)
        {
            mmpIspEnable(MMP_ISP_DEINTERLACE);

            if (gtIspTransformParm[resIdx].bframeDouble)
            {
                if (tEntry.bTopField)
                    mmpIspEnable(MMP_ISP_DEINTER_FIELD_TOP);
                else
                    mmpIspEnable(MMP_ISP_DEINTER_FIELD_BOTTOM);
            }
        }
        else
        {
            mmpIspDisable(MMP_ISP_DEINTERLACE);
        }

        mmpIspSingleProcess(&ispctxt);

        memcpy(&gtIspFireEntry, &tEntry, sizeof(CAPTURE_RESULT));
    }
}

static void
_DoEncode(
    void)
{
    CAPTURE_RESULT tEntry = {0};

    if (gbAVCEncodeFire)
        return;

    if (QUEUE_NOT_EMPTY == gtAVCEncoderFlow.pfEnDequeue(&tEntry))
    {
        gEncodeIdx                                 = tEntry.resIdx;
        //gptAVCEncoder[gEncodeIdx]->streamBufSelect = (gptAVCEncoder[gEncodeIdx]->streamBufSelect + 1)
        //                               % (gptAVCEncoder[gEncodeIdx]->streamBufCount);
        gtAVCEncoderFlow.encodedFrameCount         = tEntry.frameCount;
        gtAVCEncoderFlow.encodedTimeStamp          = tEntry.timeStamp;
        gptAVCEncoder[gEncodeIdx]->sourceBufSelect = tEntry.frameIndex;
        gbAVCEncodeFire                            = MMP_TRUE;
        mmpAVCEncodeFire(gptAVCEncoder[gEncodeIdx]);
    }
}

static void
_DoJPEGEncode(
    void)
{
    CAPTURE_RESULT tEntry = {0};

    if (gbAVCEncodeFire)
        return;

    if (gbJPGEncodeFire)
    {
        jpeg_encoder_isr();
    }

    if (QUEUE_NOT_EMPTY == gtAVCEncoderFlow.pfEnDequeue(&tEntry) && !tEntry.bAVCVideo)
    {
        gEncodeIdx = tEntry.resIdx;

        //gptAVCEncoder[gEncodeIdx]->streamBufSelect = (gptAVCEncoder[gEncodeIdx]->streamBufSelect + 1)
        //                               % (gptAVCEncoder[gEncodeIdx]->streamBufCount);
        //gtAVCEncoderFlow.encodedFrameCount              = tEntry.frameCount;
        //gtAVCEncoderFlow.encodedTimeStamp               = tEntry.timeStamp;
        //gptAVCEncoder[gEncodeIdx]->sourceBufSelect      = tEntry.frameIndex;
        gptJPGEncoder->sourceBufSelect = tEntry.frameIndex;

        if ((gtAVCEncoderFlow.bOneShot && !gtAVCEncoderFlow.bJPPGFirstFire) || !gtAVCEncoderFlow.bOneShot)
        {
            gbJPGEncodeFire                 = MMP_TRUE;
            mmpJPGEncodeFire(gptJPGEncoder);
            gtAVCEncoderFlow.bJPPGFirstFire = MMP_TRUE;
        }
    }
}

static void
cap_isr(
    void *arg);

static void
isp_mem_isr(
    void *arg);

static void
isp_onfly_isr(
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
frameRate50Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return frameCount * 20;
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
    return (frameCount / 30) * 1001 + timeStamp[frameCount % 30];
}

static MMP_INLINE MMP_UINT
frameRate59_94Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return frameRate29_97Hz_frameCount2TimeStamp(frameCount) / 2;
}

static MMP_INLINE MMP_UINT
frameRate30Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return (frameCount / 3) * 100 + (frameCount % 3) * 33;
}

static MMP_INLINE MMP_UINT
frameRate60Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return (frameCount / 3) * 50 + (frameCount % 3) * 33 / 2;
}

static MMP_INLINE MMP_UINT
frameRate23_97Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    static MMP_UINT timeStamp[24] = {
        0,  41,  83,
        125, 166, 208,
        250, 291, 333,
        375, 417, 458,
        500, 542, 583,
        625, 667, 709,
        750, 792, 834,
        875, 917, 959
    };
    return (frameCount / 24) * 1001 + timeStamp[frameCount % 24];
}

static MMP_INLINE MMP_UINT
frameRate24Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    static MMP_UINT timeStamp[12] = {
        0,  41,  83,
        125, 166, 208,
        250, 291, 333,
        375, 416, 458
    };
    return (frameCount / 12) * 500 + timeStamp[frameCount % 12];
}

static MMP_INLINE MMP_UINT
frameRateVESA_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return gtAVSyncTimer.curAudTime + 33;
}

static frameCount2TimeStamp _frameCount2TimeStamp_TABLE[16] =
{
    MMP_NULL,
    frameRate25Hz_frameCount2TimeStamp,
    frameRate50Hz_frameCount2TimeStamp,
    frameRate30Hz_frameCount2TimeStamp,
    frameRate60Hz_frameCount2TimeStamp,
    frameRate29_97Hz_frameCount2TimeStamp,
    frameRate59_94Hz_frameCount2TimeStamp,
    frameRate23_97Hz_frameCount2TimeStamp,
    frameRate24Hz_frameCount2TimeStamp,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    frameRateVESA_frameCount2TimeStamp,
    frameRateVESA_frameCount2TimeStamp
};

static MMP_INLINE frameCount2TimeStamp
_VIDEO_ENCODER_GetTimeStampConverter(
    MMP_CAP_FRAMERATE frameRate)
{
    frameCount2TimeStamp pf = MMP_NULL;

    if (frameRate <= 15)
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

    //_CaptureAndIsp_Init();
    mmpTimerCtrlEnable(TIMER_NUM, MMP_TIMER_UPCOUNT);

    gtVideoEncoderHandle.mpsState                       = MPS_STATE_STOP;
    gtVideoEncoderHandle.ptElement                      = mpsSys_GetElement(MPS_VIDEO_ENCODER);
    gtVideoEncoderHandle.systemType                     = (MPS_SYSTEM_TYPE)arg;

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
    avSyncSetVideoStable(MMP_FALSE);

    gtDevice_ID      = CAPTURE_DEV_UNKNOW;
    gtPreDevice_ID   = CAPTURE_DEV_UNKNOW;
    gtDevice_VIDMode = CAPTURE_VIDEO_SOURCE_UNKNOW;
    gbAVCEncodeFire  = MMP_FALSE;
    gbJPGEncodeFire  = MMP_FALSE;
    gbEnableOnfly    = MMP_FALSE;
    gtEnAVEngine     = MMP_TRUE;
    gEncodeIdx       = 0;
}

void
taskVideoEncoder_Terminate(
    MMP_UINT32 arg)
{
    if (arg) { } // avoid compiler warning

    _CaptureAndIsp_Terminate();

    mmpTimerCtrlDisable(TIMER_NUM, MMP_TIMER_EN);
    mmpTimerResetCounter(TIMER_NUM);

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
    MMP_UINT32 i;

    if (mmpAVCEncodeInit() != 0)
        dbg_msg(DBG_MSG_TYPE_INFO, "[264 TEST] mmpAVCEncodeInit Fail\n");

    for (i = 0; i < ENCODE_STREAM_NUM; i++)
    {
        if (MMP_NULL == gptAVCEncoder[i])
            gptAVCEncoder[i] = (AVC_ENCODER *)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(AVC_ENCODER));

        if (MMP_NULL != gptAVCEncoder[i])
            PalMemset(gptAVCEncoder[i], 0, sizeof(AVC_ENCODER));
    }

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
    gptAVCEncoder[0]->initialDelay             = 1000;
    gptAVCEncoder[0]->chromaQpOffset           = 0; //-12~+12 need to use 5 bits 2's complement, default -2
    gptAVCEncoder[0]->constrainedIntraPredFlag = 0;
    gptAVCEncoder[0]->disableDeblk             = 0;
    gptAVCEncoder[0]->deblkFilterOffsetAlpha   = 0;
    gptAVCEncoder[0]->deblkFilterOffsetBeta    = 0;
    gptAVCEncoder[0]->vbvBufferSize            = 0;
    gptAVCEncoder[0]->intraRefresh             = 0;
    gptAVCEncoder[0]->rcIntraQp                = 0;
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

#if defined(MULTIPLE_INSTANCES)

    if (ENCODE_STREAM_NUM >= 2)
    {
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
        gptAVCEncoder[1]->initialDelay             = 1000;
        gptAVCEncoder[1]->chromaQpOffset           = 0; //-12~+12 need to use 5 bits 2's complement, default -2
        gptAVCEncoder[1]->constrainedIntraPredFlag = 0;
        gptAVCEncoder[1]->disableDeblk             = 0;
        gptAVCEncoder[1]->deblkFilterOffsetAlpha   = 0;
        gptAVCEncoder[1]->deblkFilterOffsetBeta    = 0;
        gptAVCEncoder[1]->vbvBufferSize            = 0;
        gptAVCEncoder[1]->intraRefresh             = 0;
        gptAVCEncoder[1]->rcIntraQp                = 0;
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
    }

    if (ENCODE_STREAM_NUM >= 3)
    {
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
        gptAVCEncoder[2]->initialDelay             = 1000;
        gptAVCEncoder[2]->chromaQpOffset           = 0; //-12~+12 need to use 5 bits 2's complement, default -2
        gptAVCEncoder[2]->constrainedIntraPredFlag = 0;
        gptAVCEncoder[2]->disableDeblk             = 0;
        gptAVCEncoder[2]->deblkFilterOffsetAlpha   = 0;
        gptAVCEncoder[2]->deblkFilterOffsetBeta    = 0;
        gptAVCEncoder[2]->vbvBufferSize            = 0;
        gptAVCEncoder[2]->intraRefresh             = 0;
        gptAVCEncoder[2]->rcIntraQp                = 0;
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
    }
#endif

    if (MMP_NULL == gptJPGEncoder)
        gptJPGEncoder = (JPG_ENCODER *)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(JPG_ENCODER));

    if (MMP_NULL != gptJPGEncoder)
        PalMemset(gptJPGEncoder, 0, sizeof(JPG_ENCODER));

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
    static PAL_CLOCK_T   captureStartTime;
    static PAL_CLOCK_T   queryStableTime;
    static MMP_UINT16    reTryCounter    = 0;
    MMP_BOOL             doBreak         = MMP_FALSE;
    MMP_BOOL             bSignalStable   = MMP_TRUE;
    MMP_UINT16           captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);
    MMP_RESULT           result;
    MMP_UINT32           i;

    do
    {
        //dbg_msg(DBG_MSG_TYPE_INFO, "state(%d)\n", state);
        switch (gtVideoEncoderHandle.captureState)
        {
        case CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR:
            if (captureErrState & 0x0F00)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "[Debug] Capture Error State Code = 0x%x\n", captureErrState);
                gtVideoEncoderHandle.captureState = CAPTURE_STATE_RESET_CAPTURE_AND_ISP;
            }
            else
            {
                gtVideoEncoderHandle.captureState = CAPTURE_STATE_POLLING_DEVICE_STATUS;
            }
            break;

        case CAPTURE_STATE_POLLING_DEVICE_STATUS:
            if (!mmpCapIsFire() || PalGetDuration(queryStableTime) >= 500)
            {
                bSignalStable   = mmpCapDeviceIsSignalStable();
                queryStableTime = PalGetClock();
            }

            if (!bSignalStable)
                avSyncSetVideoStable(MMP_FALSE);

            if (bSignalStable && !mmpCapIsFire())
            {
                gtVideoEncoderHandle.captureState = CAPTURE_STATE_FIRE_CAPTURE;
            }
            else
            {
                gtVideoEncoderHandle.captureState = CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR;
                doBreak = MMP_TRUE;
            }
            break;

        case CAPTURE_STATE_FIRE_CAPTURE:
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
            {
#ifdef ENABLE_MENCODER
                gptAVCEncoder[i]->bMP4format        = (gtVideoEncoderHandle.systemType == MPS_SYSTEM_TYPE_AVI_MUX);
                gptAVCEncoder[i]->bEnableRingBuffer = (gtVideoEncoderHandle.systemType == MPS_SYSTEM_TYPE_AVI_MUX); //kenny20150324
#endif
                if (mmpAVCEncodeOpen(gptAVCEncoder[i]) != 0)
                    dbg_msg(DBG_MSG_TYPE_INFO, "[264 TEST] mmpAVCEncodeOpen Fail Num : %d\n", i);

                mmpAVCEncodeCreateHdr(gptAVCEncoder[i]);
            }

            gbAVCEncodeFire  = MMP_FALSE;
            gbJPGEncodeFire  = MMP_FALSE;
            _CaptureAndIsp_Fire();
            dbg_msg(DBG_MSG_TYPE_INFO, "Fire Capture and ISP Engine Device(%d)!\n", mmpCapGetCaptureDevice());
            reTryCounter     = 0;
            gtVideoEncoderHandle.captureState = CAPTURE_STATE_WAIT_CAPTURE_SYNC;
            captureStartTime = PalGetClock();
            doBreak          = MMP_TRUE;
            break;

        case CAPTURE_STATE_WAIT_CAPTURE_SYNC:
            if (!mmpCapIsFire())
                gtVideoEncoderHandle.captureState = CAPTURE_STATE_RESET_CAPTURE_AND_ISP;
            else if (mmpCapIsFire() && !(captureErrState & 0x000F))
            {
                gtVideoEncoderHandle.captureState = CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR;
            }
            else
            {
                if (PalGetDuration(captureStartTime) > 200)
                {
#if defined(SENSOR_DEV)
                    gtVideoEncoderHandle.captureState = CAPTURE_STATE_RESET_CAPTURE_AND_ISP;
#else
                    if (mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA && reTryCounter < 3)
                        gtVideoEncoderHandle.captureState = CAPTURE_STATE_RETRY_CAPTURE_POLARITY;
                    else
                        gtVideoEncoderHandle.captureState = CAPTURE_STATE_RESET_CAPTURE_AND_ISP;
#endif
                }
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
            gtVideoEncoderHandle.captureState = CAPTURE_STATE_WAIT_CAPTURE_SYNC;
            captureStartTime = PalGetClock();
            doBreak          = MMP_TRUE;
            break;

        case CAPTURE_STATE_RESET_CAPTURE_AND_ISP:
            _WaitAllQueue_Empty();
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
            {
                if (gtAVCEncoderFlow.frameRateState == FRAME_RATE_SETTING_STATE_INIT)
                    mmpAVCEncodeCreateHdr(gptAVCEncoder[i]);

                if (MMP_SUCCESS != mmpAVCEncodeClose(gptAVCEncoder[i]))
                    dbg_msg(DBG_MSG_TYPE_INFO, "mmpAVCEncodeClose time out Num : %d\n", i);
            }

            gbAVCEncodeFire = MMP_FALSE;
            gbJPGEncodeFire = MMP_FALSE;

            if (mmpCapIsOnflyMode())
            {
                mmpCapOnflyResetAllEngine();
            }
            else
            {
                mmpCapResetEngine();
                mmpCapEnableInterrupt(MMP_TRUE);
            }
            avSyncSetVideoStable(MMP_FALSE);
            dbg_msg(DBG_MSG_TYPE_INFO, "Reset Capture and ISP Engine Device(%d)!\n", mmpCapGetCaptureDevice());
            gtVideoEncoderHandle.captureState = CAPTURE_STATE_POLLING_DEVICE_STATUS;
            break;
        }
    } while (!doBreak);

    gtAVSyncTimer.audSampleRate = avSyncGetAudioSampleRate();;

    //if (gtDevice_ID != CAPTURE_DEV_HDMIRX)
    //    _DoAVSync();

    {
        static MMP_BOOL bInit;
        if (!bInit)
        {
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
                gptAVCEncoder[i]->sourceBufSelect = 0;
            bInit = MMP_TRUE;
        }
    }

    {
        ENCODED_RESULT tEntry = {0};

        if (FRAME_RATE_SETTING_STATE_DETECTED == gtAVCEncoderFlow.frameRateState)
        {
#if !defined(MULTIPLE_INSTANCES)
            for (i = 0; i < ENCODE_STREAM_NUM; i++)
                mmpAVCEncodeCreateHdr(gptAVCEncoder[i]);
#endif
            gtAVCEncoderFlow.frameRateState = FRAME_RATE_SETTING_STATE_SET;
        }
    }
}

static void
_VIDEO_ENCODER_DoStop(
    void)
{
    MMP_UINT32 i;

    avSyncSetVideoStable(MMP_FALSE);
    _CaptureAndIsp_Terminate();

    mmpAVCEncodeDisableInterrupt();
    for (i = 0; i < ENCODE_STREAM_NUM; i++)
        mmpAVCEncodeClose(gptAVCEncoder[i]);
    PalMemset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    avSyncSetCurrentTime(0);
    _CaptureResultQ_Reset();
    _IspResultQ_Reset();
    _EncodedResultQ_Reset();
    gtEnAVEngine = MMP_TRUE;
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
                gtVideoEncoderHandle.mpsState  = MPS_STATE_RUN;
                gtAVCEncoderFlow.bSkipAVCVideo = MMP_FALSE;
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP:
                gtAVCEncoderFlow.bSkipAVCVideo = MMP_FALSE;
                gtVideoEncoderHandle.mpsState  = MPS_STATE_STOP;
                _VIDEO_ENCODER_DoStop();
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_TERMINATE:
                gtVideoEncoderHandle.mpsState = MPS_STATE_ZERO;
                goto end;

            //case MPS_COMMAND_SET_ENCODE_PARAMETER:
            //    {
            //        VIDEO_ENCODER_PARAMETER* para;
            //        ptData  = (MPS_PROPERITY_DATA*)gtVideoEncoderHandle.tReadCmdObj.extraData;
            //        para    = (VIDEO_ENCODER_PARAMETER*)ptData->data;
            //        gptAVCEncoder[para->InstanceNum]->frameWidth    = para->EnWidth;
            //        gptAVCEncoder[para->InstanceNum]->frameHeight   = para->EnHeight;
            //        gptAVCEncoder[para->InstanceNum]->bitRate       = para->EnBitrate;
            //        dbg_msg(DBG_MSG_TYPE_INFO, "width(%d) height(%d) bitrate(%d)\n", para->EnWidth, para->EnHeight, para->EnBitrate);
            //        _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
            //    }
            //    break;

            //case MPS_COMMAND_SET_CAPTURE_DEVICE:
            //	  {
            //        CAPTURE_DEVICE_INFO* para;
            //        ptData      = (MPS_PROPERITY_DATA*)gtVideoEncoderHandle.tReadCmdObj.extraData;
            //        para        = (CAPTURE_DEVICE_INFO*)ptData->data;
            //        gtDevice_ID = para->deviceId;
            //        gtDevice_VIDMode = para->videoMode;
            //        dbg_msg(DBG_MSG_TYPE_INFO, "MPS_COMMAND_SET_CAPTURE_DEVICE %d mode %d\n", gtDevice_ID, gtDevice_VIDMode);
            //        _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
            //    }
            //	  break;
            //case MPS_COMMAND_SET_ISP_MODE:
            //	  {
            //        ptData           = (MPS_PROPERITY_DATA*)gtVideoEncoderHandle.tReadCmdObj.extraData;
            //        gbEnableOnfly    = (MMP_BOOL)ptData->data;
            //        dbg_msg(DBG_MSG_TYPE_INFO, "MPS_COMMAND_SET_ISP_MODE %d\n", gbEnableOnfly);
            //        _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
            //    }
            //	  break;
            //case MPS_COMMAND_SET_ENABLE_AV_ENGINE:
            //	  {
            //        ptData           = (MPS_PROPERITY_DATA*)gtVideoEncoderHandle.tReadCmdObj.extraData;
            //        gtEnAVEngine     = (MMP_BOOL)ptData->data;
            //        //dbg_msg(DBG_MSG_TYPE_INFO, "task_video_encoder MPS_COMMAND_SET_ENABLE_AV_ENGINE %d\n", gtEnAVEngine);
            //        _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
            //    }
            //	  break;
            case MPS_COMMAND_START_JPEG_RECORD:
                {
                    JPEG_CONTEXT *para;
                    ptData = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                    para   = (JPEG_CONTEXT *)ptData->data;
                    _JPEG_ENCODER_Open(para);
                    //printf("Encode JPEG %d %d %d %x\n", para->bOneShot, para->encodingPeriod, para->quality, para->pfJPEGCallback);
                    _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                }
                break;

            case MPS_COMMAND_STOP_JPEG_RECORD:
                gbJPEGInRECORD  = MMP_FALSE;
                gbJPGEncodeFire = MMP_FALSE;  //kenny

                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_START_RECORD:
            case MPS_COMMAND_STOP_RECORD:
            case MPS_COMMAND_CLOSE:
            case MPS_COMMAND_GET_PROPERTY:
                _VIDEO_ENCODER_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_SET_PROPERTY:
                ptData = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                switch (ptData->properityId)
                {
                case MPS_PROPERITY_SET_ENCODE_PARAMETER:
                    {
                        VIDEO_ENCODER_PARAMETER *para;
                        ptData                                        = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                        para                                          = (VIDEO_ENCODER_PARAMETER *)ptData->data;
                        gptAVCEncoder[para->InstanceNum]->frameWidth  = para->EnWidth;
                        gptAVCEncoder[para->InstanceNum]->frameHeight = para->EnHeight;
                        gptAVCEncoder[para->InstanceNum]->bitRate     = para->EnBitrate;
                        dbg_msg(DBG_MSG_TYPE_INFO, "width(%d) height(%d) bitrate(%d)\n", para->EnWidth, para->EnHeight, para->EnBitrate);
                        printf("width(%d) height(%d) bitrate(%d)\n", para->EnWidth, para->EnHeight, para->EnBitrate);
                    }
                    break;

                case MPS_PROPERITY_SET_CAPTURE_DEVICE:
                    {
                        CAPTURE_DEVICE_INFO *para;
                        ptData           = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                        para             = (CAPTURE_DEVICE_INFO *)ptData->data;
                        gtDevice_ID      = para->deviceId;
                        gtDevice_VIDMode = para->videoMode;
                        dbg_msg(DBG_MSG_TYPE_INFO, "MPS_PROPERITY_SET_CAPTURE_DEVICE %d mode %d\n", gtDevice_ID, gtDevice_VIDMode);
                    }
                    break;

                case MPS_PROPERITY_SET_ISP_MODE:
                    ptData        = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                    gbEnableOnfly = (MMP_BOOL)ptData->data;
                    dbg_msg(DBG_MSG_TYPE_INFO, "MPS_PROPERITY_SET_ISP_MODE %d\n", gbEnableOnfly);
                    break;

                case MPS_PROPERITY_SET_ENABLE_AV_ENGINE:
                    ptData       = (MPS_PROPERITY_DATA *)gtVideoEncoderHandle.tReadCmdObj.extraData;
                    gtEnAVEngine = (MMP_BOOL)ptData->data;
                    //dbg_msg(DBG_MSG_TYPE_INFO, "task_video_encoder MPS_PROPERITY_SET_ENABLE_AV_ENGINE %d\n", gtEnAVEngine);
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

    if (gtDevice_ID == CAPTURE_DEV_HDMIRX && mmpCapGetDeviceReboot())
        _HDMIRXDiableHPD(MMP_TRUE);

    //Select Capture Device
    mmpCapSetCaptureDevice(gtDevice_ID);
    gtDevice_ID = mmpCapGetCaptureDevice();

    //1st Capture Init, switch IO direction
    mmpCapInitialize();

    //2nd Device Init
    mmpCapDeviceInitialize();

    //3rd ISP Init
    mmpIspInitialize();

    //    //Register IRQ
    //    mmpCapRegisterIRQ(cap_isr);
    //
    //    //Register IRQ
    //    mmpIspRegisterIRQ(isp_isr);

    if (gtDevice_ID == CAPTURE_DEV_HDMIRX && mmpCapGetDeviceReboot())
        _HDMIRXDiableHPD(MMP_FALSE);

    avSyncSetDeviceInitFinished(MMP_TRUE);
    gtVideoEncoderHandle.captureState = CAPTURE_STATE_CHECK_CAPTURE_IS_ERROR;

    return result;
}

MMP_RESULT
_CaptureAndIsp_Fire(
    void)
{
    MMP_RESULT               result   = MMP_SUCCESS;
    MMP_ISP_SEQUENCE_SHARE   *ispctxt = MMP_NULL;
    MMP_CAP_SHARE            *capctxt = MMP_NULL;
    MMP_UINT16               i, j;
    MMP_UINT                 oldTimeStamp;
    //MMP_CAP_FRAMERATE framerate;
    VIDEO_ENCODER_INPUT_INFO index;
    VIDEO_ENCODER_PARAMETER  EnPara;
    MMP_UINT32               srcAdrOffset, dstAdrOffset;

    //Capture setting
    capctxt = calloc(1, sizeof(MMP_CAP_SHARE));
    if (capctxt == MMP_NULL)
        dbg_msg(DBG_MSG_TYPE_INFO, "capctxt alloc Fail ! %s [#%d]\n", __FILE__, __LINE__);

    memset(capctxt, 0, sizeof(MMP_CAP_SHARE));

    //get device information
    mmpCapGetDeviceInfo(capctxt);
    if (capctxt->bMatchResolution == MMP_FALSE)
        goto lab_end;

    //set capture parameter
    //mmpCapParameterSetting(capctxt);

    oldTimeStamp                           = gtAVCEncoderFlow.currTimeStamp;
    PalMemset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    gtAVCEncoderFlow.baseTimeStamp         = oldTimeStamp;
    gtAVCEncoderFlow.frameRate             = capctxt->FrameRate;

    gtAVCEncoderFlow.pfToTimeStamp         = _VIDEO_ENCODER_GetTimeStampConverter(gtAVCEncoderFlow.frameRate);

    mmpIspResetEngine();
    mmpIspContextReset();

    //setting video color
    {
        MMP_ISP_COLOR_CTRL ispCtrl;
        GRABBER_CTRL_PARAM grabberCtrl;

        grabberCtrl.flag   = GRABBER_CTRL_VIDEO_COLOR;
        GrabberControlGetParam(&grabberCtrl);

        ispCtrl.brightness = grabberCtrl.COLOR.brightness;
        ispCtrl.contrast   = grabberCtrl.COLOR.contrast;
        ispCtrl.hue        = grabberCtrl.COLOR.hue;
        ispCtrl.saturation = grabberCtrl.COLOR.saturation;
        mmpIspSetColorCtrl(&ispCtrl);
    }

#ifdef AUTO_ONFLY_MODE
    gbEnableOnfly = !capctxt->IsInterlaced;
    _OnFly_PowerSaving(gbEnableOnfly);
#endif

#if !defined(MULTIPLE_INSTANCES)
    if (gbEnableOnfly == MMP_FALSE)
    {
        mmpCapMemoryClear();

        gtAVCEncoderFlow.pfEnDequeue = _IspResultQ_Dequeue;

        //Register IRQ
        if (gbEnableOnfly == MMP_FALSE)
        {
            mmpCapRegisterIRQ(cap_isr);
            mmpCapFunEnable(MMP_CAP_INTERRUPT);
        }
        else
        {
            mmpCapDisableIRQ();
            mmpCapFunDisable(MMP_CAP_INTERRUPT);
        }

        mmpCapFunDisable(MMP_CAP_ONFLY_MODE);
        mmpIspRegisterIRQ(isp_mem_isr);

        index = mmpCapGetInputSrcInfo();

        coreGetVideoEnPara(gtDevice_VIDMode, index, &EnPara);

        gtAVCEncoderFlow.ispFrameRate = coreGetCapFrameRate(index);

        if (coreIsInterlaceSrc(index))
        {
            if (EnPara.EnDeinterlaceOn == MMP_FALSE || EnPara.EnFrameDouble == MMP_FALSE)
                gtAVCEncoderFlow.ispFrameRate = gtAVCEncoderFlow.ispFrameRate / 2;
        }
        else
        {
            if (EnPara.EnSkipMode == VIDEO_ENCODER_SKIP_BY_TWO)
                gtAVCEncoderFlow.ispFrameRate = gtAVCEncoderFlow.ispFrameRate / 2;
        }
        gtAVCEncoderFlow.frameRateDiff = gtAVCEncoderFlow.ispFrameRate - EnPara.EnFrameRate;
        gtAVCEncoderFlow.frameCount    = 1;
        gtAVCEncoderFlow.skipCount     = 1;

        if (EnPara.EnWidth == 1280 && EnPara.EnHeight == 720 &&
            (index >= 10 && index <= 20)) // src = 1080P
        {
            capctxt->OutWidth = 1280;
        }

        gtARParm.inWidth   = capctxt->OutWidth;
        gtARParm.inHeight  = capctxt->OutHeight;
        gtARParm.outWidth  = EnPara.EnWidth;
        gtARParm.outHeight = EnPara.EnHeight;

        if (mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX && mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
            gtARParm.aspectRatioMode = AR_LETTER_BOX;
        else
            gtARParm.aspectRatioMode = AR_FULL;
        _CalcAspectRatio(&gtARParm);

        dbg_msg(DBG_MSG_TYPE_INFO, "---src = %d, %d, %d, %d, dst = %d, %d, %d, %d, input PAR = %d---\n",
                gtARParm.srcARPosX, gtARParm.srcARPosY, gtARParm.srcARWidth, gtARParm.srcARHeight,
                gtARParm.dstARPosX, gtARParm.dstARPosY, gtARParm.dstARWidth, gtARParm.dstARHeight,
                capctxt->inputPAR);

        if (gtARParm.aspectRatioMode == AR_LETTER_BOX)
        {
            srcAdrOffset                   = 0x0;
            dstAdrOffset                   = 0x0;
            capctxt->OutAddrOffset         = gtARParm.dstARPosY * CAP_MEM_BUF_PITCH + gtARParm.dstARPosX;
            capctxt->OutWidth              = gtARParm.dstARWidth;
            gtIspTransformParm[0].inWidth  = gtARParm.outWidth;
            gtIspTransformParm[0].inHeight = gtARParm.srcARHeight;
        }
        else
        {
            srcAdrOffset                   = gtARParm.srcARPosY * CAP_MEM_BUF_PITCH + gtARParm.srcARPosX;
            dstAdrOffset                   = gtARParm.dstARPosY * gptAVCEncoder[0]->framePitchY + gtARParm.dstARPosX;
            capctxt->OutAddrOffset         = 0x0;
            gtIspTransformParm[0].inWidth  = gtARParm.srcARWidth;
            gtIspTransformParm[0].inHeight = gtARParm.srcARHeight;
        }

        for (i = 0; i < 3; i++)
        {
            gtIspTransformParm[0].inAddrY[i]  = capctxt->OutAddrY[i] + srcAdrOffset;
            gtIspTransformParm[0].inAddrUV[i] = capctxt->OutAddrUV[i] + srcAdrOffset;
        }

        for (i = 0; i < gptAVCEncoder[0]->sourceBufCount; i++)
        {
            gtIspTransformParm[0].outAddrY[i]  = (MMP_UINT32) gptAVCEncoder[0]->pSourceBufAdrY[i] + dstAdrOffset;
            gtIspTransformParm[0].outAddrUV[i] = (MMP_UINT32) gptAVCEncoder[0]->pSourceBufAdrU[i] + dstAdrOffset;
        }

        if (capctxt->IsInterlaced && !EnPara.EnDeinterlaceOn)
            gptAVCEncoder[0]->interlaced_frame = MMP_TRUE;
        else
            gptAVCEncoder[0]->interlaced_frame = MMP_FALSE;

        gtIspTransformParm[0].outWidth      = gtARParm.dstARWidth;
        gtIspTransformParm[0].outHeight     = gtARParm.dstARHeight;

        gtIspTransformParm[0].outWidth      = gptAVCEncoder[0]->frameWidth = EnPara.EnWidth;
        gtIspTransformParm[0].outHeight     = gptAVCEncoder[0]->frameHeight = EnPara.EnHeight;

        gptJPGEncoder->frameWidth           = EnPara.EnWidth;
        gptJPGEncoder->frameHeight          = EnPara.EnHeight;

        gptAVCEncoder[0]->bitRate           = EnPara.EnBitrate;
        gptAVCEncoder[0]->gopSize           = EnPara.EnGOPSize;
        gptAVCEncoder[0]->EnFrameRate       = EnPara.EnFrameRate;
        gptAVCEncoder[0]->bISPOnFly         = gbEnableOnfly;

        gtIspTransformParm[0].deinterlaceOn = MMP_TRUE;  //EnPara.EnDeinterlaceOn;
        gtIspTransformParm[0].bframeDouble  = MMP_FALSE; //EnPara.EnFrameDouble;

        dbg_msg(DBG_MSG_TYPE_INFO, "Index %d W %d H %d B %d De %d %d S %d GOP %d AS %d\n", index,
                EnPara.EnWidth, EnPara.EnHeight, EnPara.EnBitrate, EnPara.EnDeinterlaceOn, EnPara.EnFrameDouble,
                EnPara.EnSkipMode, EnPara.EnGOPSize, EnPara.EnAspectRatio);
    }
    else
    {
        gtAVCEncoderFlow.pfEnDequeue = _CaptureResultQ_Dequeue;

        //Register IRQ
        mmpIspRegisterIRQ(isp_onfly_isr);

        mmpCapDisableIRQ();
        mmpCapFunDisable(MMP_CAP_INTERRUPT);
        mmpCapFunEnable(MMP_CAP_ONFLY_MODE);

        index = mmpCapGetInputSrcInfo();

        coreGetVideoEnPara(gtDevice_VIDMode, index, &EnPara);

        gtAVCEncoderFlow.ispFrameRate = coreGetCapFrameRate(index);

        if (coreIsInterlaceSrc(index))
        {
            if (EnPara.EnDeinterlaceOn == MMP_FALSE || EnPara.EnFrameDouble == MMP_FALSE)
                gtAVCEncoderFlow.ispFrameRate = gtAVCEncoderFlow.ispFrameRate / 2;
        }
        else
        {
            if (EnPara.EnSkipMode == VIDEO_ENCODER_SKIP_BY_TWO)
                gtAVCEncoderFlow.ispFrameRate = gtAVCEncoderFlow.ispFrameRate / 2;
        }
        gtAVCEncoderFlow.frameRateDiff = gtAVCEncoderFlow.ispFrameRate - EnPara.EnFrameRate;
        gtAVCEncoderFlow.frameCount    = 1;
        gtAVCEncoderFlow.skipCount     = 1;

        //ISP Setting
        ispctxt                        = calloc(1, sizeof(MMP_ISP_SEQUENCE_SHARE));
        if (ispctxt == MMP_NULL)
            dbg_msg(DBG_MSG_TYPE_INFO, "ispctxt alloc Fail ! %s [#%d]\n", __FILE__, __LINE__);

        ispctxt->In_Width     = capctxt->OutWidth;
        ispctxt->In_Height    = capctxt->OutHeight;
        ispctxt->In_PitchY    = CAP_MEM_BUF_PITCH;
        ispctxt->In_PitchUV   = CAP_MEM_BUF_PITCH;
        ispctxt->In_Format    = MMP_ISP_IN_NV12;
        ispctxt->In_BufferNum = 0;

        // Signal Process Output Parameter
        for (i = 0; i < gptAVCEncoder[0]->sourceBufCount; ++i)
        {
            ispctxt->Out_AddrY[i] = (MMP_UINT32) gptAVCEncoder[0]->pSourceBufAdrY[i];
            ispctxt->Out_AddrU[i] = (MMP_UINT32) gptAVCEncoder[0]->pSourceBufAdrU[i];
        }

        ispctxt->Out_Width          = gptAVCEncoder[0]->frameWidth = EnPara.EnWidth;
        ispctxt->Out_Height         = gptAVCEncoder[0]->frameHeight = EnPara.EnHeight;
        ispctxt->Out_PitchY         = gptAVCEncoder[0]->framePitchY;
        ispctxt->Out_PitchUV        = gptAVCEncoder[0]->framePitchY;
        ispctxt->Out_Format         = MMP_ISP_OUT_NV12;
        ispctxt->Out_BufferNum      = gptAVCEncoder[0]->sourceBufCount;

        gptJPGEncoder->frameWidth   = EnPara.EnWidth;
        gptJPGEncoder->frameHeight  = EnPara.EnHeight;

        //for sequence process
        ispctxt->EnCapOnflyMode     = MMP_TRUE;
        ispctxt->EnOnflyInFieldMode = capctxt->IsInterlaced;

        mmpIspEnable(MMP_ISP_INTERRUPT);
        mmpIspEnable(MMP_ISP_REMAP_ADDRESS);
        mmpIspDisable(MMP_ISP_DEINTERLACE);
        mmpIspSetSequenceOutputInfo(ispctxt);
        mmpIspSequenceProcess(ispctxt);

        if (capctxt->IsInterlaced)
            gptAVCEncoder[0]->interlaced_frame = MMP_TRUE;
        else
            gptAVCEncoder[0]->interlaced_frame = MMP_FALSE;

        gptAVCEncoder[0]->bitRate     = EnPara.EnBitrate;
        gptAVCEncoder[0]->gopSize     = EnPara.EnGOPSize;
        gptAVCEncoder[0]->EnFrameRate = EnPara.EnFrameRate;
        gptAVCEncoder[0]->bISPOnFly   = gbEnableOnfly;

        //gtAVCEncoderFlow.encoderInfo = index;
        //
        //if (EnPara.EnSkipMode != VIDEO_ENCODER_NO_DROP)
        //    gtAVCEncoderFlow.bSkipFrame = MMP_TRUE;
        //else
        //    gtAVCEncoderFlow.bSkipFrame = MMP_FALSE;

        if (ispctxt)
            free(ispctxt);
    }
#else
    gtAVCEncoderFlow.pfEnDequeue = _IspResultQ_Dequeue;

    //Register IRQ
    mmpCapRegisterIRQ(cap_isr);
    mmpIspRegisterIRQ(isp_mem_isr);

    mmpCapFunEnable(MMP_CAP_INTERRUPT);
    mmpCapFunDisable(MMP_CAP_ONFLY_MODE);

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

        gtIspTransformParm[i].outWidth      = gptAVCEncoder[i]->frameWidth;
        gtIspTransformParm[i].outHeight     = gptAVCEncoder[i]->frameHeight;

        gptAVCEncoder[i]->interlaced_frame  = MMP_FALSE;
        gtIspTransformParm[i].deinterlaceOn = MMP_FALSE;
        gtIspTransformParm[i].bframeDouble  = MMP_FALSE;
    }

    if (capctxt->FrameRate != MMP_CAP_FRAMERATE_24HZ 
     && capctxt->FrameRate != MMP_CAP_FRAMERATE_25HZ 
     && capctxt->FrameRate != MMP_CAP_FRAMERATE_29_97HZ 
     && capctxt->FrameRate != MMP_CAP_FRAMERATE_30HZ
     && capctxt->FrameRate != MMP_CAP_FRAMERATE_23_97HZ)
        EnPara.EnSkipMode = MMP_CAPTURE_SKIP_BY_TWO;
    else
        EnPara.EnSkipMode = MMP_CAPTURE_NO_DROP;

    if (capctxt->FrameRate == MMP_CAP_FRAMERATE_30HZ && capctxt->OutWidth >= 1920)
        EnPara.EnSkipMode = MMP_CAPTURE_SKIP_30FPS_TO_25FPS;
    else if (capctxt->FrameRate == MMP_CAP_FRAMERATE_60HZ && capctxt->OutWidth >= 1920)
        EnPara.EnSkipMode = MMP_CAPTURE_SKIP_60FPS_TO_25FPS;
#endif

    //set capture parameter
    mmpCapParameterSetting(capctxt);
    mmpCapSetSkipMode(EnPara.EnSkipMode);

    //Capture Fire
    mmpCapFire();

lab_end:
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
    avSyncSetDeviceInitFinished(MMP_FALSE);
    mmpIspTerminate();
    mmpCapTerminate();

    return result;
}

static void
isp_onfly_isr(
    void *arg)
{
    CAPTURE_RESULT    tEntry          = {0};
    MMP_BOOL          bIsIdle         = MMP_FALSE;
    MMP_UINT32        dur             = 0;
    static MMP_UINT32 initClock       = 0;
    static MMP_UINT32 totalDur        = 0;
    MMP_UINT16        captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);

    mmpIspClearInterrupt();
    mmpIspOnflyUpdateColorMatrix();

    //Error frame not encode
    if ((captureErrState & 0x0F00) && gtAVCEncoderFlow.captureFrameCount == 0)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "---Error frame not encode---\n");
        return;
    }

    tEntry.frameIndex = mmpIspReturnWrBufIndex();
    tEntry.frameCount = ++gtAVCEncoderFlow.captureFrameCount;

    if (initClock)
    {
        _DO_AV_SYNC(AV_SYNC_TIMER_GET);
        dur = gtAVSyncTimer.frameDuration;
    }
    else
        dur = 0;

    initClock = 1;

    if (tEntry.frameCount <= FRAME_RATE_CHECK_COUNT)
    {
        if (1 == tEntry.frameCount)
        {
            totalDur = 0;

            if (gtAVCEncoderFlow.baseTimeStamp != 0)
            {
                gtAVCEncoderFlow.currTimeStamp     =
                    gtAVCEncoderFlow.baseTimeStamp = gtAVCEncoderFlow.baseTimeStamp
                                                     + dur;
            }
            _DO_AV_SYNC(AV_SYNC_TIMER_INIT);
            return;
        }
        else if (2 == tEntry.frameCount)
            return;
        else if (FRAME_RATE_CHECK_COUNT == tEntry.frameCount)
        {
            totalDur                        += dur;
            dur                              = totalDur / (FRAME_RATE_CHECK_COUNT - 2); // average time of 3 frames

            gtAVCEncoderFlow.detectFrameRate = mmpCapGetOutputFrameRate(&gtAVSyncTimer.frameRate);

            dbg_msg(DBG_MSG_TYPE_INFO, "dur(%d) frame rate(%d) = (%d)\n", dur, gtAVCEncoderFlow.detectFrameRate, gtAVSyncTimer.frameRate);

            if (MMP_CAP_FRAMERATE_UNKNOW != gtAVCEncoderFlow.detectFrameRate)
            {
                switch (gtAVCEncoderFlow.detectFrameRate)
                {
                case MMP_CAP_FRAMERATE_23_97HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_23_97HZ;
                    break;

                case MMP_CAP_FRAMERATE_24HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_24HZ;
                    break;

                case MMP_CAP_FRAMERATE_25HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_25HZ;
                    break;

                case MMP_CAP_FRAMERATE_29_97HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_29_97HZ;
                    break;

                case MMP_CAP_FRAMERATE_30HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_30HZ;
                    break;

                case MMP_CAP_FRAMERATE_50HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_50HZ;
                    break;

                case MMP_CAP_FRAMERATE_59_94HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_59_94HZ;
                    break;

                case MMP_CAP_FRAMERATE_60HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_60HZ;
                    break;

                case MMP_CAP_FRAMERATE_VESA_30HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_VESA_30HZ;
                    break;

                case MMP_CAP_FRAMERATE_VESA_60HZ:
                    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_VESA_60HZ;
                    break;
                }

                gtAVCEncoderFlow.frameRate      = gptAVCEncoder[0]->frameRate;
                gtAVCEncoderFlow.pfToTimeStamp  = _VIDEO_ENCODER_GetTimeStampConverter(gtAVCEncoderFlow.frameRate);
                gtAVCEncoderFlow.frameRateState = FRAME_RATE_SETTING_STATE_DETECTED;

                dbg_msg(DBG_MSG_TYPE_INFO, "Encode w %d h %d b %d deinter %d %d fps %d\n", 
                        gptAVCEncoder[0]->frameWidth,
                        gptAVCEncoder[0]->frameHeight,
                        gptAVCEncoder[0]->bitRate,
                        gtIspTransformParm[0].deinterlaceOn,
                        gtIspTransformParm[0].bframeDouble,
                        gptAVCEncoder[0]->frameRate);
                //mmpAVCEncodeCreateHdr(gptAVCEncoder[0]);
            }

            avSyncSetVideoStable(MMP_TRUE);
        }
        else
        {
            totalDur += dur;
            return;
        }
    }

    if (gtAVCEncoderFlow.pfToTimeStamp)
    {
        tEntry.timeStamp             = gtAVCEncoderFlow.currTimeStamp
                                     = gtAVCEncoderFlow.baseTimeStamp
                                       + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount);

        if (tEntry.frameCount == FRAME_RATE_CHECK_COUNT)
            _DO_AV_SYNC(AV_SYNC_DO_SYNC_INIT);

        if ((captureErrState & 0x0F00) == 0)
        {
            if (tEntry.frameCount > FRAME_RATE_CHECK_COUNT)
            {
                _DO_AV_SYNC(AV_SYNC_DO_SYNC);

                if (tEntry.frameCount != gtAVCEncoderFlow.captureFrameCount)
                {
                    return;
                }
            }
        }

        avSyncSetCurrentTime(tEntry.timeStamp);
    }

    //Error frame not encode
    if (captureErrState & 0x0F00)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "---Error frame not encode---\n");
        return;
    }

    if (!gtEnAVEngine)
        return;

    if (FRAME_RATE_SETTING_STATE_SET == gtAVCEncoderFlow.frameRateState)
    {
        tEntry.bAVCVideo = gbJPEGInRECORD ? MMP_FALSE : MMP_TRUE;

        if (!_Chk_Skip_Frame())
            _CaptureResultQ_Enqueue(&tEntry);
    }

    if (gbJPEGInRECORD == MMP_FALSE)
    {
        if (MMP_SUCCESS == mmpAVCEncodeQuery(AVC_ENCODER_ENGINE_IDLE, (MMP_UINT32 *)&bIsIdle)
            && bIsIdle)
        {
            _DoEncode();
        }
    }
    else
    {
        if (mmpJPGEncodeQueryIdle())
        {
            _DoJPEGEncode();
        }
    }
}

static void
cap_isr(
    void *arg)
{
    CAPTURE_RESULT      tEntry          = {0};
    MMP_BOOL            bIsIdle         = MMP_FALSE;
    MMP_UINT32          dur             = 0;
    static MMP_UINT32   initClock       = 0;
    static MMP_UINT32   totalDur        = 0;
    MMP_UINT16          captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);
    MMP_UINT16          i;
    volatile MMP_UINT16 value;
    MMP_CAP_SHARE       *capctxt        = MMP_NULL;
    MMP_UINT32          idx;
    MMP_BOOL            bSkipFrame      = MMP_FALSE;

    //disable Interrupt
    mmpCapEnableInterrupt(MMP_FALSE);
    //Clear Interrupt
    //mmpCapClearInterrupt();

    //Error frame not encode
    if ((captureErrState & 0x0F00) && gtAVCEncoderFlow.captureFrameCount == 0)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "---Error frame not encode---\n");
        return;
        //goto end;
    }

    tEntry.frameIndex = mmpCapReturnWrBufIndex();
    tEntry.frameCount = ++gtAVCEncoderFlow.captureFrameCount;

    if (initClock)
    {
        _DO_AV_SYNC(AV_SYNC_TIMER_GET);
        dur = gtAVSyncTimer.frameDuration;
    }
    else
        dur = 0;

    initClock = 1;

    if (tEntry.frameCount <= FRAME_RATE_CHECK_COUNT)
    {
        if (1 == tEntry.frameCount)
        {
            totalDur = 0;

            if (gtAVCEncoderFlow.baseTimeStamp != 0)
            {
                gtAVCEncoderFlow.currTimeStamp     =
                    gtAVCEncoderFlow.baseTimeStamp = gtAVCEncoderFlow.baseTimeStamp
                                                     + dur;
            }
            _DO_AV_SYNC(AV_SYNC_TIMER_INIT);
            goto end;
        }
        else if (2 == tEntry.frameCount)
            goto end;
        else if (FRAME_RATE_CHECK_COUNT == tEntry.frameCount)
        {
            totalDur                        += dur;
            dur                              = totalDur / (FRAME_RATE_CHECK_COUNT - 2);

            gtAVCEncoderFlow.detectFrameRate = mmpCapGetOutputFrameRate(&gtAVSyncTimer.frameRate);

            dbg_msg(DBG_MSG_TYPE_INFO, "dur(%d) frame rate(%d) = (%d)\n", dur, gtAVCEncoderFlow.detectFrameRate, gtAVSyncTimer.frameRate);

            if (MMP_CAP_FRAMERATE_UNKNOW != gtAVCEncoderFlow.detectFrameRate)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "Frame rate %d\n", gtAVCEncoderFlow.detectFrameRate);
                switch (gtAVCEncoderFlow.detectFrameRate)
                {
                case MMP_CAP_FRAMERATE_23_97HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_23_97HZ;
                        //gptAVCEncoder[i]->gopSize = 24 * 2;
                    }
                    break;

                case MMP_CAP_FRAMERATE_24HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_24HZ;
                        //gptAVCEncoder[i]->gopSize = 24 * 2;
                    }
                    break;

                case MMP_CAP_FRAMERATE_25HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        if (gtIspTransformParm[i].bframeDouble)
                        {
                            gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_50HZ;
                            //gptAVCEncoder[i]->gopSize = 50 * 2;
                        }
                        else
                        {
                            gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_25HZ;
                            //gptAVCEncoder[i]->gopSize = 25 * 2;
                        }
                    }
                    break;

                case MMP_CAP_FRAMERATE_50HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_50HZ;
                        //gptAVCEncoder[i]->gopSize = 50 * 2;
                    }
                    break;

                case MMP_CAP_FRAMERATE_30HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        if (gtIspTransformParm[i].bframeDouble)
                        {
                            gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_60HZ;
                            //gptAVCEncoder[i]->gopSize = 60 * 2;
                        }
                        else
                        {
                            gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_30HZ;
                            //gptAVCEncoder[i]->gopSize = 30 * 2;
                        }
                    }
                    break;

                case MMP_CAP_FRAMERATE_60HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_60HZ;
                        //gptAVCEncoder[i]->gopSize = 60 * 2;
                    }
                    break;

                case MMP_CAP_FRAMERATE_29_97HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        if (gtIspTransformParm[i].bframeDouble)
                        {
                            gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_59_94HZ;
                            //gptAVCEncoder[i]->gopSize = 60 * 2;
                        }
                        else
                        {
                            gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_29_97HZ;
                            //gptAVCEncoder[i]->gopSize = 30 * 2;
                        }
                    }
                    break;

                case MMP_CAP_FRAMERATE_59_94HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_59_94HZ;
                        //gptAVCEncoder[i]->gopSize = 60 * 2;
                    }
                    break;

                case MMP_CAP_FRAMERATE_VESA_30HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_VESA_30HZ;
                        //gptAVCEncoder[i]->gopSize = 30 * 2;
                    }
                    break;

                case MMP_CAP_FRAMERATE_VESA_60HZ:
                    for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    {
                        gptAVCEncoder[i]->frameRate = AVC_FRAME_RATE_VESA_60HZ;
                        //gptAVCEncoder[i]->gopSize = 60 * 2;
                    }
                    break;
                }
                gtAVCEncoderFlow.frameRate      = gptAVCEncoder[0]->frameRate;
                gtAVCEncoderFlow.pfToTimeStamp  = _VIDEO_ENCODER_GetTimeStampConverter(gtAVCEncoderFlow.frameRate);
                gtAVCEncoderFlow.frameRateState = FRAME_RATE_SETTING_STATE_DETECTED;

                for (i = 0; i < ENCODE_STREAM_NUM; i++)
                    dbg_msg(DBG_MSG_TYPE_INFO, "Encode w %d h %d b %d deinter %d %d fps %d\n", gptAVCEncoder[i]->frameWidth,
                            gptAVCEncoder[i]->frameHeight,
                            gptAVCEncoder[i]->bitRate,
                            gtIspTransformParm[i].deinterlaceOn,
                            gtIspTransformParm[i].bframeDouble,
                            gptAVCEncoder[i]->frameRate);
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
        if (gtIspTransformParm[0].bframeDouble)
            tEntry.timeStamp             = gtAVCEncoderFlow.currTimeStamp
                                         = gtAVCEncoderFlow.baseTimeStamp
                                           + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount * 2 - 1);
        else
            tEntry.timeStamp             = gtAVCEncoderFlow.currTimeStamp
                                         = gtAVCEncoderFlow.baseTimeStamp
                                           + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount);

        if (tEntry.frameCount == FRAME_RATE_CHECK_COUNT)
            _DO_AV_SYNC(AV_SYNC_DO_SYNC_INIT);

        if ((captureErrState & 0x0F00) == 0)
        {
            if (tEntry.frameCount > FRAME_RATE_CHECK_COUNT)
            {
                _DO_AV_SYNC(AV_SYNC_DO_SYNC);

                if (tEntry.frameCount != gtAVCEncoderFlow.captureFrameCount)
                {
                    goto end;
                }
            }
        }

        avSyncSetCurrentTime(tEntry.timeStamp);
    }

    //Error frame not encode
    if (captureErrState & 0x0F00)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "---Error frame not encode---\n");
        return;
        //goto end;
    }

    if (!gtEnAVEngine)
        goto end;

    if (FRAME_RATE_SETTING_STATE_SET == gtAVCEncoderFlow.frameRateState)
    {
        for (tEntry.resIdx = 0; tEntry.resIdx < ISP_TRANSFORM_NUM; tEntry.resIdx++)
        {
            tEntry.bAVCVideo = gbJPEGInRECORD ? MMP_FALSE : MMP_TRUE;

            bSkipFrame       = _Chk_Skip_Frame();

            tEntry.bTopField = MMP_TRUE;

            if (!bSkipFrame)
                _CaptureResultQ_Enqueue(&tEntry);

            if (gtIspTransformParm[tEntry.resIdx].bframeDouble)
            {
                bSkipFrame = _Chk_Skip_Frame();

                if (gtAVCEncoderFlow.pfToTimeStamp)
                {
                    tEntry.timeStamp             = gtAVCEncoderFlow.currTimeStamp
                                                 = gtAVCEncoderFlow.baseTimeStamp
                                                   + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount * 2);
                }
                tEntry.bTopField = MMP_FALSE;

                if (!bSkipFrame)
                    _CaptureResultQ_Enqueue(&tEntry);
            }
        }
    }

    if (mmpIspIsEngineIdle())
    {
        _DoIsp();
    }

end:
    mmpCapEnableInterrupt(MMP_TRUE);
}

static void
isp_mem_isr(
    void *arg)
{
    MMP_BOOL bIsIdle = MMP_FALSE;

    mmpIspClearInterrupt();

#ifdef _DEBUG_ISP_RUN_TIME
    durIsp = (mmpTimerReadCounter(TIMER_NUM) - initClockIsp);
    dbg_msg(DBG_MSG_TYPE_INFO, "isp run time = %d ms\n", (durIsp / 80000));
#endif

    _IspResultQ_Enqueue(&gtIspFireEntry);

    if (gbJPEGInRECORD == MMP_FALSE)
    {
        if (MMP_SUCCESS == mmpAVCEncodeQuery(AVC_ENCODER_ENGINE_IDLE, (MMP_UINT32 *)&bIsIdle)
            && bIsIdle)
        {
            _DoEncode();
        }
    }
    else
    {
        if (mmpJPGEncodeQueryIdle())
        {
            _DoJPEGEncode();
        }
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
    gbAVCEncodeFire = MMP_FALSE;

    if (mmpAVCEncodeGetStream(gptAVCEncoder[gEncodeIdx], &streamLen, &bFrmEnd) == 0 && bFrmEnd == MMP_TRUE)
    {
        MMP_UINT8            *pData         = gptAVCEncoder[gEncodeIdx]->pStreamBufAdr[gptAVCEncoder[gEncodeIdx]->streamBufSelect];
        MMP_UINT             dataSize       = streamLen;

        QUEUE_MGR_ERROR_CODE errorCode      = QUEUE_NO_ERROR;
        VIDEO_SAMPLE         *ptVideoSample = MMP_NULL;

        errorCode = (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfGetFree(
            gtVideoEncoderHandle.tStreamMuxHandle.queueId,
            (void **) &ptVideoSample);

        if (errorCode == QUEUE_NO_ERROR)
        {
            ptVideoSample->frameCount                        = gtAVCEncoderFlow.encodedFrameCount;
            ptVideoSample->timeStamp                         = gtAVCEncoderFlow.encodedTimeStamp;
            ptVideoSample->pData                             = pData;
            ptVideoSample->dataSize                          = dataSize;
            ptVideoSample->InstanceNum                       = gEncodeIdx;

            ptVideoSample->frameRate                         = gptAVCEncoder[gEncodeIdx]->frameRate;
            ptVideoSample->EnFrameRate                       = gptAVCEncoder[gEncodeIdx]->EnFrameRate;
            ptVideoSample->width                             = gptAVCEncoder[gEncodeIdx]->frameWidth;
            ptVideoSample->height                            = gptAVCEncoder[gEncodeIdx]->frameHeight;
            ptVideoSample->frameType                         = (gptAVCEncoder[gEncodeIdx]->bIFrame)
                                                               ? VIDEO_FRAME_TYPE_I
                                                               : VIDEO_FRAME_TYPE_P;
            ptVideoSample->bitRate                           = gptAVCEncoder[gEncodeIdx]->bitRate;
            ptVideoSample->binterlaced_frame                 = gptAVCEncoder[gEncodeIdx]->interlaced_frame;
    #ifdef ENABLE_MENCODER
            ptVideoSample->AVCDecoderConfigurationRecord     = gptAVCEncoder[gEncodeIdx]->pHdrBufAddr[0];
            ptVideoSample->AVCDecoderConfigurationRecordSize = gptAVCEncoder[gEncodeIdx]->ParaSetHdrSize[0];
    #endif
            ptVideoSample->bAVCVideo                         = MMP_TRUE;

            or32_invalidate_cache(pData, dataSize);
            (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfSetReady(
                gtVideoEncoderHandle.tStreamMuxHandle.queueId,
                (void **) &ptVideoSample);
        }
    }
    else
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "Encoder_ISR Error : stremLen %d\n", streamLen);
        mmpAVCEncodeSWRest();
    }

    _DoEncode();
}

static void
jpeg_encoder_isr(
    void)
//void* arg)
{
    MMP_UINT32           streamLen;
    MMP_BOOL             bFrmEnd;
    MMP_UINT8            *pData         = gptJPGEncoder->pStreamBufAdr[gptJPGEncoder->streamBufSelect];
    MMP_UINT             dataSize;
    QUEUE_MGR_ERROR_CODE errorCode      = QUEUE_NO_ERROR;
    VIDEO_SAMPLE         *ptVideoSample = MMP_NULL;

    mmpJPGEncodeClearInterrupt();
    gbJPGEncodeFire = MMP_FALSE;

    if (mmpJPGEncodeGetStream(&streamLen, &bFrmEnd, gptJPGEncoder) == 0 && bFrmEnd == MMP_TRUE)
    {
        dataSize  = streamLen;

        errorCode = (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfGetFree(
            gtVideoEncoderHandle.tStreamMuxHandle.queueId,
            (void **) &ptVideoSample);

        if (errorCode == QUEUE_NO_ERROR)
        {
            ptVideoSample->frameCount                        = gtAVCEncoderFlow.encodedFrameCount;
            ptVideoSample->timeStamp                         = gtAVCEncoderFlow.encodedTimeStamp;
            ptVideoSample->pData                             = pData;
            ptVideoSample->dataSize                          = dataSize;
            ptVideoSample->InstanceNum                       = gEncodeIdx;
#ifdef ENABLE_MENCODER
            ptVideoSample->AVCDecoderConfigurationRecord     = gptJPGEncoder->pHdrBufAddr;
            ptVideoSample->AVCDecoderConfigurationRecordSize = gptJPGEncoder->ParaSetHdrSize;
#endif
            ptVideoSample->bAVCVideo                         = MMP_FALSE;

            or32_invalidate_cache(pData, dataSize);

            (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfSetReady(
                gtVideoEncoderHandle.tStreamMuxHandle.queueId,
                (void **) &ptVideoSample);
        }
    }
    else
    {
        dataSize = streamLen;
        //errorCode = (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfGetFree(
        //    gtVideoEncoderHandle.tStreamMuxHandle.queueId,
        //    (void**) &ptVideoSample);
        //
        //if (errorCode == QUEUE_NO_ERROR)
        //{
        //    ptVideoSample->frameCount   = gtAVCEncoderFlow.encodedFrameCount;
        //    ptVideoSample->timeStamp    = gtAVCEncoderFlow.encodedTimeStamp;
        //    ptVideoSample->pData        = pData;
        //    ptVideoSample->dataSize     = 0;
        //    ptVideoSample->InstanceNum  = gEncodeIdx;
        //    ptVideoSample->bAVCVideo    = MMP_FALSE;
        //
        //    //or32_invalidate_cache(pData, dataSize);
        //
        //    (gtVideoEncoderHandle.tStreamMuxHandle.ptQueueHandle)->pfSetReady(
        //        gtVideoEncoderHandle.tStreamMuxHandle.queueId,
        //        (void**) &ptVideoSample);
        //}
        gbJPGEncodeFire = MMP_TRUE;
        //printf("JPEG_ISR Not Idle : stremLen %d\n", streamLen);
    }

    //_DoJPEGEncode();
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

static void
_DO_AV_SYNC(
    AV_SYNC_STATE state)
{
    static MMP_UINT32 bInitClock  = MMP_FALSE;
    MMP_UINT32        curSYSClock;
    static MMP_UINT32 preSYSClock;
    static MMP_UINT32 accSYSTime  = 0;
    static MMP_UINT32 accSYSClock = 0;
    static MMP_UINT32 curSYSTimer;
    static MMP_UINT32 preSYSTimer;
    static MMP_UINT32 curAudTime;
    MMP_UINT32        aud         = 0;
    static MMP_UINT32 preAudTime  = 0;
    static MMP_INT32  accAudTime  = 0;
    MMP_UINT32        sysDiff, audDiff;
    static MMP_INT32  diff, diff0, curPTSDiff;
    static MMP_INT32  prePTSDiff  = 0;
    static MMP_INT32  initPTSDiff = 0;
    static MMP_UINT16 pllSDMValue;
    static MMP_UINT16 initClkMax;
    static MMP_UINT16 initClkMin;
    static MMP_BOOL   initAVSync = MMP_TRUE;
    MMP_UINT32        frameCount = gtAVCEncoderFlow.captureFrameCount;
    MMP_UINT32        samplerate = gtAVSyncTimer.audSampleRate;

    if (state == AV_SYNC_TIMER_INIT)
    {
        mmpTimerCtrlDisable(TIMER_NUM, MMP_TIMER_EN);
        mmpTimerResetCounter(TIMER_NUM);
        mmpTimerCtrlEnable(TIMER_NUM, MMP_TIMER_EN);
        mmpAVSyncCounterCtrl((AUDIO_COUNTER_SEL | I2S_SOURCE_SEL), 0);
        mmpAVSyncCounterReset(AUDIO_COUNTER_CLEAR);
        bInitClock = MMP_TRUE;

        PalMemset(&gtAVSyncTimer, 0, sizeof(gtAVSyncTimer));

        preSYSClock = accSYSClock = preSYSTimer = preAudTime = 0;
        prePTSDiff  = initPTSDiff = 0;

        accSYSTime  = accAudTime = gtAVCEncoderFlow.baseTimeStamp;
        dbg_msg(DBG_MSG_TYPE_INFO, "AV_SYNC_TIMER_INIT -- baseTime = %d\n", gtAVCEncoderFlow.baseTimeStamp);
    }

    if (state == AV_SYNC_TIMER_GET && bInitClock)
    {
        curSYSClock = mmpTimerReadCounter(TIMER_NUM);
        mmpAVSyncCounterLatch(AUDIO_COUNTER_LATCH);

        if (samplerate != 0)
            aud = ((MMP_UINT64)mmpAVSyncCounterRead(AUDIO_COUNTER_SEL) * 1000) / samplerate;

        //SYSTimer
        if (curSYSClock < preSYSClock)      //(4*1024*1024*1024)/80000 = 53687......7296
        {
            accSYSClock += 7296;
            if (accSYSClock > 80000)
            {
                accSYSTime  += 53688;
                accSYSClock -= 80000;
            }
            else
            {
                accSYSTime += 53687;
            }
            curSYSTimer = ((MMP_UINT64)accSYSClock + curSYSClock) / 80000 + accSYSTime;
        }
        else
        {
            curSYSTimer = ((MMP_UINT64)accSYSClock + curSYSClock) / 80000 + accSYSTime;
        }
        preSYSClock = curSYSClock;

        //Audio Timer
        curAudTime  = aud + accAudTime;
        sysDiff     = curSYSTimer - preSYSTimer;
        audDiff     = curAudTime - preAudTime;

        diff        = sysDiff - audDiff;

        if (diff > 1)
        {
            accAudTime += diff;
            curAudTime  = aud + accAudTime;
        }
        else if (diff < -1)
        {
            mmpAVSyncCounterReset(AUDIO_COUNTER_CLEAR);
            curAudTime = preAudTime + sysDiff;
            accAudTime = curAudTime;
        }
        else
        {
            preSYSTimer = curSYSTimer;
            preAudTime  = curAudTime;
        }

        gtAVSyncTimer.frameDuration = curAudTime - gtAVSyncTimer.curAudTime;
        gtAVSyncTimer.curAudTime    = curAudTime;

        //dbg_msg(DBG_MSG_TYPE_INFO, "timer %u aud %d diff %d ab %d acc %d fc %d PTS %d D %d SR %d\n", curSYSTimer, curAudTime, diff, aud, accAudTime, frameCount, gtAVCEncoderFlow.currTimeStamp, gtAVCEncoderFlow.currTimeStamp - curAudTime, samplerate);
    }

    curPTSDiff = gtAVCEncoderFlow.currTimeStamp - gtAVSyncTimer.curAudTime;

    if (state == AV_SYNC_DO_SYNC_INIT && bInitClock)
    {
        if (gtAVSyncTimer.frameRate != 0)
            gtAVSyncTimer.framePeriod = 1000000 / gtAVSyncTimer.frameRate;

        initPTSDiff = curPTSDiff;

        dbg_msg(DBG_MSG_TYPE_INFO, "AV_SYNC_DO_SYNC_INIT Vid %d  Aud %d FR %d FP %d initDiff %d\n", gtAVCEncoderFlow.currTimeStamp,  gtAVSyncTimer.curAudTime, gtAVSyncTimer.frameRate, gtAVSyncTimer.framePeriod, initPTSDiff);
    }

    if (state == AV_SYNC_DO_SYNC && bInitClock)
    {
        //dbg_msg(DBG_MSG_TYPE_INFO, "timer %u aud %d diff %d ab %d acc %d fc %d PTS %d D %d SR %d\n", curSYSTimer, curAudTime, diff, aud, accAudTime, frameCount, gtAVCEncoderFlow.currTimeStamp, gtAVCEncoderFlow.currTimeStamp - curAudTime, samplerate);

        if (gtDevice_ID != CAPTURE_DEV_HDMIRX)
        {
            if (avSyncIsAudioInitFinished() && frameCount >= 20)
            {
                if (initAVSync)
                {
                    HOST_ReadRegister(0xC6, (MMP_UINT16 *)&pllSDMValue);
                    pllSDMValue = (pllSDMValue & 0x07FF);
                    initClkMax  = pllSDMValue + 2;
                    initClkMin  = pllSDMValue - 2;
                    initAVSync  = MMP_FALSE;
                    initPTSDiff = curPTSDiff;
                }
                else
                {
                    diff = curPTSDiff - prePTSDiff;

                    if (diff > 2 || diff < -2)
                    {
                        initPTSDiff = curPTSDiff;
                    }
                    else
                    {
                        diff = curPTSDiff - initPTSDiff;

                        if (diff > 3)
                        {
                            if (pllSDMValue < initClkMax)
                            {
                                pllSDMValue = (pllSDMValue & 0x07FF) + 1;
                                HOST_WriteRegisterMask(0xC6, pllSDMValue, 0x07FF);
                                initPTSDiff = curPTSDiff;
                                dbg_msg(DBG_MSG_TYPE_INFO, "adjust +1 %x diff %d\n", pllSDMValue, curPTSDiff);
                            }
                        }
                        else if (diff < -3)
                        {
                            if (pllSDMValue > initClkMin)
                            {
                                pllSDMValue = (pllSDMValue & 0x07FF) - 1;
                                HOST_WriteRegisterMask(0xC6, pllSDMValue, 0x07FF);
                                initPTSDiff = curPTSDiff;
                                dbg_msg(DBG_MSG_TYPE_INFO, "adjust -1 %x diff %d\n", pllSDMValue, curPTSDiff);
                            }
                        }
                    }
                }
                prePTSDiff = curPTSDiff;
            }
            else
                initAVSync = MMP_TRUE;
        }
        else
        {
            if (gtAVSyncTimer.frameRate == 0)
                return;

            // check PTS difference
            diff0 = curPTSDiff - initPTSDiff;

            if (abs(diff0) >= gtAVSyncTimer.framePeriod)
            {
                if (diff0 < 0)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "Time error -- Modified PTS : add (%d) fc %d  Cur %d Init %d\n", diff0, frameCount, curPTSDiff, initPTSDiff);

                    curPTSDiff                     -= diff0;
                    initPTSDiff                     = curPTSDiff;
                    gtAVCEncoderFlow.baseTimeStamp -= diff0;
                }
                else
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "Time error -- Modified PTS : add (%d) fc %d  Cur %d Init %d\n", diff0, frameCount, curPTSDiff, initPTSDiff);

                    curPTSDiff  = initPTSDiff;
                    initPTSDiff = curPTSDiff;

                    gtAVCEncoderFlow.captureFrameCount--;
                }
            }
        }
    }
}

static MMP_BOOL
_Chk_Skip_Frame(
    void)
{
    MMP_UINT32 skipNum;
    MMP_BOOL   bSkipFrm = MMP_FALSE;

    if (!gbJPEGInRECORD)
    {
        if (gtAVCEncoderFlow.bSkipAVCVideo)
        {
            bSkipFrm = MMP_TRUE;
            goto End;
        }

        if (gtAVCEncoderFlow.frameRateDiff == 0)
        {
            bSkipFrm = MMP_FALSE;
            goto End;
        }

        skipNum = (gtAVCEncoderFlow.ispFrameRate * gtAVCEncoderFlow.skipCount + (gtAVCEncoderFlow.frameRateDiff >> 1))
                  / gtAVCEncoderFlow.frameRateDiff;

        if (gtAVCEncoderFlow.frameCount++ == skipNum)
        {
            if (gtAVCEncoderFlow.skipCount++ == gtAVCEncoderFlow.frameRateDiff)
                gtAVCEncoderFlow.skipCount = 1;

            if (gtAVCEncoderFlow.frameCount == (gtAVCEncoderFlow.ispFrameRate + 1))
                gtAVCEncoderFlow.frameCount = 1;

            bSkipFrm = MMP_TRUE;
        }
        else
            bSkipFrm = MMP_FALSE;
    }
    else
    {
        if (gtAVCEncoderFlow.bJPGFirstFrm)
            gtAVCEncoderFlow.frameCount = 1;

        if (gtAVCEncoderFlow.bOneShot || (gtAVCEncoderFlow.frameCount++ % (gtAVCEncoderFlow.ispFrameRate * gtAVCEncoderFlow.encodingPeriod)) == 1)
            bSkipFrm = MMP_FALSE;
        else
            bSkipFrm = MMP_TRUE;

        gtAVCEncoderFlow.bJPGFirstFrm  = MMP_FALSE;
        gtAVCEncoderFlow.bSkipAVCVideo = MMP_TRUE;
    }
End:

    //dbg_msg(DBG_MSG_TYPE_INFO, "IsSkip(%d) FC %d SkipCount %d SkipNum %d\n", bSkipFrm, gtAVCEncoderFlow.frameCount, gtAVCEncoderFlow.skipCount, skipNum);
    return bSkipFrm;
}

static void
_OnFly_PowerSaving(MMP_BOOL enable)
{
    if (enable)
    {
        // DRAM arbiter
        HOST_WriteRegister(0x0378, 0x0310);
        HOST_WriteRegister(0x0384, 0x0802);
    }
    else
    {
        // DRAM arbiter
        HOST_WriteRegister(0x0378, 0x0302);
        HOST_WriteRegister(0x0384, 0x0810);
    }
}

void
_HDMIRXDiableHPD(
    MMP_BOOL enable)
{
    if (enable)
    {
        GPIO_SetState(HDMIRX_REINT_GPIO, GPIO_STATE_HI);
        GPIO_SetMode(HDMIRX_REINT_GPIO, GPIO_MODE_OUTPUT);
        ithGpioSetMode(HDMIRX_REINT_GPIO, ITH_GPIO_MODE0);
    }
    else
    {
        ithGpioCtrlDisable(HDMIRX_REINT_GPIO, ITH_GPIO_PULL_UP); //pull down
        ithGpioCtrlEnable(HDMIRX_REINT_GPIO, ITH_GPIO_PULL_ENABLE);
        GPIO_SetMode(HDMIRX_REINT_GPIO, GPIO_MODE_INPUT);
        ithGpioSetMode(HDMIRX_REINT_GPIO, ITH_GPIO_MODE0);
    }
}

void
_CalcAspectRatio(
    ASPECT_RATIO_PARAMETER *pARParm)
{
    MMP_FLOAT  calcFloat;
    MMP_UINT16 calcInt;

    if (pARParm->inWidth == pARParm->outWidth && pARParm->inHeight == pARParm->outHeight)
    {
        pARParm->srcARPosX   = 0;
        pARParm->srcARPosY   = 0;
        pARParm->srcARWidth  = pARParm->inWidth;
        pARParm->srcARHeight = pARParm->inHeight;

        pARParm->dstARPosX   = 0;
        pARParm->dstARPosY   = 0;
        pARParm->dstARWidth  = pARParm->outWidth;
        pARParm->dstARHeight = pARParm->outHeight;
    }
    else if (pARParm->aspectRatioMode == AR_FULL)
    {
        pARParm->srcARPosX   = 0;
        pARParm->srcARPosY   = 0;
        pARParm->srcARWidth  = pARParm->inWidth;
        pARParm->srcARHeight = pARParm->inHeight;

        pARParm->dstARPosX   = 0;
        pARParm->dstARPosY   = 0;
        pARParm->dstARWidth  = pARParm->outWidth;
        pARParm->dstARHeight = pARParm->outHeight;
    }
    else if (pARParm->aspectRatioMode == AR_LETTER_BOX)
    {
        pARParm->srcARPosX   = 0;
        pARParm->srcARPosY   = 0;
        pARParm->srcARWidth  = pARParm->inWidth;
        pARParm->srcARHeight = pARParm->inHeight;

        calcFloat            = ((pARParm->inWidth * pARParm->outHeight + (pARParm->inWidth >> 1)) / (MMP_FLOAT)pARParm->inHeight);
        calcInt              = (((MMP_UINT16)calcFloat) >> 2) << 2;
        if (calcInt > pARParm->outWidth)
            pARParm->dstARWidth = pARParm->outWidth;
        else
            pARParm->dstARWidth = calcInt;
        pARParm->dstARHeight = pARParm->outHeight;
        pARParm->dstARPosX   = ((pARParm->outWidth - pARParm->dstARWidth) >> 2) << 1;
        pARParm->dstARPosY   = 0;
    }
    else if (pARParm->aspectRatioMode == AR_PAN_SCAN)
    {
        calcFloat           = ((pARParm->outHeight * pARParm->inWidth + (pARParm->outWidth >> 1)) / (MMP_FLOAT)pARParm->outWidth);
        calcInt             = (((MMP_UINT16)calcFloat) >> 2) << 2;
        pARParm->srcARWidth = pARParm->inWidth;
        if (calcInt > pARParm->inHeight)
            pARParm->srcARHeight = pARParm->inHeight;
        else
            pARParm->srcARHeight = calcInt;
        pARParm->srcARPosX   = 0;
        pARParm->srcARPosY   = ((pARParm->inHeight - pARParm->srcARHeight) >> 2) << 1;

        pARParm->dstARPosX   = 0;
        pARParm->dstARPosY   = 0;
        pARParm->dstARWidth  = pARParm->outWidth;
        pARParm->dstARHeight = pARParm->outHeight;
    }
}

static
void CopyJPEGInfo(void)
{
    MMP_UINT32 i = 0;

    // copy memory info to JPEG structure
    gptJPGEncoder->frameWidth  = gptAVCEncoder[0]->frameWidth;
    gptJPGEncoder->frameHeight = gptAVCEncoder[0]->frameHeight;
    gptJPGEncoder->framePitchY = gptAVCEncoder[0]->framePitchY;

    gptJPGEncoder->pHdrBufAddr = gptAVCEncoder[0]->pHdrBufAddr[2];

    //for (i=0; i < 8; i++)
    //    gptJPGEncoder->pStreamBufAdr[i] = gptAVCEncoder[0]->pStreamBufAdr[i];
    for (i = 0; i < 5; i++)
        gptJPGEncoder->pSourceBufAdrY[i] = gptAVCEncoder[0]->pSourceBufAdrY[i];
    for (i = 0; i < 5; i++)
        gptJPGEncoder->pSourceBufAdrU[i] = gptAVCEncoder[0]->pSourceBufAdrU[i];
    for (i = 0; i < 5; i++)
        gptJPGEncoder->pSourceBufAdrV[i] = gptAVCEncoder[0]->pSourceBufAdrV[i];

    gptJPGEncoder->ParaSetHdrSize   = gptAVCEncoder[0]->ParaSetHdrSize[2];

    gptJPGEncoder->streamBufSize    = 1433600;
    gptJPGEncoder->pStreamBufAdr[0] = gptAVCEncoder[0]->pStreamBufAdr[0];
    gptJPGEncoder->pStreamBufAdr[1] = gptJPGEncoder->pStreamBufAdr[0] + gptJPGEncoder->streamBufSize;
    gptJPGEncoder->pStreamBufAdr[2] = gptJPGEncoder->pStreamBufAdr[1] + gptJPGEncoder->streamBufSize;
    gptJPGEncoder->pStreamBufAdr[3] = gptJPGEncoder->pStreamBufAdr[2] + gptJPGEncoder->streamBufSize;
    gptJPGEncoder->pStreamBufAdr[4] = gptJPGEncoder->pStreamBufAdr[3] + gptJPGEncoder->streamBufSize;

    gptJPGEncoder->streamBufCount   = 5;
}

static void
_JPEG_ENCODER_Open(JPEG_CONTEXT *ptJpgCtx)
{
    CopyJPEGInfo();

    if (mmpJPGEncodeOpen(gptJPGEncoder, ptJpgCtx->quality) != 0)
        printf("[JPG] mmpJPGEncodeOpen Fail\n");

    gtAVCEncoderFlow.bOneShot       = ptJpgCtx->bOneShot;
    gtAVCEncoderFlow.bJPPGFirstFire = MMP_FALSE;
    gtAVCEncoderFlow.bJPGFirstFrm   = MMP_TRUE;
    gtAVCEncoderFlow.bSkipAVCVideo  = MMP_TRUE;
    gbJPEGInRECORD                  = MMP_TRUE;

    if (ptJpgCtx->encodingPeriod == 0)
        gtAVCEncoderFlow.encodingPeriod = 1;
    else
        gtAVCEncoderFlow.encodingPeriod = ptJpgCtx->encodingPeriod;
}