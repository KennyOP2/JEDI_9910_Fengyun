/*
 * Copyright (c) 2005 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  JPEG Engine HW setting
 *
 * @author Irene Lin
 * @version 0.01
 */

//=============================================================================
//                              Include Files
//=============================================================================

#include "host/host.h"
#include "sys/sys.h"

#include "jpg/jpg_reg.h"
#include "jpg/jpg_type.h"


//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

// 0x00
void JPEG_SetCodecCtrlReg(MMP_UINT16 data)
{
	HOST_WriteRegisterMask(REG_JPEG_CODEC_CTRL, (MMP_UINT16)(data), (MMP_UINT16)(~JPEG_MSK_BITSTREAM_READ_BYTE_POS));
}

void JPEG_SetBitstreamReadBytePosReg(MMP_UINT16 data)
{
	HOST_WriteRegisterMask(REG_JPEG_CODEC_CTRL,
	                       (MMP_UINT16)(data << JPEG_SHT_BITSTREAM_READ_BYTE_POS),
	                       JPEG_MSK_BITSTREAM_READ_BYTE_POS);
}

// 0x02
void JPEG_SetDriReg(MMP_UINT16 data)
{
	HOST_WriteRegister(REG_JPEG_DRI_SETTING, (MMP_UINT16)(data));
}

// 0x04
void JPEG_SetTableSpecifyReg(const JPEG_IMAGE_COMPONENT *imageComponent)
{
	MMP_UINT16 data = 0;

	// Hw issue
	MMP_UINT16  qTableSel_Y  = 0x0;  // 00
	MMP_UINT16  qTableSel_UV = 0x1;  // 01

	data = (imageComponent->imageInfo[0].acHuffmanTableSel << JPEG_SHT_COMPONENT_A_AC_HUFFMAN_TABLE) |
	       (imageComponent->imageInfo[0].dcHuffmanTableSel << JPEG_SHT_COMPONENT_A_DC_HUFFMAN_TABLE) |
	       (qTableSel_Y << JPEG_SHT_COMPONENT_A_Q_TABLE) |
	       (imageComponent->imageInfo[1].acHuffmanTableSel << JPEG_SHT_COMPONENT_B_AC_HUFFMAN_TABLE) |
	       (imageComponent->imageInfo[1].dcHuffmanTableSel << JPEG_SHT_COMPONENT_B_DC_HUFFMAN_TABLE) |
	       (qTableSel_UV << JPEG_SHT_COMPONENT_B_Q_TABLE) |
	       (imageComponent->imageInfo[2].acHuffmanTableSel << JPEG_SHT_COMPONENT_C_AC_HUFFMAN_TABLE) |
	       (imageComponent->imageInfo[2].dcHuffmanTableSel << JPEG_SHT_COMPONENT_C_DC_HUFFMAN_TABLE) |
	       (qTableSel_UV << JPEG_SHT_COMPONENT_C_Q_TABLE) |
	       (imageComponent->imageInfo[3].acHuffmanTableSel << JPEG_SHT_COMPONENT_D_AC_HUFFMAN_TABLE) |
	       (imageComponent->imageInfo[3].dcHuffmanTableSel << JPEG_SHT_COMPONENT_D_DC_HUFFMAN_TABLE) |
	       (imageComponent->imageInfo[3].qTableSel << JPEG_SHT_COMPONENT_D_Q_TABLE);
	HOST_WriteRegister(REG_JPEG_TABLE_SPECIFY, data);
}

// 0x06, 0x08, 0xF0~0xFA
void JPEG_SetImageSizeInfoReg(const JPEG_IMAGE_SIZE_INFO *sizeInfo)
{
	HOST_WriteRegisterMask(REG_JPEG_DISPLAY_MCU_WIDTH_Y,  sizeInfo->mcuDispWidth,  JPEG_MSK_MCU);
	HOST_WriteRegisterMask(REG_JPEG_DISPLAY_MCU_HEIGHT_Y, sizeInfo->mcuDispHeight, JPEG_MSK_MCU);

	HOST_WriteRegisterMask(REG_JPEG_ORIGINAL_MCU_WIDTH,  sizeInfo->mcuRealWidth,  JPEG_MSK_MCU);
	HOST_WriteRegisterMask(REG_JPEG_ORIGINAL_MCU_HEIGHT, sizeInfo->mcuRealHeight, JPEG_MSK_MCU);

	HOST_WriteRegisterMask(REG_JPEG_LEFT_MCU_OFFSET,  (MMP_UINT16)(sizeInfo->mcuDispLeft + 1),  JPEG_MSK_MCU);
	HOST_WriteRegisterMask(REG_JPEG_RIGHT_MCU_OFFSET, sizeInfo->mcuDispRight, JPEG_MSK_MCU);
	HOST_WriteRegisterMask(REG_JPEG_UP_MCU_OFFSET,    (MMP_UINT16)(sizeInfo->mcuDispUp + 1),    JPEG_MSK_MCU);
	HOST_WriteRegisterMask(REG_JPEG_DOWN_MCU_OFFSET,  sizeInfo->mcuDispDown,  JPEG_MSK_MCU);
}

void JPEG_SetTilingMode(void)
{
	HOST_WriteRegisterMask(REG_JPEG_DISPLAY_MCU_WIDTH_Y,  JPEG_MSK_TILING_ENABLE,  JPEG_MSK_TILING_ENABLE);
	HOST_WriteRegisterMask(REG_JPEG_DISPLAY_MCU_HEIGHT_Y, JPEG_MSK_NV12_ENABLE, JPEG_MSK_NV12_ENABLE);
}

// 0x0A~0x1A
void JPEG_SetLineBufInfoReg(const JPEG_LINE_BUF_INFO *bufInfo)
{
	MMP_UINT32 addr = 0;

	// component 1 starting address
	if (bufInfo->component1Addr != MMP_NULL)
	{
		addr = (MMP_UINT32)bufInfo->component1Addr - (MMP_UINT32)HOST_GetVramBaseAddress();
		addr >>= 2;
	}
	else
	{
		addr = 0;
	}
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_ADDR_A_COMPONENT_L, (MMP_UINT16)(addr & JPEG_MSK_LINE_BUF_ADDR_L), JPEG_MSK_LINE_BUF_ADDR_L);
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_ADDR_A_COMPONENT_H, (MMP_UINT16)((addr >> 16) & JPEG_MSK_LINE_BUF_ADDR_H), JPEG_MSK_LINE_BUF_ADDR_H);

	// component 2 starting address
	if (bufInfo->component2Addr != MMP_NULL)
	{
		addr = (MMP_UINT32)bufInfo->component2Addr - (MMP_UINT32)HOST_GetVramBaseAddress();
		addr >>= 2;
	}
	else
	{
		addr = 0;
	}
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_ADDR_B_COMPONENT_L, (MMP_UINT16)(addr & JPEG_MSK_LINE_BUF_ADDR_L), JPEG_MSK_LINE_BUF_ADDR_L);
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_ADDR_B_COMPONENT_H, (MMP_UINT16)((addr >> 16) & JPEG_MSK_LINE_BUF_ADDR_H), JPEG_MSK_LINE_BUF_ADDR_H);

	// component 3 starting address
	if (bufInfo->component3Addr != MMP_NULL)
	{
		addr = (MMP_UINT32)bufInfo->component3Addr - (MMP_UINT32)HOST_GetVramBaseAddress();
		addr >>= 2;
	}
	else
	{
		addr = 0;
	}
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_ADDR_C_COMPONENT_L, (MMP_UINT16)(addr & JPEG_MSK_LINE_BUF_ADDR_L), JPEG_MSK_LINE_BUF_ADDR_L);
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_ADDR_C_COMPONENT_H, (MMP_UINT16)((addr >> 16) & JPEG_MSK_LINE_BUF_ADDR_H), JPEG_MSK_LINE_BUF_ADDR_H);

	// component 1 pitch
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_PITCH_COMPONENT_A,
	                       (MMP_UINT16)((bufInfo->component1Pitch >> 2) & JPEG_MSK_LINE_BUF_PITCH),
	                       JPEG_MSK_LINE_BUF_PITCH);
	// component 2/3 pitch
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_PITCH_COMPONENT_BC,
	                       (MMP_UINT16)((bufInfo->component23Pitch >> 2) & JPEG_MSK_LINE_BUF_PITCH),
	                       JPEG_MSK_LINE_BUF_PITCH);
}

// 0x16
void JPEG_SetLineBufSliceUnitReg(MMP_UINT16 data, MMP_UINT16 yVerticalSamp)
{

	
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_SLICE_NUM,
	                       (MMP_UINT16)(data & JPEG_MSK_LINE_BUF_SLICE_NUM),
	                       JPEG_MSK_LINE_BUF_SLICE_NUM);
	
	//HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_SLICE_NUM,
	//                       (MMP_UINT16)((data / yVerticalSamp) & JPEG_MSK_LINE_BUF_SLICE_NUM),
	//                       JPEG_MSK_LINE_BUF_SLICE_NUM);
}

// 0x1C
void JPEG_SetLineBufSliceWriteNumReg(MMP_UINT16 data)
{
	HOST_WriteRegisterMask(REG_JPEG_LINE_BUF_SLICE_WRITE,
	                       (MMP_UINT16)(data & JPEG_MSK_LINE_BUF_SLICE_WRITE),
	                       JPEG_MSK_LINE_BUF_SLICE_WRITE);
}

// 0x1E, 0x20, 0x22, 0x24
void JPEG_SetBitStreamBufInfoReg(const JPEG_BITSTREAM_BUF_INFO *bufInfo)
{
	MMP_UINT32 addr = 0;
	MMP_UINT32 size = 0;

	/**
	 * Bit-Stream buffer starting address
	 */
	if (bufInfo->addr != MMP_NULL)
	{
		addr = (MMP_UINT32)bufInfo->addr - (MMP_UINT32)HOST_GetVramBaseAddress();
		addr >>= 2;
	}
	else
	{
		addr = 0;
	}
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_BUF_ADDR_L, (MMP_UINT16)(addr & JPEG_MSK_BITSTREAM_BUF_ADDR_L), JPEG_MSK_BITSTREAM_BUF_ADDR_L);
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_BUF_ADDR_H, (MMP_UINT16)((addr >> 16) & JPEG_MSK_BITSTREAM_BUF_ADDR_H), JPEG_MSK_BITSTREAM_BUF_ADDR_H);

	/**
	 * Bit-Stream buffer size
	 */
	size = bufInfo->size >> 2;
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_BUF_SIZE_L, (MMP_UINT16)(size & JPEG_MSK_BITSTREAM_BUF_SIZE_L), JPEG_MSK_BITSTREAM_BUF_SIZE_L);
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_BUF_SIZE_H, (MMP_UINT16)((size >> 16) & JPEG_MSK_BITSTREAM_BUF_SIZE_H), JPEG_MSK_BITSTREAM_BUF_SIZE_H);
}

// 0x26, 0x28
void JPEG_SetBitstreamBufRwSizeReg(MMP_UINT32 data)
{
	MMP_UINT32 wrSize = data >> 2;
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_RW_SIZE_L, (MMP_UINT16)(wrSize & JPEG_MSK_BITSTREAM_RW_SIZE_L), JPEG_MSK_BITSTREAM_RW_SIZE_L);
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_RW_SIZE_H, (MMP_UINT16)((wrSize >> 16) & JPEG_MSK_BITSTREAM_RW_SIZE_H), JPEG_MSK_BITSTREAM_RW_SIZE_H);
}

// 0x2A, 0x2D
void JPEG_SetSamplingFactorReg(const JPEG_IMAGE_COMPONENT *imageComponent)
{
	MMP_UINT16 data = 0;
	data = (imageComponent->imageInfo[0].horizonSamp  << JPEG_SHT_SAMPLING_FACTOR_A_H) |
	       (imageComponent->imageInfo[0].verticalSamp << JPEG_SHT_SAMPLING_FACTOR_A_V) |
	       (imageComponent->imageInfo[1].horizonSamp  << JPEG_SHT_SAMPLING_FACTOR_B_H) |
	       (imageComponent->imageInfo[1].verticalSamp << JPEG_SHT_SAMPLING_FACTOR_B_V);
	HOST_WriteRegister(REG_JPEG_SAMPLING_FACTOR_AB, data);

	data = 0;
	//data = (imageComponent->imageInfo[2].horizonSamp  << JPEG_SHT_SAMPLING_FACTOR_C_H) |
	//       (imageComponent->imageInfo[2].verticalSamp << JPEG_SHT_SAMPLING_FACTOR_C_V) |
	//       (imageComponent->imageInfo[3].horizonSamp  << JPEG_SHT_SAMPLING_FACTOR_D_H) |
	//       (imageComponent->imageInfo[3].verticalSamp << JPEG_SHT_SAMPLING_FACTOR_D_V);
	//HOST_WriteRegister(REG_JPEG_SAMPLING_FACTOR_CD, data);
	       
	data = (imageComponent->imageInfo[2].horizonSamp  << JPEG_SHT_SAMPLING_FACTOR_C_H) |
	       (imageComponent->imageInfo[2].verticalSamp << JPEG_SHT_SAMPLING_FACTOR_C_V);	      
	
	HOST_WriteRegisterMask(REG_JPEG_SAMPLING_FACTOR_CD, data, (JPEG_MSK_SAMPLING_FACTOR_C_H | JPEG_MSK_SAMPLING_FACTOR_C_V));

	/**
	 * Set MCU block height
	 */
	HOST_WriteRegisterMask(REG_JPEG_LEFT_MCU_OFFSET,
	                       (MMP_UINT16)(imageComponent->imageInfo[0].verticalSamp << JPEG_SHT_MCU_HEIGHT_BLOCK),
	                       JPEG_MSK_MCU_HEIGHT_BLOCK);

	/**
	 * Set MCU block number
	 */
	data = (imageComponent->imageInfo[0].horizonSamp * imageComponent->imageInfo[0].verticalSamp) +
	       (imageComponent->imageInfo[1].horizonSamp * imageComponent->imageInfo[1].verticalSamp) +
	       (imageComponent->imageInfo[2].horizonSamp * imageComponent->imageInfo[2].verticalSamp) - 1;
	HOST_WriteRegisterMask(REG_JPEG_RIGHT_MCU_OFFSET,
	                       (MMP_UINT16)(data << JPEG_SHT_BLOCK_MCU_NUM),
	                       JPEG_MSK_BLOCK_MCU_NUM);
}

// 0x2E~0xED
// input the Zig-zag order
void JPEG_SetQtableReg(JPEG_Q_TABLE *qTable)
{
	MMP_UINT8 i = 0;
	MMP_UINT8 j = 0;
	MMP_UINT8 *table = MMP_NULL;
	MMP_UINT16 reg = REG_JPEG_INDEX0_QTABLE;

	for (j = 0; j < qTable->tableCount; j++)
	{
		table = qTable->Table[ctxt->imageComponent.imageInfo[j].qTableSel];
		switch (j)
		{
			case 0:
				reg = REG_JPEG_INDEX0_QTABLE;
				break;

			case 1:
				reg = REG_JPEG_INDEX1_QTABLE;
				break;

			case 2:
				reg = REG_JPEG_INDEX2_QTABLE;
				break;
		}

		for (i = 0; i < JPEG_Q_TABLE_SIZE; i += 2)
		{
			HOST_WriteRegister((MMP_UINT16)(reg + i), (MMP_UINT16)(table[i] | (table[i+1] << 8)));
		}
	}
}

// 0xEE
void JPEG_DropHv(MMP_UINT16 data)
{
	HOST_WriteRegister(REG_JPEG_DROP_DUPLICATE, (MMP_UINT16)(data & JPEG_MSK_DROP_H_V));
}

// 0xFC
void JPEG_StartReg(void)
{
	HOST_WriteRegister(REG_JPEG_CODEC_FIRE, (MMP_UINT16)JPEG_MSK_START_CODEC);
}

// 0xFE
MMP_UINT16 JPEG_GetEngineStatusReg(void)
{
	MMP_UINT16 value = 0;

	HOST_ReadRegister(REG_JPEG_ENGINE_STATUS_0,  &value);

	return value;
}

// 0x100
MMP_UINT16 JPEG_GetEngineStatus1Reg(void)
{
	MMP_UINT16 value = 0;

	HOST_ReadRegister(REG_JPEG_ENGINE_STATUS_1,  &value);

	return value;
}

// 0x102
void JPEG_SetLineBufCtrlReg(MMP_UINT16 data)
{
	HOST_WriteRegister(REG_JPEG_LINE_BUF_CTRL, (MMP_UINT16)(data & JPEG_MSK_LINE_BUF_CTRL));
}

// 0x104
MMP_UINT16 JPEG_GetLineBufValidSliceReg(void)
{
	MMP_UINT16 value = 0;

	HOST_ReadRegister(REG_JPEG_LINE_BUF_VALID_SLICE, &value);

	return (MMP_UINT16)(value & JPEG_MSK_LINE_BUF_VALID_SLICE);
}

// 0x106
void JPEG_SetBitstreamBufCtrlReg(MMP_UINT16 data)
{
	HOST_WriteRegisterMask(REG_JPEG_BITSTREAM_BUF_CTRL, (MMP_UINT16)(data & JPEG_MSK_BITSTREAM_BUF_CTRL), JPEG_MSK_BITSTREAM_BUF_CTRL);
}

// 0x108
MMP_UINT32 JPEG_GetBitStreamValidSizeReg(void)
{
	volatile MMP_UINT16 low = 0;
	volatile MMP_UINT16 high = 0;
	MMP_UINT32 size = 0;

	HOST_ReadRegister(REG_JPEG_BITSTREAM_VALID_SIZE_L, (MMP_UINT16 *)&low);
	HOST_ReadRegister(REG_JPEG_BITSTREAM_VALID_SIZE_H, (MMP_UINT16 *)&high);
	size = ((((high & JPEG_MSK_BITSTREAM_VALID_SIZE_H) << 16) | low) << 2);

	return (size);
}

// 0x10C
void
JPEG_SetHuffmanCodeCtrlReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeLength)
{
	MMP_UINT16 Selection = 0x0000;
	MMP_UINT16 data = 0x0000;
	MMP_UINT16 i = 0;

	switch (tableSelection)
	{
		case (JPEG_HUUFFMAN_Y_DC):
			Selection = (JPEG_HUFFMAN_DC_TABLE | JPEG_HUFFMAN_LUMINANCE);
			break;

		case (JPEG_HUUFFMAN_UV_DC):
			Selection = (JPEG_HUFFMAN_DC_TABLE | JPEG_HUFFMAN_CHROMINANCE);
			break;

		case (JPEG_HUUFFMAN_Y_AC):
			Selection = (JPEG_HUFFMAN_AC_TABLE | JPEG_HUFFMAN_LUMINANCE);
			break;

		case (JPEG_HUUFFMAN_UV_AC):
			Selection = (JPEG_HUFFMAN_AC_TABLE | JPEG_HUFFMAN_CHROMINANCE);
			break;

		default:
			return;
	}
	// Reset
	HOST_WriteRegister(REG_JPEG_HUFFMAN_CTRL,  (MMP_UINT16)(Selection));
	sdk_msg(SDK_MSG_TYPE_JPG_DATA, "\n\nReg = 0x%X, %s [#%d]\n", REG_JPEG_HUFFMAN_CTRL, __FILE__, __LINE__);

	// Write Length number
	for (i = 1; i < 16; i++)
	{
		data = (Selection | (i << 8) | (*(pCodeLength + i - 1)));
		HOST_WriteRegister(REG_JPEG_HUFFMAN_CTRL,  (MMP_UINT16)(data));
		{
			if (!(i % 10) && i != 0)
				sdk_msg(SDK_MSG_TYPE_JPG_DATA, " \n");
			sdk_msg(SDK_MSG_TYPE_JPG_DATA, "0x%04X ", data);
		}
	}
}

// 0xB0E
void
JPEG_SetDcHuffmanValueReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeValue,
    MMP_UINT16 totalCodeLengthCount)
{
	MMP_UINT16 data = 0x0000;
	MMP_UINT16 wTotalHTNum = 0x0000;
	MMP_UINT16 i = 0;

	switch (tableSelection)
	{
		case (JPEG_HUUFFMAN_Y_DC):
			wTotalHTNum = (totalCodeLengthCount + 1) >> 1;
			sdk_msg(SDK_MSG_TYPE_JPG_DATA, "\n\nJPEG_HUUFFMAN_Y_DC: reg = 0x%X, %s [#%d]\n", REG_JPEG_HUFFMAN_DC_CTRL, __FILE__, __LINE__);

			for (i = 0; i < wTotalHTNum; i++)
			{
				// Encode and Decode are the same
				if ( (totalCodeLengthCount % 2) && (i == (wTotalHTNum - 1)) )
				{
					data = (JPEG_HUFFMAN_DC_LUMINANCE_TABLE | (i << 8) | (0 << 4) | (*(pCodeValue + 2 * i)));
				}
				else
				{
					data = (JPEG_HUFFMAN_DC_LUMINANCE_TABLE | (i << 8) | ((*(pCodeValue + 2 * i + 1)) << 4) | (*(pCodeValue + 2 * i)));
				}

				HOST_WriteRegister(REG_JPEG_HUFFMAN_DC_CTRL,  (MMP_UINT16)(data));

				/*
				{
				    if (!(i % 10) && i != 0)
				        sdk_msg(SDK_MSG_TYPE_JPG_DATA, " \n");
				    sdk_msg(SDK_MSG_TYPE_JPG_DATA, "0x%04X ", data);
				}
				*/
			}
			break;

		case (JPEG_HUUFFMAN_UV_DC):
			wTotalHTNum = (totalCodeLengthCount + 1) / 2;
			sdk_msg(SDK_MSG_TYPE_JPG_DATA, "\n\nJPEG_HUUFFMAN_UV_DC: reg = 0x%X, %s [#%d]\n", REG_JPEG_HUFFMAN_DC_CTRL, __FILE__, __LINE__);
			for (i = 0; i < wTotalHTNum; i++)
			{
				// Encode and Decode are the same
				if ( (totalCodeLengthCount % 2) && (i == (wTotalHTNum - 1)) )
				{
					data = (JPEG_HUFFMAN_DC_CHROMINANCE_TABLE | (i << 8) | (0 << 4) | (*(pCodeValue + 2 * i)));
				}
				else
				{
					data = (JPEG_HUFFMAN_DC_CHROMINANCE_TABLE | (i << 8) | ((*(pCodeValue + 2 * i + 1)) << 4) | (*(pCodeValue + 2 * i)));
				}

				HOST_WriteRegister(REG_JPEG_HUFFMAN_DC_CTRL,  (MMP_UINT16)(data));

				/*
				{
				    if (!(i % 10) && i != 0)
				        sdk_msg(SDK_MSG_TYPE_JPG_DATA, " \n");
				    sdk_msg(SDK_MSG_TYPE_JPG_DATA, "0x%04X ", data);
				}
				*/
			}
			break;

		default:
			break;
	}
}

// 0x110
// 0x112
void
JPEG_SetEncodeAcHuffmanValueReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeValue,
    MMP_UINT16 totalCodeLengthCount)
{
	MMP_UINT16 data = 0x0000;
	MMP_UINT16 wTotalHTNum = 0x0000;
	MMP_UINT16 i = 0;

	switch (tableSelection)
	{
		case (JPEG_HUUFFMAN_Y_AC):
			wTotalHTNum = totalCodeLengthCount;
			sdk_msg(SDK_MSG_TYPE_JPG_DATA, "\n\nJPEG_HUUFFMAN_Y_AC: reg = 0x%X, %s [#%d]\n", REG_JPEG_HUFFMAN_AC_LUMINANCE_CTRL, __FILE__, __LINE__);

			for (i = 0; i < wTotalHTNum; i++)
			{
				// Encode and Decode are different
				data = ((*(pCodeValue + i) << 8) | i);
				HOST_WriteRegister(REG_JPEG_HUFFMAN_AC_LUMINANCE_CTRL,  (MMP_UINT16)(data));
				/*
				{
				    if (!(i % 10) && i != 0)
				        sdk_msg(SDK_MSG_TYPE_JPG_DATA, " \n");
				    sdk_msg(SDK_MSG_TYPE_JPG_DATA, "0x%04X ", data);
				}
				*/
			}
			break;

		case (JPEG_HUUFFMAN_UV_AC):
			wTotalHTNum = totalCodeLengthCount;
			sdk_msg(SDK_MSG_TYPE_JPG_DATA, "\n\nJPEG_HUUFFMAN_UV_AC: reg = 0x%X, %s [#%d]\n", REG_JPEG_HUFFMAN_AC_CHROMINANCE_CTRL, __FILE__, __LINE__);

			for (i = 0; i < wTotalHTNum; i++)
			{
				// Encode and Decode are different
				data = ((*(pCodeValue + i) << 8) | i );
				HOST_WriteRegister(REG_JPEG_HUFFMAN_AC_CHROMINANCE_CTRL,  (MMP_UINT16)(data));
				/*
				{
				    if (!(i % 10) && i != 0)
				        sdk_msg(SDK_MSG_TYPE_JPG_DATA, " \n");
				    sdk_msg(SDK_MSG_TYPE_JPG_DATA, "0x%04X ", data);
				}
				*/
			}
			break;

		default:
			break;
	}
}

// 0x110
// 0x112
void
JPEG_SetDecodeAcHuffmanValueReg(
    JPEG_HUFFMAN_TABLE_SELECTION tableSelection,
    MMP_UINT8 *pCodeValue,
    MMP_UINT16 totalCodeLengthCount)
{
	MMP_UINT16 data = 0x0000;
	MMP_UINT16 wTotalHTNum = 0x0000;
	MMP_UINT16 i = 0;

	switch (tableSelection)
	{
		case JPEG_HUUFFMAN_Y_AC:
			wTotalHTNum = totalCodeLengthCount;
			for (i = 0; i < wTotalHTNum; i++)
			{
				// Encode and Decode are different
				data =  ((i << 8) | (*(pCodeValue + i)));
				HOST_WriteRegister(REG_JPEG_HUFFMAN_AC_LUMINANCE_CTRL,  (MMP_UINT16)(data));
			}
			break;

		case JPEG_HUUFFMAN_UV_AC:
			wTotalHTNum = totalCodeLengthCount;
			for (i = 0; i < wTotalHTNum; i++)
			{
				// Encode and Decode are different
				data =  ((i << 8) | (*(pCodeValue + i)));
				HOST_WriteRegister(REG_JPEG_HUFFMAN_AC_CHROMINANCE_CTRL,  (MMP_UINT16)(data));
			}
			break;

		default:
			break;
	}
}

void
JPEG_SetYuv2RgbMatrix(
    JPEG_YUV_TO_RGB     *matrix)
{
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_11, matrix->_11);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_13, matrix->_13);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_21, matrix->_21);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_22, matrix->_22);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_23, matrix->_23);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_31, matrix->_31);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_32, matrix->_32);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_CONST_R, matrix->ConstR);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_CONST_G, matrix->ConstG);
	HOST_WriteRegister(REG_JPEG_YUV_TO_RGB_CONST_B, matrix->ConstB);
}

void
JPEG_SetRgb565DitherKey(
    JPEG_DITHER_KEY     *ditherKeyInfo)
{
	if( ditherKeyInfo->bEnDitherKey == MMP_TRUE )
	{
		HOST_WriteRegister(REG_JPEG_EN_DITHER_KEY, 0x1);
		HOST_WriteRegister(REG_JPEG_SET_DITHER_KEY, ditherKeyInfo->ditherKey);
		HOST_WriteRegister(REG_JPEG_SET_MASK_DITHER_KEY, ditherKeyInfo->ditherKeyMask);
		HOST_WriteRegister(REG_JPEG_SET_DITHER_KEY_BG, ditherKeyInfo->bgColor);
	}
	else
	{
		HOST_WriteRegister(REG_JPEG_EN_DITHER_KEY, 0x0);
	}
}

//======================================
//
//======================================
void
JPEG_Reset(void)
{	
	HOST_WriteRegisterMask(0x36, 0xc000, 0xc000);
	MMP_Sleep(1);
	HOST_WriteRegisterMask(0x36, 0x0000, 0xc000);
}

void
JPEG_ResetEngine(void)
{	
	MMP_UINT32 i;
	
	HOST_WriteRegisterMask(0x36, 0x4000, 0x4000);
	for (i=0; i < 100; i++)
	     __asm__("");
	HOST_WriteRegisterMask(0x36, 0x0000, 0x4000);
}

//=============================================================================
/**
 * JPEG Power up
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPEG_PowerUp(void)
{
	HOST_WriteRegister(0x38, 0x8800);

	JPEG_Reset();
}

//=============================================================================
/**
 * JPEG encoder Power down (No isp)
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPEG_EncPowerDown(void)
{
	volatile MMP_UINT32 hwStatus = 0;
	MMP_UINT16 timeOut = 0;

	hwStatus = JPEG_GetEngineStatusReg();
	if ((hwStatus & 0xF800) == 0x3000)
	{
		MMP_Sleep(2);
		hwStatus = JPEG_GetEngineStatusReg();
		if ((hwStatus & 0xF800) == 0x3000)
		{
			JPEG_Reset();
			LOG_ERROR " time out !!\n" LOG_END
			goto end;
		}
	}

	hwStatus = JPEG_GetEngineStatus1Reg();
	while (!(hwStatus & JPEG_STATUS_ENCODE_COMPLETE))
	{
		MMP_Sleep(10);
		timeOut++;
		if (timeOut > 50)
		{
			// 1 sec timeOut
			JPEG_Reset();
			LOG_ERROR "time out (status= 0x%x)!!", hwStatus LOG_END
			break;
		}
		hwStatus = 0;
		hwStatus = JPEG_GetEngineStatus1Reg();
	}

end :
	// ------------------------------
	// for power consumption
//    timeOut = 0;
//    do
//    {
//        // Wait JPG streaming buffer empty, avoid cpu hang
//        hwStatus = JPEG_GetEngineStatusReg();
//        if ((hwStatus & 0xc000) != 0 && timeOut < 1000)
//           MMP_Sleep(1);
//        else
//			break;
//    } while(timeOut++);
//    HOST_JPEG_Reset();          // clear regist value
//    HOST_JPEG_DisableClock();   // disable clock
	// -------------------------------------
	return;
}


//=============================================================================
/**
 * JPEG decoder Power down
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPEG_DecPowerDown(void)
{
	volatile MMP_UINT32 hwStatus = 0;
	MMP_UINT16 timeOut = 0;

	hwStatus = JPEG_GetEngineStatusReg();
	if ((hwStatus & 0xF800) == 0x3000)
	{
		MMP_Sleep(2);
		hwStatus = JPEG_GetEngineStatusReg();
		if ((hwStatus & 0xF800) == 0x3000)
		{
			JPEG_Reset();
			MMP_Sleep(500);
			HOST_ISP_Reset();
			LOG_ERROR "time out !!" LOG_END
			goto end;
		}
	}

	hwStatus = JPEG_GetEngineStatus1Reg();
	while (!(hwStatus & JPEG_STATUS_DECODE_COMPLETE) )
	{
		MMP_Sleep(10);
		timeOut++;
		if (timeOut > 50)
		{
			// 1 sec timeOut
			JPEG_Reset();
			MMP_Sleep(500);
			HOST_ISP_Reset();
			LOG_ERROR "time out (status= 0x%x)!!", hwStatus LOG_END
			break;
		}
		hwStatus = 0;
		hwStatus = JPEG_GetEngineStatus1Reg();
	}

end :
	// ------------------------------
	// for power consumption
	// maybe have side effect (When enter suspend, jpg no return ack and then isp busy ??)
//    timeOut = 0;
//    do
//    {
//        // Wait JPG streaming buffer empty, avoid cpu hang
//        hwStatus = JPEG_GetEngineStatusReg();
//        if ((hwStatus & 0xc000) != 0 && timeOut < 1000)
//           MMP_Sleep(1);
//        else
//			break;
//    } while(timeOut++);
//    HOST_JPEG_Reset();          // clear regist value
//    HOST_JPEG_DisableClock();   // disable clock
	// -------------------------------------
	return;
}

void
JPEG_EnableInterrupt(void)
{
	HOST_WriteRegisterMask(REG_JPEG_SAMPLING_FACTOR_CD, (JPEG_MSK_INTERRUPT_ENCODE_FULL | JPEG_MSK_INTERRUPT_ENCODE_END),
	                       (JPEG_MSK_INTERRUPT_ENCODE_FULL | JPEG_MSK_INTERRUPT_ENCODE_END));

}

void
JPEG_DisableInterrupt(void)
{
	HOST_WriteRegisterMask(REG_JPEG_SAMPLING_FACTOR_CD, 0,
	                       (JPEG_MSK_INTERRUPT_ENCODE_FULL | JPEG_MSK_INTERRUPT_ENCODE_END));

}

void
JPEG_ClearInterrupt(void)
{
	HOST_WriteRegister(REG_JPEG_CODEC_FIRE, JPEG_MSK_INTERRUPT_CLEAR);

}
