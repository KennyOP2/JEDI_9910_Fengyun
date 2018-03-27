
#ifndef MEM_H
#define MEM_H

#include "mmp.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef enum MEM_USER_ID_TAG
{
    MEM_USER_MCB_NOT_ALLOCATED = 0,
    MEM_USER_FREE = 1,              // 1

    MEM_USER_BASE,
    MEM_USER_MMP = MEM_USER_BASE,   // 2
    MEM_USER_LCD,                   // 3
    MEM_USER_CMDQ,                  // 4
    MEM_USER_M2D,                   // 5
    MEM_USER_ISP,                   // 6
    MEM_USER_JPEG,                  // 7
    MEM_USER_MPEG,                  // 8
    MEM_USER_MMC,                   // 9
    MEM_USER_AUDIO,                 // 10
    MEM_USER_TSI,                   // 11
    MEM_USER_SD,                    // 12
    MEM_USER_MSPRO,                 // 13
    MEM_USER_USB_DEVICE,            // 14
    MEM_USER_NOR,                   // 15
    MEM_USER_USBEX,                 // 16
    MEM_USER_VIDEO,                 // 17
    MEM_USER_VG,                    // 18
    MEM_USER_UART,                  // 19
    MEM_USER_XD,                    // 20
    MEM_USER_NF,                    // 21
    MEM_USER_DPU,                   // 22
    MEM_USER_DECOMPRESS,            // 23
    MEM_TOTAL_USERS
} MEM_USER_ID;

typedef enum MEM_STATUS_TAG
{
    MEM_STATUS_SUCCESS,                             // 0
    MEM_STATUS_ERROR_OVER_MICROP_HEAP_SIZE,         // 1
    MEM_STATUS_ERROR_NONE_FREE_MEMORY,              // 2
    MEM_STATUS_ERROR_ALLOCATE_ZERO_MEMORY,          // 3
    MEM_STATUS_ERROR_ADDRESS_IS_ZERO,               // 4
    MEM_STATUS_ERROR_ADDRESS_NOT_FOUND,             // 5
    MEM_STATUS_ERROR_RELEASE_FREE_MEMORY,           // 6
    MEM_STATUS_ERROR_INITIALIZE_MEMORY,             // 7
    MEM_TOTOAL_STATUS
} MEM_STATUS;
//
//typedef enum MEM_DEPLOY_TAG
//{
//    MEM_DEPLOY_VIDEO_RING    = 0,
//    MEM_DEPLOY_TSI,                                     // 1
//    MEM_DEPLOY_AUDIO_RING,                              // 2
//    MEM_DEPLOY_SUBTITLE_RING,                           // 3
//    MEM_DEPLOY_TELETEXT_RING,                           // 4
//    MEM_DEPLOY_VIDEO,                                   // 5
//    MEM_DEPLOY_SLIDE            = MEM_DEPLOY_VIDEO,     // 5
//    MEM_DEPLOY_JPEG             = MEM_DEPLOY_VIDEO,     // 5
//    MEM_DEPLOY_BLANK,                                   // 6
//    MEM_DEPLOY_MPEG2_VOB,                               // 7
//    MEM_DEPLOY_H264_TC,                                 // 8
//    MEM_DEPLOY_H264_VLD,                                // 9
//    MEM_DEPLOY_H264_DB,                                 // 10
//    MEM_DEPLOY_H264_COL,                                // 11
//    MEM_DEPLOY_VIDEO_BS0,                               // 12
//    MEM_DEPLOY_VIDEO_BS1,                               // 13
//    MEM_DEPLOY_VIDEO_BS2,                               // 14
//    MEM_DEPLOY_MPEG2_BS0,                               // 15
//    MEM_DEPLOY_MPEG2_BS1,                               // 16
//    MEM_DEPLOY_COUNT
//} MEM_DEPLOY;
//
//typedef enum MEM_POOL_BLOCK_TYPE_TAG
//{
//    BLOCK_1K = 10,
//    BLOCK_2K = 11,
//    BLOCK_4K = 12
//} MEM_POOL_BLOCK_TYPE;
//
//#define MAX_POOL_COUNT  (2)
//
////=============================================================================
////                              Function Declaration
////=============================================================================
//
//#if defined(__FREERTOS__)
//MMP_API void
//MEM_Initialize(
//    void);
//
//// get reserve buf address
//MMP_API MMP_INLINE void*
//MEM_Deploy(
//    MMP_UINT32 usage);
//
//MMP_API MMP_INLINE MMP_UINT32
//MEM_DeploySize(
//    MMP_UINT32 usage);
//
////MMP_API void*
////MEM_DeployEdge(
////    MMP_UINT32 size);
//
//
////MMP_API MMP_BOOL
////MEM_ReleaseEdge(
////    void* address);
//
//#endif

MMP_API void*
MEM_Allocate(
    MMP_UINT32 memSize,
    MEM_USER_ID user);

//MMP_API void*
//MEM_Memalign(
//    MMP_UINT32 byteAlign,
//    MMP_UINT32 memSize,
//    MEM_USER_ID user);

MMP_API MEM_STATUS
MEM_Release(
    void* address);

//MMP_API MMP_UINT32
//MEM_GetMaxFreeBlockSize(
//    void);
//
//MMP_API void
//SYS_CleanMemory(
//    void* dest,
//    MMP_UINT32 size);
//
//MMP_API void *
//SYS_Malloc(
//    MMP_UINT32 size);
//
//MMP_API void
//SYS_Memcpy(
//    void* dest,
//    const void* src,
//    MMP_UINT32 size);
//
//MMP_API void
//SYS_Free(
//    void* address);
//
//MMP_API MMP_BOOL
//MEM_CreatePool(
//    MMP_INT             poolIndex,
//    MMP_UINT8*          pStartAddr,
//    MMP_SIZE_T          poolSize,
//    MEM_POOL_BLOCK_TYPE blockType);
//
//MMP_API MMP_BOOL
//MEM_TerminatePool(
//    MMP_INT poolIndex);
//
//MMP_API void*
//MEM_PoolAlloc(
//    MMP_INT poolIndex,
//    MMP_SIZE_T size);
//
//MMP_API void
//MEM_PoolFree(
//    MMP_INT    poolIndex,
//    void*      pAddr);

#ifdef __cplusplus
}
#endif

#endif
