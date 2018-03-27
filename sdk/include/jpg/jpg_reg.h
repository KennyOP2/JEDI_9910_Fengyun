/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as JPEG HW register header file.
 *
 * @author 
 * @version 0.01
 */

#ifndef JPG_REG_H
#define JPG_REG_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define REG_JPEG_BASE                              (0x0A00)        /* Base Register Address */


//====================================================================
/*
 * 1. 0x0A00
 *    JPEG Codec Control Register
 */
//====================================================================
#define REG_JPEG_CODEC_CTRL                         (REG_JPEG_BASE + 0x0000)

#define JPEG_MSK_OP_MODE                            (0x0007)
#define JPEG_MSK_DEC_COMPONENT_A_VALID              (0x0008)
#define JPEG_MSK_DEC_COMPONENT_B_VALID              (0x0010)
#define JPEG_MSK_DEC_COMPONENT_C_VALID              (0x0020)
#define JPEG_MSK_DEC_COMPONENT_D_VALID              (0x0040)
#define JPEG_MSK_DEC_COMPONENT_VALID                (0x0078)
#define JPEG_MSK_LINE_BUF_COMPONENT_1_VALID         (0x0080)
#define JPEG_MSK_LINE_BUF_COMPONENT_2_VALID         (0x0100)
#define JPEG_MSK_LINE_BUF_COMPONENT_3_VALID         (0x0200)
#define JPEG_MSK_LINE_BUF_COMPONENT_4_VALID         (0x0400)
#define JPEG_MSK_LINE_BUF_COMPONENT_VALID           (0x0780)
#define JPEG_MSK_ENC_TRIGGER_MODE                   (0x0800)
#define JPEG_MSK_BITSTREAM_READ_BYTE_POS            (0x3000)

typedef enum JPEG_OP_MODE_TAG
{
	JPEG_OP_ENCODE              = 0x0000,
	JPEG_OP_DECODE              = 0x0001,
	JPEG_OP_DECODE_PROGRESSIVE  = 0x0002,
	JPEG_OP_DECODE_DC           = 0x0003,
	JPEG_OP_DECODE_MPEG1        = 0x0004,
	JPEG_OP_DECODE_RGB565       = 0x0005
                              }
                              JPEG_OP_MODE;

typedef enum JPEG_TRIGGER_MODE_TAG
{
	JPEG_ISP_TRIGGER            = 0x0000, // only for decode
	JPEG_COMMAND_TRIGGER        = 0x0800  // 0000 1000 0000 0000
} JPEG_TRIGGER_MODE;

#define JPEG_SHT_LINE_BUF_COMPONENT_VALID          7
#define JPEG_SHT_BITSTREAM_READ_BYTE_POS           12
#define JPEG_DC_MODE_HW_FLAG                       0x4000 // reg[0xA00] => bit[14]

//====================================================================
/*
 * 2. 0x0A02
 *    DRI Setting Register
 */
//====================================================================
#define REG_JPEG_DRI_SETTING                        (REG_JPEG_BASE + 0x0002)


//====================================================================
/*
 * 3. 0x0A04
 *    Table specify register
 */
//====================================================================
#define REG_JPEG_TABLE_SPECIFY                      (REG_JPEG_BASE + 0x0004)

#define JPEG_MSK_COMPONENT_A_AC_HUFFMAN_TABLE       (0x0001)
#define JPEG_MSK_COMPONENT_A_DC_HUFFMAN_TABLE       (0x0002)
#define JPEG_MSK_COMPONENT_A_Q_TABLE                (0x000C)
#define JPEG_MSK_COMPONENT_B_AC_HUFFMAN_TABLE       (0x0010)
#define JPEG_MSK_COMPONENT_B_DC_HUFFMAN_TABLE       (0x0020)
#define JPEG_MSK_COMPONENT_B_Q_TABLE                (0x00C0)
#define JPEG_MSK_COMPONENT_C_AC_HUFFMAN_TABLE       (0x0100)
#define JPEG_MSK_COMPONENT_C_DC_HUFFMAN_TABLE       (0x0200)
#define JPEG_MSK_COMPONENT_C_Q_TABLE                (0x0C00)
#define JPEG_MSK_COMPONENT_D_AC_HUFFMAN_TABLE       (0x1000)
#define JPEG_MSK_COMPONENT_D_DC_HUFFMAN_TABLE       (0x2000)
#define JPEG_MSK_COMPONENT_D_Q_TABLE                (0xC000)

#define JPEG_SHT_COMPONENT_A_AC_HUFFMAN_TABLE       0
#define JPEG_SHT_COMPONENT_A_DC_HUFFMAN_TABLE       1
#define JPEG_SHT_COMPONENT_A_Q_TABLE                2
#define JPEG_SHT_COMPONENT_B_AC_HUFFMAN_TABLE       4
#define JPEG_SHT_COMPONENT_B_DC_HUFFMAN_TABLE       5
#define JPEG_SHT_COMPONENT_B_Q_TABLE                6
#define JPEG_SHT_COMPONENT_C_AC_HUFFMAN_TABLE       8
#define JPEG_SHT_COMPONENT_C_DC_HUFFMAN_TABLE       9
#define JPEG_SHT_COMPONENT_C_Q_TABLE                10
#define JPEG_SHT_COMPONENT_D_AC_HUFFMAN_TABLE       12
#define JPEG_SHT_COMPONENT_D_DC_HUFFMAN_TABLE       13
#define JPEG_SHT_COMPONENT_D_Q_TABLE                14


//====================================================================
/*
 * 4. 0x0A06 ~ 0x0A08
 *    Display MCU Width/Height of 1st (Y) Component
 */
//====================================================================
#define REG_JPEG_DISPLAY_MCU_WIDTH_Y                (REG_JPEG_BASE + 0x0006)
#define REG_JPEG_DISPLAY_MCU_HEIGHT_Y               (REG_JPEG_BASE + 0x0008)

#define JPEG_MSK_MCU                                (0x07FF)

#define JPEG_MSK_TILING_ENABLE                      (0x8000) 
#define JPEG_MSK_NV12_ENABLE                        (0x8000)


//====================================================================
/*
 * 5. 0x0A0A ~ 0x0A14
 *    The Line Buffer Base Address of 1st~3rd Component
 * NOTE: 24 bits of line buffer address in 32-bit unit can address up
 *       to 64M bytes SRAM.
 */
//====================================================================
#define REG_JPEG_LINE_BUF_ADDR_A_COMPONENT_L        (REG_JPEG_BASE + 0x000A)
#define REG_JPEG_LINE_BUF_ADDR_A_COMPONENT_H        (REG_JPEG_BASE + 0x000C)
#define REG_JPEG_LINE_BUF_ADDR_B_COMPONENT_L        (REG_JPEG_BASE + 0x000E)
#define REG_JPEG_LINE_BUF_ADDR_B_COMPONENT_H        (REG_JPEG_BASE + 0x0010)
#define REG_JPEG_LINE_BUF_ADDR_C_COMPONENT_L        (REG_JPEG_BASE + 0x0012)
#define REG_JPEG_LINE_BUF_ADDR_C_COMPONENT_H        (REG_JPEG_BASE + 0x0014)

#define JPEG_MSK_LINE_BUF_ADDR_L                    (0xFFFF)
#define JPEG_MSK_LINE_BUF_ADDR_H                    (0x00FF)


//====================================================================
/*
 * 6. 0x0A16
 *    The Number of Line Buffer in Slice Unit
 */
//====================================================================
#define REG_JPEG_LINE_BUF_SLICE_NUM                 (REG_JPEG_BASE + 0x0016)

#define JPEG_MSK_LINE_BUF_SLICE_NUM                 (0x07FF)


//====================================================================
/*
 * 7. 0x0A18 ~ 0x0A1A
 *    The Pitch of 1st/2nd 3rd Component Line Buffer in 32-bits Unit.
 */
//====================================================================
#define REG_JPEG_LINE_BUF_PITCH_COMPONENT_A         (REG_JPEG_BASE + 0x0018)
#define REG_JPEG_LINE_BUF_PITCH_COMPONENT_BC        (REG_JPEG_BASE + 0x001A)

#define JPEG_MSK_LINE_BUF_PITCH                     (0x0FFF)


//====================================================================
/*
 * 8. 0x0A1C
 *    Line Buf write size Register
 *
 *  Note: In Slice unit.
 *  Note: Only for Encoding and Command mode.
 */
//====================================================================
#define REG_JPEG_LINE_BUF_SLICE_WRITE               (REG_JPEG_BASE + 0x001C)

#define JPEG_MSK_LINE_BUF_SLICE_WRITE               (0x07FF)


//====================================================================
/*
 * 9. 0x0A1E ~ 0x0A20
 *    The Bit-stream Buffer Base Address
 *
 * NOTE: 24 bits of line buffer address in 32-bit unit can address up
 *       to 64M bytes SRAM.
 */
//====================================================================
#define REG_JPEG_BITSTREAM_BUF_ADDR_L               (REG_JPEG_BASE + 0x001E)
#define REG_JPEG_BITSTREAM_BUF_ADDR_H               (REG_JPEG_BASE + 0x0020)

#define JPEG_MSK_BITSTREAM_BUF_ADDR_L               (0xFFFF)
#define JPEG_MSK_BITSTREAM_BUF_ADDR_H               (0x00FF)


//====================================================================
/*
 * 10. 0x0A22 ~ 0x0A24
 *    The Bit-stream Buffer size
 *
 *  Note: In 32-bits unit. Byte address must be divided by 4
 *        to transfer to 32-bits unit.
 */
//====================================================================
#define REG_JPEG_BITSTREAM_BUF_SIZE_L               (REG_JPEG_BASE + 0x0022)
#define REG_JPEG_BITSTREAM_BUF_SIZE_H               (REG_JPEG_BASE + 0x0024)

#define JPEG_MSK_BITSTREAM_BUF_SIZE_L               (0xFFFF)
#define JPEG_MSK_BITSTREAM_BUF_SIZE_H               (0x00FF)


//====================================================================
/*
 * 11. 0x0A26 ~ 0x0A28
 *    BitStream R/W Size Register
 *
 *  Note: In 32-bits unit. Byte size must be divided by 4
 *        to transfer to 32-bits unit.
 */
//====================================================================
#define REG_JPEG_BITSTREAM_RW_SIZE_L               (REG_JPEG_BASE + 0x0026)
#define REG_JPEG_BITSTREAM_RW_SIZE_H               (REG_JPEG_BASE + 0x0028)

#define JPEG_MSK_BITSTREAM_RW_SIZE_L               (0xFFFF)
#define JPEG_MSK_BITSTREAM_RW_SIZE_H               (0x00FF)


//====================================================================
/*
 * 12. 0x0A2A ~ 0x0A2C
 *    Horizontal, Vertical sampling factor
 */
//====================================================================
#define REG_JPEG_SAMPLING_FACTOR_AB                 (REG_JPEG_BASE + 0x002A)
#define REG_JPEG_SAMPLING_FACTOR_CD                 (REG_JPEG_BASE + 0x002C)

#define JPEG_MSK_SAMPLING_FACTOR_A_H                (0x000F)
#define JPEG_MSK_SAMPLING_FACTOR_A_V                (0x00F0)
#define JPEG_MSK_SAMPLING_FACTOR_B_H                (0x0F00)
#define JPEG_MSK_SAMPLING_FACTOR_B_V                (0xF000)

#define JPEG_MSK_SAMPLING_FACTOR_C_H                (0x000F)
#define JPEG_MSK_SAMPLING_FACTOR_C_V                (0x00F0)
#define JPEG_MSK_SAMPLING_FACTOR_D_H                (0x0F00)
#define JPEG_MSK_SAMPLING_FACTOR_D_V                (0xF000)

#define JPEG_MSK_INTERRUPT_ENCODE_FULL              (0x4000)
#define JPEG_MSK_INTERRUPT_ENCODE_END               (0x0200)

#define JPEG_SHT_SAMPLING_FACTOR_A_H                0
#define JPEG_SHT_SAMPLING_FACTOR_A_V                4
#define JPEG_SHT_SAMPLING_FACTOR_B_H                8
#define JPEG_SHT_SAMPLING_FACTOR_B_V                12

#define JPEG_SHT_SAMPLING_FACTOR_C_H                0
#define JPEG_SHT_SAMPLING_FACTOR_C_V                4
#define JPEG_SHT_SAMPLING_FACTOR_D_H                8
#define JPEG_SHT_SAMPLING_FACTOR_D_V                12


//====================================================================
/*
 * 13. 0x0A2E ~ 0x0A6D, 0x0A6E ~ 0x0AAD, 0x0AAE ~ 0x0AED
 *    Index 0/1/2 Q-Table Register
 */
//====================================================================
#define REG_JPEG_INDEX0_QTABLE                      (REG_JPEG_BASE + 0x002E)
#define REG_JPEG_INDEX1_QTABLE                      (REG_JPEG_BASE + 0x006E)
#define REG_JPEG_INDEX2_QTABLE                      (REG_JPEG_BASE + 0x00AE)


//====================================================================
/*
 * 14. 0x0AEE
 *    Component Drop and Duplicate Specify Register
 */
//====================================================================
#define REG_JPEG_DROP_DUPLICATE                     (REG_JPEG_BASE + 0x00EE)

#define JPEG_MSK_DROP_H_V                           0x0333
#define JPEG_MSK_DUPLICATE_H_V                      0xBBBB


//====================================================================
/*
 * 15. 0x0AF0 ~ 0x0AF2
 *    Original MCU Width/Height of 1st (Y) Component
 */
//====================================================================
#define REG_JPEG_ORIGINAL_MCU_WIDTH                 (REG_JPEG_BASE + 0x00F0)
#define REG_JPEG_ORIGINAL_MCU_HEIGHT                (REG_JPEG_BASE + 0x00F2)


//====================================================================
/*
 * 16. 0x0AF4 ~ 0x0AFA
 *    Partial display MCU offset of 1st (Y) Component
 */
//====================================================================
#define REG_JPEG_LEFT_MCU_OFFSET                    (REG_JPEG_BASE + 0x00F4)
#define REG_JPEG_RIGHT_MCU_OFFSET                   (REG_JPEG_BASE + 0x00F6)
#define REG_JPEG_UP_MCU_OFFSET                      (REG_JPEG_BASE + 0x00F8)
#define REG_JPEG_DOWN_MCU_OFFSET                    (REG_JPEG_BASE + 0x00FA)

// 0xAF4
#define JPEG_MSK_MCU_HEIGHT_BLOCK                   (0x3800)
#define JPEG_SHT_MCU_HEIGHT_BLOCK                   11
// 0xAF6
#define JPEG_MSK_BLOCK_MCU_NUM                      (0x7800)
#define JPEG_SHT_BLOCK_MCU_NUM                      11


//====================================================================
/*
 * 17. 0x0AFC
 *    Start/Reset Register
 */
//====================================================================
#define REG_JPEG_CODEC_FIRE                         (REG_JPEG_BASE + 0x00FC)

#define JPEG_MSK_START_CODEC                        (0x0001)
#define JPEG_MSK_RESET                              (0x0100)

#define JPEG_MSK_INTERRUPT_CLEAR                    (0x8000)


//====================================================================
/*
 * 18. 0x0AFE ~ 0xB00
 *    JPEG Engine Status Register
 */
//====================================================================
#define REG_JPEG_ENGINE_STATUS_0                    (REG_JPEG_BASE + 0x00FE)

#define REG_JPEG_ENGINE_STATUS_1                    (REG_JPEG_BASE + 0x0100)

#define JPEG_STATUS_DECODE_COMPLETE                 (0x0001)
#define JPEG_STATUS_ENCODE_COMPLETE                 (0x0002)
#define JPEG_STATUS_DECODE_ERROR                    (0x0004)
#define JPEG_STATUS_LINE_BUF_EMPTY                  (0x0010)
#define JPEG_STATUS_LINE_BUF_FULL                   (0x0020)
#define JPEG_STATUS_BITSTREAM_BUF_EMPTY             (0x0040)
#define JPEG_STATUS_BITSTREAM_BUF_FULL              (0x0080)


//====================================================================
/*
 * 19. 0xB02
 *    Line Buffer Control Register
 */
//====================================================================
#define REG_JPEG_LINE_BUF_CTRL                      (REG_JPEG_BASE + 0x0102)

#define JPEG_MSK_LINE_BUF_CTRL                      (0x0007)
#define JPEG_MSK_LINE_BUF_RESET                     (0x0001)
#define JPEG_MSK_LINE_BUF_WRITE_END                 (0x0002)
#define JPEG_MSK_LAST_ENCODE_DATA                   (0x0004)

#define JPEG_LINE_BUF_CTRL_CLEAN                    (0x0000)


//====================================================================
/*
 * 19. 0xB04
 *    Valid Number for Line Buffer in Slice Unit.
 */
//====================================================================
#define REG_JPEG_LINE_BUF_VALID_SLICE               (REG_JPEG_BASE + 0x0104)

#define JPEG_MSK_LINE_BUF_VALID_SLICE               (0x7FF)


//====================================================================
/*
 * 20. 0xB06
 *    BitStream Buffer Control Register
 */
//====================================================================
#define REG_JPEG_BITSTREAM_BUF_CTRL                 (REG_JPEG_BASE + 0x0106)

#define JPEG_MSK_BITSTREAM_BUF_CTRL                 (0x0007)
#define JPEG_MSK_BITSTREAM_BUF_RESET                (0x0001)
#define JPEG_MSK_BITSTREAM_BUF_RW_END            (0x0002)
#define JPEG_MSK_LAST_BITSTREAM_DATA                (0x0004)

#define JPEG_BITSTREAM_BUF_CTRL_CLEAN               (0x0000)


//====================================================================
/*
 * 21. 0xB08 ~ 0xB0A
 *    BitStream Buf Valid Size Register
 *
 *  Note: In 32-bit unit. Byte size must be divided by 4
 *        to transfer to 32-bit unit.
 */
//====================================================================
#define REG_JPEG_BITSTREAM_VALID_SIZE_L             (REG_JPEG_BASE + 0x0108)
#define REG_JPEG_BITSTREAM_VALID_SIZE_H             (REG_JPEG_BASE + 0x010A)

#define JPEG_MSK_BITSTREAM_VALID_SIZE_L             (0xFFFF)
#define JPEG_MSK_BITSTREAM_VALID_SIZE_H             (0x00FF)


//====================================================================
/*
 * 22. 0xB0C
 *    Number of Huffman Codes of each Length
 */
//====================================================================
#define REG_JPEG_HUFFMAN_CTRL                       (REG_JPEG_BASE + 0x010C)

#define JPEG_MSK_HUFFMAN_LEN_NUM                    (0x00FF)
#define JPEG_MSK_HUFFMAN_LEN_ID                     (0x0F00)

#define JPEG_SHT_HUFFMAN_LEN_ID                     8

#define JPEG_HUFFMAN_LUMINANCE                      (0x0000)
#define JPEG_HUFFMAN_CHROMINANCE                    (0x1000)
#define JPEG_HUFFMAN_DC_TABLE                       (0x0000)
#define JPEG_HUFFMAN_AC_TABLE                       (0x2000)

/**
 *  JPEG Huffman Table selection
 */
typedef enum JPEG_HUFFMAN_TABLE_SELECTION_TAG
{
	JPEG_HUUFFMAN_Y_DC  = 0,
	JPEG_HUUFFMAN_UV_DC = 1,
	JPEG_HUUFFMAN_Y_AC  = 2,
	JPEG_HUUFFMAN_UV_AC = 3
} JPEG_HUFFMAN_TABLE_SELECTION;


//====================================================================
/*
 * 23. 0xB0E
 *    DC value of each Huffman Code
 */
//====================================================================
#define REG_JPEG_HUFFMAN_DC_CTRL                    (REG_JPEG_BASE + 0x010E)

#define JPEG_MSK_HUFFMAN_DC_VALUE                   (0x00FF)
#define JPEG_MSK_HUFFMAN_DC_LEN_ID                  (0x0700)

#define JPEG_SHT_HUFFMAN_DC_LEN_ID                  8

#define JPEG_HUFFMAN_DC_LUMINANCE_TABLE             (0x0000)
#define JPEG_HUFFMAN_DC_CHROMINANCE_TABLE           (0x0800)


//====================================================================
/*
 * 24. 0xB10
 *    AC Luminance value of each Huffman Code
 */
//====================================================================
#define REG_JPEG_HUFFMAN_AC_LUMINANCE_CTRL          (REG_JPEG_BASE + 0x0110)


//====================================================================
/*
 * 25. 0xB12
 *    AC Chrominance value of each Huffman Code
 */
//====================================================================
#define REG_JPEG_HUFFMAN_AC_CHROMINANCE_CTRL        (REG_JPEG_BASE + 0x0112)


//====================================================================
/*
 * 26. 0xF26 ~ 0xF38
 *    Color matrix Jpg engine output RGB565 format
 */
//====================================================================
#define REG_JPEG_YUV_TO_RGB_11                      0xF26
#define REG_JPEG_YUV_TO_RGB_13                      0xF28
#define REG_JPEG_YUV_TO_RGB_21                      0xF2A
#define REG_JPEG_YUV_TO_RGB_22                      0xF2C
#define REG_JPEG_YUV_TO_RGB_23                      0xF2E
#define REG_JPEG_YUV_TO_RGB_31                      0xF30
#define REG_JPEG_YUV_TO_RGB_32                      0xF32
#define REG_JPEG_YUV_TO_RGB_CONST_R                 0xF34
#define REG_JPEG_YUV_TO_RGB_CONST_G                 0xF36
#define REG_JPEG_YUV_TO_RGB_CONST_B                 0xF38


//====================================================================
/*
 * 27. 0xF00 ~ 0xF04, 0xF5A
 *    Jpg engine use color key when outputing RGB565 format
 */
//====================================================================
#define REG_JPEG_EN_DITHER_KEY                      0xF5A
#define REG_JPEG_SET_DITHER_KEY                     0xF00
#define REG_JPEG_SET_MASK_DITHER_KEY                0xF02
#define REG_JPEG_SET_DITHER_KEY_BG                  0xF04





#ifdef __cplusplus
}
#endif

#endif //JPG_REG_H
