/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 */
/** @file gpio.c
 *
 * @author Evan Chang
 */


#if defined(__OPENRTOS__)
#include "ite/ith.h"
#endif
#include "host/ahb.h"
#include "host/host.h"
#include "host/gpio.h"

//=============================================================================
//                              Macro Definition
//=============================================================================
#if defined(MM9070) || defined(MM9910)
#define GPIO_OFFSET					0x40

#define GPIO_DATASET_REG			0x0C
#define GPIO_DATACLR_REG			0x10
#else
//           |23                           0
// **-- ---* | ---- ---- ---- ---- ---- ----
// ||      -> SGPIO_GROUP
// |-> PADSEL_TAG
// -> DGPIO_TAG
#define GET_GPIO_TAG(value)         ((0x80 << 24) & value)
#define GET_PADSEL_TAG(value)       ((0x40 << 24) & value)
#define GET_BIT(value)              (~(0xFF << 24) & value)
#define PADSEL_BIT_COUNT            32

// select bits [14], [12] and [10]
#define GET_SGPIO_ENABLE_BIT(sgpio) (((0x04 & sgpio) << 12) | ((0x02 & sgpio) << 11) | ((0x01 & sgpio) << 10))
#define GET_SGPIO_GROUP(sgpio)      ((0x01 << 24) & sgpio)

//=============================================================================
//                              Global Data Definition
//=============================================================================

MMP_UINT32 padsel[PADSEL_BIT_COUNT][2] = {
//PADSEL     set 0        set 1
/*PADSEL0 */ {DGPIO(0)  ,0         },
/*PADSEL1 */ {DGPIO(0)  ,0         },
/*PADSEL2 */ {DGPIO(1)  ,0         },
/*PADSEL3 */ {DGPIO(1)  ,0         },
/*PADSEL4 */ {DGPIO(2)  ,0         },
/*PADSEL5 */ {DGPIO(2)  ,0         },
/*PADSEL6 */ {DGPIO(3)  ,0         },
/*PADSEL7 */ {DGPIO(3)  ,0         },
/*PADSEL8 */ {DGPIO(4)  ,0         },
/*PADSEL9 */ {DGPIO(4)  ,0         },
/*PADSEL10*/ {DGPIO(5)  ,0         },
/*PADSEL11*/ {DGPIO(5)  ,0         },
/*PADSEL12*/ {DGPIO(6)  ,0         },
/*PADSEL13*/ {DGPIO(6)  ,0         },
/*PADSEL14*/ {DGPIO(7)  ,0         },
/*PADSEL15*/ {DGPIO(7)  ,0         },                 
/*PADSEL16*/ {DGPIO(8)  ,0         },
/*PADSEL17*/ {DGPIO(8)  ,0         },
/*PADSEL18*/ {DGPIO(9)  ,0         },
/*PADSEL19*/ {DGPIO(9)  ,0         },
/*PADSEL20*/ {DGPIO(10) ,0         },
/*PADSEL21*/ {DGPIO(10) ,0         },
/*PADSEL22*/ {DGPIO(11) ,0         },
/*PADSEL23*/ {DGPIO(11) ,0         },
/*PADSEL24*/ {DGPIO(12) ,0         },
/*PADSEL25*/ {DGPIO(12) ,0         },
/*PADSEL26*/ {DGPIO(13) ,0         },
/*PADSEL27*/ {DGPIO(13) ,0         },
/*PADSEL28*/ {DGPIO(14) ,0         },
/*PADSEL29*/ {DGPIO(14) ,0         },
/*PADSEL30*/ {DGPIO(15) ,0         },
/*PADSEL31*/ {DGPIO(15) ,0         }
};
#endif
//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void 
_GPIO_Toggle(
    MMP_UINT32 gpio, 
    MMP_BOOL bToggle);

static void 
_PADSEL_Set(
    MMP_UINT32 bits,
    MMP_BOOL bToggle);

//=============================================================================
//                              Function Definition
//=============================================================================

void 
GPIO_Enable(
    MMP_UINT32 gpio)
{
    _GPIO_Toggle(gpio, MMP_TRUE);
}

void 
GPIO_Disable(
    MMP_UINT32 gpio)
{
    _GPIO_Toggle(gpio, MMP_FALSE);
}

/** Just for DGPIO */
MMP_UINT32
GPIO_GetNum(
	MMP_UINT32 gpio)
{
#if defined(MM9070) || defined(MM9910)
	#if defined(__OPENRTOS__)
	uint32_t i;
	#else
	MMP_UINT32 i;
	#endif

	for(i=0; i<32; i++)
	{
		if((gpio>>i) & 0x1)
			return i;
	}
	printf(" Get GPIO Number error??? gpio = 0x%08X \n", gpio);
#else
	MMP_INT i;

	for(i=0; i<24; i++)
	{
		if((gpio>>i) & 0x1)
			return i;
	}
	printf(" Get DGPIO Number error??? gpio = 0x%08X \n", gpio);
#endif
	return 0;
}

void 
GPIO_SetMode(
    MMP_UINT32 gpio, 
    GPIO_MODE mode)
{    
#if defined(MM9070) || defined(MM9910)
	#if defined(__OPENRTOS__)
	{
		if (mode == GPIO_MODE_OUTPUT)
			ithGpioSetOut(gpio);
		else
			ithGpioSetIn(gpio);
	}
	#else
	{
		MMP_UINT32 value = ((mode == GPIO_MODE_INPUT) ? 0 : 0xFFFFFFFF);
		MMP_UINT32 mask = 0;

		if (gpio < 32)
		{
			mask = (1 << gpio);
			AHB_WriteRegisterMask(GPIO_BASE + GPIO_PINDIR_REG, value, mask);
		}
		else
		{
			mask = (1 << (gpio - 32));
			AHB_WriteRegisterMask(GPIO_BASE + GPIO_PINDIR_REG + GPIO_OFFSET, value, mask);
		}
	}
	#endif
#else
    if (GET_GPIO_TAG(gpio) == DGPIO_TAG)
    {
        MMP_UINT32 mask = 0;
        MMP_UINT32 value = 0;
        
        mask = GET_BIT(gpio);
        value = ((mode == GPIO_MODE_OUTPUT) ? 0xFFFFFFFF : 0);
        AHB_WriteRegisterMask(GPIO_BASE+GPIO_PINDIR_REG, value, mask);
    }
    else
    {
        MMP_UINT16 i = 0;
        MMP_UINT16 mask = 0;
        MMP_UINT16 value = 0;
        MMP_UINT16 address = MMP_GENERAL_GPIO_REG_100;
        
        if (GET_SGPIO_GROUP(gpio))
            address = MMP_GENERAL_GPIO_REG_110;

        value = ((mode == GPIO_MODE_OUTPUT) ? 0 : 0x0007); // mode bits [2:0]
        for (; i<8; i++, gpio = (gpio >> 3))
        {
            if (mask = (0x0007 & gpio)) // mode bits [2:0]
            {
                HOST_WriteRegisterMask(address+i*2, value, mask);
                break;
            }
        }
    }
#endif
}

void 
GPIO_SetState(
    MMP_UINT32 gpio, 
    GPIO_STATE state)
{
#if defined(MM9070) || defined(MM9910)
	#if defined(__OPENRTOS__)
	{
		if (state == GPIO_STATE_HI)
			ithGpioSet(gpio);
		else
			ithGpioClear(gpio);
	}
	#else
	{
		if (state == GPIO_STATE_HI)
		{
			if (gpio < 32)
				AHB_WriteRegister(GPIO_BASE + GPIO_DATASET_REG, (1 << gpio));
			else
				AHB_WriteRegister(GPIO_BASE + GPIO_DATASET_REG + GPIO_OFFSET, (1 << (gpio - 32)));
		}
		else
		{
			if (gpio < 32)
				AHB_WriteRegister(GPIO_BASE + GPIO_DATACLR_REG, (1 << gpio));
			else
				AHB_WriteRegister(GPIO_BASE + GPIO_DATACLR_REG + GPIO_OFFSET, (1 << (gpio - 32)));
		}
	}
	#endif
#else
    if (GET_GPIO_TAG(gpio) == DGPIO_TAG)
    {
        MMP_UINT32 mask = 0;
        MMP_UINT32 value = 0;
        
        mask = GET_BIT(gpio);
        value = ((state==GPIO_STATE_HI) ? mask : 0);
        AHB_WriteRegisterMask(GPIO_BASE+GPIO_DATAOT_REG, value, mask);
    }
    else
    {
        MMP_UINT16 i = 0;
        MMP_UINT16 mask = 0;
        MMP_UINT32 value = 0;
        MMP_UINT16 address = MMP_GENERAL_GPIO_REG_100;
        
        if (GET_SGPIO_GROUP(gpio))
            address = MMP_GENERAL_GPIO_REG_110;

        for (; i<8; i++, gpio = (gpio >> 3))
        {            
            if (mask = (0x0007 & gpio))
            {                
                mask = mask << 3; // move mask to output data bits [5:3]
                value = ((state==GPIO_STATE_HI) ? (0x0007 << 3) : 0);
                HOST_WriteRegisterMask(address+i*2, value, mask);
                break;
            }
        }
    }
#endif
}

GPIO_STATE
GPIO_GetState(
    MMP_UINT32 gpio)
{
#if defined(MM9070) || defined(MM9910)
	#if defined(__OPENRTOS__)
	{
		return ithGpioGet(gpio) ? GPIO_STATE_HI : GPIO_STATE_LO;
	}
	#else
	{
		MMP_UINT32 value = 0;
		MMP_UINT32 mask = 0;

		if (gpio < 32)
		{
			mask = (1 << gpio);
			AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG, &value);
			return ((value & mask) ? GPIO_STATE_HI : GPIO_STATE_LO);
		}
		else
		{
			mask = (1 << (gpio - 32));
			AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG + GPIO_OFFSET, &value);
			return ((value & mask) ? GPIO_STATE_HI : GPIO_STATE_LO);
		}
	}
	#endif
#else
    if (GET_GPIO_TAG(gpio) == DGPIO_TAG)
    {
        MMP_UINT32 mask = 0;
        MMP_UINT32 readValue = 0;
        
        mask = GET_BIT(gpio);

        AHB_ReadRegister(GPIO_BASE+GPIO_DATAIN_REG, &readValue);
        return (mask & readValue) ? GPIO_STATE_HI : GPIO_STATE_LO;
    }
    else
    {
        MMP_UINT16 i = 0;
        MMP_UINT16 mask = 0;
        MMP_UINT16 readValue = 0;
        MMP_UINT16 address = MMP_GENERAL_GPIO_REG_100;
        
        if (GET_SGPIO_GROUP(gpio))
            address = MMP_GENERAL_GPIO_REG_110;

        for (; i<8; i++, gpio = (gpio >> 3))
        {
            if (mask = (0x0007 & gpio))
            {
                HOST_ReadRegister(address+i*2, &readValue);
                mask = mask << 6; // move mask to input data bits [8:6]
                return (mask & readValue) ? GPIO_STATE_HI : GPIO_STATE_LO;
            }
        }                
    }
#endif
}

void 
PADSEL_Enable(
    PADSEL_MODE mode)
{
    _PADSEL_Set((MMP_UINT32) mode, MMP_TRUE);
}

void 
PADSEL_Disable(
    PADSEL_MODE mode)
{
    _PADSEL_Set((MMP_UINT32) mode, MMP_FALSE);
}


#if defined(MM9070) || defined(MM9910) && !defined(__OPENRTOS__)

static GpioIntrHandler gpioHandlerTable[64];
static void* gpioArgTable[64];

static void GpioDefaultHandler(MMP_UINT32 pin, void* arg)
{
    // DO NOTHING
}

void GPIO_EnableIntr(MMP_UINT32 pin)
{
    if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + GPIO_INTREN_REG, 0x1 << pin, 0x1 << pin);
    else
        AHB_WriteRegisterMask(GPIO_BASE + GPIO_INTREN_REG + GPIO_OFFSET, 0x1 << pin, 0x1 << pin);
}

void GPIO_ClearIntr(MMP_UINT32 pin)
{
    if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + GPIO_INTRCLR_REG, 1 << pin, 1 << pin);
    else
        AHB_WriteRegisterMask(GPIO_BASE + GPIO_INTRCLR_REG + GPIO_OFFSET, 1 << (pin - 32), 1 << (pin - 32));
}

void GPIO_RegisterIntrHandler(MMP_UINT32 pin, GpioIntrHandler handler, void* arg)
{
	if (pin < 64)
	{
        gpioHandlerTable[pin]   = handler ? handler : GpioDefaultHandler;
        gpioArgTable[pin]       = arg;
    }
}

void GPIO_DoIntr(void)
{
    MMP_UINT32 gpioNum     = 0;
    MMP_UINT32 intrFlags;
	AHB_ReadRegister(GPIO_BASE + GPIO_INTRMASKSTATE_REG, &intrFlags);   // read interrupt source

    // Test to see if there is a flag set, if not then don't do anything
    while (intrFlags != 0)
    {
        if ((intrFlags & 0xFFFF) == 0)
        {
            gpioNum += 16;
            intrFlags >>= 16;
        }

        if ((intrFlags & 0xFF) == 0)
        {
            gpioNum += 8;
            intrFlags >>= 8;
        }

        if ((intrFlags & 0xF) == 0)
        {
            gpioNum += 4;
            intrFlags >>= 4;
        }

        if ((intrFlags & 0x3) == 0)
        {
            gpioNum += 2;
            intrFlags >>= 2;
        }

        if ((intrFlags & 0x1) == 0)
        {
            gpioNum += 1;
            intrFlags >>= 1;
        }

		// Clear Intr
		AHB_WriteRegisterMask(GPIO_BASE + GPIO_INTRCLR_REG, 0x1 << gpioNum, 0x1 << gpioNum);

        // Call the handler
        gpioHandlerTable[gpioNum](gpioNum, gpioArgTable[gpioNum]);
          
        intrFlags &= ~1;
    }
    
    gpioNum     = 32;
    AHB_ReadRegister(GPIO_BASE + GPIO_INTRMASKSTATE_REG + GPIO_OFFSET, &intrFlags);   // read interrupt source

    // Test to see if there is a flag set, if not then don't do anything
    while (intrFlags != 0)
    {
        if ((intrFlags & 0xFFFF) == 0)
        {
            gpioNum += 16;
            intrFlags >>= 16;
        }

        if ((intrFlags & 0xFF) == 0)
        {
            gpioNum += 8;
            intrFlags >>= 8;
        }

        if ((intrFlags & 0xF) == 0)
        {
            gpioNum += 4;
            intrFlags >>= 4;
        }

        if ((intrFlags & 0x3) == 0)
        {
            gpioNum += 2;
            intrFlags >>= 2;
        }

        if ((intrFlags & 0x1) == 0)
        {
            gpioNum += 1;
            intrFlags >>= 1;
        }

		// Clear Intr
		AHB_WriteRegisterMask(GPIO_BASE + GPIO_OFFSET + GPIO_INTRCLR_REG, 0x1 << gpioNum, 0x1 << gpioNum);

        // Call the handler
        gpioHandlerTable[gpioNum](gpioNum, gpioArgTable[gpioNum]);
          
        intrFlags &= ~1;
    }
}

void GPIO_EnableBounce(MMP_UINT32 pin)
{
    if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + GPIO_BOUNCEEN_REG, 0x1 << pin, 0x1 << pin);
    else
        AHB_WriteRegisterMask(GPIO_BASE + GPIO_BOUNCEEN_REG + GPIO_OFFSET, 0x1 << (pin - 32), 0x1 << (pin - 32));
}

void GPIO_DisableBounce(MMP_UINT32 pin)
{
    if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + GPIO_BOUNCEEN_REG, 0 << pin, 0x1 << pin);
    else
        AHB_WriteRegisterMask(GPIO_BASE + GPIO_BOUNCEEN_REG + GPIO_OFFSET, 0 << (pin - 32), 0x1 << (pin - 32));
}

#define GET_BUS_CLOCK		27000000 // TODO
void GPIO_SetDebounceClock(MMP_UINT32 clk)
{
	AHB_WriteRegister(GPIO_BASE + GPIO_BOUNCEPRESCALE_REG, GET_BUS_CLOCK / clk - 1);
}

void ithGpioSetMode(unsigned int pin, ITHGpioMode mode)
{
	MMP_UINT32 value, mask;

	// for UART1 output
	if (mode == GPIO_MODE_TX)
	{
		if (pin < 32)
		{
			value = 0x1 << pin;
			mask = 0x3 << pin;
			AHB_WriteRegisterMask(GPIO_BASE + 0xD8, value, mask);
		}
		else
		{
			value = 0x1 << (pin - 32);
			mask = 0x3 << (pin - 32);
			AHB_WriteRegisterMask(GPIO_BASE + 0xDC, value, mask);
		}

		mode = 0;
	}
	else if (mode == GPIO_MODE_RX)
	{
		if (pin < 32)
		{
			value = 0x1 << pin;
			mask = 0x3 << pin;
			AHB_WriteRegisterMask(GPIO_BASE + 0xE0, value, mask);
		}
		else
		{
			value = 0x1 << (pin - 32);
			mask = 0x3 << (pin - 32);
			AHB_WriteRegisterMask(GPIO_BASE + 0xE4, value, mask);
		}

		mode = 0;
	}

	if (pin < 16)
	{
		value = mode << (pin * 2);
		mask = 0x3 << (pin * 2);
		AHB_WriteRegisterMask(GPIO_BASE + 0x90, value, mask);
	}
	else if (pin < 32)
	{
		value = mode << ((pin - 16) * 2);
		mask = 0x3 << ((pin - 16) * 2);
		AHB_WriteRegisterMask(GPIO_BASE + 0x94, value, mask);
	}
	else if (pin < 48)
	{
		value = mode << ((pin - 32) * 2);
		mask = 0x3 << ((pin - 32) * 2);
		AHB_WriteRegisterMask(GPIO_BASE + 0x98, value, mask);
	}
	else
	{
		value = mode << ((pin - 48) * 2);
		mask = 0x3 << ((pin - 48) * 2);
		AHB_WriteRegisterMask(GPIO_BASE + 0x9C, value, mask);
	}
}	

void ithGpioCtrlEnable(unsigned int pin, ITHGpioCtrl ctrl)
{
    //ithEnterCritical();
    
    switch (ctrl)
    {
    case ITH_GPIO_PULL_ENABLE:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x14, 0x1 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x54, 0x1 << (pin - 32), 0x1 << (pin - 32));
        break;
        
    case ITH_GPIO_PULL_UP:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x18, 0x1 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x58, 0x1 << (pin - 32), 0x1 << (pin - 32));
        break;
        
    case ITH_GPIO_INTR_LEVELTRIGGER:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x30, 0x1 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x70, 0x1 << (pin - 32), 0x1 << (pin - 32));
        break;
        
    case ITH_GPIO_INTR_BOTHEDGE:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x34, 0x1 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x74, 0x1 << (pin - 32), 0x1 << (pin - 32));
        break;        

    case ITH_GPIO_INTR_TRIGGERFALLING:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x38, 0x1 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x78, 0x1 << (pin - 32), 0x1 << (pin - 32));
        break;
    }

    //ithExitCritical();
}

void ithGpioCtrlDisable(unsigned int pin, ITHGpioCtrl ctrl)
{
    //ithEnterCritical();
    
    switch (ctrl)
    {
    case ITH_GPIO_PULL_ENABLE:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x14, 0 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x54, 0 << (pin - 32), 0x1 << (pin - 32));
        break;
        
    case ITH_GPIO_PULL_UP:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x18, 0 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x58, 0 << (pin - 32), 0x1 << (pin - 32));
        break;
        
    case ITH_GPIO_INTR_LEVELTRIGGER:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x30, 0 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x70, 0 << (pin - 32), 0x1 << (pin - 32));
        break;
        
    case ITH_GPIO_INTR_BOTHEDGE:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x34, 0 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x74, 0 << (pin - 32), 0x1 << (pin - 32));
        break;        

    case ITH_GPIO_INTR_TRIGGERFALLING:
        if (pin < 32)
		AHB_WriteRegisterMask(GPIO_BASE + 0x38, 0 << pin, 0x1 << pin);
        else
		AHB_WriteRegisterMask(GPIO_BASE + 0x78, 0 << (pin - 32), 0x1 << (pin - 32));
        break;
    }

    //ithExitCritical();
}
#endif // defined(MM9070) || defined(MM9910) && !defined(__OPENRTOS__)

//=============================================================================
//                              Private Function Definition
//=============================================================================

static void 
_PADSEL_Set(
    MMP_UINT32 bits,
    MMP_BOOL bToggle)
{
#if defined(MM9070) || defined(MM9910)
#else
    MMP_UINT32 i = 0;
    MMP_UINT32 tag = DGPIO_TAG;
    MMP_UINT32 mask = 0;
    MMP_UINT32 value = 0;

    if (GET_PADSEL_TAG(bits))
        tag = PADSEL_TAG;

    bits = GET_BIT(bits);
    for (; i<PADSEL_BIT_COUNT; i++)
    {
        if ((padsel[i][0] & tag) && (padsel[i][0] & bits))
        {
            mask |= (1 << i);
            if (bToggle==MMP_FALSE)
                value |= (1 << i);
        }
        else if ((padsel[i][1] & tag) && (padsel[i][1] & bits))
        {
            mask |= (1 << i);
            if (bToggle==MMP_TRUE)
                value |= (1 << i);
        }
    }

    AHB_WriteRegisterMask(GPIO_BASE+GPIO_PADSEL_REG, value, mask);
#endif
}

static void 
_GPIO_Toggle(
    MMP_UINT32 gpio, 
    MMP_BOOL bToggle)
{
#if defined(MM9070) || defined(MM9910)
	#if defined(__OPENRTOS__)
	{
		if (bToggle == MMP_TRUE)
			ithGpioEnable(gpio);
	}
	#else
	{
		MMP_UINT32 mode = 0;
		MMP_UINT32 value = 0;
		MMP_UINT32 mask = 0x3;

		if (bToggle == MMP_TRUE)
		{
			if (gpio < 16)
			{
				value = mode << (gpio * 2);
				mask = mask << (gpio * 2);
				AHB_WriteRegisterMask(GPIO_BASE + 0x90, value, mask);
			}
			else if (gpio < 32)
			{
				value = mode << ((gpio - 16) * 2);
				mask = mask << ((gpio - 16) * 2);
				AHB_WriteRegisterMask(GPIO_BASE + 0x94, value, mask);
			}
			else if (gpio < 48)
			{
				value = mode << ((gpio - 32) * 2);
				mask = mask << ((gpio - 32) * 2);
				AHB_WriteRegisterMask(GPIO_BASE + 0x98, value, mask);
			}
			else
			{
				value = mode << ((gpio - 48) * 2);
				mask = mask << ((gpio - 48) * 2);
				AHB_WriteRegisterMask(GPIO_BASE + 0x9C, value, mask);
			}
		}
	}
	#endif
#else
    if (GET_GPIO_TAG(gpio) == DGPIO_TAG)
    {
        // additional enable bit [15] for DGPIO[3:0] at general register 0x0100
        if (0x0000000F & gpio)            
            HOST_WriteRegisterMask(MMP_GENERAL_GPIO_REG_100, (bToggle ? 0x8000 : 0), 0x8000);        
        
        _PADSEL_Set(gpio, bToggle);
    }
    else
    {
        MMP_UINT16 i = 0;
        MMP_UINT16 mask = 0;
        MMP_UINT16 value = 0;
        MMP_UINT16 address = MMP_GENERAL_GPIO_REG_100;
        
        if (GET_SGPIO_GROUP(gpio))
        {
            address = MMP_GENERAL_GPIO_REG_110;
            // additional enable bit [15] for SGPIO[41:30] at general register 0x0114
            if (0x0003FFC0 & gpio)
            {
                value = (bToggle ? 0x8000 : 0);              
                HOST_WriteRegisterMask(MMP_GENERAL_GPIO_REG_114, value, 0x8000);
            }
        }
            
        value = (bToggle ? 0x5400 : 0); // select bits [14], [12] and [10]
        for (; i<8; i++, gpio = (gpio >> 3)) 
        {
            if (mask = (0x0007 & gpio))
            {
                mask = GET_SGPIO_ENABLE_BIT(mask); // move mask to select bits [14], [12] and [10]
                HOST_WriteRegisterMask(address+i*2, value, mask);
                break;
            }
        }
    }          
#endif
}
