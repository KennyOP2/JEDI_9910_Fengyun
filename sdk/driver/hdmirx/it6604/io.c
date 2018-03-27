///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iT6607.c>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: HDMIRX_SAMPLE_3.11
//******************************************/

#include "hdmirx/it6604/typedef.h"
#include "hdmirx/it6604/it6604.h"
#include "hdmirx/it6604/io.h"
#include "mmp_iic.h"

static BYTE   IT6604_IICADDR = 0x92 >> 1;

BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr)
{
    BYTE d;
    BOOL flag;
    
    mmpIicLockModule();
    if (0 != (flag = mmpIicReceiveData(IIC_MASTER_MODE, IT6604_IICADDR, RegAddr, &d, 1)))
    {
        printf("HDMIRX I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return d;
}


SYS_STATUS HDMIRX_WriteI2C_Byte(BYTE RegAddr, BYTE d)
{
    BOOL flag;
    
    if (gtTriStateEnable && RegAddr == REG_RX_TRISTATE_CTRL)
        return 0;
    
    mmpIicLockModule();    
    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, IT6604_IICADDR, RegAddr, &d, 1)))
    {
        printf("HDMIRX I2c write error, reg = %02x val =%02x\n", RegAddr, d);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;

}


SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr, BYTE *pData, int N)
{
    BOOL flag; 
    
    mmpIicLockModule(); 
    if (0 != (flag = mmpIicReceiveData(IIC_MASTER_MODE, IT6604_IICADDR, RegAddr, pData, N)))
    {
        printf("HDMIRX Read byte error\n");
        mmpIicGenStop();
    }   
    mmpIicReleaseModule();
    
    return flag;

}


SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr, BYTE *pData, int N)
{
    BOOL flag; 
    
    mmpIicLockModule();
    if ( 0 != (flag = mmpIicSendData(IIC_MASTER_MODE, IT6604_IICADDR, RegAddr, pData, N)))
    {
        printf("HDMIRX Write i2c byte error\n");
        mmpIicGenStop();
    }   
    mmpIicReleaseModule();    
    
    return flag;    

}

SYS_STATUS HDMIRX_SetI2C_Byte(SHORT RegAddr, BYTE Mask, BYTE Value)
{
    BYTE  p_data;
    BOOL    FLAG;
    BYTE i;

    mmpIicLockModule();

    if( Mask != 0xFF )
    {
        for( i = 0; i < 3; i++ )
        {
            FLAG = mmpIicReceiveData(IIC_MASTER_MODE, IT6604_IICADDR ,RegAddr, &p_data, 1);
            if( FLAG == 0 ) break;
            printf("RX Read I2C ERROR !!!");
            printf("=====  Read Reg0x%x=  \n", RegAddr);
            mmpIicGenStop();
        }

        // printf("reg%02X = %02X ->",(int)RegAddr,(int)p_data) ;
        p_data &= ~Mask; // printf(" %02x ->",(int)p_data) ;
        Value &= Mask;    // printf(" %02x ->",(int)p_data) ;
        p_data |= Value; // printf(" %02x\n",(int)p_data) ;
    }
    else
    {
        p_data = Value;
    }

    for( i = 0; i < 3; i++ )
    {
        FLAG = mmpIicSendData(IIC_MASTER_MODE, IT6604_IICADDR, RegAddr, &p_data , 1);
        if(FLAG == 0) break;
        printf("RX Read I2C ERROR !!!");
        printf("=====  Write Reg0x%x <- %x  \n", RegAddr, p_data);
        mmpIicGenStop();
    }

    mmpIicReleaseModule();
    
    return !FLAG;

}
