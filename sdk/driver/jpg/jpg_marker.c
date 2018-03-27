/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  jpg_marker.c JPEG Engine HW Config
 *  Date: 
 *
 * @author 
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include "mem/mem.h"
#include "host/host.h"
#include "sys/sys.h"
#include "jpg/jpg_marker.h"
#include "jpg/jpg.h"

//=============================================================================
//                              Macro Definition
//=============================================================================

#define SKIP_WORD(ptr)                          (ptr) += sizeof(MMP_UINT16);

#define CLEAN_BYTE_VALUE(ptr)                   (*(ptr)) = (0x00);

#define GET_HIGH_BYTE_VALUE(ptr, wValue)        (wValue) = ((*(ptr) & 0xf0) >> 4);
#define SET_HIGH_BYTE_VALUE(ptr, wValue)        (*(ptr)) |= (((wValue) << 4) & 0xf0);

#define GET_HIGH_WORD_VALUE(dwValue, wValue)    (wValue) = (MMP_UINT16)(((dwValue) & 0xFFFF0000) >> 16);
#define GET_LOW_WORD_VALUE(dwValue, wValue)     (wValue) = (MMP_UINT16)((dwValue) & 0x0000FFFF);

#define GET_LOW_BYTE_VALUE(ptr, wValue)         (wValue) = (*(ptr) & 0x0f);
#define SET_LOW_BYTE_VALUE(ptr, wValue)         (*(ptr)) |= ((wValue) & 0x0f);

#define GET_BYTE_VALUE(ptr, wValue)   \
        {                             \
            (wValue) = *(ptr);        \
            (ptr++);                  \
        }

#define SET_BYTE_VALUE(ptr, wValue)   \
        {                             \
            (*(ptr)) = (wValue);      \
            (ptr++);                  \
        }

#define GET_WORD_VALUE(ptr, wValue)     \
        {                               \
            (wValue) = (*(ptr) << 8);   \
            (ptr++);                    \
            (wValue) |= *(ptr);         \
            (ptr++);                    \
        }

#define SET_WORD_VALUE(ptr, wValue)         \
        {                                   \
            (*(ptr)) = ((wValue) >> 8);     \
            (ptr++);                        \
            (*(ptr)) = (MMP_UINT8)(wValue); \
            (ptr++);                        \
        }

#define SKIP_SEGMENT(ptr)                         \
        {                                         \
            MMP_UINT16 segLength = *(ptr) << 8;   \
            (ptr)++;                              \
            segLength |= *(ptr);                  \
            (ptr) += segLength - 1;               \
        }

#define UPDATE_LINEBUFFER_WRITE_PTR(dwWritePtr, dwCopyDataSize)         \
        {                                                               \
            (dwWritePtr) = ((dwWritePtr) + (dwCopyDataSize)) & 0x003f;  \
        }

#define START_OF_IMAGE(ptr)                         \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_START_OF_IMAGE_MARKER;   \
            (ptr)++;                                \
        }

#define DEFINE_QUANZITATION_TABLE(ptr)              \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_Q_TABLE_MARKER;          \
            (ptr)++;                                \
        }

#define BASELINE_DCT(ptr)                           \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_BASELINE_DCT_MARKER;     \
            (ptr)++;                                \
        }

#define DEFINE_HUFFMAN_TABLE(ptr)                   \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_HUFFMAN_TABLE_MARKER;    \
            (ptr)++;                                \
        }

#define START_OF_SCAN(ptr)                          \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_START_OF_SCAN_MARKER;    \
            (ptr)++;                                \
        }

#define DEFINE_DRI_MARKER(ptr)                      \
        {                                           \
            (*(ptr)) = JPG_MARKER_START;            \
            (ptr)++;                                \
            (*(ptr)) = JPG_DRI_MARKER;              \
            (ptr)++;                                \
        }

//=============================================================================
//                              Structure Definition
//=============================================================================
#if !defined(DTV_DISABLE_JPG_ENC)
// Q table
static const MMP_UINT8 gY_QTable[JPEG_Q_TABLE_SIZE] =
{
	// Luminance Quantization Table
	0x10,
	0x0b, 0x0c,
	0x0e, 0x0c, 0x0a,
	0x10, 0x0e, 0x0d, 0x0e,
	0x12, 0x11, 0x10, 0x13, 0x18,
	0x28, 0x1a, 0x18, 0x16, 0x16, 0x18,
	0x31, 0x23, 0x25, 0x1d, 0x28, 0x3a, 0x33,
	0x3d, 0x3c, 0x39, 0x33, 0x38, 0x37, 0x40, 0x48,
	0x5c, 0x4e, 0x40, 0x44, 0x57, 0x45, 0x37,
	0x38, 0x50, 0x6d, 0x51, 0x57, 0x5f,
	0x62, 0x67, 0x68, 0x67, 0x3e,
	0x4d, 0x71, 0x79, 0x70,
	0x64, 0x78, 0x5c,
	0x65, 0x67,
	0x63,
};

static const MMP_UINT8 gUV_QTable[JPEG_Q_TABLE_SIZE] =
{
	// Chrominance Quantization Table
	0x11,
	0x12, 0x12,
	0x18, 0x15, 0x18,
	0x2f, 0x1a, 0x1a, 0x2f,
	0x63, 0x42, 0x38, 0x42, 0x63,
	0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63, 0x63,
	0x63, 0x63, 0x63,
	0x63, 0x63,
	0x63,
};

// Huffman table
static MMP_UINT8 DCHuffmanTable[2][28] =
{
	{
		// Luminance Quantization Table
		// the list of code lengths (16 bytes)
		0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// the set of value following this list (12 bytes)
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
	},
	{
		// Chrominance Quantization Table
		// the list of code lengths (16 bytes)
		0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
		// the set of value following this list (12 bytes)
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
	},
};

static MMP_UINT8 ACHuffmanTable[2][178] =
{
	{
		// Luminance Quantization Table
		// the list of code lengths (16 bytes)
		0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
		0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
		// the set of value following this list (162 bytes)
		0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
		0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
		0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
		0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
		0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
		0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
		0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
		0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
		0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
		0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
		0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
		0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
		0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
		0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
		0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
		0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
		0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
		0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
		0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0xf9, 0xfa,
	},
	{
		// Chrominance Quantization Table
		// the list of code lengths (16 bytes)
		0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
		0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
		// the set of value following this list (162 bytes)
		0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
		0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
		0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
		0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
		0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
		0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
		0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
		0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
		0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
		0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
		0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
		0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
		0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
		0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
		0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
		0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
		0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
		0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
		0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0xf9, 0xfa,
	},
};
#endif // #if !defined(DTV_DISABLE_JPG_ENC)

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_UINT8 *GetQuantizationTable(JPEG_Q_TABLE *pQTable, MMP_UINT8 *pReadPtr);
static MMP_UINT8 *GetHuffmanTable(JPEG_IMAGE_COMPONENT *imageComp, MMP_UINT8 *pReadPtr);
static MMP_UINT8 *GetScanParameter(JPEG_IMAGE_COMPONENT *imageComp, MMP_UINT8 *pReadPtr);
static MMP_UINT8 *GetDRIParameter(MMP_UINT16 *pDRI, MMP_UINT8 *pReadPtr);
static MMP_UINT8 *GetFrameParameter(JPEG_IMAGE_COMPONENT *imageComp, MMP_UINT8 *pReadPtr);
static MMP_UINT8 *GetProgressiveParam(JPEG_IMAGE_COMPONENT *imageComp, MMP_UINT8 *pReadPtr);  // for jpeg progressive

#if !defined(DTV_DISABLE_JPG_ENC)
    static MMP_UINT8 *SetQuantizationTable(const MMP_UINT8 *pY_Table,
                                           const MMP_UINT8 *pUV_Table,
                                           MMP_UINT8 *pEncodedBuf);
    static MMP_UINT8 *SetDRIParameter(MMP_UINT16 restartInterval, MMP_UINT8 *pEncodedBuf);
    static MMP_UINT8 *SetHuffmanTable(HUFFMAN_TABLE *pHTable, MMP_UINT8 *pEncodedBuf);
    static MMP_UINT8 *SetScanParameter(JPEG_IMAGE_COMPONENT *imageComp, MMP_UINT8 *pEncodedBuf);
    static MMP_UINT8 *SetFrameParameter(JPEG_IMAGE_COMPONENT *imageComp, MMP_UINT8 *pEncodedBuf);
    static void SetDefaultHuffmanTable(JPEG_IMAGE_COMPONENT *imageComp);    //add for motion jpeg

#endif // #if !defined(DTV_DISABLE_JPG_ENC)

//=============================================================================
//                              Public Function Definition
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
MarkerParser(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               **pCurrent,
    MMP_UINT32              DataSize)
{
	MMP_UINT8   *pReadPtr = *pCurrent;
	MMP_UINT8   *pEndPtr = (*pCurrent) + DataSize;
	MMP_BOOL    bHufTableLoad = MMP_FALSE;

	while ( pReadPtr < pEndPtr)
	{
		if ((*pReadPtr) == JPG_MARKER_START)
		{
			//MMP_Sleep(1);
			pReadPtr++;
			if ( (*pReadPtr) == JPG_MARKER_START)
			{
				continue;
			}
			// skip application data segment
			if (((*pReadPtr) & JPG_MSK_APPL_RESERVED_MARKER) == JPG_APPL_RESERVED_MARKER)
			{
				pReadPtr++;
				SKIP_SEGMENT(pReadPtr);
				continue;
			}

			// skip JPEG extensions segment
			if (((*pReadPtr) & JPG_MSK_JPEG_EXTENSIONS_MARKER) == JPG_JPEG_EXTENSIONS_MARKER)
			{
				pReadPtr++;
				SKIP_SEGMENT(pReadPtr);
				continue;
			}

			switch (*pReadPtr)
			{
				case JPG_BASELINE_DCT_MARKER :
					pReadPtr++;
					pReadPtr = GetFrameParameter(imageComp, pReadPtr);
					break;

					// not supported feature
					/*case JPG_EXTENDED_SEQUENTIAL_DCT :
					case JPG_LOSSLESS_SEQUENTIAL :
					    pCurrent++;
					    SKIP_SEGMENT(pCurrent);
					    break;*/

				case JPG_PROGRESSIVE_DCT :
					pReadPtr++;
					imageComp->progressive = MMP_TRUE;
					pReadPtr = GetProgressiveParam(imageComp, pReadPtr);
					break;

				case JPG_HUFFMAN_TABLE_MARKER :
					pReadPtr++;
					pReadPtr = GetHuffmanTable(imageComp, pReadPtr);
					if (imageComp->progressive != MMP_TRUE)
						bHufTableLoad = MMP_TRUE;
					break;

				case JPG_START_OF_IMAGE_MARKER :
					pReadPtr++;
					break;

				case JPG_START_OF_SCAN_MARKER :
					pReadPtr++;
					pReadPtr = GetScanParameter(imageComp, pReadPtr);

#ifdef NOT_SUPPORT_NON_INTERLEAVED
					if (imageComp->bNonInterleaved == MMP_TRUE)
					{
						return (ERROR_JPG_MarkerParser_Non_Interleaved);
					}
#endif
					break;

				case JPG_Q_TABLE_MARKER :
					pReadPtr++;
					pReadPtr = GetQuantizationTable(&imageComp->qTable , pReadPtr);
					break;

				case JPG_DRI_MARKER :
					pReadPtr++;
					pReadPtr = GetDRIParameter(&imageComp->restartInterval, pReadPtr);
					break;

				case 0x00:
					*pCurrent = (pReadPtr - 1);
					return (JPG_ERROR_SUCCESS);

				default :
					// not supported feature like EXTENDED_SEQUENTIAL_DCT, PROGRESSIVE_DCT, LOSSLESS_SEQUENTIAL
					return (ERROR_JPG_MarkerParser_Unsupported_Marker);
			}
		} // End of if ((*pJpgStream) == JPG_MSK_MARKER)
		else
		{
			*pCurrent = pReadPtr;
			if (!bHufTableLoad)
			{
#if !defined(DTV_DISABLE_JPG_ENC)
				SetDefaultHuffmanTable(imageComp);
#else
				return (ERROR_JPG_MarkerParser);
#endif
			}
			return (JPG_ERROR_SUCCESS);
		}
	} // End of while (1)

	LOG_ERROR "ERROR_JPG_MarkerParser, %s [#%d]\n", __FILE__, __LINE__ LOG_END
	
	return (ERROR_JPG_MarkerParser);
}


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
DataParser(
    MMP_UINT8 **pCurrent,
    MMP_UINT32 *pdwToBSSize,
    MMP_UINT32 DataSize,
    MMP_BOOL bNonInterleaved)
{
	MMP_UINT8 *pReadPtr = *pCurrent;
	MMP_UINT8 *pEndPtr = (*pCurrent) + DataSize;

	//mylin
	//if(ctxt->gChipVersion > MMP220_A0) // not A0 version
	{
		*pdwToBSSize = DataSize;
		return (JPG_ERROR_SUCCESS);
	}

	if (bNonInterleaved == MMP_FALSE)
	{
		// interleaved image
		MMP_UINT32 dwToBSSize = DataSize;

		while (pReadPtr < pEndPtr)
		{
			if (((*pReadPtr) == JPG_MARKER_START) && ((*(pReadPtr + 1)) == JPG_END_OF_IMAGE_MARKER))
			{
				dwToBSSize = pReadPtr - (*pCurrent);
				*pdwToBSSize = dwToBSSize;
				if (DataSize > (dwToBSSize + 2))
				{
					//LOG_INFO "Have other bit-stream data = 0x%X \n", (DataSize - (dwToBSSize+2)) );
				}
				return (JPG_ERROR_SUCCESS);
			}
			pReadPtr++;
		}
		return (ERROR_JPG_DataParser);
	}
	else
	{
		// non-unterleaved image
		MMP_UINT32 dwToBSSize = 0;

		while (pReadPtr != pEndPtr)
		{
			if ((*pReadPtr) == JPG_MARKER_START)
			{
				if ((*(pReadPtr + 1)) == JPG_END_OF_IMAGE_MARKER || (*(pReadPtr + 1)) == JPG_START_OF_SCAN_MARKER)
				{
					dwToBSSize = pReadPtr - (*pCurrent);
					*pdwToBSSize = dwToBSSize;
					return (JPG_ERROR_SUCCESS);
				}
			}
			pReadPtr++;
		}
		return (ERROR_JPG_DataParser);
	}
}

JPG_ERROR_CODE
DataParserMultiSection(
    MMP_UINT8   **pCurrent,
    MMP_UINT32  *pdwToBSSize,
    MMP_UINT32  DataSize,
    MMP_BOOL    bNonInterleaved)
{
	MMP_UINT8 *pReadPtr = *pCurrent;
	MMP_UINT8 *pEndPtr = (*pCurrent) + DataSize;

	//mylin
	//if(ctxt->gChipVersion > MMP220_A0) // not A0 version
	{
		*pdwToBSSize = DataSize;
		return (JPG_ERROR_SUCCESS);
	}

	if (bNonInterleaved == MMP_FALSE)
	{
		// interleaved image
		MMP_UINT32 dwToBSSize = DataSize;

		while (pReadPtr < pEndPtr)
		{
			if (((*pReadPtr) == JPG_MARKER_START) && ((*(pReadPtr + 1)) == JPG_END_OF_IMAGE_MARKER))
			{
				dwToBSSize = pReadPtr - (*pCurrent);
				*pdwToBSSize = dwToBSSize;
				if (DataSize > (dwToBSSize + 2))
				{
					//CORE_Printf("Have other bit-stream data = 0x%X \n", (DataSize - (dwToBSSize+2)));
				}
				return (JPG_ERROR_SUCCESS);
			}
			pReadPtr++;
		}
		*pdwToBSSize = DataSize;
		return (JPG_ERROR_SUCCESS);
	}
	else
	{
		// non-interleaved image
		MMP_UINT32 dwToBSSize = 0;

		while (pReadPtr != pEndPtr)
		{
			if ((*pReadPtr) == JPG_MARKER_START)
			{
				if ((*(pReadPtr + 1)) == JPG_END_OF_IMAGE_MARKER || (*(pReadPtr + 1)) == JPG_START_OF_SCAN_MARKER)
				{
					dwToBSSize = pReadPtr - (*pCurrent);
					*pdwToBSSize = dwToBSSize;
					return (JPG_ERROR_SUCCESS);
				}
			}
			pReadPtr++;
		}
		*pdwToBSSize = DataSize;
		return (JPG_ERROR_SUCCESS);
	}
}

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
JPEG_SetDecodeColorFormat(
    JPEG_IMAGE_COMPONENT    *imageComp)
{
	/**
	 * Support format
	 *         Y         U         V
	 *       H   V     H   V     H   V
	 * 444   1   1     1   1     1   1
	 * 422   2   1     1   1     1   1
	 * 422R  1   2     1   1     1   1
	 * 420   2   2     1   1     1   1
	 * 411   4   1     1   1     1   1
	 */
	imageComp->widthUnit  = imageComp->imageInfo[0].horizonSamp * 8;
	imageComp->heightUnit = imageComp->imageInfo[0].verticalSamp * 8;	

    LOG_ERROR "Image Sample Frequency = (%d %d %d %d %d %d)  \n",
	        imageComp->imageInfo[0].horizonSamp,
	        imageComp->imageInfo[0].verticalSamp,
	        imageComp->imageInfo[1].horizonSamp,
	        imageComp->imageInfo[1].verticalSamp,
	        imageComp->imageInfo[2].horizonSamp,
	        imageComp->imageInfo[2].verticalSamp,
	        imageComp->imageInfo[3].horizonSamp,
	        imageComp->imageInfo[3].verticalSamp LOG_END
    
	//dkwang mark
	//if( (imageComp->imageInfo[1].horizonSamp  != 1) ||
	//    (imageComp->imageInfo[1].verticalSamp != 1) ||
	//    (imageComp->imageInfo[2].horizonSamp  != 1) ||
	//    (imageComp->imageInfo[2].verticalSamp != 1) )
	//{
	//    return (ERROR_JPG_SetDecodeColorFormat);
	//}

	imageComp->realWidth  = (imageComp->imageWidth  + (imageComp->widthUnit - 1)) & ~(imageComp->widthUnit - 1);
	imageComp->realHeight = (imageComp->imageHeight + (imageComp->heightUnit - 1)) & ~(imageComp->heightUnit - 1);

	if (imageComp->singleChannel)
	{
		imageComp->colorFormat = JPEG_YUV_444;
		return (JPG_ERROR_SUCCESS);
	}

	if ( (imageComp->imageInfo[0].horizonSamp == 1) &&
	     (imageComp->imageInfo[0].verticalSamp == 1) )
	{
		imageComp->colorFormat = JPEG_YUV_444;
	}
	else if ( (imageComp->imageInfo[0].horizonSamp == 2) &&
	          (imageComp->imageInfo[0].verticalSamp == 1) )
	{
		imageComp->colorFormat = JPEG_YUV_422;
	}
	else if ( (imageComp->imageInfo[0].horizonSamp == 1) &&
	          (imageComp->imageInfo[0].verticalSamp == 2) )
	{
		imageComp->colorFormat = JPEG_YUV_422R;
	}
	else if ( (imageComp->imageInfo[0].horizonSamp == 2) &&
	          (imageComp->imageInfo[0].verticalSamp == 2) )
	{
		if ( (imageComp->imageInfo[1].horizonSamp == 1) &&
		     (imageComp->imageInfo[1].verticalSamp == 2) )
			imageComp->colorFormat = JPEG_YUV_422;
		else
			imageComp->colorFormat = JPEG_YUV_420;
	}
	else if ( (imageComp->imageInfo[0].horizonSamp == 4) &&
	          (imageComp->imageInfo[0].verticalSamp == 1) )
	{
		imageComp->colorFormat = JPEG_YUV_411;
	}
	else
	{		
		LOG_ERROR "ERROR_JPG_SetDecodeColorFormat, %s [#%d]\n", __FILE__, __LINE__ LOG_END
	        
		return (ERROR_JPG_SetDecodeColorFormat);
	}

	return (JPG_ERROR_SUCCESS);
}


#if !defined(DTV_DISABLE_JPG_ENC)
//=============================================================================
/**
 * Set Marker
 *
 * @param imageComp [IN]
 * @param pQTable_Y [IN]
 * @param pQTable_UV [IN]
 * @param pCurrent [Out]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 *
 */
//=============================================================================
JPG_ERROR_CODE
SetMarker(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               **pCurrent)
{
	MMP_UINT8  stuffByte, i;
	MMP_UINT8 *pTempBuf = *pCurrent;	

	START_OF_IMAGE(pTempBuf);
	pTempBuf = SetQuantizationTable(&(imageComp->qTable.Table[0][0]),
	                                &(imageComp->qTable.Table[1][0]),
	                                pTempBuf);

	//pTempBuf = SetDRIParameter(imageComp->restartInterval, pTempBuf);

	pTempBuf = SetFrameParameter(imageComp, pTempBuf);

	pTempBuf = SetHuffmanTable(&imageComp->huffmanTable, pTempBuf);

//******************************************************************************
//#ifdef _CREATE_EVEN_BYTE_HEADER_
//    stuffByte = (pTempBuf - *pCurrent) % 8;
//	
//	for (i = 0; i < stuffByte; i++)
//	{
//		*pTempBuf = 0xFF;
//		pTempBuf++;
//	}		
//#endif
//******************************************************************************

	pTempBuf = SetScanParameter(imageComp, pTempBuf);

	*pCurrent = pTempBuf;

	return (JPG_ERROR_SUCCESS);
}


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
JpgQualityTable(
    MMP_UINT8   *pY_Table,
    MMP_UINT8   *pUV_Table,
    MMP_UINT16  wQuality)
{
	MMP_UINT16 i;

	// adjust Q table according to quality value
	if (wQuality == 0)
	{
		wQuality = 1;
	}

	if (wQuality == 1)
	{
		for (i = 0; i < JPEG_Q_TABLE_SIZE; i++)
		{
			pY_Table[i] = gY_QTable[i];
			pUV_Table[i] = gUV_QTable[i];
		}
	}
	else
	{
		if (wQuality < 50)
		{
			wQuality = 5000 / wQuality;
		}
		else
		{
			wQuality = 200 - wQuality * 2;
		}

		// adjust Q table
		for (i = 0; i < JPEG_Q_TABLE_SIZE; i++)
		{
			// luminance
			pY_Table[i] = (gY_QTable[i] * wQuality + 50) / 100;
			if (pY_Table[i] == 0)
			{
				pY_Table[i] = 1;
			}

			// chrominance
			pUV_Table[i] = (gUV_QTable[i] * wQuality + 50) / 100;
			if (pUV_Table[i] == 0)
			{
				pUV_Table[i] = 1;
			}
		}// for (i=0; i<Q_TABLE_SIZE; i++)
	}// if(wQuality == 1)
}


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
JPG_Encode_GetHuffmanTable(
    MMP_UINT8   **pY_DC,
    MMP_UINT8   **pY_AC,
    MMP_UINT8   **pUV_DC,
    MMP_UINT8   **pUV_AC)
{
	*pY_DC  = &DCHuffmanTable[0][0];
	*pY_AC  = &ACHuffmanTable[0][0];
	*pUV_DC = &DCHuffmanTable[1][0];
	*pUV_AC = &ACHuffmanTable[1][0];
}


//=============================================================================
/**
 * Set Image component
 *
 * @param imageComp [Out]
 * @return void
 *
 */
//=============================================================================
void JPG_SetEncodeImageComp(
    JPEG_IMAGE_COMPONENT    *imageComp)
{

	switch (imageComp->encodeFmt)
	{
		case JPEG_YUV_422:
			imageComp->imageInfo[0].horizonSamp       = 2;
			imageComp->imageInfo[0].verticalSamp      = 1;

			imageComp->imageInfo[1].horizonSamp       = 1;
			imageComp->imageInfo[1].verticalSamp      = 1;

			imageComp->imageInfo[2].horizonSamp       = 1;
			imageComp->imageInfo[2].verticalSamp      = 1;
			break;

		case JPEG_YUV_420:
			imageComp->imageInfo[0].horizonSamp       = 2;
			imageComp->imageInfo[0].verticalSamp      = 2;

			imageComp->imageInfo[1].horizonSamp       = 1;
			imageComp->imageInfo[1].verticalSamp      = 1;

			imageComp->imageInfo[2].horizonSamp       = 1;
			imageComp->imageInfo[2].verticalSamp      = 1;
			break;

		default :
			
			LOG_ERROR "Worng Encode format !! %s [#%d]\n", __FILE__, __LINE__ LOG_END
			
			break;
	}


	imageComp->imageInfo[0].qTableSel         = 0;
	imageComp->imageInfo[0].dcHuffmanTableSel = 0;
	imageComp->imageInfo[0].acHuffmanTableSel = 0;


	imageComp->imageInfo[1].qTableSel         = 1;
	imageComp->imageInfo[1].dcHuffmanTableSel = 1;
	imageComp->imageInfo[1].acHuffmanTableSel = 1;


	imageComp->imageInfo[2].qTableSel         = 1;
	imageComp->imageInfo[2].dcHuffmanTableSel = 1;
	imageComp->imageInfo[2].acHuffmanTableSel = 1;

}

#endif // #if !defined(DTV_DISABLE_JPG_ENC)

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Get Quantization Table
 *
 * @param pQTable [Out]
 * @param pReadPtr [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
GetQuantizationTable(
    JPEG_Q_TABLE   *pQTable,
    MMP_UINT8      *pReadPtr)
{
	MMP_UINT16  wSegLength = 0;
	MMP_UINT16  wQTableCount = 0;
	MMP_UINT16  wQTableIdx = 0;
	MMP_UINT16  i;
	MMP_BOOL    bOneQTable = MMP_FALSE;

	GET_WORD_VALUE(pReadPtr, wSegLength);

	wQTableCount = (wSegLength - JPG_LENGTH_FIELD) / (JPG_Q_TABLE_SEGMENT_LENGTH);

	for (i = 0; i < wQTableCount; i++)
	{
		GET_LOW_BYTE_VALUE(pReadPtr, wQTableIdx);

		// skip the precision & destination field identifier of Q table
		pReadPtr++;

		SYS_Memcpy(pQTable->Table[wQTableIdx], pReadPtr, JPG_Q_TABLE_ELEMENT_NUM);
		pReadPtr += JPG_Q_TABLE_ELEMENT_NUM;
	}

	pQTable->tableCount += wQTableCount;

	return pReadPtr;
}


//=============================================================================
/**
 * Get Huffman Table
 *
 * @param pReadPtr [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
GetHuffmanTable(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               *pReadPtr)
{
	MMP_UINT16  wSegLength = 0;
	MMP_UINT16  wHuffmanTableClass = 0;    // 0:DC, 1:AC
	MMP_UINT16  wHuffmanTableDst = 0;
	MMP_UINT8   *pReadPtrEnd = 0;
	MMP_UINT16  i;

	GET_WORD_VALUE(pReadPtr, wSegLength);
	pReadPtrEnd = pReadPtr + wSegLength - JPG_LENGTH_FIELD;

	while (pReadPtr < pReadPtrEnd)
	{
		GET_HIGH_BYTE_VALUE(pReadPtr, wHuffmanTableClass);
		GET_LOW_BYTE_VALUE(pReadPtr, wHuffmanTableDst);

		pReadPtr++;

		if (wHuffmanTableClass == 0)
		{
			// DC Huffman table
			MMP_UINT16 index = 0;

			index = ((wHuffmanTableDst == 0) ? wHuffmanTableDst : 1);
			imageComp->huffmanTable.DC[index].pHuffmanTable = pReadPtr;
			imageComp->huffmanTable.DC[index].totalCodeLengthCount = 0;

			for (i = 0; i < 16; i++)
			{
				imageComp->huffmanTable.DC[index].totalCodeLengthCount += *(imageComp->huffmanTable.DC[index].pHuffmanTable + i);
			}

			pReadPtr += 16;
			pReadPtr += imageComp->huffmanTable.DC[index].totalCodeLengthCount;
		}
		else
		{
			// AC huffman table
			MMP_UINT16 index = 0;

			index = ((wHuffmanTableDst == 0) ? wHuffmanTableDst : 1);
			imageComp->huffmanTable.AC[index].pHuffmanTable = pReadPtr;
			imageComp->huffmanTable.AC[index].totalCodeLengthCount = 0;

			for (i = 0; i < 16; i++)
			{
				imageComp->huffmanTable.AC[index].totalCodeLengthCount += *(imageComp->huffmanTable.AC[index].pHuffmanTable + i);
			}

			pReadPtr += 16;
			pReadPtr += imageComp->huffmanTable.AC[index].totalCodeLengthCount;
		}
	}
	imageComp->bDefaultHuffmanTable = MMP_TRUE;
	return pReadPtr;
}

//=============================================================================
/**
 * Get JPEG progressive header
 *
 * @param pReadPtr [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
GetProgressiveParam(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               *pReadPtr)
{
	MMP_UINT16  frameHeaderLength = 0;
	MMP_UINT16  SamplePrecision = 0;
	MMP_UINT16  componentIndex = 0;
	MMP_UINT8   *pReadPtrEnd = 0;
	MMP_UINT16  tmpValue = 0;

	GET_WORD_VALUE(pReadPtr, frameHeaderLength);
	pReadPtrEnd = pReadPtr + frameHeaderLength - JPG_LENGTH_FIELD;

	GET_BYTE_VALUE(pReadPtr, SamplePrecision);

	GET_WORD_VALUE(pReadPtr, imageComp->imageHeight);
	GET_WORD_VALUE(pReadPtr, imageComp->imageWidth);

	GET_BYTE_VALUE(pReadPtr, imageComp->componentNum);

	if (imageComp->componentNum == 1)
		imageComp->singleChannel = MMP_TRUE;
	else
		imageComp->singleChannel = MMP_FALSE;

	while (pReadPtr < pReadPtrEnd)
	{
		GET_BYTE_VALUE(pReadPtr, componentIndex);
		switch (componentIndex)
		{
			case 1:
				GET_BYTE_VALUE(pReadPtr, tmpValue);
				imageComp->imageInfo[0].horizonSamp = (tmpValue & 0xF0) >> 4;
				imageComp->imageInfo[0].verticalSamp = (tmpValue & 0x0F);
				imageComp->imageInfo[0].componentId = 1;
				GET_BYTE_VALUE(pReadPtr, imageComp->imageInfo[0].qTableSel);

				/*printf("comp 1: SampH = %d, SampV = %d, Quant_Tbl_Sel = 0x%x\n", imageComp->imageInfo[0].horizonSamp,
				                                                                 imageComp->imageInfo[0].verticalSamp,
				                                                                 imageComp->imageInfo[0].qTableSel); //*/
				break;

			case 2:
				GET_BYTE_VALUE(pReadPtr, tmpValue);
				imageComp->imageInfo[1].horizonSamp = (tmpValue & 0xF0) >> 4;
				imageComp->imageInfo[1].verticalSamp = (tmpValue & 0x0F);
				imageComp->imageInfo[1].componentId = 2;
				GET_BYTE_VALUE(pReadPtr, imageComp->imageInfo[1].qTableSel);

				/*printf("comp 2: SampH = %d, SampV = %d, Quant_Tbl_Sel = 0x%x\n", imageComp->imageInfo[1].horizonSamp,
				                                                                 imageComp->imageInfo[1].verticalSamp,
				                                                                 imageComp->imageInfo[1].qTableSel);//*/
				break;

			case 3:
				GET_BYTE_VALUE(pReadPtr, tmpValue);
				imageComp->imageInfo[2].horizonSamp = (tmpValue & 0xF0) >> 4;
				imageComp->imageInfo[2].verticalSamp = (tmpValue & 0x0F);
				imageComp->imageInfo[2].componentId = 3;
				GET_BYTE_VALUE(pReadPtr, imageComp->imageInfo[2].qTableSel);

				/*printf("comp 3: SampH = %d, SampV = %d, Quant_Tbl_Sel = 0x%x\n", imageComp->imageInfo[2].horizonSamp,
				                                                                 imageComp->imageInfo[2].verticalSamp,
				                                                                 imageComp->imageInfo[2].qTableSel);//*/
				break;

			case 4:
				GET_BYTE_VALUE(pReadPtr, tmpValue);
				imageComp->imageInfo[3].horizonSamp = (tmpValue & 0xF0) >> 4;
				imageComp->imageInfo[3].verticalSamp = (tmpValue & 0x0F);
				imageComp->imageInfo[3].componentId = 4;
				GET_BYTE_VALUE(pReadPtr, imageComp->imageInfo[3].qTableSel);

				/*printf("comp 4: SampH = %d, SampV = %d, Quant_Tbl_Sel = 0x%x\n", imageComp->imageInfo[3].horizonSamp,
				                                                                 imageComp->imageInfo[3].verticalSamp,
				                                                                 imageComp->imageInfo[3].qTableSel);//*/
				break;

		}

	}

	return pReadPtr;

}

//=============================================================================
/**
 * Get Scan Parameter
 *
 * @param imageComp [Out]
 * @param pReadPtr [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
GetScanParameter(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               *pReadPtr)
{
	MMP_UINT16  wSegLength = 0;
	MMP_UINT16  wComponentCount = 0;
	MMP_UINT16  wCompIdx = 0;
	MMP_UINT16  i, j;
	MMP_UINT16  bIndexFrom0 = MMP_FALSE;
	MMP_UINT16  tmpIndex = 0;

	GET_WORD_VALUE(pReadPtr, wSegLength);

	GET_BYTE_VALUE(pReadPtr, wComponentCount);

	imageComp->validComponent = 0;
	for (i = 0; i < wComponentCount; i++)
	{
		GET_BYTE_VALUE(pReadPtr, wCompIdx);

		for (j = 0; j < wComponentCount; j++)
		{
			if (imageComp->imageInfo[j].componentId == wCompIdx)
			{
				GET_HIGH_BYTE_VALUE(pReadPtr, imageComp->imageInfo[j].dcHuffmanTableSel);
				GET_LOW_BYTE_VALUE(pReadPtr, imageComp->imageInfo[j].acHuffmanTableSel);
				pReadPtr++;

				switch (j)
				{
					case 0:
						imageComp->validComponent |= JPEG_MSK_DEC_COMPONENT_A_VALID;
						break;

					case 1:
						imageComp->validComponent |= JPEG_MSK_DEC_COMPONENT_B_VALID;
						break;

					case 2:
						imageComp->validComponent |= JPEG_MSK_DEC_COMPONENT_C_VALID;
						break;

					case 3:
						imageComp->validComponent |= JPEG_MSK_DEC_COMPONENT_D_VALID;
						break;
				}
			}
		}
	}

	// todo : Ss, Se, Ah, Al
	pReadPtr += 3;
	//imageComp->bNonInterleaved = ((wComponentCount == 1) ? MMP_TRUE : MMP_FALSE);

	if (!imageComp->singleChannel && wComponentCount == 1)
		imageComp->bNonInterleaved = MMP_TRUE;
	else
		imageComp->bNonInterleaved = MMP_FALSE;

	imageComp->componentNum = wComponentCount;
	return pReadPtr;
}


//=============================================================================
/**
 * Get DRI Parameter
 *
 * @param pRestartInterval [Out]
 * @param pReadPtr [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
GetDRIParameter(
    MMP_UINT16  *pDRI,
    MMP_UINT8   *pReadPtr)
{
	MMP_UINT16  wSegLength;
	MMP_UINT16  restartInterval;

	GET_WORD_VALUE(pReadPtr, wSegLength);
	GET_WORD_VALUE(pReadPtr, restartInterval);

	*pDRI = restartInterval;
	return pReadPtr;
}


//=============================================================================
/**
 * Get Frame Parameter
 *
 * @param imageComp [Out]
 * @param pReadPtr [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
GetFrameParameter(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               *pReadPtr)
{
	MMP_UINT16  wSegLength = 0;
	MMP_UINT16  wComponentCount = 0;
	MMP_UINT16  wCompIdx = 0;
	MMP_UINT16  i;
	MMP_UINT16  bIndexFrom0 = MMP_FALSE;

	GET_WORD_VALUE(pReadPtr, wSegLength);

	// skip the sample precision field of scan segment
	pReadPtr++;

	GET_WORD_VALUE(pReadPtr, imageComp->imageHeight);
	GET_WORD_VALUE(pReadPtr, imageComp->imageWidth);

	GET_BYTE_VALUE(pReadPtr, wComponentCount);

	if (wComponentCount == 1)
		imageComp->singleChannel = MMP_TRUE;
	else
		imageComp->singleChannel = MMP_FALSE;

	for (i = 0; i < wComponentCount; i++)
	{
		GET_BYTE_VALUE (pReadPtr, imageComp->imageInfo[i].componentId);
		GET_HIGH_BYTE_VALUE (pReadPtr, imageComp->imageInfo[i].horizonSamp);
		GET_LOW_BYTE_VALUE (pReadPtr, imageComp->imageInfo[i].verticalSamp);

		pReadPtr++;

//        GET_BYTE_VALUE(pReadPtr, imageComp->imageInfo[wCompIdx-1].qTableSel);
		GET_BYTE_VALUE(pReadPtr, imageComp->imageInfo[i].qTableSel);
	}

	return pReadPtr;
}


#if !defined(DTV_DISABLE_JPG_ENC)
//=============================================================================
/**
 * Set Quantization Table
 *
 * @param pY_Table [IN]
 * @param pUV_Table [IN]
 * @param pEncodedBuf [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
SetQuantizationTable(
    const MMP_UINT8     *pY_Table,
    const MMP_UINT8     *pUV_Table,
    MMP_UINT8           *pEncodedBuf)
{
	MMP_UINT16   wQTableCount = 2;
	MMP_UINT16   wSegLength   = JPG_LENGTH_FIELD + JPG_Q_TABLE_SEGMENT_LENGTH * wQTableCount ;

	// add Q Table marker
	DEFINE_QUANZITATION_TABLE(pEncodedBuf);
	SET_WORD_VALUE(pEncodedBuf, wSegLength);

	CLEAN_BYTE_VALUE(pEncodedBuf);
	SET_HIGH_BYTE_VALUE(pEncodedBuf, 0);
	SET_LOW_BYTE_VALUE(pEncodedBuf, 0);

	// skip the precision & destination field identifier of Q table
	pEncodedBuf++;
	SYS_Memcpy(pEncodedBuf, pY_Table, JPG_Q_TABLE_ELEMENT_NUM);
	pEncodedBuf += JPG_Q_TABLE_ELEMENT_NUM;

	CLEAN_BYTE_VALUE(pEncodedBuf);
	SET_HIGH_BYTE_VALUE(pEncodedBuf, 0);
	SET_LOW_BYTE_VALUE(pEncodedBuf, 1);

	// skip the precision & destination field identifier of Q table
	pEncodedBuf++;
	SYS_Memcpy(pEncodedBuf, pUV_Table, JPG_Q_TABLE_ELEMENT_NUM);
	pEncodedBuf += JPG_Q_TABLE_ELEMENT_NUM;

	return pEncodedBuf;
}

//=============================================================================
/**
 * Set DRI Parameter
 *
 * @param pEncodedBuf [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
SetDRIParameter(
    MMP_UINT16  restartInterval,
    MMP_UINT8  *pEncodedBuf)
{
	MMP_UINT16  wSegLength = 4;

	// to do :
	DEFINE_DRI_MARKER(pEncodedBuf);
	SET_WORD_VALUE(pEncodedBuf, wSegLength);
	SET_WORD_VALUE(pEncodedBuf, restartInterval);

	return pEncodedBuf;
}

//=============================================================================
/**
 * Set Huffman Table
 *
 * @param pEncodedBuf [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
SetHuffmanTable(
    HUFFMAN_TABLE   *pHTable,
    MMP_UINT8      *pEncodedBuf)
{
	MMP_UINT16  wHuffmanTableDst = 0;
	MMP_UINT16  wSegLength = JPG_LENGTH_FIELD + 29 * 2 + 179 * 2; // todo : constant

	// add Huffman table amrker
	DEFINE_HUFFMAN_TABLE(pEncodedBuf);

	SET_WORD_VALUE(pEncodedBuf, wSegLength);

	for (wHuffmanTableDst = 0; wHuffmanTableDst < 2; wHuffmanTableDst++)
	{
		// DC Huffman table
		CLEAN_BYTE_VALUE(pEncodedBuf);
		SET_HIGH_BYTE_VALUE(pEncodedBuf, 0);
		SET_LOW_BYTE_VALUE(pEncodedBuf, wHuffmanTableDst);

		pEncodedBuf++;

		// copy "Number of Huffman codes of length i"
		SYS_Memcpy(pEncodedBuf, &DCHuffmanTable[wHuffmanTableDst][0], 28);

		pHTable->DC[wHuffmanTableDst].pHuffmanTable = (&DCHuffmanTable[wHuffmanTableDst][0]);
		pHTable->DC[wHuffmanTableDst].totalCodeLengthCount = 12;    // 12 = 28 - 16
		pEncodedBuf += 28;

		// AC Huffman table
		CLEAN_BYTE_VALUE(pEncodedBuf);
		SET_HIGH_BYTE_VALUE(pEncodedBuf, 1);
		SET_LOW_BYTE_VALUE(pEncodedBuf, wHuffmanTableDst);

		pEncodedBuf++;

		// copy "Number of Huffman codes of length i"
		SYS_Memcpy(pEncodedBuf, &ACHuffmanTable[wHuffmanTableDst][0], 178);

		pHTable->AC[wHuffmanTableDst].pHuffmanTable = (&ACHuffmanTable[wHuffmanTableDst][0]);
		pHTable->AC[wHuffmanTableDst].totalCodeLengthCount = 162;   // 162 = 178 - 16
		pEncodedBuf += 178;
	}

	return pEncodedBuf;
}


//=============================================================================
/**
 * Set Scan Parameter
 *
 * @param pEncodedBuf [IN]
 * @param imageComp [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
SetScanParameter(
    JPEG_IMAGE_COMPONENT    *imageComp,
    MMP_UINT8               *pEncodedBuf)
{
	MMP_UINT16  wComponentCount = 3;
	MMP_UINT16  wSegLength = 6 + 2 * wComponentCount;
	MMP_UINT16  wCompIdx = 0;

	// add start of scan marker
	START_OF_SCAN(pEncodedBuf);

	SET_WORD_VALUE(pEncodedBuf, wSegLength);

	SET_BYTE_VALUE(pEncodedBuf, (MMP_UINT8)wComponentCount);

	for (wCompIdx = 0; wCompIdx < wComponentCount; wCompIdx++)
	{
		SET_BYTE_VALUE(pEncodedBuf, (MMP_UINT8)(wCompIdx + 1));

		CLEAN_BYTE_VALUE(pEncodedBuf);
		SET_HIGH_BYTE_VALUE(pEncodedBuf, imageComp->imageInfo[wCompIdx].dcHuffmanTableSel);
		SET_LOW_BYTE_VALUE(pEncodedBuf, imageComp->imageInfo[wCompIdx].acHuffmanTableSel);
		pEncodedBuf++;
	}

	// skip Ss
	CLEAN_BYTE_VALUE(pEncodedBuf);
	pEncodedBuf++;

	// set Se to 63
	SET_BYTE_VALUE(pEncodedBuf, 63);

	// skip Ah, Al
	CLEAN_BYTE_VALUE(pEncodedBuf);
	pEncodedBuf++;

	return pEncodedBuf;
}


//=============================================================================
/**
 * Set Frame Parameter
 *
 * @param ImageWidth [IN]
 * @param ImageHeight [IN]
 * @param pEncodedBuf [IN]
 * @param imageComp [IN]
 * @return current pointer
 *
 */
//=============================================================================
static MMP_UINT8*
SetFrameParameter(
    JPEG_IMAGE_COMPONENT     *imageComp,
    MMP_UINT8               *pEncodedBuf)
{
	MMP_UINT16  wComponentCount = 3;
	MMP_UINT16  wSegLength = 8 + 3 * wComponentCount;
	MMP_UINT16  i;

	// add Baseline DCT marker
	BASELINE_DCT(pEncodedBuf);

	SET_WORD_VALUE(pEncodedBuf, wSegLength);

	// set "8" for the sample precision field of scan segment
	SET_BYTE_VALUE(pEncodedBuf, 8);

	SET_WORD_VALUE(pEncodedBuf, imageComp->imageHeight);
	SET_WORD_VALUE(pEncodedBuf, imageComp->imageWidth);

	SET_BYTE_VALUE(pEncodedBuf, (MMP_UINT8)wComponentCount);

	for (i = 0; i < wComponentCount; i++)
	{
		SET_BYTE_VALUE(pEncodedBuf, (i + 1));

		CLEAN_BYTE_VALUE(pEncodedBuf);
		SET_HIGH_BYTE_VALUE(pEncodedBuf, imageComp->imageInfo[i].horizonSamp);
		SET_LOW_BYTE_VALUE(pEncodedBuf, imageComp->imageInfo[i].verticalSamp);
		pEncodedBuf++;

		SET_BYTE_VALUE(pEncodedBuf, (MMP_UINT8)imageComp->imageInfo[i].qTableSel);
	}

	return pEncodedBuf;
}


//=============================================================================
/**
 * Set Default Huffman Table if no Huffman Table in the bitstream
 *
 */
//=============================================================================
static void
SetDefaultHuffmanTable(
    JPEG_IMAGE_COMPONENT    *imageComp)
{
	MMP_UINT16 wHuffmanTableDst = 0;

	for (wHuffmanTableDst = 0; wHuffmanTableDst < 2; wHuffmanTableDst++)
	{
		imageComp->huffmanTable.DC[wHuffmanTableDst].pHuffmanTable = (&DCHuffmanTable[wHuffmanTableDst][0]);
		imageComp->huffmanTable.DC[wHuffmanTableDst].totalCodeLengthCount = 12;

		imageComp->huffmanTable.AC[wHuffmanTableDst].pHuffmanTable = (&ACHuffmanTable[wHuffmanTableDst][0]);
		imageComp->huffmanTable.AC[wHuffmanTableDst].totalCodeLengthCount = 162;
	}
}

#endif // #if !defined(DTV_DISABLE_JPG_ENC)

