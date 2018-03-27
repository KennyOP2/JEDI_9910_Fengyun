/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as DMA Controller Register Definition.
 *
 * @author Irene Lin
 */
#ifndef	DMA_REG_H
#define	DMA_REG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "dma/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Register Definition
//=============================================================================

#define DMA_REG_INT_STATUS                          (DMA_BASE + 0x0000)
#define DMA_REG_INT_TC_STATUS                       (DMA_BASE + 0x0004)
#define DMA_REG_INT_TC_CLEAR                        (DMA_BASE + 0x0008)
#define DMA_REG_INT_ERR_ABT_STATUS                  (DMA_BASE + 0x000C)
#define DMA_REG_INT_ERR_ABT_CLEAR                   (DMA_BASE + 0x0010)
#define DMA_REG_TC_STATUS                           (DMA_BASE + 0x0014)
#define DMA_REG_ERR_ABT_STATUS                      (DMA_BASE + 0x0018)
#define DMA_REG_CHANNEL_EN_STATUS                   (DMA_BASE + 0x001C)
#define DMA_REG_CHANNEL_BUSY_STATUS                 (DMA_BASE + 0x0020)

/**
 * 0x24 CSR Register
 */
#define DMA_REG_MAIN_CSR                            (DMA_BASE + 0x0024)

#define DMA_MSK_CTRL_EN                             0x00000001
#define DMS_MSK_M0_BIG_ENDIAN                       0x00000002
#define DMS_MSK_M1_BIG_ENDIAN                       0x00000004

/**
 * 0x28 CSR Register
 */
#define DMA_REG_SYNC                                (DMA_BASE + 0x0028)
#define DMA_REG_FEATURE                             (DMA_BASE + 0x0034)

/** if the bridge option is turned on and DMAC32 is applied. */
#define DMA_REG_C0_DEVICE_DATA_ADDR                 (DMA_BASE + 0x0040) 
#define DMA_REG_C0_DEVICE_CTRL_REG_BASE             (DMA_BASE + 0x0080) 

//===============================================================
/**
 * 0x100 CSR Register
 */
//===============================================================
#define DMA_REG_C0_CSR                              (DMA_BASE + 0x0100) 

#define DMA_MSK_TC                                  0x80000000
#define DMA_MSK_FIFO_TH                             0x07000000
#define DMA_MSK_CHANNEL_PRIORITY                    0x00C00000
#define DMA_MSK_SRC_BURST_SIZE                      0x00070000
#define DMA_MSK_TX_ABORT                            0x00008000
#define DMA_MSK_SRC_TX_WIDTH                        0x00003800
#define DMA_MSK_DST_TX_WIDTH                        0x00000700
#define DMA_MSK_HW_HANDSHAKE_MODE                   0x00000080
#define DMA_MSK_SRC_ADDR_CTRL                       0x00000060
#define DMA_MSK_DST_ADDR_CTRL                       0x00000018
#define DMA_MSK_SRC_AHB1                            0x00000004
#define DMA_MSK_DST_AHB1                            0x00000002
#define DMA_MSK_CHANNEL_EN                          0x00000001

#define DMA_SHT_FIFO_TH								24
#define DMA_SHT_CHANNEL_PRIORITY                    22
#define DMA_SHT_SRC_BURST_SIZE                      16
#define DMA_SHT_SRC_TX_WIDTH                        11
#define DMA_SHT_DST_TX_WIDTH                        8
#define DMA_SHT_SRC_ADDR_CTRL                       5
#define DMA_SHT_DST_ADDR_CTRL                       3

enum
{
	DMA_FIFO_TH_1	=   0x00000000,
	DMA_FIFO_TH_2	=   0x01000000,
	DMA_FIFO_TH_4	=   0x02000000,
	DMA_FIFO_TH_8	=   0x03000000,
	DMA_FIFO_TH_16	=   0x04000000
};

enum
{
    DMA_SRC_TX_WIDTH_8_BITS    = (DMA_TX_WIDTH_8<<DMA_SHT_SRC_TX_WIDTH),
    DMA_SRC_TX_WIDTH_16_BITS   = (DMA_TX_WIDTH_16<<DMA_SHT_SRC_TX_WIDTH),
    DMA_SRC_TX_WIDTH_32_BITS   = (DMA_TX_WIDTH_32<<DMA_SHT_SRC_TX_WIDTH)
};

enum
{
    DMA_SRC_BURSET_SIZE_1    = 0x00000000,
    DMA_SRC_BURSET_SIZE_4    = 0x00010000,
    DMA_SRC_BURSET_SIZE_8    = 0x00020000,
    DMA_SRC_BURSET_SIZE_16   = 0x00030000,
    DMA_SRC_BURSET_SIZE_32   = 0x00040000,
    DMA_SRC_BURSET_SIZE_64   = 0x00050000,
    DMA_SRC_BURSET_SIZE_128  = 0x00060000,
    DMA_SRC_BURSET_SIZE_256  = 0x00070000
};

enum
{
    DMA_DST_TX_WIDTH_8_BITS    = (DMA_TX_WIDTH_8<<DMA_SHT_DST_TX_WIDTH),
    DMA_DST_TX_WIDTH_16_BITS   = (DMA_TX_WIDTH_16<<DMA_SHT_DST_TX_WIDTH),
    DMA_DST_TX_WIDTH_32_BITS   = (DMA_TX_WIDTH_32<<DMA_SHT_DST_TX_WIDTH)
};

enum
{
    DMA_SRC_ADDR_CTRL_INC      = (DMA_ADDR_CTRL_INC<<DMA_SHT_SRC_ADDR_CTRL),
    DMA_SRC_ADDR_CTRL_DEC      = (DMA_ADDR_CTRL_DEC<<DMA_SHT_SRC_ADDR_CTRL),
    DMA_SRC_ADDR_CTRL_FIX      = (DMA_ADDR_CTRL_FIX<<DMA_SHT_SRC_ADDR_CTRL),
};

enum
{
    DMA_DST_ADDR_CTRL_INC      = (DMA_ADDR_CTRL_INC<<DMA_SHT_DST_ADDR_CTRL),
    DMA_DST_ADDR_CTRL_DEC      = (DMA_ADDR_CTRL_DEC<<DMA_SHT_DST_ADDR_CTRL),
    DMA_DST_ADDR_CTRL_FIX      = (DMA_ADDR_CTRL_FIX<<DMA_SHT_DST_ADDR_CTRL),
};

/**
 * 0x104 CSR Register
 */
#define DMA_REG_C0_CFG                              (DMA_BASE + 0x0104) 

#define DMA_MSK_DST_HW_HANDSHAKING_EN               0x00002000
#define DMA_MSK_DST_REQUEST_SEL                     0x00001E00
#define DMA_MSK_CHANNEL_BUSY                        0x00000100
#define DMA_MSK_SRC_HW_HANDSHAKING_EN               0x00000080
#define DMA_MSK_SRC_REQUEST_SEL                     0x00000078
#define DMA_MSK_INTR_ABT_MSK                        0x00000004
#define DMA_MSK_INTR_ERR_MSK                        0x00000002
#define DMA_MSK_INTR_TC_MSK                         0x00000001

#define DMA_SHT_DST_REQUEST_SEL                     9
#define DMA_SHT_SRC_REQUEST_SEL                     3
#define DMA_INTR_MSK		                        (DMA_MSK_INTR_ABT_MSK|DMA_MSK_INTR_ERR_MSK|DMA_MSK_INTR_TC_MSK)

enum
{
    DMA_HW_HANDSHAKING_MS = 0,
    DMA_HW_IR_Cap_TX      = 0,
    DMA_HW_IR_Cap         = 1,
 	DMA_HW_SSP_TX         = 2,
    DMA_HW_SSP_RX         = 3,
	DMA_HW_SSP2_TX        = 4,
	DMA_HW_HANDSHAKING_SPDIF = DMA_HW_SSP2_TX,
	DMA_HW_SSP2_RX        = 5,
    DMA_HW_UART_TX        = 6,
    DMA_HW_UART_RX        = 7,
	DMA_HW_UART_FIR       = 8,
	DMA_HW_UART2_TX       = 9,
	DMA_HW_UART2_RX       = 10,
    DMA_HW_HANDSHAKING_CF = 12   // SD/CF/xD
    /** other engine still not implement */
};

/**
 * 0x108 CSR Register
 */
#define DMA_REG_C0_SRC_ADDR                         (DMA_BASE + 0x0108) 
#define DMA_REG_C0_DST_ADDR                         (DMA_BASE + 0x010C) 
#define DMA_REG_C0_LINKED_LIST_POINTER              (DMA_BASE + 0x0110)
#define DMA_REG_C0_TX_SIZE                          (DMA_BASE + 0x0114) 

#define DMA_CHANNEL_OFFSET                          0x20


#ifdef __cplusplus
}
#endif

#endif
