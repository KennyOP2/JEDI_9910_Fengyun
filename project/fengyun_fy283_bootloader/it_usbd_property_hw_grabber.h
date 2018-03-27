/*
 *  it_usbd_property_hw_grabber.h
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 */
#if !defined(__IT_USBD_PROPERTY_HW_GRABBER_H__)
#define __IT_USBD_PROPERTY_HW_GRABBER_H__

#include "usbd/inc/it_usbd_property.h"

extern unsigned int
it_hw_grabber_set_datetime(
    unsigned int    year,
    unsigned int    month,
    unsigned int    day,
    unsigned int    hour,
    unsigned int    minute,
    unsigned int    second);

extern unsigned int
it_hw_grabber_get_datetime(
    unsigned int*   p_year,
    unsigned int*   p_month,
    unsigned int*   p_day,
    unsigned int*   p_hour,
    unsigned int*   p_minute,
    unsigned int*   p_second);

extern unsigned int
it_hw_grabber_set_microphone_volume(
    unsigned int    current_value);

extern unsigned int
it_hw_grabber_get_microphone_volume(
    unsigned int*   p_current_value,
    unsigned int*   p_default_value,
    unsigned int*   p_min_value,
    unsigned int*   p_max_value);

extern unsigned int
it_hw_grabber_set_encode_bitrate(
    unsigned int    resolution,
    unsigned int    bit_rate);

extern unsigned int
it_hw_grabber_get_encode_bitrate(
    unsigned int    resolution,
    unsigned int*   p_bit_rate);

extern unsigned int
it_hw_grabber_set_pc_mode(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_led_analog(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_led_hdmi(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_led_signal(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_led_record(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_led_sd720p(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_led_hd1080P(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_set_hdcp(
    unsigned int    enable);

extern unsigned int
it_hw_grabber_get_hdcp(
    void);    

unsigned int
it_hw_grabber_capsense_read_i2c(
    unsigned char   *pData,
    unsigned int   NByte);

extern unsigned int
it_hw_grabber_capsense_write_i2c(
    unsigned char   *pData,
    unsigned int   NByte);

unsigned int
it_hw_grabber_capsense_InterruptState(
    void);

unsigned int
it_hw_grabber_capsense_readFW(
    unsigned char *pData,
    unsigned int NByte);

unsigned int
it_hw_grabber_capsense_writeFW(
    unsigned char SubAddr,
    unsigned char *pData,
    unsigned int NByte);

const void
it_hw_grabber_get_usb_serial_number(
    char* serialnum);

const void
it_hw_grabber_set_usb_serial_number(
    char* serialnum);

#if defined(__cplusplus)
class it_usbd_property_hw_grabber
{
private:
public:
    enum
    {
        SUB_ID_HW_GRABBER_VERSION               = 1,
        SUB_ID_HW_GRABBER_DATETIME,
        SUB_ID_HW_GRABBER_MICROPHONE_VOLUME,
        SUB_ID_HW_GRABBER_ENCODE_BITRATE_FULL_HD,
        SUB_ID_HW_GRABBER_ENCODE_BITRATE_HD,
        SUB_ID_HW_GRABBER_ENCODE_BITRATE_SD,
        SUB_ID_HW_GRABBER_PC_MODE,
        SUB_ID_HW_GRABBER_I2C_OP,
        SUB_ID_HW_GRABBER_SERIAL_NUMBER,
        SUB_ID_HW_GRABBER_LED_ANALOG            =0x1000001,
        SUB_ID_HW_GRABBER_LED_HDMI,
        SUB_ID_HW_GRABBER_LED_SIGNAL,
        SUB_ID_HW_GRABBER_LED_RECORD,
        SUB_ID_HW_GRABBER_LED_SD720P,
        SUB_ID_HW_GRABBER_LED_HD1080P,
        SUB_ID_HW_GRABBER_HDCP
    };

    enum {
        ENCODE_RESOLUTION_FULL_HD       = 0,
        ENCODE_RESOLUTION_HD,
        ENCODE_RESOLUTION_SD,
        ENCODE_RESOLUTION_TOTAL
    };

    typedef struct
    {
        struct {
            IT_USBD_PROPERTY_BASE;
        };
    } property;

    typedef struct
    {
        it_usbd_property_hw_grabber::property   property;
        unsigned int                            sub_id;
    } property_hw_grabber;

    typedef struct
    {
        unsigned int                            customer_code;
        unsigned int                            project_code;
        unsigned int                            sdk_major_version;
        unsigned int                            sdk_minor_version;
        unsigned int                            build_number;
        unsigned int                            api_version;
    } hw_grabber_version;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber           header;
        it_usbd_property_hw_grabber::hw_grabber_version            data;
    } property_hw_grabber_version;

    typedef struct
    {
        unsigned int                            year;
        unsigned int                            month;
        unsigned int                            day;
        unsigned int                            hour;
        unsigned int                            minute;
        unsigned int                            second;
    } hw_grabber_datetime;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber           header;
        it_usbd_property_hw_grabber::hw_grabber_datetime           data;
    } property_hw_grabber_datetime;

    typedef struct
    {
        unsigned int                            current_value;
        unsigned int                            default_value;
        unsigned int                            min_value;
        unsigned int                            max_value;
    } hw_grabber_microphone_volume;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber           header;
        it_usbd_property_hw_grabber::hw_grabber_microphone_volume  data;
    } property_hw_grabber_microphone_volume;

    typedef struct
    {
        unsigned int                            bitrate;
    } hw_grabber_encode_bitrate;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber           header;
        it_usbd_property_hw_grabber::hw_grabber_encode_bitrate     data;
    } property_hw_grabber_encode_bitrate;

    typedef struct
    {
        unsigned int                            enable;
    } hw_grabber_pc_mode;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber        header;
        it_usbd_property_hw_grabber::hw_grabber_pc_mode         data;
    } property_hw_grabber_pc_mode;

    typedef struct
    {
        unsigned int                            size;
        unsigned char                           data[512 - sizeof(it_usbd_property_hw_grabber::property_hw_grabber) - sizeof(unsigned int)];
    } hw_grabber_i2c_op;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber        header;
        it_usbd_property_hw_grabber::hw_grabber_i2c_op          data;
    } property_hw_grabber_i2c_op;

    typedef struct
    {
        char                                    data[126];
    } hw_grabber_serial_number;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber        header;
        it_usbd_property_hw_grabber::hw_grabber_serial_number   data;
    } property_hw_grabber_serial_number;

    typedef struct
    {
        unsigned int                            enable;
    } hw_grabber_led;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber        header;
        it_usbd_property_hw_grabber::hw_grabber_led             data;
    } property_hw_grabber_led;

    typedef struct
    {
        unsigned int                            enable;
    } hw_grabber_hdcp;

    typedef struct
    {
        it_usbd_property_hw_grabber::property_hw_grabber        header;
        it_usbd_property_hw_grabber::hw_grabber_hdcp            data;
    } property_hw_grabber_hdcp;

private:

public:
    static int
    do_hw_grabber_version_property(
        property_hw_grabber_version&    property_in,
        property_hw_grabber_version&    property_out);

    static int
    do_hw_grabber_datetime_property(
        property_hw_grabber_datetime&   property_in,
        property_hw_grabber_datetime&   property_out);

    static int
    do_hw_grabber_microphone_volume_property(
        property_hw_grabber_microphone_volume&  property_in,
        property_hw_grabber_microphone_volume&  property_out);

    static int
    do_hw_grabber_encode_bitrate_full_hd_property(
        property_hw_grabber_encode_bitrate& property_in,
        property_hw_grabber_encode_bitrate& property_out);

    static int
    do_hw_grabber_encode_bitrate_hd_property(
        property_hw_grabber_encode_bitrate& property_in,
        property_hw_grabber_encode_bitrate& property_out);

    static int
    do_hw_grabber_encode_bitrate_sd_property(
        property_hw_grabber_encode_bitrate& property_in,
        property_hw_grabber_encode_bitrate& property_out);

    static int
    do_hw_grabber_pc_mode_property(
        property_hw_grabber_pc_mode&    property_in,
        property_hw_grabber_pc_mode&    property_out);

    static int
    do_hw_grabber_i2c_op_property(
        property_hw_grabber_i2c_op&   property_in,
        property_hw_grabber_i2c_op&   property_out);

    static int
    do_hw_grabber_serial_number_property(
        property_hw_grabber_serial_number&  property_in,
        property_hw_grabber_serial_number&  property_out);

    static int
    do_hw_grabber_led_analog_property(
        property_hw_grabber_led&            property_in,
        property_hw_grabber_led&            property_out);

    static int
    do_hw_grabber_led_hdmi_property(
        property_hw_grabber_led&            property_in,
        property_hw_grabber_led&            property_out);

    static int
    do_hw_grabber_led_signal_property(
        property_hw_grabber_led&            property_in,
        property_hw_grabber_led&            property_out);

    static int
    do_hw_grabber_led_record_property(
        property_hw_grabber_led&            property_in,
        property_hw_grabber_led&            property_out);

    static int
    do_hw_grabber_led_sd720p_property(
        property_hw_grabber_led&            property_in,
        property_hw_grabber_led&            property_out);

    static int
    do_hw_grabber_led_hd1080p_property(
        property_hw_grabber_led&            property_in,
        property_hw_grabber_led&            property_out);

    static int
    do_hw_grabber_hdcp_property(
        property_hw_grabber_hdcp&            property_in,
        property_hw_grabber_hdcp&            property_out);    
};
#endif
#endif
