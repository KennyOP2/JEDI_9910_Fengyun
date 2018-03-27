/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file host.h
 *
 * @author Vincent Lee
 */

#ifndef HOST_H
#define HOST_H

#include "mmp_types.h"
#include "mmp.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MMP_GENERAL_CONFIG_REG_00           0x0000
#define MMP_GENERAL_CONFIG_REG_02           0x0002
#define MMP_GENERAL_CONFIG_REG_04           0x0004
#define MMP_GENERAL_INTERRUPT_REG_06        0x0006
#define MMP_GENERAL_INTERRUPT_REG_08        0x0008
#define MMP_GENERAL_INTERRUPT_REG_0A        0x000A
#define MMP_GENERAL_INTERRUPT_REG_0C        0x000C

#define MMP_HOST_CLOCK_REG_10               0x0010
#define MMP_HOST_CLOCK_REG_12               0x0012
#define MMP_MEM_CLOCK_REG_14                0x0014
#define MMP_MEM_CLOCK_REG_16                0x0016
#define MMP_AHB_CLOCK_REG_18                0x0018
#define MMP_AHB_CLOCK_REG_1A                0x001A
#define MMP_APB_CLOCK_REG_1C                0x001C
#define MMP_APB_CLOCK_REG_1E                0x001E
#define MMP_APB_CLOCK_REG_20                0x0020
#define MMP_APB_CLOCK_REG_22                0x0022
#define MMP_2D_CLOCK_REG_24                 0x0024
#define MMP_2D_CLOCK_REG_26                 0x0026
#define MMP_LCD_CLOCK_REG_28                0x0028
#define MMP_LCD_CLOCK_REG_2A                0x002A
#define MMP_TVE_CLOCK_REG_2C                0x002C
#define MMP_TVE_CLOCK_REG_2E                0x002E
#define MMP_ISP_CLOCK_REG_30                0x0030
#define MMP_ISP_CLOCK_REG_32                0x0032
#define MMP_VIDEO_CLOCK_REG_34              0x0034
#define MMP_VIDEO_CLOCK_REG_36              0x0036
#define MMP_VIDEO_CLOCK_REG_38              0x0038
#define MMP_AUDIO_CLOCK_REG_3A              0x003A
#define MMP_AUDIO_CLOCK_REG_3C              0x003C
#define MMP_AUDIO_CLOCK_REG_3E              0x003E
#define MMP_PCR_CLOCK_REG_40                0x0040
#define MMP_PCR_CLOCK_REG_42                0x0042
#define MMP_RISC_CLOCK_REG_44               0x0044
#define MMP_USB_CLOCK_REG_46                0x0046
#define MMP_TSI_CLOCK_REG_48                0x0048
#define MMP_DEMOD_CLOCK_REG_4A              0x004A
#define MMP_GENERL_MISC_REG                 0x004E        // General MISC Register 

#define MMP_PROBE_SETTING_REG_70            0x0070
#define MMP_GENERAL_DFT_REG_72              0x0072
#define MMP_GENERAL_DFT_REG_74              0x0074
#define MMP_GENERAL_DFT_REG_76              0x0076
#define MMP_GENERAL_DFT_REG_78              0x0078
#define MMP_GENERAL_DFT_REG_7A              0x007A

#define MMP_OSCI_REG_88                     0x0088

#define MMP_DEMOD_PLL_REG_A0                0x00A0
#define MMP_DEMOD_PLL_REG_A2                0x00A2
#define MMP_DEMOD_PLL_REG_A4                0x00A4
#define MMP_DEMOD_PLL_REG_A6                0x00A6
#define MMP_DEMOD_PLL_REG_A8                0x00A8
#define MMP_DEMOD_PLL_REG_AA                0x00AA
#define MMP_DEMOD_PLL_REG_AC                0x00AC
#define MMP_DEMOD_PLL_REG_AE                0x00AE

#define MMP_GENERAL_GPIO_REG_100            0x0100
#define MMP_GENERAL_GPIO_REG_102            0x0102
#define MMP_GENERAL_GPIO_REG_104            0x0104
#define MMP_GENERAL_GPIO_REG_106            0x0106
#define MMP_GENERAL_GPIO_REG_108            0x0108
#define MMP_GENERAL_GPIO_REG_10A            0x010A
#define MMP_GENERAL_GPIO_REG_10C            0x010C
#define MMP_GENERAL_GPIO_REG_10E            0x010E
#define MMP_GENERAL_GPIO_REG_110            0x0110
#define MMP_GENERAL_GPIO_REG_112            0x0112
#define MMP_GENERAL_GPIO_REG_114            0x0114
#define MMP_GENERAL_GPIO_REG_116            0x0116
#define MMP_GENERAL_GPIO_REG_118            0x0118
#define MMP_GENERAL_GPIO_REG_11A            0x011A

#define MMP_HOST_BUS_CONTROLLER_REG_200     0x0200
#define MMP_HOST_BUS_CONTROLLER_REG_202     0x0202
#define MMP_HOST_BUS_CONTROLLER_REG_204     0x0204
#define MMP_HOST_BUS_CONTROLLER_REG_206     0x0206

// MMP_GENERAL_CONFIG_REG_00           0x0000
// MMP_GENERAL_CONFIG_REG_02           0x0002
// MMP_GENERAL_CONFIG_REG_04           0x0004
// MMP_GENERAL_INTERRUPT_REG_06        0x0006
// MMP_GENERAL_INTERRUPT_REG_08        0x0008
// MMP_GENERAL_INTERRUPT_REG_0A        0x000A
// MMP_GENERAL_INTERRUPT_REG_0C        0x000C
// 
// MMP_HOST_CLOCK_REG_10               0x0010
#define MMP_HOST_RESET                      0x1000  // [12] HOST reset

// MMP_HOST_CLOCK_REG_12               0x0012
// MMP_MEM_CLOCK_REG_14                0x0014

// MMP_MEM_CLOCK_REG_16                0x0016
#define MMP_MEM_RESET                       0x1000  // [12] MEM reset

// MMP_AHB_CLOCK_REG_18                0x0018
#define MMP_AHB_EN_DIV_NCLK                 0x8000  // [15]
#define MMP_AHB_UPD_SRC_NCLK                0x4000  // [14]
#define MMP_AHB_SRC_NCLK                    0x3000  // [13:12]
#define MMP_AHB_UPD_RAT_NCLK                0x0800  // [11]
#define MMP_AHB_RAT_NCLK                    0x07FF  // [10: 0]

// MMP_AHB_CLOCK_REG_1A                0x001A
#define MMP_AHB_RESET                       0x1000  // [12] AHB reset
#define MMP_AHB_EN_N1CLK                    0x0008  // [ 3] DMA clock

// MMP_APB_CLOCK_REG_1C                0x001C
#define MMP_APB_EN_DIV_WCLK                 0x8000  // [15]
#define MMP_APB_SRC_WCLK                    0x3000  // [13:12]
#define MMP_APB_UPD_RAT_WCLK                0x0800  // [11]
#define MMP_APB_RAT_WCLK                    0x07FF  // [10: 0]

// MMP_APB_CLOCK_REG_1E                0x001E
#define MMP_UART_RESET                      0x4000  // [14] UART reset
#define MMP_SDIP_RESET                      0x2000  // [13] SD_IP reset
#define MMP_APB_RESET                       0x1000  // [12] APB reset
#define MMP_APB_EN_W5CLK                    0x0800  // [11] PWM clock
#define MMP_APB_EN_W4CLK                    0x0200  // [ 9] IIC clock
#define MMP_APB_EN_W3CLK                    0x0080  // [ 7] SSP clock
#define MMP_APB_EN_W2CLK                    0x0020  // [ 5] Interrupt clock
#define MMP_APB_EN_W1CLK                    0x0008  // [ 3] GPIO clock
#define MMP_APB_EN_W0CLK                    0x0002  // [ 1] RTC, WTDG, IRCAP clock

// MMP_APB_CLOCK_REG_20                0x0020
#define MMP_APB_EN_W10CLK                   0x0200  // [ 9] KEBC clock
#define MMP_APB_EN_W9CLK                    0x0080  // [ 7] Memory Stick clock
#define MMP_APB_EN_W8CLK                    0x0020  // [ 5] SPDIF clock
#define MMP_APB_EN_W7CLK                    0x0008  // [ 3] SD_IP clock
#define MMP_APB_EN_W6CLK                    0x0002  // [ 1] UART clock

// MMP_APB_CLOCK_REG_22                0x0022
#define MMP_APB_EN_CLK48M                   0x8000  // [15]
#define MMP_APB_SRC_CLK48M                  0x4000  // [14]

// MMP_2D_CLOCK_REG_24                 0x0024
#define MMP_2D_EN_DIV_GCLK                  0x8000  // [15]
#define MMP_2D_SRC_GCLK                     0x3000  // [13:12]
#define MMP_2D_RAT_GCLK                     0x000F  // [ 3: 0]

// MMP_2D_CLOCK_REG_26                 0x0026
#define MMP_2DCMQ_RESET                     0x2000  // [13]
#define MMP_2D_RESET                        0x1000  // [12]
#define MMP_2D_EN_M3CLK                     0x0020  // [ 5] memory clock in command queue
#define MMP_2D_EN_M2CLK                     0x0008  // [ 3] memory clock in 2D
#define MMP_2D_EN_DG_M2CLK                  0x0004  // [ 2]
#define MMP_2D_EN_GCLK                      0x0002  // [ 1] 2D clock
#define MMP_2D_EN_DG_GCLK                   0x0001  // [ 0] 

// MMP_LCD_CLOCK_REG_28                0x0028
#define MMP_LCD_EN_DIV_DCLK                 0x8000  // [15]
#define MMP_LCD_SRC_DCLK                    0x3000  // [13:12]
#define MMP_LCD_RAT_DCLK                    0x00FF  // [ 7: 0]

// MMP_LCD_CLOCK_REG_2A                0x002A
#define MMP_LCDGBL_RESET                    0x2000  // [13]
#define MMP_LCD_RESET                       0x1000  // [12]
#define MMP_LCD_EN_M4CLK                    0x0008  // [ 3] memory clock in LCD
#define MMP_LCD_EN_DCLK                     0x0002  // [ 1] LCD clock

// MMP_TVE_CLOCK_REG_2C                0x002C
#define MMP_TVE_EN_DIV_ECLK                 0x8000  // [15]
#define MMP_TVE_SRC_ECLK                    0x3000  // [13:12]
#define MMP_TVE_RAT_ECLK                    0x000F  // [ 3: 0]

// MMP_TVE_CLOCK_REG_2E                0x002E
#define MMP_TVE_RESET                       0x1000  // [12]
#define MMP_TVE_EN_ECLK                     0x0002  // [ 1] TVE clock

// MMP_ISP_CLOCK_REG_30                0x0030
#define MMP_ISP_EN_DIV_ICLK                 0x8000  // [15]
#define MMP_ISP_SRC_ICLK                    0x3000  // [13:12]
#define MMP_ISP_RAT_ICLK                    0x000F  // [ 3: 0]

// MMP_ISP_CLOCK_REG_32                0x0032
#define MMP_ISPCMQGBL_RESET                 0x4000  // [14]
#define MMP_ISPGBL_RESET                    0x2000  // [13]
#define MMP_ISP_RESET                       0x1000  // [12]
#define MMP_ISP_EN_N5CLK                    0x0020  // [ 5] AHB clock
#define MMP_ISP_EN_M5CLK                    0x0008  // [ 3] memory clock in ISP
#define MMP_ISP_EN_ICLK                     0x0003  // [ 1] ISP clock

// MMP_VIDEO_CLOCK_REG_34              0x0034
#define MMP_VIDEO_EN_DIV_XMCLK              0x8000  // [15]
#define MMP_VIDEO_SRC_XMCLK                 0x3000  // [13:12]
#define MMP_VIDEO_RAT_XMCLK                 0x000F  // [ 3: 0]

// MMP_VIDEO_CLOCK_REG_36              0x0036
#define MMP_JPEGGBL_RESET                   0x8000  // [15]
#define MMP_JPEG_RESET                      0x4000  // [14]
#define MMP_VIDEOGBL_RESET                  0x2000  // [13]
#define MMP_VIDEO_RESET                     0x1000  // [12]
#define MMP_VIDEO_EN_M7CLK                  0x0800  // [11] memory clock in H.264
#define MMP_VIDEO_EN_X0CLK                  0x0002  // [ 1] MPEG2 clock

// MMP_VIDEO_CLOCK_REG_38              0x0038

// MMP_AUDIO_CLOCK_REG_3A              0x003A
#define MMP_AUDIO_EN_DIV_AMCLK              0x8000  // [15]
#define MMP_AUDIO_SRC_AMCLK                 0x3000  // [13:12]
#define MMP_AUDIO_RAT_AMCLK                 0x007F  // [ 6: 0]

// MMP_AUDIO_CLOCK_REG_3C              0x003C
#define MMP_AUDIO_EN_DIV_ZCLK               0x8000  // [15]
#define MMP_AUDIO_RAT_ZCLK                  0x03FF  // [ 9: 0]

// MMP_AUDIO_CLOCK_REG_3E              0x003E
#define MMP_DAC_RESET                       0x4000  // [14]
#define MMP_I2SGBL_RESET                    0x2000  // [13]
#define MMP_I2S_RESET                       0x1000  // [12]
#define MMP_AUDIO_EN_M9CLK                  0x0080  // [ 7] memory clock in ADC
#define MMP_AUDIO_EN_M8CLK                  0x0020  // [ 5] memory clock in DAC
#define MMP_AUDIO_EN_ZCLK                   0x0008  // [ 3] I2S DAC clock
#define MMP_AUDIO_EN_AMCLK                  0x0002  // [ 1] system clock in I2S alsve mode

// MMP_PCR_CLOCK_REG_40                0x0040
#if defined(IT9063)
#define MMP_PCR_EN_DIV_PCLK                 0x8001  // [15]
#else
#define MMP_PCR_EN_DIV_PCLK                 0x9009  // [15]
#endif
#define MMP_PCR_SRC_PCLK                    0x3000  // [13:12]
#define MMP_PCR_RAT_PCLK                    0x000F  // [ 3: 0]

// MMP_PCR_CLOCK_REG_42                0x0042
#define MMP_PCR_RESET                       0x1000  // [12] PCR reset
#define MMP_PCR_EN_PCLK                     0x0002  // [ 1] PCR clock

// MMP_RISC_CLOCK_REG_44               0x0044
#define MMP_RISC1_RESET                     0x4000  // [14]
#define MMP_JTAG_RESET                      0x2000  // [13]
#define MMP_RISC0_RESET                     0x1000  // [12]
#define MMP_RISC_EN_M10CLK                  0x0020  // [ 5] memory clock in RISC
#define MMP_RISC_EN_DG_M10CLK               0x0010  // [ 4]
#define MMP_RISC_EN_N4CLK                   0x0008  // [ 3] AHB clock in RISC1
#define MMP_RISC_EN_DG_N4CLK                0x0004  // [ 2]
#define MMP_RISC_EN_N3CLK                   0x0002  // [ 1] AHB clock in RISC0
#define MMP_RISC_EN_DG_N3CLK                0x0001  // [ 0]

// MMP_USB_CLOCK_REG_46                0x0046
#define MMP_USB_RESET                       0x1000  // [12]
#define MMP_USB_EN_N6CLK                    0x0008  // [ 3] AHB clock in USB
#define MMP_USB_EN_M11CLK                   0x0002  // [ 1] memory clock in USB

// MMP_TSI_CLOCK_REG_48                0x0048
#define MMP_TSI1_RESET                      0x2000  // [13]
#define MMP_TSI0_RESET                      0x1000  // [12]
#define MMP_TSI_EN_M13CLK                   0x0008  // [ 3] memory clock in TSI1
#define MMP_TSI_EN_M12CLK                   0x0002  // [ 1] memory clock in TSI0

// MMP_DEMOD_CLOCK_REG_4A              0x004A
#define MMP_DEMOD_RESET                     0x1000  // [12]
#define MMP_DEMOD_EN_M14CLK                 0x0002  // [ 1] memory clock in demodulator

/*MISC Register*/
#define REG_BIT_CLK_48M_EN                 (1u << 15)

// MMP_HOST_BUS_CONTROLLER_REG_202     0x0202
#define MMP_HOST_BUS_EN_MMIO_LCD            0x4000
#define MMP_HOST_BUS_EN_MMIO_ISP            0x2000
#define MMP_HOST_BUS_EN_MMIO_USB            0x1000
//#define MMP_HOST_BUS_EN_MMIO_NFC             0x0800 // un-used
#define MMP_HOST_BUS_EN_MMIO_FPC            0x0400
#define MMP_HOST_BUS_EN_MMIO_2D             0x0200
#define MMP_HOST_BUS_EN_MMIO_RISC0          0x0100
#define MMP_HOST_BUS_EN_MMIO_CMQ            0x0080
#define MMP_HOST_BUS_EN_MMIO_TSI1           0x0040
#define MMP_HOST_BUS_EN_MMIO_TSI0           0x0020
#define MMP_HOST_BUS_EN_MMIO_TVE            0x0010
#define MMP_HOST_BUS_EN_MMIO_VIDEO          0x0008
#define MMP_HOST_BUS_EN_MMIO_MPEG           0x0004
#define MMP_HOST_BUS_EN_MMIO_JPEG           0x0002
#define MMP_HOST_BUS_EN_MMIO_I2S            0x0001

// clock source definition
#define MMP_CLOCK_SRC_PLL1_OUTPUT1          0x0000
#define MMP_CLOCK_SRC_PLL1_OUTPUT2          0x1000
#define MMP_CLOCK_SRC_PLL2_OUTPUT1          0x2000
#define MMP_CLOCK_SRC_PLL2_OUTPUT2          0x3000

typedef enum MMP_RESET_TAG
{
    MMP_RESET_HOST,
    MMP_RESET_MEM,
    MMP_RESET_AHB,
    MMP_RESET_APB,
    MMP_RESET_SD_IP,
    MMP_RESET_UART,
    //MMP_RESET_2D,
    MMP_RESET_VG,
    MMP_RESET_CMQ,
    MMP_RESET_LCD,
    MMP_RESET_LCD_GLOBAL,
    MMP_RESET_TV_ENCODER,
    MMP_RESET_ISP,
    MMP_RESET_ISP_GLOBAL,
    MMP_RESET_ISP_CMQ_GLOBAL,
    MMP_RESET_VIDEO,
    MMP_RESET_VIDEO_GLOBAL,
    MMP_RESET_JPEG,
    MMP_RESET_JPEG_GLOBAL,
    MMP_RESET_I2S,
    MMP_RESET_I2S_GLOBAL,
    MMP_RESET_DAC,
    MMP_RESET_PCR,
    MMP_RESET_RISC0,
    MMP_RESET_JTAG,
    MMP_RESET_RISC1,
    MMP_RESET_USB,
    MMP_RESET_TSI0,
    MMP_RESET_TSI1,
    MMP_RESET_DEMODULATOR,
    MMP_RESET_COUNT
} MMP_RESET;

typedef enum MMP_INTERRUPT_TAG
{
    MMP_INTERRUPT_I2S,
    MMP_INTERRUPT_JPEG,
    MMP_INTERRUPT_MPEG,
    MMP_INTERRUPT_VIDEO,
    MMP_INTERRUPT_TVE,
    MMP_INTERRUPT_TSI0,
    MMP_INTERRUPT_TSI1,
    MMP_INTERRUPT_2D_CMQ,
    MMP_INTERRUPT_2D,
    MMP_INTERRUPT_FPC,
    MMP_INTERRUPT_NFC
} MMP_INTERRUPT;

// used by suspend mode
#define SRAM_START_ADDR 0x70000000

//=============================================================================
//                              Structure Definition                           
//=============================================================================

typedef struct REG_SETTING_TAG
{
	MMP_UINT16 reg;
	MMP_UINT16 data;
} REG_SETTING;

//=============================================================================
//                              Function Declaration
//=============================================================================

/* !!! referenced by usb device mode driver !!! */
MMP_API MMP_UINT8*
HOST_GetVramBaseAddress(
    void);

#ifdef WIN32
//MMP_API void
//HOST_ResetAllEngine(
//    void);

MMP_API void
HOST_ReadOffsetBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte);
#endif

MMP_API void
HOST_SetBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT8  bytePattern,
    MMP_ULONG  sizeInByte);

MMP_API void
HOST_WriteBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte);

MMP_API void
HOST_ReadBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte);

MMP_API void
HOST_ReadRegister(
    MMP_UINT16 destAddress,
    MMP_UINT16* data);

MMP_API void
HOST_WriteRegister(
    MMP_UINT16 destAddress,
    MMP_UINT16 data);

MMP_API void
HOST_WriteRegisterMask(
    MMP_UINT16 destAddress,
    MMP_UINT16 data,
    MMP_UINT16 mask);

MMP_API MMP_UINT8*
HOST_GetVramBaseAddress(
    void);

//MMP_API MMP_BOOL 
//HOST_IsEngineIdle(
//    MMP_UINT16 reg, 
//    MMP_UINT16 mask, 
//    MMP_UINT16 condition);
//
//MMP_API MMP_UINT32
//HOST_GetChipVersion(
//    void);
//
//MMP_API void
//HOST_ResetEngine(
//    MMP_UINT16 engine);
//
//MMP_API void
//HOST_EnableInterrupt(
//    MMP_UINT16 engine);
//
//MMP_API MMP_UINT16
//HOST_GetInterruptStatus(
//    MMP_UINT16 engine);
//
//// MMP_APB_CLOCK_REG_1C, MMP_APB_CLOCK_REG_1E, MMP_APB_CLOCK_REG_20, MMP_APB_CLOCK_REG_22
//MMP_API void
//HOST_APB_SetClockSource(
//    MMP_UINT16 clockSource);
//
//MMP_API MMP_UINT16 
//HOST_APB_GetClockSource(
//    void);
//
//MMP_API void
//HOST_APB_SetDivideRatio(
//    MMP_UINT16 divideRatio_int,
//    MMP_UINT16 divideRatio_frc);
//
//// MMP_2D_CLOCK_REG_24, MMP_2D_CLOCK_REG_26
//MMP_API void
//HOST_2D_EnableClock(
//    void);
//    
//MMP_API void
//HOST_2D_DisableClock(
//    void);
//
//MMP_API void
//HOST_2DCMQ_EnableClock(
//    void);
//    
//MMP_API void
//HOST_2DCMQ_DisableClock(
//    void);
//
//MMP_API void
//HOST_2D_SetClockSource(
//    MMP_UINT16 clockSource);
//
//MMP_API void
//HOST_2D_SetDivideRatio(
//    MMP_UINT16 divideRatio);
//
//// MMP_LCD_CLOCK_REG_28, MMP_LCD_CLOCK_REG_2A
//MMP_API void
//HOST_LCD_EnableClock(
//    void);
//
//MMP_API void
//HOST_LCD_DisableClock(
//    void);
//
//void 
//HOST_LCD_Reset(
//    void);
//
//MMP_API void
//HOST_LCD_SetClockSource(
//    MMP_UINT16 clockSource);
//
//MMP_API void
//HOST_LCD_FIRE(
//    void);
//
//MMP_API void
//HOST_LCD_SetDivideRatio(
//    MMP_UINT16 divideRatio);
//
//// MMP_TVE_CLOCK_REG_2C, MMP_TVE_CLOCK_REG_2E
//MMP_API void
//HOST_TVE_EnableClock(
//    void);
//    
//MMP_API void
//HOST_TVE_DisableClock(
//    void);
//
//MMP_API void
//HOST_TVE_SetClockSource(
//    MMP_UINT16 clockSource);
//
//MMP_API void
//HOST_TVE_SetDivideRatio(
//    MMP_UINT16 divideRatio);
//
//MMP_API void 
//HOST_TVE_Reset(
//    void);
    
// MMP_ISP_CLOCK_REG_30, MMP_ISP_CLOCK_REG_32
MMP_API void
HOST_ISP_EnableClock(
    void);
    
MMP_API void
HOST_ISP_DisableClock(
    void);

//MMP_API void
//HOST_ISP_SetClockSource(
//    MMP_UINT16 clockSource);
//
//MMP_API void
//HOST_ISP_SetDivideRatio(
//    MMP_UINT16 divideRatio);

MMP_API void 
HOST_ISP_Reset(
    void);

//MMP_API void
//HOST_ISPCMQ_Reset(
//    void);
//
//// MMP_VIDEO_CLOCK_REG_34, MMP_VIDEO_CLOCK_REG_36
//MMP_API void
//HOST_JPEG_EnableClock(
//    void);
//
//MMP_API void
//HOST_JPEG_DisableClock(
//    void);
//
//MMP_API void 
//HOST_JPEG_Reset(
//    void);
//
//MMP_API void
//HOST_MPEG2_EnableClock(
//    void);
//
//MMP_API void
//HOST_MPEG2_DisableClock(
//    void);
//
//MMP_API void
//HOST_MPEG2_Reset(
//    void);
//
//// MMP_PCR_CLOCK_REG_42
MMP_API void 
HOST_PCR_EnableClock(
    void);
//
//MMP_API void 
//HOST_PCR_DisableClock(
//    void);
//
//MMP_API void 
//HOST_PCR_Reset(
//    void);
//
//// MMP_USB_CLOCK_REG_46
//MMP_API void
//HOST_USB_EnableClock(
//    void);
//
//MMP_API void
//HOST_USB_DisableClock(
//    void);
//
//// MMP_TSI_CLOCK_REG_48
//MMP_API void
//HOST_TSI0_EnableClock(
//    void);
//
//MMP_API void
//HOST_TSI0_DisableClock(
//    void);
//
//MMP_API void
//HOST_TSI1_EnableClock(
//    void);
//
//MMP_API void
//HOST_TSI1_DisableClock(
//    void);
//
//// MMP_DEMOD_CLOCK_REG_4A
//MMP_API void
//HOST_DEMOD_EnableClock(
//    void);
//
//MMP_API void
//HOST_DEMOD_DisableClock(
//    void);
//
//MMP_API MMP_UINT16 
//HOST_PLL_GetClock(
//    MMP_UINT16 source);
//
//
/**
 * Storage
 */
MMP_API void* 
HOST_GetStorageSemaphore(void);

MMP_API MMP_BOOL 
HOST_IsCardInserted(MMP_UINT32 card);

MMP_API void 
HOST_CardPowerReset(MMP_UINT32 card, MMP_UINT32 sleeptime);

MMP_API MMP_BOOL 
HOST_IsCardLocked(MMP_UINT32 card);

MMP_API void 
HOST_StorageIoSelect(MMP_UINT32 card);

MMP_API void 
HOST_StorageIoUnSelect(MMP_UINT32 card);


#ifdef __cplusplus
}
#endif

#endif
