/*
 *  it_usbd_property_hw_grabber.cpp
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mmp_types.h"
#include "mmp_util.h"
#include "usbd/inc/it_usbd.h"
#include "usbd/inc/it_usbd_property.h"
#include "it_usbd_property_hw_grabber.h"
#include "pal/pal.h"

int
it_usbd_property::is_hw_grabber_enabled()
{
    return 1;
}

int
it_usbd_property::do_hw_grabber_property(
    it_usbd_property::property&     _property_in,
    it_usbd_property::property&     _property_out)
{
    it_usbd_property_hw_grabber::property_hw_grabber& property_in((it_usbd_property_hw_grabber::property_hw_grabber&)_property_in);
    it_usbd_property_hw_grabber::property_hw_grabber& property_out((it_usbd_property_hw_grabber::property_hw_grabber&)_property_out);
    int                             status = 0;
    unsigned int                    flags;
    unsigned int                    sub_id;

    sub_id = le32_to_cpu(property_in.sub_id);
    property_out.property.size     = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber));
    property_out.property.id       = property_in.property.id;
    property_out.property.flags  = property_in.property.flags;
    property_out.sub_id          = property_in.sub_id;

    printf("%s(%d)\n", __FUNCTION__, __LINE__);
        printf("      id:    0X%08X\n", le32_to_cpu(property_in.property.id));
        printf("      flags: 0X%08X\n", le32_to_cpu(property_in.property.flags));
        printf("      size:  %d\n",     le32_to_cpu(property_in.property.size));
        printf("      sub_id:%d\n",     le32_to_cpu(property_in.sub_id));
    switch (sub_id)
    {
    default:
l_unhandled:
        printf("[X] Unknown property!\n");
        printf("      id:    0X%08X\n", le32_to_cpu(property_in.property.id));
        printf("      flags: 0X%08X\n", le32_to_cpu(property_in.property.flags));
        printf("      size:  %d\n",     le32_to_cpu(property_in.property.size));
        printf("      sub_id:%d\n",     le32_to_cpu(property_in.sub_id));
        status = -1;
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_VERSION:
        status = it_usbd_property_hw_grabber::do_hw_grabber_version_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_version&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_version&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_DATETIME:
        status = it_usbd_property_hw_grabber::do_hw_grabber_datetime_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_datetime&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_datetime&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_MICROPHONE_VOLUME:
        status = it_usbd_property_hw_grabber::do_hw_grabber_microphone_volume_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_microphone_volume&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_microphone_volume&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_ENCODE_BITRATE_FULL_HD:
        status = it_usbd_property_hw_grabber::do_hw_grabber_encode_bitrate_full_hd_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_ENCODE_BITRATE_HD:
        status = it_usbd_property_hw_grabber::do_hw_grabber_encode_bitrate_hd_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_ENCODE_BITRATE_SD:
        status = it_usbd_property_hw_grabber::do_hw_grabber_encode_bitrate_sd_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_PC_MODE:
        status = it_usbd_property_hw_grabber::do_hw_grabber_pc_mode_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_pc_mode&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_pc_mode&)property_out);
        break;

    case it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_I2C_OP:
        status = it_usbd_property_hw_grabber::do_hw_grabber_i2c_op_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_i2c_op&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_i2c_op&)property_out);
        break;

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_SERIAL_NUMBER:
        status = it_usbd_property_hw_grabber::do_hw_grabber_serial_number_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_serial_number&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_serial_number&)property_out);
        break;
		
    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_LED_ANALOG:
        status = it_usbd_property_hw_grabber::do_hw_grabber_led_analog_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_out);
        break;

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_LED_HDMI:
        status = it_usbd_property_hw_grabber::do_hw_grabber_led_hdmi_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_out);
        break;

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_LED_SIGNAL:
        status = it_usbd_property_hw_grabber::do_hw_grabber_led_signal_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_out);
        break;

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_LED_RECORD:
        status = it_usbd_property_hw_grabber::do_hw_grabber_led_record_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_out);
        break;		

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_LED_SD720P:
        status = it_usbd_property_hw_grabber::do_hw_grabber_led_sd720p_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_out);
        break;

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_LED_HD1080P:
        status = it_usbd_property_hw_grabber::do_hw_grabber_led_hd1080p_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_led&)property_out);
        break;		

    case  it_usbd_property_hw_grabber::SUB_ID_HW_GRABBER_HDCP:
        status = it_usbd_property_hw_grabber::do_hw_grabber_hdcp_property(
            (it_usbd_property_hw_grabber::property_hw_grabber_hdcp&)property_in,
            (it_usbd_property_hw_grabber::property_hw_grabber_hdcp&)property_out);
        break;		
    }

    property_out.property.status   = cpu_to_le32((unsigned int)status);
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_version_property(
    property_hw_grabber_version&    property_in,
    property_hw_grabber_version&    property_out)
{
    int             status = -1;
    unsigned int    flags = le32_to_cpu(property_in.header.property.flags);

    if (!(flags & it_usbd_property::FLAG_GET))
    {
        goto l_exit;
    }

    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_version));
    property_out.data.customer_code     = cpu_to_le32(CUSTOMER_CODE);
    property_out.data.project_code      = cpu_to_le32(PROJECT_CODE);
    property_out.data.sdk_major_version = cpu_to_le32(SDK_MAJOR_VERSION);
    property_out.data.sdk_minor_version = cpu_to_le32(SDK_MINOR_VERSION);
    property_out.data.build_number      = cpu_to_le32(BUILD_NUMBER);
    property_out.data.api_version       = cpu_to_le32(1);

    printf("property.size(%d) %d:(%d).(%d).(%d).(%d).(%d)\n",
        sizeof(it_usbd_property_hw_grabber::property_hw_grabber_version),
        1,
        CUSTOMER_CODE,
        PROJECT_CODE,
        SDK_MAJOR_VERSION,
        SDK_MINOR_VERSION,
        BUILD_NUMBER);
    status = 0;

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_datetime_property(
    property_hw_grabber_datetime&   property_in,
    property_hw_grabber_datetime&   property_out)
{
    int             status = -1;
    unsigned int    flags = le32_to_cpu(property_in.header.property.flags);

    static unsigned int year     = 0;
    static unsigned int month    = 0;
    static unsigned int day      = 0;
    static unsigned int hour     = 0;
    static unsigned int minute   = 0;
    static unsigned int second   = 0;

    if (flags & it_usbd_property::FLAG_GET)
    {
        status = it_hw_grabber_get_datetime(
            &year, &month, &day, &hour, &minute, &second);
        property_out.header.property.size  = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_datetime));
        property_out.data.year      = cpu_to_le32(year);
        property_out.data.month     = cpu_to_le32(month);
        property_out.data.day       = cpu_to_le32(day);
        property_out.data.hour      = cpu_to_le32(hour);
        property_out.data.minute    = cpu_to_le32(minute);
        property_out.data.second    = cpu_to_le32(second);
    }
    else if (flags & it_usbd_property::FLAG_SET)
    {
        //printf("%s(%d)\n", __FUNCTION__, __LINE__);
        year    = le32_to_cpu(property_in.data.year);
        month   = le32_to_cpu(property_in.data.month);
        day     = le32_to_cpu(property_in.data.day);
        hour    = le32_to_cpu(property_in.data.hour);
        minute  = le32_to_cpu(property_in.data.minute);
        second  = le32_to_cpu(property_in.data.second);

        //coreRtcSetDateTime(year, month, day, hour, minute, second);
        it_hw_grabber_set_datetime(year, month, day, hour, minute, second);
        printf("year:   %d\n", year);
        printf("month:  %d\n", month);
        printf("day:    %d\n", day);
        printf("hour:   %d\n", hour);
        printf("minute: %d\n", minute);
        printf("second: %d\n", second);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_microphone_volume_property(
    property_hw_grabber_microphone_volume&  property_in,
    property_hw_grabber_microphone_volume&  property_out)
{
    int             status = -1;
    unsigned int    flags = le32_to_cpu(property_in.header.property.flags);

    unsigned int current_value   = 0;
    unsigned int default_value   = 0;
    unsigned int min_value       = 0;
    unsigned int max_value       = 0;

    if (flags & it_usbd_property::FLAG_GET)
    {
        it_hw_grabber_get_microphone_volume(&current_value,
                                            &default_value,
                                            &min_value,
                                            &max_value);
        property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_microphone_volume));
        property_out.data.current_value     = cpu_to_le32(current_value);
        property_out.data.default_value     = cpu_to_le32(default_value);
        property_out.data.min_value         = cpu_to_le32(min_value);
        property_out.data.max_value         = cpu_to_le32(max_value);
        status = 0;
    }
    else if (flags & it_usbd_property::FLAG_SET)
    {
        //printf("%s(%d)\n", __FUNCTION__, __LINE__);
        current_value   = le32_to_cpu(property_in.data.current_value);
        it_hw_grabber_set_microphone_volume(current_value);

        printf("current_value:  %d\n", current_value);
        //printf("default_value:  %d\n", default_value);
        //printf("min_value:      %d\n", min_value);
        //printf("max_value:      %d\n", max_value);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_encode_bitrate_full_hd_property(
    property_hw_grabber_encode_bitrate& property_in,
    property_hw_grabber_encode_bitrate& property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);
    unsigned int    resolution  = ENCODE_RESOLUTION_FULL_HD;

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_GET)
    {
        unsigned int    bitrate = 0;
        it_hw_grabber_get_encode_bitrate(resolution, &bitrate);
        property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate));
        property_out.data.bitrate           = cpu_to_le32(bitrate);
        printf("size(%d) bitrate(%d)\n", sizeof(it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate), bitrate);
        status = 0;
    }
    else if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int bitrate = le32_to_cpu(property_in.data.bitrate);
        it_hw_grabber_set_encode_bitrate(resolution, bitrate);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_encode_bitrate_hd_property(
    property_hw_grabber_encode_bitrate& property_in,
    property_hw_grabber_encode_bitrate& property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);
    unsigned int    resolution  = ENCODE_RESOLUTION_HD;

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_GET)
    {
        unsigned int    bitrate = 0;
        it_hw_grabber_get_encode_bitrate(resolution, &bitrate);
        property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate));
        property_out.data.bitrate           = cpu_to_le32(bitrate);
        printf("bitrate(%d)\n", bitrate);
        status = 0;
    }
    else if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int bitrate = le32_to_cpu(property_in.data.bitrate);
        it_hw_grabber_set_encode_bitrate(resolution, bitrate);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_encode_bitrate_sd_property(
    property_hw_grabber_encode_bitrate& property_in,
    property_hw_grabber_encode_bitrate& property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);
    unsigned int    resolution  = ENCODE_RESOLUTION_SD;

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_GET)
    {
        unsigned int    bitrate = 0;
        it_hw_grabber_get_encode_bitrate(resolution, &bitrate);
        property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_encode_bitrate));
        property_out.data.bitrate           = cpu_to_le32(bitrate);
        printf("bitrate(%d)\n", bitrate);
        status = 0;
    }
    else if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int bitrate = le32_to_cpu(property_in.data.bitrate);
        it_hw_grabber_set_encode_bitrate(resolution, bitrate);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_pc_mode_property(
    property_hw_grabber_pc_mode&    property_in,
    property_hw_grabber_pc_mode&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_pc_mode(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_i2c_op_property(
    property_hw_grabber_i2c_op&    property_in,
    property_hw_grabber_i2c_op&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);
    unsigned char   statusbuf[14];
    MMP_UINT16      buildno;
    //unsigned long    NByte;
    int i;
    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)//IIC Write
    {
        static MMP_UINT32  index = 0;
        MMP_UINT8   ctrlBuffer[14][6] = {
        {0xa5, 0x01, 0x00, 0x00, 0x00, 0x00},
        {0xa5, 0x03, 0x00, 0x00, 0x00, 0x00},
        {0xa5, 0x05, 0x00, 0x00, 0x00, 0x00},
        {0xa5, 0x09, 0x00, 0x00, 0x00, 0x00},
        {0xa5, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xa5, 0x00, 0x00, 0x00, 0x00, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x01, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x02, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x03, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x04, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x05, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x06, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x07, 0x80},
        {0xa5, 0x00, 0x00, 0x00, 0x08, 0x80}};

        if(it_hw_grabber_capsense_write_i2c((MMP_UINT8 *)ctrlBuffer[index], 6))
            printf("---coreCapsenseWriteI2C error!!!---\n");

        if (index == 13)
            index = 0;
        else
            index = index + 1;

        printf("---capsense set led index = %d ---\n", index);
        status = 0;
    }
    else//IIC Read
    {
        MMP_UINT8   changed;    //0=no change; 0x80 struct has changed since last I2C
        MMP_UINT8   but_state;  //bit vector of buttons; bit0=Mute bit1=Boost bit2=Unlock bit3=unused
        MMP_UINT16  vol_rpt;    //"volume level" 0-32 (incorporates Boost, Vol Slider and Mute states)
        MMP_UINT16  slider;     //raw slider value 0-2048 if valid; 4095 if invalid
        MMP_UINT16  ver_maj;    //FW Major Version; currently 0
        MMP_UINT16  ver_min;    //FW Minor Version; currently 8
        MMP_UINT16  buildno;    //FW Build Date;    currently 31234
        MMP_UINT16  EEupdates;  // number of times eeProm data has been saved (updates on every "Lock"

        MMP_UINT8   stateBuffer[14];

        if (it_hw_grabber_capsense_InterruptState())
        {
            if (it_hw_grabber_capsense_read_i2c((MMP_UINT8 *)stateBuffer, 14))
                printf("---coreCapsenseReadI2C error!!!---\n");
            else
            {
                changed         = stateBuffer[0];
                but_state       = stateBuffer[1];
                vol_rpt         = ((MMP_UINT16)stateBuffer[2] << 8) | (MMP_UINT16)stateBuffer[3];
                slider          = ((MMP_UINT16)stateBuffer[4] << 8) | (MMP_UINT16)stateBuffer[5];
                ver_maj         = ((MMP_UINT16)stateBuffer[6] << 8) | (MMP_UINT16)stateBuffer[7];
                ver_min         = ((MMP_UINT16)stateBuffer[8] << 8) | (MMP_UINT16)stateBuffer[9];
                buildno         = ((MMP_UINT16)stateBuffer[10] << 8) | (MMP_UINT16)stateBuffer[11];
                EEupdates       = ((MMP_UINT16)stateBuffer[12] << 8) | (MMP_UINT16)stateBuffer[13];

                printf("---capsense changed = 0x%02x, but_state = 0x%02x, vol_rpt = %d, slider = %d---\n",
                        changed, but_state, vol_rpt, slider);

                //printf("Touch Panel ver_maj = %d, ver_min = %d, buildno = %d, EEupdates = %d\n",
                //      ver_maj, ver_min, buildno, EEupdates);
            }
        }
        else
            printf("---capsense no change---\n");
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_serial_number_property(
    property_hw_grabber_serial_number& property_in,
    property_hw_grabber_serial_number& property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);
    int             i;
    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_hw_grabber_set_usb_serial_number(&property_in.data.data[0]);
        printf("%s(%d): %02X%s\n", __FUNCTION__, __LINE__,
            property_in.data.data[0],
            property_in.data.data);
        status = 0;
    }
    else
    {
        it_hw_grabber_get_usb_serial_number(&property_out.data.data[0]);
        property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_serial_number));
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_led_analog_property(
    property_hw_grabber_led&    property_in,
    property_hw_grabber_led&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_led_analog(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_led_hdmi_property(
    property_hw_grabber_led&    property_in,
    property_hw_grabber_led&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_led_hdmi(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_led_signal_property(
    property_hw_grabber_led&    property_in,
    property_hw_grabber_led&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_led_signal(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_led_record_property(
    property_hw_grabber_led&    property_in,
    property_hw_grabber_led&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_led_record(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_led_sd720p_property(
    property_hw_grabber_led&    property_in,
    property_hw_grabber_led&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_led_sd720p(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_led_hd1080p_property(
    property_hw_grabber_led&    property_in,
    property_hw_grabber_led&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_led_hd1080P(enable);
        status = 0;
    }

l_exit:
    return status;
}

int
it_usbd_property_hw_grabber::do_hw_grabber_hdcp_property(
    property_hw_grabber_hdcp&    property_in,
    property_hw_grabber_hdcp&    property_out)
{
    int             status      = -1;
    unsigned int    flags       = le32_to_cpu(property_in.header.property.flags);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

	if (flags & it_usbd_property::FLAG_GET)
    {
        unsigned int    Isdisablehdcp = 0;
        Isdisablehdcp = it_hw_grabber_get_hdcp();
        property_out.header.property.size   = cpu_to_le32(sizeof(it_usbd_property_hw_grabber::property_hw_grabber_hdcp));
        property_out.data.enable            = cpu_to_le32(Isdisablehdcp);
        status = 0;
    }
	else if (flags & it_usbd_property::FLAG_SET)
    {
        unsigned int enable = le32_to_cpu(property_in.data.enable);
        it_hw_grabber_set_hdcp(enable);
        status = 0;
    }

l_exit:
    return status;
}
