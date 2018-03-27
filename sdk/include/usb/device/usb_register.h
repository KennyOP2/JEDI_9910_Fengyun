/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as USB Register Definition.
 */


#ifndef USB_REG_H
#define USB_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/** for device driver use */
#ifdef USB0_OTG_USB1_HOST
#define USE_USB0
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================
#define USB0_INTERNAL_BASE    USB0_BASE
#define USB1_INTERNAL_BASE    USB1_BASE


#if defined(USE_USB0)
#define USB_INTERNAL_BASE    USB0_INTERNAL_BASE
#else
#define USB_INTERNAL_BASE    USB1_INTERNAL_BASE
#endif

#define BIT0    0x00000001
#define BIT1    0x00000002
#define BIT2    0x00000004
#define BIT3    0x00000008
#define BIT4    0x00000010
#define BIT5    0x00000020
#define BIT6    0x00000040
#define BIT7    0x00000080
#define BIT8    0x00000100
#define BIT9    0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000

//==================================
// Hardware related constant
//==================================
#define DEVICE_MAX_ENDPOINT_NUM		8
#define DEVICE_MAX_FIFO_NUM	        4
#define EP0MAXPACKETSIZE	        0x40

//==================================
// Hardware related constant - for FIFO and endpoint
//==================================
#define MASK_F0				0xF0

// Block Size define
#define BLK512BYTE		1
#define BLK1024BYTE		2

#define BLK64BYTE		1
#define BLK128BYTE		2

// Block toggle number define
#define SINGLE_BLK		1
#define DOUBLE_BLK		2
#define TRIBLE_BLK		3

// Endpoint transfer type
#define TF_TYPE_ISOCHRONOUS		1
#define TF_TYPE_BULK			2
#define TF_TYPE_INTERRUPT		3

// Endpoint or FIFO direction define
#define DIRECTION_IN	0
#define DIRECTION_OUT	1

// FIFO number define
#define FIFO0	0x0
#define FIFO1	0x1
#define FIFO2	0x2
#define FIFO3	0x3
#define FIFO4	0x4
#define FIFO5	0x5
#define FIFO6	0x6
#define FIFO7	0x7
#define FIFO8	0x8
#define FIFO9	0x9

// Endpoint number define
#define EP0        0x00
#define EP1        0x01
#define EP2        0x02
#define EP3        0x03
#define EP4        0x04
#define EP5        0x05
#define EP6        0x06
#define EP7        0x07
#define EP8        0x08
#define EP9        0x09
#define EP10        0x10
#define EP11        0x11
#define EP12        0x12
#define EP13        0x13
#define EP14        0x14
#define EP15        0x15



//=============================================================================
//                              FOTG210 Register Definition
//=============================================================================

//===================================
//    Host Controller Register
//===================================

//=============================================================================
/** 0x000h
 * HC Capability Register
 */
//=============================================================================
#define USB_HOST_REG_CAPABILITY                         (USB_INTERNAL_BASE + 0x0000)

#define USB_HOST_MSK_VERSION_NUM                        0xFFFF0000
#define USB_HOST_MSK_CAP_LENGTH                         0x000000FF

#define USB_HOST_SHT_VERSION_NUM                        16
#define USB_HOST_SHT_CAP_LENGTH                         0

//=============================================================================
/** 0x004h
 * HCSPARAMS - HC structural Parameters
 */
//=============================================================================
#define USB_HOST_REG_HCSPARAMS                          (USB_INTERNAL_BASE + 0x0004)

#define USB_HOST_MSK_PORT_NUM                           0x0000000F
#define USB_HOST_SHT_PORT_NUM                           0

//=============================================================================
/** 0x008h
 * HCCPARAMS - HC Capability Parameters
 */
//=============================================================================
#define USB_HOST_REG_HCCPARAMS                          (USB_INTERNAL_BASE + 0x0008)

#define USB_HOST_MSK_ASYNCHRONOUS_SCHEDULE_PARK_CAP     0x00000004
#define USB_HOST_MSK_PROGRAMMABLE_FRAME_LIST_FLAG       0x00000002

//=============================================================================
/** 0x010h
 * USBCMD - HC USB Command Register
 */
//=============================================================================
#define USB_HOST_REG_USBCMD                             (USB_INTERNAL_BASE + 0x0010)

#define USB_HOST_MSK_INT_THRESHOLD_CTRL                 0x00FF0000
#define USB_HOST_MSK_ASYN_PK_EN                         BIT11
#define USB_HOST_MSK_ASYN_PK_CNT                        (BIT9|BIT8)
#define USB_HOST_MSK_INT_OAAD                           BIT6
#define USB_HOST_MSK_ASCH_EN                            BIT5
#define USB_HOST_MSK_PSCH_EN                            BIT4
#define USB_HOST_MSK_FRL_SIZE                           (BIT3|BIT2)
#define USB_HOST_MSK_HC_RESET                           BIT1
#define USB_HOST_MSK_RUN_STOP                           BIT0

#define USB_HOST_SHT_INT_THRESHOLD_CTRL                 16
#define USB_HOST_SHT_ASYN_PK_CNT                        8
#define USB_HOST_SHT_FRL_SIZE                           2

typedef enum USB_HOST_INT_INTERVAL_TAG
{
    USB_HOST_INT_INTERVAL_RESERVED          = 0x00,
    USB_HOST_INT_INTERVAL_1_MICRO_FRAME     = 0x01,
    USB_HOST_INT_INTERVAL_2_MICRO_FRAME     = 0x02,
    USB_HOST_INT_INTERVAL_4_MICRO_FRAME     = 0x04,
    USB_HOST_INT_INTERVAL_8_MICRO_FRAME     = 0x08,
    USB_HOST_INT_INTERVAL_10_MICRO_FRAME    = 0x10,
    USB_HOST_INT_INTERVAL_20_MICRO_FRAME    = 0x20,
    USB_HOST_INT_INTERVAL_40_MICRO_FRAME    = 0x40
} USB_HOST_INT_INTERVAL;


/** Interrupt definition */
enum
{
    USB_HOST_INT_ON_ASYNC_ADVANCE       = BIT5,
    USB_HOST_INT_SYSTEM_ERROR           = BIT4,
    USB_HOST_INT_FRAME_ROLLOVER         = BIT3,
    USB_HOST_INT_PORT_CHANGE_DETECT     = BIT2,
    USB_HOST_INT_ERROR                  = BIT1,
    USB_HOST_INT_COMPLETE_TRANSACTION   = BIT0
};

//=============================================================================
/** 0x014h
 * USBSTS - HC USB Status Register
 */
//=============================================================================
#define USB_HOST_REG_USBSTS                             (USB_INTERNAL_BASE + 0x0014)

#define USB_HOST_MSK_ASCH_STATUS                        BIT15
#define USB_HOST_MSK_PSCH_STATUS                        BIT14
#define USB_HOST_MSK_RECLAMATION                        BIT13
#define USB_HOST_MSK_HALTED                             BIT12
#define USB_HOST_MSK_STS_INT_ON_ASYNC_ADVANCE           USB_HOST_INT_ON_ASYNC_ADVANCE
#define USB_HOST_MSK_STS_SYSTEM_ERROR                   USB_HOST_INT_SYSTEM_ERROR
#define USB_HOST_MSK_STS_FRAME_ROLLOVER                 USB_HOST_INT_FRAME_ROLLOVER
#define USB_HOST_MSK_STS_PORT_CHANGE_DETECT             USB_HOST_INT_PORT_CHANGE_DETECT
#define USB_HOST_MSK_STS_ERROR                          USB_HOST_INT_ERROR
#define USB_HOST_MSK_STS_COMPLETE_TRANSACTION           USB_HOST_INT_COMPLETE_TRANSACTION

//=============================================================================
/** 0x018h
 * USBINTR - HC USB Interrupt Enable Register
 */
//=============================================================================
#define USB_HOST_REG_USBINTR                           (USB_INTERNAL_BASE + 0x0018)

#define USB_HOST_MSK_EN_INT_ON_ASYNC_ADVANCE           USB_HOST_INT_ON_ASYNC_ADVANCE
#define USB_HOST_MSK_EN_SYSTEM_ERROR                   USB_HOST_INT_SYSTEM_ERROR
#define USB_HOST_MSK_EN_FRAME_ROLLOVER                 USB_HOST_INT_FRAME_ROLLOVER
#define USB_HOST_MSK_EN_PORT_CHANGE_DETECT             USB_HOST_INT_PORT_CHANGE_DETECT
#define USB_HOST_MSK_EN_ERROR                          USB_HOST_INT_ERROR
#define USB_HOST_MSK_EN_COMPLETE_TRANSACTION           USB_HOST_INT_COMPLETE_TRANSACTION
#define USB_HOST_MSK_INTERRUPT                         0x0000003F
#define USB_HOST_MSK_INTERRUPT_NONE                    0x00000000

//=============================================================================
/** 0x01Ch
 * FRINDEX - HC Frame Index Register
 */
//=============================================================================
#define USB_HOST_REG_FRINDEX                           (USB_INTERNAL_BASE + 0x001C)

//=============================================================================
/** 0x024h
 * PERIODICLISTBASE - HC Periodic Frame List Base Address Register
 */
//=============================================================================
#define USB_HOST_REG_PERIODIC_BASE_ADDR                (USB_INTERNAL_BASE + 0x0024)

#define USB_HOST_MSK_PERIODIC_BASE_ADDR                 0xFFFFF000

//=============================================================================
/** 0x028h
 * ASYNCLISTADDR - HC Current Asynchronous List Address Register
 */
//=============================================================================
#define USB_HOST_REG_ASYNCHRONOUS_BASE_ADDR            (USB_INTERNAL_BASE + 0x0028)

#define USB_HOST_MSK_ASYNCHRONOUS_BASE_ADDR            0xFFFFFFE0

//=============================================================================
/** 0x030h
 * PORTSC - HC Port Status and Control Register
 */
//=============================================================================
#define USB_HOST_REG_PORTSC                             (USB_INTERNAL_BASE + 0x0030)

#define USB_HOST_MSK_PORT_TEST                          0x000F0000
#define USB_HOST_MSK_LINE_STATE                         (BIT11|BIT10)
#define USB_HOST_MSK_PORT_RESET                         BIT8
#define USB_HOST_MSK_PORT_SUSPEND                       BIT7
#define USB_HOST_MSK_PORT_FORCE_RESUME                  BIT6
#define USB_HOST_MSK_PORT_CHANGE_EN                     BIT3
#define USB_HOST_MSK_PORT_ENABLE                        BIT2
#define USB_HOST_MSK_CONNECT_STATUS_CHANGE              BIT1
#define USB_HOST_MSK_CONNECT_STATUS                     BIT0

#define USB_HOST_SHT_PORT_TEST                          16
#define USB_HOST_SHT_LINE_STATE                         10

//=============================================================================
/** 0x040h
 * Miscellaneous Register
 */
//=============================================================================
#define USB_HOST_REG_MISC                              (USB_INTERNAL_BASE + 0x0040)

#define USB_HOST_MSK_EOF2_TIME                          (BIT5|BIT4)
#define USB_HOST_MSK_EOF1_TIME                          (BIT3|BIT2)

#define USB_HOST_SHT_EOF2_TIME                          4
#define USB_HOST_SHT_EOF1_TIME                          2



//===================================
//    On-The-Go Controller Register
//===================================

//=============================================================================
/** 0x080h
 * Miscellaneous Register
 */
//=============================================================================
#define USB_OTG_REG_CONTROL_STATUS                     (USB_INTERNAL_BASE + 0x0080)

#define USB_OTG_MSK_FULL_SPEED_PHY_WR                   BIT28
#define USB_OTG_MSK_HOST_SPEED_TYPE                     (BIT23|BIT22)
#define USB_OTG_MSK_CURRENT_ID                          BIT21
#define USB_OTG_MSK_CURRENT_ROLE                        BIT20
#define USB_OTG_MSK_HOST_VBUS_VALID                     BIT19
#define USB_OTG_MSK_HOST_SESSION_VALID                  BIT18
#define USB_OTG_MSK_DEVICE_SESSION_VALID                BIT17
#define USB_OTG_MSK_DEVICE_SESSION_END                  BIT16
#define USB_OTG_MSK_PHY_RESET                           BIT15
#define USB_OTG_MSK_FORCE_HIGH_SPEED                    BIT14
#define USB_OTG_MSK_FORCE_FULL_SPEED                    BIT12

#define USB_OTG_MSK_HOST_SRP_RESPONSE_TYPE              BIT8
#define USB_OTG_MSK_HOST_SRP_DETECTION_EN               BIT7
#define USB_OTG_MSK_HOST_SET_DEVICE_HNP_EN              BIT6
#define USB_OTG_MSK_HOST_BUS_DROP                       BIT5
#define USB_OTG_MSK_HOST_BUS_REQUEST                    BIT4
#define USB_OTG_MSK_DEVICE_DISCHARGE_VBUS               BIT2
#define USB_OTG_MSK_DEVICE_HNP_EN                       BIT1
#define USB_OTG_MSK_DEVICE_BUS_REQUEST                  BIT0

#define USB_OTG_SHT_HOST_SPEED_TYPE                     22

enum
{
    USB_OTG_ID_A,
    USB_OTG_ID_B
};

enum
{
    USB_OTG_HIGH_SPEED_TYPE,
    USB_OTG_FULL_SPEED_TYPE
};

//=============================================================================
/** 0x084h
 * OTG Interrupt Status Register
 */
//=============================================================================
#define USB_OTG_REG_INTERRUPT_STATUS                   (USB_INTERNAL_BASE + 0x0084)

enum
{
    USB_OTG_INT_B_SRP_DN        = BIT0,
    USB_OTG_INT_A_SRP_DET       = BIT4,
    USB_OTG_INT_A_VBUS_ERROR    = BIT5,
    USB_OTG_INT_RLCHG           = BIT8,
    USB_OTG_INT_ID_CHANGE       = BIT9,
    USB_OTG_INT_OVC             = BIT10,
    USB_OTG_INT_B_PLGRMV        = BIT11,
    USB_OTG_INT_A_PLGRMV        = BIT12
};

#define USB_OTG_INT_A_TYPE      (USB_OTG_INT_A_SRP_DET | \
                                 USB_OTG_INT_A_VBUS_ERROR | \
                                 USB_OTG_INT_RLCHG | \
                                 USB_OTG_INT_ID_CHANGE | \
                                 USB_OTG_INT_OVC | \
                                 USB_OTG_INT_B_PLGRMV | \
                                 USB_OTG_INT_A_PLGRMV)

#define USB_OTG_INT_B_TYPE      (USB_OTG_INT_B_SRP_DN | \
                                 USB_OTG_INT_A_VBUS_ERROR | \
                                 USB_OTG_INT_RLCHG | \
                                 USB_OTG_INT_ID_CHANGE | \
                                 USB_OTG_INT_OVC)

//=============================================================================
/** 0x088h
 * OTG Interrupt Status Register
 */
//=============================================================================
#define USB_OTG_REG_INTERRUPT_ENABLE                   (USB_INTERNAL_BASE + 0x0088)

#define USB_OTG_MSK_INT_A_PLGRMV                        USB_OTG_INT_A_PLGRMV
#define USB_OTG_MSK_INT_B_PLGRMV                        USB_OTG_INT_B_PLGRMV
#define USB_OTG_MSK_INT_OVC                             USB_OTG_INT_OVC
#define USB_OTG_MSK_INT_ID_CHANGE                       USB_OTG_INT_ID_CHANGE
#define USB_OTG_MSK_INT_RLCHG                           USB_OTG_INT_RLCHG
#define USB_OTG_MSK_INT_A_VBUS_ERROR                    USB_OTG_INT_A_VBUS_ERROR
#define USB_OTG_MSK_INT_A_SRP_DET                       USB_OTG_INT_A_SRP_DET    
#define USB_OTG_MSK_INT_B_SRP_DN                        USB_OTG_INT_B_SRP_DN   


//===================================
//    Global Controller Register  (0x0C0 ~ 0x0FF)
//===================================

//=============================================================================
/** 0x0C0h
 * Gloabl HC/OTG/DEVICE Interrupt Status Register
 */
//=============================================================================
#define USB_GBL_REG_INTERRUPT_STATUS                   (USB_INTERNAL_BASE + 0x00C0)

#define USB_GBL_MSK_INTERRUPT                          (BIT0|BIT1|BIT2)
#define USB_GBL_MSK_INTERRUPT_HC                        BIT2
#define USB_GBL_MSK_INTERRUPT_OTG                       BIT1
#define USB_GBL_MSK_INTERRUPT_DEVICE                    BIT0

//=============================================================================
/** 0x0C4h
 * Gloabl Mask of HC/OTG/DEVICE Interrupt Register
 */
//=============================================================================
#define USB_GBL_REG_INTERRUPT_MASK                     (USB_INTERNAL_BASE + 0x00C4)

#define USB_GBL_MSK_INTERRUPT_HC_MASK                  BIT2
#define USB_GBL_MSK_INTERRUPT_OTG_MASK                 BIT1
#define USB_GBL_MSK_INTERRUPT_DEVICE_MASK              BIT0



//===================================
//    Device Controller Register  (0x100 ~ 0x1FF)
//===================================

//=============================================================================
/** 0x100h
 * Device Main Control Register
 */
//=============================================================================
#define USB_DEVICE_REG_MAIN_CONTROL                   (USB_INTERNAL_BASE + 0x0100)

#define USB_DEVICE_MSK_FORCE_FULL_SPEED                BIT9
#define USB_DEVICE_MSK_DMA_RESET                       BIT8
#define USB_DEVICE_MSK_HIGH_SPEED_STATUS               BIT6
#define USB_DEVICE_MSK_CHIP_EN                         BIT5
#define USB_DEVICE_MSK_SW_RESET                        BIT4
#define USB_DEVICE_MSK_PHY_GO_SUSPEND                  BIT3
#define USB_DEVICE_MSK_GLB_INTERRUPT_EN                BIT2
#define USB_DEVICE_MSK_HALF_SPEED_EN                   BIT1
#define USB_DEVICE_MSK_REMOTE_WAKEUP_CAP               BIT0

//=============================================================================
/** 0x104h
 * Device Main Control Register
 */
//=============================================================================
#define USB_DEVICE_REG_DEVICE_ADDRESS                  (USB_INTERNAL_BASE + 0x0104)

#define USB_DEVICE_MSK_AFTER_SET_CONFIG                BIT7
#define USB_DEVICE_MSK_DEVICE_ADDRESS                  0x0000007F

//=============================================================================
/** 0x108h
 * Device Test Register
 */
//=============================================================================
#define USB_DEVICE_REG_DEVICE_TEST                     (USB_INTERNAL_BASE + 0x0108)

#define USB_DEVICE_MSK_CLEAR_FIFO                       BIT0

//=============================================================================
/** 0x10Ch
 * SOF Frame Number Register
 */
//=============================================================================
#define USB_DEVICE_REG_SOF_FRAME_NUMBER                (USB_INTERNAL_BASE + 0x010C)

#define USB_DEVICE_MSK_SOF_MICRO_FRAME_NUM              0x00003800
#define USB_DEVICE_MSK_SOF_FRAME_NUM                    0x000007FF

#define USB_DEVICE_SHT_SOF_MICRO_FRAME_NUM              11
#define USB_DEVICE_SHT_SOF_FRAME_NUM                    0

//=============================================================================
/** 0x110h
 * SOF Mask Timer Register
 */
//=============================================================================
#define USB_DEVICE_REG_SOF_MASK_TIMER                  (USB_INTERNAL_BASE + 0x0110)

#define USB_DEVICE_MSK_SOF_MASK_TIMER                  0x0000FFFF

//=============================================================================
/** 0x114h
 * PHY Test Mode Selector Register
 */
//=============================================================================
#define USB_DEVICE_REG_PHY_TEST_MODE                   (USB_INTERNAL_BASE + 0x0114)

#define USB_DEVICE_MSK_TEST_MODE                       0x0000001E
#define USB_DEVICE_MSK_UNPLUG                          BIT0

enum
{
    PHY_TEST_MODE_J_STATE    = 0x02,
    PHY_TEST_MODE_K_STATE    = 0x04,
    PHY_TEST_MODE_SE0_NAK    = 0x08,
    PHY_TEST_MODE_PACKET     = 0x10
};
	
//=============================================================================
/** 0x118h
 * Vender Specific I/O Control Register
 */
//=============================================================================
#define USB_DEVICE_VENDER_SPECIFIC_IO_CONTROL_REGISTER                  (USB_INTERNAL_BASE + 0x0118)

	
//=============================================================================
/** 0x120h
 * Device CX Configuration and FIFO Empty Status Register
 */
//=============================================================================
#define USB_DEVICE_REG_CX_CONFIG_FIFO_EMPTY_STATUS     (USB_INTERNAL_BASE + 0x0120)

#define USB_DEVICE_MSK_CX_FIFO_BYTE_CNT                0x7F000000
#define USB_DEVICE_MSK_FIFO_FULLY_EMPTY                0x00000F00
#define USB_DEVICE_MSK_CX_FIFO_EMPTY                   BIT5
#define USB_DEVICE_MSK_CX_FIFO_FULL                    BIT4
#define USB_DEVICE_MSK_CLEAR_CX_FIFO_DATA              BIT3
#define USB_DEVICE_MSK_STALL_CX                        BIT2
#define USB_DEVICE_MSK_TST_PKDONE                      BIT1
#define USB_DEVICE_MSK_CX_DONE                         BIT0

#define USB_DEVICE_SHT_CX_FIFO_BYTE_CNT                24

//=============================================================================
/** 0x124h
 * Device Idle Counter Register
 */
//=============================================================================
#define USB_DEVICE_REG_IDLE_COUNTER                    (USB_INTERNAL_BASE + 0x0124)

#define USB_DEVICE_MSK_IDLE_COUNTER                    0x00000007
#define USB_DEVICE_SHT_IDLE_COUNTER                    0

//=============================================================================
/** 0x130h
 * Device Mask of Interrupt Group Register
 */
//=============================================================================
#define USB_DEVICE_REG_MASK_INT_GROUP                  (USB_INTERNAL_BASE + 0x0130)

#define USB_DEVICE_MSK_INT_GROUP2_MASK                 BIT2
#define USB_DEVICE_MSK_INT_GROUP1_MASK                 BIT1
#define USB_DEVICE_MSK_INT_GROUP0_MASK                 BIT0

//=============================================================================
/** 0x134h
 * Device Mask of Interrupt Source Group 0 Register
 */
//=============================================================================
#define USB_DEVICE_REG_MASK_INT_GROUP0                 (USB_INTERNAL_BASE + 0x0134)

#define USB_DEVICE_MSK_INT_MCX_CMD_ABORT                BIT5
#define USB_DEVICE_MSK_INT_MCX_CMD_FAIL                 BIT4
#define USB_DEVICE_MSK_INT_MCX_CMD_END                  BIT3
#define USB_DEVICE_MSK_INT_MCX_OUT                      BIT2
#define USB_DEVICE_MSK_INT_MCX_IN                       BIT1
#define USB_DEVICE_MSK_INT_MCX_SETUP                    BIT0

enum
{
    DEV_INT_G0_CX_SETUP         = BIT0,
    DEV_INT_G0_CX_IN            = BIT1,
    DEV_INT_G0_CX_OUT           = BIT2,
    DEV_INT_G0_CX_END           = BIT3,
    DEV_INT_G0_CX_CMD_FAIL      = BIT4,
    DEV_INT_G0_CX_CMD_ABORT     = BIT5
};

//=============================================================================
/** 0x138h
 * Device Mask of Interrupt Source Group 1 Register
 */
//=============================================================================
#define USB_DEVICE_REG_MASK_INT_GROUP1                 (USB_INTERNAL_BASE + 0x0138)

enum
{
    DEV_INT_G1_FIFO3_IN        = BIT19,
    DEV_INT_G1_FIFO2_IN        = BIT18,
    DEV_INT_G1_FIFO1_IN        = BIT17,
    DEV_INT_G1_FIFO0_IN        = BIT16,
    DEV_INT_G1_FIFO3_SPK       = BIT7,
    DEV_INT_G1_FIFO3_OUT       = BIT6,
    DEV_INT_G1_FIFO2_SPK       = BIT5,
    DEV_INT_G1_FIFO2_OUT       = BIT4,
    DEV_INT_G1_FIFO1_SPK       = BIT3,
    DEV_INT_G1_FIFO1_OUT       = BIT2,
    DEV_INT_G1_FIFO0_SPK       = BIT1,
    DEV_INT_G1_FIFO0_OUT       = BIT0
};

#define DEV_INT_FIFO0_OUT       DEV_INT_G1_FIFO0_OUT|DEV_INT_G1_FIFO0_SPK
#define DEV_INT_FIFO1_OUT       DEV_INT_G1_FIFO1_OUT|DEV_INT_G1_FIFO1_SPK
#define DEV_INT_FIFO2_OUT       DEV_INT_G1_FIFO2_OUT|DEV_INT_G1_FIFO2_SPK
#define DEV_INT_FIFO3_OUT       DEV_INT_G1_FIFO3_OUT|DEV_INT_G1_FIFO3_SPK

#define DEV_INT_FIFO_OUT_ALL    DEV_INT_FIFO0_OUT| \
                                DEV_INT_FIFO1_OUT| \
                                DEV_INT_FIFO2_OUT| \
                                DEV_INT_FIFO3_OUT

#define DEV_INT_FIFO_IN_ALL     DEV_INT_G1_FIFO0_IN| \
                                DEV_INT_G1_FIFO1_IN| \
                                DEV_INT_G1_FIFO2_IN| \
                                DEV_INT_G1_FIFO3_IN

//=============================================================================
/** 0x13Ch
 * Device Mask of Interrupt Source Group 2 Register
 */
//=============================================================================
#define USB_DEVICE_REG_MASK_INT_GROUP2                 (USB_INTERNAL_BASE + 0x013C)

enum
{
    DEV_INT_G2_WAKEUP_BY_VBUS       = BIT10,
    DEV_INT_G2_IDLE                 = BIT9,
    DEV_INT_G2_DMA_ERROR            = BIT8,
    DEV_INT_G2_DMA_COMPLETION       = BIT7,
    DEV_INT_G2_RX_ZERO_BYTE         = BIT6,
    DEV_INT_G2_TX_ZERO_BTYE         = BIT5,
    DEV_INT_G2_ISO_SEQUENTIAL_ABORT = BIT4,
    DEV_INT_G2_ISO_SEQUENTIAL_ERROR = BIT3,
    DEV_INT_G2_RESUME               = BIT2,
    DEV_INT_G2_SUSPEND              = BIT1,
    DEV_INT_G2_BUS_RESET            = BIT0
};

//=============================================================================
/** 0x140h
 * Device Interrupt Status Register
 */
//=============================================================================
#define USB_DEVICE_REG_INT_GROUP_STATUS                (USB_INTERNAL_BASE + 0x0140)

//=============================================================================
/** 0x144h
 * Device Interrupt Source Group 0 Status Register
 */
//=============================================================================
#define USB_DEVICE_REG_INT_GROUP0_STATUS               (USB_INTERNAL_BASE + 0x0144)

//=============================================================================
/** 0x148h
 * Device Interrupt Source Group 1 Status Register
 */
//=============================================================================
#define USB_DEVICE_REG_INT_GROUP1_STATUS               (USB_INTERNAL_BASE + 0x0148)

//=============================================================================
/** 0x14Ch
 * Device Interrupt Source Group 2 Status Register
 */
//=============================================================================
#define USB_DEVICE_REG_INT_GROUP2_STATUS               (USB_INTERNAL_BASE + 0x014C)

//=============================================================================
/** 0x150h
 * Device Receive Zero-Length Data Packet Register
 */
//=============================================================================
#define USB_DEVICE_REG_RX_ZERO_PACKET_STATUS           (USB_INTERNAL_BASE + 0x0150)

//=============================================================================
/** 0x154h
 * Device Transfer Zero-Length Data Packet Register
 */
//=============================================================================
#define USB_DEVICE_REG_TX_ZERO_PACKET_STATUS           (USB_INTERNAL_BASE + 0x0154)

//=============================================================================
/** 0x158h
 * Device Isochronous Sequential Error/Abort Register
 */
//=============================================================================
#define USB_DEVICE_REG_ISO_SEQ_ABORT_ERROR             (USB_INTERNAL_BASE + 0x0158)

#define USB_DEVICE_MSK_ISO_SEQ_ERROR                    0x00FF0000
#define USB_DEVICE_MSK_ISO_ABORT_ERROR                  0x000000FF

#define USB_DEVICE_SHT_ISO_SEQ_ERROR                    16
#define USB_DEVICE_SHT_ISO_ABORT_ERROR                  0

//=============================================================================
/** 0x160h
 * IN Endpoint x MaxPacketSize Register
 * (One per Endpoint, x=1~8) (Address=160+4(x-1)h)
 */
//=============================================================================
#define USB_DEVICE_REG_IN_EPX_MAX_PACKETSIZE           (USB_INTERNAL_BASE + 0x0160)

#define USB_DEVICE_MSK_TX0BYTE_IEPX                    BIT15
#define USB_DEVICE_MSK_TX_NUM_HBW_IEPX                 BIT14|BIT13
#define USB_DEVICE_MSK_RSTG_IEPX                       BIT12
#define USB_DEVICE_MSK_STALL_IEPX                      BIT11
#define USB_DEVICE_MSK_MAXPS_IEPX                      0x000007FF

//=============================================================================
/** 0x180h
 * OUT Endpoint x MaxPacketSize Register
 * (One per Endpoint, x=1~8) (Address=180+4(x-1)h)
 */
//=============================================================================
#define USB_DEVICE_REG_OUT_EPX_MAX_PACKETSIZE          (USB_INTERNAL_BASE + 0x0180)

#define USB_DEVICE_MSK_RSTG_OEPX                       BIT12
#define USB_DEVICE_MSK_STALL_OEPX                      BIT11
#define USB_DEVICE_MSK_MAXPS_OEPX                      0x000007FF

//=============================================================================
/** 0x1A0h 0x1A4h
 * Device Endpoint 1~4 Map Register
 */
//=============================================================================
#define USB_DEVICE_REG_ENDPOINT1_4_MAP                (USB_INTERNAL_BASE + 0x01A0)
#define USB_DEVICE_REG_ENDPOINT5_8_MAP                (USB_INTERNAL_BASE + 0x01A4)

//=============================================================================
/** 0x1A8h
 * FIFO Map Register
 */
//=============================================================================
#define USB_DEVICE_REG_FIFO_MAP                       (USB_INTERNAL_BASE + 0x01A8)

//=============================================================================
/** 0x1ACh
 * FIFO Configuration Register
 */
//=============================================================================
#define USB_DEVICE_REG_FIFO_CONFIG                    (USB_INTERNAL_BASE + 0x01AC)

#define USB_DEVICE_MSK_FIFO_ENABLE                     0x20

//=============================================================================
/** 0x1B0h
 * FIFO x Instruction and byte count Register
 */
//=============================================================================
#define USB_DEVICE_REG_FIFO0_INSTRUCTION_BYTE_CNT     (USB_INTERNAL_BASE + 0x01B0)

#define USB_DEVICE_MSK_FIFO_RESET                      BIT12
#define USB_DEVICE_MSK_FIFO_DONE                       BIT11
#define USB_DEVICE_MSK_FIFO_OUT_BYTE_CNT               0x000007FF

//=============================================================================
/** 0x1C0h
 * Device DMA target FIFO number Register
 */
//=============================================================================
#define USB_DEVICE_REG_DMA_FIFO_NUM                   (USB_INTERNAL_BASE + 0x01C0)

#define USB_DEVICE_MSK_DMA_2_FIFO_CX                   BIT4
#define USB_DEVICE_MSK_DMA_2_FIFO_3                    BIT3
#define USB_DEVICE_MSK_DMA_2_FIFO_2                    BIT2
#define USB_DEVICE_MSK_DMA_2_FIFO_1                    BIT1
#define USB_DEVICE_MSK_DMA_2_FIFO_0                    BIT0
#define USB_DEVICE_MSK_DMA_2_FIFO_NUM                  0x1F

enum
{
    DEVICE_DMA_2_FIFO_NON   = 0x0,
    DEVICE_DMA_2_FIFO_0     = BIT0,
    DEVICE_DMA_2_FIFO_1     = BIT1,
    DEVICE_DMA_2_FIFO_2     = BIT2,
    DEVICE_DMA_2_FIFO_3     = BIT3,
    DEVICE_DMA_2_FIFO_CX    = BIT4
};

//=============================================================================
/** 0x1C8h
 * DMA Controller Parameter setting 1 Register
 */
//=============================================================================
#define USB_DEVICE_REG_DMA_SETTING1                   (USB_INTERNAL_BASE + 0x01C8)

#define USB_DEVICE_MSK_DMA_LENGTH                      0x01FFFF00
#define USB_DEVICE_MSK_CLEAR_FIFO_WHEN_DMA_ABORT       BIT4
#define USB_DEVICE_MSK_DMA_ABORT                       BIT3
#define USB_DEVICE_MSK_DMA_IO_TO_IO                    BIT2
#define USB_DEVICE_MSK_DMA_DIRECTION                   BIT1
#define USB_DEVICE_MSK_DMA_START                       BIT0

#define USB_DEVICE_SHT_DMA_LENGTH                      8

//=============================================================================
/** 0x1CCh
 * DMA Controller Parameter setting 2 Register
 */
//=============================================================================
#define USB_DEVICE_REG_DMA_ADDR                      (USB_INTERNAL_BASE + 0x01CC)

#define USB_DEVICE_REG_DMA_SETTING3                  (USB_INTERNAL_BASE + 0x01D0)

//=============================================================================
/** 0x1D0Ch
 * DMA Controller Parameter setting 3 Register
 */
//=============================================================================
#define USB_DEVICE_REG_SETUP_CMD_READ_PORT           (USB_INTERNAL_BASE + 0x01D0)



#ifdef __cplusplus
}
#endif
	
#endif /* USB_REG_H  */
