#include "mmp_types.h"
#include "host/ahb.h"
#include "host/host.h"
#include "host/gpio.h"
#include "mmp_capture.h"
#include "hdmitx/hdmitx_sys.h"
#include "mmp_hdmitx.h"
#include "mmp_hdmirx.h"
#ifndef EXTERNAL_HDMIRX
    #include "hdmirx/it6607/mmp_it6607.h"
#else
    #include "hdmirx/it6604/mmp_it6604.h"
#endif

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================
#define DEFAULT_SAMPLING_RATE  48000
#define DEFAULT_CHANNEL_NUMBER 2

static MMP_BOOL   gtDestroyThread    = MMP_FALSE;
static MMP_BOOL   gtThreadClosed     = MMP_FALSE;
static MMP_BOOL   gtIsHDMITxInit     = MMP_FALSE;
static MMP_UINT32 gtAudioChannelNum  = 0;
static MMP_UINT32 gtAudioSampleRate  = 0;

static MMP_UINT16 oldHDMIResolution  = 0xFF;
static MMP_UINT16 oldYPbPrResolution = 0xFF;
static MMP_UINT16 oldCVBSResolution  = 0xFF;

static MMP_BOOL   gtEnSPDIFIn        = MMP_FALSE;
//=============================================================================
//                              Private Function Declaration
//=============================================================================
void
_HDMIRX_Loop(
    void)
{
    MMP_BOOL              modeChange;
    MMP_UINT16            hdmiResolution;
    MMP_HDMITX_VIDEO_TYPE videoMode;
    MMP_UINT32            hdmiRxVideoMode;
    MMP_BOOL              isYUVMode;
    static MMP_BOOL       isHDMIAVMute  = MMP_FALSE;
    static MMP_UINT16     preEnableHDCP = 0;
    MMP_BOOL              changeHDCP    = MMP_FALSE;

#ifdef HDCP_ON
    MMP_UINT16            enableHDCP    = 1;
#else
    MMP_UINT16            enableHDCP    = 0;
#endif

    if (mmpCapDeviceIsSignalStable())// && hdmiResolution < CAP_HDMI_INPUT_VESA)
    {
#ifdef HDCP_ON
        enableHDCP = mmpHDMIRXIsHDCPOn() ? 1 : 0;

        if (preEnableHDCP != enableHDCP)
            changeHDCP = MMP_TRUE;
        else
            changeHDCP = MMP_FALSE;

        preEnableHDCP     = enableHDCP;
#endif

        hdmiResolution    = mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX);
        gtAudioChannelNum = mmpHDMIRXGetProperty(HDMIRX_AUDIO_CHANNEL_NUMBER);
        gtAudioSampleRate = mmpHDMIRXGetProperty(HDMIRX_AUDIO_SAMPLERATE);
        hdmiRxVideoMode   = mmpHDMIRXGetProperty(HDMIRX_OUTPUT_VIDEO_MODE);

        if (hdmiRxVideoMode == 0)
            isYUVMode = MMP_FALSE;
        else
            isYUVMode = MMP_TRUE;

        if ((oldHDMIResolution != hdmiResolution && hdmiResolution != 0xFF) ||
            (isHDMIAVMute && hdmiResolution != 0xFF))
            modeChange = 1;
        else
            modeChange = 0;

        if (modeChange)
        {
            mmpHDMITXAVMute(MMP_TRUE);
            isHDMIAVMute      = MMP_TRUE;
            mmpHDMITXDisable();
            oldHDMIResolution = 0xFF;

            if (gtIsHDMITxInit == MMP_FALSE)
            {
                mmpHDMITXInitialize(MMP_HDHITX_IN_HDMIRX);
                gtIsHDMITxInit = MMP_TRUE;
            }
        }

        if ((oldHDMIResolution != hdmiResolution && hdmiResolution != 0xFF) || changeHDCP)
        {
            if (hdmiResolution < CAP_HDMI_INPUT_VESA)
            {
                switch (hdmiResolution)
                {
                case 0:  videoMode = HDMI_480i60;       break;
                case 1:  videoMode = HDMI_480p60;       break;
                case 2:  videoMode = HDMI_576i50;       break;
                case 3:  videoMode = HDMI_576p50;       break;
                case 4:  videoMode = HDMI_720p60;       break;
                case 5:  videoMode = HDMI_720p50;       break;
                case 6:  videoMode = HDMI_1080p60;      break;
                case 7:  videoMode = HDMI_1080p50;      break;
                case 8:  videoMode = HDMI_1080i60;      break;
                case 9:  videoMode = HDMI_1080i50;      break;
                case 10: videoMode = HDMI_1080p24;      break;
                case 11: videoMode = HDMI_640x480p60;   break;
                case 12: videoMode = HDMI_1080p30;      break;
                case 13: videoMode = HDMI_1080p25;      break;
                default: videoMode = HDMI_Unkown;       break;
                }
                mmpHDMITXSetDisplayOption(videoMode, HDMITX_UNKNOWN_MODE, enableHDCP, isYUVMode);
            }
            else
            {
                MMP_UINT16          HActive, VActive;
                MMP_HDMITX_VESA_ID  VesaId = HDMITX_UNKNOWN_MODE;
                MMP_HDMIRX_DETIMING DETiming;

                VActive = (MMP_UINT16)mmpHDMIRXGetProperty(HDMIRX_HEIGHT);
                HActive = (MMP_UINT16)mmpHDMIRXGetProperty(HDMIRX_WIDTH);

                //if (HActive == 800 && VActive == 600)
                //    VesaId = HDMITX_VESA_800x600p60;
                //else if (HActive == 1024 && VActive == 768)
                //    VesaId = HDMITX_VESA_1024x768p60;
                //else if (HActive == 1280 && VActive == 768)
                //    VesaId = HDMITX_VESA_1280x768p60;
                //else if (HActive == 1280 && VActive == 800)
                //    VesaId = HDMITX_VESA_1280x800p60;
                //else if (HActive == 1280 && VActive == 960)
                //    VesaId = HDMITX_VESA_1280x960p60;
                //else if (HActive == 1280 && VActive == 1024)
                //    VesaId = HDMITX_VESA_1280x1024p60;
                //else if (HActive == 1360 && VActive == 768)
                //    VesaId = HDMITX_VESA_1360X768p60;
                //else if (HActive == 1440 && VActive == 900)
                //    VesaId = HDMITX_VESA_1440x900p60;
                //else if (HActive == 1600 && VActive == 900)
                //    VesaId = VESA_1600x900p60;
                //else if (HActive == 1600 && VActive == 1200)
                //    VesaId = VESA_1600x1200p60;
                //else if (HActive == 1680 && VActive == 1050)
                //    VesaId = HDMITX_VESA_1680x1050p60;

                VesaId = HDMITX_VESA_GET_HDMIRX_DE;
                mmpHDMIRXGetDETiming(&DETiming);
                mmpHDMITXSetDETiming(DETiming.HDES, DETiming.HDEE, DETiming.VDES, DETiming.VDEE);

                mmpHDMITXSetDisplayOption(HDMI_VESA, VesaId, enableHDCP, isYUVMode);
            }
        }
        mmpHDMITXDevLoopProc(modeChange,
                             gtAudioSampleRate,
                             gtAudioChannelNum,
                             MMP_HDHITX_IN_HDMIRX,
                             gtEnSPDIFIn);
        if (isHDMIAVMute)
            isHDMIAVMute = MMP_FALSE;

        oldHDMIResolution = hdmiResolution;
    }
    else // HDMI Rx not Stable
    {
        if (!isHDMIAVMute)
        {
            mmpHDMITXAVMute(MMP_TRUE);
            mmpHDMITXDisable();
            isHDMIAVMute  = MMP_TRUE;
            preEnableHDCP = 0;
        }
    }
}

void
_COMPOSITE_Loop(
    void)
{
    MMP_BOOL              modeChange;
    MMP_UINT16            cvbsResolution;
    MMP_HDMITX_VIDEO_TYPE videoMode;
    static MMP_BOOL       isCVBSAVMute = MMP_FALSE;

    //#ifdef HDCP_ON
    //    MMP_UINT16 enableHDCP = 1;
    //#else
    MMP_UINT16 enableHDCP = 0;
    //#endif

    cvbsResolution = mmpCapGetResolutionIndex(MMP_CAP_DEV_ADV7180);

    if (mmpCapDeviceIsSignalStable())
    {
        if ((oldCVBSResolution != cvbsResolution && cvbsResolution != 0xFF) ||
            (isCVBSAVMute && cvbsResolution != 0xFF))
            modeChange = 1;
        else
            modeChange = 0;

        if (modeChange)
        {
            mmpHDMITXAVMute(MMP_TRUE);
            isCVBSAVMute      = MMP_TRUE;
            mmpHDMITXDisable();
            oldCVBSResolution = 0xFF;
            if (!gtIsHDMITxInit)
            {
                mmpHDMITXInitialize(MMP_HDHITX_IN_CVBS);
                gtIsHDMITxInit = MMP_TRUE;
            }
        }

        gtAudioChannelNum = DEFAULT_CHANNEL_NUMBER;
        gtAudioSampleRate = DEFAULT_SAMPLING_RATE;

        if (oldCVBSResolution != cvbsResolution && cvbsResolution != 0xFF)
        {
            switch (cvbsResolution)
            {
            case 0:  videoMode = HDMI_480i60;  break;
            case 1:  videoMode = HDMI_576i50;  break;
            default: videoMode = HDMI_480i60;  break;
            }
            mmpHDMITXSetDisplayOption(videoMode, HDMITX_UNKNOWN_MODE, enableHDCP, MMP_TRUE);
        }
        mmpHDMITXDevLoopProc(modeChange,
                             gtAudioSampleRate,
                             gtAudioChannelNum,
                             MMP_HDHITX_IN_CVBS,
                             MMP_FALSE);
        if (isCVBSAVMute)
            isCVBSAVMute = MMP_FALSE;

        oldCVBSResolution = cvbsResolution;
    }
    else // not Stable
    {
        if (!isCVBSAVMute)
        {
            mmpHDMITXAVMute(MMP_TRUE);
            mmpHDMITXDisable();
            isCVBSAVMute = MMP_TRUE;
        }
    }
}

void
_COMPONENT_Loop(
    void)
{
    MMP_BOOL              modeChange;
    MMP_UINT16            ypbprResolution;
    MMP_HDMITX_VIDEO_TYPE videoMode;
    MMP_BOOL              bSignalStable;
    static MMP_BOOL       bYPbPrSignalTrigger = MMP_FALSE;
    static MMP_UINT32     keepYPbPrSignalTime = 0;
    static MMP_BOOL       isYPbPrAVMute       = MMP_FALSE;

    //#ifdef HDCP_ON
    //    MMP_UINT16 enableHDCP = 1;
    //#else
    MMP_UINT16 enableHDCP = 0;
    //#endif

    ypbprResolution = mmpCapGetResolutionIndex(MMP_CAP_DEV_CAT9883);
    bSignalStable   = mmpCapDeviceIsSignalStable();

    // signal stable
    if (!bSignalStable)
    {
        bYPbPrSignalTrigger = MMP_FALSE;
    }
    else if (bSignalStable && (!bYPbPrSignalTrigger))
    {
        keepYPbPrSignalTime = PalGetClock();
        bYPbPrSignalTrigger = MMP_TRUE;
    }

    if (PalGetDuration(keepYPbPrSignalTime) >= 2000 && bYPbPrSignalTrigger)
    {
        if ((oldYPbPrResolution != ypbprResolution && ypbprResolution != 0xFF) ||
            (isYPbPrAVMute && ypbprResolution != 0xFF))
            modeChange = 1;
        else
            modeChange = 0;

        if (modeChange)
        {
            mmpHDMITXAVMute(MMP_TRUE);
            isYPbPrAVMute      = MMP_TRUE;
            mmpHDMITXDisable();
            oldYPbPrResolution = 0xFF;
            if (!gtIsHDMITxInit)
            {
                mmpHDMITXInitialize(MMP_HDHITX_IN_YPBPR);
                gtIsHDMITxInit = MMP_TRUE;
            }
        }

        gtAudioChannelNum = DEFAULT_CHANNEL_NUMBER;
        gtAudioSampleRate = DEFAULT_SAMPLING_RATE;

        if (oldYPbPrResolution != ypbprResolution && ypbprResolution != 0xFF)
        {
            switch (ypbprResolution)
            {
            case 0:  videoMode  = HDMI_480i60;  break;
            case 1:  videoMode  = HDMI_480p60;  break;
            case 2:  videoMode  = HDMI_576i50;  break;
            case 3:  videoMode  = HDMI_576p50;  break;
            case 4:  videoMode  = HDMI_720p60;  break;
            case 5:  videoMode  = HDMI_720p50;  break;
            case 6:  videoMode  = HDMI_1080p60; break;
            case 7:  videoMode  = HDMI_1080p50; break;
            case 8:  videoMode  = HDMI_1080i60; break;
            case 9:  videoMode  = HDMI_1080i50; break;
            case 10:  videoMode = HDMI_1080p24; break;
            default: videoMode  = HDMI_480i60;  break;
            }
            mmpHDMITXSetDisplayOption(videoMode, HDMITX_UNKNOWN_MODE, enableHDCP, MMP_TRUE);
        }
        mmpHDMITXDevLoopProc(modeChange,
                             gtAudioSampleRate,
                             gtAudioChannelNum,
                             MMP_HDHITX_IN_YPBPR,
                             MMP_FALSE);

        if (isYPbPrAVMute)
            isYPbPrAVMute = MMP_FALSE;

        oldYPbPrResolution = ypbprResolution;
    }
    else // not Stable
    {
        if (!isYPbPrAVMute)
        {
            mmpHDMITXAVMute(MMP_TRUE);
            mmpHDMITXDisable();
            isYPbPrAVMute = MMP_TRUE;
        }
    }
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
HDMILoopThrough_DestroyThread()
{
    gtDestroyThread = MMP_TRUE;
}

void
HDMILoopThrough_CreateThread()
{
    gtDestroyThread = MMP_FALSE;
}

MMP_BOOL
IsHDMILoopThrough_Thread_Destroy()
{
    if (gtThreadClosed)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

void *HDMILoopThrough_ThreadFun(void *data)
{
    static MMP_CAP_DEVICE_ID preDeviceID = MMP_CAP_UNKNOW_DEVICE;
    MMP_CAP_DEVICE_ID        DeviceID;

    gtThreadClosed = MMP_FALSE;

    for (;;)
    {
        if (gtDestroyThread)
            goto end;

        if (!mmpCapGetDeviceReboot()) //Keep Through
            goto lable_Sleep;

        DeviceID = mmpCapGetCaptureDevice();
        if (preDeviceID != DeviceID)
        {
            gtIsHDMITxInit = MMP_FALSE;
            preDeviceID    = DeviceID;
        }

        switch (DeviceID)
        {
#ifdef COMPOSITE_DEV
        case MMP_CAP_DEV_ADV7180:
            _COMPOSITE_Loop();
            oldHDMIResolution  = 0xFF;
            oldYPbPrResolution = 0xFF;
            break;
#endif

#ifdef COMPONENT_DEV
        case MMP_CAP_DEV_CAT9883:
            _COMPONENT_Loop();
            oldHDMIResolution = 0xFF;
            oldCVBSResolution = 0xFF;
            break;
#endif

        case MMP_CAP_DEV_HDMIRX:
            _HDMIRX_Loop();
            oldYPbPrResolution = 0xFF;
            oldCVBSResolution  = 0xFF;
            break;

        default:
            break;
        }

lable_Sleep:
        PalSleep(100);
    }
end:
    gtThreadClosed = MMP_TRUE;
    return;
}

void HDMITXAudioUseSPDIFIn(
    MMP_BOOL flag)
{
    gtEnSPDIFIn = flag;
}