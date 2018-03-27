 //*****************************************************************************
// Name: ramdisk.h
//
// Description:
//     ram disk
//
// Author: Powei
// Date: 2007/08/07
//
// Copyright(c)2007-2010 SMedia Corp. All rights reserved.
//*****************************************************************************

#ifndef __MMP_FILE_H
#define __MMP_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================	
//=============================================================================
//                              Constant Definition
//=============================================================================
//=============================================================================
//                              Function Declaration
//=============================================================================	
int 
RAMDISK_Initialize(void);    

int 
RAMDISK_Terminate(void);    

int
RAMDISK_GetCapacity(unsigned int* lastBlockId, unsigned int* blockLength);

int
RAMDISK_WriteSector(unsigned int blockId, unsigned int sizeInSector, unsigned short* srcBuffer);

int
RAMDISK_ReadSector(unsigned int blockId, unsigned int sizeInSector, unsigned short* destBuffer);


#ifdef __cplusplus
}
#endif

#endif
