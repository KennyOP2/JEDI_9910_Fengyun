/*
 * Copyright (c) 2004 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Co-processor API functoin file.
 *      Date: 2005/09/30
 *
 * @author Peter Lin
 * @version 1.0
 *
 * @author Franz Hsieh
 * @version 1.5
 */

#if !defined(DISABLE_AUDIO_CODEC)
    #include "host/host.h"
    #include "sys/sys.h"
    #include "mem/mem.h"
    #include "pal/timer.h"

    #include "mmp_types.h"
    #include "mmp_aud.h"
    #include "aud/configs.h"

//#include "aud/i2s.h"
    #if defined(__FREERTOS__)
        #include <stdio.h>
/* TASK */
        #include "FreeRTOS.h"
        #include "task.h"
        #include "tickTimer.h"
        #include "mmio.h"
        #include "pal/file.h"
    #else
        #define DrvDecode_Skip_Bits 2
        #define DrvDecode_Skip      (1 << DrvDecode_Skip_Bits)        // D[2]
        #define DrvAudioCtrl2       0x169c

    #endif

    #include "i2s.h"

// to output file
//#define OUTPUT_TO_FILE
//#define DONOT_PLAY_AUDIO
    #define MAX(a, b) (((a) > (b)) ? (a) : (b))

    #define AUDIO_WRAP_AROUND_THRESHOLD   (0x3E80000) // 65536 seconds
    #define AUDIO_WRAP_AROUND_JUDGE_RANGE (0x1F40000) // 36768 seconds

//#define AUDIO_DRIVER_SHOW_WRITE_BITRATE
    #if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
PAL_CLOCK_T  gtAudioDriverWirteBitrate;
unsigned int gAudioWrite         = 0;
unsigned int gAudioWriteDuration = 0;
    #endif

    #if defined(OUTPUT_TO_FILE)
        #if defined(__FREERTOS__)
            #include "api_f.h"
        #endif
        #include "pal/file.h"
PAL_FILE *fout = NULL;
    #endif

    #if !defined(__FREERTOS__)

    #else

        #if defined(ENABLE_CODECS_PLUGIN)
            #include "pal/file.h"
            #include "aud/codecs.h"
static MMP_WCHAR         codec_path[128] = { 0 };
static MMP_WCHAR         codec_file[128] = { 0 };
static xTaskHandle       g_task_codec = (xTaskHandle)NULL;
extern struct _codec_api codec_api;
extern char              codecbuf[CODEC_SIZE];
extern char              mp2enc_codec[CODEC_ARRAY_SIZE];
struct _codec_header     *__header = (struct _codec_header *)codecbuf;
extern int               codec_start_addr;
extern int               codec_end_addr;
            #if defined(ENABLE_CODECS_ARRAY)
extern int               *codecptr[];
            #endif
        #else //defined(ENABLE_CODECS_PLUGIN)
        #endif // defined(ENABLE_CODECS_PLUGIN)

    #endif
static MMP_UINT32 gPtsTimeBaseOffset = 0;
static MMP_UINT32 gLastDecTime       = 0;

//=============================================================================
//                              Macro Definition
//=============================================================================
    #define BufThreshold   64
    #define BSPPOSITION    (AUD_baseAddress + 0)
    #define PROPOSITION    (AUD_baseAddress + 0x10000)
    #define RISC_FIRE      (0x168c)
    #define RISC_PC        (0x16b4)

    #define MMIO_PTS_WRIDX (0x16ae)
    #define MMIO_PTS_HI    (0x16b2)
    #define MMIO_PTS_LO    (0x16b0)

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef void (*CODEC_INITIALIZE)(void);
typedef void (*CODEC_TERMINATE) (void);
typedef void (*CODEC_SETVOLUME) (MMP_UINT level);

typedef struct AUDIO_CODEC_TAG
{
    MMP_BOOL         codecInit;
    MMP_UINT         curVolume;
    MMP_UINT         adcRatio;
    MMP_UINT         dacRatio;
    MMP_UINT         sampleRate;
    MMP_UINT         codecMode;
    CODEC_INITIALIZE CODEC_Initialize;
    CODEC_TERMINATE  CODEC_Terminate;
    CODEC_SETVOLUME  CODEC_SetVolume;
} AUDIO_CODEC;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_UINT         Audio_DefEncode_Bufptr;     // default encode buffer address
static MMP_UINT         Audio_Encode_Bufptr;        // encode buffer address
static MMP_UINT         Audio_Encode_Buflen;        // encode buffer length
static MMP_UINT         Audio_Encode_Rdptr;         // encode buffer read  port address
static MMP_UINT         Audio_Encode_Wrptr;         // encode buffer write port address

static MMP_UINT         Audio_DefDecode_Bufptr;     // default decode buffer address
static MMP_UINT         Audio_Decode_Bufptr;        // decode buffer address
static MMP_UINT         Audio_Decode_Buflen;        // decode buffer length
static MMP_UINT         Audio_Decode_Rdptr;         // decode buffer read  port address
static MMP_UINT         Audio_Decode_Wrptr;         // decode buffer write port address

static MMP_BOOL         AUD_SKIP        = MMP_FALSE;
static PAL_CLOCK_T      lastTime        = 0;
static MMP_UINT         AUD_baseAddress = 0;
static MMP_UINT         AUD_freqAddress = 0;
static MMP_UINT         AUD_curPlayPos  = 0;
static MMP_UINT16       AUD_HostPll     = 0;
static MMP_BOOL         AUD_Init        = MMP_FALSE;
static MMP_AUDIO_ENGINE AUD_EngineType  = 0;                    // engine type now
static MMP_AUDIO_ENGINE AUD_PlugInType  = 0;                    // engine type now
static MMP_UINT         AUD_EngineState = MMP_AUDIO_ENGINE_UNKNOW;
static AUDIO_CODEC      AUD_Codec       = {MMP_FALSE, 0, 0, 0, 0, 0, NULL, NULL, NULL};
static MMP_UINT         AUD_decSampleRate     = 0;
static MMP_UINT         AUD_decChannel        = 0;
static MMP_UINT         AUD_nCodecSampleRate  = 0;     // audio codec set sample rate
static MMP_UINT         AUD_nCodecChannels    = 0;     // audio codec set channels
static MMP_BOOL         AUD_bPlayMusic        = MMP_FALSE;
static MMP_BOOL         AUD_bPtsSync          = MMP_FALSE;
static MMP_BOOL         AUD_bI2SInit          = MMP_FALSE;
static MMP_BOOL         AUD_bMPlayer          = MMP_FALSE;
static MMP_BOOL         AUD_bMPlayerSTCReady  = MMP_FALSE;
static MMP_BOOL         AUD_bDecodeError      = MMP_FALSE;
static MMP_BOOL         AUD_bIsPause          = MMP_FALSE;
static MMP_BOOL         AUD_bEnableSTC_Sync   = MMP_FALSE;
static MMP_BOOL         AUD_bSPDIFNonPCM      = MMP_FALSE;
static MMP_INT          AUD_nMPlayerPTS       = 0;
static MMP_UINT         AUD_nDropData         = 0;
static MMP_UINT32       AUD_nPauseSTCTime     = 0;
static MMP_UINT8        *AUD_pEngineAddress;
static MMP_UINT32       AUD_nEngineLength;
static MMP_UINT         AUD_nFadeIn           = 0;
static MMP_BOOL         AUD_bI2SOutFull       = MMP_FALSE;
static MMP_AUDIO_ENGINE AUD_SuspendEngineType = MMP_RESERVED;     // engine type now

// encode data
static MMP_UINT8        gEncodeFrame[MMP_AUDIO_ENCODE_SIZE];
static MMP_UINT         gEncodeTimeStamp            = 0; //ms
static MMP_UINT         gPreEncodeTime              = 0;
static MMP_UINT         gEncodeTimeAdding           = 0;
static MMP_INT          gReLoadEngine               = 0;
static MMP_UINT         gEncodeStartTime            = 0;
static MMP_UINT8        wave_header[48];
static MMP_UINT         write_waveHeader            = MMP_FALSE;
static MMP_UINT         wave_ADPCM                  = 0;
    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static short            Audio_Plugin_Message_Buf[16 * 1024];                            // plugin buffer address
static unsigned int     Audio_Plugin_Message_Buflen = sizeof(Audio_Plugin_Message_Buf); // plugin buffer length
    #endif

// encode bitrate
    #define AUDIO_ENCODE_DEFAULT_BITRATE 192000
// support mpeg audio bitrate range
static MMP_UINT gEncodeMPABitrate[14] =
{0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000};
// support aac audio bitrate range
static MMP_UINT gEncodeAACBitrate[11] =
{0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000};

    #if defined(WIN32) // For PC
void MMIO_Write(MMP_UINT16 reg, MMP_UINT16 val)
{
    HOST_WriteRegister(reg, val);
}

int MMIO_Read(int reg)
{
    MMP_UINT16 data;
    HOST_ReadRegister(reg, &data);
    return data;
}

    #endif
static AUDIO_ENCODE_PARAM gADEncode;      // audio driver encode parameter
static AUDIO_ENCODE_PARAM *pgAudioEncode; // pointer to audio encoder reserve structure

static unsigned int       gAudioEncodePointer;
static MMP_BOOL           writeStream      = MMP_FALSE;

static MMP_BOOL           g_EarPhoneDetect = MMP_FALSE;
    #if defined(__FREERTOS__)
static unsigned char      gMp2Encoder[]    = {
        #include "mp2encode.bin"
};
static unsigned char      gAACEncoder[] = {
        #include "aacencode.bin"
};

    #endif

//=============================================================================
//                              Extern Reference
//=============================================================================
MMP_RESULT
AUD_ResetControl(
    void);

MMP_RESULT
AUD_FireEngine(
    void);

MMP_RESULT
AUD_ResetEngine(
    void);

MMP_RESULT
AUD_ConfigEngine(
    void);

MMP_RESULT
AUD_SetRISCIIS(
    void);

    #ifdef ENABLE_AUDIO_PROCESSOR
static void
AUD_ResetAudioProcessor(
    void);

static void
AUD_FireAudioProcessor(
    void);
    #endif
MMP_RESULT
AUD_ResetAudioEngineType(
    void);

//=============================================================================
//                              Function Definition
//=============================================================================

    #ifdef ENABLE_AUDIO_PROCESSOR
static void
AUD_ResetAudioProcessor(
    void)
{
    MMP_UINT32 i;
    MMP_UINT16 reg   = 0;
    MMP_UINT32 clock = PalGetClock();
    LOG_ENTER "AUD_ResetAudioProcessor()\n" LOG_END

    // 0x44 bit14, Reset the RISC1, 0: Normal Operation, 1: Reset RISC1
        #if !defined(__FREERTOS__) // WIN32

        #else

    // force halt the second cpu.
    HOST_WriteRegister(DrvDecode_WrPtr, 0xFFFF);
    while (1)
    {
        HOST_ReadRegister(DrvAudioCtrl2, &reg);

        if ((reg & DrvAudio_RESET)
            || PalGetDuration(clock) > 66)
        {
            if (PalGetDuration(clock) > 66)
            {
                //sdk_msg(SDK_MSG_TYPE_ERROR, "%s(%d), duration: %u\n", __FILE__, __LINE__, PalGetDuration(clock));
            }
            break;
        }

        for (i = 0; i < 64; i++)
            asm ("");
    }
    for (i = 0; i < 64; i++)
        asm ("");

    HOST_WriteRegister(DrvDecode_WrPtr, 0x0);
    HOST_WriteRegisterMask(DrvAudioCtrl2, (0 << DrvAudio_Reset_Bits), (1 << DrvAudio_Reset_Bits));
    PalSleep(1);
    // Halt CPU2 wishbone request
    HOST_WriteRegisterMask(0x16f4, (1 << 3) | (1 << 4), (1 << 3) | (1 << 4));
    for (i = 0; i < 4096; i++)
        asm ("");
    // CPU2 Stall
    HOST_WriteRegisterMask(0x16ca, (1 << 1) | (0 << 0), (1 << 1) | (1 << 0));
    for (i = 0; i < 4096; i++)
        asm ("");
    // Reset CPU
    {
        HOST_WriteRegisterMask(0x0044, 1 << 14, 1 << 14);
        PalSleep(1);
        //for (i = 0; i < 2048; i++) asm("");
        HOST_WriteRegisterMask(0x0044, 0 << 14, 1 << 14);
    }
    // Restart CPU2 wishbone request
    HOST_WriteRegisterMask(0x16f4, (0 << 3) | (0 << 4), (1 << 3) | (1 << 4));
        #endif
}

static void
AUD_FireAudioProcessor(
    void)
{
    MMP_UINT16 nRegData;

    LOG_ENTER "AUD_FireAudioProcessor()\n" LOG_END

        #if !defined(__FREERTOS__) // WIN32
    HOST_WriteRegister(GENEARL_BASE + MISC_RISC1_REG, 0x1);
        #else
    HOST_ReadRegister(0x16ca, &nRegData);

    // Unstall and Fire CPU
    {
        int i;
        HOST_WriteRegisterMask(0x16ca, ((0 << 1) | (1 << 0)), ((1 << 1) | (1 << 0)));
        for (i = 0; i < 10; i++)
            asm ("");
        HOST_WriteRegisterMask(0x16ca, (0 << 0), (1 << 0));
    }
    HOST_WriteRegister(AUDIO_DECODER_START_FALG, 0);

        #endif
}

    #endif
MMP_RESULT
AUD_ChangePll(
    MMP_UINT16 target_numerator)
{
    MMP_UINT16 current_numerator;
    LOG_ENTER "AUD_ChangePll()\n" LOG_END

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_GetEngineStatus(
    MMP_UINT *status)
{
    MMP_UINT i = 0;
    LOG_ENTER "AUD_GetEngineStatus()\n" LOG_END

    #if !defined(__FREERTOS__)
    * status = MMP_AUDIO_ENGINE_IDLE;
    return MMP_RESULT_SUCCESS;
    #else
    *status  = MMP_AUDIO_ENGINE_RUNNING;
    return MMP_RESULT_SUCCESS;
    #endif
}

// stream
MMP_RESULT
AUD_GetWriteAvailableBufferLength(
    MMP_UINT *wrBuflen)
{
    MMP_UINT16 hwpr;
    MMP_UINT16 swpr;

    PRECONDITION(Audio_Decode_Buflen >= 4);
    PRECONDITION(Audio_Decode_Buflen % 2 == 0); // word align
    //LOG_ENTER "AUD_GetWriteAvailableBufferLength()\n" LOG_END
    swpr = Audio_Decode_Wrptr;

    HOST_ReadRegister(DrvDecode_RdPtr, &hwpr);

    hwpr = ((hwpr >> 1) << 1); // word align

    // Get the avalible length of buffer
    if (swpr == hwpr)
    {
        (*wrBuflen) = Audio_Decode_Buflen - 2;
    }
    else if (swpr > hwpr)
    {
        (*wrBuflen) = Audio_Decode_Buflen - (swpr - hwpr) - 2;
    }
    else
    {
        (*wrBuflen) = hwpr - swpr - 2;
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_GetReadAvailableBufferLength(
    MMP_UINT *rdBuflen)
{
    MMP_UINT16 hwpr;
    MMP_UINT16 swpr;

    PRECONDITION(Audio_Encode_Buflen >= 4);
    PRECONDITION(Audio_Encode_Buflen % 2 == 0); // word align

    //LOG_ENTER "AUD_GetReadAvailableBufferLength()\n" LOG_END

    swpr = Audio_Encode_Rdptr;

    HOST_ReadRegister(DrvEncode_WrPtr, &hwpr);
    hwpr = ((hwpr >> 1) << 1); //word align

    if (swpr == hwpr)
    {
        (*rdBuflen) = 0;
    }
    else if (swpr > hwpr)
    {
        (*rdBuflen) = Audio_Encode_Buflen - (swpr - hwpr);
    }
    else
    {
        (*rdBuflen) = hwpr - swpr;
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_SetOutEOF(
    void)
{
    MMP_UINT16 regData = 0;
    #if defined(ENABLE_SXA_MPS) && (ENABLE_SXA_MPS)
    MMP_UINT16 timeout = 3000;
    #else
    MMP_UINT16 timeout = 0;
    #endif //defined(ENABLE_SXA_MPS) && (ENABLE_SXA_MPS)

    LOG_ENTER "AUD_SetOutEOF()\n" LOG_END

    HOST_WriteRegisterMask(DrvAudioCtrl, 0xffff, DrvDecode_EOF);
    do
    {
        HOST_ReadRegister(DrvAudioCtrl, &regData);
        if (!(regData & DrvDecode_EOF))
        {
            return MMP_RESULT_SUCCESS;
        }
        if ((regData & DrvDecode_STOP))
        {
            return MMP_RESULT_SUCCESS;
        }
    #if defined(__FREERTOS__)
        MMP_Sleep(1);
    } while (1);
    #else
        if (AUD_GetEngineStatus(&AUD_EngineState) == MMP_RESULT_SUCCESS)
        {
            HOST_ReadRegister(DrvDecode_RdPtr, &regData);
            timeout = (Audio_Decode_Wrptr > regData) ? (Audio_Decode_Wrptr - regData) : (regData - Audio_Decode_Wrptr);
            if (timeout == 0)
            {
                return MMP_RESULT_SUCCESS;
            }
        }
        else
        {
            timeout = 0;
            goto ERROR_EOF_RESET_ENGINE;
        }
    } while (timeout);
    #endif

ERROR_EOF_RESET_ENGINE:
    LOG_ERROR "Can not correctly set end of stream, reset engine\n" LOG_END
    AUD_ResetEngine();
    AUD_ConfigEngine();
    AUD_SetRISCIIS();
    AUD_FireEngine();
    return MMP_RESULT_ERROR;
}

// engine
MMP_RESULT
AUD_PowerOnRisc(
    void)
{
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_PowerOffRisc(
    void)
{
    LOG_ENTER "AUD_PowerOffRisc()\n" LOG_END
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_SetRISCIIS(
    void)
{
    MMP_UINT16 regData = 0;

    LOG_ENTER "AUD_SetRISCIIS()\n" LOG_END

    // Set RISC I2S register
    // 1. Set master/slave mode
    //if (!AUD_Codec.codecInit)
    //    return AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE;

    //select internal DAC mode
    //HOST_WriteRegister(0x1640, (MMP_UINT16)0x201f);

    // Select PLL3, internal DAC, DAC Ration 0x1f
    //HOST_WriteRegister(0x1642, (MMP_UINT16)0xE00F);

    //HOST_WriteRegister(0x1644, (MMP_UINT16)0x3000);

    //ADC-DAC Sync
    //HOST_WriteRegisterMask(0x1640, (MMP_UINT16)0x8000,0x8000);
    //MMP_Sleep(5);
    //HOST_WriteRegisterMask(0x1640, (MMP_UINT16)0x0,0x8000);
    //MMP_Sleep(5);

    // 3. Set oversampling rate
    /*
        TODO:
     */
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_InitialCodec(
    void)
{
    LOG_ENTER "AUD_InitialCodec()\n" LOG_END

    // Use script to initilize CODEC (script writen by FAE)
    if (AUD_Codec.codecInit != MMP_TRUE)
    {
        if (AUD_Codec.CODEC_Initialize != NULL)
        {
            AUD_Codec.CODEC_Initialize();
            AUD_Codec.codecInit = MMP_TRUE;
        }
        else
        {
            LOG_ERROR "Codec cannot be initialized\n" LOG_END
        }
    }

    return AUD_SetRISCIIS();
}

MMP_RESULT
AUD_TerminateCodec(
    void)
{
    LOG_ENTER "AUD_TerminateCodec()\n" LOG_END

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    #if !defined(__FREERTOS__)
    /* S034 TODO
        Wolfson implementation needed!
     */
    AUD_Codec.CODEC_Terminate();
    #elif !defined(ENABLE_CODECS_PLUGIN) && !defined(ENABLE_AUDIO_PROCESSOR) // defined(__FREERTOS__)
    // Terminate the audio task
        #if defined(HAVE_MP3)
    vTaskSuspend(g_task_mp3_dec);
        #endif
    #elif  !defined(ENABLE_AUDIO_PROCESSOR)// ENABLE_CODECS_PLUGIN
    vTaskSuspend(g_task_codec);
    #endif // !defined(__FREERTOS__)

    #if defined(OUTPUT_TO_FILE)
    if (fout)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] PalFileClose\n");
        PalFileClose(fout, MMP_NULL);
        fout = NULL;
    }
    #endif

    return MMP_RESULT_SUCCESS;
}

MMP_UINT8 *
audioReadCodec(MMP_AUDIO_ENGINE audio_type, unsigned int *pImagesize)
{
    unsigned int nImagesize, nResult = 0;
    MMP_UINT8 *pAddress = MMP_NULL;
    if (audio_type == MMP_MP2_ENCODE)
    {
        pAddress    = gMp2Encoder;
        *pImagesize = (unsigned int)sizeof(gMp2Encoder);
        //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] open mp2 encode engine address 0x%x nSize %d #line %d \n",pAddress,pImagesize,__LINE__);
    }
    else if (audio_type == MMP_AAC_ENCODE)
    {
        pAddress    = gAACEncoder;
        *pImagesize = (unsigned int)sizeof(gAACEncoder);
        //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] open aac encode engine address 0x%x nSize %d #line %d \n",pAddress,pImagesize,__LINE__);
    }
    return pAddress;
}

MMP_RESULT
AUD_LoadEngine(
    MMP_AUDIO_ENGINE audio_type, MMP_UINT8 *pAddress, MMP_UINT32 length)
{
    MMP_UINT8 *pTemp;
    MMP_UINT16 nTemp, nCount;
    MMP_BOOL bMPlayer;
    LOG_ENTER "AUD_LoadEngine()\n" LOG_END
    #if !defined(__FREERTOS__)
    PRECONDITION(AUD_baseAddress != 0);
    #endif

    #if defined(ENABLE_AUDIO_PROCESSOR)
    set_audio_processor_timer(1);
    #else
    set_audio_processor_timer(0);
    #endif

    #if !defined(__FREERTOS__)
    if (AUD_GetEngineStatus(&AUD_EngineState) == MMP_RESULT_SUCCESS)
    {
        if (AUD_EngineState == MMP_AUDIO_ENGINE_RUNNING)
        {
            if (AUD_EngineType == audio_type)
                return MMP_RESULT_SUCCESS;
            else
                return MMP_RESULT_ERROR;
        }
    }
    #endif

    if (AUD_EngineType == audio_type)
    {
        gReLoadEngine = 0;
        HOST_WriteRegister(AUDIO_DECODER_START_FALG, 1);

        //goto RESET_ENGINE;
    }
    else
    {
        gReLoadEngine = 1;
    }

    if (gReLoadEngine)
    {
        AUD_SKIP           = MMP_FALSE;
        Audio_Encode_Wrptr = 0;
        Audio_Encode_Rdptr = 0;
        Audio_Decode_Wrptr = 0;
        Audio_Decode_Rdptr = 0;
        AUD_freqAddress    = 0;
        AUD_ResetEngine();
    }
    #if defined(ENABLE_CODECS_PLUGIN) && !defined(ENABLE_CODECS_ARRAY)
    // Load Plug-Ins to memory
    if (AUD_PlugInType != audio_type || gReLoadEngine == 1)
    {
        signed portBASE_TYPE ret = pdFAIL;
        static PAL_FILE *pFile   = NULL;
        #if !defined(ENABLE_AUDIO_PROCESSOR)
        // Audio task should suspend normally.
        // Suspend again to prevent abnormal audio terminate.
        if (g_task_codec)
        {
            vTaskSuspend(g_task_codec);
        }
        #else
        AUD_ResetAudioProcessor();  // Stop CPU
        #endif

        if (!pAddress || length <= 0)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] Load Engine load address error %d %d #line %d \n", pAddress, length, __LINE__);
            AUD_SKIP = MMP_TRUE;
            return MMP_RESULT_ERROR;
        }
        //PalWcscpy(codec_file, codec_path);
        switch (audio_type)
        {
        case MMP_MP3_DECODE:
        #if HAVE_MP3

        #else
            AUD_SKIP = MMP_TRUE;
        #endif
            break;

        case MMP_MP2_ENCODE:
        case MMP_AAC_ENCODE:

            break;

        default:
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] Load Engine unknow_engine_type %d #line %d \n", audio_type, __LINE__);
            return MMP_RESULT_ERROR;
            break;
        }

        if (AUD_SKIP)
            return MMP_RESULT_SUCCESS;

        //if (1)
        //{
        //    MMP_WCHAR *ptr;

        //sdk_msg(SDK_MSG_TYPE_INFO,"Open plug-ins: %d",length);
        //for(ptr = codec_file; *ptr != L'\0'; ptr++)
        //sdk_msg(SDK_MSG_TYPE_INFO,"%c", (char)*ptr);
        //sdk_msg(SDK_MSG_TYPE_INFO,"\n");
        //}
        //length = PalTGetFileLength( codec_file );
        //sdk_msg(SDK_MSG_TYPE_INFO,"0x%x 0x%x 0x%x 0x%x \n",pAddress,pAddress[0],pAddress[1],pAddress[2] );

        //if (pFile)
        //    PalFileClose(pFile, MMP_NULL);
        //pFile = PalTFileOpen( codec_file, PAL_FILE_RB, MMP_NULL);
        //if (pFile == NULL)
        //{
        //    LOG_ERROR "Can not open CODEC plugin\n" LOG_END
        //    AUD_SKIP = MMP_TRUE;
        //    return MMP_RESULT_ERROR;
        //}

        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] Load Engine codec_start_addr 0x%x 0x%x length %d \n", &codec_start_addr, &codec_end_addr, (int)&codec_end_addr - (int)&codec_start_addr);

        if (length >= (int)&codec_end_addr - (int)&codec_start_addr)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] Not enougth memory to put CODEC plugin length %d #line %d  \n", length, __LINE__);
            AUD_SKIP = MMP_TRUE;
            return MMP_RESULT_ERROR;
        }
        //if (pFile)
        {
            //if (length != PalFileRead((void*)&codec_start_addr, sizeof(char), length, pFile, MMP_NULL))
            //{
            //    LOG_ERROR "Can not read CODEC plugin\n" LOG_END
            //    AUD_SKIP = MMP_TRUE;
            //    return MMP_RESULT_ERROR;
            //}
            //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver]codec_start_addr 0x%x __header 0x%x __header 0x%x\n",(int)&codec_start_addr,__header,(int)&codec_end_addr);
            pTemp = (MMP_UINT8 *)(int)&codec_start_addr;
            PalMemcpy((MMP_UINT8 *)pTemp, (MMP_UINT8 *)pAddress, length);
            if (__header->magic != CODEC_MAGIC ||
                __header->target_id != TARGET_ID ||
                __header->api_version < CODEC_API_VERSION ||
                (int)__header->load_addr != (int)&codec_start_addr ||
                (int)__header->end_addr > (int)&codec_end_addr)
            {
                sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] CODEC header error pTemp 0x%x 0x%x #line %d \n", pTemp, pAddress, __LINE__);
                sdk_msg(SDK_MSG_TYPE_INFO, " __header->magic       0x%08x, expect value = 0x%08x\n", __header->magic, CODEC_MAGIC);
                sdk_msg(SDK_MSG_TYPE_INFO, " __header->target_id   0x%04x, expect value = 0x%04x\n", __header->target_id, TARGET_ID);
                sdk_msg(SDK_MSG_TYPE_INFO, " __header->api_version 0x%04x, expect value < 0x%04x\n", __header->api_version, CODEC_API_VERSION);
                sdk_msg(SDK_MSG_TYPE_INFO, " __header->load_addr   0x%08x, expect value = 0x%08x\n", __header->load_addr, &codec_start_addr);
                sdk_msg(SDK_MSG_TYPE_INFO, " __header->end_addr    0x%08x, expect value < 0x%08x\n", __header->end_addr, &codec_end_addr);
                AUD_SKIP = MMP_TRUE;
                return MMP_RESULT_ERROR;
            }
        }
        //PalFileClose(pFile, MMP_NULL);
        //pFile = NULL;
        //LOG_DEBUG "Load audio plugin complete.\n" LOG_END
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver]Load audio plugin complete \n");
        // Flush Instruction Cacahe
        //ic_invalidate();
        // Restart Task
        #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_FireAudioProcessor();
        ret = 1;
        #else
        ret = xTaskRestart(g_task_codec, (pdTASK_CODE)__header->entry_point,
                           (void *)&codec_api);
        #endif
        if (pdFAIL == ret)
        {
            LOG_ERROR "Failed to create audio task\n" LOG_END
            AUD_SKIP = MMP_TRUE;
            return MMP_RESULT_ERROR;
        }
        AUD_PlugInType = audio_type;
    }
    else if (gReLoadEngine == 1)
    {
        // Resume the task that stores in the memory
        #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();
        #else
        vTaskResume(g_task_codec);
        #endif
    }
    #elif defined(ENABLE_CODECS_PLUGIN) && defined(ENABLE_CODECS_ARRAY)
    switch (audio_type)
    {
    case MMP_MP3_DECODE:
        #ifndef HAVE_MP3
        AUD_SKIP = MMP_TRUE;
        #endif
        break;

    case MMP_MP2_ENCODE:
    case MMP_AAC_ENCODE:
        break;

    default:
        return MMP_RESULT_ERROR;
        break;
    }

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    // Load Plug-Ins to memory
    if (AUD_PlugInType == 0 && audio_type == MMP_MP3_DECODE)
    {
        // The MP3 CODEC had already in memory on initial state.
        if (__header->magic != CODEC_MAGIC ||
            __header->target_id != TARGET_ID ||
            __header->api_version < CODEC_API_VERSION ||
            (int)__header->load_addr != (int)&codec_start_addr ||
            (int)__header->end_addr > (int)&codec_end_addr)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] CODEC header error\n");
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->magic       0x%08x, expect value = 0x%08x\n", __header->magic, CODEC_MAGIC);
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->target_id   0x%04x, expect value = 0x%04x\n", __header->target_id, TARGET_ID);
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->api_version 0x%04x, expect value < 0x%04x\n", __header->api_version, CODEC_API_VERSION);
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->load_addr   0x%08x, expect value = 0x%08x\n", __header->load_addr, codec_start_addr);
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->end_addr    0x%08x, expect value < 0x%08x\n", __header->end_addr, codec_end_addr);
            AUD_SKIP = MMP_TRUE;
            return MMP_RESULT_ERROR;
        }
        AUD_PlugInType = audio_type;
        #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();
        #else
        vTaskResume(g_task_codec);
        #endif
    }
    else if (AUD_PlugInType != audio_type)
    {
        signed portBASE_TYPE ret = pdFAIL;

        if (codecptr[audio_type] != (int *)codecbuf)
        {
            int i, tmp;
            int *src, *dst;

        #if !defined(ENABLE_AUDIO_PROCESSOR)
            // Audio task should suspend normally.
            // Suspend again to prevent abnormal audio terminate.
            if (g_task_codec)
            {
                vTaskSuspend(g_task_codec);
            }
        #else
            AUD_ResetAudioProcessor();  // Stop CPU
        #endif

            // SWAP the CODEC
            dst = (int *)codecbuf;
            src = codecptr[audio_type];
            for (i = 0; i < CODEC_ARRAY_SIZE / sizeof(int); i++)
            {
                tmp    = dst[i];
                dst[i] = src[i];
                src[i] = tmp;
            }

            // swap the CODEC pointer
            tmp                      = (int)codecptr[audio_type];
            codecptr[audio_type]     = codecptr[AUD_PlugInType];
            codecptr[AUD_PlugInType] = (int *)tmp;

            // Flush Instruction Cache
            ic_invalidate();
        }

        if (__header->magic != CODEC_MAGIC ||
            __header->target_id != TARGET_ID ||
            __header->api_version < CODEC_API_VERSION ||
            (int)__header->load_addr != (int)&codec_start_addr ||
            (int)__header->end_addr > (int)&codec_end_addr)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "CODEC header error\n");
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->magic       0x%08x, expect value = 0x%08x\n");
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->target_id   0x%04x, expect value = 0x%04x\n");
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->api_version 0x%04x, expect value < 0x%04x\n");
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->load_addr   0x%08x, expect value = 0x%08x\n");
            sdk_msg(SDK_MSG_TYPE_INFO, " __header->end_addr    0x%08x, expect value < 0x%08x\n");
            AUD_SKIP = MMP_TRUE;
            return MMP_RESULT_ERROR;
        }

        // Restart Task
        #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_FireAudioProcessor();
        ret = 1;
        #else
        ret = xTaskRestart(g_task_codec, (pdTASK_CODE)__header->entry_point,
                           (void *)&codec_api);
        #endif
        if (pdFAIL == ret)
        {
            LOG_ERROR "Failed to create audio task\n" LOG_END
            AUD_SKIP = MMP_TRUE;
            return MMP_RESULT_ERROR;
        }
        AUD_PlugInType = audio_type;
    }
    else
    {
        // Resume the task that stores in the memory
        #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();
        #else
        vTaskResume(g_task_codec);
        #endif
    }
    #endif // !defined(ENABLE_CODECS_PLUGIN)

    write_waveHeader = MMP_FALSE;
    wave_ADPCM       = 0;

    #if !defined(ENABLE_CODECS_PLUGIN) && defined(__FREERTOS__)
    //sdk_msg(SDK_MSG_TYPE_INFO," ! ENABLE_CODECS_PLUGIN \n");
    if (AUD_EngineType != audio_type)
    {
        switch (AUD_EngineType)
        {
        #if defined(HAVE_MP3)
        case MMP_MP3_DECODE:
            vTaskSuspend(g_task_mp3_dec);
            break;
        #endif

        default:
            break;
        }
    }
    #endif

    #if  defined(__FREERTOS__)
    //nTemp = 0;
    //do {
    //    HOST_ReadRegister(AUDIO_DECODER_START_FALG, &nTemp);
    //} while (nTemp==0);
    nTemp  = 0;
    nCount = 0;
    do
    {
        MMP_UINT32 i = 0;
        HOST_ReadRegister(AUDIO_DECODER_START_FALG, &nTemp);
        for (i = 0; i < 1024; i++)
            asm ("");
        //PalSleep(1);
        nCount++;
    } while (nTemp == 0 && nCount <= 2000);
    if (nCount > 2000)
    {
        sdk_msg(SDK_MSG_TYPE_ERROR, "[Audio Driver] wait decoder start fail #line %d \n", __LINE__);
    }
    #endif

    switch (audio_type)
    {
    case MMP_MP3_DECODE:
    #if HAVE_MP3
        #if !defined(__FREERTOS__)
        HOST_WriteBlockMemory(BSPPOSITION, (MMP_UINT)MP3BSP,  sizeof(MP3BSP));
        HOST_WriteBlockMemory(PROPOSITION, (MMP_UINT)MP3PROG, sizeof(MP3PROG));
        Audio_Decode_Bufptr = MP3BUFPTR;
        Audio_Decode_Buflen = MP3BUFLEN;
        AUD_freqAddress     = AUD_baseAddress + FreqInfoBase;
        #elif !defined(ENABLE_CODECS_PLUGIN)
        MP3_GetBufInfo(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress, &AUD_curPlayPos);
        //AUD_freqAddress =  AUD_baseAddress + FreqInfoBase;
        vTaskResume(g_task_mp3_dec);
        #else

            #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress, &AUD_curPlayPos, &Audio_Plugin_Message_Buf, &Audio_Plugin_Message_Buflen);
            #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress, &AUD_curPlayPos);
            #endif

        #endif
    #else
        AUD_SKIP = MMP_TRUE;
    #endif
        break;

    case MMP_MP2_ENCODE:
    #if !defined(ENABLE_CODECS_PLUGIN)
        MP2Encode_GetBufInfo(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen);
        //AUD_freqAddress =  AUD_baseAddress + FreqInfoBase;
        vTaskResume(g_task_mp3_dec);
    #else

        #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen, &Audio_Plugin_Message_Buf, &Audio_Plugin_Message_Buflen);
        #else
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] mp2 audio plugin dec buf  %d, enc buf %d, 0x%x \n", Audio_Decode_Buflen, Audio_Encode_Buflen, pgAudioEncode);
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen, &gAudioEncodePointer);
        pgAudioEncode = (AUDIO_ENCODE_PARAM *)gAudioEncodePointer;
        memcpy(pgAudioEncode, &gADEncode, sizeof(gADEncode));
        if (gADEncode.nInputBufferType == 1)
        {
            Audio_Decode_Bufptr = (MMP_UINT)gADEncode.pInputBuffer;
            Audio_Decode_Buflen = gADEncode.nBufferLength;
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] mp2 input type 1 Audio_Decode_Bufptr 0x%x length %d  \n", Audio_Decode_Bufptr, Audio_Decode_Buflen);
        }
        else if (gADEncode.nInputBufferType == 2)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "set i2s start porinter 0x%x \n", gADEncode.nStartPointer);
            I2S_AD32_SET_RP(gADEncode.nStartPointer);
        }

        //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] mp2 audio plugin dec buf  %d, enc buf %d, 0x%x \n",Audio_Decode_Buflen,Audio_Encode_Buflen,pgAudioEncode);
        #endif
    #endif
        break;

    case MMP_AAC_ENCODE:
    #if !defined(ENABLE_CODECS_PLUGIN)
        MP2Encode_GetBufInfo(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen);
        //AUD_freqAddress =  AUD_baseAddress + FreqInfoBase;
        vTaskResume(g_task_mp3_dec);
    #else

        #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen, &Audio_Plugin_Message_Buf, &Audio_Plugin_Message_Buflen);
        #else
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] aac audio plugin dec buf  %d, enc buf %d, 0x%x \n", Audio_Decode_Buflen, Audio_Encode_Buflen, pgAudioEncode);
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen, &gAudioEncodePointer);
        pgAudioEncode = (AUDIO_ENCODE_PARAM *)gAudioEncodePointer;
        memcpy(pgAudioEncode, &gADEncode, sizeof(gADEncode));
        if (gADEncode.nInputBufferType == 1)
        {
            Audio_Decode_Bufptr = (MMP_UINT)gADEncode.pInputBuffer;
            Audio_Decode_Buflen = gADEncode.nBufferLength;
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] aac input type 1 Audio_Decode_Bufptr 0x%x length %d  \n", Audio_Decode_Bufptr, Audio_Decode_Buflen);
        }
        else if (gADEncode.nInputBufferType == 2)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "set i2s start porinter 0x%x \n", gADEncode.nStartPointer);
            I2S_AD32_SET_RP(gADEncode.nStartPointer);
        }

        //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] aac audio plugin dec buf  %d, enc buf %d, 0x%x \n",Audio_Decode_Buflen,Audio_Encode_Buflen,pgAudioEncode);
        #endif
    #endif
        break;

    default:
        return MMP_RESULT_ERROR;
        break;
    }

    gEncodeTimeStamp = gEncodeStartTime;
    sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] set up , audio plugin buffer-length %d 0x%x time offset %d \n", Audio_Decode_Buflen, Audio_Decode_Bufptr, gEncodeTimeStamp);
    mmpAudioGetAttrib(MMP_AUDIO_ENABLE_MPLAYER, &bMPlayer);

    sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] write start flag 0x%x 2 \n", AUDIO_DECODER_START_FALG);
    HOST_WriteRegister(AUDIO_DECODER_START_FALG, 2);

    Audio_DefDecode_Bufptr = Audio_Decode_Bufptr;
    Audio_DefEncode_Bufptr = Audio_Encode_Bufptr;
    AUD_EngineType         = audio_type;

    goto END;

RESET_ENGINE:
    AUD_ResetEngine();

END:
    AUD_ConfigEngine();

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    //sdk_msg(SDK_MSG_TYPE_ERROR, "audio.c(%d), [Audio Driver] audio load engine success \n", __LINE__);
    return AUD_InitialCodec();
}

MMP_RESULT
AUD_ConfigEngine(
    void)
{
    LOG_ENTER "AUD_ConfigEngine()\n" LOG_END

    switch (AUD_EngineType)
    {
    #if defined(__FREERTOS__)
    case MMP_MP2_ENCODE:
    case MMP_AAC_ENCODE:
        break;
    default:
        SYS_PANIC();
        break;
    #endif
    }
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_FireEngine(
    void)
{
    #if !defined(__FREERTOS__)
    MMP_UINT16 regdata = DrvEnable;
    MMP_UINT16 cnt     = 0;
    #endif

    LOG_ENTER "AUD_FireEngine()\n" LOG_END

    #if !defined(__FREERTOS__)
    // 0. if engine is running, stop fire engine
    if (AUD_GetEngineStatus(&AUD_EngineState) == MMP_RESULT_SUCCESS)
    {
        switch (AUD_EngineState)
        {
        case MMP_AUDIO_ENGINE_RUNNING:
            return MMP_RESULT_ERROR;
        case MMP_AUDIO_ENGINE_IDLE:
            return MMP_RESULT_SUCCESS;
        default:
            break;
        }
    }

    // 1. write enable bit
    HOST_WriteRegisterMask(DrvAudioCtrl, 0xffff, DrvEnable);

    // 2. fire engine
    HOST_WriteRegister(RISC_FIRE, 0x1);
    HOST_WriteRegister(RISC_FIRE, 0x0);
    // 3. wait for HW clean the enable bit
    do
    {
        HOST_ReadRegister (DrvAudioCtrl, &regdata);
        regdata &= DrvEnable;
        if (regdata == 0)
        {
            return MMP_RESULT_SUCCESS;
        }
        cnt++;
        MMP_Sleep(5);
    } while (cnt < 400);

    return MMP_RESULT_ERROR;
    #else

    return MMP_RESULT_SUCCESS;
    #endif
}

MMP_RESULT
AUD_ResetEngine(
    void)
{
    LOG_ENTER "AUD_ResetEngine()\n" LOG_END

    #if !defined(__FREERTOS__)
    // stop engine process

    // enter CPU debugging mode

    //Reset AHB

    #endif
    /*
       #ifdef ENABLE_AUDIO_PROCESSOR
        sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] AUD_ResetEngine \n");

        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();

       #endif
     */
    //MUTE

    //DAC control reset
    //HOST_WriteRegister(0x1658,0);
    // MMP_Sleep(1);
    //DAC write point reset
    //HOST_WriteRegister(0x1656,0);
    // MMP_Sleep(1);

    // reset engine buffer address to default.
    Audio_Encode_Bufptr = Audio_DefEncode_Bufptr;
    Audio_Decode_Bufptr = Audio_DefDecode_Bufptr;
    Audio_Encode_Wrptr  = 0;
    Audio_Encode_Rdptr  = 0;
    Audio_Decode_Wrptr  = 0;
    Audio_Decode_Rdptr  = 0;

    // Reset the pause control
    AUD_ResetControl();

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_StopRISC(
    void)
{
    LOG_ENTER "AUD_StopRISC()\n" LOG_END

    AUD_ResetEngine();
    //AUD_PowerOffRisc();

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
AUD_ResetControl(
    void)
{
    LOG_ENTER "AUD_ResetControl()\n" LOG_END

    #if defined(__FREERTOS__)
    // Un-Pause
    HOST_WriteRegisterMask(DrvAudioCtrl, 0, DrvDecode_PAUSE);
    #endif

    return MMP_RESULT_SUCCESS;
}

/*
 *  reset AudioCtrl2
 *
 */
MMP_RESULT AUD_ResetUserDefineIO3(
    void)
{
    #if defined(__FREERTOS__)
    HOST_WriteRegister(DrvAudioCtrl2,  0);
    #endif
    return MMP_TRUE;
}

/*
 *  reset AudioEngineType
 *
 */
MMP_RESULT AUD_ResetAudioEngineType(
    void)
{
    AUD_EngineType = MMP_RESERVED;
    return MMP_TRUE;
}

void AUD_OccupyReadBuffer()
{
    unsigned int nWriteProtect = 0;

    do
    {
        nWriteProtect = getAudioReadBufferStatus();
        PalSleep(1);
    } while (nWriteProtect == 1);
    occupyAudioReadBuffer();
}

void AUD_ReleaseReadBuffer()
{
    releaseAudioReadBuffer();
}

void AUD_OccupyWriteBuffer()
{
    unsigned int nWriteProtect = 0;
    do
    {
        nWriteProtect = getAudioWriteBufferStatus();
        PalSleep(1);
    } while (nWriteProtect == 1);

    occupyAudioWriteBuffer();
}

void AUD_ReleaseWriteBuffer()
{
    //sdk_msg(SDK_MSG_TYPE_INFO,"AUD_ReleaseWriteBuffer %d \n", getAudioWriteBufferStatus());

    releaseAudioWriteBuffer();
}

int AUD_CheckEncodeParam(AUDIO_ENCODE_PARAM tAudioEncode, MMP_AUDIO_ENGINE audio_type)
{
    int i;
    // check channel
    if (tAudioEncode.nChannels > 8 || tAudioEncode.nChannels == 0)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport channels %d #line %d \n", tAudioEncode.nChannels, __LINE__);
        return MMP_RESULT_ERROR;
    }
    // check audio input type
    if (tAudioEncode.nInputAudioType != AUDIO_INPUT_TYPE_PCM)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport audio input type %d #line %d \n", tAudioEncode.nInputAudioType, __LINE__);
        return MMP_RESULT_ERROR;
    }
    if ( (tAudioEncode.nInputAudioType == AUDIO_INPUT_TYPE_PCM) && (tAudioEncode.nSampleSize != 16 && tAudioEncode.nSampleSize != 32))
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport audio input sample size %d #line %d \n", tAudioEncode.nSampleSize, __LINE__);
        return MMP_RESULT_ERROR;
    }
    // check sample rate
    if (tAudioEncode.nSampleRate != 48000 && tAudioEncode.nSampleRate != 44100 && tAudioEncode.nSampleRate != 32000)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport nSampleRate %d #line %d \n", tAudioEncode.nSampleRate, __LINE__);
        return MMP_RESULT_ERROR;
    }
    if (tAudioEncode.nOutSampleRate == 0)
    {
        tAudioEncode.nOutSampleRate = tAudioEncode.nSampleRate;
    }

    if (tAudioEncode.nOutSampleRate != tAudioEncode.nSampleRate && tAudioEncode.nOutSampleRate != 48000)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport nSampleRate %d #line %d \n", tAudioEncode.nSampleRate, __LINE__);
        return MMP_RESULT_ERROR;
    }
    // check audio input buffer type
    if (tAudioEncode.nInputBufferType > 2 || tAudioEncode.nInputBufferType < 0)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport nInputBufferType %d #line %d \n", tAudioEncode.nInputBufferType, __LINE__);
        return MMP_RESULT_ERROR;
    }
    if (tAudioEncode.nInputBufferType == 1 && tAudioEncode.nBufferLength == 0)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unsupport nBufferLength %d #line %d \n", tAudioEncode.nBufferLength, __LINE__);
        return MMP_RESULT_ERROR;
    }

    // check bitrate
    if (audio_type == MMP_MP2_ENCODE)
    {
        for (i = 1; i < 14; i++)
        {
            if (tAudioEncode.nBitrate == gEncodeMPABitrate[i])
            {
                break;
            }
        }
        if (tAudioEncode.nBitrate <= gEncodeMPABitrate[0])
        {
            tAudioEncode.nBitrate = AUDIO_ENCODE_DEFAULT_BITRATE;
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam bitrate==0, set to %d #line %d \n", AUDIO_ENCODE_DEFAULT_BITRATE, __LINE__);
        }
        else if (tAudioEncode.nBitrate > gEncodeMPABitrate[13])
        {
            tAudioEncode.nBitrate = AUDIO_ENCODE_DEFAULT_BITRATE;
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam bitrate>=%d, set to %d #line %d \n", gEncodeMPABitrate[13], AUDIO_ENCODE_DEFAULT_BITRATE, __LINE__);
        }
        else if (i >= 14)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unknown bitrate %d, set to %d #line %d \n", tAudioEncode.nBitrate, AUDIO_ENCODE_DEFAULT_BITRATE, __LINE__);
            tAudioEncode.nBitrate = AUDIO_ENCODE_DEFAULT_BITRATE;
        }
    }
    else if (audio_type == MMP_AAC_ENCODE)
    {
        for (i = 1; i < 11; i++)
        {
            if (tAudioEncode.nBitrate == gEncodeAACBitrate[i])
            {
                break;
            }
        }
        if (tAudioEncode.nBitrate <= gEncodeAACBitrate[0])
        {
            tAudioEncode.nBitrate = AUDIO_ENCODE_DEFAULT_BITRATE;
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam bitrate==0, set to %d #line %d \n", AUDIO_ENCODE_DEFAULT_BITRATE, __LINE__);
        }
        else if (tAudioEncode.nBitrate > gEncodeAACBitrate[10])
        {
            tAudioEncode.nBitrate = AUDIO_ENCODE_DEFAULT_BITRATE;
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam bitrate>=%d, set to %d #line %d \n", gEncodeAACBitrate[10], AUDIO_ENCODE_DEFAULT_BITRATE, __LINE__);
        }
        else if (i >= 11)
        {
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] AUD_CheckEncodeParam unknown bitrate %d, set to %d #line %d \n", tAudioEncode.nBitrate, AUDIO_ENCODE_DEFAULT_BITRATE, __LINE__);
            tAudioEncode.nBitrate = AUDIO_ENCODE_DEFAULT_BITRATE;
        }
    }

    memcpy(&gADEncode, &tAudioEncode, sizeof(gADEncode));
    sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] encode parameter buffer type %d ch %d sample rate %d sample size %d bitrate %d \n", gADEncode.nInputBufferType, gADEncode.nChannels, gADEncode.nSampleRate, gADEncode.nSampleSize, gADEncode.nBitrate);

    return 0;
}

MMP_RESULT
mmpAudioInitializeTask(
    void)
{
    #if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

        #if !defined(ENABLE_CODECS_PLUGIN)

            #if defined(HAVE_MP3)
    ret = xTaskCreate(mp3decode_task, "mp3_decode",
                      MAX(2 * 1024 / sizeof(portSTACK_TYPE), configMINIMAL_STACK_SIZE) * 2,
                      NULL, tskIDLE_PRIORITY + 1, &g_task_mp3_dec);
    if (pdFAIL == ret)
    {
        //sdk_msg(SDK_MSG_TYPE_INFO,"ERROR: Failed to create MP3 task\n");
        return MMP_FALSE;
    }
    vTaskSuspend(g_task_mp3_dec);
            #endif

        #elif !defined(ENABLE_AUDIO_PROCESSOR) // ENABLE_CODECS_PLUGIN
            #if defined(ENABLE_CODECS_ARRAY)
    ret = xTaskCreate((pdTASK_CODE)__header->entry_point, "task_codec",
                      MAX(2 * 1024 / sizeof(portSTACK_TYPE), configMINIMAL_STACK_SIZE) * 2,
                      (void *)&codec_api, tskIDLE_PRIORITY + 1, &g_task_codec);
            #else
    ret = xTaskCreate( (pdTASK_CODE)NULL, "task_codec",
                       MAX(2 * 1024 / sizeof(portSTACK_TYPE), configMINIMAL_STACK_SIZE) * 2,
                       (void *)NULL, tskIDLE_PRIORITY + 1, &g_task_codec);
            #endif

    if (pdFAIL == ret)
    {
        //sdk_msg(SDK_MSG_TYPE_INFO,"ERROR: Failed to create CODEC task\n");
        return MMP_FALSE;
    }
    vTaskSuspend(g_task_codec);
        #endif // !defined(ENABLE_CODECS_PLUGIN)
    #endif     // __FREERTOS__

    AUD_PlugInType = 0;
    return MMP_TRUE;
}

MMP_RESULT
mmpAudioInitialize(
    void)
{
    LOG_ENTER "mmpAudioInitialize()\n" LOG_END

    lastTime        = PalGetClock();

    #if defined(__FREERTOS__)
    AUD_baseAddress = 0;
    #else
    AUD_baseAddress = (MMP_UINT32)HOST_GetVramBaseAddress();
    #endif

    #if !defined(__FREERTOS__)
    AUD_PowerOnRisc();
    #endif

    #ifdef OUTPUT_TO_FILE
    f_enterFS();
    #endif
    memset(&gADEncode, 0, sizeof(gADEncode));

    AUD_Init = MMP_TRUE;
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioSetCodec(
    MMP_ULONG *attribList)
{
    LOG_ENTER "mmpAudioSetCodec()\n" LOG_END

    if (!attribList)
        return MMP_RESULT_ERROR;

    if (!AUD_Codec.codecInit)
    {
        while (*attribList != MMP_AUDIO_CODEC_NONE)
        {
            switch (*attribList++)
            {
            case MMP_AUDIO_CODEC_INITIALIZE:
                AUD_Codec.CODEC_Initialize = (CODEC_INITIALIZE) *attribList++;
                break;

            case MMP_AUDIO_CODEC_TERMINATE:
                AUD_Codec.CODEC_Terminate = (CODEC_TERMINATE) *attribList++;
                break;

            case MMP_AUDIO_CODEC_SETVOLUME:
                AUD_Codec.CODEC_SetVolume = (CODEC_SETVOLUME) *attribList++;
                break;

            case MMP_AUDIO_CODEC_MODE:
                AUD_Codec.codecMode = (MMP_UINT) *attribList++;
                break;

            case MMP_AUDIO_CODEC_ADC_RATIO:
                AUD_Codec.adcRatio = (MMP_UINT) *attribList++;
                break;

            case MMP_AUDIO_CODEC_DAC_RATIO:
                AUD_Codec.dacRatio = (MMP_UINT) *attribList++;
                break;

            case MMP_AUDIO_CODEC_OVERSAMPLING_RATE:
                AUD_Codec.sampleRate = (MMP_UINT) *attribList++;
                break;
            }
        }
    }

    if (AUD_InitialCodec() != MMP_RESULT_SUCCESS)
        return MMP_RESULT_ERROR;

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioGetAvailableBufferLength(
    MMP_AUDIO_BUFFER_TYPE bufferType,
    MMP_ULONG             *bufferLength)
{
    //LOG_ENTER "mmpAudioGetAvailableBufferLength()\n" LOG_END

    if (AUD_SKIP)
    {
        *bufferLength = 65535;
        return MMP_RESULT_SUCCESS;
    }

    switch (bufferType)
    {
    case MMP_AUDIO_INPUT_BUFFER:
        if (AUD_GetReadAvailableBufferLength((MMP_UINT *)bufferLength) == MMP_RESULT_SUCCESS)
            return MMP_RESULT_SUCCESS;
        break;
    case MMP_AUDIO_OUTPUT_BUFFER:
        if (AUD_GetWriteAvailableBufferLength((MMP_UINT *)bufferLength) == MMP_RESULT_SUCCESS)
        {
            if (*bufferLength < BufThreshold)
                *bufferLength = 0;

            return MMP_RESULT_SUCCESS;
        }
        break;
    case MMP_AUDIO_MIXER_BUFFER:
        return MMP_RESULT_SUCCESS;
    default:
        break;
    }

    *bufferLength = 0;
    return MMP_RESULT_ERROR;
}

MMP_RESULT
mmpAudioGetEngineAttrib(
    MMP_AUDIO_ENGINE_ATTRIB attrib,
    void                    *value)
{
    LOG_ENTER "mmpAudioGetEngineAttrib()\n" LOG_END

    switch (attrib)
    {
    case MMP_AUDIO_ENGINE_TYPE:
        *((MMP_AUDIO_ENGINE *)value) = AUD_EngineType;
        break;
    case MMP_AUDIO_ENGINE_STATUS:
        AUD_GetEngineStatus((MMP_UINT *)value);
        break;
    default:
        return MMP_RESULT_ERROR;
    }
    return MMP_RESULT_SUCCESS;
}

MMP_ULONG
mmpAudioReadStream(
    MMP_UINT8 *stream,
    MMP_ULONG streamBufferSize,
    MMP_ULONG streamthreshold)
{
    MMP_UINT readableQueueLen;
    MMP_UINT bottomLen;
    MMP_UINT topLen;
    MMP_UINT residualLength;

    LOG_ENTER "mmpAudioReadStream()\n" LOG_END

    if (AUD_SKIP)
    {
        return streamBufferSize;
    }

    switch (AUD_EngineType)
    {
    case MMP_MP2_ENCODE:
    case MMP_AAC_ENCODE:
        break;
    default:
        return 0;
    }

    // word align
    streamBufferSize = (streamBufferSize >> 1);
    streamBufferSize = (streamBufferSize << 1);

    residualLength   = streamBufferSize;

    if (streamthreshold > Audio_Encode_Buflen)
        streamthreshold = (Audio_Encode_Buflen - 2);

    AUD_GetReadAvailableBufferLength(&readableQueueLen);

    // Wait buffer avaliable
    if (readableQueueLen < streamthreshold)
    {
        return 0;
    }

    PRECONDITION(readableQueueLen < Audio_Encode_Buflen);

    AUD_OccupyReadBuffer();
    if (readableQueueLen > residualLength)
        readableQueueLen = residualLength;

    if (readableQueueLen == 0)
    {
        // Nonthing to do.
    }
    else
    {
        bottomLen = Audio_Encode_Buflen - Audio_Encode_Rdptr;

        if (bottomLen > readableQueueLen)
            bottomLen = readableQueueLen;

    #if defined(__FREERTOS__)
        or32_invalidate_cache((MMP_UINT8 *)(AUD_baseAddress + Audio_Encode_Bufptr + Audio_Encode_Rdptr), MMP_AUDIO_ENCODE_SIZE);
        memcpy((void *)stream,
               (const void *)(AUD_baseAddress + Audio_Encode_Bufptr + Audio_Encode_Rdptr),
               bottomLen);
    #else
        HOST_ReadBlockMemory((MMP_UINT)stream, (MMP_UINT)(AUD_baseAddress + Audio_Encode_Bufptr + Audio_Encode_Rdptr), bottomLen);
    #endif
        stream += bottomLen;

        topLen  = readableQueueLen - bottomLen;

        if (topLen > 0)
        {
    #if defined(__FREERTOS__)
            or32_invalidate_cache((MMP_UINT8 *)(AUD_baseAddress + Audio_Encode_Bufptr), topLen);
            memcpy((void *)(stream),
                   (const void *)(AUD_baseAddress + Audio_Encode_Bufptr),
                   topLen);
    #else
            HOST_ReadBlockMemory((MMP_UINT)(stream), (MMP_UINT)(AUD_baseAddress + Audio_Encode_Bufptr), topLen);
    #endif
            stream += topLen;
        }
    }

    // Update Read Pointer (word alignment)
    Audio_Encode_Rdptr += readableQueueLen;
    if (Audio_Encode_Rdptr >= Audio_Encode_Buflen)
    {
        Audio_Encode_Rdptr -= Audio_Encode_Buflen;
    }
    HOST_WriteRegister(DrvEncode_RdPtr, Audio_Encode_Rdptr);
    AUD_ReleaseReadBuffer();

    return readableQueueLen;
}

MMP_RESULT
mmpAudioWriteStream(
    MMP_UINT8 *stream,
    MMP_ULONG length)
{
    MMP_UINT drvTimeout = 0;
    MMP_UINT writableQueueLen;
    MMP_UINT bottomLen;
    MMP_UINT topLen;
    MMP_UINT residualLength;
    MMP_UINT wav_length   = 0;
    MMP_UINT adpcm_length = 4;

    // LOG_ENTER "mmpAudioWriteStream()\n" LOG_END

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    #if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gAudioWrite += length;
    if (PalGetDuration(gtAudioDriverWirteBitrate) - gAudioWriteDuration >= 1000)
    {
        gAudioWriteDuration += 1000;
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] w duration %d data %d \n", PalGetDuration(gtAudioDriverWirteBitrate), gAudioWrite);
        gAudioWrite          = 0;
    }
    #endif

    #if 0
    if (write_waveHeader == MMP_TRUE)
    {
        if (wave_ADPCM == 4)
        {
            adpcm_length = 0;
        }

        do
        {
            AUD_GetWriteAvailableBufferLength(&wav_length);
            //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] wav_length %d \n",wav_length);
            if ((sizeof(wave_header) - adpcm_length) > length)
            {
                MMP_Sleep(1);
            }
            else
            {
                break;
            }
            drvTimeout++;
        } while (drvTimeout < 200);

        if (drvTimeout < 200)
        {
        #if defined(__FREERTOS__)
            memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr),
                   (const void *)wave_header,
                   (sizeof(wave_header) - adpcm_length));
            Audio_Decode_Wrptr = (MMP_UINT16)(Audio_Decode_Wrptr + (sizeof(wave_header) - adpcm_length) );
        #else
            HOST_WriteBlockMemory((MMP_UINT)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr), (MMP_UINT)wave_header, (sizeof(wave_header) - adpcm_length));
            Audio_Decode_Wrptr = (MMP_UINT16)(Audio_Decode_Wrptr + (sizeof(wave_header) - adpcm_length) );
            if (Audio_Decode_Wrptr >= Audio_Decode_Buflen)
            {
                Audio_Decode_Wrptr -= Audio_Decode_Buflen;
            }
        #endif
            write_waveHeader = MMP_FALSE;
        }
        else if (drvTimeout >= 200)
        {
            write_waveHeader = MMP_FALSE;
        }
    }
    #endif
    drvTimeout = 0;

    #if defined(OUTPUT_TO_FILE)
    if (!fout && AUD_EngineType == MMP_AAC_DECODE)
    {
        fout = PalTFileOpen(L"C:/audio_aac.aac", PAL_FILE_WB, MMP_NULL);
        sdk_msg(SDK_MSG_TYPE_INFO, "[audio driver] file  open  \n");
        f_enterFS();

        if (fout == NULL)
        {
            LOG_ERROR "Can not create audio output file %d \n", f_getlasterror() LOG_END
            return MMP_RESULT_ERROR;
        }
        if (AUD_EngineType == MMP_WAV_DECODE)
        {
            PalFileWrite(wave_header, sizeof(char), (sizeof(wave_header) - adpcm_length), fout, MMP_NULL);
        }
    }

    if (fout)
    {
        if (length != PalFileWrite(stream, sizeof(char), length, fout, MMP_NULL))
        {
            LOG_ERROR "Can not write audio output file\n" LOG_END
            return MMP_RESULT_ERROR;
        }
    }
    #endif

    #if defined(DONOT_PLAY_AUDIO)
    return MMP_RESULT_SUCCESS;
    #endif

    switch (AUD_EngineType)
    {
    case MMP_AMR_ENCODE:
        return MMP_RESULT_ERROR;
    default:
        break;
    }

    residualLength = length;

    AUD_OccupyWriteBuffer();
    while (residualLength)
    {
        // Wait buffer avaliable
        PRECONDITION(Audio_Decode_Buflen > BufThreshold);

        do
        {
            AUD_GetWriteAvailableBufferLength(&writableQueueLen);
            if (writableQueueLen < BufThreshold)
            {
                MMP_Sleep(1);
            }
            else
            {
                break;
            }
            drvTimeout++;
        } while (drvTimeout < 200);

        if (drvTimeout == 200 && (writableQueueLen < BufThreshold))
        {
            if (AUD_GetEngineStatus(&AUD_EngineState) != MMP_RESULT_SUCCESS)
            {
                AUD_ResetEngine();
                return MMP_RESULT_ERROR;
            }
            else
            {
                continue;
            }
        }

        if (residualLength <= writableQueueLen)
        {
            writableQueueLen = residualLength;
        }

        // streamptr += writableQueueLen;

        if (writableQueueLen == 0)
        {
            // Nonthing to do.
        }
        else
        {
            bottomLen = Audio_Decode_Buflen - Audio_Decode_Wrptr;
            if (bottomLen > writableQueueLen)
            {
                bottomLen = writableQueueLen;
            }

    #if defined(__FREERTOS__)
            memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr),
                   (const void *)stream,
                   bottomLen);
    #else
            HOST_WriteBlockMemory((MMP_UINT)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr), (MMP_UINT)stream, bottomLen);
    #endif
            stream += bottomLen;

            topLen  = writableQueueLen - bottomLen;

            if (topLen > 0)
            {
    #if defined(__FREERTOS__)
                memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr),
                       (const void *)(stream),
                       topLen);
    #else
                HOST_WriteBlockMemory((MMP_UINT)(AUD_baseAddress + Audio_Decode_Bufptr), (MMP_UINT)(stream), topLen);
    #endif
                stream += topLen;
            }
        }

        // Update Write Pointer (word alignment)
        Audio_Decode_Wrptr = (MMP_UINT16)(Audio_Decode_Wrptr + writableQueueLen);

        residualLength    -= writableQueueLen;

        if (Audio_Decode_Wrptr >= Audio_Decode_Buflen)
        {
            Audio_Decode_Wrptr -= Audio_Decode_Buflen;
        }

        HOST_WriteRegister(DrvDecode_WrPtr, Audio_Decode_Wrptr);
    }
    AUD_ReleaseWriteBuffer();

    if (writeStream == MMP_FALSE)
    {
        writeStream = MMP_TRUE;
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioOpenEngine(
    MMP_AUDIO_ENGINE audio_type, AUDIO_ENCODE_PARAM tAudioEncode)
{
    MMP_UINT8 *pAddress;
    unsigned int length;

    LOG_ENTER "mmpAudioOpenEngine()\n" LOG_END
    if (!AUD_Init)
    {
        mmpAudioInitialize();
    }
    #if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gtAudioDriverWirteBitrate = PalGetClock();
    gAudioWriteDuration       = 0;
    #endif

    // check audio encode parameter
    if ( (audio_type == MMP_MP2_ENCODE || audio_type == MMP_AAC_ENCODE ) && AUD_CheckEncodeParam(tAudioEncode, audio_type))
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] encode parameter error #line %d \n", __LINE__);
        return MMP_RESULT_ERROR;
    }

    // JEDI,Encode only
    if (audio_type == MMP_MP2_ENCODE || audio_type == MMP_AAC_ENCODE)
    {
        pAddress = audioReadCodec(audio_type, &length);
        //sdk_msg(SDK_MSG_TYPE_INFO,"audioReadCodec 0x%x %d \n",pAddress,length);
    }
    else
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] mmpAudioOpenEngine unknown encode type %d \n", audio_type);
        return MMP_RESULT_ERROR;
    }
    //
    if (AUD_LoadEngine(audio_type, pAddress, length) != MMP_RESULT_SUCCESS)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] mmpAudioOpenEngineAUD_LoadEngine err #line %d \n");
        return MMP_RESULT_ERROR;
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioActiveEngine(
    void)
{
    LOG_ENTER "mmpAudioActiveEngine()\n" LOG_END

    lastTime = PalGetClock();
    switch (AUD_FireEngine())
    {
    //case AUDIO_ERROR_ENGINE_IS_RUNNING:
    case MMP_RESULT_SUCCESS:
        {
            writeStream = MMP_FALSE;
            return MMP_RESULT_SUCCESS;
        }
    default:
        return MMP_RESULT_ERROR;
    }
}

MMP_RESULT
mmpAudioSetVolume(
    MMP_UINT vol)
{
    LOG_ENTER "mmpAudioSetVolume()\n" LOG_END
    if (AUD_Codec.CODEC_SetVolume)
    {
        AUD_Codec.CODEC_SetVolume(vol);
        AUD_Codec.curVolume = vol;
    }
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioGetVolume(
    MMP_UINT *vol)
{
    LOG_ENTER "mmpAudioGetVolume()\n" LOG_END

    if (AUD_Codec.CODEC_SetVolume)
    {
        *vol = AUD_Codec.curVolume;
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioStopEngine(
    void)
{
    LOG_ENTER "mmpAudioStopEngine()\n" LOG_END

    #if !defined(__FREERTOS__)
    AUD_StopRISC();
    #endif
    //AUD_ChangePll(AUD_HostPll);

    //MUTE
    /*
        if (HOST_CHIP_REV == CHIP_REV_A0) {
            HOST_WriteRegisterMask(0x166A, (MMP_UINT16)0x8080,0x8080);
        } else {
            HOST_WriteRegisterMask(0x166A, (MMP_UINT16)0x0000,0x8080);
        }
     */

    AUD_Init = MMP_FALSE; // need to init again
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioTerminate(
    void)
{
    LOG_ENTER "mmpAudioTerminate()\n" LOG_END

    AUD_TerminateCodec();
    AUD_ResetUserDefineIO3();
    AUD_EngineType = 0;
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioSetAttrib(
    const MMP_AUDIO_ATTRIB attrib,
    const void             *value)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    MMP_UINT16 nTemp;
    LOG_ENTER "mmpAudioSetAttrib()\n" LOG_END

    switch (attrib)
    {
    case MMP_AUDIO_STREAM_SAMPLERATE:
        AUD_decSampleRate = *((MMP_UINT *)(value));
        break;
    case MMP_AUDIO_STREAM_CHANNEL:
        AUD_decChannel    = *((MMP_UINT *)(value));
        break;
    #if defined(ENABLE_CODECS_PLUGIN)
    case MMP_AUDIO_PLUGIN_PATH:
        PalWcscpy(codec_path, (MMP_WCHAR *)(value));
        break;
    #endif

    case MMP_AUDIO_CODEC_SET_SAMPLE_RATE:
        if (*((MMP_UINT *)(value)) > 0)
        {
            AUD_nCodecSampleRate = *((MMP_UINT *)(value));
        }
        if (AUD_bEnableSTC_Sync == MMP_TRUE && AUD_bMPlayer == MMP_TRUE && AUD_decSampleRate != AUD_nCodecSampleRate)
        {
            AUD_decSampleRate = AUD_nCodecSampleRate;
        }
        break;

    case MMP_AUDIO_CODEC_SET_CHANNEL:
        if (*((MMP_UINT *)(value)) > 0)
        {
            AUD_nCodecChannels = *((MMP_UINT *)(value));
        }
        if (AUD_bEnableSTC_Sync == MMP_TRUE && AUD_bMPlayer == MMP_TRUE && AUD_decChannel != AUD_nCodecChannels)
        {
            AUD_decChannel = AUD_nCodecChannels;
        }
        break;

    case MMP_AUDIO_MUSIC_PLAYER_ENABLE:
        if (*((MMP_UINT *)(value)) > 0)
        {
            AUD_bPlayMusic = *((MMP_UINT *)(value));
        }
        break;

    case MMP_AUDIO_PTS_SYNC_ENABLE:
        if (*((MMP_UINT *)(value)) > 0)
        {
            AUD_bPtsSync = *((MMP_UINT *)(value));
        }
        break;

    case MMP_AUDIO_ENGINE_ADDRESS:
        if ((MMP_UINT8 *)value > 0)
        {
            AUD_pEngineAddress = (MMP_UINT8 *)(value);
        }
        break;
    case MMP_AUDIO_ENGINE_LENGTH:
        if (*((MMP_UINT32 *)(value)) > 0)
        {
            AUD_nEngineLength = *((MMP_UINT32 *)(value));
        }
        break;

    case MMP_AUDIO_I2S_INIT:
        if (*((MMP_UINT *)(value)) >= 0)
        {
            AUD_bI2SInit = *((MMP_UINT *)(value));
        }
        break;

    case MMP_AUDIO_FADE_IN:
        // fade in
        if (*((MMP_UINT *)(value)) > 0)
        {
            AUD_nFadeIn = *((MMP_UINT *)(value));
            //enableFadeInOut(AUD_nFadeIn);
        }
        break;

    case MMP_AUDIO_ENABLE_MPLAYER:
        if (*((MMP_UINT *)(value)) >= 0)
        {
            AUD_bMPlayer = *((MMP_UINT *)(value));
        }
        break;

    case MMP_AUDIO_ADJUST_MPLAYER_PTS:
        AUD_nMPlayerPTS = *((MMP_INT *)(value));
        break;

    case MMP_AUDIO_MPLAYER_STC_READY:
        AUD_bMPlayerSTCReady = *((MMP_UINT *)(value));
        break;

    case MMP_AUDIO_DECODE_ERROR:
        if (*((MMP_UINT *)(value)) == 0 || *((MMP_UINT *)(value)) == 1)
        {
            AUD_bDecodeError = *((MMP_UINT *)(value));
            nTemp            = AUD_bDecodeError;
    #if  defined(__FREERTOS__)
            HOST_WriteRegister(AUDIO_DECODER_WRITE_DECODE_ERROR, nTemp);
    #endif
        }
        break;

    case MMP_AUDIO_DECODE_DROP_DATA:
        if (*((MMP_UINT *)(value)) >= 0)
        {
            AUD_nDropData = *((MMP_UINT *)(value));
            nTemp         = AUD_nDropData;
            //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver]set drop audio %d \n",nTemp);
    #if  defined(__FREERTOS__)
            HOST_WriteRegister(AUDIO_DECODER_DROP_DATA, nTemp);
    #endif
        }
        break;

    case MMP_AUDIO_PAUSE_STC_TIME:
        if (*((MMP_UINT32 *)(value)) >= 0)
        {
            AUD_nPauseSTCTime = *((MMP_UINT32 *)(value));
            //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver]set pause stc time %d \n",AUD_nPauseSTCTime);
        }
        break;

    case MMP_AUDIO_ENABLE_STC_SYNC:
        if (*((MMP_UINT *)(value)) >= 0)
        {
            AUD_bEnableSTC_Sync = *((MMP_UINT *)(value));
        }
        break;

    case MMP_AUDIO_SPDIF_NON_PCM:
        if (*((MMP_UINT *)(value)) >= 0)
        {
            AUD_bSPDIFNonPCM = *((MMP_UINT *)(value));
        }
        break;

    case MMP_AUDIO_ENCODE_START_TIME:
        gEncodeStartTime = *((MMP_UINT *)(value));
        break;

    default:
        result = MMP_RESULT_ERROR;
        break;
    }

    return result;
}

MMP_RESULT
mmpAudioGetAttrib(
    const MMP_AUDIO_ATTRIB attrib,
    void                   *value)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    MMP_UINT I2SBufLen;
    MMP_UINT I2SRdPtr;
    MMP_UINT I2SWrPtr;
    MMP_UINT I2SDataLength;
    MMP_UINT16 data = 0;

    LOG_ENTER "mmpAudioSetAttrib()\n" LOG_END
    #if defined(__FREERTOS__)
    switch (attrib)
    {
    case MMP_AUDIO_GET_IS_EOF:
        *((MMP_UINT *)(value)) = isEOF();
        break;

    case MMP_AUDIO_CODEC_SET_SAMPLE_RATE:
        *((MMP_UINT *)(value)) = AUD_nCodecSampleRate;
        break;

    case MMP_AUDIO_CODEC_SET_CHANNEL:
        *((MMP_UINT *)(value)) = AUD_nCodecChannels;
        break;

    case MMP_AUDIO_MUSIC_PLAYER_ENABLE:
        *((MMP_UINT *)(value)) = AUD_bPlayMusic;
        break;

    case MMP_AUDIO_PTS_SYNC_ENABLE:
        *((MMP_UINT *)(value)) = AUD_bPtsSync;
        break;

    case MMP_AUDIO_ENGINE_ADDRESS:
        value = AUD_pEngineAddress;
        break;

    case MMP_AUDIO_ENGINE_LENGTH:
        *((MMP_UINT *)(value)) = AUD_nEngineLength;
        break;

    case MMP_AUDIO_I2S_INIT:
        *((MMP_UINT *)(value)) = AUD_bI2SInit;
        break;

    case MMP_AUDIO_ENABLE_MPLAYER:
        *((MMP_UINT *)(value)) = AUD_bMPlayer;
        break;

    case MMP_AUDIO_MPLAYER_STC_READY:
        *((MMP_UINT *)(value)) = AUD_bMPlayerSTCReady;
        break;

    case MMP_AUDIO_DECODE_ERROR:
        AUD_bDecodeError = MMP_FALSE;
        #if  defined(__FREERTOS__)
        HOST_ReadRegister(AUDIO_DECODER_WRITE_DECODE_ERROR, &data);
        if (data > 0)
        {
            AUD_bDecodeError = MMP_TRUE;
        }
        #endif
        *((MMP_UINT *)(value)) = AUD_bDecodeError;
        break;

    case MMP_AUDIO_I2S_OUT_FULL:
        AUD_bI2SOutFull = MMP_FALSE;
        #if  defined(__FREERTOS__)
        HOST_ReadRegister(MMIO_I2S_OUT_BUF_LEN, &data);
        I2SBufLen       = (MMP_UINT)data;
        HOST_ReadRegister(MMIO_I2S_OUT_BUF_RDPTR, &data);
        I2SRdPtr        = (MMP_UINT)data;
        HOST_ReadRegister(MMIO_I2S_OUT_BUF_WRPTR, &data);
        I2SWrPtr        = (MMP_UINT)data;
        I2SDataLength   = (I2SRdPtr > I2SWrPtr) ? (I2SBufLen - (I2SRdPtr - I2SWrPtr)) : (I2SWrPtr - I2SRdPtr);

        if (I2SDataLength + (I2SBufLen / 2) > I2SBufLen)
        {
            AUD_bI2SOutFull = MMP_TRUE;
            //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] i2s data %d buf length %d \n",I2SDataLength,I2SBufLen);
        }
        #endif
        *((MMP_UINT *)(value)) = AUD_bI2SOutFull;
        break;

    case MMP_AUDIO_DECODE_DROP_DATA:
        AUD_nDropData = 0;
        #if  defined(__FREERTOS__)
        HOST_ReadRegister(AUDIO_DECODER_DROP_DATA, &data);
        if (data > 0)
        {
            AUD_nDropData = data;
        }
        //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver]get AUD_nDropData %d \n",AUD_nDropData);
        #endif
        *((MMP_UINT *)(value)) = AUD_nDropData;
        break;

    case MMP_AUDIO_PAUSE_STC_TIME:
        *((MMP_UINT32 *)(value)) = AUD_nPauseSTCTime;
        break;

    case MMP_AUDIO_IS_PAUSE:
        AUD_bIsPause           = 0;
        #if  defined(__FREERTOS__)
        HOST_ReadRegister(DrvAudioCtrl, &data);
        AUD_bIsPause           = (data & DrvDecode_PAUSE) != 0;
        #endif
        *((MMP_UINT *)(value)) = AUD_bIsPause;
        break;

    case MMP_AUDIO_ENABLE_STC_SYNC:
        *((MMP_UINT *)(value)) = AUD_bEnableSTC_Sync;
        break;

    case MMP_AUDIO_DIRVER_DECODE_BUFFER_LENGTH:
        *((MMP_UINT *)(value)) = Audio_Decode_Buflen;
        break;

    case MMP_AUDIO_SPDIF_NON_PCM:
        *((MMP_UINT *)(value)) = AUD_bSPDIFNonPCM;
        break;

    case MMP_AUDIO_ENCODE_START_TIME:
        *((MMP_UINT *)(value)) = gEncodeStartTime;
        break;

    default:
        result = MMP_RESULT_ERROR;
        break;
    }
    #endif
    return result;
}

MMP_RESULT
mmpAudioGetDecodeTimeV2(
    MMP_UINT *time)
{
    #if 1
    MMP_UINT16 data = 0;
    MMP_UINT I2SBufLen;
    MMP_UINT I2SRdPtr;
    MMP_UINT I2SWrPtr;
    MMP_UINT I2SDataLength;
    MMP_UINT32 decTime = 0;
    MMP_UINT32 offset  = 0;
    MMP_RESULT result  = MMP_RESULT_SUCCESS;
    MMP_UINT16 nTemp, timeout = 0;

    //LOG_ENTER "mmpAudioGetDecodeTimeV2()\n" LOG_END
    // We can't get time of RAW mode output through SPDIF,
    // therefore, we assume such case is same as no audio data.
    switch (AUD_EngineType)
    {
    case MMP_AC3_SPDIF_DECODE:
        *time = 0;
        return MMP_RESULT_SUCCESS;
    default:
        break;
    }

    if (AUD_SKIP)
    {
        *time = PalGetDuration(lastTime);
        return MMP_RESULT_SUCCESS;
    }

    if (AUD_decChannel == 0 || AUD_decSampleRate == 0)
    {
        result = MMP_RESULT_ERROR;
    }
    else
    {
WAIT:
        timeout = 0;
        do
        {
            nTemp = mmpAudioGetAuidoProcessorWritingStatus();
            if (nTemp > 0)
            {
                //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver]wait %d \n",timeout);
                PalSleep(1);
                timeout++;
            }
        } while (nTemp > 0 && timeout < 5);

        HOST_ReadRegister((DrvDecode_Frame + 2), &data);
        decTime   = (MMP_UINT)(data << 16);

        HOST_ReadRegister(DrvDecode_Frame, &data);
        decTime  += (MMP_UINT)data;

        HOST_ReadRegister(0x1654, &data);
        I2SBufLen = (MMP_UINT)data;
        HOST_ReadRegister(0x165C, &data);
        I2SRdPtr  = (MMP_UINT)data;
        nTemp     = mmpAudioGetAuidoProcessorWritingStatus();
        // need to re-read register
        if (nTemp > 0)
        {
            //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver]wait write \n");
            goto WAIT;
        }
        HOST_ReadRegister(0x1656, &data);

        I2SWrPtr      = (MMP_UINT)data;

        I2SDataLength = (I2SRdPtr > I2SWrPtr) ? (I2SBufLen - (I2SRdPtr - I2SWrPtr)) : (I2SWrPtr - I2SRdPtr);
        I2SDataLength = (AUD_decChannel == 2) ? (I2SDataLength >> 2) : (I2SDataLength >> 1);
        I2SDataLength = ((I2SDataLength << 16) / AUD_decSampleRate);

        if (gLastDecTime)
        {
            // time increment wrap around
            if (decTime < gLastDecTime
                && gLastDecTime - decTime >= AUDIO_WRAP_AROUND_JUDGE_RANGE)
            {
                gPtsTimeBaseOffset += AUDIO_WRAP_AROUND_THRESHOLD;
            }
            else if ((decTime - gLastDecTime) >= AUDIO_WRAP_AROUND_JUDGE_RANGE) // time decrement wrap around
            {
                if (gPtsTimeBaseOffset)
                    gPtsTimeBaseOffset -= AUDIO_WRAP_AROUND_THRESHOLD;
            }
        }

        gLastDecTime = decTime;
        offset       = gPtsTimeBaseOffset;

        if (decTime > I2SDataLength)
        {
            decTime -= I2SDataLength;
        }
        else // decTime <= I2SDataLength
        {
            if (offset)
            {
                offset -= (((((I2SDataLength - decTime) & 0xffff) * 1000) >> 16)
                           + (((I2SDataLength - decTime) >> 16) * 1000));
            }
            else
                decTime = I2SDataLength;
        }

        (*time) = ((((decTime) & 0xffff) * 1000) >> 16) + (((decTime) >> 16) * 1000) + offset;
    }

    return result;

    #else
    MMP_UINT16 data = 0;
    MMP_UINT I2SBufLen;
    MMP_UINT I2SRdPtr;
    MMP_UINT I2SWrPtr;
    MMP_UINT I2SDataLength;
    MMP_UINT32 decTime = 0;
    MMP_RESULT result  = MMP_RESULT_SUCCESS;

    //LOG_ENTER "mmpAudioGetDecodeTimeV2()\n" LOG_END

    if (AUD_SKIP)
    {
        *time = PalGetDuration(lastTime);
        return MMP_RESULT_SUCCESS;
    }

    if (AUD_decChannel == 0 || AUD_decSampleRate == 0)
    {
        result = MMP_RESULT_ERROR;
    }
    else
    {
        HOST_ReadRegister((DrvDecode_Frame + 2), &data);
        (*time)  = (MMP_UINT)(data << 16);

        HOST_ReadRegister(DrvDecode_Frame, &data);
        (*time) += (MMP_UINT)data;

        #if defined(MM680)
        HOST_ReadRegister(0x1654, &data);
        I2SBufLen = (MMP_UINT)data;

        HOST_ReadRegister(0x165C, &data);
        I2SRdPtr  = (MMP_UINT)data;
        HOST_ReadRegister(0x1656, &data);
        I2SWrPtr  = (MMP_UINT)data;
        //#else // MM365, MM370
        //        HOST_ReadRegister(0x16D0, &data);
        //        I2SBufLen = (MMP_UINT)data;
        //
        //        HOST_ReadRegister(0x16C2, &data);
        //        I2SRdPtr = (MMP_UINT)data;
        //        HOST_ReadRegister(0x16C4, &data);
        //        I2SWrPtr = (MMP_UINT)data;
        #endif // MM680

        I2SDataLength = (I2SRdPtr > I2SWrPtr) ? (I2SBufLen - (I2SRdPtr - I2SWrPtr)) : (I2SWrPtr - I2SRdPtr);
        I2SDataLength = (AUD_decChannel == 2) ? (I2SDataLength >> 2) : (I2SDataLength >> 1);
        I2SDataLength = ((I2SDataLength << 16) / AUD_decSampleRate);

        if ((*time) > I2SDataLength)
            (*time) -= I2SDataLength;
        else
            (*time) = I2SDataLength;

        (*time) = ((((*time) & 0xffff) * 1000) >> 16) + (((*time) >> 16) * 1000);
    }

    return result;
    #endif
}

MMP_RESULT
mmpAudioGetDecodeTime(
    MMP_UINT *time)
{
    MMP_UINT16 data = 0;

    //LOG_ENTER "mmpAudioGetDecodeTime()\n" LOG_END
    if (AUD_SKIP)
    {
        *time = PalGetDuration(lastTime);
        return MMP_RESULT_SUCCESS;
    }

    if (AUD_EngineType == MMP_AMR_ENCODE)
        return MMP_RESULT_ERROR;

    HOST_ReadRegister((DrvDecode_Frame + 2), &data);
    (*time)  = (MMP_UINT)data * 1000;

    HOST_ReadRegister(DrvDecode_Frame, &data);
    (*time) += ((MMP_UINT)(data * 1000) >> 16);

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioGetEncodeTime(
    MMP_UINT *time)
{
    MMP_UINT16 data = 0;

    //LOG_ENTER "mmpAudioGetEncodeTime()\n" LOG_END

    if (AUD_SKIP)
    {
        *time = PalGetDuration(lastTime);
        return MMP_RESULT_SUCCESS;
    }

    if (  (AUD_EngineType != MMP_AMR_ENCODE) & (AUD_EngineType != MMP_AMR_CODEC) && (AUD_EngineType != MMP_WAV_DECODE )  )
        return MMP_RESULT_ERROR;

    HOST_ReadRegister((DrvEncode_Frame + 2), &data);
    (*time)  = (MMP_UINT)data * 1000;

    HOST_ReadRegister(DrvEncode_Frame, &data);
    (*time) += ((MMP_UINT)(data * 1000) >> 16);

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioEndStream(
    void)
{
    LOG_ENTER "mmpAudioEndStream()\n" LOG_END

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    if (AUD_SetOutEOF() != MMP_RESULT_SUCCESS)
    {
        mmpAudioStop();  // if codec timeout ,set stop bit to return clearbuffer
        return MMP_RESULT_ERROR;
    }

    Audio_Decode_Wrptr = 0;
    gPtsTimeBaseOffset = 0;
    gLastDecTime       = 0;

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioPause(
    MMP_BOOL enable)
{
    LOG_ENTER "mmpAudioPause(%d)\n", enable LOG_END

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    if (AUD_bEnableSTC_Sync == MMP_TRUE)
    {
        // mplayer using mmpAudioSTCSyncPause()
        if (AUD_bMPlayer == MMP_TRUE)
        {}
        else
        {
            if (enable == MMP_TRUE)
            {
                HOST_WriteRegisterMask(DrvAudioCtrl, 1, DrvDecode_PAUSE);
            }
            else
            {
                HOST_WriteRegisterMask(DrvAudioCtrl, 0, DrvDecode_PAUSE);
            }
        }
    }
    else
    {
        if (enable == MMP_TRUE)
        {
            HOST_WriteRegisterMask(DrvAudioCtrl, 1, DrvDecode_PAUSE);
        }
        else
        {
            HOST_WriteRegisterMask(DrvAudioCtrl, 0, DrvDecode_PAUSE);
        }
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioSTCSyncPause(
    MMP_BOOL enable)
{
    if (enable == MMP_TRUE)
    {
        HOST_WriteRegisterMask(DrvAudioCtrl, 1, DrvDecode_PAUSE);
    }
    else
    {
        HOST_WriteRegisterMask(DrvAudioCtrl, 0, DrvDecode_PAUSE);
    }
}

MMP_RESULT
mmpAudioSetWaveMode(
    MMP_INT mode)
{
    #if  defined(__FREERTOS__)

    if (mode == 0)
    {   // set mode decode
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, DrvWAV_Mode);
    }
    else if (mode == 1)
    {   // set mode encode
        HOST_WriteRegisterMask(DrvAudioCtrl2, 1 << DrvWAV_ModeBits, DrvWAV_Mode);
    }
    mmpAudioStop();
    #endif
}

MMP_RESULT
mmpAudioSetWaveEncode(
    MMP_INT nType, MMP_INT nChannels, MMP_INT nSampleRate)
{
    unsigned int ctrl;
    #if  defined(__FREERTOS__)
    //sdk_msg(SDK_MSG_TYPE_INFO,"mmpAudioSetWaveEncode nType  %d %d \n",nType,nSampleRate);
    switch (nType)
    {
    case DrvWAV_TypePCM16:
        HOST_WriteRegisterMask(DrvAudioCtrl2, DrvWAV_TypePCM16, 7 << DrvWAV_TypeBits);
        break;

    case DrvWAV_TypePCM8:
        HOST_WriteRegisterMask(DrvAudioCtrl2, DrvWAV_TypePCM8, 7 << DrvWAV_TypeBits);
        break;

    case DrvWAV_TypeADPCM:
        HOST_WriteRegisterMask(DrvAudioCtrl2, DrvWAV_TypeADPCM, 7 << DrvWAV_TypeBits);
        break;

    case DrvWAV_TypeALAW:
        HOST_WriteRegisterMask(DrvAudioCtrl2, DrvWAV_TypeALAW, 7 << DrvWAV_TypeBits);
        break;

    case DrvWAV_TypeULAW:
        HOST_WriteRegisterMask(DrvAudioCtrl2, DrvWAV_TypeULAW, 7 << DrvWAV_TypeBits);
        break;

    default:
        HOST_WriteRegisterMask(DrvAudioCtrl2, DrvWAV_TypeADPCM, 7 << DrvWAV_TypeBits);
        break;
    }

    if (nChannels == 1)
    {   // set mode decode
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, 1 << DrvWAV_EncChannelBits);
    }
    else if (nChannels == 2)
    {   // set mode encode
        HOST_WriteRegisterMask(DrvAudioCtrl2, 1 << DrvWAV_EncChannelBits, 1 << DrvWAV_EncChannelBits);
    }

    switch (nSampleRate)
    {
    case WAVE_SRATE_6000:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_6000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_8000:
        HOST_WriteRegisterMask(DrvAudioCtrl2,  WAVE_SRATE_8000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_11025:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_11025 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_12000:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_12000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_16000:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_16000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_22050:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_22050 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_24000:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_24000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_32000:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_32000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    case WAVE_SRATE_44100:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_44100 << DrvWAV_EncSampRateBits, WAVE_SRATE_44100 << DrvWAV_EncSampRateBits);

        break;

    case WAVE_SRATE_48000:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_48000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;

    default:
        HOST_WriteRegisterMask(DrvAudioCtrl2, WAVE_SRATE_16000 << DrvWAV_EncSampRateBits, 15 << DrvWAV_EncSampRateBits);
        break;
    }

    #endif
}

MMP_RESULT
mmpAudioSeek(
    MMP_BOOL enable)
{
    MMP_UINT16 regData = 0;
    MMP_UINT timeout   = 2000;

    LOG_ENTER "mmpAudioSeek(%d)\n", enable LOG_END

    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    if (enable == MMP_TRUE)
    {
    #if defined(__FREERTOS__)
        HOST_WriteRegisterMask(DrvAudioCtrl2, (1 << 2), DrvDecode_Skip);
        do
        {
            HOST_ReadRegister(DrvAudioCtrl2, &regData);
            if (!(regData & DrvDecode_Skip))
            {
                Audio_Decode_Wrptr = 0;
                return MMP_TRUE;
            }
            MMP_Sleep(1);
            timeout--;
            if (timeout == 0)
            {
                LOG_ERROR "Can not correctly set end of stream, reset engine\n" LOG_END
                return MMP_FALSE;
            }
        } while (1);
    #endif
    }
    else
    {
    #if defined(__FREERTOS__)
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, DrvDecode_Skip);
    #endif
    }

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioStop(
    void)
{
    MMP_UINT16 regData = 0;
    MMP_UINT timeout   = 1000;
    MMP_UINT32 nTmp    = 0;
    MMP_UINT nTemp     = 0;

    #if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gAudioWrite         = 0;
    gAudioWriteDuration = 0;
    #endif

    LOG_ENTER "mmpAudioStop()\n" LOG_END
    #if defined(OUTPUT_TO_FILE)
    if (fout)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] PalFileClose\n");
        PalFileClose(fout, MMP_NULL);
        fout = NULL;
    }
    #endif

    lastTime = PalGetClock();
    if (AUD_SKIP)
        return MMP_RESULT_SUCCESS;

    if (AUD_EngineType == MMP_WAV_DECODE)
    {
        write_waveHeader = MMP_TRUE;
    }
    gEncodeTimeStamp  = 0;
    gEncodeTimeAdding = 0;
    mmpAudioSetAttrib(MMP_AUDIO_MPLAYER_STC_READY, (void *)&nTemp);
    mmpAudioSetAttrib(MMP_AUDIO_ADJUST_MPLAYER_PTS, (void *)&nTemp);
    mmpAudioSetAttrib(MMP_AUDIO_DECODE_DROP_DATA, (void *)&nTemp);
    mmpAudioSetAttrib(MMP_AUDIO_PAUSE_STC_TIME, (void *)&nTmp);
    memset(&gADEncode, 0, sizeof(gADEncode));

    HOST_WriteRegisterMask(DrvAudioCtrl, (1 << 6), DrvDecode_STOP);

    //MMIO_Write(MMIO_PTS_WRIDX, 0);
    //MMIO_Write(MMIO_PTS_HI, 0);
    //MMIO_Write(MMIO_PTS_LO, 0);

    do
    {
        HOST_ReadRegister(DrvAudioCtrl, &regData);
        if (!(regData & DrvDecode_STOP))
        {
            Audio_Decode_Wrptr = 0;
            Audio_Encode_Wrptr = 0;
            Audio_Encode_Rdptr = 0;
            gPtsTimeBaseOffset = 0;
            gLastDecTime       = 0;
            return MMP_RESULT_SUCCESS;
        }
        MMP_Sleep(1);
        timeout--;
        if (timeout == 0)
        {
            HOST_WriteRegisterMask(DrvAudioCtrl, (0 << DrvDecode_STOP_Bits), (1 << DrvDecode_STOP_Bits));
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] Can not correctly set end of stream, reset engine\n");
            return MMP_RESULT_ERROR;
        }
    } while (1);
}

MMP_RESULT
mmpAudioPowerOnAmplifier(void)
{
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioPowerOffAmplifier(
    void)
{
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpAudioGetPlayPos(
    MMP_INT32 *pos)
{
    #ifndef _WIN32
    *pos = AUD_curPlayPos;
    #endif

    return MMP_SUCCESS;
}

MMP_RESULT
mmpAudioSetPlayPos(
    MMP_INT32 pos)
{
    #ifndef _WIN32
    AUD_curPlayPos = pos;
    #endif

    return MMP_SUCCESS;
}

    #ifdef HAVE_WAV
MMP_RESULT
mmpAudioSetWaveDecodeHeader(MMP_WaveInfo wavInfo)
{
    MMP_UINT16 ans            = 0;
    MMP_UINT16 regData;

    MMP_UINT32 nSamplesPerSec = 44100;
    MMP_UINT16 wFormatTag;
    MMP_UINT16 i;
    MMP_UINT8 *ref;

    LOG_ENTER "mmpAudioSetWaveDecodeHeader()\n" LOG_END
    AUD_decSampleRate = wavInfo.sampRate;
    if (wavInfo.nChans > 2 && wavInfo.nChans <= 6)
    {
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] set wav channels %d \n", wavInfo.nChans);
        AUD_decChannel = 2;
    }
    else if (wavInfo.nChans <= 2)
    {
        AUD_decChannel = wavInfo.nChans;
    }

    // set decode header
    for (i = 0; i < sizeof(wave_header); i++)
    {
        wave_header[i] = 0;
    }

    //ckID
    wave_header[0]  = 'R';
    wave_header[1]  = 'I';
    wave_header[2]  = 'F';
    wave_header[3]  = 'F';
    //cksize
    wave_header[4]  = 0x24;
    wave_header[5]  = 0x60;
    wave_header[6]  = 0x01;
    wave_header[7]  = 0x00;
    //WAVEID
    wave_header[8]  = 'W';
    wave_header[9]  = 'A';
    wave_header[10] = 'V';
    wave_header[11] = 'E';
    //ckID
    wave_header[12] = 'f';
    wave_header[13] = 'm';
    wave_header[14] = 't';
    wave_header[15] = ' ';
    //cksize ADPCM is 20, others is 16
    if (wavInfo.format == 3)
    {
        wave_header[16] = 0x14;
        wave_ADPCM      = 4;
        ref             = wavInfo.pvData;
        wave_header[36] = 0x02;
        wave_header[37] = 0;
        wave_header[38] = ref[0];
        wave_header[39] = ref[1];
    }
    else
    {
        wave_header[16] = 0x10;
    }
    wave_header[17] = 0;
    wave_header[18] = 0;
    wave_header[19] = 0;

    //wFormatTag
    // set format
    switch (wavInfo.format)
    {
    case MMP_WAVE_FORMAT_PCM:
        wFormatTag = 1;
        if (wavInfo.bitsPerSample == 8)
        {
            wave_header[34] = 0x08;
        }
        else
        {
            wave_header[34] = 0x10;
        }
        break;
    case MMP_WAVE_FORMAT_ALAW:
        wFormatTag      = 6;
        wave_header[34] = 0x08;
        break;
    case MMP_WAVE_FORMAT_MULAW:
        wFormatTag      = 7;
        wave_header[34] = 0x08;
        break;
    case MMP_WAVE_FORMAT_DVI_ADPCM:
        wFormatTag      = 0x11;
        wave_header[34] = 0x04;
        break;
    default:
        wFormatTag      = 0;
        wave_header[34] = 0x10;
        break;
    }

    wave_header[21] = (MMP_UINT8) ((wFormatTag >> 8) & 0xff);
    wave_header[20] = (MMP_UINT8) ((wFormatTag) & 0xff);

    if (wavInfo.nChans <= 6)
    {
        wave_header[23] = 0;
        wave_header[22] = wavInfo.nChans;
    }
    else
    {
        wave_header[23] = 0;
        wave_header[22] = 0x01;
        sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] set wave header channels %d > 6\n", wavInfo.nChans);
    }

    if (wavInfo.sampRate <= 96000)
    {
        nSamplesPerSec = wavInfo.sampRate;
    }

    wave_header[27]              = (MMP_UINT8) ((nSamplesPerSec >> 24) & 0xff);
    wave_header[26]              = (MMP_UINT8) ((nSamplesPerSec >> 16) & 0xff);
    wave_header[25]              = (MMP_UINT8) ((nSamplesPerSec >> 8) & 0xff);
    wave_header[24]              = (MMP_UINT8) ((nSamplesPerSec) & 0xff);

    wave_header[36 + wave_ADPCM] = 'd';
    wave_header[37 + wave_ADPCM] = 'a';
    wave_header[38 + wave_ADPCM] = 't';
    wave_header[39 + wave_ADPCM] = 'a';

    wave_header[40 + wave_ADPCM] = 0;
    wave_header[41 + wave_ADPCM] = 0;
    wave_header[42 + wave_ADPCM] = 0;
    wave_header[43 + wave_ADPCM] = 0;

    write_waveHeader             = MMP_TRUE;

    return MMP_SUCCESS;
}

    #else
MMP_RESULT
mmpAudioSetWaveDecodeHeader(MMP_WaveInfo wavInfo)
{
    return MMP_SUCCESS;
}

    #endif

MMP_RESULT
mmpAudioSetPts(
    MMP_UINT32 pts)
{
    MMP_RESULT result  = MMP_RESULT_ERROR;
    MMP_UINT16 swpr    = 0;
    MMP_UINT16 pts_low = 0;
    MMP_UINT16 pts_hi  = 0;
    MMP_BOOL bPtsSync;

    HOST_ReadRegister(0x16AE, &swpr);
    HOST_ReadRegister(0x16B0, &pts_low);
    HOST_ReadRegister(0x16B2, &pts_hi);

    if (swpr == 0 && pts_low == 0 && pts_hi == 0)
    {
        MMP_UINT16 regdata;
        HOST_ReadRegister(DrvDecode_WrPtr, &regdata);
        HOST_WriteRegister(0x16AE, regdata);
        HOST_WriteRegister(0x16B0, (MMP_UINT16)(pts & 0xFFFFL));
        HOST_WriteRegister(0x16B2, (MMP_UINT16)(pts >> 16));
        result = MMP_RESULT_SUCCESS;
    }
    bPtsSync = MMP_TRUE;
    mmpAudioSetAttrib(MMP_AUDIO_PTS_SYNC_ENABLE, &bPtsSync);

    return result;
}

MMP_RESULT
mmpAudioSetMPlayerPts(
    MMP_INT pts)
{
    MMP_RESULT result  = MMP_RESULT_ERROR;
    MMP_UINT16 swpr    = 0;
    MMP_UINT16 pts_low = 0;
    MMP_UINT16 pts_hi  = 0;
    MMP_BOOL bPtsSync;

    MMP_UINT16 regdata;
    //sdk_msg(SDK_MSG_TYPE_INFO,"[Audio Driver] write %d \n",pts);
    result = MMP_RESULT_SUCCESS;

    return result;
}

void
mmpAudioSetEarPhonePlugin(MMP_BOOL detect)
{
    g_EarPhoneDetect = detect;
}

MMP_BOOL
mmpAudioGetEarPhonePlugin(void)
{
    return g_EarPhoneDetect;
}

void
mmpAudioTurnOnStereoToMono(void)
{
    #if defined(__FREERTOS__)
    HOST_WriteRegisterMask(DrvAudioCtrl2, 0x0002, 0x0002);
    #endif
}

void
mmpAudioTurnOffStereoToMono(void)
{
    #if defined(__FREERTOS__)
    HOST_WriteRegisterMask(DrvAudioCtrl2, 0x0000, 0x0002);
    #endif
}

MMP_AUDIO_MODE
mmpAudioGetMode(void)
{
    #if defined(__FREERTOS__)
    MMP_UINT16 regdata;
    HOST_ReadRegister(DrvAudioCtrl2, &regdata);
    return ((regdata >> 3) & 3);
    #else
    return 0;
    #endif
}

void
mmpAudioSetMode(MMP_AUDIO_MODE mode)
{
    #if defined(__FREERTOS__)
    HOST_WriteRegisterMask(DrvAudioCtrl2, (mode << 3), (3 << 3));
    #endif
}

void
mmpAudioSetWaveDecEndian(
    MMP_INT mode)
{
    #if defined(__FREERTOS__)
    if (mode == 0)
    {   //  to big endian
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, DrvPCM_DecEndian);
    }
    else if (mode == 1)
    {   // to little endian
        HOST_WriteRegisterMask(DrvAudioCtrl2, 1 << DrvPCM_DecEndianBits, DrvPCM_DecEndian);
    }
    #endif
}

void
mmpAudioSetShowSpectrum(
    MMP_INT mode)
{
    #if defined(__FREERTOS__)
    if (mode == 0)
    {      //  not show spectrum
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, DrvShowSpectrum);
    }
    else if (mode == 1)
    {      // show spectrum
        HOST_WriteRegisterMask(DrvAudioCtrl2, 1 << DrvShowSpectrumMode_Bits, DrvShowSpectrum);
    }
    #endif
}

void
mmpAudioSetUpSampling(
    MMP_INT nEnable)
{
    #if defined(__FREERTOS__)
    if (nEnable == 0)
    {   //  not enable upSampling
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, DrvEnableUpSampling);
    }
    else if (nEnable == 1)
    {   // enable upSampling
        HOST_WriteRegisterMask(DrvAudioCtrl2, 1 << DrvEnableUpSampling_Bits, DrvEnableUpSampling);
    }
    #endif
}

void
mmpAudioSetUpSamplingOnly2x(
    MMP_INT nEnable)
{
    #if defined(__FREERTOS__)
    if (nEnable == 0)
    {   //  not enable upSampling
        HOST_WriteRegisterMask(DrvAudioCtrl2, 0, DrvUpSampling2x);
    }
    else if (nEnable == 1)
    {   // enable upSampling
        HOST_WriteRegisterMask(DrvAudioCtrl2, 1 << DrvUpSampling2x_Bits, DrvUpSampling2x);
    }
    #endif
}

void
mmpAudioSetSuspendEngine(
    MMP_INT    nSuspend,
    MMP_UINT16 volume_status)
{
    MMP_INT nResult;
    MMP_UINT8 *pAddress;
    MMP_UINT32 length;
    MMP_UINT16 value;
    MMP_UINT32 i;
    AUDIO_ENCODE_PARAM tAEncode;

    if (nSuspend == 0)
    {   //  not nSuspend
       //  not nSuspend
        HOST_WriteRegister(0x3A, 0xB000);
        HOST_WriteRegister(0x3C, 0x800F);
        HOST_WriteRegister(0x3E, 0x00AA);

        HOST_WriteRegister(0x1640, 0x000F);
        HOST_WriteRegister(0x1642, 0x400F);
        HOST_WriteRegister(0x1644, 0x3003);
        PalSleep(10);

        HOST_WriteRegisterMask(0x1672, 0xF5F7, 0xffff);     // Mute
        // 166E ==> discharge
        HOST_WriteRegisterMask(0x1670, 0x00C0, 0xFFFF);     // AMP power on
        PalSleep(50);                                       // wait 100ms
        HOST_WriteRegisterMask(0x1670, 0x0000, 0xFFFF);     // DACIP power on
        PalSleep(50);                                       // wait 100ms
        HOST_WriteRegisterMask(0x166e, 0x0000, 0xFFFF);     // AMP charge
        PalSleep(100);

        value = volume_status >> 12;

        for (i = 15; i >= value; i--)
        {
            HOST_WriteRegisterMask(0x1672, ((i << 12) | (volume_status & 0xFFF)), 0xFFFF);
            PalSleep(1);
            //fix while loop issue
            if (i == 0)
                break;
        }

        //mmpAudioGetAttrib(MMP_AUDIO_ENGINE_ADDRESS, pAddress);
        //mmpAudioGetAttrib(MMP_AUDIO_ENGINE_LENGTH, &length);
        nResult = mmpAudioOpenEngine(AUD_SuspendEngineType, tAEncode);
    }
    else if (nSuspend == 1)
    {   // nSuspend
        AUD_SuspendEngineType = AUD_EngineType;
    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_ResetAudioEngineType();
    #endif
        value = volume_status >> 12;
        for (i = value; i <= 15; i++)
        {
            HOST_WriteRegisterMask(0x1672, ((i << 12) | (volume_status & 0xFFF)), 0xFFFF);
            PalSleep(1);
        }
        HOST_WriteRegisterMask(0x1672, 0xF5F7, 0xffff);                     // Mute
        HOST_WriteRegisterMask(MMIO_ISS_CODEC_INT_VOL, 0x1000, 0x1000);     // AMP discharge
        // hw need sleep 100 ms
        PalSleep(100);                                                      // wait 100 ms
        HOST_WriteRegisterMask(0x1670, 0x00C0, 0x00C0);                     // DACIP power down
        PalSleep(10);                                                       // wait 10ms
        HOST_WriteRegisterMask(0x1670, 0x10C0, 0x10C0);                     // AMP power down
    }
}

MMP_UINT16
mmpAudioGetAuidoProcessorWritingStatus()
{
    #if defined(__FREERTOS__)
    MMP_UINT16 nRegData;
    HOST_ReadRegister(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, &nRegData);
    return nRegData;
    #endif
}

void
mmpAudioSetAuidoProcessorWritingStatus(
    MMP_UINT16 nRegData)
{
    #if defined(__FREERTOS__)
    HOST_WriteRegister(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, nRegData);
    #endif
}

    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
// get audio codec api buffers
AUDIO_API MMP_UINT16 *
mmpAudioGetAudioCodecAPIBuffer(MMP_UINT32 *nLength)
{
    *nLength = Audio_Plugin_Message_Buflen;
    return (MMP_UINT16 *)&Audio_Plugin_Message_Buf;
}

    #endif

// return 0 : no encoded data, return 1:get encode data
AUDIO_API int
mmpAudioGetEncodeData(MMP_AUDIO_ENCODE_DATA *ptEncodeData)
{
    int nResult;
    MMP_ULONG bufEncSize       = 0;

    MMP_ULONG streamBufferSize = MMP_AUDIO_ENCODE_SIZE;
    unsigned int *pBuf;
    nResult = mmpAudioGetAvailableBufferLength(MMP_AUDIO_INPUT_BUFFER, &bufEncSize);
    if (bufEncSize >= MMP_AUDIO_ENCODE_SIZE)
    {
    #if defined(__FREERTOS__)
        //dc_invalidate();
        //or32_invalidate_cache((MMP_UINT8*)ptEncodeData->ptData, streamBufferSize);
    #endif
        nResult = mmpAudioReadStream((MMP_UINT8 *) gEncodeFrame, streamBufferSize, streamBufferSize);
        if (nResult == 0)
        {
            return 0;
        }
        pBuf                     = (unsigned int *)gEncodeFrame;
        ptEncodeData->nTimeStamp = gEncodeTimeStamp;
        gPreEncodeTime           = pBuf[0] / 1000;
        gEncodeTimeAdding       += pBuf[0] - (gPreEncodeTime * 1000);
        // adding one
        if (gEncodeTimeAdding > 1000)
        {
            gEncodeTimeStamp  += 1;
            gEncodeTimeAdding -= 1000;
        }
        ptEncodeData->nDataSize = pBuf[1];
        gEncodeTimeStamp       += gPreEncodeTime;
        if (ptEncodeData->nDataSize <= MMP_AUDIO_ENCODE_SIZE)
            memcpy(ptEncodeData->ptData, &gEncodeFrame[8], ptEncodeData->nDataSize);

        if (ptEncodeData->nDataSize == 0)
            sdk_msg(SDK_MSG_TYPE_INFO, "[Audio Driver] get encode data timeStamp %d, data size %d bufEncSize %d %d read %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", ptEncodeData->nTimeStamp, ptEncodeData->nDataSize, bufEncSize, gEncodeTimeAdding, nResult, ptEncodeData->ptData[0], ptEncodeData->ptData[1], ptEncodeData->ptData[2], ptEncodeData->ptData[3], ptEncodeData->ptData[4], ptEncodeData->ptData[5]);
        return 1;
    }
    else
    {
        return 0;
    }
}

AUDIO_API void
mmpAudioSetMP2EncChMode(MMP_INT ch)
{
    int mode;  // mode 0,ch 1 ; mode 1 ,ch 2
    mode = ch - 1;
    if (mode < 0 || mode > 1)
        return;

    #if defined(__FREERTOS__)
    HOST_WriteRegisterMask(MP2_ENCODER_PARAMETER, (mode << 0), (1 << 0));
    #endif
}

AUDIO_API void
mmpAudioSetMP2EncSampleRateMode(MMP_INT sampleRate)
{
    int mode;
    if (sampleRate == 48000)
        mode = 0;
    else if (sampleRate == 44100)
        mode = 1;
    else if (sampleRate == 32000)
        mode = 2;

    if (mode < 0 || mode > 2)
        return;

    #if defined(__FREERTOS__)
    HOST_WriteRegisterMask(MP2_ENCODER_PARAMETER, (mode << 1), (7 << 1));
    #endif
}

    #define TEST_MIXER
    #ifdef TEST_MIXER
// mixer buffer read/write pointer
static unsigned int gMixerRdptr;
static unsigned int gMixerWrptr;
static unsigned int gDigitalVolume = 5;
    #endif

// getMixerReadPorinter() , must implement
unsigned int getMixerReadPorinter()
{
    #ifdef TEST_MIXER
    return (unsigned int) gMixerRdptr;

    #else
    return 0;
    #endif
}

// getMixerWritePorinter() , must implement
unsigned int getMixerWritePorinter()
{
    #ifdef TEST_MIXER
        #if defined(__FREERTOS__)
    dc_invalidate();     // Flush Data Cache
        #endif

    return (unsigned int) gMixerWrptr;
    #else

    return 20000;
    #endif
}

// setMixerReadPorinter() , must implement
void setMixerReadPorinter(unsigned int nReadPointer)
{
    #ifdef TEST_MIXER
    gMixerRdptr = nReadPointer;
    #else

    #endif
}

// setMixerWritePorinter()
void setMixerWritePorinter(unsigned int nWritePointer)
{
    #ifdef TEST_MIXER

    gMixerWrptr = nWritePointer;
        #if defined(__FREERTOS__)
    dc_invalidate(); // Flush Data Cache
        #endif

    #else

    #endif
}

unsigned int getAudioVolume()
{
    return (unsigned int) gDigitalVolume;
}

void setAudioVolume(unsigned int volume)
{
    gDigitalVolume = volume;
}

#else // !defined(DISABLE_AUDIO_CODEC)
    #include "audio_null.c"
#endif // !defined(DISABLE_AUDIO_CODEC)