/***************************************************************************
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 *
 * @file
 * Codecs Code
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/

#if defined(ENABLE_CODECS_PLUGIN)
#include <string.h>
#include <stdio.h>
#include "aud/codecs.h"
#include "i2s.h"
#include "pal/thread.h"
#include "pal/timer.h"
#include "pal/file.h"
#include "mmp_aud.h"
#include "api_f.h"
#include "pal/heap.h"
#include "mmp_i2s.h"

int*   eqinfo;
int*   revbinfo;

#if defined(ENABLE_CODECS_ARRAY)
char codecbuf[CODEC_SIZE] __attribute__ ((section (".codecs_header"))) = {
#include "mp3.hex"
};
#else
char codecbuf[CODEC_SIZE] __attribute__ ((section (".codecs_header")));
#endif

#if defined(ENABLE_CODECS_ARRAY)

char mp2enc_codec[CODEC_ARRAY_SIZE] __attribute__ ((aligned(4))) = {
#include "mp2encode.hex"

// Initial Codec pointer
int *codecptr[] = {
    (int*)NULL,                 // Reserved

#if HAVE_MP3                    // MMP_MP3_DECODE     = 1
    (int*)&codecbuf,
#else
    (int*)NULL,
#endif

#if HAVE_AAC                    // MMP_AAC_DECODE     = 2
    (int*)&aac_codec,
#else
    (int*)NULL,
#endif

    (int*)NULL,                 // MMP_AACPLUS_DECODE = 3
    (int*)NULL,                 // MMP_BSAC_DECODE    = 4

#if HAVE_WMA                    // MMP_WMA_DECODE     = 5
    (int*)&wma_codec,
#else
    (int*)NULL,
#endif

    (int*)NULL,                 // MMP_AMR_ENCODE     = 6
    (int*)NULL,                 // MMP_AMR_DECODE     = 7

#if HAVE_AMR                    // MMP_AMR_CODEC      = 8
    (int*)&amr_codec,
#else
    (int*)NULL,
#endif

    (int*)NULL,                 // MMP_MIXER          = 9
    (int*)NULL,                 // MMP_MIDI           = 10
    (int*)NULL,                 // MMP_PCM_CODEC      = 11

#if HAVE_WAV                    // MMP_WAV_DECODE     = 12
    (int*)&wav_codec,
#else
    (int*)NULL,
#endif

#if HAVE_AC3                    // MMP_AC3_DECODE     = 13
    (int*)&ac3_codec,
#else
    (int*)&ac3_spdif,
#endif

    (int*)NULL,                 // MMP_OGG_DECODE     = 14
    (int*)&mp2enc_codec,    
    (int*)NULL,                 // Reserved           = 15
};
#endif // defined(ENABLE_CODECS_ARRAY)

#ifndef ENABLE_AUDIO_PROCESSOR
void yield(void) 
{
    taskYIELD(); // taskYIELD is macro, not a real function body
}
#else
void yield(void) 
{

}
#endif

void NULL_FUN() 
{
}

#ifndef ENABLE_AUDIO_PROCESSOR
void MySleep(unsigned long ms) 
{
    PalSleep(ms);
}
PAL_CLOCK_T MyGetClock()
{
    return PalGetClock();
}
MMP_ULONG MyGetDuration()
{
    return 0;
}
int MyGetSysClock()
{
    return or32_getSysCLK();
}

#else
void MySleep(unsigned long ms)
{
    int i = 0;
    //for(i=0; i<ms*133000/5; i++);
    if (ms >50)
    {
        printf("sleep ms %d \n",ms);
    }
    for(i=0; i<ms*1000; i++) asm("");
}
PAL_CLOCK_T MyGetClock()
{
    return 0;
}
MMP_ULONG MyGetDuration()
{
    return 0;
}
int MyGetSysClock()
{
    return or32_getSysCLK();
}

#endif

#if defined(__FREERTOS__)
struct _codec_api codec_api = {
    (char*)&eqinfo,
    (char*)&revbinfo,

    /* kernel/system */
    MySleep,
    MyGetClock,
    MyGetDuration,
    MyGetSysClock,    
    NULL, //PalWFileOpen,
    NULL, //PalFileWrite,
    NULL, //PalFileClose,        
    NULL, //f_enterFS,    
#ifdef  SMTK_PRINT_MEM_USAGE
    dbg_PalHeapAlloc,
    dbg_PalHeapFree,
#else
    PalHeapAlloc,
    PalHeapFree, 
#endif
    yield,
    or32_invalidate_cache,
    dc_invalidate,

    /* IIS */
    NULL,//initDAC,
    NULL,//initADC,
    NULL,//initCODEC,
    NULL, //pauseDAC,
    NULL, //pauseADC,
    NULL, //deactiveDAC,
    NULL, //deactiveADC,
    I2S_AD32_GET_RP,
    I2S_AD32_GET_WP,
    I2S_AD32_SET_RP,
    getMixerReadPorinter,
    getMixerWritePorinter,
    setMixerReadPorinter,
    getAudioVolume,
    printf,
};
#else
struct _codec_api codec_api = {
    /* kernel/system */
    NULL, // PalSleep,
    NULL, //PalGetClock
    NULL, //PalGetDuration
    NULL, //PalGetSysClock
    NULL, //PalTFileOpen
    NULL, //PalTFileWrite
    NULL, //PalTFileClose  
    NULL, //f_enterFS
    NULL,//PalHeapAlloc
    NULL,//PalHeapFree
    NULL, // taskYIELD,
    NULL, // or32_invalidate_cache,
    NULL, // dc_invalidate,

    /* IIS */
    NULL, // initDAC,
    NULL, // initADC,
    NULL, // initCODEC,
    NULL, // pauseDAC,
    NULL, // pauseADC,
    NULL, // deactiveDAC,
    NULL, // deactiveADC,
    NULL, // I2S_AD32_GET_RP,
    NULL, // I2S_AD32_GET_WP,
    NULL, // I2S_AD32_SET_RP,
    NULL, // getMixerReadPorinter,
    NULL, // getMixerWritePorinter
    NULL, // setMixerReadPorinter
    NULL, // getAudioVolume
    NULL, // printf,
};
#endif // __FREERTOS__

#endif // ENABLE_CODECS_PLUGIN
