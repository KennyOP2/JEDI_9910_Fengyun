///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iT6607.c>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: HDMIRX_SAMPLE_3.11
//******************************************/

#include "hdmirx/it6607/iT6607.h"
#include "hdmirx/it6607/debug.h"
#include "hdmirx/it6607/io.h"
#include "hdmirx/it6607/timerprocess.h"
#include "mmp_iic.h"

#define AUTO_CORRECT_AUDIO_FS

#define HDMIRX_OUTPUT_VID_MODE        (F_MODE_EN_UDFILT)
#define HDMIRX_OUTPUT_VID_SIG         0
#define HDMIRX_OUTPUT_VID_SIG2        0 // B_HALF_PCLK
#define HDMIRX_OUTPUT_LOW_BIT_GATTING 0 // 0xC0 // 0x80
//#define   ColorDepth_Reset
#define  Auto_Reset
unsigned char       IT6607_INT_FLAG      = 1;
static unsigned int gtDisableHDCP        = 0;
static unsigned int gtIsHDCPKeyEmpty     = 0;
static unsigned int gtAudioErrorType1Cnt = 0;

HDMIRXDEV           it6607dev;
#define VideoTimeOutCheck(x)       TimeOutCheck(it6607dev.m_VideoCountingTimer, (x))
#define AudioTimeOutCheck(x)       TimeOutCheck(it6607dev.m_AudioCountingTimer, (x))
#define AssignMuteVirtualTime()    {it6607dev.m_MuteResumingTimer = GetCurrentVirtualTime(); }
#define AssignSWResetVirtualTime() {it6607dev.m_SWResetTimeOut = GetCurrentVirtualTime(); }
#define AssignVideoVirtualTime()   {it6607dev.m_VideoCountingTimer = GetCurrentVirtualTime(); }
#define AssignAudioVirtualTime()   {it6607dev.m_AudioCountingTimer = GetCurrentVirtualTime(); }

#ifdef USE_MODE_TABLE
static HDMI_VTiming _CODE s_VMTable[] = {
    { 1, 0, 640, 480, 800, 525, 25175000L, 0x89, 16, 96, 48, 10, 2, 33, PROG, Vneg, Hneg },                      //640x480@60Hz
    { 2, 0, 720, 480, 858, 525, 27000000L, 0x80, 16, 62, 60, 9, 6, 30, PROG, Vneg, Hneg },                       //720x480@60Hz
    { 3, 0, 720, 480, 858, 525, 27000000L, 0x80, 16, 62, 60, 9, 6, 30, PROG, Vneg, Hneg },                       //720x480@60Hz
    { 4, 0, 1280, 720, 1650, 750, 74250000L, 0x2E, 110, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },                   //1280x720@60Hz
    { 5, 0, 1920, 540, 2200, 562, 74250000L, 0x2E, 88, 44, 148, 2, 5, 15, INTERLACE, Vpos, Hpos },               //1920x1080(I)@60Hz
    { 6, 1, 720, 240, 858, 262, 13500000L, 0x100, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },                 //720x480(I)@60Hz
    { 7, 1, 720, 240, 858, 262, 13500000L, 0x100, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },                 //720x480(I)@60Hz
    { 8, 1, 720, 240, 858, 262, 13500000L, 0x100, 19, 62, 57, 4, 3, 15, PROG, Vneg, Hneg },                      //720x480(I)@60Hz
    { 9, 1, 720, 240, 858, 262, 13500000L, 0x100, 19, 62, 57, 4, 3, 15, PROG, Vneg, Hneg },                      //720x480(I)@60Hz
    { 10, 2, 720, 240, 858, 262, 54000000L, 0x40, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },                 //720x480(I)@60Hz
    { 11, 2, 720, 240, 858, 262, 54000000L, 0x40, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },                 //720x480(I)@60Hz
    { 12, 2, 720, 240, 858, 262, 54000000L, 0x40, 19, 62, 57, 4, 3, 15, PROG, Vneg, Hneg },                      //720x480(I)@60Hz
    { 13, 2, 720, 240, 858, 262, 54000000L, 0x40, 19, 62, 57, 4, 3, 15, PROG, Vneg, Hneg },                      //720x480(I)@60Hz
    { 14, 1, 1440, 480, 1716, 525, 54000000L, 0x40, 32, 124, 120, 9, 6, 30, PROG, Vneg, Hneg },                  //1440x480@60Hz
    { 15, 1, 1440, 480, 1716, 525, 54000000L, 0x40, 32, 124, 120, 9, 6, 30, PROG, Vneg, Hneg },                  //1440x480@60Hz
    { 16, 0, 1920, 1080, 2200, 1125, 148500000L, 0x17, 88, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },                //1920x1080@60Hz
    { 17, 0, 720, 576, 864, 625, 27000000L, 0x80, 12, 64, 68, 5, 5, 39, PROG, Vneg, Hneg },                      //720x576@50Hz
    { 18, 0, 720, 576, 864, 625, 27000000L, 0x80, 12, 64, 68, 5, 5, 39, PROG, Vneg, Hneg },                      //720x576@50Hz
    { 19, 0, 1280, 720, 1980, 750, 74250000L, 0x2E, 440, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },                  //1280x720@50Hz
    { 20, 0, 1920, 540, 2640, 562, 74250000L, 0x2E, 528, 44, 148, 2, 5, 15, INTERLACE, Vpos, Hpos },             //1920x1080(I)@50Hz
    { 21, 1, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },                //1440x576(I)@50Hz
    { 22, 1, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },                //1440x576(I)@50Hz
    { 23, 1, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, PROG, Vneg, Hneg },                     //1440x288@50Hz
    { 24, 1, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, PROG, Vneg, Hneg },                     //1440x288@50Hz
    { 25, 2, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },                //1440x576(I)@50Hz
    { 26, 2, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },                //1440x576(I)@50Hz
    { 27, 2, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, PROG, Vneg, Hneg },                     //1440x288@50Hz
    { 28, 2, 720, 288, 864, 312, 13500000L, 0x100, 12, 63, 69, 2, 3, 19, PROG, Vneg, Hneg },                     //1440x288@50Hz
    { 29, 1, 1440, 576, 1728, 625, 54000000L, 0x40, 24, 128, 136, 5, 5, 39, PROG, Vpos, Hneg },                  //1440x576@50Hz
    { 30, 1, 1440, 576, 1728, 625, 54000000L, 0x40, 24, 128, 136, 5, 5, 39, PROG, Vpos, Hneg },                  //1440x576@50Hz
    { 31, 0, 1920, 1080, 2640, 1125, 148500000L, 0x17, 528, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },               //1920x1080@50Hz
    { 32, 0, 1920, 1080, 2750, 1125, 74250000L, 0x2E, 638, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },                //1920x1080@24Hz
    { 33, 0, 1920, 1080, 2640, 1125, 74250000L, 0x2E, 528, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },                //1920x1080@25Hz
    { 34, 0, 1920, 1080, 2200, 1125, 74250000L, 0x2E, 88, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },                 //1920x1080@30Hz

    { 35, 2, 2880, 480, 1716 * 2, 525, 108000000L, 0x20, 32 * 2, 124 * 2, 120 * 2, 9, 6, 30, PROG, Vneg, Hneg }, //2880x480@60Hz
    { 36, 2, 2880, 480, 1716 * 2, 525, 108000000L, 0x20, 32 * 2, 124 * 2, 120 * 2, 9, 6, 30, PROG, Vneg, Hneg }, //2880x480@60Hz
    { 37, 1, 2880, 576, 3456, 625, 108000000L, 0x20, 24 * 2, 128 * 2, 136 * 2, 5, 5, 39, PROG, Vneg, Hneg },     //2880x576@50Hz
    { 38, 2, 2880, 576, 3456, 625, 108000000L, 0x20, 24 * 2, 128 * 2, 136 * 2, 5, 5, 39, PROG, Vneg, Hneg },     //2880x576@50Hz

    { 39, 0, 1920, 540, 2304, 625, 72000000L, 0x17, 32, 168, 184, 23, 5, 57, INTERLACE, Vneg, Hpos },            //1920x1080@50Hz
                                                                                                                 // 100Hz
    { 40, 0, 1920, 540, 2640, 562, 148500000L, 0x17, 528, 44, 148, 2, 5, 15, INTERLACE, Vpos, Hpos },            //1920x1080(I)@100Hz
    { 41, 0, 1280, 720, 1980, 750, 148500000L, 0x17, 440, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },                 //1280x720@100Hz
    { 42, 0, 720, 576, 864, 625,   54000000L, 0x40, 12, 64, 68, 5, 5, 39, PROG, Vneg, Hneg },                    //720x576@100Hz
    { 43, 0, 720, 576, 864, 625,   54000000L, 0x40, 12, 64, 68, 5, 5, 39, PROG, Vneg, Hneg },                    //720x576@100Hz
    { 44, 1, 720, 288, 864, 312,   27000000L, 0x80, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },               //1440x576(I)@100Hz
    { 45, 1, 720, 288, 864, 312,   27000000L, 0x80, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },               //1440x576(I)@100Hz
                                                                                                                 // 120Hz
    { 46, 0, 1920, 540, 2200, 562, 148500000L, 0x17, 88, 44, 148, 2, 5, 15, INTERLACE, Vpos, Hpos },             //1920x1080(I)@120Hz
    { 47, 0, 1280, 720, 1650, 750, 148500000L, 0x17, 110, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },                 //1280x720@120Hz
    { 48, 0, 720, 480, 858, 525, 54000000L, 0x40, 16, 62, 60, 9, 6, 30, PROG, Vneg, Hneg },                      //720x480@120Hz
    { 49, 0, 720, 480, 858, 525, 54000000L, 0x40, 16, 62, 60, 9, 6, 30, PROG, Vneg, Hneg },                      //720x480@120Hz
    { 50, 1, 720, 240, 858, 262, 27000000L, 0x80, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },                 //720x480(I)@120Hz
    { 51, 1, 720, 240, 858, 262, 27000000L, 0x80, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },                 //720x480(I)@120Hz

    // 200Hz
    { 52, 0, 720, 576, 864, 625, 108000000L, 0x20, 12, 64, 68, 5, 5, 39, PROG, Vneg, Hneg },        //720x576@200Hz
    { 53, 0, 720, 576, 864, 625, 108000000L, 0x20, 12, 64, 68, 5, 5, 39, PROG, Vneg, Hneg },        //720x576@200Hz
    { 54, 1, 720, 288, 864, 312, 54000000L, 0x40, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },    //1440x576(I)@200Hz
    { 55, 1, 720, 288, 864, 312, 54000000L, 0x40, 12, 63, 69, 2, 3, 19, INTERLACE, Vneg, Hneg },    //1440x576(I)@200Hz
                                                                                                    // 240Hz
    { 56, 0, 720, 480, 858, 525, 108000000L, 0x20, 16, 62, 60, 9, 6, 30, PROG, Vneg, Hneg },        //720x480@120Hz
    { 57, 0, 720, 480, 858, 525, 108000000L, 0x20, 16, 62, 60, 9, 6, 30, PROG, Vneg, Hneg },        //720x480@120Hz
    { 58, 1, 720, 240, 858, 262, 54000000L, 0x40, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },    //720x480(I)@120Hz
    { 59, 1, 720, 240, 858, 262, 54000000L, 0x40, 19, 62, 57, 4, 3, 15, INTERLACE, Vneg, Hneg },    //720x480(I)@120Hz
                                                                                                    // 720p low resolution
    { 60, 0, 1280, 720, 3300, 750, 59400000L, 0x3A, 1760, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },    //1280x720@24Hz
    { 61, 0, 1280, 720, 3960, 750, 74250000L, 0x2E, 2420, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },    //1280x720@25Hz
    { 62, 0, 1280, 720, 3300, 750, 74250000L, 0x2E, 1760, 40, 220, 5, 5, 20, PROG, Vpos, Hpos },    //1280x720@30Hz
                                                                                                    // 1080p high refresh rate
    { 63, 0, 1920, 1080, 2200, 1125, 297000000L, 0x0B, 88, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },   //1920x1080@120Hz
    { 64, 0, 1920, 1080, 2640, 1125, 297000000L, 0x0B, 528, 44, 148, 4, 5, 36, PROG, Vpos, Hpos },  //1920x1080@100Hz
                                                                                                    // VESA mode
    { 0, 0, 640, 350, 832, 445, 31500000L, 0x6D, 32, 64, 96, 32, 3, 60, PROG, Vneg, Hpos },         // 640x350@85
    { 0, 0, 640, 400, 832, 445, 31500000L, 0x6D, 32, 64, 96, 1, 3, 41, PROG, Vneg, Hneg },          // 640x400@85
    { 0, 0, 832, 624, 1152, 667, 57283000L, 0x3C, 32, 64, 224, 1, 3, 39, PROG, Vneg, Hneg },        // 832x624@75Hz
    { 0, 0, 720, 350, 900, 449, 28322000L, 0x7A, 18, 108, 54, 59, 2, 38, PROG, Vneg, Hneg },        // 720x350@70Hz
    { 0, 0, 720, 400, 900, 449, 28322000L, 0x7A, 18, 108, 54, 13, 2, 34, PROG, Vpos, Hneg },        // 720x400@70Hz
    { 0, 0, 720, 400, 936, 446, 35500000L, 0x61, 36, 72, 108, 1, 3, 42, PROG, Vpos, Hneg },         // 720x400@85
    { 0, 0, 640, 480, 800, 525, 25175000L, 0x89, 16, 96, 48, 10, 2, 33, PROG, Vneg, Hneg },         // 640x480@60
    { 0, 0, 640, 480, 832, 520, 31500000L, 0x6D, 24, 40, 128, 9, 3, 28, PROG, Vneg, Hneg },         // 640x480@72
    { 0, 0, 640, 480, 840, 500, 31500000L, 0x6D, 16, 64, 120, 1, 3, 16, PROG, Vneg, Hneg },         // 640x480@75
    { 0, 0, 640, 480, 832, 509, 36000000L, 0x60, 56, 56, 80, 1, 3, 25, PROG, Vneg, Hneg },          // 640x480@85
    { 0, 0, 800, 600, 1024, 625, 36000000L, 0x60, 24, 72, 128, 1, 2, 22, PROG, Vpos, Hpos },        // 800x600@56
    { 0, 0, 800, 600, 1056, 628, 40000000L, 0x56, 40, 128, 88, 1, 4, 23, PROG, Vpos, Hpos },        // 800x600@60
    { 0, 0, 800, 600, 1040, 666, 50000000L, 0x45, 56, 120, 64, 37, 6, 23, PROG, Vpos, Hpos },       // 800x600@72
    { 0, 0, 800, 600, 1056, 625, 49500000L, 0x45, 16, 80, 160, 1, 3, 21, PROG, Vpos, Hpos },        // 800x600@75
    { 0, 0, 800, 600, 1048, 631, 56250000L, 0x3D, 32, 64, 152, 1, 3, 27, PROG, Vpos, Hpos },        // 800X600@85
    { 0, 0, 848, 480, 1088, 517, 33750000L, 0x66, 16, 112, 112, 6, 8, 23, PROG, Vpos, Hpos },       // 840X480@60
    { 0, 0, 1024, 384, 1264, 408, 44900000L, 0x4C, 8, 176, 56, 0, 4, 20, INTERLACE, Vpos, Hpos },   //1024x768(I)@87Hz
    { 0, 0, 1024, 768, 1344, 806, 65000000L, 0x35, 24, 136, 160, 3, 6, 29, PROG, Vneg, Hneg },      // 1024x768@60
    { 0, 0, 1024, 768, 1328, 806, 75000000L, 0x2E, 24, 136, 144, 3, 6, 29, PROG, Vneg, Hneg },      // 1024x768@70
    { 0, 0, 1024, 768, 1312, 800, 78750000L, 0x2B, 16, 96, 176, 1, 3, 28, PROG, Vpos, Hpos },       // 1024x768@75
    { 0, 0, 1024, 768, 1376, 808, 94500000L, 0x24, 48, 96, 208, 1, 3, 36, PROG, Vpos, Hpos },       // 1024x768@85
    { 0, 0, 1152, 864, 1600, 900, 108000000L, 0x20, 64, 128, 256, 1, 3, 32, PROG, Vpos, Hpos },     // 1152x864@75
    { 0, 0, 1280, 768, 1440, 790, 68250000L, 0x32, 48, 32, 80, 3, 7, 12, PROG, Vneg, Hpos },        // 1280x768@60-R
    { 0, 0, 1280, 768, 1664, 798, 79500000L, 0x2B, 64, 128, 192, 3, 7, 20, PROG, Vpos, Hneg },      // 1280x768@60
    { 0, 0, 1280, 768, 1696, 805, 102250000L, 0x21, 80, 128, 208, 3, 7, 27, PROG, Vpos, Hneg },     // 1280x768@75
    { 0, 0, 1280, 768, 1712, 809, 117500000L, 0x1D, 80, 136, 216, 3, 7, 31, PROG, Vpos, Hneg },     // 1280x768@85

    { 0, 0, 1280, 800, 1440, 823, 71000000L, 0x31, 48, 32, 80, 3, 6, 14, PROG, Vpos, Hneg },        // 1280x800@60Hz
    { 0, 0, 1280, 800, 1680, 831, 83500000L, 0x29, 72, 128, 200, 3, 6, 22, PROG, Vpos, Hneg },      // 1280x800@60Hz
    { 0, 0, 1280, 800, 1696, 838, 106500000L, 0x20, 80, 128, 208, 3, 6, 29, PROG, Vpos, Hneg },     // 1280x800@75Hz
    { 0, 0, 1280, 800, 1712, 843, 122500000L, 0x1C, 80, 136, 216, 3, 6, 34, PROG, Vpos, Hneg },     // 1280x800@85Hz

    { 0, 0, 1280, 960, 1800, 1000, 108000000L, 0x20, 96, 112, 312, 1, 3, 36, PROG, Vpos, Hpos },    // 1280x960@60
    { 0, 0, 1280, 960, 1728, 1011, 148500000L, 0x17, 64, 160, 224, 1, 3, 47, PROG, Vpos, Hpos },    // 1280x960@85
    { 0, 0, 1280, 1024, 1688, 1066, 108000000L, 0x20, 48, 112, 248, 1, 3, 38, PROG, Vpos, Hpos },   // 1280x1024@60
    { 0, 0, 1280, 1024, 1688, 1066, 135000000L, 0x19, 16, 144, 248, 1, 3, 38, PROG, Vpos, Hpos },   // 1280x1024@75
    { 0, 0, 1280, 1024, 1728, 1072, 157500000L, 0x15, 64, 160, 224, 1, 3, 44, PROG, Vpos, Hpos },   // 1280X1024@85
    { 0, 0, 1360, 768, 1792, 795, 85500000L, 0x28, 64, 112, 256, 3, 6, 18, PROG, Vpos, Hpos },      // 1360X768@60

    { 0, 0, 1366, 768, 1792, 798, 85500000L, 0x28, 70, 143, 213, 3, 3, 24, PROG, Vpos, Hpos },      // 1366X768@60
    { 0, 0, 1366, 768, 1500, 800, 72000000L, 0x30, 14, 56, 64, 1, 3, 28, PROG, Vpos, Hpos },        // 1360X768@60
    { 0, 0, 1400, 1050, 1560, 1080, 101000000L, 0x22, 48, 32, 80, 3, 4, 23, PROG, Vneg, Hpos },     // 1400x768@60-R
    { 0, 0, 1400, 1050, 1864, 1089, 121750000L, 0x1C, 88, 144, 232, 3, 4, 32, PROG, Vpos, Hneg },   // 1400x768@60
    { 0, 0, 1400, 1050, 1896, 1099, 156000000L, 0x16, 104, 144, 248, 3, 4, 42, PROG, Vpos, Hneg },  // 1400x1050@75
    { 0, 0, 1400, 1050, 1912, 1105, 179500000L, 0x13, 104, 152, 256, 3, 4, 48, PROG, Vpos, Hneg },  // 1400x1050@85
    { 0, 0, 1440, 900, 1600, 926, 88750000L, 0x26, 48, 32, 80, 3, 6, 17, PROG, Vneg, Hpos },        // 1440x900@60-R
    { 0, 0, 1440, 900, 1904, 934, 106500000L, 0x20, 80, 152, 232, 3, 6, 25, PROG, Vpos, Hneg },     // 1440x900@60
    { 0, 0, 1440, 900, 1936, 942, 136750000L, 0x19, 96, 152, 248, 3, 6, 33, PROG, Vpos, Hneg },     // 1440x900@75
    { 0, 0, 1440, 900, 1952, 948, 157000000L, 0x16, 104, 152, 256, 3, 6, 39, PROG, Vpos, Hneg },    // 1440x900@85
    { 0, 0, 1600, 1200, 2160, 1250, 162000000L, 0x15, 64, 192, 304, 1, 3, 46, PROG, Vpos, Hpos },   // 1600x1200@60
    { 0, 0, 1600, 1200, 2160, 1250, 175500000L, 0x13, 64, 192, 304, 1, 3, 46, PROG, Vpos, Hpos },   // 1600x1200@65
    { 0, 0, 1600, 1200, 2160, 1250, 189000000L, 0x12, 64, 192, 304, 1, 3, 46, PROG, Vpos, Hpos },   // 1600x1200@70
    { 0, 0, 1600, 1200, 2160, 1250, 202500000L, 0x11, 64, 192, 304, 1, 3, 46, PROG, Vpos, Hpos },   // 1600x1200@75
    { 0, 0, 1600, 1200, 2160, 1250, 229500000L, 0x0F, 64, 192, 304, 1, 3, 46, PROG, Vpos, Hpos },   // 1600x1200@85
    { 0, 0, 1680, 1050, 1840, 1080, 119000000L, 0x1D, 48, 32, 80, 3, 6, 21, PROG, Vneg, Hpos },     // 1680x1050@60-R
    { 0, 0, 1680, 1050, 2240, 1089, 146250000L, 0x17, 104, 176, 280, 3, 6, 30, PROG, Vpos, Hneg },  // 1680x1050@60
    { 0, 0, 1680, 1050, 2272, 1099, 187000000L, 0x12, 120, 176, 296, 3, 6, 40, PROG, Vpos, Hneg },  // 1680x1050@75
    { 0, 0, 1680, 1050, 2288, 1105, 214750000L, 0x10, 128, 176, 304, 3, 6, 46, PROG, Vpos, Hneg },  // 1680x1050@85
    { 0, 0, 1792, 1344, 2448, 1394, 204750000L, 0x10, 128, 200, 328, 1, 3, 46, PROG, Vpos, Hneg },  // 1792x1344@60
    { 0, 0, 1792, 1344, 2456, 1417, 261000000L, 0x0D, 96, 216, 352, 1, 3, 69, PROG, Vpos, Hneg },   // 1792x1344@75
    { 0, 0, 1856, 1392, 2528, 1439, 218250000L, 0x0F, 96, 224, 352, 1, 3, 43, PROG, Vpos, Hneg },   // 1856x1392@60
    { 0, 0, 1856, 1392, 2560, 1500, 288000000L, 0x0C, 128, 224, 352, 1, 3, 104, PROG, Vpos, Hneg }, // 1856x1392@75
    { 0, 0, 1920, 1200, 2080, 1235, 154000000L, 0x16, 48, 32, 80, 3, 6, 26, PROG, Vneg, Hpos },     // 1920x1200@60-R
    { 0, 0, 1920, 1200, 2592, 1245, 193250000L, 0x11, 136, 200, 336, 3, 6, 36, PROG, Vpos, Hneg },  // 1920x1200@60
    { 0, 0, 1920, 1200, 2608, 1255, 245250000L, 0x0E, 136, 208, 344, 3, 6, 46, PROG, Vpos, Hneg },  // 1920x1200@75
    { 0, 0, 1920, 1200, 2624, 1262, 281250000L, 0x0C, 144, 208, 352, 3, 6, 53, PROG, Vpos, Hneg },  // 1920x1200@85
    { 0, 0, 1920, 1440, 2600, 1500, 234000000L, 0x0E, 128, 208, 344, 1, 3, 56, PROG, Vpos, Hneg },  // 1920x1440@60
    { 0, 0, 1920, 1440, 2640, 1500, 297000000L, 0x0B, 144, 224, 352, 1, 3, 56, PROG, Vpos, Hneg },  // 1920x1440@75
};

    #define     SizeofVMTable (sizeof(s_VMTable) / sizeof(HDMI_VTiming))
#else
    #define     SizeofVMTable 0
#endif

////////////////////////////////////////////////////////////////////////
// VIDEO and AUDIO DRIVING STRENGTH CONTROL
////////////////////////////////////////////////////////////////////////
// usually defined in the config.h

#ifndef CONFIG_VIDEO_DRIVING_STRENGTH
    #define CONFIG_VIDEO_DRIVING_STRENGTH 1 // 0 : weakest -> 1 -> 2 -> 3: strongest
#endif // CONFIG_VIDEO_DRIVING_STRENGTH

#ifndef CONFIG_AUDIO_DRIVING_STRENGTH
    #define CONFIG_AUDIO_DRIVING_STRENGTH 1 // 0 : weakest -> 1 -> 2 -> 3: strongest
#endif // CONFIG_AUDIO_DRIVING_STRENGTH

#if (CONFIG_VIDEO_DRIVING_STRENGTH == 3)
    #define VALUE_VIO_CTRL                0xBB
#elif  (CONFIG_VIDEO_DRIVING_STRENGTH == 2)
    #define VALUE_VIO_CTRL                0xAA
#elif  (CONFIG_VIDEO_DRIVING_STRENGTH == 1)
    #define VALUE_VIO_CTRL                0x99
#else
    #define VALUE_VIO_CTRL                0x89
#endif

#ifndef VCLK_PHASE_VALUE
    #define VCLK_PHASE_VALUE              0x16 // for IT6607 A1, it need to reverse.
#endif

BYTE JudgeFS();

typedef struct _REGPAIR {
    BYTE ucAddr;
    BYTE ucMask;
    BYTE ucValue;
} REGPAIR;

REGPAIR acDefaultValue[] =
{
#ifndef DISABLE_PIXEL_REPEAT_REMOVE
    { REG_RX_PWD_CTRL1, B_DIS_PIX_RPT, 0 },
#else
    { REG_RX_PWD_CTRL1, B_DIS_PIX_RPT, B_DIS_PIX_RPT },
#endif
    { REG_RX_VIO_CTRL, 0xFF, VALUE_VIO_CTRL },
    { REG_RX_AIO_CTRL, 0xFF, 0x99 },
#ifdef Enable_HDMI_Tx
    { REG_RX_VCLK_CTRL, 0xFF, VCLK_PHASE_VALUE },
#else
    { REG_RX_VCLK_CTRL, 0xFF, VCLK_PHASE_VALUE }, // 2011/12/19 modified by jau-chih.tseng
#endif

    { REG_RX_I2C_CTRL, 0xFF, 0x19 }, // Reg57
    { 0x96, 0xFF, 0x0A },            //0x0A -> 0x02
    { 0x73, 0xFF, 0x05 },            //0x05 -> 0x01

    { 0x19, 0xFF, 0x30 },            //0x05 -> 0x01
                                     //  {0x3B,0xFF,0x40},
                                     //  {REG_RX_PLL_CTRL,0xFF,0x03},
                                     //  {0x6B,0xFF,0x11},
                                     //  {REG_RX_EQUAL_CTRL2,0xFF,0x00},
                                     //  {REG_RX_DES_CTRL1,0xFF,0x64},
                                     //  {REG_RX_DES_CTRL2,0xFF,0x0C},
                                     //
                                     //  {0x93,0xFF,0x43},
                                     //  {0x94,0xFF,0x4F},
                                     //  {0x95,0xFF,0x87},
                                     //  {0x96,0xFF,0x33},

    { 0x93, 0xFF, 0xC7 },            //Ching-Lin,
    { 0x94, 0xFF, 0xC7 },            //Ching-Lin,
    { 0x95, 0xFF, 0xC7 },            //Ching-Lin,

#ifdef Auto_Reset
    //  {0x56,0xFF,0x01},
    { 0x97, 0xFF, 0x0E },
#endif

    { 0x97, 0xFF, 0x0E },

    { 0xF, 0xFF, 0x1 },

    { 0x7F, 0xFF, 0x6C }, //block 1 otp

    { 0x7C, 0xFF, 0x0 },
    { 0x7E, 0xFF, HDMIRX_OUTPUT_LOW_BIT_GATTING | HDMIRX_OUTPUT_VID_SIG2 },
    { 0xF, 0xFF, 0x0 },

    //6607的register裏面有一個default值需要更新
    //Reg0x56(7) bit 7，HW default為1，請幫我設成0
    //這個bit影響的是我們目前在做實驗的Reg0x8B的clock change interrupt
    //這個bit設起來的時候，類似插拔的狀況0x8B(7)可能會清不掉

    { 0x56, 0x80, 0x00 },

    //Please change It6607 register [68] bit7( IPRPLL_ENI1 ) default value  from 0 to 1.
    //Reason: To solve the issue “ PCLK jitter is too big especially with BW 1-dot pattern”
    //          The register change will increase IPRPLL bandwidth.
    //
    //Regards,
    //劉音尚 2011/01/11
    //
    { 0x68, 0x80, 0x80 }, //mylin mark for self test

    { REG_RX_INTERRUPT_CTRL, 0xFF, 0x30 },
    // video output mode
    { REG_RX_MISC_CTRL, 0xFF, 0x14 },
    { REG_RX_VIDEO_MAP, 0xFF, 0x00 },
    { REG_RX_VIDEO_CTRL1, 0xFF, HDMIRX_OUTPUT_VID_SIG },
    { REG_RX_PG_CTRL2, 0xFF, 0x00 },

    { REG_RX_I2S_CTRL, 0xFF, 0x60 },
    { REG_RX_I2S_MAP, 0xFF, 0xE4 },

    { REG_RX_HWMUTE_CTRL, 0xFF, 0x08 },
    { REG_RX_HWAMP_CTRL, 0xFF, 0x00 },
    { REG_RX_MCLK_CTRL, 0xFF, 0xC1 },

    //{0x93, 0x9f}, //mylin for self test
    //{0x94, 0x9f}, //mylin for self test
    //{0x95, 0x9f}, //mylin for self test

    { 0, 0, 0 }
};

#define SUPPORT_INPUTRGB
#define SUPPORT_INPUTYUV444
#define SUPPORT_INPUTYUV422
#define _HBR_I2S_

#if defined(SUPPORT_INPUTYUV444) || defined(SUPPORT_INPUTYUV422)
    #define SUPPORT_INPUTYUV
#endif

// #define SUPPORT_OUTPUTYUV
// #define SUPPORT_OUTPUTYUV444
// #define SUPPORT_OUTPUTYUV422

#if (defined(SUPPORT_OUTPUTYUV444)) || (defined(SUPPORT_OUTPUTYUV422))
    #define SUPPORT_OUTPUTYUV
#endif

#if !(defined(SUPPORT_OUTPUTRGB)) && !(defined(SUPPORT_OUTPUTYUV444)) && !(defined(SUPPORT_OUTPUTYUV422))
    #define SUPPORT_OUTPUTRGB
#endif

extern _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[];
extern _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[];
extern _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[];
extern _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[];
extern _CODE BYTE bCSCMtx_YUV2RGB_ITU601_16_235[];
extern _CODE BYTE bCSCMtx_YUV2RGB_ITU601_0_255[];
extern _CODE BYTE bCSCMtx_YUV2RGB_ITU709_16_235[];
extern _CODE BYTE bCSCMtx_YUV2RGB_ITU709_0_255[];

// static _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[]=
// {
//  0x00,0x80,0x10,//Reg21 ~ 23
//  0xB2,0x04,0x65,0x02,0xE9,0x00,//Reg24 ~ 29
//  0x93,0x3C,0x18,0x04,0x55,0x3F,//Reg2A ~ 2F
//  0x49,0x3D,0x9F,0x3E,0x18,0x04   //Reg30 ~ 35
// };
//
// static _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[]=
// {
//  0x10,0x80,0x10,//Reg21 ~ 23
//  0x09,0x04,0x0E,0x02,0xC9,0x00,//Reg24 ~ 29
//  0x0F,0x3D,0x84,0x03,0x6D,0x3F,//Reg2A ~ 2F
//  0xAB,0x3D,0xD1,0x3E,0x84,0x03   //Reg30 ~ 35
// };
//
// static _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[]=
// {
//  0x00,0x80,0x10,//Reg21 ~ 23
//  0xB8,0x05,0xB4,0x01,0x94,0x00,//Reg24 ~ 29
//  0x4A,0x3C,0x17,0x04,0x9F,0x3F,//Reg2A ~ 2F
//  0xD9,0x3C,0x10,0x3F,0x17,0x04   //Reg30 ~ 35
// };
//
// static _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[]=
// {
//  0x10,0x80,0x10,//Reg21 ~ 23
//  0xEA,0x04,0x77,0x01,0x7F,0x00,//Reg24 ~ 29
//  0xD0,0x3C,0x83,0x03,0xAD,0x3F,//Reg2A ~ 2F
//  0x4B,0x3D,0x32,0x3F,0x83,0x03   //Reg30 ~ 35
// };
//
//
// BYTE _CODE bCSCMtx_YUV2RGB_ITU601_16_235[] =
// {
//  0x00,0x00,0x00,//Reg21 ~ 23
//  0x00,0x08,0x6B,0x3A,0x50,0x3D,//Reg24 ~ 29
//  0x00,0x08,0xF5,0x0A,0x02,0x00,//Reg2A ~ 2F
//  0x00,0x08,0xFD,0x3F,0xDA,0x0D   //Reg30 ~ 35
// };
//
// BYTE _CODE bCSCMtx_YUV2RGB_ITU601_0_255[] =
// {
//  0x04,0x00,0xA7,//Reg21 ~ 23
//  0x4F,0x09,0x81,0x39,0xDD,0x3C,//Reg24 ~ 29
//  0x4F,0x09,0xC4,0x0C,0x01,0x00,//Reg2A ~ 2F
//  0x4F,0x09,0xFD,0x3F,0x1F,0x10   //Reg30 ~ 35
// };
//
// BYTE _CODE bCSCMtx_YUV2RGB_ITU709_16_235[] =
// {
//  0x00,0x00,0x00,//Reg21 ~ 23
//  0x00,0x08,0x55,0x3C,0x88,0x3E,//Reg24 ~ 29
//  0x00,0x08,0x51,0x0C,0x00,0x00,//Reg2A ~ 2F
//  0x00,0x08,0x00,0x00,0x84,0x0E   //Reg30 ~ 35
// };
//
// BYTE _CODE bCSCMtx_YUV2RGB_ITU709_0_255[] =
// {
//  0x04,0x00,0xA7,//Reg21 ~ 23
//  0x4F,0x09,0xBA,0x3B,0x4B,0x3E,//Reg24 ~ 29
//  0x4F,0x09,0x57,0x0E,0x02,0x00,//Reg2A ~ 2F
//  0x4F,0x09,0xFE,0x3F,0xE8,0x10   //Reg30 ~ 35
// };
//
//
CHAR _CODE *VStateStr[] = {
    "VSTATE_Off",
    "VSTATE_PwrOff",
    "VSTATE_SyncWait",
    "VSTATE_SWReset",
    "VSTATE_SyncChecking",
    "VSTATE_HDCPSet",
    "VSTATE_HDCP_Reset",
    "VSTATE_ModeDetecting",
    "VSTATE_VideoOn",
    "VSTATE_ColorDetectReset",
    "VSTATE_Reserved"
};

CHAR _CODE *AStateStr[] = {
    "ASTATE_AudioOff",
    "ASTATE_RequestAudio",
    "ASTATE_ResetAudio",
    "ASTATE_WaitForReady",
    "ASTATE_AudioOn",
    "ASTATE_Reserved"
};

//#define SetAVMUTE() SetMUTE(B_VDO_MUTE_DISABLE,(B_VDO_MUTE_DISABLE|B_TRI_AUDIO))
#define StartAutoMuteOffTimer()                  { it6607dev.m_MuteAutoOff = ON; }
#define EndAutoMuteOffTimer()                    { it6607dev.m_MuteAutoOff = OFF; }

// #define Switch_HDMIRX_Bank(x) HDMIRX_WriteI2C_Byte(0x0f,(x)&1)
#define HDMIRX_SetReg_Byte(reg, andmask, ormask) HDMIRX_WriteI2C_Byte(reg, ((HDMIRX_ReadI2C_Byte(reg) & (andmask)) | (ormask)))

////////////////////////////////////////////////////////////////////////////////////
// Static Function Prototype here.
////////////////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// initial function
// ***************************************************************************
#define clearHDMIRX_Event()        {it6607dev.m_EventFlags = 0; }
#define setHDMIRX_Event(EventMask) {it6607dev.m_EventFlags |= (EventMask); }

unsigned int hdmirx_select_otp()
{
    unsigned int  addr   = 0x0;
    unsigned char rddata = 0;

    HDMIRX_WriteI2C_Byte(0x67, 0x12);

    HDMIRX_WriteI2C_Byte(0x0f, 0xc1);
    HDMIRX_WriteI2C_Byte(0x0f, 0x01);
    HDMIRX_WriteI2C_Byte(0x7f, 0x09);
    HDMIRX_WriteI2C_Byte(0x57, 0x12);
    HDMIRX_WriteI2C_Byte(0x57, 0x02);
    HDMIRX_WriteI2C_Byte(0x56, 0x01);
    HDMIRX_WriteI2C_Byte(0x59, 0x75);

    HDMIRX_WriteI2C_Byte(0x50, (0x02 + ((addr & 0xF00) >> 8)));
    HDMIRX_WriteI2C_Byte(0x51, addr & 0xFF);
    HDMIRX_WriteI2C_Byte(0x53, 0x04);

    // Read back 1Byte
    rddata = HDMIRX_ReadI2C_Byte(0x61);

    HDMIRX_WriteI2C_Byte(0x0f, 0x00);

    if (gtDisableHDCP)
    {
        gtIsHDCPKeyEmpty = 1;
        //select empty otp
        if (rddata == 0x00)
            return 1;
        else
            return 0;
    }
    else
    {
        gtIsHDCPKeyEmpty = 0;

        if (rddata == 0x00)
            return 0;
        else
            return 1;
    }
}

void hdmirx_SetDefaultRegisterValue()

{
    unsigned int i;
    unsigned int otp_block = 0, enblank1 = 0;

    otp_block = hdmirx_select_otp();

    for (i = 0; acDefaultValue[i].ucAddr != 0; i++)
    {
        if (enblank1)
        {
            if (otp_block == 0 && acDefaultValue[i].ucAddr == 0x7F)      //block 0 otp
                acDefaultValue[i].ucValue = acDefaultValue[i].ucValue & 0xFB;
            else if (otp_block == 1 && acDefaultValue[i].ucAddr == 0x7F) //block 1 otp
                acDefaultValue[i].ucValue = ((acDefaultValue[i].ucValue & 0xFB) | 0x04);

            if (acDefaultValue[i].ucAddr == 0x7F)
                printf("9919 Block = %d, (%d)\n", otp_block, gtDisableHDCP);
        }

        HDMIRX_SetI2C_Byte(acDefaultValue[i].ucAddr, acDefaultValue[i].ucMask, acDefaultValue[i].ucValue);

        if (acDefaultValue[i].ucAddr == 0xF && acDefaultValue[i].ucValue == 0x1)
            enblank1 = 1;
        else if (acDefaultValue[i].ucAddr == 0xF && acDefaultValue[i].ucValue == 0x0)
            enblank1 = 0;
    }
}

// ---------------------------------------------------------------------------
void hdmiregister_reset(void)
{
    // i2c_write(0x90,0x05,0x1,0xff);    // Register reset !!!
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_REGRST);
}

// ---------------------------------------------------------------------------
void SWReset_HDMIRX(void)
{
    HDMIRX_FUNCTION_PRINTF((" SWReset_HDMIRX() \n"));
    HDMIRX_DEBUG_PRINTF((" SWReset_HDMIRX() \n"));

    IT6607SwitchVideoState(VSTATE_SWReset);

#ifdef Enable_HDMI_Tx
    SetAllTxAVMute();
    ResetTxVideo();         //2011/01/05
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_Terminator_Off()
{
    BYTE uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_PWD_CTRL1) | (B_PWD_AFEALL | B_PWDC_ETC);

    if (it6607dev.m_ucCurrentHDMIPort == CAT_HDMI_PORTA)
    {
        uc &= ~B_PORT_SEL_B;
    }
    else
    {
        uc |= B_PORT_SEL_B;
    }

    HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, uc);

#ifdef     _SUPPORT_HDCP_RX_REPEATER_
    hdmirx_HDCPRepeaterCapabilityClear(B_KSV_READY);
#endif

    HDMIRX_FUNCTION_PRINTF(("hdmirx_Terminator_Off(),reg07=0x%02x\n", uc));
}

// ---------------------------------------------------------------------------
void TurnOff_HDMIRX()
{
    HDMIRX_DEBUG_PRINTF(("123!!!!!!!!!!!!!!!!!TurnOff_HDMIRX() !!!!!!!!!!!!!!!!!!!!!\n"));
    //HotPlug(HPDOFF);
    hdmirx_Terminator_Off();
#ifdef Enable_HDMI_Tx
    m_HPDGPIO = FALSE;      //2011/02/18 for select port issue
#endif

    // IT6607SwitchVideoState(VSTATE_SWReset);
    IT6607SwitchVideoState(VSTATE_Off);
}

// ---------------------------------------------------------------------------

void TurnOn_HDMIRX(BOOL bRepeaterEnable)
{
    HDMIRX_DEBUG_PRINTF(("123!!!!!!!!!!!!!!!!!TurnOn_HDMIRX() !!!!!!!!!!!!!!!!!!!!!\n"));

    // IT6607SwitchVideoState(VSTATE_PwrOff);

#ifdef _SUPPORT_HDCP_RX_REPEATER_
    #ifdef _ENABLE_HDCP_REPEATER_
    EnableRepeater(bRepeaterEnable);
    #else
    EnableRepeater(FALSE);
    #endif
#else
    bRepeaterEnable = 0;
#endif
    hdmirx_ini();
    //HotPlug(HPDOFF);

#ifdef Enable_HDMI_Tx
    m_HPDGPIO = TRUE;   //2011/02/18 for select port issue
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_Terminator_On()
{
    BYTE uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_PWD_CTRL1) & ~(B_PWD_AFEALL | B_PWDC_ETC);

    if (it6607dev.m_ucCurrentHDMIPort == CAT_HDMI_PORTA)
    {
        uc &= ~B_PORT_SEL_B;
    }
    else
    {
        uc |= B_PORT_SEL_B;
    }

    HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, uc);

    HDMIRX_FUNCTION_PRINTF(("hdmirx_Terminator_On(),reg07=0x%02x\n", uc));
}

// ---------------------------------------------------------------------------
void IT6607Standby(void)
{
    HDMIRX_WriteI2C_Byte(0x06, 0xFE);    //Req DDCWR ByteAdr 06  ByteNo 01 WData  FE;
    HDMIRX_WriteI2C_Byte(0x96, 0x1A);    //Req DDCWR ByteAdr 96  ByteNo 01 WData  19;
    HDMIRX_WriteI2C_Byte(0x97, 0x80);    //Req DDCWR ByteAdr 97  ByteNo 01 WData  80;
    HDMIRX_WriteI2C_Byte(0x89, 0xBF);    //Req DDCWR ByteAdr 89  ByteNo 01 WData  BF;
    HDMIRX_WriteI2C_Byte(0x0F, 0x01);    //Req DDCWR ByteAdr 0F  ByteNo 01 WData  01;

    HDMIRX_WriteI2C_Byte(0x7E, 0xF8);    //Req DDCWR ByteAdr 7E  ByteNo 01 WData  F8;
    HDMIRX_WriteI2C_Byte(0x0F, 0x00);    //Req DDCWR ByteAdr 0F  ByteNo 01 WData  01;
}

// ---------------------------------------------------------------------------
void hdmirx_ini(void)
{
    BYTE uc;

    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_REGRST);

    uc = HDMIRX_ReadI2C_Byte(REG_RX_RST_CTRL);

    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, uc | B_SWRST | B_HDCPRST | B_EN_AUTOVDORST);
    // HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, uc & ~B_SWRST ) ;
    clearHDMIRX_Event();

    hdmirx_SetDefaultRegisterValue();

    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK1, (/*B_VIDMODE_CHG|B_HDMIMODE_CHG|*/ B_SCDTOFF | B_SCDTON | B_PWR5VOFF | B_PWR5VON));
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK2, (B_NEW_AUD_PKT | B_NEW_AVI_PKG | B_PKT_SET_MUTE | B_PKT_CLR_MUTE | B_NEW_ACP_PKT));
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK3, (B_AUDFIFOERR | B_ECCERR | B_R_AUTH_START));

    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_MASK4, (0));

    if (gtIsHDCPKeyEmpty)
    {
        HDMIRX_WriteI2C_Byte(0x0A, 0);
        HDMIRX_WriteI2C_Byte(0x0B, 0);
        HDMIRX_WriteI2C_Byte(REG_RX_HDCP_CTRL, 0x89);
        HDMIRX_WriteI2C_Byte(REG_RX_HDCP_CTRL, 0x99);
        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_SWRST | B_HDCPRST | B_EN_AUTOVDORST); // sw reset
        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_EN_AUTOVDORST);                       // sw reset
        HDMIRX_WriteI2C_Byte(REG_RX_HDCP_CTRL, 0x00);
    }

    hdmirx_Terminator_Off();
    IT6607SwitchVideoState(VSTATE_SWReset);
}

// ---------------------------------------------------------------------------
void Init_HDMIRX()
{
    // IT6607SwitchVideoState(VSTATE_SWReset);
    //init_it6607dev() ;
    hdmirx_ini();
}

// ---------------------------------------------------------------------------
void init_it6607dev()
{
    // it6607dev.m_ucCurrentHDMIPort=0xFF;
    it6607dev.m_ucVideoOnCount        = 0;
    it6607dev.m_ucSCDTOffCount        = 0;
    it6607dev.m_ucEccCount            = 0;
    it6607dev.m_ucDVISCDToffCNT       = 0;
    it6607dev.m_VState                = VSTATE_Off;
    it6607dev.m_AState                = ASTATE_AudioOff;
    it6607dev.m_RxHDCPState           = RxHDCP_PwrOff;

    it6607dev.m_SWResetTimeOut        = 0;
    it6607dev.m_MuteResumingTimer     = 0;
    it6607dev.m_VideoCountingTimer    = 0;
    it6607dev.m_AudioCountingTimer    = 0;

    it6607dev.m_bVideoOnCountFlag     = FALSE;

    it6607dev.m_MuteAutoOff           = FALSE;
    it6607dev.m_bUpHDMIMode           = FALSE;
    it6607dev.m_bUpHDCPMode           = FALSE;
    it6607dev.m_NewAVIInfoFrameF      = FALSE;
    it6607dev.m_NewAUDInfoFrameF      = FALSE;
    it6607dev.m_HDCPRepeater          = FALSE;
    it6607dev.m_MuteByPKG             = OFF;

    it6607dev.m_bOutputVideoMode      = HDMIRX_OUTPUT_VID_MODE;
    it6607dev.m_ucAudioSampleClock    = 0x03;
    it6607dev.m_ucHDMIAudioErrorCount = 0;
    it6607dev.m_bRxAVmute             = FALSE;

    it6607dev.m_bOldReg8B             = 0;
    it6607dev.m_ucNewSCDT             = 0;
    it6607dev.m_ucOldSCDT             = 0;

    it6607dev.m_ucSCDTonCount         = 0;
    it6607dev.m_ucVideoModeChange     = 0;
    clearHDMIRX_Event();

#ifdef _IT6607_GeNPacket_Usage_
    it6607dev.m_PollingPacket      = 0;
    it6607dev.m_PacketState        = 0;
    it6607dev.m_ACPState           = 0;
    it6607dev.m_GamutPacketRequest = FALSE;
    it6607dev.m_GeneralRecPackType = 0x00;
#endif
}

// ---------------------------------------------------------------------------
BOOL CheckHDMIRX()
{
    IT6607Interrupt_Handler();
    IT6607VideoHandler();
    //IT6607Interrupt_Handler();
    IT6607AudioHandler();
    //IT6607Interrupt_Handler();
#ifdef _RX_HDCP_STATE_MACHINE_
    IT6607_HDCPHandler();
#endif

    if (it6607dev.m_VState == VSTATE_VideoOn)
    {
        return TRUE;
    }
    return FALSE;
}

// ***************************************************************************
// Interrupt function
// ***************************************************************************

void hdmirx_INT_5V_PwrOff()
{
    if (!HDMIRX_IsPlg5VPwr())
    {
        IT6607SwitchVideoState(VSTATE_SWReset);

#ifdef Enable_HDMI_Tx
        bEnableAuth     = FALSE;
        m_BackEndActive = TRUE;     //2010/07/21
        ResetTxVideo();
#endif
    }
}

// ---------------------------------------------------------------------------
void hdmirx_INT_5V_PwrOn()
{
    if (HDMIRX_IsPlg5VPwr())
    {
        IT6607SwitchVideoState(VSTATE_SyncWait);
    }
}

// ---------------------------------------------------------------------------
void hdmirx_INT_SCDT_On(void)
{
    if (HDMIRX_IsSCDT())
    {
        IT6607SwitchVideoState(VSTATE_SyncChecking);
    }
}

// ---------------------------------------------------------------------------
void hdmirx_INT_SCDT_Off()
{
    IT6607SwitchVideoState(VSTATE_SyncWait);

#ifdef _SUPPORT_HDCP_RX_REPEATER_
    IT6607_SwitchRxHDCPState(RxHDCP_PwrOff);        //max7088 _20080923_ for init hdcp state machine
#endif

    IT6607SwitchAudioState(ASTATE_AudioOff);

#ifdef Enable_HDMI_Tx
    ResetTxVideo();     //2011/01/05
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_INT_VideoMode_Chg()
{
    if (it6607dev.m_VState == VSTATE_VideoOn)
    {
        IT6607SwitchVideoState(VSTATE_SyncWait);

#ifdef Enable_HDMI_Tx
        ResetTxVideo();     //2011/01/05
#endif
    }
    it6607dev.m_ucVideoModeChange++;
}

// ---------------------------------------------------------------------------
void hdmirx_INT_HDMIMode_Chg()
{
    HDMIRX_DEBUG_PRINTF(("#### HDMI Mode Changed ####\n"));

    if (HDMIRX_IsHDMIMode())
    {
        if (it6607dev.m_VState == VSTATE_VideoOn)
        {
            IT6607SwitchAudioState(ASTATE_RequestAudio);
        }
        it6607dev.m_bUpHDMIMode = TRUE;
    }
    else
    {
        IT6607SwitchAudioState(ASTATE_AudioOff);
        it6607dev.m_NewAVIInfoFrameF = FALSE;
        if (it6607dev.m_VState == VSTATE_VideoOn)
        {
            hdmirx_SetDVIVideoOutput();
        }
        it6607dev.m_bUpHDMIMode = FALSE;
    }
}

// ---------------------------------------------------------------------------
void hdmirx_INT_ResetAudio()
{
    if (it6607dev.m_AState != ASTATE_AudioOff)
    {
        IT6607SwitchAudioState(ASTATE_RequestAudio);
    }
}

// ---------------------------------------------------------------------------
//void hdmirx_INT_ResetHDCP()
//{
////    BYTE uc;
//  if(it6607dev.m_VState==VSTATE_VideoOn) {
//      HDMIRX_DEBUG_PRINTF(("!!!!!!!!!!!!!!!!B_CLR_ECC_INT !!!!!!!!!!!!! \n"));
//      if(it6607dev.m_ucEccCount++>10) {
//          HDMIRX_DEBUG_PRINTF(("!!!!!!!!!!!!!!!!B_CLR_ECC_INT active  !!!!!!!!!!!!!\n"));
//          SWReset_HDMIRX();
//      }
//  }
//}
// ---------------------------------------------------------------------------
void hdmirx_INT_AVMute_Set()
{
    SetVideoMute(TRUE);
    IT6607AudioOutput(OFF);

    it6607dev.m_MuteByPKG = ON;

#ifdef Enable_HDMI_Tx
    SetTxAVMute();  //8-16 AV mute
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_INT_AVMute_Clear()
{
    SetVideoMute(FALSE);
#ifdef Enable_HDMI_Tx
    ClrTxAVMute();
#endif
    if (it6607dev.m_AState == ASTATE_AudioOn)
    {
        IT6607AudioOutput(ON);
    }

    it6607dev.m_MuteByPKG = OFF;
}

void hdmirx_INT_CheckACP()
{
    BYTE uc;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_REGPKTFLAG_CTRL);     //reg1F=0x00
    HDMIRX_WriteI2C_Byte(REG_RX_REGPKTFLAG_CTRL, uc & (~B_RX_INT_EVERY_ACP));

    uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL);      //reg19=0x02 -> 0x30
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL, uc | 0x02);
    HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL, uc & 0xFD);
}

void hdmirx_INT_SetNewAVIInfo(void)
{
    it6607dev.m_NewAVIInfoFrameF = TRUE;

    if (it6607dev.m_VState == VSTATE_VideoOn)
    {
        hdmirx_SetNewInfoVideoOutput();

#ifdef Enable_HDMI_Tx
        SetupTxVideo();
#endif
    }
}

// ---------------------------------------------------------------------------
void hdmirx_XVYccCheck(void)    // if AVI change then check XVYCC again !!
{
    BYTE uc;

    uc  = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB2);
    uc &= M_AVI_CLRMET_MASK << O_AVI_CLRMET;

    if (uc == (B_AVI_CLRMET_Extend << O_AVI_CLRMET))
    {
        // Switch_ACPRecPackState(1);   // Request ACP packet to receive xvYcc
        // Enable XVYCC (Gamut Get)
    }
    else
    {
        // Switch_ACPRecPackState(0);   // Indicate HDMI source no support XVYCC
        // Enable XVYCC (Gamut Get)
    }
}

// void ClearGeneralRecPackType(void)
// {
//     //   BYTE uc;
//
//  Switch_GeneralRecPackState(0);
//
//  //HDMIRX_WriteI2C_Byte(REG_RX_PKT_REC_TYPE,0x03);
//
//     // Support3DFramePacking=FALSE;
//     ////if( uHDMIRXRev > 0xA2 )
//     //{
//     //    uc = HDMIRX_ReadI2C_Byte(0x3C);
//     //    uc &= ~(1<<2);
//     //    HDMIRX_WriteI2C_Byte(0x3C,uc);
//     //
//     //    HDMIRX_DEBUG_PRINTF(("Support3DFramePacking -> FALSE .\n"));
//     //}
//
//
// }

// ---------------------------------------------------------------------------
void IT6607Interrupt_Handler(void)
{
    unsigned char Reg13h;
    unsigned char Reg14h;
    unsigned char Reg15h;
    unsigned char Reg8Bh;
    unsigned char uc;

    // modified by JJ -- The interrupt signal checking should be moved out to main loop instead off the inner.
    // #ifdef Enable_RX_INT
    //  if(IT6607_INT_FLAG==1)  //polling RX_INT pin at P3.2 INT0
    //  return;
    // #else
    //  if(IT6607_INT)  //polling RX_INT pin at P3.2 INT0
    //  return;
    // #endif

    if (it6607dev.m_VState == VSTATE_SWReset || it6607dev.m_VState == VSTATE_Off)
    {
        return;
    }

    Reg8Bh = HDMIRX_ReadI2C_Byte(0x8B) & 0xC7;  // for GenPKT,ISR1,ISR2 packet detect !!!

    Reg13h = HDMIRX_ReadI2C_Byte(0x13);
    HDMIRX_WriteI2C_Byte(0x13, Reg13h | ((Reg8Bh & 0xC0) ? 0x20 : 0x00));
    Reg14h = HDMIRX_ReadI2C_Byte(0x14);
    HDMIRX_WriteI2C_Byte(0x14, Reg14h);
    Reg15h = HDMIRX_ReadI2C_Byte(0x15) & 0xFD;
    HDMIRX_WriteI2C_Byte(0x15, Reg15h);

#ifdef Enable_RX_INT
    //for enable 8051 INT0
    IT6607_INT_FLAG = 1;
    EX0             = 1; // Enable ExInt0

#endif

#if 0
    HDMIRX_DEBUG_PRINTF(("1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n")); //for verfiy int event after power on and sw reset
    HDMIRX_DEBUG_PRINTF(("Reg13h=0x%02x\n", Reg13h));
    HDMIRX_DEBUG_PRINTF(("Reg14h = 0x%02x\n", Reg14h));
    HDMIRX_DEBUG_PRINTF(("Reg15h = 0x%02x\n", Reg15h));
    HDMIRX_DEBUG_PRINTF(("Reg8Bh = 0x%02x\n", Reg8Bh));
#endif

    if (Reg13h != 0x00)
    {
        // video parameter handling sequence:
        // 1st: 5V off <- need to do termination off or any reset action here.
        // 2st: 5V on <- need "also check status",only action when status also on.
        // 3rd: SCDT off <- switch video state to it6607dev.VState_WaitSync,start a video mode probing.
        // 4th: SCDT on <- begin count video mode.
        // 5th: HDMI/Video mode change: only available on VideoOn state. Other state beacuse the
        //      video format will be redetect when switch into VideoOn state,can be ignored.

        if (Reg13h & 0x02)
        {
            HDMIRX_DEBUG_PRINTF(("#### Power 5V OFF ####\n"));
            setHDMIRX_Event(EVENT_5V_OFF);

            hdmirx_INT_5V_PwrOff();
        }

        if (it6607dev.m_VState == VSTATE_SWReset || it6607dev.m_VState == VSTATE_Off)
        {
            return;
        }

        if (Reg13h & 0x01)
        {
            HDMIRX_DEBUG_PRINTF(("#### Power 5V ON ####\n"));
            hdmirx_INT_5V_PwrOn();
            setHDMIRX_Event(EVENT_5V_ON);
        }

#ifdef ENABLE_SELF_TIMER_INTERRUPT
        if (Reg13h & 0x40)
        {
            HDMIRX_WriteI2C_Byte(0x13, 0x40);
            HDMIRX_DEBUG_PRINTF(("#### Timer Counter Tntterrupt ####\n"));
            bDumpTiming = TRUE; //2011/06/17 xxxxx
        }
#endif

        if (Reg13h & 0x08)
        {
            HDMIRX_DEBUG_PRINTF(("#### SCDT OFF  !!! %d !!!####\n", (int)it6607dev.m_ucSCDTonCount));
            hdmirx_INT_SCDT_Off();
            setHDMIRX_Event(EVENT_SCDT_OFF);
            it6607dev.m_ucSCDTOffCount = 0;
        }

        if (Reg13h & 0x04)
        {
            HDMIRX_DEBUG_PRINTF(("#### SCDT ON  !!! %d !!!####\n", it6607dev.m_ucSCDTonCount++));
            hdmirx_INT_SCDT_On();
            setHDMIRX_Event(EVENT_SCDT_ON);
        }

        {
            // only video on mode,the video change and HDMI change situation should be handlled,
            // otherwise,ignore them.
            if (Reg13h & 0x20)
            {
                HDMIRX_DEBUG_PRINTF(("#### Video Mode Changed ####\n"));
                hdmirx_INT_VideoMode_Chg();
            }

            if (Reg13h & 0x10)
            {
                HDMIRX_DEBUG_PRINTF(("#### HDMI Mode Changed ####\n"));
                hdmirx_INT_HDMIMode_Chg();
            }
        }
    }

    if (it6607dev.m_bUpHDMIMode) // only HDMI mode process this.
    {
        if (Reg14h != 0x00)
        {
            // video package handling:
            // hdmirx_INT_AVMute_Set()
            if (Reg14h & 0x01)
            {
                HDMIRX_DEBUG_PRINTF(("#### Set Mute Packet Received ####\n"));
                HDMIRX_DEBUG_PRINTF(("reg65 = 0x%02x !!!\n", HDMIRX_ReadI2C_Byte(0x65)));
                hdmirx_INT_AVMute_Set();
                setHDMIRX_Event(EVENT_SET_AVMUTE);
            }

            if (Reg14h & 0x80)
            {
                HDMIRX_DEBUG_PRINTF(("#### Packet Left Mute ####\n"));
                hdmirx_INT_AVMute_Clear();
                HDMIRX_DEBUG_PRINTF(("reg65 = 0x%02x !!!\n", HDMIRX_ReadI2C_Byte(0x65)));
                setHDMIRX_Event(EVENT_CLR_AVMUTE);
            }

            if (Reg14h & 0x40)
            {
                HDMIRX_DEBUG_PRINTF(("#### New Audio InfoFrame Received ####\n"));
                setHDMIRX_Event(EVENT_NEW_AUD);
            }

            if (Reg14h & 0x20)
            {
                HDMIRX_DEBUG_PRINTF(("#### New ACP Packet Received ####\n"));
#ifdef _IT6607_ACPPacket_Usage_
                // UpdateACPPacket();
                // Check if Get XVYCC
                hdmirx_INT_CheckACP();
                setHDMIRX_Event(EVENT_NEW_ACP);
#endif
            }

            if (Reg14h & 0x10)
            {
                HDMIRX_DEBUG_PRINTF(("#### New SPD Packet Received ####\n"));
            }

            if (Reg14h & 0x08)
            {
                HDMIRX_DEBUG_PRINTF(("#### New MPEG InfoFrame Received ####\n"));
            }

            if (Reg14h & 0x04)
            {
                HDMIRX_DEBUG_PRINTF(("#### New AVI InfoFrame Received ####\n"));
                setHDMIRX_Event(EVENT_NEW_AVI);
                hdmirx_INT_SetNewAVIInfo();
#ifdef _IT6607_ACPPacket_Usage_
                hdmirx_XVYccCheck();
#endif
            }

            if (Reg14h & 0x02)
            {
                HDMIRX_DEBUG_PRINTF(("#### No AVI InfoFrame Received ####\n"));
                setHDMIRX_Event(EVENT_NO_AVI);
            }
        }
    }

    if (1)                           //( Reg15h!=0x00 && Reg15h!=0x04 ) {
    {
        if (it6607dev.m_bUpHDMIMode) // only HDMI mode process this.
        {
            if (Reg15h & 0x80)
            {
                HDMIRX_DEBUG_PRINTF(("#### No General Packet 2 Received ####\n"));
                uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL);
                HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL, uc | 0x08);
                //HDMIRX_WriteI2C_Byte(0x15,0x80);
                //HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL,uc);
            }

            if (Reg15h & 0x40)
            {
                HDMIRX_DEBUG_PRINTF(("#### No General Packet Received ####\n"));
                uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL);
                HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL, uc | 0x04);
                //HDMIRX_WriteI2C_Byte(0x15,0x40);
                //HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL,uc);
            }

            if (Reg15h & 0x20)
            {
                HDMIRX_DEBUG_PRINTF(("#### No Audio InfoFrame Received ####\n"));
            }

            if (Reg15h & 0x10)
            {
                HDMIRX_DEBUG_PRINTF(("#### Audio Auto Mute ####\n"));
                hdmirx_SetHWMuteClrMode();
            }

            if (Reg15h & 0x08)
            {
                HDMIRX_DEBUG_PRINTF(("#### Audio FIFO Error ####, 0x%x, 0x%x\n", it6607dev.m_ucHDMIAudioErrorCount, gtAudioErrorType1Cnt));
                it6607dev.m_ucHDMIAudioErrorCount++;
#if 1
                hdmirx_ResetAudio();
#else
                hdmirx_INT_ResetAudio();
#endif
            }
            else
            {
                if (it6607dev.m_ucHDMIAudioErrorCount != 0)
                    gtAudioErrorType1Cnt++;
                else
                    gtAudioErrorType1Cnt = 0;
                it6607dev.m_ucHDMIAudioErrorCount = 0;
            }
        }

        if (Reg15h & 0x04)
        {
            HDMIRX_DEBUG_PRINTF(("#### ECC Error ####\n"));
        }

        if (Reg15h & 0x01)
        {
            HDMIRX_DEBUG_PRINTF(("#### HDCP Authentication Start ####\n"));
            setHDMIRX_Event(EVENT_AUTH_START);

#ifdef Enable_HDMI_Tx
    #ifdef _ENABLE_HDCP_REPEATER_
            bEnableAuth = TRUE;
            Tx_SwitchHDCPState(TxHDCP_Off);
    #endif
#endif

#ifdef _SUPPORT_HDCP_RX_REPEATER_
            IT6607_SwitchRxHDCPState(RxHDCP_ModeCheck);
#endif
        }

#if 0
        // *** IT6607 must disable HDCP authentication Done interrupt !!! ***

        if (Reg15h & 0x02)
        {
            HDMIRX_WriteI2C_Byte(0x15, 0x02);
            setHDMIRX_Event(EVENT_AUTH_DONE);
            HDMIRX_DEBUG_PRINTF(("#### HDCP Authentication Done ####\n"));
        }
#endif
    }

    if (Reg8Bh != 0x00)
    {
        if (it6607dev.m_bOldReg8B != Reg8Bh)
        {
            HDMIRX_DEBUG_PRINTF(("123!!!!!!!!   Reg8Bh : [0x%02x] !!!!!!!!\n", Reg8Bh));
            it6607dev.m_bOldReg8B = Reg8Bh;
        }

#ifdef _IT6607_GeNPacket_Usage_
    #ifdef AUTO_SEARCH_EQ_ALGORITHM
        if (Reg8Bh & (0xC0))
        {
            HDMIRX_DEBUG_PRINTF(("---------------------------\n456,RXCK chg.\n---------------------------\n"));
            HDMIRX_DEBUG_PRINTF(("reg10 = %02X\n", (int)HDMIRX_ReadI2C_Byte(0x10)));
            AutoSearchEQPhase = TRUE;
        }
    #endif                           // AUTO_SEARCH_EQ_ALGORITHM

        if (it6607dev.m_bUpHDMIMode) // only HDMI mode process this.
        {
            if ((Reg8Bh & B_GenPKT_DET) == B_GenPKT_DET)
            {
                //HDMIRX_DEBUG_PRINTF(("Reg1Fh = 0x%02x\n",HDMIRX_ReadI2C_Byte(0x1f)));
                //HDMIRX_DEBUG_PRINTF(("Reg10h = 0x%02x\n",HDMIRX_ReadI2C_Byte(0x10)));
                //HDMIRX_DEBUG_PRINTF(("Reg19h = 0x%02x\n",HDMIRX_ReadI2C_Byte(0x19)));

                // for debug 8051 INT0 interrupt routine
                //      if(it6607dev.m_ucSCDTOffCount++>5)
                //          EX0 = 0;    // Disable ExIn0

                // UpdateVSDBPacket();
                setHDMIRX_Event(EVENT_NEW_VSI);
                uc = HDMIRX_ReadI2C_Byte(REG_RX_INTERRUPT_CTRL);      //reg19=0x02 -> 0x30
                HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL, uc | 0x02);
                HDMIRX_WriteI2C_Byte(REG_RX_INTERRUPT_CTRL, uc & 0xFD);
            }
#endif
    }
}
}
// ***************************************************************************
// Video Setup Function
// ***************************************************************************
void IT6607VideoOutput(BOOL bEnable)
{
    BYTE uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL);

    if (bEnable == TRUE)
    {
        uc &= ~(B_TRI_VIDEO);
    }
    else
    {
        uc |= (B_TRI_VIDEO);
    }

    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc);
}

// ---------------------------------------------------------------------------
void hdmirx_SetVideoOutputFormat(void)
{
    switch (it6607dev.m_bOutputVideoMode & F_MODE_CLRMOD_MASK)
    {
    case F_MODE_RGB444:
        HDMIRX_WriteI2C_Byte(REG_RX_PG_CTRL2, (B_OUTPUT_RGB24 << O_OUTPUT_COLOR_MODE));
        break;

    case F_MODE_YUV422:
        HDMIRX_WriteI2C_Byte(REG_RX_PG_CTRL2, (B_OUTPUT_YUV422 << O_OUTPUT_COLOR_MODE));
        break;

    case F_MODE_YUV444:
        HDMIRX_WriteI2C_Byte(REG_RX_PG_CTRL2, (B_OUTPUT_YUV444 << O_OUTPUT_COLOR_MODE));
        break;
    }
}

// ---------------------------------------------------------------------------
void SetVideoMute(unsigned char bMute)
{
#if 1
    unsigned char uc;

    #ifdef Debug_AVMute
    HDMIRX_WriteI2C_Byte(0x0F, 0);
    HDMIRX_DEBUG_PRINTF(("SetVideoMute(0x%02x) --Start \n", (BYTE)bMute));
    HDMIRX_DEBUG_PRINTF(("(1)reg20= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x20)));
    HDMIRX_DEBUG_PRINTF(("(1)reg3d= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x3d)));
    HDMIRX_DEBUG_PRINTF(("(1)reg89= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x89)));
    HDMIRX_DEBUG_PRINTF(("(1)reg65= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x65)));
    HDMIRX_WriteI2C_Byte(0x0F, 1);
    HDMIRX_DEBUG_PRINTF(("(1)reg7E= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x7E)));
    HDMIRX_WriteI2C_Byte(0x0F, 0);
    #endif

    if (bMute)
    {
        // implement the video gatting for video output.
        // (1) Set reg20[7][5]='1''1' for video gatting and Sync o/p enable
        HDMIRX_SetI2C_Byte(REG_RX_CSC_CTRL, (B_VDIO_GATTING | B_VIO_SEL), (B_VDIO_GATTING | B_VIO_SEL));

        // (2) Set reg3D[7,6]= '0' for RGB,'1' for YUV 422,'2' for YUV 444 [o/p color mode]
        // hdmirx_SetVideoOutputFormat();

        // (3) Set reg89[7]='0' to '1' for Change Auto AV Mute to Manual Video Gatting
        // HDMIRX_WriteI2C_Byte(0x0F,1);

        HDMIRX_WriteI2C_Byte(REG_RX_VIO_CTRL, 0x88);
        Switch_HDMIRX_Bank(1);
        HDMIRX_SetI2C_Byte(REG_BK1_TRISTATE_CTRL, 0x38, 0x38); // disable video data output.
        Switch_HDMIRX_Bank(0);

        // uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL);
        // // //HDMIRX_DEBUG_PRINTF(("(mute on) reg89= 0x%02x \n",uc));
        // // //if( it6607dev.m_VState == Vid_EnOutPut )
        // uc &= ~(B_TRI_VIDEO_SYNC);   // | B_TRI_VIDEOIO);
        // uc |= B_VDO_MUTE_DISABLE;
        // HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL,uc); //reg89[7]='1' disable auto AV mute
        // //HDMIRX_DEBUG_PRINTF(("(mute on) reg89= 0x%02x \n",HDMIRX_ReadI2C_Byte(0x89)));

    #ifdef Debug_AVMute
        HDMIRX_WriteI2C_Byte(0x0F, 0);
        HDMIRX_DEBUG_PRINTF(("(2)reg20= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x20)));
        HDMIRX_DEBUG_PRINTF(("(2)reg3d= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x3d)));
        HDMIRX_DEBUG_PRINTF(("(2)reg89= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x89)));
        HDMIRX_DEBUG_PRINTF(("(2)reg65= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x65)));
        HDMIRX_WriteI2C_Byte(0x0F, 1);
        HDMIRX_DEBUG_PRINTF(("(2)reg7E= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x7E)));
        HDMIRX_WriteI2C_Byte(0x0F, 0);
    #endif

        // IT6607SwitchAudioState(ASTATE_AudioOff);
    }
    else
    {
        HDMIRX_WriteI2C_Byte(REG_RX_VIO_CTRL, VALUE_VIO_CTRL);

        uc = HDMIRX_ReadI2C_Byte(REG_RX_VIDEO_CTRL1);
        HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_CTRL1, uc | B_565FFRST);
        HDMIRX_WriteI2C_Byte(REG_RX_VIDEO_CTRL1, uc & (~B_565FFRST));

        //20091106 Enable output sync when reg89[7]='0' Auto AV Mute enable
        uc  = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL);
        uc |= (B_VDIO_GATTING | B_VIO_SEL);
        HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc);
        uc &= ~B_VDIO_GATTING;
        HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc);

        //reg20[7][5]='1''1'enable video gating and Sync o/p enable
        //end

        HDMIRX_WriteI2C_Byte(0x0F, 0);

        // //reg89[7]='0' enable Auto AV mute
        // uc = HDMIRX_RI2C_Byte(REG_RX_TRISTATE_CTRL);
        // //HDMIRX_DEBUG_PRINTF(("(mute off) reg89= 0x%02x \n",uc));
        // uc &= ~(B_TRI_VIDEO_SYNC);           // | B_TRI_VIDEOIO);
        // uc &= ~ B_VDO_MUTE_DISABLE;
        // HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL,uc);
        // //HDMIRX_DEBUG_PRINTF(("(mute off) reg89= 0x%02x \n",HDMIRX_ReadI2C_Byte(0x89)));
        //

        // enable video io gatting
        Switch_HDMIRX_Bank(1);
        uc  = HDMIRX_ReadI2C_Byte(REG_BK1_TRISTATE_CTRL);
        uc |= 0x38;
        HDMIRX_WriteI2C_Byte(REG_BK1_TRISTATE_CTRL, uc); //reg89[7]='1' disable auto AV mute
        uc &= ~0x38;
        HDMIRX_WriteI2C_Byte(REG_BK1_TRISTATE_CTRL, uc); //reg89[7]='1' disable auto AV mute
        Switch_HDMIRX_Bank(0);
        HDMIRX_WriteI2C_Byte(0x0F, 0);

        uc  = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL);
        uc |= B_VDIO_GATTING | B_VIO_SEL;
        //reg20[7]='1' enable Video Gatting
        HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc);
        //HDMIRX_DEBUG_PRINTF(("(2)reg20= 0x%02x \n",HDMIRX_ReadI2C_Byte(0x20)));
        uc &= ~(B_VDIO_GATTING);
        //reg20[7]='0' disable Video Gatting
        HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc);
        //HDMIRX_DEBUG_PRINTF(("(2)reg20= 0x%02x \n",HDMIRX_ReadI2C_Byte(0x20)));

    #ifdef Debug_AVMute
        HDMIRX_WriteI2C_Byte(0x0F, 0);
        HDMIRX_DEBUG_PRINTF(("(3)reg20= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x20)));
        HDMIRX_DEBUG_PRINTF(("(3)reg3d= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x3d)));
        HDMIRX_DEBUG_PRINTF(("(3)reg89= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x89)));
        HDMIRX_DEBUG_PRINTF(("(3)reg65= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x65)));
        HDMIRX_WriteI2C_Byte(0x0F, 1);
        HDMIRX_DEBUG_PRINTF(("(3)reg7E= 0x%02x \n", HDMIRX_ReadI2C_Byte(0x7E)));
        HDMIRX_WriteI2C_Byte(0x0F, 0);
    #endif
    }

    it6607dev.m_bRxAVmute = bMute;

    // HDMIRX_DEBUG_PRINTF(("SetVideoMute(0x%02x) --End \n",(BYTE)bMute));
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_SetOuputSyncPolarity()
{
    BYTE tmp, uc;

    tmp  = HDMIRX_ReadI2C_Byte(REG_RX_MISC_CTRL);
    tmp &= ~(B_VSYNC_OUT_POL | B_HSYNC_OUT_POL);
    uc   = HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST);
    tmp |= (uc & 0x3);
    HDMIRX_WriteI2C_Byte(REG_RX_MISC_CTRL, tmp);
}

// ---------------------------------------------------------------------------
void hdmirx_SetVideoInputFormatWithoutInfoFrame(BYTE bInMode)
{
    BYTE i;

    i                            = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL);
    i                           |= B_FORCE_COLOR_MODE;
    it6607dev.m_bInputVideoMode &= ~F_MODE_CLRMOD_MASK;

    switch (bInMode)
    {
    case F_MODE_YUV444:
        i                           &= ~(M_INPUT_COLOR_MASK << O_INPUT_COLOR_MODE);
        i                           |= B_INPUT_YUV444 << O_INPUT_COLOR_MODE;
        it6607dev.m_bInputVideoMode |= F_MODE_YUV444;
        break;

    case F_MODE_YUV422:
        i                           &= ~(M_INPUT_COLOR_MASK << O_INPUT_COLOR_MODE);
        i                           |= B_INPUT_YUV422 << O_INPUT_COLOR_MODE;
        it6607dev.m_bInputVideoMode |= F_MODE_YUV422;
        break;

    case F_MODE_RGB444:
        i                           &= ~(M_INPUT_COLOR_MASK << O_INPUT_COLOR_MODE);
        i                           |= B_INPUT_RGB24 << O_INPUT_COLOR_MODE;
        it6607dev.m_bInputVideoMode |= F_MODE_RGB444;
        break;

    default:
        return;
    }
    HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, i);
}

// ---------------------------------------------------------------------------
void hdmirx_SetVideoInputFormatWithInfoFrame()
{
    BYTE i;
    BOOL bAVIColorModeIndicated = FALSE;
    //  BYTE bOldInputVideoMode=it6607dev.m_bInputVideoMode;

    Switch_HDMIRX_Bank(0);
    /*
       HDMIRX_DEBUG_PRINTF(("regAD = %02X reg AE = %02X\n",
        (int)HDMIRX_ReadI2C_Byte(0xAD),
        (int)HDMIRX_ReadI2C_Byte(0xAE)        )) ;*/
    i                            = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB1);
    it6607dev.m_bInputVideoMode &= ~F_MODE_CLRMOD_MASK;

    switch ((i >> O_AVI_COLOR_MODE) & M_AVI_COLOR_MASK)
    {
    case B_AVI_COLOR_YUV444:
        HDMIRX_VIDEO_PRINTF(("input YUV444 mode "));
        it6607dev.m_bInputVideoMode |= F_MODE_YUV444;
        break;

    case B_AVI_COLOR_YUV422:
        HDMIRX_VIDEO_PRINTF(("input YUV422 mode "));
        it6607dev.m_bInputVideoMode |= F_MODE_YUV422;
        break;

    case B_AVI_COLOR_RGB24:
        HDMIRX_VIDEO_PRINTF(("input RGB24 mode "));
        it6607dev.m_bInputVideoMode |= F_MODE_RGB444;
        break;

    default:
        return;
    }

    HDMIRX_DEBUG_PRINTF(("hdmirx_SetOuputSyncPolarity - RegAE=0x%02x it6607dev.m_bInputVideoMode=0x%02x\n", i, it6607dev.m_bInputVideoMode));
    i  = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL);
    i &= ~B_FORCE_COLOR_MODE;
    HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, i);
}

// ---------------------------------------------------------------------------
void hdmirx_SetColorimetryByMode(/*PSYNC_INFO pSyncInfo*/)
{
    BYTE RxClkXCNT;
    RxClkXCNT                    = getHDMIRXxCnt();

    it6607dev.m_bInputVideoMode &= ~F_MODE_ITU709;

    if (RxClkXCNT < 0x34)
    {
        it6607dev.m_bInputVideoMode |= F_MODE_ITU709;
    }
    else
    {
        it6607dev.m_bInputVideoMode &= ~F_MODE_ITU709;
    }
}

// ---------------------------------------------------------------------------
BOOL hdmirx_SetColorimetryByInfoFrame()
{
    BYTE i;
    //    BYTE bOldInputVideoMode=it6607dev.m_bInputVideoMode;

    if (it6607dev.m_NewAVIInfoFrameF)
    {
        i  = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB2);
        i &= M_AVI_CLRMET_MASK << O_AVI_CLRMET;

        if (i == (B_AVI_CLRMET_ITU601 << O_AVI_CLRMET))
        {
            it6607dev.m_bInputVideoMode &= ~F_MODE_ITU709;
            return TRUE;
        }
        else if (i == (B_AVI_CLRMET_ITU709 << O_AVI_CLRMET))
        {
            it6607dev.m_bInputVideoMode |= F_MODE_ITU709;
            return TRUE;
        }

        //        if((it6607dev.m_bInputVideoMode & F_MODE_ITU709)!=(bOldInputVideoMode & F_MODE_ITU709))
        //        {
        //
        //        }
    }

    return FALSE;
}

// ---------------------------------------------------------------------------
void hdmirx_SetColorSpaceConvert()
{
    BYTE uc, csc;
    BYTE filter = 0; // filter is for Video CTRL DN_FREE_GO,EN_DITHER,and ENUDFILT

    HDMIRX_VIDEO_PRINTF(("\n!!! hdmirx_SetColorSpaceConvert( ) !!!\n"));

    //HDMIRX_VIDEO_PRINTF(("Input mode is YUV444 "));
    switch (it6607dev.m_bOutputVideoMode & F_MODE_CLRMOD_MASK)
    {
#if defined(SUPPORT_OUTPUTYUV444)
    case F_MODE_YUV444:
        HDMIRX_VIDEO_PRINTF(("Output mode is YUV444\n"));
        switch (it6607dev.m_bInputVideoMode & F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMIRX_VIDEO_PRINTF(("Input mode is YUV444\n"));
            csc = B_CSC_BYPASS;
            break;

        case F_MODE_YUV422:
            HDMIRX_VIDEO_PRINTF(("Input mode is YUV422\n"));
            csc = B_CSC_BYPASS;
            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_UDFILT) // RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER;
            }

            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_DITHER) // RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO;
            }

            break;

        case F_MODE_RGB444:
            HDMIRX_VIDEO_PRINTF(("Input mode is RGB444\n"));
            csc = B_CSC_RGB2YUV;
            break;
        }
        break;
#endif

#if defined(SUPPORT_OUTPUTYUV422)

    case F_MODE_YUV422:
        switch (it6607dev.m_bInputVideoMode & F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMIRX_VIDEO_PRINTF(("Input mode is YUV444\n"));
            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_UDFILT)
            {
                filter |= B_RX_EN_UDFILTER;
            }
            csc = B_CSC_BYPASS;
            break;

        case F_MODE_YUV422:
            HDMIRX_VIDEO_PRINTF(("Input mode is YUV422\n"));
            csc = B_CSC_BYPASS;

            // if output is YUV422 and 16 bit or 565,then the dither is possible when
            // the input is YUV422 with 24bit input,however,the dither should be selected
            // by customer,thus the requirement should set in ROM,no need to check
            // the register value .
            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_DITHER) // RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO;
            }
            break;

        case F_MODE_RGB444:
            HDMIRX_VIDEO_PRINTF(("Input mode is RGB444\n"));
            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_UDFILT) // RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER;
            }
            csc = B_CSC_RGB2YUV;
            break;
        }
        break;
#endif

#if defined(SUPPORT_OUTPUTRGB)
    case F_MODE_RGB444:
        HDMIRX_VIDEO_PRINTF(("Output mode is RGB24\n"));
        switch (it6607dev.m_bInputVideoMode & F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMIRX_VIDEO_PRINTF(("Input mode is YUV444\n"));
            csc = B_CSC_YUV2RGB;
            break;

        case F_MODE_YUV422:
            HDMIRX_VIDEO_PRINTF(("Input mode is YUV422\n"));
            csc = B_CSC_YUV2RGB;
            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_UDFILT) // RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER;
            }
            if (it6607dev.m_bOutputVideoMode & F_MODE_EN_DITHER) // RGB24 to YUV422 need up/dn filter.
            {
                filter |= B_RX_EN_UDFILTER | B_RX_DNFREE_GO;
            }
            break;

        case F_MODE_RGB444:
            HDMIRX_VIDEO_PRINTF(("Input mode is RGB444\n"));
            csc = B_CSC_BYPASS;
            break;
        }
        break;
#endif
    }

#if defined(SUPPORT_OUTPUTYUV)
    // set the CSC associated registers
    if (csc == B_CSC_RGB2YUV)
    {
        // HDMIRX_VIDEO_PRINTF(("CSC = RGB2YUV "));
        if (it6607dev.m_bInputVideoMode & F_MODE_ITU709)
        {
            HDMIRX_VIDEO_PRINTF(("ITU709 "));

            if (it6607dev.m_bInputVideoMode & F_MODE_16_235)
            {
                HDMIRX_VIDEO_PRINTF((" 16-235\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_RGB2YUV_ITU709_16_235, SIZEOF_CSCMTX);
            }
            else
            {
                HDMIRX_VIDEO_PRINTF((" 0-255\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_RGB2YUV_ITU709_0_255, SIZEOF_CSCMTX);
            }
        }
        else
        {
            HDMIRX_VIDEO_PRINTF(("ITU601 "));
            if (it6607dev.m_bInputVideoMode & F_MODE_16_235)
            {
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_RGB2YUV_ITU601_16_235, SIZEOF_CSCMTX);
                HDMIRX_VIDEO_PRINTF((" 16-235\n"));
            }
            else
            {
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_RGB2YUV_ITU601_0_255, SIZEOF_CSCMTX);
                HDMIRX_VIDEO_PRINTF((" 0-255\n"));
            }
        }
    }
#endif
#if defined(SUPPORT_OUTPUTRGB)
    if (csc == B_CSC_YUV2RGB)
    {
        HDMIRX_VIDEO_PRINTF(("CSC = YUV2RGB "));
        if (it6607dev.m_bInputVideoMode & F_MODE_ITU709)
        {
            HDMIRX_VIDEO_PRINTF(("ITU709 "));
            if (it6607dev.m_bOutputVideoMode & F_MODE_16_235)
            {
                HDMIRX_VIDEO_PRINTF(("16-235\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_YUV2RGB_ITU709_16_235, SIZEOF_CSCMTX);
            }
            else
            {
                HDMIRX_VIDEO_PRINTF(("0-255\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_YUV2RGB_ITU709_0_255, SIZEOF_CSCMTX);
            }
        }
        else
        {
            HDMIRX_VIDEO_PRINTF(("ITU601 "));
            if (it6607dev.m_bOutputVideoMode & F_MODE_16_235)
            {
                HDMIRX_VIDEO_PRINTF(("16-235\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_YUV2RGB_ITU601_16_235, SIZEOF_CSCMTX);
            }
            else
            {
                HDMIRX_VIDEO_PRINTF(("0-255\n"));
                HDMIRX_WriteI2C_ByteN(REG_RX_CSC_YOFF, (BYTE *)bCSCMtx_YUV2RGB_ITU601_0_255, SIZEOF_CSCMTX);
            }
        }
    }
#endif     // SUPPORT_OUTPUTRGB

    uc = HDMIRX_ReadI2C_Byte(REG_RX_CSC_CTRL);
    uc = (uc & ~M_CSC_SEL_MASK) | csc;
    HDMIRX_WriteI2C_Byte(REG_RX_CSC_CTRL, uc);

    // set output Up/Down Filter,Dither control

    HDMIRX_SetI2C_Byte(REG_RX_VIDEO_CTRL1, (B_RX_DNFREE_GO | B_RX_EN_DITHER | B_RX_EN_UDFILTER), filter);

#if 0
    if (csc)
    {
        HDMIRX_DEBUG_PRINTF(("\n\rReg21="));
        for (uc = 0; uc < 3; uc++)
            HDMIRX_DEBUG_PRINTF(("%02bX,", HDMIRX_ReadI2C_Byte(0x21 + uc)));

        HDMIRX_DEBUG_PRINTF(("\n\rReg24="));
        for (uc = 0; uc < 6; uc++)
            HDMIRX_DEBUG_PRINTF(("%02bX,", HDMIRX_ReadI2C_Byte(0x24 + uc)));

        HDMIRX_DEBUG_PRINTF(("\n\rReg2A="));
        for (uc = 0; uc < 6; uc++)
            HDMIRX_DEBUG_PRINTF(("%02bX,", HDMIRX_ReadI2C_Byte(0x2A + uc)));

        HDMIRX_DEBUG_PRINTF(("\n\rReg30="));
        for (uc = 0; uc < 6; uc++)
            HDMIRX_DEBUG_PRINTF(("%02bX,", HDMIRX_ReadI2C_Byte(0x30 + uc)));
        HDMIRX_DEBUG_PRINTF(("\n\n"));
    }
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_SetDVIVideoOutput()
{
    HDMIRX_DEBUG_PRINTF(("hdmirx_SetDVIVideoOutput() \n"));
    hdmirx_SetVideoInputFormatWithoutInfoFrame(F_MODE_RGB444);

    switch (it6607dev.m_bInputVideoMode & F_MODE_CLRMOD_MASK)
    {
        it6607dev.m_bInputVideoMode  &= (~F_MODE_CLRMOD_MASK);

    case F_MODE_YUV444:
        it6607dev.m_bOutputVideoMode |= F_MODE_YUV444;
        break;

    case F_MODE_YUV422:
        it6607dev.m_bOutputVideoMode |= F_MODE_YUV444;
        break;

    case F_MODE_RGB444:
        it6607dev.m_bOutputVideoMode |= F_MODE_RGB444;
        break;
    }

    hdmirx_SetColorimetryByMode();
    hdmirx_SetColorSpaceConvert();
#ifdef AUTO_ADJUST_OUTPUT_BITS
    Switch_HDMIRX_Bank(1);
    HDMIRX_SetI2C_Byte(0x7E, 0xC0, 0xC0);  // DVI input, gatting the LL/LH output.
    Switch_HDMIRX_Bank(0);
#endif
}

// ---------------------------------------------------------------------------
void hdmirx_SetNewInfoVideoOutput()
{
    HDMIRX_DEBUG_PRINTF(("hdmirx_SetNewInfoVideoOutput() \n"));

    hdmirx_SetVideoInputFormatWithInfoFrame();

    switch (it6607dev.m_bInputVideoMode & F_MODE_CLRMOD_MASK)
    {
        it6607dev.m_bInputVideoMode  &= (~F_MODE_CLRMOD_MASK);

    case F_MODE_YUV444:
        it6607dev.m_bOutputVideoMode |= F_MODE_YUV444;
        break;

    case F_MODE_YUV422:
        it6607dev.m_bOutputVideoMode |= F_MODE_YUV444;
        break;

    case F_MODE_RGB444:
        it6607dev.m_bOutputVideoMode |= F_MODE_RGB444;
        break;
    }

    hdmirx_SetColorimetryByInfoFrame();
    hdmirx_SetColorSpaceConvert();
    it6607dev.m_NewAVIInfoFrameF = FALSE;
}

// ---------------------------------------------------------------------------

#ifdef AUTO_SEARCH_EQ_ALGORITHM
BOOL AutoSearchEQPhase = TRUE;
void AutoCSRS(BYTE eq[]);
void AutoEQPhase();

_CODE BYTE EQ_Code[] = {
    0xDF, 0x5F, 0xFF, 0x7F, 0xCF, 0x4F, 0xEF, 0x6F,
    0xC7, 0x47, 0xE7, 0x67, 0xC3, 0x43, 0xE3, 0x63,
    0xC1, 0x41, 0xE1, 0x61, 0xC0, 0x40, 0xE0, 0x60
};

_CODE EQ_ReadBackCode[] =
{
    0x83, 0x82, 0x87, 0x86,
    0x8F, 0x8E, 0x9F, 0x9E,
    0xBF, 0xBE, 0xFF, 0xFE
};
BYTE FoundEQIdx   = 0;
BYTE FoundEQStage = 0;

void AutoEQPhase()
{
    BYTE FoundEQ[6];
    BYTE InitPhase[3], NewPhase[3], Phase[3];
    BYTE diff;

    int i, j, newEQ;

    HDMIRX_MISC_PRINTF(("reg10 = %02X\n", (int)HDMIRX_ReadI2C_Byte(0x10)));

    HDMIRX_SetI2C_Byte(0xF, 1, 0);
    HDMIRX_WriteI2C_Byte(0x93, 0xC7);
    HDMIRX_WriteI2C_Byte(0x94, 0xC7);
    HDMIRX_WriteI2C_Byte(0x95, 0xC7);

    HDMIRX_WriteI2C_Byte(0x6B, 0x00);
    HDMIRX_WriteI2C_Byte(0x96, 0x9);
    HDMIRX_WriteI2C_Byte(0x6E, 0x42);

    HDMIRX_WriteI2C_Byte(0x3B, 0x8C);
    HDMIRX_WriteI2C_Byte(0x3B, 0xAC);

    HDMIRX_WriteI2C_Byte(0x67, 0x00); InitPhase[0] = HDMIRX_ReadI2C_Byte(0x37);
    HDMIRX_WriteI2C_Byte(0x67, 0x02); InitPhase[1] = HDMIRX_ReadI2C_Byte(0x37);
    HDMIRX_WriteI2C_Byte(0x67, 0x03); InitPhase[2] = HDMIRX_ReadI2C_Byte(0x37);

    HDMIRX_MISC_PRINTF(("Init EQ Phase = %02X %02X %02X\n", (int)InitPhase[0], (int)InitPhase[1], (int)InitPhase[2]));

    AutoCSRS(FoundEQ);

    HDMIRX_MISC_PRINTF(("Found EQ =      %02X %02X %02X\n", (int)FoundEQ[0], (int)FoundEQ[1], (int)FoundEQ[2]));
    HDMIRX_MISC_PRINTF(("/////////////////////////////////////////////////////////////////\n"));

    for (i = 0; i < 3; i++)
    {
        NewPhase[i]  = ((InitPhase[i] & 0x7F) + 0x40) & 0xFF;
        NewPhase[i] |= 0x80;
    }
    HDMIRX_MISC_PRINTF(("New EQ Phase = %02X %02X %02X\n", (int)NewPhase[0], (int)NewPhase[1], (int)NewPhase[2]));
    for (i = 0; i < 100; i++)
    {
        HDMIRX_WriteI2C_Byte(0x6F, NewPhase[0]);
        HDMIRX_WriteI2C_Byte(0x3B, 0x8C);
        HDMIRX_WriteI2C_Byte(0x3B, 0xAC);
        HDMIRX_WriteI2C_Byte(0x6F, 0);  // for auto search Phase;

        HDMIRX_WriteI2C_Byte(0x67, 0x00);
        Phase[0] = HDMIRX_ReadI2C_Byte(0x37);
        diff     = (Phase[0] - NewPhase[0]) & 0xFF;
        diff    |= (diff & 0x40) << 1; // sign extension;
        HDMIRX_MISC_PRINTF(("get phase[0] %02X from %02X on %d loop\n", (int)Phase[0], (int)NewPhase[0], i));
        if (diff > 0xF0 || diff < 0x10)
        {
            break;
        }
    }
    for (i = 0; i < 100; i++)
    {
        HDMIRX_WriteI2C_Byte(0x70, NewPhase[1]);
        HDMIRX_WriteI2C_Byte(0x3B, 0x8C);
        HDMIRX_WriteI2C_Byte(0x3B, 0xAC);
        HDMIRX_WriteI2C_Byte(0x70, 0);  // for auto search Phase;

        HDMIRX_WriteI2C_Byte(0x67, 0x02);
        Phase[1] = HDMIRX_ReadI2C_Byte(0x37);
        diff     = (Phase[1] - NewPhase[1]) & 0xFF;
        diff    |= (diff & 0x40) << 1; // sign extension;
        HDMIRX_MISC_PRINTF(("get phase[0] %02X from %02X on %d loop\n", (int)Phase[1], (int)NewPhase[1], i));
        if (diff > 0xF0 || diff < 0x10)
        {
            break;
        }
    }
    for (i = 0; i < 100; i++)
    {
        HDMIRX_WriteI2C_Byte(0x71, NewPhase[2]);
        HDMIRX_WriteI2C_Byte(0x3B, 0x8C);
        HDMIRX_WriteI2C_Byte(0x3B, 0xAC);
        HDMIRX_WriteI2C_Byte(0x71, 0);  // for auto search Phase;

        HDMIRX_WriteI2C_Byte(0x67, 0x03);
        Phase[2] = HDMIRX_ReadI2C_Byte(0x37);
        diff     = (Phase[2] - NewPhase[2]) & 0xFF;
        diff    |= (diff & 0x40) << 1; // sign extension;
        HDMIRX_MISC_PRINTF(("get phase[2] %02X from %02X on %d loop\n", (int)Phase[2], (int)NewPhase[2], i));
        if (diff > 0xF0 || diff < 0x10)
        {
            break;
        }
    }

    AutoCSRS(FoundEQ + 3);

    for (i = 0; i < 6; i++)
    {
        HDMIRX_MISC_PRINTF(("EQ[%d] = %02X ", i, (int)FoundEQ[i]));
    }
    printf("\n");
    for (i = 0, newEQ = 0; i < 6; i++)
    {
        for (j = 0; j < sizeof(EQ_Code); j++)
        {
            if (FoundEQ[i] == EQ_Code[j])
            {
                HDMIRX_MISC_PRINTF(("newEQ(%d) += %d\n", (int)newEQ, j));
                newEQ += j;
                break;
            }
        }
    }
    newEQ += 3; newEQ /= 6;
    newEQ &= 0xFE;   HDMIRX_MISC_PRINTF(("newEQ = %d,%02X\n", (int)newEQ, (int)EQ_Code[newEQ]));
    for (i = 0; i < 3; i++)
    {
        HDMIRX_WriteI2C_Byte(0x93 + i, EQ_Code[newEQ]);
    }
    FoundEQIdx = newEQ;
    HDMIRX_MISC_PRINTF(("FoundEQIdx = %d %d\n", (int)FoundEQIdx, (int)FoundEQStage));

    HDMIRX_WriteI2C_Byte(0x96, 0x0a);
    HDMIRX_WriteI2C_Byte(0x6b, 0x1d);
    // 2011/12/22 by Dr. Liu
    // Please add the following code:
    // 1.    After set [6B]=1D
    // Set [0C]=40
    // Set [0C]=0
    HDMIRX_SetI2C_Byte(0x0c, 0x40, 0x40);
    HDMIRX_SetI2C_Byte(0x0c, 0x40, 0x00);
    return;
    // Than
    // 2. IF SCDT is not stable in 100ms
    // Set new EQ = EQ_find – 6
    delay1ms(100);
    diff = HDMIRX_ReadI2C_Byte(0x10);
    if (diff & B_SCDT)
    {
        return;
    }
    newEQ = (FoundEQIdx > 6) ? (FoundEQIdx - 6) : 0;
    for (i = 0; i < 3; i++)
    {
        HDMIRX_WriteI2C_Byte(0x93 + i, EQ_Code[newEQ]);
    }

    HDMIRX_MISC_PRINTF(("Cound not stable in 100ms,adjust EQidx from %d(%02X) to %d(%02X)\n", (int)FoundEQIdx, (int)EQ_Code[FoundEQIdx], (int)newEQ, (int)EQ_Code[newEQ]));
    delay1ms(100);
    diff = HDMIRX_ReadI2C_Byte(0x10);
    if (diff & B_SCDT)
    {
        return;
    }
    newEQ = (FoundEQIdx < 18) ? (FoundEQIdx + 6) : 23;
    for (i = 0; i < 3; i++)
    {
        HDMIRX_WriteI2C_Byte(0x93 + i, EQ_Code[newEQ]);
    }

    HDMIRX_MISC_PRINTF(("Cound not stable in 100ms,adjust EQidx from %d(%02X) to %d(%02X)\n", (int)FoundEQIdx, (int)EQ_Code[FoundEQIdx], (int)newEQ, (int)EQ_Code[newEQ]));

    // IF SCDT is not stable in 100ms
    //  Set new EQ= EQ_find + 6
    //        IF SCDT is not stable in 100ms
    //            Set [93][94][95]=C7
    //                  END
    //
    delay1ms(100);
    diff = HDMIRX_ReadI2C_Byte(0x10);
    if (diff & B_SCDT)
    {
        return;
    }
    for (i = 0; i < 3; i++)
    {
        HDMIRX_WriteI2C_Byte(0x93 + i, 0xC7);
    }

    HDMIRX_MISC_PRINTF(("Cound not stable in 100ms,adjust EQidx from %d(%02X) to 0xC7\n", (int)FoundEQIdx, (int)EQ_Code[FoundEQIdx], (int)newEQ, (int)EQ_Code[newEQ]));
}

_CODE BYTE EQ_CHANNEL[] = { 0x00, 0x02, 0x03 };
void AutoCSRS(BYTE eq[])
{
    BYTE xdata countCSRS[2][12];
    int i, j, channel;
    BYTE uc, maxCSRS07, maxCSRS00;

    delay1ms(1);
    HDMIRX_SetI2C_Byte(0xF, 1, 0);

    for (channel = 0; channel < 3; channel++)
    {
        for (i = 0; i < 12; i++)
        {
            countCSRS[0][i] = 0;
            countCSRS[1][i] = 0;
        }
        // Find Max EQ for channel 0;
        HDMIRX_SetI2C_Byte(0x93 + channel, 0xC0, 0xC0);
        HDMIRX_WriteI2C_Byte(0x67, EQ_CHANNEL[channel]);
        //HDMIRX_SetI2C_Byte(0x93+channel,0x80,0x80);
        HDMIRX_MISC_PRINTF(("reg%02X = %02X,reg67 = %02X\n", (int)(0x93 + channel), (int)HDMIRX_ReadI2C_Byte(0x93 + channel), (int)HDMIRX_ReadI2C_Byte(0x67)));
        HDMIRX_WriteI2C_Byte(0x6C, 0x00);

        for (i = 0; i < 100; i++)
        {
            uc = HDMIRX_ReadI2C_Byte(0x98);
            HDMIRX_MISC_PRINTF(("%02X(0) ", (int)uc));
            for (j = 0; j < 12; j++)
            {
                if (uc == EQ_ReadBackCode[j])
                {
                    //HDMIRX_MISC_PRINTF(("uc = %02X,j = %d ",(int)uc,(int)j)) ;
                    countCSRS[0][j]++;
                    //HDMIRX_MISC_PRINTF(("countCSRS[%d] = %d\n",(int)j,(int)countCSRS[0][j])) ;
                }
            }
        }
        //HDMIRX_SetI2C_Byte(0x93+channel,0x80,0x00); HDMIRX_MISC_PRINTF(("\n")) ;
        HDMIRX_WriteI2C_Byte(0x6C, 0x07);
        for (i = 0; i < 100; i++)
        {
            uc = HDMIRX_ReadI2C_Byte(0x98);
            HDMIRX_MISC_PRINTF(("%02X(1) ", (int)uc));
            for (j = 0; j < 12; j++)
            {
                if (uc == EQ_ReadBackCode[j])
                {
                    //HDMIRX_MISC_PRINTF(("uc = %02X,j = %d ",(int)uc,(int)j)) ;
                    countCSRS[1][j]++;
                    //HDMIRX_MISC_PRINTF(("countCSRS[%d] = %d\n",(int)j,(int)countCSRS[1][j])) ;
                }
            }
        }
        HDMIRX_MISC_PRINTF(("\n"));
        maxCSRS00 = 0; uc = 0;
        for (i = 0; i < 12; i++)
        {
            if ((countCSRS[0][i]) > uc)
            {
                HDMIRX_MISC_PRINTF(("countCSRS[0][%d] = %d\n", (int)i, (int)countCSRS[0][i]));
                HDMIRX_MISC_PRINTF(("EQ = %02X -> %02X\n", (int)EQ_ReadBackCode[maxCSRS00], (int)EQ_ReadBackCode[i]));
                maxCSRS00 = i;
                uc        = countCSRS[0][i];
            }
        }
        maxCSRS07 = 0; uc = 0;
        for (i = 0; i < 12; i++)
        {
            if ((countCSRS[1][i]) > uc)
            {
                HDMIRX_MISC_PRINTF(("countCSRS[1][%d] = %d\n", (int)i, (int)countCSRS[1][i]));
                HDMIRX_MISC_PRINTF(("EQ = %02X -> %02X\n", (int)EQ_ReadBackCode[maxCSRS00], (int)EQ_ReadBackCode[i]));
                maxCSRS07 = i;
                uc        = countCSRS[1][i];
            }
        }
        printf("maxCSRS idx = %d(%02X) under reg6C = 0x00,maxCSRS idx = %d(%02X) under reg6C = 0x07\n",
               (int)maxCSRS00, (int)EQ_ReadBackCode[maxCSRS00], (int)maxCSRS07, (int)EQ_ReadBackCode[maxCSRS07]);
        i            = (maxCSRS07 > maxCSRS00) ? maxCSRS07 : maxCSRS00;

        uc           = EQ_ReadBackCode[i];
        HDMIRX_MISC_PRINTF((" EQ_ReadBackCode[%d] = %02X,", (int)i, (int)uc));
        eq[channel]  = ((uc & 0x7C) >> 2); HDMIRX_MISC_PRINTF(("eq[%d] = %02X ->", (int)channel, (int)eq[channel]));
        eq[channel] |= ((uc & 0x3) << 5); HDMIRX_MISC_PRINTF((" %02X ->", (int)eq[channel]));
        // 2011/12/21 modified by jau-chih.tseng@ite.com.tw
        // By Dr. Liu,when CSRS COUNT equal on EHAI as '0' and '1',choose '1'
        eq[channel] |= 0x80;

        HDMIRX_SetI2C_Byte(0x93 + channel, 0x00, 0x40);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HDMIRX_WriteI2C_Byte(0x6C, 0x03);
    HDMIRX_MISC_PRINTF(("eq[0] = %02X eq[1] = %02X eq[2] = %02X \n", (int)eq[0], (int)eq[1], (int)eq[2]));
}

#endif // AUTO_SEARCH_EQ_ALGORITHM

// ------------------------------------------------------------------------------------------
void IT6607SwitchVideoState(Video_State_Type eNewVState)
{
    BYTE uc;

    if (it6607dev.m_VState == eNewVState)
        return;

    if (it6607dev.m_VState == VSTATE_SWReset && eNewVState != VSTATE_PwrOff)
    {
        return;
    }
    //HDMIRX_VIDEO_PRINTF(("[%06u] +++ ",ucTickCount));
    HDMIRX_VIDEO_PRINTF((VStateStr[(BYTE)eNewVState]));
    HDMIRX_VIDEO_PRINTF(("+++ \n"));

    it6607dev.m_VState             = eNewVState;
    it6607dev.m_VideoCountingTimer = GetCurrentVirtualTime();  // get current time tick, and the next tick judge in the polling handler.

    switch (it6607dev.m_VState)
    {
    case VSTATE_SWReset:
        // 1. Power 5V off interrupt
        // 2. HPD off event
        // 3. SW reset
        // 4. change input port
        //HPD and Termination need 500ms Low for compliance issue
#ifndef Enable_HDMI_Tx
        //HotPlug(HPDOFF);
#endif
        IT6607VideoOutput(OFF);
        hdmirx_Terminator_Off();

        uc  = HDMIRX_ReadI2C_Byte(REG_RX_RST_CTRL);
#ifdef Auto_Reset
        uc |= (B_SWRST | B_EN_AUTOVDORST | B_HDCPRST | B_AUDRST);
#else
        uc |= (B_SWRST | B_HDCPRST | B_AUDRST);
#endif

        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, uc);
#ifdef AUTO_SEARCH_EQ_ALGORITHM
        AutoSearchEQPhase = TRUE;
#endif     // AUTO_SEARCH_EQ_ALGORITHM

        HDMIRX_WriteI2C_Byte(0x13, 0xff);
        HDMIRX_WriteI2C_Byte(0x14, 0xff);
        HDMIRX_WriteI2C_Byte(0x15, 0xff);

        it6607dev.m_ucVideoOnCount    = 0;
        it6607dev.m_ucSCDTOffCount    = 0;
        it6607dev.m_ucVideoModeChange = 0;
        it6607dev.m_NewAVIInfoFrameF  = FALSE;
        it6607dev.m_NewAUDInfoFrameF  = FALSE;
        it6607dev.m_ucVideoModeChange = 0;

#ifdef _SUPPORT_HDCP_RX_REPEATER_
        IT6607_SwitchRxHDCPState(RxHDCP_PwrOff);
#endif
        // CreatTimerTask(eVideoCountingTimer,SWReset_COUNT,SysTimerTask_OnceRun);
        // AssignVideoTimerTimeout(SWReset_COUNT) ;
        /*
           HDMIRX_DEBUG_PRINTF((
            "##############################################################################"
            "reg93~95 = %02X %02X %02X after Video ON;\n"
            "##############################################################################\n",
            (int)HDMIRX_ReadI2C_Byte(0x93),
            (int)HDMIRX_ReadI2C_Byte(0x94),
            (int)HDMIRX_ReadI2C_Byte(0x95)));
         */
        break;

    case VSTATE_PwrOff:
#ifdef AUTO_SEARCH_EQ_ALGORITHM
        AutoSearchEQPhase = TRUE;
#endif     // AUTO_SEARCH_EQ_ALGORITHM

#ifdef Auto_Reset
        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_EN_AUTOVDORST | B_AUDRST);
#else
        HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, B_AUDRST);
#endif
        hdmirx_Terminator_On();
#ifndef _AllionComplianceTest_
        //HotPlug(HPDON);
#endif

        hdmirx_SetVideoOutputFormat();

        break;

    case VSTATE_SyncWait:
        // 1. SCDT off interrupt
        // 2. VideoMode_Chg interrupt
        //  HDMIRX_WriteI2C_Byte(0x0f,0x00);
        //  HDMIRX_WriteI2C_Byte(0x06,0x00);
        HDMIRX_WriteI2C_Byte(0x96, 0x0A);
#ifndef Auto_Reset
        HDMIRX_WriteI2C_Byte(0x97, 0x00);
#endif
        IT6607VideoOutput(OFF);
        it6607dev.m_ucSCDTOffCount++;
        it6607dev.m_NewAVIInfoFrameF  = FALSE;
        it6607dev.m_NewAUDInfoFrameF  = FALSE;
        it6607dev.m_ucVideoModeChange = 0;
#ifdef _IT6607_GeNPacket_Usage_
        // ClearGeneralRecPackType();
#endif
        break;

    case VSTATE_SyncChecking:
        // 1. SCDT on interrupt
        // CreatTimerTask(eVideoCountingTimer,VSATE_CONFIRM_SCDT_COUNT,SysTimerTask_OnceRun);

#ifdef _IT6607_GeNPacket_Usage_
        // Switch_GeneralRecPackState(1);
#endif

#ifdef AUTO_SEARCH_EQ_ALGORITHM
        if (AutoSearchEQPhase == TRUE)
        {
            if ((HDMIRX_ReadI2C_Byte(0x10) & 0xA2) == 0xA2)
            {
                AutoEQPhase();
                AutoSearchEQPhase = FALSE;
                HDMIRX_DEBUG_PRINTF((
                                        "##############################################################################"
                                        "reg93~95 = %02X %02X %02X after AutoSearchEQPhase() in VSTATE_SyncChecking;\n"
                                        "##############################################################################\n"
                                        , (int)HDMIRX_ReadI2C_Byte(0x93), (int)HDMIRX_ReadI2C_Byte(0x94), (int)HDMIRX_ReadI2C_Byte(0x95)));
            }
        }
#endif         // AUTO_SEARCH_EQ_ALGORITHM
        break;

    case VSTATE_VideoOn:

        it6607dev.m_ucEccCount  = 0;
        it6607dev.m_ucVideoOnCount++;

        it6607dev.m_bUpHDMIMode = HDMIRX_IsHDMIMode();

        //--- setup IT6607 output format  ---//

        if (it6607dev.m_bUpHDMIMode == FALSE)
        {
            hdmirx_SetDVIVideoOutput();
            IT6607SwitchAudioState(ASTATE_AudioOff);
            SetVideoMute(OFF);
        }
        else
        {
            hdmirx_SetNewInfoVideoOutput();
            it6607dev.m_NewAVIInfoFrameF = FALSE;

            //it6607dev.m_ucHDMIAudioErrorCount=0;

            uc  = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL);
            uc &= ~B_FORCE_FS;
            HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, uc);

            IT6607SwitchAudioState(ASTATE_RequestAudio);

#ifndef DISABLE_PIXEL_REPEAT_REMOVE
            uc = (HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST) & 0xF0) ? 0x00 : 0x20; // if pixel repetition ?
#else
            uc = 0x20;
#endif
            HDMIRX_SetI2C_Byte(REG_RX_VCLK_CTRL, 0x20, uc);

            it6607dev.m_MuteByPKG = (HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST) & B_AVMUTE) ? TRUE : FALSE;
            SetVideoMute(it6607dev.m_MuteByPKG);
#ifdef AUTO_ADJUST_OUTPUT_BITS
            uc                    = HDMIRX_ReadI2C_Byte(REG_RX_FS);
            Switch_HDMIRX_Bank(1);
            switch (uc & 0x70)
            {
            case 0x60:
                HDMIRX_SetI2C_Byte(0x7E, 0xC0, 0x00); break;   // input is 36 bit
            case 0x50:
                HDMIRX_SetI2C_Byte(0x7E, 0xC0, 0x80); break;   // input is 30 bit
            default:
                HDMIRX_SetI2C_Byte(0x7E, 0xC0, 0xC0); break;   // input is 24 bit
            }
            Switch_HDMIRX_Bank(0);
#endif
        }

        hdmirx_SetOuputSyncPolarity();

        IT6607VideoOutput(ON);

        HDMIRX_DEBUG_PRINTF((
                                "##############################################################################"
                                "reg93~95 = %02X %02X %02X after Video ON;\n"
                                "##############################################################################\n",
                                (int)HDMIRX_ReadI2C_Byte(0x93),
                                (int)HDMIRX_ReadI2C_Byte(0x94),
                                (int)HDMIRX_ReadI2C_Byte(0x95)));

        break;
    }
}

// ---------------------------------------------------------------------------
void IT6607VideoHandler(void)
{
    BYTE sys_stat;

#if 0
    if (it6607dev.m_VState == VSTATE_SyncWait || it6607dev.m_VState == VSTATE_SyncChecking)
    {
        //if(it6607dev.m_SWResetTimeOut==0)
        if (TimeOutCheck(eSWResetTimeOut) == TRUE)
        {
            HDMIRX_DEBUG_PRINTF(("!!! it6607dev.m_SWResetTimeOut time out  !!! VideoOnCount = 0x%02x !!! SCDTOffCount = 0x%02x\n", it6607dev.m_ucVideoOnCount, it6607dev.m_ucSCDTOffCount));
            if ((it6607dev.m_ucVideoOnCount == 0) || (it6607dev.m_ucSCDTOffCount > 5))
                SWReset_HDMIRX();

            return;
        }
    }
#endif

    switch (it6607dev.m_VState)
    {
    case VSTATE_SWReset:
        {
            if (VideoTimeOutCheck(SWReset_COUNT))
            {
                //HPD and Termination need 500ms Low for compliance issue
                IT6607SwitchVideoState(VSTATE_PwrOff);
            }
        }
        break;

    case VSTATE_PwrOff:
        if (HDMIRX_IsPlg5VPwr())
        {
            AssignSWResetVirtualTime();
            IT6607SwitchVideoState(VSTATE_SyncWait);
        }
        break;

    case VSTATE_SyncWait:
        if (!HDMIRX_IsPlg5VPwr())
        {
            IT6607SwitchVideoState(VSTATE_SWReset);
        }

        if (HDMIRX_IsSCDT())
        {
            IT6607SwitchVideoState(VSTATE_SyncChecking);
        }

#ifdef AUTO_SEARCH_EQ_ALGORITHM
        if (AutoSearchEQPhase == TRUE)
        {
            if ((HDMIRX_ReadI2C_Byte(0x10) & 0xA2) == 0xA2)
            {
                AutoEQPhase();
                AutoSearchEQPhase = FALSE;
                HDMIRX_DEBUG_PRINTF((
                                        "##############################################################################"
                                        "reg93~95 = %02X %02X %02X after AutoSearchEQPhase() in VideoHandler;\n"
                                        "##############################################################################\n",

                                        (int)HDMIRX_ReadI2C_Byte(0x93), (int)HDMIRX_ReadI2C_Byte(0x94), (int)HDMIRX_ReadI2C_Byte(0x95)));
            }
        }
        else
        {
            if ((HDMIRX_ReadI2C_Byte(0x10) & 0xA0) != 0xA0)
            {
                AutoSearchEQPhase = TRUE;
            }
        }
#endif         // AUTO_SEARCH_EQ_ALGORITHM

        if (VideoTimeOutCheck(VSTATE_MISS_SYNC_COUNT))
        {
            HDMIRX_DEBUG_PRINTF(("------------SyncWait time out -----------\n"));
            SWReset_HDMIRX();
            return;
        }
        break;

    case VSTATE_SyncChecking:

        if (!HDMIRX_IsPlg5VPwr())
        {
            IT6607SwitchVideoState(VSTATE_SWReset);
        }

        if (!HDMIRX_IsSCDT())
        {
            IT6607SwitchVideoState(VSTATE_SyncWait);
        }

        //Delay 400ms for ignore Video mode change interrupt !!!
        if (VideoTimeOutCheck(VSATE_CONFIRM_SCDT_COUNT))
        {
            IT6607SwitchVideoState(VSTATE_VideoOn);
        }
        break;

    case VSTATE_VideoOn:

        if (!HDMIRX_IsPlg5VPwr())
        {
            IT6607SwitchVideoState(VSTATE_SWReset);
        }

        if (!HDMIRX_IsSCDT())
        {
            IT6607SwitchVideoState(VSTATE_SyncWait);
        }

        /*
           if(it6607dev.m_NewAVIInfoFrameF==TRUE)
           {
            if((it6607dev.m_RxHDCPState!=RxHDCP_ModeCheck) ){
                hdmirx_SetNewInfoVideoOutput();
                it6607dev.m_NewAVIInfoFrameF=FALSE;
            }

           }

           if(it6607dev.m_bRxAVmute!=FALSE) {
            if((it6607dev.m_HDCPRepeater==FALSE) || (it6607dev.m_RxHDCPState==RxHDCP_Authenticated)) {
                if(TimeOutCheck(eAVmute)==TRUE){
                    SetVideoMute(FALSE);
                }
            }
           }
         */

        /*
           if(TimeOutCheck(eVideoCountingTimer)==TRUE) {
                    dump_vidmode();
            if( HDMIRX_IsHDMIMode() ) dump_InfoFrame();
                    dump_audSts();
           }
         */

        //#ifdef _IT6607_GeNPacket_Usage_
        //UpdateVSDBPacket();
        //#endif

        break;
    }
}

// ***************************************************************************
// Video information function
// ***************************************************************************
BOOL HDMIRX_IsPlg5VPwr()
{
    BYTE sys_stat;
    // BYTE uc;
    sys_stat = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATUS);

    if (it6607dev.m_ucCurrentHDMIPort != CAT_HDMI_PORTA)
    {
        return (sys_stat & B_PWR5V_DET_PORTB) ? TRUE : FALSE;
    }
    else
    {
        return (sys_stat & B_PWR5V_DET_PORTA) ? TRUE : FALSE;
    }
}

// ---------------------------------------------------------------------------
BOOL HDMIRX_IsHDMIMode()
{
    BOOL HDMI_mode;

    HDMI_mode = ((HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATUS) & B_HDMIRX_MODE) ? TRUE : FALSE);

    return HDMI_mode;
}

// ---------------------------------------------------------------------------
BOOL HDMIRX_IsInterlace()
{
    BYTE data = HDMIRX_ReadI2C_Byte(REG_RX_VID_MODE);
    return !!(data & B_INTERLACE);
}

// ---------------------------------------------------------------------------
BOOL HDMIRX_IsHDCP()
{
    BYTE uc;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_HDCP_STATUS);
    if (uc & B_HDCP_MODE)
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_VID_INPUT_ST);
        return (uc & B_RX_DIS_HDCP) ? FALSE : TRUE;
    }

    return FALSE;
}

// ---------------------------------------------------------------------------
//#define MASK_SCDT_RDY  (B_RXPLL_LOCK|B_RXCK_VALID|B_SCDT|B_VCLK_DET)
#define MASK_SCDT_RDY (B_RXCK_VALID | B_SCDT | B_VCLK_DET) //mylin for 27Mhz self test
BOOL HDMIRX_IsSCDT()
{
    BYTE uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATUS) & MASK_SCDT_RDY;
    return (uc == MASK_SCDT_RDY) ? TRUE : FALSE;
}

BOOL HDMIRX_IsVideoStable()
{
    //  BYTE uc;

    return HDMIRX_IsSCDT();
}

void HDMIRX_SelectHDMIPort(BYTE ucPort)
{
    BYTE uc;
    BYTE PortBOn;
    BYTE PortAOn;

    //  HDMIRX_MISC_PRINTF(("HDMIRX_SelectHDMIPort ()    \n"));

    uc      = HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATUS);
    PortAOn = (uc & B_PWR5V_DET_PORTA);
    PortBOn = (uc & B_PWR5V_DET_PORTB) >> 3;
    uc      = HDMIRX_ReadI2C_Byte(REG_RX_PWD_CTRL1);

    if (ucPort != CAT_HDMI_PORTB)
    {
        ucPort = CAT_HDMI_PORTA;
        uc    &= (~B_PORT_SEL_B);
    }
    else
    {
        uc |= B_PORT_SEL_B;
    }
    HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, uc);

    if (it6607dev.m_ucCurrentHDMIPort != ucPort)
    {
        HDMIRX_MISC_PRINTF(("\n =============== HDMIRX_SelectHDMIPort ucPort=0x%02x ============== \n", (BYTE)ucPort));
        HDMIRX_MISC_PRINTF(("it6607dev.m_ucCurrentHDMIPort=0x%02x,reg10=0x%02x,ucport=0x%02x \n", it6607dev.m_ucCurrentHDMIPort, HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATUS), ucPort));

        it6607dev.m_ucCurrentHDMIPort = ucPort;

        IT6607SwitchVideoState(VSTATE_SWReset);
#ifdef Enable_HDMI_Tx
        ResetTxVideo(); //2011/02/18 test for green image issue
#endif
    }
}

BYTE getHDMIRXCurrentPort()
{
    return it6607dev.m_ucCurrentHDMIPort;
}

BYTE getHDMIRXPortPlug()
{
    return HDMIRX_ReadI2C_Byte(REG_RX_SYS_STATUS);
}

BOOL getHDMIRX_Event(unsigned int EventMask)
{
    BYTE retv;
    retv                    = (it6607dev.m_EventFlags & EventMask) ? TRUE : FALSE;
    it6607dev.m_EventFlags &= ~EventMask;
    return retv ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
WORD getHDMIRXHorzTotal()
{
    BYTE uc[2];
    WORD hTotal;

    uc[0]    = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_L); //reg59[7:0]
    uc[1]    = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_H); //reg5A[3:0]
    hTotal   = (WORD)(uc[1] & 0x0F);
    hTotal <<= 8;
    hTotal  |= (WORD)uc[0];

    return hTotal;  //hTotal[11:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXHorzActive()
{
    BYTE uc[2];

    WORD hActive;

    uc[0]     = HDMIRX_ReadI2C_Byte(REG_RX_VID_HACT_L);   //reg5B[7:0]
    uc[1]     = HDMIRX_ReadI2C_Byte(REG_RX_VID_HTOTAL_H); //reg5A[7:4]
    hActive   = (WORD)(uc[1] & 0xF0);
    hActive <<= 4;
    hActive  |= (WORD)uc[0];

    return hActive; //hActive[11:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXHorzFrontPorch()
{
    BYTE uc[2];
    WORD hFrontPorch;

    uc[0]         = HDMIRX_ReadI2C_Byte(REG_RX_VID_H_FT_PORCH_L); //reg5E[7:0]
    uc[1]         = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_H);  //reg5D[7:4]
    hFrontPorch   = (WORD)(uc[1] & 0xF0);
    hFrontPorch <<= 4;
    hFrontPorch  |= (WORD)uc[0];

    return hFrontPorch; //hFrontPorch[11:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXHorzSyncWidth()
{
    BYTE uc[2];
    WORD hSyncWidth;

    uc[0]        = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_L); //reg5C[7:0]
    uc[1]        = HDMIRX_ReadI2C_Byte(REG_RX_VID_HSYNC_WID_H); //reg5D[0]

    hSyncWidth   = (WORD)(uc[1] & 0x01);
    hSyncWidth <<= 8;
    hSyncWidth  |= (WORD)uc[0];

    return hSyncWidth;  //syncWidth[8:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXHorzBackPorch()
{
    WORD hBackPorch;

    hBackPorch = getHDMIRXHorzTotal() - getHDMIRXHorzActive() - getHDMIRXHorzFrontPorch() - getHDMIRXHorzSyncWidth();

    return hBackPorch;
}

// ---------------------------------------------------------------------------
WORD getHDMIRXVertTotal()
{
    BYTE uc[2];
    WORD vTotal;
    uc[0]    = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_L); //reg5F[7:0]
    uc[1]    = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_H); //reg60[3:0]

    vTotal   = (WORD)(uc[1] & 0x0F);
    vTotal <<= 8;
    vTotal  |= (WORD)uc[0];

    return vTotal;  //vTotal[11:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXVertActive()
{
    BYTE uc[2];
    WORD vActive;

    uc[0]     = HDMIRX_ReadI2C_Byte(REG_RX_VID_VACT_L);   //reg61[7:0]
    uc[1]     = HDMIRX_ReadI2C_Byte(REG_RX_VID_VTOTAL_H); //reg60[7:4]

    vActive   = (WORD)(uc[1] & 0xF0);
    vActive <<= 4;
    vActive  |= (WORD)uc[0];

    return vActive; //vActive[11:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXVertFrontPorch()
{
    WORD vFrontPorch;

    vFrontPorch = (WORD)(HDMIRX_ReadI2C_Byte(REG_RX_VID_V_FT_PORCH) & 0x3F); //reg63[5:0]

    return vFrontPorch;                                                      //vFrontPorch[5:0]
}

WORD getHDMIRXVertSyncWidth()
{
    WORD vSyncWidth;
    vSyncWidth = (WORD)(HDMIRX_ReadI2C_Byte(REG_RX_VID_VSYNC2DE) & 0x1F); //Reg62[4:0]
    return vSyncWidth;                                                    //vSyncWidth[4:0]
}

// ---------------------------------------------------------------------------
WORD getHDMIRXVertSyncBackPorch()
{
    WORD vBackPorch;
    vBackPorch = getHDMIRXVertTotal() - getHDMIRXVertActive() - getHDMIRXVertFrontPorch() - getHDMIRXVertSyncWidth();
    return vBackPorch;
}

// ---------------------------------------------------------------------------
BYTE getHDMIRXxCnt()
{
    return HDMIRX_ReadI2C_Byte(REG_RX_VID_XTALCNT_128PEL);
}

// ---------------------------------------------------------------------------
BYTE getHDMIRXOutputColorDepth()
{
    BYTE uc;
    uc = HDMIRX_ReadI2C_Byte(REG_RX_FS) & 0xF0;  //Reg84[7:4]
    return (uc >> 4);
}

// ***************************************************************************
// Audio function
// ***************************************************************************

// ---------------------------------------------------------------------------
void hdmirx_SetHWMuteClrMode()
{
    hdmirx_SetHWMuteCTRL((~B_HW_AUDMUTE_CLR_MODE), (B_HW_AUDMUTE_CLR_MODE));
}

// ---------------------------------------------------------------------------
void hdmirx_SetHWMuteClr()
{
    hdmirx_SetHWMuteCTRL((~B_HW_MUTE_CLR), (B_HW_MUTE_CLR));
}

// ---------------------------------------------------------------------------
void hdmirx_ClearHWMuteClr()
{
    hdmirx_SetHWMuteCTRL((~B_HW_MUTE_CLR), 0);
}

// ---------------------------------------------------------------------------
void hdmirx_SetHWMuteCTRL(BYTE AndMask, BYTE OrMask)
{
    BYTE i;

    if (AndMask)
    {
        i = HDMIRX_ReadI2C_Byte(REG_RX_HWMUTE_CTRL);
    }
    i &= AndMask;
    i |= OrMask;
    HDMIRX_WriteI2C_Byte(REG_RX_HWMUTE_CTRL, i);
}

// ---------------------------------------------------------------------------
//void SetAudioMute(BOOL bMute)
//{
//    HDMIRX_AUDIO_PRINTF(("SetAudioMute(%d)\n",(int)bMute));
//    if(bMute==TRUE)
//    {
//        SetMUTE(~B_TRI_AUDIO,B_TRI_AUDIO);
//    }
//    else
//    {
//
//    SetMUTE(~B_TRI_AUDIO,0);
//
//    }
//}
// ---------------------------------------------------------------------------
void IT6607AudioOutput(BOOL bEnable)
{
#if 1
    BYTE uc;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL);
    HDMIRX_AUDIO_PRINTF(("IT6607AudioOutput(%s) Reg89 = %02X ->", bEnable ? "ON" : "OFF", (int)uc));

    if (bEnable == TRUE)
    {
        uc &= ~(B_TRI_I2S0 | B_TRI_I2S1 | B_TRI_I2S2 | B_TRI_I2S3 | B_TRI_SPDIF);
    }
    else
    {
        uc |= (B_TRI_I2S0 | B_TRI_I2S1 | B_TRI_I2S2 | B_TRI_I2S3 | B_TRI_SPDIF);
    }

    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc);
    HDMIRX_AUDIO_PRINTF((" %02X \n", (int)uc));
#endif
}

// ---------------------------------------------------------------------------

void hdmirx_ResetAudio()
{
    BYTE uc;

    uc  = HDMIRX_ReadI2C_Byte(REG_RX_RST_CTRL);
    uc |= B_AUDRST;
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, uc);

    // uc=HDMIRX_ReadI2C_Byte(REG_RX_RST_CTRL);
    uc &= ~(B_AUDRST);
    HDMIRX_WriteI2C_Byte(REG_RX_RST_CTRL, uc);

#ifdef AUTO_CORRECT_AUDIO_FS
    // force set REG_RX_FS_SET 4 times to trigger FIFO error again if happening - steven
    uc = HDMIRX_ReadI2C_Byte(REG_RX_FS_SET);
    HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc);
    HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc);
    HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc);
    HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc);
#endif

    gIsAudioReset = 1;
}

// ---------------------------------------------------------------------------
unsigned int hdmirx_SetAudioSampleRate()
{
    BYTE uc;
    unsigned int decodeFS, audioSampleRate;

    uc       = HDMIRX_ReadI2C_Byte(REG_RX_FS);
    decodeFS = (uc & M_Fs);

    switch (decodeFS)
    {
    // case B_Fs_192KHz:
    // audioSampleRate = B_Fs_48KHz;
    // break;

    case B_Fs_48KHz:
        audioSampleRate = B_Fs_44p1KHz;
        break;

    case B_Fs_44p1KHz:
        audioSampleRate = B_Fs_32KHz;
        break;

    case B_Fs_32KHz:
        //audioSampleRate = B_Fs_96KHz;
        audioSampleRate = B_Fs_48KHz;
        break;

    // case B_Fs_88p2KHz:
    // audioSampleRate = B_Fs_176p4KHz;
    // break;
    // case B_Fs_96KHz:
    // audioSampleRate = B_Fs_88p2KHz;
    // break;
    // case B_Fs_176p4KHz:
    // audioSampleRate = B_Fs_192KHz;
    // break;
    default:
        audioSampleRate = B_Fs_48KHz;
        break;
    }

    return audioSampleRate;
}

// ---------------------------------------------------------------------------

void IT6607SwitchAudioState(Audio_State_Type state)
{
    //  BYTE uc;

    if (it6607dev.m_AState == state)
    {
        return;
    }

    //  HDMIRX_VIDEO_PRINTF(("[%06u] +++ ",ucTickCount));
    HDMIRX_VIDEO_PRINTF((AStateStr[(BYTE)state]));
    HDMIRX_VIDEO_PRINTF(("+++ \n"));

    it6607dev.m_AState = state;
    AssignAudioVirtualTime();
    switch (it6607dev.m_AState)
    {
    case ASTATE_AudioOff:
        HDMIRX_SetI2C_Byte(REG_RX_RST_CTRL, B_AUDRST, B_AUDRST);
        IT6607AudioOutput(OFF);
        break;

    case ASTATE_RequestAudio:
        IT6607AudioOutput(OFF);

#ifdef Enable_HDMI_Tx
        TX_SwitchAudioState(ASTATE_TxAudioOff);
        TurnONTxAudio(FALSE);       //20100625
        DisTxAudio();
#endif
        break;

    case ASTATE_WaitForReady:
        hdmirx_SetHWMuteClr();
        hdmirx_ClearHWMuteClr();
        break;

    case ASTATE_AudioOn:
        HDMIRX_DEBUG_PRINTF(("it6607dev.m_MuteByPKG=%d,reg65=%02X\n", (WORD)it6607dev.m_MuteByPKG, (int)HDMIRX_ReadI2C_Byte(0x65)));

        IT6607AudioOutput(it6607dev.m_MuteByPKG ? OFF : ON);

        HDMIRX_AUDIO_PRINTF(("Cat6023 Audio--> Audio flag=%02X,Ch No=%02X,Fs=%02X ... \n",
                             (int)it6607dev.m_RxAudioCaps.AudioFlag,
                             (int)it6607dev.m_RxAudioCaps.AudSrcEnable,
                             (int)it6607dev.m_RxAudioCaps.SampleFreq));
        break;
    }
}

BYTE JudgeFS()
{
    ULONG CTS;
    ULONG N;
    BYTE reg78;
    BYTE uc;
    BYTE CK_Speed;
    ULONG FS;
    ULONG TMDS_DIV_128;
    reg78 = HDMIRX_ReadI2C_Byte(0x78);

    HDMIRX_WriteI2C_Byte(0x78, reg78 | 8);

    delay1ms(20);
    CTS      = (ULONG)HDMIRX_ReadI2C_Byte(0x83); CTS <<= 8;
    CTS     |= (ULONG)HDMIRX_ReadI2C_Byte(0x82); CTS <<= 4;
    N        = (ULONG)HDMIRX_ReadI2C_Byte(0x81);
    CTS     |= (N >> 4) & 0xF;
    N       &= 0xF;      N <<= 8;
    N       |= (ULONG)HDMIRX_ReadI2C_Byte(0x80); N <<= 8;
    N       |= (ULONG)HDMIRX_ReadI2C_Byte(0x7F);

    CK_Speed = HDMIRX_ReadI2C_Byte(0x38);
    if (CK_Speed == 0)
    {
        return AUDFS_OTHER;
    }
    TMDS_DIV_128 = ((ULONG)27000) / CK_Speed;

    HDMIRX_DEBUG_PRINTF(("FS = %ld ->", FS));
    if (CTS == 0)
    {
        return AUDFS_OTHER;
    }
    FS = (TMDS_DIV_128 * N) / CTS;

    // printf("uc: %u, CTS: 0x%X(%U), N: %u, TMDS_DIV_128: %u, FS: %u\n", uc, CTS, CTS, N, TMDS_DIV_128, FS);

    HDMIRX_WriteI2C_Byte(0x78, reg78);
    if (FS < 38)
    {
        return AUDFS_32KHz;
    }
    else if (FS < 47)
    {
        return AUDFS_44p1KHz;
    }
    else if (FS < 64)
    {
        return AUDFS_48KHz;
    }
    else if (FS < 92)
    {
        return AUDFS_88p2KHz;
    }
    else if (FS < 100)
    {
        return AUDFS_96KHz;
    }
    else if (FS < 180)
    {
        return AUDFS_176p4KHz;
    }
    return AUDFS_192KHz;
}

void IT6607AudioHandler()
{
    BYTE uc;

    if (it6607dev.m_RxHDCPState == RxHDCP_ModeCheck)
        return;

    switch (it6607dev.m_AState)
    {
    case ASTATE_RequestAudio:
        getHDMIRXInputAudio(&it6607dev.m_RxAudioCaps);

        if (it6607dev.m_RxAudioCaps.AudioFlag & B_CAP_AUDIO_ON)
        {
            uc  = HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) & 0xF8;
            uc |= 0x1;
            HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL, uc);

            if (it6607dev.m_RxAudioCaps.AudioFlag & B_CAP_HBR_AUDIO)
            {
                HDMIRX_AUDIO_PRINTF(("+++++++++++ B_CAP_HBR_AUDIO +++++++++++++++++\n"));
                Switch_HDMIRX_Bank(0);

#ifdef _HBR_I2S_
                uc  = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL);
                uc &= ~(1 << 4);
                HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL, uc);
#else
                HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL, (HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL) | 0x10));
#endif

                uc = HDMIRX_ReadI2C_Byte(REG_RX_MCLK_CTRL) & 0xF8;
                // uc |=0x0;
                HDMIRX_WriteI2C_Byte(REG_RX_MCLK_CTRL, uc);

                hdmirx_SetHWMuteClrMode();
                hdmirx_ResetAudio();
            }
            else if (it6607dev.m_RxAudioCaps.AudioFlag & B_CAP_DSD_AUDIO)
            {
                hdmirx_SetHWMuteClrMode();
                hdmirx_ResetAudio();
            }
            else
            {
                uc = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL);
                HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL, uc & (~0x10));

                BYTE getSamplingRate = JudgeFS();
                BYTE bModeChange     = 0;
                it6607dev.m_ucHDMIAudioErrorCount++;
                HDMIRX_DEBUG_PRINTF(("===[m_ucHDMIAudioErrorCount]= %d\n", (int)it6607dev.m_ucHDMIAudioErrorCount));
                uc = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL);

                if (getSamplingRate == AUDFS_OTHER)
                {
                    printf("it6607.c(%d), Abnormal Fs\n", __LINE__);
                    break;
                }

                if (it6607dev.m_ucAudioSampleClock != getSamplingRate)
                {
                    bModeChange = 1;
                }
                it6607dev.m_ucAudioSampleClock = getSamplingRate;
                if (it6607dev.m_ucHDMIAudioErrorCount > 10 || bModeChange)
                {
                    it6607dev.m_ucHDMIAudioErrorCount = 0;
                    printf("===[Audio FS Error ]=== force FS change: new_fs: %u\n", it6607dev.m_ucAudioSampleClock);
                    HDMIRX_DEBUG_PRINTF(("===[Audio FS Error ]===\n"));
                    uc                               |= B_FORCE_FS;
                }

                HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, uc);

                uc  = (HDMIRX_ReadI2C_Byte(REG_RX_FS_SET) & 0xCF);
                uc |= 0x70;
                HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, uc);

                HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL, HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL) | 0x80);
                HDMIRX_WriteI2C_Byte(REG_RX_FS_SET, it6607dev.m_ucAudioSampleClock);

                hdmirx_SetHWMuteClrMode();
                HDMIRX_WriteI2C_Byte(0x15, 0x18);
                hdmirx_ResetAudio();
            }
            /*
               else
               {
                uc = HDMIRX_ReadI2C_Byte(REG_RX_HWAMP_CTRL);
                HDMIRX_WriteI2C_Byte(REG_RX_HWAMP_CTRL,uc &(~0x10));
                hdmirx_SetHWMuteClrMode();
                hdmirx_ResetAudio();

               }
             */
            //SetIntMask3(~(B_AUTOAUDMUTE|B_AUDFIFOERR),(B_AUTOAUDMUTE|B_AUDFIFOERR));

            IT6607SwitchAudioState(ASTATE_WaitForReady);
        }
        break;

    case ASTATE_WaitForReady:
        //HDMIRX_WriteI2C_Byte(REG_RX_AUDIO_CTRL,HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CTRL) | 0x80); // 20090918 for test NLPCM to i2s output!
        if (AudioTimeOutCheck(AUDIO_READY_TIMEOUT))
        {
            IT6607SwitchAudioState(ASTATE_AudioOn);
        }
        break;

    case ASTATE_AudioOn:
        if (AudioTimeOutCheck(AUDIO_MONITOR_TIMEOUT) == TRUE)
        {
            AUDIO_CAPS CurAudioCaps;
            it6607dev.m_AudioCountingTimer = GetCurrentVirtualTime();

            getHDMIRXInputAudio(&CurAudioCaps);

            if (it6607dev.m_RxAudioCaps.AudioFlag != CurAudioCaps.AudioFlag
                || it6607dev.m_RxAudioCaps.AudSrcEnable != CurAudioCaps.AudSrcEnable
                || it6607dev.m_RxAudioCaps.SampleFreq != CurAudioCaps.SampleFreq)
            {
                it6607dev.m_ucHDMIAudioErrorCount = 0;
                IT6607SwitchAudioState(ASTATE_RequestAudio);
            }
            {
                BYTE judgeFs = JudgeFS();
                if (judgeFs != AUDFS_OTHER)
                {
                    // printf("cur: %d, judge: %d, err: %d\n",
                    // it6607dev.m_ucAudioSampleClock,
                    // judgeFs,
                    // it6607dev.m_ucHDMIAudioErrorCount);
                    if (it6607dev.m_ucAudioSampleClock != judgeFs || it6607dev.m_ucHDMIAudioErrorCount > 10)
                    {
                        IT6607SwitchAudioState(ASTATE_RequestAudio);
                    }
                }
            }
        }
        break;
    }
}

// ***************************************************************************
// Audio informatino function
// ***************************************************************************
BOOL HDMIRX_IsAudioOn()
{
    if (it6607dev.m_AState == ASTATE_AudioOn)
    {
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Name - getHDMIRXAudioStatus()
// Parameter - N/A
// return -
//         D[7:4] - audio type
//                  1100 - high bit rate
//                  1010 - one bit audio(DSD)
//                  1001 - NLPCM audio(compress)
//                  1000 - LPCM audio
//         D[3]   - layout
//         D[2:0] - enabled source.
// ---------------------------------------------------------------------------
BYTE getHDMIRXAudioStatus()
{
    BYTE uc, audio_status;

    Switch_HDMIRX_Bank(0);

    uc           = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT);
    audio_status = 0;

    if ((uc & (B_AUDIO_ON | B_HBRAUDIO | B_DSDAUDIO)) == (BYTE)(B_AUDIO_ON | B_HBRAUDIO))
    {
        audio_status = T_AUDIO_HBR;
    }
    else if ((uc & (B_AUDIO_ON | B_HBRAUDIO | B_DSDAUDIO)) == (BYTE)(B_AUDIO_ON | B_DSDAUDIO))
    {
        audio_status = T_AUDIO_DSD;
    }
    else if (uc & B_AUDIO_ON)
    {
        if (HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0) & (1 << 1))
        {
            // NLPCM/compressed audio
            audio_status = T_AUDIO_NLPCM;
        }
        else
        {
            audio_status = T_AUDIO_LPCM;
        }

        if (uc & B_AUDIO_LAYOUT)
        {
            audio_status |= F_AUDIO_LAYOUT_1;
        }

        if (uc & (1 << 3))
        {
            audio_status |= 4;
        }
        else if (uc & (1 << 2))
        {
            audio_status |= 3;
        }
        else if (uc & (1 << 1))
        {
            audio_status |= 2;
        }
        else if (uc & (1 << 0))
        {
            audio_status |= 1;
        }
    }

    return audio_status;
}

BYTE getHDMIRXAudioChannelNum()
{
    BYTE uc, channelNum = 0;

    Switch_HDMIRX_Bank(0);
    uc = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT);

    if (uc & B_AUDIO_ON)
    {
        if (uc & (1 << 3))
        {
            channelNum += 2;
        }

        if (uc & (1 << 2))
        {
            channelNum += 2;
        }

        if (uc & (1 << 1))
        {
            channelNum += 2;
        }

        if (uc & (1 << 0))
        {
            channelNum += 2;
        }
    }

    return channelNum;
}

// ---------------------------------------------------------------------------
// Parameter out - ucIEC60958ChStat[5]
// return - TRUE if ucIEC60958ChStat is returned
//
//                 ucIEC60958ChStat[0]
//                 - D[0] Comsumer used for channel status block
//                 - D[1] 0 - LPCM 1 - for IEC61937 spec.
//                 - D[2] 0 - Software for which copyright assert.
//                        1 - Software for which no copyright assert.
//                 - D[5:3] addition information.
//                 - D[7:6] channel status mode
//
//                 ucIEC60958ChStat[1]
//                 - D[7:0] categery of audio.
//
//                 ucIEC60958ChStat[2]
//                 - D[7:4] - Channel number(0/1/2)
//                 - D[3:0] - Source number(0..15)
//
//                 ucIEC60958ChStat[3]
//                 - D[5:4] = Clock accurency    - ret9F[1:0]
//                 - D[3:0] = Sample Word Length - reg84[3:0]
//
//                 ucIEC60958ChStat[4]
//                 - D[7:4] = Original sampling frequency
//                 - D[3:0] = Sample Word Length - reg9F[7:4]
// ---------------------------------------------------------------------------
BOOL getHDMIRXAudioChannelStatus(BYTE ucIEC60958ChStat[])
{
    BYTE fs, audio_status;

    audio_status = getHDMIRXAudioStatus();

    if (((audio_status & T_AUDIO_MASK) == T_AUDIO_OFF) ||
        ((audio_status & T_AUDIO_MASK) == T_AUDIO_DSD))
    {
        // return false if no audio or one-bit audio.
        return FALSE;
    }

    Switch_HDMIRX_Bank(0);
    ucIEC60958ChStat[0] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0);
    ucIEC60958ChStat[1] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT1);
    ucIEC60958ChStat[2] = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT2);
    fs                  = HDMIRX_ReadI2C_Byte(REG_RX_FS) & M_Fs;

    if ((audio_status & T_AUDIO_MASK) == T_AUDIO_HBR)
    {
        fs                   = B_Fs_HBR;
        ucIEC60958ChStat[0] |= B_AUD_NLPCM;
    }

    ucIEC60958ChStat[3]   = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT3);
    //
    ucIEC60958ChStat[4]   = (ucIEC60958ChStat[3] >> 4) & 0xF;
    ucIEC60958ChStat[4]  |= ((~fs) & 0xF) << 4;

    ucIEC60958ChStat[3]  &= 3;
    ucIEC60958ChStat[3] <<= 4;
    ucIEC60958ChStat[3]  |= fs & 0xF;

    return TRUE;
}

//
//// ---------------------------------------------------------------------------
//// BOOL getHDMIRXAudioInfo(BYTE *pbSampleFreq,BYTE *pbValidCh);
//// Parameter: pointer of BYTE pbSampleFreq - return sample freq
//// pointer of BYTE pbValidCh - return valid audio channel.
//// Return:    FALSE - no valid audio information during DVI mode.
////         TRUE - valid audio information returned.
//// Remark:    if pbSampleFreq is not NULL,*pbSampleFreq will be filled in with one of the following values:
////         0 - 44.1KHz
////         2 - 48KHz
////         3 - 32KHz
////         8 - 88.2 KHz
////         10 - 96 KHz
////         12 - 176.4 KHz
////         14 - 192KHz
////         Otherwise - invalid audio frequence.
////         if pbValidCh is not NULL,*pbValidCh will be identified with the bit valie:
////         bit[0] - '0' means audio channel 0 is not valid,'1' means it is valid.
////         bit[1] - '0' means audio channel 1 is not valid,'1' means it is valid.
////         bit[2] - '0' means audio channel 2 is not valid,'1' means it is valid.
////         bit[3] - '0' means audio channel 3 is not valid,'1' means it is valid.
//// ---------------------------------------------------------------------------
//BOOL getHDMIRXAudioInfo(BYTE *pbAudioSampleFreq,BYTE *pbValidCh)
//{
//#ifndef DISABLE_AUDIO_SUPPORT
//    if(HDMIRX_IsHDMIMode())
//    {
//        if(pbAudioSampleFreq)
//        {
//            *pbAudioSampleFreq = HDMIRX_ReadI2C_Byte(REG_RX_FS)& M_Fs;
//        }
//
//        if(pbValidCh)
//        {
//            *pbValidCh = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT);
//            if(*pbValidCh & B_AUDIO_LAYOUT)
//            {
//                *pbValidCh &= M_AUDIO_CH;
//            }
//            else
//            {
//                *pbValidCh = B_AUDIO_SRC_VALID_0;
//            }
//        }
//        return TRUE;
//    }
//    else
//#endif
//    {
//        return FALSE;
//    }
//}
// ---------------------------------------------------------------------------
void getHDMIRX_AudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus)
{
    BYTE uc;

    uc                                     = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0);

    RegChannelstatus->ISLPCM               = (uc & 0x02) >> 1;
    RegChannelstatus->CopyRight            = (uc & 0x04) >> 2;
    RegChannelstatus->AdditionFormatInfo   = (uc & 0x18) >> 3;
    RegChannelstatus->ChannelStatusMode    = (uc & 0xE0) >> 5;

    uc                                     = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT1);
    RegChannelstatus->CategoryCode         = uc;

    uc                                     = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT2);
    RegChannelstatus->SourceNumber         = uc & 0x0F;
    RegChannelstatus->ChannelNumber        = (uc & 0xf0) >> 4;

    uc                                     = HDMIRX_ReadI2C_Byte(REG_RX_FS);
    RegChannelstatus->SamplingFreq         = uc & 0x0F;

    uc                                     = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT3);
    RegChannelstatus->ClockAccuary         = uc & 0x03;
    RegChannelstatus->WorldLen             = (uc & 0xF0) >> 4;

    uc                                     = HDMIRX_ReadI2C_Byte(0x8E);
    RegChannelstatus->OriginalSamplingFreq = (uc & 0xF0) >> 4;
}

// ---------------------------------------------------------------------------
void getHDMIRXInputAudio(AUDIO_CAPS *pAudioCaps)
{
#if 1
    BYTE uc;

    Switch_HDMIRX_Bank(0);

    uc                        = HDMIRX_ReadI2C_Byte(REG_RX_FS);
    pAudioCaps->SampleFreq    = uc & M_Fs;

    uc                        = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT);
    pAudioCaps->AudioFlag     = uc & 0xF0;
    pAudioCaps->AudSrcEnable  = uc & M_AUDIO_CH;
    delay1ms(1);
    pAudioCaps->AudSrcEnable |= HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) & M_AUDIO_CH;
    delay1ms(1);
    pAudioCaps->AudSrcEnable |= HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) & M_AUDIO_CH;
    delay1ms(1);
    pAudioCaps->AudSrcEnable |= HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT) & M_AUDIO_CH;

    if ((uc & (B_HBRAUDIO | B_DSDAUDIO)) == 0)
    {
        uc = HDMIRX_ReadI2C_Byte(REG_RX_AUD_CHSTAT0);

        if ((uc & B_NLPCM) == 0)
        {
            pAudioCaps->AudioFlag |= B_CAP_LPCM;
        }
    }
#else
    HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_CH_STAT);
#endif
}

// ---------------------------------------------------------------------------
BOOL getHMDIRX_AudioInfo(AUDIO_CAPS *pAudioCaps, RX_REG_AUDIO_CHSTS *RegChannelstatus)
{
    if (it6607dev.m_AState == ASTATE_AudioOn)
    {
        if (pAudioCaps != NULL)
        {
            getHDMIRXInputAudio(pAudioCaps);
        }
        if (RegChannelstatus != NULL)
        {
            getHDMIRX_AudioChannelStatus(RegChannelstatus);
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// ---------------------------------------------------------------------------
// Get Info Frame and HDMI Package
// Need upper program pass information and read them.
// ---------------------------------------------------------------------------
BOOL getHDMIRX_AVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame)
{
    BYTE checksum;
    int i;

    if (pAVIInfoFrame == NULL)
    {
        return FALSE;
    }
    //if(it6607dev.m_NewAVIInfoFrameF==TRUE) {
    if (HDMIRX_IsHDMIMode() == TRUE)
    {
        pAVIInfoFrame->pktbyte.AVI_HB[0] = AVI_INFOFRAME_TYPE;
        pAVIInfoFrame->pktbyte.AVI_HB[1] = HDMIRX_ReadI2C_Byte(REG_RX_AVI_VER);
        pAVIInfoFrame->pktbyte.AVI_HB[2] = AVI_INFOFRAME_LEN;

        HDMIRX_ReadI2C_ByteN(REG_RX_AVI_DB1, pAVIInfoFrame->pktbyte.AVI_DB, AVI_INFOFRAME_LEN);

        HDMIRX_DEBUG_PRINTF(("AVI infoframe: 0x%02x,0x%02x\n", pAVIInfoFrame->pktbyte.AVI_DB[0], pAVIInfoFrame->pktbyte.AVI_DB[1]));
        checksum = HDMIRX_ReadI2C_Byte(REG_RX_AVI_DB0);
        for (i = 0; i < 3; i++)
        {
            checksum += pAVIInfoFrame->pktbyte.AVI_HB[i];
        }

        for (i = 0; i < AVI_INFOFRAME_LEN; i++)
        {
            checksum += pAVIInfoFrame->pktbyte.AVI_DB[i];
        }

        if (checksum == 0)
            return TRUE;
        else
            return FALSE;
    }

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL getHDMIRX_VENDORSPECInfoFrame(BYTE *pData)
{
    // BYTE checksum;
    int i;
    BYTE uc;

    if (pData == NULL)
    {
        return FALSE;
    }

    uc = HDMIRX_ReadI2C_Byte(REG_RX_GENPKT_HB0);
    if (uc != (VENDORSPEC_INFOFRAME_TYPE | 0x80))
    {
        return FALSE;
    }
    pData[0] = uc;
    for (i = 1; i < 3 + 28; i++)
    {
        pData[i] = HDMIRX_ReadI2C_Byte(REG_RX_GENPKT_HB0 + i);
    }

    return TRUE;
}

//---------------------------------------------------------------------------

BOOL getHDMIRX_AudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
    BYTE checksum;

    int i;

    if (pAudioInfoFrame != NULL)
    {
        pAudioInfoFrame->pktbyte.AUD_HB[0] = AUDIO_INFOFRAME_TYPE;
        pAudioInfoFrame->pktbyte.AUD_HB[1] = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_VER);
        pAudioInfoFrame->pktbyte.AUD_HB[2] = AUDIO_INFOFRAME_LEN;

        HDMIRX_ReadI2C_ByteN(REG_RX_AUDIO_DB1, pAudioInfoFrame->pktbyte.AUD_DB, 5);

        checksum = HDMIRX_ReadI2C_Byte(REG_RX_AUDIO_DB0);

        for (i = 0; i < 3; i++)
            checksum += pAudioInfoFrame->pktbyte.AUD_HB[i];

        for (i = 0; i < 5; i++)
            checksum += pAudioInfoFrame->pktbyte.AUD_DB[i];

        if (checksum == 0)
            return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Timer Control Function
////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
void DumpHDMIRXReg(void)
{
#if 1
    unsigned int i, j;
    //BYTE reg;
    //BYTE bank;
    BYTE ucData;

    HDMIRX_DEBUG_PRINTF(("########################################################################\n"));
    HDMIRX_DEBUG_PRINTF(("#Dump IT6607\r\n"));
    HDMIRX_DEBUG_PRINTF(("########################################################################\n" "       "));

    for (j = 0; j < 16; j++)
    {
        HDMIRX_DEBUG_PRINTF((" %02X", (unsigned int)j));
        if ((j == 3) || (j == 7) || (j == 11))
        {
            HDMIRX_DEBUG_PRINTF((" :"));
        }
    }
    HDMIRX_DEBUG_PRINTF(("\n---------------------------------------------------------------------------\n"));

    HDMIRX_WriteI2C_Byte(0x0F, 0);

    for (i = 0; i < 0x100; i += 16)
    {
        HDMIRX_DEBUG_PRINTF(("[%03X]  ", i));
        for (j = 0; j < 16; j++)
        {
            ucData = HDMIRX_ReadI2C_Byte((BYTE)((i + j) & 0xFF));
            HDMIRX_DEBUG_PRINTF((" %02X", (int)ucData));
            if ((j == 3) || (j == 7) || (j == 11))
            {
                HDMIRX_DEBUG_PRINTF((" :"));
            }
        }
        HDMIRX_DEBUG_PRINTF(("\r\n"));
        if ((i % 0x40) == 0x30)
        {
            HDMIRX_DEBUG_PRINTF(("        -------------------------------------------------------\r\n"));
        }
    }

    HDMIRX_DEBUG_PRINTF(("        -------------------------------------------------------\r\n"));
    HDMIRX_WriteI2C_Byte(0x0F, 1);

    for (i = 0x150; i < 0x200; i += 16)
    {
        HDMIRX_DEBUG_PRINTF(("[%03X]  ", i));
        for (j = 0; j < 16; j++)
        {
            ucData = HDMIRX_ReadI2C_Byte((BYTE)((i + j) & 0xFF));
            HDMIRX_DEBUG_PRINTF((" %02X", (int)ucData));
            if ((j == 3) || (j == 7) || (j == 11))
            {
                HDMIRX_DEBUG_PRINTF((" :"));
            }
        }
        HDMIRX_DEBUG_PRINTF(("\r\n"));
        if ((i % 0x40) == 0)
        {
            HDMIRX_DEBUG_PRINTF(("        -------------------------------------------------------\r\n"));
        }
    }

    HDMIRX_WriteI2C_Byte(0x0F, 0);
#endif
}

// ***************************************************************************
// Tri-State Enable
// ***************************************************************************
void Set_IT6607_Tri_State_Enable()
{
    BYTE uc;
    BOOL flag;

    uc = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL);
    uc = (uc | 0x3f);
    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc);

#ifndef COMPONENT_DEV
    #if (defined(IT9919_144TQFP) && defined (EVB_BOARD) && defined (HDMI_LOOPTHROUGH))
    //Disable IT6604 for Video EVB
    mmpIicLockModule();
    uc = 0xFF;
    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, 0x92 >> 1, 0x89, &uc, 1)))
    {
        printf("HDMIRX IT6604 I2c write error, reg = %02x val =%02x\n", 0x89, uc);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    #endif
#endif

    gtTriStateEnable = 1;
}

// ***************************************************************************
// Tri-State Disable
// ***************************************************************************
void Set_IT6607_Tri_State_Disable()
{
    BYTE uc;

    gtTriStateEnable = 0;

    uc               = HDMIRX_ReadI2C_Byte(REG_RX_TRISTATE_CTRL);
    uc               = (uc & 0xc0);
    HDMIRX_WriteI2C_Byte(REG_RX_TRISTATE_CTRL, uc);
}

// ***************************************************************************
// PowerDown
// ***************************************************************************
void IT6607_PowerDown(
    BOOL enable)
{
    if (enable)
    {
        HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, (1 << 7) | (1 << 3) | (1 << 2) | (1 << 0));
        HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL0, 0xFF); //B_PWD_ALL);
    }
    else
    {
        HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL1, 0x00);
        HDMIRX_WriteI2C_Byte(REG_RX_PWD_CTRL0, 0x00);
    }
}

// ***************************************************************************
// get output video mode
// ***************************************************************************
unsigned int
IT6607_GetOutputVideoMode(
    void)
{
    return (it6607dev.m_bOutputVideoMode & F_MODE_CLRMOD_MASK);
}

// ***************************************************************************
// hdmi idle
// ***************************************************************************
BOOL
IT6607_IsEngineIdle(
    void)
{
    if ((it6607dev.m_VState == VSTATE_SWReset) ||
        (it6607dev.m_VState == VSTATE_PwrOff) ||
        (it6607dev.m_VState == VSTATE_SyncWait))
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

// ***************************************************************************
// HDCP On/Off
// ***************************************************************************
void IT6607_DisableHDCP(
    BOOL flag)
{
    if (flag)
        gtDisableHDCP = 1;
    else
        gtDisableHDCP = 0;
}

// ***************************************************************************
// HDCP On/Off
// ***************************************************************************
BOOL
IT6607_IsDisableHDCP(
    void)
{
    if (gtDisableHDCP)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

// ***************************************************************************
// Get HDCP Key State
// ***************************************************************************
BOOL
IT6607_IsHDCPKeyEmpty(
    void)
{
    if (gtIsHDCPKeyEmpty)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

void
IT6607_ResetAudioFS(
    void)
{
    hdmirx_ResetAudio();
}