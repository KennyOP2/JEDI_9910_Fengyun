/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file mem_mgr.h
 * A local memory manager
 * @author Steven Hsiao
 * @version 0.1
 */
#ifndef MEM_MGR_H
#define MEM_MGR_H

#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MEM_MGR_TOTAL_RING_COUNT         (8)

typedef enum MEM_MGR_ERROR_CODE_TAG
{
    MEM_MGR_NO_ERROR = 0,
    MEM_MGR_INIT_FAIL,
    MEM_MGR_NOT_INITED,
    MEM_MGR_NO_AVAILABLE_POOL,
    MEM_MGR_NO_AVAILABLE_RING,
    MEM_MGR_NO_AVAILABLE_SAMPLE,
    MEM_MGR_INVALID_INPUT,
    MEM_MGR_INVALID_PTR,
    MEM_MGR_HASH_NO_HIT,
    MEM_MGR_HASH_DUPLICATE,
    MEM_MGR_FREE_FAIL,
    MEM_MGR_ALLOCATION_FAIL,
    MEM_MGR_RESIZE_FAIL
} MEM_MGR_ERROR_CODE;

typedef enum MEM_MGR_POOL_TYPE_TAG
{
    MEM_MGR_RING_BUFFER = 0
} MEM_MGR_POOL_TYPE;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MEM_MGR_RING_ALLOC_INFO_TAG
{
    MMP_UINT8*  pStartAddr;
    MMP_UINT8*  pEndAddr;
    MMP_BOOL    bWrap;
    MMP_UINT8*  pRingStart;
    MMP_UINT8*  pRingEnd;
} MEM_MGR_RING_ALLOC_INFO;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
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
    void);

//=============================================================================
/**
 * Terminate the memory partition pool manager
 * return   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *          fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_Terminate(
    void);

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
    MMP_UINT32* pOutRingId);

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
    MMP_UINT32  ringId);

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
    MEM_MGR_RING_ALLOC_INFO*    ptOutAllocInfo);

#if defined(ENABLE_MPLAYER) || defined(WIN32)
MEM_MGR_ERROR_CODE
MemMgr_ReduceSize(
    MMP_UINT32                  ringId,
    MMP_UINT32                  newSize,
    MEM_MGR_RING_ALLOC_INFO*    ptAllocInfo);
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
    MMP_BOOL    bReverse);

//=============================================================================
/**
 * Query information of ring buffer
 *
 * @param ringId        The ring identifier of a specific memory ring buffer.
 * @param pRemainSize   The pointer to query remain size of ring buffer.
 * @param pBufferSize   The pointer to query total ring buffer size.
 * return               MEM_MGR_ERROR_CODE to indicate whether the operation is
 *                      fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_RingQuery(
    MMP_UINT32  ringId,
    MMP_UINT32* pRemainSize,
    MMP_UINT32* pBufferSize);

//=============================================================================
/**
 * Print out the buffer usuage information on the screen.
 * return   MEM_MGR_ERROR_CODE to indicate whether the operation is
 *          fail or success.
 */
//=============================================================================
MEM_MGR_ERROR_CODE
MemMgr_ShowMemUsage(
    void);

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
//    void**              pptHandle);

#ifdef __cplusplus
}
#endif

#endif
