/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Interrupt Driver API header file.
 *
 * @author Irene Lin
 */
#ifndef MMP_INTR_H
#define MMP_INTR_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if defined(_WIN32) || defined(_WIN32_WCE)

	#if defined(INTR_EXPORTS)
		#define INTR_API __declspec(dllexport)
	#else
		#define INTR_API __declspec(dllimport)
	#endif
#else
	#define INTR_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */


//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
/**
 * for interrupt controller 1
 */
#define IRQ_SW1                 (0)
#define IRQ_HOST                (1)
#define IRQ_AHBL0               (2)
#define IRQ_AHBL1               (3)
#define IRQ_DMA                 (4)
#define IRQ_DMA_TERMINAL_CNT    (5)
#define IRQ_DMA_ERR             (6)
#define IRQ_APB                 (7)
#define IRQ_GPIO                (8)
#define IRQ_I2C                 (9)
#define IRQ_KEYBOARD_RX         (10)
#define IRQ_KEYBOARD_TX         (11)
#define IRQ_KEYBOARD            (12)
#define IRQ_KBC                 (13)
#define IRQ_MS                  (14)
#define IRQ_RTC                 (15)
#define IRQ_SPDIF               (16)
#define IRQ_SPI                 (17)
#define IRQ_TIMER1              (18)
#define IRQ_TIMER2              (19)
#define IRQ_SD                  (20)
#define IRQ_TIMER               (21)
#define IRQ_WATCH_DOG           (22)
#define IRQ_WATCH_DOG_EXT       (23)
#define IRQ_UART1               (24)
#define IRQ_UART2               (25)
#define IRQ_IR                  (26)
#define IRQ_RTC_SEC             (27)
#define IRQ_RTC_MIN             (28)
#define IRQ_RTC_HOUR            (29)
#define IRQ_RTC_DAY             (30)
#define IRQ_RTC_ALARM           (31)
#define IRQ_INTR1_MAX           IRQ_RTC_ALARM

/**
 * for interrupt controller 2
 */
#define IRQ_INTR2_SHT           (32)
#define IRQ_SW2                 (IRQ_INTR2_SHT+0)
#define IRQ_CPU0                (IRQ_INTR2_SHT+1)
#define IRQ_CPU1                (IRQ_INTR2_SHT+2)
#define IRQ_RESERVED            (IRQ_INTR2_SHT+3)
#define IRQ_USB0_HOST           (IRQ_INTR2_SHT+4)
#define IRQ_USB0_DEVICE         (IRQ_INTR2_SHT+5)
#define IRQ_USB1_HOST           (IRQ_INTR2_SHT+6)
#define IRQ_USB1_DEVICE         (IRQ_INTR2_SHT+7)
#define IRQ_INTR2_MAX           (IRQ_USB1_DEVICE-IRQ_INTR2_SHT)


/**
 * for HOST group interrupt
 */
#define IRQ_HOST_SHT            (64)
#define IRQ_I2S                 (IRQ_HOST_SHT+0)
#define IRQ_JPEG                (IRQ_HOST_SHT+1)
#define IRQ_MPEG                (IRQ_HOST_SHT+2)
#define IRQ_VIDEO               (IRQ_HOST_SHT+3)
#define IRQ_TV_ENC              (IRQ_HOST_SHT+4)
#define IRQ_TSI0                (IRQ_HOST_SHT+5)
#define IRQ_TSI1                (IRQ_HOST_SHT+6)
#define IRQ_CMDQ                (IRQ_HOST_SHT+7)
#define IRQ_RISC                (IRQ_HOST_SHT+8)
#define IRQ_2D                  (IRQ_HOST_SHT+9)
#define IRQ_FPC                 (IRQ_HOST_SHT+10)
#define IRQ_NFC                 (IRQ_HOST_SHT+11)
#define IRQ_HOST_MAX            (IRQ_NFC-IRQ_HOST_SHT)

/**
 * for GPIO group interrupt
 */
#define IRQ_GPIO_SHT            (86)
#define IRQ_GPIO_MAX            (IRQ_GPIO_SHT+15)


/**
 * for interrupt related flags
 */
typedef enum MMP_INTR_FLAG_TAG
{
    INTR_FLAG_TRIGGER_MODE           = (0x1 << 0),
#define EDGE_TRIGGER     (0x1 << 0)
#define LEVEL_TRIGGER    (0)

    INTR_FLAG_TRIGGER_LEVEL          = (0x1 << 1),
#define ACTIVE_LOW      (0x1 << 1)
#define ACTIVE_HIGH     (0)
#define FALLING_EDGE    (0x1 << 1)
#define RISING_EDGE     (0)

    INTR_FLAG_GPIO_TRIGGER_MODE      = (0x1 << 2),
#define GPIO_LEVEL_TRIGGER    (0x1 << 2)
#define GPIO_EDGE_TRIGGER     (0)

    INTR_FLAG_GPIO_BOTH_EDGE         = (0x1 << 3),
#define GPIO_BOTH_EDGES       (0x1 << 3)
#define GPIO_SINGLE_EDGE      (0)

    INTR_FLAG_GPIO_TRIGGER_LEVEL     = (0x1 << 4),
#define GPIO_ACTIVE_LOW       (0x1 << 4)
#define GPIO_ACTIVE_HIGH      (0)
#define GPIO_FALLING_EDGE     (0x1 << 4)
#define GPIO_RISING_EDGE      (0)

    INTR_FLAG_GPIO_BOUNCE_EN         = (0x1 << 5),
#define GPIO_BOUNCE_EN        (0x1 << 5)
#define GPIO_BOUNCE_DIS       (0)
} MMP_INTR_FLAG;



/**
 * for isr 
 */
typedef MMP_INT (*MMP_IRQ_HANDLER)(void *);
#define IRQ_NONE	(0)
#define IRQ_HANDLED	(1)


//=============================================================================
//                              Function Declaration
//=============================================================================
#if defined(__FREERTOS__) && defined(ENABLE_INTR)

INTR_API MMP_INT mmpIntrInitialize(void);

/**
 * Interrupt request irq and register isr routine.
 */
INTR_API MMP_INT mmpIntrRequestIrq(
                    MMP_UINT32 irq,
                    MMP_IRQ_HANDLER handler,
                    MMP_UINT32 flags,
                    void* data);

/**
 * Interrupt enable/disable.
 */
INTR_API MMP_INT mmpIntrEnableIrq(
                    MMP_UINT32 irq,
                    MMP_BOOL enable);

#else // #if defined(__FREERTOS__)

#define mmpIntrInitialize()              MMP_NULL
#define mmpIntrRequestIrq(a,b,c,d)       MMP_NULL
#define mmpIntrEnableIrq(a,b)            MMP_NULL

#endif // #if defined(__FREERTOS__)



#ifdef __cplusplus
}
#endif

#endif /* MMP_INTR_H */
