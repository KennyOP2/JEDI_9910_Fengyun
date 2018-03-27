/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as JPEG type header file.
 *
 * @version 0.1
 */

#ifndef JPG_TYPE_H
#define JPG_TYPE_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "jpg/config.h"
#include "mmp_isp.h"
#include "mmp_jpg.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define JPEG_Q_TABLE_SIZE                    (64)
#define JPEG_MAX_COMPONENT_NUM               4
#define JPEG_BLOCK_SIZE                      8
#define JPEG_READ_BUF_SIZE                   128*1024


//=============================================================================
//                              Structure Definition
//=============================================================================
/**
 * Jpeg line buffer type
 **/
typedef enum JPEG_LINE_BUF_TYPE_TAG
{
	JPEG_LINE_BUF_TYPE_ISP_TRIGGER,
	JPEG_LINE_BUF_TYPE_RESERVE,
	JPEG_LINE_BUF_TYPE_CMD_TRIGGER
}
JPEG_LINE_BUF_TYPE;

typedef enum JPEG_BITSTREAM_MODE_TAG
{
	JPEG_BITSTREAM_ENCODE,
	JPEG_BITSTREAM_DECODE,
	JPEG_BITSTREAM_MotionJPEG,
	MP1_BITSTREAM_DECODE
} JPEG_BITSTREAM_MODE;

/**
 *  JPEG Color format
 */
typedef enum JPEG_COLOR_FORMAT_TAG
{
	JPEG_YUV_444,
	JPEG_YUV_422,
	JPEG_YUV_420,
	JPEG_YUV_411,
	JPEG_YUV_422R
} JPEG_COLOR_FORMAT;


/**
 * YUV to RGB transform matrix
 */
typedef struct JPEG_YUV_TO_RGB_TAG
{
	MMP_UINT16              _11;
	MMP_UINT16              _13;
	MMP_UINT16              _21;
	MMP_UINT16              _22;
	MMP_UINT16              _23;
	MMP_UINT16              _31;
	MMP_UINT16              _32;
	MMP_UINT16              ConstR;
	MMP_UINT16              ConstG;
	MMP_UINT16              ConstB;
	MMP_UINT16              Reserved;
} JPEG_YUV_TO_RGB;

/**
 * Output RGB Dithrer key
 */
typedef struct JPEG_DITHER_KEY_TAG
{
	MMP_BOOL                bEnDitherKey;
	MMP_UINT16              ditherKey;
	MMP_UINT16              ditherKeyMask;
	MMP_UINT16              bgColor;

} JPEG_DITHER_KEY;

/**
 *  Non-interleaved Component
 */
typedef enum JPEG_NON_INTERLEAVED_TYPE_TAG
{
	Y_TYPE        = 0,
	U_TYPE        = 1,
	V_TYPE        = 2,
	Reserved_TYPE = 0x7FFFFFFF
} JPEG_NON_INTERLEAVED_TYPE;

/**
 *  JPEG Image information
 */
typedef struct JPEG_IMAGE_INFO_TAG
{
	MMP_UINT16 horizonSamp;
	MMP_UINT16 verticalSamp;
	MMP_UINT16 qTableSel;
	MMP_UINT16 dcHuffmanTableSel;
	MMP_UINT16 acHuffmanTableSel;
	MMP_UINT16 componentId;
	MMP_UINT16 reserved;
} JPEG_IMAGE_INFO;

/**
 *  Q table
 */
typedef struct JPEG_Q_TABLE_TAG
{
	MMP_UINT8  Table[4][JPEG_Q_TABLE_SIZE];
	MMP_UINT16 tableCount;
	MMP_UINT16 reserved;
} JPEG_Q_TABLE;

/**
 *  Huffman table
 */
typedef struct H_TABLE_TAG
{
	MMP_UINT8  *pHuffmanTable;
	MMP_UINT16 totalCodeLengthCount;
	MMP_UINT16 reserved;
} H_TABLE;

/**
 *  Huffman table
 */
typedef struct HUFFMAN_TABLE_TAG
{
	H_TABLE DC[2];  // 0->Y, 1->UV
	H_TABLE AC[2];  // 0->Y, 1->UV
} HUFFMAN_TABLE;

/**
 *  JPEG Image component
 */
typedef struct JPEG_IMAGE_COMPONENT_TAG
{
	JPEG_COLOR_FORMAT     colorFormat;
	JPEG_COLOR_FORMAT     encodeFmt;
	MMP_UINT16            imageHeight;
	MMP_UINT16            imageWidth;
	MMP_UINT16            realHeight;
	MMP_UINT16            realWidth;
	MMP_UINT16            heightUnit;
	MMP_UINT16            widthUnit;
	MMP_UINT16            restartInterval;
	MMP_UINT16            componentNum;
	MMP_UINT16            validComponent;
	MMP_UINT16            reserved;
	JPEG_IMAGE_INFO       imageInfo[JPEG_MAX_COMPONENT_NUM];
	JPEG_Q_TABLE          qTable;
	HUFFMAN_TABLE         huffmanTable;
	JPEG_NON_INTERLEAVED_TYPE  NonInterleaved_Type;
	MMP_BOOL              bNonInterleaved;
	MMP_BOOL              bDefaultHuffmanTable;
	MMP_BOOL              singleChannel;

	MMP_BOOL              progressive;
} JPEG_IMAGE_COMPONENT;

/**
 *  JPEG Image size info
 */
typedef struct JPEG_IMAGE_SIZE_INFO_TAG
{
	MMP_UINT16          realWidth;
	MMP_UINT16          realHeight;
	MMP_UINT16          startX;
	MMP_UINT16          startY;
	MMP_UINT16          dispWidth;
	MMP_UINT16          dispHeight;

	MMP_UINT16          mcuRealWidth;
	MMP_UINT16          mcuRealHeight;
	MMP_UINT16          mcuDispWidth;
	MMP_UINT16          mcuDispHeight;
	MMP_UINT16          mcuDispLeft;
	MMP_UINT16          mcuDispRight;
	MMP_UINT16          mcuDispUp;
	MMP_UINT16          mcuDispDown;
} JPEG_IMAGE_SIZE_INFO;

/**
 *  Line buffer info.
 */
typedef struct JPEG_LINE_BUF_INFO_TAG
{
	MMP_UINT8          *addrAlloc;
	MMP_UINT8          *component1Addr;
	MMP_UINT8          *component2Addr;
	MMP_UINT8          *component3Addr;
	MMP_UINT16          component1Pitch;
	MMP_UINT16          component23Pitch;
	MMP_UINT16          sliceNum;
	MMP_UINT16          reserved;
	MMP_UINT32          size;
	MMP_UINT32          ySliceByteSize;
	MMP_UINT32          uSliceByteSize;
	MMP_UINT32          vSliceByteSize;

	JPEG_LINE_BUF_TYPE   type;
} JPEG_LINE_BUF_INFO;

/**
 *  Bit-stream buffer info.
 */
typedef struct JPEG_BITSTREAM_BUF_INFO_TAG
{
	MMP_UINT8          *addrAlloc;
	MMP_UINT8          *addr;
	MMP_UINT32          rwSize;
	MMP_UINT32          size;
	MMP_UINT32          toBitstreamSize;
	MMP_UINT16          shiftByteNum;
} JPEG_BITSTREAM_BUF_INFO;

/**
 *  Jpeg decode multisection infomation.
 */
typedef struct JPEG_DECODE_MULTISECTION_INFO_TAG
{
	MMP_BOOL    bWait1stProcess;
	MMP_UINT32  ul1stProcessParsed;
	MMP_BOOL    bMultiDecodeFinished;
} JPEG_DECODE_MULTISECTION_INFO;

/**
 *  Jpeg Partial Encode Infomation.
 */
typedef struct JPEG_PARTIAL_ENC_INFO_TAG
{
	MMP_BOOL    verify;
	MMP_BOOL    isFirstSection;
	MMP_BOOL    isLastSection;
	MMP_UINT    sectionHeight;
} JPEG_PARTIAL_ENC_INFO;


/**
 *  System memory information.
 */
typedef struct
{
	MMP_BOOL    initial;
	MMP_UINT8  *start;
	MMP_UINT8  *current;
	MMP_UINT32  size;
} JPEG_SYSRAM_INFO;

/**
 *  JPEG HW control setting
 */
typedef struct JPG_HW_CTRL_TAG
{
	MMP_UINT16 codecCtrl;
	MMP_UINT16 wLineBufSliceWrite;
	MMP_UINT32 dwBSBufRWDataSize;
	MMP_UINT8 *qTableY;
	MMP_UINT8 *qTableUv;
	MMP_UINT16 wCodecFireCMD;
	MMP_UINT16 wLineBufCtrl;
	MMP_UINT16 wValidLineBufSlice;
	MMP_UINT16 wBSBufCtrl;
	MMP_UINT16 wValidBSBufSize;
	MMP_UINT16 dcHuffmanwTotalCodeLengthCount[2];      // 0xbe         ; 0->Y, 1->UV
	MMP_UINT16 acHuffmanwTotalCodeLengthCount[2];      // 0xbe         ; 0->Y, 1->UV
	MMP_UINT8 *dcHuffmanTable[2];                      // 0xc0         ; 0->Y, 1->UV
	MMP_UINT8 *acHuffmanTable[2];                      // 0xc2, 0xc4   ; 0->Y, 1->UV
} JPG_HW_CTRL;

typedef enum JPEG_FLAGS_TAG
{
	JPEG_FLAGS_ENC_PARTIAL_OUT           = (0x00000001 << 0),
	JPEG_FLAGS_MJPG_FIRST_FRAME          = (0x00000001 << 1),
	JPEG_FLAGS_MJPG                      = (0x00000001 << 2),
	JPEG_FLAGS_DEC_GET_INFO              = (0x00000001 << 3),
	JPEG_FLAGS_OUTPUT_RGB565             = (0x00000001 << 5),
	JPEG_FLAGS_EN_RGB565_Dither_KEY      = (0x00000001 << 6),
	JPEG_FLAGS_DEC_DC_ONLY               = (0x00008000),
	JPEG_FLAGS_DEC_UV_HOR_DOWNSAMPLE     = (0x01100000),
	JPEG_FLAGS_DEC_UV_VER_DOWNSAMPLE     = (0x02200000),
	JPEG_FLAGS_DEC_UV_HOR_DUPLICATE      = (0x04400000),
	JPEG_FLAGS_DEC_Y_HOR_DOWNSAMPLE      = (0x00010000)
} JPEG_FLAGS;

/**
 *  Jpeg context struct.
 */
typedef struct JPEG_CONTEXT_TAG
{
	MMP_UINT32                  gChipVersion;
	MMP_UINT32                  flags;
	JPEG_IMAGE_COMPONENT        imageComponent;
	JPEG_IMAGE_SIZE_INFO        sizeInfo;
	JPEG_LINE_BUF_INFO          lineBufInfo;
	JPEG_BITSTREAM_BUF_INFO     bitstreamBufInfo;
	JPEG_DECODE_MULTISECTION_INFO decMultisectionInfo;
	JPEG_PARTIAL_ENC_INFO       partialEncInfo;
	JPEG_SYSRAM_INFO            sysMemInfo;
	JPG_HW_CTRL                 hwCtrl;
	//MMP_ISP_SHARE               ispShare;
	JPEG_DISPLAY_INFO           displayInfo;
	MMP_UINT32                  apReqSize;

	// motion jpg
	JPEG_BITSTREAM_BUF_INFO     motionJpgBsBufInfo[2];
	JPEG_LINE_BUF_INFO          motionJpgFrameBufInfo[3];
	MMP_UINT16                  motionJpgBsBufIndex;

	// rgb565 output info
	MMP_UINT8                   *outBufAddr;
	MMP_UINT32                  destPitch;
	JPEG_YUV_TO_RGB             yuv2RgbMatrix;
	JPEG_DITHER_KEY             ditherKeyInfo;
} JPEG_CONTEXT;

extern JPEG_CONTEXT *ctxt;


#if defined(JPG_ENABLE_DEC_JPROG)
///////////////////////////////////////////////////////////////////////////////
// JPEG Progressive Decode process
///////////////////////////////////////////////////////////////////////////////

#define SMTK_MAX_DC_SCAN    5
#define SMTK_MAX_AC_SCAN    18
/**
 *  Huffman Info
 */
typedef struct HUFFMAN_INFO_TAG
{
	MMP_BOOL        verify;

	// info of header
	MMP_UINT8  lengthHuffCode[16];  // sizeof() = 16
	MMP_UINT8  valueHuffCode[162];  // sizeof()  = 162

	// data of re-building huffman table
	MMP_UINT32 groupUpBound[16];  // for checking which group
	MMP_UINT32 acc[16];      // accumulate of groupUpBound, it can be use to get the codeword index on huffman table.

} HUFFMAN_INFO;

/**
 * Jprog bit stream
 */
typedef struct JPROG_BIT_STREAM_TAG
{

	MMP_UINT16 bufCurr2Bytes;
	MMP_UINT16 bufNext2Bytes;

	MMP_UINT currPos;  // 0 ~ 15

	MMP_UINT8 *tail;    // the tail address of input bit stream
	MMP_UINT8 *start;   // the start address of input bit stream

	MMP_UINT length;   // the length of used bit stream
	MMP_UINT seekBytePos; // current seek position in file
	MMP_UINT seekBitPos;   // current position of bit in a byte

	MMP_BOOL isFileEnd;
	MMP_BOOL keepLast2bytes;

} JPROG_BIT_STREAM;

/**
 *  JPEG Progressive DC Scan Info
 */
typedef struct JPROG_DC_SCAN_INFO_TAG
{
	MMP_BOOL        verify;

	MMP_UINT        positionDHT[2]; // 0 = Y, 1 = UV
	MMP_UINT        lengthDHT[2];   // 0 = Y, 1 = UV
	MMP_UINT        positionSOS;
	MMP_UINT        lengthSOS;
	MMP_UINT        payloadPosition;

	MMP_UINT        *inBufAddr;

	MMP_UINT8       numCompInScan;

	MMP_INT         previousDC[4];
	MMP_UINT        countMCU;

	MMP_UINT        restartCountDc;

	//HUFFMAN_TABLE huffmanTable; // just record address, the info is decoded in HW

	HUFFMAN_INFO    huffInfo[2];  // 0 = Y, 1 = UV
	JPEG_IMAGE_INFO imageInfo[4];

	MMP_UINT8       Ss;
	MMP_UINT8       Se;
	MMP_UINT8       Ah;
	MMP_UINT8       Al;

	// bit stream contril
	JPROG_BIT_STREAM bs;

} JPROG_DC_SCAN_INFO;

/**
 *  JPEG Progressive AC Scan Info
 */
typedef struct JPROG_AC_SCAN_INFO_TAG
{
	MMP_BOOL        verify;

	//MMP_BOOL        existDHT;
	MMP_UINT        positionDHT[2];
	MMP_UINT        lengthDHT[2];
	MMP_UINT        positionSOS;
	MMP_UINT        lengthSOS;
	MMP_UINT        payloadPosition;

	MMP_UINT8       numCompInScan;
	MMP_UINT8       componentId;

	MMP_UINT        countMCU;

	MMP_UINT        EOBRUN;
	MMP_UINT        restartCountAc;

	//HUFFMAN_TABLE huffmanTable;// just record address, the info is decoded in HW

	HUFFMAN_INFO    huffInfo;    // it must be AC huff
	MMP_UINT8       dcHuffmanTableSel;
	MMP_UINT8       acHuffmanTableSel;

	MMP_UINT8       Ss;
	MMP_UINT8       Se;
	MMP_UINT8       Ah;
	MMP_UINT8       Al;

	// bit stream contril
	JPROG_BIT_STREAM bs;

} JPROG_AC_SCAN_INFO;


/**
 *  MCUs Info
 */
typedef struct JPROG_MCUS_INFO_TAG
{
	MMP_UINT16  mcuWidth;
	MMP_UINT16  mcuHeight;
	MMP_UINT16  numMcuPreRow;
	MMP_UINT16  numMcuPreCol;
	//MMP_UINT16    numBlocksPreRow;
	//MMP_UINT16    numBlocksPreCol;

	MMP_UINT16  numBlocksInMcu[4];

	MMP_UINT16  totalBlocksInMcu;

	MMP_BOOL    skipBlk_v;
	MMP_UINT16  skipBlkPitch;
	MMP_BOOL    skipBlk_h;

} JPROG_MCUS_INFO;

/**
 *  JPEG Progressive Frame Header Info
 */
typedef struct JPROG_FRAME_HEADER_INFO_TAG
{
	MMP_INT    positionDQT[4];
	MMP_INT    lengthDQT[4];
	JPEG_Q_TABLE qTable;

	MMP_INT    positionSOF;
	MMP_INT    lengthSOF;
	MMP_INT    height;  // Number of lines
	MMP_INT    width;   // Number of samples per line
	MMP_UINT8  numCompInFrame;
	JPEG_IMAGE_INFO imageInfo[4];  // 4 compononts info

	JPROG_DC_SCAN_INFO jprogDcScan[SMTK_MAX_DC_SCAN]; // record 5 scan info
	JPROG_AC_SCAN_INFO jprogAcScan[SMTK_MAX_AC_SCAN]; // record 30 scan info

	JPROG_MCUS_INFO    jprogMcusInfo;

	MMP_UINT16      restartInterval;

	// for HW VLD format
	MMP_BOOL    isFirstSection;
	MMP_UINT8   bitPlaneIndx;

	MMP_BOOL     beWait;
	MMP_UINT     mcuIndexVldHw;
	MMP_UINT     compIndexVldHw;
	MMP_UINT     blkIndexVldHw;
	MMP_UINT     ZZIndexVldHw;
	MMP_UINT     currBlk_v;
	MMP_UINT     currBlk_h;

} JPROG_FRAME_HEADER_INFO;

extern JPROG_FRAME_HEADER_INFO *jprogHeaderInfo;

#endif // #if defined(JPG_ENABLE_DEC_JPROG)

#ifdef __cplusplus
}
#endif

#endif // #define JPG_TYPE_H
