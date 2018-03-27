///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mcu.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/08/12
//   @fileversion: CAT9883_V43
//******************************************/

MMP_UINT8 CAT9883_ReadI2C_Byte(MMP_UINT8 RegAddr);
MMP_RESULT CAT9883_WriteI2C_Byte(MMP_UINT8 RegAddr, MMP_UINT8 d);
MMP_RESULT CAT9883_WriteI2C_ByteN(MMP_UINT8 RegAddr, MMP_UINT8 *pData, int N);
MMP_RESULT CAT9883_WriteI2c_ByteMask(MMP_UINT8 RegAddr, MMP_UINT8 data, MMP_UINT8 mask);


