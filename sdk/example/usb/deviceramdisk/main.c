/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "mmp_usbex.h"
//#include "mmp_sd.h"
#include "stdio.h"
#include "ramdisk.h"
#include "common/fat.h" /** just for link error */
#include "pal/pal.h"
#include "config.h"

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif

enum
{
    RAM_DISCK,
    MAX_LUN_NUM,
    SMEDIA_SD,
};

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

static MMP_BOOL cardReady[MAX_LUN_NUM] = {0};
static MMP_BOOL cardEject[MAX_LUN_NUM] = {0};

#define INQUIRY_LENGTH          36     /*36Byte*/
MMP_UINT8 INQUIRY_TABLE_SD[INQUIRY_LENGTH]={
    0x00,               /*Qualifier, device type code*/
    0x80,               /*RMB, device type modification child*/
    0x00,               /*ISO Version, ECMA Version, ANSI Version*/
    0x01,               /*Response data form*/
    0x1F,               /*addition data length*/            
    0x00,0x00,0x00,     /*reserved*/
    'G','e','n','e','r','i','c',' ',                                    /*vender ID*/
    'S','t','o','r','a','g','e',' ','S','D','/','M','M','C',' ',' ',    /*product ID*/
    '1','.','0','0'                                                     /*Product Revision*/
};

MMP_UINT8 INQUIRY_TABLE_RAMDISC[INQUIRY_LENGTH]={
    0x00,               /*Qualifier, device type code*/
    0x80,               /*RMB, device type modification child*/
    0x00,               /*ISO Version, ECMA Version, ANSI Version*/
    0x01,               /*Response data form*/
    0x1F,               /*addition data length*/            
    0x00,0x00,0x00,     /*reserved*/
    'G','e','n','e','r','i','c',' ',                                    /*vender ID*/
    'S','t','o','r','a','g','e',' ','R','a','m','D','i','s','c',' ',    /*product ID*/
    '1','.','0','0'                                                     /*Product Revision*/
};


MMP_INT
CARD_GetMaxLunNum(
    MMP_UINT8* lunNum)
{
    MMP_INT result = 0;

    (*lunNum) = MAX_LUN_NUM - 1;

    return result;
}


MMP_INT
_CARD_Initialize(
    MMP_UINT8 lun) // lun is meaningful
{
    MMP_INT result = 0;

    switch(lun)
    {
    case SMEDIA_SD:
        //result = mmpSdInitialize();
        if(result)
            goto end;
        break;
    case RAM_DISCK:
        RAMDISK_Initialize();
        break;
    default:
        break;
    };
    cardReady[lun] = MMP_TRUE;
    cardEject[lun] = MMP_FALSE;

end:
    return result;
}

MMP_INT
_CARD_Terminate(
    MMP_UINT8 lun) // lun is meaningful
{
    MMP_INT result = 0;

    switch(lun)
    {
    case SMEDIA_SD:
        //mmpSdTerminate();
        break;
    case RAM_DISCK:
        RAMDISK_Terminate();
        break;
    default:
        break;
    };
    cardReady[lun] = MMP_FALSE;

    return result;
}



MMP_INT
CARD_Initialize(
    MMP_UINT8 lun)
{
    MMP_INT result = 0;

    /*
    result = mmpSdInitialize();
    if(!result)
        cardReady[SMEDIA_SD] = MMP_TRUE;
    */

    RAMDISK_Initialize();
    cardReady[RAM_DISCK] = MMP_TRUE;

    return 0;
}

MMP_INT
CARD_Terminate(
    MMP_UINT8 lun)
{
    //mmpSdTerminate();
    RAMDISK_Terminate();
    memset(cardReady, 0x0, sizeof(cardReady));
    memset(cardEject, 0x0, sizeof(cardEject));

    return 0;
}

MMP_BOOL
CARD_Response(
    MMP_UINT8 lun)
{
    MMP_BOOL ready = MMP_FALSE;
    switch(lun)
    {
    case SMEDIA_SD:
        //ready = mmpSdGetCardState(INSERTED);
        break;
    case RAM_DISCK:
        ready = MMP_TRUE;
        break;
    default:
        ready =  MMP_FALSE;
        break;
    };

    /** for card insert/remove */
    if(ready != cardReady[lun])
    {
        if(ready)
            _CARD_Initialize(lun);
        else
            _CARD_Terminate(lun);
    }

    /** for card eject */
    if(cardEject[lun] == MMP_TRUE)
        ready = MMP_FALSE;

    return ready;
}

MMP_INT
CARD_Inquiry(
    MMP_UINT8 lun,
    MMP_UINT8** inquiryData)
{
    MMP_INT result = 0;

    switch(lun)
    {
    case SMEDIA_SD:
        (*inquiryData) = INQUIRY_TABLE_SD;
        break;

    case RAM_DISCK:
        (*inquiryData) = INQUIRY_TABLE_RAMDISC;
        break;

    default:
        (*inquiryData) = MMP_NULL;
        result = 1;
        break;
    };

    return result;
}

MMP_BOOL
CARD_IsLock(
    MMP_UINT8 lun)
{
    MMP_BOOL isLock = MMP_FALSE;

    switch(lun)
    {
    case SMEDIA_SD:
        isLock = MMP_FALSE;
        break;

    case RAM_DISCK:
        isLock = MMP_FALSE;
        break;

    default:
        isLock = MMP_TRUE;
        break;
    };

    return isLock;
}

MMP_INT
CARD_Eject(
    MMP_UINT8 lun)
{
    MMP_INT result = 0;

    cardEject[lun] = MMP_TRUE;

    return result;
}

MMP_INT
CARD_GetCapacity(
    MMP_UINT8   lun,
    MMP_UINT32* lastBlockId,
    MMP_UINT32* blockLength)
{
    MMP_INT result = 0;

    switch(lun)
    {
    case SMEDIA_SD:
        //result = mmpSdGetCapacity(lastBlockId, blockLength);
        (*lastBlockId) = (*lastBlockId) - 1;
        break;
    case RAM_DISCK:
        RAMDISK_GetCapacity(lastBlockId, blockLength);
        (*lastBlockId) = (*lastBlockId) - 1;
        break;
    default:
        (*lastBlockId) = 0;
        blockLength = 0;
        result = 1;
        break;
    };

    return result;
}

MMP_INT
CARD_ReadSector(
    MMP_UINT8   lun,
    MMP_UINT32  blockId, 
    MMP_UINT32  sizeInSector, 
    MMP_UINT16* srcBuffer)
{
    MMP_INT result = 0;

    switch(lun)
    {
    case SMEDIA_SD:
        //result = mmpSdReadMultiSector(blockId, sizeInSector,(void*)srcBuffer);
        break;
    case RAM_DISCK:
        RAMDISK_ReadSector(blockId, sizeInSector, srcBuffer);
        break;
    default:
        result = 1;
        break;
    };

    return result;
}

MMP_INT
CARD_WriteSector(
    MMP_UINT8   lun,
    MMP_UINT32  blockId, 
    MMP_UINT32  sizeInSector, 
    MMP_UINT16* dstBuffer)
{
    MMP_INT result = 0;

    switch(lun)
    {
    case SMEDIA_SD:
        //result = mmpSdWriteMultiSector(blockId, sizeInSector,(unsigned char*)dstBuffer);
        break;
    case RAM_DISCK:
        RAMDISK_WriteSector(blockId, sizeInSector, dstBuffer);
        break;
    default:
        result = 1;
        break;
    };

    return result;
}

static MMP_INT
Initialize(void)
{
    signed portBASE_TYPE ret = pdFAIL;
    xTaskHandle usb_task = NULL;

	MMP_INT result = 0;

    //<<=============== These codes are in main.c Initialize() =================
    /** initialize usb HC driver, and register mass storage driver */
    result = mmpUsbExInitialize();
    if(result)
    {
        printf(" mmpUsbExInitialize() error 0x%08X \n", result);
        while(1);
    }

    /** This is usb device driver task, and it will never be destroyed. */
    ret = xTaskCreate(DEVICE_ThreadFunc, "usb_device",
                        10*1024,
                        NULL, 
                        tskIDLE_PRIORITY + 3, 
                        &usb_task );
    if (pdFAIL == ret) 
    {
        printf(" Create USB device task fail~~ \n", result);
        while(1);
    }
    PalSleep(100);

    return result;
}

static const MMP_ULONG attribList[] =
{
    MMP_OTG_ATTRIB_DEVICE_INITIALIZE, (MMP_ULONG)CARD_Initialize,
    MMP_OTG_ATTRIB_DEVICE_TERMINATE, (MMP_ULONG)CARD_Terminate,
    MMP_OTG_ATTRIB_DEVICE_GET_CAPACITY, (MMP_ULONG)CARD_GetCapacity,
    MMP_OTG_ATTRIB_DEVICE_READ_SECTOR, (MMP_ULONG)CARD_ReadSector,
    MMP_OTG_ATTRIB_DEVICE_WRITE_SECTOR, (MMP_ULONG)CARD_WriteSector,
    MMP_OTG_ATTRIB_DEVICE_GET_MAX_LUN_NUM, (MMP_ULONG)CARD_GetMaxLunNum,
    MMP_OTG_ATTRIB_DEVICE_RESPONSE, (MMP_ULONG)CARD_Response,
    MMP_OTG_ATTRIB_DEVICE_INQUIRY, (MMP_ULONG)CARD_Inquiry,
    MMP_OTG_ATTRIB_DEVICE_IS_LOCK, (MMP_ULONG)CARD_IsLock,
    MMP_OTG_ATTRIB_DEVICE_EJECT, (MMP_ULONG)CARD_Eject,
    MMP_OTG_ATTRIB_NONE
};

static MMP_INT
MainLoop(
    void)
{
    MMP_INT result = 0;
    MMP_BOOL connected = MMP_FALSE;

    for(;;)
    {
        if(mmpOtgIsDeviceMode())
        {
            if(connected == MMP_FALSE)
            {
                LOG_INFO " Device mode connected!\n" LOG_END
                mmpOtgDeviceModeOpen(attribList);
                connected = MMP_TRUE;
            }
        }
        else
        {
            if(connected == MMP_TRUE)
            {
                LOG_INFO " Device mode disconnected!\n" LOG_END
                mmpOtgDeviceModeClose();
                connected = MMP_FALSE;
            }
        }

        PalSleep(1);
    }

    return result;
}

static MMP_INT
Terminate(void)
{
    MMP_INT result = 0;

    return result;
}

void DoTest(void)
{
    MMP_INT result;

	ithIntrInit();

    result = Initialize();
    if (result)
        goto end;

    result = MainLoop();
    if (result)
        goto end;

    result = Terminate();
    if (result)
        goto end;

end:
    return;
}

void main(int argc, char** argv)
{	
    MMP_INT result = 0;

#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    HOST_GetChipVersion();

    ret = xTaskCreate(main_task_func, "main",
        configMINIMAL_STACK_SIZE * 6,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    //=======================Important!!!=========================================
    // For RISC version.
    //
    // Solution: Call f_init() before PalThreadInitialize()
    // Call file system functions can fix link error. =___=|||
    //============================================================================

    f_init();
    if(PalThreadInitialize())
    {
        printf(" PalInitialize()() fail \n");
        while(1);
    }

    vTaskStartScheduler();
#endif
	
    DoTest();
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    DoTest();
}
#endif


