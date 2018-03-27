#include "pal/pal.h"
#include "mmp_types.h"
#include "common/fat.h"
#include "mmp.h"
#include "../sdk/share/fat/ftl/ftldrv.h"
#include "../sdk/share/fat/mmc/single/mmc.h"

//#include "type.h"
#include "storage_mgr.h"

/** card driver **/
#include "../sdk/share/fat/mmc/single/mmc_smedia.h"
//#include "nordrv_f.h"
//#include "mmp_nor.h"
#include "host/ahb.h"
#include "mmp_sd.h"
#ifdef DTV_MS_ENABLE
    #include "mmp_mspro.h"
#endif

#ifdef DTV_USB_ENABLE
    #include "mmp_usbex.h"
    #include "mmp_msc.h"
#endif
//#include "mmp_ptpfile.h"
#include "msg_core.h"

//=============================================================================
//                              Macro Definition
//=============================================================================

#define _f_toupper(ch) (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))

//=============================================================================
//                              Structure Definition
//=============================================================================

/** Card Current Status **/
typedef enum DTV_CARD_STATUS_TAG
{
    DTV_CARD_STATUS_REMOVE = 0,
    DTV_CARD_STATUS_INSERT,
    DTV_CARD_STATUS_FAIL_CARD,
    DTV_CARD_STATUS_INTERNAL,
    DTV_CARD_STATUS_USB_WAITING,
    DTV_CARD_STATUS_USB_UNSUPPORT,
} DTV_CARD_STATUS;

typedef enum DEV_MODE_CHK_STATE_TAG
{
    DEV_MODE_CHK_STATE_IN_STANDALONE_MODE,
    DEV_MODE_CHK_STATE_IN_DEVICE_MODE,
    DEV_MODE_CHK_STATE_IN_CHARG_MODE,
} DEV_MODE_CHK_STATE;

/** Dir Table **/
typedef struct DTV_DIR_TABLE_TAG
{
    MMP_INT  srcDirIndex;
    MMP_INT  totalSubDir;
    MMP_INT8 dirLevel;
    MMP_UINT fileCountInDir[DTV_FILE_TYPE_COUNT];
    SF_POS   position;                              // the position info in storage
} DTV_DIR_TABLE;

/** Base File Table **/
typedef struct DTV_BASE_FILE_TABLE_TAG
{
    MMP_UINT16 dirIndex;
    SF_POS     position;                            // the position info in storage
} DTV_BASE_FILE_TABLE;

typedef struct DTV_MAPPING_FILE_INDEX_TABLE_TAG
{
    MMP_UINT fileIndex;
} DTV_MAPPING_FILE_INDEX_TABLE;

typedef struct DTV_CARD_EVENT_INFO_TAG
{
    MMP_BOOL       bRefresh;
    DTV_CARD_EVENT cardEvent;
} DTV_CARD_EVENT_INFO;

typedef struct DTV_VOLUME_INFO_TAG
{
    FILE_SYSTEM_FORMAT           fileSystemFormat;
    MMP_BOOL                     bVolumeUsed;
    MMP_WCHAR                    *startPath;

    MMP_UINT8                    cardType;
    MMP_UINT8                    partitionIdx;
    MMP_UINT16                   dirCount;
    DTV_DIR_TABLE                *dirTable[DTV_MAX_DIR_COUNT / DTV_SUB_DIR_COUNT];
    MMP_UINT16                   fileCount[DTV_FILE_TYPE_COUNT];
    MMP_UINT16                   totalFileCount;
    DTV_MAPPING_FILE_INDEX_TABLE *mappingFileIndexTable[DTV_FILE_TYPE_COUNT];
    DTV_BASE_FILE_TABLE          *baseFileTable[DTV_FILE_TYPE_COUNT][DTV_MAX_FILE_COUNT / DTV_SUB_FILE_TABLE_COUNT];

    MMP_BOOL                     bReadyCollectVolumeInfo;
    MMP_BOOL                     bInCollectStatus;
    MMP_BOOL                     bRecursiveSearch;
} DTV_VOLUME_INFO;

typedef struct DTV_DEVICE_HANDLE_TAG
{
    MMP_BOOL   bUsed;           // for performace (indicate if we support this card type or not)
    MMP_BOOL   bInsert;         // card status (keep insert or not)
    MMP_BOOL   bInitCard;       // card is initialized or not (driver layer)
    MMP_BOOL   bFailCard;       // for send fail_card_msg
    MMP_UINT16 retryCount;
    MMP_BOOL   bInitialVolume;  // volume is initialized or not (FAT layer)
    MMP_UINT8  totalVolume;
} DTV_DEVICE_HANDLE;

typedef struct DTV_STORAGE_MGR_TAG
{
    DTV_VOLUME_INFO     volumeInfo[DTV_VOLUME_NUM];             // 0 -> internal private; 1 -> internal public; other -> inserted card
    DTV_DEVICE_HANDLE   deviceHandle[PAL_DEVICE_TYPE_COUNT];

    MMP_BOOL            bIsDeviceMode;
    MMP_UINT            deviceModeChkState;
    MMP_BOOL            bDBCreating;

    MMP_EVENT           eventMgrToThread;
    MMP_EVENT           eventThreadToMgr;

    MMP_BOOL            bStartSearch;
    MMP_BOOL            bInterrupt;

    PAL_DEVICE_TYPE     currCardType;
    MMP_BOOL            bUsbSuspendMode;

#ifdef ENABLE_STORAGE_THREAD_POLLING
    DTV_CARD_EVENT_INFO cardEventInfo[PAL_DEVICE_TYPE_COUNT + 1];   // the last info for saving client USB
#endif

    MMP_BOOL            bInitialized;
    MMP_UINT32          initDuration[PAL_DEVICE_TYPE_COUNT];        // some usb pen disk need to initial 800 ms
} DTV_STORAGE_MGR;

////////////////////////////////////////////////
#define END_OF_STORAGE_DEVICE_TABLE { PAL_DEVICE_TYPE_RESERVED, NULL }

typedef struct STORAGE_DEVICE_TAG
{
    MMP_UINT cardType;
    void     *f_dev_init;
} STORAGE_DEVICE;

static const STORAGE_DEVICE g_storage_device[] = {
#ifdef DTV_SD1_ENABLE
    { PAL_DEVICE_TYPE_SD,   sd_initfunc   },
#endif
#ifdef DTV_SD2_ENABLE
    { PAL_DEVICE_TYPE_SD2,  sd2_initfunc  },
#endif
#ifdef DTV_MS_ENABLE
    { PAL_DEVICE_TYPE_MS,   ms_initfunc   },
#endif
#ifdef DTV_MMC_ENABLE
    { PAL_DEVICE_TYPE_MMC,  sd_initfunc   },
#endif
#ifdef DTV_USB_ENABLE
    { PAL_DEVICE_TYPE_USB0, usb0_initfunc },
    { PAL_DEVICE_TYPE_USB1, usb1_initfunc },
    { PAL_DEVICE_TYPE_USB2, usb2_initfunc },
    { PAL_DEVICE_TYPE_USB3, usb3_initfunc },
    { PAL_DEVICE_TYPE_USB4, usb4_initfunc },
    { PAL_DEVICE_TYPE_USB5, usb5_initfunc },
    //{ PAL_DEVICE_TYPE_USB6, usb6_initfunc },
    //{ PAL_DEVICE_TYPE_USB7, usb7_initfunc },
#endif
    END_OF_STORAGE_DEVICE_TABLE
};
////////////////////////////////////////////////////////////////

static DTV_STORAGE_MGR      storageMgr = {0};

#ifdef DTV_USB_ENABLE
USB_DEVICE_INFO             usb_device[2] = {0};                    /** 0: For Host+Device, 1: For Host only */
MMP_BOOL                    ptp_device_connect = MMP_FALSE;         //For PTP Use
#endif

////////////////////////////////////////////////////////////////////////
// Private Function Definition
///////////////////////////////////////////////////////////////////////

static void
_storageMgrPrintString(
    MMP_CHAR    *prefixTxt,
    MMP_WCHAR   *string)
{
    MMP_INT i;

    if (prefixTxt)
        dbg_msg(DBG_MSG_TYPE_INFO, "%s", prefixTxt);

    for (i = 0; i < PalWcslen(string); i++)
        dbg_msg(DBG_MSG_TYPE_INFO, "%c", (char)string[i]);
    dbg_msg(DBG_MSG_TYPE_INFO, "\n");
}

#ifdef __FREERTOS__

static void *
get_initfunc(
    PAL_DEVICE_TYPE cardType)
{
    void    *f_dev_init = MMP_NULL;
    MMP_INT i;

    for (i = 0; g_storage_device[i].cardType != PAL_DEVICE_TYPE_RESERVED; ++i)
    {
        if (g_storage_device[i].cardType == cardType)
        {
            f_dev_init = g_storage_device[i].f_dev_init;
        }
    }

    return f_dev_init;
}

static MMP_INT
_InitVolume(
    MMP_UINT volume,
    PAL_DEVICE_TYPE cardType,
    MMP_UINT partitionNum)
{
    MMP_INT result      = 0;
    void    *f_dev_init = get_initfunc(cardType);

    if (!f_dev_init)
        return MMP_RESULT_ERROR;

    result = PalInitVolume(volume, f_dev_init, cardType, partitionNum);
    #ifndef CONFIG_HAVE_NTFS
    if (!result)
        storageMgr.deviceHandle[cardType].totalVolume = (MMP_UINT8)PalGetPartitionCount(cardType, f_dev_init);
    #endif

    return result;
}

#endif /* #ifdef __FREERTOS__ */

static DTV_CARD_STATUS
_IsXdCardInsert(
    void)
{
#if 1
    return DTV_CARD_STATUS_REMOVE;
#else
    MMP_UINT32 xdCardDetectPin   = 0x00;
    MMP_UINT32 xdCardPowerEnable = 0x00;
    MMP_BOOL   bUseGpio          = MMP_FALSE;
    MMP_UINT32 gpioPin           = 0;
    MMP_BOOL   bXdInsert         = MMP_FALSE;

    HOST_GetXDGPIO(&xdCardPowerEnable, &xdCardDetectPin);
    bUseGpio = xdCardDetectPin & 0x80000000;
    gpioPin  = xdCardDetectPin & 0x7FFFFFFF;

    if (bUseGpio)
    {   // GPIO
        MMP_UINT32 gpioPos  = 0;
        MMP_UINT16 readReg  = 0x50;
        MMP_UINT16 regValue = 0x00;
        MMP_UINT32 offset   = 0;

        {
            MMP_UINT32 i = 0;
            for (i = 0; i < 31; i++)
            {
                if ( (gpioPin >> i) & 0x01)
                {
                    gpioPos = i;
                }
            }
        }

        switch (gpioPos)
        {
        case 0:
            readReg = 0x0050;
            offset  = 12;
            break;

        case 1:
            readReg = 0x0050;
            offset  = 13;
            break;

        case 2:
            readReg = 0x0050;
            offset  = 14;
            break;

        case 3:
            readReg = 0x0050;
            offset  = 15;
            break;

        case 4:
            readReg = 0x0052;
            offset  = 12;
            break;

        case 5:
            readReg = 0x0052;
            offset  = 13;
            break;

        case 6:
            readReg = 0x0052;
            offset  = 14;
            break;

        case 7:
            readReg = 0x0052;
            offset  = 15;
            break;

        case 8:
            readReg = 0x0054;
            offset  = 12;
            break;

        case 9:
            readReg = 0x0054;
            offset  = 13;
            break;

        case 10:
            readReg = 0x0054;
            offset  = 14;
            break;

        case 11:
            readReg = 0x0054;
            offset  = 15;
            break;

        case 12:
            readReg = 0x0056;
            offset  = 12;
            break;
        }

        // Set GPIO to "Select GPIO pin"
        HOST_WriteRegisterMask(readReg, 0x0001 << offset, 0x0001 << offset);

        // Set GPIO to "Disable output mode (default)"
        if (readReg == 0x0056)
        {
            HOST_WriteRegisterMask(readReg, 0x0001, 0x0001);
        }
        else
        {
            HOST_WriteRegisterMask(readReg, 0x0001 << (offset - 12), 0x0001 << (offset - 12));
        }

        HOST_ReadRegister(readReg, &regValue);

        if (readReg == 0x0056)
        {
            if (regValue & 0x0100)
            {
                bXdInsert = MMP_FALSE;
            }
            else
            {
                bXdInsert = MMP_TRUE;
            }
        }
        else
        {
            if ( (regValue & 0x0F00) & (0x0001 << (offset - 4)) )
            {
                bXdInsert = MMP_FALSE;
            }
            else
            {
                bXdInsert = MMP_TRUE;
            }
        }
    }
    else
    {       // DGPIO
        //#define GPIO_BASE       0x68000000
        //#define GPIO_DATAIN_REG 0x4

        MMP_UINT32 data = 0;

        switch (gpioPin)
        {
        case MMP_GPIO0:
        case MMP_GPIO1:
        case MMP_GPIO2:
            break;

        case MMP_GPIO3:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, MMP_GPIO1, MMP_GPIO1);
            break;

        case MMP_GPIO4:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, MMP_GPIO2, MMP_GPIO2);
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, MMP_GPIO15);
            break;

        case MMP_GPIO5:
        case MMP_GPIO6:
        case MMP_GPIO7:
        case MMP_GPIO8:
        case MMP_GPIO9:
        case MMP_GPIO10:
        case MMP_GPIO11:
        case MMP_GPIO12:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, (MMP_GPIO7 | MMP_GPIO11));
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, MMP_GPIO3, MMP_GPIO3);
            break;

        case MMP_GPIO13:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, MMP_GPIO8);
            break;

        case MMP_GPIO14:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, MMP_GPIO12);
            break;

        case MMP_GPIO15:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, MMP_GPIO13);
            break;

        case MMP_GPIO16:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, MMP_GPIO14);
            break;

        case MMP_GPIO17:
        case MMP_GPIO18:
        case MMP_GPIO19:
        case MMP_GPIO20:
        case MMP_GPIO21:
            break;

        case MMP_GPIO22:
            AHB_WriteRegisterMask(GPIO_BASE + 0x48, 0, MMP_GPIO9);
            break;

        case MMP_GPIO23:
            break;

        default:
            break;
        }

        AHB_WriteRegisterMask(GPIO_BASE + 0x8, 0, gpioPin);
        AHB_ReadRegister(GPIO_BASE + GPIO_DATAIN_REG, &data);

        if (data & gpioPin)
        {
            bXdInsert = DTV_CARD_STATUS_REMOVE;
        }
        else
        {
            bXdInsert = DTV_CARD_STATUS_INSERT;
        }

        //#undef GPIO_BASE
        //#undef GPIO_DATAIN_REG
    }

    return bXdInsert;
#endif
}

#ifdef DTV_MS_ENABLE
static DTV_CARD_STATUS
_IsMSCardInsert(
    void)
{
    MMP_INT         result     = 0;
    PAL_DEVICE_TYPE sharePinCardType;
    DTV_CARD_STATUS cardStatus = DTV_CARD_STATUS_REMOVE;

    #if defined(DTV_SET_SD1_INTERNAL)
    sharePinCardType = PAL_DEVICE_TYPE_SD2;
    #elif defined(DTV_SET_SD2_INTERNAL)
    sharePinCardType = PAL_DEVICE_TYPE_SD;
    #else
    // set NAND to internal device ????
    sharePinCardType = PAL_DEVICE_TYPE_SD;
    #endif

    if (mmpIsCardInserted(MMP_CARD_MS) )
    {
        if (storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].retryCount == DTV_MAX_VERIFY_DEVICE_COUNT)
        {
            cardStatus = DTV_CARD_STATUS_FAIL_CARD;
        }
        else
        {
            if (storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].bInitCard == MMP_TRUE)
                cardStatus = DTV_CARD_STATUS_INSERT;
            else
            {
                MMP_INT retryCount = 0;
                // Maybe happen something wrong if one fail card is inserted.
                do
                {
                    if (storageMgr.deviceHandle[sharePinCardType].bInitCard == MMP_FALSE)
                    {
                        storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].retryCount++;
                        dbg_msg(DBG_MSG_TYPE_INFO, "...............init MS \n");
                        result = mmpMsproInitialize();
                        if (result)
                        {
                            dbg_msg_ex(DBG_MSG_TYPE_ERROR, "mmpMsproInitialize() fail !! (err: 0x%x)\n", result);
                        }
                        else
                        {
                            storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].retryCount = 0;
                            storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].bInitCard  = MMP_TRUE;
                            cardStatus                                             = DTV_CARD_STATUS_INSERT;
                            break;
                        }
                    }
                    else
                        break;

                    retryCount++;
                } while (retryCount < 2);
            }
        }
    }
    else
    {
        storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].retryCount = 0;
        storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].bInitCard  = MMP_FALSE;
        cardStatus                                             = DTV_CARD_STATUS_REMOVE;
    }

    return cardStatus;
}

#endif

#if defined(DTV_SD1_ENABLE)
static DTV_CARD_STATUS
_IsSDCardInsert(
    PAL_DEVICE_TYPE deviceType)
{
    MMP_INT         result     = 0;
    DTV_CARD_STATUS cardStatus = DTV_CARD_STATUS_REMOVE;
    MMP_CARD        cardType;
    MMP_INT         sdIndex;

    switch (deviceType)
    {
    case PAL_DEVICE_TYPE_SD:
        cardType = MMP_CARD_SD_I;
        sdIndex  = SD_1;
        break;

    case PAL_DEVICE_TYPE_SD2:
        cardType = MMP_CARD_SD2;
        sdIndex  = SD_2;
        break;
    }

    if (mmpIsCardInserted(cardType))    // detect if the card is inserted depending on the GPIO state
    {
        if (storageMgr.deviceHandle[deviceType].retryCount == DTV_MAX_VERIFY_DEVICE_COUNT)
            cardStatus = DTV_CARD_STATUS_FAIL_CARD;
        else if (storageMgr.deviceHandle[deviceType].bInitCard)
            cardStatus = DTV_CARD_STATUS_INSERT;
        else
        {
            MMP_INT retryCount = 0;

            // Maybe happen something wrong if one fail card is inserted.
            do
            {
                if (storageMgr.deviceHandle[PAL_DEVICE_TYPE_MS].bInitCard)
                    break;

                storageMgr.deviceHandle[deviceType].retryCount++;
                result = mmpSdInitializeEx(sdIndex);
                if (result == MMP_SUCCESS)
                {
                    storageMgr.deviceHandle[deviceType].retryCount = 0;
                    storageMgr.deviceHandle[deviceType].bInitCard  = MMP_TRUE;
                    cardStatus                                     = DTV_CARD_STATUS_INSERT;
                    break;
                }
                else
                {
                    dbg_msg_ex(DBG_MSG_TYPE_ERROR, "mmpSdInitialize() fail !! (err: 0x%x)\n", result);
                }

                retryCount++;
            } while (retryCount < 5);
        }
    }
    else
    {
        storageMgr.deviceHandle[deviceType].retryCount = 0;
        storageMgr.deviceHandle[deviceType].bInitCard  = MMP_FALSE;
    }

    return cardStatus;
}

#endif

#ifdef DTV_USB_ENABLE

static DTV_CARD_STATUS
_IsUsbCardInsert(
    PAL_DEVICE_TYPE deviceType)
{
    DTV_CARD_STATUS   cardStatus    = DTV_CARD_STATUS_REMOVE;
    MMP_UINT8         lun           = (deviceType - PAL_DEVICE_TYPE_USB0);
    MMP_INT           usb_state     = 0;
    USB_DEVICE_INFO   device_info   = {0};
    DTV_DEVICE_HANDLE *devHandle    = &storageMgr.deviceHandle[deviceType];
    MMP_UINT32        *initDuration = &storageMgr.initDuration[deviceType];
    MMP_INT           usbIndex;
    MMP_UINT32        mscRet        = 0;

    if (storageMgr.bUsbSuspendMode == MMP_TRUE)
        usbIndex = (0x10 | USB0);
    else
        usbIndex = USB0;

    // check USB 0
    if (mmpUsbExCheckDeviceState(usbIndex, &usb_state, &device_info) == MMP_RESULT_SUCCESS)
    {
        // check status
        if (USB_DEVICE_CONNECT(usb_state) == MMP_TRUE)
        {
            if (USB_DEVICE_MSC(device_info.type))
            {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " MSC (USB0) is inserted !!!!!!!!!!! \n");

                usb_device[0].type = device_info.type;
                usb_device[0].ctxt = device_info.ctxt;

                *initDuration      = PalGetClock();
                mmpMscResetErrStatus();
            }
            /*else if (USB_DEVICE_PTP(device_info.type))
               {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " PTP is interted!!\n");
                ptp_device_connect = MMP_TRUE;
               }//*/
        }

        if (USB_DEVICE_DISCONNECT(usb_state) == MMP_TRUE)
        {
            if (USB_DEVICE_MSC(device_info.type))
            {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " MSC (USB0) device is disconnected !!!!!!!!!!!! \n");
                usb_device[0].type = 0;
                usb_device[0].ctxt = MMP_NULL;

                *initDuration      = (MMP_UINT32)(-1);
                mmpMscResetErrStatus();
            }
            /*else if (USB_DEVICE_PTP(device_info.type))
               {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " PTP device is disconnected!\n" );
                ptp_device_connect = MMP_FALSE;
               }//*/
        }
    }
    else
    {
        devHandle->bInitCard = MMP_FALSE;
        cardStatus           = DTV_CARD_STATUS_USB_UNSUPPORT;
        goto end;
    }

    // check USB 1
    memset((void *)&device_info, 0x0, sizeof(USB_DEVICE_INFO));

    if (mmpUsbExCheckDeviceState(USB1, &usb_state, &device_info) == MMP_RESULT_SUCCESS)
    {
        // check status
        if (USB_DEVICE_CONNECT(usb_state) )
        {
            if (USB_DEVICE_MSC(device_info.type) )
            {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " MSC (USB1) is inserted !!!!!!!!!!! \n");
                //bUsbInsert = MMP_TRUE;

                //if (usb0IsOtg)
                //    memcpy((void*)&usb_device[1], &device_info, sizeof(USB_DEVICE_INFO));
                //else
                memcpy((void *)&usb_device[0], &device_info, sizeof(USB_DEVICE_INFO));
            }
            /*else if (USB_DEVICE_PTP(device_info.type))
               {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " PTP is interted!!\n" );
                ptp_device_connect = MMP_TRUE;
               }//*/
        }

        if (USB_DEVICE_DISCONNECT(usb_state) )
        {
            if (USB_DEVICE_MSC(device_info.type) )
            {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " MSC (USB1) device is disconnected !!!!!!!!!!!! \n");
                //bUsbInsert = MMP_FALSE;

                //if (usb0IsOtg)
                //    memset((void*)&usb_device[1], 0x0, sizeof(USB_DEVICE_INFO));
                //else
                memset((void *)&usb_device[0], 0x0, sizeof(USB_DEVICE_INFO));
            }
            /*else if (USB_DEVICE_PTP(device_info.type))
               {
                dbg_msg(DBG_MSG_TYPE_STORG_INFO, " PTP device is disconnected!\n" );
                ptp_device_connect = MMP_FALSE;
               }//*/
        }
    }
    //
    //#ifdef SMTK_USB_HOST_NOT_ON_BOARD
    //    bUsbInsert = mmpMscGetStatus(usb_device[1].ctxt, lun) ? MMP_FALSE : MMP_TRUE;
    //#else
    //    bUsbInsert = mmpMscGetStatus(usb_device[0].ctxt, lun) ? MMP_FALSE : MMP_TRUE;
    //#endif

    if (usb_device[0].ctxt == MMP_NULL)
    {
        devHandle->bInitCard = MMP_FALSE;
        cardStatus           = DTV_CARD_STATUS_REMOVE;
    }
    else
    {
        if (devHandle->bFailCard == MMP_TRUE ||
            ((*initDuration) != (MMP_UINT32)(-1) &&
             PalGetDuration(*initDuration) > DTV_MAX_VERIFY_DEVICE_DURATION) )
        {
            cardStatus = DTV_CARD_STATUS_FAIL_CARD;
            goto end;
        }

        mscRet = mmpMscGetStatus(usb_device[0].ctxt, lun);

        if (mscRet == MMP_RESULT_SUCCESS)
        {
            devHandle->bInitCard = MMP_TRUE;
            cardStatus           = DTV_CARD_STATUS_INSERT;
            (*initDuration)      = (MMP_UINT32)(-1);
        }
        else
        {
            if (devHandle->bInitCard == MMP_TRUE)
            {
                // Initialize successly.
                // But card fail at some unknow reason.
                cardStatus            = DTV_CARD_STATUS_FAIL_CARD;
                devHandle->retryCount = DTV_MAX_VERIFY_DEVICE_COUNT;
            }
            else if ( (*initDuration) != -1 &&
                      PalGetDuration(*initDuration) < DTV_MAX_VERIFY_DEVICE_DURATION)
            {
                // get msc status may need 800 ms
                cardStatus            = DTV_CARD_STATUS_USB_WAITING;
                devHandle->retryCount = 0;
            }
            else
            {
                cardStatus = DTV_CARD_STATUS_REMOVE;
            }
        }
    }

end:
    return cardStatus;
}

#endif

static DTV_CARD_STATUS
_storageIsInsert(
    PAL_DEVICE_TYPE deviceType)
{
    DTV_CARD_STATUS result = DTV_CARD_STATUS_REMOVE;
    PAL_DEVICE_TYPE sharePinCardType;

    switch (deviceType)
    {
        // SD
#ifdef DTV_SD1_ENABLE
    case PAL_DEVICE_TYPE_SD:
    #ifdef DTV_SET_SD1_INTERNAL
        result = DTV_CARD_STATUS_INTERNAL;
    #else
        result = _IsSDCardInsert(deviceType);
    #endif
        break;
#endif

#ifdef DTV_SD2_ENABLE
    case PAL_DEVICE_TYPE_SD2:
    #ifdef DTV_SET_SD2_INTERNAL
        result = DTV_CARD_STATUS_INTERNAL;
    #else
        result = _IsSDCardInsert(deviceType);
    #endif
        break;
#endif

    // USB
    case PAL_DEVICE_TYPE_USB0:
    case PAL_DEVICE_TYPE_USB1:
    case PAL_DEVICE_TYPE_USB2:
    case PAL_DEVICE_TYPE_USB3:
    case PAL_DEVICE_TYPE_USB4:
    case PAL_DEVICE_TYPE_USB5:
    case PAL_DEVICE_TYPE_USB6:
    case PAL_DEVICE_TYPE_USB7:
#ifdef DTV_USB_ENABLE
        result = _IsUsbCardInsert(deviceType);
        if (result == DTV_CARD_STATUS_REMOVE ||
            result == DTV_CARD_STATUS_USB_WAITING)
        {
            // try again
            result = _IsUsbCardInsert(deviceType);
        }
#else
        result = DTV_CARD_STATUS_REMOVE;
#endif
        break;

#if 0   //def DTV_PTP_ENABLE
    case PAL_DEVICE_TYPE_PTP:
        result = mmpOtgHostCheckPtpDriveReady();
        break;
#endif

        // MS
#ifdef DTV_MS_ENABLE
    case PAL_DEVICE_TYPE_MS:
    #if defined(DTV_SET_SD1_INTERNAL)
        sharePinCardType = PAL_DEVICE_TYPE_SD2;
    #elif defined(DTV_SET_SD2_INTERNAL)
        sharePinCardType = PAL_DEVICE_TYPE_SD;
    #else
        // set NAND to internal device ????
        sharePinCardType = PAL_DEVICE_TYPE_SD;
    #endif

        // SD was already detected.
        /*
                    if ( _IsSDCardInsert(sharePinCardType) )
                    {
                                        result = DTV_CARD_STATUS_REMOVE;
                    }
                    else
         */
        {
            result = _IsMSCardInsert();
        }
        break;
#endif
    //        case PAL_DEVICE_TYPE_xD:
    //#ifdef DTV_USE_TWO_USB_DEVICE
    //            result = mmpUsbHost_CheckDeviceIsReady(3);
    //#else
    //            if (IsSDCardInsert() || IsMSCardInsert())
    //            {
    //                result = MMP_FALSE;
    //            }
    //            else
    //            {
    //                result = _IsXdCardInsert();
    //            }
    //#endif
    //            break;
    //

    //        case PAL_DEVICE_TYPE_CF:
    //#ifdef DTV_USE_TWO_USB_DEVICE
    //            result = mmpUsbHost_CheckDeviceIsReady(0);
    //#endif
    //            break;
    //

    default:
        break;
    }
    return result;
}

static MMP_INT
_GetVolumeName(
    MMP_INT volume,
    MMP_WCHAR *volumeName)
{
    MMP_INT result = 0;
    if (volume >= 26)
    {
        result = -1;
        goto end;
    }

    PalWcscpy(volumeName, L"A:/");

    volumeName[0] = L'A' + volume;

end:
    return result;
}

MMP_INT
storageMgrGetVolumeNumber(
    MMP_WCHAR *pathname)
{
    MMP_INT result = 0;

    result = pathname[0] - L'A';

    return result;
}

static MMP_INT
_GetEmptyVolume(
    void)
{
    MMP_INT  volume = -1;
    MMP_UINT i      = 0;

    i = DTV_RESERVE_VOLUME_NUM;

    for (i; i < DTV_VOLUME_NUM; i++)
    {
        if (storageMgr.volumeInfo[i].bVolumeUsed != MMP_TRUE)
        {
            volume = i;
            break;
        }
    }

    return volume;
}

static MMP_BOOL
_checkFileExtention(
    MMP_WCHAR *ext,
    MMP_WCHAR *filename)
{
    MMP_UINT extLen   = PalWcslen(ext);
    MMP_UINT nameLeng = PalWcslen(filename);
    MMP_INT  i;

    for (i = 0; i < extLen; i++)
    {
        if (ext[extLen - i] == L'*')
            break;

        if (_f_toupper(filename[nameLeng - i]) != _f_toupper(ext[extLen - i]) )
            return MMP_FALSE;
    }

    return MMP_TRUE;
}

#ifdef DTV_INTERNAL_DEV_ENABLE
static MMP_INT
_InitialInternalStorage(
    void)
{
    MMP_INT         result = 0;
    MMP_INT         publicVolume, privateVolume;
    PAL_DEVICE_TYPE internalCardType;

    #ifdef DTV_RESOURCE_AT_INTERNAL
    privateVolume = 0;
    #else
    privateVolume = -1;
    #endif

    #if defined(DTV_SET_SD1_INTERNAL)
    // set sd1 to internal device
    internalCardType = PAL_DEVICE_TYPE_SD;
    result           = mmpSdInitialize();
    if (result != MMP_RESULT_SUCCESS)
    {
        dbg_msg_ex(DBG_MSG_TYPE_ERROR, "\nIniternal device init fail (return = 0x%x) !! ", result);
        goto end;
    }
    #elif defined(DTV_SET_SD2_INTERNAL)
    // set sd2 to internal device
    internalCardType = PAL_DEVICE_TYPE_SD2;
    result           = mmpSd2Initialize();
    if (result != MMP_RESULT_SUCCESS)
    {
        dbg_msg_ex(DBG_MSG_TYPE_ERROR, "\nIniternal device init fail (return = 0x%x) !! ", result);
        goto end;
    }
    #else
    // set nand to internal device
    internalCardType = PAL_DEVICE_TYPE_NAND;
    #endif

    publicVolume     = (privateVolume + 1);
    dbg_msg(DBG_MSG_TYPE_INFO, "publicVolume = %d, internalCardType = %d\n", publicVolume, internalCardType);

    #ifndef _WIN32
    storageMgr.deviceHandle[internalCardType].bUsed   = MMP_TRUE;
    storageMgr.deviceHandle[internalCardType].bInsert = MMP_TRUE;

    result                                            = _InitVolume(publicVolume, internalCardType, 0);
    #endif

    if (result & ~NTFS_RESULT_FLAG)
    {
        dbg_msg_ex(DBG_MSG_TYPE_ERROR, "\nIniternal device init fail (return = 0x%x) !! ", result);
        goto end;
    }
    else
    {
        storageMgr.deviceHandle[internalCardType].bInitialVolume = MMP_TRUE;
        if (result & NTFS_RESULT_FLAG)
        {
            storageMgr.volumeInfo[publicVolume].fileSystemFormat = NTFS_FILE_SYSTEM;
        }
        else
        {
            storageMgr.volumeInfo[publicVolume].fileSystemFormat = FAT_FILE_SYSTEM;
        }
        storageMgr.volumeInfo[publicVolume].bVolumeUsed  = MMP_TRUE;
        storageMgr.volumeInfo[publicVolume].cardType     = internalCardType;
        storageMgr.volumeInfo[publicVolume].partitionIdx = 0;
    }

    #ifdef DTV_RESOURCE_AT_INTERNAL
        #ifndef _WIN32
    // init PRIVATE volume
    storageMgr.deviceHandle[internalCardType].bUsed   = MMP_TRUE;
    storageMgr.deviceHandle[internalCardType].bInsert = MMP_TRUE;
    result                                            = _InitVolume(privateVolume, internalCardType, 1);
        #endif

    if (result & ~NTFS_RESULT_FLAG)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "!!! check (error = 0x%x) %s [#%d]\n", result,  __FILE__, __LINE__);
        goto end;
    }
    else
    {
        storageMgr.deviceHandle[internalCardType].bInitialVolume = MMP_TRUE;
        if (result & NTFS_RESULT_FLAG)
        {
            storageMgr.volumeInfo[privateVolume].fileSystemFormat = NTFS_FILE_SYSTEM;
        }
        else
        {
            storageMgr.volumeInfo[privateVolume].fileSystemFormat = FAT_FILE_SYSTEM;
        }
        storageMgr.volumeInfo[privateVolume].bVolumeUsed             = MMP_TRUE;
        storageMgr.volumeInfo[privateVolume].cardType                = internalCardType;
        storageMgr.volumeInfo[privateVolume].partitionIdx            = 1;
        storageMgr.volumeInfo[privateVolume].bReadyCollectVolumeInfo = MMP_TRUE;
    }
    #endif

end:
    return result;
}

#endif

#if defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS)
static MMP_INT
_InitialExternalStorage(
    PAL_DEVICE_TYPE deviceType)
{
    MMP_INT result = 0;
    MMP_INT i;

    storageMgr.deviceHandle[deviceType].bUsed = MMP_TRUE;
    if (_storageIsInsert(deviceType) == DTV_CARD_STATUS_INSERT)
    {
        int     count_valid = 0;
        MMP_INT volume;
        MMP_INT count;
        void    *f_dev_init = get_initfunc(deviceType);

        if (f_dev_init == MMP_NULL)
            return MMP_RESULT_ERROR;

        storageMgr.deviceHandle[deviceType].bInsert = MMP_TRUE;

        count                                       = PalGetPartitionCount(deviceType, get_initfunc(deviceType));
        for (i = 0; i < count; i++)
        {
            volume = _GetEmptyVolume();
            if (volume != -1)
            {
                result = _InitVolume(volume, deviceType, i);
                if (result & ~NTFS_RESULT_FLAG)
                {
                    PalDelVolume(volume);
                    if (_storageIsInsert(deviceType) == DTV_CARD_STATUS_REMOVE)
                        PalDelDriver(deviceType);

                    storageMgr.deviceHandle[deviceType].retryCount++;
                }
                else
                {
                    storageMgr.deviceHandle[deviceType].bInitialVolume = MMP_TRUE;
                    if (result & NTFS_RESULT_FLAG)
                    {
                        storageMgr.volumeInfo[volume].fileSystemFormat = NTFS_FILE_SYSTEM;
                    }
                    else
                    {
                        storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                    }
                    storageMgr.volumeInfo[volume].bVolumeUsed  = MMP_TRUE;
                    storageMgr.volumeInfo[volume].cardType     = deviceType;
                    storageMgr.volumeInfo[volume].partitionIdx = i;
                    count_valid++;
                }
            }
        }
        storageMgr.deviceHandle[deviceType].totalVolume = count_valid;
    }

    return result;
}

#else
static MMP_INT
_InitialExternalStorage(
    PAL_DEVICE_TYPE deviceType)
{
    MMP_INT result    = 0;
    MMP_INT tmpResult = 0;
    MMP_INT i;

    storageMgr.deviceHandle[deviceType].bUsed = MMP_TRUE;
    if (_storageIsInsert(deviceType) == DTV_CARD_STATUS_INSERT)
    {
        MMP_INT volume;
        void    *f_dev_init = get_initfunc(deviceType);

        if (f_dev_init == MMP_NULL)
            return MMP_RESULT_ERROR;

        storageMgr.deviceHandle[deviceType].bInsert = MMP_TRUE;

        volume                                      = _GetEmptyVolume();
        if (volume != -1)
        {
            dbg_msg(DBG_MSG_TYPE_STORG_INFO, "\n\n%d initVolume\n", deviceType);
            result = _InitVolume(volume, deviceType, 0);

            if (result & ~NTFS_RESULT_FLAG)
            {
                PalDelVolume(volume);
                if (_storageIsInsert(deviceType) == DTV_CARD_STATUS_REMOVE)
                    PalDelDriver(deviceType);

                storageMgr.deviceHandle[deviceType].retryCount++;
            }
            else
            {
                storageMgr.deviceHandle[deviceType].bInitialVolume = MMP_TRUE;
                if (result & NTFS_RESULT_FLAG)
                {
                    storageMgr.volumeInfo[volume].fileSystemFormat = NTFS_FILE_SYSTEM;
                }
                else
                {
                    storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                }
                storageMgr.volumeInfo[volume].bVolumeUsed  = MMP_TRUE;
                storageMgr.volumeInfo[volume].cardType     = deviceType;
                storageMgr.volumeInfo[volume].partitionIdx = 0;

                // initial other partition in SD
                // To do
                for (i = 1; i < storageMgr.deviceHandle[deviceType].totalVolume; i++)
                {
                    volume = _GetEmptyVolume();
                    if (volume != -1)
                    {
                        tmpResult = _InitVolume(volume, deviceType, i);
                        if ((tmpResult & ~NTFS_RESULT_FLAG) == 0)
                        {
                            if (tmpResult & NTFS_RESULT_FLAG)
                            {
                                storageMgr.volumeInfo[volume].fileSystemFormat = NTFS_FILE_SYSTEM;
                            }
                            else
                            {
                                storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                            }
                            storageMgr.volumeInfo[volume].bVolumeUsed  = MMP_TRUE;
                            storageMgr.volumeInfo[volume].cardType     = deviceType;
                            storageMgr.volumeInfo[volume].partitionIdx = i;
                        }
                        else
                            break;
                    }
                    else
                        break;
                }
            }
        }
    }
}

#endif

static MMP_INT
_InitialAllExternalStorages(
    void)
{
#if defined(__FREERTOS__)
    const PAL_DEVICE_TYPE external_storages[] =
    {
    #if defined(DTV_SD1_ENABLE) && !defined(DTV_SET_SD1_INTERNAL)
        PAL_DEVICE_TYPE_SD,
    #endif
    #ifdef DTV_USB_ENABLE
        PAL_DEVICE_TYPE_USB0,
        PAL_DEVICE_TYPE_USB1,
        PAL_DEVICE_TYPE_USB2,
        PAL_DEVICE_TYPE_USB3,
        PAL_DEVICE_TYPE_USB4,
        PAL_DEVICE_TYPE_USB5,
        //PAL_DEVICE_TYPE_USB6,
        //PAL_DEVICE_TYPE_USB7,
    #endif
    #if defined(DTV_SD2_ENABLE) && !defined(DTV_SET_SD2_INTERNAL)
        PAL_DEVICE_TYPE_SD2,
    #endif
    #ifdef DTV_MS_ENABLE
        PAL_DEVICE_TYPE_MS,
    #endif
    #ifdef DTV_CF_ENABLE
        PAL_DEVICE_TYPE_CF,
    #endif
    #ifdef DTV_xD_ENABLE
        PAL_DEVICE_TYPE_xD,
    #endif
    #ifdef DTV_PTP_ENABLE
        PAL_DEVICE_TYPE_PTP,
    #endif
        PAL_DEVICE_TYPE_RESERVED
    };
    MMP_UINT              i;

    for (i = 0; external_storages[i] != PAL_DEVICE_TYPE_RESERVED; ++i)
        _InitialExternalStorage(external_storages[i]);
#endif /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */

    return 0;
}

static void
_AddStorageInfo(
    MMP_INT volume,
    MMP_UINT usedFileTable,
    MMP_UINT subFileIndex,
    PAL_FILE_FIND hSearch,
    MMP_UINT fileType,
    MMP_UINT currDirIdx)
{
    MMP_UINT        len;
    DTV_VOLUME_INFO *currVolumeInfo = &storageMgr.volumeInfo[volume];

    if (subFileIndex == 0)
    {
        currVolumeInfo->baseFileTable[fileType][usedFileTable] = (void *)PalHeapAlloc(PAL_HEAP_DEFAULT, (DTV_SUB_FILE_TABLE_COUNT * sizeof(DTV_BASE_FILE_TABLE)));
        PalMemset((void *)currVolumeInfo->baseFileTable[fileType][usedFileTable], 0, (DTV_SUB_FILE_TABLE_COUNT * sizeof(DTV_BASE_FILE_TABLE)));
    }

    PalMemcpy((void *) &currVolumeInfo->baseFileTable[fileType][usedFileTable][subFileIndex].position,
              PalWFileFindGetPos(hSearch),
              sizeof(SF_POS));

    currVolumeInfo->baseFileTable[fileType][usedFileTable][subFileIndex].dirIndex = currDirIdx;
    //    dbg_msg(DBG_MSG_TYPE_INFO, "dirIndex = %d\n", currVolumeInfo->baseFileTable[fileType][usedFileTable][subFileIndex].dirIndex);

    currVolumeInfo->fileCount[fileType]++;
}

static STORAGE_ERR
_CollectFileInfo(
    MMP_WCHAR *searchpath,
    MMP_BOOL recursive,
    MMP_UINT srcDirIndex,
    MMP_UINT dirLevel)
{
    STORAGE_ERR     result          = 0;
    MMP_BOOL        fFinished       = MMP_FALSE;
    MMP_UINT        len;
    MMP_UINT        offset          = 0;
    MMP_BOOL        bValid          = MMP_FALSE;
    PAL_FILE_FIND   hSearch;
    MMP_INT         volume          = storageMgrGetVolumeNumber(searchpath);;
    DTV_VOLUME_INFO *currVolumeInfo = &storageMgr.volumeInfo[volume];
    MMP_UINT        loopCount       = 0;

    //    // todo ptp search
    //    MMP_WCHAR   checkName[3] = {0};
    //    MMP_INT     i;
    //    MMP_UINT16 cardType = 0;
    //
    //    for (i = 0; i < 3; i++)
    //    {
    //        checkName[i] =(MMP_WCHAR)_f_toupper(*searchpath);
    //        searchpath++;
    //    }
    //
    //    for(i = 0; i < 3; i++)
    //    {
    //        searchpath--;
    //    }
    //
    //    if (checkName[0] == 0x50 && checkName[1] == 0x54 && checkName[2] == 0x50)
    //    {
    //        cardType = PAL_DEVICE_PTP;
    //    }

    offset = PalWcslen(searchpath);

    PalWcscpy(&searchpath[offset], DTV_ALL_SEARCH);

    //    dbg_msg(DBG_MSG_TYPE_STORG_INFO, "check -> ");
    //    _storageMgrPrintString(MMP_NULL, searchpath);

    hSearch = PalTFileFindFirst(searchpath, MMP_NULL, MMP_NULL);
    if (hSearch == MMP_NULL)
    {
        goto end;
    }

    while (!fFinished)
    {
        if ( (loopCount & 0x3F) == 0)
            PalSleep(3);

        if (storageMgr.bInterrupt == MMP_TRUE)
        {
            result = STORAGE_ERR_INTERRUPT;
            break;
        }

        if (currVolumeInfo->totalFileCount >= DTV_MAX_FILE_COUNT)
        {
            // Limit the total files to DTV_MAX_FILE_COUNT for memory issue
            result = STORAGE_ERR_OUT_MAX_FILE_COUNT;
            break;
        }

        if (PalTFindAttrIsDirectory(hSearch) &&
            //!(PalGetFindAttr(hSearch) & F_ATTR_HIDDEN) &&    // PalGetFindAttr() for not show hidden attribute
            PalWcscmp(L"..", (MMP_WCHAR *)PalTFileFindGetName(hSearch)) &&
            PalWcscmp(L".", (MMP_WCHAR *)PalTFileFindGetName(hSearch)) &&
            dirLevel < DTV_MAX_DIR_LEVEL &&
            currVolumeInfo->dirCount < DTV_MAX_DIR_COUNT
            /*&&cardType != PAL_DEVICE_PTP*/)
        {
            MMP_UINT offset1      = PalWcslen(PalTFileFindGetName(hSearch));
            MMP_UINT dirCount     = currVolumeInfo->dirCount;
            MMP_UINT usedDirTable = dirCount / DTV_SUB_DIR_COUNT;
            MMP_UINT subDirIndex  = dirCount - (usedDirTable * DTV_SUB_DIR_COUNT);

            if (subDirIndex == 0)
            {
                currVolumeInfo->dirTable[usedDirTable] = (void *)PalHeapAlloc(PAL_HEAP_DEFAULT, (DTV_SUB_DIR_COUNT * sizeof(DTV_DIR_TABLE)));
                PalMemset((void *)currVolumeInfo->dirTable[usedDirTable], 0, (DTV_SUB_DIR_COUNT * sizeof(DTV_DIR_TABLE)));
            }

            PalMemset(&searchpath[offset], 0, PalWcslen(DTV_ALL_SEARCH) * sizeof(MMP_WCHAR));
            PalWcscpy(&searchpath[offset], PalTFileFindGetName(hSearch));
            searchpath[offset + offset1] = L'/';
            offset1++;

            //len = (PalWcslen(PalTFileFindGetName(hSearch)) + 2) * sizeof(MMP_WCHAR);
            //currVolumeInfo->dirTable[usedDirTable][subDirIndex].dirLastName = (MMP_WCHAR*) PalHeapAlloc(PAL_HEAP_DEFAULT, len);
            //PalMemset(currVolumeInfo->dirTable[usedDirTable][subDirIndex].dirLastName, 0, len);
            //PalWcscpy(currVolumeInfo->dirTable[usedDirTable][subDirIndex].dirLastName, PalTFileFindGetName(hSearch));
            PalMemcpy((void *) &currVolumeInfo->dirTable[usedDirTable][subDirIndex].position,
                      PalWFileFindGetPos(hSearch),
                      sizeof(SF_POS));

            currVolumeInfo->dirTable[usedDirTable][subDirIndex].srcDirIndex = srcDirIndex;
            currVolumeInfo->dirTable[usedDirTable][subDirIndex].dirLevel    = dirLevel + 1;

            //dbg_msg(DBG_MSG_TYPE_STORG_INFO, "\n*****  folder path (dirIndex = %d, srcDirIndex = %d): ", currVolumeInfo->dirCount, srcDirIndex);
            //_storageMgrPrintString(MMP_NULL, searchpath);

            currVolumeInfo->dirCount++;

            {
                MMP_UINT tmpUsedDirTable = currVolumeInfo->dirTable[usedDirTable][subDirIndex].srcDirIndex / DTV_SUB_DIR_COUNT;
                MMP_UINT tmpSubDirIndex  = currVolumeInfo->dirTable[usedDirTable][subDirIndex].srcDirIndex - (tmpUsedDirTable * DTV_SUB_DIR_COUNT);

                currVolumeInfo->dirTable[tmpUsedDirTable][tmpSubDirIndex].totalSubDir++;
            }

            if (recursive)
            {
                result = _CollectFileInfo(
                    searchpath,
                    recursive,
                    dirCount,
                    currVolumeInfo->dirTable[usedDirTable][subDirIndex].dirLevel);
            }

            PalMemset(&searchpath[offset], 0, offset1 * sizeof(MMP_WCHAR));
        }
        else
        {
            MMP_UINT      currDirIdx   = srcDirIndex;
            MMP_UINT      usedDirTable = currDirIdx / DTV_SUB_DIR_COUNT;
            MMP_UINT      subDirIndex  = currDirIdx - (usedDirTable * DTV_SUB_DIR_COUNT);

            MMP_INT       i;
            MMP_INT       itemGap      = 0;
            DTV_FILE_TYPE tmpType      = DTV_FILE_TYPE_JPG;
            MMP_WCHAR     *findName    = (MMP_WCHAR *)PalTFileFindGetName(hSearch);

            //            _storageMgrPrintString(MMP_NULL, findName);

            for (i = DTV_EXT_SEARCH_JPG; i < DTV_EXT_SEARCH_COUNT; i++)
            {
                if (i == DTV_EXT_SEARCH_PHOTO_COUNT ||
                    i == DTV_EXT_SEARCH_AUDIO_COUNT ||
                    i == DTV_EXT_SEARCH_VIDEO_COUNT ||
                    i == DTV_EXT_SEARCH_RECORD_COUNT ||
                    i == DTV_EXT_SEARCH_FW_COUNT)
                {
                    itemGap++;
                    continue;
                }

                if (i > DTV_EXT_SEARCH_FW_COUNT)
                {
                    tmpType = DTV_FILE_TYPE_VOICE;
                }
                else if (i > DTV_EXT_SEARCH_RECORD_COUNT)
                {
                    tmpType = DTV_FILE_TYPE_FW;
                }
                else if (i > DTV_EXT_SEARCH_VIDEO_COUNT)
                {
                    tmpType = DTV_FILE_TYPE_RECORD;
                }
                else if (i > DTV_EXT_SEARCH_AUDIO_COUNT)
                {
                    tmpType = DTV_FILE_TYPE_VIDEO;
                }
                else if (i > DTV_EXT_SEARCH_PHOTO_COUNT)
                {
                    tmpType = DTV_FILE_TYPE_AUDIO;
                }
                else
                {
                    tmpType = DTV_FILE_TYPE_JPG;
                }

                // limit the max files, which every file type, to DTV_MAX_FILE_COUNT
                if (_checkFileExtention(FileExtArray[i - itemGap], findName) &&
                    (currVolumeInfo->fileCount[tmpType] < DTV_MAX_FILE_COUNT - 1) )
                {
                    MMP_UINT fileCount     = currVolumeInfo->fileCount[tmpType];
                    MMP_UINT usedFileTable = fileCount / DTV_SUB_FILE_TABLE_COUNT;
                    MMP_UINT subFileIndex  = fileCount - (usedFileTable * DTV_SUB_FILE_TABLE_COUNT);

                    currVolumeInfo->totalFileCount++;
                    currVolumeInfo->dirTable[usedDirTable][subDirIndex].fileCountInDir[tmpType]++;
                    _AddStorageInfo(volume, usedFileTable, subFileIndex, hSearch, tmpType, currDirIdx);
                    break;
                }
            }
        }

        if (PalTFileFindNext(hSearch, MMP_NULL, MMP_NULL))
            fFinished = MMP_TRUE;

        loopCount++;
    }
    PalTFileFindClose(hSearch, MMP_NULL, MMP_NULL);
end:
    // reset the searchpath char
    PalMemset(&searchpath[offset], 0, (PalWcslen(searchpath) - offset) * sizeof(MMP_WCHAR));
    return result;
}

static void
_CreateStorageDataBase(
    MMP_UINT volume,
    MMP_WCHAR   *startPath,
    MMP_BOOL bRecursiveSearch)
{
    MMP_INT         result          = 0;
    STORAGE_ERR     tmpResult       = STORAGE_ERR_OK;
    MMP_WCHAR       filepath[256]   = {0};
    MMP_UINT        offset          = 0;
    PAL_FILE_FIND   hSearch;
    DTV_VOLUME_INFO *currVolumeInfo = &storageMgr.volumeInfo[volume];

    storageMgr.bDBCreating                  = MMP_TRUE;

    currVolumeInfo->dirCount                = 0;
    currVolumeInfo->totalFileCount          = 0;

    currVolumeInfo->bInCollectStatus        = MMP_TRUE;
    currVolumeInfo->bReadyCollectVolumeInfo = MMP_FALSE;

    if (startPath)
        PalWcscpy(filepath, startPath);
    else
        _GetVolumeName(volume, filepath);

    offset = PalWcslen(filepath);
    PalWcscpy(&filepath[offset], DTV_ALL_SEARCH);

    //    dbg_msg(DBG_MSG_TYPE_INFO, "0. check -> ");
    //    _storageMgrPrintString(MMP_NULL, filepath);

    hSearch = PalTFileFindFirst(filepath, MMP_NULL, MMP_NULL);
    if (hSearch == MMP_NULL)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "No file in this volume. %s [#%d]\n", __FILE__, __LINE__);
        goto end;
    }

    currVolumeInfo->dirTable[0] = (void *)PalHeapAlloc(PAL_HEAP_DEFAULT, (DTV_SUB_DIR_COUNT * sizeof(DTV_DIR_TABLE)));
    PalMemset((void *)currVolumeInfo->dirTable[0], 0, (DTV_SUB_DIR_COUNT * sizeof(DTV_DIR_TABLE)));
    PalMemcpy((void *) &currVolumeInfo->dirTable[0][currVolumeInfo->dirCount].position,
              PalWFileFindGetPos(hSearch),
              sizeof(SF_POS));

    PalMemset(&filepath[offset], 0, PalWcslen(DTV_ALL_SEARCH) * sizeof(MMP_WCHAR));

    PalTFileFindClose(hSearch, MMP_NULL, MMP_NULL);

    currVolumeInfo->dirTable[0][currVolumeInfo->dirCount].srcDirIndex = (-1);
    currVolumeInfo->dirTable[0][currVolumeInfo->dirCount].dirLevel    = 0;

    currVolumeInfo->dirCount++;

    tmpResult = _CollectFileInfo(filepath, bRecursiveSearch, 0, 0);
    if (tmpResult == STORAGE_ERR_INTERRUPT)
    {
        storageMgr.bDBCreating                  = MMP_FALSE;
        currVolumeInfo->bInCollectStatus        = MMP_FALSE;
        currVolumeInfo->bReadyCollectVolumeInfo = MMP_TRUE;
        storageMgrClearDataBase(volume);
        return;
    }

    if (currVolumeInfo->bRecursiveSearch == MMP_TRUE)
    {
        // sort file ( mappingFileIndexTable ) with dir index
        MMP_UINT i, j, k, l, fileIndex, currDir;
        MMP_UINT *dirFileCount = MMP_NULL;
        dirFileCount = (void *)PalHeapAlloc(PAL_HEAP_DEFAULT, (currVolumeInfo->dirCount * sizeof(MMP_UINT)));

        for (k = 0; k < DTV_FILE_TYPE_COUNT - 1; k++)
        {
            PalMemset((void *)dirFileCount, 0, (currVolumeInfo->dirCount * sizeof(MMP_UINT)));
            if (currVolumeInfo->fileCount[k] > 0)
            {
                currVolumeInfo->mappingFileIndexTable[k] = (void *)PalHeapAlloc(PAL_HEAP_DEFAULT, (currVolumeInfo->fileCount[k] * sizeof(DTV_MAPPING_FILE_INDEX_TABLE)));
                PalMemset((void *)currVolumeInfo->mappingFileIndexTable[k], 0, (currVolumeInfo->fileCount[k] * sizeof(DTV_MAPPING_FILE_INDEX_TABLE)));
                currDir                                  = 0;

                for (j = 0; j < currVolumeInfo->fileCount[k]; j++)
                {
                    MMP_UINT usedFileTableIdx = j / DTV_SUB_FILE_TABLE_COUNT;
                    MMP_UINT subFileIdx       = j - (usedFileTableIdx * DTV_SUB_FILE_TABLE_COUNT);

                    if (currVolumeInfo->baseFileTable[k][usedFileTableIdx][subFileIdx].dirIndex != currDir)
                    {
                        currDir   = currVolumeInfo->baseFileTable[k][usedFileTableIdx][subFileIdx].dirIndex;
                        fileIndex = 0;
                        for (i = 0; i < currDir; i++)
                        {
                            fileIndex += currVolumeInfo->dirTable[i / DTV_SUB_DIR_COUNT][i % DTV_SUB_DIR_COUNT].fileCountInDir[k];
                        }

                        if (dirFileCount[currDir] == 0)
                            dirFileCount[currDir] = fileIndex;
                        else
                            dirFileCount[currDir] = dirFileCount[currDir];
                    }
                    currVolumeInfo->mappingFileIndexTable[k][dirFileCount[currDir]].fileIndex = j;
                    dirFileCount[currDir]++;
                }
            }
        }

        if (dirFileCount)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, dirFileCount);
            dirFileCount = MMP_NULL;
        }
    }

end:
    currVolumeInfo->bInCollectStatus        = MMP_FALSE;
    currVolumeInfo->bReadyCollectVolumeInfo = MMP_TRUE;

    storageMgr.bDBCreating                  = MMP_FALSE;
}

static void
_DestroyStorageDataBase(
    MMP_UINT volume)
{
    DTV_VOLUME_INFO *currVolumeInfo = &storageMgr.volumeInfo[volume];

    if (currVolumeInfo->bReadyCollectVolumeInfo == MMP_TRUE)
    {
        MMP_UINT i, j, k = 0;
        MMP_UINT maxTableCount = (DTV_MAX_FILE_COUNT / DTV_SUB_FILE_TABLE_COUNT);

        for (j = 0; j < DTV_FILE_TYPE_COUNT; j++)
        {
            if (currVolumeInfo->mappingFileIndexTable[j])
            {
                PalHeapFree(PAL_HEAP_DEFAULT, currVolumeInfo->mappingFileIndexTable[j]);
                currVolumeInfo->mappingFileIndexTable[j] = MMP_NULL;
            }

            for (i = 0; i < maxTableCount; i++)
            {
                if (currVolumeInfo->baseFileTable[j][i])
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, currVolumeInfo->baseFileTable[j][i]);
                    currVolumeInfo->baseFileTable[j][i] = MMP_NULL;
                }
            }
        }

        maxTableCount = (DTV_MAX_DIR_COUNT / DTV_SUB_DIR_COUNT);

        for (j = 0; j < maxTableCount; j++)
        {
            if (currVolumeInfo->dirTable[j])
            {
                PalHeapFree(PAL_HEAP_DEFAULT, currVolumeInfo->dirTable[j]);
                currVolumeInfo->dirTable[j] = MMP_NULL;
            }
        }

        if (currVolumeInfo->startPath)
        {
            PalHeapFree(0, currVolumeInfo->startPath);
            currVolumeInfo->startPath = MMP_NULL;
        }
    }

    PalMemset((void *)&storageMgr.volumeInfo[volume], 0, sizeof(DTV_VOLUME_INFO));
}

static void
_GetCardEvent(
    DTV_CARD_EVENT   *cardEvent,
    MMP_INT          *cardId)
{
    MMP_UINT i, j;
    MMP_INT  result = 0;
    MMP_INT  volume;

    *cardEvent = DTV_CARD_NOCHANGE;
    *cardId    = -1;

    //todo partition handle
    for (i = PAL_DEVICE_TYPE_SD; i < PAL_DEVICE_TYPE_COUNT; i++)
    {
        if (storageMgr.deviceHandle[i].bUsed == MMP_TRUE)
        {
            DTV_CARD_STATUS cardStatus = _storageIsInsert(i);

            switch (cardStatus)
            {
            default:
            case DTV_CARD_STATUS_INTERNAL:
                break;

            case DTV_CARD_STATUS_FAIL_CARD:
                if (storageMgr.deviceHandle[i].retryCount == DTV_MAX_VERIFY_DEVICE_COUNT &&
                    storageMgr.deviceHandle[i].bFailCard == MMP_FALSE)
                {
                    dbg_msg(DBG_MSG_TYPE_ERROR, "This card fail (type: %d) !!\n", i);
                    storageMgr.deviceHandle[i].bFailCard = MMP_TRUE;
                    *cardEvent                           = DTV_CARD_FAIL_CARD;
                    *cardId                              = i;

                    if (storageMgr.deviceHandle[i].bInsert == MMP_TRUE)
                    {
                        // clear all volume in this removed storage
                        for (j = 0; j < DTV_VOLUME_NUM; j++)
                        {
                            if (storageMgr.volumeInfo[j].cardType == i)
                            {
                                volume = j;
                            }
                            else
                                continue;

                            if (storageMgr.volumeInfo[volume].bInCollectStatus)
                            {
                                do
                                {
                                    PalSleep(5);
                                } while (storageMgr.volumeInfo[volume].bInCollectStatus);
                            }

                            _DestroyStorageDataBase(volume);
                            PalDelVolume(volume);
                        }

                        PalDelDriver(i);
                        switch (i)
                        {
#if (defined(DTV_SD1_ENABLE) || defined(DTV_MMC_ENABLE))
    #ifdef DTV_MMC_ENABLE
                        case PAL_DEVICE_TYPE_MMC:
    #endif
    #ifdef DTV_SD1_ENABLE
                        case PAL_DEVICE_TYPE_SD:
    #endif
                            mmpSdTerminate();
                            break;
#endif

#ifdef DTV_SD2_ENABLE
                        case PAL_DEVICE_TYPE_SD2:
                            mmpSd2Terminate();
                            break;
#endif

#ifdef DTV_MS_ENABLE
                        case PAL_DEVICE_TYPE_MS:
                            mmpMsTerminate();
                            break;
#endif

                        default:
                            storageMgr.deviceHandle[i].bInitCard = MMP_FALSE;
                            break;
                        }

                        storageMgr.deviceHandle[i].bInitialVolume = MMP_FALSE;
                        storageMgr.deviceHandle[i].retryCount     = DTV_MAX_VERIFY_DEVICE_COUNT;
                        storageMgr.deviceHandle[i].totalVolume    = 0;
                        goto end;
                    }
                }
                break;

            case DTV_CARD_STATUS_USB_UNSUPPORT:
                *cardEvent = DTV_CARD_USB_UNSUPPORT;
                *cardId    = -1;
                goto end;
                break;

            case DTV_CARD_STATUS_INSERT:
                if (storageMgr.deviceHandle[i].bInsert == MMP_FALSE)
                {
#if defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS)
                    int count_valid = 0;
                    int count       = PalGetPartitionCount(i, get_initfunc(i));

                    // The first time to insert card case
                    storageMgr.deviceHandle[i].bInsert = MMP_TRUE;
                    storageMgr.deviceHandle[i].retryCount++;

                    for (j = 0; j < count; j++)
                    {
                        volume = _GetEmptyVolume();
                        if (volume != -1)
                        {
                            result = _InitVolume(volume, i, j);
                            //PalDiskGetFreeSpace(volume, &freeSpace_h, &freeSpace_l, MMP_NULL);

                            if (result & ~NTFS_RESULT_FLAG)
                            {
                                PalDelVolume(volume);
                                if (_storageIsInsert(i) == DTV_CARD_STATUS_REMOVE)
                                    PalDelDriver(i);

                                storageMgr.deviceHandle[i].retryCount++;
                            }
                            else
                            {
                                storageMgr.deviceHandle[i].bInitialVolume = MMP_TRUE;
                                if (result & NTFS_RESULT_FLAG)
                                {
                                    trac("volume: %u\n", volume);
                                    storageMgr.volumeInfo[volume].fileSystemFormat = NTFS_FILE_SYSTEM;
                                }
                                else
                                {
                                    trac("volume: %u\n", volume);
                                    storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                                }
                                storageMgr.volumeInfo[volume].bVolumeUsed  = MMP_TRUE;
                                storageMgr.volumeInfo[volume].cardType     = i;
                                storageMgr.volumeInfo[volume].partitionIdx = j;
                                *cardEvent                                 = DTV_CARD_INSERT;
                                *cardId                                    = i;
                                count_valid++;
                            }
                        }
                    }
                    storageMgr.deviceHandle[i].totalVolume = count_valid;
#else               /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */
                    // The first time to insert card case
                    storageMgr.deviceHandle[i].bInsert     = MMP_TRUE;
                    volume                                 = _GetEmptyVolume();

                    if (volume != -1)
                    {
                        result = _InitVolume(volume, i, 0);

                        if (result)
                        {
                            PalDelVolume(volume);
                            if (_storageIsInsert(i) == DTV_CARD_STATUS_REMOVE)
                                PalDelDriver(i);

                            storageMgr.deviceHandle[i].retryCount++;
                        }
                        else
                        {
                            //PalDiskGetFreeSpace(volume, &freeSpace_h, &freeSpace_l, MMP_NULL);
                            storageMgr.deviceHandle[i].bInitialVolume      = MMP_TRUE;
                            storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                            storageMgr.volumeInfo[volume].bVolumeUsed      = MMP_TRUE;
                            storageMgr.volumeInfo[volume].cardType         = i;
                            storageMgr.volumeInfo[volume].partitionIdx     = 0;
                            *cardEvent                                     = DTV_CARD_INSERT;
                            *cardId                                        = i;

                            // initial other partition in storage
                            // To do
                            for (j = 1; j < storageMgr.deviceHandle[i].totalVolume; j++)
                            {
                                volume = _GetEmptyVolume();
                                if (volume != -1)
                                {
                                    result = _InitVolume(volume, i, j);
                                    if (result == 0)
                                    {
                                        storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                                        storageMgr.volumeInfo[volume].bVolumeUsed      = MMP_TRUE;
                                        storageMgr.volumeInfo[volume].cardType         = i;
                                        storageMgr.volumeInfo[volume].partitionIdx     = j;
                                    }
                                    else
                                        break;
                                }
                                else
                                    break;
                            }

                            goto end;
                        }
                    }
#endif              /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */
                }
                else
                {
                    // already be inserted card case
                    if (storageMgr.deviceHandle[i].bInitialVolume == MMP_FALSE &&
                        storageMgr.deviceHandle[i].retryCount < DTV_MAX_VERIFY_DEVICE_COUNT)
                    {
#if defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS)
                        int count_valid = 0;
                        int count       = PalGetPartitionCount(i, get_initfunc(i));
                        storageMgr.deviceHandle[i].retryCount++;

                        for (j = 0; j < count; j++)
                        {
                            volume = _GetEmptyVolume();
                            if (volume != -1)
                            {
                                result = _InitVolume(volume, i, j);

                                if (result & ~NTFS_RESULT_FLAG)
                                {
                                    PalDelVolume(volume);
                                    if (_storageIsInsert(i) == DTV_CARD_STATUS_REMOVE)
                                        PalDelDriver(i);

                                    storageMgr.deviceHandle[i].retryCount++;
                                }
                                else
                                {
                                    storageMgr.deviceHandle[i].bInitialVolume = MMP_TRUE;
                                    if (result & NTFS_RESULT_FLAG)
                                    {
                                        storageMgr.volumeInfo[volume].fileSystemFormat = NTFS_FILE_SYSTEM;
                                    }
                                    else
                                    {
                                        storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                                    }
                                    storageMgr.volumeInfo[volume].bVolumeUsed  = MMP_TRUE;
                                    storageMgr.volumeInfo[volume].cardType     = i;
                                    storageMgr.volumeInfo[volume].partitionIdx = j;
                                    *cardEvent                                 = DTV_CARD_INSERT;
                                    *cardId                                    = i;
                                    count_valid++;
                                }
                            }
                        }
                        storageMgr.deviceHandle[i].totalVolume = count_valid;
#else                   /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */
                        volume                                 = _GetEmptyVolume();
                        if (volume != -1)
                        {
                            result = _InitVolume(volume, i, 0);

                            if (result)
                            {
                                PalDelVolume(volume);
                                if (_storageIsInsert(i) == DTV_CARD_STATUS_REMOVE)
                                    PalDelDriver(i);

                                storageMgr.deviceHandle[i].retryCount++;
                            }
                            else
                            {
                                storageMgr.deviceHandle[i].bInitialVolume      = MMP_TRUE;
                                storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                                storageMgr.volumeInfo[volume].bVolumeUsed      = MMP_TRUE;
                                storageMgr.volumeInfo[volume].cardType         = i;
                                storageMgr.volumeInfo[volume].partitionIdx     = 0;
                                *cardEvent                                     = DTV_CARD_INSERT;
                                *cardId                                        = i;

                                // initial other partition in storage
                                // To do
                                for (j = 1; j < storageMgr.deviceHandle[i].totalVolume; j++)
                                {
                                    volume = _GetEmptyVolume();
                                    if (volume != -1)
                                    {
                                        result = _InitVolume(volume, i, j);
                                        if (result == 0)
                                        {
                                            storageMgr.volumeInfo[volume].fileSystemFormat = FAT_FILE_SYSTEM;
                                            storageMgr.volumeInfo[volume].bVolumeUsed      = MMP_TRUE;
                                            storageMgr.volumeInfo[volume].cardType         = i;
                                            storageMgr.volumeInfo[volume].partitionIdx     = j;
                                        }
                                        else
                                            break;
                                    }
                                    else
                                        break;
                                }

                                goto end;
                            }
                        }
#endif                  /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */
                    }
                }
                *cardEvent                            = DTV_CARD_INSERT;
                *cardId                               = i;
                storageMgr.cardEventInfo[i].bRefresh  = MMP_TRUE;
                storageMgr.cardEventInfo[i].cardEvent = *cardEvent;
                break;

            case DTV_CARD_STATUS_REMOVE:
                // remove card case and clear device handle info
                if (storageMgr.deviceHandle[i].bInsert == MMP_TRUE)
                {
                    storageMgr.deviceHandle[i].bInsert   = MMP_FALSE;
                    storageMgr.deviceHandle[i].bInitCard = MMP_FALSE;
                    storageMgr.deviceHandle[i].bFailCard = MMP_FALSE;

                    /*if (storageMgr.deviceHandle[i].bInitialVolume == MMP_FALSE)
                       {
                        storageMgr.deviceHandle[i].retryCount = 0;
                        storageMgr.deviceHandle[i].totalVolume = 0;
                       }
                       else//*/
                    {
                        // clear all volume in this removed storage
                        for (j = 0; j < DTV_VOLUME_NUM; j++)
                        {
                            if (storageMgr.volumeInfo[j].cardType == i)
                            {
                                volume = j;
                            }
                            else
                                continue;

                            if (storageMgr.volumeInfo[volume].bInCollectStatus)
                            {
                                do
                                {
                                    PalSleep(5);
                                } while (storageMgr.volumeInfo[volume].bInCollectStatus);
                            }

                            _DestroyStorageDataBase(volume);
                            PalDelVolume(volume);
                        }

                        PalDelDriver(i);
                        switch (i)
                        {
#if (defined(DTV_SD1_ENABLE) || defined(DTV_MMC_ENABLE))
    #ifdef DTV_MMC_ENABLE
                        case PAL_DEVICE_TYPE_MMC:
    #endif
    #ifdef DTV_SD1_ENABLE
                        case PAL_DEVICE_TYPE_SD:
    #endif
                            mmpSdTerminate();
                            break;
#endif

#ifdef DTV_SD2_ENABLE
                        case PAL_DEVICE_TYPE_SD2:
                            mmpSd2Terminate();
                            break;
#endif

#ifdef DTV_MS_ENABLE
                        case PAL_DEVICE_TYPE_MS:
                            mmpMsTerminate();
                            break;
#endif

                        //case PAL_DEVICE_TYPE_USB0...PAL_DEVICE_TYPE_USB7:
                        //    // USB can't be terminated, so switch USB mode by _IsUsbCardInsert().
                        //    _IsUsbCardInsert(i);
                        //    break;

                        default:
                            break;
                        }

                        storageMgr.deviceHandle[i].bInitialVolume = MMP_FALSE;
                        storageMgr.deviceHandle[i].retryCount     = 0;
                        storageMgr.deviceHandle[i].totalVolume    = 0;

                        *cardEvent                                = DTV_CARD_REMOVE;
                        *cardId                                   = i;
                        goto end;
                    }
                }
                break;
            }
        }
    }

end:
    return;
}

static void
_GetClientEvent(
    DTV_CARD_EVENT   *cardEvent,
    MMP_INT          *cardId)
{
#ifdef DTV_USB_ENABLE
    MMP_BOOL               bDeviceMode  = mmpOtgIsDeviceMode();
    static const MMP_ULONG attribList[] =
    {
        MMP_OTG_ATTRIB_NONE
    };

    #if 1 //kenny patch geniatech pc grabber
    switch (storageMgr.deviceModeChkState)
    {
    case DEV_MODE_CHK_STATE_IN_STANDALONE_MODE:
        if (bDeviceMode)
        {
            // double check
            MMP_Sleep(20);
            if (bDeviceMode != mmpOtgIsDeviceMode())
                break;  // false alarm

            // state changed
            if (mmpOtgDeviceModeOpen(attribList) == MMP_RESULT_SUCCESS)
            {
                storageMgr.bIsDeviceMode      = MMP_TRUE;
                *cardEvent                    = DTV_CARD_INSERT;
                *cardId                       = -1;
                storageMgr.deviceModeChkState = DEV_MODE_CHK_STATE_IN_DEVICE_MODE;
            }
            else
            {
                // Initial Fail
                mmpOtgDeviceModeClose();
                storageMgr.deviceModeChkState = DEV_MODE_CHK_STATE_IN_CHARG_MODE;
            }
        }
        break;

    case DEV_MODE_CHK_STATE_IN_DEVICE_MODE:
        if (!bDeviceMode)
        {
            MMP_RESULT result;

            // double check
            MMP_Sleep(20);
            if (bDeviceMode != mmpOtgIsDeviceMode())
                break;  // false alarm

            // state changed
            mmpOtgDeviceModeClose();
            storageMgr.bIsDeviceMode = MMP_FALSE;
            *cardEvent               = DTV_CARD_REMOVE;
            *cardId                  = -1;
            SendDelayedMsg(MSG_HOST_MODE, 100, 0, 0, 0);
        #if !defined(CONFIG_HAVE_USBD)
            result                   = storageMgrInitialize(MMP_TRUE);
            if (result)
                dbg_msg_ex(DBG_MSG_TYPE_ERROR, "storageMgrTerminate() fail (return 0x%x) !! \n", result);
        #endif
            storageMgr.deviceModeChkState = DEV_MODE_CHK_STATE_IN_STANDALONE_MODE;
        }
        break;

    case DEV_MODE_CHK_STATE_IN_CHARG_MODE:
        if (!bDeviceMode)
        {
            // double check
            MMP_Sleep(20);
            if (bDeviceMode != mmpOtgIsDeviceMode())
                break;  // false alarm

            // state changed
            storageMgr.deviceModeChkState = DEV_MODE_CHK_STATE_IN_STANDALONE_MODE;
        }
        break;
    }
    #else
    if (storageMgr.bIsDeviceMode != mmpOtgIsDeviceMode())
    {
        MMP_Sleep(20);
        bDeviceMode = mmpOtgIsDeviceMode();
        if (storageMgr.bIsDeviceMode != bDeviceMode)
        {
            MMP_INT result = 0;

            if (bDeviceMode == MMP_TRUE)
            {
        #if 0
            #if !defined(CONFIG_HAVE_USBD)
                result = storageMgrTerminate();
                if (result)
                {
                    dbg_msg_ex(DBG_MSG_TYPE_ERROR, "storageMgrTerminate() fail (return 0x%x) !!", result);
                    return;
                }
            #endif
        #endif

                SendDelayedMsg(MSG_DEVICE_MODE, 100, 0, 0, 0);
                if (mmpOtgDeviceModeOpen(attribList) == MMP_RESULT_SUCCESS)
                {
                    storageMgr.bIsDeviceMode = MMP_TRUE;
                    *cardEvent               = DTV_CARD_INSERT;
                    *cardId                  = -1;
                }
                else
                {
                    //Initial Fail
                    mmpOtgDeviceModeClose();
                }
            }
            else
            {
                mmpOtgDeviceModeClose();
                storageMgr.bIsDeviceMode = MMP_FALSE;
                *cardEvent               = DTV_CARD_REMOVE;
                *cardId                  = -1;
                SendDelayedMsg(MSG_HOST_MODE, 100, 0, 0, 0);
        #if !defined(CONFIG_HAVE_USBD)
                result                   = storageMgrInitialize(MMP_TRUE);
                if (result)
                    dbg_msg_ex(DBG_MSG_TYPE_ERROR, "storageMgrTerminate() fail (return 0x%x) !! \n", result);
        #endif
            }
        }
    }
    #endif
#endif
}

static void *
ThreadFunc(
    void *arg)
{
    MMP_UINT i = 0;

    // todo how to handle create

    while (1)
    {
        MMP_ULONG sleepTime = 200;         // ms

#ifdef ENABLE_STORAGE_THREAD_POLLING
        // polling card
        DTV_CARD_EVENT cardEvent = DTV_CARD_NOCHANGE;
        MMP_INT        cardId    = -1;

        if (storageMgr.bInitialized == MMP_TRUE)
        {
            //if (storageMgr.bIsDeviceMode == MMP_FALSE)
            _GetCardEvent(&cardEvent, &cardId);
    #ifdef ENABLE_USB_DEVICE
            _GetClientEvent(&cardEvent, &cardId);   // to detect if this device is inerted into PC
    #endif

            if (cardEvent != DTV_CARD_NOCHANGE)
            {
                // get card event
                if (cardId == -1)
                {
                    // get client event
                    storageMgr.cardEventInfo[PAL_DEVICE_TYPE_COUNT].bRefresh  = MMP_TRUE;
                    storageMgr.cardEventInfo[PAL_DEVICE_TYPE_COUNT].cardEvent = cardEvent;
                    //dbg_msg(DBG_MSG_TYPE_INFO, "get client event (%d)...........\n", storageMgr.cardEventInfo[PAL_DEVICE_TYPE_COUNT].cardEvent);
                }
                else
                {
                    // get card event
                    storageMgr.cardEventInfo[cardId].bRefresh  = MMP_TRUE;
                    storageMgr.cardEventInfo[cardId].cardEvent = cardEvent;
                }
                sleepTime = 5;
            }
        }

#endif

        //PalWaitEvent(storageMgr.eventMgrToThread, PAL_EVENT_INFINITE);
        if (storageMgr.bStartSearch == MMP_TRUE)
        {
            for (i = 0; i < DTV_VOLUME_NUM; i++)
            {
                if (storageMgr.bInterrupt == MMP_TRUE)
                    break;

                if (storageMgr.volumeInfo[i].bVolumeUsed == MMP_TRUE &&
                    storageMgr.volumeInfo[i].bReadyCollectVolumeInfo == MMP_FALSE)
                {
                    _CreateStorageDataBase(i,
                                           storageMgr.volumeInfo[i].startPath,
                                           storageMgr.volumeInfo[i].bRecursiveSearch);
                }
            }
        }

        PalSleep(sleepTime);
    }

    //PalSetEvent(storageMgr.eventThreadToMgr);
}

////////////////////////////////////////////////////////////////////////
// Public Function Definition
///////////////////////////////////////////////////////////////////////
static PAL_THREAD searchThread = MMP_NULL;

STORAGE_ERR
storageMgrInitialize(
    MMP_BOOL bFirstInit)
{
    STORAGE_ERR result = STORAGE_ERR_OK;
    MMP_UINT    i      = 0;

    PalMemset((void *)&storageMgr, 0, sizeof(DTV_STORAGE_MGR));

    storageMgr.bInitialized = MMP_FALSE;
    PalMemset((void *)storageMgr.initDuration, -1, sizeof(MMP_UINT32) * PAL_DEVICE_TYPE_COUNT);

#ifdef DTV_INTERNAL_DEV_ENABLE
    result                  = _InitialInternalStorage();
    if (result)
        dbg_msg(DBG_MSG_TYPE_ERROR, "\n\nerror 0x%x !! %s [#%d]\n", result, __FILE__, __LINE__);
#endif

    result = _InitialAllExternalStorages();
    if (result & ~NTFS_RESULT_FLAG)
        dbg_msg(DBG_MSG_TYPE_ERROR, "Init external storage error (0x%x) !! %s [#%d]\n", result, __FILE__, __LINE__);

    if (bFirstInit == MMP_TRUE)
    {
        searchThread = PalCreateThread(PAL_THREAD_STORAGE,
                                       ThreadFunc,
                                       MMP_NULL,
                                       2000,
                                       PAL_THREAD_PRIORITY_NORMAL);

        if (searchThread == MMP_NULL)
        {
            dbg_msg(DBG_MSG_TYPE_ERROR, "Create thread fail !! %s [#%d]\n", __FILE__, __LINE__);
            result = STORAGE_ERR_CREATE_THREAD_FAIL;
        }
    }

    storageMgr.bInitialized = MMP_TRUE;

    return result;
}

STORAGE_ERR
storageMgrTerminate(
    void)
{
    STORAGE_ERR result        = STORAGE_ERR_OK;
    MMP_INT     i             = 0;
    MMP_BOOL    bIsDeviceMode = MMP_FALSE;

    storageMgr.bStartSearch = MMP_FALSE;

    if (storageMgr.bDBCreating == MMP_TRUE)
    {
        do
        {
            PalSleep(5);
        } while (storageMgr.bDBCreating);
    }

    for (i = 0; i < DTV_VOLUME_NUM; i++)
    {
        if (storageMgr.volumeInfo[i].bVolumeUsed == MMP_TRUE)
            PalDelVolume(i);

        if (storageMgr.volumeInfo[i].bReadyCollectVolumeInfo == MMP_TRUE)
            _DestroyStorageDataBase(i);
    }

    for (i = 0; i < PAL_DEVICE_TYPE_COUNT; i++)
    {
        PalDelDriver(i);
        switch (i)
        {
#if (defined(DTV_SD1_ENABLE) || defined(DTV_MMC_ENABLE))
    #ifdef DTV_MMC_ENABLE
        case PAL_DEVICE_TYPE_MMC:
    #endif
    #ifdef DTV_SD1_ENABLE
        case PAL_DEVICE_TYPE_SD:
    #endif
            mmpSdTerminate();
            break;
#endif

#ifdef DTV_SD2_ENABLE
        case PAL_DEVICE_TYPE_SD2:
            mmpSd2Terminate();
            break;
#endif

#ifdef DTV_MS_ENABLE
        case PAL_DEVICE_TYPE_MS:
            mmpMsTerminate();
            break;
#endif

#ifdef DTV_USB_ENABLE
        case PAL_DEVICE_TYPE_USB0 ... PAL_DEVICE_TYPE_USB7:
            // USB can't be terminated, so switch USB mode by _IsUsbCardInsert().
            _IsUsbCardInsert(i);
            break;
#endif
        }
    }

    bIsDeviceMode            = storageMgr.bIsDeviceMode;
    PalMemset((void *)&storageMgr, 0, sizeof(DTV_STORAGE_MGR));
    storageMgr.bIsDeviceMode = bIsDeviceMode;

    return result;
}

void
storageMgrStartSearch(
    MMP_BOOL bRecursiveSearch,
    PAL_DEVICE_TYPE cardType,
    MMP_INT partitionIdx,
    MMP_WCHAR          *startPath)
{
    MMP_INT volumeNum = 0;

    if ( (volumeNum = storageMgrGetCardVolume(cardType, partitionIdx)) == -1)
    {
        dbg_msg_ex(DBG_MSG_TYPE_ERROR, "Search WRONG volume index !!\n");
        return;
    }

    if (storageMgr.volumeInfo[volumeNum].bReadyCollectVolumeInfo == MMP_TRUE)
        storageMgrClearDataBase(volumeNum);

    if (storageMgr.volumeInfo[volumeNum].startPath)
    {
        PalHeapFree(0, storageMgr.volumeInfo[volumeNum].startPath);
        storageMgr.volumeInfo[volumeNum].startPath = MMP_NULL;
    }

    if (startPath)
    {
        MMP_INT length = 1 + PalWcslen(startPath);      // L'\0'

        storageMgr.volumeInfo[volumeNum].startPath = (MMP_WCHAR *)PalHeapAlloc(0, length * sizeof(MMP_WCHAR));
        PalMemset(storageMgr.volumeInfo[volumeNum].startPath, 0, length * sizeof(MMP_WCHAR));
        PalWcscpy(storageMgr.volumeInfo[volumeNum].startPath, startPath);
    }
    else
        storageMgr.volumeInfo[volumeNum].startPath = MMP_NULL;

    storageMgr.volumeInfo[volumeNum].bRecursiveSearch = bRecursiveSearch;

    storageMgr.currCardType                           = cardType;
    storageMgr.bInterrupt                             = MMP_FALSE;
    storageMgr.bStartSearch                           = MMP_TRUE;
}

void
storageMgrInterruptSearch(
    void)
{
    storageMgr.bInterrupt   = MMP_TRUE;
    storageMgr.bStartSearch = MMP_FALSE;
}

void
storageMgrClearDataBase(
    MMP_UINT volume)
{
    DTV_VOLUME_INFO *currVolumeInfo = &storageMgr.volumeInfo[volume];
    MMP_UINT        maxTableCount   = (DTV_MAX_FILE_COUNT / DTV_SUB_FILE_TABLE_COUNT);
    MMP_BOOL        bStartSearch    = storageMgr.bStartSearch;

    storageMgr.bStartSearch = MMP_FALSE;

    if (storageMgr.bDBCreating == MMP_TRUE)
    {
        do
        {
            PalSleep(5);
        } while (storageMgr.bDBCreating);
    }

    if (currVolumeInfo->bReadyCollectVolumeInfo == MMP_TRUE)
    {
        MMP_UINT i, j, k = 0;
        for (j = 0; j < DTV_FILE_TYPE_COUNT; j++)
        {
            if (currVolumeInfo->mappingFileIndexTable[j])
            {
                PalHeapFree(PAL_HEAP_DEFAULT, currVolumeInfo->mappingFileIndexTable[j]);
                currVolumeInfo->mappingFileIndexTable[j] = MMP_NULL;
            }

            for (i = 0; i < maxTableCount; i++)
            {
                if (currVolumeInfo->baseFileTable[j][i])
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, currVolumeInfo->baseFileTable[j][i]);
                    currVolumeInfo->baseFileTable[j][i] = MMP_NULL;
                }
            }

            currVolumeInfo->fileCount[j] = 0;
        }

        maxTableCount = (DTV_MAX_DIR_COUNT / DTV_SUB_DIR_COUNT);

        for (j = 0; j < maxTableCount; j++)
        {
            if (currVolumeInfo->dirTable[j])
            {
                PalHeapFree(PAL_HEAP_DEFAULT, currVolumeInfo->dirTable[j]);
                currVolumeInfo->dirTable[j] = MMP_NULL;
            }
        }

        if (currVolumeInfo->startPath)
        {
            PalHeapFree(0, currVolumeInfo->startPath);
            currVolumeInfo->startPath = MMP_NULL;
        }
    }

    currVolumeInfo->dirCount                = 0;
    currVolumeInfo->bReadyCollectVolumeInfo = MMP_FALSE;
    currVolumeInfo->bInCollectStatus        = MMP_FALSE;

    storageMgr.bStartSearch                 = bStartSearch;
}

void
storageMgrGetCardEvent(
    DTV_CARD_EVENT   *cardEvent,
    MMP_INT          *cardId)
{
#ifdef ENABLE_STORAGE_THREAD_POLLING
    MMP_INT i;

    for (i = PAL_DEVICE_TYPE_SD; i < PAL_DEVICE_TYPE_COUNT; i++)
    {
        if (storageMgr.cardEventInfo[i].bRefresh == MMP_TRUE)
        {
            *cardEvent                            = storageMgr.cardEventInfo[i].cardEvent;
            *cardId                               = i;

            storageMgr.cardEventInfo[i].cardEvent = DTV_CARD_NOCHANGE;
            storageMgr.cardEventInfo[i].bRefresh  = MMP_FALSE;
            break;
        }
    }
#else
    _GetCardEvent(cardEvent, cardId);
#endif
}

MMP_BOOL
storageMgrGetUSBEvent(
    MMP_INT cardId)
{
    return (storageMgr.cardEventInfo[cardId].cardEvent == DTV_CARD_INSERT);
}

DTV_CARD_EVENT
storageMgrGetUSBEventStatus(
    MMP_INT cardId)
{
    return storageMgr.cardEventInfo[cardId].cardEvent;
}

void
storageMgrGetClientEvent(
    DTV_CARD_EVENT   *cardEvent,
    MMP_INT          *cardId)
{
#ifdef ENABLE_STORAGE_THREAD_POLLING
    if (storageMgr.cardEventInfo[PAL_DEVICE_TYPE_COUNT].bRefresh == MMP_TRUE)
    {
        *cardEvent                                               = storageMgr.cardEventInfo[PAL_DEVICE_TYPE_COUNT].cardEvent;
        *cardId                                                  = -1;

        storageMgr.cardEventInfo[PAL_DEVICE_TYPE_COUNT].bRefresh = MMP_FALSE;
    }
#else
    _GetClientEvent(cardEvent, cardId);
#endif
}

MMP_INT
storageMgrFormatVolume(
    MMP_INT volume)
{
    MMP_INT  result       = 0;
    MMP_UINT cardType     = 0;
    MMP_INT  partitionIdx = 0;
    void     *f_dev_init  = MMP_NULL;

    cardType     = storageMgr.volumeInfo[volume].cardType;
    partitionIdx = storageMgr.volumeInfo[volume].partitionIdx;

    f_dev_init = get_initfunc(cardType);
    if (!f_dev_init)
        return MMP_RESULT_ERROR;

    result = PalFormat(volume, cardType, partitionIdx, f_dev_init);
    if (result)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "PalFormat() fail (return = %d) !! %s [#%d]\n", result, __FILE__, __LINE__);
        goto end;
    }

    storageMgrClearDataBase(volume);

end:
    return result;
}

MMP_INT
storageMgrDeleteDir(
    MMP_WCHAR *searchpath)
{
    MMP_INT        result     = 0;
    MMP_BOOL       bFineDir   = MMP_FALSE;
    MMP_BOOL       fFinished  = MMP_FALSE;
    MMP_UINT       offset     = 0;
    MMP_WCHAR      path[256]  = {0};
    PAL_FILE_FIND  hSearch    = MMP_NULL;
    static MMP_INT currlevel  = 0;

    MMP_CHAR       tmpBuf[32] = {0};

    offset = PalWcslen(searchpath);

    PalWcscpy(&searchpath[offset], DTV_ALL_SEARCH);

    currlevel++;
    //sprintf(tmpBuf, "%*s1.", currlevel, " ");
    //_storageMgrPrintString(tmpBuf, searchpath);

    hSearch = PalTFileFindFirst(searchpath, MMP_NULL, MMP_NULL);
    if (hSearch == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    while (!fFinished)
    {
        if (PalTFindAttrIsDirectory(hSearch) &&
            PalWcscmp(L"..", (MMP_WCHAR *)PalTFileFindGetName(hSearch)) &&
            PalWcscmp(L".", (MMP_WCHAR *)PalTFileFindGetName(hSearch)) )
        {
            MMP_UINT offset1 = PalWcslen(PalTFileFindGetName(hSearch));

            bFineDir = MMP_TRUE;

            PalMemset(&searchpath[offset], 0, PalWcslen(DTV_ALL_SEARCH) * sizeof(MMP_WCHAR));
            PalWcscpy(&searchpath[offset], (MMP_WCHAR *)PalTFileFindGetName(hSearch));

            searchpath[offset + offset1] = L'/';
            offset1++;
            searchpath[offset + offset1] = L'\0';

            result                       = storageMgrDeleteDir(searchpath);
            if (result)
                return result;

            PalMemset(&searchpath[offset], 0, offset1 * sizeof(MMP_WCHAR));
        }
        else
        {
            MMP_INT   tmpLength     = 0;
            MMP_WCHAR *fileLastName = (MMP_WCHAR *)PalTFileFindGetName(hSearch);

            // Get the file name of the search handle
            PalWcscpy(path, searchpath);

            tmpLength = (bFineDir == MMP_TRUE) ? PalWcslen(path) : (PalWcslen(path) - PalWcslen(DTV_ALL_SEARCH));
            PalWcscpy(&path[tmpLength], fileLastName);

            if (PalWcscmp(L"..", fileLastName) &&
                PalWcscmp(L".", fileLastName) )
            {
                //sprintf(tmpBuf, "%*sDelFile: ", currlevel, " ");
                //_storageMgrPrintString(tmpBuf, path);

                PalTFileDelete(path, MMP_NULL, MMP_NULL);
            }
        }

        if (PalTFileFindNext(hSearch, MMP_NULL, MMP_NULL))
            fFinished = MMP_TRUE;
    }

    PalTFileFindClose(hSearch, MMP_NULL, MMP_NULL);

end:
    // reset the searchpath char
    PalMemset(&searchpath[offset], 0, (PalWcslen(searchpath) - offset) * sizeof(MMP_WCHAR));

    PalWcscpy(path, searchpath);
    path[PalWcslen(path) - 1] = L'\0';

    //sprintf(tmpBuf, "%*sDelDir: ", currlevel, " ");
    //_storageMgrPrintString(tmpBuf, path);

    PalTRemoveDir(path, MMP_NULL, MMP_NULL);

    currlevel--;
    return result;
}

///////////////////////////////////////////////////////////////
// Get ( Set ) Function Declaration
//////////////////////////////////////////////////////////////
MMP_BOOL
storageMgrCheckFileType(
    DTV_FILE_TYPE extType,
    MMP_WCHAR *filename)
{
    MMP_BOOL valid       = MMP_FALSE;

    MMP_INT  i, itemGap = 0;
    MMP_INT  searchStart = 0, searchEnd = 0;

    switch (extType)
    {
    case DTV_FILE_TYPE_JPG:
        searchStart = DTV_EXT_SEARCH_JPG;
        searchEnd   = DTV_EXT_SEARCH_PHOTO_COUNT;
        break;

    case DTV_FILE_TYPE_AUDIO:
        searchStart = DTV_EXT_SEARCH_MP3;
        searchEnd   = DTV_EXT_SEARCH_AUDIO_COUNT;
        break;

    case DTV_FILE_TYPE_VIDEO:
        searchStart = DTV_EXT_SEARCH_3GP;
        searchEnd   = DTV_EXT_SEARCH_VIDEO_COUNT;
        break;

    case DTV_FILE_TYPE_RECORD:
        searchStart = DTV_EXT_SEARCH_REC;
        searchEnd   = DTV_EXT_SEARCH_RECORD_COUNT;
        break;

    case DTV_FILE_TYPE_FW:
        searchStart = DTV_EXT_SEARCH_IMG;
        searchEnd   = DTV_EXT_SEARCH_FW_COUNT;
        break;

    case DTV_FILE_TYPE_VOICE:
        searchStart = DTV_EXT_SEARCH_AMR;
        searchEnd   = DTV_EXT_SEARCH_COUNT;
        break;

    default:
        searchStart = 0;
        searchEnd   = 0;
        break;
    }

    itemGap = 0;
    for (i = searchStart; i < searchEnd; i++)
    {
        if (i > DTV_EXT_SEARCH_FW_COUNT)
            itemGap = 5;
        else if (i > DTV_EXT_SEARCH_RECORD_COUNT)
            itemGap = 4;
        else if (i > DTV_EXT_SEARCH_VIDEO_COUNT)
            itemGap = 3;
        else if (i > DTV_EXT_SEARCH_AUDIO_COUNT)
            itemGap = 2;
        else if (i > DTV_EXT_SEARCH_PHOTO_COUNT)
            itemGap = 1;
        else
            itemGap = 0;

        if (_checkFileExtention(FileExtArray[i - itemGap], (MMP_WCHAR *)filename) )
        {
            valid = MMP_TRUE;
            break;
        }
    }

    return valid;
}

MMP_BOOL
storageMgrIsVolumeReady(
    PAL_DEVICE_TYPE cardType,
    MMP_INT partitionIdx)
{
    MMP_BOOL result = MMP_FALSE;
    MMP_INT  i;

    for (i = 0; i < DTV_VOLUME_NUM; i++)
    {
        if (storageMgr.volumeInfo[i].cardType == cardType &&
            storageMgr.volumeInfo[i].partitionIdx == partitionIdx &&
            storageMgr.volumeInfo[i].bReadyCollectVolumeInfo == MMP_TRUE)
            result = MMP_TRUE;
    }

    return result;
}

MMP_BOOL
storageMgrIsCardInsert(
    PAL_DEVICE_TYPE cardType)
{
#if 1
    // main thread will use to read sector in device
    if (_storageIsInsert(cardType) == DTV_CARD_STATUS_INSERT)
        return MMP_TRUE;
    else
        return MMP_FALSE;
#else
    return storageMgr.deviceHandle[cardType].bInsert;
#endif
}

MMP_BOOL
storageMgrGetClientStatus(
    void)
{
    return storageMgr.bIsDeviceMode;
}

//void
//storageMgrSetClientStatus(
//    MMP_BOOL bIsDeviceMode)
//{
//    storageMgr.bIsDeviceMode = bIsDeviceMode;
//}

PAL_DEVICE_TYPE
storageMgrGetVolumeCard(
    MMP_INT volume,
    MMP_INT     *partitionIdx)
{
    if (partitionIdx)
        *partitionIdx = storageMgr.volumeInfo[volume].partitionIdx;

    return storageMgr.volumeInfo[volume].cardType;
}

MMP_INT
storageMgrGetCardVolume(
    PAL_DEVICE_TYPE cardType,
    MMP_INT partitionIdx)
{
    MMP_INT i;

    for (i = 0; i < DTV_VOLUME_NUM; i++)
    {
        if (storageMgr.volumeInfo[i].cardType == cardType &&
            storageMgr.volumeInfo[i].partitionIdx == partitionIdx)
            return i;
    }

    return (-1);
}

MMP_BOOL
storageMgrIsSearchRecursive(
    MMP_INT volume)
{
    return storageMgr.volumeInfo[volume].bRecursiveSearch;
}

MMP_INT
storageMgrGetVolumeCountInCard(
    PAL_DEVICE_TYPE cardType)
{
    return storageMgr.deviceHandle[cardType].totalVolume;
}

MMP_INT
storageMgrGetVolumeInfo(
    MMP_INT volume,
    DTV_FILE_TYPE fileType,
    MMP_UINT *dirCount,
    MMP_UINT *fileCount,
    MMP_BOOL *volumeUsed,
    MMP_BOOL *databaseStatus)
{
    MMP_INT result = 0;

    /*if(storageMgr.volumeInfo[volume].bReadyCollectVolumeInfo == MMP_FALSE)
       {
        result = -1;
        goto end;
       }*/

    if (volume == -1)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "Wrong volume index !! %s [#%d]\n", __FILE__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    if (dirCount)
        *dirCount = storageMgr.volumeInfo[volume].dirCount;

    if (fileCount)
        *fileCount = storageMgr.volumeInfo[volume].fileCount[fileType];

    if (volumeUsed)
        *volumeUsed = storageMgr.volumeInfo[volume].bVolumeUsed;

    if (databaseStatus)
        *databaseStatus = storageMgr.volumeInfo[volume].bReadyCollectVolumeInfo;

end:
    return result;
}

MMP_INT
storageMgrGetFileInfo(
    MMP_INT volume,
    DTV_FILE_TYPE fileType,
    MMP_UINT fileIndex,
    MMP_WCHAR        *fPathName,
    MMP_UINT32       *fSize,
    MMP_UINT32       *fDateTime,
    MMP_WCHAR        *fLastName,
    SF_POS           *currFilePos)
{
    MMP_INT         result                             = 0;
    MMP_UINT        i                                  = 0;
    MMP_UINT        dirCount                           = 0;
    MMP_UINT        dirIndex                           = 0;
    MMP_UINT        tempcount                          = 0;
    PAL_FILE_FIND   hSearch;
    MMP_BOOL        findpos                            = MMP_FALSE;
    MMP_WCHAR       path[MAX_FILE_FULL_PATH_LEN]       = {0};
    MMP_WCHAR       tmpPath[256]                       = {0};
    SF_POS          pos;
    MMP_UINT        len                                = 0;
    MMP_UINT        mappingfileindex                   = 0;
    MMP_INT         usedFileTableIdx                   = 0;
    MMP_INT         subFileIndex                       = 0;
    MMP_INT         usedDirTableIdx[DTV_MAX_DIR_LEVEL] = { -1};
    MMP_INT         subDirIndex[DTV_MAX_DIR_LEVEL]     = { -1};
    DTV_VOLUME_INFO *currVolumeInfo                    = MMP_NULL;

    if (volume == -1)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "Wrong volume index !! %s [#%d]\n", __FILE__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    if (storageMgr.volumeInfo[volume].bReadyCollectVolumeInfo == MMP_FALSE)
    {
        result = MMP_RESULT_ERROR;
        dbg_msg(DBG_MSG_TYPE_ERROR, "This volume is not ready !! %s [#%d]\n", __FILE__, __LINE__);
        goto end;
    }

    currVolumeInfo = &storageMgr.volumeInfo[volume];

    if (currVolumeInfo->bRecursiveSearch == MMP_TRUE)
    {
        mappingfileindex = currVolumeInfo->mappingFileIndexTable[fileType][fileIndex].fileIndex;
    }
    else
    {
        mappingfileindex = fileIndex;
    }

    //if (fPathName || fLastName)
    findpos = MMP_TRUE;

    if (currFilePos)
    {
        usedFileTableIdx = mappingfileindex / DTV_SUB_FILE_TABLE_COUNT;
        subFileIndex     = mappingfileindex - (usedFileTableIdx * DTV_SUB_FILE_TABLE_COUNT);

        PalMemcpy((void *)currFilePos,
                  (void *)&currVolumeInfo->baseFileTable[fileType][usedFileTableIdx][subFileIndex].position,
                  sizeof(SF_POS));
    }

    if (findpos)
    {
        if (currVolumeInfo->startPath)
            PalWcscpy(path, currVolumeInfo->startPath);
        else
            _GetVolumeName(volume, path);

        // ---------------------------------------
        if (currVolumeInfo->bRecursiveSearch == MMP_TRUE)
        {
            // get dir path
            MMP_INT tmpUsedDirTableIdx = 0;
            MMP_INT tmpSubDirIndex     = 0;

            dirCount = currVolumeInfo->dirCount;
            for (i = 0; i < dirCount; i++)
            {
                tempcount += currVolumeInfo->dirTable[i / DTV_SUB_DIR_COUNT][i % DTV_SUB_DIR_COUNT].fileCountInDir[fileType];
                if (tempcount > fileIndex)
                {
                    dirIndex = i;
                    break;
                }
            }

            while (dirIndex > 0)
            {
                MMP_INT8 dirLevel = 0;

                tmpUsedDirTableIdx            = dirIndex / DTV_SUB_DIR_COUNT;
                tmpSubDirIndex                = dirIndex - (tmpUsedDirTableIdx * DTV_SUB_DIR_COUNT);

                dirLevel                      = currVolumeInfo->dirTable[tmpUsedDirTableIdx][tmpSubDirIndex].dirLevel;

                usedDirTableIdx[dirLevel - 1] = tmpUsedDirTableIdx;
                subDirIndex[dirLevel - 1]     = tmpSubDirIndex;
                dirIndex                      = currVolumeInfo->dirTable[tmpUsedDirTableIdx][tmpSubDirIndex].srcDirIndex;
            }

            for (i = 0; i < DTV_MAX_DIR_LEVEL; i++)
            {
                if (usedDirTableIdx[i] <= 0 && subDirIndex[i] <= 0)
                    break;

                tmpUsedDirTableIdx = usedDirTableIdx[i];
                tmpSubDirIndex     = subDirIndex[i];

                PalWcscpy(tmpPath, path);

                hSearch = PalWFileFindByPos(volume, tmpPath, &currVolumeInfo->dirTable[tmpUsedDirTableIdx][tmpSubDirIndex].position);
                if (hSearch)
                {
                    PalWcscpy(path, tmpPath);
                    PalWcscpy(&path[PalWcslen(path)], PalTFileFindGetName(hSearch));
                    PalWcscpy(&path[PalWcslen(path)], L"/");

                    PalTFileFindClose(hSearch, MMP_NULL, MMP_NULL);
                }
                else
                {
                    result = MMP_RESULT_ERROR;
                    dbg_msg(DBG_MSG_TYPE_ERROR, "fail !!!!! %s [#%d]\n", __FILE__, __LINE__);
                    goto end;
                }
            }
        }
        // ---------------------------------------------------------

        usedFileTableIdx = mappingfileindex / DTV_SUB_FILE_TABLE_COUNT;
        subFileIndex     = mappingfileindex - (usedFileTableIdx * DTV_SUB_FILE_TABLE_COUNT);

        PalMemset(&pos, 0, sizeof(SF_POS));
        PalMemcpy(&pos, &currVolumeInfo->baseFileTable[fileType][usedFileTableIdx][subFileIndex].position, sizeof(SF_POS));
        //dbg_msg(DBG_MSG_TYPE_INFO, "check pos = %d-th\n", pos.pos);

        // get file name
        hSearch = PalWFileFindByPos(volume, path, &pos);
        if (hSearch)
        {
            if (fPathName)
            {
                PalWcscpy(fPathName, path);
                PalWcscpy(&fPathName[PalWcslen(fPathName)], PalTFileFindGetName(hSearch));
            }

            if (fLastName)
                PalWcscpy(fLastName, PalTFileFindGetName(hSearch));

            if (fSize)
                *fSize = PalTFileFindGetSize(hSearch);

            if (fDateTime)
            {
                *fDateTime = PalTFileFindGetDate(hSearch);
                *fDateTime = (*fDateTime << 16) + PalTFileFindGetTime(hSearch);
            }

            PalTFileFindClose(hSearch, MMP_NULL, MMP_NULL);
        }
        else
        {
            dbg_msg(DBG_MSG_TYPE_ERROR, "fail !!!!! %s [#%d]\n", __FILE__, __LINE__);
            result = MMP_RESULT_ERROR;
        }
    }

end:
    return result;
}

MMP_INT
storageMgrGetDirInfo(
    MMP_INT volume,
    DTV_FILE_TYPE fileType,
    MMP_UINT currDirIndex,
    MMP_WCHAR        *dirPath,
    MMP_WCHAR        *dirName,
    SF_POS           *currDirPos)
{
    MMP_INT         result          = 0;
    MMP_UINT        i               = 0;
    MMP_WCHAR       tempPath[256]   = {0};
    MMP_WCHAR       path[256]       = {0};
    PAL_FILE_FIND   hSearch;
    SF_POS          pos;
    DTV_VOLUME_INFO *currVolumeInfo = MMP_NULL;
    MMP_UINT        usedDirTable    = currDirIndex / DTV_SUB_DIR_COUNT;
    MMP_UINT        subDirIndex     = currDirIndex - (usedDirTable * DTV_SUB_DIR_COUNT);

    if (volume == -1)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "Wrong volume index !! %s [#%d]\n", __FILE__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }

    currVolumeInfo = &storageMgr.volumeInfo[volume];

    //  if ( fileCount )
    //      *fileCount = currVolumeInfo->dirTable[usedDirTable][subDirIndex].fileCountInDir[fileType];

    if (currDirPos)
        PalMemcpy((void *)currDirPos,
                  (void *)&currVolumeInfo->dirTable[usedDirTable][subDirIndex].position,
                  sizeof(SF_POS));

    if (dirName || dirPath)
    {
        PalMemset(&pos, 0, sizeof(SF_POS));
        PalMemcpy(&pos, &currVolumeInfo->dirTable[usedDirTable][subDirIndex].position, sizeof(SF_POS));
        //dbg_msg(DBG_MSG_TYPE_INFO, "check pos = %d-th\n", pos.pos);

        if (currVolumeInfo->startPath)
            PalWcscpy(path, currVolumeInfo->startPath);
        else
            _GetVolumeName(volume, path);

        if (currDirIndex != 0)
        {
            hSearch = PalWFileFindByPos(volume, path, &pos);
            if (hSearch)
            {
                if (dirName)
                    PalWcscpy(dirName, PalTFileFindGetName(hSearch));

                if (dirPath)
                {
                    PalWcscpy(dirPath, path);
                    PalWcscpy(&dirPath[PalWcslen(dirPath)], PalTFileFindGetName(hSearch));
                    PalWcscpy(&dirPath[PalWcslen(dirPath)], L"/");
                }

                PalTFileFindClose(hSearch, MMP_NULL, MMP_NULL);
            }
        }
        else
        {
            if (dirName)
                PalWcscpy(dirName, path);

            if (dirPath)
                PalWcscpy(dirPath, path);
        }
    }

end:
    return result;
}

void
storageMgrUsbSuspendMode(
    MMP_BOOL bEnable)
{
    storageMgr.bUsbSuspendMode = bEnable;
}

//DTV_USB_SPEED_TYPE
//storageMgrGetUsbSpeedType(
//    void)
//{
//    MMP_INT speed = 0;
//    MMP_INT usbIndex;
//
//    if ( storageMgr.bUsbSuspendMode == MMP_TRUE )
//        usbIndex = (0x10 | USB0);
//    else
//        usbIndex = USB0;
//
//    speed = mmpUsbExGetSpeed(usbIndex);
//
//    switch( speed )
//    {
//        case USBEX_SPEED_FULL:
//        case USBEX_SPEED_LOW:
//            return USB_1_1;
//            break;
//
//        case USBEX_SPEED_HIGH:
//            return USB_2_0;
//            break;
//
//        default:
//            return USB_UNKNOWN;
//            break;
//    }
//}

FILE_SYSTEM_FORMAT
storageMgrGetVolumeFileSystemFormat(
    MMP_INT volume)
{
    //trac("volume: %u\n", volume);
    if (volume < DTV_VOLUME_NUM)
    {
        //trac("format: %u\n", storageMgr.volumeInfo[volume].fileSystemFormat);
        return storageMgr.volumeInfo[volume].fileSystemFormat;
    }
    else
    {
        return FAT_FILE_SYSTEM;
    }
}
