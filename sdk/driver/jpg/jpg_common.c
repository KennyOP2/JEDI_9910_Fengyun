/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  JPEG encoder/decoder common setting
 *
 * @author 
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "pal/pal.h"
#include "jpg/config.h"
#include "jpg/jpg_hw.h"
#include "host/host.h"
#include "jpg/jpg.h"


//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              Structure Definition
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================

MMP_UINT16 jpegBufferControl[5] =
{
	0,
	JPEG_MSK_LINE_BUF_COMPONENT_1_VALID,
	(JPEG_MSK_LINE_BUF_COMPONENT_1_VALID | JPEG_MSK_LINE_BUF_COMPONENT_2_VALID),
	(JPEG_MSK_LINE_BUF_COMPONENT_1_VALID | JPEG_MSK_LINE_BUF_COMPONENT_2_VALID | JPEG_MSK_LINE_BUF_COMPONENT_3_VALID),
	(JPEG_MSK_LINE_BUF_COMPONENT_1_VALID | JPEG_MSK_LINE_BUF_COMPONENT_2_VALID | JPEG_MSK_LINE_BUF_COMPONENT_3_VALID | JPEG_MSK_LINE_BUF_COMPONENT_4_VALID)
};

//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Public Function Definition
//=============================================================================
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
    JPEG_BITSTREAM_MODE     jpegMode,
    JPEG_IMAGE_COMPONENT   *imageComp,
    JPEG_IMAGE_SIZE_INFO   *sizeInfo)
{
	JPG_ERROR_CODE        result = JPG_ERROR_SUCCESS;
	JPEG_DISPLAY_INFO     *dispInfo = &ctxt->displayInfo;

	sizeInfo->realWidth     = imageComp->realWidth;
	sizeInfo->realHeight    = imageComp->realHeight;

	sizeInfo->mcuRealWidth  = sizeInfo->realWidth / imageComp->widthUnit;
	sizeInfo->mcuRealHeight = sizeInfo->realHeight / imageComp->heightUnit;

	if (jpegMode == JPEG_BITSTREAM_ENCODE)
	{
		sizeInfo->mcuDispWidth  = sizeInfo->mcuRealWidth;
		sizeInfo->mcuDispHeight = sizeInfo->mcuRealHeight;
	}
	else
	{
		sizeInfo->startX     = dispInfo->videoWin.startX;
		sizeInfo->startY     = dispInfo->videoWin.startY;
		sizeInfo->dispWidth  = dispInfo->videoWin.width;
		sizeInfo->dispHeight = dispInfo->videoWin.height;

		if ((sizeInfo->startX + sizeInfo->dispWidth) > sizeInfo->realWidth)
		{			      
			LOG_ERROR "startX = %d, dispWidth = %d, realWidth = %d, %s [#%d]\n",
			        sizeInfo->startX, sizeInfo->dispWidth, sizeInfo->realWidth, __FILE__, __LINE__ LOG_END
			        
			result = ERROR_JPG_DEC_DISPLAY_WIN;
			goto end;
		}

		if ((sizeInfo->startY + sizeInfo->dispHeight) > sizeInfo->realHeight)
		{			       
			LOG_ERROR "startY = %d, dispHeight = %d, realHeight = %d, %s [#%d]\n",
			        sizeInfo->startY, sizeInfo->dispHeight, sizeInfo->realHeight, __FILE__, __LINE__ LOG_END
			
			result = ERROR_JPG_DEC_DISPLAY_WIN;
			goto end;
		}

		sizeInfo->mcuDispLeft = sizeInfo->startX / imageComp->widthUnit;
		sizeInfo->mcuDispUp   = sizeInfo->startY / imageComp->heightUnit;

		if (sizeInfo->dispWidth && sizeInfo->dispHeight)
		{
			sizeInfo->mcuDispWidth  = sizeInfo->dispWidth / imageComp->widthUnit;
			if( sizeInfo->dispWidth % imageComp->widthUnit )
				sizeInfo->mcuDispWidth++;

			sizeInfo->mcuDispHeight = sizeInfo->dispHeight / imageComp->heightUnit;
			if( sizeInfo->dispHeight % imageComp->heightUnit )
				sizeInfo->mcuDispHeight++;
		}
		else
		{
			sizeInfo->mcuDispWidth  = sizeInfo->mcuRealWidth;
			sizeInfo->mcuDispHeight = sizeInfo->mcuRealHeight;
			sizeInfo->dispWidth  = imageComp->imageWidth;
			sizeInfo->dispHeight = imageComp->imageHeight;
		}

		sizeInfo->mcuDispRight = sizeInfo->mcuDispLeft + sizeInfo->mcuDispWidth;
		sizeInfo->mcuDispDown  = sizeInfo->mcuDispUp + sizeInfo->mcuDispHeight;

		if (sizeInfo->mcuDispRight > sizeInfo->mcuRealWidth)
		{			        
			LOG_ERROR "mcuDispRight=%d > mcuRealWidth=%d, %s [#%d]\n",
			        sizeInfo->mcuDispRight, sizeInfo->mcuRealWidth, __FILE__, __LINE__ LOG_END			
			        
			result = ERROR_JPG_DEC_DISPLAY_WIN;
			goto end;
		}

		if (sizeInfo->mcuDispDown > sizeInfo->mcuRealHeight)
		{			        
			LOG_ERROR "mcuDispDown=%d > mcuRealHeight=%d \n",
			        sizeInfo->mcuDispDown, sizeInfo->mcuRealHeight, __FILE__, __LINE__ LOG_END
			        
			result = ERROR_JPG_DEC_DISPLAY_WIN;
			goto end;
		}
	}

end:
	return result;
}


//=============================================================================
/**
 * JPEG Engine setup
 *
 * @param ctxt
 * @return void
 */
//=============================================================================
void
JPEG_EngineSetup(void)
{
	JPEG_IMAGE_COMPONENT   *imageComp = &ctxt->imageComponent;
	JPG_HW_CTRL            *jpegCtrl  = &ctxt->hwCtrl;

	if (ctxt == MMP_NULL)
		return;

	JPEG_SetCodecCtrlReg(jpegCtrl->codecCtrl);

	JPEG_SetDriReg(imageComp->restartInterval);

	JPEG_SetTableSpecifyReg(imageComp);

	JPEG_SetImageSizeInfoReg(&ctxt->sizeInfo);

	JPEG_SetLineBufInfoReg(&ctxt->lineBufInfo);

	////if(ctxt->flags & JPEG_FLAGS_DEC_DROP_H_V)
	////    JPEG_DropHv();

	//if ( ctxt->flags & JPEG_FLAGS_OUTPUT_RGB565 )
	//{
	//	JPEG_SetYuv2RgbMatrix(&ctxt->yuv2RgbMatrix);
    //
	//	if( ctxt->flags & JPEG_FLAGS_EN_RGB565_Dither_KEY )
	//		JPEG_SetRgb565DitherKey(&ctxt->ditherKeyInfo);
	//}

	JPEG_SetLineBufSliceUnitReg(ctxt->lineBufInfo.sliceNum, imageComp->imageInfo[0].verticalSamp);

	//if (ctxt->flags & JPEG_FLAGS_MJPG)
	//{
	//	JPEG_SetBitStreamBufInfoReg(&ctxt->motionJpgBsBufInfo[ctxt->motionJpgBsBufIndex]);
	//}
	//else
	//{
	JPEG_SetBitStreamBufInfoReg(&ctxt->bitstreamBufInfo);
	//}

	JPEG_SetSamplingFactorReg(imageComp);

	/**
	 * set Q table
	 */
	JPEG_SetQtableReg(&imageComp->qTable);

	/**
	 * set huffman table
	 */
	JPEG_SetHuffmanCodeCtrlReg(JPEG_HUUFFMAN_Y_DC , jpegCtrl->dcHuffmanTable[0]);
	JPEG_SetHuffmanCodeCtrlReg(JPEG_HUUFFMAN_UV_DC, jpegCtrl->dcHuffmanTable[1]);
	JPEG_SetHuffmanCodeCtrlReg(JPEG_HUUFFMAN_Y_AC , jpegCtrl->acHuffmanTable[0]);
	JPEG_SetHuffmanCodeCtrlReg(JPEG_HUUFFMAN_UV_AC, jpegCtrl->acHuffmanTable[1]);

	JPEG_SetDcHuffmanValueReg(JPEG_HUUFFMAN_Y_DC,
	                          (jpegCtrl->dcHuffmanTable[0] + 16),
	                          jpegCtrl->dcHuffmanwTotalCodeLengthCount[0]);

	JPEG_SetDcHuffmanValueReg(JPEG_HUUFFMAN_UV_DC,
	                          (jpegCtrl->dcHuffmanTable[1] + 16),
	                          jpegCtrl->dcHuffmanwTotalCodeLengthCount[1]);


	if ( (jpegCtrl->codecCtrl & JPEG_MSK_OP_MODE) == JPEG_OP_ENCODE )
	{

		JPEG_SetEncodeAcHuffmanValueReg(JPEG_HUUFFMAN_Y_AC,
		                                (jpegCtrl->acHuffmanTable[0] + 16),
		                                jpegCtrl->acHuffmanwTotalCodeLengthCount[0]);

		JPEG_SetEncodeAcHuffmanValueReg(JPEG_HUUFFMAN_UV_AC,
		                                (jpegCtrl->acHuffmanTable[1] + 16),
		                                jpegCtrl->acHuffmanwTotalCodeLengthCount[1]);

	}
	else
	{
		JPEG_SetDecodeAcHuffmanValueReg(JPEG_HUUFFMAN_Y_AC,
		                                (jpegCtrl->acHuffmanTable[0] + 16),
		                                jpegCtrl->acHuffmanwTotalCodeLengthCount[0]);

		JPEG_SetDecodeAcHuffmanValueReg(JPEG_HUUFFMAN_UV_AC,
		                                (jpegCtrl->acHuffmanTable[1] + 16),
		                                jpegCtrl->acHuffmanwTotalCodeLengthCount[1]);
	}

	JPEG_DropHv((MMP_UINT16)(ctxt->flags >> 16));
	
	JPEG_SetTilingMode();
}

void
JPEG_LogReg(void)
{
	MMP_UINT16 reg, p;
	MMP_UINT i, j, count;

	reg     = 0x0A00;
	count   = (0x0B12 - 0x0A00) / sizeof(MMP_UINT16);

	j = 0;
	p = reg;

	printf("\n\t   0    2    4    6    8    A    C    E\n");
	for (i = 0; i < count; ++i)
	{
		MMP_UINT16 value = 0;

		HOST_ReadRegister(p, &value);
		if (j == 0)
			printf("0x%04X:", p);

		printf(" %04X", value);

		if (j >= 7)
		{
			printf("\r\n");
			j = 0;
		}
		else
			j++;

		p += 2;
	}

	if (j > 0)
		printf("\r\n");

}

