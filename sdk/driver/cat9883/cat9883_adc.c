///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <ADC.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/08/12
//   @fileversion: CAT9883_V43
//******************************************/
//#define _CAT9983_CODE_

#include "pal/pal.h"
#include "mmp_types.h"
#include "host/host.h"

#include "cat9883/cat9883_mcu.h"
#include "cat9883/cat9883_adc.h"
#include "capture/capture.h"

#define _TV_Mode_External_PLL_

#ifdef IT9913_128LQFP
    #define CAT9883_OUTPUT_YUV422
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variable
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MMP_UINT16 gtCAT9883CurMode  = 0;
static MMP_UINT16 gtCAT9883NextMode = 0;

typedef struct _REGPAIR {
    MMP_UINT8 ucAddr;
    MMP_UINT8 ucValue;
} REGPAIR;

REGPAIR CAT9883C_ini_value[] =
{
    {0x01, 0x69},
    {0x02, 0xd0},
    {0x03, 0x88},
    //   {0x07,0xf0},
    {0x8f, 0x68},
    {0x86, 0x28},       // Clive  061011  0x28 => 0x29
    {0x8d, 0x80},       // Clive  061608  0xc0 => 0x80
    {0x84, 0x00},
    {0x87, 0x69},       // Clive 041108  0x09 => 0x69
    {0x8E, 0x00},       // Clock out no inverter
    {0x91, 0x30},       // Clive 041108  0x60 => 0x30
#ifdef  _TV_Mode_External_PLL_
    {0x96, 0x22},
#else
    {0x96, 0x25},
#endif
    {0x98, 0x19},
    {0x84, 0x0C},
    {0x99, 0x08},
};

REGPAIR TVdefaultinit[] =
{
    {0x94, 0x55},
    {0x96, 0x22},
};

REGPAIR TV480iini[] =
{
    {0x91, 0x70}, // Clive 041108  0x60 => 0x70
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00}, // Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0x08},
    {0x04, 0xa8},
    {0x94, 0x81},//41H
    {0x96, 0x2a},
#else
    {0x94, 0xF1},//51H
    {0x96, 0x29},
#endif
};
REGPAIR TV576iini[] =
{
    {0x91, 0x70}, // Clive 041108  0x60 => 0x70
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00}, // Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0x08},
    {0x04, 0xc0},
    {0x94, 0x81},//41H
    {0x96, 0x2a},
#else
    {0x94, 0xF1},//51H
    {0x96, 0x29},
#endif
};
REGPAIR TV480pini[] =
{
    {0x91, 0x70},
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00},// Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0xa8},
    {0x04, 0x38},
    {0x94, 0x41},//41H
    {0x96, 0x22},
#else
    {0x94, 0x31},//51H
    {0x96, 0x29},
#endif
};
REGPAIR TV576pini[] =
{
    {0x91, 0x70},
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00},// Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0xa8},
    {0x04, 0x38},
    {0x94, 0x41},//41H
    {0x96, 0x22},
#else
    {0x94, 0x31},//51H
    {0x96, 0x29},
#endif
};
REGPAIR TV720pini[] =
{
    {0x91, 0x70},
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00},// Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0xb8},
    {0x04, 0x38},
    {0x94, 0x31},//31H
    {0x96, 0x22},
#else
    {0x94, 0x31},//51H
    {0x96, 0x29},
#endif
};
REGPAIR TV1080iini[] =
{
    {0x91, 0x70},
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00},// Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0xa8},
    {0x04, 0x38},
    {0x94, 0x81},//31H
    {0x96, 0x22},
#else
    {0x94, 0x51},//51H
    {0x96, 0x29},
#endif
};

REGPAIR TV1080pini[] =
{
    {0x91, 0x70},

#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00},//Clock Out no inverter
    {0x86, 0x29},
    {0x87, 0xe9},
    {0x03, 0xe8},
    {0x04, 0x38},
    {0x94, 0x81},//31H
    {0x96, 0x22},
#else
    {0x94, 0x51},//51H
    {0x96, 0x29},
#endif
};

REGPAIR TV1080p24ini[] =
{
    {0x91, 0x70},
#ifdef  _TV_Mode_External_PLL_
    {0x8E, 0x00},// Clock out no inverter
    {0x86, 0x28},
    {0x87, 0x69},
    {0x03, 0xb8},
    {0x04, 0x38},
    {0x94, 0x31},//31H
    {0x96, 0x22},
#else
    {0x94, 0x31},//51H
    {0x96, 0x29},
#endif
};

///////////////////// CAT9883C settings //////////////////////////////

MMP_UINT8 PC_576Px50[] =
{
    0x35, 0xf0, 0x68, 0x80, 0x20, 0x10, 0x80,       // Reg0x04: 0x80 => 0xb0
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_480Px60[] =
{
    0x35, 0x90, 0x28, 0x38, 0x20, 0x10, 0x80,       // Reg0x04: 0x80 => 0xb0
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_720Px60[] =
{
    0x67, 0x10, 0xa0, 0x38, 0x40, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x44, 0x6e,        // reg0x0e => 0x44  , Reg0x04: 0x80 => 0xc0
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1080ix60[] =
{
    0x89, 0x70, 0xa0, 0x38, 0x40, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x44, 0x6e,        // reg0x0e => 0x44  , Reg0x04: 0x80 => 0x10
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_640x480x60[] =
{
    0x31, 0xf0, 0x30, 0x70, 0x10, 0x10, 0x80,           // 0x70 => 0x30
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_640x480x72[] =
{
    0x33, 0xf0, 0x70, 0x70, 0x10, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_640x480x75[] =
{
    0x34, 0x70, 0x70, 0x70, 0x10, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_640x480x85[] =
{
    0x33, 0xf0, 0x70, 0x70, 0x10, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_800x600x56[] =
{
    0x3f, 0xf0, 0x70, 0x70, 0x10, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_800x600x60[] =
{
    0x41, 0xf0, 0x60, 0x70, 0x10, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_800x600x72[] =
{
    0x40, 0xf0, 0x70, 0x70, 0x10, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_800x600x75[] =
{
    0x41, 0xf0, 0x70, 0x70, 0x10, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_800x600x85[] =
{
    0x41, 0x70, 0x70, 0x70, 0x10, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1024x768x60[] =
{
    0x53, 0xf0, 0xa8, 0x70, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1024x768x70[] =
{
    0x52, 0xf0, 0xa8, 0x70, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1024x768x75[] =
{
    0x51, 0xf0, 0xa8, 0x70, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1024x768x80[] =
{
    0x53, 0x70, 0xa8, 0x70, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1024x768x85[] =
{
    0x55, 0xf0, 0xa8, 0x70, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1280x1024x60[] =
{
    0x69, 0x70, 0xa8, 0x38, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1280x1024x75[] =
{
    0x69, 0x70, 0xf0, 0x38, 0x10, 0x40, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1280x1024x85[] =
{
    0x6b, 0xf0, 0xa8, 0x38, 0x10, 0x40, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1600x1200x60[] =
{
    0x86, 0xf0, 0xe8, 0x40, 0x10, 0x80, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1360x768x60[] =                     // Clive 041008
{
    0x6f, 0xf0, 0x90, 0x10, 0x10, 0x40, 0x80,    //reg0x03 B8=> 90
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1680x1050x60[] =
{
    0x8b, 0xf0, 0xf0, 0xa8, 0x40, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1440x900x60[] =
{
    0x76, 0xf0, 0xa8, 0x00, 0x20, 0x20, 0x80,        // reg0x03 0x70=> 0xa8   Clive 04072011    106MHz
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1280x800x60[] =                       // mode = 0xC8         Clive 0407     1680 pixel
{
    0x68, 0xf0, 0xa8, 0x00, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1280x960x60[] =                       // mode = 0x73
{
    0x70, 0x70, 0xb0, 0x00, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1920x1080x60[] =
{
    0x89, 0x70, 0xf0, 0x80, 0x40, 0x40, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,        // mode = 0x83
    0xb8, 0x19, 0x00, 0x00
};

MMP_UINT8 PC_1920x1200xReduce[] =
{
    0x81, 0xf0, 0xf0, 0x80, 0x20, 0x20, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,        // mode = 0x5b
    0xb8, 0x19, 0x00, 0x00
};

///////// CAT9883C TV Mode Register Settings ////////////

MMP_UINT8 TV_480ix60[] =
{
    0x35, 0x90, 0x08, 0xa8, 0x08, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x60, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_480Px60[] =
{
    0x35, 0x90, 0x28, 0x38, 0x08, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x60, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_576ix50[] =
{
    0x35, 0xf0, 0x08, 0x80, 0x08, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x60, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_576Px50[] =
{
    0x35, 0xf0, 0x28, 0x80, 0x08, 0x10, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x60, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_720Px60[] =
{
    0x67, 0x10, 0xa0, 0x38, 0x40, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x44, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_720Px50[] =
{
    0x7b, 0xb0, 0xa0, 0x38, 0x40, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x44, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_1080ix50[] =
{
    0xa4, 0xf0, 0x88, 0x38, 0x40, 0x20, 0xf0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x44, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_1080ix60[] =
{
    0x89, 0x70, 0x88, 0x38, 0x40, 0x20, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x44, 0x6e,
    0x8d, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_1080Px24[] =
{
    0xab, 0xd0, 0xe8, 0x38, 0x40, 0x20, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xbd, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_1080Px50[] =
{
    0xa4, 0xf0, 0xe8, 0x38, 0x40, 0x20, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xbd, 0x19, 0x08, 0x00
};

MMP_UINT8 TV_1080Px60[] =
{
    0x89, 0x70, 0xe8, 0x38, 0x40, 0x20, 0xa0,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xbd, 0x19, 0x08, 0x00
};

MMP_UINT8 Default_ini[] =
{
    0x69, 0xd0, 0xa8, 0x38, 0x10, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x6e,
    0xbd, 0x19, 0x00, 0x00
};

void Load_Reg_ini(REGPAIR *pREGPAIR, MMP_UINT16 N)
{
    MMP_UINT8  i;
    MMP_UINT8  Value;
    MMP_RESULT result;

    for (i = 0; i < N; i++)
    {
        result    = CAT9883_WriteI2C_Byte(pREGPAIR->ucAddr, pREGPAIR->ucValue);
        pREGPAIR += 1;
    }
}

MMP_BOOL Mode_stable(MMP_BOOL isFrameRateCheck)
{
    static MMP_BOOL VsToggle = 0;

    if (isFrameRateCheck == MMP_TRUE)
    {
        if ( (gtCAT9883CurMode == 0) ||
             ((gtCAT9883CurMode & 0x00ff) == 0x00ff) ||
             ((gtCAT9883CurMode & 0xff00) == 0xff00) )
        {
            return MMP_FALSE;
        }
    }
    else
    {
        if ( (gtCAT9883CurMode == 0) ||
             ((gtCAT9883CurMode & 0xff00) == 0xff00))
        {
            return MMP_FALSE;
        }
    }

    VsToggle          = 1;
    gtCAT9883NextMode = Get_mode_number(isFrameRateCheck);

    if (Is_TV_mode())
    {
        if ((gtCAT9883NextMode & 0x00FF) != 0x60 && (gtCAT9883NextMode & 0x00FF) != 0x50 && (gtCAT9883NextMode & 0x00FF) != 0x24 && VsToggle)
        {
            gtCAT9883NextMode = gtCAT9883CurMode;
            return MMP_TRUE;
        }
        else if ((gtCAT9883NextMode == gtCAT9883CurMode) && VsToggle)
        {
            return MMP_TRUE;
        }
    }
    else
    {
        if ((gtCAT9883NextMode == gtCAT9883CurMode) && VsToggle)
        {
            return MMP_TRUE;
        }
    }

    return MMP_FALSE;
}

void Set_CAT9883_Tri_State_Enable()
{
    MMP_UINT8 val;
    val  = CAT9883_ReadI2C_Byte(0x85);
    val &= ~(1 << 6);
    CAT9883_WriteI2C_Byte(0x85, val);
    val  = CAT9883_ReadI2C_Byte(0x85);
}

void Set_CAT9883_Tri_State_Disable()
{
    MMP_UINT8 val;
    val  = CAT9883_ReadI2C_Byte(0x85);
    val |= (1 << 6);
    CAT9883_WriteI2C_Byte(0x85, val);
    val  = CAT9883_ReadI2C_Byte(0x85);
}

void ADC_Mode_Change()
{
    Reg_Pair_initial(CAT9883C_ini_value);

    //Wait CAT9883 Initial Clock State
    MMP_Sleep(100);

    ModeDetectingprocess();
    //Wait CAT9883 TV Mode Clock State
    MMP_Sleep(200);
    AutoColorCalibration();
}

void CAT9883CInitial()
{
    Reg_Pair_initial(CAT9883C_ini_value);

    gtCAT9883CurMode  = 0;
    gtCAT9883NextMode = 0;
}

void ModeDetectingprocess()
{
    MMP_UINT16 val;

    Set_mode(Get_mode_number(MMP_FALSE));
}

MMP_UINT16 Hsync_counter()
{
    MMP_UINT16 i = 0, j = 0;

    CAT9883_WriteI2C_Byte(0x8F, 0x68);

    i = CAT9883_ReadI2C_Byte(0xAB);
    j = CAT9883_ReadI2C_Byte(0xAC);
    i = ((j << 4) & 0x0F00) + i;
    return i;
}

MMP_UINT16 Vsync_timer()
{
    MMP_UINT16 RawVTotal;
    MMP_UINT16 framerate;
    static     prevRawVTotal = 0;
    static     prevframerate = 0;

    HOST_ReadRegister (0x1F2E, (MMP_UINT16 *)&RawVTotal);

    if (RawVTotal != prevRawVTotal)
    {
        MMP_UINT32 MemClk;
        MMP_FLOAT  MCLK_Freq;

        MemClk        = or32_getMemCLK();
        MCLK_Freq     = (MMP_FLOAT)(MemClk / 1000.0);

        framerate     = ((3906 * MCLK_Freq) / RawVTotal);
        prevRawVTotal = RawVTotal;
        prevframerate = framerate;
    }
    else
        framerate = prevframerate;

    return framerate;
}

MMP_UINT16 Get_mode_number(MMP_BOOL isFrameRateCheck)
{
    if (Is_TV_mode())
        return TV_mode(isFrameRateCheck);
    else
        return PC_mode();
}

MMP_BOOL Is_TV_mode()
{
    MMP_UINT8 i;

    i                  = CAT9883_ReadI2C_Byte(0x14);

    CAT9883_InIsTVMode = (i & 0x40) >> 4;

    return (i & 0x40) ? MMP_TRUE : MMP_FALSE;
}

void AutoColorCalibration()
{
    CAT9883_WriteI2C_Byte(0x84, 0x0c);          // Calibration once
}

///////////////  TV Mode ////////////////////////
MMP_UINT16 TV_mode(MMP_BOOL isFrameRateCheck)
{
    MMP_UINT16 Resolution_mode;
    MMP_UINT16 tv_mode;
    MMP_UINT16 framerate;
    MMP_UINT16 hsynccnt;

    framerate = Frame_rate();
    hsynccnt  = Hsync_counter();

    if     (hsynccnt < 0x0070)   { Resolution_mode= 0xff00;}// <xxx unknow
    else if(hsynccnt < 0x0120)   { Resolution_mode= 0xa400;}// 480i = 0x107
    else if(hsynccnt < 0x0180)   { Resolution_mode= 0xa500;}// 576i = 0x138
    else if(hsynccnt < 0x0220)   { Resolution_mode= 0xb400;}// 480P = 0x20d
    else if(hsynccnt < 0x0250)   { Resolution_mode= 0xa100;}// 1080i= 0x233
    else if(hsynccnt < 0x02a0)   { Resolution_mode= 0xb500;}// 576P = 0x271
    else if(hsynccnt < 0x03a0)   { Resolution_mode= 0xb700;}// 720P = 0x2ee
    else if(hsynccnt < 0x04a0)   { Resolution_mode= 0xb100;}// 1080P= 0x465
    else                         { Resolution_mode= 0xff00;}// unknow

    tv_mode = Resolution_mode + framerate;

    //printf("---tv_mode = 0x%x---\n", tv_mode);

    switch (tv_mode)
    {
    case 0xa460:     //480i
    case 0xa550:     //576i
    case 0xb460:     //480P
    case 0xa150:     //1080i50
    case 0xa160:     //1080i60
    case 0xb550:     //576p
    case 0xb750:     //720p50
    case 0xb760:     //720p60
    case 0xb124:     //1080p24
    case 0xb150:     //1080p50
    case 0xb160:     //1080p60
        break;

    default:
        if (isFrameRateCheck)
            tv_mode = 0xFFFF;
        break;
    }

    return  (tv_mode);
}

///////////////  PC Mode ////////////////////////
MMP_UINT16 PC_mode()
{
    MMP_UINT16 Resolution_mode;

    if     (                            Hsync_counter() < 0x0170)   { Resolution_mode= 0xff00; } // <xxx unknow
    else if(Hsync_counter() >=0x01a0 && Hsync_counter() < 0x0240)   { Resolution_mode= 0x6400; } // 640x480 = 0x20d
    else if(Hsync_counter() >=0x0240 && Hsync_counter() < 0x02d0)   { Resolution_mode= 0x8000; } // 800x600 = 0x0274

    else if(Hsync_counter() >=0x02d0 && Hsync_counter() < 0x0310)   { Resolution_mode= 0x0300; } // 1280x720 = 0x02EE          //Clive

    else if(Hsync_counter() >=0x0310 && Hsync_counter() < 0x031d)   { Resolution_mode= 0x1300; } // 1360x768 = 0x031b
    else if(Hsync_counter() >=0x031e && Hsync_counter() < 0x0331)   { Resolution_mode= 0x1000; } // 1024x768 = 0x0326/(320@75)
    else if(Hsync_counter() >=0x0332 && Hsync_counter() < 0x0352)   { Resolution_mode= 0xC800; } // 1280x800  V_total=828=0x33c  Clive 0407
    else if(Hsync_counter() >=0x0360 && Hsync_counter() < 0x039f)   { Resolution_mode= 0x1480; } // 1440x900 reduced blanking
    else if(Hsync_counter() >=0x03a0 && Hsync_counter() <=0x03a7)   { Resolution_mode= 0x1400; } // 1440x900
    else if(Hsync_counter() >=0x03c0 && Hsync_counter() < 0x0400)   { Resolution_mode= 0xC900; } // 1280x960 = 0x03e8
    else if(Hsync_counter() >=0x0420 && Hsync_counter() < 0x0437)   { Resolution_mode= 0x1200; } // 1280x1024 = 0x042a
    else if(Hsync_counter() >=0x0437 && Hsync_counter() <=0x043a)   { Resolution_mode= 0x1680; } // 1680x1050 reduced blanking =
    else if(Hsync_counter() >=0x0440 && Hsync_counter() < 0x0459)   { Resolution_mode= 0x1600; } // 1680x1050
    else if(Hsync_counter() >=0x0459 && Hsync_counter() < 0x0490)   { Resolution_mode= 0x1900; } // 1920x1080 = 0x465  // Clive 041108 0x460 => 0x459
    else if(Hsync_counter() >=0x04d0 && Hsync_counter() <=0x04d6)   { Resolution_mode= 0x1980; } // 1920x1200 reduced blanking = 0x04d3
    else if(Hsync_counter() >=0x04da && Hsync_counter() < 0x0510)   { Resolution_mode= 0x1C00; } // 1600x1200 = 0x04e2
    else                                                            { Resolution_mode= 0xff00; } // >=1300 unknow

    if ((gtCAT9883CurMode & 0xFFFF) != (Resolution_mode + Frame_rate()))
    {
        dbg_msg(SDK_MSG_TYPE_INFO, "");

        if     (Resolution_mode == 0x6400)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 640x480--------\n");
        else if(Resolution_mode == 0x8000)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 800x600--------\n");
        else if(Resolution_mode == 0x0300)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1280x720--------\n");
        else if(Resolution_mode == 0x1300)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1360x768--------\n");
        else if(Resolution_mode == 0x1000)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1024x768--------\n");
        else if(Resolution_mode == 0xC800)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1280x800--------\n");
        else if(Resolution_mode == 0x1480)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1440x900 reduced blanking--------\n");
        else if(Resolution_mode == 0x1400)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1440x900--------\n");
        else if(Resolution_mode == 0xC900)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1280x960--------\n");
        else if(Resolution_mode == 0x1200)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1280x1024--------\n");
        else if(Resolution_mode == 0x1680)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1680x1050 reduced blanking--------\n");
        else if(Resolution_mode == 0x1600)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1680x1050--------\n");
        else if(Resolution_mode == 0x1900)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1920x1080--------\n");
        else if(Resolution_mode == 0x1980)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1920x1200 reduced blanking--------\n");
        else if(Resolution_mode == 0x1C00)  dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 PC Mode Resolution = 1600x1200--------\n");

        if (Resolution_mode != 0xFF00 && Frame_rate() != 0xFF)
            dbg_msg(SDK_MSG_TYPE_INFO, "--------frame rate = %x fps--------\n", Frame_rate());
    }

    return  (Resolution_mode + Frame_rate());
}

MMP_UINT16 Frame_rate()
{
    MMP_UINT16        framerate;
    MMP_UINT16        FrameRate_mode;
    static MMP_UINT16 preFrameRate_mode = 0xff;

    framerate = Vsync_timer();

    /*
        if (framerate > 100) //caputre reset
            return preFrameRate_mode;

        if (framerate > 84)
            FrameRate_mode= 0x0085;
        else if(framerate > 74)
            FrameRate_mode= 0x0075;
        else if(framerate > 71)
            FrameRate_mode= 0x0072;
        else if(framerate > 69)
            FrameRate_mode= 0x0070;
        else if(framerate > 59)
            FrameRate_mode= 0x0060;
        else if(framerate > 55)
            FrameRate_mode= 0x0056;
        else if(framerate > 49)
            FrameRate_mode= 0x0050;
        else if(framerate > 29)
            FrameRate_mode= 0x0030;
        else if(framerate > 23)
            FrameRate_mode= 0x0024;
        else
            FrameRate_mode= 0x00ff;     // unknow
     */

    if ((23988 > framerate) && (framerate > 23946))       // 23.976fps
    {
        FrameRate_mode = 0x24;
    }
    else if ((24030 > framerate) && (framerate > 23987))  // 24fps
    {
        FrameRate_mode = 0x24;
    }
    else if ((25030 > framerate) && (framerate > 24970))  // 25fps
    {
        FrameRate_mode = 0x25;
    }
    else if ((29985 > framerate) && (framerate > 29940))  // 29.97fps
    {
        FrameRate_mode = 0x30;
    }
    else if ((30030 > framerate) && (framerate > 29984))  // 30fps
    {
        FrameRate_mode = 0x30;
    }
    else if ((50030 > framerate) && (framerate > 49970))  // 50fps
    {
        FrameRate_mode = 0x50;
    }
    else if ((59970 > framerate) && (framerate > 59910))  // 59.94fps
    {
        FrameRate_mode = 0x60;
    }
    else if ((60030 > framerate) && (framerate > 59969))  // 60fps
    {
        FrameRate_mode = 0x60;
    }
    else
    {
        FrameRate_mode = 0xFF;
    }

    preFrameRate_mode = FrameRate_mode;

    return FrameRate_mode;
}

//                                                                          reduced blanking
// PC Frame Rate    50Hz    56Hz    60Hz    70Hz    72Hz    75Hz    85Hz    60Hz
//  640x480                         6460            6472    6475    6485
//  800x600                 8056    8060            8072    8075    8085
// 1024x768                         1060    1070            1075    1085
// 1280x960                         C960
// 1280x1024                        1260                    1275    1285
// 1360x768                         1360
// 1440x900RB                                                               1480+60
// 1440x900                         1460                    1475
// 1680x1050RB                                                              1680+60
// 1680x1050                        1660
// 1600x1200                        1C60
// 1920x1080                        1960
// 1920x1200RB                                                              1980+60
void Set_mode(MMP_UINT16 Mode_number)
{
    //dbg_msg(SDK_MSG_TYPE_INFO, "Hsync_counter = 0x%x\n", Hsync_counter());
    //dbg_msg(SDK_MSG_TYPE_INFO, "Mode_number = 0x%x\n", Mode_number);

    if (!(Is_TV_mode()))
    {
        CAT9883_WriteI2C_Byte(0x94, 0x59);
        CAT9883_WriteI2C_Byte(0x96, 0x25);   //set to internal PLL on PC mode Clive 041008
    }

    switch (Mode_number)
    {
    case 0x0150:
        CAT9883_WriteI2C_ByteN(0x01, PC_576Px50, sizeof(PC_576Px50));
        CAT9883_InWidth       = 720;
        CAT9883_InHeight      = 576;
        CAT9883_InFrameRate   = 5000;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x0260:
        CAT9883_WriteI2C_ByteN(0x01, PC_480Px60, sizeof(PC_480Px60));
        CAT9883_InWidth       = 720;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 5994;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x0360:
        CAT9883_WriteI2C_ByteN(0x01, PC_720Px60, sizeof(PC_720Px60));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 720;
        CAT9883_InFrameRate   = 5994;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x0460:
        CAT9883_WriteI2C_ByteN(0x01, PC_1080ix60, sizeof(PC_1080ix60));
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 2997;
        CAT9883_InIsInterlace = 1;
        break;

    //---------------640x480----------------//
    case 0x6460:
        CAT9883_WriteI2C_ByteN(0x01, PC_640x480x60, sizeof(PC_640x480x60));
        CAT9883_InWidth       = 640;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x6472:
        CAT9883_WriteI2C_ByteN(0x01, PC_640x480x72, sizeof(PC_640x480x72));
        CAT9883_InWidth       = 640;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 7200;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x6475:
        CAT9883_WriteI2C_ByteN(0x01, PC_640x480x75, sizeof(PC_640x480x75));
        CAT9883_InWidth       = 640;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 7500;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x6485:
        CAT9883_WriteI2C_ByteN(0x01, PC_640x480x85, sizeof(PC_640x480x85));
        CAT9883_InWidth       = 640;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 8500;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------800x600----------------//
    case 0x8056:
        CAT9883_WriteI2C_ByteN(0x01, PC_800x600x56, sizeof(PC_800x600x56));
        CAT9883_InWidth       = 800;
        CAT9883_InHeight      = 600;
        CAT9883_InFrameRate   = 5600;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x8060:
        CAT9883_WriteI2C_ByteN(0x01, PC_800x600x60, sizeof(PC_800x600x60));
        CAT9883_InWidth       = 800;
        CAT9883_InHeight      = 600;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x8072:
        CAT9883_WriteI2C_ByteN(0x01, PC_800x600x72, sizeof(PC_800x600x72));
        CAT9883_InWidth       = 800;
        CAT9883_InHeight      = 600;
        CAT9883_InFrameRate   = 7200;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x8075:
        CAT9883_WriteI2C_ByteN(0x01, PC_800x600x75, sizeof(PC_800x600x75));
        CAT9883_InWidth       = 800;
        CAT9883_InHeight      = 600;
        CAT9883_InFrameRate   = 7500;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x8085:
        CAT9883_WriteI2C_ByteN(0x01, PC_800x600x85, sizeof(PC_800x600x85));
        CAT9883_InWidth       = 800;
        CAT9883_InHeight      = 600;
        CAT9883_InFrameRate   = 8500;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1024x768---------------//
    case 0x1060:
        CAT9883_WriteI2C_ByteN(0x01, PC_1024x768x60, sizeof(PC_1024x768x60));
        CAT9883_InWidth       = 1024;
        CAT9883_InHeight      = 768;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x1070:
        CAT9883_WriteI2C_ByteN(0x01, PC_1024x768x70, sizeof(PC_1024x768x70));
        CAT9883_InWidth       = 1024;
        CAT9883_InHeight      = 768;
        CAT9883_InFrameRate   = 7000;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x1075:
        CAT9883_WriteI2C_ByteN(0x01, PC_1024x768x75, sizeof(PC_1024x768x75));
        CAT9883_InWidth       = 1024;
        CAT9883_InHeight      = 768;
        CAT9883_InFrameRate   = 7500;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x1085:
        CAT9883_WriteI2C_ByteN(0x01, PC_1024x768x85, sizeof(PC_1024x768x85));
        CAT9883_InWidth       = 1024;
        CAT9883_InHeight      = 768;
        CAT9883_InFrameRate   = 8500;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1280x800---------------//        Clive 0407
    case 0xC860:
        CAT9883_WriteI2C_ByteN(0x01, PC_1280x800x60, sizeof(PC_1280x800x60));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 800;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1280x960---------------//
    case 0xC960:
        CAT9883_WriteI2C_ByteN(0x01, PC_1280x960x60, sizeof(PC_1280x960x60));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 960;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1280x1024---------------//
    case 0x1260:
        CAT9883_WriteI2C_ByteN(0x01, PC_1280x1024x60, sizeof(PC_1280x1024x60));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 1024;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x1275:
        CAT9883_WriteI2C_ByteN(0x01, PC_1280x1024x75, sizeof(PC_1280x1024x75));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 1024;
        CAT9883_InFrameRate   = 7500;
        CAT9883_InIsInterlace = 0;
        break;

    case 0x1285:
        CAT9883_WriteI2C_ByteN(0x01, PC_1280x1024x85, sizeof(PC_1280x1024x85));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 1024;
        CAT9883_InFrameRate   = 8500;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1360x768---------------//
    case 0x1360:
        CAT9883_WriteI2C_ByteN(0x01, PC_1360x768x60, sizeof(PC_1360x768x60));
        CAT9883_InWidth       = 1360;
        CAT9883_InHeight      = 768;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1440x900---------------//
    case 0x1460:
        CAT9883_WriteI2C_ByteN(0x01, PC_1440x900x60, sizeof(PC_1440x900x60));
        CAT9883_InWidth       = 1440;
        CAT9883_InHeight      = 900;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        //---------------1680x1050---------------//
        break;

    case 0x1660:
        CAT9883_WriteI2C_ByteN(0x01, PC_1680x1050x60, sizeof(PC_1680x1050x60));
        CAT9883_InWidth       = 1680;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1600x1200---------------//
    case 0x1C60:
        CAT9883_WriteI2C_ByteN(0x01, PC_1600x1200x60, sizeof(PC_1600x1200x60));
        CAT9883_InWidth       = 1600;
        CAT9883_InHeight      = 1200;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1920x1200---------------//
    case 0x19e0:
        CAT9883_WriteI2C_ByteN(0x01, PC_1920x1200xReduce, sizeof(PC_1920x1200xReduce));
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1200;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    //---------------1920x1080---------------//
    case 0x1960:
        CAT9883_WriteI2C_ByteN(0x01, PC_1920x1080x60, sizeof(PC_1920x1080x60));
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 6000;
        CAT9883_InIsInterlace = 0;
        break;

    // TV Frame Rate    50Hz    56Hz    60Hz
    //  480i                            a450
    //  576i            a550
    //  480P                            b460
    // 1080i            a150            a160
    //  576P            b550
    //  720P            b750            b760
    // 1080P                            b160
    case 0xa460:
        CAT9883_WriteI2C_ByteN(0x01, TV_480ix60, sizeof(TV_480ix60));
        CAT9883_InWidth       = 720;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 2997;
        CAT9883_InIsInterlace = 1;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 480I60--------\n");
        break;

    case 0xa550:
        CAT9883_WriteI2C_ByteN(0x01, TV_576ix50, sizeof(TV_576ix50));
        CAT9883_InWidth       = 720;
        CAT9883_InHeight      = 576;
        CAT9883_InFrameRate   = 2500;
        CAT9883_InIsInterlace = 1;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 576I50--------\n");
        break;

    case 0xb460:
        CAT9883_WriteI2C_ByteN(0x01, TV_480Px60, sizeof(TV_480Px60));
        CAT9883_InWidth       = 720;
        CAT9883_InHeight      = 480;
        CAT9883_InFrameRate   = 5994;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 480P60--------\n");
        break;

    case 0xa150:
        CAT9883_WriteI2C_ByteN(0x01, TV_1080ix50, sizeof(TV_1080ix50));
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 2500;
        CAT9883_InIsInterlace = 1;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 1080I50--------\n");
        break;

    case 0xa160:
        CAT9883_WriteI2C_ByteN(0x01, TV_1080ix60, sizeof(TV_1080ix60));
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 2997;
        CAT9883_InIsInterlace = 1;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 1080I60--------\n");
        break;

    case 0xb550:
        CAT9883_WriteI2C_ByteN(0x01, TV_576Px50, sizeof(TV_576Px50));
        CAT9883_InWidth       = 720;
        CAT9883_InHeight      = 576;
        CAT9883_InFrameRate   = 5000;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 576P50--------\n");
        break;

    case 0xb750:
        CAT9883_WriteI2C_ByteN(0x01, TV_720Px50, sizeof(TV_720Px50));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 720;
        CAT9883_InFrameRate   = 5000;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 720P50--------\n");
        break;

    case 0xb760:
        CAT9883_WriteI2C_ByteN(0x01, TV_720Px60, sizeof(TV_720Px60));
        CAT9883_InWidth       = 1280;
        CAT9883_InHeight      = 720;
        CAT9883_InFrameRate   = 5994;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 720P60--------\n");
        break;

    case 0xb124:
        CAT9883_WriteI2C_ByteN(0x01, TV_1080Px24, sizeof(TV_1080Px24));       // CC 0511
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 2400;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 1080P24--------\n");
        break;

    case 0xb150:
        CAT9883_WriteI2C_ByteN(0x01, TV_1080Px50, sizeof(TV_1080Px50));       // CC 0511
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 5000;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 1080P50--------\n");
        break;

    case 0xb160:
        CAT9883_WriteI2C_ByteN(0x01, TV_1080Px60, sizeof(TV_1080Px60));
        CAT9883_InWidth       = 1920;
        CAT9883_InHeight      = 1080;
        CAT9883_InFrameRate   = 5994;
        CAT9883_InIsInterlace = 0;
        dbg_msg(SDK_MSG_TYPE_INFO, "--------CAT9883 TV Mode Resolution = 1080P60--------\n");
        break;

    default:
        CAT9883_WriteI2C_ByteN(0x01, Default_ini, sizeof(Default_ini));
        CAT9883_InWidth       = 0;
        CAT9883_InHeight      = 0;
        CAT9883_InFrameRate   = 0;
        CAT9883_InIsInterlace = 0;
        break;
    }

#ifdef CAT9883_OUTPUT_YUV422
    CAT9883_WriteI2c_ByteMask(0x15, 0x0000, 0x0002);
#else
    CAT9883_WriteI2c_ByteMask(0x15, 0x0002, 0x0002);
#endif

    //Set PLL
    if (Is_TV_mode())
    {
        switch (Mode_number)
        {
        case 0xb160:
            Reg_Pair_initial(TV1080pini);             // Add 1080P with internal PLL
            break;

        case 0xb150:
            Reg_Pair_initial(TV1080pini);             // Add 1080P with internal PLL
            break;

        case 0xb124:
            Reg_Pair_initial(TV1080p24ini);
            break;

        case 0xa150:
            Reg_Pair_initial(TV1080iini);
            break;

        case 0xa160:
            Reg_Pair_initial(TV1080iini);
            break;

        case 0xb750:
            Reg_Pair_initial(TV720pini);
            break;

        case 0xb760:
            Reg_Pair_initial(TV720pini);
            break;

        case 0xa550:
            Reg_Pair_initial(TV576iini);
            break;

        case 0xb550:
            Reg_Pair_initial(TV576pini);
            break;

        case 0xa460:
            Reg_Pair_initial(TV480iini);
            break;

        case 0xb460:
            Reg_Pair_initial(TV480pini);
            break;

        default:
            Reg_Pair_initial(TVdefaultinit);
            break;
        }
        MMP_Sleep(10);
    }

    gtCAT9883CurMode = Mode_number;
}

void CAT9883_PowerDown(
    MMP_BOOL enable)
{
    if (enable)
        CAT9883_WriteI2c_ByteMask(0x0F, 0x00, 0x02);
    else
        CAT9883_WriteI2c_ByteMask(0x0F, 0x02, 0x02);
}

MMP_BOOL CAT9883_IsNTSCorPAL(
    void)
{
    if (Is_TV_mode())
    {
        MMP_UINT16 mode;

        mode = TV_mode(MMP_FALSE);

        if (((mode & 0xFF00) == 0xa400) ||
            ((mode & 0xFF00) == 0xa500))
            return MMP_TRUE;
        else
            return MMP_FALSE;
    }
    else
        return MMP_FALSE;
}