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
    MSG_KEYPAD_RECORD = 0x1000,
    MSG_KEYPAD_CHANGE_INPUT_SOURCE,
    MSG_KEYPAD_CHANGE_ENCODE_RESOLUTION,
    MSG_USB_DISK_PLUG_IN,
    MSG_USB_DISK_PLUG_OUT,
    MSG_INPUT_SOURCE_RESOLUTION_CHANGE,
    MSG_CONFIG_STORE,
};
#endif
