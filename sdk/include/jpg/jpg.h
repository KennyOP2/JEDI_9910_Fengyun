/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as JPEG error code header file.
 *
 * @version 0.01
 */

#ifndef JPG_H
#define JPG_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_jpg.h"
#include "jpg/config.h"
#include "jpg_type.h"
#include "jpg/jpg_hw.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
extern MMP_UINT16 jpegBufferControl[5];

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
//=====================
// jpg_common
//=====================
//=============================================================================
/**
 * JPEG set image size information (MCU unit)
 *
 * @param jpegMode [IN]
 * @param imageComp [IN]
 * @param sizeInfo [OUT]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 *
 */
//=============================================================================
JPG_ERROR_CODE
JPEG_SetImageSize(
    JPEG_BITSTREAM_MODE   jpegMode,
    JPEG_IMAGE_COMPONENT *imageComp,
    JPEG_IMAGE_SIZE_INFO *sizeInfo);


//=============================================================================
/**
 * JPEG Engine setup
 *
 * @param ctxt
 * @return void
 */
//=============================================================================
void
JPEG_EngineSetup(void);

//=====================
// jpg_encode
//=====================
//=============================================================================
/**
 * JPEG Encode setup
 *
 * @param ispToJpeg [IN]
 * @param quality [IN]
 * @param triggerMode [IN]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 */
//=============================================================================
JPG_ERROR_CODE
JPEG_EncodeSetup(
    JPG_ENCODER            *ptJpgEncoder,
    MMP_UINT16              quality,
    JPEG_TRIGGER_MODE       triggerMode);


#ifdef __cplusplus
}
#endif

#endif //JPG_H
