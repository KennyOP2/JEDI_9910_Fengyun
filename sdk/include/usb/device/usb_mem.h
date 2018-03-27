/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as USB memory related header file.
 */

#ifndef USB_MEM_H
#define USB_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
static MMP_INT
USB_DEVICE_AllocateDmaAddr(
    MMP_UINT8** addr,
    MMP_UINT32  size);

static MMP_INT
USB_DEVICE_ReleaseDmaAddr(
    void);



#ifdef __cplusplus
}
#endif

#endif //
