//*****************************************************************************
// Name: mmc_controller.h
//
// Description:
//     MMC controller's api functions head file
//
// Author: Arcane Liu
//  Date: 2003/10/14
//
// Copyright(c)2003-2004 Silicon Integrated Systems Corp. All rights reserved.
//*****************************************************************************
#ifndef __MMC_CONTROLLER_H
#define __MMC_CONTROLLER_H

//#include "mmp_globalDef.h"
//#include "mmp_api.h"
#include "mmc_spec.h"
#include "mmp_types.h"
#include "mmp.h"
#include "mmp_mmc.h"
//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#ifdef __cplusplus
extern "C"
{
#endif // End of #ifdef __cplusplus

    /*
typedef
void
(*PMMC_INT_CALLBACK) (
    void);
*/


//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//===============
// mmc_controller.c
//===============
MMP_BOOL
InitialMMCController(
    void);

MMP_BOOL
AcquireCardsUpdate(
    void);

MMP_BOOL
AcquireSingleCardUpdate(
    void);

MMP_BOOL
InitialCardStack(
    MMP_BOOL bSingleCard);

MMP_BOOL
CheckCardsStack(
    MMP_UINT16 wCarCounter,
    MMP_UINT16 wRemovedCardIdx);

MMP_BOOL
SetupCardConnection(
    MMP_UINT32 dwRCA);

MMP_BOOL
StartStreamRead(
    MMP_UINT32 dwRCA,
    MMP_UINT32 dwReadBaseAddr);

MMP_BOOL
StopStreamRead(
    void);

MMP_BOOL
MMC_ReadBlock(
    MMP_UINT32 dwRCA,
    MMP_UINT32 dwReadBaseAddr,
    MMP_UINT32 dwReadBlockLen,
    MMP_UINT32 dwBlockCounter);

MMP_BOOL
MMC_WriteBlock(
    MMP_UINT32 dwRCA,
    MMP_UINT32 dwWriteBaseAddr,
    MMP_UINT32 dwWriteBlockLen,
    MMP_UINT32 dwBlockCounter);

#if !defined(__FREERTOS__)
void
MMCInterruptHandler(
    PMMC_INT_CALLBACK callBackFunc);
#endif

MMP_BOOL
GetMMCCardInfo(
MMP_UINT32 dwCardRCAId,
PMMC_CSD_DATA pMMC_CSD_DATA);

MMP_BOOL
ReadMMCCapacity(
    MMP_UINT32 dwCardId,
    MMP_UINT32* pdwLastBlockAddress,
    MMP_UINT32* pdwBlockLength);

void
API_MMP_ALLOCATE_MMC_BUF(
    MMP_UINT32 dwBufferSizeInByte,
    MMP_UINT8 *pbyStatus);

void
API_MMP_RELEASE_MMC_BUF(
    MMP_UINT8 *pbyStatus);

MMP_BOOL
Api_Clean_MMC_Interrupt(void);

void
Allocate_MMC_Buf(
    MMP_UINT8 *pbyStatus);

MMP_BOOL
Api_WriteDataToMMC(
    MMP_UINT32 dwMMCAddresss,
    MMP_UINT32 dwTransferLengthInByte,
    MMP_UINT16 *pwSrcBuf);

MMP_BOOL
Api_ReadDataFromMMC(
    MMP_UINT32 dwMMCAddresss,
    MMP_UINT32 dwTransferLengthInByte,
    MMP_UINT16 *pwDestBuf);


MMP_BOOL
Api_Polling_MMC_RW_Response(
	void);

#ifdef __cplusplus
}
#endif // End of #ifdef __cplusplus

#endif // End of #ifndef __MMC_CONTROLLER_H


