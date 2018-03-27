/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file mod_ctrl.c
 * Used to control modulator.
 *
 * @author Barry Wu
 * @version 0.1
 */

//#include "pal/pal.h"
//#include "sys/sys.h"
//#include "mmp_iic.h"
//#include "IT9507.h"
#include "mod_ctrl.h"
#include "mmp_usbex.h"

//=============================================================================
//				  Constant Definition
//=============================================================================
//#define EAGLE_9507_ADDR     0x38

//=============================================================================
//				  Structure Definition
//=============================================================================
//typedef struct MOD_EAGLE_INFO_TAG
//{
//    Eagle                 EagleModulator;
//    MMP_BOOL              bFilterOn;
//} MOD_EAGLE_INFO;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MOD_EAGLE_INFO    *pModulatorInfo = MMP_NULL;
static USB_DEVICE_INFO    usb_mod_device[2]; 

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================
//extern USB_DEVICE_INFO usb_mod_device[2]; 

//=============================================================================
/**
 * Lock I2C module for modulator usage
 * return none.
 */
//=============================================================================
static void
_MODCTRL_LockIic(
    void)
{
    // Switch PAD_Sel Bit 5 to switch IIC mode to Keyboard mode to prevent
    // internal IIC signal spill out.
    //mmpIicLockModule();
    return;
}

//=============================================================================
/**
 * Release I2C module for other module usage
 * return none.
 */
//=============================================================================
static void
_MODCTRL_ReleaseIic(
    void)
{
    //mmpIicReleaseModule();
    return;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
void*
UsbMod_ThreadFunc(
    void* arg)
{
    MMP_UINT i = 0;

    for (;;)
    {
        MMP_ULONG           sleepTime  = 200; // ms
        MMP_INT             usb_state = 0;
        USB_DEVICE_INFO     device_info = {0};

        // polling card
        if (mmpUsbExCheckDeviceState(USB0, &usb_state, &device_info) == MMP_RESULT_SUCCESS)
        {  
            if (USB_DEVICE_CONNECT(usb_state) == MMP_TRUE)
            {
                usb_mod_device[0].type = device_info.type;
                usb_mod_device[0].ctxt = device_info.ctxt;
                
                printf(" Eagle modulator (USB0) is inserted !!!!!!!!!!! \n");
                break;
            }
        }
        PalSleep(sleepTime);
    }

}     

//=============================================================================
/**
 * Used to Init the modulator.
 * @return none.
 */
//=============================================================================
void
ModCtrl_Init(Modulator* modinfo, MMP_UINT8* pExternalIQ, MMP_UINT32 tableSize)
{
    Dword result = ModulatorError_NO_ERROR;
    IQtable*   pIQTable = MMP_NULL;
    MMP_UINT8* pIQBuf = MMP_NULL;
    MMP_UINT32 iqTableCount = 0;
    MMP_UINT32 frequency = 0;
    MMP_UINT16 amp;
    MMP_UINT16  phi;
    MMP_UINT32 i;
    MMP_UINT8* pCurPos = MMP_NULL;

    _MODCTRL_LockIic();

    pModulatorInfo = (MOD_EAGLE_INFO*)malloc(sizeof(MOD_EAGLE_INFO));
    if( !pModulatorInfo )
    {
        printf("Memory allocation is failed for modulator Eagle info\n");
    } 
    memset((void*)pModulatorInfo, 0x0, sizeof(MOD_EAGLE_INFO));

    if (modinfo->busId == Bus_I2C)
    {
        pModulatorInfo->EagleModulator.busId = modinfo->busId;
        pModulatorInfo->EagleModulator.tsInterfaceType = SERIAL_TS_INPUT;
    }
    else
    {
        pModulatorInfo->EagleModulator.busId = modinfo->busId;
        pModulatorInfo->EagleModulator.tsInterfaceType = SERIAL_TS_INPUT;

        do{
            PalSleep(50);
          //if( sleepTime > 2500 )
          //{
              
          //    printf("Wait Eagle Usb Modulator insert time out !! ");
          //    break;
          //}
          //sleepTime+=50;
        }while( !usb_mod_device[0].ctxt );
    }
    
    pModulatorInfo->EagleModulator.usb_info = usb_mod_device[0].ctxt;

    result = EagleUser_setBus((Modulator*) &(pModulatorInfo->EagleModulator), pModulatorInfo->EagleModulator.busId, EagleUser_IIC_ADDRESS);
    if( result != ModulatorError_NO_ERROR )
    {
        printf("modulator set bus is failed: reason - 0x%X\n", result);
    }

    result = IT9507_initialize ((Modulator*) &(pModulatorInfo->EagleModulator), pModulatorInfo->EagleModulator.tsInterfaceType, pModulatorInfo->EagleModulator.busId, EagleUser_IIC_ADDRESS);
    if( result != ModulatorError_NO_ERROR )
    {
        printf("modulator init is failed: reason - 0x%X\n", result);
    }
    else
    {
        if (pExternalIQ && tableSize > 16)
        {
            iqTableCount = pExternalIQ[14] << 8 | pExternalIQ[15];
            printf("iqCount: %u\n", iqTableCount);
            if (iqTableCount)
            {
                pIQBuf  = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(IQtable) * iqTableCount);
                printf("pIQ StartPos: 0x%X\n", pIQBuf);
                PalMemset(pIQBuf, 0x0, sizeof(IQtable) * iqTableCount);
                pCurPos = &pExternalIQ[16];
                for (i = 0; i < iqTableCount; i++, pCurPos += 8)
                {
                    pIQTable = (IQtable*) (pIQBuf + sizeof(IQtable) * i);
                    pIQTable->frequency = pCurPos[3] << 24 | pCurPos[2] << 16 | pCurPos[1] << 8 | pCurPos[0];
                    pIQTable->dAmp = (MMP_INT16) (pCurPos[5] << 8 | pCurPos[4]);
                    pIQTable->dPhi = (MMP_INT16) (pCurPos[7] << 8 | pCurPos[6]);
                    printf("%d, addr: 0x%X, frequency: %u, dAmp: %d, dPhi: %d\n",
                           i, pIQTable, pIQTable->frequency, pIQTable->dAmp, pIQTable->dPhi);
                }
                IT9507_setIQtable(&(pModulatorInfo->EagleModulator), (IQtable*) pIQBuf);
            }
        }
        printf("Eagle modulator init successfully\n");
    }
    _MODCTRL_ReleaseIic();
}

//=============================================================================
/**
 * Used to Terminate the modulator.
 * @return none.
 */
//=============================================================================
void
ModCtrl_Terminate(void)
{
    Dword result = ModulatorError_NO_ERROR;

    if (pModulatorInfo != MMP_NULL)
    {
        _MODCTRL_LockIic();

        result = IT9507_finalize ((Modulator*) &(pModulatorInfo->EagleModulator));
        
        _MODCTRL_ReleaseIic();

        if( result != ModulatorError_NO_ERROR )
        {
            printf("modulator reset is failed: reason - 0x%X\n", result);
        }
       
        free(pModulatorInfo);
        pModulatorInfo = MMP_NULL;
    }
    
}

//=============================================================================
/**
 * Used to set modulator's TX Channel Modulation.
 * @return none.
 */
//=============================================================================
MMP_RESULT
ModCtrl_setTXChannelModulation(ChannelModulation* channelModulation)
{
    Dword result = ModulatorError_NO_ERROR;    

    _MODCTRL_LockIic();
    
    result = IT9507_setTXChannelModulation((Modulator*) &(pModulatorInfo->EagleModulator), channelModulation);
    
    _MODCTRL_ReleaseIic();
    
    if (result) {
        printf ("set TXChannelModulation Error = 0x%X", result);
        return result;
    }    
}

//=============================================================================
/**
 * Used to acquire modulator's TX Channel.
 * @return none.
 */
//=============================================================================
MMP_RESULT
ModCtrl_acquireTxChannel(ChannelModulation* channelModulation)
{
    Dword result = ModulatorError_NO_ERROR;

    _MODCTRL_LockIic();
    
    result = IT9507_acquireTxChannel ((Modulator*) &(pModulatorInfo->EagleModulator), channelModulation->bandwidth, channelModulation->frequency);
    
    _MODCTRL_ReleaseIic();
    
    if (result) {
        printf ("acquire channel Error = 0x%X", result);
        return result;
    }    
}

//=============================================================================
/**
 * Used to enable/disable data transmission.
 * @return none.
 */
//=============================================================================
void
ModCtrl_setTxModeEnable(void)
{
    Dword result = ModulatorError_NO_ERROR;

    _MODCTRL_LockIic();
    
    result = IT9507_setTxModeEnable((Modulator*) &(pModulatorInfo->EagleModulator), 1);
    
    _MODCTRL_ReleaseIic();
    
    if (result) {
        printf ("Enable TX Error = 0x%X", result);
    }    
}

//=============================================================================
/**
 * Used for usb transfer data.
 * @return none.
 */
//=============================================================================
MMP_RESULT
ModCtrl_Usb_TransferDataStream(
    MMP_UINT8*     buffer,
    MMP_UINT32     bufferlength)
{
    Dword result = ModulatorError_NO_ERROR;
    
    result = EagleUser_busTxData((Modulator*) &(pModulatorInfo->EagleModulator), 
                                 (Dword) bufferlength,
                                 (Byte*) buffer);
    if (result) {
        printf ("usb transfer data Error = 0x%X", result);
    }
    return result;
}

//=============================================================================
/**
 * Used to adjuste digital output gain.
 * @return status and real gain.
 */
//=============================================================================
MMP_RESULT
ModCtrl_adjustOutputGain(
    MMP_INT* gain)
{
    Dword result = ModulatorError_NO_ERROR;

    _MODCTRL_LockIic();
    
    result = IT9507_adjustOutputGain((Modulator*) &(pModulatorInfo->EagleModulator), gain);
    if (result) {
        printf ("adjuste digital output gain Error = 0x%X", result);
    }
    else
    {
        printf("adjuste digital output gain to %ddB\n", *gain);
    }
    _MODCTRL_ReleaseIic();
}

