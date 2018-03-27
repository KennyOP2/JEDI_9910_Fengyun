#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task_audio_in.h"
#include "mps_system.h"
#include "mmp_aud.h"
#include "core_interface.h"
#include "mmp_hdmirx.h"
#include "mmp_i2s.h"
#include "host/ahb.h"
#include "host/gpio.h"
#include "mmp_capture.h"
#include "mmp_spdif.h"

#define FC_MALLOC(ptr, size) \
    do { \
        (ptr) = (void *)memalign(2048, (size)); \
        if ((ptr) == NULL) SERR(); \
        if ((unsigned)(ptr) % 2048) SERR(); \
    } while (0)

#define SERR() do { dbg_msg(DBG_MSG_TYPE_INFO, "ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while (1); } while (0)

#ifdef SUPPORT_MIC_MIXED
    #define SUPPORT_DIGITAL_MIXER
#endif

//=============================================================================
//                Constant Definition
//=============================================================================
#define SPDIF_ENABLE

#define AV_RESYNC_GAP_THRESHOLD    220
#define WAIT_I2S_TIMEOUT           15
#define BUF_CTRL_GPIO              43
#define MONO_CHANNEL               1
#define STEREO_CHANNEL             2
#define DEFAULT_SAMPLING_RATE      48000
#define DEFAULT_CHANNEL_NUMBER     2

#define SPDIF_DMA_PER_BLOCK_SIZE   2 * 1024
#define SPDIF_DMA_BUFFER_COUNT     32

#define ABNORMAL_NULL_PATTERN_SIZE 128
#define I2S_NOWORK_THRESHOLD       10
#define I2S_MISMATCH_THRESHOLD     10
#define I2S_ABNORMAL_THRESHOLD     60

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum AUDIO_INIT_STATE_TAG
{
    AUDIO_INIT_I2S,
    AUDIO_WAIT_AUDIO_START,
    AUDIO_INIT_ENGINE,
    AUDIO_INIT_DONE
} AUDIO_INIT_STATE;

typedef struct TASK_AUDIO_IN_HANDLE_TAG
{
    MPS_ELEMENT      *ptElement;
    MPS_STATE        mpsState;
    STREAM_HANDLE    tStreamMuxHandle;
    PAL_THREAD       ptAudioInThread;
    MPS_CMD_OBJ      tReadCmdObj;
    MPS_CMD_OBJ      tWriteCmdObj;
    //for dexatek
    MMP_UINT8        *pI2SBuffer[4];
    MMP_UINT8        *pMixerBuffer;
    MMP_UINT32       mixerBufferSize;
    MMP_UINT32       mixerBufferWriteIndex;
    MMP_UINT32       mixerBufferReadIndex;
    MMP_BOOL         bEngineLoaded;
    MMP_BOOL         bCodecChange;
    MMP_BOOL         bAudioReset;
    MMP_UINT32       curAudioTime;
#ifdef SPDIF_ENABLE
    MMP_UINT8        *pSpdifBuffer;
#endif
    MMP_AUDIO_ENGINE audioEncoderType;
    MMP_UINT32       samplingRate;
    MMP_UINT32       bitRate;
    MMP_UINT32       channelNum;
    AUDIO_INIT_STATE initState;
    MMP_BOOL         bFullReset;
    MMP_BOOL         bAudioParamChange;
} TASK_AUDIO_IN_HANDLE;

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_EVENT elementThreadToMpsThread;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static TASK_AUDIO_IN_HANDLE gtAudioInHandle = {0};
static MMP_CAP_DEVICE_ID    gtPreviousDevice                                 = MMP_CAP_UNKNOW_DEVICE;

//For HDMI header information mismatch input workaround
static MMP_UINT32           gCheckI2sClock                                   = 0;
static MMP_UINT32           gI2sNonWorkCount                                 = 0;
static MMP_UINT32           gMisMatchSamplingCount                           = 0;
static MMP_UINT32           gAbnormalNullDataCount                           = 0;
static MMP_UINT8            gAbnormalNullPattern[ABNORMAL_NULL_PATTERN_SIZE] = { 0 };
static MMP_UINT32           gPrevI2sWritePtr = 0;

static MMP_BOOL             gtPreIsDVIMode   = MMP_FALSE;

static MMP_BOOL             gtReInitCodecDev = MMP_FALSE;
static MMP_BOOL             gtEnAVEngine     = MMP_TRUE;

 unsigned int preMixerWrptr,MixerWrptr;

#define I2S_BUFFER_SIZE (64 * 1024)

#ifdef SPDIF_ENABLE
static MMP_UINT32 gSpdifAttribute[] =
{
    MMP_SPDIF_PER_DMA_BLOCK_BUFFER_SIZE,    SPDIF_DMA_PER_BLOCK_SIZE,
    MMP_SPDIF_BUFFER_COUNT,                 SPDIF_DMA_BUFFER_COUNT,
    MMP_AP_DMA_FUNCTION_PTR,                0,
    MMP_SPDIF_INPUT_IO,                     PMCLK_SPDIF,
    MMP_SPDIF_FORCE_16BIT,                  1,
    MMP_SPDIF_INPUT_SAMPLE_SIZE,            0,
    MMP_SPDIF_ATTRIB_NONE
};
#endif

// 20140424, Vincent add
// two dirty flags to indicate start and finish of mencoder process.
MMP_BOOL        gbmencoder_OpenProcess  = MMP_FALSE;
MMP_BOOL        gbmencoder_CloseProcess = MMP_FALSE;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_AUDIO_IN_InitProcess(
    void);

static void
_AUDIO_IN_Init(
    MMP_BOOL bFullReset);

static void
_AUDIO_IN_DoPlay(
    void);

static void
_AUDIO_IN_DoStop(
    void);

static void
_AUDIO_IN_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data);

static void *
_AUDIO_IN_ThreadFunction(
    void *arg);

MMP_BOOL
_AudioNeedReset(
    void);

static void
_AudioSpdifCallback(
    MMP_UINT32 writeIndex);

void
_Audio_Buffer_IC_TriState(
    MMP_BOOL flag);

static MMP_UINT32
_AUDIO_IN_GetSamplingRate(
    MMP_UINT32 currentI2sWritePtr,
    MMP_UINT32 prevI2sWritePtr,
    MMP_UINT32 waitTime);

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
taskAudioIn_Init(
    MMP_UINT32 arg)
{
    MMP_RESULT         result            = 0;
    AUDIO_ENCODE_PARAM tAudioEncodeParam = { 0 };
    const unsigned     buf_i2s_size      = 64 * 1024;//(1280 << 10); //(2 << 20);

    if (arg) { } // avoid compiler warning

    if (gtAudioInHandle.mpsState != MPS_STATE_ZERO)
        return;
#ifdef SPDIF_ENABLE
    dbg_msg(DBG_MSG_TYPE_INFO, "init SPDIF\n");
    mmpSpdifInitialize();
    gSpdifAttribute[5]           = (MMP_UINT32) _AudioSpdifCallback;
    gSpdifAttribute[11]          = 16;
    mmpSpdifSetAttribute(gSpdifAttribute);
    #ifdef SPDIF_ENABLE
    gtAudioInHandle.pSpdifBuffer = mmpSpdifGetStartBuffer();
    dbg_msg(DBG_MSG_TYPE_INFO, "spdif init buffer: 0x%X\n", gtAudioInHandle.pSpdifBuffer);
    #endif
    #ifdef I2S_DA_ENABLE
    /* init I2S */
    {
        STRC_I2S_SPEC spec;
        memset((void *)&spec, 0, sizeof(STRC_I2S_SPEC));
        spec.use_hdmirx            = 0;
        spec.internal_hdmirx       = 1;
        spec.slave_mode            = 0;
        spec.channels              = STEREO_CHANNEL;  //mono channel = 1, stereo channel = 2
        spec.sample_rate           = DEFAULT_SAMPLING_RATE;
        spec.buffer_size           = SPDIF_DMA_PER_BLOCK_SIZE * SPDIF_DMA_BUFFER_COUNT;
        spec.is_big_endian         = 1;
        spec.sample_size           = 16;
        spec.base_out_i2s_spdif    = gtAudioInHandle.pSpdifBuffer;
        spec.postpone_audio_output = 0;
        i2s_init_DAC(&spec);
    }
    #endif
#endif

    gtAudioInHandle.bAudioReset                    = MMP_TRUE;
    gtAudioInHandle.mpsState                       = MPS_STATE_STOP;
    gtAudioInHandle.ptElement                      = mpsSys_GetElement(MPS_AUDIO_IN);
    gtAudioInHandle.initState                      = AUDIO_INIT_DONE;
    gtAudioInHandle.tStreamMuxHandle.queueId       =
        (gtAudioInHandle.ptElement)->ptDestList->ptConnector->queueId;
    gtAudioInHandle.tStreamMuxHandle.ptQueueHandle =
        queueMgr_GetCtrlHandle((gtAudioInHandle.ptElement)->ptDestList->ptConnector->queueId);

    if (MMP_NULL == gtAudioInHandle.pI2SBuffer[0])
    {
        gtAudioInHandle.pI2SBuffer[0] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
        gtAudioInHandle.pI2SBuffer[1] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
        gtAudioInHandle.pI2SBuffer[2] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
        gtAudioInHandle.pI2SBuffer[3] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
    }
    /*
        tAudioEncodeParam.nInputBufferType = 2;
        tAudioEncodeParam.nStartPointer = I2S_AD32_GET_WP();
        tAudioEncodeParam.nStartTime = avSyncGetCurrentTime();
        tAudioEncodeParam.pInputBuffer  = gtAudioInHandle.pI2SBuffer[0];
        tAudioEncodeParam.nBufferLength = buf_i2s_size;
        tAudioEncodeParam.nChannels = DEFAULT_CHANNEL_NUMBER;
        tAudioEncodeParam.nSampleRate = DEFAULT_SAMPLING_RATE;
        tAudioEncodeParam.nBitrate = 192000;
        tAudioEncodeParam.nInputAudioType = AUDIO_INPUT_TYPE_PCM;
        tAudioEncodeParam.nSampleSize = 16;

       #if (defined (IT9919_144TQFP) && defined (REF_BOARD_CAMERA))
        gtAudioInHandle.bEngineLoaded = MMP_FALSE;
       #else
        dbg_msg(DBG_MSG_TYPE_INFO, "start wp: 0x%X, timeStamp: %u\n", tAudioEncodeParam.nStartPointer, tAudioEncodeParam.nStartTime);
        result = mmpAudioOpenEngine(MMP_MP2_ENCODE, tAudioEncodeParam);
        dbg_msg(DBG_MSG_TYPE_INFO, "open engine result: %u\n", result);
        gtAudioInHandle.bEngineLoaded = MMP_TRUE;
       #endif
     */
    if (MMP_NULL == gtAudioInHandle.ptAudioInThread)
    {
        gtAudioInHandle.ptAudioInThread = PalCreateThread(PAL_THREAD_AUDIO_IN,
                                                          _AUDIO_IN_ThreadFunction,
                                                          MMP_NULL, 0, 0);
    }

    gtAudioInHandle.bEngineLoaded = MMP_FALSE;
    gtAudioInHandle.initState     = AUDIO_INIT_I2S;
    gtAudioInHandle.bFullReset    = MMP_TRUE;

    gtAudioInHandle.samplingRate  = 0;
    gtAudioInHandle.channelNum    = 0;

    gtPreviousDevice              = MMP_CAP_UNKNOW_DEVICE;
    gtReInitCodecDev              = MMP_FALSE;
    gtEnAVEngine                  = MMP_TRUE;

    _Audio_Buffer_IC_TriState(MMP_TRUE);
}

void
taskAudioIn_Terminate(
    MMP_UINT32 arg)
{
    MMP_UINT32        i             = 0;
    MMP_CAP_DEVICE_ID currentDevice = mmpCapGetCaptureDevice();

    if (arg) { } // avoid compiler warning

    if (gtAudioInHandle.ptAudioInThread != MMP_NULL)
        PalDestroyThread(gtAudioInHandle.ptAudioInThread);

    i2s_deinit_ADC(MMP_TRUE);
    mmpAudioStop();
    mmpAudioTerminate();
    avSyncSetAudioInitFinished(MMP_FALSE);

    if (currentDevice == MMP_CAP_DEV_HDMIRX)
        mmpHDMIRXSetProperty(HDMIRX_IS_AUDIO_RESET, 1);

    for (i = 0; i < 4; i++)
    {
        if (gtAudioInHandle.pI2SBuffer[i])
        {
            //free(gtAudioInHandle.pI2SBuffer[i]);
            PalHeapFree(PAL_HEAP_DEFAULT, gtAudioInHandle.pI2SBuffer[i]);
        }
    }

    gtPreviousDevice = MMP_CAP_UNKNOW_DEVICE;
    gtReInitCodecDev = MMP_FALSE;
    gtEnAVEngine     = MMP_TRUE;

#ifdef SPDIF_ENABLE
    mmpSpdifSetEngineState(MMP_FALSE);
    mmpSpdifTerminate();
#endif

    _Audio_Buffer_IC_TriState(MMP_TRUE);

    PalMemset(&gtAudioInHandle, 0x0, sizeof(TASK_AUDIO_IN_HANDLE));
}

QUEUE_MGR_ERROR_CODE
taskAudioIn_SetFree(
    QUEUE_ID queueId,
    void **pptSample)
{
    AUDIO_SAMPLE *ptSample = *((AUDIO_SAMPLE **) pptSample);
    if (ptSample->pData)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptSample->pData);
        ptSample->pData = MMP_NULL;
    }
    return queueMgr_SetFree(queueId, pptSample);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

static MMP_UINT32
_AUDIO_IN_GetSamplingRate(
    MMP_UINT32 currentI2sWritePtr,
    MMP_UINT32 prevI2sWritePtr,
    MMP_UINT32 waitTime)
{
    MMP_UINT32 samplePerMs = 0;
    if (gtAudioInHandle.channelNum)
    {
        if (currentI2sWritePtr >= prevI2sWritePtr)
        {
            samplePerMs = ((currentI2sWritePtr - prevI2sWritePtr) / (2 * gtAudioInHandle.channelNum)) / waitTime;
        }
        else
        {
            samplePerMs = ((currentI2sWritePtr + (I2S_BUFFER_SIZE - prevI2sWritePtr)) / (2 * gtAudioInHandle.channelNum)) / waitTime;
        }
    }
    if (samplePerMs > 46)
    {
        return 48000;
    }
    else if (samplePerMs > 42)
    {
        return 44100;
    }
    else if (samplePerMs > 30)
    {
        return 32000;
    }
    else
    {
        return 0;
    }
}

static void
_AUDIO_IN_InitProcess(
    void)
{
    const unsigned     buf_i2s_size      = I2S_BUFFER_SIZE;//(1280 << 10); //(2 << 20);
    unsigned char      *buf_i2s;
    unsigned char      *buf_in_hdmi[4];
    AUDIO_ENCODE_PARAM tAudioEncodeParam = { 0 };
    MMP_UINT32         result            = 0;
    STRC_I2S_SPEC      spec;
    MMP_UINT32         channelNum        = 0;
    MMP_UINT32         samplingRate      = 0;
    MMP_UINT32         audioType         = 0;
    MMP_UINT16         timeOut           = 0;
    MMP_BOOL           bAudioParamChange = MMP_FALSE;
    MMP_UINT32         prevWp            = 0;
    MMP_CAP_DEVICE_ID  currentDevice     = mmpCapGetCaptureDevice();
    MMP_BOOL           isDVIMode         = (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI) ? MMP_TRUE : MMP_FALSE;

    //printf("kenny %d,%d\r\n",avSyncIsVideoStable(),gtEnAVEngine);
    if (!avSyncIsVideoStable() && !gtEnAVEngine)
    {
        //printf("kenny audio init return\r\n");
        gtAudioInHandle.initState = AUDIO_INIT_I2S;
        return;
    }

    switch (gtAudioInHandle.initState)
    {
    case AUDIO_INIT_I2S:
        {
	   printf("kenny init i2s dvi =%d\r\n",isDVIMode);
            if (currentDevice == MMP_CAP_DEV_HDMIRX && !isDVIMode)
            {
                samplingRate = mmpHDMIRXGetProperty(HDMIRX_AUDIO_SAMPLERATE);
                if (samplingRate == 0)
                {
                    samplingRate = DEFAULT_SAMPLING_RATE;
                }
                channelNum = mmpHDMIRXGetProperty(HDMIRX_AUDIO_CHANNEL_NUMBER);
                if (channelNum == 0 || channelNum > 2)
                {
                    channelNum = DEFAULT_CHANNEL_NUMBER;
                }

                if (gtAudioInHandle.samplingRate != samplingRate || gtAudioInHandle.channelNum != channelNum)
                {
                    gtAudioInHandle.bAudioParamChange = MMP_TRUE;
                }

                gtAudioInHandle.samplingRate = samplingRate;
                gtAudioInHandle.channelNum   = channelNum;
            }
            else
            {
                //default Sampling Rate and Channel number
                if (gtAudioInHandle.samplingRate != DEFAULT_SAMPLING_RATE || gtAudioInHandle.channelNum != DEFAULT_CHANNEL_NUMBER)
                {
                    gtAudioInHandle.bAudioParamChange = MMP_TRUE;
                }
                gtAudioInHandle.samplingRate = DEFAULT_SAMPLING_RATE;
                gtAudioInHandle.channelNum   = DEFAULT_CHANNEL_NUMBER;
            }

            /* prepare I2S buffer */
            if (MMP_NULL == gtAudioInHandle.pI2SBuffer[0])
            {
                gtAudioInHandle.pI2SBuffer[0] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
                gtAudioInHandle.pI2SBuffer[1] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
                gtAudioInHandle.pI2SBuffer[2] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
                gtAudioInHandle.pI2SBuffer[3] = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, buf_i2s_size);
            }

            // Get Audio Information
            memset((void *)&spec, 0, sizeof(STRC_I2S_SPEC));

            switch (currentDevice)
            {
            case MMP_CAP_DEV_ADV7180:
            case MMP_CAP_DEV_CAT9883:
            case MMP_CAP_DEV_SENSOR:
                spec.use_hdmirx      = 0;
                spec.internal_hdmirx = 0;
                spec.slave_mode      = 0;
                spec.from_LineIN     = 1;
                spec.use_hdmitx      = 0;
                spec.digital_IN      = 0;
                break;

            case MMP_CAP_DEV_HDMIRX:
#if 0
                while (!(audioType = mmpHDMIRXGetProperty(HDMIRX_AUDIO_MODE)))
                {
                    PalSleep(100);
                }
                if (audioType != HDMIRX_AUDIO_LPCM)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "------Only Support LPCM-------Current Version!\n");
                    return;
                }
#endif
#ifdef EXTERNAL_HDMIRX
                spec.use_hdmirx      = 1;
                spec.internal_hdmirx = 0;
                spec.slave_mode      = 1;
                spec.from_LineIN     = 0;
                spec.digital_IN      = 1;
                spec.use_hdmitx      = 0;
#else
                if (isDVIMode)
                {
                    spec.use_hdmirx      = 0;
                    spec.internal_hdmirx = 0;
                    spec.slave_mode      = 0;
                    spec.from_LineIN     = 1;
                    spec.use_hdmitx      = 0;
                    spec.digital_IN      = 0;
                }
                else
                {
                    spec.use_hdmirx      = 1;
                    spec.internal_hdmirx = 1;
                    spec.slave_mode      = 1;
                    spec.from_LineIN     = 0;
                    spec.digital_IN      = 1;
    #ifdef HDMI_LOOPTHROUGH
                    spec.use_hdmitx      = 1;
    #else
                    spec.use_hdmitx      = 0;
    #endif
                }
#endif
                break;

            default:
                return;
            }

            printf(" kenny channelNum: %u, samplingRate: %u\n", gtAudioInHandle.channelNum, gtAudioInHandle.samplingRate);

            spec.channels        = STEREO_CHANNEL;    //mono channel = 1, stereo channel = 2
            spec.sample_rate     = gtAudioInHandle.samplingRate;
            spec.buffer_size     = buf_i2s_size;
            spec.is_big_endian   = 1;
            spec.sample_size     = 16;

            spec.base_in_hdmi[0] = gtAudioInHandle.pI2SBuffer[0];
            spec.base_in_hdmi[1] = gtAudioInHandle.pI2SBuffer[0];
            spec.base_in_hdmi[2] = gtAudioInHandle.pI2SBuffer[0];
            spec.base_in_hdmi[3] = gtAudioInHandle.pI2SBuffer[0];
            spec.base_in_i2s     = gtAudioInHandle.pI2SBuffer[0];
            spec.from_MIC_IN     = 1;    /* MIC in always set to 1 */
            spec.record_mode     = 1;

            printf("kenny spec.base_in_i2s = 0x%x buf_i2s_size = %d\n", spec.base_in_i2s, buf_i2s_size);

            if (gtAudioInHandle.bEngineLoaded)
            {
                if (gtAudioInHandle.bFullReset && gtAudioInHandle.bAudioParamChange)
                    i2s_deinit_ADC(MMP_TRUE);
                //else
                //    i2s_deinit_ADC(MMP_FALSE);

                if (gtAudioInHandle.bCodecChange || gtAudioInHandle.bFullReset)
                {
                    mmpAudioStop();
                }
            }

            if (gtAudioInHandle.bFullReset)
            {
                if (gtAudioInHandle.bAudioParamChange || !gtAudioInHandle.bEngineLoaded)
                {
                    i2s_deinit_ADC(MMP_TRUE);
                    i2s_init_ADC(&spec, MMP_TRUE);
                }
            }
            else
                i2s_init_ADC(&spec, MMP_FALSE);

#ifdef SPDIF_ENABLE
            if (gtReInitCodecDev || gtAudioInHandle.bAudioReset)
            {
                printf("slave mode =%d\r\n",spec.slave_mode);
                if (spec.slave_mode)
                    mmpSpdifSetEngineState(MMP_TRUE);
                else
                    mmpSpdifSetEngineState(MMP_FALSE);
            }
#endif
            gtAudioInHandle.initState = AUDIO_WAIT_AUDIO_START;
          // printf("kenny start audio\r\n");
        }

    case AUDIO_WAIT_AUDIO_START:
        {
	  // printf("kenny reset =%d,change =%d\r\n",gtAudioInHandle.bFullReset ,gtAudioInHandle.bCodecChange);
            if (gtAudioInHandle.bFullReset || gtAudioInHandle.bCodecChange)
            {
                MMP_UINT32 timeOutClock = PalGetClock();
                I2S_AD32_SET_RP(I2S_AD32_GET_WP());
                // check whether the I2S WP is running
                while (PalGetDuration(timeOutClock) < WAIT_I2S_TIMEOUT)
                {
                   // printf("i2s =%d,%d\r\n",prevWp,I2S_AD32_GET_WP());
                    if (prevWp && prevWp != I2S_AD32_GET_WP())
                    {
                        gtAudioInHandle.initState = AUDIO_INIT_ENGINE;
                        break;
                    }

		     // printf("spdif =%d,%d\r\n",preMixerWrptr,MixerWrptr);
			  				
                    if (preMixerWrptr && preMixerWrptr != MixerWrptr)//kenny0422
                    {
                        gtAudioInHandle.initState = AUDIO_INIT_ENGINE;
                        break;
                    }
                
			preMixerWrptr = MixerWrptr;		
                    prevWp = I2S_AD32_GET_WP();
                    PalSleep(1);
                }
             //   printf("kenny %d\r\n",gtAudioInHandle.initState);
                if (gtAudioInHandle.initState != AUDIO_INIT_ENGINE)
                {
                    return;
                }
            }
        }

    case AUDIO_INIT_ENGINE:
        {
	 //   printf("kenny audio init engine\r\n");
            if (avSyncIsVideoStable())
            {
                tAudioEncodeParam.nInputBufferType = 2;
                tAudioEncodeParam.nStartPointer    = I2S_AD32_GET_WP();
                if (gtAudioInHandle.bFullReset || gtAudioInHandle.bCodecChange)
                    tAudioEncodeParam.nStartTime = avSyncGetCurrentTime();
                else
                    tAudioEncodeParam.nStartTime = gtAudioInHandle.curAudioTime;

                tAudioEncodeParam.pInputBuffer   = gtAudioInHandle.pI2SBuffer[0];
                tAudioEncodeParam.nBufferLength  = buf_i2s_size;
                tAudioEncodeParam.nChannels      = gtAudioInHandle.channelNum;
                tAudioEncodeParam.nSampleRate    = gtAudioInHandle.samplingRate;
                tAudioEncodeParam.nOutSampleRate = 48000;

                if (gtAudioInHandle.bitRate == 0)
                {
                    gtAudioInHandle.bitRate = 192000;
                }
                tAudioEncodeParam.nBitrate        = gtAudioInHandle.bitRate;

                tAudioEncodeParam.nInputAudioType = AUDIO_INPUT_TYPE_PCM;
                tAudioEncodeParam.nSampleSize     = 16;
                dbg_msg(DBG_MSG_TYPE_INFO, "start wp: 0x%X, timeStamp: %u\n", tAudioEncodeParam.nStartPointer, tAudioEncodeParam.nStartTime);

                if (spec.digital_IN)
                {
                    //dbg_msg(DBG_MSG_TYPE_INFO, "Digital in mixer\n");
#ifdef SUPPORT_DIGITAL_MIXER
                    tAudioEncodeParam.nEnableMixer     = MMP_TRUE;
                    printf("enable Digital in mixer\r\n");
#else
                    tAudioEncodeParam.nEnableMixer     = MMP_FALSE;
#endif
                    tAudioEncodeParam.pInputBuffer     = gtAudioInHandle.pI2SBuffer[0];
                    tAudioEncodeParam.pMixBuffer       = gtAudioInHandle.pSpdifBuffer;
                    tAudioEncodeParam.nMixBufferLength = buf_i2s_size;
                }
                else
                {
                    //dbg_msg(DBG_MSG_TYPE_INFO, "Analog in mixer\n");
                    tAudioEncodeParam.pInputBuffer = gtAudioInHandle.pI2SBuffer[0];
                }
                dbg_msg(DBG_MSG_TYPE_INFO, "e_m: %u, sampling: %u, bitrate: %u, channel: %u\n",
                        tAudioEncodeParam.nEnableMixer,
                        tAudioEncodeParam.nSampleRate,
                        tAudioEncodeParam.nBitrate,
                        tAudioEncodeParam.nChannels);
                dbg_msg(DBG_MSG_TYPE_INFO, "start wp: 0x%X, timeStamp: %u\n", tAudioEncodeParam.nStartPointer, tAudioEncodeParam.nStartTime);
                if (currentDevice == MMP_CAP_DEV_HDMIRX)
                {
                    setAudioVolume(coreGetDigitalVolume());
                }
                else
                {
                    setAudioVolume(DIGITAL_AUDIO_DEFAULT);
                }
                if (gtEnAVEngine && (gtAudioInHandle.bFullReset || gtAudioInHandle.bAudioParamChange))
                {
                    result                        = mmpAudioOpenEngine(gtAudioInHandle.audioEncoderType, tAudioEncodeParam);
                    gtAudioInHandle.bEngineLoaded = MMP_TRUE;
                    dbg_msg(DBG_MSG_TYPE_INFO, "open engine result: %u\n", result);
                }
                else
                    gtAudioInHandle.bEngineLoaded = MMP_FALSE;

                gtAudioInHandle.bCodecChange      = MMP_FALSE;
                gtAudioInHandle.bAudioReset       = MMP_FALSE;
                avSyncSetAudioSampleRate(tAudioEncodeParam.nSampleRate);
                avSyncSetAudioInitFinished(MMP_TRUE);
                gtAudioInHandle.initState         = AUDIO_INIT_DONE;
                gtAudioInHandle.bAudioParamChange = MMP_FALSE;
                gtAudioInHandle.bFullReset        = MMP_FALSE;
            }
            break;
        }

    case AUDIO_INIT_DONE:
    default:
        {
            break;
        }
    }
}

static void
_AUDIO_IN_Init(
    MMP_BOOL bFullReset)
{
    gtAudioInHandle.initState  = AUDIO_INIT_I2S;
    gtAudioInHandle.bFullReset = bFullReset;
 //   printf("kenny audio init\r\n");
    _AUDIO_IN_InitProcess();
}

static MMP_UINT32 gPrevWIdx = 0;
static void
_AUDIO_IN_DoPlay(
    void)
{
    MMP_AUDIO_ENCODE_DATA tEncodeData    = { 0 };
    AUDIO_SAMPLE          *ptAudioSample = MMP_NULL;
    QUEUE_MGR_ERROR_CODE  errorCode      = QUEUE_NO_ERROR;
    MMP_UINT32            propertyValue  = 0;
    MMP_CAP_DEVICE_ID     currentDevice  = mmpCapGetCaptureDevice();
    MMP_BOOL              isDVIMode      = (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI) ? MMP_TRUE : MMP_FALSE;

#if defined (IT9919_144TQFP)
    #if defined (REF_BOARD_CAMERA)
    return;
    #endif
#endif

    if ((currentDevice != gtPreviousDevice) || (isDVIMode != gtPreIsDVIMode))
    {
       //printf("kenny %d,%d,%d,%d\r\n",currentDevice,gtPreviousDevice,isDVIMode,gtPreIsDVIMode);
        if (currentDevice == MMP_CAP_DEV_HDMIRX && !isDVIMode)
            _Audio_Buffer_IC_TriState(MMP_TRUE);
        else
            _Audio_Buffer_IC_TriState(MMP_FALSE);

        gtReInitCodecDev             = MMP_TRUE;
        gtAudioInHandle.bAudioReset  = MMP_TRUE;
       if (currentDevice == MMP_CAP_DEV_CAT9883 ||currentDevice == MMP_CAP_DEV_ADV7180||currentDevice == MMP_CAP_DEV_ADV7180_S)
        gtAudioInHandle.initState    = AUDIO_INIT_DONE;//kenny0504 AUDIO_INIT_DONE;
        else
        gtAudioInHandle.initState    = AUDIO_INIT_I2S;//kenny0504 AUDIO_INIT_DONE;
        
        gtPreviousDevice             = currentDevice;
        gtPreIsDVIMode               = isDVIMode;
        gtAudioInHandle.samplingRate = 0;
        gtAudioInHandle.channelNum   = 0;
    }

    if (gtAudioInHandle.initState != AUDIO_INIT_DONE)
    {
        _AUDIO_IN_InitProcess();
	//printf("kenny line =%d\r\n",__LINE__);
    }

    // Get Video Status and Audio Stage
    if (avSyncIsVideoStable())
    {
        MMP_UINT32 i2sWaitTime = PalGetDuration(gCheckI2sClock);
        if (0)//(i2sWaitTime > 100)
        {
            MMP_UINT32 currentI2sWritePtr = I2S_AD32_GET_WP();
            MMP_UINT32 samplingRate       = 0;

            if (currentI2sWritePtr == gPrevI2sWritePtr)
            {
                gI2sNonWorkCount++;
                if (!gtEnAVEngine)
                {
                    I2S_AD32_SET_RP(I2S_AD32_GET_WP());
                }
            }
            else
            {
                gI2sNonWorkCount = 0;
                samplingRate     = _AUDIO_IN_GetSamplingRate(currentI2sWritePtr, gPrevI2sWritePtr, i2sWaitTime);
                if (samplingRate && samplingRate != gtAudioInHandle.samplingRate)
                {
                    gMisMatchSamplingCount++;
                }
                else
                {
                    MMP_UINT8 *pStartBuffer = &gtAudioInHandle.pI2SBuffer[0][gPrevI2sWritePtr];
                    if (PalMemcmp(pStartBuffer, gAbnormalNullPattern, ABNORMAL_NULL_PATTERN_SIZE) == 0)
                    {
                        gAbnormalNullDataCount++;
                    }
                    else
                    {
                        gAbnormalNullDataCount = 0;
                    }
                    gMisMatchSamplingCount = 0;
                }
            }
            gPrevI2sWritePtr = currentI2sWritePtr;
            
            gCheckI2sClock   = PalGetClock();
            if (gtAudioInHandle.initState != AUDIO_INIT_I2S)
            {
                if (mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX)
                {
                    if (gI2sNonWorkCount > I2S_NOWORK_THRESHOLD
                        || gMisMatchSamplingCount > I2S_MISMATCH_THRESHOLD
                        || gAbnormalNullDataCount > I2S_ABNORMAL_THRESHOLD)
                    {
                        if (mmpHDMIRXGetProperty(IT6607_IS_AUDIO_ON))
                        {
                            printf("Try to reset audio sampling rate: nonwork: %d, mismatch: %dn abnormal: %d\n", gI2sNonWorkCount, gMisMatchSamplingCount, gAbnormalNullDataCount);
                            mmpHDMIRXResetAudioFS();
                            if (samplingRate && samplingRate != gtAudioInHandle.samplingRate)
                            {
                                gtAudioInHandle.initState = AUDIO_INIT_I2S;
                            }
                            gI2sNonWorkCount       = 0;
                            gMisMatchSamplingCount = 0;
                            gAbnormalNullDataCount = 0;
                            return;
                        }
                    }
                }
            }
        }

        if (currentDevice == MMP_CAP_DEV_HDMIRX && !isDVIMode)
        {
            if (mmpHDMIRXGetProperty(HDMIRX_IS_AUDIO_ON) && !isDVIMode)
            {
                if (gtAudioInHandle.bAudioReset == MMP_TRUE
                    && gtAudioInHandle.initState == AUDIO_INIT_DONE)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "(%d)audio is reset\n", __LINE__);
                    _AUDIO_IN_Init(MMP_TRUE);
			//	printf("kenny line =%d\r\n",__LINE__);		
                }
                else
                {
                    propertyValue = mmpHDMIRXGetProperty(HDMIRX_IS_AUDIO_RESET);
                    if (propertyValue || _AudioNeedReset())
                    {
                        MMP_UINT32 samplingRate = mmpHDMIRXGetProperty(HDMIRX_AUDIO_SAMPLERATE);
                        MMP_UINT32 channelNum   = mmpHDMIRXGetProperty(HDMIRX_AUDIO_CHANNEL_NUMBER);
                        if (propertyValue)
                        {
                            //dbg_msg(DBG_MSG_TYPE_INFO, "value: %u, audio is partial reset\n", propertyValue);
                            //mmpHDMIRXSetProperty(HDMIRX_IS_AUDIO_RESET, 0);
                        }

                        if ((gtAudioInHandle.samplingRate != samplingRate || gtAudioInHandle.channelNum != channelNum)
                            || (!gtAudioInHandle.bEngineLoaded && avSyncIsVideoStable() && gtEnAVEngine))
                        {
                            _AUDIO_IN_Init(MMP_TRUE);
				//printf("kenny line =%d\r\n",__LINE__);			
                        }
                    }
                }
            }
        }
        else // Analog input
        {
            if (gtAudioInHandle.initState == AUDIO_INIT_DONE
                && (gtAudioInHandle.bAudioReset == MMP_TRUE || gtAudioInHandle.bCodecChange))
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "(%d)audio is full reset\n", __LINE__);
                _AUDIO_IN_Init(MMP_TRUE);
			//printf("kenny line =%d\r\n",__LINE__);		
            }
        }
    }
    else
    {
        gtAudioInHandle.samplingRate = 0;
        gtAudioInHandle.channelNum   = 0;

        if (currentDevice != MMP_CAP_DEV_HDMIRX)
        {
            gtAudioInHandle.bAudioReset = MMP_TRUE;
            avSyncSetAudioInitFinished(MMP_FALSE);
        }
        return;
    }

    if (!gtEnAVEngine)
    	{
        return;
    	}
get_free_sample:
    errorCode = (gtAudioInHandle.tStreamMuxHandle.ptQueueHandle)->pfGetFree(
        gtAudioInHandle.tStreamMuxHandle.queueId,
        (void **) &ptAudioSample);
    // if we can get at least one empty audio sample from
    // the video sample queue...
    if (QUEUE_NO_ERROR == errorCode && gtAudioInHandle.initState == AUDIO_INIT_DONE)
    {
        PalMemset(&tEncodeData, 0x0, sizeof(MMP_AUDIO_ENCODE_DATA));
        tEncodeData.ptData = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, MMP_AUDIO_ENCODE_SIZE);
        if (mmpAudioGetEncodeData(&tEncodeData))
        {
            if (tEncodeData.nDataSize)
            {
                MMP_INT32  avGap        = 0;
                MMP_UINT32 samplingRate = 0;
                MMP_UINT32 channelNum   = 0;
                ptAudioSample->pData         = tEncodeData.ptData;
                ptAudioSample->dataSize      = tEncodeData.nDataSize;
                ptAudioSample->timeStamp     = tEncodeData.nTimeStamp;
                gtAudioInHandle.curAudioTime = ptAudioSample->timeStamp;
                avGap                        = (avSyncGetCurrentVideoMuxTime() - ptAudioSample->timeStamp);
                //dbg_msg(DBG_MSG_TYPE_INFO, "ptAudioSample->timeStamp = %d, gap: %d, i_w: 0x%X, i_r: 0x%X\n", ptAudioSample->timeStamp, avGap, I2S_AD32_GET_WP(), I2S_AD32_GET_RP());

                if (avGap > AV_RESYNC_GAP_THRESHOLD || avGap < -AV_RESYNC_GAP_THRESHOLD)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "avGap: %d\n", avGap);
                    PalHeapFree(PAL_HEAP_DEFAULT, tEncodeData.ptData);
                    // 20140424, Vincent add
                    // only allow audio re-init whiel not in mencoder process.
                    if (gbmencoder_OpenProcess == MMP_FALSE && gbmencoder_CloseProcess == MMP_FALSE)
                    {
                        //printf("_AUDIO_IN_Init due to av gap\n");
                        _AUDIO_IN_Init(MMP_TRUE);
                    }
                    return;
                }
                /*
                if (mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX)
                {
                    samplingRate = mmpHDMIRXGetProperty(HDMIRX_AUDIO_SAMPLERATE);
                    channelNum = mmpHDMIRXGetProperty(HDMIRX_AUDIO_CHANNEL_NUMBER);
                    if (samplingRate && channelNum)
                    {
                        if (samplingRate != gtAudioInHandle.samplingRate
                         || (channelNum != gtAudioInHandle.channelNum && channelNum <= 2))
                        {
                            dbg_msg(DBG_MSG_TYPE_INFO, "task_audio_in.c(%d) audio is change in playing\n", __LINE__);
                             _AUDIO_IN_Init(MMP_TRUE);
                        }
                    }
                }
                */
                (gtAudioInHandle.tStreamMuxHandle.ptQueueHandle)->pfSetReady(
                    gtAudioInHandle.tStreamMuxHandle.queueId,
                    (void **) &ptAudioSample);
                goto get_free_sample;
            }
            else
            {
                PalHeapFree(PAL_HEAP_DEFAULT, tEncodeData.ptData);
            }
        }
        else
        {
            PalHeapFree(PAL_HEAP_DEFAULT, tEncodeData.ptData);
        }
    }
}

static void
_AUDIO_IN_DoStop(
    void)
{
    MMP_CAP_DEVICE_ID currentDevice = mmpCapGetCaptureDevice();

    if (gtAudioInHandle.bEngineLoaded == MMP_TRUE)
    {
        i2s_deinit_ADC(/*mmpCapGetDeviceReboot()*/MMP_TRUE); //Keep Through
        mmpAudioStop();
        gtAudioInHandle.bEngineLoaded = MMP_FALSE;
    }
    avSyncSetAudioInitFinished(MMP_FALSE);
    gtAudioInHandle.bAudioReset = MMP_TRUE;

    if (currentDevice == MMP_CAP_DEV_HDMIRX)
        mmpHDMIRXSetProperty(HDMIRX_IS_AUDIO_RESET, 1);

#ifdef SPDIF_ENABLE
    mmpSpdifSetEngineState(MMP_FALSE);
#endif

    gtAudioInHandle.initState    = AUDIO_INIT_I2S;
    gtAudioInHandle.bFullReset   = MMP_TRUE;

    gtAudioInHandle.samplingRate = 0;
    gtAudioInHandle.channelNum   = 0;

    gtPreviousDevice             = MMP_CAP_UNKNOW_DEVICE;
    gtEnAVEngine                 = MMP_TRUE;
}

static void
_AUDIO_IN_EventNotify(
    MMP_UINT32 reason,
    MMP_UINT32 data)
{
    MPS_NOTIFY_REASON_DATA *ptNotifyData = MMP_NULL;

    gtAudioInHandle.tWriteCmdObj.cmd = MPS_COMMAND_EVENT_NOTIFY;
    if (reason)
    {
        ptNotifyData                           = (MPS_NOTIFY_REASON_DATA *)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                                        sizeof(MPS_NOTIFY_REASON_DATA));
        ptNotifyData->notifyReason             = reason;
        ptNotifyData->bDatatNeedFree           = MMP_FALSE;
        ptNotifyData->data                     = data;
        gtAudioInHandle.tWriteCmdObj.extraData = (void *)ptNotifyData;
        mpsCmdQ_SendCommand(gtAudioInHandle.ptElement->specialEventId, &gtAudioInHandle.tWriteCmdObj);
    }
    else
        PalSetEvent(elementThreadToMpsThread);
}

static void *
_AUDIO_IN_ThreadFunction(
    void *arg)
{
    MPS_PROPERITY_DATA *ptData = MMP_NULL;

    if (arg) { } // avoid compiler warning

    for (;;)
    {
        if (MPS_STATE_RUN == gtAudioInHandle.mpsState)
            _AUDIO_IN_DoPlay();

        mpsCmdQ_ReceiveCommand(gtAudioInHandle.ptElement->cmdQueueId, &gtAudioInHandle.tReadCmdObj);

        if ((gtAudioInHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK) != MPS_COMMAND_NULL)
        {
            switch ((gtAudioInHandle.tReadCmdObj.cmd & MPS_COMMAND_MASK))
            {
            case MPS_COMMAND_PLAY:
                gtAudioInHandle.mpsState = MPS_STATE_RUN;
                _AUDIO_IN_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_STOP:
                gtAudioInHandle.mpsState = MPS_STATE_STOP;
                _AUDIO_IN_DoStop();
                _AUDIO_IN_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_TERMINATE:
                gtAudioInHandle.mpsState = MPS_STATE_ZERO;
                goto end;

            case MPS_COMMAND_START_RECORD:
            case MPS_COMMAND_START_JPEG_RECORD:
            case MPS_COMMAND_STOP_JPEG_RECORD:
            case MPS_COMMAND_STOP_RECORD:
            case MPS_COMMAND_CLOSE:
            //case MPS_COMMAND_SET_CAPTURE_DEVICE:
            //case MPS_COMMAND_SET_ENCODE_PARAMETER:
            //case MPS_COMMAND_SET_ISP_MODE:
                _AUDIO_IN_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_GET_PROPERTY:
                ptData = (MPS_PROPERITY_DATA *)gtAudioInHandle.tReadCmdObj.extraData;
                switch (ptData->properityId)
                {
                case MPS_PROPERITY_GET_AUDIO_ENCODE_PARAMETER:
                    {
                        CORE_AUDIO_ENCODE_PARA *ptAudioEnocdePara = (CORE_AUDIO_ENCODE_PARA *) ptData->data;
                        MMP_AUDIO_ENGINE       audioEngineType    = gtAudioInHandle.audioEncoderType;

                        switch (audioEngineType)
                        {
                        case MMP_MP2_ENCODE:
                        default:
                            ptAudioEnocdePara->audioEncoderType = MPEG_AUDIO_ENCODER;
                            break;

                        case MMP_AAC_ENCODE:
                            ptAudioEnocdePara->audioEncoderType = AAC_AUDIO_ENCODER;
                            break;
                        }
                        ptAudioEnocdePara->bitRate = gtAudioInHandle.bitRate;
                        dbg_msg(DBG_MSG_TYPE_INFO, "get audio bitrate(type): %u(%d)\n", ptAudioEnocdePara->bitRate, ptAudioEnocdePara->audioEncoderType);
                        break;
                    }

                default:
                    break;
                }
                _AUDIO_IN_EventNotify(MMP_NULL, 0);
                break;

            case MPS_COMMAND_SET_PROPERTY:
                ptData = (MPS_PROPERITY_DATA *)gtAudioInHandle.tReadCmdObj.extraData;
                switch (ptData->properityId)
                {
                case MPS_PROPERITY_SET_MUXER_PARAMETER:
                    {
                        TS_MUXER_PARAMETER *ptMuxerParam   = (TS_MUXER_PARAMETER *) ptData->data;
                        MMP_AUDIO_ENGINE   audioEncodeType = MMP_MP2_ENCODE;
                        switch (ptMuxerParam->audioEncoderType)
                        {
                        case MPEG_AUDIO_ENCODER:
                        default:
                            audioEncodeType = MMP_MP2_ENCODE;
                            break;

                        case AAC_AUDIO_ENCODER:
                            audioEncodeType = MMP_AAC_ENCODE;
                            break;
                        }
                        if (gtAudioInHandle.audioEncoderType != audioEncodeType)
                        {
                            gtAudioInHandle.bCodecChange     = MMP_TRUE;
                            gtAudioInHandle.audioEncoderType = audioEncodeType;
                            _AUDIO_IN_Init(MMP_TRUE);
                        }
                    }
                    break;

                case MPS_PROPERITY_SET_AUDIO_ENCODE_PARAMETER:
                    {
                        CORE_AUDIO_ENCODE_PARA *ptAudioEnocdePara = (CORE_AUDIO_ENCODE_PARA *) ptData->data;
                        MMP_AUDIO_ENGINE       audioEngineType    = MMP_RESERVED;
                        switch (ptAudioEnocdePara->audioEncoderType)
                        {
                        case MPEG_AUDIO_ENCODER:
                        default:
                            audioEngineType = MMP_MP2_ENCODE;
                            break;

                        case AAC_AUDIO_ENCODER:
                            audioEngineType = MMP_AAC_ENCODE;
                            break;
                        }

                        if (gtAudioInHandle.audioEncoderType != audioEngineType
                         || gtAudioInHandle.bitRate != ptAudioEnocdePara->bitRate)
                        {
                            if (gtAudioInHandle.audioEncoderType != audioEngineType)
                            {
                                gtAudioInHandle.bCodecChange = MMP_TRUE;
                            }
                            gtAudioInHandle.audioEncoderType = audioEngineType;
                            gtAudioInHandle.bitRate          = ptAudioEnocdePara->bitRate;
                            dbg_msg(DBG_MSG_TYPE_INFO, "set audio bitrate(type): %u(%d)\n", gtAudioInHandle.bitRate, gtAudioInHandle.audioEncoderType);
                            _AUDIO_IN_Init(MMP_TRUE);
                        }
                    }
                    break;

                case MPS_PROPERITY_SET_ENABLE_AV_ENGINE:
                    ptData       = (MPS_PROPERITY_DATA *)gtAudioInHandle.tReadCmdObj.extraData;
                    gtEnAVEngine = (MMP_BOOL)ptData->data;
                    //dbg_msg(DBG_MSG_TYPE_INFO, "task_audio_in MPS_PROPERITY_SET_ENABLE_AV_ENGINE %d\n", gtEnAVEngine);
                    break;

                default:
                    break;
                }
                _AUDIO_IN_EventNotify(MMP_NULL, 0);
                break;

            default:
                _AUDIO_IN_EventNotify(MMP_NULL, 0);
                break;
            }
        }

        PalSleep(1);
    }

end:
    _AUDIO_IN_EventNotify(MMP_NULL, 0);
    return MMP_NULL;
}

MMP_BOOL
_AudioNeedReset(
    void)
{
    MMP_UINT32        Value;
    MMP_UINT16        div;
    MMP_BOOL          resetAudio     = MMP_FALSE;
    static MMP_UINT32 initMuteDetect = 1;
    static MMP_BOOL   curIsMute      = MMP_FALSE;
    static MMP_BOOL   preIsMute      = MMP_FALSE;

    if (avSyncIsVideoStable())
    {
        if (initMuteDetect)
        {
            Value          = or32_getMemCLK();
            div            = (MMP_UINT16)(((MMP_FLOAT)or32_getMemCLK() / 1000.0) / (192 * 10));
            mmpAVSyncCounterCtrl((MUTE_COUNTER_SEL | I2S_SOURCE_SEL), div);
            mmpAVSyncCounterReset(MUTE_COUNTER_CLEAR);
            initMuteDetect = 0;
        }

        curIsMute = mmpAVSyncMuteDetect();

        if (curIsMute == MMP_FALSE && preIsMute == MMP_TRUE)
        {
            mmpAVSyncCounterReset(MUTE_COUNTER_CLEAR);
            resetAudio = MMP_TRUE;
        }
        else
            resetAudio = MMP_FALSE;

        preIsMute = curIsMute;
    }
    else
    {
        initMuteDetect = 1;
        resetAudio     = MMP_FALSE;
        curIsMute      = MMP_FALSE;
        preIsMute      = MMP_FALSE;
    }

    //if (resetAudio)
    //{
    //    dbg_msg(DBG_MSG_TYPE_INFO, "_AudioNeedReset----audio is reset\n");
    //    gtAudioInHandle.samplingRate = 0;
    //    gtAudioInHandle.channelNum = 0;
    //}

    return resetAudio;
}

static void
_AudioSpdifCallback(
    MMP_UINT32 writeIndex)
{
#ifdef I2S_DA_ENABLE
    I2S_DA32_SET_WP(writeIndex);
#else
  //  printf( "w idx: 0x%X, i2s w: 0x%X, r: 0x%X\n", writeIndex, I2S_AD32_GET_WP(), I2S_AD32_GET_RP());
    MixerWrptr = writeIndex;
    setMixerWritePorinter(writeIndex);
#endif
}

void
_Audio_Buffer_IC_TriState(
    MMP_BOOL flag)
{
    if (flag)
    {
        //Set GPIO Initialize Value
        GPIO_SetState(BUF_CTRL_GPIO, GPIO_STATE_HI); //A->Y tri-state

        //Set GPIO Mode0
        ithGpioSetMode(BUF_CTRL_GPIO, ITH_GPIO_MODE0);

        //Set GPIO Output Mode
        GPIO_SetMode(BUF_CTRL_GPIO, GPIO_MODE_OUTPUT);
    }
    else
    {
        //Set GPIO Initialize Value
        GPIO_SetState(BUF_CTRL_GPIO, GPIO_STATE_LO); //A->Y pass

        //Set GPIO Mode0
        ithGpioSetMode(BUF_CTRL_GPIO, ITH_GPIO_MODE0);

        //Set GPIO Output Mode
        GPIO_SetMode(BUF_CTRL_GPIO, GPIO_MODE_OUTPUT);
    }
}
