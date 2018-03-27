/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia DECOMPRESS Driver API header file.
 *
 */

#ifndef MMP_DECOMPRESS_H
#define MMP_DECOMPRESS_H


#define DECOMPRESS_IRQ_ENABLE

//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"


/**
 * DLL export API declaration for Win32.
 */
#define DECOMPRESS_API extern
//=============================================================================
//                              LOG definition
//=============================================================================
//#define LOG_ZONES    (MMP_BIT_ALL /*& ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO*/)
#define LOG_ZONES    (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG /*& ~MMP_ZONE_INFO*/)
//#define LOG_ZONES     0

#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][DPU][ERROR]"
#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][DPU][WARNING]"
#define LOG_INFO    ((void) ((MMP_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][DPU][INFO]"
#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][DPU][DEBUG]"
#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][DPU][ENTER]"
#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][DPU][LEAVE]"
#define LOG_DATA    ((void) ((MMP_FALSE) ? (printf(
#define LOG_END     )), 1 : 0));

  
//=============================================================================
//                              Structure Definition
//============================================================================= 
typedef struct
{
    MMP_UINT32 DecompressIndex;
    MMP_UINT32 CmdQueIndex;
    MMP_UINT32 CmdQueDoneIndex;
    MMP_UINT32 DecompressStatus;
}DECOMPRESS_STATUS, *pDECOMPRESS_STATUS;

typedef struct
{
    MMP_UINT8 *srcbuf;
    MMP_UINT8 *dstbuf;
    MMP_UINT32 srcLen;
    MMP_UINT32 dstLen;
    MMP_UINT8 IsEnableComQ;
}DECOMPRESS_INFO, *pDECOMPRESS_INFO;
//=============================================================================
//                              Enumeration Type Definition 
//=============================================================================
/*
typedef enum 
{
    ERASE_UNIT,
    DEVICE_COUNT,
    CURRENT_DEVICE_ID,
    UN_DEFINED
}NOR_ATTITUDE;
*/

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/**
 * Initialize NOR
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorTerminate()
 */
DECOMPRESS_API MMP_RESULT
mmpDeompressInitial(
    void);

/**
 * Terminate NOR
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see NorInitial()
 */
DECOMPRESS_API MMP_RESULT
mmpDecompressTerminate(
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

DECOMPRESS_API MMP_RESULT 
mmpDecompress(
	DECOMPRESS_INFO *DecInfo);

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
DECOMPRESS_API MMP_RESULT
mmpGetDecompressSize(
    MMP_INT size);

/**
 * Get Decompress command queue status
 *
 * @param DecStatus		structure of decompress status.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */

DECOMPRESS_API MMP_RESULT	
mmpGetDecompressStatus(
	DECOMPRESS_STATUS *DecStatus);


#endif
