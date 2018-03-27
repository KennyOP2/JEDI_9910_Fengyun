#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"
#include "mmp_iic.h"
#include "host/gpio.h"
#include "host/ahb.h"
#include "iic/iic.h"
#include "adv7180/adv7180.h"

//=============================================================================
//                Constant Definition
//=============================================================================
static MMP_UINT8 ADV7180_IICADDR = 0x42 >> 1;

#define POWER_MANAGEMENT 0x0F
#define REG_STATUS1      0x10
#define REG_IDENT        0x11
#define REG_STATUS2      0x12
#define REG_STATUS3      0x13

#define RESET_MASK       (1 << 7)
#define TRI_STATE_ENABLE (1 << 7)

//#define AUTO_DETECT_INPUT

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _REGPAIR
{
    MMP_UINT8 addr;
    MMP_UINT8 value;
} REGPAIR;

typedef enum _INPUT_TYPE
{
    _NTSM_M_J = 0,
    _NTSC_4_43,
    _PAL_M,
    _PAL_60,
    _PAL_B_G_H_I_D,
    _SECAM,
    _PAL_COMBINATION_N,
    _SECAM_525
} INPUT_TYPE;

//=============================================================================
//                Global Data Definition
//=============================================================================

static MMP_UINT16 gtADV7180CurMode   = 0xFF;
static MMP_UINT16 gtADV7180PreMode   = 0xFF;
static MMP_UINT16 gtADV7180CurDev    = 0xFF;
static MMP_BOOL   gtADV7180InitDone  = MMP_FALSE;
static MMP_BOOL   gtADV7180PowerDown = MMP_FALSE;

/* 32-Lead LFCSP , page 108*/
static REGPAIR    CVBS_INPUT[]       =
{
    {0x00, 0x04}, //AIN3
    {0x04, 0x54},
    {0x17, 0x41},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x01},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},

    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
};

static REGPAIR    SVIDEO_INPUT [] =
{
    {0x00, 0x06}, //AIN1 AIN2
    {0x04, 0x54},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x05},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},

    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
};

static REGPAIR    YPrPb_INPUT [] =
{
    {0x00, 0x09},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x01},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},

    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
};

//=============================================================================
//                Private Function Definition
//=============================================================================
static MMP_UINT8 _ADV7180_ReadI2c_Byte(MMP_UINT8 RegAddr)
{
    MMP_UINT8  value;
    MMP_RESULT flag;

    mmpIicLockModule();
    if (0 != (flag = mmpIicReceiveData(IIC_MASTER_MODE, ADV7180_IICADDR, RegAddr, &value, 1)))
    {
        printf("ADV7180 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return value;
}

static MMP_RESULT _ADV7180_WriteI2c_Byte(MMP_UINT8 RegAddr, MMP_UINT8 data)
{
    MMP_RESULT flag;

    mmpIicLockModule();
    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, ADV7180_IICADDR, RegAddr, &data, 1)))
    {
        printf("ADV7180 I2c write error, reg = %02x val =%02x\n", RegAddr, data);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;
}

static MMP_RESULT _ADV7180_WriteI2c_ByteMask(MMP_UINT8 RegAddr, MMP_UINT8 data, MMP_UINT8 mask)
{
    MMP_UINT8  Value;
    MMP_RESULT flag;

    mmpIicLockModule();

    if (0 != (flag = mmpIicReceiveData(IIC_MASTER_MODE, ADV7180_IICADDR, RegAddr, &Value, 1)))
    {
        printf("ADV7180 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop();
    }

    Value = ((Value & ~mask) | (data & mask));

    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, ADV7180_IICADDR, RegAddr, &Value, 1)))
    {
        printf("ADV7180 I2c write error, reg = %02x val =%02x\n", RegAddr, Value);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;
}

static void _ADV7180_SWReset()
{
    MMP_UINT8 value;
    MMP_UINT8 dbuf[2];
    MMP_UINT8 *pdbuf = dbuf;

    value    = _ADV7180_ReadI2c_Byte(POWER_MANAGEMENT);
    value   |= RESET_MASK;

    mmpIicLockModule();
    *pdbuf++ = POWER_MANAGEMENT;
    SYS_Memcpy(pdbuf, &value, 1);
    IIC_SendData(ADV7180_IICADDR, dbuf, 2, W_STOP);
    mmpIicGenStop();
    mmpIicReleaseModule();
}

static void _Set_ADV7180_Input_CVBS(void)
{
    MMP_UINT16 i;
    MMP_UINT8  Value;

    for (i = 0; i < (sizeof(CVBS_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(CVBS_INPUT[i].addr, CVBS_INPUT[i].value);

    _ADV7180_WriteI2c_ByteMask(0x4, 0x80, 0x80);
}

static void _Set_ADV7180_Input_SVIDEO(void)
{
    MMP_UINT16 i;

    for (i = 0; i < (sizeof(SVIDEO_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(SVIDEO_INPUT[i].addr, SVIDEO_INPUT[i].value);
}

static void _Set_ADV7180_Input_YPrPb(void)
{
    MMP_UINT16 i;

    for (i = 0; i < (sizeof(YPrPb_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(YPrPb_INPUT[i].addr, YPrPb_INPUT[i].value);
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void Set_ADV7180_Tri_State_Enable()
{
    //LLC pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x1D, 0x80, 0x80);

    //TIM_OE pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x08);

    //SFL Pin Disable (DE)
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x02);

    //HS, VS, FIELD Data pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x03, 0x40, 0x40);
}

void Set_ADV7180_Tri_State_Disable()
{
    //LLC pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x1D, 0x00, 0x80);

    //TIM_OE pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x04, 0x08, 0x08);

    //SFL Pin Disable (DE)
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x02);

    //HS, VS, FIELD Data pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x03, 0x00, 0x40);
}

ADV7180_INPUT_STANDARD Get_Auto_Detection_Result()
{
    MMP_UINT8 result;

    result = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    result = (result & 0x70) >> 4;
    //kenny patch 20140428  YK  480i change to576i,display abnormal
    if (result == 1)
        PalSleep(1000);
    result = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    result = (result & 0x70) >> 4;

    switch (result)
    {
    case ADV7180_NTSM_M_J:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 480;
        ADV7180_InFrameRate = 2997;
        gtADV7180CurMode    = ADV7180_NTSM_M_J;
        //printf("NTSM M/J\n");
        break;

    case ADV7180_NTSC_4_43:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 480;
        ADV7180_InFrameRate = 2997;
        gtADV7180CurMode    = ADV7180_NTSC_4_43;
        //printf("NTSC 4.43\n");
        break;

    //case ADV7180_PAL_M:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 2997;
    //  gtADV7180CurMode = ADV7180_PAL_M;
    //  printf("PAL_M\n");
    //  break;
    //case ADV7180_PAL_60:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 3000;
    //  gtADV7180CurMode = ADV7180_PAL_60;
    //  printf("PAL_60\n");
    //  break;
    case ADV7180_PAL_B_G_H_I_D:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 576;
        ADV7180_InFrameRate = 2500;
        gtADV7180CurMode    = ADV7180_PAL_B_G_H_I_D;
        //printf("PAL B/G/H/I/D\n");
        break;

    //case ADV7180_SECAM:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 576;
    //  ADV7180_InFrameRate = 2500;
    //  gtADV7180CurMode = ADV7180_SECAM;
    //  printf("SECAM\n");
    //  break;
    //case ADV7180_PAL_COMBINATION_N:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 576;
    //  ADV7180_InFrameRate = 2500;
    //  gtADV7180CurMode = ADV7180_PAL_COMBINATION_N;
    //  printf("PAL Combination N\n");
    //  break;
    //case ADV7180_SECAM_525:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 2997;
    //  gtADV7180CurMode = ADV7180_SECAM_525;
    //  printf("SECAM 525\n");
    //  break;
    default:
        printf("Can not recognize\n");
        break;
    }
    return result;
}

MMP_UINT16 _ADV7180_InputSelection()
{
#ifdef AUTO_DETECT_INPUT
    MMP_UINT16 Value;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    if ((Value & 0x05) != 0x05)
    {
        Value = _ADV7180_ReadI2c_Byte(0x00);
        if ((Value & 0x0F) == 0x06)
        {
            ADV7180_Input_Mode(ADV7180_INPUT_CVBS);
            return ADV7180_INPUT_CVBS;
        }
        else
        {
            ADV7180_Input_Mode(ADV7180_INPUT_SVIDEO);
            return ADV7180_INPUT_SVIDEO;
        }
    }
    else
    {
        Value = _ADV7180_ReadI2c_Byte(0x00);
        if ((Value & 0x0F) == 0x06)
            return ADV7180_INPUT_SVIDEO;
        else
            return ADV7180_INPUT_CVBS;
    }
#else
    MMP_UINT16 Value;
    Value = _ADV7180_ReadI2c_Byte(0x00);
    if ((Value & 0x0F) == 0x06)
        return ADV7180_INPUT_SVIDEO;
    else
        return ADV7180_INPUT_CVBS;
#endif
}

void ADV7180Initial(ADV7180_INPUT_MODE mode)
{
    gtADV7180InitDone  = MMP_FALSE;
    _ADV7180_SWReset();
    gtADV7180PowerDown = MMP_FALSE;

    gtADV7180CurMode   = 0xFF;
    gtADV7180PreMode   = 0xFF;

    PalSleep(100);

    ADV7180_Input_Mode(mode);
    gtADV7180CurDev   = mode;
    gtADV7180InitDone = MMP_TRUE;
}

void ADV7180_Input_Mode(ADV7180_INPUT_MODE mode)
{
    if (mode == ADV7180_INPUT_CVBS)
        _Set_ADV7180_Input_CVBS();
    else if (mode == ADV7180_INPUT_SVIDEO)
        _Set_ADV7180_Input_SVIDEO();
    else if (mode == ADV7180_INPUT_YPBPR)
        _Set_ADV7180_Input_YPrPb();

    //Y Range 16 - 235, UV Range 16 - 240
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x01);

    //Lock status set by horizontal lock and subcarrier lock
    //_ADV7180_WriteI2c_ByteMask(0x51, 0x80, 0x80);

    //drive strength
    _ADV7180_WriteI2c_Byte(0xF4, 0x04);
}

MMP_BOOL ADV7180_IsStable()
{
    MMP_UINT16 Value, Value2;
    MMP_UINT16 IsStable;

    if (!gtADV7180InitDone || gtADV7180PowerDown)
        return MMP_FALSE;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);

    //if (((Value & 0x85) == 0x05) || ((Value & 0x85) == 0x81)) //Color Burst or No Color Burst
    if ((Value & 0x85) == 0x05) //Color Burst for hauppauge
    {
        Get_Auto_Detection_Result();

        if (gtADV7180CurMode != gtADV7180PreMode)
        {
            dbg_msg(SDK_MSG_TYPE_INFO, "--------ADV7180 Resolution = ");
            if     (gtADV7180CurMode == ADV7180_NTSM_M_J)           dbg_msg(SDK_MSG_TYPE_INFO, "NTSM_M_J");
            else if(gtADV7180CurMode == ADV7180_NTSC_4_43)          dbg_msg(SDK_MSG_TYPE_INFO, "NTSC_4_43");
            else if(gtADV7180CurMode == ADV7180_PAL_M)              dbg_msg(SDK_MSG_TYPE_INFO, "PAL_M");
            else if(gtADV7180CurMode == ADV7180_PAL_60)             dbg_msg(SDK_MSG_TYPE_INFO, "PAL_60");
            else if(gtADV7180CurMode == ADV7180_PAL_B_G_H_I_D)      dbg_msg(SDK_MSG_TYPE_INFO, "PAL_B_G_H_I_D");
            else if(gtADV7180CurMode == ADV7180_SECAM)              dbg_msg(SDK_MSG_TYPE_INFO, "SECAM");
            else if(gtADV7180CurMode == ADV7180_PAL_COMBINATION_N)  dbg_msg(SDK_MSG_TYPE_INFO, "PAL_COMBINATION_N");
            else if(gtADV7180CurMode == ADV7180_SECAM_525)          dbg_msg(SDK_MSG_TYPE_INFO, "SECAM_525");
            else                                                    dbg_msg(SDK_MSG_TYPE_INFO, "Unknow Format");

            if (gtADV7180CurDev == ADV7180_INPUT_CVBS)
                dbg_msg(SDK_MSG_TYPE_INFO, "----CVBS ---------\n");
            else if (gtADV7180CurDev == ADV7180_INPUT_SVIDEO)
                dbg_msg(SDK_MSG_TYPE_INFO, "----S-Video ------\n");

            gtADV7180PreMode = gtADV7180CurMode;
        }
        IsStable = MMP_TRUE;
    }
    else
    {
        gtADV7180CurDev = _ADV7180_InputSelection();
        IsStable        = MMP_FALSE;
    }

    return IsStable;
}

void ADV7180_PowerDown(
    MMP_BOOL enable)
{
    //When PDBP is set to 1, setting the PWRDWN bit switches the ADV7180 to a chip-wide power-down mode.
    _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x04, 0x04);

    if (enable)
    {
        gtADV7180PowerDown = MMP_TRUE;
        _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x20, 0x20);
    }
    else
    {
        gtADV7180PowerDown = MMP_FALSE;
        _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x00, 0x20);
    }
}

MMP_BOOL ADV7180_IsSVideoInput()
{
    MMP_UINT16 Value;

    if (!gtADV7180InitDone || gtADV7180PowerDown)
        return MMP_FALSE;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);

    if ((Value & 0x85) == 0x05) //Color Burst for hauppauge
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

MMP_BOOL ADV7180_IsPowerDown()
{
    return gtADV7180PowerDown;
}