/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file xcpu_io.h
 *
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef BUS_H
#define BUS_H

#include "pal\pal.h"

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

MMP_UINT16
xCpuIO_ReadRegister(
    MMP_UINT16 addr);

void
xCpuIO_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data);

void
xCpuIO_ReadMemory(
    MMP_UINT32 destSysRamAddress,
    MMP_UINT32 srcVRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_WriteMemory(
    MMP_UINT32 destVRamAddress,
    MMP_UINT32 srcSysRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_ReadMemoryUInt16(
    MMP_UINT32 destSysRamAddress,
    MMP_UINT32 srcVRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_WriteMemoryUInt16(
    MMP_UINT32 destVRamAddress,
    MMP_UINT32 srcSysRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_ReadMemoryUInt32(
    MMP_UINT32 destSysRamAddress,
    MMP_UINT32 srcVRamAddress,
    MMP_UINT32 sizeInByte);

void
xCpuIO_WriteMemoryUInt32(
    MMP_UINT32 destVRamAddress,
    MMP_UINT32 srcSysRamAddress,
    MMP_UINT32 sizeInByte);

#ifdef __cplusplus
}
#endif

#endif
