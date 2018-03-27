/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file mmp_spdif.h
 * API to control SPDIF module.
 * @author Steven Hsiao.
 * @version 0.1
 */

#ifndef MMP_SPDIF_H
#define MMP_SPDIF_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	#if defined(SPDIF_EXPORTS)
		#define SPDIF_API __declspec(dllexport)
	#else
		#define SPDIF_API __declspec(dllimport)
	#endif
#else
	#define SPDIF_API extern
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//#define 16_BIT 

// SSP Control Register (0x0)
//This register defines the pre-defined frame format according to the following encoding:
//000: Texas Instrument・s Synchronous Serial Port (SSP)
//001: Motorola・s Serial Peripheral Interface (SPI)
//010: National Semiconductor・s MICROWIRE
//011: Philips・ I2S
//100: Intel・s AC-link
//101: Sony/Philips Digital Interface Format (SPDIF)
//110 ~ 111: Not defined
//If these bits are set, the operation of SSP will be unpredictable.
#define SSP_SPIDF_FFMT_BITS         (0x5 << 12)
#define SPDIF_VALIDITY_BIT          (0x0 << 10)
#define SPDIF_SLAVE_MODE_BIT        (0x0 << 3)

// SSP Control Register 1 (0x4), SDL
//#ifdef (16_BIT)
//    #define SSP_SDL_SETTING             (0xF << 16)
//#else
    #define SSP_SDL_SETTING             (0x1F << 16)
//#endif

// SSP Control Register 2 (0x8)
#define SSP_ENABLE_BIT              (0x1 << 0)
#define SSP_RXCLEAR_BIT             (0x1 << 2)

// SSP Status Register (0xC)
#define SSP_BUSY_BIT                (0x1 << 2)

// 60958-3 Part 3, 5. Channel status, Page 7 ~ 13.
// SPDIF Channel Status  Info Bits 0 (0x24)
#define SPDIF_LINEAR_PCM_ON         (0x0 << 2)
// Copy right purpose. Now set all copy free.
#define SPDIF_STATUS_COPY_FREE      (0x1 << 2)
// SPDIF Stereo setting, Now all set 2CH stereo
#define SPDIF_STATUS_STEREO         (0x3 << 20)

// SPDIF Channel Status Info Bits 1 (0x28)
// Maximum audio sample word length is 20 bits.
#define SPDIF_SAMPLE_MAX_WORD_LEN   (0x0 << 0)
// Sample word is 16bits if max length is 20bits
#define SPDIF_SAMPLE_WORD_LEN       (0x1 << 1)
// Original Sampling frequency.
#define ORIGINAL_44_10K_INFO        (0xF << 4)
#define ORIGINAL_88_20K_INFO        (0x7 << 4)
#define ORIGINAL_22_05K_INFO        (0xB << 4)
#define ORIGINAL_176_40K_INFO       (0x3 << 4)
#define ORIGINAL_48K_INFO           (0xD << 4)
#define ORIGINAL_96K_INFO           (0x5 << 4)
#define ORIGINAL_24K_INFO           (0x9 << 4)
#define ORIGINAL_192K_INFO          (0x1 << 4)
#define ORIGINAL_8K_INFO            (0x6 << 4)
#define ORIGINAL_11_025K_INFO       (0xA << 4)
#define ORIGINAL_12K_INFO           (0x2 << 4)
#define ORIGINAL_11_025K_INFO       (0xA << 4)
#define ORIGINAL_32K_INFO           (0xC << 4)
#define ORIGINAL_16K_INFO           (0x8 << 4)
#define ORIGINAL_NO_INDICATION      (0x1 << 4)

// SPDIF I2S and AC3 Control Register (0x5C)
#define SPDIF_I2S_DATA_SEL          (0x1 << 0)
#define SPDIF_WORD_SWAP             (0x1 << 4)
#define SPDIF_SRC                   (0x1 << 9)
#define SPDIF_DIVIDER_BIT_OFFSET    (16)

#define AUDIO_GRANULES              (2)
#define AUDIO_CHANNEL               (2)
#define MAX_AUDIO_SAMPLE            (576)
#define MAX_BUFFER_FRAME            (16)
#define DMA_BUFFER_SIZE             (MAX_BUFFER_FRAME * MAX_AUDIO_SAMPLE * AUDIO_CHANNEL * AUDIO_GRANULES)
#define DMA_LINK_DESCRIPTOR_COUNT   (MAX_BUFFER_FRAME)

//#define I2S_DMA_TO_SPDIF
#define SPDIF_BUFFER_SIZE_ERR   (0x1 << 31)
#define SPDIF_BUFFER_COUNT_ERR  (0x1 << 30)
#define SPDIF_SAMPLE_SIZE_ERR   (0x1 << 29)
#define SPDIF_WRONG_ATTRIBUTE   (0x1 << 28)
#define SPDIF_ALLOC_FAIL        (0x1 << 27)
#define SPDIF_WRONG_IO_INPUT    (0x1 << 26)

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================

typedef enum SPDIF_MODE_TAG
{
    SPDIF_LINEAR_PCM = 0,
    SPDIF_NON_LINEAR_DATA
} SPDIF_MODE;

typedef enum SPDIF_SAMPLING_RATE_TAG
{
    SPDIF_11_025K_SAMPLING = 0,
    SPDIF_22_05K_SAMPLING,
    SPDIF_44_10K_SAMPLING,
    SPDIF_12K_SAMPLING,
    SPDIF_24K_SAMPLING,
    SPDIF_48K_SAMPLING,
    SPDIF_8K_SAMPLING,
    SPDIF_16K_SAMPLING,
    SPDIF_32K_SAMPLING,
    SPDIF_SAMPLING_NO_INDICATION
} SPDIF_SAMPLING_RATE; 

typedef enum MMP_SPDIF_INPUT_IO_SEL_TAG
{
    Z2_SPDIF = 0,
    Z3_SPDIF = 1,
    IO_SPDIF = 2,
    PMCLK_SPDIF = 3
} MMP_SPDIF_INPUT_IO_SEL;

typedef enum MMP_SPDIF_ATTRIB_TAG
{
    MMP_SPDIF_PER_DMA_BLOCK_BUFFER_SIZE          = 0,
    MMP_SPDIF_BUFFER_COUNT                       = 1,
    MMP_AP_DMA_FUNCTION_PTR                      = 2,
    MMP_SPDIF_INPUT_IO                           = 3,
    MMP_SPDIF_FORCE_16BIT                        = 4,
    MMP_SPDIF_BIT_ENDIAN_SWAP                    = 5,
    MMP_SPDIF_INPUT_SAMPLE_SIZE                  = 6,
	MMP_SPDIF_ATTRIB_NONE                        = 0xFFFFFFFF
} MMP_SPDIF_ATTRIB;

typedef void (*SPDIF_DMA_CALLBACK) (MMP_UINT32 writeIndex);


//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
//                             SPDIF Mode Functions
//=============================================================================

//=============================================================================
/**
 * Initialize the SPDIF module.
 * @param none.
 * @return none
 */
//=============================================================================
SPDIF_API void
mmpSpdifInitialize(
    void);

//=============================================================================
/**
 * Terminate the SPDIF module.
 *
 * @param none.
 * @return none
 */
//=============================================================================
SPDIF_API void
mmpSpdifTerminate(
    void);

//=============================================================================
/**
 * Setup the SPDIF attribute.
 *
 * @param mode attribList
 * @return none
 */
//=============================================================================
SPDIF_API MMP_UINT32
mmpSpdifSetAttribute(
    const MMP_UINT32*  attribList);

//=============================================================================
/**
 * Fire or Disable the SPDIF engine.
 *
 * @param bEnable Whether fire the SPDIF Engine.
 * @return none
 */
//=============================================================================
SPDIF_API void
mmpSpdifSetEngineState(
    MMP_BOOL bEnable);

//=============================================================================
/**
 * Whether SPDIF Engine Idle or not.
 * @param none.
 * @return Whether the SPDIF Engine Idle.
 */
//=============================================================================   
SPDIF_API MMP_BOOL
mmpSpdifWaitEngineIdle(
    void);

SPDIF_API void
mmpSpdifSetMute(
    void);

SPDIF_API void
mmpSpdifSetUnMute(
    void);

SPDIF_API MMP_UINT32
mmpSpdifGetCurrentWriteIndex(
    void);

SPDIF_API MMP_UINT8*
mmpSpdifGetStartBuffer(
    void);

#ifdef __cplusplus
}
#endif

#endif

