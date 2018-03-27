/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Caster operation API
 *
 * @author I-Chun Lai
 * @version 1.0
 */
#ifndef USB2SPI_H
#define USB2SPI_H

#include "mmp_types.h"

#if defined(WIN32) || defined(_WIN32_WCE)

#ifdef usb2spi_EXPORTS
#define USB2SPI_API __declspec(dllexport)
#else
#define USB2SPI_API __declspec(dllimport)
#endif

#else
#define USB2SPI_API extern

#endif
#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
//                              Internal Function Declaration
//=============================================================================


//=============================================================================
//                              External Function Declaration
//=============================================================================

//=============================================================================
/**
 * Read Castor Register
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
USB2SPI_API MMP_RESULT
usb2spi_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *value);

//=============================================================================
/**
 * Use Casotr FW to Write Castor Register
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
USB2SPI_API MMP_RESULT
usb2spi_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 value);

//=============================================================================
/**
 * Read Castor Memory
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
USB2SPI_API MMP_RESULT
usb2spi_ReadMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte);

//=============================================================================
/**
 * Write Castor Memory
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
USB2SPI_API MMP_RESULT
usb2spi_WriteMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte);

//=============================================================================
/**
 * Castor open
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
USB2SPI_API MMP_INT
usb2spi_Open(
    void);

//=============================================================================
/**
 * Castor close
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
USB2SPI_API MMP_INT
usb2spi_Close(
    void);

#ifdef __cplusplus
}
#endif

#endif /* CASTOR_H */
