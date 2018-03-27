/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file mod_ctrl.c
 * Used to control modulator.
 *
 * @author Barry Wu
 * @version 0.1
 */

#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_iic.h"
#include "IT9507/IT9507.h"

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct MOD_EAGLE_INFO_TAG
{
    Modulator             EagleModulator;
    MMP_BOOL              bFilterOn;
} MOD_EAGLE_INFO;

void
ModCtrl_Init(Modulator* modinfo, MMP_UINT8* pExternalIQ, MMP_UINT32 tableSize);

void
ModCtrl_Terminate(void);

//=============================================================================
/**
 * Used to set modulator's TX Channel Modulation.
 * @return none.
 */
//=============================================================================
MMP_RESULT
ModCtrl_setTXChannelModulation(ChannelModulation* channelModulation);

//=============================================================================
/**
 * Used to acquire modulator's TX Channel.
 * @return none.
 */
//=============================================================================
MMP_RESULT
ModCtrl_acquireTxChannel(ChannelModulation* channelModulation);

//=============================================================================
/**
 * Used to enable/disable data transmission.
 * @return none.
 */
//=============================================================================
void
ModCtrl_setTxModeEnable(void);

//=============================================================================
/**
 * Used for usb transfer data.
 * @return none.
 */
//=============================================================================
MMP_RESULT
ModCtrl_Usb_TransferDataStream(
    MMP_UINT8*     buffer,
    MMP_UINT32     bufferlength);

//=============================================================================
/**
* Used to adjuste digital output gain.
* @return status and real gain.
*/
//=============================================================================
MMP_RESULT
ModCtrl_adjustOutputGain(
    MMP_INT* gain);

