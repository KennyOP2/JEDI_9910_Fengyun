#ifndef MPS_CONTROL_H
#define MPS_CONTROL_H

#include "mmp_types.h"
#include "mmp_capture.h"
#include "mps_system.h"
#include "core_interface.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MPS_CALLBACK_REASON_TAG
{
    CALLBACK_REASON_OPEN_FILE_SUCCESS       = MPS_NOTIFY_REASON_OPEN_FILE_SUCCESS,
    CALLBACK_REASON_OPEN_FILE_FAIL          = MPS_NOTIFY_REASON_OPEN_FILE_FAIL,
    CALLBACK_REASON_END_OF_FILE             = MPS_NOTIFY_REASON_END_OF_FILE,
    CALLBACK_REASON_FILE_WRITE_FAIL         = MPS_NOTIFY_REASON_FILE_WRITE_FAIL,
    CALLBACK_REASON_FILE_NO_MORE_STORAGE    = MPS_NOTIFY_REASON_NO_MORE_STORAGE,
    CALLBACK_REASON_FILE_CLOSE_FAIL         = MPS_NOTIFY_REASON_FILE_CLOSE_FAIL,
    CALLBACK_REASON_FILE_CLOSE_SUCCESS      = MPS_NOTIFY_REASON_FILE_CLOSE_SUCCESS,
    CALLBACK_REASON_WRITE_SPEED_TOO_SLOW    = MPS_NOTIFY_REASON_WRITE_SPEED_TOO_SLOW,
    CALLBACK_REASON_TERMINATE_DONE,
    CALLBACK_REASON_TOTAL
} MPS_CALLBACK_REASON;

//=============================================================================
//                              Macro Definition
//=============================================================================
typedef void (*MPS_NOTIFY_CALLBACK)(MPS_CALLBACK_REASON, MMP_UINT32);

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function  Definition
//=============================================================================

//=============================================================================
/**
 * Used to Init the MPS system
 * @param inputFormat   Specify the player format.
 * @return              none.
 */
//=============================================================================
void
mpsCtrl_Init(
    MMP_MUX_TYPE);

//=============================================================================
/**
 * Used to Terminate the player system.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_Terminate(
    void);

//=============================================================================
/**
 * Used to announce the player system to start playing.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_Play(
    MMP_BOOL    bSync);

//=============================================================================
/**
 * Used to announce the player system about the stop operation.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_Stop(
    MMP_BOOL    bSync);

void
mpsCtrl_SetProperity(
    MMP_UINT32  properity,
    MMP_UINT32  data,
    MMP_BOOL    bSync);

void
mpsCtrl_GetProperity(
    MMP_UINT32 properity,
    MMP_UINT32 data,
    MMP_BOOL   bSync);

//void
//mpsCtrl_SetEncodeParameter(
//    VIDEO_ENCODER_PARAMETER*   para);

//void
//mpsCtrl_SetCaptureDevice(
//    CAPTURE_DEVICE_INFO*   capDeviceInfo);

//void
//mpsCtrl_EnableISPOnFly(
//    MMP_BOOL bEnableOnly);

void
mpsCtrl_EnableAVEngine(
    MMP_BOOL bEnableAVEngine);

//=============================================================================
/**
 * Used to Attach callback function for a specific reason handling.
 * @param reason        The desired notification callback reason.
 * @param pfCallback    The callback function for handling the callback reason.
 * return               none.
 */
//=============================================================================
void
mpsCtrl_AttachCallback(
    MPS_CALLBACK_REASON reason,
    MPS_NOTIFY_CALLBACK pfCallback);

//=============================================================================
/**
 * Used to Detach callback function for a specific reason handling.
 * @param reason        The desired notification callback reason.
 * return               none.
 */
//=============================================================================
void
mpsCtrl_DetachCallback(
    MPS_CALLBACK_REASON reason);

#ifdef ENABLE_MENCODER
//=============================================================================
/**
 * Used to announce the player system to start recording.
 * @param demodId    A related demod id and record index.
 * @param pfCallback A notification function will be called once the player
 *                   is triggered by some special events.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StartRecord(
    MMP_WCHAR*          filePath,
    //MMP_CHAR*           filePath,
    MPS_NOTIFY_CALLBACK pfCallback,
    MMP_BOOL            bSync);

//=============================================================================
/**
 * Used to announce the player system to stop recording.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StopRecord(
    MMP_BOOL    bSync);
#endif

//=============================================================================
/**
 * Used to announce the player system to start jpeg recording.
 * @param pfCallback A notification function will be called once the player
 *                   is triggered by some special events.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StartJPEGRecord(
    JPEG_CONTEXT* ptJpgCtx,
    MMP_BOOL      bSync);

//=============================================================================
/**
 * Used to announce the player system to stop jpeg recording.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StopJPEGRecord(
    MMP_BOOL    bSync);
#endif
