/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file host_clock.c
 *
 * @author Vincent Lee
 */

#include "host/host.h"

//=============================================================================
//                              Function Definition
//=============================================================================

//=============================================================================
// MMP_VIDEO_CLOCK_REG_34, MMP_VIDEO_CLOCK_REG_36
//void
//HOST_JPEG_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0xFFFF, MMP_VIDEO_EN_M7CLK | MMP_VIDEO_EN_X3CLK | MMP_VIDEO_EN_X2CLK);
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_34, 0xFFFF, MMP_VIDEO_EN_DIV_XMCLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_JPEG);
//}

//void
//HOST_JPEG_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0x0000, MMP_VIDEO_EN_M7CLK | MMP_VIDEO_EN_X3CLK | MMP_VIDEO_EN_X2CLK);
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_34, 0x0000, MMP_VIDEO_EN_DIV_XMCLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_JPEG);
//}

//void 
//HOST_JPEG_Reset(
//    void)
//{
//	HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0xFFFF, MMP_JPEG_RESET | MMP_JPEGGBL_RESET);
//	MMP_Sleep(1);
//	HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0x0000, MMP_JPEG_RESET | MMP_JPEGGBL_RESET);
//	MMP_Sleep(1);
//}

//void 
//HOST_VIDEO_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0xFFFF, MMP_VIDEO_EN_M6CLK | MMP_VIDEO_EN_M7CLK | MMP_VIDEO_EN_X0CLK | MMP_VIDEO_EN_X1CLK | MMP_VIDEO_EN_X3CLK);
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_34, 0xFFFF, MMP_VIDEO_EN_DIV_XMCLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_MPEG);
//}

//void 
//HOST_VIDEO_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0x0000, MMP_VIDEO_EN_M6CLK | MMP_VIDEO_EN_M7CLK | MMP_VIDEO_EN_X0CLK | MMP_VIDEO_EN_X1CLK | MMP_VIDEO_EN_X3CLK);
//    HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_34, 0x0000, MMP_VIDEO_EN_DIV_XMCLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_MPEG);
//}

//void 
//HOST_VIDEO_Reset(
//    void)
//{
//	MMP_UINT32 i;
//	HOST_VIDEO_DisableClock();
//	for(i=0; i<100; i++);
//	HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0xFFFF, MMP_VIDEO_RESET);
//	for(i=0; i<100; i++);
//	HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0xFFFF, MMP_VIDEOGBL_RESET );
//	for(i=0; i<500; i++);	
//	HOST_WriteRegisterMask(MMP_VIDEO_CLOCK_REG_36, 0x0000, MMP_VIDEO_RESET | MMP_VIDEOGBL_RESET);
//	for(i=0; i<500; i++);
//	HOST_VIDEO_EnableClock();
//}

//=============================================================================
// MMP_PCR_CLOCK_REG_42
//void 
//HOST_PCR_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_42, 0xFFFF, MMP_PCR_EN_PCLK);
//    HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_40, 0xFFFF, MMP_PCR_EN_DIV_PCLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_FPC);
//}

//void 
//HOST_PCR_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_42, 0x0000, MMP_PCR_EN_PCLK);
//    HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_40, 0x0000, MMP_PCR_EN_DIV_PCLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_FPC);
//}

//void 
//HOST_PCR_Reset(
//    void)
//{
//	HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_42, 0xFFFF, MMP_PCR_RESET);
//	MMP_Sleep(1);
//	HOST_WriteRegisterMask(MMP_PCR_CLOCK_REG_42, 0x0000, MMP_PCR_RESET);
//	MMP_Sleep(1);
//}

//=============================================================================
// MMP_USB_CLOCK_REG_46
//void 
//HOST_USB_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_USB_CLOCK_REG_46, 0xFFFF, MMP_USB_EN_N6CLK | MMP_USB_EN_M11CLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_USB);
//}

//void 
//HOST_USB_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_USB_CLOCK_REG_46, 0x0000, MMP_USB_EN_N6CLK | MMP_USB_EN_M11CLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_USB);
//}

//=============================================================================
// MMP_TSI_CLOCK_REG_48
//void
//HOST_TSI0_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_TSI_CLOCK_REG_48, 0xFFFF, MMP_TSI_EN_M12CLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_TSI0);
//}

//void
//HOST_TSI0_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_TSI_CLOCK_REG_48, 0x0000, MMP_TSI_EN_M12CLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_TSI0);
//}

//void
//HOST_TSI1_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_TSI_CLOCK_REG_48, 0xFFFF, MMP_TSI_EN_M13CLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0xFFFF, MMP_HOST_BUS_EN_MMIO_TSI1);
//}

//void
//HOST_TSI1_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_TSI_CLOCK_REG_48, 0x0000, MMP_TSI_EN_M13CLK);
//    HOST_WriteRegisterMask(MMP_HOST_BUS_CONTROLLER_REG_202, 0x0000, MMP_HOST_BUS_EN_MMIO_TSI1);
//}

//=============================================================================
// MMP_DEMOD_CLOCK_REG_4A
//void 
//HOST_DEMOD_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_DEMOD_CLOCK_REG_4A, 0xFFFF, MMP_DEMOD_EN_M14CLK);
//}

//void 
//HOST_DEMOD_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_DEMOD_CLOCK_REG_4A, 0x0000, MMP_DEMOD_EN_M14CLK);
//}

//void 
//HOST_AUDIO_EnableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_AUDIO_CLOCK_REG_3A, (1<<15), (1<<15));
//    HOST_WriteRegisterMask(MMP_AUDIO_CLOCK_REG_3C, (1<<15), (1<<15));
//}

//void 
//HOST_AUDIO_DisableClock(
//    void)
//{
//    HOST_WriteRegisterMask(MMP_AUDIO_CLOCK_REG_3A, 0, (1<<15));
//    HOST_WriteRegisterMask(MMP_AUDIO_CLOCK_REG_3C, 0, (1<<15));
//
//}
