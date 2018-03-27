/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file mmp_tsi.h
 * Used to receive data through the transport stream interface (TSI).
 *
 * @author I-Chun Lai
 * @version 0.1
 * @example
 *  #define   MAX_READ_SIZE   0xFFEE00
 *
 *  void main()
 *  {
 *      MMP_UINT8*  sysBuf  = MMP_NULL;
 *      MMP_UINT32  totalReadSize = 0;
 *      MMP_RESULT  result;
 *      MMP_UINT32  size = 0;
 *
 *      mmpTsiInitialize();
 *      mmpTsiEnable();
 *
 *      sysBuf = (MMP_UINT8*)SYS_Malloc(MAX_READ_SIZE);
 *
 *      do
 *      {
 *          result = mmpTsiReceive(
 *              (void*)(sysBuf + totalReadSize),
 *              MAX_READ_SIZE - totalReadSize,
 *              &size);
 *
 *          if (result == MMP_SUCCESS && size > 0)
 *          {
 *              totalReadSize += size;
 *              printf("receive %d bytes\n", size);
 *          }
 *      } while(totalReadSize < MAX_READ_SIZE);
 *
 *      mmpTsiDisable();
 *      mmpTsiTerminate();
 *  }
 */
#ifndef MMP_TSI_H
#define MMP_TSI_H


#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
	#if defined(TSI_EXPORTS)
		#define TSI_API __declspec(dllexport)
	#else
		#define TSI_API __declspec(dllimport)
	#endif
#else
	#define TSI_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_RESULT_SHIFT            16
#define MMP_TSI                     1
#define MAX_TSI_COUNT               2

/**
 * Result codes
 */
#define MMP_TSI_BAD_PARAM           (MMP_TSI << MMP_RESULT_SHIFT | 0x1)
#define MMP_TSI_OUT_OF_MEM          (MMP_TSI << MMP_RESULT_SHIFT | 0x2)
#define MMP_TSI_IS_DISABLED         (MMP_TSI << MMP_RESULT_SHIFT | 0x3)
#define MMP_TSI_ENGINE_BUSY         (MMP_TSI << MMP_RESULT_SHIFT | 0x4)
#define MMP_TSP_IS_NONINITED        (MMP_TSI << MMP_RESULT_SHIFT | 0x5)

#define TSI_TOTAL_FILTER_COUNT      (32)
#define TSI_MAX_PMT_FILTER_COUNT    (23)

// The index of PID filter table. However, due to more than one active PMT pids
// in a specific frequency channel, the index of PMT here is just the first PMT
// index.
typedef enum TSI_PID_FILTER_INDEX_TAG
{
    TSI_PID_FILTER_PAT_INDEX        = 0,
    TSI_PID_FILTER_SDT_INDEX,
    TSI_PID_FILTER_EIT_INDEX,
    TSI_PID_FILTER_TOT_TDT_INDEX,
    TSI_PID_FILTER_NIT_INDEX,
    TSI_PID_FILTER_PMT_INDEX,
    TSI_PID_FILTER_TELETEXT_INDEX   = (TSI_TOTAL_FILTER_COUNT - 4),
    TSI_PID_FILTER_SUBTITLE_INDEX   = (TSI_TOTAL_FILTER_COUNT - 3),
    TSI_PID_FILTER_AUDIO_INDEX      = (TSI_TOTAL_FILTER_COUNT - 2),
    TSI_PID_FILTER_VIDEO_INDEX      = (TSI_TOTAL_FILTER_COUNT - 1)
} TSI_PID_FILTER_INDEX;

//=============================================================================
//                              Function Declaration
//=============================================================================

TSI_API MMP_RESULT
mmpTsiInitialize(
    MMP_UINT32 tsiId);

TSI_API MMP_RESULT
mmpTsiTerminate(
    MMP_UINT32 tsiId);

TSI_API MMP_RESULT
mmpTsiEnable(
    MMP_UINT32 tsiId);

TSI_API MMP_RESULT
mmpTsiDisable(
    MMP_UINT32 tsiId);

#ifdef WIN32
TSI_API MMP_RESULT
mmpTsiReceive(
    MMP_UINT32  tsiId,
    void*       buffer,
    MMP_ULONG   maxSize,
    MMP_ULONG*  actualSize);
#else
TSI_API MMP_RESULT
mmpTsiReceive(
    MMP_UINT32  tsiId,
    MMP_UINT8** ppOutBuffer,
    MMP_ULONG*  outSize);
#endif

TSI_API MMP_BOOL
mmpTsiIsPcrInterruptTriggered(
    MMP_UINT32 tsiId);

TSI_API MMP_UINT32
mmpTsiGetPcrValue(
    MMP_UINT32 tsiId);

TSI_API MMP_UINT16
mmpTsiGetPcrPid(
    MMP_UINT32 tsiId);

TSI_API MMP_UINT32
mmpTsiGetPCRCnt(
    MMP_UINT32 tsiId);

TSI_API void
mmpTsiClearInterrupt(
    MMP_UINT32 tsiId);

TSI_API MMP_RESULT
mmpTsiEnablePcrPidFilter(
    MMP_UINT32 tsiId,
    MMP_UINT16 pid);

TSI_API void
mmpTsiDisablePcrPidFilter(
    MMP_UINT32 tsiId);

TSI_API void
mmpTsiEnablePCRCntThreshold(
    MMP_UINT32 tsiId,
    MMP_UINT16 threshold);

TSI_API void
mmpTsiDisablePCRCntThreshold(
    MMP_UINT32 tsiId);

TSI_API MMP_RESULT
mmpTsiUpdatePidFilter(
    MMP_UINT32              tsiId,
    MMP_UINT32              pid,
    TSI_PID_FILTER_INDEX    pidIndex);

TSI_API void
mmpTsiResetPidFilter(
    MMP_UINT32  tsiId);

#ifdef __cplusplus
}
#endif

#endif
