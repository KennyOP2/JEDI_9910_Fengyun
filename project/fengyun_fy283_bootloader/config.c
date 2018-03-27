#include "config.h"
#include "mmp_nor.h"
#include "msg_route.h"
#include "crc.h"
#include "mps_system.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define CONFIG_SIGNATURE                   (('I' << 24) | ('T' << 16) | ('E' << 8))
#define CONFIG_VERSION                     16
#define CONFIG_HEADER_SIZE                 16                   // sizeof(gtConfig.signature) + sizeof(gtConfig.size) + sizeof(gtConfig.crc) + sizeof(gtConfig.version);
#define NOR_SIGNLE_BLOCK_SIZE              (64 * 1024)
#define NOR_RESERVE_CONFIG_SIZE            (128 * 1024)
#define NOR_RESERVE_USB_SERIAL_NUMBER_SIZE (64 * 1024)

//=============================================================================
//                              Macro Definition
//=============================================================================
#define _CONFIG_STORE()        SendDelayedMsg(MSG_CONFIG_STORE, 500, 0, 0, 0)
#define _SERIAL_NUMBER_STORE() SendDelayedMsg(MSG_SERIAL_NUMBER_STORE, 500, 0, 0, 0)
#define _LOCK()                PalWaitMutex(gtMutex, PAL_MUTEX_INFINITE)
#define _UNLOCK()              PalReleaseMutex(gtMutex)

#define _CRC(config)                                        \
    _crc(((MMP_UINT8 *)&config.crc + sizeof(config.crc)),   \
         config.size - ((MMP_UINT32)&config.crc - (MMP_UINT32)&config + sizeof(config.crc)))

 // use a better crc calculation algorithm
#define _CRC2(config)                                           \
    CalcCRC32(((MMP_UINT8 *)&config.crc + sizeof(config.crc)),  \
              config.size - ((MMP_UINT32)&config.crc - (MMP_UINT32)&config + sizeof(config.crc)))

#ifdef CFG_DEV_TEST
    #define _CONFIG_STORE(...) config_store()
    #define SendDelayedMsg(...)
    #define trac(...)
#endif

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_MUTEX               gtMutex;
static MMP_UINT32              gConfigStoreBaseAddr;
static MMP_UINT32              gConfigStoreMaxAddr;
static MMP_UINT32              gConfigStoreAddr;
static MMP_UINT32              gSingleConfigReservedSize;
static MMP_UINT                gUsbSerialNumStoreBaseAddr;
static CONFIG                  gtConfig = {0};
static USB_SERIAL_NUMBER       gtUsbSerianlNumber = {0};
static VIDEO_ENCODER_PARAMETER gtdefaultVideoEnPara[3][VIDEO_ENCODER_INPUT_INFO_NUM];
static VIDEO_ENCODER_PARAMETER *gptVideoEnPara[3][VIDEO_ENCODER_INPUT_INFO_NUM];
static MMP_BOOL                gbApplyALL    = MMP_FALSE;

//============================================================================
//                              Private Function Declaration
//=============================================================================
void
_set_default_videncodepara(
    void);

MMP_UINT32
_crc(
    MMP_UINT8  *addr,
    MMP_UINT32 size);

MMP_RESULT
_NorWrite(
    void     *psrc,
    MMP_UINT addr,
    MMP_INT  size);

//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_UINT32
config_get_single_config_reserved_size(
    MMP_UINT32 config_size)
{
    MMP_UINT32 result = 0;
    if (0 < config_size && config_size <= NOR_SIGNLE_BLOCK_SIZE)
    {
        int i = 8;
        --config_size;
        for (; (config_size >> i) > 0; ++i)
            ;
        result = 1 << i;
    }
    return result;
}

MMP_UINT32
config_set_video_color_param(
    MMP_INT32 brightness,
    MMP_FLOAT contrast,
    MMP_INT32 hue,
    MMP_FLOAT saturation)
{
    _LOCK();
    gtConfig.brightness = brightness;
    gtConfig.contrast   = contrast;
    gtConfig.hue        = hue;
    gtConfig.saturation = saturation;
    _UNLOCK();

    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_video_color_param(
    MMP_INT32 *brightness,
    MMP_FLOAT *contrast,
    MMP_INT32 *hue,
    MMP_FLOAT *saturation)
{
    _LOCK();
    *brightness = gtConfig.brightness;
    *contrast   = gtConfig.contrast;
    *hue        = gtConfig.hue;
    *saturation = gtConfig.saturation;
    _UNLOCK();
    return MMP_SUCCESS;
}

//kenny patch 20140428
MMP_UINT32
config_set_rec_index(
    MMP_INT32 recindex)
{
    _LOCK();
    gtConfig.recFileIndex = recindex;
    _UNLOCK();

    config_store();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_rec_index(
    MMP_INT32 *recindex)
{
    _LOCK();
    *recindex = gtConfig.recFileIndex;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_microphone_volume(
    MMP_UINT32 volume)
{
    _LOCK();
    gtConfig.microphone_volume = volume;
    _UNLOCK();

    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_microphone_volume(
    MMP_UINT32 *p_volume)
{
    _LOCK();
    *p_volume = gtConfig.microphone_volume;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_encode_bitrate(
    MMP_UINT32 encode_resolution,
    MMP_UINT32 encode_bitrate)
{
    if (encode_resolution < ENCODE_RESOLUTION_TOTAL)
    {
        _LOCK();
        gtConfig.encode_bitrate[encode_resolution] = encode_bitrate;
        _UNLOCK();

        _CONFIG_STORE();
        return MMP_SUCCESS;
    }
    return MMP_RESULT_ERROR;
}

MMP_UINT32
config_get_encode_bitrate(
    MMP_UINT32 encode_resolution,
    MMP_UINT32 *p_encode_bitrate)
{
    if (encode_resolution < ENCODE_RESOLUTION_TOTAL)
    {
        _LOCK();
        *p_encode_bitrate = gtConfig.encode_bitrate[encode_resolution];
        _UNLOCK();

        return MMP_SUCCESS;
    }
    return MMP_RESULT_ERROR;
}

MMP_UINT32
config_set_mic_mute(
    MMP_UINT32 micmute)
{
    _LOCK();
    gtConfig.micmute = micmute;
    _UNLOCK();

    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_mic_mute(
    MMP_UINT32 *micmute)
{
    _LOCK();
    *micmute = gtConfig.micmute;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_line_boost(
    MMP_UINT32 lineboost)
{
    _LOCK();
    gtConfig.lineboost = lineboost;
    _UNLOCK();

    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_line_boost(
    MMP_UINT32 *lineboost)
{
    _LOCK();
    *lineboost = gtConfig.lineboost;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_digital_audio_volume(
    DIGITAL_AUDIO_VOLUME volume)
{
    _LOCK();
    gtConfig.digitalvolume = volume;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_digital_audio_volume(
    DIGITAL_AUDIO_VOLUME *volume)
{
    _LOCK();
    *volume = gtConfig.digitalvolume;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_audio_bitrate(
    MMP_UINT32 bitrate)
{
    _LOCK();
    gtConfig.audiobitrate = bitrate;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_audio_bitrate(
    MMP_UINT32 *bitrate)
{
    _LOCK();
    *bitrate = gtConfig.audiobitrate;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_aspectratio(
    MMP_UINT32 aspectratio)
{
    _LOCK();
    gtConfig.aspectratio = aspectratio;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_aspectratio(
    MMP_UINT32 *aspectratio)
{
    _LOCK();
    *aspectratio = gtConfig.aspectratio;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_vid_resolution(
    MMP_UINT32 resolution)
{
    _LOCK();
    gtConfig.vidresolution = resolution;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_vid_resolution(
    MMP_UINT32 *resolution)
{
    _LOCK();
    *resolution = gtConfig.vidresolution;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_capturedev(
    CAPTURE_VIDEO_SOURCE capturedev)
{
    _LOCK();
    gtConfig.capturedev = capturedev;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_capturedev(
    CAPTURE_VIDEO_SOURCE *capturedev)
{
    _LOCK();
    *capturedev = gtConfig.capturedev;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_hdcp_status(
    MMP_BOOL flag)
{
    _LOCK();
    gtConfig.disableHDCP = flag;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_hdcp_status(
    MMP_BOOL *flag)
{
    _LOCK();
    *flag = gtConfig.disableHDCP;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_dev_type(
    PAL_DEVICE_TYPE type)
{
    _LOCK();
    gtConfig.devicetype = type;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_dev_type(
    PAL_DEVICE_TYPE *type)
{
    _LOCK();
    *type = gtConfig.devicetype;
    _UNLOCK();
    return MMP_SUCCESS;
}

MMP_UINT32
config_set_record_resolution(
    RECORD_RESOLUTION resolution)
{
    _LOCK();
    gtConfig.resolution = resolution;
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

MMP_UINT32
config_get_record_resolution(
    RECORD_RESOLUTION *resolution)
{
    _LOCK();
    *resolution = gtConfig.resolution;
    _UNLOCK();
    return MMP_SUCCESS;
}

void
config_get_videoenpara(
    CONFIG **config)
{
    _LOCK();
    *config = &gtConfig;
    _UNLOCK();
}

MMP_UINT32
config_set_videoenpara(
    CONFIG     config,
    MMP_BOOL   is_pc_connect_mode,
    MMP_UINT32 devnum,
    MMP_UINT32 index)
{
    _LOCK();
    if (is_pc_connect_mode)
        PalMemcpy(&gtConfig.tVideoEnPara_pc[devnum][index], &config.tVideoEnPara[devnum][index], sizeof(VIDEO_ENCODER_PARAMETER));
    else
        PalMemcpy(&gtConfig.tVideoEnPara[devnum][index], &config.tVideoEnPara[devnum][index], sizeof(VIDEO_ENCODER_PARAMETER));
    _UNLOCK();
    _CONFIG_STORE();
    return MMP_SUCCESS;
}

void
config_get_default_videoenpara(
    VIDEO_ENCODER_PARAMETER **para,
    MMP_UINT32              devnum,
    MMP_UINT32              index)
{
    _LOCK();
    *para = &gtdefaultVideoEnPara[devnum][index];
    _UNLOCK();
}

void
get_usb_serial_number(
    MMP_CHAR **serialnum)
{
    _LOCK();
    *serialnum = gtUsbSerianlNumber.serial_number;
    _UNLOCK();
}

void
set_usb_serial_number(
    MMP_CHAR *serialnum)
{
    MMP_UINT len;

    if (!serialnum)
        return;
    len = strlen((MMP_CHAR *)serialnum);

    _LOCK();
    if (0 < len && len < 126)
        strcpy((MMP_CHAR *)&gtUsbSerianlNumber.serial_number[0], (MMP_CHAR *)serialnum);
    else
        gtUsbSerianlNumber.serial_number[0] = 0;
    _UNLOCK();

    _SERIAL_NUMBER_STORE();
}

MMP_UINT32
config_load()
{
    MMP_RESULT result;
    MMP_UINT32 capacity;
    MMP_UINT32 crc;

    if (MMP_NULL == gtMutex)
        gtMutex = PalCreateMutex(MMP_NULL);

    _LOCK();
    do
    {
        result = NorInitial();
        if (MMP_SUCCESS != result)
            break;

        capacity = NorCapacity();
        if ((capacity < (1024 * 1024)) || (capacity > (512 * 1024 * 1024)))
        {
            result = MMP_RESULT_ERROR;
            break;
        }
        gConfigStoreBaseAddr       = capacity - NOR_RESERVE_CONFIG_SIZE;
        gConfigStoreMaxAddr        = gConfigStoreBaseAddr + NOR_SIGNLE_BLOCK_SIZE;
        gUsbSerialNumStoreBaseAddr = capacity - NOR_RESERVE_CONFIG_SIZE - NOR_RESERVE_USB_SERIAL_NUMBER_SIZE;
        gSingleConfigReservedSize  = config_get_single_config_reserved_size(sizeof(gtConfig));

        for (gConfigStoreAddr = gConfigStoreMaxAddr - gSingleConfigReservedSize;
             gConfigStoreAddr >= gConfigStoreBaseAddr;
             gConfigStoreAddr -= gSingleConfigReservedSize)
        {
            MMP_UINT8 header[CONFIG_HEADER_SIZE];
            CONFIG    *ptConfig = (CONFIG *)&header[0];

            //printf("READ gConfigStoreAddr(0x%X)\n", gConfigStoreAddr);
            result = NorRead(&header[0], gConfigStoreAddr, sizeof(header));
            if (MMP_SUCCESS != result)
            {
                continue;
            }

            //printf("gtConfig.signature(%d) CONFIG_SIGNATURE(%d)\n", ptConfig->signature, CONFIG_SIGNATURE);
            //printf("gtConfig.size(%d) sizeof(gtConfig)(%d)\n", ptConfig->size, sizeof(gtConfig));
            //printf("gtConfig.version(%d) CONFIG_VERSION(%d)\n", ptConfig->version, CONFIG_VERSION);
            if ((CONFIG_SIGNATURE != ptConfig->signature)
                || (sizeof(gtConfig) != ptConfig->size)
                || (CONFIG_VERSION != ptConfig->version))
            {
                result = MMP_RESULT_ERROR;
                continue;
            }

            result = NorRead(&gtConfig, gConfigStoreAddr, sizeof(gtConfig));
            if (MMP_SUCCESS != result)
            {
                //trac("");
                continue;
            }

            //crc = _crc(((MMP_UINT8*)&gtConfig.crc + sizeof(gtConfig.crc)) ,
            //           gtConfig.size - ((MMP_UINT32)&gtConfig.crc - (MMP_UINT32)&gtConfig + sizeof(gtConfig.crc)));
            crc = _CRC2(gtConfig);
            //printf("READ gConfigStoreAddr(0x%X)CRC(%08X)crc(%08X)\n", gConfigStoreAddr, gtConfig.crc, crc);
            if (crc != gtConfig.crc)
            {
                //printf("gtConfig.crc(%08X) crc(%08X)\n", gtConfig.crc, crc);
                result = MMP_RESULT_ERROR;
                continue;
            }
            else
                break;
        }

        if (gConfigStoreAddr < gConfigStoreBaseAddr)
        {
            result = MMP_RESULT_ERROR;
            break;
        }
    } while (0);

    if (MMP_SUCCESS != result)
    {
            // reset to default;
            memset(&gtConfig, 0, sizeof(gtConfig));
            dbg_msg(DBG_MSG_TYPE_INFO, "----- config load reset to default -----\n");
            gtConfig.signature                                 = CONFIG_SIGNATURE;
            gtConfig.size                                      = sizeof(gtConfig);
            gtConfig.version                                   = CONFIG_VERSION;
            gtConfig.microphone_volume                         = 16;
            gtConfig.encode_bitrate[ENCODE_RESOLUTION_FULL_HD] = 16000;
            gtConfig.encode_bitrate[ENCODE_RESOLUTION_HD]      = 16000;
            gtConfig.encode_bitrate[ENCODE_RESOLUTION_SD]      = 4000;
            gtConfig.lineboost                                 = LINE_BOOST_SUB_12_DB;
            gtConfig.digitalvolume                             = DIGITAL_AUDIO_DEFAULT;
            gtConfig.aspectratio                               = 0x0;
            gtConfig.brightness                                = 0;
            gtConfig.contrast                                  = 1.0;
            gtConfig.hue                                       = 0;
            gtConfig.saturation                                = 1.0;
            gtConfig.recFileIndex                              = 0; //kenny patch 20140428
            gtConfig.capturedev                                = CAPTURE_VIDEO_SOURCE_HDMI;
            gtConfig.disableHDCP                               = 0; //default hdcp on;
            gtConfig.devicetype                                = PAL_DEVICE_TYPE_USB0;
            gtConfig.resolution                                = SD_RESOLUTION;
            _set_default_videncodepara();
        _UNLOCK();
        _CONFIG_STORE();
        return result;
    }
    else
    {
        _UNLOCK();
        return MMP_SUCCESS;
    }
}

MMP_UINT32
config_store()
{
    return 0;
}

MMP_UINT32
serial_number_load()
{
    MMP_RESULT result = 0;
    MMP_UINT32 capacity;
    MMP_UINT32 addr;
    MMP_UINT32 crc;

    _LOCK();
    do
    {
        // USB Serial Number
        result = NorRead(&gtUsbSerianlNumber, gUsbSerialNumStoreBaseAddr, sizeof(gtUsbSerianlNumber));
        if (MMP_SUCCESS != result)
            break;

        if ((CONFIG_SIGNATURE != gtUsbSerianlNumber.signature)
         || (sizeof(gtUsbSerianlNumber) != gtUsbSerianlNumber.size))
        {
            result = MMP_RESULT_ERROR;
            break;
        }

        //crc = _crc(((MMP_UINT8*)&gtUsbSerianlNumber.crc + sizeof(gtUsbSerianlNumber.crc)) ,
        //           gtUsbSerianlNumber.size - ((MMP_UINT32)&gtUsbSerianlNumber.crc - (MMP_UINT32)&gtUsbSerianlNumber + sizeof(gtUsbSerianlNumber.crc)));
        //crc = _CRC(gtUsbSerianlNumber);
        //if (crc != gtUsbSerianlNumber.crc)
        //{
        //    result = MMP_RESULT_ERROR;
        //    break;
        //}
    } while (0);

    if (MMP_SUCCESS != result)
    {
        // reset to default;
        dbg_msg(DBG_MSG_TYPE_INFO, "----- config reset to default -----\n");
        memset((void *)&gtUsbSerianlNumber, 0, sizeof(gtUsbSerianlNumber));
        gtUsbSerianlNumber.signature = CONFIG_SIGNATURE;
        gtUsbSerianlNumber.size      = sizeof(gtUsbSerianlNumber);
        _UNLOCK();
        return result;
    }
    else
    {
        _UNLOCK();
        return MMP_SUCCESS;
    }
}

MMP_UINT32
serial_number_store()
{
    MMP_UINT32 result;

    _LOCK();

    if (gUsbSerialNumStoreBaseAddr > 0)
    {
        gtUsbSerianlNumber.crc = _CRC(gtUsbSerianlNumber);
        result                 = NorWrite(&gtUsbSerianlNumber, gUsbSerialNumStoreBaseAddr, sizeof(gtUsbSerianlNumber));
    }

    _UNLOCK();
    return result;
}

MMP_UINT32
load_default_encodepara(void)
{
    MMP_UINT32 i, j;
    MMP_UINT32 index;

    index = VIDEO_ENCODER_INPUT_INFO_640X480_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 640;
        gtdefaultVideoEnPara[i][index].EnHeight        = 480;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 2600;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_720X480_59I
    index                                          = VIDEO_ENCODER_INPUT_INFO_720X480_59I;
    // CVBS
    gtdefaultVideoEnPara[0][index].EnWidth         = 720;
    gtdefaultVideoEnPara[0][index].EnHeight        = 480;
    gtdefaultVideoEnPara[0][index].EnBitrate       = 4000;
    gtdefaultVideoEnPara[0][index].EnDeinterlaceOn = MMP_TRUE;
    gtdefaultVideoEnPara[0][index].EnFrameDouble   = MMP_TRUE;
    gtdefaultVideoEnPara[0][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
    gtdefaultVideoEnPara[0][index].EnFrameRate     = 60;
    gtdefaultVideoEnPara[0][index].EnGOPSize       = 120;
    gtdefaultVideoEnPara[0][index].EnAspectRatio   = AR_FULL;
    // YPbPr & HDMI
    for (i = 1; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 720;
        gtdefaultVideoEnPara[i][index].EnHeight        = 480;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 3000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_720X480_59P
    index = VIDEO_ENCODER_INPUT_INFO_720X480_59P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 720;
        gtdefaultVideoEnPara[i][index].EnHeight        = 480;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 4000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_720X480_60I
    index                                          = VIDEO_ENCODER_INPUT_INFO_720X480_60I;
    //CVBS
    gtdefaultVideoEnPara[0][index].EnWidth         = 720;
    gtdefaultVideoEnPara[0][index].EnHeight        = 480;
    gtdefaultVideoEnPara[0][index].EnBitrate       = 4000;
    gtdefaultVideoEnPara[0][index].EnDeinterlaceOn = MMP_TRUE;
    gtdefaultVideoEnPara[0][index].EnFrameDouble   = MMP_TRUE;
    gtdefaultVideoEnPara[0][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
    gtdefaultVideoEnPara[0][index].EnFrameRate     = 60;
    gtdefaultVideoEnPara[0][index].EnGOPSize       = 120;
    gtdefaultVideoEnPara[0][index].EnAspectRatio   = AR_FULL;
    //YPbPr & HDMI
    for (i = 1; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 720;
        gtdefaultVideoEnPara[i][index].EnHeight        = 480;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 3000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_720X480_60P
    index = VIDEO_ENCODER_INPUT_INFO_720X480_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 720;
        gtdefaultVideoEnPara[i][index].EnHeight        = 480;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 4000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_720X576_50I
    index                                          = VIDEO_ENCODER_INPUT_INFO_720X576_50I;
    //CVBS
    gtdefaultVideoEnPara[0][index].EnWidth         = 720;
    gtdefaultVideoEnPara[0][index].EnHeight        = 576;
    gtdefaultVideoEnPara[0][index].EnBitrate       = 4000;
    gtdefaultVideoEnPara[0][index].EnDeinterlaceOn = MMP_TRUE;
    gtdefaultVideoEnPara[0][index].EnFrameDouble   = MMP_TRUE;
    gtdefaultVideoEnPara[0][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
    gtdefaultVideoEnPara[0][index].EnFrameRate     = 50;
    gtdefaultVideoEnPara[0][index].EnGOPSize       = 100;
    gtdefaultVideoEnPara[0][index].EnAspectRatio   = AR_FULL;
    //YPbPr & HDMI
    for (i = 1; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 720;
        gtdefaultVideoEnPara[i][index].EnHeight        = 576;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 3000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 25;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 50;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_720X576_50P
    index = VIDEO_ENCODER_INPUT_INFO_720X576_50P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 720;
        gtdefaultVideoEnPara[i][index].EnHeight        = 576;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 4000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 50;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 100;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X720_50P
    index = VIDEO_ENCODER_INPUT_INFO_1280X720_50P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 720;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 50;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 100;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X720_59P
    index = VIDEO_ENCODER_INPUT_INFO_1280X720_59P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 720;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X720_60P
    index = VIDEO_ENCODER_INPUT_INFO_1280X720_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 720;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_23P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_23P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 24;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 48;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_24P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_24P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 24;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 48;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_25P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_25P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 25;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 50;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_29P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_29P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 30;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_30P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_30P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 30;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_50I
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_50I;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 25;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 50;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_50P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_50P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 25;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 50;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_59I
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_59I;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_59P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_59P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_60I
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_60I;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_TRUE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1920X1080_60P
    index = VIDEO_ENCODER_INPUT_INFO_1920X1080_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_800X600_60P
    index = VIDEO_ENCODER_INPUT_INFO_800X600_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 800;
        gtdefaultVideoEnPara[i][index].EnHeight        = 600;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 5000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1024X768_60P
    index = VIDEO_ENCODER_INPUT_INFO_1024X768_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1024;
        gtdefaultVideoEnPara[i][index].EnHeight        = 768;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X768_60P
    index = VIDEO_ENCODER_INPUT_INFO_1280X768_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 768;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X800_60P
    index = VIDEO_ENCODER_INPUT_INFO_1280X800_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 800;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X960_60P
    index = VIDEO_ENCODER_INPUT_INFO_1280X960_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 960;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1280X1024_60P
    index = VIDEO_ENCODER_INPUT_INFO_1280X1024_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1280;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1024;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1360X768_60P
    index = VIDEO_ENCODER_INPUT_INFO_1360X768_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1360;
        gtdefaultVideoEnPara[i][index].EnHeight        = 768;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1366X768_60P
    index = VIDEO_ENCODER_INPUT_INFO_1366X768_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1366;  //kenny patch 1360
        gtdefaultVideoEnPara[i][index].EnHeight        = 768;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000; //kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 60;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 120;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1440X900_60P
    index = VIDEO_ENCODER_INPUT_INFO_1440X900_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1440;
        gtdefaultVideoEnPara[i][index].EnHeight        = 900;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1400X1050_60P
    index = VIDEO_ENCODER_INPUT_INFO_1400X1050_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1400;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1050;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1440X1050_60P
    index = VIDEO_ENCODER_INPUT_INFO_1440X1050_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1440;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1050;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1600X900_60P
    index = VIDEO_ENCODER_INPUT_INFO_1600X900_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1600;
        gtdefaultVideoEnPara[i][index].EnHeight        = 900;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1600X1200_60P
    index = VIDEO_ENCODER_INPUT_INFO_1600X1200_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1600;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1200;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_1680X1050_60P
    index = VIDEO_ENCODER_INPUT_INFO_1680X1050_60P;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1680;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1050;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_SKIP_BY_TWO;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    //VIDEO_ENCODER_INPUT_INFO_ALL
    index = VIDEO_ENCODER_INPUT_INFO_ALL;
    for (i = 0; i < 3; i++)
    {
        gtdefaultVideoEnPara[i][index].EnWidth         = 1920;
        gtdefaultVideoEnPara[i][index].EnHeight        = 1080;
        gtdefaultVideoEnPara[i][index].EnBitrate       = 16000;//kenny test 1800
        gtdefaultVideoEnPara[i][index].EnDeinterlaceOn = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnFrameDouble   = MMP_FALSE;
        gtdefaultVideoEnPara[i][index].EnSkipMode      = VIDEO_ENCODER_NO_DROP;
        gtdefaultVideoEnPara[i][index].EnFrameRate     = 30;
        gtdefaultVideoEnPara[i][index].EnGOPSize       = 60;
        gtdefaultVideoEnPara[i][index].EnAspectRatio   = AR_FULL;
    }

    return MMP_SUCCESS;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
void
_set_default_videncodepara(
    void)
{
    //MMP_UINT32 i, j;
    PalMemcpy(&gtConfig.tVideoEnPara, &gtdefaultVideoEnPara, sizeof(gtdefaultVideoEnPara));
    PalMemcpy(&gtConfig.tVideoEnPara_pc, &gtdefaultVideoEnPara, sizeof(gtdefaultVideoEnPara));
    gtConfig.applyall = MMP_FALSE;

    /*
        for (i=0; i < 3; i++)
        {
            for (j=0; j < VIDEO_ENCODER_INPUT_INFO_NUM; j++)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnWidth = %d\n", i,j,gtConfig.tVideoEnPara[i][j].EnWidth);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnHeight = %d\n", i,j,gtConfig.tVideoEnPara[i][j].EnHeight);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnBitrate = %d\n",i,j,gtConfig.tVideoEnPara[i][j].EnBitrate);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnDeinterlaceOn=%d\n", i,j,gtConfig.tVideoEnPara[i][j].EnDeinterlaceOn);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnFrameDouble=%d\n",i,j,gtConfig.tVideoEnPara[i][j].EnFrameDouble);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnSkipModes=%d\n", i,j,gtConfig.tVideoEnPara[i][j].EnSkipMode);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnFrameRate=%d\n",i,j,gtConfig.tVideoEnPara[i][j].EnFrameRate);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnGOPSize=%d\n",  i,j,gtConfig.tVideoEnPara[i][j].EnGOPSize);
                dbg_msg(DBG_MSG_TYPE_INFO, "tNorConfig->tVideoEnPara[%d][%d].EnAspectRatio=%d\n",i,j,gtConfig.tVideoEnPara[i][j].EnAspectRatio);
             }
        }
        while(1);
     */
}

MMP_UINT32
_crc(
    MMP_UINT8  *addr,
    MMP_UINT32 size)
{
    MMP_UINT32 result = 0;
    MMP_UINT32 i;

    for (i = 0; i < size; ++i)
    {
        result += *(addr + i);
    }
    return result;
}

MMP_RESULT
_NorWrite(
    void *psrc,
    MMP_UINT addr,
    MMP_INT size)
{
    MMP_RESULT result = MMP_RESULT_ERROR;
    if ((addr > 0)
        && (gConfigStoreBaseAddr <= addr) && (addr < gConfigStoreMaxAddr))
    {
        if ((addr & (~(NOR_SIGNLE_BLOCK_SIZE - 1))) == addr)
            result = NorWrite(psrc, addr, size);
        else
            result = NorWriteWithoutErase(psrc, addr, size);
    }
    return result;
}