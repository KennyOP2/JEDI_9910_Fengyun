
#ifndef MMP_H
#define MMP_H

#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32.
 */
#if defined(WIN32)
//#if defined(MMP_EXPORTS)
#define MMP_API __declspec(dllexport)
//#else
//#define MMP_API __declspec(dllimport)
//#endif
#else
#define MMP_API extern
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#if defined(__FREERTOS__)
#define MMP_ALIGN32	__attribute__((aligned(32)))
#define MMP_ALIGN16	__attribute__((aligned(16)))
//
//#define UNREFERENCED_PARAMETER(p) (void)(p)
#define UNREFERENCED_VARIABLE(p) (void)(p)
//
///* initialize file system will also initialize MMC */
//#define INIT_PHER_FILESYSTEM    0x0001
//#define INIT_PHER_LCD           0x0002
///* intialize 2D will also initialize command queue */
//#define INIT_PHER_M2D           0x0004
//#define INIT_PHER_CMQ           0x0008
//
///* Only for TEST */
//#define INIT_PHER_MMC           0x0010
//
//#define INIT_PHER_USB           0x0020
//
//
//#define INIT_PHER_ALL  (INIT_PHER_FILESYSTEM|INIT_PHER_LCD|INIT_PHER_M2D|INIT_PHER_CMQ|INIT_PHER_USB)
//
///* Return zero when any pheripheral fails, non-zero otherwise.
//   Pass INIT_PHER_ALL to 'select' by default. */
//int init_pheripherals(unsigned int select);

#else
	#define MMP_ALIGN16
	#define MMP_ALIGN32	
#endif

//typedef enum MODULECLOCK_TAG
// {
//    LEAVE0 = 0,
//    LEAVE1 = 1,
//    LEAVE2 = 2,
//    LEAVE3 = 3
//} MODULECLOCK;

typedef enum MMP_GPIO_GROUP_TAG
{
    MMP_SHARED_GPIO,
    MMP_DEDICATED_GPIO
} MMP_GPIO_GROUP;

typedef enum MMP_GPIO_MODE_TAG
{
    MMP_GPIO_OUTPUT_MODE,
    MMP_GPIO_INPUT_MODE
} MMP_GPIO_MODE;

typedef enum MMP_GPIO_PIN_TAG
{
    MMP_GPIO0  = 0x1,
    MMP_GPIO1  = (0x1 << 1),
    MMP_GPIO2  = (0x1 << 2),
    MMP_GPIO3  = (0x1 << 3),
    MMP_GPIO4  = (0x1 << 4),
    MMP_GPIO5  = (0x1 << 5),
    MMP_GPIO6  = (0x1 << 6),
    MMP_GPIO7  = (0x1 << 7),
    MMP_GPIO8  = (0x1 << 8),
    MMP_GPIO9  = (0x1 << 9),
    MMP_GPIO10 = (0x1 << 10),
    MMP_GPIO11 = (0x1 << 11),
    MMP_GPIO12 = (0x1 << 12),
    MMP_GPIO13 = (0x1 << 13),
    MMP_GPIO14 = (0x1 << 14),
    MMP_GPIO15 = (0x1 << 15),
    MMP_GPIO16 = (0x1 << 16),
    MMP_GPIO17 = (0x1 << 17),
    MMP_GPIO18 = (0x1 << 18),
    MMP_GPIO19 = (0x1 << 19),
    MMP_GPIO20 = (0x1 << 20),
    MMP_GPIO21 = (0x1 << 21),
    MMP_GPIO22 = (0x1 << 22),
    MMP_GPIO23 = (0x1 << 23)
} MMP_GPIO_PIN;

/**
 * For GPIO Customize
 */
typedef enum MMP_GPIO_INIT_ATTRIB_TAG
{
    MMP_GPIO_CARD_POWER_ENABLE         = 0,
    MMP_GPIO_CARD_DETECT               = 1,
    MMP_GPIO_CARD_WP                   = 2,
    MMP_GPIO_IR                        = 3,
    MMP_GPIO_SPDIF                     = 4,
    MMP_GPIO_KEYPAD_IN                 = 5,
    MMP_GPIO_FPC_CTL                   = 6,
    MMP_GPIO_FPC_CLK                   = 7,
    MMP_GPIO_FPC_DAT                   = 8,
    MMP_GPIO_IIC                       = 9,
    MMP_GPIO_UART                      = 10,
    MMP_GPIO_CARD2_POWER_ENABLE        = 11,
    MMP_GPIO_CARD2_DETECT              = 12,
    MMP_GPIO_CARD2_WP                  = 13,
    MMP_GPIO_2SD_SWITCH                = 14,
    MMP_GPIO_MAX                       = 15
} MMP_GPIO_INIT_ATTRIB;

/**
 * Storage
 */ 
typedef enum MMP_CARD_TAG
{
    MMP_CARD_SD_I,
    MMP_CARD_MS, // mode 3 
    MMP_CARD_SD2,
    MMP_CARD_CF,
    MMP_CARD_XD_IP,
    MMP_CARD_MS_mode4
} MMP_CARD;


#define MMP_CD_ALWAYS_ON        0xFF    /** Card Detect has no GPIO control and always ON */
#define MMP_CD_ALWAYS_OFF       0x0     /** Card Detect has no GPIO control and always OFF */
#define MMP_WP_ALWAYS_ON        0xFF    /** SD Write Protect has no GPIO control and always ON */
#define MMP_WP_ALWAYS_OFF       0x0     /** SD Write Protect has no GPIO control and always OFF */
#define MMP_POWER_ALWAYS_ON     0xFF    /** Power Enable has no GPIO control and always ON */
#define MMP_POWER_ALWAYS_OFF    0x0     /** Power Enable has no GPIO control and always OFF (No card) */


//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//#define GPIO_PULL_HI(GPIO)  (GPIO)
//#define GPIO_PULL_LO(GPIO)  (GPIO & ~GPIO)

//=============================================================================
//                              Function Declaration
//=============================================================================

#ifdef WIN32
MMP_API MMP_RESULT
MMP_Initialize(
    MMP_UINT32 vramOffset);

MMP_API MMP_RESULT
MEM_Initialize(
    MMP_UINT32 heapSize);
#endif

MMP_API void
mmpInitializeGPIO(
    MMP_ULONG *attribList);

MMP_API MMP_BOOL 
mmpIsCardInserted(
    MMP_UINT32 card);

#ifdef __cplusplus
}
#endif

#endif
