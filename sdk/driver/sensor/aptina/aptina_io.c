
#include "mmp_types.h"
#include "iic/iic.h"
#include "mmp_iic.h"
#include "sensor/aptina/aptina_io.h"    

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static MMP_UINT8   APTINA_IICADDR = 0xBA >> 1;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
MMP_UINT32 
APTINA_ReadI2C_32Bit(
    MMP_UINT16 RegAddr)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8*  pdbuf = dbuf;
    MMP_UINT16 value;

    *pdbuf++ = (MMP_UINT8)((RegAddr&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(RegAddr&0x00ff);
        
    mmpIicLockModule();    
    
    result = IIC_SendData(APTINA_IICADDR, dbuf, 2, W_STOP); 
    if(result == MMP_RESULT_SUCCESS)
        result = IIC_ReceiveData(APTINA_IICADDR, pdbuf, 4);

    value = ((dbuf[2] & 0xFF) << 23) | ((dbuf[3] & 0xFF) << 16) | ((dbuf[4] & 0xFF) << 8) | (dbuf[5] & 0xFF);              
          
    mmpIicReleaseModule();
    
    return value;
}


MMP_UINT16 
APTINA_ReadI2C_16Bit(
    MMP_UINT16 RegAddr)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8*  pdbuf = dbuf;
    MMP_UINT32 value;

    *pdbuf++ = (MMP_UINT8)((RegAddr&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(RegAddr&0x00ff);
        
    mmpIicLockModule();    
    
    result = IIC_SendData(APTINA_IICADDR, dbuf, 2, W_STOP); 
    if(result == MMP_RESULT_SUCCESS)
        result = IIC_ReceiveData(APTINA_IICADDR, pdbuf, 2);

    value = ((dbuf[2] & 0xFF) << 8) | (dbuf[3] & 0xFF);  
        
    mmpIicReleaseModule();
    
    return value;
}

MMP_UINT8 
APTINA_ReadI2C_8Bit(
    MMP_UINT16 RegAddr)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8*  pdbuf = dbuf;
    MMP_UINT8 value;

    *pdbuf++ = (MMP_UINT8)((RegAddr&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(RegAddr&0x00ff);
        
    mmpIicLockModule();    
    
    result = IIC_SendData(APTINA_IICADDR, dbuf, 2, W_STOP); 
    if(result == MMP_RESULT_SUCCESS)
        result = IIC_ReceiveData(APTINA_IICADDR, pdbuf, 1);

    mmpIicReleaseModule();
    
    value = (dbuf[2] & 0xFF); 
    
    return value;
}

MMP_RESULT 
APTINA_WriteI2C_32Bit(
    MMP_UINT16 RegAddr,
    MMP_UINT32 data)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8*  pdbuf = dbuf;
    MMP_UINT8  data_1;
    MMP_UINT8  data_2;
    MMP_UINT8  data_3;
    MMP_UINT8  data_4;
    
    *pdbuf++ = (MMP_UINT8)((RegAddr&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(RegAddr&0x00ff);
    *pdbuf++ = (MMP_UINT8)((data&0xff000000)>>24); //1
    *pdbuf++ = (MMP_UINT8)((data&0x00ff0000)>>16); //2
    *pdbuf++ = (MMP_UINT8)((data&0x0000ff00)>>8); //3    
    *pdbuf++ = (MMP_UINT8)(data&0x000000ff); //4    

    mmpIicLockModule(); 
    if(0!= (result = IIC_SendData(APTINA_IICADDR, dbuf, 6, W_STOP)))
    {
        printf("APTINA_WriteI2C_32Bit I2c Write Error, reg=%04x val=%08x\n", RegAddr, data);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    
    return result;
}

MMP_RESULT 
APTINA_WriteI2C_16Bit(
    MMP_UINT16 RegAddr,
    MMP_UINT16 data)
{

    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8*  pdbuf = dbuf;
    MMP_UINT8  data_he;
    MMP_UINT8  data_le;
    
    *pdbuf++ = (MMP_UINT8)((RegAddr&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(RegAddr&0x00ff);
    *pdbuf++ = (MMP_UINT8)((data&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(data&0x00ff);

    mmpIicLockModule();        
    if(0 != (result = IIC_SendData(APTINA_IICADDR, dbuf, 4, W_STOP)))
    {
        printf("APTINA_WriteI2C_16Bit I2c Write Error, reg=%04x val=%04x\n", RegAddr, data);
        mmpIicGenStop();
    }       
    mmpIicReleaseModule();    
    
    return result;
}

MMP_RESULT 
APTINA_WriteI2C_8Bit(
    MMP_UINT16 RegAddr,
    MMP_UINT8  data)
{
    MMP_RESULT result;
    MMP_UINT8  dbuf[256];
    MMP_UINT8*  pdbuf = dbuf;
    
    *pdbuf++ = (MMP_UINT8)((RegAddr&0xff00)>>8);
    *pdbuf++ = (MMP_UINT8)(RegAddr&0x00ff);
    *pdbuf = (MMP_UINT8)(data);

    mmpIicLockModule();    
    if(0 != (result = IIC_SendData(APTINA_IICADDR, dbuf, 3, W_STOP)))
    {
        printf("APTINA_WriteI2C_8Bit I2c Write Error, reg=%04x val=%02x\n", RegAddr, data);
        mmpIicGenStop();
    }           
    mmpIicReleaseModule();    
    
    return result;
}

