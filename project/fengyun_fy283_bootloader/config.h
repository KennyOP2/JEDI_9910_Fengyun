#include "mmp_types.h"
#include "../../core/core_interface.h"
#include "pal/file.h"

#ifndef CONFIG_H
    #define CONFIG_H

    #ifdef __cplusplus
extern "C" {
    #endif

enum {
    ENCODE_RESOLUTION_FULL_HD = 0,
    ENCODE_RESOLUTION_HD,
    ENCODE_RESOLUTION_SD,
    ENCODE_RESOLUTION_TOTAL
};

typedef enum RECORD_RESOLUTION_TAG
{
    UNKNOWN,
    SD_RESOLUTION,
    FULL_HD_RESOLUTION,
} RECORD_RESOLUTION;

typedef struct {
    MMP_UINT32              signature;
    MMP_UINT32              size;
    MMP_UINT32              crc;
    MMP_UINT32              version;
    MMP_UINT32              microphone_volume;
    MMP_UINT32              encode_bitrate[ENCODE_RESOLUTION_TOTAL];
    MMP_UINT32              micmute;       // microphone mute
    MMP_UINT32              lineboost;     //microphone boost
    MMP_UINT32              vidresolution; //video resolution
    MMP_UINT32              aspectratio;
    MMP_UINT32              audiobitrate;
    CAPTURE_VIDEO_SOURCE    capturedev;
    MMP_BOOL                applyall;
    MMP_BOOL                disableHDCP;
    VIDEO_ENCODER_PARAMETER tVideoEnPara[3][VIDEO_ENCODER_INPUT_INFO_NUM];
    VIDEO_ENCODER_PARAMETER tVideoEnPara_pc[3][VIDEO_ENCODER_INPUT_INFO_NUM];
    MMP_INT32               brightness;
    MMP_FLOAT               contrast;
    MMP_INT32               hue;
    MMP_FLOAT               saturation;
    MMP_UINT32              recFileIndex;//kenny patch 20140428
    DIGITAL_AUDIO_VOLUME    digitalvolume;
    PAL_DEVICE_TYPE         devicetype;
    RECORD_RESOLUTION       resolution;
} CONFIG;

typedef struct {
    MMP_UINT32 signature;
    MMP_UINT32 size;
    MMP_UINT32 crc;
    MMP_UINT8  serial_number[128];
} USB_SERIAL_NUMBER;

extern MMP_UINT32
config_set_video_color_param(
    MMP_INT32 brightness,
    MMP_FLOAT contrast,
    MMP_INT32 hue,
    MMP_FLOAT saturation);

extern MMP_UINT32
config_get_video_color_param(
    MMP_INT32 *brightness,
    MMP_FLOAT *contrast,
    MMP_INT32 *hue,
    MMP_FLOAT *saturation);

extern MMP_UINT32
config_set_microphone_volume(
    MMP_UINT32 volume);

extern MMP_UINT32
config_get_microphone_volume(
    MMP_UINT32 *p_volume);

/*
   extern MMP_UINT32
   config_set_microphone_vol(
    MMP_UINT32  volume);

   extern MMP_UINT32
   config_get_microphone_vol(
    MMP_UINT32* p_volume);
 */
extern MMP_UINT32
config_set_encode_bitrate(
    MMP_UINT32 encode_resolution,
    MMP_UINT32 encode_bitrate);

extern MMP_UINT32
config_get_encode_bitrate(
    MMP_UINT32 encode_resolution,
    MMP_UINT32 *p_encode_bitrate);

extern MMP_UINT32
config_set_mic_mute(
    MMP_UINT32 micmute);

extern MMP_UINT32
config_get_mic_mute(
    MMP_UINT32 *micmute);

extern MMP_UINT32
config_set_line_boost(
    MMP_UINT32 lineboost);

extern MMP_UINT32
config_get_line_boost(
    MMP_UINT32 *lineboost);

extern MMP_UINT32
config_set_audio_bitrate(
    MMP_UINT32 bitrate);

extern MMP_UINT32
config_get_audio_bitrate(
    MMP_UINT32 *bitrate);

extern MMP_UINT32
config_set_aspectratio(
    MMP_UINT32 aspectratio);

extern MMP_UINT32
config_get_aspectratio(
    MMP_UINT32 *aspectratio);

extern MMP_UINT32
config_set_vid_resolution(
    MMP_UINT32 resolution);

extern MMP_UINT32
config_get_vid_resolution(
    MMP_UINT32 *resolution);

extern void
config_get_videoenpara(
    CONFIG **);

extern MMP_UINT32
config_set_videoenpara(
    CONFIG     config,
    MMP_BOOL   is_pc_connect_mode,
    MMP_UINT32 devnum,
    MMP_UINT32 index);

extern MMP_UINT32
config_set_capturedev(
    CAPTURE_VIDEO_SOURCE capturedev);

extern MMP_UINT32
config_get_capturedev(
    CAPTURE_VIDEO_SOURCE *capturedev);

extern MMP_UINT32
config_set_hdcp_status(
    MMP_BOOL flag);

MMP_UINT32
config_get_hdcp_status(
    MMP_BOOL *flag);

extern MMP_UINT32
config_set_dev_type(
    PAL_DEVICE_TYPE type);

extern MMP_UINT32
config_get_dev_type(
    PAL_DEVICE_TYPE *type);

extern MMP_UINT32
config_set_record_resolution(
    RECORD_RESOLUTION resolution);

extern MMP_UINT32
config_get_record_resolution(
    RECORD_RESOLUTION *resolution);

extern void
config_get_default_videoenpara(
    VIDEO_ENCODER_PARAMETER **para,
    MMP_UINT32              devnum,
    MMP_UINT32              index);

extern MMP_UINT32
config_set_digital_audio_volume(
    DIGITAL_AUDIO_VOLUME volume);

extern MMP_UINT32
config_get_digital_audio_volume(
    DIGITAL_AUDIO_VOLUME *volume);

extern void
get_usb_serial_number(
    MMP_CHAR **);

extern void
set_usb_serial_number(
    MMP_CHAR *serialnum);

extern MMP_UINT32
load_default_encodepara(
    void);

extern MMP_UINT32
config_load();

extern MMP_UINT32
config_store();

extern MMP_UINT32
serial_number_load();

extern MMP_UINT32
serial_number_store();

    #ifdef __cplusplus
}
    #endif

#endif