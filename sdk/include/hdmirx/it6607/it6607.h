///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iT6607.h>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: HDMIRX_SAMPLE_3.11
//******************************************/

#ifndef _IT6607_H_
#define _IT6607_H_

#include "hdmirx/it6607/config.h"

#ifdef _MCU_8051_
    #include "mcu.h"
//#include "it6607/io.h"
    #include "hdmirx/it6607/utility.h"
    #include "hdmirx/it6607/TimerProcess.h"
#else
//#include <windows.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include "hdmirx/it6607/typedef.h"
//#include "it6607/io.h" // for Windows
#endif

#include "hdmirx/it6607/debug.h"

// #include "EDID.h"
// #include "HDCP.h"

unsigned int gIsAudioReset;
unsigned int gtTriStateEnable;

#define Switch_HDMIRX_Bank(x) HDMIRX_SetI2C_Byte(0x0f, 1, (x) & 1)

#define HDMI_RX_I2C_SLAVE_ADDR     0x90

#define REG_RX_VID_L               0x00
#define REG_RX_VID_H               0x01
#define REG_RX_DEVID_L             0x02
#define REG_RX_DEVID_H             0x03
#define REG_RX_DEVREV              0x04
#define REG_RX_RST_CTRL            0x05
#define NORMAL_RST_VAL             0x00

#define B_CDRRST                   (1 << 7)
#define B_EN_CDRAUTORST            (1 << 6)
#define B_EN_AUTOVDORST            (1 << 5)
#define B_REGRST                   (1 << 4)
#define B_HDCPRST                  (1 << 3)
#define B_AUDRST                   (1 << 2)
#define B_VDORST                   (1 << 1)
#define B_SWRST                    (1 << 0)

#define REG_RX_PWD_CTRL0           0x06
#define B_PWD_ALL                  (1 << 0)

#define REG_RX_PWD_CTRL1           0x07
#define B_DIS_PIX_RPT              (1 << 6)
#define B_PORT_SEL_B               (1 << 4)
#define B_PORT_SEL_A               (0 << 4)
#define B_PORT_MASK                (1 << 7)
#define B_DIS_PIX_RPT              (1 << 6)
#define B_HF_I2C_CLK               (1 << 5)
#define B_PWD_AFEALL               (1 << 3)
#define B_PWDC_ETC                 (1 << 2)
#define B_PWDC_SRV                 (1 << 1)
#define B_EN_AUTOPWD               (1 << 0)

#define REG_RX_VIO_CTRL            0x08
#define REG_RX_AIO_CTRL            0x09
#define REG_RX_OTP_CTRL            0x0A
#define REG_RX_OTP_XOR             0x0B
#define REG_RX_BIST_CTRL           0x0C
#define REG_RX_BIST_RESULT1        0x0D
#define REG_RX_BIST_RESULT2        0x0E

#define REG_RX_BLOCK_SEL           0x0F

#define REG_RX_SYS_STATUS          0x10

#define B_RXPLL_LOCK               (1 << 7)
#define B_RXCK_SPEED               (1 << 6)
#define B_RXCK_VALID               (1 << 5)
#define B_HDMIRX_MODE              (1 << 4)

#define B_SCDT                     (1 << 2)
#define B_VCLK_DET                 (1 << 1)
#define B_PWR5V_DET                (1 << 0)
#define B_PWR5V_DET_PORTA          (1 << 0)
#define B_PWR5V_DET_PORTB          (1 << 3)

#define REG_RX_HDCP_CTRL           0x11
#define REG_RX_HDCP_STATUS         0x12
#define B_HDCP_MODE                (1 << 0)

#define REG_RX_INTERRUPT1          0x13
#define REG_RX_INTERRUPT_MASK1     0x16

#define B_VIDMODE_CHG              (1 << 5)
#define B_HDMIMODE_CHG             (1 << 4)
#define B_SCDTOFF                  (1 << 3)
#define B_SCDTON                   (1 << 2)
#define B_PWR5VOFF                 (1 << 1)
#define B_PWR5VON                  (1 << 0)

#define REG_RX_INTERRUPT2          0x14
#define REG_RX_INTERRUPT_MASK2     0x17
#define B_PKT_CLR_MUTE             (1 << 7)
#define B_NEW_AUD_PKT              (1 << 6)
#define B_NEW_ACP_PKT              (1 << 5)
#define B_NEW_SPD_PKT              (1 << 4)
#define B_NEW_MPG_PKG              (1 << 3)
#define B_NEW_AVI_PKG              (1 << 2)
#define B_NO_AVI_RCV               (1 << 1)
#define B_PKT_SET_MUTE             (1 << 0)

#define REG_RX_INTERRUPT3          0x15
#define REG_RX_INTERRUPT_MASK3     0x18
#define B_AUTOAUDMUTE              (1 << 4)
#define B_AUDFIFOERR               (1 << 3)
#define B_ECCERR                   (1 << 2)
#define B_R_AUTH_DONE              (1 << 1)
#define B_R_AUTH_START             (1 << 0)

#define REG_RX_INTERRUPT4          0x8B
#define REG_RX_INTERRUPT_MASK4     0x8C

#define B_CD_DET                   (1 << 4)
#define B_GENPKT_DET               (1 << 2)
#define B_ISRC2_DET                (1 << 1)
#define B_ISRC1_DET                1

#define B_SYM_ERR                  (1 << 3)

#define REG_RX_INTERRUPT_CTRL1     0x18
#define B_CLR_HDCP_INT             (1 << 7)

#define REG_RX_INTERRUPT_CTRL      0x19

#define    B_CLR_MODE_INT          (1 << 0)
#define    B_CLR_PKT_INT           (1 << 1)
#define    B_CLR_ECC_INT           (1 << 2)
#define    B_CLR_AUDIO_INT         (1 << 3)
#define    B_INTPOL                (1 << 4)
#define    B_INTROUTTYPE           (1 << 5)
#define B_CLR_MUTECLR_INT          (1 << 6)
#define B_CLR_MUTESET_INT          (1 << 7)

#define REG_RX_MISC_CTRL           0x1A

#define B_RXCK_FILT                (1 << 5)
#define B_TIMEOUT_EN               (1 << 4)
#define B_EN_DEBUG                 (1 << 3)
#define B_DE_POL                   (1 << 2)
#define B_VSYNC_OUT_POL            (1 << 1)
#define B_HSYNC_OUT_POL            (1 << 0)

#define REG_RX_VIDEO_MAP           0x1B
#define B_OUTPUT_16BIT             (1 << 5)
#define B_SWAP_CH422               (1 << 4)
#define B_SWAP_OUTRB               (1 << 3)
#define B_SWAP_ML                  (1 << 2)
#define B_SWAP_POL                 (1 << 1)
#define B_SWAP_RB                  (1 << 0)

#define REG_RX_VIDEO_CTRL1         0x1C
#define B_RX_DNFREE_GO             (1 << 7)
#define B_SYNC_EMBEDDED            (1 << 6)
#define B_RX_EN_DITHER             (1 << 5)
#define B_RX_EN_UDFILTER           (1 << 4)
#define B_DDR_OUTPUT               (1 << 3)
#define B_CCIR565                  (1 << 2)
#define B_565FFRST                 (1 << 1)
#define B_EN_AVMUTE_RST            (1 << 0)

#define REG_RX_VCLK_CTRL           0x1D
#define REG_RX_I2CIO_CTRL          0x1E

//#ifdef _IT6607_GeNPacket_Usage_
#define REG_RX_REGPKTFLAG_CTRL     0x1F
#define B_RX_INT_EVERY_ACP         (1 << 5)
#define B_RX_INT_EVERY_GenPkt      (1 << 4)
#define B_INT_EVERYAVI             1
//#endif

#define REG_RX_CSC_CTRL            0x20
#define B_VDIO_GATTING             (1 << 7)
#define B_VDIO_DISABLE             (1 << 6)
#define B_VIO_SEL                  (1 << 5)

#define B_FORCE_COLOR_MODE         (1 << 4)
#define O_INPUT_COLOR_MODE         2
#define M_INPUT_COLOR_MASK         3
#define B_INPUT_RGB24              0
#define B_INPUT_YUV422             1
#define B_INPUT_YUV444             2

#define O_CSC_SEL                  0
#define M_CSC_SEL_MASK             3
#define B_CSC_BYPASS               0
#define B_CSC_RGB2YUV              2
#define B_CSC_YUV2RGB              3

#define REG_RX_CSC_YOFF            0x21
#define REG_RX_CSC_COFF            0x22
#define REG_RX_CSC_RGBOFF          0x23
#define REG_RX_CSC_MTX11_L         0x24
#define REG_RX_CSC_MTX11_H         0x25
#define REG_RX_CSC_MTX12_L         0x26
#define REG_RX_CSC_MTX12_H         0x27
#define REG_RX_CSC_MTX13_L         0x28
#define REG_RX_CSC_MTX13_H         0x29
#define REG_RX_CSC_MTX21_L         0x2A
#define REG_RX_CSC_MTX21_H         0x2B
#define REG_RX_CSC_MTX22_L         0x2C
#define REG_RX_CSC_MTX22_H         0x2D
#define REG_RX_CSC_MTX23_L         0x2E
#define REG_RX_CSC_MTX23_H         0x2F
#define REG_RX_CSC_MTX31_L         0x30
#define REG_RX_CSC_MTX31_H         0x31
#define REG_RX_CSC_MTX32_L         0x32
#define REG_RX_CSC_MTX32_H         0x33
#define REG_RX_CSC_MTX33_L         0x34
#define REG_RX_CSC_MTX33_H         0x35
#define REG_RX_CSC_GAIN1V_L        0x36
#define REG_RX_CSC_GAIN1V_H        0x37
#define REG_RX_CSC_GAIN2V_L        0x38
#define REG_RX_CSC_GAIN2V_H        0x39
#define REG_RX_CSC_GAIN3V_L        0x3A
#define REG_RX_CSC_GAIN3V_H        0x3B
#define REG_RX_PG_CTRL1            0x3C
#define REG_RX_PG_CTRL2            0x3D

#define O_OUTPUT_COLOR_MODE        6
#define M_OUTPUT_COLOR_MASK        3
#define B_OUTPUT_RGB24             0
#define B_OUTPUT_YUV422            1
#define B_OUTPUT_YUV444            2

#define REG_RX_PG_COLR             0x3E
#define REG_RX_PG_COLG             0x3F
#define REG_RX_PG_COLB             0x40
#define REG_RX_PG_COLBLANK         0x41
#define REG_RX_PG_COLBLANKY        0x42
#define REG_RX_PG_HACTST_L         0x43
#define REG_RX_PG_HACTST_H         0x44
#define REG_RX_PG_HACTED_H         0x45
#define REG_RX_PG_VACTST_L         0x46
#define REG_RX_PG_VACTST_H         0x47
#define REG_RX_PG_VACTED_H         0x48
#define REG_RX_PG_VACTST2ND_L      0x49
#define REG_RX_PG_VACTST2ND_H      0x4A
#define REG_RX_PG_VACTED2ND_H      0x4B
#define REG_RX_PG_HTOTAL_L         0x4C
#define REG_RX_PG_HTOTAL_H         0x4D
#define REG_RX_PG_HSYNCST_H        0x4E
#define REG_RX_PG_HSYNCED_L        0x4F
#define REG_RX_PG_VTOTAL_L         0x50
#define REG_RX_PG_VTOTAL_H         0x51
#define REG_RX_PG_VSYNCST_H        0x52
#define REG_RX_PG_VSYNCED          0x53
#define REG_RX_PG_CHINC            0x54
#define REG_RX_PG_CVINC            0x55

#define REG_RX_I2C_CTRL            0x57
#define B_FILT_TAP                 (1 << 5)
#define B_FILT_TYPE                (1 << 4)
#define B_DEGLITCH                 (1 << 3)
#define B_CMD_FILTTAP              (1 << 2)
#define B_CMD_FILTTYPE             (1 << 1)
#define B_CMD_DEGLITCH             (1 << 0)

#define REG_RX_VID_MODE            0x58
#define B_PX_VIDEOSTABLE           (1 << 3)
#define B_VIDFIELD                 (1 << 2)
#define B_INTERLACE                (1 << 1)

#define REG_RX_VID_HTOTAL_L        0x59
#define REG_RX_VID_HTOTAL_H        0x5A

#define O_HTOTAL_H                 0
#define M_HTOTAL_H                 0xF
#define O_HACT_H                   4
#define M_HACT_H                   0x7

#define REG_RX_VID_HACT_L          0x5B

#define REG_RX_VID_HSYNC_WID_L     0x5C
#define REG_RX_VID_HSYNC_WID_H     0x5D

#define O_HSYNC_WID_H              0
#define M_HSYNC_WID_H              1
#define O_H_FT_PORCH               4
#define M_H_FT_PORCH               3

#define REG_RX_VID_H_FT_PORCH_L    0x5E
#define REG_RX_VID_VTOTAL_L        0x5F
#define REG_RX_VID_VTOTAL_H        0x60
#define O_VTOTAL_H                 0
#define M_VTOTAL_H                 0xF
#define O_VACT_H                   4
#define M_VACT_H                   0x7

#define REG_RX_VID_VACT_L          0x61
#define REG_RX_VID_VSYNC2DE        0x62
#define REG_RX_VID_V_FT_PORCH      0x63
#define REG_RX_VID_XTALCNT_128PEL  0x64
#define REG_RX_VID_INPUT_ST        0x65

#define O_PIXEL_REPEAT             4
#define M_PIXEL_REPEAT             0xF
#define B_RX_DIS_HDCP              (1 << 3)
#define B_AVMUTE                   (1 << 2)
#define B_VSYNC_IN_POL             (1 << 1)
#define B_HSYNC_IN_POL             (1 << 0)

#define REG_RX_DEBUG_CTRL          0x67
#define REG_RX_PLL_CTRL            0x68
#define REG_RX_TERM_CTRL1          0x69
#define REG_RX_TERM_CTRL2          0x6A

#define REG_RX_EQUAL_CTRL2         0x6C
#define REG_RX_DES_CTRL1           0x6D
#define REG_RX_DES_CTRL2           0x6E
#define REG_RX_DES_PHASE_B         0x6F
#define REG_RX_DES_PHASE_G         0x70
#define REG_RX_DES_PHASE_R         0x71
#define REG_RX_BCH_MODE            0x72
#define O_BCH_THRSH                3
#define M_BCH_THRSH                ((1 << (6 - 3)) - 1)

#define B_TERC_INV                 (1 << 2)
#define B_TERC_CHECK               (1 << 1)
#define B_DIS_BCH                  (1 << 0)

#define REG_RX_CDEPTH_CTRL         0x73
#define B_EN_DefPhase              (1 << 0)
#define B_DIS_DEEP_MODE            (1 << 1)
#define B_SEL_CD_DEBUG             (1 << 2)
#define B_RSTCD                    (1 << 3)
#define B_ENABLE_FAST              (1 << 4)
#define B_ENABLE_FEATURE_1P1       (1 << 5)
#define B_KSV_READY                (1 << 6)
#define B_ENABLE_REPEATER          (1 << 7)

#define REG_RX_APLL_CTRL           0x74
#define B_APLL_ENI2A               (1 << 7)
#define B_APLL_EC1                 (1 << 6)
#define B_APLL_ENIA                (1 << 5)
#define B_RSVED                    (1 << 4)
#define B_DIEN                     (1 << 3)
#define B_DIS_SDM                  (1 << 2)
#define B_APLL_REFSW               (1 << 1)
#define B_APLL_AUTOSW              (1 << 0)

#define REG_RX_I2S_CTRL            0x75

#define REG_RX_I2S_MAP             0x76
#define O_I2S_CH3SEL               6
#define O_I2S_CH2SEL               4
#define O_I2S_CH1SEL               2
#define O_I2S_CH0SEL               0

#define M_I2S_CHSEL                3
#define B_I2S_CHSEL0               0
#define B_I2S_CHSEL1               1
#define B_I2S_CHSEL2               2
#define B_I2S_CHSEL3               3

#define REG_RX_AUDIO_CTRL          0x77

#define B_AUDCK_BBEN               (1 << 7)
#define B_FORCE_FS                 (1 << 6)

#define B_DIS_FALSE_DE             (1 << 5)
#define B_AUD_INFO_FORCE           (1 << 4)
#define B_AVMUTE_VALUE             (1 << 3)
#define B_FORCE_AVMUTE             (1 << 2)
#define B_DIS_VDO_MUTE             (1 << 1)
#define B_DIS_AUD_MUTE             (1 << 0)

#define REG_RX_MCLK_CTRL           0x78
#define B_OSC_EN                   (1 << 7)
#define B_OSCSEL                   (1 << 6)

#define B_CTSINI_EN                (1 << 5)

#define B_FORCE_CTS                (1 << 4)
#define B_FORCE_CTSMODE            (1 << 3)
#define O_MCLKSEL                  0
#define M_MCLKSEL                  0x07
#define B_128FS                    0
#define B_256FS                    1
#define B_512FS                    3
#define B_1024FS                   7

#define REG_RX_N_SET1              0x79
#define REG_RX_N_SET2              0x7A
#define REG_RX_N_SET3              0x7B
#define REG_RX_CTS_SET2            0x7C
#define REG_RX_CTS_SET3            0x7D
#define REG_RX_FS_SET              0x7E
#define REG_RX_N_RCV1              0x7F
#define REG_RX_N_RCV2              0x80
#define REG_RX_N_RCV3              0x81
#define REG_RX_CTS_RCV2            0x82
#define REG_RX_CTS_RCV3            0x83
#define REG_RX_FS                  0x84
#define M_GCP_CD                   0xF0
#define O_GCP_CD                   4

#define M_Fs                       0xF
#define B_Fs_44p1KHz               0
#define B_Fs_48KHz                 2
#define B_Fs_32KHz                 3
#define B_Fs_88p2KHz               8
#define B_Fs_96KHz                 0xA
#define B_Fs_176p4KHz              0xC
#define B_Fs_192KHz                0xE
#define B_Fs_768KHz                0x9 // 1001
#define B_Fs_HBR                   0x9 // 1001

#define REG_RX_CHANNEL_ERR         0x85
#define B_PKT_2BERR                (1 << 7)
#define B_PKT_1BERR                (1 << 6)
#define O_CH2_ERR                  4
#define O_CH1_ERR                  2
#define O_CH0_ERR                  0
#define REG_RX_HWMUTE_RATE         0x86

#define REG_RX_HWMUTE_CTRL         0x87
#define B_HW_FORCE_MUTE            (1 << 6)
#define B_HW_AUDMUTE_CLR_MODE      (1 << 5)
#define B_HW_MUTE_CLR              (1 << 4)
#define B_HW_MUTE_EN               (1 << 3)

#define M_HWMUTE_RATE_MASK         7

#define REG_RX_HWAMP_CTRL          0x88
#define M_HWAMP_CTRL_MASK          3

#define REG_RX_TRISTATE_CTRL       0x89
#define B_VDO_MUTE_DISABLE         (1 << 7)
#define O_TRI_VIDEOIO              (6)
#define O_TRI_VIDEO                (5)
#define O_TRI_SPDIF                (4)
#define O_TRI_I2S3                 (3)
#define O_TRI_I2S2                 (2)
#define O_TRI_I2S1                 (1)
#define O_TRI_I2S0                 (0)

#define B_Audio_FIFO_ERROR_TRIGGER (1 << 0)
//#define B_Audio_FIFO_ERROR_TRIGGER (1<<6)
#define B_TRI_VIDEO                (1 << 5)
#define B_TRI_SPDIF                (1 << 4)
#define B_TRI_I2S3                 (1 << 3)
#define B_TRI_I2S2                 (1 << 2)
#define B_TRI_I2S1                 (1 << 1)
#define B_TRI_I2S0                 (1 << 0)

//#define REG_RX_TRISTATE_CTRL  0x89
#define B_VDO_MUTE_DISABLE         (1 << 7)
#define B_TRI_VIDEO_SYNC           (1 << 5)

#define REG_BK1_TRISTATE_CTRL      0x7E
#define B_TRI_VIDEOIO              (1 << 4)
#define B_HALF_PCLK                (1 << 2)

#define B_TRI_AUDIO                (B_TRI_SPDIF | B_TRI_I2S3 | B_TRI_I2S2 | B_TRI_I2S1 | B_TRI_I2S0)

#define REG_RX_AUDIO_CH_STAT       0x8A
#define B_AUDIO_ON                 (1 << 7)
#define B_HBRAUDIO                 (1 << 6)
#define B_DSDAUDIO                 (1 << 5)
#define B_AUDIO_LAYOUT             (1 << 4)
#define M_AUDIO_CH                 0xF
#define B_AUDIO_SRC_VALID_3        (1 << 3)
#define B_AUDIO_SRC_VALID_2        (1 << 2)
#define B_AUDIO_SRC_VALID_1        (1 << 1)
#define B_AUDIO_SRC_VALID_0        (1 << 0)

#define F_AUDIO_ON                 (1 << 7)
#define F_AUDIO_HBR                (1 << 6)
#define F_AUDIO_DSD                (1 << 5)
#define F_AUDIO_NLPCM              (1 << 4)
#define F_AUDIO_LAYOUT_1           (1 << 3)
#define F_AUDIO_LAYOUT_0           (0 << 3)

#define T_AUDIO_MASK               0xF0
#define T_AUDIO_OFF                0
#define T_AUDIO_HBR                (F_AUDIO_ON | F_AUDIO_HBR)
#define T_AUDIO_DSD                (F_AUDIO_ON | F_AUDIO_DSD)
#define T_AUDIO_NLPCM              (F_AUDIO_ON | F_AUDIO_NLPCM)
#define T_AUDIO_LPCM               (F_AUDIO_ON)

#define REG_RX_RDROM_OFFSET        0x8F
#define REG_RX_RDROM_REQBYTE       0x90
#define REG_RX_RDROM_CTRL          0x91

#define REG_RX_RDROM_STATUS        0x92
#define B_ROMIDLE                  1
#define REG_RX_RDROM_CLKCTRL       0x9B
#define B_ROM_CLK_SEL_REG          (1 << 7)
#define B_ROM_CLK_VALUE            (1 << 6)

#define B_CLKRST_SEL               (1 << 5)
#define B_HDCP_NOAVMUTE            (1 << 4)
#define O_ANALOG_BACKUP            2
#define O_DIGITAL_BACKUP           0

#define REG_RX_AUD_CHSTAT0         0x9C
#define B_AUD_NLPCM                (1 << 1)
#define B_NLPCM                    (1 << 1)
#define B_SW_COPYRIGHT_ASSERT      (1 << 2)

#define REG_RX_AUD_CHSTAT1         0x9D

#define REG_RX_AUD_CHSTAT2         0x9E
#define M_CH_NUM                   0xF0
#define O_CH_NUM                   4
#define M_SRC_NUM                  0x0F
#define O_SRC_NUM                  0

#define REG_RX_AUD_CHSTAT3         0x9F
#define M_CLK_ACCURANCE            0x03
#define M_SW_LEN                   0xF0
#define O_SW_LEN                   4

//#ifdef _IT6607_GeNPacket_Usage_
//2010/01/27 3D PACKET
#define REG_RX_NEWPKT_SEL          0xA6
#define B_GENPKT_HL                (1 << 5)
#define B_ISRC2                    (1 << 3)
#define B_ISRC1                    (1 << 2)
#define B_ACP                      (1 << 1)
#define B_GENERAL                  (1 << 0)     // 0 for record idenetical packet, 1 for record un-support packet

#define REG_RX_PKT_REC_TYPE_HEAD   0xA7
#define REG_RX_PKT_REC_TYPE        0xA8

#define REG_RX_PACKET_DET          0x8B
#define B_GenPKT_DET               (1 << 2)
//    #define B_ISRC2_DET           (1<<1)
//    #define B_ISRC1_DET           (1<<0)

//end of 3D packet
//#endif

#define SPD_INFOFRAME              0
#define ISRC1_PKT                  1
#define ISRC2_PKT                  2
#define DSD_PKT                    3
#define AVI_INFOFRAME              4
#define AUDIO_INFOFRAME            5
#define MPEG_INFOFRAME             6
#define VENDORSPEC_INFOFRAME       7
#define ACP_PKT                    8
#define UNRECOGNIZED_PKT           9
#define AUDIO_SAMPLE_PKT           10
#define GENERAL_CONTROL_PKT        11

#define REG_RX_AVI_VER             0xAC
#define REG_RX_AVI_DB0             0xAD

#define O_AVI_COLOR_MODE           5
#define M_AVI_COLOR_MASK           3
#define B_AVI_COLOR_RGB24          0
#define B_AVI_COLOR_YUV422         1
#define B_AVI_COLOR_YUV444         2

#define B_AVI_PRESENT              (1 << 4)

#define O_AVI_BAR_INFO             2
#define M_AVI_BAR_INFO_MASK        3
#define B_AVI_BAR_NOT_VALID        0
#define B_AVI_BAR_VINFO_VALID      1
#define B_AVI_BAR_HINFO_VALID      2
#define B_AVI_BAR_VHINFO_VALID     3

#define O_AVI_SCAN_INFO            0
#define M_AVI_SCAN_INFO_MASK       3
#define M_AVI_SCAN_NODATA          0
#define M_AVI_SCAN_OVER_SCAN       1
#define M_AVI_SCAN_UNDER_SCAN      2

#define REG_RX_AVI_DB1             0xAE
// D[7:6] C1C0 Colorimetry
#define O_AVI_CLRMET               6
#define M_AVI_CLRMET_MASK          3
#define B_AVI_CLRMET_NODATA        0
#define B_AVI_CLRMET_ITU601        1
#define B_AVI_CLRMET_ITU709        2
#define B_AVI_CLRMET_Extend        3 // _SupportXVYcc_ --> indicated in bits EC0~EC2
// D[5:4] M1M0 Picture Aspect Ratio
#define O_AVI_PIC_ASRATIO          4
#define M_AVI_PIC_ASRATIO_MASK     3
#define B_AVI_PIC_ASRATIO_NODATA   0
#define B_AVI_PIC_ASRATIO_4_3      1
#define B_AVI_PIC_ASRATIO_16_9     2
// D[3:0] R3R2R1R0 Active Format Aspect Ratio
#define O_AVI_FMT_ASRATIO          0
#define M_AVI_FMT_ASRATIO_MASK     0xF
#define M_AVI_FMT_ASRATIO_EQPIC    0x8
#define M_AVI_FMT_ASRATIO_4_3      0x9
#define M_AVI_FMT_ASRATIO_16_9     0xA
#define M_AVI_FMT_ASRATIO_14_9     0xB
#define REG_RX_AVI_DB2             0xAF

#define O_AVI_NUNI_SCALE           0
#define M_AVI_NUNI_SCALE_MASK      3
#define B_AVI_NUNI_SCALE_NODATA    0
#define B_AVI_NUNI_SCALE_HORZ      1
#define B_AVI_NUNI_SCALE_VERT      2
#define B_AVI_NUNI_SCALE_HORZVERT  3

#define REG_RX_AVI_DB3             0xB0

#define O_AVI_VIDCODE              0
#define O_AVI_VIDCODE_MASK         0x7f
#define REG_RX_AVI_DB4             0xB1

#define O_AVI_PIXREPT              0
#define M_AVI_PIXREPT_MASK         0xF

#define REG_RX_AVI_DB5             0xB2
#define REG_RX_AVI_DB6             0xB3
#define REG_RX_AVI_DB7             0xB4
#define REG_RX_AVI_DB8             0xB5
#define REG_RX_AVI_DB9             0xB6
#define REG_RX_AVI_DB10            0xB7
#define REG_RX_AVI_DB11            0xB8
#define REG_RX_AVI_DB12            0xB9
#define REG_RX_AVI_DB13            0xBA
#define REG_RX_AVI_DB14            0xBB
#define REG_RX_AVI_DB15            0xBC
#define REG_RX_GENPKT_HB0          0xBD
#define REG_RX_GENPKT_HB1          0xBE
#define REG_RX_GENPKT_HB2          0xBF
#define REG_RX_GENPKT_DB0          0xC0
#define REG_RX_GENPKT_DB1          0xC1
#define REG_RX_GENPKT_DB2          0xC2
#define REG_RX_GENPKT_DB3          0xC3
#define REG_RX_GENPKT_DB4          0xC4
#define REG_RX_GENPKT_DB5          0xC5
#define REG_RX_GENPKT_DB6          0xC6
#define REG_RX_GENPKT_DB7          0xC7
#define REG_RX_GENPKT_DB8          0xC8
#define REG_RX_GENPKT_DB9          0xC9
#define REG_RX_GENPKT_DB10         0xCA
#define REG_RX_GENPKT_DB11         0xCB
#define REG_RX_GENPKT_DB12         0xCC
#define REG_RX_GENPKT_DB13         0xCD
#define REG_RX_GENPKT_DB14         0xCE
#define REG_RX_GENPKT_DB15         0xCF
#define REG_RX_GENPKT_DB16         0xD0
#define REG_RX_GENPKT_DB17         0xD1
#define REG_RX_GENPKT_DB18         0xD2
#define REG_RX_GENPKT_DB19         0xD3
#define REG_RX_GENPKT_DB20         0xD4
#define REG_RX_GENPKT_DB21         0xD5
#define REG_RX_GENPKT_DB22         0xD6
#define REG_RX_GENPKT_DB23         0xD7
#define REG_RX_GENPKT_DB24         0xD8
#define REG_RX_GENPKT_DB25         0xD9
#define REG_RX_GENPKT_DB26         0xDA
#define REG_RX_GENPKT_DB27         0xDB
#define REG_RX_AUDIO_VER           0xDC
#define REG_RX_AUDIO_DB0           0xDD

#define O_AUD_CODTYPE              4
#define M_AUD_CODTYPE_MASK         0xF
#define B_AUD_CODTYPE_STREAMDEF    0
#define B_AUD_CODTYPE_IEC60958     1
#define B_AUD_CODTYPE_AC3          2
#define B_AUD_CODTYPE_MPEG1        3
#define B_AUD_CODTYPE_MP3          4
#define B_AUD_CODTYPE_MPEG2        5
#define B_AUD_CODTYPE_AAC          6
#define B_AUD_CODTYPE_DTS          7
#define B_AUD_CODTYPE_ATRAC        8

#define O_AUD_CHCNT                0
#define M_AUD_CHCNT_MASK           0x7
#define B_AUD_CHCNT_STREAMDEF      0
#define B_AUD_CHCNT_CH2            1
#define B_AUD_CHCNT_CH3            2
#define B_AUD_CHCNT_CH4            3
#define B_AUD_CHCNT_CH5            4
#define B_AUD_CHCNT_CH6            5
#define B_AUD_CHCNT_CH7            6
#define B_AUD_CHCNT_CH8            7
#define REG_RX_AUDIO_DB1           0xDE

#define O_AUD_SMP_FREQ             2
#define M_AUD_SMP_FREQ_MASK        7
#define B_AUD_SMP_FREQ_STREAMDEF   0
#define B_AUD_SMP_32K              3
#define B_AUD_SMP_44K              0
#define B_AUD_SMP_48K              2
#define B_AUD_SMP_88K              8
#define B_AUD_SMP_96K              10
#define B_AUD_SMP_176K             12
#define B_AUD_SMP_192K             14

#define O_AUD_SMP_SIZE             0
#define O_AUD_SMP_SIZE_MASK        0x3
#define O_AUD_SMP_SIZE_STREAMDEF   0
#define O_AUD_SMP_16BIT            1
#define O_AUD_SMP_20BIT            2
#define O_AUD_SMP_24BIT            3
#define REG_RX_AUDIO_DB2           0xDF

#define REG_RX_AUDIO_DB3           0xE0

#define REG_RX_AUDIO_DB4           0xE1

#define B_PROHIBITED               (1 << 7)

#define O_AUD_LEVEL_SHIFT_VALUE    3
#define O_AUD_LEVEL_SHIFT_MASK     0xF
#define REG_RX_AUDIO_DB5           0xE2
#define REG_RX_AUDIO_LEN           0xE3
#define REG_RX_MPEG_VER            0xE4
#define REG_RX_MPEG_LEN            0xE5
#define REG_RX_MPEG_DB0            0xE6
#define REG_RX_MPEG_DB1            0xE7
#define REG_RX_MPEG_DB2            0xE8
#define REG_RX_MPEG_DB3            0xE9
#define REG_RX_MPEG_DB4            0xEA
#define REG_RX_MPEG_DB5            0xEB
/*********************************marked by hermes  20080226
   #define REG_RX_VS_VER    0xEC

   #define REG_RX_VS_DB0    0xEB
   #define REG_RX_VS_DB1    0xEC
   #define REG_RX_VS_DB2    0xED
   #define REG_RX_VS_DB3    0xEE
   #define REG_RX_VS_DB4    0xEF
   #define REG_RX_VS_DB5    0xF0
   #define REG_RX_VS_DB6    0xF1
   #define REG_RX_VS_DB7    0xF2
   #define REG_RX_VS_DB8    0xF3
 *******************************************************/

#define REG_RX_ACP_HB0             0xEC
#define REG_RX_ACP_HB1             0xED
#define REG_RX_ACP_HB2             0xEE
#define REG_RX_ACP_DB0             0xEF
#define REG_RX_ACP_DB1             0xF0
#define REG_RX_ACP_DB2             0xF1
#define REG_RX_ACP_DB3             0xF2
#define REG_RX_ACP_DB4             0xF3
#define REG_RX_ACP_DB5             0xF4
#define REG_RX_ACP_DB6             0xF5
#define REG_RX_ACP_DB7             0xF6
#define REG_RX_ACP_DB8             0xF7
#define REG_RX_ACP_DB9             0xF8
#define REG_RX_ACP_DB10            0xF9
#define REG_RX_ACP_DB11            0xFA
#define REG_RX_ACP_DB12            0xFB
#define REG_RX_ACP_DB13            0xFC
#define REG_RX_ACP_DB14            0xFD
#define REG_RX_ACP_DB15            0xFE
#define REG_RX_ACP_TYPE            0xFF
#define REG_RX_ACP_REC_TYPE        0xFF

#define REG_RX_ISRC1_HB0           0xA0
#define REG_RX_ISRC1_HB1           0xA1
#define REG_RX_ISRC1_HB2           0xA2
#define REG_RX_ISRC1_DB0           0xA3
#define REG_RX_ISRC1_DB1           0xA4
#define REG_RX_ISRC1_DB2           0xA5
#define REG_RX_ISRC1_DB3           0xA6
#define REG_RX_ISRC1_DB4           0xA7
#define REG_RX_ISRC1_DB5           0xA8
#define REG_RX_ISRC1_DB6           0xA9
#define REG_RX_ISRC1_DB7           0xAA
#define REG_RX_ISRC1_DB8           0xAB
#define REG_RX_ISRC1_DB9           0xAC
#define REG_RX_ISRC1_DB10          0xAD
#define REG_RX_ISRC1_DB11          0xAE
#define REG_RX_ISRC1_DB12          0xAF
#define REG_RX_ISRC1_DB13          0xB0
#define REG_RX_ISRC1_DB14          0xB1
#define REG_RX_ISRC1_DB15          0xB2
#define REG_RX_ISRC1_REC_TYPE      0xB3
#define REG_RX_ISRC2_HB0           0xB4
#define REG_RX_ISRC2_HB1           0xB5
#define REG_RX_ISRC2_HB2           0xB6
#define REG_RX_ISRC2_DB0           0xB7
#define REG_RX_ISRC2_DB1           0xB8
#define REG_RX_ISRC2_DB2           0xB9
#define REG_RX_ISRC2_DB3           0xBA
#define REG_RX_ISRC2_DB4           0xBB
#define REG_RX_ISRC2_DB5           0xBC
#define REG_RX_ISRC2_DB6           0xBD
#define REG_RX_ISRC2_DB7           0xBE
#define REG_RX_ISRC2_DB8           0xBF
#define REG_RX_ISRC2_REC_TYPE      0xC0
#define REG_RX_KSV_FIFO00          0xC1
#define REG_RX_KSV_FIFO01          0xC2
#define REG_RX_KSV_FIFO02          0xC3
#define REG_RX_KSV_FIFO03          0xC4
#define REG_RX_KSV_FIFO04          0xC5
#define REG_RX_KSV_FIFO10          0xC6
#define REG_RX_KSV_FIFO11          0xC7
#define REG_RX_KSV_FIFO12          0xC8
#define REG_RX_KSV_FIFO13          0xC9
#define REG_RX_KSV_FIFO14          0xCA
#define REG_RX_KSV_FIFO20          0xCB
#define REG_RX_KSV_FIFO21          0xCC
#define REG_RX_KSV_FIFO22          0xCD
#define REG_RX_KSV_FIFO23          0xCE
#define REG_RX_KSV_FIFO24          0xCF
#define REG_RX_KSV_FIFO30          0xD0
#define REG_RX_KSV_FIFO31          0xD1
#define REG_RX_KSV_FIFO32          0xD2
#define REG_RX_KSV_FIFO33          0xD3
#define REG_RX_KSV_FIFO34          0xD4
#define REG_RX_KSV_FIFO40          0x80
#define REG_RX_KSV_FIFO41          0x81
#define REG_RX_KSV_FIFO42          0x82
#define REG_RX_KSV_FIFO43          0x83
#define REG_RX_KSV_FIFO44          0x84
#define REG_RX_KSV_FIFO50          0x85
#define REG_RX_KSV_FIFO51          0x86
#define REG_RX_KSV_FIFO52          0x87
#define REG_RX_KSV_FIFO53          0x88
#define REG_RX_KSV_FIFO54          0x89
#define REG_RX_KSV_FIFO60          0x8A
#define REG_RX_KSV_FIFO61          0x8B
#define REG_RX_KSV_FIFO62          0x8C
#define REG_RX_KSV_FIFO63          0x8D
#define REG_RX_KSV_FIFO64          0x8E
#define REG_RX_KSV_FIFO70          0x8F
#define REG_RX_KSV_FIFO71          0x90
#define REG_RX_KSV_FIFO72          0x91
#define REG_RX_KSV_FIFO73          0x92
#define REG_RX_KSV_FIFO74          0x93

#define REG_RX_BSTATUSL            0xD5
#define REG_RX_BSTATUSH            0xD6

#define REG_RX_SHA1_H00            0xD7
#define REG_RX_SHA1_H01            0xD8
#define REG_RX_SHA1_H02            0xD9
#define REG_RX_SHA1_H03            0xDA
#define REG_RX_SHA1_H10            0xDB
#define REG_RX_SHA1_H11            0xDC
#define REG_RX_SHA1_H12            0xDD
#define REG_RX_SHA1_H13            0xDE
#define REG_RX_SHA1_H20            0xDF
#define REG_RX_SHA1_H21            0xE0
#define REG_RX_SHA1_H22            0xE1
#define REG_RX_SHA1_H23            0xE2
#define REG_RX_SHA1_H30            0xE3
#define REG_RX_SHA1_H31            0xE4
#define REG_RX_SHA1_H32            0xE5
#define REG_RX_SHA1_H33            0xE6
#define REG_RX_SHA1_H40            0xE7
#define REG_RX_SHA1_H41            0xE8
#define REG_RX_SHA1_H42            0xE9
#define REG_RX_SHA1_H43            0xEA
#define REG_RX_M0_B0               0xEB
#define REG_RX_M0_B1               0xEC
#define REG_RX_M0_B2               0xED
#define REG_RX_M0_B3               0xEE
#define REG_RX_M0_B4               0xEF
#define REG_RX_M0_B5               0xF0
#define REG_RX_M0_B6               0xF1
#define REG_RX_M0_B7               0xF2
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2012/03/27 added by jau-chih.tseng@ite.com.tw
#define EVENT_5V_OFF               (1 << 0)
#define EVENT_5V_ON                (1 << 1)
#define EVENT_SCDT_OFF             (1 << 2)
#define EVENT_SCDT_ON              (1 << 3)
#define EVENT_VIDEO_ON             (1 << 4)
#define EVENT_AUTH_START           (1 << 5)
#define EVENT_AUTH_DONE            (1 << 6)
#define EVENT_AUDIO_DONE           (1 << 7)
#define EVENT_NEW_AVI              (1 << 8)
#define EVENT_NEW_AUD              (1 << 9)
#define EVENT_NEW_VSI              (1 << 10)
#define EVENT_NO_AVI               (1 << 11)
#define EVENT_NEW_ACP              (1 << 12)
#define EVENT_NEW_GAMUT            (1 << 13)
#define EVENT_SET_AVMUTE           (1 << 14)
#define EVENT_CLR_AVMUTE           (1 << 15)
//~jau-chih.tseng@ite.com.tw 2012/03/27

#define Vendor_Specific_Video_Data 0x81
#define Source_Product_Description 0x83
//#define Gamut_Meta_Date_Packet        0x8A

#ifndef                        MS_TimeOut
    #define LOOP_MS                20
    #define                        MS_TimeOut(x) (x)
#else
    #pragma message ("MS_TimeOut predefined in other place.")
#endif //                        MS_TimeOut

/*
   #define VSTATE_MISS_SYNC_COUNT                        MS_TimeOut(2000)

   #define VSATE_CONFIRM_SCDT_COUNT                        MS_TimeOut(0)
   #define AUDIO_READY_TIMEOUT                        MS_TimeOut(10)
   #define AUDIO_STABLE_TIMEOUT                        MS_TimeOut(100)

   #define MUTE_RESUMING_TIMEOUT                        MS_TimeOut(2500)
   #define HDCP_WAITING_TIMEOUT                        MS_TimeOut(3000)

   #define FORCE_SWRESET_TIMEOUT                        MS_TimeOut(32766)
 */

#define VSTATE_MISS_SYNC_COUNT       MS_TimeOut(12000)

#define SWReset_COUNT                MS_TimeOut(500)
#define AVmute_COUNT                 MS_TimeOut(5000)
#define VSTATE_SWRESET_TIMEOUT_COUNT MS_TimeOut(500)

#define VSATE_CONFIRM_SCDT_COUNT     MS_TimeOut(150)
#define AUDIO_READY_TIMEOUT          MS_TimeOut(20)

#define AUDIO_STABLE_TIMEOUT         MS_TimeOut(150)
#define AUDIO_MONITOR_TIMEOUT        MS_TimeOut(150)

#define MUTE_RESUMING_TIMEOUT        MS_TimeOut(2500)
#define HDCP_WAITING_TIMEOUT         MS_TimeOut(3000)

#define FORCE_SWRESET_TIMEOUT        MS_TimeOut(15000)

#define TX_UNPLUG_TIMEOUT            MS_TimeOut(300)
#define TX_WAITVIDSTBLE_TIMEOUT      MS_TimeOut(100)

#define TX_HDCP_TIMEOUT              MS_TimeOut(6000)
/* Zinwell */
#define DETECT_TX_PLUG_TIMEOUT       MS_TimeOut(100)

#define CAT_HDMI_PORTA               0
#define CAT_HDMI_PORTB               1
#define SIZEOF_CSCMTX                21
#define SIZEOF_CSCGAIN               6
#define SIZEOF_CSCOFFSET             3

#ifdef _RXPOLLING_
unsigned int currHTotal;
BYTE         currXcnt;
BOOL         currScanMode;
#endif

typedef enum _Video_State_Type {
    VSTATE_Off = 0,
    VSTATE_PwrOff,
    VSTATE_SyncWait,
    VSTATE_SWReset,
    VSTATE_SyncChecking,
    VSTATE_HDCPSet,
    VSTATE_HDCP_Reset,
    VSTATE_ModeDetecting,
    VSTATE_VideoOn,
    VSTATE_ColorDetectReset,
    VSTATE_HDMI_OFF,
    VSTATE_Reserved
} Video_State_Type;

typedef enum _Audio_State_Type {
    ASTATE_AudioOff = 0,
    ASTATE_RequestAudio,
    ASTATE_ResetAudio,
    ASTATE_WaitForReady,
    ASTATE_AudioOn,
    ASTATE_Reserved
} Audio_State_Type;

typedef enum _RxHDCP_State_Type {
    RxHDCP_PwrOff = 0,
    RxHDCP_ModeCheck,
    RxHDCP_Receiver,
    RxHDCP_Repeater,
    RxHDCP_SetKSVFifoList,
    RxHDCP_GenVR,
    RxHDCP_WriteVR,
    RxHDCP_Auth_WaitRi,
    RxHDCP_Authenticated,
    RxHDCP_Reserved,
} RxHDCP_State_Type;

typedef struct _RX_REG_AUDIO_CHSTS {
    BYTE rev : 1;
    BYTE ISLPCM : 1;
    BYTE CopyRight : 1;
    BYTE AdditionFormatInfo : 3;
    BYTE ChannelStatusMode : 2;

    BYTE CategoryCode;

    BYTE SourceNumber : 4;
    BYTE ChannelNumber : 4;

    BYTE SamplingFreq : 4;
    BYTE ClockAccuary : 2;
    BYTE rev2 : 2;
    BYTE WorldLen : 4;
    BYTE OriginalSamplingFreq : 4;
} RX_REG_AUDIO_CHSTS;

#define B_CAP_AUDIO_ON  (1 << 7)
#define B_CAP_HBR_AUDIO (1 << 6)
#define B_CAP_DSD_AUDIO (1 << 5)
#define B_LAYOUT        (1 << 4)
#define B_MULTICH       (1 << 4)
#define B_HBR_BY_SPDIF  (1 << 3)
#define B_SPDIF         (1 << 2)
#define B_CAP_LPCM      (1 << 0)

typedef struct {
    BYTE AudioFlag;
    BYTE AudSrcEnable;
    BYTE SampleFreq;
} AUDIO_CAPS;

////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CAPS_MIN_PROGRESS_VFREQ  2300
#define CAPS_MAX_PROGRESS_VFREQ  10000
#define CAPS_MIN_PROGRESS_HFREQ  300
#define CAPS_MAX_PROGRESS_HFREQ  1000
#define CAPS_MIN_INTERLACE_VFREQ 2500
#define CAPS_MAX_INTERLACE_VFREQ 6000
#define CAPS_MIN_INTERLACE_HFREQ 150
#define CAPS_MAX_INTERLACE_HFREQ 550

typedef struct _Sync_Info {
    WORD  HRes, VRes;
    short VFreq;
    WORD  HTotal, VTotal;
    WORD  PCLK;
    BYTE  PixelRep;
    BYTE  Mode;
} SYNC_INFO, *PSYNC_INFO;

#define F_MODE_RGB444               0
#define F_MODE_RGB444               0
#define F_MODE_YUV422               1
#define F_MODE_YUV444               2
#define F_MODE_CLRMOD_MASK          3

#define F_MODE_INTERLACE            1

#define F_MODE_ITU709               (1 << 4)
#define F_MODE_ITU601               0

#define F_MODE_0_255                0
#define F_MODE_16_235               (1 << 5)

#define F_MODE_EN_UDFILT            (1 << 6)
#define F_MODE_EN_DITHER            (1 << 7)

/*
   #define FLAG_CLEAR_INT_MODE 1
   #define FLAG_CLEAR_INT_PKG 2
   #define FLAG_CLEAR_INT_HDCP 4
   #define FLAG_CLEAR_INT_AUDIO 8
   #define FLAG_CLEAR_INT_CLR_AVMUTE 0x40
   #define FLAG_CLEAR_INT_SET_AVMUTE 0x80
   #define FLAG_CLEAR_INT_HDCP 0x80

   #define FLAG_CLEAR_INT_ALL (FLAG_CLEAR_INT_MODE|FLAG_CLEAR_INT_PKG|FLAG_CLEAR_INT_HDCP|FLAG_CLEAR_INT_AUDIO|FLAG_CLEAR_INT_CLR_AVMUTE|FLAG_CLEAR_INT_SET_AVMUTE)
   #define FLAG_CLEAR_INT_MASK (~FLAG_CLEAR_INT_ALL)

 */
#define FLAG_CLEAR_INT_MODE         1
#define FLAG_CLEAR_INT_PKG          2
#define FLAG_CLEAR_INT_ECC          4
#define FLAG_CLEAR_INT_AUDIO        8
#define FLAG_CLEAR_INT_MUTECLR      (1 << 6)
#define FLAG_CLEAR_INT_MUTESET      (1 << 7)
#define FLAG_CLEAR_INT_HDCP         (1 << 7)
#define FLAG_CLEAR_INT_ALL          (FLAG_CLEAR_INT_MUTESET | FLAG_CLEAR_INT_MUTECLR | FLAG_CLEAR_INT_MODE | FLAG_CLEAR_INT_PKG | FLAG_CLEAR_INT_ECC | FLAG_CLEAR_INT_AUDIO)
#define FLAG_CLEAR_INT_MASK         (~FLAG_CLEAR_INT_ALL)

#define EEPROM_SYNC_POL             0x1c
#define EEPROM_HWMUTE_RATE          0xe2
#define EEPROM_HWMUTE_CTRL          0x08
#define EEPROM_HWAMP_CTRL           0x00
#define EEPROM_I2S_MODE             0x60
#define EEPROM_AUDMAP_INFO          0xe4
#define EEPROM_MCLK_SEL             0x01
#define EEPROM_AUD_TRISTATE         0x10
#define EEPROM_INT_CTRL             0x30
#define EEPROM_DEFAULT_VIDEO_OUTPUT 0x00

#define EEPROM_OUTPUT_MAP           0x00
#define EEPROM_VID_CTRL             0x00

////////////////////////////////////////////////////////////////////////
// RX_DEV
////////////////////////////////////////////////////////////////////////

#ifdef _IT6607_GeNPacket_Usage_
    #define MaxPollingPacketCount 5         // 5= Per 4100ms update Null packet for SPD and VSDB packet
//#define MaxPollingPacketCount               2 // 2= Per 250ms update Null packet for SPD and VSDB packet
#endif

typedef struct tag_HDMIRX_DEVICE {
    Video_State_Type  m_VState;
    Audio_State_Type  m_AState;
    RxHDCP_State_Type m_RxHDCPState;
    AUDIO_CAPS        m_RxAudioCaps;

    unsigned int m_SWResetTimeOut;
    unsigned int m_MuteResumingTimer;
    unsigned int m_VideoCountingTimer;
    unsigned int m_AudioCountingTimer;

    unsigned int m_EventFlags;

    BYTE m_ucCurrentHDMIPort;

    BYTE m_ucVideoOnCount;
    BYTE m_ucSCDTOffCount;
    BYTE m_ucEccCount;
    BYTE m_ucDVISCDToffCNT;

    BYTE m_bOutputVideoMode;
    BYTE m_bInputVideoMode;
    BYTE m_ucAudioSampleClock;
    BYTE m_ucHDMIAudioErrorCount;

    BYTE m_bOldReg8B;
    BYTE m_ucNewSCDT;
    BYTE m_ucOldSCDT;

    BYTE m_ucSCDTonCount;
    BYTE m_ucVideoModeChange;

#ifdef _IT6607_GeNPacket_Usage_
    BYTE m_PollingPacket;
    BYTE m_PacketState;

    BYTE m_ACPState;

    BYTE m_GeneralRecPackType;
#endif

    BYTE m_bRxAVmute : 1;
    BYTE m_bVideoOnCountFlag : 1;
    BYTE m_MuteAutoOff : 1;
    BYTE m_bUpHDMIMode : 1;
    BYTE m_bUpHDCPMode : 1;
    BYTE m_NewAVIInfoFrameF : 1;
    BYTE m_NewAUDInfoFrameF : 1;
    BYTE m_HDCPRepeater : 1;
    BYTE m_MuteByPKG : 1;

#ifdef _IT6607_GeNPacket_Usage_
    BYTE m_GamutPacketRequest : 1;

#endif
} HDMIRXDEV, *PHDMIRXDEV;

////////////////////////////////////////////////////////////////////////
// Function Prototype
////////////////////////////////////////////////////////////////////////

void IT6607Interrupt_Handler(void);

void TurnOn_HDMIRX(BOOL bRepeaterEnable);
void TurnOff_HDMIRX();

void IT6607VideoHandler(void);
void IT6607SwitchVideoState(Video_State_Type eNewVState);
//void IT6607AudioHandler(void);
//void IT6607HDCPHandler(void);

void init_it6607dev();
void HWReset_HDMIRX();
void Terminate_Reset();
void RDROM_Reset();
void Check_RDROM();
void SWReset_HDMIRX(void);
//void LoadCustomizeDefaultSetting();
//void hdmirx_SetDefaultRegisterValue();

BOOL HDMIRX_IsVideoStable();

void hdmirx_INT_5V_PwrOn();
void hdmirx_INT_5V_PwrOff();
void hdmirx_INT_SCDT_On(void);
void hdmirx_INT_SCDT_Off();
void hdmirx_INT_VideoMode_Chg();
void hdmirx_INT_HDMIMode_Chg();
void hdmirx_INT_AVMute_Set();
void hdmirx_INT_AVMute_Clear();
void hdmirx_INT_SetNewAVIInfo();
void hdmirx_INT_ResetAudio();
void hdmirx_INT_ResetHDCP();
void hdmirx_XVYccCheck(void);

void IT6607AudioHandler();

void IT6607SwitchAudioState(Audio_State_Type state);
// BOOL bGetSyncInfo();
// void Video_Handler();
void hdmirx_SetVideoOutputFormat(void);
void hdmirx_SetVideoInputFormatWithoutInfoFrame(BYTE bInMode);
void hdmirx_SetColorimetryByMode();
void hdmirx_SetVideoInputFormatWithInfoFrame();

BOOL hdmirx_SetColorimetryByInfoFrame();
//void SetCSCBYPASS(void);
void hdmirx_SetColorSpaceConvert();
void hdmirx_SetDVIVideoOutput();
void hdmirx_SetNewInfoVideoOutput();
void hdmirx_SetHWMuteClrMode();
void hdmirx_SetHWMuteClr();
void hdmirx_ClearHWMuteClr();
void hdmirx_ResetAudio();
unsigned int hdmirx_SetAudioSampleRate();
void hdmirx_SetHWMuteCTRL(BYTE AndMask, BYTE OrMask);
// void MCUTimer1ISR();
void hdmirx_SetOuputSyncPolarity();
BYTE getHDMIRXCurrentPort();
BYTE getHDMIRXPortPlug();

BYTE getHDMIRXOutputColorDepth();
void hdmirx_Terminator_Off();
void hdmirx_Terminator_On();
void IT6607_SwitchRxHDCPState(RxHDCP_State_Type state);
void IT6607_DumpInfo();
void IT6607_HDCPHandler();
//void RxAuthStartInt();

void hdmirx_HDCPRepeaterCapabilitySet(BYTE uc);

void hdmirx_HDCPRepeaterCapabilityClear(BYTE uc);
void hdmirx_ForceReplySourceAuthentication();
void hdmirx_ForceKSVFIFOReady(WORD BStatus);

void Init_HDMIRX();

void hdmiregister_reset(void);
void hdmirx_ini( void );
void RxAuthSetBStatus(WORD bstatus);

void HDMIRX_SelectHDMIPort(BYTE ucPort);
void getHDMIRXInputAudio(AUDIO_CAPS *pAudioCaps);
void getCAT6023InputChStat(AUDIO_CAPS *pAudioCaps);
void DumpStateTickCount();

void CDRReset(BYTE EnaSWCDRRest);

BOOL getHMDIRX_AudioInfo(AUDIO_CAPS *pAudioCaps, RX_REG_AUDIO_CHSTS *RegChannelstatus);
void getHDMIRX_AudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus);

void DumpHDMIRXReg(void);
void IT6607Standby(void);
BOOL    IT6607MultiPCM();
BYTE    IT6607AudioChannelNum();

void IT6607VideoOutput(BOOL bEnable);
void IT6607AudioOutput(BOOL bEnable);
void SetVideoMute(unsigned char bMute);

#ifdef _IT6607_GeNPacket_Usage_
// void ClearGeneralRecPackType(void);
void ChangeGenPktType(void);
void SetGeneralRecPackType(BYTE type);
// void Switch_GeneralRecPackState(BYTE state);
void UpdateVSDBPacket(void);
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Infomation function
//////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckHDMIRX();

BOOL HDMIRX_IsInterlace();
BOOL HDMIRX_IsHDMIMode();
BOOL HDMIRX_IsHDCP();
BOOL HDMIRX_IsSCDT();
BOOL HDMIRX_IsPlg5VPwr();

BYTE getHDMIRXAudioStatus();
BOOL getHDMIRXAudioChannelStatus(BYTE ucIEC60958ChStat[]);
void getHDMIRXInputAudio(AUDIO_CAPS *pAudioCaps);
WORD getHDMIRXHorzTotal();
WORD getHDMIRXHorzActive();
WORD getHDMIRXHorzFrontPorch();
WORD getHDMIRXHorzSyncWidth();
WORD getHDMIRXHorzBackPorch();
WORD getHDMIRXVertTotal();
WORD getHDMIRXVertActive();
WORD getHDMIRXVertFrontPorch();
WORD getHDMIRXVertSyncWidth();
WORD getHDMIRXVertSyncBackPorch();
BYTE getHDMIRXxCnt();
BYTE getHDMIRXOutputColorDepth();
BYTE getHDMIRXCurrentPort();
BYTE getHDMIRXPortPlug();

BOOL getHDMIRX_AVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame);
BOOL getHDMIRX_VENDORSPECInfoFrame(BYTE *pData);
BOOL getHDMIRX_AudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame);

BOOL getHDMIRX_GamutPacket(BYTE *pGamutPacket);

void Set_IT6607_Tri_State_Enable();
void Set_IT6607_Tri_State_Disable();

void IT6607_PowerDown(
    BOOL enable);

unsigned int
IT6607_GetOutputVideoMode(
    void);

BOOL
IT6607_IsEngineIdle(
    void);

void IT6607_DisableHDCP(
    BOOL flag);

BOOL
IT6607_IsDisableHDCP(
    void);

BOOL
IT6607_IsHDCPKeyEmpty(
    void);

void
IT6607_ResetAudioFS(
    void);

#endif