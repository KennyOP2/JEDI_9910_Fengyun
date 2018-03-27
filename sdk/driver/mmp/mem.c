///*
// * Copyright (c) 2009 ITE Technology Corp. All Rights Reserved.
// */
///** @file mem.c
// *
// * @author Vincent Lee
// */
//
#include "mem/mem.h"
//#include "host/host.h"
//
//#if defined(__FREERTOS__)
//
////=============================================================================
////                              Constant Definition
////=============================================================================
//
//#define DEPLOY_BUFFER_PITCH         1536
//#if HAVE_MP4_264
//#define DEPLOY_BUFFER_HEIGHT        5010 // 3870((576+288)*4+138*3) + 1140
//#else
//#define DEPLOY_BUFFER_HEIGHT        3156 // 2016((576+288)*2+144*2) + 1140
//#endif
////#define VIDEO_Y_WIDTH               720
////#define VIDEO_Y_HEIGHT              576
////#define VIDEO_UV_HEIGHT             288
//
//// edge line buffer list count, every line has 96 bytes
////#define DEPLOY_EDGE_LINE_COUNT      36      // VIDEO_Y_HEIGHT*2/32
////#define EDGE_SPLIT                  24      // split 96 to 24:72
//
//// every block can specified by 2 bits. So 1 byte can control 4 block.
//// 11 is used block head.
//// 10 is used block body.
//// 00 is free memory.
//// ex1. for per block size is 4K, we allocate 16K.
//// the usageTbl will be. 11 10 10 10 00 00 00
////                       ~~~~~~~~~~~ 16K
//// ex2. for per block size is 4K, we allocate 4K.
//// the usuageTbl will be. 11 00 00 00 00 00 00
//typedef struct MEM_POOL_CTRL_BLOCK_TAG
//{
//    MMP_UINT32  blockType;
//    MMP_UINT8*  pStartAddr;
//    MMP_UINT8*  pEndAddr;
//    MMP_UINT32  totalBlockCount;
//    MMP_UINT32* pUsageTbl;
//    MMP_UINT32  usageTblCount;
//} MEM_POOL_CTRL_BLOCK;
//
////=============================================================================
////                              Global Data Definition
////=============================================================================
//
//static void* deployBuffer[MEM_DEPLOY_COUNT] = { 0 };
////static MMP_UINT32 deployEdgeLine1[DEPLOY_EDGE_LINE_COUNT] = { 0 };
////static MMP_UINT32 deployEdgeLine2[DEPLOY_EDGE_LINE_COUNT] = { 0 };
//static MMP_UINT16 deployTable[MEM_DEPLOY_COUNT][2] = {
//    // x, y
//    {    0,                  0}, // MEM_DEPLOY_VIDEO_RING
//    {    0,                933}, // MEM_DEPLOY_TSI
//    {    0,                933}, // MEM_DEPLOY_EXT_PCR_RING
//    {    0,            3 + 933}, // MEM_DEPLOY_TELETEXT_RING
//    {    0,           10 + 933}, // MEM_DEPLOY_SUBTITLE_RING
//    {    0,           32 + 933}, // MEM_DEPLOY_AUDIO_RING
//    {    0,               1140}, // MEM_DEPLOY_VIDEO
//    {720*2,               1140}, // MEM_DEPLOY_BLANK
//    {720*2,          16 + 1140}, // MEM_DEPLOY_EDGE
//    {    0,       576*2 + 1140}, // MEM_DEPLOY_JPEG
//#if HAVE_MP4_264
//    {    0, (576+288)*2 + 1140}, // MEM_DEPLOY_MPEG2_VOB
//    {    0, (576+288)*4 + 1140},       // MEM_DEPLOY_VIDEO_BS0
//    {    0, (576+288)*4 + 1140 + 138}, // MEM_DEPLOY_VIDEO_BS1
//    {    0, (576+288)*4 + 1140 + 276}, // MEM_DEPLOY_VIDEO_BS2
//
//    {    0, (576+288)*4 + 1140},       // MEM_DEPLOY_MPEG2_BS0
//    {    0, (576+288)*4 + 1140 + 144}, // MEM_DEPLOY_MPEG2_BS1
//#else
//    {    0, (576+288)*2 + 1140},       // MEM_DEPLOY_MPEG2_BS0
//    {    0, (576+288)*2 + 1140 + 144}, // MEM_DEPLOY_MPEG2_BS1
//#endif
//};
//
//static MEM_POOL_CTRL_BLOCK gpPoolCtrlBlock[MAX_POOL_COUNT] = { 0 };
//
////=============================================================================
////                              Function Definition
////=============================================================================
//
//void
//MEM_Initialize(
//    void)
//{    
//    MMP_UINT32 size = DEPLOY_BUFFER_PITCH * DEPLOY_BUFFER_HEIGHT;
//    MMP_UINT32 i;
//    MMP_UINT32 offset = 0;
//    
//    deployBuffer[0] = malloc(size);
//    for (i=1; i<MEM_DEPLOY_COUNT; i++)
//    {
//        offset = deployTable[i][0] + deployTable[i][1]*DEPLOY_BUFFER_PITCH;
//        deployBuffer[i] = deployBuffer[0] + offset;
//    }
//}
//
//void*
//MEM_Deploy(
//    MMP_UINT32 usage)
//{
//    return deployBuffer[usage];
//}
//
////void*
////MEM_DeployEdge(
////    MMP_UINT32 size)
////{
////    void* address = MMP_NULL;
////    MMP_UINT32 i, j;
////
////    if ((deployBuffer[MEM_DEPLOY_EDGE] == MMP_NULL)
////     || (size > (96-EDGE_SPLIT)))
////        goto end;
////    
////    if (size <= EDGE_SPLIT)
////    {
////        for (i=0; (i<DEPLOY_EDGE_LINE_COUNT) && (deployEdgeLine1[i] == 0xFFFFFFFF); i++);
////        if (i<DEPLOY_EDGE_LINE_COUNT)
////        {
////            for (j=0; (j<32) && ((deployEdgeLine1[i]>>j) & 1) ; j++);
////            address = deployBuffer[MEM_DEPLOY_EDGE] + DEPLOY_BUFFER_PITCH*(32*i+j);
////            deployEdgeLine1[i] ^= (1<<j);
////        }
////    }
////    
////    if (address == MMP_NULL)
////    {
////        for (i=0; (i<DEPLOY_EDGE_LINE_COUNT) && (deployEdgeLine2[i] == 0xFFFFFFFF); i++);
////        if (i<DEPLOY_EDGE_LINE_COUNT)
////        {
////            for (j=0; (j<32) && ((deployEdgeLine2[i]>>j) & 1) ; j++);
////            address = deployBuffer[MEM_DEPLOY_EDGE] + DEPLOY_BUFFER_PITCH*(32*i+j);
////            address += EDGE_SPLIT;
////            deployEdgeLine2[i] ^= (1<<j);
////        }    
////    }
////
////end:
////    return address;
////}
//
////MMP_BOOL
////MEM_ReleaseEdge(
////    void* address)
////{
////    MMP_BOOL result = MMP_FALSE;
////    MMP_UINT32 i, j, index;
////
////    if ((deployBuffer[MEM_DEPLOY_EDGE] <= address ) && (address < deployBuffer[3]))
////    {
////        if ((address-deployBuffer[MEM_DEPLOY_EDGE])%DEPLOY_BUFFER_PITCH)
////        {
////            address -= EDGE_SPLIT;
////            index = (address-deployBuffer[MEM_DEPLOY_EDGE])/DEPLOY_BUFFER_PITCH;
////            i = index/32;
////            j = index%32;
////            deployEdgeLine2[i] ^= (1<<j);
////        }
////        else
////        {
////            index = (address-deployBuffer[MEM_DEPLOY_EDGE])/DEPLOY_BUFFER_PITCH;
////            i = index/32;
////            j = index%32;
////            deployEdgeLine1[i] ^= (1<<j);      
////        }
////        result = MMP_TRUE;
////    }
////    
////    return result;
////}
//
void*
MEM_Allocate(
    MMP_UINT32 memSize,
    MEM_USER_ID user)
{
    return malloc(memSize);
}
//
//void*
//MEM_Memalign(
//    MMP_UINT32 byteAlign,
//    MMP_UINT32 memSize,
//    MEM_USER_ID user)
//{
//   return memalign(byteAlign,memSize);
//}	
//
MEM_STATUS
MEM_Release(
    void* address)
{
    if (address) 
    {
        free(address);
        return MEM_STATUS_SUCCESS;
    }
    else
        return MEM_STATUS_ERROR_ADDRESS_IS_ZERO;
}
//
//MMP_UINT32
//MEM_GetMaxFreeBlockSize(
//    void)
//{
//    MMP_UINT32 i = 512;
//    MMP_UINT32 maxFreeSize = 0;
//    void* maxFreeBlock = MMP_NULL;    
//    
//    for (; i>0; i--)
//    {
//        maxFreeBlock = malloc(i*1024+4);
//        if (maxFreeBlock)
//        {
//            maxFreeSize = i*1024;
//            free(maxFreeBlock);
//            break;
//        }
//    }
//    
//    return maxFreeSize;
//}
//
//MMP_BOOL
//MEM_CreatePool(
//    MMP_INT             poolIndex,
//    MMP_UINT8*          pStartAddr,
//    MMP_SIZE_T          poolSize,
//    MEM_POOL_BLOCK_TYPE blockType)
//{
//    if (poolIndex < MAX_POOL_COUNT)
//    {
//        if (gpPoolCtrlBlock[poolIndex].pUsageTbl)
//            MEM_TerminatePool(poolIndex);
//        
//        // Make it double word alignment only.
//        if (((MMP_UINT32) pStartAddr & 0x3))
//            return MMP_FALSE;
//
//        switch(blockType)
//        {
//            case BLOCK_2K:
//            case BLOCK_4K:
//            case BLOCK_1K:
//                gpPoolCtrlBlock[poolIndex].blockType = blockType;
//                break;
//            default:
//                return MMP_FALSE;
//        }
//
//        gpPoolCtrlBlock[poolIndex].totalBlockCount = (poolSize >> gpPoolCtrlBlock[poolIndex].blockType);
//        if (0 == gpPoolCtrlBlock[poolIndex].totalBlockCount)
//            return MMP_FALSE;
//        gpPoolCtrlBlock[poolIndex].usageTblCount = (gpPoolCtrlBlock[poolIndex].totalBlockCount >> 4);
//        gpPoolCtrlBlock[poolIndex].pUsageTbl = (MMP_UINT32*) malloc(gpPoolCtrlBlock[poolIndex].usageTblCount * sizeof(MMP_UINT32));
//        memset(gpPoolCtrlBlock[poolIndex].pUsageTbl, 0x0, gpPoolCtrlBlock[poolIndex].usageTblCount * sizeof(MMP_UINT32));
//        gpPoolCtrlBlock[poolIndex].pStartAddr = pStartAddr;
//        gpPoolCtrlBlock[poolIndex].pEndAddr = pStartAddr + (gpPoolCtrlBlock[poolIndex].totalBlockCount << gpPoolCtrlBlock[poolIndex].blockType);
//        return MMP_TRUE;
//    }
//    else
//        return MMP_FALSE;
//}
//
//MMP_BOOL
//MEM_TerminatePool(
//    MMP_INT poolIndex)
//{
//    MMP_BOOL result = MMP_TRUE;
//
//    if (poolIndex < MAX_POOL_COUNT && gpPoolCtrlBlock[poolIndex].pUsageTbl)
//    {
//        free(gpPoolCtrlBlock[poolIndex].pUsageTbl);
//        memset(&gpPoolCtrlBlock[poolIndex], 0x0, sizeof(MEM_POOL_CTRL_BLOCK));
//    }
//    else
//        result = MMP_FALSE;
//    
//    return result;
//}
//
//void*
//MEM_PoolAlloc(
//    MMP_INT     poolIndex,
//    MMP_SIZE_T  size)
//{
//    MMP_UINT32 i = 0;
//    MMP_INT32  j = 0;
//    MMP_UINT32 blockNeed = size >> gpPoolCtrlBlock[poolIndex].blockType;
//    MMP_UINT32 freeBlockNum = 0xFFFFFFFF;
//    MMP_UINT32 freeCount = 0;
//    MMP_UINT32 tblValue = 0;
//    MMP_UINT32 startBlockIndex = 0;
//    MMP_UINT32 startIndexOfBlock = 0;
// 
//    if (size & ((0x1 << gpPoolCtrlBlock[poolIndex].blockType) - 1))
//        blockNeed += 1;
//
//    for (i =0; i < gpPoolCtrlBlock[poolIndex].usageTblCount; i++)
//    {
//        tblValue = gpPoolCtrlBlock[poolIndex].pUsageTbl[i];
//        for (j = 30; j >= 0; j -= 2)
//        {
//            if ((tblValue & (0x3 << j)))
//            {
//                freeBlockNum = 0xFFFFFFFF;
//                continue;
//            }
//            else if (freeBlockNum == 0xFFFFFFFF)
//            {
//                // first free block.
//                freeBlockNum = (i << 4) + ((30 - j) >> 1);
//                freeCount = 0;
//            }
//
//            if (freeBlockNum != 0xFFFFFFFF)
//                freeCount++;
//            if (freeCount == blockNeed)
//            {
//                startBlockIndex = (freeBlockNum >> 4);
//                startIndexOfBlock = (freeBlockNum & 0xF);
//                // Set head value (11)
//                gpPoolCtrlBlock[poolIndex].pUsageTbl[startBlockIndex] |= (0x3 << ((15 - startIndexOfBlock) << 1));
//                --blockNeed;
//                while (blockNeed)
//                {
//                    if (0 == (++startIndexOfBlock & 0xF))
//                        ++startBlockIndex;
//                    // Set body balue (10)
//                    gpPoolCtrlBlock[poolIndex].pUsageTbl[startBlockIndex] |= (0x2 << ((15 - startIndexOfBlock) << 1));
//                    --blockNeed;
//                }
//                return (gpPoolCtrlBlock[poolIndex].pStartAddr + (freeBlockNum << gpPoolCtrlBlock[poolIndex].blockType));
//            }
//        }
//    }
//    return MMP_NULL;
//}
//
//void
//MEM_PoolFree(
//    MMP_INT    poolIndex,
//    void*      pAddr)
//{
//    MMP_UINT32 blockNum = 0;
//    MMP_UINT32 startBlockIndex = 0;
//    MMP_UINT32 startIndexOfBlock = 0;
//    MMP_UINT8* pFreeAddr = (MMP_UINT8*) pAddr;
//    
//    if (pFreeAddr < gpPoolCtrlBlock[poolIndex].pStartAddr
//     || pFreeAddr >= gpPoolCtrlBlock[poolIndex].pEndAddr)
//        return;
//
//    blockNum = ((pFreeAddr - gpPoolCtrlBlock[poolIndex].pStartAddr) >> gpPoolCtrlBlock[poolIndex].blockType);
//    if (blockNum < gpPoolCtrlBlock[poolIndex].totalBlockCount)
//    {
//        startBlockIndex = (blockNum >> 4);
//        startIndexOfBlock = (blockNum & 0xF);
//     
//        gpPoolCtrlBlock[poolIndex].pUsageTbl[startBlockIndex] &= ~(0x3 << ((15 - startIndexOfBlock) << 1));
//        ++startIndexOfBlock;
//        startIndexOfBlock &= 0xF;
//        if(0 == startIndexOfBlock)
//            ++startBlockIndex;
//
//        // Stop either the next block is 11 or 00, or end of whole blocks.
//        while ((gpPoolCtrlBlock[poolIndex].pUsageTbl[startBlockIndex] & (0x3 << ((15 - startIndexOfBlock) << 1))) == (0x2 << ((15 - startIndexOfBlock) << 1)))
//        {
//            gpPoolCtrlBlock[poolIndex].pUsageTbl[startBlockIndex] &= ~(0x3 << ((15 - startIndexOfBlock) << 1));
//            ++startIndexOfBlock;
//            startIndexOfBlock &= 0xF;
//            if(0 == startIndexOfBlock)
//                ++startBlockIndex;
//            // end of whole blockes.
//            if (startBlockIndex >= gpPoolCtrlBlock[poolIndex].usageTblCount)
//                break;
//        }
//    }
//}
//
//#else
////=============================================================================
////                              Structure Definition
////=============================================================================
//
//typedef struct MEM_CONTROL_BLOCK_TAG
//{
//    MMP_UINT32  address;
//    MMP_UINT32  size;
//    MMP_UINT    user;
//    struct MEM_CONTROL_BLOCK_TAG* ptPrev;
//    struct MEM_CONTROL_BLOCK_TAG* ptNext;
//} MEM_CONTROL_BLOCK;
//
////=============================================================================
////                              Global Data Declaration
////=============================================================================
//
//static MEM_CONTROL_BLOCK* mcbHeader = MMP_NULL;
//
////=============================================================================
////                              Private Function Declaration
////=============================================================================
//
//static MEM_CONTROL_BLOCK* 
//_MEM_McbAllocate(
//    void);
//
//static void 
//_MEM_McbRelease(
//    MEM_CONTROL_BLOCK* mcbCurrent);
//
//static void
//_MEM_McbFission(
//    MEM_CONTROL_BLOCK* mcbCurrent,
//    MMP_UINT32 memSize,
//    MEM_USER_ID user);
//
//static void
//_MEM_McbFusion(
//    MEM_CONTROL_BLOCK* mcbCurrent);
//
////=============================================================================
////                              Public Function Definition
////=============================================================================
//
//MMP_RESULT
//MEM_Initialize(
//    MMP_UINT32 heapSize)
//{
//    MEM_CONTROL_BLOCK* mcbInit = MMP_NULL;
//	MMP_UINT8 * tempBuf1;
//
//    mcbHeader = _MEM_McbAllocate();
//    mcbHeader->address = MMP_NULL;
//    mcbHeader->user = MEM_USER_BASE;
//    
//    mcbInit = _MEM_McbAllocate();
//    mcbInit->address = (MMP_UINT32) HOST_GetVramBaseAddress();
//    mcbInit->size = heapSize;
//    mcbInit->user = MEM_USER_FREE;
//    mcbInit->ptPrev = mcbHeader;
//    mcbInit->ptNext = mcbHeader;
//
//    mcbHeader->ptPrev = mcbInit;
//    mcbHeader->ptNext = mcbInit;
//
//#ifdef _WIN32
//	tempBuf1 = (MMP_UINT8 *)MEM_Allocate(1024 , MEM_USER_MMP);
//#endif
//
//    return MMP_RESULT_SUCCESS;
//}
//
//void*
//MEM_Allocate(
//    MMP_UINT32 memSize,
//    MEM_USER_ID user)
//{
//    MEM_CONTROL_BLOCK* mcbCurrent = mcbHeader;
//    
//    memSize = ((memSize + 3) >> 2) << 2;
//    while ((mcbCurrent = mcbCurrent->ptNext) != mcbHeader)
//    {
//        if (mcbCurrent->user == MEM_USER_FREE && mcbCurrent->size >= memSize)
//        {
//            _MEM_McbFission(mcbCurrent, memSize, user);
//            break;
//        }
//    }
//    
//    return (void*) mcbCurrent->address;
//}
//
//void*
//MEM_Memalign(
//    MMP_UINT32 byteAlign,
//    MMP_UINT32 memSize,
//    MEM_USER_ID user)
//{
//   MEM_CONTROL_BLOCK* mcbCurrent = mcbHeader;
//   MMP_UINT32 allocSize = memSize + byteAlign;
//    
//    allocSize = ((allocSize + 3) >> 2) << 2;
//    while ((mcbCurrent = mcbCurrent->ptNext) != mcbHeader)
//    {
//        if (mcbCurrent->user == MEM_USER_FREE && mcbCurrent->size >= allocSize)
//        {
//            _MEM_McbFission(mcbCurrent, allocSize, user);
//            break;
//        }
//    }
//    
//	mcbCurrent->address = (MMP_UINT8 *)(((MMP_UINT32)mcbCurrent->address + (byteAlign-1)) & ~(byteAlign-1));
//	
//    return (void*) mcbCurrent->address;
//}
//
//MEM_STATUS
//MEM_Release(
//    void* address)
//{
//    MEM_CONTROL_BLOCK* mcbCurrent = mcbHeader;
//    
//    while ((mcbCurrent = mcbCurrent->ptNext) != mcbHeader)
//    {
//        if (mcbCurrent->address == (MMP_UINT32) address)
//        {
//            _MEM_McbFusion(mcbCurrent);
//
//            return MEM_STATUS_SUCCESS;
//        }
//    }
//    
//    return MEM_STATUS_ERROR_ADDRESS_NOT_FOUND;
//}
//
//MMP_UINT32
//MEM_GetMaxFreeBlockSize(
//    void)
//{
//    MMP_UINT32 maxFreeSize = 0;
//    MEM_CONTROL_BLOCK* mcbCurrent = mcbHeader;
//
//    while ((mcbCurrent = mcbCurrent->ptNext) != mcbHeader)
//    {
//        if (mcbCurrent->user == MEM_USER_FREE && mcbCurrent->size > maxFreeSize)
//        {
//            maxFreeSize = mcbCurrent->size;
//        }
//    }
//    
//    return maxFreeSize;
//}
//
////=============================================================================
////                              Private Function Definition
////=============================================================================
//
//static MEM_CONTROL_BLOCK* 
//_MEM_McbAllocate(
//    void)
//{
//    return (MEM_CONTROL_BLOCK*) SYS_Malloc(sizeof(MEM_CONTROL_BLOCK));
//}
//
//static void 
//_MEM_McbRelease(
//    MEM_CONTROL_BLOCK* mcbCurrent)
//{
//    SYS_Free(mcbCurrent);
//}
//
//static void
//_MEM_McbFission(
//    MEM_CONTROL_BLOCK* mcbCurrent,
//    MMP_UINT32 memSize,
//    MEM_USER_ID user)
//{
//    MEM_CONTROL_BLOCK* mcbSplit = _MEM_McbAllocate();
//
//    memset(mcbSplit, 0x00, sizeof(MEM_CONTROL_BLOCK));
//    mcbSplit->address = mcbCurrent->address + memSize;
//    mcbSplit->size = mcbCurrent->size - memSize;
//    mcbCurrent->ptNext->ptPrev = mcbSplit;
//    mcbSplit->user = MEM_USER_FREE;
//    mcbSplit->ptPrev = mcbCurrent;
//    mcbSplit->ptNext = mcbCurrent->ptNext;
//
////  mcbCurrent->address
//    mcbCurrent->size = memSize;
//    mcbCurrent->user = user;
//    mcbCurrent->ptNext = mcbSplit;
////  mcbCurrent->ptPrev
//}
//
//static void
//_MEM_McbFusion(
//    MEM_CONTROL_BLOCK* mcbCurrent)
//{
//    MEM_CONTROL_BLOCK* mcbPrev = mcbCurrent->ptPrev;
//    MEM_CONTROL_BLOCK* mcbNext = mcbCurrent->ptNext;
//
//    mcbCurrent->user = MEM_USER_FREE;
//
//    if (mcbPrev->user == MEM_USER_FREE)
//    {
////      mcbPrev->address
//        mcbPrev->size = mcbPrev->size + mcbCurrent->size;
////      mcbPrev->user
//        mcbPrev->ptNext = mcbNext;
////      mcbPrev->ptPrev
//        mcbNext->ptPrev = mcbPrev;
//        
//        _MEM_McbRelease(mcbCurrent);
//        
//        mcbCurrent = mcbPrev;
//        mcbPrev = mcbCurrent->ptPrev;
//    }
//    
//    if (mcbNext->user == MEM_USER_FREE)
//    {
//        mcbNext->address = mcbCurrent->address;
//        mcbNext->size = mcbCurrent->size + mcbNext->size;
////      mcbNext->user
////      mcbNext->ptNext
//        mcbNext->ptPrev = mcbPrev;
//        mcbPrev->ptNext = mcbNext;
//        
//        _MEM_McbRelease(mcbCurrent);
//    }
//}
//
//#endif
