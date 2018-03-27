#ifndef __PLUGIN_H__
#define __PLUGIN_H__

typedef struct AUDIO_ENCODE_PARAM_TAG
{
    int nInputBufferType;  // 0: buffer from audio encoder, 1:buffer from other RISC AP, 2 : buffer from I2S
    int nEnableMixer; // 0: not use mixer    
    unsigned int nStartPointer; // if nInputBufferType == 2, it can setup start pointer
    unsigned int nStartTime; // time offset    
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

#if defined(ENABLE_CODECS_PLUGIN)
#  include "aud/codecs.h"
extern struct _codec_api *ci;
#  undef  taskYIELD
#  define PalSleep              ci->sleep
#  define PalGetClock           ci->PalGetClock
#  define PalGetDuration        ci->PalGetDuration
#  define PalGetSysClock        ci->PalGetSysClock
#  define PalWFileOpen          ci->PalWFileOpen  
#  define PalFileWrite          ci->PalFileWrite
#  define PalFileClose          ci->PalFileClose
#  define f_enterFS             ci->f_enterFS
#  define PalHeapAlloc          ci->PalHeapAlloc
#  define PalHeapFree           ci->PalHeapFree
#  define taskYIELD             ci->yield
#  define deactiveDAC           ci->deactiveDAC
#  define deactiveADC           ci->deactiveADC
#  define pauseDAC              ci->pauseDAC
#  define pauseADC              ci->pauseADC
#  define initDAC               ci->initDAC
#  define initADC               ci->initADC
#  define initCODEC             ci->initCODEC
#  define or32_invalidate_cache ci->flush_dcache
#  define dc_invalidate         ci->flush_all_dcache
#  define I2S_AD32_GET_RP       ci->I2S_AD32_GET_RP
#  define I2S_AD32_GET_WP       ci->I2S_AD32_GET_WP
#  define I2S_AD32_SET_RP       ci->I2S_AD32_SET_RP
#  define getMixerReadPorinter  ci->getMixerReadPorinter
#  define getMixerWritePorinter  ci->getMixerWritePorinter
#  define setMixerReadPorinter   ci->setMixerReadPorinter
#  define getAudioVolume         ci->getAudioVolume
#  define printf                //ci->printf
#  define MP2Encode_GetBufInfo      codec_info
#endif


#endif // __PLUGIN_H__
