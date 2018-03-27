/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as JPEG error code header file.
 *  Date: 
 *
 * @author 
 * @version 0.1
 */

#ifndef _JPG_ERROR_H_
#define _JPG_ERROR_H_


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

	/**
	 *  JPEG error code
	 */

	typedef MMP_UINT32 JPG_ERROR_CODE;

#define JPG_ERROR_SUCCESS                               (MMP_RESULT_SUCCESS)

#define ERROR_JPG_BASE                                  (MMP_MODULE_JPEG << MMP_ERROR_OFFSET)

#define ERROR_JPG_Get_LineBuffer_Info                   (ERROR_JPG_BASE + 0x0001)
#define ERROR_JPG_Get_BitStreamBufferr_Info             (ERROR_JPG_BASE + 0x0002)
#define ERROR_JPG_Allocate_BitStreamBuffer              (ERROR_JPG_BASE + 0x0003)
#define ERROR_JPG_Allocate_LineBuffer_ColorFmt          (ERROR_JPG_BASE + 0x0004)
#define ERROR_JPG_Allocate_LineBuffer                   (ERROR_JPG_BASE + 0x0005)
#define ERROR_JPG_MarkerParser_Unsupported_Marker       (ERROR_JPG_BASE + 0x0006)
#define ERROR_JPG_MarkerParser                          (ERROR_JPG_BASE + 0x0007)
#define ERROR_JPG_DataParser                            (ERROR_JPG_BASE + 0x0008)
#define ERROR_JPG_SetDecodeColorFormat                  (ERROR_JPG_BASE + 0x0009)
#define ERROR_JPG_Decode_Setup_ColorFmt                 (ERROR_JPG_BASE + 0x000A)
#define ERROR_JPG_Get_LineBuf_Info_NULL_Pointer         (ERROR_JPG_BASE + 0x000B)
#define ERROR_JPG_Get_LineBuf_Info_ColorFmt             (ERROR_JPG_BASE + 0x000C)
#define ERROR_JPG_IRQ_DecodeError                       (ERROR_JPG_BASE + 0x000D)
#define ERROR_JPG_Encode_Process_NULL_Pointer           (ERROR_JPG_BASE + 0x000E)
#define ERROR_JPG_Encode_SYSRAM_SIZE                    (ERROR_JPG_BASE + 0x000F)
#define ERROR_JPG_Decode_Setup_NULL_Pointer             (ERROR_JPG_BASE + 0x0010)
#define ERROR_JPG_Decode_BitStreamBuf_TimeOut           (ERROR_JPG_BASE + 0x0011)
#define ERROR_JPG_Decode_Busy_TimeOut                   (ERROR_JPG_BASE + 0x0012)
#define ERROR_JPG_Encode_TimeOut                        (ERROR_JPG_BASE + 0x0013)
#define ERROR_JPG_Alignment_BitStreamBuffer_Size        (ERROR_JPG_BASE + 0x0014)
#define ERROR_JPG_MarkerParser_Non_Interleaved          (ERROR_JPG_BASE + 0x0015)
#define ERROR_JPG_MarkerParser_Not_Default_Huffman      (ERROR_JPG_BASE + 0x0016)
#define ERROR_JPG_Create_Interrupt_Event                (ERROR_JPG_BASE + 0x0017)
#define ERROR_JPG_ISP_Share_Data_Setting                (ERROR_JPG_BASE + 0x0018)
#define ERROR_JPG_Command_Trigger_SysRAMSize            (ERROR_JPG_BASE + 0x0020)
#define ERROR_JPG_Invalid_Mode_Change                   (ERROR_JPG_BASE + 0x0021)
#define ERROR_JPG_DEC_DISPLAY_WIN                       (ERROR_JPG_BASE + 0x0022)
#define ERROR_JPG_ENC_Command_Trigger_INFO              (ERROR_JPG_BASE + 0x0023)
#define ERROR_JPG_BitStream_Size_Exceed                 (ERROR_JPG_BASE + 0x0024)
#define ERROR_JPG_Invalid_Parameter                     (ERROR_JPG_BASE + 0x0025)
#define ERROR_JPG_Allocate_FAIL                         (ERROR_JPG_BASE + 0x0026)

#define ERROR_JPG_MarkerParser_Progressive              (ERROR_JPG_BASE + 0xAAAA)


#define ERROR_MP1_Bitstreams                            (ERROR_JPG_BASE + 0xff00)
#define ERROR_MP1_DECODE                                (ERROR_JPG_BASE + 0xff01)

#define ERROR_JPROG_DECODE                              (ERROR_JPG_BASE + 0xff03)
#ifdef __cplusplus
}
#endif

#endif
