/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia NOR Driver API header file.
 *
 */

#ifndef MMP_NOR_H
#define MMP_NOR_H

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NOR_API extern

#define BUILD_VERSION   0xA   
  
//=============================================================================
//                              Structure Definition
//============================================================================= 

//=============================================================================
//                              Enumeration Type Definition 
//=============================================================================
typedef enum 
{
    NOR_ATTITUDE_ERASE_UNIT,
    NOR_ATTITUDE_DEVICE_COUNT,
    NOR_ATTITUDE_CURRENT_DEVICE_ID,
    NOR_ATTITUDE_PAGE_SIZE,
    NOR_ATTITUDE_PAGE_PER_SECTOR,
    NOR_ATTITUDE_SECTOR_PER_BLOCK,
    NOR_ATTITUDE_BLOCK_SIZE,
    NOR_ATTITUDE_TOTAL_SIZE,
    NOR_ATTITUDE_UN_DEFINED
}NOR_ATTITUDE;

//=============================================================================
//                              Constant Definition
//=============================================================================
/*Error Code*/
#define NOR_ERROR_DEVICE_UNKNOW		1
#define NOR_ERROR_DEVICE_TIMEOUT    2
#define NOR_ERROR_STATUS_BUSY	    3
#define NOR_ERROR_STATUS_PROTECT	4
#define NOR_ERROR_ADDR_UN_ALIGNED   5

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group17 SMedia NOR Driver API
 *  The NOR module API.
 *  @{
 */

/**
 * Initialize NOR
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorTerminate()
 */
NOR_API MMP_RESULT
NorInitial(
    void);

/**
 * Terminate NOR
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorInitial()
 */
NOR_API MMP_RESULT
NorTerminate(
    void);

/**
 * Read Nor Data
 *
 * @param pdes        destination data buffer.
 * @param addr        read address.
 * @param size        read size.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorWrite()
 */
NOR_API MMP_RESULT
NorRead(
    void* pdes,
    MMP_UINT addr,
    MMP_INT size);

/**
 * Write NOR Data
 *
 * @param psrc        destination data buffer.
 * @param addr        write address.
 * @param size        write size.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorRead()
 */
NOR_API MMP_RESULT
NorWrite(
    void* psrc,
    MMP_UINT addr,
    MMP_INT size);

/**
 * Write NOR Data Without Erase
 *
 * @param psrc        destination data buffer.
 * @param addr        write address.
 * @param size        write size.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorRead()
 */
NOR_API MMP_RESULT
NorWriteWithoutErase(
    void* psrc,
    MMP_UINT addr,
    MMP_INT size);

/**
 * Erase whole NOR flash chip
 *
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorRead()
 */
NOR_API MMP_RESULT
NorBulkErase(
	void);

/**
 * Get NOR Capacity
 *
 *
 * @return NOR Capacity size.
 * @see NorGetAttitude()
 */
NOR_API MMP_UINT32
NorCapacity(
    void);

/**
 * Get NOR NOR_ATTITUDE_ERASE_UNIT,
 *         NOR_ATTITUDE_DEVICE_COUNT,
 *         NOR_ATTITUDE_CURRENT_DEVICE_ID information
 *
 *
 * @param atti        NOR attitude.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorGetDeviceName()
 */
NOR_API MMP_UINT32
NorGetAttitude(
    NOR_ATTITUDE atti);

/**
 * Get NOR Device Name
 *
 *
 * @param num        NOR Support ID num.
 *
 * @return Device Name.
 * @see NorGetAttitude()
 */
NOR_API MMP_UINT8*
NorGetDeviceName(
    MMP_UINT8 num);

/**
 * Get NOR Driver Build Info
 *
 *
 * @param version     NOR Driver Build Version.
 * @param date        NOR Driver Build Data.
 *
 * @see NorGetDeviceName()
 */
NOR_API void
NorGetBuildInfo(
    MMP_UINT8* version,
    MMP_UINT8* date);

//@}
#ifdef __cplusplus
}
#endif

#endif
