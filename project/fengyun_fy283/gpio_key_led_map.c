/*
 * Copyright (c) 2009 SMedia Technology Corp. All Rights Reserved.
 */

#include "mmp_types.h"
#include "pal/pal.h"
#include "host/ahb.h"
#include "host/host.h"
#include "host/gpio.h"
#include "gpio_key_led_map.h"

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct LED_GPIO_TAG
{
    MMP_UINT32 gpioLed_Analog;
    MMP_UINT32 gpioLed_HDMI;
    MMP_UINT32 gpioLed_720P;
    MMP_UINT32 gpioLed_1080P;
    MMP_UINT32 gpioLed_Power;
    MMP_UINT32 gpioLed_USBStorage;
    MMP_UINT32 gpioLed_SDStorage;
    MMP_UINT32 gpioLed_Record;
} LED_GPIO;

//=============================================================================
//                              Constant Definition
//=============================================================================

#define SCL_PIN     46 // 
#define SDA_PIN     47 //GPIO47


#define BUTTON_CTRL_0          27 //GPIO27
#define BUTTON_CTRL_1          28 //GPIO28
//kenny 20140617
#define MIC_IIS_SPDIF_IN       43
#define MIC_IIS_SPDIF_IN_2     44 //fongyun

/* For Customer Kworld  +*/
#define MIC_IN                 42 //GPIO13 kenny
/* For Customer Kworld  -*/

#define KEY_S1_HOLD_TIMEOUT    4000

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
LED_GPIO gtLEDCtrl = {0};

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * GPIO led Process
 */
//=============================================================================
MMP_RESULT
GpioLEDInitialize(
    void)
{
    gtLEDCtrl.gpioLed_Analog = 29;   //31;
    gtLEDCtrl.gpioLed_HDMI   = 23;   // 32;
    gtLEDCtrl.gpioLed_720P   = 31;   //29;
    gtLEDCtrl.gpioLed_1080P  = 32;   //23;
    gtLEDCtrl.gpioLed_Power  = 21;
    gtLEDCtrl.gpioLed_Record = 22;

    //Set GPIO Initialize Value
    GPIO_SetState(gtLEDCtrl.gpioLed_Analog, GPIO_STATE_LO);
    GPIO_SetState(gtLEDCtrl.gpioLed_HDMI, GPIO_STATE_LO);
    GPIO_SetState(gtLEDCtrl.gpioLed_720P, GPIO_STATE_LO);
    GPIO_SetState(gtLEDCtrl.gpioLed_1080P, GPIO_STATE_LO);
    GPIO_SetState(gtLEDCtrl.gpioLed_Power, GPIO_STATE_LO);
    GPIO_SetState(gtLEDCtrl.gpioLed_Record, GPIO_STATE_LO);

    //Set GPIO Output Mode
    GPIO_SetMode(gtLEDCtrl.gpioLed_Analog, GPIO_MODE_OUTPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_HDMI, GPIO_MODE_OUTPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_720P, GPIO_MODE_OUTPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_1080P, GPIO_MODE_OUTPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_Power, GPIO_MODE_OUTPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_Record, GPIO_MODE_OUTPUT);

    //Set GPIO Mode0
    ithGpioSetMode(gtLEDCtrl.gpioLed_Analog, ITH_GPIO_MODE0);
    ithGpioSetMode(gtLEDCtrl.gpioLed_HDMI, ITH_GPIO_MODE0);
    ithGpioSetMode(gtLEDCtrl.gpioLed_720P, ITH_GPIO_MODE0);
    ithGpioSetMode(gtLEDCtrl.gpioLed_1080P, ITH_GPIO_MODE0);
    ithGpioSetMode(gtLEDCtrl.gpioLed_Power, ITH_GPIO_MODE0);
    ithGpioSetMode(gtLEDCtrl.gpioLed_Record, ITH_GPIO_MODE0);

    GPIO_SetState(SCL_PIN, GPIO_STATE_HI);
    GPIO_SetMode(SCL_PIN, GPIO_MODE_OUTPUT);	
    ithGpioSetMode(SCL_PIN, ITH_GPIO_MODE0);

    GPIO_SetState(SDA_PIN, GPIO_STATE_HI);
    GPIO_SetMode(SDA_PIN, GPIO_MODE_OUTPUT);	
    ithGpioSetMode(SDA_PIN, ITH_GPIO_MODE0);
	
    return MMP_SUCCESS;
}

MMP_RESULT
GpioLEDTerminate(
    void)
{
    //Set GPIO Input Mode
    GPIO_SetMode(gtLEDCtrl.gpioLed_Analog, GPIO_MODE_INPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_HDMI, GPIO_MODE_INPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_720P, GPIO_MODE_INPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_1080P, GPIO_MODE_INPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_Power, GPIO_MODE_INPUT);
    GPIO_SetMode(gtLEDCtrl.gpioLed_Record, GPIO_MODE_INPUT);

    return MMP_SUCCESS;
}


void SETSDADIR(MMP_BOOL state)
{
    if(state)
    	{
       GPIO_SetState(SDA_PIN, GPIO_STATE_HI);
       GPIO_SetMode(SDA_PIN, GPIO_MODE_OUTPUT);	
       ithGpioSetMode(SDA_PIN, ITH_GPIO_MODE0);    	
    	}
    else
    	{
       GPIO_SetMode(SDA_PIN, GPIO_MODE_INPUT);
       ithGpioSetMode(SDA_PIN, ITH_GPIO_MODE0);
    	}
}
	
void
SETSCL(
    MMP_BOOL state)
{
    if (state)
        GPIO_SetState(SCL_PIN, GPIO_STATE_HI);
    else
        GPIO_SetState(SCL_PIN, GPIO_STATE_LO);
}

void
SETSDA(
    MMP_BOOL state)
{
    if (state)
        GPIO_SetState(SDA_PIN, GPIO_STATE_HI);
    else
        GPIO_SetState(SDA_PIN, GPIO_STATE_LO);
}

MMP_BOOL GETSDA()
{
    return GPIO_GetState(SDA_PIN);
}
void
GpioLedControl(
    GPIO_LED_CONTROL *pLedCtrl)
{
    if (pLedCtrl->EnAnalog)
        GPIO_SetState(gtLEDCtrl.gpioLed_Analog, GPIO_STATE_HI);
    else
        GPIO_SetState(gtLEDCtrl.gpioLed_Analog, GPIO_STATE_LO);

    if (pLedCtrl->EnHDMI)
        GPIO_SetState(gtLEDCtrl.gpioLed_HDMI, GPIO_STATE_HI);
    else
        GPIO_SetState(gtLEDCtrl.gpioLed_HDMI, GPIO_STATE_LO);

    if (pLedCtrl->En720P)
        GPIO_SetState(gtLEDCtrl.gpioLed_720P, GPIO_STATE_HI);
    else
        GPIO_SetState(gtLEDCtrl.gpioLed_720P, GPIO_STATE_LO);

    if (pLedCtrl->En1080P)
        GPIO_SetState(gtLEDCtrl.gpioLed_1080P, GPIO_STATE_HI);
    else
        GPIO_SetState(gtLEDCtrl.gpioLed_1080P, GPIO_STATE_LO);

    if (pLedCtrl->EnRecord)
        GPIO_SetState(gtLEDCtrl.gpioLed_Record, GPIO_STATE_HI);
    else
        GPIO_SetState(gtLEDCtrl.gpioLed_Record, GPIO_STATE_LO);

    if (pLedCtrl->EnPower)
        GPIO_SetState(gtLEDCtrl.gpioLed_Power, GPIO_STATE_HI);
    else
        GPIO_SetState(gtLEDCtrl.gpioLed_Power, GPIO_STATE_LO);
}

//=============================================================================
/**
 * GPIO Key Process
 */
//=============================================================================
MMP_RESULT
GpioKeyInitialize(
    void)
{
    // Set GPIO as input
    GPIO_SetMode(BUTTON_CTRL_0, GPIO_MODE_INPUT);
    GPIO_SetMode(BUTTON_CTRL_1, GPIO_MODE_INPUT);

    ithGpioSetMode(BUTTON_CTRL_0, ITH_GPIO_MODE0);
    ithGpioSetMode(BUTTON_CTRL_1, ITH_GPIO_MODE0);

    return MMP_SUCCESS;
}

MMP_RESULT
GpioKeyTerminate(
    void)
{
    // Set GPIO as input
    GPIO_SetMode(BUTTON_CTRL_0, GPIO_MODE_INPUT);
    GPIO_SetMode(BUTTON_CTRL_1, GPIO_MODE_INPUT);

    return MMP_SUCCESS;
}

KEY_CODE
GpioKeyGetKey(
    void)
{
    //static PAL_CLOCK_T lastClock = 0;
    KEY_CODE   key = KEY_CODE_UNKNOW;
    GPIO_STATE button0, button1;
    MMP_UINT32 value, mask;
    //MMP_UINT32 flag;

    //if (PalGetDuration(lastClock) >= 300)
    {
        // BUTTON_CTRL_0 output low, BUTTON_CTRL_1 input
        GPIO_SetMode(BUTTON_CTRL_0, GPIO_MODE_OUTPUT);
        GPIO_SetMode(BUTTON_CTRL_1, GPIO_MODE_INPUT);
        GPIO_SetState(BUTTON_CTRL_0, GPIO_STATE_LO);
        PalSleep(1);// have to delay

        AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG, &value);
        mask    = (1 << BUTTON_CTRL_1);
        button1 = ((value & mask) ? GPIO_STATE_HI : GPIO_STATE_LO);
        if (button1 == GPIO_STATE_LO)
        {
            // BUTTON_CTRL_0 input, BUTTON_CTRL_1 input
            GPIO_SetMode(BUTTON_CTRL_0, GPIO_MODE_INPUT);
            GPIO_SetMode(BUTTON_CTRL_1, GPIO_MODE_INPUT);
            PalSleep(1); // have to delay

            AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG, &value);

            if (!(value & ((1 << BUTTON_CTRL_1)))) //BUTTON_CTRL_1 = LO
            {
                key = KEY_CODE_S2;
            }
            else
            {
                // BUTTON_CTRL_0 output low, BUTTON_CTRL_1 input
                GPIO_SetMode(BUTTON_CTRL_0, GPIO_MODE_OUTPUT);
                GPIO_SetMode(BUTTON_CTRL_1, GPIO_MODE_INPUT);
                GPIO_SetState(BUTTON_CTRL_0, GPIO_STATE_LO);
                PalSleep(1);// have to delay

                AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG, &value);
                mask    = (1 << BUTTON_CTRL_1);
                button1 = ((value & mask) ? GPIO_STATE_HI : GPIO_STATE_LO);

                if (button1 == GPIO_STATE_LO)
                    key = KEY_CODE_S3;
            }
        }

        // BUTTON_CTRL_0 input, BUTTON_CTRL_1 input low
        GPIO_SetMode(BUTTON_CTRL_0, GPIO_MODE_INPUT);
        GPIO_SetMode(BUTTON_CTRL_1, GPIO_MODE_INPUT);
        PalSleep(1);// have to delay

        AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG, &value);
        mask    = (1 << BUTTON_CTRL_0);
        button0 = ((value & mask) ? GPIO_STATE_HI : GPIO_STATE_LO);
        mask    = (1 << BUTTON_CTRL_1);
        button1 = ((value & mask) ? GPIO_STATE_HI : GPIO_STATE_LO);

        if (button0 == GPIO_STATE_LO && button1 == GPIO_STATE_HI)
        {
            key = KEY_CODE_S1;
        }

        // no key press
        //lastClock = PalGetClock();
    }

    return key;
}

//=============================================================================
/**
 * GPIO Mic Detect Process
 */
//=============================================================================
MMP_RESULT
GpioMicInitialize(
    void)
{
    //kenny 20140617
    GPIO_SetState(MIC_IIS_SPDIF_IN, GPIO_STATE_LO);
    //Set GPIO Output Mode
    GPIO_SetMode(MIC_IIS_SPDIF_IN, GPIO_MODE_OUTPUT);
    //Set GPIO Mode0
    ithGpioSetMode(MIC_IIS_SPDIF_IN, ITH_GPIO_MODE0);

    GPIO_SetState(MIC_IIS_SPDIF_IN_2, GPIO_STATE_LO);
    //Set GPIO Output Mode
    GPIO_SetMode(MIC_IIS_SPDIF_IN_2, GPIO_MODE_OUTPUT);
    //Set GPIO Mode0
    ithGpioSetMode(MIC_IIS_SPDIF_IN_2, ITH_GPIO_MODE0);

    //Set GPIO Input Mode
    GPIO_SetMode(MIC_IN, GPIO_MODE_INPUT);
    ithGpioSetMode(MIC_IN, ITH_GPIO_MODE0);
    return MMP_SUCCESS;
}

MMP_BOOL
GpioMicIsInsert(
    void)
{
    return !GPIO_GetState(MIC_IN);
}

//kenny 20140617
MMP_BOOL
GpioInputselect(
    MMP_BOOL status)
{
    if (status)
    {
        GPIO_SetState(MIC_IIS_SPDIF_IN, GPIO_STATE_LO);   //YPBPR
        GPIO_SetState(MIC_IIS_SPDIF_IN_2, GPIO_STATE_LO); //YPBPR
    }
    else
    {
        GPIO_SetState(MIC_IIS_SPDIF_IN, GPIO_STATE_HI);   //HDMI
        GPIO_SetState(MIC_IIS_SPDIF_IN_2, GPIO_STATE_HI); //HDMI
    }
}