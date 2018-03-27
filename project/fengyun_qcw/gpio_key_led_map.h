#ifndef GPIO_KEY_LED_MAP_H
#define GPIO_KEY_LED_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mmp_capture.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
typedef enum KEY_CODE_TAG
{
    KEY_CODE_S1 = 0,
    KEY_CODE_S2,
    KEY_CODE_S3,
    KEY_CODE_S4,
    KEY_CODE_S1_HOLD,
    KEY_CODE_UNKNOW,
} KEY_CODE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct GPIO_LED_CONTROL_TAG
{
    MMP_BOOL EnAnalog;
    MMP_BOOL EnHDMI;
    MMP_BOOL En720P;
    MMP_BOOL En1080P;
    MMP_BOOL EnRecord;
    MMP_BOOL EnPower;
    MMP_BOOL EnUSB;
    MMP_BOOL EnSD;
} GPIO_LED_CONTROL;

typedef struct FULL_HD_RECORD_TABLE_TAG
{
    MMP_CAP_INPUT_INFO index;
    MMP_BOOL           isFullHDRes;
} FULL_HD_RECORD_TABLE;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static FULL_HD_RECORD_TABLE RECORD_TABLE [] = {
    {MMP_CAP_INPUT_INFO_640X480_60P, 0},
    {MMP_CAP_INPUT_INFO_720X480_59I, 0},
    {MMP_CAP_INPUT_INFO_720X480_59P, 0},
    {MMP_CAP_INPUT_INFO_720X480_60I, 0},
    {MMP_CAP_INPUT_INFO_720X480_60P, 0},
    {MMP_CAP_INPUT_INFO_720X576_50I, 0},
    {MMP_CAP_INPUT_INFO_720X576_50P, 0},
    {MMP_CAP_INPUT_INFO_1280X720_50P, 0},
    {MMP_CAP_INPUT_INFO_1280X720_59P, 0},
    {MMP_CAP_INPUT_INFO_1280X720_60P, 0},
    {MMP_CAP_INPUT_INFO_1920X1080_23P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_24P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_25P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_29P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_30P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_50I, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_50P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_59I, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_59P, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_60I, 1},
    {MMP_CAP_INPUT_INFO_1920X1080_60P, 1},
    {MMP_CAP_INPUT_INFO_800X600_60P, 0},
    {MMP_CAP_INPUT_INFO_1024X768_60P, 0},
    {MMP_CAP_INPUT_INFO_1280X768_60P, 0},
    {MMP_CAP_INPUT_INFO_1280X800_60P, 0},
    {MMP_CAP_INPUT_INFO_1280X960_60P, 1},
    {MMP_CAP_INPUT_INFO_1280X1024_60P, 1},
    {MMP_CAP_INPUT_INFO_1360X768_60P, 0},
    {MMP_CAP_INPUT_INFO_1366X768_60P, 0},
    {MMP_CAP_INPUT_INFO_1440X900_60P, 1},
    {MMP_CAP_INPUT_INFO_1400X1050_60P, 1},
    {MMP_CAP_INPUT_INFO_1440X1050_60P, 1},
    {MMP_CAP_INPUT_INFO_1600X900_60P, 1},
    {MMP_CAP_INPUT_INFO_1600X1200_60P, 1},
    {MMP_CAP_INPUT_INFO_1680X1050_60P, 1}
};

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * GPIO led Process
 */
//=============================================================================
MMP_RESULT
GpioLEDInitialize(
    void);

MMP_RESULT
GpioLEDTerminate(
    void);

//=============================================================================
/**
 * GPIO Key Process
 */
//=============================================================================
MMP_RESULT
GpioKeyInitialize(
    void);

MMP_RESULT
GpioKeyTerminate(
    void);

KEY_CODE
GpioKeyGetKey(
    void);

//=============================================================================
/**
 * GPIO Mic Detect Process
 */
//=============================================================================
MMP_RESULT
GpioMicInitialize(
    void);

MMP_BOOL
GpioMicIsInsert(
    void);

//kenny 20140617
MMP_BOOL
GpioInputselect(
    MMP_BOOL status);

#ifdef __cplusplus
}
#endif

#endif // end of GPIO_KEY_MAP_H