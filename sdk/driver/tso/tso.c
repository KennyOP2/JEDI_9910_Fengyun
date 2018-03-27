/*
 * Copyright (c) 2010 ITE technology Corp. All Rights Reserved.
 */
/** @file tsi.c
 * Used to receive data through the transport stream interface (TSO).
 *
 * @author I-Chun Lai
 * @version 0.1
 */
//=============================================================================
//                              Include Files
//=============================================================================

#include "pal/pal.h"
#include "sys/sys.h"
#include "host/host.h"
#include "host/ahb.h"
#include "mmp_tso.h"

#if defined(TSO_IRQ_ENABLE)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#else
#include "intr/intr.h"
#endif
#endif
//=============================================================================
//                              Constant Definition
//=============================================================================

//Define Register
#define TSO_START_MEM_BASE_ADDR_REG1        (0X2100)
#define TSO_START_MEM_BASE_ADDR_REG2        (0x2102)
#define TSO_END_MEM_BASE_ADDR_REG1          (0x2104)
#define TSO_END_MEM_BASE_ADDR_REG2          (0x2106)
#define TSO_WRITE_LEN_REG1                  (0x2108)
#define TSO_WRITE_LEN_REG2                  (0x210A)
#define TSO_WRITE_CTRL_REG                  (0x210C)
#define TSO_VIDEO_PID_REG                   (0x210E)
#define TSO_INTERFACE_CTRL_REG              (0x2110)
#define PCR_CLOCK_REG                       (0x2112)
#define PCR_BASE_INIT_VAL_REG1              (0x2114)
#define PCR_BASE_INIT_VAL_REG2              (0x2116)
#define PCR_EXT_INIT_VAL_REG                (0x2118)
#define TS_INTERNAL_CTRL_REG1               (0x211A)
#define TS_INTERNAL_CTRL_REG2               (0x211C)
#define TSO_FIRE_REG                        (0x211E)
#define TS_READ_LEN_REG1                    (0x2120)
#define TS_READ_LEN_REG2                    (0x2122)
#define TSO_ENG_STATUS_REG                  (0x2124)

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct TSO_MODULE_TAG
{
    MMP_MUTEX   tMgrMutex;
    MMP_UINT8*  pAllocAddr;
    MMP_UINT32  baseAddr;
    MMP_UINT32  videoPid;
    MMP_FLOAT   pcrClock;
    MMP_UINT32  pcrInitValue;
    MMP_BOOL    bInjectPcr;
    MMP_INT32   bufferSize;
    MMP_UINT32  writeIndex;
} TSO_MODULE;

//=============================================================================
//                              Global Data Definition
//=============================================================================

static TSO_MODULE  gtTso = { 0 };

//=============================================================================
//                              Private Function Declaration
//=============================================================================

void
_TSO_SetPadSel(
	MMP_UINT32 startPort);
	
//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_RESULT
mmpTsoInitialize(
    MMP_UINT32 padStart,
    MMP_UINT32 videoPid,
    MMP_FLOAT  pcrClock,
    MMP_UINT32 startPcrValue,
    MMP_UINT8* pExternalBuffer,
    MMP_INT32  bufferSize,
    MMP_BOOL   bInjectPcr)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_INT32  pcrInteger = (MMP_INT32) pcrClock;
    MMP_FLOAT  pcrDecimal = (MMP_FLOAT) (pcrClock - pcrInteger);
    MMP_UINT16 regVal = 0;

    // TSO module was inited.
    if (gtTso.tMgrMutex)
    {
        return MMP_SUCCESS;
    }

    // Make buffer is 188 byte alignment
    if (0 == bufferSize || (bufferSize % 188) || ((MMP_UINT32) pExternalBuffer & 0x7))
    {
        goto tso_init_fail;
    }
    else
    {
        if (pExternalBuffer)
        {         
            // Make the address to 16 byte aligment to gain better performance.
            gtTso.baseAddr = (MMP_UINT32) pExternalBuffer;
            HOST_WriteRegister(TSO_START_MEM_BASE_ADDR_REG1, ((gtTso.baseAddr >> 3) & 0xFFFF));
            HOST_WriteRegister(TSO_START_MEM_BASE_ADDR_REG2, ((gtTso.baseAddr >> 19) & 0xFFFF));
            HOST_WriteRegister(TSO_END_MEM_BASE_ADDR_REG1, (((gtTso.baseAddr  + bufferSize - 1) >> 3) & 0xFFFF));
            HOST_WriteRegister(TSO_END_MEM_BASE_ADDR_REG2, (((gtTso.baseAddr  + bufferSize) >> 19) & 0xFFFF));        
            PalMemset((MMP_UINT8*) gtTso.baseAddr, 0x0, bufferSize);
            gtTso.pAllocAddr = MMP_NULL;
        }
        else
        {
            gtTso.pAllocAddr = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, bufferSize + 8);
            if (MMP_NULL == gtTso.pAllocAddr)
            {
                goto tso_init_fail;
            }
            // Make the address to 16 byte aligment to gain better performance.
            gtTso.baseAddr = (((MMP_UINT32)gtTso.pAllocAddr + 8) & 0xFFFFFFF8);
            HOST_WriteRegister(TSO_START_MEM_BASE_ADDR_REG1, ((gtTso.baseAddr >> 3) & 0xFFFF));
            HOST_WriteRegister(TSO_START_MEM_BASE_ADDR_REG2, ((gtTso.baseAddr >> 19) & 0xFFFF));
            HOST_WriteRegister(TSO_END_MEM_BASE_ADDR_REG1, (((gtTso.baseAddr  + bufferSize - 1) >> 3) & 0xFFFF));
            HOST_WriteRegister(TSO_END_MEM_BASE_ADDR_REG2, (((gtTso.baseAddr  + bufferSize) >> 19) & 0xFFFF));        
            PalMemset((MMP_UINT8*) gtTso.baseAddr, 0x0, bufferSize);
        }
    }

    if (MMP_NULL == gtTso.tMgrMutex)
    {
        gtTso.tMgrMutex = PalCreateMutex(MMP_NULL);
        if (MMP_NULL == gtTso.tMgrMutex)
        {
            goto tso_init_fail;
        }
    }

    // If engine autogenerate and inject PCR into output ts packet,
    // the following parameters will be applied.
    if (bInjectPcr && videoPid && pcrInteger)
    {
        if (videoPid > 0x1FFF
         || pcrInteger > 128)
        {
            goto tso_init_fail;
        }
        // Set Video PID
        HOST_WriteRegister(TSO_VIDEO_PID_REG, ((videoPid & 0x1FFF) | (0x1 << 15)));

        // Set PCR clock
        // 8 7 6 5 4 3 2   |  1 0
        // Integer         |  Decimal
        regVal = 0;
        if (pcrDecimal >= 0.75)
        {
            regVal |= 3;
        }
        else if (pcrDecimal >= 0.5)
        {
            regVal |= 2;
        }
        else if (pcrDecimal >= 0.25)
        {
            regVal |= 1;
        }
        regVal |= (pcrInteger << 2);
        HOST_WriteRegister(PCR_CLOCK_REG, regVal);
        printf("tso.c(%d), PCR clock : %f Mhz, regVal: 0x%X\n", __LINE__, pcrClock, regVal); 

        // PCR Init value
        HOST_WriteRegister(PCR_BASE_INIT_VAL_REG1, (startPcrValue & 0xFFFF));
        HOST_WriteRegister(PCR_BASE_INIT_VAL_REG2, ((startPcrValue >> 16) & 0xFFFF));
        if (startPcrValue)
        {
            MMP_UINT16 pcrReg1 = 0, pcrReg2 = 0;
            HOST_ReadRegister(PCR_BASE_INIT_VAL_REG1, &pcrReg1);
            HOST_ReadRegister(PCR_BASE_INIT_VAL_REG2, &pcrReg2);
            printf("statr PCR: %u, reg1: 0x%X, reg2: 0x%X\n", startPcrValue, pcrReg1, pcrReg2);
        }
    }
    else
    {
        // Disable Video PCR injection
        HOST_WriteRegister(TSO_VIDEO_PID_REG, 0x0);   
    }
    // Turn on CLK inverter
    HOST_WriteRegisterMask(TSO_INTERFACE_CTRL_REG, (0x1 << 7), (0x1 << 7));
    HOST_WriteRegisterMask(TS_INTERNAL_CTRL_REG1, 0x1, 0xF);
    gtTso.videoPid = videoPid;
    gtTso.pcrClock = pcrClock;
    gtTso.pcrInitValue = startPcrValue;
    gtTso.bInjectPcr = bInjectPcr;
    gtTso.bufferSize = bufferSize;
    _TSO_SetPadSel(padStart);
    return result;

tso_init_fail:
    if (gtTso.pAllocAddr)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, (void*) gtTso.pAllocAddr);
    }
    if (gtTso.tMgrMutex)
    {
        PalDestroyMutex(gtTso.tMgrMutex);
    }
    PalMemset(&gtTso, 0x0, sizeof(TSO_MODULE));
    return MMP_TSO_INIT_FAIL;
}

MMP_RESULT
mmpTsoTerminate(
    void)
{
    if (gtTso.pAllocAddr)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, (void*) gtTso.pAllocAddr);
    }
    if (gtTso.tMgrMutex)
    {
        PalDestroyMutex(gtTso.tMgrMutex);
    }
    PalMemset(&gtTso, 0x0, sizeof(TSO_MODULE));
    return MMP_SUCCESS;
}

MMP_RESULT
mmpTsoEnable(
    void)
{
    HOST_WriteRegister(TSO_FIRE_REG, 0x1);
    return MMP_SUCCESS;
}

MMP_RESULT
mmpTsoDisable(
    void)
{
    //PalWaitMutex(gtTso.tMgrMutex, PAL_MUTEX_INFINITE);
    //PalReleaseMutex(gtTso.tMgrMutex);
    return MMP_SUCCESS;
}

MMP_RESULT
mmpTsoWrite(
    MMP_UINT8*  pInputBuffer,
    MMP_ULONG   bufferSize)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_INT32  remainBufferSize = 0;
    MMP_UINT16 regVal = 0;
    MMP_INT32  sizeToEnd = 0;
    MMP_INT32  usedLen = 0;

    PalWaitMutex(gtTso.tMgrMutex, PAL_MUTEX_INFINITE);
    
    if (pInputBuffer && bufferSize)
    {
        if (bufferSize > gtTso.bufferSize)
        {
            result = MMP_TSO_OUT_OF_MEM;
            goto exit;
        }

        do
        {
            HOST_ReadRegister(TS_READ_LEN_REG1, &regVal);
            usedLen |= regVal;
            HOST_ReadRegister(TS_READ_LEN_REG2, &regVal);
            usedLen |= ((regVal & 0x3F) << 16);
            remainBufferSize = (gtTso.bufferSize - usedLen);
            if (bufferSize > remainBufferSize)
            {
                PalSleep(1);
            }
            else
            {
                sizeToEnd = gtTso.bufferSize - gtTso.writeIndex;
                if (sizeToEnd >= bufferSize)
                {
                    PalMemcpy((MMP_UINT8*) (gtTso.baseAddr + gtTso.writeIndex),
                              pInputBuffer,
                              bufferSize);
                    //printf("1IN: %x, %x\n", pInputBuffer[0], pInputBuffer[bufferSize-1]);
                }
                else
                {
                    PalMemcpy((MMP_UINT8*) (gtTso.baseAddr + gtTso.writeIndex),
                              pInputBuffer,
                              sizeToEnd);
                    PalMemcpy((MMP_UINT8*) gtTso.baseAddr,
                              &pInputBuffer[sizeToEnd],
                              bufferSize - sizeToEnd);
                    //printf("2IN: %x, %x\n", pInputBuffer[0], pInputBuffer[sizeToEnd]);
                }
                gtTso.writeIndex += bufferSize;
                if (gtTso.writeIndex >= gtTso.bufferSize)
                {
                    gtTso.writeIndex = bufferSize - sizeToEnd;
                }
                HOST_WriteRegister(TSO_WRITE_LEN_REG1, (MMP_UINT16) (bufferSize & 0xFFFF));
                HOST_WriteRegister(TSO_WRITE_LEN_REG2, (MMP_UINT16) ((bufferSize >> 16) & 0x3F));
                HOST_WriteRegister(TSO_WRITE_CTRL_REG, 0x1);
                break;
            }
        } while (1);
    }
exit:
    PalReleaseMutex(gtTso.tMgrMutex);
    return result;
}

MMP_RESULT
mmpTsoWriteWithoutCopy(
    MMP_ULONG   bufferSize)
{
    MMP_INT32  sizeToEnd = 0;
    MMP_RESULT result = MMP_SUCCESS;    
    PalWaitMutex(gtTso.tMgrMutex, PAL_MUTEX_INFINITE);
    sizeToEnd = gtTso.bufferSize - gtTso.writeIndex;    
    gtTso.writeIndex += bufferSize;
    if (gtTso.writeIndex >= gtTso.bufferSize)
    {
        gtTso.writeIndex = bufferSize - sizeToEnd;
    }

    HOST_WriteRegister(TSO_WRITE_LEN_REG1, (MMP_UINT16) (bufferSize & 0xFFFF));
    HOST_WriteRegister(TSO_WRITE_LEN_REG2, (MMP_UINT16) ((bufferSize >> 16) & 0x3F));
    
    HOST_WriteRegister(TSO_WRITE_CTRL_REG, 0x1);
    PalReleaseMutex(gtTso.tMgrMutex);
    return result;    
}

TSO_API MMP_UINT32
mmpTsoGetStatus(
    void)
{
    MMP_UINT16 regVal = 0;
    HOST_ReadRegister(TSO_ENG_STATUS_REG, &regVal);
    return regVal;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

void
_TSO_SetPadSel(
	MMP_UINT32 startPort)
{
    MMP_UINT32  data = 0;

    if (13 == startPort)
    {
        // GPIO 13, 14, 15, 16 TSO
		printf("set gpio 13\n");
        AHB_ReadRegister(GPIO_BASE + 0x90,&data);
        data |= (0x1 << 26) | (0x1 << 28) | (0x1 << 30);
        AHB_WriteRegister(GPIO_BASE + 0x90,data);
        data = 0;
        
        AHB_ReadRegister(GPIO_BASE + 0x94,&data);
        data |= (0x1 << 0);
        AHB_WriteRegister(GPIO_BASE + 0x94,data);
    }
    else // GPIO 34, 35, 36, 37
    {
		printf("set gpio 34\n");
		// GPIO 34, 35, 36, 37 TSO
		AHB_ReadRegister(GPIO_BASE + 0x98,&data);
		data |= (0x1 << 4) | (0x1 << 6) | (0x1 << 8) | (0x1 << 10);
		AHB_WriteRegister(GPIO_BASE + 0x98, data);
    }
}
