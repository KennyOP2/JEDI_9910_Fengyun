/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file gpio.h
 *
 * @author Vincent Lee
 */

#ifndef GPIO_H
#define GPIO_H

#include "mmp.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(MM9070) || defined(MM9910)
#define DGPIO_TAG			(0)
#define DGPIO(pin)			(pin)
#define SGPIO(pin)          (pin)
#else
// DGPIO( 0-15) = 0x800X-XXXX
#define DGPIO_TAG           (0x80 << 24)
#define DGPIO(pin)          (DGPIO_TAG | (1 << pin))

// SGPIO( 0-23) = 0x00XX-XXXX
// SGPIO(24-41) = 0x010X-XXXX
#define SGPIO(pin)          (((pin/24) << 24) | (1 << (pin%24)))

// PADSEL_DEF() = 0x40XX-XXXX
#define PADSEL_TAG          (0x40 << 24)
#define PADSEL_DEF(def)     (PADSEL_TAG | (1 << def))
#endif

typedef enum GPIO_STATE_TAG
{
    GPIO_STATE_LO,
    GPIO_STATE_HI
} GPIO_STATE;

typedef enum GPIO_MODE_TAG
{
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT
} GPIO_MODE;

#if defined(MM9070) || defined(MM9910)
typedef enum PADSEL_MODE_TAG
{
	GPIO_MODE0, ITH_GPIO_MODE0 = 0,
	GPIO_MODE1, ITH_GPIO_MODE1 = 1,
	GPIO_MODE2, ITH_GPIO_MODE2 = 2,
	GPIO_MODE3, ITH_GPIO_MODE3 = 3,
	GPIO_MODE_TX, ITH_GPIO_MODE_TX = 4,
	GPIO_MODE_RX, ITH_GPIO_MODE_RX = 5
} PADSEL_MODE;

typedef PADSEL_MODE ITHGpioMode;

#if !defined(__OPENRTOS__)
#endif // #ifndef __OPENRTOS
/**
 * GPIO interupt controls definition.
 */
typedef enum
{
    ITH_GPIO_PULL_ENABLE           = 0, ///< GPIO pin pull enable
    ITH_GPIO_PULL_UP               = 1, ///< GPIO pin pull up
    ITH_GPIO_INTR_LEVELTRIGGER     = 2, ///< GPIO interrupt trigger method
    ITH_GPIO_INTR_BOTHEDGE         = 3, ///< GPIO interrupt edge trigger by both edge
    ITH_GPIO_INTR_TRIGGERFALLING   = 4, ///< GPIO interrupt trigger by falling edge
    ITH_GPIO_INTR_TRIGGERLOW       = 4  ///< GPIO interrupt trigger by low level
} ITHGpioCtrl;

/**
 * Enables specified controls.
 *
 * @param pin the GPIO pin to enable.
 * @param ctrl the controls to enable.
 */
void ithGpioCtrlEnable(unsigned int pin, ITHGpioCtrl ctrl);

/**
 * Disables specified controls.
 *
 * @param pin the GPIO pin to disable.
 * @param ctrl the controls to disable.
 */
void ithGpioCtrlDisable(unsigned int pin, ITHGpioCtrl ctrl);

#else
// available definition is from PADSEL_DEF(0) to PADSEL_DEF(23)
typedef enum PADSEL_MODE_TAG
{
    PADSEL_IIC         = PADSEL_DEF(0),
    PADSEL_DEMOD_IIC   = PADSEL_DEF(1)
} PADSEL_MODE;
#endif

//=============================================================================
//                              Function Declaration
//=============================================================================

MMP_API void
GPIO_Enable(
    MMP_UINT32 gpio);

MMP_API void
GPIO_Disable(
    MMP_UINT32 gpio);

MMP_API MMP_UINT32
GPIO_GetNum(
    MMP_UINT32 gpio);

MMP_API void
GPIO_SetMode(
    MMP_UINT32 gpio, 
    GPIO_MODE mode);

MMP_API void
GPIO_SetState(
    MMP_UINT32 gpio, 
    GPIO_STATE state);

MMP_API GPIO_STATE
GPIO_GetState(
    MMP_UINT32 gpio);

MMP_API void
PADSEL_Enable(
    PADSEL_MODE mode);

MMP_API void
PADSEL_Disable(
    PADSEL_MODE mode);

#if defined(MM9070) || defined(MM9910) && !defined(__OPENRTOS__)

typedef void (*GpioIntrHandler)(MMP_UINT32 pin, void* arg);

MMP_API void GPIO_EnableIntr(MMP_UINT32 pin);

MMP_API void GPIO_ClearIntr(MMP_UINT32 pin);

MMP_API void GPIO_RegisterIntrHandler(MMP_UINT32 pin, GpioIntrHandler handler, void* arg);

MMP_API void GPIO_DoIntr(void);

MMP_API void GPIO_SetDebounceClock(MMP_UINT32 clk);

MMP_API void GPIO_EnableBounce(MMP_UINT32 pin);

MMP_API void GPIO_DisableBounce(MMP_UINT32 pin);

MMP_API void ithGpioSetMode(unsigned int pin, ITHGpioMode mode);

#endif // defined(MM9070) || defined(MM9910) && !defined(__OPENRTOS__)


#ifdef __cplusplus
}
#endif

#endif
