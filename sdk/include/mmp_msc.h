/**
 * mmp_msc.h
 */
#ifndef MMP_MSC_H
#define MMP_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
    #if defined(MSC_EXPORTS)
        #define MSC_API __declspec(dllexport)
    #else
        #define MSC_API __declspec(dllimport)
    #endif
#else
    #define MSC_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_MSC_MAX_LUN_NUM     8

//=============================================================================
//                              Struct Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
MSC_API MMP_INT mmpMscDriverRegister(void);

//=============================================================================
/**
 * File system must call this API first when initializing a volume.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpMscTerminate()
 */
//=============================================================================
MSC_API MMP_INT mmpMscInitialize(void* us, MMP_UINT8 lun);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is removed.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpMscInitialize()
 */
//=============================================================================
MSC_API MMP_INT mmpMscTerminate(void* us, MMP_UINT8 lun);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpMscWriteMultipleSector()
 */
//=============================================================================
MSC_API MMP_INT mmpMscReadMultipleSector(void* us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void* data);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpMscWriteMultipleSector()
 */
//=============================================================================
MSC_API MMP_INT mmpMscWriteMultipleSector(void* us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void* data);

//=============================================================================
/**
 * This routine is used to get the number of sectors and block size.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSC_API MMP_INT mmpMscGetCapacity(void* us, MMP_UINT8 lun, MMP_UINT32* sectorNum, MMP_UINT32* blockLength);
    
//=============================================================================
/**
 * This routine is used to get the device status.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSC_API MMP_INT mmpMscGetStatus(void* us, MMP_UINT8 lun);   
MSC_API MMP_INT mmpMscGetStatus2(void* us, MMP_UINT8 lun);   

//=============================================================================
/**
 * This routine is used to check if the card(lun) is in write-protected status.
 *
 * @return MMP_TRUE if this lun is in write-protected status, otherwise return MMP_FALSE.
 */
//=============================================================================
MSC_API MMP_BOOL mmpMscIsLock(void* us, MMP_UINT8 lun);

/** Irene: 2011_0106
 * Is in read/write procedure? Just for application workaround. 
 */
MSC_API MMP_BOOL mmpMscInDataAccess(void* us);


// +wlHsu
MSC_API void
mmpMscResetErrStatus(
    void);
// -wlHsu

#ifdef __cplusplus
}
#endif

#endif /* MMP_MSC_H */
