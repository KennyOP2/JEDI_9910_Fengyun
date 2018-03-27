/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author
 */
#ifndef	IR_CONFIG_H
#define	IR_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
//#include "FreeRTOSConfig.h"
#include "mmp_types.h"
#include "mmp_util.h"
#include "host/host.h"
#include "host/ahb.h"
#include "host/gpio.h"
#include "sys/sys.h"

#include "mmp_ir.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define ENABLE_TASK_LOG           // undefine it to show the task log
//#define HAVE_REPEAT_KEY         //kenny patch 20140428   // undefine it to disable repeat key.

//-------------------------
// Define the IR Key TYPE
//-------------------------
#define CONFIG_IR_NORMAL
//#define CONFIG_IR_SHARP

//=============================================================================
//                              Constant Definition
//=============================================================================

//-------------------------
// Sample precision
//-------------------------
#define PRECISION               16
#define IR_TIME(n)              ((int)((n) * (1 << PRECISION))) // conver floating to fixed point integer
#define SAMP_DUR                IR_TIME(0.1)  //kenny patch 20140428 0.01  // sample duration in milliseconds

//-------------------------
// Repeat Key Define
//-------------------------
#define REPEAT_THRESHOLD_BEGIN  (5)     // accumulate number of repeat-key will start dispatch key
#define REPEAT_THRESHOLD_SPDUP  (2)     // accumulate number of key will change to high speed mode
#define REPEAT_THRESHOLD_HOLD1  (5)     // dispatch a key foreach number of repeat-key at low speed mode
#define REPEAT_THRESHOLD_HOLD2  (1)     // dispatch a key foreach number of repeat-key at high speed mode

//=============================================================================
//                              IR Key Spec Definition
//
//  CONFIG_KEY_START   IR_TIME(9.0 + 4.5)    // normal start (9ms+4.5ms)
//  CONFIG_KEY_REPEAT  IT_TIME(9.0 + 2.25)   // repeat key start (9ms+2.25ms). The repeat
//                                           //   key is the special key of start key.
//                                           //   Set the key as CONFIG_KEY_START if the IR
//                                           //   dose not have repeat key.
//  CONFIG_KEY_0       IR_TIME(0.56 * 2)     // bit0 (0.56ms+0.56ms)
//  CONFIG_KEY_1       IR_TIME(0.56 * 4)     // bit1 (0.56ms+0.56ms*3)
//  CONFIG_KEY_GAP     IR_TIME(31.5)         // minimun time gap of each key stroke
//  BIT_PER_KEY        (32)                  // Number of bit per key
//  CONFIG_LSB         1                     // LSB first of MSB first
//  
//=============================================================================
#if defined(CONFIG_IR_NORMAL)
#  define CONFIG_KEY_START        IR_TIME(9.0 + 4.5)
#  define CONFIG_KEY_REPEAT       IR_TIME(9.0 + 2.25)
#  define CONFIG_KEY_0            IR_TIME(0.56 * 2)
#  define CONFIG_KEY_1            IR_TIME(0.56 * 4)
#  define CONFIG_KEY_GAP          IR_TIME(31.5)
#  define BIT_PER_KEY             (32)
#  define CONFIG_LSB              1
#elif defined(CONFIG_IR_SHARP)
#  define CONFIG_KEY_START        IR_TIME(0.436 * 12)
#  define CONFIG_KEY_REPEAT       CONFIG_KEY_START
#  define CONFIG_KEY_0            IR_TIME(0.436 * 2)
#  define CONFIG_KEY_1            IR_TIME(0.436 * 4)
#  define CONFIG_KEY_GAP          IR_TIME(31.5)
#  define BIT_PER_KEY             (48)
#  define CONFIG_LSB              1
#else
#  error "No IR type defined."
#endif

//=============================================================================
//                              LOG definition
//=============================================================================
//#ifdef DEBUG_MODE
//#define LOG_ZONES   (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_DEBUG & ~MMP_ZONE_INFO)
////#define LOG_ZONES   (MMP_BIT_ALL & ~MMP_ZONE_ENTER & ~MMP_ZONE_LEAVE & ~MMP_ZONE_INFO)
//
//#define LOG_ERROR   ((void) ((MMP_ZONE_ERROR   & (LOG_ZONES)) ? (printf("[SMEDIA][IR][ERROR]"
//#define LOG_WARNING ((void) ((MMP_ZONE_WARNING & (LOG_ZONES)) ? (printf("[SMEDIA][IR][WARNING]"
//#define LOG_INFO    ((void) ((MMP_ZONE_INFO    & (LOG_ZONES)) ? (printf("[SMEDIA][IR][INFO]"
//#define LOG_DEBUG   ((void) ((MMP_ZONE_DEBUG   & (LOG_ZONES)) ? (printf("[SMEDIA][IR][DEBUG]"
//#define LOG_ENTER   ((void) ((MMP_ZONE_ENTER   & (LOG_ZONES)) ? (printf("[SMEDIA][IR][ENTER]"
//#define LOG_LEAVE   ((void) ((MMP_ZONE_LEAVE   & (LOG_ZONES)) ? (printf("[SMEDIA][IR][LEAVE]"
//#define LOG_DATA    ((void) ((MMP_TRUE) ? (printf(
//#define LOG_END     )), 1 : 0));
//#else
#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END         ;
//#endif

#ifdef __cplusplus
}
#endif

#endif
