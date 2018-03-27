#include "mmp_types.h"
#include "iic/iic.h"
#include "mmp_iic.h"
#include "i2s/transceiver_wm8804.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static MMP_UINT8 WM8804_IICADDR = 0x74 >> 1;

static MMP_BOOL  bInited        = MMP_FALSE;
//=============================================================================
//                Private Function Definition
//=============================================================================
static MMP_RESULT
_WOLFSON_WriteI2C_8Bit(
    MMP_UINT8 RegAddr,
    MMP_UINT8 data)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8  *pdbuf = dbuf;

    *pdbuf++ = (MMP_UINT8)(RegAddr & 0xff);
    *pdbuf++ = (MMP_UINT8)(data & 0xff);

    mmpIicLockModule();
    if (0 != (result = IIC_SendData(WM8804_IICADDR, dbuf, 2, W_STOP)))
    {
        printf("WOLFSON_WriteI2C_8Bit I2c Write Error, reg=%04x val=%04x\n", RegAddr, data);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();

    return result;
}

static MMP_UINT8
_WOLFSON_ReadI2C_8Bit(
    MMP_UINT8 RegAddr)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8  *pdbuf = dbuf;
    MMP_UINT32 value;

    *pdbuf++ = (MMP_UINT8)(RegAddr & 0xff);

    mmpIicLockModule();

    result = IIC_SendData(WM8804_IICADDR, dbuf, 1, WO_STOP);
    if (result == MMP_RESULT_SUCCESS)
        result = IIC_ReceiveData(WM8804_IICADDR, pdbuf, 1);

    value = (dbuf[1] & 0xFF);

    mmpIicReleaseModule();

    return value;
}

static void
_WOLFSON_WM8804_Initialize(
    void)
{
    //Fs = 48KHz
    if (bInited)
    {
        return;
    }

    //BCLK = 48Khz, MCLK = 256 * 48k = 12.288MHz
    _WOLFSON_WriteI2C_8Bit(0x00, 0x00); //Software reset

    _WOLFSON_WriteI2C_8Bit(0x1E, 0x02); //Enable PLL, Enable SPDIFTX, EnableOSC, AIF and outputs. SPDIFRX disabled
    _WOLFSON_WriteI2C_8Bit(0x1C, 0x42); //AIF in Master Mode - LRCLK and BCLK are outputs. AIFRX Word Length = 16 bits. AIFRX Format = I2S mode.
    _WOLFSON_WriteI2C_8Bit(0x03, 0xBA); //PLL_K
    _WOLFSON_WriteI2C_8Bit(0x04, 0x49);
    _WOLFSON_WriteI2C_8Bit(0x05, 0x0C);
    _WOLFSON_WriteI2C_8Bit(0x06, 0x08); //PLL_N = 8, PreScale = 0
    _WOLFSON_WriteI2C_8Bit(0x04, 0x49);
    _WOLFSON_WriteI2C_8Bit(0x05, 0x0C);
    _WOLFSON_WriteI2C_8Bit(0x06, 0x08); //PLL_N = 8, PreScale = 0
    _WOLFSON_WriteI2C_8Bit(0x04, 0x49);
    _WOLFSON_WriteI2C_8Bit(0x05, 0x0C);
    _WOLFSON_WriteI2C_8Bit(0x06, 0x08); //PLL_N = 8, PreScale = 0
    _WOLFSON_WriteI2C_8Bit(0x07, 0x06); //FreqMode = 2, MclkDiv = 0, ClkOut = 0
    _WOLFSON_WriteI2C_8Bit(0x08, 0x18); //MCLKSRC=0, ALWAYSVALID=0, FILLMODE=0, CLKOUTDIS=1, CLKOUTSRC=1
    _WOLFSON_WriteI2C_8Bit(0x16, 0x02); //SPDIF_TX Word Length = 16 bits

    //BCLK = 44.1Khz, MCLK = 256 * 44.1k = 11.289MHz
    //_WOLFSON_WriteI2C_8Bit(0x00, 0x00); //Software reset
    //_WOLFSON_WriteI2C_8Bit(0x1E, 0x02); //Enable PLL, Enable SPDIFTX, EnableOSC, AIF and outputs. SPDIFRX disabled
    //_WOLFSON_WriteI2C_8Bit(0x1C, 0x42); //AIF in Master Mode - LRCLK and BCLK are outputs. AIFRX Word Length = 16 bits. AIFRX Format = I2S mode.
    //_WOLFSON_WriteI2C_8Bit(0x03, 0x89); //PLL_K
    //_WOLFSON_WriteI2C_8Bit(0x04, 0xB0);
    //_WOLFSON_WriteI2C_8Bit(0x05, 0x21);
    //_WOLFSON_WriteI2C_8Bit(0x06, 0x07); //PLL_N = 7, PreScale = 0
    //_WOLFSON_WriteI2C_8Bit(0x04, 0xB0);
    //_WOLFSON_WriteI2C_8Bit(0x05, 0x21);
    //_WOLFSON_WriteI2C_8Bit(0x06, 0x07); //PLL_N = 7, PreScale = 0
    //_WOLFSON_WriteI2C_8Bit(0x04, 0xB0);
    //_WOLFSON_WriteI2C_8Bit(0x05, 0x21);
    //_WOLFSON_WriteI2C_8Bit(0x06, 0x07); //PLL_N = 7, PreScale = 0
    //_WOLFSON_WriteI2C_8Bit(0x07, 0x06); //FreqMode = 2, MclkDiv = 0, ClkOut = 0
    //_WOLFSON_WriteI2C_8Bit(0x08, 0x18); //MCLKSRC=0, ALWAYSVALID=0, FILLMODE=0, CLKOUTDIS=1, CLKOUTSRC=1
    //_WOLFSON_WriteI2C_8Bit(0x16, 0x02); //SPDIF_TX Word Length = 16 bits

    bInited = MMP_TRUE;
    printf("WM8804# init_wm8804_common 0x%x\n", _WOLFSON_ReadI2C_8Bit(0x01));
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void
mmpWM8804Initialize(
    void)
{
    //kenny patch 20140617
    if (!bInited)
    {
        extern MMP_BOOL GpioInputselect(MMP_BOOL status);
        GpioInputselect(MMP_FALSE);
    }

    _WOLFSON_WM8804_Initialize();
}

void
mmpWM8804TriState(
    void)
{
    //kenny patch 20140617
    if (bInited)
    {
        extern MMP_BOOL GpioInputselect(MMP_BOOL status);
        GpioInputselect(MMP_TRUE);
    }

    printf("wm8804 tri state\r\n");
    _WOLFSON_WriteI2C_8Bit(0x1E, 0x3F); //Tri-State All Output and power down
    bInited = MMP_FALSE;
}