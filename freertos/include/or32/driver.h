/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Driver Interface
 *
 * @author Kuoping Hsu
 * @date 2006.07.28.
 * @version 1.0
 *
 */

#ifndef __DRIVER_H__
#define __DRIVER_H__

#define TO_INT(x)   (((((x)>> 0)&0xff) << 24) + ((((x)>> 8)&0xff) << 16) + \
                     ((((x)>>16)&0xff) <<  8) + ((((x)>>24)&0xff) <<  0) )
#define TO_SHORT(x) ((((short)(x) >> 8) & 0xff) + (((short)(x) << 8) & 0xff00))

typedef struct _SHAREINFO {
    int         tag;            // tag for endian type.

    // Constant Value
    int         reverbLength;   // Number of bytes requirement for Reverb. function.
    int         decBufOffset;   //
    int         minDecBufSize;  // minimun size of decoding buffer.
    int         minEncBufSize;  // minimun size of encoding buffer.
    int         minMixBufSize;  // minimun size of mixing buffer.
    int         inBufBase;      // Address of I2S input buffer. (For encoding)
    int         inBufLen;       // Length of I2S input buffer.
    int         outBufBase;     // Address of I2S output buffer. (For decoding)
    int         outBufLen;      // Length of I2S output buffer.

    // Driver update only
    int         decBufBase;     // Address of Decoding buffer.
    int         decBufLength;   // Bytes of decoding buffer. Max 64KBytes.
    int         encBufBase;     // Address of Encoding buffer.
    int         encBufLength;   // Bytes of encoding buffer. Max 64KBytes.
    int         mixBufBase;     // Address of mixer buffer.
    int         mixBufLength;   // Bytes of mixer buffer. Max 64KBytes.
    int         mixNChannels;   // Number of channels on mixer input.
    int         mixSampleRate;  // Sample rate on mixer input.
    int         reverbBase;     // Address of Reverb buffer.

    // OpenRISC update only
    int         decodeTime;     // Decoding time in seconds, S16.16 format.
    int         encodeTime;     // Encoding time in seconds, S16.16 format.

    int         sampleRate;     // sample rate
    int         nChanels;       // Number of chanels
} SHAREINFO;

#endif // __DRIVER_H__

