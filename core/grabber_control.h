#ifndef GRABBER_CONTROL_H
#define GRABBER_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mmp_hdmirx.h"
#include "core_interface.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum GRABBER_CTRL_FLAG_TAG
{
    GRABBER_CTRL_MIC_VOL,
    GRABBER_CTRL_LINE_BOOST,
    GRABBER_CTRL_DIGITAL_AUDIO,
    GRABBER_CTRL_HDCP,
    GRABBER_CTRL_ASPECT_RATIO,
    GRABBER_CTRL_AUDIO_BITRATE,
    GRABBER_CTRL_HDMI_AUDIO_MODE,
    GRABBER_CTRL_RECORD_LED,
    GRABBER_CTRL_SIGNAL_LED,
    GRABBER_CTRL_SD720P_LED,
    GRABBER_CTRL_HD1080P_LED,
    GRABBER_CTRL_ANALOG_LED,
    GRABBER_CTRL_HDMI_LED,
    GRABBER_CTRL_GREEN_LED,
    GRABBER_CTRL_BLUE_LED,
    GRABBER_CTRL_RED_LED,
    GRABBER_CTRL_BLING_RING_LED,
    GRABBER_CTRL_VIDEO_COLOR,
} GRABBER_CTRL_FLAG;

typedef enum MIC_VOL_TAG
{
    MIC_VOL_MUTE = 0,
    MIC_VOL_01   = 1,
    MIC_VOL_02   = 2,
    MIC_VOL_03   = 3,
    MIC_VOL_04   = 4,
    MIC_VOL_05   = 5,
    MIC_VOL_06   = 6,
    MIC_VOL_07   = 7,
    MIC_VOL_08   = 8,
    MIC_VOL_09   = 9,
    MIC_VOL_10   = 10,
    MIC_VOL_11   = 11,
    MIC_VOL_12   = 12,
    MIC_VOL_13   = 13,
    MIC_VOL_14   = 14,
    MIC_VOL_15   = 15,
    MIC_VOL_16   = 16,
    MIC_VOL_17   = 17,
    MIC_VOL_18   = 18,
    MIC_VOL_19   = 19,
    MIC_VOL_20   = 20,
    MIC_VOL_21   = 21,
    MIC_VOL_22   = 22,
    MIC_VOL_23   = 23,
    MIC_VOL_24   = 24,
    MIC_VOL_25   = 25,
    MIC_VOL_26   = 26,
    MIC_VOL_27   = 27,
    MIC_VOL_28   = 28,
    MIC_VOL_29   = 29,
    MIC_VOL_30   = 30,
    MIC_VOL_31   = 31,
    MIC_VOL_32   = 32,
} MIC_VOL;

typedef enum HDMI_AUDIO_MODE_TAG
{
    HDMI_AUDIO_MODE_OFF   = HDMIRX_AUDIO_OFF,
    HDMI_AUDIO_MODE_HBR   = HDMIRX_AUDIO_HBR,
    HDMI_AUDIO_MODE_DSD   = HDMIRX_AUDIO_DSD,
    HDMI_AUDIO_MODE_NLPCM = HDMIRX_AUDIO_NLPCM,
    HDMI_AUDIO_MODE_LPCM  = HDMIRX_AUDIO_LPCM,
} HDMI_AUDIO_MODE;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct GRABBER_CTRL_PARAM_TAG
{
    GRABBER_CTRL_FLAG flag;
    union
    {
        MIC_VOL              micVol;
        MMP_UINT32           lineBoost;
        DIGITAL_AUDIO_VOLUME digitalVolume;
        MMP_BOOL             bIsHDCP;
        INPUT_ASPECT_RATIO   aspectRatio;
        MMP_UINT32           audiobitrate;
        HDMI_AUDIO_MODE      hdmiAudioMode;
        MMP_UINT32           RecordLed;
        MMP_UINT32           SignalLed;
        MMP_UINT32           SD720PLed;
        MMP_UINT32           HD1080PLed;
        MMP_UINT32           AnalogLed;
        MMP_UINT32           HDMILed;
        MMP_UINT32           BlueLed;
        MMP_UINT32           GreenLed;
        MMP_UINT32           RedLed;
        MMP_UINT32           BlingRingLed;
        struct _COLOR
        {
            MMP_INT32 brightness;                   // -128 ~ 128     default : 0
            MMP_FLOAT contrast;                     // 0.0 ~ 4.0      default : 1.0
            MMP_INT32 hue;                          // 0 ~ 359        default : 0
            MMP_FLOAT saturation;                   // 0.0 ~ 4.0      default : 1.0
        } COLOR;
    };
} GRABBER_CTRL_PARAM;

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * grabber control
 */
//=============================================================================

void
GrabberControlSetParam(
    GRABBER_CTRL_PARAM *pCtrl);

void
GrabberControlGetParam(
    GRABBER_CTRL_PARAM *pCtrl);

MMP_RESULT
TOUCH_ReadI2C_8Bit(
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

MMP_RESULT
TOUCH_WriteI2C_8Bit(
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

MMP_RESULT
TOUCH_WriteFW_I2C_8Bit(
    MMP_UINT8  SubAddr,
    MMP_UINT8  *pData,
    MMP_UINT32 NByte);

MMP_BOOL
TouchPanelIntrState(
    void);

#ifdef __cplusplus
}
#endif

#endif //GRABBER_CONTROL_H