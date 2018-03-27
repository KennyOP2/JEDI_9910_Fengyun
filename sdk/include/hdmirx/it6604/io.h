///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO.h>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: HDMIRX_SAMPLE_3.11
//******************************************/

#include "hdmirx/it6604/typedef.h"

#ifndef _IO_h_
#define _IO_h_

BYTE HDMIRX_ReadI2C_Byte (BYTE RegAddr);
SYS_STATUS HDMIRX_WriteI2C_Byte (BYTE RegAddr, BYTE d);
SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE * pData,int N);
SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE * pData,int N);
SYS_STATUS HDMIRX_SetI2C_Byte(SHORT RegAddr, BYTE Mask, BYTE Value );

#endif
