/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  JPEG encoder Control
 *
 * @author 
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "pal/pal.h"
#include "sys/sys.h"
#include "host/host.h"
#include "jpg/config.h"
#include "mmp_jpg.h"
#include "jpg/jpg_marker.h"
#include "jpg/jpg.h"
#include "jpg/jpg_type.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
//#define MIN_JPEG_ENOCDE_SYSRAM_SIZE         (1024)       // 1024 bytes
//#define MASK_128_BYTE                       (0xFFFFFF80) // 128 bytes alignment

//=============================================================================
//                              Structure Definition
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================
extern JPEG_CONTEXT *ctxt;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static JPG_ERROR_CODE
JPEG_SetLineBufferInfo(
    JPEG_LINE_BUF_INFO  *bufInfo,
    JPG_ENCODER         *ptJpgEncoder);

static JPG_ERROR_CODE
JPEG_SetBitStreamBufInfo(
    JPEG_BITSTREAM_BUF_INFO    *bufInfo,
    JPG_ENCODER                *ptJpgEncoder);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * JPEG Encode setup
 *
 * @param ptJpgEncoder [IN]
 * @param quality [IN]
 * @param triggerMode [IN]
 * @return JPG_ERROR_SUCCESS if succeed, error codes otherwise.
 */
//=============================================================================
JPG_ERROR_CODE
JPEG_EncodeSetup(
    JPG_ENCODER            *ptJpgEncoder,
    MMP_UINT16              quality,
    JPEG_TRIGGER_MODE       triggerMode)
{
	JPG_ERROR_CODE           result = JPG_ERROR_SUCCESS;
	JPEG_IMAGE_COMPONENT    *imageComp = &ctxt->imageComponent;
	JPEG_LINE_BUF_INFO      *lineBufInfo = &ctxt->lineBufInfo;	
	JPEG_BITSTREAM_BUF_INFO *streambufInfo = &ctxt->bitstreamBufInfo;
	
	JPEG_SYSRAM_INFO        *sysMemInfo = &ctxt->sysMemInfo;
	JPG_HW_CTRL             *jpegCtrl = &ctxt->hwCtrl;

	// 1. set line buffer information
	result = JPEG_SetLineBufferInfo(lineBufInfo, ptJpgEncoder);
	if (result)
		goto end;

	// 2. allocate bit-stream buffer
	//result = JPEG_AllocateBitStreamBuffer(&ctxt->bitstreamBufInfo, JPEG_BITSTREAM_ENCODE);
	//if (result)
	//	goto end;
		
	result = JPEG_SetBitStreamBufInfo(streambufInfo, ptJpgEncoder);
	if (result)
		goto end;

	sysMemInfo->initial = MMP_FALSE;

	//// 3. set encode image component
	//switch (ptJpgEncoder->format)
	//{
	//	default :
	//	case MMP_ISP_IN_YUV422:
	//		imageComp->encodeFmt = JPEG_YUV_422;
	//		break;
    //
	//	case MMP_ISP_IN_YUV420:
	//		imageComp->encodeFmt = JPEG_YUV_420;
	//		break;
	//}
	
	imageComp->encodeFmt = JPEG_YUV_420;

	imageComp->imageWidth  = ptJpgEncoder->frameWidth;
	imageComp->imageHeight = ptJpgEncoder->frameHeight;
	JPG_SetEncodeImageComp(imageComp);

	imageComp->widthUnit  = imageComp->imageInfo[0].horizonSamp * 8;
	imageComp->heightUnit = imageComp->imageInfo[0].verticalSamp * 8;
	imageComp->realWidth  = (imageComp->imageWidth  + (imageComp->widthUnit - 1)) & ~(imageComp->widthUnit - 1);
	imageComp->realHeight = (imageComp->imageHeight + (imageComp->heightUnit - 1)) & ~(imageComp->heightUnit - 1);
	imageComp->componentNum = 3;
	imageComp->restartInterval = 0;

	JPEG_SetImageSize(JPEG_BITSTREAM_ENCODE, imageComp, &ctxt->sizeInfo);

	jpegCtrl->codecCtrl = (JPEG_OP_ENCODE | triggerMode);
	jpegCtrl->codecCtrl |= (jpegBufferControl[imageComp->componentNum] & JPEG_MSK_LINE_BUF_COMPONENT_VALID);

	// 4. set huffman table
	JPG_Encode_GetHuffmanTable(&jpegCtrl->dcHuffmanTable[0],
	                           &jpegCtrl->acHuffmanTable[0],
	                           &jpegCtrl->dcHuffmanTable[1],
	                           &jpegCtrl->acHuffmanTable[1]);

	jpegCtrl->dcHuffmanwTotalCodeLengthCount[0] = 12;
	jpegCtrl->acHuffmanwTotalCodeLengthCount[0] = 162;
	jpegCtrl->dcHuffmanwTotalCodeLengthCount[1] = 12;
	jpegCtrl->acHuffmanwTotalCodeLengthCount[1] = 162;

	// 5. set Q table
	JpgQualityTable(&imageComp->qTable.Table[0][0], &imageComp->qTable.Table[1][0], quality);
	imageComp->qTable.tableCount = 2;

	// 6. set HW register
	JPEG_EngineSetup();

	// 7. start
	//JPEG_StartReg();

end:
	return result;
}

//=============================================================================
/**
 * JPEG Encode setup command trigger mode
 *
 * @param sourceSelection [IN]
 * @param ispToJpeg [IN/Out]
 * @param rawData [IN/Out]
 * @param quality [IN]
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
JPG_ERROR_CODE
JPEG_EncodeSetupOpen(
    JPG_ENCODER                        *ptJpgEncoder,
    MMP_UINT                            quality)
{
	JPG_ERROR_CODE           result = JPG_ERROR_SUCCESS;
	JPEG_IMAGE_COMPONENT    *imageComp = &ctxt->imageComponent;
	JPEG_SYSRAM_INFO        *sysMemInfo = &ctxt->sysMemInfo;

	JPEG_PowerUp();

	result = JPEG_EncodeSetup(ptJpgEncoder, (MMP_UINT16)quality, JPEG_COMMAND_TRIGGER);
    
    // Initial system memory data structure
	sysMemInfo->start   = sysMemInfo->current = ptJpgEncoder->pHdrBufAddr;
	sysMemInfo->size    = 10 * 1024;
	sysMemInfo->initial = MMP_TRUE;

	// Add JPEG header
	SetMarker(imageComp, &(sysMemInfo->current));
    
  ptJpgEncoder->ParaSetHdrSize = sysMemInfo->current - sysMemInfo->start;
    
  //PalMemcpy(ptJpgEncoder->pStreamBufAdr[0], ptJpgEncoder->pHdrBufAddr, ptJpgEncoder->ParaSetHdrSize);
  //PalMemcpy(ptJpgEncoder->pStreamBufAdr[2], ptJpgEncoder->pHdrBufAddr, ptJpgEncoder->ParaSetHdrSize);
  //PalMemcpy(ptJpgEncoder->pStreamBufAdr[4], ptJpgEncoder->pHdrBufAddr, ptJpgEncoder->ParaSetHdrSize);
             
	return (MMP_RESULT)result;
}

JPG_ERROR_CODE
JPEG_EncodeFire(
    JPG_ENCODER                        *ptJpgEncoder)
{
	JPG_ERROR_CODE           result = JPG_ERROR_SUCCESS;
	JPEG_IMAGE_COMPONENT    *imageComp = &ctxt->imageComponent;
	JPEG_LINE_BUF_INFO      *lineBufInfo = &ctxt->lineBufInfo;	
	JPEG_BITSTREAM_BUF_INFO *streambufInfo = &ctxt->bitstreamBufInfo;
	MMP_UINT16               copySliceSize = 0;	
	            
  result = JPEG_SetLineBufferInfo(lineBufInfo, ptJpgEncoder);
	if (result)
      goto end;
    
	result = JPEG_SetBitStreamBufInfo(streambufInfo, ptJpgEncoder);
	if (result)
      goto end;

	JPEG_SetLineBufInfoReg(&ctxt->lineBufInfo);
	
	copySliceSize = (imageComp->imageHeight + (imageComp->imageInfo[0].verticalSamp * 8 - 1)) / 
	                 (8* imageComp->imageInfo[0].verticalSamp);
	
	JPEG_SetLineBufSliceUnitReg(copySliceSize, imageComp->imageInfo[0].verticalSamp);
	JPEG_SetBitStreamBufInfoReg(&ctxt->bitstreamBufInfo);
  
             
  JPEG_StartReg();
	
	// set line buffer r/w data size
	JPEG_SetLineBufSliceWriteNumReg((MMP_UINT16)copySliceSize);
	// set line buffer write end
	JPEG_SetLineBufCtrlReg((MMP_UINT16)JPEG_MSK_LINE_BUF_WRITE_END);

  JPEG_SetLineBufCtrlReg((MMP_UINT16)JPEG_MSK_LAST_ENCODE_DATA);

  //JPEG_LogReg();
end :
	return (MMP_RESULT)result;
}

JPG_ERROR_CODE
JPEG_EncodeGetStream( MMP_UINT32*   encodeSize,
                      MMP_BOOL*     encodeEnd,
                      JPG_ENCODER  *ptJpgEncoder)
{
	JPG_ERROR_CODE          result = JPG_ERROR_SUCCESS;
	JPEG_SYSRAM_INFO       *sysMemInfo = &ctxt->sysMemInfo;
	MMP_UINT32              hwStatus = 0;
	MMP_UINT8              *tmpBuf = ptJpgEncoder->pStreamBufAdr[ptJpgEncoder->streamBufSelect];
	MMP_UINT32              validBitstreamSize = 0;
	MMP_UINT32              encodedSize;
	MMP_UINT32              i;
	MMP_UINT32 value;

	hwStatus = JPEG_GetEngineStatus1Reg();

  if (hwStatus & JPEG_STATUS_ENCODE_COMPLETE)
  {
  	  *encodeEnd = MMP_TRUE;  
  } else {  	    	 
  	  *encodeEnd = MMP_FALSE;
      *encodeSize = 0;
      
  	  if (hwStatus & JPEG_STATUS_BITSTREAM_BUF_FULL)
  	  {  	  	
  	  	  *encodeSize = JPEG_GetBitStreamValidSizeReg();
  	  	  JPEG_ResetEngine();
  	  }
  	  
  	  goto end;
  }                                    
   
  validBitstreamSize =  JPEG_GetBitStreamValidSizeReg();
		
  // remove redundant bytes (0xFF) and add end of image marker
  // xx xx xx xx   xx xx xx FF   xx xx FF D9       
  //tmpBuf = tmpBuf + ptJpgEncoder->ParaSetHdrSize + validBitstreamSize - 1;
  tmpBuf = tmpBuf + validBitstreamSize - 1;
       
	// find redundant byte (0xff) and remove it
	for (i = 0; i < 10; i++, tmpBuf--)
	{
      if ((*tmpBuf) != 0xff)
      {
          break;
      }
	}
	
	*(++tmpBuf) = 0xFF;
	*(++tmpBuf) = 0xD9;	
	
	// calculate valid temp length
	encodedSize = tmpBuf - ptJpgEncoder->pStreamBufAdr[ptJpgEncoder->streamBufSelect] + 1;
    
  *encodeSize = encodedSize;
 
    // set bitstream buffer read size
	JPEG_SetBitstreamBufRwSizeReg(validBitstreamSize);		
	
	// set bitstream buffer read end
	JPEG_SetBitstreamBufCtrlReg(JPEG_MSK_BITSTREAM_BUF_RW_END);

	JPEG_SetBitstreamBufCtrlReg(JPEG_MSK_LAST_BITSTREAM_DATA);	
  
  validBitstreamSize =  JPEG_GetBitStreamValidSizeReg();
	  
  //PalMemcpy(ptJpgEncoder->pStreamBufAdr[ptJpgEncoder->streamBufSelect], ptJpgEncoder->pHdrBufAddr, ptJpgEncoder->ParaSetHdrSize); 
  //JPEG_ResetEngine();
end :
	return (MMP_RESULT)result;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static JPG_ERROR_CODE
JPEG_SetLineBufferInfo(
    JPEG_LINE_BUF_INFO      *bufInfo,
    JPG_ENCODER             *ptJpgEncoder)
{
	JPG_ERROR_CODE result = JPG_ERROR_SUCCESS;
	JPEG_IMAGE_COMPONENT    *imageComp = &ctxt->imageComponent;

	bufInfo->component1Addr = (MMP_UINT8 *)ptJpgEncoder->pSourceBufAdrY[ptJpgEncoder->sourceBufSelect];
	bufInfo->component2Addr = (MMP_UINT8 *)ptJpgEncoder->pSourceBufAdrU[ptJpgEncoder->sourceBufSelect];
	bufInfo->component3Addr = (MMP_UINT8 *)ptJpgEncoder->pSourceBufAdrV[ptJpgEncoder->sourceBufSelect];
	bufInfo->component1Pitch = ptJpgEncoder->framePitchY;
	bufInfo->component23Pitch = ptJpgEncoder->framePitchY;
	bufInfo->sliceNum = (ptJpgEncoder->frameHeight + 15) >> 4;

	return result;
}

static JPG_ERROR_CODE
JPEG_SetBitStreamBufInfo(
    JPEG_BITSTREAM_BUF_INFO    *bufInfo,
    JPG_ENCODER                *ptJpgEncoder)
{
	JPG_ERROR_CODE result = JPG_ERROR_SUCCESS;

  bufInfo->size = 2 * 1024 * 1024;
		
	bufInfo->addrAlloc = bufInfo->addr = ptJpgEncoder->pStreamBufAdr[ptJpgEncoder->streamBufSelect];
	                                     //+ ptJpgEncoder->ParaSetHdrSize;	
				
	return result;
}
