#include "pal/pal.h"
#include "task_stream_mux.h"
#include "ts_mux.h"
#include "mps_system.h"
#include "host/host.h"
#include "host/ahb.h"
#include "core_interface.h"
#include "mod_ctrl.h"
#include "mmp_iic.h"
#include "mmp_tso.h"
#include "mmp_dpu.h"
#include "psi_si_table_mgr.h"
#include "core_interface.h"
#include "ts_test_pattern.h"

#define PSI_INJECT_PERIOD           200
#define INFO_UPDATE_VIDEO_SKIP_TIME 500
#define AV_GAP_THRESHOLD            500
#define STUFF_PERCENT_THRESHOLD     80 // from 0% ~ 100%
//#define ENABLE_SIT_INJECTION

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct TASK_STREAM_MUX_HANDLE_TAG
{
    MPS_ELEMENT*    ptElement;
    MPS_STATE       mpsState;
    STREAM_HANDLE   tVideoEncoderHandle;
    STREAM_HANDLE   tAudioInHandle;
    PAL_THREAD      ptStreamMuxThread;
    MPS_CMD_OBJ     tReadCmdObj;
    MPS_CMD_OBJ     tWriteCmdObj;
    TS_OUT_HANDLE*  ptTsMux;
    MMP_UINT8*      pAllocStart;
    MMP_UINT8*      pTsBuffer[512];
    MMP_UINT32      bufferSize;
    MMP_UINT32      bufferIndex;
    MMP_UINT32      videoTimeStamp;
    MMP_UINT32      audioTimeStamp;
    TX_ENCODE_INFO  tEncodeInfo;
    MMP_UINT32      injectPsiClock;
    MMP_BOOL        bUpdateEncodeInfo;
    MMP_UINT32      skipVideoClock;
    MMP_BOOL        bTsoSupport;

    MMP_BOOL        bAddStuffData;
    MMP_UINT32      videoBitRate;
    AVC_FRAME_RATE  frameRate;
    MMP_INT32       sizePerFrame;
    MMP_INT32       remainStuffBufferSize;
    MMP_BOOL        bWaitFirstI;
} TASK_STREAM_MUX_HANDLE;

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_EVENT elementThreadToMpsThread;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static TASK_STREAM_MUX_HANDLE  gtStreamMuxHandle = {0};
static MMP_BOOL gtDisableHDCP = MMP_FALSE;

#ifdef TSO_ENABLE
#define TS_PER_BUFFER_SIZE 188 * 16
#define BUFFER_VALID_COUNT 512
#else
#define TS_PER_BUFFER_SIZE 188 * 1394
#define BUFFER_VALID_COUNT 32
#endif

//#define PES_BUFFER_SIZE 512 * 1024
//#define FILE_MAX_SIZE (20 * 1024 * 1024)
//static MMP_UINT32 gWriteFileSize = 0;
//static MMP_UINT32 gTimeStamp = 0;
//static PAL_FILE* gpOutputFile = MMP_NULL;
//static PAL_FILE* gpAudioFile = MMP_NULL;

//#define EXTERNAL_IQ_TABLE
#ifdef EXTERNAL_IQ_TABLE
static MMP_UINT8 pExtIQTable[] =
{
0x49, 0x44, 0x3a, 0x00, 0x31, 0x30, 0x00, 0x00, 0x56, 0x3a, 0x01, 0x00, 0x02, 0x00, 0x00, 0x56,  /* 0x00000000 */
0xa0, 0x86, 0x01, 0x00, 0x0e, 0x00, 0x70, 0x02, 0xb0, 0xad, 0x01, 0x00, 0x21, 0x00, 0x9d, 0x03,  /* 0x00000010 */
0xc0, 0xd4, 0x01, 0x00, 0x22, 0x00, 0x96, 0x03, 0xd0, 0xfb, 0x01, 0x00, 0x1e, 0x00, 0x59, 0x03,  /* 0x00000020 */
0xe0, 0x22, 0x02, 0x00, 0x15, 0x00, 0xae, 0x02, 0xf0, 0x49, 0x02, 0x00, 0x14, 0x00, 0xa3, 0x02,  /* 0x00000030 */
0x00, 0x71, 0x02, 0x00, 0x0f, 0x00, 0x53, 0x02, 0x10, 0x98, 0x02, 0x00, 0x0e, 0x00, 0x50, 0x02,  /* 0x00000040 */
0x20, 0xbf, 0x02, 0x00, 0x0a, 0x00, 0xd9, 0x01, 0x30, 0xe6, 0x02, 0x00, 0x08, 0x00, 0xc0, 0x01,  /* 0x00000050 */
0x40, 0x0d, 0x03, 0x00, 0x05, 0x00, 0x6a, 0x01, 0x50, 0x34, 0x03, 0x00, 0x03, 0x00, 0x18, 0x01,  /* 0x00000060 */
0x60, 0x5b, 0x03, 0x00, 0x01, 0x00, 0xbd, 0x00, 0x70, 0x82, 0x03, 0x00, 0x00, 0x00, 0x6b, 0x00,  /* 0x00000070 */
0x80, 0xa9, 0x03, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x90, 0xd0, 0x03, 0x00, 0x00, 0x00, 0xda, 0xff,  /* 0x00000080 */
0xa0, 0xf7, 0x03, 0x00, 0x00, 0x00, 0x9c, 0xff, 0xb0, 0x1e, 0x04, 0x00, 0x01, 0x00, 0x63, 0xff,  /* 0x00000090 */
0xc0, 0x45, 0x04, 0x00, 0x02, 0x00, 0x32, 0xff, 0xd0, 0x6c, 0x04, 0x00, 0x02, 0x00, 0x08, 0xff,  /* 0x000000a0 */
0xe0, 0x93, 0x04, 0x00, 0x02, 0x00, 0xf1, 0xfe, 0xf0, 0xba, 0x04, 0x00, 0x02, 0x00, 0xd9, 0xfe,  /* 0x000000b0 */
0x00, 0xe2, 0x04, 0x00, 0x03, 0x00, 0xce, 0xfe, 0x10, 0x09, 0x05, 0x00, 0x03, 0x00, 0xc9, 0xfe,  /* 0x000000c0 */
0x20, 0x30, 0x05, 0x00, 0x03, 0x00, 0xd2, 0xfe, 0x30, 0x57, 0x05, 0x00, 0x02, 0x00, 0xd9, 0xfe,  /* 0x000000d0 */
0x40, 0x7e, 0x05, 0x00, 0x02, 0x00, 0xe8, 0xfe, 0x50, 0xa5, 0x05, 0x00, 0x02, 0x00, 0xf8, 0xfe,  /* 0x000000e0 */
0x60, 0xcc, 0x05, 0x00, 0x01, 0x00, 0x10, 0xff, 0x70, 0xf3, 0x05, 0x00, 0x00, 0x00, 0x24, 0xff,  /* 0x000000f0 */
0x80, 0x1a, 0x06, 0x00, 0x00, 0x00, 0x38, 0xff, 0x90, 0x41, 0x06, 0x00, 0xff, 0xff, 0x55, 0xff,  /* 0x00000100 */
0xa0, 0x68, 0x06, 0x00, 0xff, 0xff, 0x65, 0xff, 0xb0, 0x8f, 0x06, 0x00, 0xff, 0xff, 0x77, 0xff,  /* 0x00000110 */
0xc0, 0xb6, 0x06, 0x00, 0xff, 0xff, 0x8a, 0xff, 0xd0, 0xdd, 0x06, 0x00, 0xff, 0xff, 0x95, 0xff,  /* 0x00000120 */
0xe0, 0x04, 0x07, 0x00, 0xff, 0xff, 0xa1, 0xff, 0xf0, 0x2b, 0x07, 0x00, 0xfe, 0xff, 0xaa, 0xff,  /* 0x00000130 */
0x00, 0x53, 0x07, 0x00, 0xfe, 0xff, 0xb9, 0xff, 0x10, 0x7a, 0x07, 0x00, 0xff, 0xff, 0xc0, 0xff,  /* 0x00000140 */
0x20, 0xa1, 0x07, 0x00, 0xfe, 0xff, 0xcb, 0xff, 0x30, 0xc8, 0x07, 0x00, 0xfe, 0xff, 0xd2, 0xff,  /* 0x00000150 */
0x40, 0xef, 0x07, 0x00, 0xff, 0xff, 0xd4, 0xff, 0x50, 0x16, 0x08, 0x00, 0xff, 0xff, 0xe1, 0xff,  /* 0x00000160 */
0x60, 0x3d, 0x08, 0x00, 0xff, 0xff, 0xe7, 0xff, 0x70, 0x64, 0x08, 0x00, 0xff, 0xff, 0xe8, 0xff,  /* 0x00000170 */
0x80, 0x8b, 0x08, 0x00, 0xff, 0xff, 0xee, 0xff, 0x90, 0xb2, 0x08, 0x00, 0xff, 0xff, 0xf0, 0xff,  /* 0x00000180 */
0xa0, 0xd9, 0x08, 0x00, 0xff, 0xff, 0xf0, 0xff, 0xb0, 0x00, 0x09, 0x00, 0xff, 0xff, 0xee, 0xff,  /* 0x00000190 */
0xc0, 0x27, 0x09, 0x00, 0xff, 0xff, 0xf0, 0xff, 0xd0, 0x4e, 0x09, 0x00, 0xff, 0xff, 0xee, 0xff,  /* 0x000001a0 */
0xe0, 0x75, 0x09, 0x00, 0xff, 0xff, 0xea, 0xff, 0xf0, 0x9c, 0x09, 0x00, 0xff, 0xff, 0xe7, 0xff,  /* 0x000001b0 */
0x00, 0xc4, 0x09, 0x00, 0xff, 0xff, 0xe5, 0xff, 0x10, 0xeb, 0x09, 0x00, 0xff, 0xff, 0xe3, 0xff,  /* 0x000001c0 */
0x20, 0x12, 0x0a, 0x00, 0xff, 0xff, 0xe1, 0xff, 0x30, 0x39, 0x0a, 0x00, 0xff, 0xff, 0xe1, 0xff,  /* 0x000001d0 */
0x40, 0x60, 0x0a, 0x00, 0xff, 0xff, 0xdf, 0xff, 0x50, 0x87, 0x0a, 0x00, 0x00, 0x00, 0xdf, 0xff,  /* 0x000001e0 */
0x60, 0xae, 0x0a, 0x00, 0x00, 0x00, 0xda, 0xff, 0x70, 0xd5, 0x0a, 0x00, 0xff, 0xff, 0xdd, 0xff,  /* 0x000001f0 */
0x80, 0xfc, 0x0a, 0x00, 0xff, 0xff, 0xda, 0xff, 0x90, 0x23, 0x0b, 0x00, 0xff, 0xff, 0xd6, 0xff,  /* 0x00000200 */
0xa0, 0x4a, 0x0b, 0x00, 0xff, 0xff, 0xd2, 0xff, 0xb0, 0x71, 0x0b, 0x00, 0x00, 0x00, 0xd6, 0xff,  /* 0x00000210 */
0xc0, 0x98, 0x0b, 0x00, 0xff, 0xff, 0xd6, 0xff, 0xd0, 0xbf, 0x0b, 0x00, 0xff, 0xff, 0xd4, 0xff,  /* 0x00000220 */
0xe0, 0xe6, 0x0b, 0x00, 0x00, 0x00, 0xd6, 0xff, 0xf0, 0x0d, 0x0c, 0x00, 0x00, 0x00, 0xd4, 0xff,  /* 0x00000230 */
0x00, 0x35, 0x0c, 0x00, 0x00, 0x00, 0xd2, 0xff, 0x10, 0x5c, 0x0c, 0x00, 0xff, 0xff, 0xd1, 0xff,  /* 0x00000240 */
0x20, 0x83, 0x0c, 0x00, 0xff, 0xff, 0xcf, 0xff, 0x30, 0xaa, 0x0c, 0x00, 0x00, 0x00, 0xcf, 0xff,  /* 0x00000250 */
0x40, 0xd1, 0x0c, 0x00, 0x00, 0x00, 0xd1, 0xff, 0x50, 0xf8, 0x0c, 0x00, 0x00, 0x00, 0xd1, 0xff,  /* 0x00000260 */
0x60, 0x1f, 0x0d, 0x00, 0xff, 0xff, 0xcd, 0xff, 0x70, 0x46, 0x0d, 0x00, 0xff, 0xff, 0xd1, 0xff,  /* 0x00000270 */
0x80, 0x6d, 0x0d, 0x00, 0xff, 0xff, 0xcb, 0xff, 0x90, 0x94, 0x0d, 0x00, 0x00, 0x00, 0xcb, 0xff,  /* 0x00000280 */
0xa0, 0xbb, 0x0d, 0x00, 0xff, 0xff, 0xcb, 0xff, 0xb0, 0xe2, 0x0d, 0x00, 0x00, 0x00, 0xc8, 0xff,  /* 0x00000290 */
0xc0, 0x09, 0x0e, 0x00, 0xff, 0xff, 0xcb, 0xff, 0xd0, 0x30, 0x0e, 0x00, 0x00, 0x00, 0xc8, 0xff,  /* 0x000002a0 */
0xe0, 0x57, 0x0e, 0x00, 0xff, 0xff, 0xc9, 0xff, 0xf0, 0x7e, 0x0e, 0x00, 0xff, 0xff, 0xc6, 0xff,  /* 0x000002b0 */
};
#endif

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_STREAM_MUX_TableUpdate(
    void);

static void
_STREAM_MUX_DoPlay(
    void);

static void
_STREAM_MUX_DoStop(
    void);

static void
_STREAM_Mux_BufferFullCallback(
    TS_OUT_HANDLE* ptHandle);

static void
_STREAM_MUX_EventNotify(
    MMP_UINT32  reason,
    MMP_UINT32  data);

static void*
_STREAM_MUX_ThreadFunction(
    void* arg);

static void
_STREAM_MUX_ParamInit(
    TS_MUXER_PARAMETER* ptMuxerParam);

static void
_STREAM_MUX_ResetStuffConfig(
    AVC_FRAME_RATE frameRate,
    MMP_UINT32     kBitRate);

static MMP_BOOL
_STREAM_MUX_CalPerFrameCount(
    void);

MMP_INT32
_STREAM_MUX_GetStuffSize(
    MMP_UINT32 frameSize);

static MMP_BOOL
_IsContentProtection(
    void);

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
taskStreamMux_GetBufferInfo(
    MMP_UINT8** pBufferStart,
    MMP_UINT32* bufferSize)
{
    *bufferSize = 0;
    *pBufferStart = MMP_NULL;
    if (gtStreamMuxHandle.pTsBuffer[0])
    {
        *pBufferStart = gtStreamMuxHandle.pTsBuffer[0];
        *bufferSize = gtStreamMuxHandle.bufferSize * BUFFER_VALID_COUNT;
    }
}

MMP_UINT32
taskStreamMux_GetWriteIndex(
    void)
{
    return (gtStreamMuxHandle.bufferIndex * gtStreamMuxHandle.bufferSize);
}

void
taskStreamMux_Init(
    MMP_UINT32 arg)
{
    MPS_CONNECTOR_LIST_OBJ* ptListObj = MMP_NULL;
    MPS_CONNECTOR*          ptConnector = MMP_NULL;

    if (arg) { } // avoid compiler warning

    if (gtStreamMuxHandle.mpsState != MPS_STATE_ZERO)
        return;

    gtStreamMuxHandle.mpsState = MPS_STATE_STOP;
    gtStreamMuxHandle.ptElement = mpsSys_GetElement(MPS_STREAM_MUX);
    gtStreamMuxHandle.bWaitFirstI = MMP_TRUE;
    if ((ptListObj = (gtStreamMuxHandle.ptElement)->ptSrcList) != MMP_NULL)
    {
        do
        {
            ptConnector = ptListObj->ptConnector;

            switch (ptConnector->ptSrcElement->id)
            {
            case MPS_VIDEO_ENCODER:
                gtStreamMuxHandle.tVideoEncoderHandle.queueId =
                    ptListObj->ptConnector->queueId;
                gtStreamMuxHandle.tVideoEncoderHandle.ptQueueHandle =
                    queueMgr_GetCtrlHandle(gtStreamMuxHandle.tVideoEncoderHandle.queueId);
                dbg_msg(DBG_MSG_TYPE_INFO, "video queue_id: %u\n", ptListObj->ptConnector->queueId);
                break;

            case MPS_AUDIO_IN:
                gtStreamMuxHandle.tAudioInHandle.queueId =
                    ptListObj->ptConnector->queueId;
                gtStreamMuxHandle.tAudioInHandle.ptQueueHandle =
                    queueMgr_GetCtrlHandle(gtStreamMuxHandle.tAudioInHandle.queueId);
                dbg_msg(DBG_MSG_TYPE_INFO, "audio queue_id: %u\n", ptListObj->ptConnector->queueId);
                break;

            default:
                break;
            }
        } while ((ptListObj = ptListObj->ptNext) != MMP_NULL);
    }

    //gtStreamMuxHandle.tVideoEncoderHandle.queueId =
    //    gtStreamMuxHandle.ptElement->ptSrcList->ptConnector->queueId;
    //gtStreamMuxHandle.tVideoEncoderHandle.ptQueueHandle =
    //    queueMgr_GetCtrlHandle(gtStreamMuxHandle.tVideoEncoderHandle.queueId);

    if (MMP_NULL == gtStreamMuxHandle.ptStreamMuxThread)
    {
        gtStreamMuxHandle.ptStreamMuxThread = PalCreateThread(PAL_THREAD_STREAM_MUX,
                                                              _STREAM_MUX_ThreadFunction,
                                                              MMP_NULL, 0, 0);
    }
}

void
taskStreamMux_Terminate(
    MMP_UINT32 arg)
{
    if (arg) { } // avoid compiler warning

    if (gtStreamMuxHandle.ptTsMux)
    {
        TsMuxTerminateHandle(gtStreamMuxHandle.ptTsMux);
    }

    if (gtStreamMuxHandle.pAllocStart)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, gtStreamMuxHandle.pAllocStart);
        gtStreamMuxHandle.pAllocStart = MMP_NULL;
    }

    if (gtStreamMuxHandle.ptStreamMuxThread != MMP_NULL)
        PalDestroyThread(gtStreamMuxHandle.ptStreamMuxThread);

    PalMemset(&gtStreamMuxHandle, 0x0, sizeof(TASK_STREAM_MUX_HANDLE));
}

void
taskStreamMux_UndoSeal(
    MMP_BOOL enable)
{
    gtDisableHDCP = enable;
}

//QUEUE_MGR_ERROR_CODE
//taskStreamMux_SetFree(
//    QUEUE_ID queueId,
//    void** pptSample)
//{
//
//}

//=============================================================================
//                              Private Function Definition
//=============================================================================
//static void
//_STREAM_FileWriteCallback(
//    PAL_FILE* file,
//    MMP_ULONG result)
//{
//    gWriteFileSize += result;
//    gtStreamMuxHandle.writeBufferIndex = ((gtStreamMuxHandle.writeBufferIndex + 1) & 0x7);
//    if (gtStreamMuxHandle.waitingWriteCount)
//    {
//        gtStreamMuxHandle.waitingWriteCount--;
//    }
//    if (gtStreamMuxHandle.waitingWriteCount)
//    {
//        PalFileWrite(gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.writeBufferIndex], 1, gtStreamMuxHandle.bufferSize, gpOutputFile, _STREAM_FileWriteCallback);
//    }
//}

//static void
//_STREAM_FileCloseCallback(
//    PAL_FILE* file,
//    MMP_ULONG result)
//{
//    gpOutputFile = MMP_NULL;
//    gtStreamMuxHandle.bInClosing = MMP_FALSE;
//    while(1);
//}

#if 1
// Check ts output rate.
static MMP_UINT32 checkClock = 0;
static MMP_UINT32 gTotalData = 0;
#endif
static void
_STREAM_Mux_BufferFullCallback(
    TS_OUT_HANDLE* ptHandle)
{
    MMP_UINT32 result = 0;

#ifdef TSO_ENABLE
    if (mmpTsoGetStatus() & (0x1 << 7 ))
    {
        //dbg_msg(DBG_MSG_TYPE_INFO, "status: 0x%X, Abnormal End, ptr: 0x%X\n", mmpTsoGetStatus(), gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.writeBufferIndex]);
        dbg_msg(DBG_MSG_TYPE_INFO, "status: 0x%X, Abnormal End\n", mmpTsoGetStatus());
        while (1);
    }
#if 1
    // Check ts output rate.
    gTotalData += TS_PER_BUFFER_SIZE;
    if (checkClock)
    {
        MMP_UINT32 elpasedTime = 0;
        if ((elpasedTime = PalGetDuration(checkClock)) >= 1000)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "bit rate: %u\n", gTotalData * 8 / elpasedTime);
            checkClock = 0;
            gTotalData = 0;
        }
    }
    else
    {
        checkClock = PalGetClock();
    }
#endif

#if 0
    // Force frame ts out.
    if (ptHandle->bForceUpdate)
    {
        if (ptHandle->pesTsSize <= ptHandle->tsWriteIndex)
        {
            mmpTsoWriteWithoutCopy(ptHandle->pesTsSize);
        }
        else if (ptHandle->tsWriteIndex)
        {
            mmpTsoWriteWithoutCopy(ptHandle->tsWriteIndex);
        }
        ptHandle->tsLastUpdateIndex = ptHandle->tsWriteIndex;
    }
    else
    {
        mmpTsoWriteWithoutCopy(gtStreamMuxHandle.bufferSize - ptHandle->tsLastUpdateIndex);
        ptHandle->tsLastUpdateIndex = 0;
        gtStreamMuxHandle.bufferIndex = ((gtStreamMuxHandle.bufferIndex + 1) % BUFFER_VALID_COUNT);
        TsMuxSetTsBuffer(gtStreamMuxHandle.ptTsMux, (MMP_UINT8*) gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
    }
#else
    mmpTsoWriteWithoutCopy(gtStreamMuxHandle.bufferSize);
    gtStreamMuxHandle.bufferIndex = ((gtStreamMuxHandle.bufferIndex + 1) % BUFFER_VALID_COUNT);
#if 0
    if (gtStreamMuxHandle.bufferIndex == 0)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "ready to dump memory- addr: 0x%X, size: %u\n", gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], TS_PER_BUFFER_SIZE * BUFFER_VALID_COUNT);
        while (1);
    }
#endif
    TsMuxSetTsBuffer(gtStreamMuxHandle.ptTsMux, (MMP_UINT8*) gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
#endif
#else
    gtStreamMuxHandle.bufferIndex = ((gtStreamMuxHandle.bufferIndex + 1) % BUFFER_VALID_COUNT);
    //dbg_msg(DBG_MSG_TYPE_INFO, "cur index: %u, pos: 0x%X, size: %u\n", gtStreamMuxHandle.bufferIndex, gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
    TsMuxSetTsBuffer(gtStreamMuxHandle.ptTsMux, (MMP_UINT8*) gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
    //if (gWriteFileSize < FILE_MAX_SIZE && gpOutputFile && MMP_FALSE == gtStreamMuxHandle.bInClosing)
    //{
    //    if (0 == gtStreamMuxHandle.waitingWriteCount)
    //    {
    //        PalFileWrite(gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.writeBufferIndex], 1, gtStreamMuxHandle.bufferSize, gpOutputFile, _STREAM_FileWriteCallback);
    //    }
    //    else
    //    {
    //        gtStreamMuxHandle.waitingWriteCount++;
    //    }
    //    dbg_msg(DBG_MSG_TYPE_INFO, "waiting count: %u\n", gtStreamMuxHandle.waitingWriteCount);
    //    //dbg_msg(DBG_MSG_TYPE_INFO, "write: 0x%X, result: %u\n", gpOutputFile, result);
    //    gtStreamMuxHandle.bufferIndex = ((gtStreamMuxHandle.bufferIndex + 1) & 0x7);
    //    TsMuxSetTsBuffer(gtStreamMuxHandle.ptTsMux, (MMP_UINT8*) gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
    //    //gWriteFileSize += ptHandle->bufferSize;
    //}
    //if (gWriteFileSize >= FILE_MAX_SIZE)
    //{
    //    gtStreamMuxHandle.bInClosing = MMP_TRUE;
    //    result = PalFileClose(gpOutputFile, _STREAM_FileCloseCallback);
    //}
#endif
}

static void
_STREAM_MUX_TableUpdate(
    void)
{
    if (gtStreamMuxHandle.ptTsMux)
    {
        PsiSiMgrGenerateTablePacket();
        TsMuxCreatePatTable(gtStreamMuxHandle.ptTsMux);
        TsMuxCreatePmtTable(gtStreamMuxHandle.ptTsMux);
        TsMuxCreateSdtTable(gtStreamMuxHandle.ptTsMux);
        TsMuxCreateNitTable(gtStreamMuxHandle.ptTsMux);
        TsMuxGenerateNitTsPacket(gtStreamMuxHandle.ptTsMux);
        TsMuxGenerateSdtTsPacket(gtStreamMuxHandle.ptTsMux);
        TsMuxGeneratePatTsPacket(gtStreamMuxHandle.ptTsMux);
        TsMuxGeneratePmtTsPacket(gtStreamMuxHandle.ptTsMux);
    }
}

static void
_STREAM_MUX_ParamInit(
    TS_MUXER_PARAMETER* ptMuxerParam)
{
    MMP_UINT32 result = 0;
    MMP_UINT32          portStart = 0;
    MMP_UINT8*          pBufferStart = MMP_NULL;
    ChannelModulation   channelModulation;
    Modulator           modulator_info;
    MMP_UINT32          i = 0;
    AUDIO_STREAM_TYPE   audioStreamType;
    MMP_UINT16          videoPid = 0;
    MMP_UINT16          audioPid = 0;
    switch (ptMuxerParam->audioEncoderType)
    {
    case MPEG_AUDIO_ENCODER:
    default:
        gtStreamMuxHandle.tEncodeInfo.audioCodec = 0;
        gtStreamMuxHandle.tEncodeInfo.samplingRate = 48000;
        audioStreamType = MPEG_AUDIO;
        break;

    case AAC_AUDIO_ENCODER:
        gtStreamMuxHandle.tEncodeInfo.audioCodec = 1;
        gtStreamMuxHandle.tEncodeInfo.samplingRate = 48000;
        audioStreamType = AAC;
        break;
    }
    gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;

    if (gtStreamMuxHandle.ptTsMux)
    {
        TsMuxTerminateHandle(gtStreamMuxHandle.ptTsMux);
        gtStreamMuxHandle.ptTsMux = MMP_NULL;
    }

    if (ptMuxerParam->bEnableSecuirtyMode)
    {
        result = TsMuxCreateHandle(&gtStreamMuxHandle.ptTsMux, _STREAM_Mux_BufferFullCallback, MMP_TRUE, MMP_TRUE);
#ifdef AV_SENDER_SECURITY_MODE
        gtStreamMuxHandle.ptTsMux->pSessionKey = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, 16);
        gtStreamMuxHandle.ptTsMux->sessionKeyLen = 32;
        mmpDpuSetRandomKeyGenerateRule(MMP_TRUE);
        TsSecurityGenerateSessionKey(gtStreamMuxHandle.ptTsMux->pSessionKey, 16);
#endif
    }
    else
    {
        result = TsMuxCreateHandle(&gtStreamMuxHandle.ptTsMux, _STREAM_Mux_BufferFullCallback, MMP_TRUE, MMP_TRUE);
    }

    if (gtStreamMuxHandle.pAllocStart)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, gtStreamMuxHandle.pAllocStart);
        gtStreamMuxHandle.pAllocStart = MMP_NULL;
    }

    gtStreamMuxHandle.bufferSize = TS_PER_BUFFER_SIZE;
    gtStreamMuxHandle.pAllocStart = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, gtStreamMuxHandle.bufferSize * BUFFER_VALID_COUNT + 8);
    pBufferStart = (MMP_UINT8*) (((MMP_UINT32)(gtStreamMuxHandle.pAllocStart) & 0xFFFFFFF0) + 8);
    dbg_msg(DBG_MSG_TYPE_INFO, "alloc addr: 0x%X, buffer start: 0x%X\n", gtStreamMuxHandle.pAllocStart, pBufferStart);
    PalMemset(pBufferStart, 0x0, gtStreamMuxHandle.bufferSize * BUFFER_VALID_COUNT);
    for (i = 0; i < BUFFER_VALID_COUNT; i++)
    {
        gtStreamMuxHandle.pTsBuffer[i] = pBufferStart + (gtStreamMuxHandle.bufferSize * i);
        //PalMemset(gtStreamMuxHandle.pTsBuffer[i], 0x0, gtStreamMuxHandle.bufferSize);
        //dbg_msg(DBG_MSG_TYPE_INFO, "ts buffer[%d]: 0x%X\n", i, gtStreamMuxHandle.pTsBuffer[i]);
    }

    if (ptMuxerParam->bEnableTso)
    {
#if (defined (IT9919_144TQFP) || defined(IT9913_128LQFP))
        portStart = 13;
#else
        portStart = 34;
#endif
        // Terminate TSO engine first
        mmpTsoTerminate();
        // Enable PCR auto injection
        PsiSiMgrGetEsPid(0, &videoPid, &audioPid);
        mmpTsoInitialize(portStart, videoPid, 40, 0, pBufferStart, gtStreamMuxHandle.bufferSize * BUFFER_VALID_COUNT, MMP_FALSE);
        mmpTsoEnable();
        //gbFirstTime = MMP_TRUE;
        gtStreamMuxHandle.bTsoSupport = MMP_TRUE;
        dbg_msg(DBG_MSG_TYPE_INFO, "start write buffer: 0x%X\n", gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex]);
    }

    switch (ptMuxerParam->constellation)
    {
    case CONSTELATTION_QPSK:
        channelModulation.constellation = Constellation_QPSK;
        break;

    case CONSTELATTION_16QAM:
        channelModulation.constellation = Constellation_16QAM;
        break;

    case CONSTELATTION_64QAM:
    default:
        channelModulation.constellation = Constellation_64QAM;
        break;
    }

    switch (ptMuxerParam->codeRate)
    {
    case CODE_RATE_1_2:
        channelModulation.highCodeRate = CodeRate_1_OVER_2;
        break;

    case CODE_RATE_2_3:
        channelModulation.highCodeRate = CodeRate_2_OVER_3;
        break;

    case CODE_RATE_3_4:
        channelModulation.highCodeRate = CodeRate_3_OVER_4;
        break;

    case CODE_RATE_5_6:
        channelModulation.highCodeRate = CodeRate_5_OVER_6;
        break;

    case CODE_RATE_7_8:
        channelModulation.highCodeRate = CodeRate_7_OVER_8;
        break;
    }

    switch (ptMuxerParam->guardInterval)
    {
    case GUARD_INTERVAL_1_4:
        channelModulation.interval = Interval_1_OVER_4;
        break;

    case GUARD_INTERVAL_1_8:
        channelModulation.interval = Interval_1_OVER_8;
        break;

    case GUARD_INTERVAL_1_16:
        channelModulation.interval = Interval_1_OVER_16;
        break;

    case GUARD_INTERVAL_1_32:
        channelModulation.interval = Interval_1_OVER_32;
        break;
    }

    channelModulation.transmissionMode = TransmissionMode_8K;
    channelModulation.frequency = ptMuxerParam->frequency;
    channelModulation.bandwidth = ptMuxerParam->bandwidth;

    if (ptMuxerParam->bEnableEagle)
    {
        ModCtrl_Terminate();
        modulator_info.busId = Bus_I2C;
#ifdef EXTERNAL_IQ_TABLE
        ModCtrl_Init(&modulator_info, pExtIQTable, sizeof(pExtIQTable));
#else
        ModCtrl_Init(&modulator_info, 0, 0);
#endif
        result = ModCtrl_setTXChannelModulation(&channelModulation);
        if (result)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "Eagle ModCtrl_setTXChannelModulation is failed\n");
            while(1);
        }
        ModCtrl_adjustOutputGain(&ptMuxerParam->gain);
        result = ModCtrl_acquireTxChannel(&channelModulation);
        if (result)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "Eagle ModCtrl_acquireTxChannel is failed\n");
            while(1);
        }
        ModCtrl_setTxModeEnable();
    }

    if (gtStreamMuxHandle.ptTsMux)
    {
        gtStreamMuxHandle.bufferIndex = 0;
        // gtStreamMuxHandle.waitingWriteCount = 0;
        TsMuxSetTsBuffer(gtStreamMuxHandle.ptTsMux, (MMP_UINT8*) gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
        if (gtStreamMuxHandle.ptTsMux->bEnableSecurityMode)
        {
#ifdef AV_SENDER_SECURITY_MODE
            TsMuxCreateSktTable(gtStreamMuxHandle.ptTsMux);
            for (i = 0; i < gtStreamMuxHandle.ptTsMux->sktSectionCount; i++)
            {
                MMP_UINT32 j = 0;
                dbg_msg(DBG_MSG_TYPE_INFO, "section: %u\n", i);
                for (j = 0; j < 188; j++)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", gtStreamMuxHandle.ptTsMux->pSktTsPacketBuffer[TS_PACKET_LEN * i + j]);
                }
                dbg_msg(DBG_MSG_TYPE_INFO, "\n");
            }
#endif
        }
        //dbg_msg(DBG_MSG_TYPE_INFO, "start inject psi table\n");
        //TsMuxGenerateNitTsPacket(gtStreamMuxHandle.ptTsMux);
        //TsMuxGenerateSdtTsPacket(gtStreamMuxHandle.ptTsMux);
        //TsMuxGeneratePatTsPacket(gtStreamMuxHandle.ptTsMux);
        //TsMuxGeneratePmtTsPacket(gtStreamMuxHandle.ptTsMux);
        gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
    }

    gtStreamMuxHandle.bAddStuffData = ptMuxerParam->bAddStuffData;
    if (gtStreamMuxHandle.bAddStuffData)
    {
        _STREAM_MUX_ResetStuffConfig(AVC_FRAME_RATE_UNKNOW, 0);
    }
}

static void
_STREAM_MUX_DoPlay(
    void)
{
    MMP_UINT32      return_state = 0;
    VIDEO_SAMPLE*   ptVideoSample = MMP_NULL;
    AUDIO_SAMPLE*   ptAudioSample = MMP_NULL;
    QUEUE_ID        video_queueId                = gtStreamMuxHandle.tVideoEncoderHandle.queueId;
    QUEUE_OPERATION pfVideoInputQueueGetReady    = gtStreamMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfVideoInputQueueSetFree     = gtStreamMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfSetFree;
    QUEUE_ID        audio_queueId                = gtStreamMuxHandle.tAudioInHandle.queueId;
    QUEUE_OPERATION pfAudioInputQueueGetReady    = gtStreamMuxHandle.tAudioInHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfAudioInputQueueSetFree     = gtStreamMuxHandle.tAudioInHandle.ptQueueHandle->pfSetFree;
    TS_OUT_HANDLE*  ptTsMuxHandle = gtStreamMuxHandle.ptTsMux;
    MMP_UINT32      timeGap = 0;
    DPU_CONTEXT     tDpuContext = { 0 };
    MMP_UINT32      wrapTime = 0;
    MMP_BOOL        bDiscontinuity = MMP_FALSE;
    MMP_UINT32      idrFlag = 0x0;
    SERVICE_ENTRY_INFO* ptTvService = MMP_NULL;
    MMP_BOOL            bContinue = MMP_TRUE;

    if (_IsContentProtection())
    {
        while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void**)&ptVideoSample))
        {
            pfVideoInputQueueSetFree(video_queueId, (void**) &ptVideoSample);
        }
        while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void**)&ptAudioSample))
        {
            pfAudioInputQueueSetFree(audio_queueId, (void**) &ptAudioSample);
        }
        return;
    }

    if (gtStreamMuxHandle.bWaitFirstI)
    {
        while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void**)&ptVideoSample))
        {
            if (VIDEO_FRAME_TYPE_I == ptVideoSample->frameType)
            {
                gtStreamMuxHandle.bWaitFirstI = MMP_FALSE;
                TsMuxGenerateNitTsPacket(gtStreamMuxHandle.ptTsMux);
                TsMuxGenerateSdtTsPacket(gtStreamMuxHandle.ptTsMux);
                TsMuxGeneratePatTsPacket(gtStreamMuxHandle.ptTsMux);
                TsMuxGeneratePmtTsPacket(gtStreamMuxHandle.ptTsMux);
#ifdef ENABLE_SIT_INJECTION
                TsMuxGenerateSitTsPacket(gtStreamMuxHandle.ptTsMux);
#endif
                gtStreamMuxHandle.injectPsiClock = PalGetClock();
                break;
            }
            pfVideoInputQueueSetFree(video_queueId, (void**) &ptVideoSample);
        }
        while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void**)&ptAudioSample))
        {
            pfAudioInputQueueSetFree(audio_queueId, (void**) &ptAudioSample);
        }
        return;
    }

    while (1)
    {
        bContinue = MMP_TRUE;

        if (gtStreamMuxHandle.bUpdateEncodeInfo)
        {
            //TsMuxCreateSitTable(gtStreamMuxHandle.ptTsMux, &gtStreamMuxHandle.tEncodeInfo);
            //TsMuxGenerateSitTsPacket(gtStreamMuxHandle.ptTsMux);
            gtStreamMuxHandle.bUpdateEncodeInfo = MMP_FALSE;
        }

        if (PalGetDuration(gtStreamMuxHandle.injectPsiClock) >= PSI_INJECT_PERIOD)
        {
            TsMuxGenerateNitTsPacket(gtStreamMuxHandle.ptTsMux);
            TsMuxGenerateSdtTsPacket(gtStreamMuxHandle.ptTsMux);
            TsMuxGeneratePatTsPacket(gtStreamMuxHandle.ptTsMux);
            TsMuxGeneratePmtTsPacket(gtStreamMuxHandle.ptTsMux);
#ifdef ENABLE_SIT_INJECTION
            TsMuxGenerateSitTsPacket(gtStreamMuxHandle.ptTsMux);
#endif
            gtStreamMuxHandle.injectPsiClock = PalGetClock();
        }

        if (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void**)&ptVideoSample))
        {
#ifdef ENABLE_SIT_INJECTION
            if (gtStreamMuxHandle.tEncodeInfo.encoderInfo != ptVideoSample->encoderInfo)
            {
                gtStreamMuxHandle.tEncodeInfo.encoderInfo = ptVideoSample->encoderInfo;
                gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
            }

            if (gtStreamMuxHandle.tEncodeInfo.width != ptVideoSample->width
             || gtStreamMuxHandle.tEncodeInfo.height != ptVideoSample->height)
            {
                gtStreamMuxHandle.tEncodeInfo.width = ptVideoSample->width;
                gtStreamMuxHandle.tEncodeInfo.height = ptVideoSample->height;
                gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
            }

            if (gtStreamMuxHandle.tEncodeInfo.bVideoLock != avSyncIsVideoStable())
            {
                gtStreamMuxHandle.tEncodeInfo.bVideoLock = avSyncIsVideoStable();
                gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
            }

            if (gtStreamMuxHandle.tEncodeInfo.bSkipFrame != ptVideoSample->bSkipFrame)
            {
                gtStreamMuxHandle.tEncodeInfo.bSkipFrame = ptVideoSample->bSkipFrame;
                gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
            }

            if (gtStreamMuxHandle.bUpdateEncodeInfo)
            {
                TsMuxCreateSitTable(gtStreamMuxHandle.ptTsMux, &gtStreamMuxHandle.tEncodeInfo);
                TsMuxGenerateSitTsPacket(gtStreamMuxHandle.ptTsMux);
                gtStreamMuxHandle.skipVideoClock = PalGetClock();
                gtStreamMuxHandle.bUpdateEncodeInfo = MMP_FALSE;
            }
#endif
#ifdef AV_SENDER_SECURITY_MODE
            mmpDpuGenerateRandomData(ptTsMuxHandle->pInitVector, 16);
            tDpuContext.DpuMode = AES_CTR_MODE;
            tDpuContext.Keylength = KEY_LENTGH_128_BIT;
            tDpuContext.Vectorlength = VECTOR_LENTGH_4_VECTORS;
            tDpuContext.SrcAddress = (MMP_UINT32)ptVideoSample->pData;
            tDpuContext.DstAddress = (MMP_UINT32)ptVideoSample->pData;
            tDpuContext.DataSize = ((ptVideoSample->dataSize + 16) & 0xFFFFFFF0);
            tDpuContext.RefKeyAddress = (MMP_UINT32*) ptTsMuxHandle->pSessionKey;
            tDpuContext.Vectors = (MMP_UINT32*) ptTsMuxHandle->pInitVector;
            mmpDpuEncode(&tDpuContext);
#endif
            //dbg_msg(DBG_MSG_TYPE_INFO, "size: %u ", ptVideoSample->dataSize);

            //wrapTime = PalGetClock();
#ifdef MULTIPLE_INSTANCES
            ptTvService = &ptTsMuxHandle->ptServiceArray[ptVideoSample->InstanceNum];
#else
            ptTvService = &ptTsMuxHandle->ptServiceArray[0];
#endif
            switch(ptVideoSample->frameRate)
            {
            case AVC_FRAME_RATE_50HZ:
                ptTvService->frameTime = 20;
                break;

            case AVC_FRAME_RATE_30HZ:
            case AVC_FRAME_RATE_29_97HZ:
                ptTvService->frameTime = 33;
                break;

            case AVC_FRAME_RATE_60HZ:
            case AVC_FRAME_RATE_59_94HZ:
                ptTvService->frameTime = 16;
                break;

            case AVC_FRAME_RATE_24HZ:
                ptTvService->frameTime = 41;
                break;

            case AVC_FRAME_RATE_25HZ:
                ptTvService->frameTime = 40;
                break;

            case AVC_FRAME_RATE_UNKNOW:
            default:
                ptTvService->frameTime = 33;
                break;
            }

            if (gtStreamMuxHandle.videoTimeStamp <= ptVideoSample->timeStamp)
            {
                timeGap = ptVideoSample->timeStamp - gtStreamMuxHandle.videoTimeStamp;
            }
            else
            {
                timeGap = (0xFFFFFFFF - gtStreamMuxHandle.videoTimeStamp) + ptVideoSample->timeStamp;
            }

            if (timeGap > 100)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "time gap: %u is larger than 100 ms, prev: %u, curr: %u\n", timeGap, gtStreamMuxHandle.videoTimeStamp, ptVideoSample->timeStamp);
                bDiscontinuity = MMP_TRUE;
            }
            else
            {
                bDiscontinuity = MMP_FALSE;
            }
#ifdef MULTIPLE_INSTANCES
            TsMuxUpdateTimeStamp(&ptTvService->tVideoTimestamp, ptVideoSample->timeStamp);
            TsMuxGeneratePesTsPacket(ptTsMuxHandle, ptVideoSample->InstanceNum, MMP_TRUE, ptVideoSample->pData, ptVideoSample->dataSize, bDiscontinuity);
#else
            TsMuxUpdateTimeStamp(&ptTvService->tVideoTimestamp, ptVideoSample->timeStamp);
            //if ((ptVideoSample->frameCount % 30) == 1)
            //{
            //    idrFlag = 0x80000000;
            //}
            //{
            //MMP_UINT32 testClock = PalGetClock();
            //MMP_UINT32 timeDuration = 0;

            // Stop to generate PES until timeout,
            // give time to Rx for response to new encode info.
            //if (gtStreamMuxHandle.skipVideoClock && PalGetDuration(gtStreamMuxHandle.skipVideoClock) >= INFO_UPDATE_VIDEO_SKIP_TIME)
            {
                gtStreamMuxHandle.skipVideoClock = 0;
            }

            if (!gtStreamMuxHandle.skipVideoClock)
            {
                if (gtStreamMuxHandle.bAddStuffData)
                {
                    if (ptVideoSample->frameRate != gtStreamMuxHandle.frameRate
                     || ptVideoSample->bitRate != gtStreamMuxHandle.videoBitRate)
                    {
                        // Only start insert frame and calculate from Idr.
                        if (VIDEO_FRAME_TYPE_I == ptVideoSample->frameType)
                        {
                            _STREAM_MUX_ResetStuffConfig(ptVideoSample->frameRate, ptVideoSample->bitRate);
                            if (MMP_FALSE == _STREAM_MUX_CalPerFrameCount())
                            {
                                _STREAM_MUX_ResetStuffConfig(AVC_FRAME_RATE_UNKNOW, 0);
                            }
                        }
                        else
                        {
                            _STREAM_MUX_ResetStuffConfig(AVC_FRAME_RATE_UNKNOW, 0);
                        }
                    }

                    if (gtStreamMuxHandle.sizePerFrame)
                    {
                        MMP_INT32 fillStuffSize = 0;
                        if (VIDEO_FRAME_TYPE_I == ptVideoSample->frameType)
                        {
                            gtStreamMuxHandle.remainStuffBufferSize = 0;
                        }
                        fillStuffSize = _STREAM_MUX_GetStuffSize(ptVideoSample->dataSize);
                        if (fillStuffSize)
                        {
                            MMP_UINT8* pStuffStart = &ptVideoSample->pData[ptVideoSample->dataSize];
                            PalMemset(pStuffStart, 0x0, fillStuffSize);
                            ptVideoSample->dataSize += fillStuffSize;
                        }
                        TsMuxGeneratePesTsPacket(ptTsMuxHandle, 0, MMP_TRUE | idrFlag, ptVideoSample->pData, ptVideoSample->dataSize, bDiscontinuity);
                    }
                    else
                    {
                        dbg_msg(DBG_MSG_TYPE_INFO, "size per frame is 0\n");
                    }
                }
                else
                {
                    TsMuxGeneratePesTsPacket(ptTsMuxHandle, 0, MMP_TRUE | idrFlag, ptVideoSample->pData, ptVideoSample->dataSize, bDiscontinuity);
                }
            }
            else
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "(%d)\n", __LINE__);
            }
            //timeDuration = PalGetDuration(testClock);
            //if (timeDuration > 10)
            //{
            //    dbg_msg(DBG_MSG_TYPE_INFO, "data: %u, %u ms\n", ptVideoSample->dataSize, timeDuration);
            //}
            //}
#endif
            gtStreamMuxHandle.videoTimeStamp = ptVideoSample->timeStamp;
            avSyncSetCurrentVideoMuxTime(ptVideoSample->timeStamp);
            if ((ptTsMuxHandle->frameCount = ((ptTsMuxHandle->frameCount + 1) & 0x3)) == 0)
            {
                TsMuxGenerateNitTsPacket(gtStreamMuxHandle.ptTsMux);
                TsMuxGenerateSdtTsPacket(gtStreamMuxHandle.ptTsMux);
                TsMuxGeneratePatTsPacket(gtStreamMuxHandle.ptTsMux);
                TsMuxGeneratePmtTsPacket(gtStreamMuxHandle.ptTsMux);
#ifdef AV_SENDER_SECURITY_MODE
                TsMuxGenerateSktTsPacket(gtStreamMuxHandle.ptTsMux);
#endif
            }
            pfVideoInputQueueSetFree(video_queueId, (void**) &ptVideoSample);
        }
        else
        {
            if (gtStreamMuxHandle.tEncodeInfo.bVideoLock != avSyncIsVideoStable())
            {
                gtStreamMuxHandle.tEncodeInfo.bVideoLock = avSyncIsVideoStable();
                gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
            }
            bContinue = MMP_FALSE;
        }

        if (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void**)&ptAudioSample))
        {
#ifdef AV_SENDER_SECURITY_MODE
            //static PAL_CLOCK_T sysBaseClock = 0;
            //PAL_CLOCK_T sysClock;

            mmpDpuGenerateRandomData(ptTsMuxHandle->pInitVector, 16);
            //PalMemcpy(ptTsMuxHandle->pSessionKey, pAesKey, 16);
            tDpuContext.DpuMode = AES_CTR_MODE;
            tDpuContext.Keylength = KEY_LENTGH_128_BIT;
            tDpuContext.Vectorlength = VECTOR_LENTGH_4_VECTORS;
            tDpuContext.SrcAddress = (MMP_UINT32)ptAudioSample->pData;
            tDpuContext.DstAddress = (MMP_UINT32)ptAudioSample->pData;
            tDpuContext.DataSize = ((ptAudioSample->dataSize + 16) & 0xFFFFFFF0);
            tDpuContext.RefKeyAddress = (MMP_UINT32*) ptTsMuxHandle->pSessionKey;
            tDpuContext.Vectors = (MMP_UINT32*) ptTsMuxHandle->pInitVector;
            mmpDpuEncode(&tDpuContext);
#endif

            TsMuxUpdateTimeStamp(&ptTsMuxHandle->ptServiceArray[0].tAudioTimestamp, ptAudioSample->timeStamp);
            TsMuxGeneratePesTsPacket(ptTsMuxHandle, 0, MMP_FALSE, ptAudioSample->pData, ptAudioSample->dataSize, MMP_FALSE);
            gtStreamMuxHandle.audioTimeStamp = ptAudioSample->timeStamp;
            bContinue = MMP_TRUE;
            pfAudioInputQueueSetFree(audio_queueId, (void**) &ptAudioSample);

            //if (clock == 0)
            //{
            //    clock = PalGetClock();
            //    sysClock = sysBaseClock = gtStreamMuxHandle.videoTimeStamp;
            //}
            //else
            //{
            //    sysClock = sysBaseClock + PalGetDuration(clock);
            //}
            //
            //dbg_msg(DBG_MSG_TYPE_INFO, "v(%d)a(%d)va-gap(%d)\n",
            //    gtStreamMuxHandle.videoTimeStamp,
            //    gtStreamMuxHandle.audioTimeStamp,
            //    (MMP_INT)gtStreamMuxHandle.videoTimeStamp - gtStreamMuxHandle.audioTimeStamp);
            if ((MMP_INT)(gtStreamMuxHandle.videoTimeStamp - gtStreamMuxHandle.audioTimeStamp) > AV_GAP_THRESHOLD ||
                (MMP_INT)(gtStreamMuxHandle.videoTimeStamp - gtStreamMuxHandle.audioTimeStamp) < -AV_GAP_THRESHOLD)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "v(%d)a(%d)va-gap(%d)\n",
                    gtStreamMuxHandle.videoTimeStamp,
                    gtStreamMuxHandle.audioTimeStamp,
                    (MMP_INT)gtStreamMuxHandle.videoTimeStamp - gtStreamMuxHandle.audioTimeStamp);
            }
        }

        if (MMP_FALSE == bContinue)
        {
            break;
        }
    }
}

static void
_STREAM_MUX_DoStop(
    void)
{
    VIDEO_SAMPLE*   ptVideoSample = MMP_NULL;
    AUDIO_SAMPLE*   ptAudioSample = MMP_NULL;
    QUEUE_ID        video_queueId                = gtStreamMuxHandle.tVideoEncoderHandle.queueId;
    QUEUE_OPERATION pfVideoInputQueueGetReady    = gtStreamMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfVideoInputQueueSetFree     = gtStreamMuxHandle.tVideoEncoderHandle.ptQueueHandle->pfSetFree;
    QUEUE_ID        audio_queueId                = gtStreamMuxHandle.tAudioInHandle.queueId;
    QUEUE_OPERATION pfAudioInputQueueGetReady    = gtStreamMuxHandle.tAudioInHandle.ptQueueHandle->pfGetReady;
    QUEUE_OPERATION pfAudioInputQueueSetFree     = gtStreamMuxHandle.tAudioInHandle.ptQueueHandle->pfSetFree;
    MMP_UINT32      portStart;
    MMP_UINT16      videoPid = 0;
    MMP_UINT16      audioPid = 0;

    while (QUEUE_NO_ERROR == pfVideoInputQueueGetReady(video_queueId, (void**)&ptVideoSample))
    {
        pfVideoInputQueueSetFree(video_queueId, (void**)&ptVideoSample);
    }
    while (QUEUE_NO_ERROR == pfAudioInputQueueGetReady(audio_queueId, (void**)&ptAudioSample))
    {
        pfAudioInputQueueSetFree(audio_queueId, (void**)&ptAudioSample);
    }
    gtStreamMuxHandle.bufferIndex = 0;
    // gtStreamMuxHandle.writeBufferIndex = 0;
    gtStreamMuxHandle.videoTimeStamp = 0;
    gtStreamMuxHandle.audioTimeStamp = 0;

    gtStreamMuxHandle.frameRate = AVC_FRAME_RATE_UNKNOW;
    gtStreamMuxHandle.sizePerFrame = 0;
    gtStreamMuxHandle.remainStuffBufferSize = 0;
    gtStreamMuxHandle.bWaitFirstI = MMP_TRUE;
    if (gtStreamMuxHandle.ptTsMux)
    {
        gtStreamMuxHandle.ptTsMux->tsWriteIndex = 0;
        gtStreamMuxHandle.ptTsMux->frameCount = 0;
        if (gtStreamMuxHandle.bTsoSupport)
        {
#if (defined (IT9919_144TQFP) || defined(IT9913_128LQFP))
            portStart = 13;
#else
            portStart = 34;
#endif
            // Terminate TSO engine first
            mmpTsoTerminate();
            // Enable PCR auto injection
            //mmpTsoInitialize(portStart, gtStreamMuxHandle.ptTsMux->ptServiceArray[0].videoPid, 40, 0, gtStreamMuxHandle.pTsBuffer[0], gtStreamMuxHandle.bufferSize * BUFFER_VALID_COUNT, MMP_FALSE);
            PsiSiMgrGetEsPid(0, &videoPid, &audioPid);
            mmpTsoInitialize(portStart, videoPid, 40, 0, gtStreamMuxHandle.pTsBuffer[0], gtStreamMuxHandle.bufferSize * BUFFER_VALID_COUNT, MMP_FALSE);
            mmpTsoEnable();
        }
        TsMuxSetTsBuffer(gtStreamMuxHandle.ptTsMux, (MMP_UINT8*) gtStreamMuxHandle.pTsBuffer[gtStreamMuxHandle.bufferIndex], gtStreamMuxHandle.bufferSize);
    }
}

static void
_STREAM_MUX_EventNotify(
    MMP_UINT32  reason,
    MMP_UINT32  data)
{
    MPS_NOTIFY_REASON_DATA* ptNotifyData = MMP_NULL;

    gtStreamMuxHandle.tWriteCmdObj.cmd = MPS_COMMAND_EVENT_NOTIFY;
    if (reason)
    {
        ptNotifyData = (MPS_NOTIFY_REASON_DATA*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                             sizeof(MPS_NOTIFY_REASON_DATA));
        ptNotifyData->notifyReason = reason;
        ptNotifyData->bDatatNeedFree = MMP_FALSE;
        ptNotifyData->data = data;
        gtStreamMuxHandle.tWriteCmdObj.extraData = (void*)ptNotifyData;
        mpsCmdQ_SendCommand(gtStreamMuxHandle.ptElement->specialEventId, &gtStreamMuxHandle.tWriteCmdObj);
    }
    else
        PalSetEvent(elementThreadToMpsThread);
}

static void*
_STREAM_MUX_ThreadFunction(
    void* arg)
{
    MPS_PROPERITY_DATA* ptData = MMP_NULL;

    if (arg) { } // avoid compiler warning

    for (;;)
    {
        if (MPS_STATE_RUN == gtStreamMuxHandle.mpsState)
            _STREAM_MUX_DoPlay();

        mpsCmdQ_ReceiveCommand(gtStreamMuxHandle.ptElement->cmdQueueId, &gtStreamMuxHandle.tReadCmdObj);

        if ((gtStreamMuxHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK) != MPS_COMMAND_NULL)
        {
            switch (gtStreamMuxHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK)
            {
            case MPS_COMMAND_PLAY:
                gtStreamMuxHandle.mpsState = MPS_STATE_RUN;
                _STREAM_MUX_TableUpdate();
                _STREAM_MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP:
                gtStreamMuxHandle.mpsState = MPS_STATE_STOP;
                _STREAM_MUX_DoStop();
                _STREAM_MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_TERMINATE:
                gtStreamMuxHandle.mpsState = MPS_STATE_ZERO;
                goto end;

            /*
            case MPS_COMMAND_SET_CAPTURE_DEVICE:
                {
                    CAPTURE_DEVICE Device_ID;
                    ptData      = (MPS_PROPERITY_DATA*)gtStreamMuxHandle.tReadCmdObj.extraData;
                    Device_ID   = (CAPTURE_DEVICE)ptData->data;

                    switch(Device_ID)
                    {
                    case MMP_CAP_DEV_ADV7180:
                        gtStreamMuxHandle.tEncodeInfo.videoInputSource = 1;
                        break;

                    case MMP_CAP_DEV_CAT9883:
                    case MMP_CAP_DEV_HDMIRX:
                        gtStreamMuxHandle.tEncodeInfo.videoInputSource = 0;
                        break;

                    case MMP_CAP_DEV_SENSOR:
                        gtStreamMuxHandle.tEncodeInfo.videoInputSource = 2;
                        break;

                    case MMP_CAP_UNKNOW_DEVICE:
                    default:
                        gtStreamMuxHandle.tEncodeInfo.videoInputSource = 15;
                        break;
                    }
                    gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;

                    _STREAM_MUX_EventNotify(MMP_NULL, 0);
                    break;
                }
            case MPS_COMMAND_SET_ENCODE_PARAMETER:
            case MPS_COMMAND_SET_ISP_MODE:
            case MPS_COMMAND_SET_ENABLE_AV_ENGINE:
            */
            case MPS_COMMAND_GET_PROPERTY:
                _STREAM_MUX_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_SET_PROPERTY:
                ptData = (MPS_PROPERITY_DATA*)gtStreamMuxHandle.tReadCmdObj.extraData;
                switch (ptData->properityId)
                {
                case MPS_PROPERITY_SET_MUXER_PARAMETER:
                    _STREAM_MUX_DoStop();
                    _STREAM_MUX_ParamInit((TS_MUXER_PARAMETER*) ptData->data);
                    break;

                case MPS_PROPERITY_SET_CAPTURE_DEVICE:
                    {
                        CAPTURE_DEVICE Device_ID;
                        ptData      = (MPS_PROPERITY_DATA*)gtStreamMuxHandle.tReadCmdObj.extraData;
                        Device_ID   = (CAPTURE_DEVICE)ptData->data;

                        switch(Device_ID)
                        {
                        case MMP_CAP_DEV_ADV7180:
                            gtStreamMuxHandle.tEncodeInfo.videoInputSource = 1;
                            break;

                        case MMP_CAP_DEV_CAT9883:
                        case MMP_CAP_DEV_HDMIRX:
                            gtStreamMuxHandle.tEncodeInfo.videoInputSource = 0;
                            break;

                        case MMP_CAP_DEV_SENSOR:
                            gtStreamMuxHandle.tEncodeInfo.videoInputSource = 2;
                            break;

                        case MMP_CAP_UNKNOW_DEVICE:
                        default:
                            gtStreamMuxHandle.tEncodeInfo.videoInputSource = 15;
                            break;
                        }
                        gtStreamMuxHandle.bUpdateEncodeInfo = MMP_TRUE;
                    }
                    break;

                default:
                    break;
                }
                _STREAM_MUX_EventNotify(MMP_NULL, 0);
                break;

            default:
                break;
            }
        }

        PalSleep(5);
    }

end:
    _STREAM_MUX_EventNotify(MMP_NULL, 0);
    return MMP_NULL;
}

void
_STREAM_MUX_ResetStuffConfig(
    AVC_FRAME_RATE frameRate,
    MMP_UINT32     kBitRate)
{
    gtStreamMuxHandle.frameRate = frameRate;
    gtStreamMuxHandle.videoBitRate = kBitRate;
    gtStreamMuxHandle.sizePerFrame = 0;
    gtStreamMuxHandle.remainStuffBufferSize = 0;
}

MMP_BOOL
_STREAM_MUX_CalPerFrameCount(
    void)
{
    MMP_UINT32 thresholdByteRate = (((gtStreamMuxHandle.videoBitRate * 1024) * STUFF_PERCENT_THRESHOLD) / 100) / 8;
    MMP_BOOL bResult = MMP_TRUE;
    switch (gtStreamMuxHandle.frameRate)
    {
    case AVC_FRAME_RATE_25HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 25;
        break;

    case AVC_FRAME_RATE_50HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 50;
        break;

    case AVC_FRAME_RATE_30HZ:
        case AVC_FRAME_RATE_VESA_30HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 30;
        break;

    case AVC_FRAME_RATE_60HZ:
        case AVC_FRAME_RATE_VESA_60HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 60;
        break;

    case AVC_FRAME_RATE_29_97HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 30;
        break;

    case AVC_FRAME_RATE_59_94HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 60;
        break;

    case AVC_FRAME_RATE_24HZ:
    case AVC_FRAME_RATE_23_97HZ:
        gtStreamMuxHandle.sizePerFrame = thresholdByteRate / 24;
        break;

    default:
        bResult = MMP_FALSE;
        break;
    }

    dbg_msg(DBG_MSG_TYPE_INFO, "threshold byte rate: %u, per frame size: %u bytes\n", thresholdByteRate, gtStreamMuxHandle.sizePerFrame);
    if (0 == gtStreamMuxHandle.sizePerFrame)
    {
        bResult = MMP_FALSE;
    }
    return bResult;
}

MMP_INT32
_STREAM_MUX_GetStuffSize(
    MMP_UINT32 frameSize)
{
    MMP_INT32 outStuffSize = gtStreamMuxHandle.sizePerFrame - frameSize;
    gtStreamMuxHandle.remainStuffBufferSize += outStuffSize;
    if (outStuffSize > 0 && gtStreamMuxHandle.remainStuffBufferSize > 0)
    {
        if (gtStreamMuxHandle.remainStuffBufferSize < outStuffSize)
        {
            outStuffSize =  gtStreamMuxHandle.remainStuffBufferSize;
        }
        gtStreamMuxHandle.remainStuffBufferSize -= outStuffSize;
        return outStuffSize;
    }
    else
    {
        return 0;
    }
}

static MMP_BOOL
_IsContentProtection(
    void)
{
    if (gtDisableHDCP)
        return MMP_FALSE;
    else if ((mmpCapGetCaptureDevice() != CAPTURE_DEV_HDMIRX)
          || (!mmpHDMIRXIsHDCPOn()))
        return MMP_FALSE;
    else
        return MMP_TRUE;
}
