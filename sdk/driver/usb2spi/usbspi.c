/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Caster operation API
 *
 * @author I-Chun Lai
 * @version 1.0
 */

#include "stdio.h"
#include "pal\pal.h"
#include "usb2spi\spi.h"
#include "usb2spi\bus.h"
#include "usb2spi\usb2spi.h"
#include <windows.h>
//=============================================================================
//                              Constant Definition
//=============================================================================
//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
CRITICAL_SECTION    CriticalSection;
//=============================================================================
//                              External Function Declaration
//=============================================================================


//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Read Castor Register
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
usb2spi_ReadRegister(
    MMP_UINT16 addr,
    MMP_UINT16 *value)
{
    MMP_RESULT result = MMP_SUCCESS;

    EnterCriticalSection(&CriticalSection);
    *value = xCpuIO_ReadRegister(addr);
    LeaveCriticalSection(&CriticalSection);

    return result;
}

//=============================================================================
/**
 * Read Castor Register
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
usb2spi_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 value)
{
    MMP_RESULT result = MMP_SUCCESS;

    EnterCriticalSection(&CriticalSection);
    xCpuIO_WriteRegister(addr, value);
    LeaveCriticalSection(&CriticalSection);

    return result;
}

//=============================================================================
/**
 * Read Castor Memory
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
usb2spi_ReadMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    MMP_RESULT result = MMP_SUCCESS;

    EnterCriticalSection(&CriticalSection);
    xCpuIO_ReadMemory(destAddress, srcAddress, sizeInByte);
    LeaveCriticalSection(&CriticalSection);

    return result;
}

//=============================================================================
/**
 * Write Castor Memory
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
usb2spi_WriteMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    MMP_RESULT result = MMP_SUCCESS;

    EnterCriticalSection(&CriticalSection);
    xCpuIO_WriteMemory(destAddress, srcAddress, sizeInByte);
    LeaveCriticalSection(&CriticalSection);

    return result;
}


//=============================================================================
/**
 * Castor open
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_INT
usb2spi_Open(
    void)
{
    MMP_INT result;
    result =  SpiOpen(2);
    InitializeCriticalSection(&CriticalSection);
    return result;
}

//=============================================================================
/**
 * Castor close
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_INT
usb2spi_Close(
    void)
{
    MMP_INT result;
    
    result = SpiClose();
    DeleteCriticalSection(&CriticalSection);
    return result;
}