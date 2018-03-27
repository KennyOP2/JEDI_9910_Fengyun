/*
 * Copyright (c) 2009 ITE Technology Corp. All Rights Reserved.
 */
/** @file msg.h
 *
 * @author I-Chun Lai
 */

#ifndef MSG_H
#define MSG_H

#include "pal\pal.h"
#include "pal\keypad.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef enum
{
    MSG_NULL,
    MSG_KEYPAD_RECORD,
    MSG_KEYPAD_CHANGE_INPUT_SOURCE,
    MSG_KEYPAD_CHANGE_ENCODE_RESOLUTION,
    MSG_USB_DISK_PLUG_IN,
    MSG_USB_DISK_PLUG_OUT,
    MSG_INPUT_SOURCE_RESOLUTION_CHANGE,
    MSG_DEVICE_MODE,
    MSG_HOST_MODE,
    MSG_CONFIG_STORE,
    MSG_SERIAL_NUMBER_STORE,
    MSG_START_ENCODER,
    MSG_STOP_ENCODER
} MSG_NAME;

typedef enum
{
    MSG_RECEIVER_ALL = 0,
    MSG_RECEIVER_NORMAL_OBJ,
    MSG_RECEIVER_GLOBAL_OBJ
} MSG_RECEIVER;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct
{
    MSG_NAME        name;           // name of message (an enumerated type works well)
    MMP_UINT32      sender_id;      // unique id of sender module
    MMP_UINT32      receiver_id;    // unique id of receiver module
    PAL_CLOCK_T     delivery_time;  // deliver message at this time
    PAL_CLOCK_T     delay;

    // Note that the sender_id and receiver_id are not pointers to modules.
    // Since messages can be delayed, the sender or receiver may get removed
    // from the game and a pointer would become dangerously invalid.

    // You can add right here any data you want to be passed
    // along with every message - sometimes it's helpful to let
    // messages convey more info by using extra data.
    // For example, a damaged message could carry with it the amount of damage
    MMP_UINT32      extraData;
} MSG_OBJECT;

#endif
