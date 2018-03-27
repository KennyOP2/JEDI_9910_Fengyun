/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for OpenRISC function
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __OR32_HOST_H__
#define __OR32_HOST_H__

#include "proj_defs.h"

// Define the HAVE_INTISR will call the ISR when then Audio Engine finish
// It will consume some computation power. Default is off.
//#define HAVE_INTISR

#define GIR_SELECT        0x6
#define GIR_SET           0x8
#define GIR_CLEAR         0xA
#define GIR_STATUS        0xC

/* Bits in GIR mmio registers */

#  define GIR_USB                 (1<<12)
#  define GIR_NFC                 (1<<11)
#  define GIR_SPI                 (1<<10)
#  define GIR_2D                  (1<<9)
#  define GIR_OPENRISC            (1<<8)
#  define GIR_CMDQ                (1<<7)
#  define GIR_OPENRISC1           (1<<6)
#  define GIR_MICROP0             (1<<5)
#  define GIR_MMC                 (1<<4)
#  define GIR_H264                (1<<3)
#  define GIR_MPEG                (1<<2)
#  define GIR_JPEG                (1<<1)
#  define GIR_I2S                 (1)


#endif // __OR32_HOST_H__

