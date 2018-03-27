/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as JPEG error code header file.
 *
 * @version 0.01
 */

#ifndef JPG_HW_H
#define JPG_HW_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "jpg/config.h"
#include "jpg_reg.h"
#include "jpg_type.h"

//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              Structure Definition
//=============================================================================


//=============================================================================
//                              Macro Definition
//=============================================================================
#define setByteSwap(data)       (((data>>8) & 0x00FF) | ((data<<8) & 0xFF00))

//=============================================================================
//                              Function Declaration
//=============================================================================

/**
 *  Note: 1) All the APIs input/output use 8-bit unit.
 *        2) Inside these APIs, 8-bit unit will be automatically
 *           transferred to 32 bit unit.
 */

// 0x00
void JPEG_SetCodecCtrlReg(MMP_UINT16 data);
void JPEG_SetBitstreamReadBytePosReg(MMP_UINT16 data);

// 0x02
void JPEG_SetDriReg(MMP_UINT16 data);

// 0x04
void JPEG_SetTableSpecifyReg(const JPEG_IMAGE_COMPONENT *imageComponent);

// 0x06, 0x08, 0xF0~0xFA
void JPEG_SetImageSizeInfoReg(const JPEG_IMAGE_SIZE_INFO *sizeInfo);

// 0x0A~0x1A
void JPEG_SetLineBufInfoReg(const JPEG_LINE_BUF_INFO *bufInfo);

// 0x16
void JPEG_SetLineBufSliceUnitReg(MMP_UINT16 data, MMP_UINT16 yVerticalSamp);

// 0x1C
void JPEG_SetLineBufSliceWriteNumReg(MMP_UINT16 data);

// 0x1E, 0x20, 0x22, 0x24
void JPEG_SetBitStreamBufInfoReg(const JPEG_BITSTREAM_BUF_INFO *bufInfo);

// 0x26, 0x28
void JPEG_SetBitstreamBufRwSizeReg(MMP_UINT32 data);

// 0x2A, 0x2D
void JPEG_SetSamplingFactorReg(const JPEG_IMAGE_COMPONENT *imageComponent);

// 0x2E~0xED
// input the Zig-zag order
void JPEG_SetQtableReg(const JPEG_Q_TABLE *qTable);

// 0xEE
void JPEG_DropHv(MMP_UINT16 data);

// 0xFC
void JPEG_StartReg(void);

// 0xFE
MMP_UINT16 JPEG_GetEngineStatusReg(void);

// 0x100
MMP_UINT16 JPEG_GetEngineStatus1Reg(void);

// 0x102
void JPEG_SetLineBufCtrlReg(MMP_UINT16 data);

// 0x104
MMP_UINT16 JPEG_GetLineBufValidSliceReg(void);

// 0x106
void JPEG_SetBitstreamBufCtrlReg(MMP_UINT16 data);

// 0x108
MMP_UINT32 JPEG_GetBitStreamValidSizeReg(void);

// 0x10C
void
JPEG_SetHuffmanCodeCtrlReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeLength);

// 0xB0E
void
JPEG_SetDcHuffmanValueReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeValue,
    MMP_UINT16 totalCodeLengthCount);


#if !defined(DTV_DISABLE_JPG_ENC)

// 0x110
// 0x112
void
JPEG_SetEncodeAcHuffmanValueReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeValue,
    MMP_UINT16 totalCodeLengthCount);

// 0x110
// 0x112
void
JPEG_SetDecodeAcHuffmanValueReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeValue,
    MMP_UINT16 totalCodeLengthCount);

#endif // #if !defined(DTV_DISABLE_JPG_ENC)

// YUV to RGB color matrix
// 0xF26 ~ 0xF38
void
JPEG_SetYuv2RgbMatrix(
    JPEG_YUV_TO_RGB     *matrix);


// set RGB color key
// 0xF00 ~ 0xF04, 0xf5A
void
JPEG_SetRgb565DitherKey(
    JPEG_DITHER_KEY     *ditherKeyInfo);

//======================================
//
//======================================
void
JPEG_Reset(void);

//=============================================================================
/**
 * JPEG Power up
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPEG_PowerUp(void);


//=============================================================================
/**
 * JPEG encoder Power down (No isp)
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPEG_EncPowerDown(void);

//=============================================================================
/**
 * JPEG decoder Power down
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPEG_DecPowerDown(void);


#ifdef __cplusplus
}
#endif

#endif //JPG_HW_H
