/*
 * Copyright (c) 2005 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  I2C API functoin file.
 *      Date: 2005/10/7
 *
 * @author Alex.C Hsieh
 * @version 0.95
 */

#if defined(__FREERTOS__)
    #include "FreeRTOS.h"
    #include "task.h"
#endif

#include "sys/sys.h"
#include "mmp.h"
#include "host/ahb.h"
#include "host/host.h"
#include "host/gpio.h"
#include "mmp_iic.h"
#include "iic.h"
#include "i2c_hwreg.h"
#include "pal/pal.h"
#include <stdio.h>
//=============================================================================
//                              Extern Reference
//=============================================================================
#ifdef _WIN32
static MMP_UINT32 gpio_iic = DGPIO(11); //DGPIO(2); // DGPIO(11)
#else
extern MMP_UINT32 gpio_iic;
#endif
//=============================================================================
//                              Macro Definition
//=============================================================================
#define TIMEOUT                0x20000 //0X10000
#define CLK_1M                 1024 * 1024
#define CLK_100K               (100 * 1024)
#define IIC_MAX_SIZE           256
#define SEM_ENABLE             0
#define TASK_SUSPEND_ALL       0
#define MMP_MAX_IIC_DEVICE_NUM 0x10
#define CLK_32K                32768
#define REF_CLOCK              40000000 // 36M
//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct IIC_DEVICE_TAG IIC_DEVICE;

typedef void (*pIIC_EnableTransfer)(
    MMP_BOOL enableTransfer,
    IIC_DEVICE *iic_dev);

struct IIC_DEVICE_TAG
{
    MMP_BOOL            inuse;
    MMP_GPIO_GROUP      gpio_group;
    MMP_ULONG           sclk;
    MMP_ULONG           data;
    MMP_ULONG           delay;
    pIIC_EnableTransfer iic_enable_device;
};

typedef enum IIC_STATE_FLAG_TAG
{
    TRANSMIT_DATA    = 0x1,
    RECEIVE_DATA     = 0x2,
    TRANSMIT_SERVICE = 0x4,
    RECRIVE_SERVICE  = 0x8,
    NACK_EN          = 0x10
} IIC_STATE_FLAG;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static IIC_DEVICE         g_IIC_Device[MMP_MAX_IIC_DEVICE_NUM];
static MMP_UINT           g_IIC_DeviceCount  = 0;
static MMP_UINT           g_IIC_SemaphoreRef = 0;
static void               *g_IIC_Semaphore   = NULL;
static MMP_UINT32         gCurrentClock;
static IIC_HW_CONFIG_MODE gHwMode;
static void               *gIicModuleMutex   = MMP_NULL;
//=============================================================================
//                              Function Definition
//=============================================================================
void
_IIC_Null(
    MMP_BOOL enableTransfer,
    IIC_DEVICE *iic_dev)
{
    // This is a null function.
}

void
_IIC_EnableDevice(
    MMP_BOOL enableTransfer,
    IIC_DEVICE *iic_dev)
{
    // [20100106] Remove by Vincent
    //mmpEnableGpio(enableTransfer, iic_dev->gpio_group, iic_dev->sclk|iic_dev->data);
}

void
_IIC_Delay(
    MMP_ULONG loop)
{
    while (loop)
        loop--;
    return;
}

MMP_RESULT
IIC_WaitDeviceAck(
    IIC_DEVICE *iic_dev)
{
    MMP_UINT ack;

    // Set to input mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE, iic_dev->gpio_group, iic_dev->data);

#if defined (MMP_AUDIO_CODEC_WM8728) || defined(MMP_AUDIO_CODEC_WM8778) || defined(MMP_AUDIO_CODEC_WM8750BL)
    _IIC_Delay(iic_dev->delay); //liang

    // [20100125] Remove by Vincent
    //mmpGetGpioState(iic_dev->gpio_group, iic_dev->data, &ack); //liang
#else
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->data, GPIO_PULL_HI(iic_dev->data));
#endif // defined(MMP_AUDIO_CODEC_WM8728) || defined(MMP_AUDIO_CODEC_WM8778) || defined(MMP_AUDIO_CODEC_WM8750BL)

    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_LO(iic_dev->sclk));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_HI(iic_dev->sclk));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpGetGpioState(iic_dev->gpio_group, iic_dev->data, &ack);
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_LO(iic_dev->sclk));

    // Set to input mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE, iic_dev->gpio_group, iic_dev->data);

    if ((ack & iic_dev->data) == 0)
        return MMP_RESULT_SUCCESS;

    return MMP_RESULT_ERROR;
}

MMP_RESULT
IIC_SendDeviceAck(
    IIC_DEVICE *iic_dev)
{
    // Set to output mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_OUTPUT_MODE, iic_dev->gpio_group, iic_dev->data);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_LO(iic_dev->sclk));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->data, GPIO_PULL_LO(iic_dev->data));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_HI(iic_dev->sclk));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_LO(iic_dev->sclk));

    // Set to input mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE, iic_dev->gpio_group, iic_dev->data);

    return MMP_RESULT_SUCCESS;
}

void
IIC_StartCondition(
    IIC_DEVICE *iic_dev)
{
    // if device pins share with host, enable pins to connect to device.
    iic_dev->iic_enable_device(MMP_TRUE, iic_dev);
    _IIC_Delay(iic_dev->delay);

    // SCLK/DATA pull high
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk|iic_dev->data, GPIO_PULL_HI(iic_dev->sclk)|GPIO_PULL_HI(iic_dev->data));
    _IIC_Delay(iic_dev->delay);

    // DATA pull low
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->data, GPIO_PULL_LO(iic_dev->data));
    _IIC_Delay(iic_dev->delay);

    // SCLK pull low
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_LO(iic_dev->sclk));
}

void
IIC_StopCondition(
    IIC_DEVICE *iic_dev)
{
    // SCLK/DATA pull low
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk|iic_dev->data, GPIO_PULL_LO(iic_dev->sclk)|GPIO_PULL_LO(iic_dev->data));
    _IIC_Delay(iic_dev->delay);
    // SCLK pull high

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk, GPIO_PULL_HI(iic_dev->sclk));
    _IIC_Delay(iic_dev->delay);

    // DATA pull high
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->data, GPIO_PULL_HI(iic_dev->data));
    _IIC_Delay(iic_dev->delay);

    // if device pins share with host, disable pins, connect back to host.
    iic_dev->iic_enable_device(MMP_FALSE, iic_dev);
}

MMP_RESULT
IIC_SendDeviceWrite(
    IIC_DEVICE *iic_dev,
    MMP_UINT16 data)
{
    MMP_UINT i = 0;
    // send address
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80)
        {
            // [20100125] Remove by Vincent
            //mmpSetGpioState(iic_dev->gpio_group, iic_dev->data,GPIO_PULL_HI(iic_dev->data));
        }
        else
        {
            // [20100125] Remove by Vincent
            //mmpSetGpioState(iic_dev->gpio_group, iic_dev->data,GPIO_PULL_LO(iic_dev->data));
        }
        _IIC_Delay(iic_dev->delay);

        // CLK pull low, DATA prepare
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk,GPIO_PULL_LO(iic_dev->sclk));
        _IIC_Delay(iic_dev->delay);

        // CLK pull high, DATA transfer
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk,GPIO_PULL_HI(iic_dev->sclk));
        _IIC_Delay(iic_dev->delay);

        // CLK pull low, DATA ready
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk,GPIO_PULL_LO(iic_dev->sclk));
        _IIC_Delay(iic_dev->delay);
        data <<= 1;
    }

    if (IIC_WaitDeviceAck(iic_dev) != MMP_RESULT_SUCCESS)
    {
        MMP_DbgPrint(("Write to device error (NO ACK)"));
        return MMP_RESULT_ERROR;
    }
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
IIC_SendDeviceRead(
    IIC_DEVICE *iic_dev,
    MMP_UINT16 *data)
{
    MMP_UINT i        = 0;
    MMP_UINT readBack = 0x0000;
    for (i = 0; i < 8; i++)
    {
        // CLK pull high
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk,GPIO_PULL_HI(iic_dev->sclk));
        _IIC_Delay(iic_dev->delay);
        // [20100125] Remove by Vincent
        //mmpGetGpioState(iic_dev->gpio_group, iic_dev->data,&readBack);
        _IIC_Delay(iic_dev->delay);
        // CLK pull low
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->sclk,GPIO_PULL_LO(iic_dev->sclk));
        _IIC_Delay(iic_dev->delay);

        if (readBack & iic_dev->data)
            (*data) |= 1;

        (*data) <<= 1;
    }
    (*data) >>= 1;

    if (IIC_SendDeviceAck(iic_dev) != MMP_RESULT_SUCCESS)
    {
        MMP_DbgPrint(("Write to device error (NO ACK)"));
        return MMP_RESULT_ERROR;
    }
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
IIC_GetFreeDevice(
    MMP_IIC_HANDLE *hDevice)
{
    MMP_UINT   i      = 0;
    MMP_RESULT result = MMP_RESULT_ERROR;

    (*hDevice) = 0;
    for (i = 0; i < MMP_MAX_IIC_DEVICE_NUM; i++)
    {
        if (g_IIC_Device[i].inuse == MMP_FALSE)
        {
            *(hDevice) = i;
            result     = MMP_RESULT_SUCCESS;
            break;
        }
    }

    return result;
}

MMP_RESULT
IIC_SearchDevice(
    MMP_GPIO_GROUP gpio_group,
    MMP_GPIO_PIN sclk_pin,
    MMP_GPIO_PIN data_pin)
{
    MMP_UINT   i      = 0;
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    for (i = 0; i < MMP_MAX_IIC_DEVICE_NUM; i++)
    {
        if (g_IIC_Device[i].inuse == MMP_FALSE)
            continue;

        if (g_IIC_Device[i].data == data_pin)
        {
            result = MMP_RESULT_ERROR;
            break;
        }

        if (g_IIC_Device[i].sclk == sclk_pin)
        {
            result = MMP_RESULT_ERROR;
            break;
        }
    }

    return result;
}

MMP_RESULT
IIC_AddDevice(
    MMP_IIC_HANDLE *hDevice,
    MMP_GPIO_GROUP gpio_group,
    MMP_GPIO_PIN sclk_pin,
    MMP_GPIO_PIN data_pin,
    MMP_ULONG delay)
{
    MMP_RESULT result;
    SYS_WaitSemaphore(g_IIC_Semaphore);

    if (!hDevice)
        goto END;
    if ((result = IIC_SearchDevice(gpio_group, sclk_pin, data_pin)) != MMP_RESULT_SUCCESS)
        goto END;

    if ((result = IIC_GetFreeDevice(hDevice)) == MMP_RESULT_SUCCESS)
    {
        g_IIC_Device[(*hDevice)].inuse      = MMP_TRUE;
        g_IIC_Device[(*hDevice)].gpio_group = gpio_group;
        g_IIC_Device[(*hDevice)].sclk       = sclk_pin;
        g_IIC_Device[(*hDevice)].data       = data_pin;
        g_IIC_Device[(*hDevice)].delay      = delay;

        // [20100125] Remove by Vincent
        //if(gpio_group == MMP_SHARED_GPIO)
        //    g_IIC_Device[(*hDevice)].iic_enable_device = _IIC_EnableDevice;
        //else
        g_IIC_Device[(*hDevice)].iic_enable_device = _IIC_Null;

        g_IIC_DeviceCount++;
    }

END:
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
    return result;
}

MMP_RESULT
IIC_RemoveDevice(
    MMP_IIC_HANDLE hDevice)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    SYS_WaitSemaphore(g_IIC_Semaphore);

    if (g_IIC_Device[hDevice].inuse != MMP_TRUE)
    {
        result = MMP_RESULT_ERROR;
        goto END;
    }

    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE,
    //               g_IIC_Device[hDevice].gpio_group,
    //               g_IIC_Device[hDevice].sclk|g_IIC_Device[hDevice].data);

    g_IIC_Device[hDevice].inuse             = MMP_FALSE;
    g_IIC_Device[hDevice].gpio_group        = 0;
    g_IIC_Device[hDevice].sclk              = 0;
    g_IIC_Device[hDevice].data              = 0;
    g_IIC_Device[hDevice].delay             = 0;
    g_IIC_Device[hDevice].iic_enable_device = _IIC_Null;

    g_IIC_DeviceCount--;

END:
    SYS_ReleaseSemaphore(g_IIC_Semaphore);

    return result;
}

//=============================================================================
/**
 *  FARADAY FTIIC010
 *  Formula : SCLout = PCLK/(2*COUNT - GSR + 1)
 *
 *  PCLK is formed by REG_WCLK
 *  COUNT is REG_I2C_CLOCK_DIV[9:0]
 *  GSR is REG_I2C_GLITCH[12:10]
 */
//=============================================================================
static MMP_UINT32
IIC_SetClockRate(
    MMP_UINT32 clock)
{
    MMP_UINT32 pclk, div, glitch;
    MMP_UINT32 count, gsr;
    MMP_UINT16 data, pllNum;
    MMP_UINT32 pll;

#if defined(__OPENRTOS__)
    pclk = ithGetBusClock();
#elif defined(__FREERTOS__)
    pclk = or32_getBusCLK();
#else
    pclk = REF_CLOCK; //fpga extern clk
#endif

    AHB_ReadRegister(REG_I2C_GLITCH, &glitch);
    div           = (pclk / clock);
    gsr           = (glitch & REG_MASK_GSR) >> REG_SHIFT_GSR;
    count         = (((div - gsr - 4) / 2) & REG_MASK_CLK_DIV_COUNT); //count           = (((div + gsr -1)/2) & REG_MASK_CLK_DIV_COUNT) ;
    gCurrentClock = (MMP_UINT32)(pclk / (2 * count + gsr + 4));       //gCurrentClock   = (MMP_UINT32)(pclk/(2*count - gsr +1));

    //set i2c div
    AHB_WriteRegister(REG_I2C_CLOCK_DIV, count);
    return gCurrentClock;
}

static MMP_RESULT
IIC_CheckAck(
    IIC_STATE_FLAG state)
{
    MMP_UINT   i      = 0;
    MMP_UINT32 data;
    MMP_RESULT result = 0;

    while (++i)
    {
        AHB_ReadRegister(REG_I2C_STATUS, &data);

        if (state & TRANSMIT_DATA)
        {
            if (data & REG_BIT_STATUS_DATA_TRANSFER_DONE)
            {
                if (data & REG_BIT_STATUS_NON_ACK)
                {
                    result = I2C_NON_ACK;
                    goto end;
                }
                if (data & REG_BIT_STATUS_ARBITRATION_LOSS)
                {
                    result = I2C_ARBITRATION_LOSS;
                    goto end;
                }

                if (state & RECRIVE_SERVICE)
                {
                    if (!(data & REG_BIT_STATUS_RECEIVE_MODE))
                    {
                        result = I2C_MODE_TRANSMIT_ERROR;
                        goto end;
                    }
                }
                else
                {
                    if (data & REG_BIT_STATUS_RECEIVE_MODE)
                    {
                        result = I2C_MODE_TRANSMIT_ERROR;
                        goto end;
                    }
                }

                //no error and leave loop
                result = MMP_RESULT_SUCCESS;
                goto end;
            }
        }
        else if (state & RECEIVE_DATA)
        {
            if (data & REG_BIT_STATUS_DATA_RECEIVE_DONE)
            {
                if (state & NACK_EN)
                {
                    if (!(data & REG_BIT_STATUS_NON_ACK))
                    {
                        result = I2C_INVALID_ACK;
                        goto end;
                    }
                }
                else
                {
                    if (data & REG_BIT_STATUS_NON_ACK)
                    {
                        result = I2C_NON_ACK;
                        goto end;
                    }
                }

                if (!(data & REG_BIT_STATUS_RECEIVE_MODE))
                {
                    result = I2C_MODE_RECEIVE_ERROR;
                    goto end;
                }
                //no error and leave loop
                result = MMP_RESULT_SUCCESS;
                goto end;
            }
        }

        if (i > TIMEOUT)
        {
            result = I2C_WAIT_TRANSMIT_TIME_OUT;
            goto end;
        }
    }

end:
    return result;
}

static MMP_RESULT
IIC_Initialize(
    MMP_ULONG delay)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    MMP_UINT32 data;
#if 0 // add by Steven for 4 wire case
    HOST_ReadRegister(0x784A, &data);
    data |= 0x1;
    HOST_WriteRegister(0x784A, data);
#endif
    //Reset HW
    AHB_WriteRegister(REG_I2C_CONTROL, REG_BIT_CONTL_I2C_RESET);
    MMP_Sleep(5);
    AHB_WriteRegister(REG_I2C_CONTROL, 0u);

    IIC_SetClockRate(CLK_100K);
    if (delay)
    {
        AHB_ReadRegister(REG_I2C_GLITCH, &data);
        data &= ~(REG_MASK_TSR);
        data |= delay;
        // Setup GSR
        data |= (0x7 << 10);
        AHB_WriteRegister(REG_I2C_GLITCH, data);
    }
#if SEM_ENABLE
    //create semaphore
    if (g_IIC_Semaphore == MMP_NULL)
        g_IIC_Semaphore = SYS_CreateSemaphore(1, "MMP_IIC_INIT");
#endif
    return result;
}

static MMP_RESULT
I2C_Terminate(
    void)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //disable HW
    AHB_WriteRegisterMask(REG_I2C_CONTROL, 0u, REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE);
#if SEM_ENABLE

    //release semaphore
    if (g_IIC_Semaphore)
    {
        SYS_DeleteSemaphore(g_IIC_Semaphore);
        g_IIC_Semaphore = MMP_NULL;
    }
#endif
    return result;
}

MMP_RESULT
IIC_SendData(
    MMP_UINT8 slaveAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size,
    IIC_DATA_MODE wStop)
{
    MMP_UINT16 i;
    MMP_RESULT result;
    MMP_UINT16 data;

    //set slave addr and sent start command
    //slave address bit[7:1]
    slaveAddr <<= 1;
    //r/w bit[0]
    slaveAddr  &= ~REG_BIT_READ_ENABLE;
    AHB_WriteRegister(REG_I2C_DATA, (MMP_UINT16)slaveAddr);
    AHB_WriteRegister(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_START);

    //check Ack
    result = IIC_CheckAck(TRANSMIT_DATA);
    if (result != MMP_RESULT_SUCCESS)
        goto end;

    for (i = 0; i < size; i++)
    {
        data = (MMP_UINT16)pbuffer[i];
        AHB_WriteRegister(REG_I2C_DATA, data);
        //last byte
        if (wStop && i == (size - 1))
        {
            //enable STOP Flag
            AHB_WriteRegister(REG_I2C_CONTROL,
                              REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | \
                              REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE | \
                              REG_BIT_CONTL_STOP);
        }
        else
        {
            AHB_WriteRegister(REG_I2C_CONTROL,
                              REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | \
                              REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE);
        }

        //check Ack
        result = IIC_CheckAck(TRANSMIT_DATA);
        if (result != MMP_RESULT_SUCCESS)
            goto end;
    }

end:
    return result;
}

MMP_RESULT
IIC_ReceiveData(
    MMP_UINT8 slaveAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size)
{
    MMP_UINT16 i;
    MMP_RESULT result;
    MMP_UINT32 data;

    //set slave addr and sent start command
    //slave address bit[7:1]
    slaveAddr <<= 1;
    //r/w bit[0]
    slaveAddr  |= REG_BIT_READ_ENABLE;
    AHB_WriteRegister(REG_I2C_DATA, slaveAddr);
    AHB_WriteRegister(REG_I2C_CONTROL,
                      REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | REG_BIT_CONTL_CLK_ENABLE |
                      REG_BIT_CONTL_I2C_ENABLE | REG_BIT_CONTL_START);

    //check Ack
    result = IIC_CheckAck(TRANSMIT_DATA | RECRIVE_SERVICE);
    if (result != MMP_RESULT_SUCCESS)
        goto end;

    for (i = 0; i < size; i++)
    {
        //last byte
        if (i == (size - 1))
        {
            //enable STOP / NACk Flag
            AHB_WriteRegister(REG_I2C_CONTROL,
                              REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | \
                              REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE | \
                              REG_BIT_CONTL_STOP | REG_BIT_CONTL_NACK);

            //check Ack
            result = IIC_CheckAck(RECEIVE_DATA | NACK_EN);
            if (result == I2C_NON_ACK)
                result = MMP_RESULT_SUCCESS;

            AHB_ReadRegister(REG_I2C_DATA, &data);
            pbuffer[i] = (MMP_UINT8)data;
        }
        else
        {
            AHB_WriteRegister(REG_I2C_CONTROL,
                              REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | \
                              REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE);

            //check Ack
            result = IIC_CheckAck(RECEIVE_DATA);
            if (result != MMP_RESULT_SUCCESS)
                goto end;

            AHB_ReadRegister(REG_I2C_DATA, &data);
            pbuffer[i] = (MMP_UINT8)data;
        }
    }

end:
    return result;
}

MMP_RESULT
mmpIicInitialize(
    IIC_HW_CONFIG_MODE mode,
    MMP_IIC_HANDLE *hDevice,
    MMP_GPIO_GROUP gpio_group,
    MMP_GPIO_PIN sclk_pin,
    MMP_GPIO_PIN data_pin,
    MMP_ULONG delay,
    MMP_BOOL bModuleLock)
{
#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    gHwMode = mode;

    if (bModuleLock && MMP_NULL == gIicModuleMutex)
    {
        gIicModuleMutex = SYS_CreateSemaphore(1, "MMP_IIC_MODULE_MUTEX");
    }

    if (gHwMode == CONTROLLER_MODE)
    {
        MMP_RESULT result;
        MMP_UINT32 regData;

        //IIC Mode external IIC + IIC controller, no power of internal hdmi rx
        AHB_WriteRegisterMask(GPIO_BASE + 0xD0, (0x01 << 28), (0x3 << 28));

        // Set GPIO18 and GPIO19 mode 1(IIC mode)
        AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x01 << 4) | (0x01 << 6), 0x000000F0);
        result = IIC_Initialize(delay);

#if TASK_SUSPEND_ALL
        xTaskResumeAll();
#endif

        return result;
    }
    else
    {
        MMP_UINT i;

        if (!g_IIC_Semaphore)
            g_IIC_Semaphore = SYS_CreateSemaphore(1, "MMP_IIC_INIT");

        ++g_IIC_SemaphoreRef;

        SYS_WaitSemaphore(g_IIC_Semaphore);

        if (g_IIC_DeviceCount == 0)
        {
            for (i = 0; i < MMP_MAX_IIC_DEVICE_NUM; i++)
            {
                g_IIC_Device[i].inuse             = MMP_FALSE;
                g_IIC_Device[i].gpio_group        = 0;
                g_IIC_Device[i].sclk              = 0;
                g_IIC_Device[i].data              = 0;
                g_IIC_Device[i].delay             = 0;
                g_IIC_Device[i].iic_enable_device = _IIC_Null;
            }
        }

        SYS_ReleaseSemaphore(g_IIC_Semaphore);

#if TASK_SUSPEND_ALL
        xTaskResumeAll();
#endif

        return IIC_AddDevice(hDevice, gpio_group, sclk_pin, data_pin, delay);
    }
}

MMP_RESULT
mmpIicTerminate(
    MMP_IIC_HANDLE hDevice)
{
#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    if (gIicModuleMutex)
    {
        SYS_DeleteSemaphore(gIicModuleMutex);
        gIicModuleMutex = MMP_NULL;
    }

    if (gHwMode == CONTROLLER_MODE)
    {
        MMP_RESULT result;
        result = I2C_Terminate();

#if TASK_SUSPEND_ALL
        xTaskResumeAll();
#endif
        return result;
    }
    else
    {
        MMP_RESULT result = IIC_RemoveDevice(hDevice);

        --g_IIC_SemaphoreRef;

        if (g_IIC_SemaphoreRef == 0)
        {
            SYS_DeleteSemaphore(g_IIC_Semaphore);
            g_IIC_Semaphore = 0;
        }
#if TASK_SUSPEND_ALL
        xTaskResumeAll();
#endif
        return result;
    }
}

MMP_RESULT
mmpIicStart(
    MMP_IIC_HANDLE hDevice)
{
    if (gHwMode == CONTROLLER_MODE)
    {
        MMP_RESULT result = MMP_RESULT_SUCCESS;

        AHB_WriteRegister(REG_I2C_CONTROL,
                          REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | REG_BIT_CONTL_CLK_ENABLE |
                          REG_BIT_CONTL_I2C_ENABLE | REG_BIT_CONTL_START);
        return result;
    }
    else
    {
        if (g_IIC_Device[hDevice].inuse == MMP_TRUE)
        {
            IIC_StartCondition(&g_IIC_Device[hDevice]);
            return MMP_RESULT_SUCCESS;
        }
        return MMP_RESULT_ERROR;
    }
}

MMP_RESULT
mmpIicStop(
    MMP_IIC_HANDLE hDevice)
{
    if (gHwMode == CONTROLLER_MODE)
    {
        MMP_RESULT result = MMP_RESULT_SUCCESS;

        AHB_WriteRegister(REG_I2C_CONTROL,
                          REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | \
                          REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE | \
                          REG_BIT_CONTL_STOP);
        return result;
    }
    else
    {
        if (g_IIC_Device[hDevice].inuse == MMP_TRUE)
        {
            IIC_StopCondition(&g_IIC_Device[hDevice]);
            return MMP_RESULT_SUCCESS;
        }
        return MMP_RESULT_ERROR;
    }
}

MMP_RESULT
mmpIicRecieve(
    MMP_IIC_HANDLE hDevice,
    MMP_UINT       *data)
{
    if (gHwMode == CONTROLLER_MODE)
    {
        MMP_RESULT result;
        AHB_ReadRegister(REG_I2C_DATA, (MMP_UINT32 *)data);
        result = IIC_CheckAck(RECEIVE_DATA);
        return result;
    }
    else
    {
        if (g_IIC_Device[hDevice].inuse == MMP_TRUE)
            return IIC_SendDeviceRead(&g_IIC_Device[hDevice], (MMP_UINT16 *)data);

        return MMP_RESULT_ERROR;
    }
}

MMP_RESULT
mmpIicSend(
    MMP_IIC_HANDLE hDevice,
    MMP_UINT data)
{
    if (gHwMode == CONTROLLER_MODE)
    {
        MMP_RESULT result;
        AHB_WriteRegister(REG_I2C_DATA, data);
        result = IIC_CheckAck(TRANSMIT_DATA);
        return result;
    }
    else
    {
        if (g_IIC_Device[hDevice].inuse == MMP_TRUE)
            return IIC_SendDeviceWrite(&g_IIC_Device[hDevice], data);

        return MMP_RESULT_ERROR;
    }
}

MMP_RESULT
mmpIicSendData(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
    MMP_UINT8 regAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8  *pdbuf = dbuf;
    MMP_UINT16 size2;

#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    PRECONDITION(pbuffer);
    PRECONDITION(size < IIC_MAX_SIZE);
#if SEM_ENABLE
    SYS_WaitSemaphore(g_IIC_Semaphore);
#endif

    *pdbuf++ = regAddr;
    SYS_Memcpy(pdbuf, pbuffer, size);
    size2    = size + 1;
    result   = IIC_SendData(slaveAddr, dbuf, size2, W_STOP);
#if SEM_ENABLE
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
#endif

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif

    return result;
}

MMP_RESULT
mmpIicSendDataEx(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size)
{
    MMP_RESULT result;

#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    PRECONDITION(pbuffer);
    PRECONDITION(size < IIC_MAX_SIZE);
#if SEM_ENABLE
    SYS_WaitSemaphore(g_IIC_Semaphore);
#endif

    result = IIC_SendData(slaveAddr, pbuffer, size, W_STOP);

#if SEM_ENABLE
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
#endif

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif

    return result;
}

MMP_RESULT
mmpIicReceiveData(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
    MMP_UINT8 regAddr,
    MMP_UINT8 *pbuffer,
    MMP_UINT16 size)
{
    MMP_RESULT result;

#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    PRECONDITION(pbuffer);
    PRECONDITION(size < IIC_MAX_SIZE);
#if SEM_ENABLE
    SYS_WaitSemaphore(g_IIC_Semaphore);
#endif

    result = IIC_SendData(slaveAddr, &regAddr, 1, WO_STOP);

    if (result == MMP_RESULT_SUCCESS)
        result = IIC_ReceiveData(slaveAddr, pbuffer, size);

#if SEM_ENABLE
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
#endif

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif

    return result;
}

MMP_RESULT
mmpIicReceiveDataEx(
    IIC_OP_MODE mode,
    MMP_UINT8 slaveAddr,
    MMP_UINT8 *pwbuffer,
    MMP_UINT16 wsize,
    MMP_UINT8 *prbuffer,
    MMP_UINT16 rsize)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;

#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    PRECONDITION(prbuffer);
    PRECONDITION(wsize < IIC_MAX_SIZE);
    PRECONDITION(rsize < IIC_MAX_SIZE);
#if SEM_ENABLE
    SYS_WaitSemaphore(g_IIC_Semaphore);
#endif
    if (pwbuffer && wsize)
        result = IIC_SendData(slaveAddr, pwbuffer, wsize, WO_STOP);

    if (result == MMP_RESULT_SUCCESS)
        result = IIC_ReceiveData(slaveAddr, prbuffer, rsize);
#if SEM_ENABLE
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
#endif

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif

    return result;
}

MMP_UINT32
mmpIicSetClockRate(
    MMP_UINT32 clock)
{
    return (IIC_SetClockRate(clock));
}

MMP_UINT32
mmpIicGetClockRate(
    void)
{
    return gCurrentClock;
}

MMP_RESULT
mmpIicGenStop(
    void)
{
#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    AHB_WriteRegister(REG_I2C_CONTROL,
                      REG_BIT_INTR_TRIG | REG_BIT_CONTL_TRANSFER_BYTE | \
                      REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE | \
                      REG_BIT_CONTL_STOP | REG_BIT_CONTL_NACK);

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif

    return MMP_SUCCESS;
}

void
mmpIicLockModule(
    void)
{
#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    if (gIicModuleMutex)
        SYS_WaitSemaphore(gIicModuleMutex);

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif
}

void
mmpIicReleaseModule(
    void)
{
#if TASK_SUSPEND_ALL
    vTaskSuspendAll();
#endif

    if (gIicModuleMutex)
        SYS_ReleaseSemaphore(gIicModuleMutex);

#if TASK_SUSPEND_ALL
    xTaskResumeAll();
#endif
}

#define IIC_REG_BIT(var, n) (((var & (1 << n)) > 0) ? 1 : 0)

/**
 * mmpIicSlaveRead()
 *
 * @param outputBuffer: To receive data from IIC master.
 * @param outputBufferLength: The buffer length of outputBuffer.
 *
 * @return MMP_TRUE if data received. MMP_FALSE if no data received.
 */
MMP_UINT32
mmpIicSlaveRead(
    MMP_UINT8 *inutBuffer,
    MMP_UINT32 inputBufferLength)
{
#define IIC_SLAVE_ADDR (0x78 >> 1)

    MMP_UINT32 outputBufferWriteIndex = 0;
    MMP_UINT32 regData                = 0;
    MMP_UINT32 ccc                    = 0;

    /* Set slave address */
    AHB_WriteRegister(REG_I2C_SLAVE_ADDR, IIC_SLAVE_ADDR);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    AHB_WriteRegister(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_GC |
        REG_BIT_CONTL_I2C_ENABLE);

    /* Read status, check if recevice matched slave address */
    AHB_ReadRegister(REG_I2C_STATUS, &regData);
    if (IIC_REG_BIT(regData, 8) == 1 &&         /* SAM */
        IIC_REG_BIT(regData, 5) == 1 &&         /* DR */
        IIC_REG_BIT(regData, 2) == 1 &&         /* I2CB */
        IIC_REG_BIT(regData, 1) == 0 &&         /* ACK */
        IIC_REG_BIT(regData, 0) == 0)           /* RW */
    {
        while (1)
        {
            /* Enable transfer bit */
            AHB_WriteRegister(
                REG_I2C_CONTROL,
                REG_BIT_INTR_ALL |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_GC |
                REG_BIT_CONTL_I2C_ENABLE);

            AHB_ReadRegister(REG_I2C_STATUS, &regData);
            if (IIC_REG_BIT(regData, 7) == 1)                   /* Detect STOP condition */
            {
                /* Receive STOP */
                goto end;
            }

            if (IIC_REG_BIT(regData, 5) == 1                            /* Receive new data */
                /*&& IIC_REG_BIT(regData, 1) == 0*/)                    /* Receive ack */
            {
                /* Read data */
                MMP_UINT32 iicInputData = 0;

                AHB_ReadRegister(REG_I2C_DATA, &iicInputData);
                //printf("[%d]iicInputData = %u\n", ccc++, iicInputData);
                inutBuffer[outputBufferWriteIndex] = iicInputData & 0x000000FF;
                outputBufferWriteIndex++;
                if (outputBufferWriteIndex >= inputBufferLength)
                {
                    goto end;
                }
            }
        }
    }

end:
    return outputBufferWriteIndex;
}

MMP_RESULT
mmpIicSlaveWrite(
    MMP_UINT8 *outputBuffer,
    MMP_UINT32 outputBufferLength)
{
#define IIC_SLAVE_ADDR (0x78 >> 1)

    MMP_RESULT iicResult              = 0;
    MMP_UINT32 outputBufferWriteIndex = 0;
    MMP_UINT32 regData                = 0;

    /* Set slave address */
    AHB_WriteRegister(REG_I2C_SLAVE_ADDR, IIC_SLAVE_ADDR);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    AHB_WriteRegister(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_GC |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_I2C_ENABLE);

    /* Read status, check if recevice matched slave address */
    AHB_ReadRegister(REG_I2C_STATUS, &regData);
    if (IIC_REG_BIT(regData, 8) == 1 &&         /* SAM */
        IIC_REG_BIT(regData, 5) == 1 &&         /* DR */
        IIC_REG_BIT(regData, 2) == 1 &&         /* I2CB */
        IIC_REG_BIT(regData, 1) == 0 &&         /* ACK */
        IIC_REG_BIT(regData, 0) == 1)           /* RW */
    {
        MMP_UINT32 writeIndex = 0;

        for (writeIndex = 0; writeIndex < outputBufferLength; writeIndex++)
        {
            AHB_WriteRegister(REG_I2C_DATA, outputBuffer[writeIndex]);
            /* Enable transfer bit */
            AHB_WriteRegister(
                REG_I2C_CONTROL,
                REG_BIT_INTR_ALL |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_GC |
                REG_BIT_CONTL_I2C_ENABLE);
            /* Check ACK */
            iicResult = IIC_CheckAck(TRANSMIT_DATA);
            if (iicResult)
            {
                // Check ACK fail
                goto end;
            }
        }
    }

end:
    return iicResult;
}