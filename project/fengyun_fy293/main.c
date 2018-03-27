#include "mmp_types.h"
#include "pal/thread.h"
#include "mmp_i2s.h"
#include "mmp_usbex.h"
#include "core_interface.h"
#include "pal/pal.h"
#include "hdmi_loop_through.h"
#include "mmp_capture.h"
#include "gpio_key_led_map.h"
#include "mmp_nor.h"
#include "mmp_timer.h"
#include "msg_route.h"
#include "config.h"
#include "grabber_control.h"
#include "mmp_isp.h"
#include "storage_mgr.h"

#define INPUT_SWITCH_YPBPR_CVBS
#define AUTO_SWITCH_SOURCE
#define SUPPORT_IR_CONTROL

//#define MPEG_AUDIO_FOR_FY
//disable #define ENABLE_AAC mencoder.c

///286 project
    #define PROJECT_FY286
    //#undef INPUT_SWITCH_YPBPR_CVBS

//mencoder.c enable update remain space
#define SEND_SPACE_INFO

//=============================================================================
//                              Constant Definition
//=============================================================================
#ifdef IT9913_128LQFP
    #define IIC_DELAY            0
#else
    #define IIC_DELAY            100
#endif

#define USB0_ENABLE              (0x1 << 0)
#define USB1_ENABLE              (0x1 << 1)
#define USB_ENABLE_MODE          (USB0_ENABLE | USB1_ENABLE)
#define USB_MIN_FREESPACE_MB     60                 // MB

#define LED_TIMER_NUM            5
#define LED_TIMER_TIMEOUT        100000             // us

#define WAIT_USB_WRITE_DONE_TIME 2 * 1000           //ms   kenny patch
#define CONFIG_SIGNATURE         (('I' << 24) | ('T' << 16) | ('E' << 8))

#define DEF_MICVOLGAINDB         17
#define DEF_MICBOOSTGAIN         1
#define DEF_HDMIBOOSTGAIN        5            //0db
#define DEF_ANALOGBOOSTGAIN      5            //0db

typedef enum SYSTEM_STATUS_TAG
{
    SYSTEM_USB_UNPLUG_ERROR       = (1 << 0),
    SYSTEM_USB_NO_FREESPACE_ERROR = (1 << 1),
    SYSTEM_USB_MOUNT_ERROR        = (1 << 2),
    SYSTEM_USB_NO_STORAGE_ERROR   = (1 << 3),
    SYSTEM_UNSTABLE_ERROR         = (1 << 4),
    SYSTEM_HDCP_PROTECTED         = (1 << 5),
    SYSTEM_IN_RECORD_STATUS       = (1 << 6),
    SYSTEM_IN_UPGRADE_STATUS      = (1 << 7),
    SYSTEM_USB_BUSY_STATUS        = (1 << 8),
    SYSTEM_OUT_RECORD_STATUS      = (1 << 9),
    SYSTEM_PC_CONNECTED_MODE      = (1 << 10),
    STSTEM_STAND_ALONE_MODE       = (1 << 11),
    SYSTEM_FULL_HD_RECORD         = (1 << 12),
    SYSTEM_DIGITAL_INPUT          = (1 << 13),
    SYSTEM_SWITCH_DIGITAL_ANALOG  = (1 << 14),
    SYSTEM_STARTUP_LED            = (1 << 15),
    SYSTEM_HDCP_SWITCH            = (1 << 16),
    SYSTEM_SELECT_USB_STORAGE     = (1 << 17),
    SYSTEM_STORAGE_SWITCH         = (1 << 18),
    SYSTEM_INITIALIZE             = (1 << 19),
#ifdef INPUT_SWITCH_YPBPR_CVBS
    SYSTEM_YPBPR_INPUT            = (1 << 20),
    SYSTEM_CVBS_INPUT             = (1 << 21),
#endif
} SYSTEM_STATUS;

//typedef enum ENCODE_RESOLUTION_TAG
//{
//    ENCODE_RESOLUTION_720P,
//    ENCODE_RESOLUTION_1080P,
//    ENCODE_RESOLUTION_UNKNOWN
//} ENCODE_RESOLUTION;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct AUDIO_CODEC_CONTROL_TAG
{
    MMP_BOOL   bPCConnectedMode;
    MMP_BOOL   bHDMIInput;
    MMP_BOOL   bDVIMode;
    MMP_BOOL   bEnRecord;
    MMP_BOOL   bInsertMic;

    MMP_UINT32 hdmiBoostGain;      //000 = Mute 001 = -12dB ...3dB steps up to 111 = +6dB
    MMP_UINT32 analogBoostGain;    //000 = Mute 001 = -12dB ...3dB steps up to 111 = +6dB
    MMP_INT32  micVolume;
    MMP_UINT32 micBoostGain;       //0:+0db, 1:+13db, 2:+20db, 3:+29db
} AUDIO_CODEC_CONTROL;

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
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
#ifdef HAVE_FAT
static PAL_THREAD thread_Usb        = MMP_NULL;
    #ifdef ENABLE_USB_DEVICE
static PAL_THREAD thread_Usb_device = MMP_NULL;
    #endif
#endif

//static PAL_THREAD           thread_HDMI_Loop_Through = MMP_NULL;
//static PAL_THREAD           thread_KEY_MANAGER = MMP_NULL;

static MMP_BOOL            gbEnableRecord        = MMP_FALSE;
static MMP_BOOL            gbRecording           = MMP_FALSE;

static SYSTEM_STATUS       gtSystemStatus        = SYSTEM_UNSTABLE_ERROR;
static MMP_BOOL            gbSrcChangeResolution = MMP_FALSE;

MMP_UINT                   gYear                 = 0, gMonth = 0, gDay = 0;
MMP_UINT8                  gHour                 = 0, gMin = 0, gSec = 0;
MMP_UINT32                 gFileIndex            = 0; //kenny
static MMP_UINT32          gTimerIsrCount        = 0;
MMP_BOOL                   gbDeviceMode          = MMP_FALSE;
static MMP_BOOL            gbChangeDevice        = MMP_FALSE;
static MMP_BOOL            gbAutoChange          = MMP_FALSE;
static MMP_BOOL            gbHDMIRXReboot        = MMP_FALSE;
static AUDIO_CODEC_CONTROL gtAudioCodec          = {0};
static GPIO_LED_CONTROL    gtLedCtrl = {0};
//static INPUT_ASPECT_RATIO   gtAspectRatioCtrl = INPUT_ASPECT_RATIO_AUTO;
static PCMODE_LED_CTRL     gtPCModeLed;
MMP_BOOL                   gbPC_MODE_ENALBE_RECORD = MMP_FALSE;
static MMP_UINT32          gtHDCPSwitchLedCount    = 0;
static MMP_BOOL            gtDisableHDCP           = MMP_FALSE;
static PAL_DEVICE_TYPE     gtdevicetype            = 0;
static MMP_BOOL            gbLockatSDResolution    = MMP_FALSE;

MMP_UINT8                  *pJPEGHeaderData;
MMP_UINT32                 jpegheaderSize          = 0;
MMP_UINT8                  *pJPEGFileData;
MMP_UINT32                 jpegdataSize            = 0;
#define FileIndexMax 10000
MMP_UINT32                 JPEGClock               = 0;
static JPEG_CONTEXT        tJpgCtx;
MMP_UINT32                 bshotinterval           = 1;
MMP_BOOL                   OneShotMode             = MMP_TRUE;
static MMP_BOOL            gbJPEGRecord            = MMP_FALSE;
#ifdef SEND_SPACE_INFO
MMP_UINT32 Gremainspace = 0;
MMP_UINT32 Gtotalspace = 0;
MMP_UINT32 Gremain = 0;
static MMP_UINT32 failwritecount = 0;
#endif
//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_FirmwareUpgrade(
    void);

static void *
_KeyManagerFunction(
    void *data);

static void
_UsbManagerFunction(
    void);

//static void
//_Led_SetEncodeResolution(
//    ENCODE_RESOLUTION res);

/*static*/ MMP_BOOL
_CheckUsbFreeSpace(
    MMP_UINT32 *remainspace);

static void
_ProcessMsg(
    MSG_OBJECT *ptMsg);

static void
_LedTimerIsr(
    void *data);

static void
_LedTimerInit(
    void);

static void
_LedTimerTerminate(
    void);

static MMP_BOOL
_IsContentProtection(
    void);

static MMP_BOOL
_IsPCConnectMode(
    void);

static void
_SetRtcFromFile(
    void);

static void
_AudioCodecControl(
    AUDIO_CODEC_CONTROL *pCodec);

static void
_AutoChangeDevice(
    void);

static void
_GetNextRecordFileName(
    char       *filename,
    MMP_UINT32 rec_resolution);

static void
_DisableHDCPDetect(
    MMP_BOOL flag)
{
    gtDisableHDCP = flag;
    coreSetRoot(flag);
}

static void
JPEG_Write_File(JPEG_STREAM *ptJpgStream);
static void
JPEG_Write_Card(MMP_UINT8 *pHeader, MMP_UINT32 headerSize, MMP_UINT8 *pData, MMP_UINT32 dataSize);


#ifdef SEND_SPACE_INFO
static MMP_BOOL
_CheckUsbTotalSpace(
    MMP_UINT32* totalspace);

void CheckSpaceInfo();

#endif


//=============================================================================
//                              Public Function Definition
//=============================================================================
static MMP_INT
Initialize(
    void)
{
    MMP_INT              result           = 0;
    TS_MUXER_PARAMETER   tMuxerParam      = { 0 };
    MMP_UINT             year             = 0, month = 0, day = 0;
    MMP_UINT32           micCurrVol;
    MMP_UINT8            pServiceName[32] = { 0 };
    CAPTURE_VIDEO_SOURCE capdev;
    MMP_UINT32           lineboost;
    MMP_BOOL             flag;
    RECORD_RESOLUTION    resolution;
    PAL_THREAD           thread_KEY_MANAGER       = MMP_NULL;
#ifdef HDMI_LOOPTHROUGH
    PAL_THREAD           thread_HDMI_Loop_Through = MMP_NULL;
#endif

    _coreIsPCConnectMode = _IsPCConnectMode;
    _coreInitVideoEnPara = projectInitVideoEnPara;
    _coreSetVideoEnPara  = projectSetVideoEnPara;
    _coreGetVideoEnPara  = projectGetVideoEnPara;

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
    // enable in script, reset
    mmpWatchDogDisable();
    mmpWatchDogEnable(5);
    ithIntrInit();
    _LedTimerInit();

    printf("version: %d.%d.%d.%d.%d\n", CUSTOMER_CODE, PROJECT_CODE, SDK_MAJOR_VERSION, SDK_MINOR_VERSION, BUILD_NUMBER);
    // wait for creating tasks, may reduce or remove later
    PalSleep(100);
#ifdef PROJECT_FY286
    printf("project_286\r\n");
#else
    printf("project_281_282_283\r\n");
#endif
#ifdef INPUT_SWITCH_YPBPR_CVBS
    printf("support input switch\r\n");
#else
    printf("not support input switch\r\n");
#endif

    GpioLEDInitialize();
#ifdef SUPPORT_IR_CONTROL
    mmpIrInitialize(0);
#endif

    //gtSystemStatus |= (STSTEM_STAND_ALONE_MODE | SYSTEM_SELECT_USB_STORAGE);
    gtSystemStatus |= (STSTEM_STAND_ALONE_MODE /*| SYSTEM_STARTUP_LED*/ | SYSTEM_USB_NO_STORAGE_ERROR | SYSTEM_INITIALIZE);

    RegisterProcessMsgRoutine(_ProcessMsg);
    InitDelayedMessages();

    //ithIntrInit(); // move ahead
    mmpDmaInitialize();

    result = mmpIicInitialize(0, 0, 0, 0, 0, IIC_DELAY, MMP_TRUE);
    if (result)
    {
        PalAssert("IIC init error\n");
        goto end;
    }

    result = mmpIicSetClockRate(200 * 1000);
    if (result)
        dbg_msg(DBG_MSG_TYPE_ERROR, "iic clock %d !\n", result);

    load_default_encodepara();
    config_load();
    serial_number_load();
    /*
       {
        MMP_UINT8 serialnum[128]= {0};
        MMP_RESULT result;
        coreGetUsbSerialNumber((MMP_CHAR*)&serialnum);
        dbg_msg(DBG_MSG_TYPE_INFO, "++++ serial num = %s +++++\n", serialnum);
        if (serialnum[0] == MMP_NULL)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, " ++ Write usb serial number for test ++\n");
            {
                MMP_UINT32 serialnumaddr;
                MMP_UINT32 capacity;
                MMP_UINT8  temp[] = "1369021";
                USB_SERIAL_NUMBER usbserialnum ={0};
                usbserialnum.signature = CONFIG_SIGNATURE;
                usbserialnum.size = sizeof(usbserialnum);
                strcpy(usbserialnum.serial_number, temp);
                dbg_msg(DBG_MSG_TYPE_INFO, "string usb serial num = %s\n", usbserialnum.serial_number);
                capacity = NorCapacity();
                serialnumaddr = capacity - (128*1024)-(64*1024);
                result = NorWrite(&usbserialnum, serialnumaddr, sizeof(usbserialnum));
                NorRead(&usbserialnum, serialnumaddr, sizeof(usbserialnum));
                dbg_msg(DBG_MSG_TYPE_INFO, " -- Write usb serial number for test --\n");
            }
        }
       }
     */
#ifdef HAVE_FAT
    // USB/Device initial
    result = mmpUsbExInitialize(USB_ENABLE_MODE);
    if (result)
    {
        PalAssert(!"INIT USBEX FAIL");
        goto end;
    }

    // Create task for USB host/device driver
    /** This is HC driver task, and it will never be destroyed. */
    thread_Usb = PalCreateThread(PAL_THREAD_USBEX,
                                 USBEX_ThreadFunc,
                                 MMP_NULL,
                                 2000,
                                 PAL_THREAD_PRIORITY_NORMAL);
    if (!thread_Usb)
        PalAssert(!" Create USB host task fail~~");

    #ifdef ENABLE_USB_DEVICE
    thread_Usb_device = PalCreateThread(PAL_THREAD_USB_DEVICE,
                                        DEVICE_ThreadFunc,
                                        MMP_NULL,
                                        2000,
                                        PAL_THREAD_PRIORITY_NORMAL);
    if (!thread_Usb_device)
        PalAssert(!" Create USB device task fail~~");
    #endif

    // register usb function drivers to USB driver
    mmpMscDriverRegister();
    PalSleep(1);

    result = storageMgrInitialize(MMP_TRUE);
    if (result & ~NTFS_RESULT_FLAG)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "storageMgrInitialize() fail !! %s [#%d]\n", __FILE__, __LINE__);
    }
    PalSleep(0);

    result = PalFileInitialize();
    if (result)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "PalFileInitialize() fail !! %s [#%d]\n", __FILE__, __LINE__);
        goto end;
    }

    #if 0 //def PROJECT_FY286
    gtdevicetype = PAL_DEVICE_TYPE_USB0;
    config_set_dev_type(gtdevicetype);
    #endif
    config_get_dev_type(&gtdevicetype);
   printf("start up gtdevicetype =%d\r\n",gtdevicetype);
    //_LedTimerInit(); // move ahead
    PalSleep(3000);
    if (gtdevicetype == PAL_DEVICE_TYPE_USB0)
    {
        if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_USB0))
        {
            gtSystemStatus |= SYSTEM_SELECT_USB_STORAGE;
            gtSystemStatus &= ~SYSTEM_USB_NO_STORAGE_ERROR;
        }
    }
    else
    {
        if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_SD))
        {
            gtSystemStatus &= ~SYSTEM_SELECT_USB_STORAGE;
            gtSystemStatus &= ~SYSTEM_USB_NO_STORAGE_ERROR;
        }
    }
    _FirmwareUpgrade();
#endif

    config_get_record_resolution(&resolution);
    if (resolution == FULL_HD_RESOLUTION)
    {
        gtSystemStatus |= SYSTEM_FULL_HD_RECORD;
        dbg_msg(DBG_MSG_TYPE_INFO, "startup full hd\r\n");
    }
    else
    {
        if (RECORD_TABLE[mmpCapGetInputSrcInfo()].isFullHDRes)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "start up full hd2\r\n");
            // gbLockatSDResolution = MMP_TRUE;
        }
        gtSystemStatus &= (~SYSTEM_FULL_HD_RECORD);
    }

    mmpCapSetDeviceReboot(MMP_TRUE);
    coreInitialize(MMP_MP4_MUX);
    coreInitVideoEnPara();
	config_get_capturedev(&capdev);
	if(capdev != CAPTURE_VIDEO_SOURCE_HDMI)
		{
	capdev = CAPTURE_VIDEO_SOURCE_HDMI;//kenny 20150825
	 config_set_capturedev(capdev);
	 printf("default device hdmi\r\n");
		}

    coreSetCaptureSource(capdev);

    config_get_hdcp_status(&flag);
    coreDisableHDCP(flag);

    coreEnableISPOnFly(MMP_FALSE);
    coreEnableAVEngine(MMP_FALSE);
    //coreEnableISPOnFly(MMP_TRUE);
#ifndef INPUT_SWITCH_YPBPR_CVBS

    if ((capdev == CAPTURE_VIDEO_SOURCE_HDMI) ||
        (capdev == CAPTURE_VIDEO_SOURCE_DVI))
        gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
    else
        gtSystemStatus &= (~SYSTEM_DIGITAL_INPUT);
#else
    if ((capdev == CAPTURE_VIDEO_SOURCE_HDMI) ||
        (capdev == CAPTURE_VIDEO_SOURCE_DVI))
    {
        gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
        gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
        gtSystemStatus &= (~SYSTEM_CVBS_INPUT);
       printf("HDMI start\r\n");
    }
    else if (capdev == CAPTURE_VIDEO_SOURCE_CVBS)
    {
        gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
        gtSystemStatus &= (~SYSTEM_DIGITAL_INPUT);
        gtSystemStatus |= SYSTEM_CVBS_INPUT;
	 printf("CVBS start\r\n");
    }
    else
    {
         gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
         gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
         gtSystemStatus &= (~SYSTEM_CVBS_INPUT);       
	  printf("HDMI start\r\n");		 
    }
    dbg_msg(DBG_MSG_TYPE_INFO, "gtSystemStatus =%x,capdev=%d\r\n", gtSystemStatus, capdev);
#endif
#ifdef TSO_ENABLE
    tMuxerParam.bEnableTso    = MMP_FALSE;
    tMuxerParam.bEnableEagle  = MMP_FALSE;
    tMuxerParam.constellation = CONSTELATTION_64QAM;
    tMuxerParam.codeRate      = CODE_RATE_7_8;
    tMuxerParam.guardInterval = GUARD_INTERVAL_1_32;
    tMuxerParam.frequency     = 887000;
    tMuxerParam.bandwidth     = 6000;
#endif
    sprintf(pServiceName, "AIR_CH_%d_%dM", tMuxerParam.frequency / 1000, tMuxerParam.bandwidth / 1000);
#ifdef AV_SENDER_SECURITY_MODE
    tMuxerParam.bEnableSecuirtyMode = MMP_TRUE;
#else
    tMuxerParam.bEnableSecuirtyMode = MMP_FALSE;
#endif
    tMuxerParam.bAddStuffData       = MMP_FALSE;
#ifdef MPEG_AUDIO_FOR_FY
    tMuxerParam.audioEncoderType    = MPEG_AUDIO_ENCODER;
#else
    tMuxerParam.audioEncoderType    = AAC_AUDIO_ENCODER;
#endif
    // Step 1: Removed all saved services
    coreTsRemoveServices();
    // Step 2: Specifiy the country code for NIT default setting (network_id, original_network_id, private_data_specifier_descriptor, LCN rule).
    coreTsUpdateCountryId(CORE_COUNTRY_TAIWAN);
    // Step 3: Update the modulation parameter for NIT.
    coreTsUpdateModulationParameter(tMuxerParam.frequency, tMuxerParam.bandwidth,
                                    tMuxerParam.constellation, tMuxerParam.codeRate, tMuxerParam.guardInterval);
    // Step 4: Insert desried service parameter for PMT, SDT, and NIT
    coreTsInsertService(0x100, 0x1000,
                        0x1011, H264_VIDEO_STREAM,
#ifdef MPEG_AUDIO_FOR_FY
                        0x1100, MPEG_AUDIO,
#else
                        0x1100, AAC,
#endif
                        "ITE", sizeof("ITE"),
                        pServiceName, strlen(pServiceName));
    // Step 5: Notify the table setup is done, then generate the SI/PSI table including PAT, PMT, SDT, and NIT
    coreTsUpdateTable();
    coreSetMuxerParameter((void *) &tMuxerParam);

    config_get_microphone_volume(&micCurrVol);

    //micCurrVol = 16; //30dB

    codec_initialize();
    config_get_line_boost(&lineboost);
    gtAudioCodec.bPCConnectedMode = MMP_FALSE;
    gtAudioCodec.bHDMIInput       = ((coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_HDMI) || (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI)) ? MMP_TRUE : MMP_FALSE;
    gtAudioCodec.bDVIMode         = (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI) ? MMP_TRUE : MMP_FALSE;
    gtAudioCodec.bEnRecord        = MMP_FALSE;
    gtAudioCodec.bInsertMic       = MMP_FALSE;
    gtAudioCodec.hdmiBoostGain    = DEF_HDMIBOOSTGAIN;
    gtAudioCodec.analogBoostGain  = DEF_ANALOGBOOSTGAIN;
    gtAudioCodec.micVolume        = DEF_MICVOLGAINDB;
    gtAudioCodec.micBoostGain     = DEF_MICBOOSTGAIN;

    _AudioCodecControl(&gtAudioCodec);

    corePlay();

#ifdef HDMI_LOOPTHROUGH
    thread_HDMI_Loop_Through = PalCreateThread(PAL_THREAD_HDMILooPThrough,
                                               HDMILoopThrough_ThreadFun,
                                               MMP_NULL,
                                               2000,
                                               PAL_THREAD_PRIORITY_NORMAL);

    HDMILoopThrough_CreateThread();
    if (!thread_HDMI_Loop_Through)
        PalAssert(!" Create HDMI Loop Through task fail~~");
#endif

    thread_KEY_MANAGER = PalCreateThread(PAL_THREAD_KEY_MANAGER,
                                         _KeyManagerFunction,
                                         MMP_NULL,
                                         2000,
                                         PAL_THREAD_PRIORITY_NORMAL);
    if (!thread_KEY_MANAGER)
        PalAssert(!" Create KEY MANAGER task fail~~");

    GpioKeyInitialize();
    GpioMicInitialize();
    //mmpWatchDogEnable(30); // move ahead
#ifdef YK_MINI_GRABBER
    gYear  = 2014;
    gMonth = 1;
    gDay   = 1;
    gHour  = 0;
    gMin   = 0;
    gSec   = 0;
#endif
    _SetRtcFromFile();
    if (KEY_CODE_S1 == GpioKeyGetKey())
    {
        config_set_rec_index(0);
        dbg_msg(DBG_MSG_TYPE_INFO, "index set default\n");
    }


#ifdef SEND_SPACE_INFO
{
       //    extern MMP_BOOL GpioInputselect(MMP_BOOL status);
           GpioInputselect2(MMP_FALSE);
	   failwritecount = 0;
}
#endif


    //kenny
    mmpExRtcGetDate(&year, &month, &day);

    dbg_msg(DBG_MSG_TYPE_INFO, "RTC: %d-%d-%d\r\n", year, month, day);

    gtSystemStatus |= (SYSTEM_STARTUP_LED);

#ifdef PROJECT_FY286
    //kenny patch 20140428
    #if (defined(COMPONENT_DEV) || defined(COMPOSITE_DEV))
    //do nothing
    #else
    gtSystemStatus &= (~SYSTEM_UNSTABLE_ERROR);
    #endif
#endif

#if 0 //def MPEG_AUDIO_FOR_FY
    {
        CORE_AUDIO_ENCODE_PARA aencoder_param;
        aencoder_param.audioEncoderType = MPEG_AUDIO_ENCODER;
        aencoder_param.bitRate          = 192000;
        coreSetAudioEncodeParameter(&aencoder_param);
    }
#endif

    // turn off DPU clock
    HOST_WriteRegisterMask(0x16, (0x0 << 5), (0x1 << 5));

    //Hardware Trap
    AHB_WriteRegisterMask((GPIO_BASE + 0xD4), 0x00000, 0x00ff0);

    gtSystemStatus &= ~(SYSTEM_INITIALIZE);
#if 0
    vTaskGetRunTimeStats((char *)0);
    vTaskGetRunCodeStackStats((char *)0);
    malloc_stats();
#endif

end:
    return result;
}

static MMP_INT
Terminate(
    void)
{
    MMP_INT result = 0;

    codec_terminate();
    GpioLEDTerminate();
    GpioKeyTerminate();
    _LedTimerTerminate();
    coreTerminate();
    TerminateDelayedMessages();

    return result;
}

static MMP_INT
MainLoop(
    void)
{
    MMP_INT                  result             = 0;
    MMP_UINT32               CheckRecordClock   = PalGetClock();
    MMP_UINT32               CheckDeviceClock   = PalGetClock();
    //MMP_UINT32 clockLED = PalGetClock();
    MMP_UINT32               videoUnstableClock = 0;
    MMP_UINT32               CheckRtcClock      = PalGetClock();
    //MMP_UINT32 recFileIndex = 0;
    MMP_UINT32               remainspace        = 0;
    MMP_UINT8                filename[512];
    //MMP_BOOL   keyswitch = MMP_FALSE;
    PAL_FILE                 *fp;
    //MMP_BOOL Trigger = MMP_FALSE;
    //static MMP_BOOL prevTrigger = MMP_FALSE;
    MMP_UINT32               durationTime = 0;
    MMP_UINT32               adjustTime   = 0;
    static RECORD_RESOLUTION prev         = UNKNOWN, current = UNKNOWN;

    //#ifdef Change_Resolution
    static MMP_INT           preres       = 0xFF;
    MMP_INT                  resinput     = 0xFF;
    extern MMP_UINT16        gtHDMIResolution;
    extern MMP_UINT16        gtYPBPRResolution;
    extern MMP_UINT16        gtCVBSResolution;
    //#endif

    MMP_UINT32               recFileIndex  = 0;
    static MMP_UINT32        recresolution = 0;//kenny patch resolution flag

    for (;;)
    {
        PalSleep(33);

        SendDelayedMessages();

        _AutoChangeDevice();
        _UsbManagerFunction();
        //it_ts_file_UpgradeFirmware(); //PC config , upgrade Firmware
        mmpWatchDogRefreshTimer();

 #ifdef SEND_SPACE_INFO
   CheckSpaceInfo();
 #endif


#if 1
        if ((durationTime = PalGetDuration(CheckRtcClock)) >= 1000)
        {
            adjustTime += (durationTime - 1000);
            if (adjustTime >= 1000)
            {
                gSec++;
                adjustTime -= 1000;
            }
            gSec++;
            if (gSec >= 60)
            {
                gSec -= 60;
                gMin++;
            }

            if (gMin == 60)
            {
                gMin -= 60;
                gHour++;
            }

            if (gHour == 24)
            {
                gHour -= 24;
                gDay++;
            }

            switch (gMonth)
            {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                if (gDay > 31)
                {
                    gMonth++;
                    gDay -= 31;
                }
                break;

            case 4:
            case 6:
            case 9:
            case 11:
                if (gDay > 30)
                {
                    gMonth++;
                    gDay -= 30;
                }
                break;

            case 2:
                if (gYear % 4 == 0)
                {
                    if (gDay > 29)
                    {
                        gDay -= 29;
                        gMonth++;
                    }
                }
                else
                {
                    if (gDay > 28)
                    {
                        gDay -= 28;
                        gMonth++;
                    }
                }
                break;
            }
            if (gMonth > 13)
            {
                gMonth -= 12;
                gYear++;
            }
            CheckRtcClock = PalGetClock();
        }

        if (PalGetDuration(CheckRecordClock) > 300)
        {
            MMP_WCHAR       wfilename[255];
            static MMP_BOOL preEnableRecord = MMP_FALSE;

            if (gbEnableRecord && !preEnableRecord)
            {
                VIDEO_ENCODER_INPUT_INFO   srcIdx = coreGetInputSrcInfo();
                CAPTURE_VIDEO_SOURCE       capdev = coreGetCaptureSource();
                VIDEO_ENCODER_UPDATE_FLAGS flags  = 0;
                VIDEO_ENCODER_PARAMETER    tEnPara;

                //update Default
                flags         = VIDEO_ENCODER_FLAGS_UPDATE_DEFAULT;
                coreSetVideoEnPara(capdev, flags, srcIdx, &tEnPara);
                recresolution = 0;
                if (RECORD_TABLE[srcIdx].isFullHDRes && srcIdx <= MMP_CAP_INPUT_INFO_ALL)
                {
                    coreGetVideoEnPara(capdev, srcIdx, &tEnPara);
                    recresolution = 1;
                    if (((gtSystemStatus & SYSTEM_FULL_HD_RECORD) == 0) && (!gbJPEGRecord)) //720p case
                    {
                        recresolution         = 2;
                        tEnPara.EnWidth       = 1280;
                        tEnPara.EnHeight      = 720;
                        tEnPara.EnFrameRate   = tEnPara.EnFrameRate * 2;
                        tEnPara.EnGOPSize     = tEnPara.EnGOPSize * 2;
                        tEnPara.EnAspectRatio = AR_LETTER_BOX;

                        flags                 = VIDEO_ENCODER_FLAGS_UPDATE_WIDTH_HEIGHT |
                                                VIDEO_ENCODER_FLAGS_UPDATE_FRAME_RATE |
                                                VIDEO_ENCODER_FLAGS_UPDATE_GOP_SIZE |
                                                VIDEO_ENCODER_FLAGS_UPDATE_ASPECT_RATIO;

                        coreSetVideoEnPara(capdev, flags, srcIdx, &tEnPara);
                    }
                }

                dbg_msg(DBG_MSG_TYPE_INFO, "---- corestop Dev = %d ----\n", mmpCapGetCaptureDevice());
                mmpCapSetDeviceReboot(MMP_FALSE);
                if (!gbJPEGRecord)
                    coreStop();
                coreEnableAVEngine(MMP_TRUE);
                mmpWatchDogRefreshTimer();
                corePlay();
                dbg_msg(DBG_MSG_TYPE_INFO, "---- corePlay Dev = %d ----\n", mmpCapGetCaptureDevice());
            }
            else if (!mmpCapGetDeviceReboot() && avSyncIsVideoStable())
                mmpCapSetDeviceReboot(MMP_TRUE);

            preEnableRecord = gbEnableRecord;

            if (gbEnableRecord && !gbRecording && avSyncIsVideoStable())
            {
                if (_CheckUsbFreeSpace(&remainspace))
                {
                    if (gbJPEGRecord)
                    {
                        tJpgCtx.bOneShot       = OneShotMode;
                        tJpgCtx.encodingPeriod = bshotinterval;
                        tJpgCtx.quality        = 95; //kenny patch
                        tJpgCtx.pfJPEGCallback = JPEG_Write_File;
                        coreStartJPEGRecord(&tJpgCtx);
                        dbg_msg(DBG_MSG_TYPE_INFO, "coreStartJPEGRecord %d (%d-%d)\n", PalGetDuration(JPEGClock), tJpgCtx.bOneShot, tJpgCtx.encodingPeriod);
                    }
                    else
                    {
                        MMP_INT         i;
                        RECORD_MODE     recMode = {0};
                        MMP_UINT        volumeIndex;
                        PAL_DEVICE_TYPE devtype = (gtSystemStatus & SYSTEM_SELECT_USB_STORAGE)
                                                  ? PAL_DEVICE_TYPE_USB0
                                                  :  PAL_DEVICE_TYPE_SD;

                        _GetNextRecordFileName(filename, recresolution);
                        for (i = strlen(filename) + 1; i >= 0; --i)
                            wfilename[i] = (MMP_WCHAR)filename[i];

                        volumeIndex  = storageMgrGetCardVolume(devtype, 0);
                        if (volumeIndex < 0)
                            dbg_msg(DBG_MSG_TYPE_INFO, "--- Get invalid volindex ----\n");
                        wfilename[0] = PAL_T('A') + volumeIndex;

                        fp           = PalWFileOpen(wfilename, PAL_FILE_WB, MMP_NULL, MMP_NULL);
                        if (!fp)
                            PalWFileDelete(wfilename, MMP_NULL, MMP_NULL);
                        else
                            PalFileClose(fp, MMP_NULL, MMP_NULL);

    #if 1
                        recMode.bDisableFileSplitter = MMP_FALSE;
    #else
                        recMode.bDisableFileSplitter =
                            (FAT_FILE_SYSTEM != storageMgrGetVolumeFileSystemFormat(storageMgrGetVolumeNumber(wfilename)));
    #endif
                        coreSetRecordMode(&recMode);
                        coreStartRecord(wfilename, MMP_NULL);
                        dbg_msg(DBG_MSG_TYPE_INFO, "coreStartRecord (%s)\n", filename);
                    }
                    gbRecording = MMP_TRUE;
                    //#ifdef Change_Resolution
                    if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_HDMI)
                    {
                        preres = gtHDMIResolution;
                    }
                    else if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_YPBPR)
                    {
                        preres = gtYPBPRResolution;
                    }
                    else if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_CVBS)
                    {
                        preres = gtCVBSResolution;
                    }
                    //#endif
                }
                else
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- USB No FreeSpace ----\n");
                    gtSystemStatus |= SYSTEM_USB_NO_FREESPACE_ERROR;
                    gbEnableRecord  = MMP_FALSE;
                }
            }

            if (gbRecording)
            {
                if (_IsContentProtection())
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- HDCP On , Can not Record ----\n");
                    gbEnableRecord = MMP_FALSE;
                    PalTFileDelete(wfilename, MMP_NULL, MMP_NULL);
                }

                //dbg_msg(DBG_MSG_TYPE_INFO, "remainspace(%d), WriteMB(%d), diff (%d)\n", remainspace, mencoder_GetWriteSize(), remainspace-mencoder_GetWriteSize());
              #ifdef SEND_SPACE_INFO
		Gremainspace = remainspace - mencoder_GetWriteSize();
	     #endif
		 
		if (remainspace - mencoder_GetWriteSize() <= USB_MIN_FREESPACE_MB)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- NO Free Space ----\n");
                    gtSystemStatus |= SYSTEM_USB_NO_FREESPACE_ERROR;
                    gbEnableRecord  = MMP_FALSE;
                }

                // If un-stable state lasts 2 more seconds, then stop recording.
                if (avSyncIsVideoStable())
                    videoUnstableClock = 0;
                else
                {
                    if (videoUnstableClock > 0)
                    {
                        if (PalGetDuration(videoUnstableClock) > 5000 && gbEnableRecord)  //kenny201527 2000
                        {
                            dbg_msg(DBG_MSG_TYPE_INFO, "---- Video Unstable , Can not Record ----\n");
                            gbEnableRecord = MMP_FALSE;
                        }
                    }
                    else
                        videoUnstableClock = PalGetClock();
                }

                //#ifdef Change_Resolution

                if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_HDMI)
                    resinput = gtHDMIResolution;
                else if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_YPBPR)
                    resinput = gtYPBPRResolution;
                else if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_CVBS)
                    resinput = gtCVBSResolution;

                if (preres != resinput)
                {
                    if (!gbJPEGRecord)
                        gbSrcChangeResolution = MMP_TRUE;
                    preres        = resinput;
                    recresolution = 0;
                    VIDEO_ENCODER_INPUT_INFO srcIdx = coreGetInputSrcInfo();

                    if (RECORD_TABLE[srcIdx].isFullHDRes && srcIdx < MMP_CAP_INPUT_INFO_ALL)
                    {
                        recresolution = 1;
                    }
                }
                //#endif

                if (gbSrcChangeResolution)
                {
                    MMP_INT         i;
                    RECORD_MODE     recMode = {0};
                    PAL_DEVICE_TYPE devtype = PAL_DEVICE_TYPE_USB0;
                    MMP_UINT        volumeIndex;
                    //MMP_UINT32 clock;
                    
                    gtSystemStatus |= SYSTEM_OUT_RECORD_STATUS;
                    if (gbJPEGRecord)
                    {
                        coreStopJPEGRecord();
                        gbJPEGRecord = MMP_FALSE;
                    }
                    else
                    {
                        coreStopRecord();
                    }
                    gtSystemStatus &= ~SYSTEM_OUT_RECORD_STATUS;

                    devtype         = (gtSystemStatus & SYSTEM_SELECT_USB_STORAGE)
                                      ? PAL_DEVICE_TYPE_USB0
                                      : PAL_DEVICE_TYPE_SD;

                    _GetNextRecordFileName(filename, recresolution);
                    for (i = strlen(filename) + 1; i >= 0; --i)
                        wfilename[i] = (MMP_WCHAR)filename[i];

                    volumeIndex                  = storageMgrGetCardVolume(devtype, 0);
                    if (volumeIndex < 0)
                        dbg_msg(DBG_MSG_TYPE_INFO, "--- Get invalid volindex ----\n");
                    wfilename[0]                 = PAL_T('A') + volumeIndex;
    #if 1
                    recMode.bDisableFileSplitter = MMP_FALSE;
    #else
                    recMode.bDisableFileSplitter =
                        (FAT_FILE_SYSTEM != storageMgrGetVolumeFileSystemFormat(storageMgrGetVolumeNumber(wfilename)));
    #endif
                    coreSetRecordMode(&recMode);
                    coreStartRecord(wfilename, MMP_NULL);
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- coreStartRecord (%s) ----\n", filename);
                    gbSrcChangeResolution = MMP_FALSE;
                }
            }

            if (gbEnableRecord == MMP_FALSE)
            {
                if (gbRecording)
                {
                    //PAL_CLOCK_T clock;

                    gbRecording     = MMP_FALSE;
                    gtSystemStatus |= SYSTEM_OUT_RECORD_STATUS;
                    if (gbJPEGRecord)
                    {
                        coreStopJPEGRecord();
                    }
                    else
                    {
                        coreStopRecord();
                    }
                    dbg_msg(DBG_MSG_TYPE_INFO, "Write Test is Done (%d)\n", __LINE__);
                   #ifdef SEND_SPACE_INFO
		    _CheckUsbFreeSpace(&Gremainspace);
		    printf("record stop G space=%d,R space=%d\r\n",Gtotalspace,Gremainspace);
		  #endif
                    if (!gbJPEGRecord)
                    {
                        MMP_UINT32 WriteDoneClock = PalGetClock();
                        while (PalGetDuration(WriteDoneClock) < WAIT_USB_WRITE_DONE_TIME)
                            PalSleep(1);
                    }
                    if (gbJPEGRecord)
                        gbJPEGRecord = MMP_FALSE;

                    coreEnableAVEngine(MMP_FALSE);
                    gtSystemStatus &= ~SYSTEM_IN_RECORD_STATUS;
                    gtSystemStatus &= ~SYSTEM_OUT_RECORD_STATUS;
                }
            }
            CheckRecordClock = PalGetClock();
        }

    #if (defined(COMPONENT_DEV) || defined(COMPOSITE_DEV))
        //Auto Detect Device
        if (PalGetDuration(CheckDeviceClock) > 500)
        {
            CAPTURE_VIDEO_SOURCE CapDev = coreGetCaptureSource();
            CAPTURE_VIDEO_SOURCE newCapDev;

            if (avSyncIsVideoStable())
            {
                //static CAPTURE_VIDEO_SOURCE stableSource = MMP_CAP_VIDEO_SOURCE_UNKNOW;
                CAPTURE_VIDEO_SOURCE stableSource;
                CAPTURE_VIDEO_SOURCE newstableSource = coreGetCaptureSource();
        #ifdef AUTO_SWITCH_SOURCE
                if (gbAutoChange == MMP_TRUE)
                {
                    config_get_capturedev(&stableSource);
                    printf( "---- stable source changed from %d to %d ----\n", stableSource, newstableSource);
                    if (stableSource != newstableSource)
                    {
                        config_set_capturedev(newstableSource);
                        stableSource = newstableSource;
                    }

                    gbAutoChange = MMP_FALSE;

                    if (stableSource == CAPTURE_VIDEO_SOURCE_HDMI)
                    {
                        gtSystemStatus &= (~SYSTEM_CVBS_INPUT);
                        gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
                        gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
                        printf( "auto: HDMI\r\n");
                    }
                    else if (stableSource == CAPTURE_VIDEO_SOURCE_CVBS)
                    {
                        gtSystemStatus &= (~SYSTEM_DIGITAL_INPUT);
                        gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
                        gtSystemStatus |= SYSTEM_CVBS_INPUT;
                        printf("auto: CVBS\r\n");
                    }
                }
        #endif
                if (_IsContentProtection())
                    gtSystemStatus |= SYSTEM_HDCP_PROTECTED;
                else
                    gtSystemStatus &= (~SYSTEM_HDCP_PROTECTED);

                gtSystemStatus &= (~SYSTEM_UNSTABLE_ERROR);
            }
            else
            {
                gtSystemStatus &= (~SYSTEM_HDCP_PROTECTED);
                gtSystemStatus |= SYSTEM_UNSTABLE_ERROR;
            }

            if (gbChangeDevice || gbHDMIRXReboot)
            {
                gtAudioCodec.bEnRecord = MMP_FALSE;
                _AudioCodecControl(&gtAudioCodec);

                if (gbHDMIRXReboot && (CapDev == CAPTURE_VIDEO_SOURCE_HDMI))
                {
                    MMP_UINT32 timeOut = 0;

                    PalSleep(3000); //wait hdmi rx disconnected

                    while (avSyncIsVideoStable())
                    {
                        PalSleep(1);
                        if (++timeOut > 2000)
                        {
                            dbg_msg(DBG_MSG_TYPE_INFO, "!!Wait avSyncIsVideoStable() unstable TimeOut, %s()#%d\n", __FUNCTION__, __LINE__);
                            while (1)
                                PalSleep(1);
                        }
                    }

                    newCapDev = CAPTURE_VIDEO_SOURCE_HDMI;
                    printf( "----1. HDMIRX---");
                }
        #ifdef AUTO_SWITCH_SOURCE
                if (gbAutoChange == MMP_TRUE)
                {
                    if (CapDev == CAPTURE_VIDEO_SOURCE_HDMI)
                    {
                        newCapDev = CAPTURE_VIDEO_SOURCE_CVBS;
                        printf("----2.ADV7180---");
                    }
                    else if (CapDev == CAPTURE_VIDEO_SOURCE_CVBS)
                    {
                        newCapDev = CAPTURE_VIDEO_SOURCE_HDMI;
                        printf("----2. HDMIRX---");
                    }
                }
        #endif
        #ifdef INPUT_SWITCH_YPBPR_CVBS
                if (gbAutoChange == MMP_FALSE)
                {
                    if (gtSystemStatus & SYSTEM_CVBS_INPUT)
                    {
                        newCapDev = CAPTURE_VIDEO_SOURCE_CVBS;
                        printf( "----ADV7180---");
                    }
                    if (gtSystemStatus & SYSTEM_DIGITAL_INPUT)
                    {
                        newCapDev = CAPTURE_VIDEO_SOURCE_HDMI;
                        printf( "----HDMIRX---");
                    }
                }
        #endif
                gtSystemStatus |= SYSTEM_UNSTABLE_ERROR;
                printf( "---- coreStop Dev = %d ----\n", newCapDev);
                mmpCapSetDeviceReboot(MMP_TRUE);
                coreStop();
                //codec_terminate();
                codec_initialize();
                gtAudioCodec.bPCConnectedMode = MMP_FALSE;
                gtAudioCodec.bHDMIInput       = ((newCapDev == CAPTURE_VIDEO_SOURCE_HDMI) || (newCapDev == CAPTURE_VIDEO_SOURCE_DVI)) ? MMP_TRUE : MMP_FALSE;
                gtAudioCodec.bDVIMode         = (newCapDev == CAPTURE_VIDEO_SOURCE_DVI) ? MMP_TRUE : MMP_FALSE;
                gtAudioCodec.bEnRecord        = MMP_FALSE;
                _AudioCodecControl(&gtAudioCodec);

                gbEnableRecord                = MMP_FALSE;
                mmpWatchDogRefreshTimer();
                coreSetCaptureSource(newCapDev);

                coreEnableAVEngine(MMP_FALSE);
                CapDev = newCapDev;
                corePlay();
                dbg_msg(DBG_MSG_TYPE_INFO, "---- corePlay Dev = %d ----\n", newCapDev);
                //config_set_capturedev(newCapDev);  //doesn`t saving config all the time.
                gbChangeDevice  = MMP_FALSE;
                gbHDMIRXReboot  = MMP_FALSE;
                gtSystemStatus &= (~SYSTEM_SWITCH_DIGITAL_ANALOG);
            }

            CheckDeviceClock = PalGetClock();
        }
    #endif // #if (defined(COMPONENT_DEV) || defined(COMPOSITE_DEV))
           //check if  record resolution changed
        if (gtSystemStatus & SYSTEM_FULL_HD_RECORD)
            current = FULL_HD_RESOLUTION;
        else
            current = SD_RESOLUTION;

        if (prev != current)
        {
            //dbg_msg(DBG_MSG_TYPE_INFO, "--- current resolution = %d ---\n", current);
            config_set_record_resolution(current);
        }
        prev = current;

    #if defined(CONFIG_HAVE_USBD)
        if (gbDeviceMode)
        {
            extern int it_usbd_main();

            it_usbd_main();
        }
    #endif
#endif
    }

    return result;
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

    result = Terminate();
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
        if (pCtrl->micVol == MIC_VOL_MUTE)
        {
            //mute
        }
        else
        {
            gtAudioCodec.micVolume    = DEF_MICVOLGAINDB;
            gtAudioCodec.micBoostGain = DEF_MICBOOSTGAIN;
        }
        config_set_microphone_volume((MMP_UINT32)pCtrl->micVol);
        break;

    case GRABBER_CTRL_LINE_BOOST:
        gtAudioCodec.hdmiBoostGain   = pCtrl->lineBoost;
        gtAudioCodec.analogBoostGain = pCtrl->lineBoost;
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
            MMP_ISP_COLOR_CTRL ispCtrl;
            ispCtrl.brightness = pCtrl->COLOR.brightness;
            ispCtrl.contrast   = pCtrl->COLOR.contrast;
            ispCtrl.hue        = pCtrl->COLOR.hue;
            ispCtrl.saturation = pCtrl->COLOR.saturation;
            mmpIspSetColorCtrl(&ispCtrl);
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
    //MMP_UINT32 i;
    //MMP_UINT32 aspectRatio;
    MMP_UINT32           volume;
    MMP_UINT32           lineboost;
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
        pCtrl->bIsHDCP = _IsContentProtection();
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
IsRecordState(
    void)
{
    return gbEnableRecord ? MMP_TRUE : MMP_FALSE;
}

static MMP_BOOL
_IsPCConnectMode(
    void)
{
    return (gtSystemStatus & SYSTEM_PC_CONNECTED_MODE) ? MMP_TRUE : MMP_FALSE;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_FirmwareUpgrade(
    void)
{
    MMP_INT    volumeIndex      = 0;
    //MMP_BOOL    bUpgradeFinish = MMP_FALSE;
    MMP_WCHAR  filePath[16]     = {0};
    PAL_FILE   *pRomFile        = MMP_NULL;
    MMP_UINT8  *pReadFileBuffer = MMP_NULL;
    //MMP_UINT8*  pReadNorBuffer = MMP_NULL;
    //MMP_UINT32  writeNorSize = 0;
    MMP_UINT32 fileSize         = 0;
    MMP_UINT8  pFileIdentifier[16];
    MMP_RESULT result           = MMP_RESULT_ERROR;

    //    if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_SD))
    //    {
    //        volumeIndex = storageMgrGetCardVolume(PAL_DEVICE_TYPE_SD, 0);
    //        if (volumeIndex < 0)
    //            return;
    //        filePath[0] = PAL_T('A') + volumeIndex;
    //        PalMemcpy(&filePath[1], &PAL_T(":/jedi.img"), sizeof(PAL_T(":/jedi.img")));
    //        pRomFile = PalTFileOpen(filePath, PAL_FILE_RB, MMP_NULL, MMP_NULL);
    //        if (pRomFile == MMP_NULL)
    //        {
    //            dbg_msg(DBG_MSG_TYPE_INFO, "no rom file exist\n");
    //            return;
    //        }
    //        fileSize = PalTGetFileLength(filePath);
    //        PalTFileRead(pFileIdentifier, 1, 16, pRomFile, MMP_NULL, MMP_NULL);
    //        fileSize -= 16;
    //        if ((pFileIdentifier[0] == 0 && pFileIdentifier[1] == 0)
    //         || (CUSTOMER_CODE==0 && PROJECT_CODE==0))
    //        {
    //            // always pass, no need to check version
    //        }
    //        else if ((pFileIdentifier[0] != CUSTOMER_CODE)
    //              || (pFileIdentifier[1] != PROJECT_CODE))
    //        {
    //            PalTFileClose(pRomFile, MMP_NULL, MMP_NULL);
    //            dbg_msg(DBG_MSG_TYPE_INFO, "version invalid\n");
    //            return;
    //        }
    //
    //        pReadFileBuffer = PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
    //        pReadNorBuffer = PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
    //
    //        while (1)
    //        {
    //            if (bUpgradeFinish == MMP_FALSE)
    //            {
    //                if (fileSize == writeNorSize)
    //                {
    //                    NorRead(pReadNorBuffer, 0, writeNorSize);
    //                    if (0 == memcmp(pReadFileBuffer, pReadNorBuffer, writeNorSize))
    //                    {
    //                        PalTFileClose(pRomFile, MMP_NULL, MMP_NULL);
    //                        PalTFileDelete(filePath, MMP_NULL, MMP_NULL);
    //                        dbg_msg(DBG_MSG_TYPE_INFO, "upgrade success\n");
    //                    }
    //                    else
    //                    {
    //                        PalTFileClose(pRomFile, MMP_NULL, MMP_NULL);
    //                        dbg_msg(DBG_MSG_TYPE_INFO, "upgrade fail\n");
    //                    }
    //                    PalHeapFree(PAL_HEAP_DEFAULT, pReadFileBuffer);
    //                    PalHeapFree(PAL_HEAP_DEFAULT, pReadNorBuffer);
    //                    bUpgradeFinish = MMP_TRUE;
    //                    dbg_msg(DBG_MSG_TYPE_INFO, "upgrade finish\n");
    //                    mmpWatchDogEnable(1);   //1s
    //                    dbg_msg(DBG_MSG_TYPE_INFO, "---- Reboot SYSTEM ----\n");
    //                    while (1)
    //                    {
    //                        mmpWatchDogRefreshTimer();
    //                        PalSleep(1000);
    //                    }
    //                }
    //                else
    //                {
    //                    PalTFileRead(pReadFileBuffer, 1, fileSize, pRomFile, MMP_NULL, MMP_NULL);
    //                    HOST_WriteRegister(0x7c90, 0x4000);
    //                    HOST_WriteRegister(0x7c92, 0x0054);
    //                    NorInitial();
    //                    dbg_msg(DBG_MSG_TYPE_INFO, "upgrade start\n");
    //                    NorWrite(pReadFileBuffer, 0, fileSize);
    //                    writeNorSize += fileSize;
    //                }
    //            }
    //        }
    //    }
    //    else
    if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_USB0))
    {
        volumeIndex = storageMgrGetCardVolume(PAL_DEVICE_TYPE_USB0, 0);
        if (volumeIndex < 0)
            return;
        filePath[0] = PAL_T('A') + volumeIndex;
        PalMemcpy(&filePath[1], &PAL_T(":/jedi.img"), sizeof(PAL_T(":/jedi.img")));
        pRomFile    = PalTFileOpen(filePath, PAL_FILE_RB, MMP_NULL, MMP_NULL);
        if (pRomFile == MMP_NULL)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "no rom file exist\n");
            return;
        }

        fileSize        = PalTGetFileLength(filePath);
        pReadFileBuffer = PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
        PalTFileRead(pReadFileBuffer, 1, fileSize, pRomFile, MMP_NULL, MMP_NULL);

        gtSystemStatus |= SYSTEM_IN_UPGRADE_STATUS;
        result          = coreFirmwareUpgrade(pReadFileBuffer, fileSize);
        gtSystemStatus &= ~SYSTEM_IN_UPGRADE_STATUS;
        PalTFileClose(pRomFile, MMP_NULL, MMP_NULL);
        PalTFileDelete(filePath, MMP_NULL, MMP_NULL);
        PalHeapFree(PAL_HEAP_DEFAULT, pReadFileBuffer);
        //if (result == MMP_RESULT_ERROR) // user MUST reboot manually after firmware update
        while (1);
    }

    if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_SD))
    {
        volumeIndex = storageMgrGetCardVolume(PAL_DEVICE_TYPE_SD, 0);
        if (volumeIndex < 0)
            return;
        filePath[0] = PAL_T('A') + volumeIndex;
        PalMemcpy(&filePath[1], &PAL_T(":/jedi.img"), sizeof(PAL_T(":/jedi.img")));
        pRomFile    = PalTFileOpen(filePath, PAL_FILE_RB, MMP_NULL, MMP_NULL);
        if (pRomFile == MMP_NULL)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "no rom file exist\n");
            return;
        }

        fileSize        = PalTGetFileLength(filePath);
        pReadFileBuffer = PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
        PalTFileRead(pReadFileBuffer, 1, fileSize, pRomFile, MMP_NULL, MMP_NULL);

        gtSystemStatus |= SYSTEM_IN_UPGRADE_STATUS;
        result          = coreFirmwareUpgrade(pReadFileBuffer, fileSize);
        gtSystemStatus &= ~SYSTEM_IN_UPGRADE_STATUS;
        PalTFileClose(pRomFile, MMP_NULL, MMP_NULL);
        PalTFileDelete(filePath, MMP_NULL, MMP_NULL);
        PalHeapFree(PAL_HEAP_DEFAULT, pReadFileBuffer);
        //if (result == MMP_RESULT_ERROR) // user MUST reboot manually after firmware update
        while (1);
    }
	
}

static void *
_KeyManagerFunction(
    void *data)
{
    KEY_CODE          key;
    MMP_UINT32        pollingTime       = 50;
    MMP_UINT32        buttonPollingTime = 50; // polling time of key pressed, extend to avoid repeat
    MMP_UINT32        index             = 0;
    MMP_UINT32        lockS3KeyClock    = 0;
    MMP_BOOL          gbLockS3Key       = MMP_FALSE;
    MMP_UINT32        lockS4KeyClock    = 0;
    MMP_BOOL          gbLockS4Key       = MMP_FALSE;
    static MMP_UINT16 keypress          = MMP_FALSE;
    static MMP_UINT32 keytime           = 0;

    for (;;)
    {
        if (avSyncIsAudioInitFinished())
        {
            static MMP_BOOL temp = MMP_FALSE;

            if (GpioMicIsInsert())
            {
#ifndef PROJECT_FY286
                gtAudioCodec.bInsertMic = MMP_TRUE;
#endif
                if (temp == MMP_TRUE)
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "mic insert\r\n");
                    temp = MMP_FALSE;
                }
            }
            else
            {
                gtAudioCodec.bInsertMic = MMP_FALSE;
                temp                    = MMP_TRUE;
            }
        }

        if (gbLockS3Key && PalGetDuration(lockS3KeyClock) > 4000)
        {
            gbLockS3Key = MMP_FALSE;
        }

        if (gbLockS4Key && PalGetDuration(lockS4KeyClock) > 4000)
        {
            gbLockS4Key = MMP_FALSE;
        }

        if (avSyncIsVideoStable())
        {
            MMP_BOOL isFULLHDSource;

            if (RECORD_TABLE[mmpCapGetInputSrcInfo()].isFullHDRes)
                isFULLHDSource = MMP_TRUE;
            else
            {
                isFULLHDSource       = MMP_FALSE;
                gbLockatSDResolution = MMP_FALSE;
            }

            if (!isFULLHDSource && gtSystemStatus & SYSTEM_FULL_HD_RECORD)
            {
                gtSystemStatus &= (~SYSTEM_FULL_HD_RECORD);
            }

            if (isFULLHDSource && !gbLockatSDResolution && !(gtSystemStatus & SYSTEM_FULL_HD_RECORD))
            {
                gtSystemStatus |= SYSTEM_FULL_HD_RECORD;
            }
        }

        buttonPollingTime = buttonPollingTime - pollingTime;
        if (buttonPollingTime == 0)
        {
            // polling time of key pressed, extend to avoid repeat
            buttonPollingTime = 2000;

            key               = GpioKeyGetKey();
#ifdef SUPPORT_IR_CONTROL
            {
                MMP_UINT32 tempkey = 0;

                tempkey = mmpIrGetKey();

                if (tempkey != 0)
                    dbg_msg(DBG_MSG_TYPE_INFO, "IR Key =%x,status =%x\n", tempkey, gtSystemStatus);
                if ((tempkey & 0xFFFF) == 0xFF02)
                {
                    tempkey = (tempkey >> 16) & 0xFFFF;

                    if ((tempkey == 0xFF00) && (gtSystemStatus & SYSTEM_IN_RECORD_STATUS))
                        key = KEY_CODE_S1;                                              //stop record
                    if (( tempkey == 0xFF00 ) && ( (gtSystemStatus & SYSTEM_IN_RECORD_STATUS) == 0))
                        key = KEY_CODE_S1;                                              //start record

                    if ( (tempkey == 0xFD02) && (gtSystemStatus & SYSTEM_FULL_HD_RECORD))
                        key = KEY_CODE_S2;                                              //720p
                    if ((tempkey == 0xFC03) && ( (gtSystemStatus & SYSTEM_FULL_HD_RECORD) == 0))
                        key = KEY_CODE_S2;                                              //1080p

                    if (tempkey == 0xF609)
                        key = KEY_CODE_S4;
    //kenny add
            if(!gbEnableRecord)
            	{
                   if(tempkey == 0xa55a ||tempkey == 0x5aa5)
                   	{
                   	       config_set_rec_index(0);
                               printf( "index set default\n");
			        GPIO_SetState(22, MMP_TRUE);       	
				PalSleep(500);
		                GPIO_SetState(22, MMP_FALSE);       
                   	}
            	}
    #if (defined(COMPONENT_DEV) || defined(COMPOSITE_DEV))

        #ifndef PROJECT_FY286
                    if ((gtSystemStatus & STSTEM_STAND_ALONE_MODE) &&
                        (gtSystemStatus & SYSTEM_IN_RECORD_STATUS) == 0x0 &&
                        (gtSystemStatus & SYSTEM_SWITCH_DIGITAL_ANALOG) == 0x0 &&
                        !gbLockS3Key)
                    {
                        if (( tempkey == 0xFB04) && ((gtSystemStatus & SYSTEM_CVBS_INPUT) == 0))
                        {
                            gtSystemStatus &= (~SYSTEM_DIGITAL_INPUT);
                            gtSystemStatus |= SYSTEM_YPBPR_INPUT;
                            key             = KEY_CODE_S3; //AV
                        }
                        if ((tempkey == 0xFA05) && (( gtSystemStatus & SYSTEM_YPBPR_INPUT) == 0))
                        {
                            gtSystemStatus &= (~SYSTEM_CVBS_INPUT);
                            gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
                            key             = KEY_CODE_S3; //YPBPR
                        }
                        if ((tempkey == 0xF708 ) && ( (gtSystemStatus & SYSTEM_DIGITAL_INPUT) == 0))
                        {
                            gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
                            gtSystemStatus |= SYSTEM_CVBS_INPUT;
                            key             = KEY_CODE_S3; //HDMI
                        }
                    }

        #endif

    #endif
                }
            }
#endif

            if (!gbEnableRecord)
            {
                if ((key == KEY_CODE_S4)) //stop take shot
                {
                    OneShotMode = MMP_TRUE;
                    dbg_msg(DBG_MSG_TYPE_INFO, "  JPEG one snap \r\n");
                }
            }
            if (gbEnableRecord && !gbJPEGRecord)
            {
                if ((key == KEY_CODE_S4)) //stop take shot
                {
                    key = KEY_CODE_UNKNOW;
                    dbg_msg(DBG_MSG_TYPE_INFO, " disable JPEG record \r\n");
                }
            }

#ifdef PROJECT_FY286
            if ((key == KEY_CODE_S3)) //stop take shot
            {
                key = KEY_CODE_UNKNOW;
                dbg_msg(DBG_MSG_TYPE_INFO, " disable Key3 \r\n");
            }
#endif
            switch (key)
            {
            case KEY_CODE_S1:
                if (gtSystemStatus & SYSTEM_PC_CONNECTED_MODE)
                {
                    if (avSyncIsVideoStable() && !_IsContentProtection())
                    {
                        MMP_BOOL enable;
                        coreGetPCModeEnableRecord(&enable);
                        enable = enable ? MMP_FALSE : MMP_TRUE;
                        coreSetPCModeEnableRecord(enable);
                    }
                }
                else if (avSyncIsVideoStable())
                {
                    MMP_UINT32 errorMask;

                    errorMask = SYSTEM_USB_UNPLUG_ERROR |
                                SYSTEM_USB_NO_FREESPACE_ERROR |
                                SYSTEM_USB_MOUNT_ERROR |
                                SYSTEM_USB_NO_STORAGE_ERROR |
                                SYSTEM_UNSTABLE_ERROR |
                                SYSTEM_OUT_RECORD_STATUS |
                                SYSTEM_USB_BUSY_STATUS;

                    if ((gtSystemStatus & errorMask))
                    {
                        dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S1 NOT WORK ----\n");
                    }
                    else
                    {
                        if (!_IsContentProtection())
                        {
                            gbEnableRecord = !gbEnableRecord;

                            if (gbEnableRecord)
                                gtSystemStatus |= SYSTEM_IN_RECORD_STATUS;

                            dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S1 ----\n");
                        }
                        else
                        {
                            dbg_msg(DBG_MSG_TYPE_INFO, "---- HDCP PROTECT ----\n");
                        }
                    }
                }
                break;

            case KEY_CODE_S2:
                if (avSyncIsVideoStable() && (gtSystemStatus & STSTEM_STAND_ALONE_MODE))
                {
                    if ((gtSystemStatus & SYSTEM_IN_RECORD_STATUS) == 0x0)
                    {
                        MMP_BOOL isFULLHDSource;

                        if (RECORD_TABLE[mmpCapGetInputSrcInfo()].isFullHDRes)
                            isFULLHDSource = MMP_TRUE;
                        else
                            isFULLHDSource = MMP_FALSE;

                        if (isFULLHDSource)
                        {
                            if (gtSystemStatus & SYSTEM_FULL_HD_RECORD)
                            {
                                gtSystemStatus      &= (~SYSTEM_FULL_HD_RECORD);
                                gbLockatSDResolution = MMP_TRUE;
                            }
                            else
                            {
                                gtSystemStatus      |= SYSTEM_FULL_HD_RECORD;
                                gbLockatSDResolution = MMP_FALSE;
                            }
                        }
                        else
                        {
                            gtSystemStatus      &= (~SYSTEM_FULL_HD_RECORD);
                            gbLockatSDResolution = MMP_FALSE;
                        }
                        dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S2 ----\n");
                    }
                }
                break;

            case KEY_CODE_S3:
                if ((gtSystemStatus & STSTEM_STAND_ALONE_MODE))
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S3 ----\n");
                    //dbg_msg(DBG_MSG_TYPE_INFO, "--- adv7180 status= %d ---\n", mmpCapGetDeviceIsSignalStable(MMP_CAP_DEV_ADV7180));
                    if ((gtSystemStatus & SYSTEM_IN_RECORD_STATUS) == 0x0 &&
                        (gtSystemStatus & SYSTEM_SWITCH_DIGITAL_ANALOG) == 0x0 &&
                        !gbLockS3Key)
                    {
#ifndef INPUT_SWITCH_YPBPR_CVBS
                        if (gtSystemStatus & SYSTEM_DIGITAL_INPUT)
                        {
                            gtSystemStatus &= (~SYSTEM_DIGITAL_INPUT);
                            gtSystemStatus |= SYSTEM_SWITCH_DIGITAL_ANALOG;
                        }
                        else
                        {
                            gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
                            gtSystemStatus |= SYSTEM_SWITCH_DIGITAL_ANALOG;
                        }
#else
                        dbg_msg(DBG_MSG_TYPE_INFO, "gtSystemStatus =%x\r\n", gtSystemStatus);
                        if (gtSystemStatus & SYSTEM_DIGITAL_INPUT)
                        {
                            gtSystemStatus &= (~SYSTEM_DIGITAL_INPUT);
                            gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
                            gtSystemStatus |= SYSTEM_CVBS_INPUT;
                            dbg_msg(DBG_MSG_TYPE_INFO, "input: CVBS\r\n");
                        }
                        else if (gtSystemStatus & SYSTEM_CVBS_INPUT)
                        {
                            gtSystemStatus &= (~SYSTEM_CVBS_INPUT);
                            gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
                            gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
                            dbg_msg(DBG_MSG_TYPE_INFO, "input: HDMI\r\n");
                        }
			   else
			   {
                            gtSystemStatus &= (~SYSTEM_CVBS_INPUT);
                            gtSystemStatus &= (~SYSTEM_YPBPR_INPUT);
                            gtSystemStatus |= SYSTEM_DIGITAL_INPUT;
                            dbg_msg(DBG_MSG_TYPE_INFO, "default input: HDMI\r\n");			     
			   }
                        gtSystemStatus |= SYSTEM_SWITCH_DIGITAL_ANALOG;
#endif
                        //
                        gbLockS3Key     = MMP_TRUE;
                        lockS3KeyClock  = PalGetClock();
                    }
                    else
                        dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S3 NOT WORK ----\n");
                }
                break;

            case KEY_CODE_S4:
                if (avSyncIsVideoStable())
                {
                    MMP_UINT32 errorMask;

                    errorMask = SYSTEM_USB_UNPLUG_ERROR |
                                SYSTEM_USB_NO_FREESPACE_ERROR |
                                SYSTEM_USB_MOUNT_ERROR |
                                SYSTEM_USB_NO_STORAGE_ERROR |
                                SYSTEM_UNSTABLE_ERROR |
                                SYSTEM_HDCP_PROTECTED |
                                SYSTEM_OUT_RECORD_STATUS |
                                SYSTEM_USB_BUSY_STATUS;

                    if ((gtSystemStatus & errorMask))
                    {
                        dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S4 NOT WORK ----%x\n", gtSystemStatus);
                    }
                    else
                    {
                        if (!_IsContentProtection())
                        {
                            gbEnableRecord = !gbEnableRecord;

                            if (gbEnableRecord)
                            {
                                gbJPEGRecord    = MMP_TRUE;//kenny test
                                gtSystemStatus |= SYSTEM_IN_RECORD_STATUS;
                                JPEGClock       = PalGetClock();
                            }
                            bshotinterval = 1;

                            dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S4 ----\n");
                        }
                        else
                        {
                            dbg_msg(DBG_MSG_TYPE_INFO, "---- HDCP PROTECT ----\n");
                        }
                    }
                }
                else
                {
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- KEY_CODE_S4 NOT WORK ----%x\n", gtSystemStatus);
                }

                break;

            case KEY_CODE_S1_HOLD:
                if ((gtSystemStatus & STSTEM_STAND_ALONE_MODE) &&
                    ((gtSystemStatus & SYSTEM_HDCP_SWITCH) == 0x0) &&
                    (gtSystemStatus & SYSTEM_DIGITAL_INPUT) &&
                    (gbEnableRecord == MMP_FALSE))
                {
                    MMP_BOOL flag = coreIsDisableHDCP();
                    flag            = flag ? MMP_FALSE : MMP_TRUE;
                    coreDisableHDCP(flag);
                    dbg_msg(DBG_MSG_TYPE_INFO, "---- HDCP SWITCH %d----\n", flag);
                    gtSystemStatus |= SYSTEM_HDCP_SWITCH;
                }
                break;

            default:
                // polling time of key without pressing, reduce for sensitivity
                buttonPollingTime = 50;
                break;
            }
        }

        if (gtSystemStatus & STSTEM_STAND_ALONE_MODE)
        {
            MMP_BOOL isDiableHDCP   = mmpHDMIRXIsDisableHDCP();
            MMP_BOOL isHDCPKeyEmpty = mmpHDMIRXIsHDCPKeyEmpty();

            if (isDiableHDCP != isHDCPKeyEmpty)
                gbHDMIRXReboot = MMP_TRUE;
            else
                gbHDMIRXReboot = MMP_FALSE;
        }
        else
            gbHDMIRXReboot = MMP_FALSE;

        gtAudioCodec.bHDMIInput = ((coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_HDMI) || (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI)) ? MMP_TRUE : MMP_FALSE;
        gtAudioCodec.bDVIMode   = (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI) ? MMP_TRUE : MMP_FALSE;
        gtAudioCodec.bEnRecord  = gbEnableRecord;
        _AudioCodecControl(&gtAudioCodec);

        if ((gtSystemStatus & SYSTEM_PC_CONNECTED_MODE))
        {
#if 0
            if (gtPCModeLed.BlingRingLed == LED_ON)
            {
                gtPCModeLed.GreenLed = LED_OFF;
                gtPCModeLed.BlueLed  = LED_OFF;
            }
            else
            {
                if (avSyncIsVideoStable())
                {
                    if (_IsContentProtection())
                        gtPCModeLed.GreenLed = LED_BLINK;
                    else
                        gtPCModeLed.GreenLed = LED_ON;

                    gtPCModeLed.BlueLed = LED_OFF;
                }
                else
                {
                    gtPCModeLed.GreenLed = LED_OFF;
                    gtPCModeLed.BlueLed  = LED_BLINK;
                }
            }
#endif
        }

content_switch:
        PalSleep(pollingTime);
    }
}

static void
_UsbManagerFunction(
    void)
{
    MMP_UINT32        remainspace;
    MMP_INT           volumeIndex  = 0;
    static MMP_BOOL   bUsbInsert   = MMP_FALSE;
    static MMP_CHAR   *buf         = MMP_NULL;
    static PAL_FILE   *fp          = MMP_NULL;
    static MMP_UINT32 writeMBCount = 0;
    static MMP_WCHAR  filePath[16] = {0};
    //static PAL_DEVICE_TYPE   devtype = PAL_DEVICE_TYPE_USB0;
    MMP_BOOL          stable       = MMP_FALSE;

    //dbg_msg(DBG_MSG_TYPE_INFO, "device type = %d\n", gtdevicetype);
    if (gbJPEGRecord && gbRecording)
        JPEG_Write_Card(pJPEGHeaderData, jpegheaderSize, pJPEGFileData, jpegdataSize);

    if (gtSystemStatus & SYSTEM_USB_NO_STORAGE_ERROR)
        stable = MMP_FALSE;
    else if ((gtdevicetype == PAL_DEVICE_TYPE_USB0) && !storageMgrGetUSBEvent(PAL_DEVICE_TYPE_USB0))
        stable = MMP_FALSE;
    else if ((gtdevicetype == PAL_DEVICE_TYPE_SD) && !storageMgrGetUSBEvent(PAL_DEVICE_TYPE_SD))
        stable = MMP_FALSE;
    else if (gtSystemStatus & SYSTEM_STORAGE_SWITCH)
        stable = MMP_FALSE;
    else
        stable = MMP_TRUE;

    if (!stable)
    {
        bUsbInsert = MMP_FALSE;
        if (gtdevicetype == PAL_DEVICE_TYPE_USB0)
        {
#if 0//def PROJECT_FY286
            if (0)
#else
            if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_SD))
#endif
            {
                gtSystemStatus &= (~SYSTEM_SELECT_USB_STORAGE);
                gtSystemStatus &= (~SYSTEM_USB_NO_STORAGE_ERROR);
                gtSystemStatus &= (~SYSTEM_STORAGE_SWITCH);
                gtdevicetype    = PAL_DEVICE_TYPE_SD;
                config_set_dev_type(gtdevicetype);
            }
            else if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_USB0))
            {
                gtSystemStatus |= SYSTEM_SELECT_USB_STORAGE;
                gtSystemStatus &= (~SYSTEM_USB_NO_STORAGE_ERROR);
                gtSystemStatus &= (~SYSTEM_STORAGE_SWITCH);
                gtdevicetype    = PAL_DEVICE_TYPE_USB0;
                config_set_dev_type(gtdevicetype);
            }
            else
            {
                            //kenny
                       if (gbEnableRecord)
    		    {
     		       gbEnableRecord  = MMP_FALSE;
          		  //dbg_msg(DBG_MSG_TYPE_INFO, " USB unplug , Stop Record!!!!!\n");
         		   gtSystemStatus |= SYSTEM_USB_UNPLUG_ERROR;
         		   gtSystemStatus &= ~SYSTEM_IN_RECORD_STATUS;
      			  }
                gtSystemStatus |= SYSTEM_USB_NO_STORAGE_ERROR;
                goto End;
            }
        }
        else
        {
            if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_USB0))
            {
                gtSystemStatus |= SYSTEM_SELECT_USB_STORAGE;
                gtSystemStatus &= (~SYSTEM_USB_NO_STORAGE_ERROR);
                gtSystemStatus &= (~SYSTEM_STORAGE_SWITCH);
                gtdevicetype    = PAL_DEVICE_TYPE_USB0;
                config_set_dev_type(gtdevicetype);
            }
#if 1//ndef PROJECT_FY286
            else if (storageMgrGetUSBEvent(PAL_DEVICE_TYPE_SD))
            {
                gtSystemStatus &= (~SYSTEM_SELECT_USB_STORAGE);
                gtSystemStatus &= (~SYSTEM_USB_NO_STORAGE_ERROR);
                gtSystemStatus &= (~SYSTEM_STORAGE_SWITCH);
                gtdevicetype    = PAL_DEVICE_TYPE_SD;
                config_set_dev_type(gtdevicetype);
            }
#endif
            else
            {
                            //kenny
                       if (gbEnableRecord)
    		    {
     		       gbEnableRecord  = MMP_FALSE;
          		  //dbg_msg(DBG_MSG_TYPE_INFO, " USB unplug , Stop Record!!!!!\n");
         		   gtSystemStatus |= SYSTEM_USB_UNPLUG_ERROR;
         		   gtSystemStatus &= ~SYSTEM_IN_RECORD_STATUS;
      			  }
                gtSystemStatus |= SYSTEM_USB_NO_STORAGE_ERROR;
                goto End;
            }
        }
    }

    /* Check USB Plug/Unplug */
    if (!storageMgrGetUSBEvent(gtdevicetype))
    {
        if (gbEnableRecord)
        {
            gbEnableRecord  = MMP_FALSE;
            //dbg_msg(DBG_MSG_TYPE_INFO, " USB unplug , Stop Record!!!!!\n");
            gtSystemStatus |= SYSTEM_USB_UNPLUG_ERROR;
            gtSystemStatus &= ~SYSTEM_IN_RECORD_STATUS;
        }
        gtSystemStatus |= SYSTEM_USB_NO_STORAGE_ERROR;
        bUsbInsert      = MMP_FALSE;
    }
    else //USB HotPlug Detect
    {
        if (bUsbInsert == MMP_FALSE)
        {
            gtSystemStatus |= SYSTEM_USB_BUSY_STATUS;

            /* Check USB Mount */
            if (!storageMgrGetCardVolume(gtdevicetype, 0))
                gtSystemStatus |= SYSTEM_USB_MOUNT_ERROR;
            else
                gtSystemStatus &= ~SYSTEM_USB_MOUNT_ERROR;

            /*Check USB FreeSpace*/
            bUsbInsert = MMP_TRUE;
            if (!_CheckUsbFreeSpace(&remainspace))
            {
                gtSystemStatus |= SYSTEM_USB_NO_FREESPACE_ERROR;
                writeMBCount    = 0;
                return;
            }
            else
                gtSystemStatus &= ~SYSTEM_USB_NO_FREESPACE_ERROR;

            // Write Test
            volumeIndex = storageMgrGetCardVolume(gtdevicetype, 0);
            if (volumeIndex < 0)
                return;
            filePath[0] = PAL_T('A') + volumeIndex;
            PalMemcpy(&filePath[1], &PAL_T(":/rec.tmp"), sizeof(PAL_T(":/rec.tmp")));

            fp          = PalWFileOpen(filePath, PAL_FILE_WB, MMP_NULL, MMP_NULL);
            if (fp)
            {
                if (buf)
                    PalHeapFree(PAL_HEAP_DEFAULT, buf);
                buf          = PalHeapAlloc(PAL_HEAP_DEFAULT, 1048576);
                writeMBCount = 1;
                dbg_msg(DBG_MSG_TYPE_INFO, "write test start\n");
            }
            else
            {
                writeMBCount = 0;
            }

            /* Clear Error Flag */
            gtSystemStatus &= ~SYSTEM_USB_NO_STORAGE_ERROR;
            gtSystemStatus &= ~SYSTEM_USB_UNPLUG_ERROR;
            dbg_msg(DBG_MSG_TYPE_INFO, "USB hotplug Detect gtSystemStatus = 0x%x\n", gtSystemStatus);
        }
        else
        {
            if (gtSystemStatus & SYSTEM_USB_BUSY_STATUS)
            {
                if (writeMBCount)
                {
                    if (buf && fp)
                        PalFileWrite(buf, 1, 1048576, fp, MMP_NULL, MMP_NULL);
                    writeMBCount--;
                }
                else
                {
                    if (buf)
                    {
                        PalHeapFree(PAL_HEAP_DEFAULT, buf);
                        buf = MMP_NULL;
                    }
                    if (fp)
                    {
                        PalTFileClose(fp, MMP_NULL, MMP_NULL);
                        fp = MMP_NULL;
                    }
                    PalTFileDelete(filePath, MMP_NULL, MMP_NULL);
                    gtSystemStatus &= ~SYSTEM_USB_BUSY_STATUS;
                    dbg_msg(DBG_MSG_TYPE_INFO, "write test end\n");
	          #ifdef SEND_SPACE_INFO
		    _CheckUsbTotalSpace(&Gtotalspace);
		    _CheckUsbFreeSpace(&Gremainspace);
		    printf("start G space=%d,R space=%d\r\n",Gtotalspace,Gremainspace);
		  #endif

                }
            }
        }
    }
End:
    if (gtSystemStatus & SYSTEM_USB_NO_STORAGE_ERROR)
        dbg_msg(DBG_MSG_TYPE_STREAM_READER, "No Storage Device\n");
}

/*static*/ MMP_BOOL
_CheckUsbFreeSpace(
    MMP_UINT32 *remainspace)
{
    MMP_UINT32      availableSize_h;
    MMP_UINT32      availableSize_l;
    //MMP_FLOAT   availableSize;
    MMP_UINT32      freespace = 0;
    PAL_DEVICE_TYPE devtype   = PAL_DEVICE_TYPE_USB0;

    if (gtSystemStatus & SYSTEM_SELECT_USB_STORAGE)
        devtype = PAL_DEVICE_TYPE_USB0;
    else
        devtype = PAL_DEVICE_TYPE_SD;

    /* Check USB FreeSpace */
    PalDiskGetFreeSpace(storageMgrGetCardVolume(devtype, 0), &availableSize_h, &availableSize_l, MMP_NULL, MMP_NULL);

    if (availableSize_l >= (1ul << 20) * 1000 || availableSize_h > 0)
        *remainspace = freespace = (availableSize_l >> 20) + (availableSize_h * 4 * 1024);
    else if (availableSize_l < (1ul << 20) * 1000 && availableSize_l >= (1ul << 10) * 1000)
        *remainspace = freespace = ((availableSize_l >> 10) / 1024);
    else if (availableSize_l < (1ul << 10) * 1000)
        *remainspace = freespace = 0;
    dbg_msg(DBG_MSG_TYPE_INFO, "freespace = %d MB\n", freespace);

    if (freespace <= USB_MIN_FREESPACE_MB)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "!!!! USB FULL MB !!!!\n");
        return MMP_FALSE;
    }
    else
        return MMP_TRUE;

    //    if (availableSize_l >= (1ul << 20) * 1000 || availableSize_h > 0)
    //    {
    //        freespace = (availableSize_l >> 20) + (availableSize_h*4 *1024);
    //        *remainspace = freespace;
    //
    //        dbg_msg(DBG_MSG_TYPE_INFO, "freespace = %d MB\n", freespace);
    //        if (freespace < USB_MIN_FREESPACE_MB)
    //        {
    //            dbg_msg(DBG_MSG_TYPE_INFO, "!!!! USB FULL GB !!!!\n");
    //            return MMP_FALSE;
    //        }
    //        else
    //            return MMP_TRUE;
    //    }
    //    else if (availableSize_l < (1ul << 20) * 1000 && availableSize_l >= (1ul << 10) * 1000)
    //    {
    //        freespace = ((availableSize_l >> 10) /1024);
    //        *remainspace = freespace;
    //        dbg_msg(DBG_MSG_TYPE_INFO, "%s %d freespace = %d\n", __FUNCTION__, __LINE__, freespace);
    //        if (freespace < USB_MIN_FREESPACE_MB)
    //        {
    //            dbg_msg(DBG_MSG_TYPE_INFO, "!!!! USB FULL MB !!!!\n");
    //            return MMP_FALSE;
    //        }
    //        else
    //            return MMP_TRUE;
    //    }
    //    else if (availableSize_l < (1ul << 10) * 1000)
    //    {
    //        *remainspace = 0;
    //        //dbg_msg(DBG_MSG_TYPE_INFO, "availableSize = %u KB\n", availableSize);
    //        return MMP_FALSE;
    //    }
}


#ifdef SEND_SPACE_INFO
static MMP_BOOL
_CheckUsbTotalSpace(
    MMP_UINT32* tatolspace)
{

    MMP_UINT32  availableSize_h;
	MMP_UINT32  availableSize_l;
    //MMP_FLOAT   availableSize;
    MMP_UINT32  tatolspace1 =  0;

    PAL_DEVICE_TYPE devtype = PAL_DEVICE_TYPE_USB0;

    if (gtSystemStatus & SYSTEM_SELECT_USB_STORAGE)
        devtype = PAL_DEVICE_TYPE_USB0;
    else
        devtype = PAL_DEVICE_TYPE_SD;
	
    /* Check USB tatolSpace */
    PalDiskGetTotalSpace(storageMgrGetCardVolume(devtype, 0), &availableSize_h, &availableSize_l, MMP_NULL, MMP_NULL);
        if (availableSize_l >= (1ul << 20) * 1000 || availableSize_h > 0)
        *tatolspace = tatolspace1 = (availableSize_l >> 20) + (availableSize_h*4 *1024);
    else if (availableSize_l < (1ul << 20) * 1000 && availableSize_l >= (1ul << 10) * 1000)
        *tatolspace = tatolspace1 = ((availableSize_l >> 10) /1024);
    else if (availableSize_l < (1ul << 10) * 1000)
        *tatolspace = tatolspace1 = 0;
 //   printf( "tatolspace = %d MB\n", tatolspace1);
   return MMP_TRUE;
}

void CheckSpaceInfo()
{
static MMP_UINT32 LastGremainspace = 1;
static MMP_UINT32  locksapceClock = 1;
int flag,i=0,temp =0;
MMP_UINT32 Lastspace = 0;
MMP_UINT8 DATA=0;

 //  extern MMP_BOOL GpioInputselect(MMP_BOOL status);

if(failwritecount >6)
{
    return;
}

 if(gtSystemStatus & SYSTEM_USB_NO_STORAGE_ERROR)
 {
     Gtotalspace =0;
     Gremainspace = 0;
	// printf("no storage error\r\n");
 }
 
   if((PalGetDuration(locksapceClock) > 3*1000 && !gbRecording)||(PalGetDuration(locksapceClock) > 60*1000 && gbRecording))
  { 	
//printf(" Tatol = %d,Remain =%d\r\n",Gtotalspace,Gremainspace);
         if(Gremainspace !=LastGremainspace)
     	{
           if(Gtotalspace !=0 )
           	{
	            Gremain =( Gremainspace*100)/Gtotalspace;
           	}
	     else
	     	{
	     	   Gremain =0;
	     	}
		   
     	   GpioInputselect2(MMP_TRUE);
	    PalSleep(5);
     	    mmpIicLockModule();
			
	   Lastspace = Gtotalspace;		
	    for(i=0;i<4;i++)
	    {
		temp= 8*i;
		if(temp !=0)
		{
	             DATA= (Lastspace>>temp) &0xFF;
		}
		else
		{

		     DATA= (Lastspace) &0xFF;
		}
           //     printf("set Gtotalspace %d =%x\r\n",i,DATA);
		PalSleep(5);
                if(0 != (flag = mmpIicSendData(0x01, 0x50, 0x10+i, &DATA, 1)))
               {
    	        printf("24C02# IIC Write Fail!1\n");
    	        mmpIicGenStop();
	                if(0 != (flag = mmpIicSendData(0x01, 0x50, 0x10+i, &DATA, 1)))
             		  {
    	       			 printf("24C02# IIC Write Fail!11\n");
    	       			 mmpIicGenStop();				
               		 }			
                }
	   }
	    Lastspace = Gremainspace;	
	    for(i=0;i<4;i++)
	    {
		temp= 8*i;
		if(temp !=0)
		{
	             DATA= (Lastspace>>temp) &0xFF;
		}
		else
		{

		     DATA= (Lastspace) &0xFF;
		}
            //     printf("set Gremainspace %d =%x\r\n",i,DATA);
		PalSleep(5);		 
                if(0 != (flag = mmpIicSendData(0x01, 0x50, 0x20+i, &DATA, 1)))
               {
    	        printf("24C02# IIC Write Fail!2\n");	
    	        mmpIicGenStop();
			  if(0 != (flag = mmpIicSendData(0x01, 0x50, 0x20+i, &DATA, 1)))
             		  {           	
    	       			 printf("24C02# IIC Write Fail!22\n");
    	       			 mmpIicGenStop();				
               		 }
                }
	   }

            DATA = Gremain ;
	    printf("remain space percentage = %d\r\n",DATA);	
		PalSleep(5);
	     if(0 != (flag = mmpIicSendData(0x01, 0x50, 0x30, &DATA, 1)))
               {
    	        printf("24C02# IIC Write Fail!3\n");
		failwritecount++;
    	        mmpIicGenStop();
	                if(0 != (flag = mmpIicSendData(0x01, 0x50, 0x30, &DATA, 1)))
             		  {
    	       			 printf("24C02# IIC Write Fail!33\n");
				 failwritecount++;
    	       			 mmpIicGenStop();				
               		 }
		        else
		        {
		             failwritecount = 0;
		        }
                }
	     else
	     	{
	     	    failwritecount = 0;
	     	}
		 
            mmpIicReleaseModule();
	   PalSleep(5);
	   GpioInputselect2(MMP_FALSE);
	 
     	     printf("set Disc Space Tatol = %d,Remain =%d\r\n",Gtotalspace,Gremainspace);
	     LastGremainspace = Gremainspace;
     	}

        locksapceClock = PalGetClock();
   }
   
}

#endif


static void
_LedTimerInit(
    void)
{
    mmpTimerResetTimer(LED_TIMER_NUM);

    // Initialize Timer IRQ
    ithIntrDisableIrq(LED_TIMER_NUM);
    ithIntrClearIrq(LED_TIMER_NUM);

    // register Timer Handler to IRQ
    ithIntrRegisterHandlerIrq(ITH_INTR_TIMER6, _LedTimerIsr, (void *)ITH_INTR_TIMER6);

    // set Timer IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_TIMER6, ITH_INTR_EDGE);

    // set Timer IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_TIMER6, ITH_INTR_HIGH_RISING);

    // Enable Timer IRQ
    ithIntrEnableIrq(ITH_INTR_TIMER6);

    mmpTimerSetTimeOut(LED_TIMER_NUM, LED_TIMER_TIMEOUT);
    mmpTimerCtrlEnable(LED_TIMER_NUM, MMP_TIMER_EN);

    dbg_msg(DBG_MSG_TYPE_INFO, "Led Timer Init\n");
}

static void
_LedTimerTerminate(
    void)
{
    mmpTimerCtrlDisable(LED_TIMER_NUM, MMP_TIMER_EN);
    mmpTimerResetTimer(LED_TIMER_NUM);
    ithIntrDisableIrq(ITH_INTR_TIMER6);
    ithIntrClearIrq(ITH_INTR_TIMER6);
}

static void
_LedTimerIsr(
    void *data)
{
    uint32_t timer       = (uint32_t)data;
    MMP_BOOL isRecordLed = MMP_FALSE;
    //MMP_BOOL isRedLedBlink = MMP_FALSE;

    //MMP_BOOL isAnalogLedOn = MMP_FALSE;
    //MMP_BOOL isHDMILedOn   = MMP_FALSE;

    if (gTimerIsrCount % 12 == 0)
        mmpWatchDogRefreshTimer();

    if (gTimerIsrCount % 3 == 0) // interval is LED_TIMER_TIMEOUT *3
    {
        if ((gtSystemStatus & STSTEM_STAND_ALONE_MODE))
        {
            //Record Led Control
            {
                MMP_UINT32 statusMaskOn;
                MMP_UINT32 statusMaskBlink;

                statusMaskOn    = SYSTEM_IN_RECORD_STATUS;
                statusMaskBlink = SYSTEM_OUT_RECORD_STATUS |
                                  SYSTEM_IN_UPGRADE_STATUS;

                /* kenny     if (gtSystemStatus & SYSTEM_HDCP_SWITCH)
                     {
                         isRecordLed = MMP_TRUE;
                         gtLedCtrl.EnRecord =  MMP_TRUE;
                         gtHDCPSwitchLedCount++;

                         if (mmpHDMIRXForISRIsDisableHDCP()) //HDCP OFF
                         {
                             if (gtHDCPSwitchLedCount%2 == 0)
                             {
                                 gtLedCtrl.EnRecord = MMP_FALSE;
                                 gtSystemStatus &= (~SYSTEM_HDCP_SWITCH);
                                 gtHDCPSwitchLedCount = 0;
                             }
                         }
                         else //HDCP ON
                         {
                             if (gtHDCPSwitchLedCount%4 == 2)
                             {
                                 gtLedCtrl.EnRecord = MMP_FALSE;
                             }
                             else if (gtHDCPSwitchLedCount%4 == 3)
                             {
                                 gtLedCtrl.EnRecord = MMP_TRUE;
                             }
                             else if (gtHDCPSwitchLedCount%4 == 0)
                             {
                                 gtLedCtrl.EnRecord = MMP_FALSE;
                                 gtSystemStatus &= (~SYSTEM_HDCP_SWITCH);
                                 gtHDCPSwitchLedCount = 0;
                             }
                         }
                     }
                     else*/if (gtSystemStatus & statusMaskBlink)
                {
                    isRecordLed        = MMP_TRUE;
                    gtLedCtrl.EnRecord = (gtLedCtrl.EnRecord) ? MMP_FALSE : MMP_TRUE;
                }
                else if (gtSystemStatus & statusMaskOn)
                {
                    isRecordLed        = MMP_TRUE;
                    gtLedCtrl.EnRecord = MMP_TRUE;
                }
                else
                {
                    isRecordLed        = MMP_FALSE;
                    gtLedCtrl.EnRecord = MMP_FALSE;
                }
            }

            //Signal Led Control
            {
                MMP_UINT32 statusMaskBlink;

                statusMaskBlink = SYSTEM_INITIALIZE;

                if (gtSystemStatus & statusMaskBlink)
                    gtLedCtrl.EnPower = (gtLedCtrl.EnPower) ? MMP_FALSE : MMP_TRUE;
                else
                    gtLedCtrl.EnPower = MMP_TRUE;
            }
            //720P & 1080P Control
            if (gtSystemStatus & SYSTEM_STARTUP_LED)
            {
                static MMP_BOOL rec1080p = MMP_FALSE;
                if (gtSystemStatus & SYSTEM_FULL_HD_RECORD)
                {
                    if (rec1080p == MMP_FALSE)
                    {
                        GPIO_SetState(32, 0);
                        GPIO_SetMode(32, 1);
                        ithGpioSetMode(32, 0);
                        rec1080p = MMP_TRUE;
                    }
                    gtLedCtrl.En720P  = MMP_FALSE;
                    gtLedCtrl.En1080P = MMP_TRUE;
                }
                else
                {
                    rec1080p          = MMP_FALSE;
                    gtLedCtrl.En720P  = MMP_TRUE;
                    gtLedCtrl.En1080P = MMP_FALSE;
                }
            }/*
                else
                {
                gtLedCtrl.En720P = MMP_TRUE;
                gtLedCtrl.En1080P = MMP_TRUE;
                }*/

            //Analog & Digital Control
            if (gtSystemStatus & SYSTEM_STARTUP_LED)
            {
                MMP_UINT32     statusMaskOn;
                MMP_UINT32     statusMaskOff;
                MMP_UINT32     statusMaskBlink;
                MMP_UINT32     statusMaskInterBlink;
                static MMP_INT sync = MMP_TRUE;

                statusMaskOn    = SYSTEM_IN_RECORD_STATUS;
                statusMaskOff   = SYSTEM_UNSTABLE_ERROR;
                statusMaskBlink = SYSTEM_HDCP_PROTECTED;

                if (gtSystemStatus & SYSTEM_DIGITAL_INPUT)
                {
                    sync = MMP_TRUE;
                    /* if (gtSystemStatus & statusMaskOn)
                       {
                         gtLedCtrl.EnHDMI = MMP_FALSE;
                         gtLedCtrl.EnAnalog = MMP_TRUE;
                       }
                       else if (gtSystemStatus & statusMaskOff)
                       {
                         gtLedCtrl.EnHDMI = MMP_TRUE;
                         gtLedCtrl.EnAnalog = MMP_TRUE;
                       }
                       else*/if ((gtSystemStatus & statusMaskOff) && !gbEnableRecord)
                    {
                        // gtLedCtrl.EnAnalog = gtLedCtrl.EnHDMI;
                        gtLedCtrl.EnAnalog = MMP_FALSE;
                        gtLedCtrl.EnHDMI   = (gtLedCtrl.EnHDMI) ? MMP_FALSE : MMP_TRUE;
                    }
                    else
                    {
                        gtLedCtrl.EnHDMI   = MMP_TRUE;
                        gtLedCtrl.EnAnalog = MMP_FALSE;
                    }
                }
#ifdef INPUT_SWITCH_YPBPR_CVBS
                if (gtSystemStatus & SYSTEM_CVBS_INPUT)
                {
                    if (sync == MMP_TRUE)
                    {
                        gtLedCtrl.EnHDMI   = MMP_TRUE;
                        gtLedCtrl.EnAnalog = MMP_TRUE;
                        sync               = MMP_FALSE;
                    }

                    if ((gtSystemStatus & statusMaskOff) && !gbEnableRecord)
                    {
                        gtLedCtrl.EnHDMI   = (gtLedCtrl.EnHDMI) ? MMP_FALSE : MMP_TRUE;
                        gtLedCtrl.EnAnalog = (gtLedCtrl.EnAnalog) ? MMP_FALSE : MMP_TRUE;
                    }
                    else
                    {
                        gtLedCtrl.EnHDMI   = MMP_TRUE;
                        gtLedCtrl.EnAnalog = MMP_TRUE;
                    }
                }
#endif
            }
            //SD and USB Stroage Ctrl
            {
                MMP_UINT32 statusMaskOff;
                MMP_UINT32 statusMaskBlink;
                MMP_UINT32 statusMaskInterBlink;

                statusMaskOff        = SYSTEM_USB_NO_STORAGE_ERROR | SYSTEM_OUT_RECORD_STATUS;
                statusMaskBlink      = SYSTEM_IN_RECORD_STATUS | SYSTEM_IN_UPGRADE_STATUS;
                statusMaskInterBlink = SYSTEM_USB_NO_FREESPACE_ERROR;

                if (gtSystemStatus & statusMaskOff)
                {
                    gtLedCtrl.EnUSB = MMP_TRUE;
                    gtLedCtrl.EnSD  = MMP_TRUE;
                }
                else
                {
                    if (gtSystemStatus & SYSTEM_SELECT_USB_STORAGE)
                    {
                        if (gtSystemStatus & statusMaskBlink)
                        {
                            gtLedCtrl.EnUSB = (gtLedCtrl.EnUSB) ? MMP_FALSE : MMP_TRUE;
                            gtLedCtrl.EnSD  = MMP_TRUE;
                        }
                        else if (gtSystemStatus & statusMaskInterBlink)
                        {
                            gtLedCtrl.EnUSB = (gtLedCtrl.EnUSB) ? MMP_FALSE : MMP_TRUE;
                            gtLedCtrl.EnSD  = (gtLedCtrl.EnSD) ? MMP_FALSE : MMP_TRUE;
                        }
                        else
                        {
                            gtLedCtrl.EnUSB = MMP_FALSE;
                            gtLedCtrl.EnSD  = MMP_TRUE;
                        }
                    }
                    else
                    {
                        if (gtSystemStatus & statusMaskBlink)
                        {
                            gtLedCtrl.EnSD  = (gtLedCtrl.EnSD) ? MMP_FALSE : MMP_TRUE;
                            gtLedCtrl.EnUSB = MMP_TRUE;
                        }
                        else if (gtSystemStatus & statusMaskInterBlink)
                        {
                            gtLedCtrl.EnUSB = !gtLedCtrl.EnSD;
                            gtLedCtrl.EnUSB = (gtLedCtrl.EnUSB) ? MMP_FALSE : MMP_TRUE;
                            gtLedCtrl.EnSD  = (gtLedCtrl.EnSD) ? MMP_FALSE : MMP_TRUE;
                        }
                        else
                        {
                            gtLedCtrl.EnSD  = MMP_FALSE;
                            gtLedCtrl.EnUSB = MMP_TRUE;
                        }
                    }
                }
            }
        }
        else if ((gtSystemStatus & SYSTEM_PC_CONNECTED_MODE))
        {
            //Record Led Control
            if (gtPCModeLed.RecordLed == LED_ON)
                gtLedCtrl.EnRecord = MMP_TRUE;
            else if (gtPCModeLed.RecordLed == LED_OFF)
                gtLedCtrl.EnRecord = MMP_FALSE;
            else if (gtPCModeLed.RecordLed == LED_BLINK)
                gtLedCtrl.EnRecord = (gtLedCtrl.EnRecord) ? MMP_FALSE : MMP_TRUE;

            //Signal Led Control
            if (gtPCModeLed.SignalLed == LED_ON)
                gtLedCtrl.EnPower = MMP_TRUE;
            else if (gtPCModeLed.SignalLed == LED_OFF)
                gtLedCtrl.EnPower = MMP_FALSE;
            else if (gtPCModeLed.SignalLed == LED_BLINK)
                gtLedCtrl.EnPower = (gtLedCtrl.EnPower) ? MMP_FALSE : MMP_TRUE;

            //720P & 1080P Led Control
            if (gtPCModeLed.SD720PLed == LED_ON)
                gtLedCtrl.En720P = MMP_TRUE;
            else if (gtPCModeLed.SD720PLed == LED_OFF)
                gtLedCtrl.En720P = MMP_FALSE;
            else if (gtPCModeLed.SD720PLed == LED_BLINK)
                gtLedCtrl.En720P = (gtLedCtrl.En720P) ? MMP_FALSE : MMP_TRUE;

            if (gtPCModeLed.HD1080PLed == LED_ON)
                gtLedCtrl.En1080P = MMP_TRUE;
            else if (gtPCModeLed.HD1080PLed == LED_OFF)
                gtLedCtrl.En1080P = MMP_FALSE;
            else if (gtPCModeLed.HD1080PLed == LED_BLINK)
                gtLedCtrl.En1080P = (gtLedCtrl.En1080P) ? MMP_FALSE : MMP_TRUE;

            //Analog & Digital Led Control
            if (gtPCModeLed.AnalogLed == LED_ON)
                gtLedCtrl.EnAnalog = MMP_TRUE;
            else if (gtPCModeLed.AnalogLed == LED_OFF)
                gtLedCtrl.EnAnalog = MMP_FALSE;
            else if (gtPCModeLed.AnalogLed == LED_BLINK)
                gtLedCtrl.EnAnalog = (gtLedCtrl.EnAnalog) ? MMP_FALSE : MMP_TRUE;

            if (gtPCModeLed.HDMILed == LED_ON)
                gtLedCtrl.EnHDMI = MMP_TRUE;
            else if (gtPCModeLed.HDMILed == LED_OFF)
                gtLedCtrl.EnHDMI = MMP_FALSE;
            else if (gtPCModeLed.HDMILed == LED_BLINK)
                gtLedCtrl.EnHDMI = (gtLedCtrl.EnHDMI) ? MMP_FALSE : MMP_TRUE;
        }
    }

    GpioLedControl(&gtLedCtrl);
    gTimerIsrCount++;
    AHB_WriteRegisterMask(TIMER_BASE + 0x7C, 0x7 << (timer * 4), 0x7 << (timer * 4));
}

static MMP_BOOL
_IsContentProtection(
    void)
{
    if (gtDisableHDCP)
        return MMP_FALSE;
    else if ((mmpCapGetCaptureDevice() != CAPTURE_DEV_HDMIRX)
             || (!mmpHDMIRXIsHDCPOn()))
        return MMP_FALSE;
    else
        return MMP_TRUE;
}

// This is a special function from customer's request.
// Read file created by user to set RTC current time.
static void
_SetRtcFromFile(
    void)
{
#ifdef HAVE_FAT
    MMP_INT    volumeIndex      = 0;
    MMP_WCHAR  filePath[16]     = {0};
    PAL_FILE   *pRtcFile        = MMP_NULL;
    MMP_UINT8  *pReadFileBuffer = MMP_NULL;
    MMP_UINT32 fileSize         = 0;
    MMP_UINT8  *pDateStr        = MMP_NULL;
    MMP_UINT   i                = 0;
    MMP_UINT8  *str[6];
    MMP_UINT8  buf[10];

    if (storageMgrIsCardInsert(PAL_DEVICE_TYPE_SD) || storageMgrIsCardInsert(PAL_DEVICE_TYPE_USB0))
    {
        if (storageMgrIsCardInsert(PAL_DEVICE_TYPE_SD))
        {
            volumeIndex = storageMgrGetCardVolume(PAL_DEVICE_TYPE_SD, 0);
        }
        else
        {
            volumeIndex = storageMgrGetCardVolume(PAL_DEVICE_TYPE_USB0, 0);
        }
        if (volumeIndex < 0)
            return;
        filePath[0] = PAL_T('A') + volumeIndex;
        PalMemcpy(&filePath[1], &PAL_T(":/rtc_setup.txt"), sizeof(PAL_T(":/rtc_setup.txt")));
        pRtcFile    = PalTFileOpen(filePath, PAL_FILE_RB, MMP_NULL, MMP_NULL);

        if (pRtcFile)
        {
            fileSize        = PalTGetFileLength(filePath);
            dbg_msg(DBG_MSG_TYPE_INFO, "found JEDI rtc_setup file, size: %u\n", fileSize);
            pReadFileBuffer = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, fileSize);
            if (pReadFileBuffer)
            {
                PalTFileRead(pReadFileBuffer, 1, fileSize, pRtcFile, MMP_NULL, MMP_NULL);
            }

            pDateStr = strtok(pReadFileBuffer, "/ :");
            while (pDateStr != MMP_NULL)
            {
                PalMemset(buf, 0, 10);
                //dbg_msg(DBG_MSG_TYPE_INFO, "pDateStr= %s\n", pDateStr);
                str[i]   = pDateStr;
                //dbg_msg(DBG_MSG_TYPE_INFO, "str[%d] = %s\n", i , str[i]);

                pDateStr = strtok(MMP_NULL, "/ :");
                switch (i)
                {
                case 0:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gYear = atoi(buf);
                    break;

                case 1:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gMonth = atoi(buf);
                    break;

                case 2:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gDay = atoi(buf);
                    break;

                case 3:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gHour = atoi(buf);
                    break;

                case 4:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gMin = atoi(buf);
                    break;

                case 5:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gSec = atoi(buf);
                    break;

                case 6:
                    PalMemcpy(buf, str[i], strlen(str[i]));
                    gFileIndex = atoi(buf);
                    break;

                default:
                    break;
                }
                i++;
            }
            //dbg_msg(DBG_MSG_TYPE_INFO, "y = %d, month= %d, day =%d, hour =%d, min = %d ,sec = %d\n",
            //gYear, gMonth, gDay, gHour, gMin, gSec);

            // mmpExRtcSetDate(gYear, gMonth, gDay);
            //    mmpExRtcSetTime(gHour, gMin, gSec);
            config_set_rec_index(gFileIndex);
            mmpExRtcSetDateTime(gYear, gMonth, gDay, gHour, gMin, gSec);
            PalHeapFree(PAL_HEAP_DEFAULT, pReadFileBuffer);
            PalFileClose(pRtcFile, MMP_NULL, MMP_NULL);
            //   PalTFileDelete(filePath, MMP_NULL, MMP_NULL);
        }
    }
#endif
}

void
_DeviceModeProcess(
    void)
{
    TS_MUXER_PARAMETER   tMuxerParam      = { 0 };
    MMP_UINT8            pServiceName[32] = { 0 };
    MMP_UINT32           micCurrVol;
    CAPTURE_VIDEO_SOURCE capdev;
    MMP_UINT32           lineboost;

    if (gbEnableRecord && gbRecording)
    {
        gbRecording     = MMP_FALSE;
        gbEnableRecord  = MMP_FALSE;
        gtSystemStatus |= SYSTEM_OUT_RECORD_STATUS;
        if (gbJPEGRecord)
        {
            coreStopJPEGRecord();
            gbJPEGRecord = MMP_FALSE;
        }
        else
        {
            coreStopRecord();
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "Write Test is Done (%d)\n", __LINE__);
              #ifdef SEND_SPACE_INFO
		    _CheckUsbFreeSpace(&Gremainspace);
		    printf("record stop G space=%d,R space=%d\r\n",Gtotalspace,Gremainspace);
	    #endif
        {
            MMP_UINT32 WriteDoneClock = PalGetClock();
            while (PalGetDuration(WriteDoneClock) < WAIT_USB_WRITE_DONE_TIME)
                PalSleep(1);
        }

        coreEnableAVEngine(MMP_FALSE);
        gtSystemStatus &= ~SYSTEM_IN_RECORD_STATUS;
        gtSystemStatus &= ~SYSTEM_OUT_RECORD_STATUS;
    }

    dbg_msg(DBG_MSG_TYPE_INFO, "---- Enter PC Connected Mode ----\n");
    //_SetDeviceMode(MMP_TRUE);

    gtSystemStatus        |= SYSTEM_PC_CONNECTED_MODE;
    gtSystemStatus        &= ~STSTEM_STAND_ALONE_MODE;

    gbHDMIRXReboot         = MMP_FALSE;

    gtPCModeLed.AnalogLed  = LED_OFF;
    gtPCModeLed.HDMILed    = LED_OFF;
    gtPCModeLed.SD720PLed  = LED_OFF;
    gtPCModeLed.HD1080PLed = LED_OFF;
    gtPCModeLed.SignalLed  = LED_OFF;
    gtPCModeLed.RecordLed  = LED_OFF;

    gtLedCtrl.EnAnalog     = MMP_FALSE;
    gtLedCtrl.EnHDMI       = MMP_FALSE;
    gtLedCtrl.En720P       = MMP_FALSE;
    gtLedCtrl.En1080P      = MMP_FALSE;
    gtLedCtrl.EnRecord     = MMP_FALSE;
    gtLedCtrl.EnPower      = MMP_FALSE;
    GpioLedControl(&gtLedCtrl);
    coreStop();
    //codec_terminate();
    coreTerminate();
    PalSleep(1);
    coreInitialize(MMP_TS_MUX);
    config_get_capturedev(&capdev);
    coreSetCaptureSource(capdev);
    coreEnableAVEngine(MMP_TRUE);
    config_get_microphone_volume(&micCurrVol);
    //micCurrVol = 16; //30dB
    codec_initialize();
    config_get_line_boost(&lineboost);
    gtAudioCodec.bPCConnectedMode = MMP_TRUE;
    gtAudioCodec.bHDMIInput       = !!((coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_HDMI) || (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI));
    gtAudioCodec.bDVIMode         = !!(coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI);
    gtAudioCodec.bEnRecord        = MMP_FALSE;
    gtAudioCodec.bInsertMic       = MMP_FALSE;
    gtAudioCodec.hdmiBoostGain    = DEF_HDMIBOOSTGAIN;   //+0db
    gtAudioCodec.analogBoostGain  = DEF_ANALOGBOOSTGAIN; //+0db
    gtAudioCodec.micVolume        = DEF_MICVOLGAINDB;
    gtAudioCodec.micBoostGain     = DEF_MICBOOSTGAIN;
    _AudioCodecControl(&gtAudioCodec);

#ifdef TSO_ENABLE
    tMuxerParam.bEnableTso    = MMP_FALSE;
    tMuxerParam.bEnableEagle  = MMP_FALSE;
    tMuxerParam.constellation = CONSTELATTION_64QAM;
    tMuxerParam.codeRate      = CODE_RATE_7_8;
    tMuxerParam.guardInterval = GUARD_INTERVAL_1_32;
    tMuxerParam.frequency     = 887000;
    tMuxerParam.bandwidth     = 6000;
#endif
    sprintf(pServiceName, "AIR_CH_%d_%dM", tMuxerParam.frequency / 1000, tMuxerParam.bandwidth / 1000);
#ifdef AV_SENDER_SECURITY_MODE
    tMuxerParam.bEnableSecuirtyMode = MMP_TRUE;
#else
    tMuxerParam.bEnableSecuirtyMode = MMP_FALSE;
#endif
    tMuxerParam.bAddStuffData       = MMP_TRUE;
#ifdef MPEG_AUDIO_FOR_FY
    tMuxerParam.audioEncoderType    = MPEG_AUDIO_ENCODER;
#else
    tMuxerParam.audioEncoderType    = AAC_AUDIO_ENCODER;
#endif
    // Step 1: Removed all saved services
    coreTsRemoveServices();
    // Step 2: Specifiy the country code for NIT default setting (network_id, original_network_id, private_data_specifier_descriptor, LCN rule).
    coreTsUpdateCountryId(CORE_COUNTRY_TAIWAN);
    // Step 3: Update the modulation parameter for NIT.
    coreTsUpdateModulationParameter(tMuxerParam.frequency, tMuxerParam.bandwidth,
                                    tMuxerParam.constellation, tMuxerParam.codeRate, tMuxerParam.guardInterval);
    // Step 4: Insert desried service parameter for PMT, SDT, and NIT
    coreTsInsertService(0x100, 0x1000,
                        0x7D1, H264_VIDEO_STREAM,
#ifdef MPEG_AUDIO_FOR_FY
                        0x7D2, MPEG_AUDIO,
#else
                        0x7D2, AAC,
#endif
                        "ITE", sizeof("ITE"),
                        pServiceName, strlen(pServiceName));
    // Step 5: Notify the table setup is done, then generate the SI/PSI table including PAT, PMT, SDT, and NIT
    coreTsUpdateTable();
    coreSetMuxerParameter((void *) &tMuxerParam);
    //corePlay();
    gbDeviceMode = MMP_TRUE;
}

void
_HostModeProcess(
    void)
{
    TS_MUXER_PARAMETER   tMuxerParam      = { 0 };
    MMP_UINT8            pServiceName[32] = { 0 };
    MMP_INT32            micCurrVol;
    CAPTURE_VIDEO_SOURCE capdev;
    MMP_UINT32           lineboost;

    //_SetDeviceMode(MMP_FALSE);
    dbg_msg(DBG_MSG_TYPE_INFO, "---- Enter Stand Alone Mode ----\n");
    gtSystemStatus        &= ~SYSTEM_PC_CONNECTED_MODE;
    gtSystemStatus        |= STSTEM_STAND_ALONE_MODE;

    gbHDMIRXReboot         = MMP_FALSE;

    gtPCModeLed.AnalogLed  = LED_OFF;
    gtPCModeLed.HDMILed    = LED_OFF;
    gtPCModeLed.SD720PLed  = LED_OFF;
    gtPCModeLed.HD1080PLed = LED_OFF;
    gtPCModeLed.SignalLed  = LED_OFF;
    gtPCModeLed.RecordLed  = LED_OFF;

    gtLedCtrl.EnAnalog     = MMP_FALSE;
    gtLedCtrl.EnHDMI       = MMP_FALSE;
    gtLedCtrl.En720P       = MMP_FALSE;
    gtLedCtrl.En1080P      = MMP_FALSE;
    gtLedCtrl.EnRecord     = MMP_FALSE;
    gtLedCtrl.EnPower      = MMP_FALSE;
    GpioLedControl(&gtLedCtrl);
    coreSetRoot(MMP_FALSE);
    coreStop();
    //codec_terminate();
    coreTerminate();
    coreInitialize(MMP_MP4_MUX);
    config_get_capturedev(&capdev);
    coreSetCaptureSource(capdev);
    coreEnableAVEngine(MMP_TRUE);

    config_get_microphone_volume(&micCurrVol);
    //micCurrVol = 16; //30dB
    codec_initialize();
    config_get_line_boost(&lineboost);
    gtAudioCodec.bPCConnectedMode = MMP_FALSE;
    gtAudioCodec.bHDMIInput       = ((coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_HDMI) || (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI)) ? MMP_TRUE : MMP_FALSE;
    gtAudioCodec.bDVIMode         = (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI) ? MMP_TRUE : MMP_FALSE;
    gtAudioCodec.bEnRecord        = MMP_FALSE;
    gtAudioCodec.bInsertMic       = MMP_FALSE;
    gtAudioCodec.hdmiBoostGain    = DEF_HDMIBOOSTGAIN;   //+0db
    gtAudioCodec.analogBoostGain  = DEF_HDMIBOOSTGAIN;   //+0db
    gtAudioCodec.micVolume        = DEF_MICVOLGAINDB;
    gtAudioCodec.micBoostGain     = DEF_MICBOOSTGAIN;
    _AudioCodecControl(&gtAudioCodec);

#ifdef TSO_ENABLE
    tMuxerParam.bEnableTso    = MMP_FALSE;
    tMuxerParam.bEnableEagle  = MMP_FALSE;
    tMuxerParam.constellation = CONSTELATTION_64QAM;
    tMuxerParam.codeRate      = CODE_RATE_7_8;
    tMuxerParam.guardInterval = GUARD_INTERVAL_1_32;
    tMuxerParam.frequency     = 887000;
    tMuxerParam.bandwidth     = 6000;
#endif
    sprintf(pServiceName, "AIR_CH_%d_%dM", tMuxerParam.frequency / 1000, tMuxerParam.bandwidth / 1000);
#ifdef AV_SENDER_SECURITY_MODE
    tMuxerParam.bEnableSecuirtyMode = MMP_TRUE;
#else
    tMuxerParam.bEnableSecuirtyMode = MMP_FALSE;
#endif
    tMuxerParam.bAddStuffData       = MMP_FALSE;
#ifdef MPEG_AUDIO_FOR_FY
    tMuxerParam.audioEncoderType    = MPEG_AUDIO_ENCODER;
#else
    tMuxerParam.audioEncoderType    = AAC_AUDIO_ENCODER;
#endif
    // Step 1: Removed all saved services
    coreTsRemoveServices();
    // Step 2: Specifiy the country code for NIT default setting (network_id, original_network_id, private_data_specifier_descriptor, LCN rule).
    coreTsUpdateCountryId(CORE_COUNTRY_TAIWAN);
    // Step 3: Update the modulation parameter for NIT.
    coreTsUpdateModulationParameter(tMuxerParam.frequency, tMuxerParam.bandwidth,
                                    tMuxerParam.constellation, tMuxerParam.codeRate, tMuxerParam.guardInterval);
    // Step 4: Insert desried service parameter for PMT, SDT, and NIT
    coreTsInsertService(0x100, 0x1000,
                        0x7D1, H264_VIDEO_STREAM,
#ifdef MPEG_AUDIO_FOR_FY
                        0x7D2, MPEG_AUDIO,
#else
                        0x7D2, AAC,
#endif
                        "ITE", sizeof("ITE"),
                        pServiceName, strlen(pServiceName));
    // Step 5: Notify the table setup is done, then generate the SI/PSI table including PAT, PMT, SDT, and NIT
    coreTsUpdateTable();
    coreSetMuxerParameter((void *) &tMuxerParam);
    corePlay();
    gbDeviceMode = MMP_FALSE;
}

static void
_ProcessMsg(
    MSG_OBJECT *ptMsg)
{
    switch (ptMsg->name)
    {
    case MSG_CONFIG_STORE:
        config_store();
        break;

    case MSG_SERIAL_NUMBER_STORE:
        serial_number_store();
        break;

    case MSG_DEVICE_MODE:
        //_DEVICE_MODE_CTRL();
        _DeviceModeProcess();
        break;

    case MSG_HOST_MODE:
        //_SetDeviceMode(MMP_FALSE);
        _HostModeProcess();
        break;
    }
}

static void
_AudioCodecControl(
    AUDIO_CODEC_CONTROL *pCodec)
{
    MMP_UINT32 micCurrVol = 0, micBoostGain = 0;

    if (pCodec->bEnRecord || pCodec->bPCConnectedMode)
    {
        if (pCodec->bInsertMic)
        {
            if (pCodec->bHDMIInput && !pCodec->bDVIMode)
            {
                codec_set_device(CODEC_INPUT_ONLY_MIC, 0);
            }
            else
            {
                codec_set_device(CODEC_INPUT_MIC_LINE, 1);
            }
        }
        else //mute case
        {
            if (pCodec->bHDMIInput && !pCodec->bDVIMode)
            {
                codec_set_device(CODEC_INPUT_NONE, 0);
            }
            else
            {
                codec_set_device(CODEC_INPUT_ONLY_LINE, 1);
            }
        }
    }
    else
    {
        if (pCodec->bInsertMic)
        {
            if (pCodec->bHDMIInput && !pCodec->bDVIMode)
            {
                codec_set_device(CODEC_INPUT_ONLY_MIC, 0);
            }
            else
            {
                codec_set_device(CODEC_INPUT_MIC_LINE, 1);
            }
        }
        else //mute case
        {
            if (pCodec->bHDMIInput && !pCodec->bDVIMode)
            {
                codec_set_device(CODEC_INPUT_NONE, 0);
            }
            else
            {
                codec_set_device(CODEC_INPUT_ONLY_LINE, 1);
            }
        }
    }

    micBoostGain = pCodec->micBoostGain;

    if (pCodec->micVolume < 0)
        micCurrVol = 23 - ((MMP_UINT32)((MMP_FLOAT)(pCodec->micVolume * -1.0) / 0.75));
    else
        micCurrVol = (MMP_UINT32)((MMP_FLOAT)((MMP_FLOAT)pCodec->micVolume) / 0.75) + 23;

    //dbg_msg(DBG_MSG_TYPE_INFO, "---(%d), %d, %d %d %d %d %d---\n", pCodec->micVolume, micBoostGain, micCurrVol, pCodec->hdmiBoostGain, pCodec->analogBoostGain,
    //                                        pCodec->bInsertMic, pCodec->bEnRecord);
    //dbg_msg(DBG_MSG_TYPE_INFO, "---- %d %d  %d----\n", pCodec->bPCConnectedMode, pCodec->bDVIMode,pCodec->bHDMIInput);
    codec_set_mic_boost(micBoostGain);
    codec_set_mic_vol(micCurrVol);
    codec_set_line_boost(1, pCodec->hdmiBoostGain);
    codec_set_line_boost(2, pCodec->analogBoostGain);
}

static void
_AutoChangeDevice(
    void)
{
    MMP_BOOL          bDevSignalStable;
    static MMP_BOOL   bDevNoSignalTrigger = MMP_FALSE;
    static MMP_UINT32 keepDevNoSignalTime = 0;
    MMP_BOOL          detectChange        = MMP_FALSE;

    if (gbHDMIRXReboot)
        detectChange = MMP_FALSE;
    else if (coreGetCaptureSource() == CAPTURE_VIDEO_SOURCE_DVI)
        detectChange = MMP_FALSE;
    else if (!avSyncGetDeviceInitFinished())
        detectChange = MMP_FALSE;
    else if (gbChangeDevice)
        detectChange = MMP_FALSE;
    else if (gbEnableRecord)
        detectChange = MMP_FALSE;
    else if (gtSystemStatus & (SYSTEM_IN_RECORD_STATUS | SYSTEM_OUT_RECORD_STATUS))
        detectChange = MMP_FALSE;
    else if (!mmpHDMIRXGetProperty(HDMIRX_CHECK_ENGINE_IDLE) && mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX)
        detectChange = MMP_FALSE;
    else
        detectChange = MMP_TRUE;

    if (detectChange)
    {
        bDevSignalStable = mmpCapDeviceIsSignalStable();
        // dbg_msg(DBG_MSG_TYPE_INFO, "---%d, (%d), %d----\n", bDevSignalStable, PalGetDuration(keepDevNoSignalTime), bDevNoSignalTrigger);

        // signal stable
        if (bDevSignalStable || gbChangeDevice)
        {
            keepDevNoSignalTime = PalGetClock();
            bDevNoSignalTrigger = MMP_FALSE;
        }
        else if (!bDevSignalStable && (!bDevNoSignalTrigger))
        {
            keepDevNoSignalTime = PalGetClock();
            bDevNoSignalTrigger = MMP_TRUE;
        }

        if (PalGetDuration(keepDevNoSignalTime) >= 4000 && bDevNoSignalTrigger)
        {
            gbAutoChange        = MMP_TRUE;
            gbChangeDevice      = MMP_TRUE;
            bDevNoSignalTrigger = MMP_FALSE;
        }
    }
    else
    {
        keepDevNoSignalTime = PalGetClock();
        bDevNoSignalTrigger = MMP_FALSE;
    }

    if (gtSystemStatus & SYSTEM_SWITCH_DIGITAL_ANALOG)
    {
        gbChangeDevice = MMP_TRUE;
    }
#ifdef INPUT_SWITCH_YPBPR_CVBS
    if (/*(gtSystemStatus & SYSTEM_DIGITAL_INPUT) &&*/
        (gtSystemStatus & SYSTEM_SWITCH_DIGITAL_ANALOG) == 0)
    {
    #ifndef AUTO_SWITCH_SOURCE
        gbChangeDevice      = MMP_FALSE;
        keepDevNoSignalTime = PalGetClock();
        bDevNoSignalTrigger = MMP_FALSE;
    #endif
    }
    else if (gtSystemStatus & SYSTEM_SWITCH_DIGITAL_ANALOG)
    {
        gbChangeDevice      = MMP_TRUE;
        gbAutoChange        = MMP_FALSE;
        // printf("input switch device\r\n");
    #ifdef AUTO_SWITCH_SOURCE
        keepDevNoSignalTime = PalGetClock();
        bDevNoSignalTrigger = MMP_FALSE;
    #endif
    }
#endif
}

static void
_GetNextRecordFileName(
    char       *filename,
    MMP_UINT32 rec_resolution)
{

#ifdef EXTERNAL_RTC
    mmpExRtcGetDate(&gYear, &gMonth, &gDay);
    mmpExRtcGetTime(&gHour, &gMin, &gSec);
#endif

#if  0//kenny 20150825 def EXTERNAL_RTC
    mmpExRtcGetDate(&gYear, &gMonth, &gDay);
    mmpExRtcGetTime(&gHour, &gMin, &gSec);
    sprintf(filename, "C:/%4d%02d%02d%02d%02d_%02d.mp4", gYear, gMonth, gDay, gHour, gMin, gSec);
#else
    #define MAX_REC_INDEX 10000
    MMP_UINT32 recFileIndex = 0;
    MMP_INT    i;
	
    config_get_rec_index(&recFileIndex);
#if 1

        if (recFileIndex >= MAX_REC_INDEX)
            recFileIndex = 0;
    #ifdef REGIA_FILE_NAME
        if (recFileIndex < 1)
            recFileIndex = 1;
        switch (rec_resolution)
        {
        case 0:
        default:
            sprintf(filename, "C:/REGIA_%04d.mp4", recFileIndex);
            break;

        case 1:
            sprintf(filename, "C:/REGIA_1080P_%04d.mp4", recFileIndex);
            break;

        case 2:
            sprintf(filename, "C:/REGIA_720P_%04d.mp4", recFileIndex);
            break;
        }
    #else
        switch (rec_resolution)
        {
        case 0:
        default:
            sprintf(filename, "C:/Encode_%d.mp4", recFileIndex);
            break;

        case 1:
            sprintf(filename, "C:/Encode_1080P_%d.mp4", recFileIndex);
            break;

        case 2:
            sprintf(filename, "C:/Encode_720P_%d.mp4", recFileIndex);
            break;
        }
    #endif

        recFileIndex++;
#else
    for (i = 0; i < MAX_REC_INDEX; ++i)
    {
           //move up
        if (recFileIndex >= MAX_REC_INDEX)
            recFileIndex = 0;

    #ifdef REGIA_FILE_NAME
        if (recFileIndex < 1)
            recFileIndex = 1;
        switch (rec_resolution)
        {
        case 0:
        default:
            sprintf(filename, "C:/REGIA_%04d.mp4", recFileIndex);
            break;

        case 1:
            sprintf(filename, "C:/REGIA_1080P_%04d.mp4", recFileIndex);
            break;

        case 2:
            sprintf(filename, "C:/REGIA_720P_%04d.mp4", recFileIndex);
            break;
        }
    #else
        switch (rec_resolution)
        {
        case 0:
        default:
            sprintf(filename, "C:/Encode_%d.mp4", recFileIndex);
            break;

        case 1:
            sprintf(filename, "C:/Encode_1080P_%d.mp4", recFileIndex);
            break;

        case 2:
            sprintf(filename, "C:/Encode_720P_%d.mp4", recFileIndex);
            break;
        }
    #endif

        recFileIndex++;

        {
           MMP_WCHAR       tmpfilename[255];
	     for (i = strlen(filename) + 1; i >= 0; --i)
                        tmpfilename[i] = (MMP_WCHAR)filename[i];
		 
            PAL_FILE *fp;
            printf("filename(%s)\n", filename);
            fp = PalWFileOpen(tmpfilename, PAL_FILE_RB, MMP_NULL, MMP_NULL);//kenny 20150815
            if (!fp)
            {
                break;
            }
            else
            {
                PalFileClose(fp, MMP_NULL, MMP_NULL);
            }
        }

    }
#endif
    config_set_rec_index(recFileIndex);
#endif
}

void JPEG_Write_File(JPEG_STREAM *ptJpgStream)
{
    if (ptJpgStream->streamSize != 0)
    {
        pJPEGHeaderData = ptJpgStream->pHdrBuf;
        jpegheaderSize  = ptJpgStream->headerSize;
        pJPEGFileData   = ptJpgStream->pStreamBuf;
        jpegdataSize    = ptJpgStream->streamSize;
        //   printf("Capture data %x %d \n", ptJpgStream->pStreamBuf, ptJpgStream->streamSize);
    }
    else
    {
        printf("JPEG_Write_File : Encoding Fail\n");
    }
}

void JPEG_Write_Card(MMP_UINT8 *pHeader, MMP_UINT32 headerSize, MMP_UINT8 *pData, MMP_UINT32 dataSize)
{
    MMP_UINT8         filename[40];
    MMP_WCHAR         out_filename[40];
    static MMP_UINT32 frame_count  = 0;
    PAL_FILE          *fp          = MMP_NULL;
    MMP_INT           i;
    PAL_CLOCK_T       clock        = PalGetClock();
    MMP_UINT32        recFileIndex = 0;
    MMP_INT           volumeIndex  = 0;
    PAL_DEVICE_TYPE   devtype      = PAL_DEVICE_TYPE_USB0;

    if (gtSystemStatus & SYSTEM_SELECT_USB_STORAGE)
        devtype = PAL_DEVICE_TYPE_USB0;
    else
        devtype = PAL_DEVICE_TYPE_SD;

    if (dataSize != 0)
    {
        config_get_rec_index(&recFileIndex);
        if (recFileIndex > FileIndexMax)
            recFileIndex = 0;

        (void)strcpy(filename, "");
        (void)sprintf(filename, "C:/IMG_%d.JPG", recFileIndex);

        // printf("jpeg %d_%d %s\r\n",bbFolderIndex,bbFileIndex,filename);

        recFileIndex++;
        config_set_rec_index(recFileIndex);

        i = strlen(filename) + 1;
        for (; i >= 0; --i)
            out_filename[i] = (MMP_WCHAR)filename[i];

        volumeIndex     = storageMgrGetCardVolume(devtype, 0);
        if (volumeIndex < 0)
            printf("--- Get invalid volindex ----\n");
        out_filename[0] = PAL_T('A') + volumeIndex;

        fp              = PalWFileOpen(out_filename, PAL_FILE_WB, MMP_NULL, MMP_NULL);

        if (fp)
        {
            PalFileWrite(pHeader, 1, headerSize, fp, MMP_NULL, MMP_NULL);
            PalFileWrite(pData, 1, dataSize, fp, MMP_NULL, MMP_NULL);
        }

        if (fp)
            PalFileClose(fp, MMP_NULL, MMP_NULL);

        if (tJpgCtx.bOneShot)
        {
            //  coreStopJPEGRecord();
            gbEnableRecord = MMP_FALSE;
        }
        // printf("write data %x %d \n", pJPEGFileData, jpegdataSize);
        jpegdataSize = 0;
        printf("JPEG_%d Time %d\n", recFileIndex - 1, PalGetDuration(JPEGClock));
    }
    else
    {
        //  printf("JPEG_Write_File to card: Encoding Fail\n");
    }
}


