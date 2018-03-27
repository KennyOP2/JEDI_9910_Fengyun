///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <debug.h>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: HDMIRX_SAMPLE_3.11
//******************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_

/*************************************/
/*Port Using Define                  */
/*************************************/
//#define   DEBUG_EDID
//#define DEBUG_TX_HDCP
//#define   DEBUG_TX_MISC
//#define   DEBUG_TX_VIDEO
//#define   DEBUG_TX_AUDIO
//#define   _PRINT_HDMI_TX_

//#define   DEBUG_RX_AUDIO
//#define   DEBUG_RX_VIDEO
//#define   DEBUG_RX_REG
//#define   DEBUG_RX_FUNCTION
//#define   DEBUG_RX_MISC
//#define   DEBUG_RX_HDCP
//#define   _HDMITX_DEBUG_PRINT_
//#define   DEBUG_RX_INT

//#define   _EDID_DEBUG_PRINT_
//
//#define _HDMIRX_DEBUG_PRINT_
//#define Debug_AVMute

/////////////////////////////////////////////////////////////////////////////
// Debug sub items
/////////////////////////////////////////////////////////////////////////////


#ifdef _PRINT_HDMI_TX_HDCP_
    #define _TXHDCP_DEBUG_PRINT_
#endif

#ifdef _PRINT_HDMI_TX_

    #define _HDMITX_DEBUG_PRINT1_
    #define _HDMITX_DEBUG_PRINT2_
#endif

#ifdef _PRINT_HDMI_TX_HDCP_
    #define _TXHDCP_DEBUG_PRINT_
#endif

#ifdef _PRINT_HDMI_RX_
    #define _DUMPRX_DEBUG_PRINT_

    #define _HDMIRX_DEBUG_PRINT1_
    #define _HDMIRX_DEBUG_PRINT2_
#endif

#ifdef _PRINT_HDMI_RX_HDCP_
    #define _RXHDCP_DEBUG_PRINT_
#endif

#ifdef _PRINT_EDID_

    #define _EDID_DEBUG_PRINT1_
    #define _EDID_DEBUG_PRINT2_
#endif

#ifdef  DEBUG_EDID
#define EDID_PRINTF(x) printf x
#else
#define EDID_PRINTF(x)
#endif

#ifdef  DEBUG_TX_HDCP
#define HDMITX_HDCP_PRINTF(x) printf x
#else
#define HDMITX_HDCP_PRINTF(x)
#endif

#ifdef  DEBUG_TX_MISC
#define HDMITX_MISC_PRINTF(x) printf x
#else
#define HDMITX_MISC_PRINTF(x)
#endif

#ifdef  DEBUG_TX_VIDEO
#define HDMITX_VIDEO_PRINTF(x) printf x
#else
#define HDMITX_VIDEO_PRINTF(x)
#endif

#ifdef  DEBUG_TX_AUDIO
#define HDMITX_AUDIO_PRINTF(x) printf x
#else
#define HDMITX_AUDIO_PRINTF(x)
#endif

#ifdef  DEBUG_RX_AUDIO
#define HDMIRX_AUDIO_PRINTF(x) printf x
#else
#define HDMIRX_AUDIO_PRINTF(x)
#endif

#ifdef  DEBUG_RX_VIDEO
#define HDMIRX_VIDEO_PRINTF(x) printf x
#else
#define HDMIRX_VIDEO_PRINTF(x)
#endif

#ifdef  DEBUG_RX_REG
#define HDMIRX_REG_PRINTF(x) printf x
#else
#define HDMIRX_REG_PRINTF(x)
#endif

#ifdef  DEBUG_RX_FUNCTION
#define HDMIRX_FUNCTION_PRINTF(x) printf x
#else
#define HDMIRX_FUNCTION_PRINTF(x)
#endif

#ifdef  DEBUG_RX_MISC
#define HDMIRX_MISC_PRINTF(x) printf x
#else
#define HDMIRX_MISC_PRINTF(x)
#endif

#ifdef  DEBUG_RX_HDCP
#define HDMIRX_HDCP_PRINTF(x) printf x
#else
#define HDMIRX_HDCP_PRINTF(x)
#endif

#ifdef  DEBUG_RX_INT
#define HDMIRX_INT_PRINTF(x) printf x
#else
#define HDMIRX_INT_PRINTF(x)
#endif

#ifdef _DUMPRX_DEBUG_PRINT_
    #define DUMPRX_DEBUG_PRINTF(x) printf x

#else
    #define DUMPRX_DEBUG_PRINTF(x)
#endif

#ifdef _DUMPTX_DEBUG_PRINT_
    #define DUMPTX_DEBUG_PRINTF(x) printf x

#else
    #define DUMPTX_DEBUG_PRINTF(x)
#endif

#ifdef _DEBUG_MSG_PRINT_
    #define DEBUG_MSG_PRINTF(x) printf x
#else
    #define DEBUG_MSG_PRINTF(x)
#endif

#ifdef _HDMIRX_DEBUG_PRINT_
    #define HDMIRX_DEBUG_PRINTF(x) printf x
#else
    #define HDMIRX_DEBUG_PRINTF(x)
#endif

#ifdef _HDMIRX_DEBUG_PRINT1_
    #define HDMIRX_DEBUG_PRINTF1(x) printf x
#else
    #define HDMIRX_DEBUG_PRINTF1(x)
#endif

#ifdef _HDMIRX_DEBUG_PRINT2_
    #define HDMIRX_DEBUG_PRINTF2(x) printf x
#else
    #define HDMIRX_DEBUG_PRINTF2(x)
#endif

#ifdef _RXHDCP_DEBUG_PRINT_
    #define RXHDCP_DEBUG_PRINTF(x) printf x
#else
    #define RXHDCP_DEBUG_PRINTF(x)
#endif

#ifdef _DEBUG_MSG_PRINT_
    #define DEBUG_MSG_PRINTF(x) printf x
#else
    #define DEBUG_MSG_PRINTF(x)
#endif

#ifdef _HDMITX_DEBUG_PRINT_
    #define HDMITX_DEBUG_PRINTF(x) printf x

#else
    #define HDMITX_DEBUG_PRINTF(x)
#endif

#ifdef _HDMITX_DEBUG_PRINT1_
    #define HDMITX_DEBUG_PRINTF1(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF1(x)
#endif

#ifdef _HDMITX_DEBUG_PRINT2_
    #define HDMITX_DEBUG_PRINTF2(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF2(x)
#endif

#ifdef _TXHDCP_DEBUG_PRINT_
    #define TXHDCP_DEBUG_PRINTF(x) printf x
#else
    #define TXHDCP_DEBUG_PRINTF(x)
#endif

#ifdef _EDID_DEBUG_PRINT_
    #define EDID_DEBUG_PRINTF(x) printf x
#else
    #define EDID_DEBUG_PRINTF(x)
#endif

#ifdef _EDID_DEBUG_PRINT1_
    #define EDID_DEBUG_PRINTF1(x) printf x
#else
    #define EDID_DEBUG_PRINTF1(x)
#endif

#ifdef _EDID_DEBUG_PRINT2_
    #define EDID_DEBUG_PRINTF2(x) printf x
#else
    #define EDID_DEBUG_PRINTF2(x)
#endif

extern  void ShowClock(void);
extern  void ClrClock(void);

#endif
