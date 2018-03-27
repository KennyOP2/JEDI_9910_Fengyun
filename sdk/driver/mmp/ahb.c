/*
 * Copyright (c) 2004 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Co-processor API functoin file.
 *      Date: 2007/10/24
 *
 * @author Jeimei Cheng
 * @version 0.1
 */

#include "host/host.h"
#include "host/ahb.h"

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================
//=============================================================================
/**
 * Read register value
 *
 * @param  destAddress  register address
 * @param  data         return value
 * @return none
 */
//=============================================================================
void
AHB_ReadRegister(
    MMP_UINT32 destAddress,
    MMP_UINT32* data)
{
#if defined (WIN32)
    MMP_UINT16 data_H, data_L;

	HOST_ReadRegister((MMP_UINT16)(destAddress + 2),(MMP_UINT16*)&data_H);
	HOST_ReadRegister((MMP_UINT16)(destAddress),(MMP_UINT16*)&data_L);
	*data = (MMP_UINT32)((data_H << 16) | data_L);
#endif

#if defined(__FREERTOS__)
//	printf("read *destAddr %X\r\n",*(volatile int *)destAddress);
//	printf("read data %X\r\n",*data);
    *data = *(volatile int *)destAddress;
//	printf("read after *destAddr %X\r\n",*(volatile int *)destAddress);	
#endif
}

//=============================================================================
/**
 * Write register value
 *
 * @param  destAddress  register address
 * @param  data         written value
 * @return none
 */
//=============================================================================
void
AHB_WriteRegister(
    MMP_UINT32 destAddress,
    MMP_UINT32 data)
{
#if defined(WIN32)
	MMP_UINT16 data_H, data_L;

	data_L = (MMP_UINT16) (data & 0x0000ffff);
	data_H = (MMP_UINT16) (data >> 16);

	HOST_WriteRegister((MMP_UINT16) (destAddress + 2) ,data_H);
	HOST_WriteRegister((MMP_UINT16) (destAddress)  	,data_L);
#endif

#if defined(__FREERTOS__)
//	printf("write *destAddr %X\r\n",*(volatile int *)destAddress);
//	printf("write data %X\r\n",data);
    *(volatile int *)destAddress = data;
//	printf("write after *destAddr %X\r\n",*(volatile int *)destAddress);		
#endif
}

//=============================================================================
/**
 * Write register value with mask
 *
 * Usage:
 *    MMIO 0x0500 old value = 0xF301
 *    AHB_WriteRegisterMask(0x0500, 0x0005, 0x0F0F)
 *    MMIO 0x0500 new value = 0xF005
 *
 * @param destAddress   mmio location
 * @param data          data value will be set
 * @param mask          mask
 * @return none
 */
//=============================================================================

void
AHB_WriteRegisterMask(
    MMP_UINT32 destAddress,
    MMP_UINT32 data,
    MMP_UINT32 mask)
{
#if defined(WIN32)
	MMP_UINT16 data_H, data_L;
	MMP_UINT16 mask_H, mask_L;

	data_L = (MMP_UINT16) (data & 0x0000ffff);
	data_H = (MMP_UINT16) (data >> 16);
	mask_L = (MMP_UINT16) (mask & 0x0000ffff);
	mask_H = (MMP_UINT16) (mask >> 16);	

	HOST_WriteRegisterMask((MMP_UINT16) (destAddress + 2) ,data_H,mask_H);
	HOST_WriteRegisterMask((MMP_UINT16) (destAddress)  	,data_L,mask_L);
#endif

#if defined(__FREERTOS__)
	MMP_UINT32 oldValue;
//	printf("write *destAddr %X\r\n",*(volatile int *)destAddress);
//	printf("write data %X\r\n",data);
    AHB_ReadRegister(destAddress,&oldValue);
    data = (data & mask) | (oldValue & (~mask));
    *(volatile int *)destAddress = data;
//	printf("write after *destAddr %X\r\n",*(volatile int *)destAddress);		
#endif
}
