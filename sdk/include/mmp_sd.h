/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia SD/MMC Card Driver API header file.
 *
 * @author Irene Lin
 */
#ifndef MMP_SD_H
#define MMP_SD_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if defined(WIN32) || defined(_WIN32_WCE)
    #if defined(SD_EXPORTS)
    #define SD_API __declspec(dllexport)
    #else
    #define SD_API __declspec(dllimport)
    #endif
#else
    #define SD_API extern
#endif /* defined(WIN32) */


//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum SD_CARD_STATE_TAG
{
    SD_INIT_OK,
    SD_INSERTED
}SD_CARD_STATE;

typedef enum SD_INDEX_TAG
{
    SD_1,
    SD_2,
    SD_NUM
}SD_INDEX;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group21 SMedia CF Driver API
 *  The supported API for CF.
 *  @{
 */
//=============================================================================
/**
 * File system must call this API first when initializing a volume.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdTerminate()
 */
//=============================================================================
SD_API MMP_INT mmpSdInitializeEx(MMP_INT index);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is removed.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdInitialize()
 */
//=============================================================================
SD_API MMP_INT mmpSdTerminateEx(MMP_INT index);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdWriteMultipleSector()
 */
//=============================================================================
SD_API MMP_INT mmpSdReadMultipleSectorEx(MMP_INT index, MMP_UINT32 sector, MMP_INT count, void* data);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdReadMultipleSector()
 */
//=============================================================================
SD_API MMP_INT mmpSdWriteMultipleSectorEx(MMP_INT index, MMP_UINT32 sector, MMP_INT count, void* data);

//=============================================================================
/**
 * This routine is used to get the number of sectors and block size.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SD_API MMP_INT mmpSdGetCapacityEx(MMP_INT index, MMP_UINT32* sectorNum, MMP_UINT32* blockLength);
    
//=============================================================================
/**
 * This routine is used to get the device status.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SD_API MMP_BOOL mmpSdGetCardStateEx(MMP_INT index, SD_CARD_STATE state);   

//=============================================================================
/**
 * This routine is used to check if CF card is in write-protected status.
 *
 * @return MMP_TRUE if CF card is in write-protected status, otherwise return MMP_FALSE.
 */
//=============================================================================
SD_API MMP_BOOL mmpSdIsLockEx(MMP_INT index);

/** Get CID information */
SD_API void mmpSdGetCidEx(MMP_INT index, MMP_UINT8* buf);


#define mmpSdInitialize()               mmpSdInitializeEx(SD_1)
#define mmpSdTerminate()                mmpSdTerminateEx(SD_1)
#define mmpSdReadMultipleSector(a,b,c)  mmpSdReadMultipleSectorEx(SD_1, a, b, c)
#define mmpSdWriteMultipleSector(a,b,c) mmpSdWriteMultipleSectorEx(SD_1, a, b, c)
#define mmpSdGetCapacity(a,b)           mmpSdGetCapacityEx(SD_1, a, b)
#define mmpSdGetCardState(a)            mmpSdGetCardStateEx(SD_1, a)
#define mmpSdIsLock()                   mmpSdIsLockEx(SD_1)
#define mmpSdGetCID(a)                  mmpSdGetCidEx(SD_1, a)

#define mmpSd2Initialize()               mmpSdInitializeEx(SD_2)
#define mmpSd2Terminate()                mmpSdTerminateEx(SD_2)
#define mmpSd2ReadMultipleSector(a,b,c)  mmpSdReadMultipleSectorEx(SD_2, a, b, c)
#define mmpSd2WriteMultipleSector(a,b,c) mmpSdWriteMultipleSectorEx(SD_2, a, b, c)
#define mmpSd2GetCapacity(a,b)           mmpSdGetCapacityEx(SD_2, a, b)
#define mmpSd2GetCardState(a)            mmpSdGetCardStateEx(SD_2, a)
#define mmpSd2IsLock()                   mmpSdIsLockEx(SD_2)
#define mmpSd2GetCID(a)                  mmpSdGetCidEx(SD_2, a)



//@}


#ifdef __cplusplus
}
#endif

#endif /* MMP_SD_H */
