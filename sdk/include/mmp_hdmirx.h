#ifndef __MMP_HDMIRX_H__
#define __MMP_HDMIRX_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXTERNAL_HDMIRX
    #include "hdmirx/it6607/mmp_it6607.h"
#else
    #include "hdmirx/it6604/mmp_it6604.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(WIN32)

    #if defined(HDMIRX_EXPORTS)
        #define HDMIRX_API __declspec(dllexport)
    #else
        #define HDMIRX_API __declspec(dllimport)
    #endif

#else
    #define HDMIRX_API     extern
#endif //#if defined(WIN32)

//=============================================================================
//                              Constant Definition
//=============================================================================
#ifndef EXTERNAL_HDMIRX

typedef enum MMP_HDMIRX_PROPERTY_TAG
{
    HDMIRX_IS_DVI_MODE          = IT6607_IS_DVI_MODE,
    HDMIRX_CHECK_ENGINE_IDLE    = IT6607_CHECK_ENGINE_IDLE,
    HDMIRX_ASPECT_RATIO         = IT6607_ASPECT_RATIO,
    HDMIRX_OUTPUT_VIDEO_MODE    = IT6607_OUTPUT_VIDEO_MODE,
    HDMIRX_PCLK                 = IT6607_PCLK,
    HDMIRX_HEIGHT               = IT6607_HEIGHT,
    HDMIRX_WIDTH                = IT6607_WIDTH,
    HDMIRX_HTOTAL               = IT6607_HTOTAL,
    HDMIRX_VTOTAL               = IT6607_VTOTAL,
    HDMIRX_COLOR_DEPTH          = IT6607_COLOR_DEPTH,
    HDMIRX_IS_INTERLACE         = IT6607_IS_INTERLACE,
    HDMIRX_IS_VID_STABLE        = IT6607_IS_VID_STABLE,
    HDMIRX_IS_AUDIO_ON          = IT6607_IS_AUDIO_ON,
    HDMIRX_IS_AUDIO_RESET       = IT6607_IS_AUDIO_RESET,
    HDMIRX_AUDIO_SAMPLERATE     = IT6607_AUDIO_SAMPLERATE,
    HDMIRX_AUDIO_CHANNEL_NUMBER = IT6607_AUDIO_CHANNEL_NUMBER,
    HDMIRX_AUDIO_MODE           = IT6607_AUDIO_MODE,
    HDMIRX_CHECK_PLG5V_PWR      = IT6607_CHECK_PLG5V_PWR,
} MMP_HDMIRX_PROPERTY;

typedef enum MMP_HDMIRX_AUDIO_MODE_TAG
{
    HDMIRX_AUDIO_OFF   = IT6607_AUDIO_OFF,
    HDMIRX_AUDIO_HBR   = IT6607_AUDIO_HBR,
    HDMIRX_AUDIO_DSD   = IT6607_AUDIO_DSD,
    HDMIRX_AUDIO_NLPCM = IT6607_AUDIO_NLPCM,
    HDMIRX_AUDIO_LPCM  = IT6607_AUDIO_LPCM,
} MMP_HDMIRX_AUDIO_MODE;

typedef enum MMP_HDMIRX_PAR_TAG
{
    HDMIRX_PAR_4_3  = IT6607_PAR_4_3,
    HDMIRX_PAR_16_9 = IT6607_PAR_16_9,
} MMP_HDMIRX_PAR;

#else

typedef enum MMP_HDMIRX_PROPERTY_TAG
{
    HDMIRX_IS_DVI_MODE          = IT6604_IS_DVI_MODE,
    HDMIRX_CHECK_ENGINE_IDLE    = IT6604_CHECK_ENGINE_IDLE,
    HDMIRX_ASPECT_RATIO         = IT6604_ASPECT_RATIO,
    HDMIRX_OUTPUT_VIDEO_MODE    = IT6604_OUTPUT_VIDEO_MODE,
    HDMIRX_PCLK                 = IT6604_PCLK,
    HDMIRX_HEIGHT               = IT6604_HEIGHT,
    HDMIRX_WIDTH                = IT6604_WIDTH,
    HDMIRX_HTOTAL               = IT6604_HTOTAL,
    HDMIRX_VTOTAL               = IT6604_VTOTAL,
    HDMIRX_COLOR_DEPTH          = IT6604_COLOR_DEPTH,
    HDMIRX_IS_INTERLACE         = IT6604_IS_INTERLACE,
    HDMIRX_IS_VID_STABLE        = IT6604_IS_VID_STABLE,
    HDMIRX_IS_AUDIO_ON          = IT6604_IS_AUDIO_ON,
    HDMIRX_IS_AUDIO_RESET       = IT6604_IS_AUDIO_RESET,
    HDMIRX_AUDIO_SAMPLERATE     = IT6604_AUDIO_SAMPLERATE,
    HDMIRX_AUDIO_CHANNEL_NUMBER = IT6604_AUDIO_CHANNEL_NUMBER,
    HDMIRX_AUDIO_MODE           = IT6604_AUDIO_MODE,
    HDMIRX_CHECK_PLG5V_PWR      = IT6604_CHECK_PLG5V_PWR,
} MMP_HDMIRX_PROPERTY;

typedef enum MMP_HDMIRX_AUDIO_MODE_TAG
{
    HDMIRX_AUDIO_OFF   = IT6604_AUDIO_OFF,
    HDMIRX_AUDIO_HBR   = IT6604_AUDIO_HBR,
    HDMIRX_AUDIO_DSD   = IT6604_AUDIO_DSD,
    HDMIRX_AUDIO_NLPCM = IT6604_AUDIO_NLPCM,
    HDMIRX_AUDIO_LPCM  = IT6604_AUDIO_LPCM,
} MMP_HDMIRX_AUDIO_MODE;

typedef enum MMP_HDMIRX_PAR_TAG
{
    HDMIRX_PAR_4_3  = IT6604_PAR_4_3,
    HDMIRX_PAR_16_9 = IT6604_PAR_16_9,
} MMP_HDMIRX_PAR;

#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct MMP_HDMIRX_DETIMING_TAG
{
    MMP_UINT32 HDES;
    MMP_UINT32 HDEE;
    MMP_UINT32 VDES;
    MMP_UINT32 VDEE;
} MMP_HDMIRX_DETIMING;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Device Initialize
 */
//=============================================================================
HDMIRX_API void
mmpHDMIRXInitialize(
    void);

//=============================================================================
/**
 * Device Terminate
 */
//=============================================================================
void
mmpHDMIRXTerminate(
    void);

//=============================================================================
/**
 * Device Output Pin Tri-State.
 */
//=============================================================================
HDMIRX_API void
mmpHDMIRXOutputPinTriState(
    MMP_BOOL flag);

//=============================================================================
/**
 * Device Signal State
 */
//=============================================================================
HDMIRX_API MMP_BOOL
mmpHDMIRXIsSignalStable(
    void);

//=============================================================================
/**
 * Device property.
 */
//=============================================================================
HDMIRX_API MMP_UINT32
mmpHDMIRXGetProperty(
    MMP_HDMIRX_PROPERTY property);

//=============================================================================
/**
 * * Device set property.
 */
//=============================================================================
HDMIRX_API void
mmpHDMIRXSetProperty(
    MMP_HDMIRX_PROPERTY property, MMP_UINT32 value);

//=============================================================================
/**
 * * Check HDCP.
 */
//=============================================================================
MMP_BOOL
mmpHDMIRXIsHDCPOn(
    void);

//=============================================================================
/**
 * * Get DE Timing.
 */
//=============================================================================
void
mmpHDMIRXGetDETiming(
    MMP_HDMIRX_DETIMING *pDETiming);

//=============================================================================
/**
 * Device power down
 */
//=============================================================================
MMP_BOOL
mmpHDMIRXPowerDown(
    MMP_BOOL enable);

//=============================================================================
/**
 * HDCP ON/OFF
 */
//=============================================================================
void
mmpHDMIRXDisableHDCP(
    MMP_BOOL flag);

MMP_BOOL
mmpHDMIRXIsDisableHDCP(
    void);

MMP_BOOL
mmpHDMIRXIsHDCPKeyEmpty(
    void);

void
mmpHDMIRXResetAudioFS(
    void);

#ifdef __cplusplus
}
#endif

#endif