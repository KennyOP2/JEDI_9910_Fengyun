/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Global header file for ahb api.
 *
 * @author Jeimei Cheng
 * @version 0.1
 */
#ifndef AHB_H
#define AHB_H

#include "mmp_types.h"
#include "mmp.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================


#if defined(__FREERTOS__)

#define HOST_BASE               0xC0000000
#define FPC_BASE                0xC0001D00
#define AHBLAYER0_BASE          0xD0000000
#define AHBLAYER1_BASE          0xD0100000
#define DMA_BASE                0xD0200000
#define APB_BASE                0xD0300000
#define USB0_BASE               0xD0400000
#define USB1_BASE               0xD0500000
#define OPEN_VG_BASE            0xD0600000
#define NF_BASE                 0xD0700000
#define ETHERNET_BASE           0xD0800000
#define DPU_BASE                0xD0900000
#define GPIO_BASE               0xDE000000
#define I2C_BASE                0xDE100000
#define INT_CONT_BASE           0xDE200000
#define KEYPAD_BASE             0xDE300000
#define MSPRO_BASE              0xDE400000
#define RTC_BASE                0xDE500000
#define UART1_BASE              0xDE600000
#define UART2_BASE              0xDE700000
#define SSP1_BASE               0xDE800000
#define SSP2_BASE               0xDE900000
#define TIMER_BASE              0xDEA00000
#define WATCH_DOG_BASE          0xDEB00000
#define SIGNAL_CAP_BASE         0xDEC00000
#define SD_BASE                 0xDEE00000
#define SRAM_BASE               0xDED00400

#else

#define HOST_BASE               0x0000
#define FPC_BASE                0x1D00
#define AHBLAYER0_BASE          0x5400
#define AHBLAYER1_BASE          0x5800
#define DMA_BASE                0x5C00
#define APB_BASE                0x6000
#define USB0_BASE               0x6400
#define USB1_BASE               0x6800
#define OPEN_VG_BASE            0x6C00
#define NF_BASE                 0x7000
#define ETHERNET_BASE           0x7400
#define DPU_BASE                0x7800
#define GPIO_BASE               0x7C00
#define I2C_BASE                0x8000
#define INT_CONT_BASE           0x8400
#define KEYPAD_BASE             0x8800
#define MSPRO_BASE              0x8C00
#define RTC_BASE                0x9000
#define UART1_BASE              0x9400
#define UART2_BASE              0x9800
#define SSP1_BASE               0x9C00
#define SSP2_BASE               0xA000
#define TIMER_BASE              0xA400
#define WATCH_DOG_BASE          0xA800
#define SIGNAL_CAP_BASE         0xAC00
#define SD_BASE                 0xB000
#define SRAM_BASE               0xB400


#endif


//---------------------------
// Device Offset Definition
//---------------------------
//----------------------
// APB Bridge Config
//----------------------
#define APBBC_INCR_REG          0xc4
#define APBBC_ACCU_REG          0xc8

//-----------------
// INTC
//-----------------
#define INTCL_IRQSRC_REG        0x0
#define INTCL_IRQENA_REG        0x4
#define INTCL_IRQCLR_REG        0x8
#define INTCL_IRQTMD_REG        0xc
#define INTCL_IRQTLL_REG        0x10
#define INTCL_IRQSTA_REG        0x14
#define INTCL_IRQSFT_REG        0x18
//                              0x1c
#define INTCL_FIQSRC_REG        0x20
#define INTCL_FIQENA_REG        0x24
#define INTCL_FIQCLR_REG        0x28
#define INTCL_FIQTMD_REG        0x2c
#define INTCL_FIQTLL_REG        0x30
#define INTCL_FIQSTA_REG        0x34
#define INTCL_FIQSFT_REG        0x38
//                              0x3c
#define INTCL_BBIRQENA_REG      0x40
#define INTCL_SRIRQENA_REG      0x44
#define INTCL_BBIRQSTA_REG      0x48
#define INTCL_SRIRQSTA_REG      0x4c
#define INTCL_REVISE_REG        0x50
#define INTCL_FEANUM_REG        0x54
#define INTCL_IRQDEB_REG        0x58
#define INTCL_FIRDEB_REG        0x5c
#define INTCH_IRQSRC_REG        0x100
#define INTCH_IRQENA_REG        0x104
#define INTCH_IRQCLR_REG        0x108
#define INTCH_IRQTMD_REG        0x10c
#define INTCH_IRQTLL_REG        0x110
#define INTCH_IRQSTA_REG        0x114
#define INTCH_IRQSFT_REG        0x118
//                              0x11c
#define INTCH_FIQSRC_REG        0x120
#define INTCH_FIQENA_REG        0x124
#define INTCH_FIQCLR_REG        0x128
#define INTCH_FIQTMD_REG        0x12c
#define INTCH_FIQTLL_REG        0x130
#define INTCH_FIQSTA_REG        0x134
#define INTCH_FIQSFT_REG        0x138
//                              0x13c
#define INTCH_BBIRQENA_REG      0x140
#define INTCH_SRIRQENA_REG      0x144
#define INTCH_BBIRQSTA_REG      0x148
#define INTCH_SRIRQSTA_REG      0x14c
#define INTCH_REVISE_REG        0x150
#define INTCH_FEANUM_REG        0x154
#define INTCH_IRQDEB_REG        0x158
#define INTCH_FIRDEB_REG        0x15c

//-----------------
// TIMR
//-----------------
#define TIMR_T1COUT_REG         0x0
#define TIMR_T1LOAD_REG         0x4
#define TIMR_T1MAT1_REG         0x8
#define TIMR_T1MAT2_REG         0xc
#define TIMR_T2COUT_REG         0x10
#define TIMR_T2LOAD_REG         0x14
#define TIMR_T2MAT1_REG         0x18
#define TIMR_T2MAT2_REG         0x1c
//                              0x20
//                              0x24
//                              0x28
//                              0x2c
#define TIMR_S0CTRL_REG         0x30
#define TIMR_S0INTR_REG         0x34
#define TIMR_S0MASK_REG         0x38
#define TIMR_T3COUT_REG         0x40
#define TIMR_T3LOAD_REG         0x44
#define TIMR_T3MAT1_REG         0x48
#define TIMR_T3MAT2_REG         0x4c
#define TIMR_T4COUT_REG         0x50
#define TIMR_T4LOAD_REG         0x54
#define TIMR_T4MAT1_REG         0x58
#define TIMR_T4MAT2_REG         0x5c
#define TIMR_T1CTRL_REG         0x60
#define TIMR_T2CTRL_REG         0x64
#define TIMR_T3CTRL_REG         0x68
#define TIMR_T4CTRL_REG         0x6c
#define TIMR_S1CTRL_REG         0x70
#define TIMR_S1INTR_REG         0x74
#define TIMR_S1MASK_REG         0x78

//-------------------------
//AHB DMA
//-------------------------
#define DMA_INT_REG             0x0
#define DMA_INT_TC_REG          0x4
#define DMA_INT_TCCLR_REG       0x8
#define DMA_INT_ERRABT_REG      0xc
#define DMA_INT_ERRABTCLR_REG   0x10
#define DMA_TC_REG              0x14
#define DMA_ERRABT_REG          0x18
#define DMA_CNEH_REG            0x1C
#define DMA_CHBUSY_REG          0x20
#define DMA_CSR_REG             0x24
#define DMA_SYNC_REG            0x28
#define DMA_REVI_REG            0x30
#define DMA_FEAT_REG            0x34
#define DMA_C0CSR_REG           0x100
#define DMA_C0CFG_REG           0x104
#define DMA_C0SRC_REG           0x108
#define DMA_C0DST_REG           0x10c
#define DMA_C0LLP_REG           0x110
#define DMA_C0SIZ_REG           0x114
#define DMA_C1CSR_REG           0x120
#define DMA_C1CFG_REG           0x124
#define DMA_C1SRC_REG           0x128
#define DMA_C1DST_REG           0x12c
#define DMA_C1LLP_REG           0x130
#define DMA_C1SIZ_REG           0x134
#define DMA_C2CSR_REG           0x140
#define DMA_C2CFG_REG           0x144
#define DMA_C2SRC_REG           0x148
#define DMA_C2DST_REG           0x14c
#define DMA_C2LLP_REG           0x150
#define DMA_C2SIZ_REG           0x154
#define DMA_C3CSR_REG           0x160
#define DMA_C3CFG_REG           0x164
#define DMA_C3SRC_REG           0x168
#define DMA_C3DST_REG           0x16c
#define DMA_C3LLP_REG           0x170
#define DMA_C3SIZ_REG           0x174
#define DMA_C4CSR_REG           0x180
#define DMA_C4CFG_REG           0x184
#define DMA_C4SRC_REG           0x188
#define DMA_C4DST_REG           0x18c
#define DMA_C4LLP_REG           0x190
#define DMA_C4SIZ_REG           0x194
#define DMA_C5CSR_REG           0x1a0
#define DMA_C5CFG_REG           0x1a4
#define DMA_C5SRC_REG           0x1a8
#define DMA_C5DST_REG           0x1ac
#define DMA_C5LLP_REG           0x1b0
#define DMA_C5SIZ_REG           0x1b4
#define DMA_C6CSR_REG           0x1c0
#define DMA_C6CFG_REG           0x1c4
#define DMA_C6SRC_REG           0x1c8
#define DMA_C6DST_REG           0x1cc
#define DMA_C6LLP_REG           0x1d0
#define DMA_C6SIZ_REG           0x1d4
#define DMA_C7CSR_REG           0x1e0
#define DMA_C7CFG_REG           0x1e4
#define DMA_C7SRC_REG           0x1e8
#define DMA_C7DST_REG           0x1ec
#define DMA_C7LLP_REG           0x1f0
#define DMA_C7SIZ_REG           0x1f4

//-------------------------
//WATCH DOG TIMER
//-------------------------
#define WDG_CONT_REG            0x0
#define WDG_LOAD_REG            0x4
#define WDG_REST_REG            0x8
#define WDG_CTRL_REG            0xc
#define WDG_STAT_REG            0x10
#define WDG_CLER_REG            0x14
#define WDG_ILEN_REG            0x18
#define WDG_REFRESH_ID          0x5ab9

//-------------------------
//Real Time Clock
//-------------------------
#define RTC_SECO_REG            0x0
#define RTC_MINU_REG            0x4
#define RTC_HOUR_REG            0x8
#define RTC_DAYS_REG            0xc
#define RTC_ASEC_REG            0x10
#define RTC_AMIN_REG            0x14
#define RTC_AHOU_REG            0x18
#define RTC_RECO_REG            0x1c
#define RTC_CTRL_REG            0x20
#define RTC_WSEC_REG            0x24
#define RTC_WMIN_REG            0x28
#define RTC_WHOU_REG            0x2c
#define RTC_WDAY_REG            0x30
#define RTC_INTR_REG            0x34
#define RTC_DIVI_REG            0x38
#define RTC_REVI_REG            0x3c

//-------------------------
//SPI
//-------------------------
#define SPI_CTRL0_REG           0x0
#define SPI_CTRL1_REG           0x4
#define SPI_CTRL2_REG           0x8
#define SPI_STAT_REG            0xc
#define SPI_INTC_REG            0x10
#define SPI_INTS_REG            0x14
#define SPI_DATA_REG            0x18
#define SPI_AC97_REG            0x20
#define SPI_REVI_REG            0x40
#define SPI_FEAT_REG            0x44

//--------------------------
//GPIO
//--------------------------
#define GPIO_DATAOT_REG         0x0
#define GPIO_DATAIN_REG         0x4
#define GPIO_PINDIR_REG         0x8
#define GPIO_DATAST_REG         0xc
#define GPIO_DATACL_REG         0x10
#define GPIO_INTREN_REG			0x1C
#define GPIO_INTRMASKSTATE_REG	0x24
#define GPIO_INTRCLR_REG		0x2C
#define GPIO_INTRTRIG_REG		0x30
#define GPIO_BOUNCEEN_REG		0x3C
#define GPIO_RTCRST_REG         0x4c
#define GPIO_BOUNCEPRESCALE_REG	0x80
#define GPIO_PADSEL_REG         0x90
#define GPIO_PADSEL_REG_2       0x94
#define GPIO_PADSEL_REG_3       0x98

//--------------------------
//I2C
//--------------------------
#define I2C_CR_REG              0x0
#define I2C_SR_REG              0x4
#define I2C_CD_REG              0x8
#define I2C_DR_REG              0xc
#define I2C_SA_REG              0x10
#define I2C_GS_REG              0x14
#define I2C_BM_REG              0x18
#define I2C_RV_REG              0x30
#define I2C_FE_REG              0x34

//-------------------------
//CAPTURE
//-------------------------
#define CAP_CCR_REG             0x0
#define CAP_CSR_REG             0x4
#define CAP_CPR_REG             0x8
#define CAP_CAP_REG             0xc

//-------------------------
//IR TX
//-------------------------
#define CAP_SCR_REG             0x10
#define CAP_SSR_REG             0x14
#define CAP_SPR_REG             0x18
#define CAP_SMR_REG             0x1c
#define CAP_SDR_REG             0x20

//-------------------------
//Keypad
//-------------------------
#define KPD_CR_REG              0x0
#define KPD_SRD_REG             0x4
#define KPD_RS_REG              0x8
#define KPD_SR_REG              0xc
#define KPD_IS_REG              0x10
#define KPD_KR_REG              0x14
#define KPD_KT_REG              0x18
#define KPD_IM_REG              0x1c
#define KPD_CX_REG              0x30
#define KPD_CY_REG              0x34
#define KPD_AS_REG              0x38
#define KPD_RR_REG              0x50
#define KPD_FR_REG              0x54

//-------------------------
//SPDIF (SSP)
//-------------------------
#define SSP_CONTROL_REG0        0x0         // SSP Control Register 0
#define SSP_CONTROL_REG1        0x4         // SSP Control Register 1
#define SSP_CONTROL_REG2        0x8         // SSP Control Register 2
#define SSP_STATUS_REG          0xC         // SSP Status Register
#define SSP_INT_CONTROL_REG     0x10        // Interrupt Control Register
#define SSP_INT_STATUS_REG      0x14        // Interrupt Status Register
#define SSP_T_R_DATA_REG        0x18        // SSP Transmit/Receive Data Register
#define SPDIF_STATUS_BIT_REG0   0x24        // SPDIF Status Bit Register
#define SPDIF_STATUS_BIT_REG1   0x28
#define SPDIF_USER_BIT_REG0     0x2C        // SPDIF User Bit Register
#define SPDIF_USER_BIT_REG1     0x30
#define SPDIF_USER_BIT_REG2     0x34
#define SPDIF_USER_BIT_REG3     0x38
#define SPDIF_USER_BIT_REG4     0x3C
#define SPDIF_USER_BIT_REG5     0x40
#define SPDIF_USER_BIT_REG6     0x44
#define SPDIF_USER_BIT_REG7     0x48
#define SPDIF_USER_BIT_REG8     0x4C
#define SPDIF_USER_BIT_REG9     0x50
#define SPDIF_USER_BIT_REG10    0x54
#define SPDIF_USER_BIT_REG11    0x58
#define SPDIF_REV_REG           0x60        // SPDIF Revision Register
#define SSP_FEATURE_REG         0x64        // SSP Feature Register

#define SPDIF_EXTRA_CONTROL_REG 0x5C        // I2S and other compressed data control register.
#define SPDIF_IO_CONTROL_REG    0x68        // SPDIF IO Control

#if 1
#define MUX_SELECT_KBC_CLK      (2 << 14)
#define MUX_SELECT_KBC_DAT      (2 << 16)
#define MUX_MASK_GPIO13         (1 << 19 | 0 << 18)
#define MUX_MASK_GPIO14         (1 << 21 | 0 << 20)
#else
#define MUX_SELECT_KBC_CLK      (2 << 17)
#define MUX_SELECT_KBC_DAT      (2 << 21)
#define MUX_MASK_GPIO9          (1 << 18 | 1 << 17)
#define MUX_MASK_GPIO11         (1 << 22 | 1 << 21)
#endif

//TODO
//#define INTR_KBC_SEL			0x68D00088
#define INTR_KBC_SEL            0xE088
//=============================================================================
//                              Structure Definition
//=============================================================================

MMP_API void
AHB_ReadRegister(
    MMP_UINT32 destAddress,
    MMP_UINT32* data);

MMP_API void
AHB_WriteRegister(
    MMP_UINT32 destAddress,
    MMP_UINT32 data);

MMP_API void
AHB_WriteRegisterMask(
    MMP_UINT32 destAddress,
    MMP_UINT32 data,
    MMP_UINT32 mask);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef AHB_H


