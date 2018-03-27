/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file mem_mgr.c
 * A local memory manager
 * @author Steven Hsiao
 * @version 0.1
 */

#include "pal/pal.h"
#include <string.h>
#include <stdio.h>
#include "mem/mem.h"
#include "mem_mgr.h"
#include "mps_system.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MEM_MGR_RING_HANDLE_TAG
{
    MMP_UINT8*  pWritePtr;
    MMP_UINT32  remainSize;
    MMP_UINT8*  pRingBufferBase;
    MMP_UINT8*  pRingBufferEnd;
} MEM_MGR_RING_HANDLE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

// Ring Buffer Control/Data Handle
static MEM_MGR_RING_HANDLE* gtRingTbl[MEM_MGR_TOTAL_RING_COUNT] = { 0 };
static MMP_MUTEX            gtMgrMutex = MMP_NULL;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void
_MEMMGR_ReleaseRing(
    MMP_UINT32  ringId);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Init the memory partition pool manager
 * return   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *          fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_Init(
    void)
{
    if (MMP_NULL == gtMgrMutex)
        gtMgrMutex = PalCreateMutex(MMP_NULL);

    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_INIT_FAIL;

#ifdef WIN32
    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);
#endif

    PalMemset(gtRingTbl, 0x0, sizeof(gtRingTbl));

#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif

    return MEM_MGR_NO_ERROR;
}

//=============================================================================
/**
 * Terminate the memory partition pool manager
 * return   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *          fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_Terminate(
    void)
{
    MMP_UINT i;

    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

#ifdef WIN32
    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);
#endif

    for (i = 0; i < MEM_MGR_TOTAL_RING_COUNT; i++)
        if (gtRingTbl[i] != MMP_NULL)
            _MEMMGR_ReleaseRing(i);

    PalMemset(gtRingTbl, 0x0, sizeof(gtRingTbl));

#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif

    PalDestroyMutex(gtMgrMutex);
    gtMgrMutex = MMP_NULL;

    return MEM_MGR_NO_ERROR;
}

//=============================================================================
/**
 * Create a ring buffer
 *
 * @param bufferSize    The desired memory size of the ring buffer.
 * @param pOutRingId    The allocated ring buffer identifier if no error
 * return               MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                      fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_CreateRing(
    MMP_UINT32  bufferSize,
    MMP_UINT32* pOutRingId)
{
    MMP_UINT32 i;    
    MEM_MGR_ERROR_CODE result = MEM_MGR_NO_ERROR;
    MEM_MGR_RING_HANDLE* ptHandle = MMP_NULL;

    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

#ifdef WIN32
    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);
#endif

    // get a free ring table for handle
    for (i = 0; i < MEM_MGR_TOTAL_RING_COUNT; i++)
        if (MMP_NULL == gtRingTbl[i])
            break;

    if (MEM_MGR_TOTAL_RING_COUNT == i)
        result = MEM_MGR_NO_AVAILABLE_RING;

    ptHandle = (MEM_MGR_RING_HANDLE*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(MEM_MGR_RING_HANDLE));

    gtRingTbl[i] = ptHandle;
    ptHandle->remainSize = bufferSize;
    //if (bAirMode)
    //{
        ptHandle->pRingBufferBase = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                             bufferSize);
    //}
    //else
    //{
    //    ptHandle->pRingBufferBase = (MMP_UINT8*)MEM_Deploy(usage);
    //}
    ptHandle->pWritePtr = ptHandle->pRingBufferBase;
    ptHandle->pRingBufferEnd = ptHandle->pRingBufferBase + bufferSize;
    *pOutRingId = i;

#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif

    return result;
}

//=============================================================================
/**
 * Destroy a created memory partition pool
 *
 * @param ringId        The pool identifier of a specific memory partition pool.
 * return               MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                      fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_DestroyRing(
    MMP_UINT32  ringId)
{
    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

#ifdef WIN32
    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);
#endif

    _MEMMGR_ReleaseRing(ringId);

#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif
    return MEM_MGR_NO_ERROR;
}

//=============================================================================
/**
 * Allocate memory from a specific ring buffer
 *
 * @param ringId            The ring identifier of a specific memory ring buffer.
 * @param bufferSize        The request Buffer size.
 * @param ptOutAllocInfo    The pointer to allocated memory data and information.
 * return                   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                          fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_RingAlloc(
    MMP_UINT32                  ringId,
    MMP_UINT32                  bufferSize,
    MEM_MGR_RING_ALLOC_INFO*    ptOutAllocInfo)
{
    MMP_UINT32              sizeToEnd = 0;
    MEM_MGR_RING_HANDLE*    ptHandle = MMP_NULL;
    MEM_MGR_ERROR_CODE      result = MEM_MGR_NO_ERROR;

#ifdef WIN32
    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

    if (MMP_NULL == ptOutAllocInfo)
        return MEM_MGR_INVALID_INPUT;

    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);

    if (ringId >= MEM_MGR_TOTAL_RING_COUNT)
    {
        result = MEM_MGR_INVALID_INPUT;
        goto exit;
    }
#endif

    ptHandle = gtRingTbl[ringId];

    if (ptHandle->remainSize < bufferSize)
    {
        result = MEM_MGR_ALLOCATION_FAIL;
        goto exit;
    }

    sizeToEnd = ptHandle->pRingBufferEnd - ptHandle->pWritePtr;

    if (bufferSize >= sizeToEnd)
    {
        ptOutAllocInfo->pStartAddr = ptHandle->pWritePtr;
        ptOutAllocInfo->pEndAddr = ptHandle->pRingBufferBase + (bufferSize - sizeToEnd);
        ptOutAllocInfo->bWrap= MMP_TRUE;
    }
    else
    {
        ptOutAllocInfo->pStartAddr = ptHandle->pWritePtr;
        ptOutAllocInfo->pEndAddr = ptHandle->pWritePtr + bufferSize;
        ptOutAllocInfo->bWrap= MMP_FALSE;
    }

    ptOutAllocInfo->pRingStart = ptHandle->pRingBufferBase;
    ptOutAllocInfo->pRingEnd = ptHandle->pRingBufferEnd;
    ptHandle->remainSize -= bufferSize;
    ptHandle->pWritePtr = ptOutAllocInfo->pEndAddr;

exit:
#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif
    return result;
}

#if defined(ENABLE_MPLAYER) || defined(WIN32)
//=============================================================================
/**
 * Reduce the size of an allocated memory from ring buffer.
 *
 * @param ringId            The ring identifier of a specific memory ring buffer.
 * @param ptAllocInfo       The pointer to allocated memory data and information.
 * @param newSize           The new size
 * return                   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                          fail or success.
 *                      
 * NOTE: This is not a standard API and should only used in MPlayer.
 *       This function is not thread safe.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_ReduceSize(
    MMP_UINT32                  ringId,
    MMP_UINT32                  newSize,
    MEM_MGR_RING_ALLOC_INFO*    ptAllocInfo)
{
    MMP_UINT32              sizeToEnd = 0;
    MEM_MGR_RING_HANDLE*    ptHandle = MMP_NULL;
    MEM_MGR_ERROR_CODE      result = MEM_MGR_NO_ERROR;
    MMP_UINT8*              pOldEndAddr = MMP_NULL;
    MMP_UINT32              oldSize = 0;

    //dbg_msg(DBG_MSG_TYPE_INFO, "MemMgr_ReduceSize+\n");
#ifdef WIN32
    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

    if (MMP_NULL == ptAllocInfo)
        return MEM_MGR_INVALID_INPUT;

    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);

    if (ringId >= MEM_MGR_TOTAL_RING_COUNT)
    {
        result = MEM_MGR_INVALID_INPUT;
        goto exit;
    }
#endif

    oldSize = ptAllocInfo->bWrap
        ? ((ptAllocInfo->pRingEnd - ptAllocInfo->pStartAddr)
        + (ptAllocInfo->pEndAddr - ptAllocInfo->pRingStart))
        : (ptAllocInfo->pEndAddr - ptAllocInfo->pStartAddr);
    if (oldSize <= newSize)
    {
        result = MEM_MGR_RESIZE_FAIL;
        goto exit;
    }

    ptHandle = gtRingTbl[ringId];
    pOldEndAddr = ptAllocInfo->pEndAddr;

    sizeToEnd = ptAllocInfo->pRingEnd - ptAllocInfo->pStartAddr;
    if (newSize >= sizeToEnd)
    {
        ptAllocInfo->pEndAddr = ptAllocInfo->pRingStart +  (newSize - sizeToEnd);
        ptAllocInfo->bWrap = MMP_TRUE;
    }
    else
    {
        ptAllocInfo->pEndAddr = ptAllocInfo->pStartAddr + newSize;
        ptAllocInfo->bWrap = MMP_FALSE;
    }

    if (ptHandle->pWritePtr == pOldEndAddr)
    {
        ptHandle->remainSize += (oldSize - newSize);
        ptHandle->pWritePtr = ptAllocInfo->pEndAddr;
    }

exit:
#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif
    return result;
}
#endif

//=============================================================================
/**
 * Free allocated memory from ring buffer
 *
 * @param ringId        The ring identifier of a specific memory ring buffer.
 * @param pStartAddr    The pointer to the desired free start address of the
 *                      memory position of ring buffer.
 * @param pEndAddr      The pointer to the desired free end address of the
 *                      memory position of ring buffer.
 * @param bReverse      Whether the reverse free operation.
 * return               MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                      fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_RingFree(
    MMP_UINT32  ringId,
    MMP_UINT8*  pStartAddr,
    MMP_UINT8*  pEndAddr,
    MMP_BOOL    bReverse)
{
    MEM_MGR_RING_HANDLE*    ptHandle = MMP_NULL;
    MEM_MGR_ERROR_CODE      result = MEM_MGR_NO_ERROR;

#ifdef WIN32
    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);

    if (ringId >= MEM_MGR_TOTAL_RING_COUNT)
    {
        result = MEM_MGR_INVALID_INPUT;
        goto exit;
    }
#endif

    ptHandle = gtRingTbl[ringId];

#ifdef WIN32
    if (pStartAddr < ptHandle->pRingBufferBase
     || pStartAddr > ptHandle->pRingBufferEnd
     || pEndAddr < ptHandle->pRingBufferBase
     || pEndAddr > ptHandle->pRingBufferEnd
     || (pStartAddr < ptHandle->pWritePtr && ptHandle->pWritePtr < pEndAddr))
    {
        result = MEM_MGR_FREE_FAIL;
        goto exit;
    }
#endif

    // No wrapped around
    if (pStartAddr <= pEndAddr)
    {
        ptHandle->remainSize += (MMP_UINT32)(pEndAddr - pStartAddr);
    }
    else
    {
        ptHandle->remainSize += (MMP_UINT32)((ptHandle->pRingBufferEnd - ptHandle->pRingBufferBase) -
                                             (pStartAddr - pEndAddr));
    }

    if (bReverse)
        ptHandle->pWritePtr = pStartAddr;

exit:

#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif
    return result;
}

//=============================================================================
/**
 * Query information of ring buffer
 *
 * @param ringId        The ring identifier of a specific memory ring buffer.
 * @param remainSize    Remain size of ring buffer for allocation.
 * @param bufferSize    Total ring buffer size.
 * return               MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                      fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_RingQuery(
    MMP_UINT32  ringId, 
    MMP_UINT32* pRemainSize,
    MMP_UINT32* pBufferSize)
{
    *pRemainSize = gtRingTbl[ringId]->remainSize;
    *pBufferSize = gtRingTbl[ringId]->pRingBufferEnd - gtRingTbl[ringId]->pRingBufferBase;

    return MEM_MGR_NO_ERROR;
}

//=============================================================================
/**
 * Print out the buffer usuage information on the screen
 * return   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *          fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_ShowMemUsage(
    void)
{
    MMP_UINT32 i;

    if (MMP_NULL == gtMgrMutex)
        return MEM_MGR_NOT_INITED;

#ifdef WIN32
    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);
#endif

    dbg_msg(DBG_MSG_TYPE_ERROR, "Ring_ID  bufferSize  remainSize  writePointer\n");
    dbg_msg(DBG_MSG_TYPE_ERROR, "-------  ----------  ----------  ------------\n");

    for (i = 0; i < MEM_MGR_TOTAL_RING_COUNT; i++)
    {
        if (MMP_NULL == gtRingTbl[i])
            continue;

        dbg_msg(DBG_MSG_TYPE_ERROR, "%7d  %10d  %10d    0x%+08X\n",
                  i,
                  (gtRingTbl[i]->pRingBufferEnd - gtRingTbl[i]->pRingBufferBase),
                   gtRingTbl[i]->remainSize,
                   gtRingTbl[i]->pWritePtr);
    }
    dbg_msg(DBG_MSG_TYPE_ERROR, "\n\n");

#ifdef WIN32
    PalReleaseMutex(gtMgrMutex);
#endif
    return MEM_MGR_NO_ERROR;
}

//=============================================================================
/**
 * Get the buffer handle for debug usage.
 * Note that not change the content of the handle to prevent data corrupted or
 * unexpected result.
 * @param type      The type of desired pool.
 * @param id        The identifier of a specific memory pool handle.
 * @param pptHandle The output pointer to a specific type of handle pointer.
 * return           MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                  fail or success.
 */
//=============================================================================
//MEM_MGR_ERROR_CODE
//MemMgr_GetHandleInfo(
//    MEM_MGR_POOL_TYPE   type,
//    MMP_UINT32          id,
//    void**              pptHandle)
//{
//    MEM_MGR_ERROR_CODE  result = MEM_MGR_NO_ERROR;
//
//    if (MMP_NULL == gtMgrMutex)
//        return MEM_MGR_NOT_INITED;
//
//#ifdef WIN32
//    PalWaitMutex(gtMgrMutex, PAL_MUTEX_INFINITE);
//#endif
//
//    switch (type)
//    {
//        case MEM_MGR_RING_BUFFER:
//            if (gtRingTbl[id])
//                *pptHandle = (void*)gtRingTbl[id];
//            break;
//
//        default:
//            result = MEM_MGR_INVALID_INPUT;
//            break;
//    }
//
//#ifdef WIN32
//    PalReleaseMutex(gtMgrMutex);
//#endif
//    return result;
//}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Release the allocated ring buffer resource
 *
 * @param ringId    The ring identifier of a specific memory ring buffer.
 * return           none.
 */
//=============================================================================
static void
_MEMMGR_ReleaseRing(
    MMP_UINT32  ringId)
{
    if (MMP_NULL == gtRingTbl[ringId])
        return;

    //if (gtRingTbl[ringId]->pRingBufferBase)
    //{
    //    if ((MMP_UINT32)gtRingTbl[ringId]->pRingBufferBase < (MMP_UINT32)MEM_Deploy(MEM_DEPLOY_VIDEO_RING)
    //     || (MMP_UINT32)gtRingTbl[ringId]->pRingBufferBase > (MMP_UINT32)MEM_Deploy(MEM_DEPLOY_AUDIO_RING))
            PalHeapFree(PAL_HEAP_DEFAULT, gtRingTbl[ringId]->pRingBufferBase);
    //}

    PalHeapFree(PAL_HEAP_DEFAULT, gtRingTbl[ringId]);
    gtRingTbl[ringId] = MMP_NULL;
}
