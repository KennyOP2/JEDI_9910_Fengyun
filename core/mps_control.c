#include "pal/pal.h"
#include "mps_control.h"
#include "queue_mgr.h"
#include "task_audio_in.h"
#include "task_video_encoder.h"
#include "task_stream_mux.h"
#ifdef ENABLE_MENCODER
    #include "task_avi_mux.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
//#define DEBUG_CMD_DISPATCH

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct MPS_CTRL_HANDLE_TAG
{
    MPS_SYSTEM_HANDLE   *ptMpsSystem;
    MPS_SYSTEM_TYPE     systemType;
    PAL_THREAD          ptMpsCtrlThread;
    MPS_STATE           mpsState;
    MPS_CMD_OBJ         tCmdObj;
    MMP_EVENT           eventCommandCompleted;
    MMP_BOOL            bTerminated;
    MPS_NOTIFY_CALLBACK pfCallbackArray[CALLBACK_REASON_TOTAL];
#ifdef ENABLE_MENCODER
    MMP_WCHAR           *ptFilePath;
    //MMP_CHAR*           ptFilePath;
#endif
} MPS_CTRL_HANDLE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MPS_CTRL_HANDLE gtMpsHandle = { 0 };
MMP_EVENT              elementThreadToMpsThread;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
//
static void
_MPS_streamMuxInit(
    void);

static void
_MPS_streamMuxTerminate(
    void);
//#else
#ifdef ENABLE_MENCODER
static void
_MPS_aviMuxInit(
    void);

static void
_MPS_aviMuxTerminate(
    void);
#endif

static void *
_MPS_CTRL_ThreadFunction(
    void *arg);

//=============================================================================
//                              Public Function Definition
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
    MMP_MUX_TYPE mux_type)
{
    if (gtMpsHandle.ptMpsSystem)
        return;

    queueMgr_InitManager();
    gtMpsHandle.ptMpsSystem = mpsSys_GetHandle();

    if (mux_type == MMP_TS_MUX)
    {
        _MPS_streamMuxInit();
        gtMpsHandle.systemType = MPS_SYSTEM_TYPE_STREAM_MUX;
    }
#ifdef ENABLE_MENCODER
    else
    {
        _MPS_aviMuxInit();
        gtMpsHandle.systemType = MPS_SYSTEM_TYPE_AVI_MUX;
    }
#endif
    elementThreadToMpsThread          = PalCreateEvent(PAL_EVENT_ELEMENT_THREAD_TO_MPS_THREAD);
    gtMpsHandle.eventCommandCompleted = PalCreateEvent(PAL_EVENT_COMMAND_COMPLETED);
    gtMpsHandle.bTerminated           = MMP_FALSE;

    if (gtMpsHandle.ptMpsCtrlThread)
        PalDestroyThread(gtMpsHandle.ptMpsCtrlThread);
    gtMpsHandle.ptMpsCtrlThread = PalCreateThread(PAL_THREAD_MPS,
                                                  _MPS_CTRL_ThreadFunction,
                                                  MMP_NULL, 0, 0);

    gtMpsHandle.tCmdObj.cmd       = MPS_COMMAND_INIT | MPS_COMMAND_SYNC_BIT_MASK;
    gtMpsHandle.tCmdObj.extraData = MMP_NULL;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

#ifdef ENABLE_MENCODER
//=============================================================================
/**
 * Used to close the opened file..
 * @param bSync      Whether the command is a synchronous command.
 * @return           none.
 */
//=============================================================================
void
mpsCtrl_Close(
    MMP_BOOL bSync)
{
    PalHeapFree(PAL_HEAP_DEFAULT, gtMpsHandle.ptFilePath);
    gtMpsHandle.ptFilePath = MMP_NULL;

    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_CLOSE | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_CLOSE;

    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}
#endif

//=============================================================================
/**
 * Used to Terminate the player system.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_Terminate(
    void)
{
    // stop first if still running
    if (MPS_STATE_RUN == gtMpsHandle.mpsState)
    {
        gtMpsHandle.tCmdObj.cmd       = MPS_COMMAND_STOP | MPS_COMMAND_SYNC_BIT_MASK;;
        gtMpsHandle.tCmdObj.extraData = MMP_NULL;
        mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
        //dbg_msg(DBG_MSG_TYPE_ERROR, "%s(%d)\n", __FILE__, __LINE__);
    }

    gtMpsHandle.tCmdObj.cmd       = MPS_COMMAND_TERMINATE | MPS_COMMAND_SYNC_BIT_MASK;
    gtMpsHandle.tCmdObj.extraData = MMP_NULL;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
    //dbg_msg(DBG_MSG_TYPE_ERROR, "%s(%d)\n", __FILE__, __LINE__);

    PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);

    switch (gtMpsHandle.systemType)
    {
    case MPS_SYSTEM_TYPE_STREAM_MUX:
        _MPS_streamMuxTerminate();
        break;

#ifdef ENABLE_MENCODER
    case MPS_SYSTEM_TYPE_AVI_MUX:
        _MPS_aviMuxTerminate();
        break;

#endif
    default:
        return;
    }
    gtMpsHandle.systemType = MPS_SYSTEM_TYPE_UNKNOWN;

    while (MMP_FALSE == gtMpsHandle.bTerminated)
        PalSleep(5);

    PalDestroyEvent(gtMpsHandle.eventCommandCompleted);
    PalDestroyEvent(elementThreadToMpsThread);
    PalDestroyThread(gtMpsHandle.ptMpsCtrlThread);
    gtMpsHandle.ptMpsCtrlThread = MMP_NULL;
}

//=============================================================================
/**
 * Used to announce the player system to start playing.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_Play(
    MMP_BOOL bSync)
{
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_PLAY | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_PLAY;

    gtMpsHandle.tCmdObj.extraData = MMP_NULL;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

//=============================================================================
/**
 * Used to announce the player system about the stop operation.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_Stop(
    MMP_BOOL bSync)
{
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_STOP | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_STOP;

    gtMpsHandle.tCmdObj.extraData = MMP_NULL;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

//void
//mpsCtrl_SetEncodeParameter(
//    VIDEO_ENCODER_PARAMETER*   para)
//{
//    MPS_PROPERITY_DATA* ptData = MMP_NULL;
//
//    gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_SET_ENCODE_PARAMETER | MPS_COMMAND_SYNC_BIT_MASK;
//
//    ptData = (MPS_PROPERITY_DATA*) PalHeapAlloc(PAL_HEAP_DEFAULT,
//                                                sizeof(MPS_PROPERITY_DATA));
//
//    para->EnWidth     = (para->EnWidth  >> 3) << 3;
//    para->EnHeight    = (para->EnHeight >> 3) << 3;
//    ptData->properityId = MPS_PROPERITY_SET_ENCODE_PARAMETER;
//    ptData->data    = (MMP_UINT32)para;
//    gtMpsHandle.tCmdObj.extraData = (void*) ptData;
//    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
//
//    PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
//}

//void
//mpsCtrl_SetCaptureDevice(
//    CAPTURE_DEVICE_INFO*   capDeviceInfo)
//{
//    MPS_PROPERITY_DATA* ptData = MMP_NULL;
//
//    gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_SET_CAPTURE_DEVICE | MPS_COMMAND_SYNC_BIT_MASK;
//
//    ptData = (MPS_PROPERITY_DATA*) PalHeapAlloc(PAL_HEAP_DEFAULT,
//                                                sizeof(MPS_PROPERITY_DATA));
//
//    ptData->properityId = MPS_COMMAND_SET_CAPTURE_DEVICE;
//    ptData->data    = (MMP_UINT32)capDeviceInfo;
//    gtMpsHandle.tCmdObj.extraData = (void*) ptData;
//    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
//
//    PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
//}

//void
//mpsCtrl_EnableISPOnFly(
//    MMP_BOOL bEnableOnly)
//{
//    MPS_PROPERITY_DATA* ptData = MMP_NULL;
//
//    gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_SET_ISP_MODE | MPS_COMMAND_SYNC_BIT_MASK;
//
//    ptData = (MPS_PROPERITY_DATA*) PalHeapAlloc(PAL_HEAP_DEFAULT,
//                                                sizeof(MPS_PROPERITY_DATA));
//
//    ptData->properityId = MPS_COMMAND_SET_ISP_MODE;
//    ptData->data    = (MMP_UINT32)bEnableOnly;
//    gtMpsHandle.tCmdObj.extraData = (void*) ptData;
//    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
//
//    PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
//}

//void
//mpsCtrl_EnableAVEngine(
//    MMP_BOOL bEnableAVEngine)
//{
//    MPS_PROPERITY_DATA* ptData = MMP_NULL;
//
//    gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_SET_ENABLE_AV_ENGINE | MPS_COMMAND_SYNC_BIT_MASK;
//
//    ptData = (MPS_PROPERITY_DATA*) PalHeapAlloc(PAL_HEAP_DEFAULT,
//                                                sizeof(MPS_PROPERITY_DATA));
//
//    ptData->properityId = MPS_COMMAND_SET_ENABLE_AV_ENGINE;
//    ptData->data    = (MMP_UINT32)bEnableAVEngine;
//    gtMpsHandle.tCmdObj.extraData = (void*) ptData;
//    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
//
//    PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
//}

//=============================================================================
/**
 * Used to Attach callback function for a specific reason handling.
 * @param reason        The desired notification callback reason.
 * @param pfCallback    The callback function for handling the callback reason.
 * return               none.
 */
//=============================================================================
void
mpsCtrl_SetProperity(
    MMP_UINT32 properity,
    MMP_UINT32 data,
    MMP_BOOL   bSync)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;
    if (MPS_STATE_ZERO == gtMpsHandle.mpsState)
        return;
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_SET_PROPERTY | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_SET_PROPERTY;
    ptData                        = (MPS_PROPERITY_DATA *) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                        sizeof(MPS_PROPERITY_DATA));
    ptData->properityId           = properity;
    ptData->data                  = data;
    gtMpsHandle.tCmdObj.extraData = (void *) ptData;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

void
mpsCtrl_GetProperity(
    MMP_UINT32 properity,
    MMP_UINT32 data,
    MMP_BOOL   bSync)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;
    if (MPS_STATE_ZERO == gtMpsHandle.mpsState)
        return;
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_GET_PROPERTY | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_GET_PROPERTY;
    ptData                        = (MPS_PROPERITY_DATA *) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                        sizeof(MPS_PROPERITY_DATA));
    ptData->properityId           = properity;
    ptData->data                  = data;
    gtMpsHandle.tCmdObj.extraData = (void *) ptData;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

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
    MPS_NOTIFY_CALLBACK pfCallback)
{
    gtMpsHandle.pfCallbackArray[reason] = pfCallback;
}

//=============================================================================
/**
 * Used to Detach callback function for a specific reason handling.
 * @param reason        The desired notification callback reason.
 * return               none.
 */
//=============================================================================
void
mpsCtrl_DetachCallback(
    MPS_CALLBACK_REASON reason)
{
    gtMpsHandle.pfCallbackArray[reason] = MMP_NULL;
}

#ifdef ENABLE_MENCODER
//=============================================================================
/**
 * Used to announce the player system to start recording.
 * @param pfCallback A notification function will be called once the player
 *                   is triggered by some special events.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StartRecord(
    MMP_WCHAR           *filePath,
    //MMP_CHAR*           filePath,
    MPS_NOTIFY_CALLBACK pfCallback,
    MMP_BOOL            bSync)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;

    if (!filePath)
        return;

    if (pfCallback != MMP_NULL)
    {
        mpsCtrl_AttachCallback(CALLBACK_REASON_OPEN_FILE_SUCCESS, pfCallback);
        mpsCtrl_AttachCallback(CALLBACK_REASON_OPEN_FILE_FAIL, pfCallback);
        mpsCtrl_AttachCallback(CALLBACK_REASON_END_OF_FILE, pfCallback);
    }

    if (gtMpsHandle.ptFilePath)
    {
        // The new file is same as the opened file.
        if (PalWcscmp(gtMpsHandle.ptFilePath, filePath) == 0)
        //if (strcmp(gtMpsHandle.ptFilePath, filePath) == 0)
        {
            goto end;
        }
        else
        {
            PalHeapFree(PAL_HEAP_DEFAULT, gtMpsHandle.ptFilePath);
            gtMpsHandle.ptFilePath        = MMP_NULL;

            // Close old opened file first.
            gtMpsHandle.tCmdObj.cmd       = MPS_COMMAND_CLOSE;
            gtMpsHandle.tCmdObj.extraData = MMP_NULL;
            mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);
        }
    }
    gtMpsHandle.ptFilePath = PalWcsdup(filePath);
    //gtMpsHandle.ptFilePath = strdup(filePath);
    PalWcscpy(gtMpsHandle.ptFilePath, filePath);
    //strcpy(gtMpsHandle.ptFilePath, filePath);

end:
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_START_RECORD | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_START_RECORD;

    ptData                        = (MPS_PROPERITY_DATA *) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                        sizeof(MPS_PROPERITY_DATA));
    //ptData->properityId = MPS_FILE_NAME_ASSIGN;
    ptData->data                  = (MMP_UINT32) gtMpsHandle.ptFilePath;

    gtMpsHandle.tCmdObj.extraData = (void *)ptData;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

//=============================================================================
/**
 * Used to announce the player system to stop recording.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StopRecord(
    MMP_BOOL bSync)
{
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_STOP_RECORD | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_STOP_RECORD;

    gtMpsHandle.tCmdObj.extraData = (void *)MMP_NULL;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}
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
    JPEG_CONTEXT *ptJpgCtx,
    MMP_BOOL     bSync)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;

    if (!ptJpgCtx)
        return;

    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_START_JPEG_RECORD | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_START_JPEG_RECORD;

    ptData                        = (MPS_PROPERITY_DATA *) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                        sizeof(MPS_PROPERITY_DATA));
    ptData->data                  = (MMP_UINT32) ptJpgCtx;

    gtMpsHandle.tCmdObj.extraData = (void *)ptData;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

//=============================================================================
/**
 * Used to announce the player system to stop jpeg recording.
 * @param bSync      Whether the command is a synchronous command.
 * @return none.
 */
//=============================================================================
void
mpsCtrl_StopJPEGRecord(
    MMP_BOOL bSync)
{
    if (bSync)
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_STOP_JPEG_RECORD | MPS_COMMAND_SYNC_BIT_MASK;
    else
        gtMpsHandle.tCmdObj.cmd = MPS_COMMAND_STOP_JPEG_RECORD;

    gtMpsHandle.tCmdObj.extraData = (void *)MMP_NULL;
    mpsCmdQ_SendCommand(MPS_CTRL_CMD_ID, &gtMpsHandle.tCmdObj);

    if (bSync)
        PalWaitEvent(gtMpsHandle.eventCommandCompleted, PAL_EVENT_INFINITE);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Used to init a ts Streaming player system.
 * @return none.
 */
//=============================================================================
static void
_MPS_streamMuxInit(
    void)
{
    //MPS_CONNECTOR*  ptCaptureConnector = MMP_NULL;
    MPS_CONNECTOR     *ptAudioConnector = MMP_NULL;
    MPS_CONNECTOR     *ptVideoConnector = MMP_NULL;

    //MPS_ELEMENT*    ptVideoCapture = MMP_NULL;
    MPS_ELEMENT       *ptAudioIn        = MMP_NULL;
    MPS_ELEMENT       *ptVideoEncoder   = MMP_NULL;
    MPS_ELEMENT       *ptStreamMux      = MMP_NULL;

    QUEUE_CTRL_HANDLE *ptHandle         = MMP_NULL;

    // The command queue acts as a event notify queue for all elements and
    // the MPS Ctrl Task.
    mpsCmdQ_Init(MPS_CTRL_EVENT_ID);

    // A special event queue to transmit some special case such as END_OF_FILE,
    // FILE_OPEN_FAILED.
    mpsCmdQ_Init(MPS_CTRL_SPECIAL_EVENT_ID);

    // The command queue is used to accept the upper layer operation request.
    mpsCmdQ_Init(MPS_CTRL_CMD_ID);

    // The following command queue is used for MPS Ctrl task to dispatch
    // commands to different tasks.
    //mpsCmdQ_Init(VIDEO_CAPTURE_CMD_ID);
    mpsCmdQ_Init(AUDIO_IN_CMD_ID);
    mpsCmdQ_Init(VIDEO_ENCODER_CMD_ID);
    mpsCmdQ_Init(STREAM_MUX_CMD_ID);

    // Video Capture -> Video Encoder
    // Step 1. Create a new connector abstraction object.
    //ptCaptureConnector = mpsSys_CreateNewConnector();

    // Step 2. Create a queue to be a data bus between two elements.
    //queueMgr_CreateQueue(CAPTURE_QUEUE_ID,
    //                     sizeof(CAPTURE_SAMPLE),
    //                     CAPTURE_SAMPLE_COUNT);
    //ptCaptureConnector->queueId = CAPTURE_QUEUE_ID;

    // Step 3. Create Source element and destination element for the connector.
    //ptVideoCapture = mpsSys_CreateNewElement(MPS_VIDEO_CAPTURE, VIDEO_CAPTURE_CMD_ID);
    //ptVideoCapture->pfInit = (OPERATION_API)taskVideoCapture_Init;
    //ptVideoCapture->pfTerminate = (OPERATION_API)taskVideoCapture_Terminate;
    //ptHandle = queueMgr_GetCtrlHandle(CAPTURE_QUEUE_ID);
    //ptHandle->pfSetFree = taskVideoCapture_SetFree;

    ptVideoEncoder              = mpsSys_CreateNewElement(MPS_VIDEO_ENCODER, VIDEO_ENCODER_CMD_ID);
    ptVideoEncoder->pfInit      = (OPERATION_API)taskVideoEncoder_Init;
    ptVideoEncoder->pfTerminate = (OPERATION_API)taskVideoEncoder_Terminate;

    // Hook the connector on the elements.
    //mpsSys_HookConnector(ptVideoCapture, MPS_SRC_ELEMENT, ptCaptureConnector);
    //mpsSys_HookConnector(ptVideoEncoder, MPS_DEST_ELEMENT, ptCaptureConnector);

    // Audio In -> Stream Mux
    // Same steps as previous input stream example to construct a audio link.
    ptAudioConnector       = mpsSys_CreateNewConnector();

    ptAudioIn              = mpsSys_CreateNewElement(MPS_AUDIO_IN, AUDIO_IN_CMD_ID);
    ptAudioIn->pfInit      = (OPERATION_API)taskAudioIn_Init;
    ptAudioIn->pfTerminate = (OPERATION_API)taskAudioIn_Terminate;

    queueMgr_CreateQueue(AUDIO_QUEUE_ID,
                         sizeof(AUDIO_SAMPLE),
                         AUDIO_SAMPLE_COUNT);
    ptHandle                  = queueMgr_GetCtrlHandle(AUDIO_QUEUE_ID);
    ptHandle->pfSetFree       = taskAudioIn_SetFree;
    ptAudioConnector->queueId = AUDIO_QUEUE_ID;

    ptStreamMux               = mpsSys_CreateNewElement(MPS_STREAM_MUX, STREAM_MUX_CMD_ID);
    ptStreamMux->pfInit       = (OPERATION_API)taskStreamMux_Init;
    ptStreamMux->pfTerminate  = (OPERATION_API)taskStreamMux_Terminate;

    mpsSys_HookConnector(ptAudioIn, MPS_SRC_ELEMENT, ptAudioConnector);
    mpsSys_HookConnector(ptStreamMux, MPS_DEST_ELEMENT, ptAudioConnector);

    // Video Encoder -> Stream Mux
    // Same steps as previous input stream example to construct a audio link.
    ptVideoConnector = mpsSys_CreateNewConnector();

    queueMgr_CreateQueue(VIDEO_QUEUE_ID,
                         sizeof(VIDEO_SAMPLE),
                         VIDEO_SAMPLE_COUNT);
    ptHandle                  = queueMgr_GetCtrlHandle(VIDEO_QUEUE_ID);
    ptHandle->pfSetFree       = taskVideoEncoder_SetFree;
    ptVideoConnector->queueId = VIDEO_QUEUE_ID;

    mpsSys_HookConnector(ptVideoEncoder, MPS_SRC_ELEMENT, ptVideoConnector);
    mpsSys_HookConnector(ptStreamMux, MPS_DEST_ELEMENT, ptVideoConnector);
}

//=============================================================================
/**
 * Used to release resouce allocated by ts Streaming player system.
 * @return none.
 */
//=============================================================================
static void
_MPS_streamMuxTerminate(
    void)
{
    // Step 1 destroy all created command queue.
    // the MPS Ctrl Task.
    mpsCmdQ_Terminate(MPS_CTRL_EVENT_ID);
    mpsCmdQ_Terminate(MPS_CTRL_SPECIAL_EVENT_ID);
    mpsCmdQ_Terminate(MPS_CTRL_CMD_ID);
    //mpsCmdQ_Terminate(VIDEO_CAPTURE_CMD_ID);
    mpsCmdQ_Terminate(AUDIO_IN_CMD_ID);
    mpsCmdQ_Terminate(VIDEO_ENCODER_CMD_ID);
    mpsCmdQ_Terminate(STREAM_MUX_CMD_ID);

    // Step 2. Destroy all created smaple queue.
    //queueMgr_DestroyQueue(CAPTURE_QUEUE_ID);
    //queueMgr_DestroyQueue(AUDIO_QUEUE_ID);
    //queueMgr_DestroyQueue(VIDEO_QUEUE_ID);
}

#ifdef ENABLE_MENCODER
//=============================================================================
/**
 * Used to init a ts Streaming player system.
 * @return none.
 */
//=============================================================================
static void
_MPS_aviMuxInit(
    void)
{
    //MPS_CONNECTOR*  ptCaptureConnector  = MMP_NULL;
    MPS_CONNECTOR     *ptAudioConnector = MMP_NULL;
    MPS_CONNECTOR     *ptVideoConnector = MMP_NULL;

    //MPS_ELEMENT*    ptVideoCapture      = MMP_NULL;
    MPS_ELEMENT       *ptAudioIn        = MMP_NULL;
    MPS_ELEMENT       *ptVideoEncoder   = MMP_NULL;
    MPS_ELEMENT       *ptAviMux         = MMP_NULL;

    QUEUE_CTRL_HANDLE *ptHandle         = MMP_NULL;

    // The command queue acts as a event notify queue for all elements and
    // the MPS Ctrl Task.
    mpsCmdQ_Init(MPS_CTRL_EVENT_ID);

    // A special event queue to transmit some special case such as END_OF_FILE,
    // FILE_OPEN_FAILED.
    mpsCmdQ_Init(MPS_CTRL_SPECIAL_EVENT_ID);

    // The command queue is used to accept the upper layer operation request.
    mpsCmdQ_Init(MPS_CTRL_CMD_ID);

    // The following command queue is used for MPS Ctrl task to dispatch
    // commands to different tasks.
    //mpsCmdQ_Init(VIDEO_CAPTURE_CMD_ID);
    mpsCmdQ_Init(AUDIO_IN_CMD_ID);
    mpsCmdQ_Init(VIDEO_ENCODER_CMD_ID);
    mpsCmdQ_Init(AVI_MUX_CMD_ID);

    // Video Capture -> Video Encoder
    // Step 1. Create a new connector abstraction object.
    //ptCaptureConnector = mpsSys_CreateNewConnector();

    // Step 3. Create Source element and destination element for the connector.
    //ptVideoCapture = mpsSys_CreateNewElement(MPS_VIDEO_CAPTURE, VIDEO_CAPTURE_CMD_ID);
    //ptVideoCapture->pfInit = (OPERATION_API)taskVideoCapture_Init;
    //ptVideoCapture->pfTerminate = (OPERATION_API)taskVideoCapture_Terminate;
    //ptHandle = queueMgr_GetCtrlHandle(CAPTURE_QUEUE_ID);
    //ptHandle->pfSetFree = taskVideoCapture_SetFree;

    ptVideoEncoder              = mpsSys_CreateNewElement(MPS_VIDEO_ENCODER, VIDEO_ENCODER_CMD_ID);
    ptVideoEncoder->pfInit      = (OPERATION_API)taskVideoEncoder_Init;
    ptVideoEncoder->pfTerminate = (OPERATION_API)taskVideoEncoder_Terminate;

    // Hook the connector on the elements.
    //mpsSys_HookConnector(ptVideoCapture, MPS_SRC_ELEMENT, ptCaptureConnector);
    //mpsSys_HookConnector(ptVideoEncoder, MPS_DEST_ELEMENT, ptCaptureConnector);

    // Audio In -> Avi Mux
    // Same steps as previous input stream example to construct a audio link.
    ptAudioConnector       = mpsSys_CreateNewConnector();

    ptAudioIn              = mpsSys_CreateNewElement(MPS_AUDIO_IN, AUDIO_IN_CMD_ID);
    ptAudioIn->pfInit      = (OPERATION_API)taskAudioIn_Init;
    ptAudioIn->pfTerminate = (OPERATION_API)taskAudioIn_Terminate;

    queueMgr_CreateQueue(AUDIO_QUEUE_ID,
                         sizeof(AUDIO_SAMPLE),
                         AUDIO_SAMPLE_COUNT);
    ptHandle                  = queueMgr_GetCtrlHandle(AUDIO_QUEUE_ID);
    ptHandle->pfSetFree       = taskAudioIn_SetFree;
    ptAudioConnector->queueId = AUDIO_QUEUE_ID;

    ptAviMux                  = mpsSys_CreateNewElement(MPS_AVI_MUX, AVI_MUX_CMD_ID);
    ptAviMux->pfInit          = (OPERATION_API)taskAviMux_Init;
    ptAviMux->pfTerminate     = (OPERATION_API)taskAviMux_Terminate;

    mpsSys_HookConnector(ptAudioIn, MPS_SRC_ELEMENT, ptAudioConnector);
    mpsSys_HookConnector(ptAviMux, MPS_DEST_ELEMENT, ptAudioConnector);

    // Video Encoder -> Avi Mux
    // Same steps as previous input stream example to construct a audio link.
    ptVideoConnector = mpsSys_CreateNewConnector();

    queueMgr_CreateQueue(VIDEO_QUEUE_ID,
                         sizeof(VIDEO_SAMPLE),
                         VIDEO_SAMPLE_COUNT);
    ptHandle                  = queueMgr_GetCtrlHandle(VIDEO_QUEUE_ID);
    ptHandle->pfSetFree       = taskVideoEncoder_SetFree;
    ptVideoConnector->queueId = VIDEO_QUEUE_ID;

    mpsSys_HookConnector(ptVideoEncoder, MPS_SRC_ELEMENT, ptVideoConnector);
    mpsSys_HookConnector(ptAviMux, MPS_DEST_ELEMENT, ptVideoConnector);
}

//=============================================================================
/**
 * Used to release resouce allocated by ts Streaming player system.
 * @return none.
 */
//=============================================================================
static void
_MPS_aviMuxTerminate(
    void)
{
    // Step 1 destroy all created command queue.
    // the MPS Ctrl Task.
    mpsCmdQ_Terminate(MPS_CTRL_EVENT_ID);
    mpsCmdQ_Terminate(MPS_CTRL_SPECIAL_EVENT_ID);
    mpsCmdQ_Terminate(MPS_CTRL_CMD_ID);
    //mpsCmdQ_Terminate(VIDEO_CAPTURE_CMD_ID);
    mpsCmdQ_Terminate(AUDIO_IN_CMD_ID);
    mpsCmdQ_Terminate(VIDEO_ENCODER_CMD_ID);
    mpsCmdQ_Terminate(AVI_MUX_CMD_ID);
}

#endif

//=============================================================================
/**
 * A task to handle the command dispatch.
 * @return none.
 */
//=============================================================================
static void *
_MPS_CTRL_ThreadFunction(
    void *arg)
{
    MPS_CMD_OBJ            tReadCmdObj    = { 0 };
    MPS_CMD_OBJ            tWriteCmdObj   = { 0 };
    MPS_CMD_OBJ            tEventObj      = { 0 };
    MPS_ELEMENT_LIST_OBJ   *ptCurrent     = MMP_NULL;
    MPS_STATE              newState;
    MPS_NOTIFY_REASON_DATA *pSpecialEvent = MMP_NULL;

    for (;;)
    {
        mpsCmdQ_ReceiveCommand(MPS_CTRL_CMD_ID, &tReadCmdObj);

        if ((tReadCmdObj.cmd & MPS_COMMAND_MASK) != MPS_COMMAND_NULL)
        {
            switch ((tReadCmdObj.cmd & MPS_COMMAND_MASK))
            {
            case MPS_COMMAND_INIT:
                newState = MPS_STATE_STOP;
                if (mpsSys_CheckStateChange(gtMpsHandle.mpsState, newState))
                    gtMpsHandle.mpsState = newState;

                ptCurrent = (gtMpsHandle.ptMpsSystem)->ptLastElementObj;
                while (ptCurrent)
                {
                    if (ptCurrent->ptElement->pfInit)
                        (*ptCurrent->ptElement->pfInit)(gtMpsHandle.systemType);

                    ptCurrent = ptCurrent->ptPrevious;
                }
                if (tReadCmdObj.cmd & MPS_COMMAND_SYNC_BIT_MASK)
                    PalSetEvent(gtMpsHandle.eventCommandCompleted);
                break;

            case MPS_COMMAND_TERMINATE:
                newState = MPS_STATE_ZERO;
                if (mpsSys_CheckStateChange(gtMpsHandle.mpsState, newState))
                    gtMpsHandle.mpsState = newState;

                ptCurrent = (gtMpsHandle.ptMpsSystem)->ptFirstElementObj;
                while (ptCurrent)
                {
                    tWriteCmdObj.cmd       = tReadCmdObj.cmd;
                    tWriteCmdObj.extraData = MMP_NULL;
                    mpsCmdQ_SendCommand(ptCurrent->ptElement->cmdQueueId,
                                        &tWriteCmdObj);
                    PalWaitEvent(elementThreadToMpsThread, PAL_EVENT_INFINITE);
                    if (ptCurrent->ptElement->pfTerminate)
                        (*ptCurrent->ptElement->pfTerminate)(MMP_NULL);
                    ptCurrent = ptCurrent->ptNext;
                }
                if (tReadCmdObj.cmd & MPS_COMMAND_SYNC_BIT_MASK)
                    PalSetEvent(gtMpsHandle.eventCommandCompleted);
                goto end;

            case MPS_COMMAND_STOP:
                newState = MPS_STATE_STOP;
                if (mpsSys_CheckStateChange(gtMpsHandle.mpsState, newState))
                    gtMpsHandle.mpsState = newState;

                ptCurrent = (gtMpsHandle.ptMpsSystem)->ptFirstElementObj;
                while (ptCurrent)
                {
                    tWriteCmdObj.cmd       = tReadCmdObj.cmd;
                    tWriteCmdObj.extraData = MMP_NULL;
                    mpsCmdQ_SendCommand(ptCurrent->ptElement->cmdQueueId,
                                        &tWriteCmdObj);
#ifdef DEBUG_CMD_DISPATCH
                    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue start: %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                    PalWaitEvent(elementThreadToMpsThread, PAL_EVENT_INFINITE);
#ifdef DEBUG_CMD_DISPATCH
                    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue done: %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                    ptCurrent = ptCurrent->ptNext;
                }
                if (tReadCmdObj.cmd & MPS_COMMAND_SYNC_BIT_MASK)
                    PalSetEvent(gtMpsHandle.eventCommandCompleted);
                break;

            case MPS_COMMAND_PLAY:
                ptCurrent = (gtMpsHandle.ptMpsSystem)->ptLastElementObj;
                while (ptCurrent)
                {
                    tWriteCmdObj.cmd       = tReadCmdObj.cmd;
                    tWriteCmdObj.extraData = MMP_NULL;
                    mpsCmdQ_SendCommand(ptCurrent->ptElement->cmdQueueId,
                                        &tWriteCmdObj);
#ifdef DEBUG_CMD_DISPATCH
                    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue start: %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                    PalWaitEvent(elementThreadToMpsThread, PAL_EVENT_INFINITE);
#ifdef DEBUG_CMD_DISPATCH
                    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue done : %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif

                    ptCurrent = ptCurrent->ptPrevious;
                }

                newState = MPS_STATE_RUN;
                if (mpsSys_CheckStateChange(gtMpsHandle.mpsState, newState))
                    gtMpsHandle.mpsState = newState;

                if (tReadCmdObj.cmd & MPS_COMMAND_SYNC_BIT_MASK)
                    PalSetEvent(gtMpsHandle.eventCommandCompleted);
                break;

            case MPS_COMMAND_START_RECORD:
            case MPS_COMMAND_STOP_RECORD:
            case MPS_COMMAND_START_JPEG_RECORD:
            case MPS_COMMAND_STOP_JPEG_RECORD:
                //case MPS_COMMAND_SET_ENCODE_PARAMETER:
                //case MPS_COMMAND_SET_CAPTURE_DEVICE:
                //case MPS_COMMAND_SET_ISP_MODE:
                //case MPS_COMMAND_SET_ENABLE_AV_ENGINE:
                ptCurrent = (gtMpsHandle.ptMpsSystem)->ptFirstElementObj;
                while (ptCurrent)
                {
                    tWriteCmdObj.cmd       = tReadCmdObj.cmd;
                    tWriteCmdObj.extraData = tReadCmdObj.extraData;
                    mpsCmdQ_SendCommand(ptCurrent->ptElement->cmdQueueId,
                                        &tWriteCmdObj);
#ifdef DEBUG_CMD_DISPATCH
                    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue start: %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                    PalWaitEvent(elementThreadToMpsThread, PAL_EVENT_INFINITE);
#ifdef DEBUG_CMD_DISPATCH
                    dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue done : %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                    ptCurrent = ptCurrent->ptNext;
                }
                if (tWriteCmdObj.extraData)
                    PalHeapFree(PAL_HEAP_DEFAULT, tWriteCmdObj.extraData);

                if (tReadCmdObj.cmd & MPS_COMMAND_SYNC_BIT_MASK)
                    PalSetEvent(gtMpsHandle.eventCommandCompleted);
                break;

            case MPS_COMMAND_SET_PROPERTY:
            case MPS_COMMAND_GET_PROPERTY:
                {
                    ptCurrent = (gtMpsHandle.ptMpsSystem)->ptFirstElementObj;
                    while (ptCurrent)
                    {
                        tWriteCmdObj.cmd       = tReadCmdObj.cmd;
                        tWriteCmdObj.extraData = tReadCmdObj.extraData;
                        mpsCmdQ_SendCommand(ptCurrent->ptElement->cmdQueueId,
                                            &tWriteCmdObj);
#ifdef DEBUG_CMD_DISPATCH
                        dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue start: %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                        PalWaitEvent(elementThreadToMpsThread, PAL_EVENT_INFINITE);
#ifdef DEBUG_CMD_DISPATCH
                        dbg_msg(DBG_MSG_TYPE_INFO, "%s(%d), cmd: %u, send to queue done: %u\n", __FILE__, __LINE__, (tReadCmdObj.cmd & MPS_COMMAND_MASK), ptCurrent->ptElement->cmdQueueId);
#endif
                        ptCurrent = ptCurrent->ptNext;
                    }
                    if (tWriteCmdObj.extraData)
                        PalHeapFree(PAL_HEAP_DEFAULT, tWriteCmdObj.extraData);

                    if (tReadCmdObj.cmd & MPS_COMMAND_SYNC_BIT_MASK)
                        PalSetEvent(gtMpsHandle.eventCommandCompleted);
                    break;
                }

            default:
                break;
            }
        }

special_event_check:
        mpsCmdQ_ReceiveCommand(MPS_CTRL_SPECIAL_EVENT_ID, &tEventObj);

        if (MPS_COMMAND_EVENT_NOTIFY == tEventObj.cmd)
        {
            pSpecialEvent = (MPS_NOTIFY_REASON_DATA *) tEventObj.extraData;
            if (pSpecialEvent)
            {
                if (pSpecialEvent->notifyReason < CALLBACK_REASON_TOTAL &&
                    gtMpsHandle.pfCallbackArray[pSpecialEvent->notifyReason] != MMP_NULL)
                {
                    (*gtMpsHandle.pfCallbackArray[pSpecialEvent->notifyReason])(pSpecialEvent->notifyReason, pSpecialEvent->data);
                }

                if (pSpecialEvent->data && pSpecialEvent->bDatatNeedFree)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, (void *)pSpecialEvent->data);
                }
                PalHeapFree(PAL_HEAP_DEFAULT, pSpecialEvent);
            }
        }

        PalSleep(10);
    }

end:
    mpsSys_DestroySystem();
    gtMpsHandle.ptMpsSystem = MMP_NULL;

    if (gtMpsHandle.pfCallbackArray[CALLBACK_REASON_TERMINATE_DONE])
        (*gtMpsHandle.pfCallbackArray[CALLBACK_REASON_TERMINATE_DONE])(CALLBACK_REASON_TERMINATE_DONE, MMP_NULL);

    PalMemset(gtMpsHandle.pfCallbackArray, 0x0, sizeof(gtMpsHandle.pfCallbackArray));
    gtMpsHandle.bTerminated = MMP_TRUE;

    return MMP_NULL;
}