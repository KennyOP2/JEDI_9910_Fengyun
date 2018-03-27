/*
 * Copyright (c) 2005 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia Audio Driver API header file.
 *
 * @author Peter Lin
 * @version 1.0
 */
#ifndef MMP_AUDIO_H
#define MMP_AUDIO_H

#include "mmp_types.h"

extern MMP_BOOL g_isMute;

#if defined(_WIN32)
    #if defined(AUD_EXPORTS)
        #define AUDIO_API __declspec(dllexport)
    #else
        #define AUDIO_API __declspec(dllimport)
    #endif
#else
    #define AUDIO_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#define MMP_AUDIO_PCM_FORMAT        0x2000
#define MMP_AUDIO_MP3_FORMAT        0x2001
#define MMP_AUDIO_AAC_FORMAT        0x2002
#define MMP_AUDIO_AMR_FORMAT        0x2003
#define MMP_AUDIO_MID_FORMAT        0x2004
#define MMP_AUDIO_BSAC_FORMAT       0x2005
#define MMP_AUDIO_AC3_FORMAT        0x2006
#define MMP_AUDIO_NIL_FORMAT        0x7FFF

#define MMP_AUDIO_AMR_ENCODE_475    (0x0 << 11)
#define MMP_AUDIO_AMR_ENCODE_515    (0x1 << 11)
#define MMP_AUDIO_AMR_ENCODE_590    (0x2 << 11)
#define MMP_AUDIO_AMR_ENCODE_670    (0x3 << 11)
#define MMP_AUDIO_AMR_ENCODE_740    (0x4 << 11)
#define MMP_AUDIO_AMR_ENCODE_795    (0x5 << 11)
#define MMP_AUDIO_AMR_ENCODE_1020   (0x6 << 11)
#define MMP_AUDIO_AMR_ENCODE_1220   (0x7 << 11)

//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================
//  Type Definition
//=============================
#define MMP_AUDIO_ENCODE_SIZE    1500
//#define MMP_AUDIO_ENCODE_BUFFER_SIZE 48000
//=============================
//  Enumeration Type Definition
//=============================
/**
 * The structure defines audio engine type
 */
typedef enum MMP_AUDIO_ENGINE_TAG
{
    MMP_MP3_DECODE     =  1,/**< MP3 decode engine */
    MMP_AAC_DECODE     =  2,/**< AAC decode engine */
    MMP_AACPLUS_DECODE =  3,/**< AAC plus decode engine */
    MMP_BSAC_DECODE    =  4,/**< BSAC decode engine */
    MMP_WMA_DECODE     =  5,/**< WMA decode engine */
    MMP_AMR_ENCODE     =  6,/**< AMR encode engine */
    MMP_AMR_DECODE     =  7,/**< AMR decode engine */
    MMP_AMR_CODEC      =  8,/**< AMR encode/decode engine */
    MMP_MIXER          =  9,/**< MIXER engine */
    MMP_MIDI           = 10,/**< MIDI engine */
    MMP_PCM_CODEC      = 11,/**< PCM engine */
    MMP_WAV_DECODE     = 12,/**< WAV (PCM16/PCM8/a-law/mu-law engine */
    MMP_AC3_DECODE     = 13,/**< AC3 engine */
    MMP_OGG_DECODE     = 14,/**< OGG Vorbis engine */
    MMP_AC3_SPDIF_DECODE = 15,/**< AC3 SPDIF engine */    
    MMP_MP2_ENCODE =16,
    MMP_AAC_ENCODE =17,    
    MMP_RESERVED       = 18
} MMP_AUDIO_ENGINE;

typedef enum MMP_AUDIO_ENGINE_ATTRIB_TAG
{
    MMP_AUDIO_ENGINE_TYPE               = 0,
    MMP_AUDIO_ENGINE_STATUS             = 1
} MMP_AUDIO_ENGINE_ATTRIB;

typedef enum MMP_AUDIO_ENGINE_STATE_TAG
{
    MMP_AUDIO_ENGINE_IDLE               = 0,
    MMP_AUDIO_ENGINE_RUNNING            = 1,
    MMP_AUDIO_ENGINE_STOPPING           = 2,
    MMP_AUDIO_ENGINE_NO_RESPONSE        = 3,
    MMP_AUDIO_ENGINE_UNKNOW             = 4,
    MMP_AUDIO_ENGINE_ERROR              = 9
} MMP_AUDIO_ENGINE_STATE;

/**
 * The structure defines codec parameters
 */
typedef enum MMP_AUDIO_CODEC_ATTRIB_TAG
{
    MMP_AUDIO_CODEC_NONE                = 0,
    MMP_AUDIO_CODEC_INITIALIZE          = 1,
    MMP_AUDIO_CODEC_TERMINATE           = 2,
    MMP_AUDIO_CODEC_SETVOLUME           = 3,
    MMP_AUDIO_CODEC_MODE                = 4,
    MMP_AUDIO_CODEC_ADC_RATIO           = 5,
    MMP_AUDIO_CODEC_DAC_RATIO           = 6,
    MMP_AUDIO_CODEC_OVERSAMPLING_RATE   = 7
} MMP_AUDIO_CODEC_ATTRIB;

typedef enum MMP_AUDIO_CODEC_MODE_TYPE_TAG
{
    MMP_AUDIO_CODEC_ADMS_DAMS_MODE,     // ADC master / DAC master mode
    MMP_AUDIO_CODEC_ADSL_DASL_MODE,     // ADC slave  / DAC slave  mode
    MMP_AUDIO_CODEC_ADMS_DASL_MODE,     // ADC master / DAC slave  mode
    MMP_AUDIO_CODEC_ADSL_DAMS_MODE,     // ADC slave  / DAC master mode
    MMP_AUDIO_CODEC_CODEC0_MS_MODE,     // Codec 0 master mode
    MMP_AUDIO_CODEC_CODEC0_SL_MODE,     // Codec 0 slave  mode
    MMP_AUDIO_CODEC_CODEC1_MS_MODE,     // Codec 1 master mode
    MMP_AUDIO_CODEC_CODEC1_SL_MODE,     // Codec 1 slave  mode
    MMP_AUDIO_CODEC_BT_ONLY_MSB_MODE,   // Bluetooth only Long  frame (MSB) mode
    MMP_AUDIO_CODEC_BT_ONLY_I2S_MODE,   // Bluetooth only short frame (I2S) mode
    MMP_AUDIO_CODEC_BT_DAC_MSB_MODE,    // Bluetooth with DAC long  frame (MSB) mode
    MMP_AUDIO_CODEC_BT_DAC_I2S_MODE,    // Bluetooth with DAC short frame (I2S) mode
    MMP_AUDIO_CODEC_BT_ADC_MSB_MODE,    // ADC with Bluttooth long  frame (MSB) mode
    MMP_AUDIO_CODEC_BT_ADC_I2S_MODE     // ADC with Bluttooth short frame (I2S) mode
} MMP_AUDIO_CODEC_TYPE_MODE;

typedef enum MMP_AUDIO_BUFFER_TYPE_TAG
{
    MMP_AUDIO_INPUT_BUFFER,
    MMP_AUDIO_OUTPUT_BUFFER,
    MMP_AUDIO_MIXER_BUFFER
} MMP_AUDIO_BUFFER_TYPE;

typedef enum {
    MMP_WAVE_FORMAT_PCM                 = 0, /* Microsoft PCM Format */
    MMP_WAVE_FORMAT_ALAW                = 1, /* Microsoft ALAW */
    MMP_WAVE_FORMAT_MULAW               = 2, /* Microsoft MULAW */
    MMP_WAVE_FORMAT_DVI_ADPCM           = 3
} MMP_WAVE_FORMAT;

typedef enum {
    MMP_WAVE_SRATE_6000                 = 0,
    MMP_WAVE_SRATE_8000                 = 1,
    MMP_WAVE_SRATE_11025                = 2,
    MMP_WAVE_SRATE_12000                = 3,
    MMP_WAVE_SRATE_16000                = 4,
    MMP_WAVE_SRATE_22050                = 5,
    MMP_WAVE_SRATE_24000                = 6,
    MMP_WAVE_SRATE_32000                = 7,
    MMP_WAVE_SRATE_44100                = 8,
    MMP_WAVE_SRATE_48000                = 9
} MMP_WAVE_SRATE;

typedef enum {
    AUDIO_INPUT_TYPE_PCM                 = 0,
    AUDIO_INPUT_TYPE_MP3                 = 1,
    AUDIO_INPUT_TYPE_AAC                = 2,
    AUDIO_INPUT_TYPE_AC3                = 3,
    AUDIO_INPUT_TYPE_DTS                 = 4
} MMP_AUDIO_INPUT_TYPE;

//=============================
//  Structure Type Definition
//=============================

/**
 * The structure defines audio attributes.
 */
typedef enum MMP_AUDIO_ATTRIB_TAG
{
    //MMP_AUDIO_DEVICE_STATUS,

    //MMP_AUDIO_STREAM_MODES,
    //MMP_AUDIO_STREAM_PCM_TIME,
    //MMP_AUDIO_STREAM_ENC_TIME,
    //MMP_AUDIO_STREAM_DEC_TIME,
    //MMP_AUDIO_STREAM_PCM_AVAIL_LENGTH,
    //MMP_AUDIO_STREAM_DEC_AVAIL_LENGTH,
    //MMP_AUDIO_STREAM_DEC_I2S_AVAIL_LENGTH,
    //MMP_AUDIO_STREAM_ENC_AVAIL_LENGTH,
    //MMP_AUDIO_STREAM_ENC_I2S_AVAIL_LENGTH,
    //MMP_AUDIO_STREAM_TYPE,
    MMP_AUDIO_STREAM_SAMPLERATE,
    //MMP_AUDIO_STREAM_BITRATE,
    MMP_AUDIO_STREAM_CHANNEL,
    //MMP_AUDIO_STREAM_FREQUENCY_INFO,
    //MMP_AUDIO_STREAM_PLAYBACKRATE,

    //MMP_AUDIO_EFFECT_EN_DRC,
    //MMP_AUDIO_EFFECT_EN_EQUALIZER,
    //MMP_AUDIO_EFFECT_EN_REVERB,
    //MMP_AUDIO_EFFECT_EN_VOICE,
    //MMP_AUDIO_EFFECT_EQ_TYPE,
    //MMP_AUDIO_EFFECT_EQ_TABLE,
    //MMP_AUDIO_EFFECT_REVERB_TYPE,
    //MMP_AUDIO_EFFECT_REVERB_TABLE,
    MMP_AUDIO_PLUGIN_PATH,
    MMP_AUDIO_GET_IS_EOF,    
    MMP_AUDIO_CODEC_SET_SAMPLE_RATE,        
    MMP_AUDIO_CODEC_SET_CHANNEL,            
    MMP_AUDIO_MUSIC_PLAYER_ENABLE,                
    MMP_AUDIO_PTS_SYNC_ENABLE,                    
    MMP_AUDIO_ENGINE_ADDRESS,
    MMP_AUDIO_ENGINE_LENGTH,
    MMP_AUDIO_I2S_INIT,
    MMP_AUDIO_FADE_IN,
    MMP_AUDIO_I2S_OUT_FULL,
    MMP_AUDIO_ENABLE_MPLAYER,
    MMP_AUDIO_ADJUST_MPLAYER_PTS,
    MMP_AUDIO_MPLAYER_STC_READY,
    MMP_AUDIO_DECODE_ERROR,
    MMP_AUDIO_DECODE_DROP_DATA,
    MMP_AUDIO_PAUSE_STC_TIME,
    MMP_AUDIO_IS_PAUSE,
    MMP_AUDIO_ENABLE_STC_SYNC,
    MMP_AUDIO_DIRVER_DECODE_BUFFER_LENGTH,
    MMP_AUDIO_SPDIF_NON_PCM,
    MMP_AUDIO_ENCODE_START_TIME,
    MMP_AUDIO_NULL_ATTRIB
} MMP_AUDIO_ATTRIB;

typedef struct MMP_WaveInfo_TAG {
    MMP_WAVE_FORMAT format;
    MMP_UINT nChans;
    MMP_UINT sampRate;
    MMP_UINT bitsPerSample;
    void*        pvData;
    MMP_UINT nDataSize;
} MMP_WaveInfo;

typedef struct MMP_AUDIO_ENCODE_DATA_TAG
{
    unsigned int    nTimeStamp;
    unsigned int    nDataSize;    
    char*             ptData;
}MMP_AUDIO_ENCODE_DATA;

typedef struct AUDIO_ENCODE_PARAM_TAG
{
    int nInputBufferType;  // 0: buffer from audio encoder, 1:buffer from other RISC AP, 2 : buffer from I2S
    int nEnableMixer; // 0: not use mixer
    unsigned int nStartPointer; // if nInputBufferType == 2, it can setup start pointer
    unsigned int nStartTime; // ms, time offset
    unsigned char* pInputBuffer;   // I2S has most 5 input buffer pointers
    unsigned char* pInputBuffer1; 
    unsigned char* pInputBuffer2;
    unsigned char* pInputBuffer3;
    unsigned char* pInputBuffer4;
    unsigned char* pMixBuffer; // buffer to be mixed    
    unsigned int nBufferLength;  // the length of the input buffer
    unsigned int nMixBufferLength;  // the length of the mix buffer    
    unsigned int nChannels; // most support to 7.1
    unsigned int nSampleRate;  // input sample rate
    unsigned int nOutSampleRate;  // output sample rate;
    unsigned int nBitrate;  // encode bitrate
    int nInputAudioType; // 0:PCM
    int nSampleSize; // 16 or 32, if audio type == pcm
}AUDIO_ENCODE_PARAM;
typedef enum MMP_AUDIO_MODE_TAG
{
    MMP_AUDIO_STEREO = 0,
    MMP_AUDIO_LEFT_CHANNEL,
    MMP_AUDIO_RIGHT_CHANNEL,
    MMP_AUDIO_MIX_LEFT_RIGHT_CHANNEL
} MMP_AUDIO_MODE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group8 SMedia Audio Driver API
 *  The supported API for audio.
 *  @{
 */

/**
 * Initialize audio task.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application must call this API when the OS is booting.
  */
AUDIO_API MMP_RESULT mmpAudioInitializeTask(void);

/**
 * Initialize audio engine.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application must call this API first when it want to use audio API.
 * Before mmpAudioTerminate() this API will be used once only.
 */
AUDIO_API MMP_RESULT mmpAudioInitialize(void);

/**
 * Initialize codec
 * @param  attribList
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application must call this API first when it want to use audio API.
 * Before mmpAudioTerminate() this API will be used once only.
 */
AUDIO_API MMP_RESULT mmpAudioSetCodec(MMP_ULONG* attribList);

/**
 * Specifies which type audio engine will be used.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application must call this API to Specifies which type audio engine is needed.
 * Before mmpAudioStopEngine() this API will be used once only.
 */
AUDIO_API MMP_RESULT mmpAudioOpenEngine(MMP_AUDIO_ENGINE audio_type,AUDIO_ENCODE_PARAM tAudioEncode);

/**
 * Active audio engine to start encode or decode except chosen MIDI engine.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark IF application choice decode function, audio engine will start to decode stream data inputted by mmpAudioWriteStream().
 * IF application choice encode function, audio engine will start to record and encode data
 * , and them application can use mmpAudioReadStream() to get encoded data.
 * IF application choice MIDI engine,  mmpAudioActiveMidiEngine() is desired to active midi engine.
 */
AUDIO_API MMP_RESULT mmpAudioActiveEngine(void);

/**
 * Get current running engine attribute
 *
 * @param attrib            Specifies engine attribute to get
 * @param value             Pointer to attribute value
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
AUDIO_API MMP_RESULT mmpAudioGetEngineAttrib(MMP_AUDIO_ENGINE_ATTRIB attrib, void* value);

/**
 * Get available buffer length
 *
 * @param bufferType        Specifies buffer type
 * @param bufferLength      Pointer to get buffer size
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Now audio engine will play back MIDI.
 */
AUDIO_API MMP_RESULT mmpAudioGetAvailableBufferLength(MMP_AUDIO_BUFFER_TYPE bufferType, MMP_ULONG* bufferLength);

/**
 * Input stream data for audio engine to play back.
 *
 * @param stream                Specifies stream data buffer.
 * @param streamBufferSize      Specifies stream buffer size .
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Audio engine play back stream data, the data format is according to audio engine type chosen by mmpAudioOpenEngine() API.
 */
AUDIO_API MMP_RESULT mmpAudioWriteStream(MMP_UINT8* stream, MMP_ULONG streamBufferSize);

/**
 * Get encoded stream data from audio engine.
 *
 * @param stream                Specifies stream data buffer.
 * @param streamBufferSize      Specifies stream buffer size.
 * @param streamthreshold       Specifies threshold value to avoid waitting.
 * @return encoded size.
 * @remark Audio engine record and encode data according to audio engine type chosen by mmpAudioOpenEngine() API.
 * Application can specify threshold value to avoid waitting, if the encoded data is smaller than threshold,
 * the function will not get enocded data and return zero.
 */
AUDIO_API MMP_ULONG mmpAudioReadStream(MMP_UINT8* stream, MMP_ULONG streamBufferSize, MMP_ULONG streamthreshold);

/*
 *  Set Wave mode when doing Decore(0) or Encode(1)
 *
*/ 
AUDIO_API MMP_RESULT
mmpAudioSetWaveMode( 
          MMP_INT mode);

AUDIO_API MMP_RESULT
mmpAudioSetWaveEncode(
        MMP_INT nType,MMP_INT nChannels , MMP_INT nSampleRate
        );

/**
 * Tell engine now is end of a song.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application can call this API to tell audio engine this is end of song,
 * and engine can prepare for beginner of next song.
 * Application do not need to call this API when encode or MIDI engine is choosen.
 */
AUDIO_API MMP_RESULT mmpAudioEndStream(void);

/**
 * Stop audio engine.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Audio engine will stop immediately when this API is called.
 * Application can use mmpAudioActiveEngine() or mmpAudioActiveMidiEngine() to restart engine.
 */
AUDIO_API MMP_RESULT mmpAudioStopEngine(void);

/**
 * Get encoding frame ID of audio engine working now.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application can use this API to know audio time form starting encoding.
 */
AUDIO_API MMP_RESULT mmpAudioGetEncodeTime(MMP_UINT *time);

/**
 * Get decoding frame ID of audio engine working now.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application can use this API to know audio time form starting decoding .
 */
AUDIO_API MMP_RESULT mmpAudioGetDecodeTime(MMP_UINT *time);

AUDIO_API MMP_RESULT mmpAudioGetDecodeTimeV2(MMP_UINT *time);
AUDIO_API MMP_RESULT mmpAudioSetAttrib(const MMP_AUDIO_ATTRIB attrib, const void* value);
AUDIO_API MMP_RESULT mmpAudioGetAttrib(const MMP_AUDIO_ATTRIB attrib,void* value);


/**
 * Set volume of audio
 *
 * @param volume            specifies the volume
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application can use this API to create reverbration effect during decoding process.
 */
AUDIO_API MMP_RESULT mmpAudioSetVolume(MMP_UINT volume);

/**
 * Get current volume of audio
 *
 * @param volume            specifies the volume pointer
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Application can use this API to create reverbration effect during decoding process.
 */
AUDIO_API MMP_RESULT mmpAudioGetVolume(MMP_UINT* volume);


/**
 *  Audio terminate function.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark After call this function, application can not use any audio API expext mmpAudioInitialize().
 */
AUDIO_API MMP_RESULT mmpAudioTerminate(void);

AUDIO_API MMP_RESULT
mmpAudioPause(
    MMP_BOOL enable);

AUDIO_API MMP_RESULT
mmpAudioSTCSyncPause(
    MMP_BOOL enable);

AUDIO_API MMP_RESULT
mmpAudioSeek(
    MMP_BOOL enable);   

AUDIO_API MMP_RESULT
mmpAudioStop(
    void);

AUDIO_API MMP_RESULT
mmpAudioStopImmediately(
    void);


AUDIO_API MMP_RESULT
mmpAudioPowerOnAmplifier(
    void);

AUDIO_API MMP_RESULT
mmpAudioPowerOffAmplifier(
    void);

AUDIO_API MMP_RESULT
mmpAudioSetWaveDecodeHeader(MMP_WaveInfo wavInfo);


/**
 *  Set the Audio play position.
 *
 * @param pos         specifies the current file position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark After call this function, application can not use any audio API expext mmpAudioInitialize().
 */
AUDIO_API MMP_RESULT mmpAudioSetPlayPos(MMP_INT32 pos);

/**
 *  Get the Audio play position.
 *
 * @param pos         get the current file position which is decoded.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark After call this function, application can not use any audio API expext mmpAudioInitialize().
 */
AUDIO_API MMP_RESULT mmpAudioGetPlayPos(MMP_INT32* pos);

AUDIO_API MMP_RESULT
mmpAudioSetPts(
    MMP_UINT32 pts);

AUDIO_API void
mmpAudioSetEarPhonePlugin(MMP_BOOL detect);

AUDIO_API MMP_BOOL
mmpAudioGetEarPhonePlugin(void);

AUDIO_API void
mmpAudioTurnOnStereoToMono(
    void);

AUDIO_API void
mmpAudioTurnOffStereoToMono(
    void);
    
AUDIO_API MMP_AUDIO_MODE
mmpAudioGetMode(void);

AUDIO_API void
mmpAudioSetMode(MMP_AUDIO_MODE mode); 

AUDIO_API void
mmpAudioSetWaveDecEndian(
    MMP_INT mode);

AUDIO_API void
mmpAudioSetShowSpectrum(
    MMP_INT mode);

AUDIO_API void
mmpAudioSetUpSampling(
    MMP_INT nEnable);

AUDIO_API void
mmpAudioSetUpSamplingOnly2x(
    MMP_INT nEnable);
   
AUDIO_API void
mmpAudioSuspendEngine();

AUDIO_API MMP_UINT16
mmpAudioGetAuidoProcessorWritingStatus();

AUDIO_API void
mmpAudioSetAuidoProcessorWritingStatus(
    MMP_UINT16 nRegData);

AUDIO_API MMP_RESULT
mmpAudioSetMPlayerPts(
    MMP_INT pts);


#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
// get audio codec api buffers
AUDIO_API MMP_UINT16*
mmpAudioGetAudioCodecAPIBuffer(MMP_UINT32* nLength);
#endif

// get audio encode data
AUDIO_API int
mmpAudioGetEncodeData(MMP_AUDIO_ENCODE_DATA* ptEncodeData);
    
AUDIO_API void
mmpAudioSetMP2EncChMode(MMP_INT ch);

AUDIO_API void
mmpAudioSetMP2EncSampleRateMode(MMP_INT sampleRate);

// getMixerReadPorinter()
unsigned int getMixerReadPorinter();

// getMixerWritePorinter()
unsigned int getMixerWritePorinter();

// setMixerReadPorinter()
void setMixerReadPorinter(unsigned int nReadPointer);
void setMixerWritePorinter(unsigned int nWritePointer);

unsigned int getAudioVolume();

void setAudioVolume(unsigned int volume);

///////////////////////////////////////////////////////////////////////////////
//@}

#ifdef __cplusplus
}
#endif

#endif //MMP_AUDIO_H//
