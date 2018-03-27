#include "usbd/inc/it_usbd.h"
#include "core_interface.h"
#include "config.h"

unsigned int
it_hw_grabber_get_datetime(
    unsigned int *p_year,
    unsigned int *p_month,
    unsigned int *p_day,
    unsigned int *p_hour,
    unsigned int *p_minute,
    unsigned int *p_second)
{
    return coreRtcGetDateTime(p_year, p_month, p_day, p_hour, p_minute, p_second);
}

unsigned int
it_hw_grabber_set_datetime(
    unsigned int year,
    unsigned int month,
    unsigned int day,
    unsigned int hour,
    unsigned int minute,
    unsigned int second)
{
    coreRtcSetDateTime(year, month, day, hour, minute, second);
    return 0;
}

unsigned int
it_hw_grabber_set_microphone_volume(
    unsigned int current_value)
{
    //coreSetMicInVolStep((MMP_UINT32)current_value);
    config_set_microphone_volume(current_value);
    return 0;
}

unsigned int
it_hw_grabber_get_microphone_volume(
    unsigned int *p_current_value,
    unsigned int *p_default_value,
    unsigned int *p_min_value,
    unsigned int *p_max_value)
{
    config_get_microphone_volume((MMP_UINT32 *)p_current_value);
    *p_default_value = 44;
    *p_min_value     = 0;
    *p_max_value     = 63;
    return 0;
}

unsigned int
it_hw_grabber_set_encode_bitrate(
    unsigned int resolution,
    unsigned int bit_rate)
{
    config_set_encode_bitrate((MMP_UINT32)resolution, (MMP_UINT32)bit_rate);
    return 0;
}

unsigned int
it_hw_grabber_get_encode_bitrate(
    unsigned int resolution,
    unsigned int *p_bit_rate)
{
    config_get_encode_bitrate((MMP_UINT32)resolution, (MMP_UINT32 *)p_bit_rate);
    return 0;
}

unsigned int
it_hw_grabber_set_pc_mode(
    unsigned int enable)
{
    //printf("enable=%d\n", enable);
    coreSetHWGrabberPcMode(enable);
    return 0;
}

unsigned int
it_hw_grabber_capsense_read_i2c(
    unsigned char *pData,
    unsigned int  NByte)
{
    return coreCapsenseReadI2C(pData, NByte);
}

unsigned int
it_hw_grabber_capsense_write_i2c(
    unsigned char *pData,
    unsigned int  NByte)
{
    return coreCapsenseWriteI2C(pData, NByte);
}

unsigned int
it_hw_grabber_capsense_InterruptState(
    void)
{
    return coreCapsenseInterruptState();
}

unsigned int
it_hw_grabber_capsense_readFW(
    unsigned char *pData,
    unsigned int  NByte)
{
    return coreCapsenseReadFW(pData, NByte);
}

unsigned int
it_hw_grabber_capsense_writeFW(
    unsigned char SubAddr,
    unsigned char *pData,
    unsigned int  NByte)
{
    return coreCapsenseWriteFW(SubAddr, pData, NByte);
}

const void
it_hw_grabber_get_usb_serial_number(
    char *serialnum)
{
    coreGetUsbSerialNumber(serialnum);
}

const void
it_hw_grabber_set_usb_serial_number(
    char *serialnum)
{
    coreSetUsbSerialNumber(serialnum);
}

unsigned int
it_hw_grabber_set_led_analog(
    unsigned int enable)
{
    coreSetAnalogLed((LED_STATUS) !enable);
    return 0;
}

unsigned int
it_hw_grabber_set_led_hdmi(
    unsigned int enable)
{
    coreSetHDMILed((LED_STATUS) !enable);
    return 0;
}

unsigned int
it_hw_grabber_set_led_signal(
    unsigned int enable)
{
    coreSetSignalLed((LED_STATUS) !enable);
    return 0;
}

unsigned int
it_hw_grabber_set_led_record(
    unsigned int enable)
{
    coreSetRecordLed((LED_STATUS) !enable);
    return 0;
}

unsigned int
it_hw_grabber_set_led_sd720p(
    unsigned int enable)
{
    coreSetSD720PLed((LED_STATUS) !enable);
    return 0;
}

unsigned int
it_hw_grabber_set_led_hd1080P(
    unsigned int enable)
{
    coreSetHD1080PLed((LED_STATUS) !enable);
    return 0;
}

unsigned int
it_hw_grabber_set_hdcp(
    unsigned int enable)
{
    coreDisableHDCP((MMP_BOOL) !enable);
    return 0;
}

unsigned int
it_hw_grabber_get_hdcp(
    void)
{
    return coreIsDisableHDCP();
}