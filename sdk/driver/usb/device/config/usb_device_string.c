/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *
 *	String
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define DT_STRING 3

#define STRING_00_LENGTH			    0x04
#define STRING_10_LENGTH			    0x1a
#define STRING_20_LENGTH			    0x22
#define STRING_30_LENGTH			    0x2e
#define STRING_40_LENGTH			    0x26
#define STRING_50_LENGTH			    0x10
#define STRING_60_LENGTH			    0x1e
#define STRING_70_LENGTH			    0x10
#define STRING_80_LENGTH			    0x0e
#define STRING_90_LENGTH			    0x00
#define STRING_03_SERIAL_NUM_LENGTH	    0x20



//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Private Structure Definition
//=============================================================================
MMP_UINT8 u8OTGStringSerialNum[STRING_03_SERIAL_NUM_LENGTH] =
{
    0x1A, 0x03, 0x31, 0x00,  
    0x34, 0x00, 0x30, 0x00,  
    0x32, 0x00, 0x34, 0x00,  
    0x36, 0x00, 0x38, 0x00,
    0x39, 0x00, 0x46, 0x00,
    0x41, 0x00, 0x30, 0x00,
    0x38, 0x00
};

MMP_UINT8  u8OTGString00Descriptor[STRING_00_LENGTH] =
{
   STRING_00_LENGTH,			// Size of this descriptor
   // STRING Descriptor type
   // Language ID, 0409: English, 0404: Chinese Taiwan
   0X03,     //bDescriptorType
   0X09,     //bLang
   0X04,
};

MMP_UINT8  u8OTGString10Descriptor[STRING_10_LENGTH] =
{
   STRING_10_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X53, 0,  //S
   0X6d, 0,  //m
   0X65, 0,  //e
   0X64, 0,  //d
   0X69, 0,  //i
   0X61, 0,  //a
   0X20, 0,  //
   0X20, 0,  // 
   0X69, 0,  //i
   0X6E, 0,  //n
   0X63, 0,  //c
   0X2E, 0   //.
};

MMP_UINT8  u8OTGString20Descriptor[STRING_20_LENGTH] =
{
   STRING_20_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X44, 0,  //D
   0X69, 0,  //i
   0X67, 0,  //g
   0X69, 0,  //i
   0X2D, 0,  //-
   0X50, 0,  //P
   0X68, 0,  //h
   0X6F, 0,  //o
   0X74, 0,  //t
   0X6F, 0,  //o
   0X2D, 0,  //-
   0X46, 0,  //F
   0X72, 0,  //r
   0X61, 0,  //a
   0X6D, 0,  //m
   0X65, 0   //e
};

MMP_UINT8  u8OTGString30Descriptor[STRING_30_LENGTH] =
{
   STRING_30_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X45, 0,  //E
   0X56, 0,  //V
   0X2D, 0,  //-
   0X62, 0,  //b
   0X6F, 0,  //o
   0X61, 0,  //a
   0X72, 0,  //r
   0X64, 0,  //d
   0X20, 0,  // 
   0X43, 0,  //C
   0X6F, 0,  //o
   0X6E, 0,  //n
   0X66, 0,  //f
   0X69, 0,  //i
   0X67, 0,  //g
   0X75, 0,  //u
   0X72, 0,  //r
   0X61, 0,  //a
   0X74, 0,  //t
   0X69, 0,  //i
   0X6F, 0,  //o
   0X6E, 0   //n
};

MMP_UINT8  u8OTGString40Descriptor[STRING_40_LENGTH] =
{
   STRING_40_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X45, 0,  //E
   0X56, 0,  //V
   0X2D, 0,  //-
   0X62, 0,  //b
   0X6F, 0,  //o
   0X61, 0,  //a
   0X72, 0,  //r
   0X64, 0,  //d
   0X20, 0,  // 
   0X49, 0,  //I
   0X6E, 0,  //n
   0X74, 0,  //t
   0X65, 0,  //e
   0X72, 0,  //r
   0X66, 0,  //f
   0X61, 0,  //a
   0X63, 0,  //c
   0X65, 0   //e
};

MMP_UINT8  u8OTGString50Descriptor[STRING_50_LENGTH] =
{
   STRING_50_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X50, 0,  //P
   0X72, 0,  //r
   0X69, 0,  //i
   0X6E, 0,  //n
   0X74, 0,  //t
   0X65, 0,  //e
   0X72, 0   //r
};

MMP_UINT8  u8OTGString60Descriptor[STRING_60_LENGTH] =
{
   STRING_60_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X44, 0,  //D
   0X69, 0,  //i
   0X67, 0,  //g
   0X69, 0,  //i
   0X74, 0,  //t
   0X61, 0,  //a
   0X6C, 0,  //l
   0X5F, 0,  //_
   0X43, 0,  //C
   0X61, 0,  //a
   0X6D, 0,  //m
   0X65, 0,  //e
   0X72, 0,  //r
   0X61, 0   //a
};

MMP_UINT8  u8OTGString70Descriptor[STRING_70_LENGTH] =
{
   STRING_70_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X53, 0,  //S
   0X74, 0,  //t
   0X6F, 0,  //o
   0X72, 0,  //r
   0X61, 0,  //a
   0X67, 0,  //g
   0X65, 0   //e
};

MMP_UINT8  u8OTGString80Descriptor[STRING_80_LENGTH] =
{
   STRING_80_LENGTH,			// Size of this descriptor
   DT_STRING,					// STRING Descriptor type
   0X4D, 0,  //M
   0X65, 0,  //e
   0X6D, 0,  //m
   0X6F, 0,  //o
   0X72, 0,  //r
   0X79, 0   //y
};



MMP_UINT8 u8OTGString00DescriptorEX[STRING_00_LENGTH];
MMP_UINT8 u8OTGString10DescriptorEX[STRING_10_LENGTH];
MMP_UINT8 u8OTGString20DescriptorEX[STRING_20_LENGTH];
MMP_UINT8 u8OTGString30DescriptorEX[STRING_30_LENGTH];
MMP_UINT8 u8OTGString40DescriptorEX[STRING_40_LENGTH];
MMP_UINT8 u8OTGString50DescriptorEX[STRING_50_LENGTH];
MMP_UINT8 u8OTGString60DescriptorEX[STRING_60_LENGTH];
MMP_UINT8 u8OTGString70DescriptorEX[STRING_70_LENGTH];
MMP_UINT8 u8OTGString80DescriptorEX[STRING_80_LENGTH];



