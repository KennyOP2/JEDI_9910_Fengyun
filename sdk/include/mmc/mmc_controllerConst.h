//*****************************************************************************
// Name: mmc_controllerConst.h
//
// Description:
//     MMC controller's api functions head file
//
// Author: Arcane Liu
//  Date: 2003/10/14
//
// Copyright(c)2003-2004 Silicon Integrated Systems Corp. All rights reserved.
//*****************************************************************************
#ifndef __MMC_CONTROLLER_CONST_H
#define __MMC_CONTROLLER_CONST_H

//=============================================================================
//                              Constant Definition
//=============================================================================

//
// Controller Registers
//
#define MMC_CMD_REG                 0x1400  // 6 bytes

#define MMC_CMD_FIRE_REG            0x1406

#define MMC_CMD_RESPONSE_REG        0x1410  // Max 16bytes

#define MMC_CNTRLER_STATUS_REG1     0x1420

#define MMC_CNTRLER_STATUS_REG2     0x1422

#define MMC_CNTRLER_STATUS_REG3     0x1424

#define MMC_CNTRLER_MODE_REG        0x1430

#define MMC_DATA_READ_BASE_REG      0x1434  // 4 bytes
#define MMC_DATA_READ_BASE2_REG     0x1436
#define MMC_DATA_WRITE_BASE_REG     0x1438  // 4 bytes

#define MMC_RW_DATABLOCK_CNT_REG    0x143C

#define MMC_RW_DATABLOCK_LEN_REG    0x143E

#define MMC_TIMEOUT_CNT_REG         0x1440

//
// Basic Setting define
//
#define MMC_DATA_READ_BASE_ADDR     0x00000000

#define MMC_DATA_WRITE_BASE_ADDR    0x00000000

//
// MMC CMD Type define
//

// Broadcast, no response
#define MMC_CMDTYP_BC               0x0000

// Broadcast, with response
#define MMC_CMDTYP_BCR              0x0001

// Addressed (point-to-point) command, no data transfer on DAT
#define MMC_CMDTYP_AC               0x0002


// Addressed (point-to-point) command, data transfer on DAT
#define MMC_CMDTYP_ADTC             0x0003

//
// MMC CMD Response Type define
//

// R1 ----- Normal Response
#define MMC_CMDRES_R1               0x0000

// R1b ----- Normal Response with Busy in Data line
#define MMC_CMDRES_R1B              0x0001

// R2 ----- CID
#define MMC_CMDRES_R2               0x0002

// R3 ----- OCR
#define MMC_CMDRES_R3               0x0003

// R4 ----- Fast I/O
#define MMC_CMDRES_R4               0x0004

// R5 ----- Interrupt Request
#define MMC_CMDRES_R5               0x0005

//
// MMC CMD class define
//

// Stream Read
#define MMC_CMDCLASS_STREAM_RD      0x0000

// Single Block Read
#define MMC_CMDCLASS_SB_RD          0x0001

// Multiple Block Read With Stop
#define MMC_CMDCLASS_MBS_RD         0x0002

// Multiple Block Read No STOP
#define MMC_CMDCLASS_MBNS_RD        0x0003

// Reserved for Stream Write
#define MMC_CMDCLASS_STREAM_WR      0x0004

// Single Block Write
#define MMC_CMDCLASS_SB_WR          0x0005

// Reserved for Multiple Block Write with STOP
#define MMC_CMDCLASS_MBS_WR         0x0006

// Multiple Block Write No STOP
#define MMC_CMDCLASS_MBNS_WR        0x0007

// STOP Command
#define MMC_CMDCLASS_STOP           0x0008

// Cancel On Running Command
#define MMC_CMDCLASS_CANCEL         0x0009

// Basic and Brocast
#define MMC_CMDCLASS_BASIC          0x000A

//
// Control Constant
//
#define MMC_MAX_CARDS               1

#define MMC_OP_BASE_W_HMASK2        0x00C0
#define MMC_OP_BASE_R_HMASK2        0x0030

#define MMC_OP_BASE_W_HSHIFT2        6
#define MMC_OP_BASE_R_HSHIFT2        4

#define MMC_OP_BASE_HMASK           0x00FF

#define MMC_OP_BASE_LMASK           0xFFFE

#define MMC_BLCK_CNT_MASK           0x0000FFFF

#define MMC_WR_POLLING_CNT          100

#define MMC_RD_POLLING_CNT          100

#define MMC_RESPONSE_POLLING_CNT    800

#define MMC_OCR_FULL_RANGE          0x00000000

#define MMC_OCR_NOT_BUSY            0x80000000

#define MMC_STATUS_TIMEOUT          0x0020

#define MMC_STATUS_CMD_RES_OK       0x0200

#define MMC_CMDCHK_OK               1

#define MMC_CMDCHK_FAIL             0

#define MMC_CMDCHK_TIME_OUT         2

#define MMC_CMDCHK_CNT_ERR          4

#define MMC_STATE_IDLE              0

#define MMC_STATE_READY             1

#define MMC_STATE_IDENT             2

#define MMC_STATE_STBY              3

#define MMC_STATE_TRAN              4

#define MMC_STATE_DATA              5

#define MMC_STATE_RCV               6

#define MMC_STATE_PRG               7

#define MMC_STATE_DIS               8

#define MMC_C_SIZE_MULT_SHIFT       2

#define MMC_MEMORY_ALLOCATE_SIZE    (2048*4*4*2)

#define MMC_WAIT_INTERRUPT_OCCUR_COUNTER 0x1000000

#define REG_BIT_4BIT_EN             (1 << 3)
#define REG_BIT_4BIT_DIS            0

/*
typedef
void
(*PMMC_INT_CALLBACK) (
    void);
*/

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

#endif // End of #ifndef __MMC_CONTROLLER_CONST_H




