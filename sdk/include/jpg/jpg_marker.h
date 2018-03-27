/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as JPEG Marker header file.
 *
 * @version 0.1
 */

#ifndef JPG_MARKER_H
#define JPG_MARKER_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "jpg/config.h"
#include "jpg/jpg_hw.h"

//=============================================================================
//                              Constant Definition
//=============================================================================



//
// JPEG file marker
//

#define JPG_MARKER_START                0xff

#define JPG_MSK_APPL_RESERVED_MARKER    0xf0
#define JPG_MSK_JPEG_EXTENSIONS_MARKER  0xf0

#define JPG_BASELINE_DCT_MARKER         0xc0
#define JPG_EXTENDED_SEQUENTIAL_DCT     0xc1
#define JPG_PROGRESSIVE_DCT             0xc2
#define JPG_LOSSLESS_SEQUENTIAL         0xc3
#define JPG_HUFFMAN_TABLE_MARKER        0xc4

#define JPG_START_OF_IMAGE_MARKER       0xd8
#define JPG_END_OF_IMAGE_MARKER         0xd9
#define JPG_START_OF_SCAN_MARKER        0xda
#define JPG_Q_TABLE_MARKER              0xdb
#define JPG_DRI_MARKER                  0xdd

#define JPG_APPL_RESERVED_MARKER        0xe0
#define JPG_JPEG_EXTENSIONS_MARKER      0xf0

#define JPG_Q_TABLE_ELEMENT_NUM         64
#define JPG_Q_TABLE_SEGMENT_LENGTH      65

#define JPG_LENGTH_FIELD                2

#define START_OF_IMAGE(ptr)                         \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_START_OF_IMAGE_MARKER;   \
            (ptr)++;                                \
        }

#define END_OF_IMAGE(ptr)                           \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_END_OF_IMAGE_MARKER;     \
            (ptr)++;                                \
        }


//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Allocate Line Buffer
 *
 * @param ImageWidth [IN]
 * @param ImageHeight [IN]
 * @param ColorFmt [IN]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 *
 */
//=============================================================================
JPG_ERROR_CODE
MarkerParser(JPEG_IMAGE_COMPONENT *pImageComp,
             MMP_UINT8 **pCurrent,
             MMP_UINT32 DataSize);


//=============================================================================
/**
 * Data Parse
 *
 * @param pCurrent [Out]
 * @param dwToBSSize [Out]
 * @param DataSize [IN]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 *
 */
//=============================================================================
JPG_ERROR_CODE
DataParser(MMP_UINT8 **pCurrent,
           MMP_UINT32 *dwToBSSize,
           MMP_UINT32 DataSize,
           MMP_BOOL bNonInterleaved);

JPG_ERROR_CODE
DataParserMultiSection(MMP_UINT8 **pCurrent,
                       MMP_UINT32 *dwToBSSize,
                       MMP_UINT32 DataSize,
                       MMP_BOOL bNonInterleaved);

//=============================================================================
/**
 * Set Marker
 *
 * @param pImageComp [IN]
 * @param pQTable_Y [IN]
 * @param pQTable_UV [IN]
 * @param pCurrent [Out]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 *
 */
//=============================================================================
JPG_ERROR_CODE
SetMarker(JPEG_IMAGE_COMPONENT *pImageComp,
          MMP_UINT8 **pCurrent);


//=============================================================================
/**
 * Adjust Q-Table
 *
 * @param pY_Table [Out]
 * @param pUV_Table [Out]
 * @param wQuality [IN]
 * @return void
 *
 */
//=============================================================================
void
JpgQualityTable(MMP_UINT8 *pY_Table,
                MMP_UINT8 *pUV_Table,
                MMP_UINT16 wQuality);

//=============================================================================
/**
 * Get Huffman-Table address
 *
 * @param pY_DC [Out]
 * @param pY_AC [Out]
 * @param pUV_DC [Out]
 * @param pUV_AC [Out]
 * @return void
 *
 */
//=============================================================================
void
JPG_Encode_GetHuffmanTable(MMP_UINT8 **pY_DC,
                           MMP_UINT8 **pY_AC,
                           MMP_UINT8 **pUV_DC,
                           MMP_UINT8 **pUV_AC);

//=============================================================================
/**
 * Set Image component
 *
 * @param pImageComp [Out]
 * @return void
 *
 */
//=============================================================================
void
JPG_SetEncodeImageComp(JPEG_IMAGE_COMPONENT *pImageComp);


//=============================================================================
/**
 * Set decode color format
 *
 * @param imageComp [Out]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 *
 */
//=============================================================================
JPG_ERROR_CODE
JPEG_SetDecodeColorFormat(JPEG_IMAGE_COMPONENT *imageComp);




#ifdef __cplusplus
}
#endif

#endif
