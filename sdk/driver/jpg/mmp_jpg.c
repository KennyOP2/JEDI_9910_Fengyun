/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  mmp_jpg.c JPEG module Control
 *  Date: 
 *
 * @author 
 * @version 0.01
 */

//=============================================================================
//                              Include Files
//=============================================================================

#include "pal/pal.h"
#include "host/host.h"
#include "mmp_jpg.h"

#include "jpg/config.h"
#include "jpg/jpg.h"
#include "jpg/jpg_hw.h"
#include "jpg/jpg_type.h"

//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              Macro Definition
//=============================================================================


//=============================================================================
//                              Private Structure Definition
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================
JPEG_CONTEXT *ctxt = MMP_NULL;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_RESULT
JPEG_ContextInitialize(void)
{
	JPG_ERROR_CODE  result = JPG_ERROR_SUCCESS;

	if( ctxt == MMP_NULL )
	{
		ctxt = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(JPEG_CONTEXT));
		if( !ctxt )
		{
			result = ERROR_JPG_Allocate_FAIL;		
			LOG_ERROR " ctxt Allocat fail\n" LOG_END
			goto end;
		}
		PalMemset((void *)ctxt, 0, sizeof(JPEG_CONTEXT));	
	}
	else
	{
		PalMemset((void *)ctxt, 0, sizeof(JPEG_CONTEXT));
	}
		
end:
	if (result)
		LOG_ERROR " %s(err = 0x%08X) \n", __FUNCTION__, result LOG_END

	return result;
}

static void
JPEG_ContextTerminate(void)
{
	if( ctxt )
	{
		PalHeapFree(PAL_HEAP_DEFAULT, (void *)ctxt);
		ctxt = MMP_NULL;
	}		
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * JPEG Encode setup command trigger mode
 *
 * @param ptJpgEncoder [IN/Out]
 * @param quality [IN]
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================


MMP_RESULT
mmpJPGEncodeOpen(    
    JPG_ENCODER                        *ptJpgEncoder,
    MMP_UINT                            quality)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;

	if ((ptJpgEncoder->frameWidth == 0) ||
	    (ptJpgEncoder->frameHeight == 0))
	{
		result = ERROR_JPG_ENC_Command_Trigger_INFO;
		goto end;
	}

	result = JPEG_ContextInitialize();
	if( result )
	{		
		LOG_ERROR "JPEG_ContextInitialize() fail\n" LOG_END
		goto end;
	}

	result = JPEG_EncodeSetupOpen(ptJpgEncoder, quality);
	
	JPEG_EnableInterrupt();
		
end:
	if (result)
      LOG_ERROR "%s(err = 0x%08X) \n", __FUNCTION__, result LOG_END

	return result;
}

MMP_RESULT
mmpJPGEncodeFire(    
    JPG_ENCODER                        *ptJpgEncoder)    
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
		 
  ptJpgEncoder->streamBufSelect = (ptJpgEncoder->streamBufSelect + 1) % (ptJpgEncoder->streamBufCount);
       
	result = JPEG_EncodeFire(ptJpgEncoder);
		
end:
	if (result)
      LOG_ERROR "%s(err = 0x%08X) \n", __FUNCTION__, result LOG_END

	return result;
}

MMP_RESULT
mmpJPGEncodeGetStream(    
    MMP_UINT32*   StreamLen,
    MMP_BOOL*     bFrameEnd,
    JPG_ENCODER  *ptJpgEncoder)    
{
	MMP_RESULT  result = MMP_RESULT_SUCCESS;  
  MMP_UINT32  encodeSize;
  MMP_BOOL    encodeEnd;

	result = JPEG_EncodeGetStream(&encodeSize, &encodeEnd, ptJpgEncoder);	
 
  *StreamLen = encodeSize;
  *bFrameEnd = encodeEnd;
  
end:
	if (result)
      LOG_ERROR "%s(err = 0x%08X) \n", __FUNCTION__, result LOG_END

	return result;
}

MMP_BOOL
mmpJPGEncodeQueryIdle(    
    void)    
{
  MMP_UINT32  hwStatus;
  
  hwStatus = JPEG_GetEngineStatus1Reg();
	
  if (hwStatus & JPEG_STATUS_ENCODE_COMPLETE)
      return MMP_TRUE;
  else
      return MMP_FALSE;
}

MMP_RESULT
mmpJPGEncodeEnableInterrupt(
    ithIntrHandler  handler)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    return;
    
    /** register interrupt handler to interrupt mgr */
    //ithIntrRegisterHandlerIrq(ITH_INTR_JPEG, handler, MMP_NULL);
    //ithIntrSetTriggerModeIrq(ITH_INTR_JPEG, ITH_INTR_EDGE);
    //ithIntrSetTriggerLevelIrq(ITH_INTR_JPEG, ITH_INTR_HIGH_RISING);
    //ithIntrEnableIrq(ITH_INTR_JPEG);
    /** enable encoder interrupt */
    
    JPEG_EnableInterrupt();
   
    return result;
}

MMP_RESULT
mmpJPGEncodeDisableInterrupt(
    void)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    return;
    
    //ithIntrDisableIrq(ITH_INTR_JPEG);
    
    JPEG_DisableInterrupt();

    return result;
}

void
mmpJPGEncodeClearInterrupt(
    void)
{       
    return;
    
    JPEG_ClearInterrupt();     
}
