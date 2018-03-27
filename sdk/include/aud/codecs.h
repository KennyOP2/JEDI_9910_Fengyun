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
#ifndef __CODECS_H__
#define __CODECS_H__


#define ENABLE_AUDIO_PROCESSOR

#if defined(__FREERTOS__)
#  include "FreeRTOS.h"
#  include "task.h"
#  include "pal/timer.h"
#  include "pal/pal.h"
#  include "pal/file.h"
#endif

#include "codecs_defs.h"

/* NOTE: To support backwards compatibility, only add new functions at
         the end of the structure.  Every time you add a new function,
         remember to increase CODEC_API_VERSION.
 */
struct _codec_api {
    /* global variable from DPF AP */
    void   *eqinfo;
    void   *revbinfo;

    /* kernel/ system */
    void   (*sleep)     (unsigned long ms);
    PAL_CLOCK_T    (*PalGetClock) (void);
    MMP_ULONG   (*PalGetDuration) (PAL_CLOCK_T clock);
    int    (*PalGetSysClock) (void);    
    PAL_FILE* (*PalWFileOpen) (const MMP_WCHAR* filename,MMP_UINT mode,PAL_FILE_CALLBACK callback);    
    MMP_SIZE_T (*PalFileWrite)(const void* buffer,MMP_SIZE_T size,MMP_SIZE_T count,PAL_FILE* stream,PAL_FILE_CALLBACK callback);
    MMP_INT (*PalFileClose)(PAL_FILE* stream,PAL_FILE_CALLBACK callback);
    int (*f_enterFS) (void);    
    void* (*PalHeapAlloc) (MMP_INT name,MMP_SIZE_T size);
    void (*PalHeapFree) (MMP_INT name,void* ptr);
    void   (*yield)     (void);
    void   (*flush_dcache)(void *start, int bytes);
    void   (*flush_all_dcache)(void);

    /* IIS */
    void   (*initDAC)   (unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);
    void   (*initADC)   (unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);
    void   (*initCODEC) (unsigned char *in, unsigned char *out, int nChannel, int sample_rate, int inbuflen, int outbuflen, int bigendian);
    void   (*pauseDAC)  (int flag);
    void   (*pauseADC)  (int flag);
    void   (*deactiveDAC)(void);
    void   (*deactiveADC)(void);
    unsigned (*I2S_AD32_GET_RP)(void);
    unsigned (*I2S_AD32_GET_WP)(void);
    void (*I2S_AD32_SET_RP)(unsigned RP32);
    unsigned int (*getMixerReadPorinter)  (void);
    unsigned int (*getMixerWritePorinter)  (void);
    void (*setMixerReadPorinter)  (unsigned int nReadPointer);
    unsigned int (*getAudioVolume)  (void);
    int    (*printf)    (const char *, ...);
};

struct _codec_header {
    unsigned long  magic;
    unsigned short target_id;
    unsigned short api_version;
    unsigned char *load_addr;
    unsigned char *end_addr;
    int (*entry_point)(struct _codec_api*);
    int (*codec_info)();
};

#endif // __CODECS_H__

