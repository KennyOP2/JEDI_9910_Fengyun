///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mcu.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/08/12
//   @fileversion: CAT9883_V43
//******************************************/
#include "host/ahb.h"
#include "pal/pal.h"
#include "mmp_types.h"
#include "mmp_iic.h"

#include "cat9883/cat9883_mcu.h"

static MMP_UINT8 CAT9883_IICADR = 0x9A >> 1;

MMP_UINT8 CAT9883_ReadI2C_Byte(MMP_UINT8 RegAddr)
{
    MMP_UINT8  p_data;
    MMP_RESULT result;

    mmpIicLockModule();
    if (MMP_RESULT_SUCCESS != (result = mmpIicReceiveData(IIC_MASTER_MODE, CAT9883_IICADR, RegAddr, &p_data, 1)))
    {
        mmpIicGenStop();
        dbg_msg(SDK_MSG_TYPE_INFO, "CAT9883 Read IIC Byte error 0x%x\n", RegAddr);
    }
    mmpIicReleaseModule();

    return p_data;
}

MMP_RESULT CAT9883_WriteI2C_Byte(MMP_UINT8 RegAddr, MMP_UINT8 d)
{
    MMP_RESULT result;

    mmpIicLockModule();
    if (MMP_RESULT_SUCCESS != (result = mmpIicSendData(IIC_MASTER_MODE, CAT9883_IICADR, RegAddr, &d, 1)))
    {
        mmpIicGenStop();
        dbg_msg(SDK_MSG_TYPE_INFO, "CAT9883 Write IIC Byte Adr = 0x%x Value = 0x%x\n", RegAddr, d);
    }
    mmpIicReleaseModule();

    return result;
}

MMP_RESULT CAT9883_WriteI2C_ByteN(MMP_UINT8 RegAddr, MMP_UINT8 *pData, int N)
{
    MMP_RESULT result;

    mmpIicLockModule();
    if (MMP_RESULT_SUCCESS != (result = mmpIicSendData(IIC_MASTER_MODE, CAT9883_IICADR, RegAddr, pData, N)))
    {
        mmpIicGenStop();
        dbg_msg(SDK_MSG_TYPE_INFO, "CAT9883 Write IIC ByteN 0x%x\n", RegAddr);
    }
    mmpIicReleaseModule();

    return result;
}

MMP_RESULT CAT9883_WriteI2c_ByteMask(MMP_UINT8 RegAddr, MMP_UINT8 data, MMP_UINT8 mask)
{
    MMP_UINT8  Value;
    MMP_RESULT flag;

    mmpIicLockModule();

    if (0 != (flag = mmpIicReceiveData(IIC_MASTER_MODE, CAT9883_IICADR, RegAddr, &Value, 1)))
    {
        printf("CAT9883 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop();
    }

    Value = ((Value & ~mask) | (data & mask));

    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, CAT9883_IICADR, RegAddr, &Value, 1)))
    {
        printf("CAT9883 I2c write error, reg = %02x val =%02x\n", RegAddr, Value);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;
}