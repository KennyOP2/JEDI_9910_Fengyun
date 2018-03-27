/*
 * Copyright (c) 2010 ITE. All Rights Reserved.
 */
/** @file
 * Configurations.
 *
 * @author Odin He
 * @version 1.0
 */
#include "mmp_types.h"
#include "hdmitx/typedef.h"
#include "hdmitx/hdmitx_drv.h"
#include "hdmitx/hdmitx_sys.h"   
#include "mmp_hdmitx.h"

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * HDMI TX initialization.
 */
//=============================================================================
void 
mmpHDMITXInitialize(
    MMP_HDMITX_INPUT_DEVICE inputDevice)
{
    InitHDMITX_Instance(inputDevice);  
}

//=============================================================================
/**
 * HDMI TX Loop Process.
 */
//=============================================================================
void 
mmpHDMITXDevLoopProc(
    MMP_BOOL bHDMIRxModeChange, 
    MMP_UINT32 AudioSampleRate, 
    MMP_UINT32 AudioChannelNum,
    MMP_HDMITX_INPUT_DEVICE inputDevice,
    MMP_BOOL EnSPDIFIn)
{
#ifdef HDMITX_IT66121
    HDMITX_DevLoopProc(bHDMIRxModeChange, AudioSampleRate, AudioChannelNum, inputDevice);
#else
    HDMITX_DevLoopProc(bHDMIRxModeChange, AudioSampleRate, AudioChannelNum, inputDevice, EnSPDIFIn);
#endif
}

//=============================================================================
/**
 * HDMI TX Disable.
 */
//=============================================================================

void
mmpHDMITXDisable(
     void)
{
    HDMITXDisable();
}     

//=============================================================================
/**
 * HDMI TX AV Mute.
 */
//=============================================================================

void
mmpHDMITXAVMute(
     MMP_BOOL isEnable)
{
    if (isEnable)
    {
        SetAVMute(TRUE);
    }
    else
    {
        SetAVMute(FALSE);
    }
}     

//=============================================================================
/**
 * HDMI TX Set Display Option.
 */
//=============================================================================
void 
mmpHDMITXSetDisplayOption(
    MMP_HDMITX_VIDEO_TYPE VideoMode,
    MMP_HDMITX_VESA_ID    VesaTiming,
    MMP_UINT16            EnableHDCP,
    MMP_BOOL              IsYUVInput)
{
    HDMITX_ChangeDisplayOption(VideoMode, VesaTiming, EnableHDCP, IsYUVInput);
}

//=============================================================================
/**
 * HDMI TX Set DE Timing.
 */
//=============================================================================
void 
mmpHDMITXSetDETiming(
    MMP_UINT32  HDES,
    MMP_UINT32  HDEE,
    MMP_UINT32  VDES,
    MMP_UINT32  VDEE)
{
    timingDE.HDES = HDES;
    timingDE.HDEE = HDEE;
    timingDE.VDES = VDES;
    timingDE.VDEE = VDEE;
}

#ifdef HDMITX_IT66121
//=============================================================================
/**
 * HDMI TX check IC.
 */
//=============================================================================
MMP_BOOL
mmpHDMITXIsChipEmpty(
    void)
{
    return HDMITX_IsChipEmpty();
}
#endif
