/*
 * Copyright (c) 2010 ITE technology Corp. All Rights Reserved.
 */
/** @file tsi.c
 * Used to receive data through the transport stream interface (TSI).
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
#include "mem/mem.h"
#include "mmp_tsi.h"

#if (!defined(WIN32)) && defined(ENABLE_INTR)
    #define TSI_IRQ_ENABLE
#endif

#if defined(__OR32__)
#include "or32.h"
#define ithInvalidateDCacheRange    or32_invalidate_cache
#endif

#if defined(TSI_IRQ_ENABLE)
#   if defined(__OPENRTOS__)
#       include "ite/ith.h"
#   else
#       include "intr/intr.h"
#   endif
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//Define Register
#define REG_TSP_0_BASE                      (0x1000)    /* Base Register Address */
#define REG_TSP_1_BASE                      (0x1080)    /* Base Register Address */
#define REG_TSI_MEMORY_BASE_L_OFFSET        (0x00)
#define REG_TSI_MEMORY_BASE_H_OFFSET        (0x02)
#define REG_TSI_MEMORY_SIZE_L_OFFSET        (0x04)
#define REG_TSI_MEMORY_SIZE_H_OFFSET        (0x06)
#define REG_TSI_READ_POINTER_L_OFFSET       (0x08)
#define REG_TSI_READ_POINTER_H_OFFSET       (0x0A)
#define REG_TSI_SETTING_OFFSET              (0x0C)
#define REG_TSI_SET_PCR_PID_FILTER_OFFSET   (0x0E)
#define REG_TSI_PCR_CNT_THRESHOLD_OFFSET    (0x10)
#define REG_TSI_WRITE_POINTER_L_OFFSET      (0x12)
#define REG_TSI_WRITE_POINTER_H_OFFSET      (0x14)
#define REG_TSI_GET_PCR_PID_OFFSET          (0x16)
#define REG_TSI_PCR_L_OFFSET                (0x18)
#define REG_TSI_PCR_M_OFFSET                (0x1A)
#define REG_TSI_STATUS_OFFSET               (0x1E)
#define REG_TSI_SYNC_BYTE_AF_LENGTH_OFFSET  (0x22)
#define REG_TSI_PCR_CNT_L_OFFSET            (0x24)
#define REG_TSI_PCR_CNT_H_OFFSET            (0x26)
#define REG_TSI_FREQUENCY_RATIO_OFFSET      (0x28)
#define REG_TSI_IO_SRC_AND_FALL_SAMP_OFFSET (0x2A)
#define REG_TSI_PID_FILTER_BASE_OFFSET      (0x2C)

/* Default buffer size definition */
#ifdef WIN32
#define TSI_MEMORY_SIZE                     (0xFFEE0)
#else
// DVB-T bit rate limit is 31,668,449 bit/s
// call mmpTsiReceive() every 20 ms (refer to ts_stream_reader.c)
// 31,668,449 / 50 * 2 = 1266737.96 = 155k byte
// MUST BE the 16kB*N (N is integer) for MM9910
#define TSI_MEMORY_SIZE                     (327680)	
#endif

#define MAX_PID_NUMBER                      (8192)

//=============================================================================
//                              Macro Definition
//=============================================================================
#define REG(tsiId, offset)  ((MMP_UINT16)(gtTsi[(tsiId)].regBaseAddr + (offset)))

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef enum TSI_FLAG_TAG
{
    TSI_ZERO        = 0,
    TSI_ENABLED     = 1
} TSI_FLAG;

typedef struct TSI_MODULE_TAG
{
    MMP_MUTEX   tMgrMutex;
    MMP_UINT32  flag;
    MMP_UINT32  refCount;
    MMP_UINT32  baseAddr;
    MMP_UINT32  size;
    MMP_UINT32  readPtr;
    MMP_UINT16  regBaseAddr;

    MMP_UINT16  totalPmtCount;
    MMP_UINT16  filterTable[TSI_TOTAL_FILTER_COUNT];
} TSI_MODULE, *MMP_TSI_HANDLE;

//=============================================================================
//                              Global Data Definition
//=============================================================================

static TSI_MODULE  gtTsi[MAX_TSI_COUNT] = { 0 };
static MMP_UINT32  lastWritePtr         = 0;

#if defined(TSI_IRQ_ENABLE)
static MMP_UINT16  g_IntrStatus = 0;
static MMP_BOOL    g_IsTsi0Intr = 0;
static MMP_BOOL    g_IsTsi1Intr = 0;
static MMP_UINT16  g_TsiPID     = 0;
static MMP_UINT32  g_TsiPCR     = 0;
static MMP_UINT32  g_TsiPcrCnt  = 0;
#endif

//=============================================================================
//                              Private Function Declaration
//=============================================================================
//static MMP_INLINE MMP_RESULT
static MMP_RESULT
_TSI_Initialize(
    MMP_UINT32 tsiId,
    MMP_UINT32 bufBaseAddr,
    MMP_UINT32 bufSize);

//static MMP_INLINE void
static void
_TSI_Terminate(
    MMP_UINT32 tsiId);

//static MMP_INLINE void
static void
_TSI_Enable(
    MMP_UINT32 tsiId);

//static MMP_INLINE void
static void
_TSI_Disable(
    MMP_UINT32 tsiId);

//static MMP_INLINE MMP_UINT32
static MMP_UINT32
_TSI_GetWritePtr(
    MMP_UINT32 tsiId);

//static MMP_INLINE void
static void
_TSI_SetReadPtr(
    MMP_UINT32 tsiId,
    MMP_UINT32 readPointerOffset);

static MMP_RESULT
_TSI_WaitEngineIdle(
    MMP_UINT32 tsiId);

//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_RESULT
mmpTsiInitialize(
    MMP_UINT32 tsiId)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT32 realaddr;

    tsiId = !!tsiId;

    if (1 == tsiId)
    {
#if 0
        HOST_WriteRegisterMask(0x100, 1 << 13, 1 << 13);
        HOST_WriteRegisterMask(0x102, 1 << 13, 1 << 13);
        HOST_WriteRegisterMask(0x102, 1 << 11, 1 << 11);
        HOST_WriteRegisterMask(0x102, 1 << 9,  1 << 9);
        HOST_WriteRegisterMask(0x104, 1 << 9,  1 << 9);
#else
        HOST_WriteRegisterMask(0x108, 1 << 13, 1 << 13);
        HOST_WriteRegisterMask(0x108, 1 << 11, 1 << 11);
        HOST_WriteRegisterMask(0x10a, 1 << 13, 1 << 13);
        HOST_WriteRegisterMask(0x10a, 1 << 11, 1 << 11);
        HOST_WriteRegisterMask(0x10a, 1 << 9,  1 << 9);
#endif
    }

    if (gtTsi[tsiId].refCount++ == 0)
    {
        if (MMP_NULL == gtTsi[tsiId].tMgrMutex)
            gtTsi[tsiId].tMgrMutex = PalCreateMutex(MMP_NULL);
        if (!gtTsi[tsiId].tMgrMutex)
        {
            result = MMP_TSI_OUT_OF_MEM;
            goto error;
        }

#ifdef WIN32
        gtTsi[tsiId].baseAddr  = (MMP_UINT32)MEM_Allocate(TSI_MEMORY_SIZE,
                                                           MEM_USER_TSI);
#else	
		#if defined(MM9910)
        realaddr  = (MMP_UINT32)malloc(TSI_MEMORY_SIZE + 0x4000);  //16kB alignment
        
        if ( realaddr & 0x3FFF)
            gtTsi[tsiId].baseAddr = (realaddr + 0x4000) & 0xFFFFC000;
        else
            gtTsi[tsiId].baseAddr = (realaddr);        
        #else
        gtTsi[tsiId].baseAddr  = (MMP_UINT32)malloc(TSI_MEMORY_SIZE);        
        #endif

        //gtTsi[tsiId].size = TSI_MEMORY_SIZE - (gtTsi[tsiId].baseAddr - realaddr);
        //printf("addr1: %x, %x\n", realaddr, (gtTsi[tsiId].baseAddr - realaddr));
#endif
        if (!gtTsi[tsiId].baseAddr)
        {
            result = MMP_TSI_OUT_OF_MEM;
            goto error;
        }
#ifdef WIN32
        gtTsi[tsiId].baseAddr -= (MMP_UINT32)HOST_GetVramBaseAddress();
        gtTsi[tsiId].size      = TSI_MEMORY_SIZE;
#else
        gtTsi[tsiId].size      = TSI_MEMORY_SIZE;
        //printf("size2: %d\n", gtTsi[tsiId].size);
#endif

        gtTsi[tsiId].regBaseAddr = tsiId ? REG_TSP_1_BASE : REG_TSP_0_BASE;
        lastWritePtr = gtTsi[tsiId].size;
        result = _TSI_Initialize(tsiId, gtTsi[tsiId].baseAddr, gtTsi[tsiId].size);
        if (MMP_SUCCESS != result)
            goto error;
    }
    return result;

error:
#ifdef WIN32
    if (gtTsi[tsiId].baseAddr)
        MEM_Release((void*)((MMP_UINT32)HOST_GetVramBaseAddress() + gtTsi[tsiId].baseAddr));
#endif
    if (gtTsi[tsiId].tMgrMutex)
    {
        PalDestroyMutex(gtTsi[tsiId].tMgrMutex);
    }
    PalMemset(&gtTsi[tsiId], 0x0, sizeof(gtTsi));
    return result;
}

MMP_RESULT
mmpTsiTerminate(
    MMP_UINT32 tsiId)
{
    tsiId = !!tsiId;

    if (gtTsi[tsiId].refCount > 0 && (--gtTsi[tsiId].refCount == 0))
    {
        mmpTsiDisable(tsiId);
#ifdef WIN32
        MEM_Release((void*)((MMP_UINT32)HOST_GetVramBaseAddress() + gtTsi[tsiId].baseAddr));
#endif

        PalDestroyMutex(gtTsi[tsiId].tMgrMutex);
        PalMemset(&gtTsi[tsiId], 0, sizeof(TSI_MODULE));
    }
    return MMP_SUCCESS;
}

MMP_RESULT
mmpTsiEnable(
    MMP_UINT32 tsiId)
{
    MMP_RESULT result = MMP_SUCCESS;

    tsiId = !!tsiId;

    PalWaitMutex(gtTsi[tsiId].tMgrMutex, PAL_MUTEX_INFINITE);

    if (gtTsi[tsiId].refCount > 0 && !(gtTsi[tsiId].flag & TSI_ENABLED))
    {
        _TSI_Enable(tsiId);
    }
    gtTsi[tsiId].readPtr = gtTsi[tsiId].size;
    while (gtTsi[tsiId].readPtr >= gtTsi[tsiId].size)
        gtTsi[tsiId].readPtr = _TSI_GetWritePtr(tsiId);

    gtTsi[tsiId].flag |= TSI_ENABLED;

    PalReleaseMutex(gtTsi[tsiId].tMgrMutex);
    return result;
}

// Caution! Disable TSI acts as Terminate TSI, all TSI's reg will be reset.
// DON'T use it.
MMP_RESULT
mmpTsiDisable(
    MMP_UINT32 tsiId)
{
    tsiId = !!tsiId;

    PalWaitMutex(gtTsi[tsiId].tMgrMutex, PAL_MUTEX_INFINITE);

    if (gtTsi[tsiId].refCount > 0 && (gtTsi[tsiId].flag & TSI_ENABLED))
    {
        _TSI_Disable(tsiId);
        gtTsi[tsiId].flag &= ~TSI_ENABLED;
    }
    PalReleaseMutex(gtTsi[tsiId].tMgrMutex);

    return MMP_SUCCESS;
}

#ifdef WIN32
MMP_RESULT
mmpTsiReceive(
    MMP_UINT32  tsiId,
    void*       buffer,
    MMP_ULONG   maxSize,
    MMP_ULONG*  actualSize)
{
    MMP_RESULT result = MMP_SUCCESS;

    tsiId = !!tsiId;

    PalWaitMutex(gtTsi[tsiId].tMgrMutex, PAL_MUTEX_INFINITE);

    if (0 == gtTsi[tsiId].refCount)
    {
        result = MMP_TSP_IS_NONINITED;
        goto exit;
    }

    if (buffer && actualSize && maxSize > 0)
    {
        if (gtTsi[tsiId].flag & TSI_ENABLED)
        {
            MMP_UINT32 tsiWritePtr = _TSI_GetWritePtr(tsiId);
            MMP_UINT32 readSize = 0;

            if (gtTsi[tsiId].readPtr < tsiWritePtr)
                readSize = tsiWritePtr - gtTsi[tsiId].readPtr;
            else if (tsiWritePtr < gtTsi[tsiId].readPtr)
                readSize = gtTsi[tsiId].size - gtTsi[tsiId].readPtr;
            else
            {
                (*actualSize) = readSize;
                goto exit;
            }

            if (readSize > maxSize)
                readSize = maxSize;

            // Copy the TS data from VRAM to system RAM
            HOST_ReadOffsetBlockMemory(
                (MMP_UINT32)buffer,
                gtTsi[tsiId].baseAddr + gtTsi[tsiId].readPtr,
                readSize);

            gtTsi[tsiId].readPtr += readSize;
            if (gtTsi[tsiId].readPtr >= gtTsi[tsiId].size)
                gtTsi[tsiId].readPtr = 0;

            _TSI_SetReadPtr(tsiId, gtTsi[tsiId].readPtr);

            (*actualSize) = readSize;
            goto exit;
        }
        else
        {
            result = MMP_TSI_IS_DISABLED;
        }
    }
    else
    {
        result = MMP_TSI_BAD_PARAM;
    }

exit:
    PalReleaseMutex(gtTsi[tsiId].tMgrMutex);
    return result;
}
#else
MMP_RESULT
mmpTsiReceive(
    MMP_UINT32  tsiId,
    MMP_UINT8** ppOutBuffer,
    MMP_ULONG*  outSize)
{
    MMP_RESULT result = MMP_SUCCESS;

    tsiId = !!tsiId;

    PalWaitMutex(gtTsi[tsiId].tMgrMutex, PAL_MUTEX_INFINITE);

    if (0 == gtTsi[tsiId].refCount)
    {
        result = MMP_TSP_IS_NONINITED;
        goto exit;
    }

    if (ppOutBuffer && outSize)
    {
        if (gtTsi[tsiId].flag & TSI_ENABLED)
        {
            MMP_UINT32 tsiWritePtr;
            MMP_UINT32 readSize = 0;
            tsiWritePtr = _TSI_GetWritePtr(tsiId);

            // workaround: avoid to read invalid writePtr
            if (tsiWritePtr < lastWritePtr)
            {
                tsiWritePtr = _TSI_GetWritePtr(tsiId);
                if (tsiWritePtr < lastWritePtr)
                {
                    tsiWritePtr = _TSI_GetWritePtr(tsiId);
                }
            }
            lastWritePtr = tsiWritePtr;

            if (tsiWritePtr < gtTsi[tsiId].size
             && gtTsi[tsiId].readPtr < tsiWritePtr)
            {
                readSize = tsiWritePtr - gtTsi[tsiId].readPtr;
             
				//printf("1read size= %d\n", readSize);
                // workaround: maybe tsi still keep data in hw buffer
                // and doesn't write to memory yet
                // so we just ignore the latest data to avoid reading wrong data
                //if (readSize > 32)
                //    readSize -= 32;
                //else
                //    readSize = 0;
            }
            else if (tsiWritePtr < gtTsi[tsiId].readPtr)
            {
                readSize = gtTsi[tsiId].size - gtTsi[tsiId].readPtr;

				//printf("2read size= %d\n", readSize);
                // workaround: maybe tsi still keep data in hw buffer
                // and doesn't write to memory yet
                // so we just ignore the latest data to avoid reading wrong data
                //if (tsiWritePtr < 32)
                //{
                //    if (readSize + tsiWritePtr > 32)
                //        readSize -= (32-tsiWritePtr);
                //    else
                //        readSize = 0;
                //}
            }
            else
            {
				//printf("read: %d\n", readSize);
                (*outSize) = readSize;
                goto exit;
            }

            if (readSize > gtTsi[tsiId].size/2)
                dbg_msg(DBG_MSG_TYPE_ERROR, "tsi.c: consume data irregularly\n");

            (*ppOutBuffer) = (MMP_UINT8*) (gtTsi[tsiId].baseAddr + gtTsi[tsiId].readPtr);

            gtTsi[tsiId].readPtr += readSize;
            if (gtTsi[tsiId].readPtr >= gtTsi[tsiId].size)
                gtTsi[tsiId].readPtr = 0;

            _TSI_SetReadPtr(tsiId, gtTsi[tsiId].readPtr);

            (*outSize) = readSize;
            goto exit;
        }
        else
        {
            result = MMP_TSI_IS_DISABLED;
        }
    }
    else
    {
        result = MMP_TSI_BAD_PARAM;
    }

exit:
    ithInvalidateDCacheRange((*ppOutBuffer), (*outSize));   //flush cache?
    PalReleaseMutex(gtTsi[tsiId].tMgrMutex);
    return result;
}
#endif

MMP_BOOL
mmpTsiIsPcrInterruptTriggered(
    MMP_UINT32 tsiId)
{
#if defined(TSI_IRQ_ENABLE)
    if (tsiId == 0)
        return g_IsTsi0Intr;
    else
        return g_IsTsi1Intr;
#else
    MMP_UINT16 interruptBit;
    MMP_UINT16 tsiStatus;
    MMP_UINT16 value;

    tsiId = !!tsiId;

    interruptBit = 0x0020 << tsiId;

    HOST_ReadRegister(MMP_GENERAL_INTERRUPT_REG_0C, &value);
    HOST_ReadRegister(REG(tsiId, REG_TSI_STATUS_OFFSET), &tsiStatus);

    //Check TSI interrupt
    if (((value     & (interruptBit)) == (interruptBit))
     && ((tsiStatus & (0x8))          == 0x8))
        return MMP_TRUE;
    else
        return MMP_FALSE;
#endif
}

MMP_UINT32
mmpTsiGetPcrValue(
    MMP_UINT32 tsiId)
{
#if defined(TSI_IRQ_ENABLE)
    return g_TsiPCR;
#else
    MMP_UINT16 PCR_L;
    MMP_UINT16 PCR_M;

    tsiId = !!tsiId;

    HOST_ReadRegister(REG(tsiId, REG_TSI_PCR_L_OFFSET), &PCR_L);
    HOST_ReadRegister(REG(tsiId, REG_TSI_PCR_M_OFFSET), &PCR_M);

    return (((MMP_UINT32)PCR_M << 16) + PCR_L);
#endif
}

//////////////////////////////////////////////////////////////////////////
// Must read PCR_L register before read this PID register
//////////////////////////////////////////////////////////////////////////
MMP_UINT16
mmpTsiGetPcrPid(
    MMP_UINT32 tsiId)
{
#if defined(TSI_IRQ_ENABLE)
    //printf("mmp-get-PID=[%x]\n",g_TsiPID);
    return g_TsiPID;    //test
#else
    MMP_UINT16 pcrPid;

    tsiId = !!tsiId;

    HOST_ReadRegister(REG(tsiId, REG_TSI_GET_PCR_PID_OFFSET), &pcrPid);

    return pcrPid;
#endif
}

//////////////////////////////////////////////////////////////////////////
// Must read PCR_L register before read this PCRCnt register
//////////////////////////////////////////////////////////////////////////
MMP_UINT32
mmpTsiGetPCRCnt(
    MMP_UINT32 tsiId)
{
#if defined(TSI_IRQ_ENABLE)
    //printf("mmp-get-PcrCnt=[%x]\n",g_TsiPcrCnt);
    return g_TsiPcrCnt;
#else
    MMP_UINT16 PCRCnt_L;
    MMP_UINT16 PCRCnt_H;
    MMP_UINT32 PCRCnt = 0xFFFFFFFF;

    tsiId = !!tsiId;

    HOST_ReadRegister(REG(tsiId, REG_TSI_PCR_CNT_L_OFFSET), &PCRCnt_L);
    HOST_ReadRegister(REG(tsiId, REG_TSI_PCR_CNT_H_OFFSET), &PCRCnt_H);

    //Check PCRCnt overflow
    if ((PCRCnt_H & (1 << 10)) == 0)
    {
        PCRCnt = (((PCRCnt_H & 0x3FF) << 16) + PCRCnt_L);
    }
    //else
    //    dbg_msg(DBG_MSG_TYPE_ERROR, "PCR Cnt Overflow\n");

    return PCRCnt;
#endif
}

//////////////////////////////////////////////////////////////////////////
// If has interrupt, must be clear interrupt
//////////////////////////////////////////////////////////////////////////
void
mmpTsiClearInterrupt(
    MMP_UINT32 tsiId)
{
#if defined(TSI_IRQ_ENABLE)
    if (tsiId == 0)
        g_IsTsi0Intr = MMP_FALSE;
    else
        g_IsTsi1Intr = MMP_FALSE;
#else
    MMP_UINT16 bit = (0x1 << 5) << tsiId;

    HOST_WriteRegisterMask(MMP_GENERAL_INTERRUPT_REG_0A, bit, bit);
#endif
}

MMP_RESULT
mmpTsiEnablePcrPidFilter(
    MMP_UINT32 tsiId,
    MMP_UINT16 pid)
{
    tsiId = !!tsiId;

    if (0 == gtTsi[tsiId].refCount)
        return MMP_TSP_IS_NONINITED;

    if (pid >= MAX_PID_NUMBER)
        return MMP_TSI_BAD_PARAM;

    HOST_WriteRegister(REG(tsiId, REG_TSI_SET_PCR_PID_FILTER_OFFSET), (pid << 1) | 0x1);

    return MMP_SUCCESS;
}

void
mmpTsiDisablePcrPidFilter(
    MMP_UINT32 tsiId)
{
    tsiId = !!tsiId;

    if (0 == gtTsi[tsiId].refCount)
        return;

    HOST_WriteRegister(REG(tsiId, REG_TSI_SET_PCR_PID_FILTER_OFFSET), 0);
}

// had better convert the counter to time duration
void
mmpTsiEnablePCRCntThreshold(
    MMP_UINT32 tsiId,
    MMP_UINT16 threshold)
{
    tsiId = !!tsiId;

    if (0 == gtTsi[tsiId].refCount)
        return;

    HOST_WriteRegister(REG(tsiId, REG_TSI_PCR_CNT_THRESHOLD_OFFSET), ((threshold & 0x3FF) << 1) | 0x1);
}

void
mmpTsiDisablePCRCntThreshold(
    MMP_UINT32 tsiId)
{
    tsiId = !!tsiId;

    if (0 == gtTsi[tsiId].refCount)
        return;

    HOST_WriteRegister(REG(tsiId, REG_TSI_PCR_CNT_THRESHOLD_OFFSET), 0);
}

//=============================================================================
/**
 * Update PID filter.
 *
 * @param tsiId The specific tsi.
 * @param pid   filter PID value.
 * @param type  The specific PID entry index.
 * @return none
 */
//=============================================================================
MMP_RESULT
mmpTsiUpdatePidFilter(
    MMP_UINT32              tsiId,
    MMP_UINT32              pid,
    TSI_PID_FILTER_INDEX    pidIndex)
{
    MMP_BOOL    bInsert = MMP_TRUE;
    MMP_BOOL    bUpdate = MMP_TRUE;
    MMP_UINT32  i = pidIndex;

    tsiId = !!tsiId;

    if (0 == gtTsi[tsiId].refCount)
        return MMP_TSP_IS_NONINITED;

    if (pid >= MAX_PID_NUMBER || pidIndex >= TSI_TOTAL_FILTER_COUNT)
        return MMP_TSI_BAD_PARAM;

    // Only the PMT table owns more than one index.
    if (TSI_PID_FILTER_PMT_INDEX == pidIndex)
    {
        for (i; i < ((MMP_UINT32)TSI_PID_FILTER_PMT_INDEX + gtTsi[tsiId].totalPmtCount); ++i)
        {
            if (pid == gtTsi[tsiId].filterTable[i])
            {
                bInsert = MMP_FALSE;
                break;
            }
        }
        if (bInsert)
        {
            if (gtTsi[tsiId].totalPmtCount >= TSI_MAX_PMT_FILTER_COUNT)
            {
                dbg_msg(DBG_MSG_TYPE_ERROR, "no more available PMT pid filter\n");
                bUpdate = MMP_FALSE;
            }
            else
                gtTsi[tsiId].totalPmtCount++;
        }
    }

    // If the pid is added to table already, ignore it.
    if (pid != gtTsi[tsiId].filterTable[i] && bUpdate)
    {
        HOST_WriteRegister(REG(tsiId, REG_TSI_PID_FILTER_BASE_OFFSET) + (MMP_UINT16)(i << 1),
            (MMP_UINT16)(pid << 1) | 0x1);
    }
    gtTsi[tsiId].filterTable[i] = (MMP_UINT16)pid;

    return MMP_SUCCESS;
}

//=============================================================================
/**
 * Turn on/off the hardware filter
 *
 * @return none
 */
//=============================================================================
void
mmpTsiResetPidFilter(
    MMP_UINT32  tsiId)
{
    MMP_UINT i;

    tsiId = !!tsiId;

    if (0 == gtTsi[tsiId].refCount)
        return;

    for (i = 0; i < 32; ++i)
    {
        HOST_WriteRegister(REG(tsiId, REG_TSI_PID_FILTER_BASE_OFFSET) + (i << 1), 0);
    }

    PalMemset(gtTsi[tsiId].filterTable, 0xFF, sizeof(gtTsi[tsiId].filterTable));
    gtTsi[tsiId].totalPmtCount = 0;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
#if defined(TSI_IRQ_ENABLE)
void tsi0_isr(void* data)
{
    MMP_UINT16  tmp=0;
    MMP_UINT16  tmpPCR1;
    MMP_UINT16  tmpPCR2;
    MMP_UINT16  tmpPID;
    MMP_UINT16  PCRCnt_L;
    MMP_UINT16  PCRCnt_H;
    MMP_UINT32 TmpWtPtr=0;
    //printf("tsi0.isr.in\n");//ITH_INTR_TSI0

    HOST_ReadRegister(REG(0, REG_TSI_STATUS_OFFSET), &tmp);

    if(tmp&0x10)
    {
        if(tmp&0x08)
        {
            //printf("int PCR\n");
            g_IntrStatus|=0x01;

            HOST_ReadRegister( REG(0, REG_TSI_PCR_L_OFFSET), &tmpPCR1); //have to read PCR1 first before check PID & PCR2 & PCR3
            HOST_ReadRegister( REG(0, REG_TSI_PCR_M_OFFSET), &tmpPCR2);
            g_TsiPCR = ((MMP_UINT32)tmpPCR2<<16) + tmpPCR1;   //merge PCR value

            HOST_ReadRegister( REG(0, REG_TSI_GET_PCR_PID_OFFSET),&tmpPID);
            g_TsiPID = tmpPID&0x1FFF; //Mask the PID value

            HOST_ReadRegister(REG(0, REG_TSI_PCR_CNT_L_OFFSET), &PCRCnt_L);
            HOST_ReadRegister(REG(0, REG_TSI_PCR_CNT_H_OFFSET), &PCRCnt_H);
            g_TsiPcrCnt = (((PCRCnt_H & 0x3FF) << 16) + PCRCnt_L);

            g_IsTsi0Intr = MMP_TRUE;
        }

        if(tmp&0x04)
        {
            g_IntrStatus|=0x02;
            TmpWtPtr = _TSI_GetWritePtr(0);
            //_TSI_SetReadPtr(0, TmpWtPtr); //for testing only
        }

        if(tmp&0x8000)
        {
            //printf("int Err non-188\n");
            g_IntrStatus|=0x02;
        }
        if(tmp&0x4000)
        {
            //printf("int Err fifo-full]\n");
            g_IntrStatus|=0x04;
        }
    }
    
    //SYS_SetEventFromIsr(Tsi0IsrEvent);
    
    //g_IsTsi0Intr = MMP_TRUE;	//Enable this code for using TSI ISR function
    
    HOST_WriteRegisterMask( REG(0, 0x6C), 0x0001, 0x0001);
    //printf("Nf.isr.out[%04x,%x,%x]\n",tmp,g_IsTsi0Intr,TmpWtPtr);
}

#if !defined(MM9910)
void tsi1_isr(void* data)
{
    MMP_UINT16  tmp=0;
    MMP_UINT16  tmpPCR1;
    MMP_UINT16  tmpPCR2;
    MMP_UINT16  tmpPID;
    MMP_UINT16  PCRCnt_L;
    MMP_UINT16  PCRCnt_H;
    MMP_UINT32 TmpWtPtr=0;

    //printf("tsi1.isr.in\n");

    HOST_ReadRegister(REG(1, REG_TSI_STATUS_OFFSET), &tmp);
    
    if(tmp&0x10)
    {
        if(tmp&0x08)
        {
            //printf("int PCR\n");
            g_IntrStatus|=0x01;

            HOST_ReadRegister( REG(1, REG_TSI_PCR_L_OFFSET), &tmpPCR1); //have to read PCR1 first before check PID & PCR2 & PCR3
            HOST_ReadRegister( REG(1, REG_TSI_PCR_M_OFFSET), &tmpPCR2);
            g_TsiPCR=((MMP_UINT32)tmpPCR2<<16) + tmpPCR1;   //merge PCR value

            HOST_ReadRegister( REG(1, REG_TSI_GET_PCR_PID_OFFSET),&tmpPID);
            g_TsiPID=tmpPID&0x1FFF; //Mask the PID value

            HOST_ReadRegister(REG(1, REG_TSI_PCR_CNT_L_OFFSET), &PCRCnt_L);
            HOST_ReadRegister(REG(1, REG_TSI_PCR_CNT_H_OFFSET), &PCRCnt_H);
            g_TsiPcrCnt = (((PCRCnt_H & 0x3FF) << 16) + PCRCnt_L);

            g_IsTsi1Intr = MMP_TRUE;
        }

        if(tmp&0x04)
        {
            g_IntrStatus|=0x02;
            TmpWtPtr = _TSI_GetWritePtr(1);
            //_TSI_SetReadPtr(0, TmpWtPtr); //for testing only
        }

        if(tmp&0x8000)
        {
            printf("int Err non-188\n");
            g_IntrStatus|=0x02;
        }
        if(tmp&0x4000)
        {
            printf("int Err fifo-full]\n");
            g_IntrStatus|=0x04;
        }
    }
    //SYS_SetEventFromIsr(Tsi0IsrEvent);
    //g_IsTsi1Intr = MMP_TRUE;
    HOST_WriteRegisterMask( REG(1, 0x6C), 0x0001, 0x0001);
    //printf("Tsi1.isr.out[%04x,%x]\n",tmp,g_IsTsi1Intr);
}
#endif

void Tsi0IntrEnable(void)
{
    // Initialize Timer IRQ
    printf("Enable Tsi0 IRQ~~\n");
    ithIntrDisableIrq(ITH_INTR_TSI0);
    ithIntrClearIrq(ITH_INTR_TSI0);

    #if defined (__FREERTOS__)
    // register NAND Handler to IRQ
    printf("register TSI0 Handler to IRQ!!\n");
    ithIntrRegisterHandlerIrq(ITH_INTR_TSI0, tsi0_isr, MMP_NULL);
    #endif // defined (__FREERTOS__)

    // set IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_TSI0, ITH_INTR_EDGE);

    // set IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_TSI0, ITH_INTR_HIGH_RISING);

    // Enable IRQ
    ithIntrEnableIrq(ITH_INTR_TSI0);

    //clear tsi intr in tsi controller
    //HOST_WriteRegister(REG(0, REG_TSI_STATUS_OFFSET), 0x0000); /** enable tsi interrupt **/

    //if(!Tsi0IsrEvent) {
        //Tsi0IsrEvent = SYS_CreateEvent();}

    //printf("Tsi0IsrEvent=%x\n",Tsi0IsrEvent);

    printf("Enable Tsi0 IRQ$$\n");
}

void Tsi0IntrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_TSI0);
}

#if !defined(MM9910)
void Tsi1IntrEnable(void)
{
    // Initialize Timer IRQ
    printf("Enable Tsi1 IRQ~~\n");
    ithIntrDisableIrq(ITH_INTR_TSI1);
    ithIntrClearIrq(ITH_INTR_TSI1);

    #if defined (__FREERTOS__)
    // register NAND Handler to IRQ
    printf("register NAND Handler to IRQ!!\n");
    ithIntrRegisterHandlerIrq(ITH_INTR_TSI1, tsi1_isr, MMP_NULL);
    #endif // defined (__FREERTOS__)

    // set IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_TSI1, ITH_INTR_EDGE);

    // set IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_TSI1, ITH_INTR_HIGH_RISING);

    // Enable IRQ
    ithIntrEnableIrq(ITH_INTR_TSI1);

    printf("Enable Tsi1 IRQ$$\n");
}

void Tsi1IntrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_TSI1);
}
#endif
#endif

MMP_RESULT
_TSI_Initialize(
    MMP_UINT32 tsiId,
    MMP_UINT32 bufBaseAddr,
    MMP_UINT32 bufSize)
{
    if (bufBaseAddr && bufSize > 0)
    {
        MMP_RESULT result;

        if (MMP_SUCCESS == (result = _TSI_WaitEngineIdle(tsiId)))
        {
            // Init TSI register
            HOST_WriteRegister(REG(tsiId, REG_TSI_MEMORY_BASE_L_OFFSET),
                               (MMP_UINT16)(bufBaseAddr & 0xFFFF));
            MMP_Sleep(1);
            HOST_WriteRegister(REG(tsiId, REG_TSI_MEMORY_BASE_H_OFFSET),
                               (MMP_UINT16)((bufBaseAddr >> 16) & 0xFFFF));
            MMP_Sleep(1);

            HOST_WriteRegister(REG(tsiId, REG_TSI_MEMORY_SIZE_L_OFFSET),
                               (MMP_UINT16)(bufSize & 0xFFFF));
            MMP_Sleep(1);
            HOST_WriteRegister(REG(tsiId, REG_TSI_MEMORY_SIZE_H_OFFSET),
                               (MMP_UINT16)((bufSize >> 16) & 0xFFFF));
            MMP_Sleep(1);

            HOST_WriteRegister(REG(tsiId, REG_TSI_READ_POINTER_L_OFFSET),
                               0x0000);
            MMP_Sleep(1);
            HOST_WriteRegister(REG(tsiId, REG_TSI_READ_POINTER_H_OFFSET),
                               0x0000);
            MMP_Sleep(1);

            //Adaptation_field_length > 6
            HOST_WriteRegister(REG(tsiId, REG_TSI_SYNC_BYTE_AF_LENGTH_OFFSET),
                               0x4706);
            MMP_Sleep(1);

            //Enable TSI and FPC PCLK
            HOST_PCR_EnableClock();

            #if defined(TSI_IRQ_ENABLE)
            //enable intr & irq
            #if defined(MM9910)
            Tsi0IntrEnable();
            #else
            if(tsiId)
            {
                Tsi1IntrEnable();
            }
            else
            {
                Tsi0IntrEnable();
            }
            #endif
            #endif
            
            return MMP_SUCCESS;
        }
        return result;
    }
    return MMP_TSI_BAD_PARAM;
}

void
_TSI_Terminate(
    MMP_UINT32 tsiId)
{
}

void
_TSI_Enable(
    MMP_UINT32 tsiId)
{
    //TSI Interrupt Enable
    MMP_UINT16 interruptBit = (0x20 << tsiId);

    HOST_WriteRegisterMask(MMP_GENERAL_INTERRUPT_REG_06, interruptBit, interruptBit);

    HOST_WriteRegister(REG(tsiId, REG_TSI_FREQUENCY_RATIO_OFFSET), (27 << 2)); //27MHz

    //if (tsiId == 1)
    {
        //Rising sample and 2T
        HOST_WriteRegister(REG(tsiId, REG_TSI_IO_SRC_AND_FALL_SAMP_OFFSET), 0x02A8);
    }

    // enable TSI, others is default

    if (tsiId == 0)
    {
        HOST_WriteRegister(REG(tsiId, REG_TSI_SETTING_OFFSET), 0x402F);
    }
    else
    {
        HOST_WriteRegister(REG(tsiId, REG_TSI_SETTING_OFFSET), 0x400F);
    }
}

void
_TSI_Disable(
    MMP_UINT32 tsiId)
{
    HOST_WriteRegisterMask(REG(tsiId, REG_TSI_SETTING_OFFSET), 0x0000, 0x0001);

    if (MMP_SUCCESS == _TSI_WaitEngineIdle(tsiId))
    {
        MMP_UINT16 bit = (0x1 << 12) << tsiId;

        HOST_WriteRegisterMask(MMP_TSI_CLOCK_REG_48, bit, bit);
        MMP_Sleep(1);
        HOST_WriteRegisterMask(MMP_TSI_CLOCK_REG_48,   0, bit);
    }
}

MMP_UINT32
_TSI_GetWritePtr(
    MMP_UINT32 tsiId)
{
    MMP_UINT16  writePtr_L, writePtr_H;

    // Get write pointer offset
    HOST_ReadRegister(REG(tsiId, REG_TSI_WRITE_POINTER_L_OFFSET), &writePtr_L);
    HOST_ReadRegister(REG(tsiId, REG_TSI_WRITE_POINTER_H_OFFSET), &writePtr_H);

    return (((MMP_UINT32)writePtr_H << 16) + writePtr_L);
}

void
_TSI_SetReadPtr(
    MMP_UINT32 tsiId,
    MMP_UINT32 readPtr)
{
    HOST_WriteRegister(REG(tsiId, REG_TSI_READ_POINTER_L_OFFSET),
                       (MMP_UINT16)(readPtr & 0xFFFF));
    HOST_WriteRegister(REG(tsiId, REG_TSI_READ_POINTER_H_OFFSET),
                       (MMP_UINT16)((readPtr >> 16) & 0xFFFF));
}

/**
 * Wait TSI engine idle!
 */
MMP_RESULT
_TSI_WaitEngineIdle(
    MMP_UINT32 tsiId)
{
    volatile MMP_UINT16 status = 0;
    MMP_UINT16 timeOut = 0;

    //
    //  Wait TSI engine idle!   D[1]  1: idle, 0: busy
    //
    do
    {
        HOST_ReadRegister(REG(tsiId, REG_TSI_STATUS_OFFSET), (MMP_UINT16*)&status);
        if (status & 0x0002)
            break;

        if(++timeOut > 2000)
            return MMP_TSI_ENGINE_BUSY;

        PalSleep(1);
    } while(MMP_TRUE);

    return MMP_SUCCESS;
}
