/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file core_interface.h
 * @author
 * @version 0.1
 */

#ifndef CORE_INTERFACE_H
#define CORE_INTERFACE_H

#include "mmp_types.h"
#include "mmp_capture.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef enum CORE_NOTIFY_EVENT_TAG
{
    CORE_NO_EVENT = 0,
    CORE_FILE_OPEN_SUCCESS,
    CORE_FILE_OPEN_FAIL,
    CORE_FILE_END_OF_FILE,
    CORE_NO_MORE_STORAGE,
    CORE_FILE_WRITE_FAIL,
    CORE_FILE_CLOSE_FAIL,
    CORE_FILE_CLOSE_SUCCESS,
} CORE_NOTIFY_EVENT;

typedef enum CONSTELLATION_MODE_TAG
{
    CONSTELATTION_QPSK,
    CONSTELATTION_16QAM,
    CONSTELATTION_64QAM
} CONSTELLATION_MODE;

typedef enum CODE_RATE_MODE_TAG
{
    CODE_RATE_1_2,
    CODE_RATE_2_3,
    CODE_RATE_3_4,
    CODE_RATE_5_6,
    CODE_RATE_7_8
} CODE_RATE_MODE;

typedef enum GUARD_INTERVAL_MODE_TAG
{
    GUARD_INTERVAL_1_4,
    GUARD_INTERVAL_1_8,
    GUARD_INTERVAL_1_16,
    GUARD_INTERVAL_1_32
} GUARD_INTERVAL_MODE;

typedef enum AUDIO_ENCODER_TYPE
{
    MPEG_AUDIO_ENCODER,
    AAC_AUDIO_ENCODER
} AUDIO_ENCODER_TYPE;

typedef enum CORE_COUNTRY_ID_TAG
{
    CORE_COUNTRY_TAIWAN,
    CORE_COUNTRY_DENMARK,
    CORE_COUNTRY_FINLAND,
    CORE_COUNTRY_NORWAY,
    CORE_COUNTRY_SWEDEN,
    CORE_COUNTRY_GERMANY,
    CORE_COUNTRY_UK,
    CORE_COUNTRY_ITALY,
    CORE_COUNTRY_AUSTRALIA,
    CORE_COUNTRY_NEW_ZEALAND,
    CORE_COUNTRY_FRANCE,
    CORE_COUNTRY_SPAIN,
    CORE_COUNTRY_POLAND,
    CORE_COUNTRY_CZECH,
    CORE_COUNTRY_NETHERLANDS,
    CORE_COUNTRY_GREECE,
    CORE_COUNTRY_RUSSIA,
    CORE_COUNTRY_SWITZERLAND,
    CORE_COUNTRY_SLOVAK,
    CORE_COUNTRY_SLOVENIA,
    CORE_COUNTRY_HUNGARY,
    CORE_COUNTRY_AUSTRIA,
    CORE_COUNTRY_LATIVA,
    CORE_COUNTRY_ISRAEL,
    CORE_COUNTRY_CROATIA,
    CORE_COUNTRY_ESTONIA,
    CORE_COUNTRY_PORTUGAL,
    CORE_COUNTRY_IRELAND,
    CORE_COUNTRY_LAST_ID
} CORE_COUNTRY_ID;

typedef enum AUDIO_STREAM_TYPE_TAG
{
    MPEG_AUDIO,
    DOLBY_AC3,
    DTS,
    AAC,
    LAST_AUDIO_STREAM_TYPE
} AUDIO_STREAM_TYPE;

typedef enum VIDEO_STREAM_TYPE_TAG
{
    H264_VIDEO_STREAM,
    LAST_VIDEO_STREAM_TYPE
} VIDEO_STREAM_TYPE;

typedef enum VIDEO_ENCODER_UPDATE_FLAGS_TAG
{
    VIDEO_ENCODER_FLAGS_UPDATE_WIDTH_HEIGHT = (0x00000001 << 0),
    VIDEO_ENCODER_FLAGS_UPDATE_BITRATE      = (0x00000001 << 1),
    VIDEO_ENCODER_FLAGS_UPDATE_DEINTERLACE  = (0x00000001 << 2),
    VIDEO_ENCODER_FLAGS_UPDATE_FRAME_RATE   = (0x00000001 << 3),
    VIDEO_ENCODER_FLAGS_UPDATE_GOP_SIZE     = (0x00000001 << 4),
    VIDEO_ENCODER_FLAGS_UPDATE_ASPECT_RATIO = (0x00000001 << 5),
    VIDEO_ENCODER_FLAGS_UPDATE_DEFAULT      = (0x00000001 << 6)
} VIDEO_ENCODER_UPDATE_FLAGS;

/**
 *  Video source Select
 */
typedef enum CAPTURE_VIDEO_SOURCE_TAG
{
    CAPTURE_VIDEO_SOURCE_UNKNOW = MMP_CAP_VIDEO_SOURCE_UNKNOW,
    CAPTURE_VIDEO_SOURCE_CVBS   = MMP_CAP_VIDEO_SOURCE_CVBS,
    CAPTURE_VIDEO_SOURCE_SVIDEO = MMP_CAP_VIDEO_SOURCE_SVIDEO,
    CAPTURE_VIDEO_SOURCE_YPBPR  = MMP_CAP_VIDEO_SOURCE_YPBPR,
    CAPTURE_VIDEO_SOURCE_VGA    = MMP_CAP_VIDEO_SOURCE_VGA,
    CAPTURE_VIDEO_SOURCE_HDMI   = MMP_CAP_VIDEO_SOURCE_HDMI,
    CAPTURE_VIDEO_SOURCE_DVI    = MMP_CAP_VIDEO_SOURCE_DVI,
    CAPTURE_VIDEO_SOURCE_SENSOR = MMP_CAP_VIDEO_SOURCE_SENSOR,
} CAPTURE_VIDEO_SOURCE;

/**
 *  Device Select
 */
typedef enum CAPTURE_DEVICE_TAG
{
    CAPTURE_DEV_UNKNOW    = MMP_CAP_UNKNOW_DEVICE,
    CAPTURE_DEV_ADV7180   = MMP_CAP_DEV_ADV7180,
    CAPTURE_DEV_CAT9883   = MMP_CAP_DEV_CAT9883,
    CAPTURE_DEV_HDMIRX    = MMP_CAP_DEV_HDMIRX,
    CAPTURE_DEV_SENSOR    = MMP_CAP_DEV_SENSOR,
    CAPTURE_DEV_ADV7180_S = MMP_CAP_DEV_ADV7180_S,
} CAPTURE_DEVICE;

/**
 *  Capture Skip Pattern
 */
typedef enum CAP_SKIP_PATTERN_TAG
{
    CAPTURE_NO_DROP       = MMP_CAPTURE_NO_DROP,
    CAPTURE_SKIP_BY_TWO   = MMP_CAPTURE_SKIP_BY_TWO,
    CAPTURE_SKIP_BY_THREE = MMP_CAPTURE_SKIP_BY_THREE
} CAP_SKIP_PATTERN;

typedef enum VIDEO_ENCODER_INPUT_INFO_TAG
{
    VIDEO_ENCODER_INPUT_INFO_640X480_60P   = MMP_CAP_INPUT_INFO_640X480_60P,
    VIDEO_ENCODER_INPUT_INFO_720X480_59I   = MMP_CAP_INPUT_INFO_720X480_59I,
    VIDEO_ENCODER_INPUT_INFO_720X480_59P   = MMP_CAP_INPUT_INFO_720X480_59P,
    VIDEO_ENCODER_INPUT_INFO_720X480_60I   = MMP_CAP_INPUT_INFO_720X480_60I,
    VIDEO_ENCODER_INPUT_INFO_720X480_60P   = MMP_CAP_INPUT_INFO_720X480_60P,
    VIDEO_ENCODER_INPUT_INFO_720X576_50I   = MMP_CAP_INPUT_INFO_720X576_50I,
    VIDEO_ENCODER_INPUT_INFO_720X576_50P   = MMP_CAP_INPUT_INFO_720X576_50P,
    VIDEO_ENCODER_INPUT_INFO_1280X720_50P  = MMP_CAP_INPUT_INFO_1280X720_50P,
    VIDEO_ENCODER_INPUT_INFO_1280X720_59P  = MMP_CAP_INPUT_INFO_1280X720_59P,
    VIDEO_ENCODER_INPUT_INFO_1280X720_60P  = MMP_CAP_INPUT_INFO_1280X720_60P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_23P = MMP_CAP_INPUT_INFO_1920X1080_23P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_24P = MMP_CAP_INPUT_INFO_1920X1080_24P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_25P = MMP_CAP_INPUT_INFO_1920X1080_25P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_29P = MMP_CAP_INPUT_INFO_1920X1080_29P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_30P = MMP_CAP_INPUT_INFO_1920X1080_30P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_50I = MMP_CAP_INPUT_INFO_1920X1080_50I,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_50P = MMP_CAP_INPUT_INFO_1920X1080_50P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_59I = MMP_CAP_INPUT_INFO_1920X1080_59I,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_59P = MMP_CAP_INPUT_INFO_1920X1080_59P,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_60I = MMP_CAP_INPUT_INFO_1920X1080_60I,
    VIDEO_ENCODER_INPUT_INFO_1920X1080_60P = MMP_CAP_INPUT_INFO_1920X1080_60P,
    VIDEO_ENCODER_INPUT_INFO_800X600_60P   = MMP_CAP_INPUT_INFO_800X600_60P,
    VIDEO_ENCODER_INPUT_INFO_1024X768_60P  = MMP_CAP_INPUT_INFO_1024X768_60P,
    VIDEO_ENCODER_INPUT_INFO_1280X768_60P  = MMP_CAP_INPUT_INFO_1280X768_60P,
    VIDEO_ENCODER_INPUT_INFO_1280X800_60P  = MMP_CAP_INPUT_INFO_1280X800_60P,
    VIDEO_ENCODER_INPUT_INFO_1280X960_60P  = MMP_CAP_INPUT_INFO_1280X960_60P,
    VIDEO_ENCODER_INPUT_INFO_1280X1024_60P = MMP_CAP_INPUT_INFO_1280X1024_60P,
    VIDEO_ENCODER_INPUT_INFO_1360X768_60P  = MMP_CAP_INPUT_INFO_1360X768_60P,
    VIDEO_ENCODER_INPUT_INFO_1366X768_60P  = MMP_CAP_INPUT_INFO_1366X768_60P,
    VIDEO_ENCODER_INPUT_INFO_1440X900_60P  = MMP_CAP_INPUT_INFO_1440X900_60P,
    VIDEO_ENCODER_INPUT_INFO_1400X1050_60P = MMP_CAP_INPUT_INFO_1400X1050_60P,
    VIDEO_ENCODER_INPUT_INFO_1440X1050_60P = MMP_CAP_INPUT_INFO_1440X1050_60P,
    VIDEO_ENCODER_INPUT_INFO_1600X900_60P  = MMP_CAP_INPUT_INFO_1600X900_60P,
    VIDEO_ENCODER_INPUT_INFO_1600X1200_60P = MMP_CAP_INPUT_INFO_1600X1200_60P,
    VIDEO_ENCODER_INPUT_INFO_1680X1050_60P = MMP_CAP_INPUT_INFO_1680X1050_60P,
    VIDEO_ENCODER_INPUT_INFO_ALL           = MMP_CAP_INPUT_INFO_ALL,
    VIDEO_ENCODER_INPUT_INFO_NUM           = MMP_CAP_INPUT_INFO_NUM,
    VIDEO_ENCODER_INPUT_INFO_UNKNOWN       = MMP_CAP_INPUT_INFO_UNKNOWN,
    VIDEO_ENCODER_INPUT_INFO_CAMERA        = MMP_CAP_INPUT_INFO_CAMERA,
} VIDEO_ENCODER_INPUT_INFO;

typedef enum VIDEO_ENCODER_SKIP_MODE_TAG
{
    VIDEO_ENCODER_NO_DROP = 0,
    VIDEO_ENCODER_SKIP_BY_TWO
} VIDEO_ENCODER_SKIP_MODE;

typedef enum ASPECT_RATIO_TAG
{
    AR_FULL = 0,
    AR_LETTER_BOX,
    AR_PAN_SCAN,
} ASPECT_RATIO;

typedef enum LINE_BOOST_TAG
{
    LINE_BOOST_MUTE      = 0x0,
    LINE_BOOST_SUB_12_DB = 0x1,
    LINE_BOOST_SUB_9_DB  = 0x2,
    LINE_BOOST_SUB_6_DB  = 0x3,
    LINE_BOOST_SUB_3_DB  = 0x4,
    LINE_BOOST_0_DB      = 0x5,
    LINE_BOOST_ADD_3_DB  = 0x6,
    LINE_BOOST_ADD_6_DB  = 0x7,
} LINE_BOOST;

typedef enum INPUT_ASPECT_RATIO_TAG
{
    INPUT_ASPECT_RATIO_AUTO = 0x0,
    INPUT_ASPECT_RATIO_4_3  = 0x1,
    INPUT_ASPECT_RATIO_16_9 = 0x2,
} INPUT_ASPECT_RATIO;

typedef enum LED_STATUS_TAG
{
    LED_ON    = 0,
    LED_OFF   = 1,
    LED_BLINK = 2
} LED_STATUS;

typedef enum DIGITAL_AUDIO_VOLUME_TAG
{
    DIGITAL_AUDIO_MUTE = 0,
    DIGITAL_AUDIO_DOWN_TO_1_32,     // -15dB
    DIGITAL_AUDIO_DOWN_TO_2_32,     // -12dB
    DIGITAL_AUDIO_DOWN_TO_6_32,     // -7dB
    DIGITAL_AUDIO_DOWN_TO_16_32,    // -3dB
    DIGITAL_AUDIO_DEFAULT,
    DIGITAL_AUDIO_UP_TO_34_32,
    DIGITAL_AUDIO_UP_TO_36_32,
    DIGITAL_AUDIO_UP_TO_37_32,
    DIGITAL_AUDIO_UP_TO_39_32,
    DIGITAL_AUDIO_UP_TO_40_32,
    DIGITAL_AUDIO_UP_TO_41_32,
    DIGITAL_AUDIO_UP_TO_44_32,
    DIGITAL_AUDIO_UP_TO_45_32,
    DIGITAL_AUDIO_UP_TO_47_32,
    DIGITAL_AUDIO_UP_TO_49_32,
    DIGITAL_AUDIO_UP_TO_52_32,
    DIGITAL_AUDIO_UP_TO_55_32,
    DIGITAL_AUDIO_UP_TO_58_32,
    DIGITAL_AUDIO_UP_TO_62_32,
    DIGITAL_AUDIO_UP_TO_64_32,
} DIGITAL_AUDIO_VOLUME;

//=============================================================================
//                              Macro Definition
//=============================================================================
#define CORE_API extern

typedef struct JPEG_STREAM_TAG
{
    MMP_UINT8  *pHdrBuf;
    MMP_UINT32 headerSize;
    MMP_UINT8  *pStreamBuf;
    MMP_UINT32 streamSize;
} JPEG_STREAM;

typedef void (*CORE_NOTIFY_CALLBACK) (CORE_NOTIFY_EVENT event, MMP_UINT32 extraData);
typedef void (*JPEG_ENCODER_CALLBACK) (JPEG_STREAM *ptJpgStream);

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct SERVICE_INFO_TAG
{
    MMP_UINT8  pServiceName[32];
    MMP_UINT32 serviceNameLen;
    MMP_UINT32 programNumber;
    MMP_UINT32 pmtPid;
    MMP_UINT32 videoPid;
    MMP_UINT32 videoType;
    MMP_UINT32 audioPid;
    MMP_UINT32 audioType;
} SERVICE_INFO;

typedef struct TS_MUXER_PARAMETER_TAG
{
    MMP_BOOL           bEnableTso;
    MMP_BOOL           bEnableEagle;
    MMP_UINT32         constellation;
    MMP_UINT32         codeRate;
    MMP_UINT32         guardInterval;
    MMP_UINT32         frequency;
    MMP_UINT32         bandwidth;
    MMP_INT            gain;
    AUDIO_ENCODER_TYPE audioEncoderType;
    MMP_UINT8          pProviderName[32];
    MMP_UINT32         providerNameLen;
    SERVICE_INFO       ptServiceInfo[8];
    MMP_UINT32         serviceCount;
    MMP_BOOL           bEnableSecuirtyMode;
    MMP_UINT32         securityMode;
    MMP_BOOL           keyRefreshSecond;
    MMP_BOOL           bAddStuffData;
} TS_MUXER_PARAMETER;

typedef struct CORE_VERSION_TYPE
{
    MMP_UINT16 customerCode;
    MMP_UINT16 projectCode;
    MMP_UINT16 sdkMajorVersion;
    MMP_UINT16 sdkMinorVersion;
    MMP_UINT16 buildNumber;
} CORE_VERSION_TYPE;

typedef struct CORE_MODULATOR_PARA
{
    MMP_UINT16          frequency;  //kHz
    MMP_UINT16          bandwidth;  //6:6M 7:7M 8:8M
    CONSTELLATION_MODE  constellation;
    CODE_RATE_MODE      codeRate;
    GUARD_INTERVAL_MODE quardInterval;
} CORE_MODULATOR_PARA;

typedef struct CORE_AUDIO_ENCODE_PARA_TAG
{
    AUDIO_ENCODER_TYPE audioEncoderType;
    MMP_UINT32         bitRate;
} CORE_AUDIO_ENCODE_PARA;

typedef struct INPUT_VIDEO_SRC_INFO_TAG
{
    MMP_UINT16           vidWidth;
    MMP_UINT16           vidHeight;
    MMP_BOOL             bInterlaceSrc;
    MMP_UINT16           vidFrameRate;
    CAPTURE_VIDEO_SOURCE vidDevice;
} INPUT_VIDEO_SRC_INFO;

typedef struct VIDEO_ENCODER_PARAMETER_TAG
{
    MMP_UINT16   EnWidth;
    MMP_UINT16   EnHeight;
    MMP_UINT16   EnBitrate;
    MMP_BOOL     EnDeinterlaceOn;
    MMP_BOOL     EnFrameDouble;
    MMP_UINT16   EnFrameRate;
    MMP_UINT16   EnGOPSize;
    ASPECT_RATIO EnAspectRatio;
    VIDEO_ENCODER_SKIP_MODE EnSkipMode;
    MMP_UINT16   InstanceNum;
} VIDEO_ENCODER_PARAMETER;

typedef struct CAPTURE_DEVICE_INFO_TAG
{
    CAPTURE_DEVICE       deviceId;         //[IN]
    CAPTURE_VIDEO_SOURCE videoMode;        //[IN]
} CAPTURE_DEVICE_INFO;

typedef struct VIDEO_COLOR_CTRL_TAG
{
    MMP_INT32 brightness;                   // -128 ~ 128     default : 0
    MMP_FLOAT contrast;                     // 0.0 ~ 4.0      default : 1.0
    MMP_INT32 hue;                          // 0 ~ 359        default : 0
    MMP_FLOAT saturation;                   // 0.0 ~ 4.0      default : 1.0
} VIDEO_COLOR_CTRL;

typedef struct RECORD_MODE_TAG
{
    MMP_BOOL bDisableFileSplitter;
} RECORD_MODE;

typedef struct JPEG_CONTEXT_TAG
{
    MMP_BOOL   bOneShot;                    // [IN]
    MMP_UINT32 encodingPeriod;              // [IN]  // second Unit
    MMP_UINT32 quality;                     // [IN]  // 1~100
    JPEG_ENCODER_CALLBACK pfJPEGCallback;   // [IN]
} JPEG_CONTEXT;

//=============================================================================
//                              Function  Definition
//=============================================================================

CORE_API void
coreInitialize(
    MMP_MUX_TYPE mux_type);

CORE_API void
coreTerminate(
    void);

CORE_API void
corePlay(
    void);

CORE_API void
coreStop(
    void);

CORE_API void
coreApiInitialize(
    MMP_MUX_TYPE mux_type);

CORE_API void
coreApiTerminate(
    void);

CORE_API void
coreApiPlay(
    void);

CORE_API void
coreApiStop(
    void);

CORE_API MMP_BOOL
coreIsPCConnectMode(
    void);
extern MMP_BOOL (*_coreIsPCConnectMode)(void);
extern MMP_BOOL coreIsPCConnectMode_default(void);

CORE_API void
coreInitVideoEnPara(
    void);
extern void (*_coreInitVideoEnPara)(void);
extern void coreInitVideoEnPara_default(void);

CORE_API void
coreSetVideoEnPara(
    CAPTURE_VIDEO_SOURCE       videoSrc,
    VIDEO_ENCODER_UPDATE_FLAGS flags,
    VIDEO_ENCODER_INPUT_INFO   index,
    VIDEO_ENCODER_PARAMETER    *ptEnPara);
extern void
(*_coreSetVideoEnPara)(
    CAPTURE_VIDEO_SOURCE       videoSrc,
    VIDEO_ENCODER_UPDATE_FLAGS flags,
    VIDEO_ENCODER_INPUT_INFO   index,
    VIDEO_ENCODER_PARAMETER    *ptEnPara);
extern void
coreSetVideoEnPara_default(
    CAPTURE_VIDEO_SOURCE       videoSrc,
    VIDEO_ENCODER_UPDATE_FLAGS flags,
    VIDEO_ENCODER_INPUT_INFO   index,
    VIDEO_ENCODER_PARAMETER    *ptEnPara);

CORE_API void
coreGetVideoEnPara(
    CAPTURE_VIDEO_SOURCE     videoSrc,
    VIDEO_ENCODER_INPUT_INFO index,
    VIDEO_ENCODER_PARAMETER  *ptEnPara);
extern void
(*_coreGetVideoEnPara)(
    CAPTURE_VIDEO_SOURCE     videoSrc,
    VIDEO_ENCODER_INPUT_INFO index,
    VIDEO_ENCODER_PARAMETER  *ptEnPara);
extern void
coreGetVideoEnPara_default(
    CAPTURE_VIDEO_SOURCE     videoSrc,
    VIDEO_ENCODER_INPUT_INFO index,
    VIDEO_ENCODER_PARAMETER  *ptEnPara);

#ifdef ENABLE_MENCODER
CORE_API MMP_BOOL
coreStartRecord(
    MMP_WCHAR            *filePath,
    //MMP_CHAR*               filePath,
    CORE_NOTIFY_CALLBACK pfCallback);

CORE_API MMP_BOOL
coreStopRecord(
    void);
#endif

CORE_API MMP_BOOL
coreStartJPEGRecord(
    JPEG_CONTEXT *ptJpgCtx);

CORE_API MMP_BOOL
coreStopJPEGRecord(
    void);

CORE_API void
coreGetTsMuxBufferInfo(
    MMP_UINT8  **pBufferStart,
    MMP_UINT32 *bufferSize);

CORE_API MMP_UINT32
coreGetTsMuxBufferWriteIndex(
    void);

CORE_API void
coreSetCaptureSource(
    CAPTURE_VIDEO_SOURCE videoSrc);

CORE_API CAPTURE_VIDEO_SOURCE
coreGetCaptureSource(
    void);

CORE_API VIDEO_ENCODER_INPUT_INFO
coreGetInputSrcInfo(
    void);

CORE_API void
coreEnableISPOnFly(
    MMP_BOOL bEnableOnly);

CORE_API void
coreEnableAVEngine(
    MMP_BOOL bEnableAVEngine);

CORE_API void
coreSetRecordMode(
    RECORD_MODE *ptRecMode);

CORE_API MMP_UINT16
coreGetMaxFrameRate(
    VIDEO_ENCODER_INPUT_INFO index,
    MMP_UINT16               outWidth,
    MMP_UINT16               outHeight);

CORE_API MMP_UINT16
coreGetCapFrameRate(
    VIDEO_ENCODER_INPUT_INFO index);

CORE_API MMP_BOOL
coreIsInterlaceSrc(
    VIDEO_ENCODER_INPUT_INFO index);

CORE_API MMP_RESULT
coreGetActiveVideoInfo(INPUT_VIDEO_SRC_INFO *activeVideo);

CORE_API MMP_BOOL
coreIsVideoStable(void);

CORE_API void
coreSetPCModeEnableRecord(MMP_BOOL enable);

CORE_API void
coreGetPCModeEnableRecord(MMP_BOOL *enable);

CORE_API void
coreSetMuxerParameter(
    void *ptMuxerInfo);

CORE_API void
coreGetAudioEncodeParameter(
    void *ptPara);

CORE_API void
coreSetAudioEncodeParameter(
    void *ptAudioEncodeInfo);

CORE_API void
coreRtcInitialize(
    void);

CORE_API MMP_RESULT
coreRtcSetTime(
    MMP_UINT8 hour,
    MMP_UINT8 min,
    MMP_UINT8 sec);

CORE_API MMP_RESULT
coreRtcGetTime(
    MMP_UINT8 *hour,
    MMP_UINT8 *min,
    MMP_UINT8 *sec);

CORE_API MMP_RESULT
coreRtcSetDate(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day);

CORE_API MMP_RESULT
coreRtcGetDate(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day);

CORE_API MMP_RESULT
coreRtcSetDateTime(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day,
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec);

CORE_API MMP_RESULT
coreRtcGetDateTime(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day,
    MMP_UINT *hour,
    MMP_UINT *min,
    MMP_UINT *sec);

CORE_API void
coreSetMicInVolStep(
    MMP_UINT32 volstep);

CORE_API MMP_UINT32
coreGetMicInVolStep(
    void);

CORE_API void
coreSetLineInBoost(
    LINE_BOOST boost);

CORE_API LINE_BOOST
coreGetLineInBoost(
    void);

CORE_API MMP_BOOL
coreGetIsContentProtection(
    void);

CORE_API void
coreSetAspectRatio(
    INPUT_ASPECT_RATIO mode);

CORE_API INPUT_ASPECT_RATIO
coreGetAspectRatio(
    void);

CORE_API INPUT_ASPECT_RATIO
coreGetInputAspectRatio(
    void);

CORE_API MMP_UINT32
coreGetHDMIAudioMode(
    void);

CORE_API void
coreSetSignalLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetSignalLed(
    void);

CORE_API void
coreSetRecordLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetRecordLed(
    void);

CORE_API void
coreSetSD720PLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetSD720PLed(
    void);

CORE_API void
coreSetHD1080PLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetHD1080PLed(
    void);

CORE_API void
coreSetAnalogLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetAnalogLed(
    void);

CORE_API void
coreSetHDMILed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetHDMILed(
    void);

CORE_API void
coreSetBlueLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetBlueLed(
    void);

CORE_API void
coreSetGreenLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetGreenLed(
    void);

CORE_API void
coreSetRedLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetRedLed(
    void);

CORE_API void
coreSetBlingRingLed(
    LED_STATUS flag);

CORE_API LED_STATUS
coreGetBlingRingLed(
    void);

CORE_API void
coreSetVideoColor(
    VIDEO_COLOR_CTRL *pCtrl);

CORE_API void
coreGetVideoColor(
    VIDEO_COLOR_CTRL *pCtrl);

CORE_API void
coreDisableHDCP(
    MMP_BOOL flag);

CORE_API MMP_BOOL
coreIsDisableHDCP(
    void);

CORE_API MMP_RESULT
coreFirmwareUpgrade(
    MMP_UINT8  *pbuffer,
    MMP_UINT32 fileSize);

CORE_API void
coreSetVersion(
    CORE_VERSION_TYPE version);

CORE_API void
coreGetVersion(
    CORE_VERSION_TYPE *version);

CORE_API VIDEO_ENCODER_INPUT_INFO
coreGetInputSrcInfo(
    void);

CORE_API void
coreSetModulatorPara(
    CORE_MODULATOR_PARA modulatorPara);

CORE_API void
coreGetModulatorPara(
    CORE_MODULATOR_PARA *modulatorPara);

CORE_API MMP_BOOL
coreTsUpdateTSID(
    MMP_UINT32 tsId);

CORE_API MMP_BOOL
coreTsUpdateNetworkName(
    MMP_UINT8  *pNetworkName,
    MMP_UINT32 nameLen);

CORE_API MMP_BOOL
coreTsUpdateNetworkId(
    MMP_UINT32 networkId);

CORE_API MMP_BOOL
coreTsUpdateONID(
    MMP_UINT32 onId);

CORE_API MMP_BOOL
coreTsUpdateServiceListDescriptor(
    MMP_UINT32 serviceId,
    MMP_UINT32 serviceType);

CORE_API MMP_BOOL
coreTsUpdateCountryId(
    CORE_COUNTRY_ID countryId);

CORE_API MMP_BOOL
coreTsUpdateLCN(
    MMP_UINT32 serviceId,
    MMP_UINT32 lcn);

CORE_API MMP_BOOL
coreTsUpdateModulationParameter(
    MMP_UINT32          frequency,
    MMP_UINT32          bandwidth,
    CONSTELLATION_MODE  constellation,
    CODE_RATE_MODE      codeRate,
    GUARD_INTERVAL_MODE guardInterval);

CORE_API MMP_BOOL
coreTsUpdateTable(
    void);

CORE_API MMP_BOOL
coreTsInsertService(
    MMP_UINT16 programNumber,
    MMP_UINT16 pmtPid,
    MMP_UINT16 videoPid,
    MMP_UINT16 videoStreamType,
    MMP_UINT16 audioPid,
    MMP_UINT16 audioStreamType,
    MMP_UINT8  *pServiceProviderName,
    MMP_UINT32 providerNameLen,
    MMP_UINT8  *pServiceName,
    MMP_UINT32 serviceNameLen);

CORE_API MMP_BOOL
coreTsUpdateServiceName(
    MMP_UINT16 programNumber,
    MMP_UINT8  *pServiceProviderName,
    MMP_UINT32 providerNameLen,
    MMP_UINT8  *pServiceName,
    MMP_UINT32 serviceNameLen);

CORE_API MMP_BOOL
coreTsUpdateServiceProgramNumber(
    MMP_UINT32 serviceIndex,
    MMP_UINT16 programNumber);

CORE_API MMP_BOOL
coreTsUpdateServicePmtPid(
    MMP_UINT16 programNumber,
    MMP_UINT16 pmtPid);

CORE_API MMP_BOOL
coreTsUpdateServiceVideoInfo(
    MMP_UINT16        programNumber,
    MMP_UINT16        videoPid,
    VIDEO_STREAM_TYPE videoStreamType);

CORE_API void
coreTsRemoveServices(
    void);

CORE_API MMP_UINT32
coreCapsenseReadI2C(
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

CORE_API MMP_UINT32
coreCapsenseWriteI2C(
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

CORE_API MMP_UINT32
coreCapsenseInterruptState(
    void);

CORE_API MMP_UINT32
coreCapsenseReadFW(
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

CORE_API MMP_UINT32
coreCapsenseWriteFW(
    MMP_UINT8  SubAddr,
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

CORE_API void
coreSetConfigCaptureDev(
    CAPTURE_VIDEO_SOURCE capturedev);

CORE_API void
coreGetConfigCaptureDev(
    CAPTURE_VIDEO_SOURCE *capturedev);

CORE_API void
coreGetUsbSerialNumber(
    MMP_CHAR *serialnum);

CORE_API void
coreSetUsbSerialNumber(
    MMP_CHAR *serialnum);

CORE_API void
coreSetRoot(MMP_BOOL flag);

CORE_API void
coreAudioCodecInit();

/*
   CORE_API void
   coreSetLedBlinkOn(
    void);

   CORE_API void
   coreSetLedBlinkOff(
    void);
 */

CORE_API void
coreSetHWGrabberPcMode(
    unsigned int enable);

CORE_API DIGITAL_AUDIO_VOLUME
coreGetDigitalVolume(void);

CORE_API void
coreSetDigitalVolume(DIGITAL_AUDIO_VOLUME volume);

unsigned int
core_read_ws106_command(
    unsigned char *pData,
    unsigned int  NByte);

unsigned int
core_write_ws106_command(
    unsigned char *pData,
    unsigned int  NByte);


extern unsigned int
read_ws106_command(
    unsigned char *pData,
    unsigned int  NByte);

extern unsigned int
write_ws106_command(
    unsigned char *pData,
    unsigned int  NByte);


#ifdef __cplusplus
}
#endif

#endif
