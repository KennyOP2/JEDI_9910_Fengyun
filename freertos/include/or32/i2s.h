/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for I2S Interface
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __I2S_H__
#define __I2S_H__

#include "mmio.h"

//#define I2S_TEST_MODE

/* Constan Declaration for I2S Interface */

/* MMIO setting for Project 220 or latter */
#define I2S_BUS_SELECT         (0x68000000 + 0x48)

#define MMIO_I2S_CLK_SETTING   0x0022
#define MMIO_I2S_ADC_SR_SET    0x1640
#define MMIO_I2S_DAC_SR_SET    0x1642
#define MMIO_I2S_CODEC_SET     0x1644
#define MMIO_I2S_IN_BASE_LO    0x1646
#define MMIO_I2S_IN_BASE_HI    0x1648
#define MMIO_I2S_IN_BUF_LEN    0x164A
#define MMIO_I2S_IN_BUF_RDPTR  0x164c
#define MMIO_I2S_IN_SET        0x164e
#define MMIO_I2S_OUT_BASE_LO   0x1650
#define MMIO_I2S_OUT_BASE_HI   0x1652
#define MMIO_I2S_OUT_BUF_LEN   0x1654
#define MMIO_I2S_OUT_BUF_WRPTR 0x1656
#define MMIO_I2S_OUT_SET       0x1658
#define MMIO_I2S_IN_BUF_WRPTR  0x165a
#define MMIO_I2S_OUT_BUF_RDPTR 0x165c
#define MMIO_I2S_ENGINE_OUT_REGISTER_STATUS 0x1660
#define MMIO_I2S_FADE_IN_OUT_CONTROL 0x1666
#define MMIO_I2S_CODEC_DAC_SET 0x1668
#define MMIO_I2S_CODEC_VOL     0x166a
#define MMIO_ISS_CODEC_INT_VOL 0x166e 

/* AMCLK, ACLK, ZCLK setting */
#define MMIO_AMCLK_SETTING   0x003a
#define MMIO_ACLK_SETTING    0x003c
#define MMIO_ZCLK_SETTING    0x003c

#define MMIO_AUDIO_REGISTER3 0x003e
#if defined(MM9070) || defined(MM9910)
#  define I2S_MMIO_BASE  0x50001680L
#else
#  define I2S_MMIO_BASE  0x80000000L
#endif // defined(MM9070) || defined(MM9910)

/* MMIO Mapping (I2S parameter) */
#define I2S_InBufBase  (I2S_MMIO_BASE + 0x00L)
#define I2S_OutBufBase (I2S_MMIO_BASE + 0x04L)
#define I2S_BufLen     (I2S_MMIO_BASE + 0x08L)
#define I2S_InRdPtr    (I2S_MMIO_BASE + 0x0CL)
#define I2S_OutWrPtr   (I2S_MMIO_BASE + 0x10L)
#define I2S_Setting    (I2S_MMIO_BASE + 0x14L)
#define I2S_Setting2   (I2S_MMIO_BASE + 0x18L)

/* MMIO Mapping (I2S status, read only) */
#define I2S_InWrPtr    (I2S_MMIO_BASE + 0x1CL)
#define I2S_OutRdPtr   (I2S_MMIO_BASE + 0x1CL)
#define I2S_STATUS     (I2S_MMIO_BASE + 0x80L)

/* MMIO 0x8000_0008 Buffer Length */
#define I2S_InBufLen     0
#define I2S_OutBufLen    16

/* MMIO 0x8000_0014 I2S Setting */
#define I2S_AudioInFire  0
#define I2S_EnAudioIn    1
#define I2S_InFormat     2
#define I2S_RecMode      3
#define I2S_InLeft       4
#define I2S_InChannel    5
#define I2S_InChSelect   6
#define I2S_AudioOutFire 8
#define I2S_EnAudioOut   9
#define I2S_OutFormat    10
#define I2S_OutLeft      11
#define I2S_OutChannel   12
#define I2S_EnInRqSize   13
#define I2S_InRqSize     14
#define I2S_EnOutRqSize  18
#define I2S_OutRqSize    19
#define I2S_OutWrPtrEnd  24
#define I2S_I2SReset     25

/* MMIO 0x8000_0018 I2S Setting */
#define I2S_EnInChkSum   12
#define I2S_EnOutChkSum  11
#define I2S_EnOutStep    10
#define I2S_OutStep      2
#define I2S_InBigEndian  1
#define I2S_OutBigEndian 0

/* MMIO 0x8000_0080 I2S Status */
#define I2S_InEmpty      0x00000002L
#define I2S_OutEmpty     0x00000004L

#if !defined(WIN32) && !defined(__CYGWIN__)
/* For PCM Mixer Input */
#define SetMixRdPtr(i) MMIO_Write(DrvMixerPCM_RdPtr,(short int)i)
#define GetMixWrPtr()  MMIO_Read(DrvMixerPCM_WrPtr)
#define GetMixRdPtr()  MMIO_Read(DrvMixerPCM_RdPtr)

/* For I2S ADC Input */
#ifdef DUMP_PCM_DATA
#  define SetInRdPtr(i) MMIO_Write(DrvEncodePCM_RdPtr,(short int)i)
#  define SetInWrPtr(i) MMIO_Write(DrvEncodePCM_WrPtr,(short int)i)
#  define GetInWrPtr()  MMIO_Read(DrvEncodePCM_WrPtr)
#elif defined(MM9070) || defined(MM9910)
#  define SetInRdPtr(i) MMIO_Write(MMIO_I2S_IN_BUF_RDPTR,(short int)i)
#  define SetInWrPtr(i) MMIO_Write(MMIO_I2S_IN_BUF_WRPTR,(short int)i)
#  define GetInWrPtr()  MMIO_Read(MMIO_I2S_IN_BUF_WRPTR)
#else
#  define SetInRdPtr(i) *(volatile unsigned int *)(I2S_InRdPtr) = (i)
#  define SetInWrPtr(i) *(volatile unsigned int *)(I2S_InWrPtr) = (i)
#  define GetInWrPtr()  (*(volatile unsigned int *)(I2S_InWrPtr) & 0x0000ffffL)
#endif

/* For I2S DAC Input */
#if defined(DUMP_PCM_DATA)
#  define SetOutWrPtr(i) MMIO_Write(DrvDecodePCM_WrPtr,(short int)i)
#  define SetOutRdPtr(i) MMIO_Write(DrvDecodePCM_RdPtr,(short int)i)
#  define GetOutRdPtr()  MMIO_Read(DrvDecodePCM_RdPtr)
#  define GetOutWrPtr()  MMIO_Read(DrvDecodePCM_WrPtr)
#elif defined(MM9070) || defined(MM9910)
#  define SetOutWrPtr(i) MMIO_Write(MMIO_I2S_OUT_BUF_WRPTR,((i)&0xfffc))
#  define SetOutRdPtr(i)
#  if defined(I2S_TEST_MODE) || defined(ENABLE_PERFORMANCE_MEASURE)
#    define GetOutRdPtr() GetOutWrPtr()
#  else
#    define GetOutRdPtr() ((unsigned short)MMIO_Read(MMIO_I2S_OUT_BUF_RDPTR))
#  endif
#  define GetOutWrPtr()   MMIO_Read(MMIO_I2S_OUT_BUF_WRPTR)
#else
#  define SetOutWrPtr(i) *(volatile unsigned int *)(I2S_OutWrPtr) = (i)
#  define SetOutRdPtr(i)
#  if defined(I2S_TEST_MODE) || defined(ENABLE_PERFORMANCE_MEASURE)
#    define GetOutRdPtr() GetOutWrPtr()
#  else
#    define GetOutRdPtr() ((*(volatile unsigned int *)(I2S_OutRdPtr) >> 16) & 0x0000ffffL)
#  endif // defined(I2S_TEST_MODE)
#  define GetOutWrPtr() ((*(volatile unsigned int *)(I2S_OutWrPtr)) & 0x0000ffffL)
#endif // defined(DUMP_PCM_DATA)
#endif // !defined(WIN32) && !defined(__CYGWIN__)

void pauseDAC(int flag);
void deactiveDAC(void);
void pauseADC(int flag);
void deactiveADC(void);
void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int isBigEndian);
void initADC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int isBigEndian);
void initCODEC(unsigned char *enc_buf, unsigned char *dec_buf, int nChannel, int sample_rate,
               int enc_buflen, int dec_buflen, int isBigEndian);
void initCODEC2(unsigned char *enc_buf, unsigned char *dec_buf,
                int nDecChannel, int nEncChannel, int sample_rate,
               int enc_buflen, int dec_buflen, int isBigEndian);
void enableFadeInOut();
void disableFadeInOut();
#endif /* __I2S_H__ */

