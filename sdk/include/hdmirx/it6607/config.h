///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: HDMIRX_SAMPLE_3.11
//******************************************/

#ifndef _CONFIG_H_
#define _CONFIG_H_


#define SUPPORT_INPUTYUV
#define SUPPORT_INPUTRGB

#define SUPPORT_OUTPUTRGB
#define SUPPORT_OUTPUTYUV444
#define SUPPORT_OUTPUTYUV422

#if (defined(SUPPORT_OUTPUTYUV444))||(defined(SUPPORT_OUTPUTYUV422))
#define SUPPORT_OUTPUTYUV
#endif


#define CONFIG_VIDEO_DRIVING_STRENGTH  2 // 0 : weakest -> 1 -> 2 -> 3: strongest
#define CONFIG_AUDIO_DRIVING_STRENGTH  1 // 0 : weakest -> 1 -> 2 -> 3: strongest


#define _SupportComposeEDID_
//#define _AllionComplianceTest_
#define _IgnoreHPDOff_

#define _EDID_CTRL_
//#define   _SupportComposeEDID_
//#define   _3D_Packet_

//#define Enable_HDMI_Tx

#define _IT6607_GeNPacket_Usage_
#define _IT6607_ACPPacket_Usage_
//#define Enable_SPD_VSDB_PACKET
// #define AUTO_SEARCH_EQ_ALGORITHM
// #define DISABLE_PIXEL_REPEAT_REMOVE

#endif
