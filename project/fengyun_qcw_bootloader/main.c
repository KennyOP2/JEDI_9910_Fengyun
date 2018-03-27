/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file main.c
 *
 * @author I-Chun Lai
 */

#include "global.h"
#include "rom_parser.h"
#include "../freertos/include/or32/sys.h"
#include "grabber_control.h"
#ifdef HAVE_FAT
    #include "mmp_usbex.h"
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct PCMODE_LED_CTRL_TAG
{
    LED_STATUS AnalogLed;
    LED_STATUS HDMILed;
    LED_STATUS SD720PLed;
    LED_STATUS HD1080PLed;
    LED_STATUS RecordLed;
    LED_STATUS SignalLed;
} PCMODE_LED_CTRL;

//=============================================================================
//                              Constant Definition
//=============================================================================
#define USB0_ENABLE     (0x1 << 0)
#define USB1_ENABLE     (0x1 << 1)
#define USB_ENABLE_MODE (USB0_ENABLE | USB1_ENABLE)

//=============================================================================
//                              Macro Definition
//=============================================================================
typedef void (*PF_CRT0_MEMCPY32)(unsigned int, unsigned int, unsigned int);

//=============================================================================
//                              Extern Reference
//=============================================================================
extern void         *heap_ptr;
extern unsigned int _heap_end;

//=============================================================================
//                              Global Data Definition
//=============================================================================
MMP_BOOL               gbDeviceMode            = MMP_TRUE;
static PCMODE_LED_CTRL gtPCModeLed;
MMP_BOOL               gbPC_MODE_ENALBE_RECORD = MMP_FALSE;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static MMP_INT
_boot(
    MMP_UINT8  *codeAddr,
    MMP_UINT32 codeSize,
    MMP_UINT   pNewCrtMemcpy32);

static MMP_BOOL
_IsPCConnectMode(
    void);

//=============================================================================
//                              Public Function Definition
//=============================================================================
static MMP_INT
Initialize(
    void)
{
    MMP_BOOL  bResult     = MMP_FALSE;
    MMP_UINT8 *pBuf       = MMP_NULL;
    MMP_UINT8 *pBinBuf    = MMP_NULL;
    MMP_UINT8 *pCpyBuffer = MMP_NULL;

    _coreIsPCConnectMode = _IsPCConnectMode;
    // chip warm up
    if (1)
    {
        volatile MMP_UINT16 value;
        MMP_UINT32          cnt  = 10;
        MMP_UINT32          cnt1 = 0;
        MMP_UINT32          i;

        // fire ISP
        HOST_WriteRegister(0x0500, 0x0019);

        for (i = 0; i < 55000 * 80; i++)
            asm ("");

        while (cnt-- != 0)
        {
            HOST_ReadRegister(0x34a, &value);
            dbg_msg(DBG_MSG_TYPE_INFO, "Rd %x\n", value);
            if ((value & 0xF) == 0xE)
                cnt1++;
        }

        // stop capture & isp
        HOST_WriteRegister(0x2018, 0x0000);
        HOST_WriteRegister(0x060a, 0x0000);

        // wait isp idle
        HOST_ReadRegister(0x6fc, &value);

        while ((value & 0x1) != 0)
        {
            HOST_ReadRegister(0x6fc, &value);
        }

        // wait capture idle
        HOST_ReadRegister(0x1f22, &value);

        while ((value & 0x80c1) != 0x80c1)
        {
            HOST_ReadRegister(0x1f22, &value);
        }

        // read dram status
        HOST_ReadRegister(0x34a, &value);

        if (((value & 0xF) == 0xC && cnt1 == 0) || (value & 0xF) == 0x8)
        {
            HOST_WriteRegister(0x340, 0x2a54);
            for (i = 0; i < 30000; i++)
                asm ("");
            dbg_msg(DBG_MSG_TYPE_INFO, "Update 0x340\n");
        }

        HOST_ReadRegister(0x3a6, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x3a6) %x\n", value);

        HOST_ReadRegister(0x340, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x340) %x\n", value);

        HOST_ReadRegister(0x342, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x342) %x\n", value);

        HOST_ReadRegister(0x344, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x344) %x\n", value);

        HOST_ReadRegister(0x346, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x346) %x\n", value);

        HOST_ReadRegister(0x348, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x348) %x\n", value);

        HOST_ReadRegister(0x34a, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x34a) %x\n", value);

        HOST_ReadRegister(0x34e, &value);
        dbg_msg(DBG_MSG_TYPE_INFO, "Mem Addr (0x34e) %x\n", value);
    }

    mmpWatchDogDisable();
    // Set the watch dog timer to 5 second to ensure later copy process can be done in time.
    mmpWatchDogEnable(5);
    printf("Bootloader init start ............................. %d\n", (MMP_UINT32) ((void *)(&_heap_end) - heap_ptr));
    ithIntrInit();
    mmpDmaInitialize();

    do
    {
        mmpWatchDogRefreshTimer();
        // 1. Init NOR
        BREAK_ON_ERROR(MMP_SUCCESS != NorInitial());

        {
#define NOR_SECTOR_SIZE (64 * 1024)
#define FW_BASE_ADDR    (0)
            ROM_INFO romInfo         = {0};
            MMP_UINT32 fwRomBaseAddr = 0;
            MMP_UINT32 norSize       = NorCapacity();
            MMP_UINT32 crc32;

            mmpWatchDogRefreshTimer();
            // 2. read boot.rom info
            pBuf = MALLOC(NOR_SECTOR_SIZE);
            BREAK_ON_ERROR(!pBuf);
            BREAK_ON_ERROR(MMP_SUCCESS != NorRead(pBuf, 0, NOR_SECTOR_SIZE));
            BREAK_ON_ERROR(MMP_SUCCESS != romParser_GetRomInfo(pBuf, NOR_SECTOR_SIZE, &romInfo));
            PalPrintf("dataOffset(0x%X) dataSize(%d) binSize(%d) crc32(0x%08X)\n",
                      romInfo.dataOffset,
                      romInfo.dataSize,
                      romInfo.binSize,
                      romInfo.crc32);

            mmpWatchDogRefreshTimer();
            // 3. calculate the base address of the firmware rom
            fwRomBaseAddr = romInfo.dataOffset + romInfo.dataSize;
            fwRomBaseAddr = (fwRomBaseAddr + NOR_SECTOR_SIZE - 1) / NOR_SECTOR_SIZE * NOR_SECTOR_SIZE;

            mmpWatchDogRefreshTimer();
            // 4. read firmware rom info
            BREAK_ON_ERROR(MMP_SUCCESS != NorRead(pBuf, fwRomBaseAddr, NOR_SECTOR_SIZE));
            BREAK_ON_ERROR(MMP_SUCCESS != romParser_GetRomInfo(pBuf, NOR_SECTOR_SIZE, &romInfo));
            PalPrintf("dataOffset(0x%X) dataSize(%d) binSize(%d) crc32(0x%08X)\n",
                      romInfo.dataOffset,
                      romInfo.dataSize,
                      romInfo.binSize,
                      romInfo.crc32);
            BREAK_ON_ERROR(norSize < (romInfo.dataOffset + romInfo.dataSize));
            FREE(pBuf);

            mmpWatchDogRefreshTimer();
            // 5. decompress firmware from rom to bin
            pBuf       = MALLOC(romInfo.dataSize);
            pBinBuf    = MALLOC(romInfo.binSize);
            //Allocate larger memeories to ensure copy function buffer address behind image buffer
            //to prevent function buffer been overwrited during image copy process.
            pCpyBuffer = MALLOC(romInfo.binSize);
            BREAK_ON_ERROR(!pBuf);
            BREAK_ON_ERROR(!pBinBuf);
            BREAK_ON_ERROR(!pCpyBuffer);

            BREAK_ON_ERROR(MMP_SUCCESS != NorRead(pBuf, fwRomBaseAddr + romInfo.dataOffset, romInfo.dataSize));
            BREAK_ON_ERROR(MMP_SUCCESS != do_decompress(&pBuf[4], pBinBuf));
            PalPrintf("pBinBuf(0x%08X)\n", pBinBuf);

            mmpWatchDogRefreshTimer();
            // 6. calculate and check the crc32 value of the decompressed firmware
            crc32 = cksum(pBinBuf, romInfo.binSize);
            PalPrintf("crc32(0x%08X)\n", crc32);
#if 1
            BREAK_ON_ERROR(crc32 != romInfo.crc32);
#else
            BREAK_ON_ERROR(crc32 == romInfo.crc32);
#endif

            mmpWatchDogRefreshTimer();
            // 7. copy the decompressed firmware to the address 0x0 and jump there to start booting
            BREAK_ON_ERROR(MMP_SUCCESS != _boot(pBinBuf, romInfo.binSize, (MMP_UINT) pCpyBuffer));
            FREE(pBuf);
            FREE(pBinBuf);
        }

        // bResult = BootDtv();
        bResult = MMP_TRUE;
    } while (0);

    if (pBuf) FREE(pBuf);
    if (pBinBuf) FREE(pBinBuf);

    PalPrintf("Boot failed! Enter rescue mode\n");

    do
    {
        config_load();
        mmpWatchDogRefreshTimer();
        serial_number_load();
        mmpWatchDogRefreshTimer();
#ifdef HAVE_FAT
        // USB/Device initial
        BREAK_ON_ERROR(MMP_SUCCESS != mmpUsbExInitialize(USB_ENABLE_MODE));
        mmpWatchDogRefreshTimer();
        // Create task for USB host/device driver
        /** This is HC driver task, and it will never be destroyed. */
        BREAK_ON_ERROR(!PalCreateThread(PAL_THREAD_USBEX,
                                        USBEX_ThreadFunc,
                                        MMP_NULL,
                                        2000,
                                        PAL_THREAD_PRIORITY_NORMAL));
        mmpWatchDogRefreshTimer();
    #ifdef ENABLE_USB_DEVICE
        BREAK_ON_ERROR(!PalCreateThread(PAL_THREAD_USB_DEVICE,
                                        DEVICE_ThreadFunc,
                                        MMP_NULL,
                                        2000,
                                        PAL_THREAD_PRIORITY_NORMAL));
    #endif
        mmpWatchDogRefreshTimer();
        // register usb function drivers to USB driver
        mmpMscDriverRegister();
        mmpWatchDogRefreshTimer();
        PalSleep(1);

        BREAK_ON_ERROR(MMP_SUCCESS != storageMgrInitialize(MMP_TRUE));
        mmpWatchDogRefreshTimer();
        PalSleep(0);

        BREAK_ON_ERROR(MMP_SUCCESS != PalFileInitialize());
#endif
    } while (0);

    if (MMP_FALSE == bResult)
        printf("%d [fail 0x%x]\n", (MMP_UINT32) ((void *)(&_heap_end) - heap_ptr), bResult);

    trac("init leave\n");
    mmpWatchDogDisable();
    return bResult;
}

static MMP_INT
MainLoop(
    void)
{
    extern int it_usbd_main();
    it_usbd_main();
}

int appmain(
    void)
{
    MMP_INT result = 0;

    result = Initialize();
    if (result)
        goto end;

    result = MainLoop();
    if (result)
        goto end;

end:
    return result;
}

void
GrabberControlSetParam(
    GRABBER_CTRL_PARAM *pCtrl)
{
    switch (pCtrl->flag)
    {
    case GRABBER_CTRL_MIC_VOL:
        config_set_microphone_volume((MMP_UINT32)pCtrl->micVol);
        break;

    case GRABBER_CTRL_LINE_BOOST:
        config_set_line_boost((MMP_UINT32)pCtrl->lineBoost);
        break;

    case GRABBER_CTRL_HDCP:
        break;

    case GRABBER_CTRL_AUDIO_BITRATE:
        config_set_audio_bitrate(pCtrl->audiobitrate);
        break;

    case GRABBER_CTRL_HDMI_AUDIO_MODE:
        break;

    case GRABBER_CTRL_ANALOG_LED:
        gtPCModeLed.AnalogLed = pCtrl->AnalogLed;
        break;

    case GRABBER_CTRL_HDMI_LED:
        gtPCModeLed.HDMILed = pCtrl->HDMILed;
        break;

    case GRABBER_CTRL_RECORD_LED:
        gtPCModeLed.RecordLed = pCtrl->RecordLed;
        break;

    case GRABBER_CTRL_SIGNAL_LED:
        gtPCModeLed.SignalLed = pCtrl->SignalLed;
        break;

    case GRABBER_CTRL_SD720P_LED:
        gtPCModeLed.SD720PLed = pCtrl->SD720PLed;
        break;

    case GRABBER_CTRL_HD1080P_LED:
        gtPCModeLed.HD1080PLed = pCtrl->HD1080PLed;
        break;

    case GRABBER_CTRL_VIDEO_COLOR:
        {
            config_set_video_color_param(
                pCtrl->COLOR.brightness,
                pCtrl->COLOR.contrast,
                pCtrl->COLOR.hue,
                pCtrl->COLOR.saturation);
        }
        break;

    case GRABBER_CTRL_DIGITAL_AUDIO:
        config_set_digital_audio_volume(pCtrl->digitalVolume);
        break;

    default:
        break;
    }
}

void
GrabberControlGetParam(
    GRABBER_CTRL_PARAM *pCtrl)
{
    MMP_UINT32 i;
    MMP_UINT32 aspectRatio;
    MMP_UINT32 volume;
    MMP_UINT32 lineboost;
    DIGITAL_AUDIO_VOLUME digitalvolume;

    switch (pCtrl->flag)
    {
    case GRABBER_CTRL_MIC_VOL:
        config_get_microphone_volume(&volume);
        pCtrl->micVol = volume;
        break;

    case GRABBER_CTRL_LINE_BOOST:
        config_get_line_boost(&lineboost);
        pCtrl->lineBoost = lineboost;
        break;

    case GRABBER_CTRL_HDCP:
        pCtrl->bIsHDCP = MMP_FALSE;
        break;

    case GRABBER_CTRL_AUDIO_BITRATE:
        config_get_audio_bitrate(&pCtrl->audiobitrate);
        break;

    case GRABBER_CTRL_HDMI_AUDIO_MODE:
        pCtrl->hdmiAudioMode = mmpHDMIRXGetProperty(HDMIRX_AUDIO_MODE);
        break;

    case GRABBER_CTRL_ANALOG_LED:
        pCtrl->AnalogLed = gtPCModeLed.AnalogLed;
        break;

    case GRABBER_CTRL_HDMI_LED:
        pCtrl->HDMILed = gtPCModeLed.HDMILed;
        break;

    case GRABBER_CTRL_RECORD_LED:
        pCtrl->RecordLed = gtPCModeLed.RecordLed;
        break;

    case GRABBER_CTRL_SIGNAL_LED:
        pCtrl->SignalLed = gtPCModeLed.SignalLed;
        break;

    case GRABBER_CTRL_SD720P_LED:
        pCtrl->SD720PLed = gtPCModeLed.SD720PLed;
        break;

    case GRABBER_CTRL_HD1080P_LED:
        pCtrl->HD1080PLed = gtPCModeLed.HD1080PLed;
        break;

    case GRABBER_CTRL_VIDEO_COLOR:
        config_get_video_color_param(
            &(pCtrl->COLOR.brightness),
            &(pCtrl->COLOR.contrast),
            &(pCtrl->COLOR.hue),
            &(pCtrl->COLOR.saturation));
        break;

    case GRABBER_CTRL_DIGITAL_AUDIO:
        config_get_digital_audio_volume(&digitalvolume);
        pCtrl->digitalVolume = digitalvolume;
        break;

    default:
        break;
    }
}

MMP_BOOL
_IsPCConnectMode(
    void)
{
    return MMP_FALSE;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_INT
_boot(
    MMP_UINT8  *codeAddr,
    MMP_UINT32 codeSize,
    MMP_UINT   pNewCrtMemcpy32)
{
    extern void crt0_memcpy32(char *dst, char *src, unsigned int length_in_byte);
    extern char crt0_memcpy32_end;
    MMP_INT result                     = -1;
    MMP_UINT crtMemcpy32Length         = ((MMP_UINT)&crt0_memcpy32_end) - ((MMP_UINT)&crt0_memcpy32);
    MMP_UINT bootOffset                = 0;
    PF_CRT0_MEMCPY32 pfn_crt0_memcpy32 = MMP_NULL;

    // relocating crt0_memcpy32()
    do
    {
        BREAK_ON_ERROR(!pNewCrtMemcpy32)
        dc_invalidate();
        ic_invalidate();
        pfn_crt0_memcpy32 = (PF_CRT0_MEMCPY32)((pNewCrtMemcpy32) & ~0x03UL); // 4-byte alignment
        memcpy((void *)pfn_crt0_memcpy32, &crt0_memcpy32, crtMemcpy32Length);

        printf("[BOOTLOADER]: Start booting...\n");
        pfn_crt0_memcpy32(bootOffset, (MMP_UINT)codeAddr + bootOffset, codeSize - bootOffset + 3);
        printf("[BOOTLOADER]: End booting...\n");

        FREE(pNewCrtMemcpy32);
    } while (0);
    return result;
}