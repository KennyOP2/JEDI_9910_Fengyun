/*
 * Copyright (c) 2014 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia JPEG Driver API header file.
 *
 * @author 
 */

#ifndef _MMP_JPG_H_
#define _MMP_JPG_H_


#ifdef __cplusplus
extern "C" {
#endif
#if defined(_WIN32)

    #if defined(JPG_EXPORTS)
        #define JPG_API __declspec(dllexport)
    #else
        #define JPG_API __declspec(dllimport)
    #endif
#else
    #define JPG_API extern
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"
#include "mmp_intr.h"
#include "intr/intr.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================
//  Enumeration Type Definition
//=============================

/**
 * JPEG Encode Source Selection
 */
typedef enum MMP_JPG_ENCODE_SOURCE_SELECTION_TAG
{
	MMP_JPG_ENCODE_SRC_FROM_ISP,       /**< Jpeg encode source is from ISP engine and AP will not used use this flag. */
	MMP_JPG_ENCODE_SRC_FROM_SYSTEM,    /**< Jpeg encode source is ready in system memory. */
	MMP_JPG_ENCODE_SRC_FROM_VRAM       /**< Jpeg encode sorrce is ready in video memory.   */
}
MMP_JPG_ENCODE_SOURCE_SELECTION;


/**
 * JPEG Decode Image Information
 */
typedef struct MMP_JPG_DECODE_IMAGE_INFO_TAG
{
	MMP_UINT width;           /**< image width     */
	MMP_UINT height;          /**< image height    */
	MMP_UINT colorFormat;
	MMP_BOOL progressive;
} MMP_JPG_DECODE_IMAGE_INFO;

/**
 * Window rectangle.
 */
typedef struct JPG_RECT_TAG
{
	MMP_INT  startX;
	MMP_INT  startY;
	MMP_UINT width;
	MMP_UINT height;
} JPG_RECT;

typedef struct JPEG_DISPLAY_INFO_TAG
{
	/**
	 * Source information (jpeg source)
	 */
	JPG_RECT            videoWin;
	MMP_UINT            videoWidth;
	MMP_UINT            videoHeight;
	/**
	 * Destination information
	 */
	JPG_RECT            dstWin;
	MMP_SURFACE         dstSurface;
	MMP_UINT            dstWidth;
	MMP_UINT            dstHeight;
	/**
	 * Some special process of decoding.
	 */
	//mylin
	//MMP_ISP_ROTATE_TYPE rotateType;
	MMP_BOOL            enableRotate;
	MMP_UINT            effect;
	MMP_BOOL            enableEffect;
	MMP_UINT            orientation;
	MMP_BOOL            isAdobe_CMYK;
} JPEG_DISPLAY_INFO;

/**
 * JPEG Decode Output Information
 */
typedef struct MMP_JPG_DECODE_OUTPUT_INFO_TAG
{
	MMP_UINT32          addrY;
	MMP_UINT32          addrV;
	MMP_UINT32          addrU;
	MMP_UINT16          width;
	MMP_UINT16          height;
	MMP_UINT16          pitchY;
	MMP_UINT16          pitchUv;
	//MMP_ISP_INFORMAT    format;
} MMP_JPG_DECODE_OUTPUT_INFO;

/**
 * Motion Jpeg deocder
 **/
typedef struct MMP_MJPG_DECODER_TAG
{
	// output buf info
	MMP_INT         frameBufIdx;
	MMP_UINT8       *frameBuf;

	// input buf info
	MMP_INT         bsBufIdx; // bit stream buf index
	MMP_UINT        bsSize;
	MMP_UINT8       *bsBuf[2];
	MMP_UINT        bsMaxBufSize;

	// disp info
	MMP_INT         dispIndex;
	MMP_UINT32      framePitchY;
	MMP_UINT32      framePitchUV;
	MMP_UINT32      frameWidth;
	MMP_UINT32      frameHeight;

	MMP_INT8        currDispFrmBufIdx;
	MMP_INT8        prevDispFrmBufIdx;
	MMP_INT8        maxFrmBufCount;

	MMP_BOOL        bKeepLastField;

	MMP_UINT32      clipStartX;
	MMP_UINT32      clipStartY;
	MMP_UINT32      clipWidth;
	MMP_UINT32      clipHeight;

} MMP_MJPG_DECODER;

typedef struct JPG_ENCODER_TAG
{                                 
    MMP_UINT32         frameWidth;        //[IN]
    MMP_UINT32         frameHeight;       //[IN]
    MMP_UINT32         framePitchY;       //[OUT]	
    MMP_UINT8         *pHdrBufAddr;       //[OUT]
    MMP_UINT8         *pStreamBufAdr[8];  //[OUT]
    MMP_UINT8         *pSourceBufAdrY[5]; //[OUT]
    MMP_UINT8         *pSourceBufAdrU[5]; //[OUT]
    MMP_UINT8         *pSourceBufAdrV[5]; //[OUT]    
    MMP_UINT32         ParaSetHdrSize;    //[OUT]
    MMP_UINT32         streamBufSize;     //[OUT]
    MMP_UINT32         streamBufSelect;   //[OUT]
    MMP_UINT32         streamBufCount;    //[OUT]
    MMP_UINT32         sourceBufSelect;   //[IN]
} JPG_ENCODER;
//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group3 SMedia JPEG Driver API
 *  The JPEG module API.
 *  @{
 */

JPG_API MMP_RESULT
mmpJPGEncodeOpen(
    JPG_ENCODER    *ptJpgEncoder,
    MMP_UINT        quality);

JPG_API MMP_RESULT
mmpJPGEncodeFire(    
    JPG_ENCODER    *ptJpgEncoder);
    

JPG_API MMP_RESULT
mmpJPGEncodeGetStream(    
    MMP_UINT32*     StreamLen,
    MMP_BOOL*       bFrameEnd,
    JPG_ENCODER    *ptJpgEncoder);
 
JPG_API MMP_BOOL
mmpJPGEncodeQueryIdle(    
    void);
    
JPG_API MMP_RESULT
mmpJPGEncodeEnableInterrupt(
    ithIntrHandler  handler);


JPG_API MMP_RESULT
mmpJPGEncodeDisableInterrupt(
    void);

JPG_API void
mmpJPGEncodeClearInterrupt(
    void);
    
#ifdef __cplusplus
}
#endif

#endif // _MMP_JPG_H_
