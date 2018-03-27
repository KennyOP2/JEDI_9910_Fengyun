#include "core_interface.h"
#include "task_stream_mux.h"
#include "msg_core.h"
#include "grabber_control.h"
#include "mmp_capture.h"
#include "mmp_hdmirx.h"
#include "mps_control.h"
#include "task_stream_mux.h"
#include "rom_parser.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MaxEnMBNum (1920 / 16) * (1088 / 16) * 30

#if (PROJECT_TYPE == yangke_yk918) || (PROJECT_TYPE == yangke_yk919) || (PROJECT_TYPE == yangke_yk929)
    #define I2C_ADDR 0x51
#elif (PROJECT_TYPE == fpxun)
    #define I2C_ADDR 0xa0
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================
extern gbPC_MODE_ENALBE_RECORD;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static CORE_NOTIFY_EVENT       gNotifyEvent  = CORE_NO_EVENT;
static CORE_NOTIFY_CALLBACK    gpfApCallback = MMP_NULL;
static MMP_BOOL                gbInRecord    = MMP_FALSE;
//static MMP_BOOL                 gtChangeResolution = MMP_FALSE;

static CORE_VERSION_TYPE       gtVersion     = {0};
static CORE_MODULATOR_PARA     gtModulatorPara = {0};
static CAPTURE_VIDEO_SOURCE    gCurCapVidSrc = CAPTURE_VIDEO_SOURCE_UNKNOW;
static MMP_MUX_TYPE            gMuxType      = MMP_TS_MUX;
static MMP_MUTEX               gtMutex;
//static DIGITAL_AUDIO_VOLUME     gDigitalVolume = DIGITAL_AUDIO_DEFAULT;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_CORE_MpsEventReceiveCallBack(
    MPS_CALLBACK_REASON reason,
    MMP_UINT32 data);

static MMP_BOOL
isInterlaceSrc(
    MMP_UINT32 index);

static MMP_UINT16
chkFrameRate(
    MMP_UINT32 index);

static MMP_UINT16
getMaxFrameRate(
    VIDEO_ENCODER_INPUT_INFO index,
    MMP_UINT16 outWidth,
    MMP_UINT16 outHeight);

static void
getWidthHeight(
    MMP_UINT32 index,
    MMP_UINT32 *Width,
    MMP_UINT32 *Height);

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
coreInitialize(
    MMP_MUX_TYPE mux_type)
{
    gMuxType = mux_type;
    if (gMuxType == MMP_TS_MUX)
    {
        PsiSiMgrInit();
    }
    mpsCtrl_Init(mux_type);
    gCurCapVidSrc = CAPTURE_VIDEO_SOURCE_UNKNOW;
}

void
coreTerminate(
    void)
{
    if (gMuxType == MMP_TS_MUX)
    {
        PsiSiMgrTerminate();
    }
    mpsCtrl_Terminate();
}

void
corePlay(
    void)
{
    mpsCtrl_Play(MMP_TRUE);
}

void
coreStop(
    void)
{
    mpsCtrl_Stop(MMP_TRUE);
}

void
coreApiInitialize(
    MMP_MUX_TYPE mux_type)
{
    coreInitialize(mux_type);
}

void
coreApiTerminate(
    void)
{
    coreTerminate();
}

void
coreApiPlay(
    void)
{
    corePlay();
}

void
coreApiStop(
    void)
{
    mmpCapSetDeviceReboot(MMP_TRUE);
    coreStop();
    codec_initialize();
    coreEnableAVEngine(MMP_TRUE);
}

MMP_BOOL(*_coreIsPCConnectMode)(void) = coreIsPCConnectMode_default;
MMP_BOOL coreIsPCConnectMode(void) { return (_coreIsPCConnectMode) ? _coreIsPCConnectMode() :  MMP_FALSE; }
MMP_BOOL coreIsPCConnectMode_default(void)
{
    return MMP_FALSE;
}

void (*_coreInitVideoEnPara)(void) = coreInitVideoEnPara_default;
void coreInitVideoEnPara(void) { if (_coreInitVideoEnPara) _coreInitVideoEnPara(); }
void coreInitVideoEnPara_default(void) {}

void
(*_coreSetVideoEnPara)(
    CAPTURE_VIDEO_SOURCE       videoSrc,
    VIDEO_ENCODER_UPDATE_FLAGS flags,
    VIDEO_ENCODER_INPUT_INFO   index,
    VIDEO_ENCODER_PARAMETER    *ptEnPara) = coreSetVideoEnPara_default;
void
coreSetVideoEnPara(
    CAPTURE_VIDEO_SOURCE       videoSrc,
    VIDEO_ENCODER_UPDATE_FLAGS flags,
    VIDEO_ENCODER_INPUT_INFO   index,
    VIDEO_ENCODER_PARAMETER    *ptEnPara)
{
    if (_coreSetVideoEnPara) _coreSetVideoEnPara(videoSrc, flags, index, ptEnPara);
}
void coreSetVideoEnPara_default(
    CAPTURE_VIDEO_SOURCE       videoSrc,
    VIDEO_ENCODER_UPDATE_FLAGS flags,
    VIDEO_ENCODER_INPUT_INFO   index,
    VIDEO_ENCODER_PARAMETER    *ptEnPara)
{}

void
(*_coreGetVideoEnPara)(
    CAPTURE_VIDEO_SOURCE     videoSrc,
    VIDEO_ENCODER_INPUT_INFO index,
    VIDEO_ENCODER_PARAMETER  *ptEnPara) = coreGetVideoEnPara_default;
void
coreGetVideoEnPara(
    CAPTURE_VIDEO_SOURCE     videoSrc,
    VIDEO_ENCODER_INPUT_INFO index,
    VIDEO_ENCODER_PARAMETER  *ptEnPara)
{
    if (_coreGetVideoEnPara) _coreGetVideoEnPara(videoSrc, index, ptEnPara);
}
void
coreGetVideoEnPara_default(
    CAPTURE_VIDEO_SOURCE     videoSrc,
    VIDEO_ENCODER_INPUT_INFO index,
    VIDEO_ENCODER_PARAMETER  *ptEnPara)
{}

#ifdef ENABLE_MENCODER
MMP_BOOL
coreStartRecord(
    MMP_WCHAR *filePath,
    //MMP_CHAR*               filePath,
    CORE_NOTIFY_CALLBACK pfCallback)
{
    if ((MMP_FALSE == gbInRecord) && filePath)
    {
        // register callback function from UI layer
        // calling for necessary notification
        if (pfCallback)
            gpfApCallback = pfCallback;

        mpsCtrl_StartRecord(filePath, _CORE_MpsEventReceiveCallBack, MMP_TRUE);
    }
    else
        return MMP_FALSE;

    // sync with command, wait for the result of open file
    while (gNotifyEvent != CORE_FILE_OPEN_FAIL && gNotifyEvent != CORE_FILE_OPEN_SUCCESS)
        PalSleep(10);

    if (CORE_FILE_OPEN_FAIL == gNotifyEvent)
    {
        gbInRecord = MMP_FALSE;
    }
    else
    {
        gbInRecord = MMP_TRUE;
    }

    gNotifyEvent = CORE_NO_EVENT;

    return gbInRecord;
}

MMP_BOOL
coreStopRecord(
    void)
{
    if (gbInRecord)
        mpsCtrl_StopRecord(MMP_TRUE);
    gbInRecord = MMP_FALSE;

    return MMP_TRUE;
}
#endif

MMP_BOOL
coreStartJPEGRecord(
    JPEG_CONTEXT *ptJpgCtx)
{
    if (ptJpgCtx)
    {
        mpsCtrl_StartJPEGRecord(ptJpgCtx, MMP_TRUE);
    }
    else
        return MMP_FALSE;

    return MMP_TRUE;
}

MMP_BOOL
coreStopJPEGRecord(
    void)
{
    mpsCtrl_StopJPEGRecord(MMP_FALSE);

    return MMP_TRUE;
}

void
coreGetTsMuxBufferInfo(
    MMP_UINT8 **pBufferStart,
    MMP_UINT32 *bufferSize)
{
    taskStreamMux_GetBufferInfo(pBufferStart, bufferSize);
}

MMP_UINT32
coreGetTsMuxBufferWriteIndex(
    void)
{
    return taskStreamMux_GetWriteIndex();
}

void
coreSetMuxerParameter(
    void *ptPara)
{
    mpsCtrl_SetProperity(MPS_PROPERITY_SET_MUXER_PARAMETER, (MMP_UINT32) ptPara, MMP_TRUE);
}

void
coreSetAudioEncodeParameter(
    void *ptPara)
{
    CORE_AUDIO_ENCODE_PARA *ptAudioEncodePara = (CORE_AUDIO_ENCODE_PARA *) ptPara;
    AUDIO_STREAM_TYPE      audioStreamType    = MPEG_AUDIO;
    GRABBER_CTRL_PARAM     grabberCtrl;

    switch (ptAudioEncodePara->audioEncoderType)
    {
    case MPEG_AUDIO_ENCODER:
        audioStreamType = MPEG_AUDIO;
        break;

    case AAC_AUDIO_ENCODER:
        audioStreamType = AAC;
        break;
    }
    grabberCtrl.flag         = GRABBER_CTRL_AUDIO_BITRATE;
    grabberCtrl.audiobitrate = ptAudioEncodePara->bitRate;
    GrabberControlSetParam(&grabberCtrl);
    PsiSiMgrUpdateAudioEncodeType(audioStreamType);
    coreTsUpdateTable();
    mpsCtrl_SetProperity(MPS_PROPERITY_SET_AUDIO_ENCODE_PARAMETER, (MMP_UINT32) ptPara, MMP_TRUE);
}

void
coreGetAudioEncodeParameter(
    void *ptPara)
{
    GRABBER_CTRL_PARAM     grabberCtrl;
    CORE_AUDIO_ENCODE_PARA *ptaudioencodedata;
    mpsCtrl_GetProperity(MPS_PROPERITY_GET_AUDIO_ENCODE_PARAMETER, (MMP_UINT32) ptPara, MMP_TRUE);
    grabberCtrl.flag           = GRABBER_CTRL_AUDIO_BITRATE;
    GrabberControlGetParam(&grabberCtrl);
    ptaudioencodedata          = (CORE_AUDIO_ENCODE_PARA *)ptPara;
    ptaudioencodedata->bitRate = grabberCtrl.audiobitrate;
}

void
coreSetCaptureSource(
    CAPTURE_VIDEO_SOURCE videoSrc)
{
    CAPTURE_DEVICE_INFO CurCapDevInfo = {0};

    switch (videoSrc)
    {
    case CAPTURE_VIDEO_SOURCE_CVBS:
        CurCapDevInfo.deviceId = MMP_CAP_DEV_ADV7180;
        break;

    case CAPTURE_VIDEO_SOURCE_SVIDEO:
        CurCapDevInfo.deviceId = MMP_CAP_DEV_ADV7180_S;
        break;

    case CAPTURE_VIDEO_SOURCE_YPBPR:
    case CAPTURE_VIDEO_SOURCE_VGA:
        CurCapDevInfo.deviceId = MMP_CAP_DEV_CAT9883;
        break;

    case CAPTURE_VIDEO_SOURCE_HDMI:
    case CAPTURE_VIDEO_SOURCE_DVI:
        CurCapDevInfo.deviceId = MMP_CAP_DEV_HDMIRX;
        break;

    case CAPTURE_VIDEO_SOURCE_SENSOR:
        CurCapDevInfo.deviceId = MMP_CAP_DEV_SENSOR;
        break;
    }

    CurCapDevInfo.videoMode = videoSrc;

    mpsCtrl_SetProperity(MPS_PROPERITY_SET_CAPTURE_DEVICE, (MMP_UINT32)&CurCapDevInfo, MMP_TRUE);

    gCurCapVidSrc = videoSrc;
}

CAPTURE_VIDEO_SOURCE
coreGetCaptureSource(
    void)
{
    return gCurCapVidSrc;
}

void
coreEnableISPOnFly(
    MMP_BOOL bEnableOnly)
{
    mpsCtrl_SetProperity(MPS_PROPERITY_SET_ISP_MODE, bEnableOnly, MMP_TRUE);
}

void
coreEnableAVEngine(
    MMP_BOOL bEnableAVEngine)
{
    mpsCtrl_SetProperity(MPS_PROPERITY_SET_ENABLE_AV_ENGINE, bEnableAVEngine, MMP_TRUE);
}

VIDEO_ENCODER_INPUT_INFO
coreGetInputSrcInfo(
    void)
{
    return mmpCapGetInputSrcInfo();
}

CORE_API void
coreSetRecordMode(
    RECORD_MODE *ptRecMode)
{
    if (!ptRecMode)
        return;

    ptRecMode->bDisableFileSplitter = !!ptRecMode->bDisableFileSplitter;
    mpsCtrl_SetProperity(MPS_PROPERITY_SET_RECORD_MODE, (MMP_UINT32)ptRecMode, MMP_TRUE);
}

MMP_UINT16
coreGetMaxFrameRate(
    VIDEO_ENCODER_INPUT_INFO index,
    MMP_UINT16 outWidth,
    MMP_UINT16 outHeight)
{
    return getMaxFrameRate(index, outWidth, outHeight);
}

MMP_UINT16
coreGetCapFrameRate(
    VIDEO_ENCODER_INPUT_INFO index)
{
    return chkFrameRate(index);
}

MMP_BOOL
coreIsInterlaceSrc(
    VIDEO_ENCODER_INPUT_INFO index)
{
    return isInterlaceSrc(index);
}

MMP_RESULT
coreGetActiveVideoInfo(
    INPUT_VIDEO_SRC_INFO *activeVideo)
{
    MMP_UINT32 index;
    MMP_UINT32 width, height;

    // return 0 if signal unstable
    PalMemset((void *)activeVideo, 0x0, sizeof(INPUT_VIDEO_SRC_INFO));

    index = mmpCapGetInputSrcInfo();

    if (index >= VIDEO_ENCODER_INPUT_INFO_NUM)
        return MMP_RESULT_ERROR;

    getWidthHeight(index, &width, &height);

    activeVideo->vidWidth      = width;
    activeVideo->vidHeight     = height;

    activeVideo->bInterlaceSrc = isInterlaceSrc(index);
    activeVideo->vidFrameRate  = chkFrameRate(index);
    activeVideo->vidDevice     = gCurCapVidSrc;

    return MMP_RESULT_SUCCESS;
}

MMP_BOOL
coreIsVideoStable(void)
{
    return avSyncIsVideoStable();
}

void
coreSetPCModeEnableRecord(MMP_BOOL enable)
{
    if (MMP_NULL == gtMutex)
        gtMutex = PalCreateMutex(MMP_NULL);

    PalWaitMutex(gtMutex, PAL_MUTEX_INFINITE);
    gbPC_MODE_ENALBE_RECORD = enable;
    PalReleaseMutex(gtMutex);
}

void
coreGetPCModeEnableRecord(MMP_BOOL *enable)
{
    if (MMP_NULL == gtMutex)
        gtMutex = PalCreateMutex(MMP_NULL);

    PalWaitMutex(gtMutex, PAL_MUTEX_INFINITE);
    *enable = gbPC_MODE_ENALBE_RECORD;
    PalReleaseMutex(gtMutex);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_CORE_MpsEventReceiveCallBack(
    MPS_CALLBACK_REASON reason,
    MMP_UINT32 data)
{
    if (data) { }

    switch (reason)
    {
    case CALLBACK_REASON_OPEN_FILE_SUCCESS:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_FILE_OPEN_SUCCESS, 0);
            }
            gNotifyEvent = CORE_FILE_OPEN_SUCCESS;
            break;
        }

    case CALLBACK_REASON_OPEN_FILE_FAIL:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_FILE_OPEN_FAIL, 0);
            }
            gNotifyEvent = CORE_FILE_OPEN_FAIL;
            break;
        }

    case CALLBACK_REASON_END_OF_FILE:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_FILE_END_OF_FILE, 0);
            }
            else
            {
                mpsCtrl_Stop(MMP_FALSE);
                mpsCtrl_Close(MMP_FALSE);
            }
            gNotifyEvent = CORE_NO_EVENT;
            break;
        }

    case CALLBACK_REASON_FILE_NO_MORE_STORAGE:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_NO_MORE_STORAGE, 0);
            }
            else
                mpsCtrl_StopRecord(MMP_FALSE);
            break;
        }

    case CALLBACK_REASON_FILE_WRITE_FAIL:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_FILE_WRITE_FAIL, 0);
            }
            else
                mpsCtrl_StopRecord(MMP_FALSE);
            break;
        }

    case CALLBACK_REASON_FILE_CLOSE_FAIL:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_FILE_CLOSE_FAIL, data);
            }
            break;
        }

    case CALLBACK_REASON_FILE_CLOSE_SUCCESS:
        {
            if (gpfApCallback)
            {
                (*gpfApCallback)(CORE_FILE_CLOSE_SUCCESS, data);
            }
            break;
        }
    }
}

void coreRtcInitialize()
{
    mmpExRtcInitialize();
}

MMP_RESULT coreRtcSetTime(MMP_UINT8 hour, MMP_UINT8 min, MMP_UINT8 sec)
{
    return mmpExRtcSetTime(hour, min, sec);
}

MMP_RESULT coreRtcGetTime(MMP_UINT8 *hour, MMP_UINT8 *min, MMP_UINT8 *sec)
{
    return mmpExRtcGetTime(hour, min, sec);
}

MMP_RESULT coreRtcSetDate(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    return mmpExRtcSetDate(year,  month, day);
}

MMP_RESULT coreRtcGetDate(MMP_UINT *year, MMP_UINT *month, MMP_UINT *day)
{
    return mmpExRtcGetDate(year, month, day);
}

MMP_RESULT coreRtcSetDateTime(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day,
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec)
{
    return mmpExRtcSetDateTime(year, month, day,  hour, min, sec);
}

MMP_RESULT
coreRtcGetDateTime(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day,
    MMP_UINT *hour,
    MMP_UINT *min,
    MMP_UINT *sec)
{
    return mmpExRtcGetDateTime(year, month, day, hour, min, sec);
}

void
coreSetMicInVolStep(
    MMP_UINT32 volstep)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag   = GRABBER_CTRL_MIC_VOL;
    grabberCtrl.micVol = volstep;
    GrabberControlSetParam(&grabberCtrl);
}

MMP_UINT32
coreGetMicInVolStep(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_MIC_VOL;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.micVol;
}

void
coreSetLineInBoost(
    LINE_BOOST boost)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag      = GRABBER_CTRL_LINE_BOOST;
    grabberCtrl.lineBoost = boost;
    GrabberControlSetParam(&grabberCtrl);
}

LINE_BOOST
coreGetLineInBoost(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_LINE_BOOST;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.lineBoost;
}

MMP_BOOL
coreGetIsContentProtection(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_HDCP;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.bIsHDCP;
}

void
coreSetAspectRatio(
    INPUT_ASPECT_RATIO mode)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag        = GRABBER_CTRL_ASPECT_RATIO;
    grabberCtrl.aspectRatio = mode;
    GrabberControlSetParam(&grabberCtrl);
}

INPUT_ASPECT_RATIO
coreGetAspectRatio(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_ASPECT_RATIO;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.aspectRatio;
}

INPUT_ASPECT_RATIO
coreGetInputAspectRatio(
    void)
{
    MMP_CAP_SHARE capctxt;

    mmpCapGetInputInfo(&capctxt);

    return capctxt.inputPAR;
}

MMP_UINT32
coreGetHDMIAudioMode(
    void)
{
    MMP_UINT32 mode = 0;

    // [20140122] Vincent note: invalid this function to avoid calling from AP.
    // It's better to create/export another function.

    //    GRABBER_CTRL_PARAM grabberCtrl;
    //
    //    grabberCtrl.flag = GRABBER_CTRL_HDMI_AUDIO_MODE;
    //    GrabberControlGetParam(&grabberCtrl);
    //
    //    switch(grabberCtrl.hdmiAudioMode)
    //    {
    //    case HDMI_AUDIO_MODE_OFF:
    //        mode = 0;
    //        break;
    //
    //    case HDMI_AUDIO_MODE_HBR:
    //        mode = 1;
    //        break;
    //
    //    case HDMI_AUDIO_MODE_DSD:
    //        mode = 2;
    //        break;
    //
    //    case HDMI_AUDIO_MODE_NLPCM:
    //        mode = 3;
    //        break;
    //
    //    case HDMI_AUDIO_MODE_LPCM:
    //        mode = 4;
    //        break;
    //    }

    return mode;
}

void
coreSetSignalLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag      = GRABBER_CTRL_SIGNAL_LED;
    grabberCtrl.SignalLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetSignalLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_SIGNAL_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.SignalLed;
}

void
coreSetSD720PLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag      = GRABBER_CTRL_SD720P_LED;
    grabberCtrl.SD720PLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetSD720PLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_SD720P_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.SD720PLed;
}

void
coreSetHD1080PLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag       = GRABBER_CTRL_HD1080P_LED;
    grabberCtrl.HD1080PLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetHD1080PLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_HD1080P_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.HD1080PLed;
}

void
coreSetRecordLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag      = GRABBER_CTRL_RECORD_LED;
    grabberCtrl.RecordLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetRecordLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_RECORD_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.RecordLed;
}

void
coreSetAnalogLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag      = GRABBER_CTRL_ANALOG_LED;
    grabberCtrl.AnalogLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetAnalogLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_ANALOG_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.AnalogLed;
}

void
coreSetHDMILed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag    = GRABBER_CTRL_HDMI_LED;
    grabberCtrl.HDMILed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetHDMILed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_HDMI_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.HDMILed;
}

void
coreSetBlueLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag    = GRABBER_CTRL_BLUE_LED;
    grabberCtrl.BlueLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetBlueLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_BLUE_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.BlueLed;
}

void
coreSetGreenLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag     = GRABBER_CTRL_GREEN_LED;
    grabberCtrl.GreenLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetGreenLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_GREEN_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.GreenLed;
}

void
coreSetRedLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag   = GRABBER_CTRL_RED_LED;
    grabberCtrl.RedLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetRedLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_RED_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.RedLed;
}

void
coreSetBlingRingLed(
    LED_STATUS flag)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag         = GRABBER_CTRL_BLING_RING_LED;
    grabberCtrl.BlingRingLed = flag;
    GrabberControlSetParam(&grabberCtrl);
}

LED_STATUS
coreGetBlingRingLed(
    void)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag = GRABBER_CTRL_BLING_RING_LED;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.BlingRingLed;
}

void
coreSetVideoColor(
    VIDEO_COLOR_CTRL *pCtrl)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag             = GRABBER_CTRL_VIDEO_COLOR;
    grabberCtrl.COLOR.brightness = pCtrl->brightness;
    grabberCtrl.COLOR.contrast   = pCtrl->contrast;
    grabberCtrl.COLOR.hue        = pCtrl->hue;
    grabberCtrl.COLOR.saturation = pCtrl->saturation;
    GrabberControlSetParam(&grabberCtrl);
}

void
coreGetVideoColor(
    VIDEO_COLOR_CTRL *pCtrl)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag  = GRABBER_CTRL_VIDEO_COLOR;
    GrabberControlGetParam(&grabberCtrl);

    pCtrl->brightness = grabberCtrl.COLOR.brightness;
    pCtrl->contrast   = grabberCtrl.COLOR.contrast;
    pCtrl->hue        = grabberCtrl.COLOR.hue;
    pCtrl->saturation = grabberCtrl.COLOR.saturation;
}

void
coreDisableHDCP(
    MMP_BOOL flag)
{
    mmpHDMIRXDisableHDCP(flag);
    config_set_hdcp_status(flag);
}

MMP_BOOL
coreIsDisableHDCP(
    void)
{
    MMP_BOOL flag;
    config_get_hdcp_status(&flag);
    return flag;
}

MMP_RESULT coreFirmwareUpgrade(MMP_UINT8 *pbuffer, MMP_UINT32 fileSize)
{
#define FILE_ID_SIZE (16)
    MMP_UINT8  *pReadFileBuffer = MMP_NULL;
    MMP_UINT8  *pReadNorBuffer  = MMP_NULL;
    MMP_BOOL   bUpgradeSuccess  = MMP_FALSE;
    MMP_UINT8  fileIdentifier[FILE_ID_SIZE];
    MMP_UINT32 NorRomVer, ImageVer;

    dbg_msg(DBG_MSG_TYPE_ERROR, "------ firmware upgrade --------\n");
    PalMemcpy(fileIdentifier, pbuffer, FILE_ID_SIZE);
    fileSize -= FILE_ID_SIZE;
    pbuffer  += FILE_ID_SIZE;

    NorInitial();

    do
    {
#define NOR_SECTOR_SIZE (64 * 1024)
#define BREAK_ON_ERROR(exp) if (exp) { trac(""); break; }

        ROM_INFO   romInfo         = {0};
        MMP_UINT32 fwBaseAddrInNOR = 0;
        MMP_UINT32 fwBaseAddrInImg = 0;
        MMP_UINT32 norSize         = NorCapacity();

        if ((fileIdentifier[0] == 0 && fileIdentifier[1] == 0) ||
            (CUSTOMER_CODE == 0 && PROJECT_CODE == 0))
        {
            // always pass, no need to check version
        }
        else if ((fileIdentifier[0] != CUSTOMER_CODE) ||
                 (fileIdentifier[1] != PROJECT_CODE))
        {
            dbg_msg(DBG_MSG_TYPE_ERROR, "version invalid\n");
            break;
        }

        dbg_msg(DBG_MSG_TYPE_INFO, "pbuffer(0x%X) fileSize(0x%X)\n", pbuffer, fileSize);
        // get firmware base address in norflash
        pReadNorBuffer = PalHeapAlloc(PAL_HEAP_DEFAULT, NOR_SECTOR_SIZE);
        BREAK_ON_ERROR(!pReadNorBuffer);
        BREAK_ON_ERROR(MMP_SUCCESS != NorRead(pReadNorBuffer, 0, NOR_SECTOR_SIZE));
        BREAK_ON_ERROR(MMP_SUCCESS != romParser_GetRomInfo(pReadNorBuffer, NOR_SECTOR_SIZE, &romInfo));
        dbg_msg(DBG_MSG_TYPE_INFO, "NOR: dataOffset(0x%X) dataSize(%d) binSize(%d) crc32(0x%08X) version(0x%08X)\n",
                romInfo.dataOffset,
                romInfo.dataSize,
                romInfo.binSize,
                romInfo.crc32,
                romInfo.version);

        NorRomVer       = romInfo.version;

        fwBaseAddrInNOR = romInfo.dataOffset + romInfo.dataSize;
        fwBaseAddrInNOR = (fwBaseAddrInNOR + NOR_SECTOR_SIZE - 1) / NOR_SECTOR_SIZE * NOR_SECTOR_SIZE;
        PalHeapFree(PAL_HEAP_DEFAULT, pReadNorBuffer);
        pReadNorBuffer  = MMP_NULL;
        // get firmware base address in image
        BREAK_ON_ERROR(MMP_SUCCESS != romParser_GetRomInfo(pbuffer, fileSize, &romInfo));
        dbg_msg(DBG_MSG_TYPE_INFO, "IMG(B): dataOffset(0x%X) dataSize(%d) binSize(%d) crc32(0x%08X) version(0x%08X)\n",
                romInfo.dataOffset,
                romInfo.dataSize,
                romInfo.binSize,
                romInfo.crc32,
                romInfo.version);

        ImageVer = romInfo.version;
        dbg_msg(DBG_MSG_TYPE_INFO, "---- NorRomVer = 0x%X, ImageVer = 0x%X ----\n", NorRomVer, ImageVer);
        if ((NorRomVer == ImageVer) && ((ImageVer != 0x34353637) && (NorRomVer != 0x34353637)))
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "---- update Image only ----\n");
            fwBaseAddrInImg = romInfo.dataOffset + romInfo.dataSize;
            fwBaseAddrInImg = (fwBaseAddrInImg + NOR_SECTOR_SIZE - 1) / NOR_SECTOR_SIZE * NOR_SECTOR_SIZE;
            BREAK_ON_ERROR(fileSize < fwBaseAddrInImg);
            fileSize       -= fwBaseAddrInImg;
            pbuffer        += fwBaseAddrInImg;
            dbg_msg(DBG_MSG_TYPE_INFO, "pbuffer(0x%X) fileSize(0x%X)\n", pbuffer, fileSize);
            BREAK_ON_ERROR(MMP_SUCCESS != romParser_GetRomInfo(pbuffer, fileSize, &romInfo));
            dbg_msg(DBG_MSG_TYPE_INFO, "IMG(F): dataOffset(0x%X) dataSize(%d) binSize(%d) crc32(0x%08X) version(0x%08X)\n",
                    romInfo.dataOffset,
                    romInfo.dataSize,
                    romInfo.binSize,
                    romInfo.crc32,
                    romInfo.version);
            BREAK_ON_ERROR(fileSize < (romInfo.dataOffset + romInfo.dataSize));
            pReadNorBuffer = PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
            BREAK_ON_ERROR(!pReadNorBuffer);
        }
        else
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "---- Update Bootloader and Image FW ----\n");
            fwBaseAddrInNOR = 0;
            pReadNorBuffer  = PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
            BREAK_ON_ERROR(!pReadNorBuffer);
        }
        HOST_WriteRegister(0x7c90, 0x4100);
        HOST_WriteRegister(0x7c92, 0x0054);
        dbg_msg(DBG_MSG_TYPE_ERROR, "upgrade start\n");
        NorWrite(pbuffer, fwBaseAddrInNOR, fileSize);
        dbg_msg(DBG_MSG_TYPE_INFO, "write nor(0x%X) size(0x%X) %c%c%c%c%c%c%c%c\n",
                fwBaseAddrInNOR, fileSize,
                pbuffer[0],
                pbuffer[1],
                pbuffer[2],
                pbuffer[3],
                pbuffer[4],
                pbuffer[5],
                pbuffer[6],
                pbuffer[7]);
        NorRead(pReadNorBuffer, fwBaseAddrInNOR, fileSize);
        if (0 == memcmp(pbuffer, pReadNorBuffer, fileSize))
        {
            bUpgradeSuccess = MMP_TRUE;
            dbg_msg(DBG_MSG_TYPE_ERROR, "upgrade success\n");
        }
        else
        {
            bUpgradeSuccess = MMP_FALSE;
            dbg_msg(DBG_MSG_TYPE_ERROR, "upgrade fail, image compare error\n");
        }
        dbg_msg(DBG_MSG_TYPE_ERROR, "upgrade finish\n");
    } while (0);

    if (pReadNorBuffer)
        PalHeapFree(PAL_HEAP_DEFAULT, pReadNorBuffer);

    if (bUpgradeSuccess)
        return MMP_RESULT_SUCCESS;
    else
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "upgrade fail, image may corrupted");
        return MMP_RESULT_ERROR;
    }
}

void
coreSetVersion(
    CORE_VERSION_TYPE version)
{
    gtVersion.customerCode    = version.customerCode;
    gtVersion.projectCode     = version.projectCode;
    gtVersion.sdkMajorVersion = version.sdkMajorVersion;
    gtVersion.sdkMinorVersion = version.sdkMinorVersion;
    gtVersion.buildNumber     = version.buildNumber;
}

void
coreGetVersion(
    CORE_VERSION_TYPE *version)
{
    version->customerCode    = gtVersion.customerCode;
    version->projectCode     = gtVersion.projectCode;
    version->sdkMajorVersion = gtVersion.sdkMajorVersion;
    version->sdkMinorVersion = gtVersion.sdkMinorVersion;
    version->buildNumber     = gtVersion.buildNumber;
}

void
coreSetModulatorPara(
    CORE_MODULATOR_PARA modulatorPara)
{
    gtModulatorPara.frequency     = modulatorPara.frequency;
    gtModulatorPara.bandwidth     = modulatorPara.bandwidth;
    gtModulatorPara.constellation = modulatorPara.constellation;
    gtModulatorPara.codeRate      = modulatorPara.codeRate;
    gtModulatorPara.quardInterval = modulatorPara.quardInterval;
}

void
coreGetModulatorPara(
    CORE_MODULATOR_PARA *modulatorPara)
{
    modulatorPara->frequency     = gtModulatorPara.frequency;
    modulatorPara->bandwidth     = gtModulatorPara.bandwidth;
    modulatorPara->constellation = gtModulatorPara.constellation;
    modulatorPara->codeRate      = gtModulatorPara.codeRate;
    modulatorPara->quardInterval = gtModulatorPara.quardInterval;
}

MMP_BOOL
coreTsUpdateTSID(
    MMP_UINT32 tsId)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateTSID(tsId);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateNetworkName(
    MMP_UINT8 *pNetworkName,
    MMP_UINT32 nameLen)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateNetworkName(pNetworkName, nameLen);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateNetworkId(
    MMP_UINT32 networkId)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateNetworkId(networkId);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateONID(
    MMP_UINT32 onId)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateONID(onId);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateServiceListDescriptor(
    MMP_UINT32 serviceId,
    MMP_UINT32 serviceType)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateServiceListDescriptor(serviceId, serviceType);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateCountryId(
    CORE_COUNTRY_ID countryId)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateCountryId(countryId);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateLCN(
    MMP_UINT32 serviceId,
    MMP_UINT32 lcn)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateLCN(serviceId, lcn);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateModulationParameter(
    MMP_UINT32 frequency,
    MMP_UINT32 bandwidth,
    CONSTELLATION_MODE constellation,
    CODE_RATE_MODE codeRate,
    GUARD_INTERVAL_MODE guardInterval)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateModulationParameter(frequency, bandwidth, constellation, guardInterval);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateTable(
    void)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrGenerateTablePacket();
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsInsertService(
    MMP_UINT16 programNumber,
    MMP_UINT16 pmtPid,
    MMP_UINT16 videoPid,
    MMP_UINT16 videoStreamType,
    MMP_UINT16 audioPid,
    MMP_UINT16 audioStreamType,
    MMP_UINT8 *pServiceProviderName,
    MMP_UINT32 providerNameLen,
    MMP_UINT8 *pServiceName,
    MMP_UINT32 serviceNameLen)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrInsertService(programNumber, pmtPid, videoPid, videoStreamType, audioPid, audioStreamType,
                                     pServiceProviderName, providerNameLen, pServiceName, serviceNameLen);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateServiceName(
    MMP_UINT16 programNumber,
    MMP_UINT8 *pServiceProviderName,
    MMP_UINT32 providerNameLen,
    MMP_UINT8 *pServiceName,
    MMP_UINT32 serviceNameLen)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateServiceName(programNumber, pServiceProviderName, providerNameLen, pServiceName, serviceNameLen);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateServiceProgramNumber(
    MMP_UINT32 serviceIndex,
    MMP_UINT16 programNumber)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateServiceProgramNumber(serviceIndex, programNumber);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateServicePmtPid(
    MMP_UINT16 programNumber,
    MMP_UINT16 pmtPid)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateServicePmtPid(programNumber, pmtPid);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateServiceVideoInfo(
    MMP_UINT16 programNumber,
    MMP_UINT16 videoPid,
    VIDEO_STREAM_TYPE videoStreamType)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateServiceVideoInfo(programNumber, videoPid, videoStreamType);
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
coreTsUpdateServiceAudioInfo(
    MMP_UINT16 programNumber,
    MMP_UINT16 audioPid,
    VIDEO_STREAM_TYPE audioStreamType)
{
    if (gMuxType == MMP_TS_MUX)
    {
        return PsiSiMgrUpdateServiceAudioInfo(programNumber, audioPid, audioStreamType);
    }
    else
    {
        return MMP_FALSE;
    }
}

void
coreTsRemoveServices(
    void)
{
    if (gMuxType == MMP_TS_MUX)
    {
        PsiSiMgrRemoveServices();
    }
}

MMP_UINT32
coreCapsenseReadI2C(
    MMP_UINT8 *pData,
    MMP_UINT32 NByte)
{
#ifdef I2C_ADDR
    MMP_RESULT flag;

    mmpIicLockModule();
    if (0 != (flag = mmpIicReceiveData(0x01, I2C_ADDR, 0x90, pData, NByte)))
    {
        printf("dm2016 I2C Read error\n");
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;
#else
    return 0;
#endif
}


unsigned int
core_read_ws106_command(
    unsigned char *pData,
    unsigned int  NByte)
{
   // return read_ws106_command(pData, NByte);
   printf("core_read_ws106_command not used\r\n");
}

unsigned int
core_write_ws106_command(
    unsigned char *pData,
    unsigned int  NByte)
{
    //return write_ws106_command(pData, NByte);
       printf("core_write_ws106_command not used\r\n");
}


MMP_UINT32
coreCapsenseWriteI2C(
    MMP_UINT8 *pData,
    MMP_UINT32 NByte)
{
#ifdef I2C_ADDR
    MMP_RESULT flag;

    mmpIicLockModule();
    if (0 != (flag = mmpIicSendData(0x01, I2C_ADDR, 0x90, pData, NByte)))
    {
        printf("dm2016 I2c write error\n");
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;
#else
    return 0;
#endif
}

MMP_UINT32
coreCapsenseInterruptState(
    void)
{
    return 0;
}

MMP_UINT32
coreCapsenseReadFW(
    MMP_UINT8 *pData,
    MMP_UINT32 NByte)
{
    return 0;
}

MMP_UINT32
coreCapsenseWriteFW(
    MMP_UINT8 SubAddr,
    MMP_UINT8 *pData,
    MMP_UINT32 NByte)
{
    return 0;
}

void
coreSetConfigCaptureDev(CAPTURE_VIDEO_SOURCE capturedev)
{
    config_set_capturedev(capturedev);
}

void
coreGetConfigCaptureDev(CAPTURE_VIDEO_SOURCE *capturedev)
{
    config_get_capturedev(capturedev);
}

void
coreGetUsbSerialNumber(MMP_CHAR *serialnum)
{
    MMP_CHAR *usbserialnum;
    get_usb_serial_number(&usbserialnum);
    strcpy(serialnum, usbserialnum);
}

void
coreSetUsbSerialNumber(MMP_CHAR *serialnum)
{
    set_usb_serial_number(serialnum);
}

void
coreSetRoot(MMP_BOOL flag)
{
    taskStreamMux_UndoSeal(flag);
}

void
coreAudioCodecInit()
{
    codec_initialize();
}

DIGITAL_AUDIO_VOLUME
coreGetDigitalVolume(void)
{
    GRABBER_CTRL_PARAM grabberCtrl;
    grabberCtrl.flag = GRABBER_CTRL_DIGITAL_AUDIO;
    GrabberControlGetParam(&grabberCtrl);
    return grabberCtrl.digitalVolume;
    //return gDigitalVolume;
}

void
coreSetDigitalVolume(DIGITAL_AUDIO_VOLUME volume)
{
    GRABBER_CTRL_PARAM grabberCtrl;

    grabberCtrl.flag          = GRABBER_CTRL_DIGITAL_AUDIO;
    grabberCtrl.digitalVolume = volume;
    GrabberControlSetParam(&grabberCtrl);
    //gDigitalVolume = volume;
}

/*
   void
   coreSetLedBlinkOn(void)
   {
    LED_STATUS flag = LED_BLINK;
    dbg_msg(DBG_MSG_TYPE_INFO, " -- %s (%d) --\n", __FUNCTION__, __LINE__);
    coreSetBlueLed(flag);
    coreSetRedLed(flag);
    coreSetGreenLed(flag);
    coreSetBlingRingLed(flag);
   }

   void
   coreSetLedBlinkOff(void)
   {
    LED_STATUS flag = LED_OFF;
    dbg_msg(DBG_MSG_TYPE_INFO, " -- %s (%d) --\n", __FUNCTION__, __LINE__);
    coreSetBlueLed(flag);
    coreSetRedLed(flag);
    coreSetGreenLed(flag);
    coreSetBlingRingLed(flag);
   }
 */

static MMP_BOOL isInterlaceSrc(MMP_UINT32 index)
{
    if (index == VIDEO_ENCODER_INPUT_INFO_720X480_59I ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_60I ||
        index == VIDEO_ENCODER_INPUT_INFO_720X576_50I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_50I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_59I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_60I)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

static MMP_UINT16 chkFrameRate(MMP_UINT32 index)
{
    if (index == VIDEO_ENCODER_INPUT_INFO_640X480_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_59P ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X720_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X720_59P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_59P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_800X600_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1024X768_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X768_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X800_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X960_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X1024_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1360X768_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1366X768_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1440X900_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1400X1050_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1440X1050_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1600X900_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1600X1200_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_1680X1050_60P ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_59I ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_60I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_59I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_60I)
        return 60;

    if (index == VIDEO_ENCODER_INPUT_INFO_720X576_50P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X720_50P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_50P ||
        index == VIDEO_ENCODER_INPUT_INFO_720X576_50I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_50I)
        return 50;

    if (index == VIDEO_ENCODER_INPUT_INFO_1920X1080_29P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_30P)
        return 30;

    if (index == VIDEO_ENCODER_INPUT_INFO_1920X1080_25P)
        return 25;

    if (index == VIDEO_ENCODER_INPUT_INFO_1920X1080_23P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_24P)
        return 24;
}

static MMP_UINT16
getMaxFrameRate(
    VIDEO_ENCODER_INPUT_INFO index,
    MMP_UINT16 outWidth,
    MMP_UINT16 outHeight)
{
    MMP_UINT32 MBNum;
    MMP_UINT16 input_fps, output_fps, maxfps;

    MBNum = (outWidth * outHeight) >> 8;

    if (MBNum == 0)
        return 30;

    output_fps = MaxEnMBNum / MBNum;

    input_fps  = chkFrameRate(index);

    if (input_fps <= 30) // 24, 25, 30
    {
        maxfps = input_fps;
    }
    else if (input_fps == 50)
    {
        if (output_fps >= 50 && outHeight <= 960)
            maxfps = 50;
        else
            maxfps = 25;
    }
    else if (input_fps == 60)
    {
        if (output_fps >= 60 && outHeight <= 800)
            maxfps = 60;
        else
            maxfps = 30;
    }

    return maxfps;
}

void
coreSetHWGrabberPcMode(
    unsigned int enable)
{
    if (enable)
        SendDelayedMsg(MSG_DEVICE_MODE, 100, 0, 0, 0);
    else
        SendDelayedMsg(MSG_HOST_MODE, 100, 0, 0, 0);
}

static void
getWidthHeight(
    MMP_UINT32 index,
    MMP_UINT32 *Width,
    MMP_UINT32 *Height)
{
    if (index == VIDEO_ENCODER_INPUT_INFO_640X480_60P)
    {
        *Width  = 640;
        *Height = 480;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_720X480_59I ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_59P ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_60I ||
        index == VIDEO_ENCODER_INPUT_INFO_720X480_60P)
    {
        *Width  = 720;
        *Height = 480;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_720X576_50I ||
        index == VIDEO_ENCODER_INPUT_INFO_720X576_50P)
    {
        *Width  = 720;
        *Height = 576;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1280X720_50P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X720_59P ||
        index == VIDEO_ENCODER_INPUT_INFO_1280X720_60P)
    {
        *Width  = 1280;
        *Height = 720;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1920X1080_23P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_24P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_25P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_29P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_30P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_50I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_50P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_59I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_59P ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_60I ||
        index == VIDEO_ENCODER_INPUT_INFO_1920X1080_60P)
    {
        *Width  = 1920;
        *Height = 1080;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_800X600_60P)
    {
        *Width  = 800;
        *Height = 600;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1024X768_60P)
    {
        *Width  = 1024;
        *Height = 768;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1280X768_60P)
    {
        *Width  = 1280;
        *Height = 768;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1280X800_60P)
    {
        *Width  = 1280;
        *Height = 800;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1280X960_60P)
    {
        *Width  = 1280;
        *Height = 960;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1280X1024_60P)
    {
        *Width  = 1280;
        *Height = 1024;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1360X768_60P)
    {
        *Width  = 1360;
        *Height = 768;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1366X768_60P)
    {
        *Width  = 1366;
        *Height = 768;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1440X900_60P)
    {
        *Width  = 1440;
        *Height = 900;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1400X1050_60P)
    {
        *Width  = 1400;
        *Height = 1050;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1440X1050_60P)
    {
        *Width  = 1440;
        *Height = 1050;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1600X900_60P)
    {
        *Width  = 1600;
        *Height = 900;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1600X1200_60P)
    {
        *Width  = 1600;
        *Height = 1200;
    }

    if (index == VIDEO_ENCODER_INPUT_INFO_1680X1050_60P)
    {
        *Width  = 1680;
        *Height = 1050;
    }
}
