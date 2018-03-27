/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for project definition
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __PROJ_DEFS_H__
#define __PROJ_DEFS_H__

/////////////////////////////////////////////////////////////////
// Project definition
/////////////////////////////////////////////////////////////////

#  define CHIP_ID MM9910


/////////////////////////////////////////////////////////////////
// For Project MM9070 || MM9910
/////////////////////////////////////////////////////////////////
#if defined(MM9070) || defined(MM9910)
// Hardware trapping
#  define MMIO_TRAP             (0x0000)
#  define PLL_CFG_BIT           (7)
#  define PLL_CFG               (0x3 << PLL_CFG_BIT)

#  define MMIO_IIS_PLL_SRC      (0x1642)
#  define IIS_PLL_SRC_BIT       (14)
#  define IIS_PLL_SRC           (3 << IIS_PLL_SRC_BIT)

#  define MMIO_MCLK_SRC         (0x0014)
#  define MMIO_NCLK_SRC         (0x0018)
#  define MMIO_WCLK_SRC         (0x001C)
#  define MCLK_SRC_BIT          (12)
#  define MCLK_SRC              (3 << NCLK_SRC_BIT)
#  define NCLK_SRC_BIT          (12)
#  define NCLK_SRC              (3 << NCLK_SRC_BIT)
#  define WCLK_SRC_BIT          (12)
#  define WCLK_SRC              (3 << WCLK_SRC_BIT)

#  define MMIO_MCLK_DIV         (0x0014)
#  define MCLK_DIV_BIT          (0)
#  define MCLK_DIV              (0x7FF << NCLK_DIV_BIT)

#  define MMIO_NCLK_DIV         (0x0018)
#  define NCLK_DIV_BIT          (0)
#  define NCLK_DIV              (0x7FF << NCLK_DIV_BIT)

#  define MMIO_WCLK_DIV         (0x001C)
#  define WCLK_DIV_BIT          (0)
#  define WCLK_DIV              (0x7FF << WCLK_DIV_BIT)

// MMIO Enable Bits for RISC
#  define MMIO_ENABLE           0x202
#  define En_RISC_MMIO          11
#  define MMIO_PLL_MASK         0x1fff

// Disable relock PLL2
#  define EnRelockPLL1_MMIO     0x4c
#  define EnRelockPLL2_MMIO     0x4c
#  define EnRelockPLL3_MMIO     0x4c
#  define EnRelockPLL1_Bits     4       // 0: Enable PLL1 relock (default)
                                        // 1: Disable PLL1 relock
#  define EnRelockPLL2_Bits     5       // 0: Enable PLL2 relock (default)
                                        // 1: Disable PLL2 relock
#  define EnRelockPLL3_Bits     6       // 0: Enable PLL3 relock (default)
                                        // 1: Disable PLL3 relock
#endif

// Remap Address, for P370, P220 or latter projects
#define MMIO_RemapAddr_Hi       0x16e0
#define MMIO_RemapAddr_Lo       0x16de

/////////////////////////////////////////////////////////////////
// Others MMIO Mapping
/////////////////////////////////////////////////////////////////
#define MMIO_PLL1       0x40
#define MMIO_PLL2       0x44
#define MMIO_PLL3       0x48

#endif // __PROJ_DEFS_H__

