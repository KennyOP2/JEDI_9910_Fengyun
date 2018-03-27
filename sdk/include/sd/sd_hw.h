/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as SD HW configure header file.
 *
 * @author Irene Lin
 */

#ifndef SD_HW_H
#define SD_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "sd/config.h"
#include "sd/sd_reg.h"
#include "sd/sd.h"


//=============================================================================
//                              Constant Definition
//=============================================================================
#define SDMMC_READ      0
#define SDMMC_WRITE     1

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#define SD_CrcBypassReg()           AHB_WriteRegisterMask(SD_REG_STS0, SD_MSK_CRC_BYPASS, SD_MSK_CRC_BYPASS)
#define SD_CrcEnableReg()           AHB_WriteRegisterMask(SD_REG_STS0, 0x0, SD_MSK_CRC_BYPASS)
#define SD_CrcNonFixReg(x)           AHB_WriteRegisterMask(SD_REG_STS0, (x==MMP_TRUE)?SD_MSK_CRC_NON_FIX:0x0, SD_MSK_CRC_NON_FIX)
#define SD_SetClockDivReg(clkDiv)   AHB_WriteRegister(SD_REG_CLK_DIV, (clkDiv<<2))
#if defined(MM9910)
#define SD_SetRespTimeoutReg(cardTick)     AHB_WriteRegister(SD_REG_RESP_TIMEOUT, cardTick)
#define SD_WrapIntrDisable()        AHB_WriteRegisterMask(SDW_REG_WRAP_STATUS, SDW_MSK_INTR_MASK, SDW_MSK_INTR_MASK)
#define SD_WrapIntrEnable()         AHB_WriteRegisterMask(SDW_REG_WRAP_STATUS, 0x0, SDW_MSK_INTR_MASK)
#define SD_SelectIo(idx)            do { \
                                        AHB_WriteRegisterMask(SD_REG_STS0, (idx<<4), (1<<4)); \
                                        HOST_StorageIoSelect((idx==SD_1) ? MMP_CARD_SD_I : MMP_CARD_SD2); \
                                    } while(0)
#else
#define SD_SetRespTimeoutReg(x)
#define SD_WrapIntrDisable()
#define SD_WrapIntrEnable()
#define SD_SelectIo(idx)            HOST_StorageIoSelect((idx==SD_1) ? MMP_CARD_SD_I : MMP_CARD_SD2)
#endif

#if defined(FPGA)
#define SDMMC_DefaultDelay()        
#define SDMMC_ReadDelay()           
#define SDMMC_WriteDelay()          
#else
#define SDMMC_DefaultDelay()        AHB_WriteRegisterMask((SD_BASE+0x008C), 0x0, 0x010000FF)
#define SDMMC_ReadDelay(hs)         do { \
                                        MMP_UINT32 timing = hs ? 0x00000020 : 0x00000000; \
                                        AHB_WriteRegisterMask((SD_BASE+0x008C), timing, 0x010000FF); \
                                    } while(0);
#define SDMMC_WriteDelay(hs)        do { \
                                        MMP_UINT32 timing = hs ? 0x00000020 : 0x00000000; \
                                        AHB_WriteRegisterMask((SD_BASE+0x008C), timing, 0x010000FF); \
                                    } while(0);
//#define SD_SetHighSpeedReg()        AHB_WriteRegisterMask(SD_REG_STS0, SD_MSK_HIGH_SPEED, SD_MSK_HIGH_SPEED)
#define SD_SetHighSpeedReg()        
#endif

//=============================================================================
//							Funtion Declaration
//=============================================================================
void SD_PowerOnReg(MMP_INT index);

void SD_PowerDownReg(SD_CTXT* ctxt);

/** reset sd card controller */
static MMP_INLINE void SD_ResetIFReg(void)
{
	/* 9070 A0 can't do this, it will trigger interrupt */
	//AHB_WriteRegisterMask(SD_REG_STS0, SD_MSK_IF_RESET, SD_MSK_IF_RESET);
    //AHB_WriteRegisterMask(SD_REG_STS0, 0, SD_MSK_IF_RESET);
#if defined(ASYNC_RESET)
    #if 1//defined(SD_RESET)
    HOST_WriteRegisterMask(0x22, (0x1<<8), (0x1<<8)); 
    MMP_Sleep(1);
    HOST_WriteRegisterMask(0x22, 0x0000, (0x1<<8));
    MMP_Sleep(1);
    #endif
#endif

    #if defined(MM9910)
    /** disable interrupt */
    AHB_WriteRegister(SD_REG_INTR, (SD_INTR_ALL<<SD_SHT_INTR_MSK)|SD_INTR_ALL);
    SD_WrapIntrDisable();
    #else
	//AHB_WriteRegisterMask(SD_REG_STS1, SD_MSK_INTR_MSK, SD_MSK_INTR_MSK); /** disable interrupt */
    #endif

    #if defined(SD_READ_FLIP_FLOP)
    /** sd data in from flip-flop out */
    AHB_WriteRegisterMask((GPIO_BASE+0xD4), (0x1<<16), (0x1<<16));
    SD_CrcNonFixReg(MMP_TRUE);
    #endif
}

/** reset sd card controller + smedia wrapper */
static MMP_INLINE void SDW_SwResetReg(void)
{
    AHB_WriteRegisterMask(SDW_REG_WRAP_CTRL, SDW_MSK_SW_RESET, SDW_MSK_SW_RESET);
    AHB_WriteRegisterMask(SDW_REG_WRAP_CTRL, 0x0, SDW_MSK_SW_RESET);
}

static MMP_INLINE void SD_SetClockReg(MMP_UINT32 clkSrc, MMP_UINT32 clkDiv)
{
    AHB_WriteRegisterMask(SD_REG_STS0, clkSrc, (SD_MSK_ASYN_CLK_SEL|SD_MSK_CLK_SRC));
    AHB_WriteRegister(SD_REG_CLK_DIV, clkDiv);
}

static MMP_INLINE void SD_SetBusWidthReg(MMP_UINT8 busWidth)
{
    MMP_UINT32 value = 0;
    switch(busWidth)
    {
    case 1:
        value = SD_BUS_WIDTH_1BIT;
        break;
    case 4:
        value = SD_BUS_WIDTH_4BIT;
        break;
    case 8:
        value = SD_BUS_WIDTH_8BIT;
        break;
    default:
        value = SD_BUS_WIDTH_1BIT;
        LOG_ERROR " SD_SetBusWidthReg() Invalid Bus Width!!! \n" LOG_END
        break;
    }
    AHB_WriteRegisterMask(SD_REG_STS0, value, SD_MSK_BUS_WIDTH);
}

static MMP_INLINE void SD_GenClockReg(void)
{
    AHB_WriteRegisterMask(SD_REG_CTL, SD_MSK_CLK_CTRL, SD_MSK_CLK_CTRL);
    MMP_Sleep(15);
    AHB_WriteRegisterMask(SD_REG_CTL, 0, SD_MSK_CLK_CTRL);
}

static MMP_INLINE void SD_GetSectorLengthReg(MMP_UINT32* length)
{
#if defined(MM9910)
    AHB_ReadRegister(SD_REG_LENGTH, length);
#else
    MMP_UINT32 tmpH = 0;
    MMP_UINT32 tmpL = 0;
    AHB_ReadRegister(SD_REG_LENGTH_H, &tmpH);
    AHB_ReadRegister(SD_REG_LENGTH_L, &tmpL);
    (*length) = ((tmpH & 0xFF) << 8) | (tmpL & 0xFF);
#endif
    (*length) += 1;
}

static MMP_INLINE void SD_SetSectorCountReg(MMP_UINT32 count)
{
    MMP_UINT32 sectorLen = 0;
    AHB_WriteRegister(SD_REG_SECTOR_COUNT_L, (MMP_UINT32)(count & 0xFF));
    AHB_WriteRegister(SD_REG_SECTOR_COUNT_H, (MMP_UINT32)((count>>8) & 0xFF));
    /** for smedia wrap */
    SD_GetSectorLengthReg(&sectorLen);
    AHB_WriteRegister(SDW_REG_DATA_LEN, (MMP_UINT32)(count * sectorLen));
}

static MMP_INLINE void SD_SetSectorLengthReg(MMP_UINT32 length)
{
    length -= 1;
#if defined(MM9910)
    AHB_WriteRegister(SD_REG_LENGTH, length);
#else
    AHB_WriteRegister(SD_REG_LENGTH_H, (MMP_UINT32)((length>>8) & 0xFF));
    AHB_WriteRegister(SD_REG_LENGTH_L, (MMP_UINT32)(length & 0xFF));
#endif
}

static MMP_INLINE MMP_UINT32 SD_IsErrorReg(void)
{
    MMP_UINT32 reg = 0;
    AHB_ReadRegister(SD_REG_STS1, &reg);
    return (reg & SD_ERROR);
}

#if defined(SD_IRQ_ENABLE)
void dma_isr(void* arg, MMP_UINT32 status);
void sd_isr(void* arg);
#endif // #if defined(SD_IRQ_ENABLE)

MMP_INT SD_SendCmdReg(
    SD_CTXT*  ctxt,
    MMP_UINT8 command,
    MMP_UINT8 arg3,
    MMP_UINT8 arg2,
    MMP_UINT8 arg1,
    MMP_UINT8 arg0,
    MMP_UINT8 condition);

void SD_SendCmdNoWaitReg(
    SD_CTXT*  ctxt,
    MMP_UINT8 command,
    MMP_UINT8 arg3,
    MMP_UINT8 arg2,
    MMP_UINT8 arg1,
    MMP_UINT8 arg0,
    MMP_UINT8 condition);

MMP_INT SD_WaitSendCmdReadyReg(SD_CTXT* ctxt);

MMP_INT SDW_WaitWrapReadyReg(SD_CTXT* ctxt);

void SD_ReadResponseReg(MMP_UINT16 readByte);

static MMP_INLINE MMP_UINT32 SD_ReadDataReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(SDW_REG_DATA_PORT, &value);
    return value;
}

static MMP_INLINE void SD_WriteDataReg(MMP_UINT32 data)
{
    AHB_WriteRegister(SDW_REG_DATA_PORT, data);
}

MMP_INT SD_WaitFifoFullReg(void);

MMP_INT SD_WaitFifoEmptyReg(void);

static MMP_INLINE void SDW_SetWrapCtrlReg(SD_CTXT*  ctxt, MMP_UINT8 dir, MMP_UINT8 dma)
{
    MMP_UINT32 value = SDW_MSK_WRAP_FIRE;
    MMP_UINT32 mask = SDW_MSK_DATA_IN | SDW_MSK_HW_HANDSHAKING | SDW_MSK_WRAP_FIRE;
    
    if(dir == SDW_DATA_IN)
        value |= SDW_MSK_DATA_IN;
    if(dma == SDW_DMA)
    {
        value |= SDW_MSK_HW_HANDSHAKING;
        ctxt->flags |= SD_FLAGS_DATA_DMA;
        #if defined(MM9910) && defined(SD_IRQ_ENABLE)
        ctxt->flags |= SD_FLAGS_WRAP_INTR;
        SD_WrapIntrEnable();
        #else
        ctxt->flags |= SD_FLAGS_WRAP; /** check wrap ready by polling */
        #endif
    }
    else
    {
        ctxt->flags &= ~SD_FLAGS_DATA_DMA;
        SD_WrapIntrDisable();
        /** if without dma, we will check wrap ready by SDW_WaitWrapReadyReg() after read/write data by PIO */
        //ctxt->flags |= SD_FLAGS_WRAP;  
    }

    AHB_WriteRegisterMask(SDW_REG_WRAP_CTRL, value, mask);
}

static MMP_INLINE MMP_BOOL SD_IsCardInserted(MMP_INT index)
{
    switch(index)
    {
    case SD_1:
        return HOST_IsCardInserted(MMP_CARD_SD_I);
    case SD_2:
        return HOST_IsCardInserted(MMP_CARD_SD2);
    }
    return MMP_FALSE;
}

void SD_DumpReg(void);

#ifdef __cplusplus
}
#endif

#endif //SD_HW_H
