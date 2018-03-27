/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * SPI bus Driver API header file.
 *
 * @author Sammy Chen
 */
#ifndef MMP_SPI_H
#define MMP_SPI_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if defined(WIN32) || defined(_WIN32_WCE)
    #if defined(SPI_EXPORTS)
    #define SPI_API __declspec(dllexport)
    #else
    #define SPI_API __declspec(dllimport)
    #endif
#else
    #define SPI_API extern
#endif /* defined(WIN32) */


//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum SPI_PORT_TAG
{
    SPI_1,
    SPI_2,
    SPI_PORT_MAX
}SPI_PORT;

typedef enum SPI_MODE_TAG
{
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3
}SPI_MODE;

typedef enum 
{
	CPO_0_CPH_0,
	CPO_1_CPH_0,
	CPO_0_CPH_1,
	CPO_1_CPH_1
}SPI_FORMAT;

typedef struct
{
	SPI_FORMAT format;
	SPI_MODE mode;
	MMP_UINT divider;
}SPI_CONTEXT, *pSPI_CONTEXT;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group21 ITE SPI Driver API
 *  The supported API for SPI.
 *  @{
 */
//=============================================================================
/**
 * File system must call this API first when initializing a spi bus.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdTerminate()
 */
//=============================================================================
SPI_API MMP_INT mmpSpiInitialize(SPI_PORT port);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is terminated.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdInitialize()
 */
//=============================================================================
SPI_API MMP_INT mmpSpiTerminate(SPI_PORT port);

//=============================================================================
/**
 * This routine is used to write data to targe device by dma.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSpiDmaWrite()
 */
//=============================================================================
SPI_API MMP_INT
mmpSpiDmaWrite(
    SPI_PORT port,
    MMP_UINT8 *inputData,
    MMP_INT   inputSize,
    MMP_UINT8* psrc,
    MMP_INT size,
    MMP_UINT8 dataLength);
//=============================================================================
/**
 * This routine is used to read data from targe device by dma.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSpiDmaRead()
 */
//=============================================================================
SPI_API MMP_INT
mmpSpiDmaRead(
    SPI_PORT port,
    MMP_UINT8 *inputData,
    MMP_INT   inputSize,
    void* pdes,
    MMP_INT size,
    MMP_UINT8 dataLength);

SPI_API MMP_INT
mmpSpiDmaTriggerRead(
    SPI_PORT	port,
    void*		pdes,
    MMP_INT		size);

SPI_API void
mmpSpiDmaTriggerReadWiatIdle();

 
//=============================================================================
/**
 * This routine is used to write data to targe device by pio.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API MMP_INT
mmpSpiPioWrite(
    SPI_PORT port,
    void *inputData,
    MMP_UINT32   inputSize,
    void *pbuf,
    MMP_UINT32 size,
    MMP_UINT8 dataLength);
//=============================================================================
/**
 * This routine is used to read data from targe device by pio.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API MMP_INT
mmpSpiPioRead(
    SPI_PORT port,
    void *inputData,
    MMP_UINT32 inputSize,
    void *outputBuf,
    MMP_UINT32 size,
    MMP_UINT8 dataLength);

SPI_API void
mmpSpiSetMode(
    SPI_PORT port,
    SPI_MODE mode);    

SPI_API void
mmpSpiSetMaster(
    SPI_PORT port);

SPI_API void
mmpSpiSetSlave(
    SPI_PORT port);

//@}


#ifdef __cplusplus
}
#endif

#endif /* MMP_SPI_H */
