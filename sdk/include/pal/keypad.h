#ifndef KEYPAD_H
#define KEYPAD_H

#include "pal/def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PAL_KEYPAD_TAG
{
    PAL_KEYPAD_NULL = 0,       // 0x0
    PAL_KEYPAD_NUMBER_0,       // 0x1
    PAL_KEYPAD_NUMBER_1,       // 0x2
    PAL_KEYPAD_NUMBER_2,       // 0x3
    PAL_KEYPAD_NUMBER_3,       // 0x4
    PAL_KEYPAD_NUMBER_4,       // 0x5
    PAL_KEYPAD_NUMBER_5,       // 0x6
    PAL_KEYPAD_NUMBER_6,       // 0x7
    PAL_KEYPAD_NUMBER_7,       // 0x8
    PAL_KEYPAD_NUMBER_8,       // 0x9
    PAL_KEYPAD_NUMBER_9,       // 0xA
    PAL_KEYPAD_STANDBY,        // 0xB
    PAL_KEYPAD_UNDEFINE_1,     // 0xC
    PAL_KEYPAD_TV_RADIO,       // 0xD
    PAL_KEYPAD_BACK,           // 0xE
    PAL_KEYPAD_MP3,            // 0xF
    PAL_KEYPAD_VIDEO,          // 0x10
    PAL_KEYPAD_PHOTO,          // 0x11
    PAL_KEYPAD_UNDEFINE_2,     // 0x12
    PAL_KEYPAD_VOLUME_UP,      // 0x13
    PAL_KEYPAD_CHANNEL_UP,     // 0x14
    PAL_KEYPAD_MUTE,           // 0x15
    PAL_KEYPAD_RECORD,         // 0x16
    PAL_KEYPAD_VOLUME_DOWN,    // 0x17
    PAL_KEYPAD_CHANNEL_DOWN,   // 0x18
    PAL_KEYPAD_FAST_REWIND,    // 0x19
    PAL_KEYPAD_PLAY_PAUSE,     // 0x1A
    PAL_KEYPAD_STOP,           // 0x1B
    PAL_KEYPAD_FAST_FORWARD,   // 0x1C
    PAL_KEYPAD_UP,             // 0x1D
    PAL_KEYPAD_LEFT,           // 0x1E
    PAL_KEYPAD_OK,             // 0x1F
    PAL_KEYPAD_RIGHT,          // 0x20
    PAL_KEYPAD_DOWN,           // 0x21
    PAL_KEYPAD_MENU,           // 0x22
    PAL_KEYPAD_EXIT,           // 0x23
    PAL_KEYPAD_RED,            // 0x24
    PAL_KEYPAD_GREEN,          // 0x25
    PAL_KEYPAD_YELLOW,         // 0x26
    PAL_KEYPAD_BLUE,           // 0x27
    PAL_KEYPAD_INFO,           // 0x28
    PAL_KEYPAD_AUDIO,          // 0x29
    PAL_KEYPAD_SUBTITLE,       // 0x2A
    PAL_KEYPAD_TELETEXT,       // 0x2B
    PAL_KEYPAD_EPG,            // 0x2C
    PAL_KEYPAD_FAVORITE,       // 0x2D
    PAL_KEYPAD_UNDEFINE_3,     // 0x2E
    PAL_KEYPAD_ZOOMIN,         // 0x2F

    PAL_KEYPAD_CUSTOM_1,       // 0x30
    PAL_KEYPAD_CUSTOM_2,       // 0x31
    PAL_KEYPAD_CUSTOM_3,       // 0x32
    PAL_KEYPAD_CUSTOM_4,       // 0x33
    PAL_KEYPAD_CUSTOM_5,       // 0x34
    PAL_KEYPAD_CUSTOM_6,       // 0x35
    PAL_KEYPAD_CUSTOM_7,       // 0x36
    PAL_KEYPAD_CUSTOM_8,       // 0x37
} PAL_KEYPAD;
                               
#ifdef __cplusplus
}
#endif

#endif
