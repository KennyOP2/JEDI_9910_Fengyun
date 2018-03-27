#ifndef _INTR_H_
#define _INTR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mmp_types.h"
#include "host/ahb.h"
#ifndef WIN32
    #include "or32.h"
    #include "isr.h"
#endif

#if defined(WIN32)
    #define inline            __inline
#endif
#define uint32_t              MMP_UINT32
#if defined(__FREERTOS__) && !defined(__OPENRTOS__)
    #if defined(MM9070)
        #define ITH_INTR_BASE (0xDE200000 + 0x40)    // +0x00 for ARM, +0x40 for RISC0, +0x60 for RISC1
    #elif defined(MM9910)
        #define ITH_INTR_BASE (0xDE200000)           // +0x00 for RISC0, +0x20 for RISC1, +0x40 for RISC2
    #endif
#else
    #define ITH_INTR_BASE     0xDE200000
#endif

#define INTR_API              extern

#ifndef WIN32
static struct ISR_HANDLE isr_int_handle;
#endif

// Interrupt
#define ITH_INTR_IRQ1_EN_REG            0x04
#define ITH_INTR_IRQ1_CLR_REG           0x08
#define ITH_INTR_IRQ1_TRIGMODE_REG      0x0C
#define ITH_INTR_IRQ1_TRIGLEVEL_REG     0x10
#define ITH_INTR_IRQ1_STATUS_REG        0x14
#if defined(MM9910)
    #define ITH_INTR_IRQ2_EN_REG        0x104
    #define ITH_INTR_IRQ2_CLR_REG       0x108
    #define ITH_INTR_IRQ2_TRIGMODE_REG  0x10C
    #define ITH_INTR_IRQ2_TRIGLEVEL_REG 0x110
    #define ITH_INTR_IRQ2_STATUS_REG    0x114
#else
    #define ITH_INTR_FIQ1_EN_REG        0x24
    #define ITH_INTR_FIQ1_CLR_REG       0x28
    #define ITH_INTR_FIQ1_TRIGMODE_REG  0x2C
    #define ITH_INTR_FIQ1_TRIGLEVEL_REG 0x30
    #define ITH_INTR_FIQ1_STATUS_REG    0x34
    #define ITH_INTR_IRQ2_EN_REG        0x104
    #define ITH_INTR_IRQ2_CLR_REG       0x108
    #define ITH_INTR_IRQ2_TRIGMODE_REG  0x10C
    #define ITH_INTR_IRQ2_TRIGLEVEL_REG 0x110
    #define ITH_INTR_IRQ2_STATUS_REG    0x114
    #define ITH_INTR_FIQ2_EN_REG        0x124
    #define ITH_INTR_FIQ2_CLR_REG       0x128
    #define ITH_INTR_FIQ2_TRIGMODE_REG  0x12C
    #define ITH_INTR_FIQ2_TRIGLEVEL_REG 0x130
    #define ITH_INTR_FIQ2_STATUS_REG    0x134
#endif

// Interrupt
typedef enum
{
#if defined(MM9910)
    ITH_INTR_SW0        = 0,
    ITH_INTR_HDMI       = 1,
    ITH_INTR_DMA        = 2,
    ITH_INTR_TIMER      = 4, // no used
    ITH_INTR_TIMER1     = 5,
    ITH_INTR_TIMER2     = 6,
    ITH_INTR_TIMER3     = 7,
    ITH_INTR_TIMER4     = 8,
    ITH_INTR_TIMER5     = 9,
    ITH_INTR_TIMER6     = 10, // no used
    ITH_INTR_RTC        = 11,
    ITH_INTR_RTCSEC     = 12,
    ITH_INTR_RTCMIN     = 13,
    ITH_INTR_RTCHOUR    = 14,
    ITH_INTR_RTCDAY     = 15,
    ITH_INTR_RTCALARM   = 16,
    ITH_INTR_GPIO       = 17,
    ITH_INTR_WD         = 18,
    ITH_INTR_I2C        = 19,
    ITH_INTR_SSP0       = 20,
    ITH_INTR_SSP1       = 21,
    ITH_INTR_UART0_I0   = 22,
    ITH_INTR_UART0_I1   = 23,
    ITH_INTR_UART1_I0   = 24,
    ITH_INTR_UART1_I1   = 25,
    ITH_INTR_IR_RX      = 26,
    ITH_INTR_IR_TX      = 27,
    ITH_INTR_MC         = 28,
    ITH_INTR_CODA       = 29,
    ITH_INTR_HDMIRX     = 30,
    ITH_INTR_KB         = 31,

    ITH_INTR_SW1        = 32, // 0
    ITH_INTR_USB0       = 33, // 1
    ITH_INTR_USB1       = 34, // 2
    ITH_INTR_MAC        = 35, // 3
    // 4
    ITH_INTR_SD         = 37, // 5
    ITH_INTR_XD         = 38, // 6
    // 7
    // 8
    ITH_INTR_CPU0       = 41, // 9
    ITH_INTR_CPU1       = 42, // 10
    ITH_INTR_CPU2       = 43, // 11
    // 12
    ITH_INTR_ISP        = 45, // 13
    // 14
    ITH_INTR_I2S        = 47, // 15
    // 16
    ITH_INTR_DPU        = 49, // 17
    ITH_INTR_DECOMPRESS = 50, // 18
    ITH_INTR_CAPTURE    = 51, // 19
    ITH_INTR_TSI0       = 52, // 20
    // 21
    ITH_INTR_TSMUX      = 54, // 22
#else
    ITH_INTR_SW0        = 0,
    ITH_INTR_HDMI       = 1,
    ITH_INTR_DMA        = 2,
    ITH_INTR_TIMER      = 4,
    ITH_INTR_TIMER1     = 5,
    ITH_INTR_TIMER2     = 6,
    ITH_INTR_TIMER3     = 7,
    ITH_INTR_TIMER4     = 8,
    ITH_INTR_TIMER5     = 9,
    ITH_INTR_TIMER6     = 10,
    ITH_INTR_RTC        = 11,
    ITH_INTR_RTCSEC     = 12,
    ITH_INTR_RTCMIN     = 13,
    ITH_INTR_RTCHOUR    = 14,
    ITH_INTR_RTCDAY     = 15,
    ITH_INTR_RTCALARM   = 16,
    ITH_INTR_GPIO       = 17,
    ITH_INTR_WD         = 18,
    ITH_INTR_I2C        = 19,
    ITH_INTR_SSP0       = 20,
    ITH_INTR_SSP1       = 21,
    ITH_INTR_UART0      = 22,
    ITH_INTR_UART1      = 23,
    ITH_INTR_RC         = 24,
    ITH_INTR_CF         = 25,
    ITH_INTR_MC         = 26,

    ITH_INTR_SW1        = 32, // 0
    ITH_INTR_USB0       = 33, // 1
    ITH_INTR_USB1       = 34, // 2
    ITH_INTR_MAC        = 35, // 3
    ITH_INTR_MS         = 36, // 4
    ITH_INTR_SD         = 37, // 5
    ITH_INTR_XD         = 38, // 6
    ITH_INTR_NAND       = 40, // 8
    ITH_INTR_CPU0       = 41, // 9
    ITH_INTR_CPU1       = 42, // 10
    ITH_INTR_CMDQ       = 43, // 11
    ITH_INTR_FPC        = 44, // 12
    ITH_INTR_ISP        = 45, // 13
    ITH_INTR_LCD        = 46, // 14
    ITH_INTR_I2S        = 47, // 15
    ITH_INTR_OPENVG     = 48, // 16
    ITH_INTR_DPU        = 49, // 17
    ITH_INTR_DECOMPRESS = 50, // 18
    ITH_INTR_CAPTURE    = 51, // 19
    ITH_INTR_TSI0       = 52, // 20
    ITH_INTR_TSI1       = 53, // 21
    ITH_INTR_TSPARSER   = 54, // 22
#endif // defined(MM9910)
} ITHIntr;

typedef enum
{
    ITH_INTR_LEVEL = 0,
    ITH_INTR_EDGE  = 1
} ITHIntrTriggerMode;

typedef enum
{
    ITH_INTR_HIGH_RISING = 0,
    ITH_INTR_LOW_FALLING = 1
} ITHIntrTriggerLevel;

typedef void (*ithIntrHandler)(void *arg);

INTR_API void ithIntrInit(void);
INTR_API void ithIntrReset(void);
INTR_API void ithIntrSetTriggerModeIrq(ITHIntr intr, ITHIntrTriggerMode mode);
INTR_API void ithIntrSetTriggerLevelIrq(ITHIntr intr, ITHIntrTriggerLevel level);
INTR_API void ithIntrRegisterHandlerIrq(ITHIntr intr, ithIntrHandler handler, void *arg);
INTR_API void ithIntrDoIrq(void);
INTR_API MMP_BOOL ithIntrIsInit(void);

static inline void ithIntrEnableIrq(ITHIntr intr)
{
    if (!ithIntrIsInit())
        ithIntrInit();

    if (intr < 32)
        AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, 0x1 << intr, 0x1 << intr);
    else
        AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, 0x1 << (intr - 32), 0x1 << (intr - 32));
}

static inline void ithIntrDisableIrq(ITHIntr intr)
{
    if (intr < 32)
        AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, 0 << intr, 0x1 << intr);
    else
        AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, 0 << (intr - 32), 0x1 << (intr - 32));
}

static inline void ithIntrClearIrq(ITHIntr intr)
{
    if (intr < 32)
        AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, 0x1 << intr, 0x1 << intr);
    else
        AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, 0x1 << (intr - 32), 0x1 << (intr - 32));
}

static inline void ithIntrGetStatusIrq(uint32_t *intr1, uint32_t *intr2)
{
    AHB_ReadRegister(ITH_INTR_BASE + ITH_INTR_IRQ1_STATUS_REG, intr1);
    AHB_ReadRegister(ITH_INTR_BASE + ITH_INTR_IRQ2_STATUS_REG, intr2);
}

#if 0
static inline void ithIntrEnableFiq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, intr - 32);
}

static inline void ithIntrDisableFiq(ITHIntr intr)
{
    if (intr < 32)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, intr);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, intr - 32);
}

static inline bool ithIntrIsInputFiq(ITHIntr intr)
{
    if (intr < 32)
        return !!ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_STATUS_REG) & (0x1 << intr);
    else
        return !!ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_STATUS_REG) & (0x1 << (intr - 32));
}

static inline void ithIntrClearFiq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, intr - 32);
}

static inline void ithIntrGetStatusFiq(uint32_t *intr1, uint32_t *intr2)
{
    *intr1 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_STATUS_REG);
    *intr2 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_STATUS_REG);
}

INTR_API void ithIntrSetTriggerModeFiq(ITHIntr intr, ITHIntrTriggerMode mode);
INTR_API void ithIntrSetTriggerLevelFiq(ITHIntr intr, ITHIntrTriggerLevel level);
INTR_API void ithIntrRegisterHandlerFiq(ITHIntr intr, ithIntrHandler handler, void *arg);
INTR_API void ithIntrDoFiq(void);
#endif

#ifdef __cplusplus
}
#endif
#endif