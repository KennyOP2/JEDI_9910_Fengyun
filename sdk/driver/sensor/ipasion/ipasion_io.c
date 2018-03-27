
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
static MMP_UINT8   IPASION_IICADDR = 0x52 >> 1;
static MMP_UINT8   IPASION_PAGE_REGISTER_ADDR = 0x70;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
MMP_UINT8 
IPASION_ReadI2C_Byte(
    MMP_UINT16 RegAddr)
{

    MMP_RESULT result;

    MMP_UINT8 buffer1[2] = {IPASION_PAGE_REGISTER_ADDR, ((MMP_UINT8)((MMP_UINT16)RegAddr >> 8) & 0xFF)};
    MMP_UINT8 buffer2[2] = {(MMP_UINT8)((MMP_UINT16)RegAddr & 0x00FF), 0};

    mmpIicLockModule();
    
    if (0 != (result = mmpIicReceiveDataEx(CONTROLLER_MODE, IPASION_IICADDR, buffer1, 2, buffer2, 1)))
    {
        printf("ipasion iic Read byte error (0x%x) result = 0x%x! %s()#%d\n", RegAddr, result, __FUNCTION__, __LINE__);
        mmpIicGenStop();
    }
    
    mmpIicReleaseModule();
    return buffer2[1];
}

MMP_RESULT 
IPASION_WriteI2C_Byte(
    MMP_UINT16 RegAddr, 
    MMP_UINT8 RegValue)
{

    MMP_RESULT result;
    MMP_UINT8 buffer1[2] = {IPASION_PAGE_REGISTER_ADDR, ((MMP_UINT8)((MMP_UINT16)RegAddr >> 8) & 0xFF)};
    MMP_UINT8 buffer2[2] = {(MMP_UINT8)((MMP_UINT16)RegAddr & 0x00FF), RegValue};

    mmpIicLockModule();
    
    if (0 != (result = mmpIicSendDataEx(CONTROLLER_MODE, IPASION_IICADDR, buffer1, 2)))
    {
        printf("ipasion  Write buffer1 i2c byte error result = 0x%x! %s()#%d\n", result, __FUNCTION__, __LINE__);
        mmpIicGenStop();
        mmpIicReleaseModule();               
        return result;      
    }
    
    //PalSleep(1);
    
    if (0 != (result = mmpIicSendDataEx(CONTROLLER_MODE, IPASION_IICADDR, buffer2, 2)))
    {
        printf("ipasion Write buffer2 i2c byte error result = 0x%x! %s()#%d\n", result, __FUNCTION__, __LINE__);
        mmpIicGenStop();
    }
    
    mmpIicReleaseModule();
       
    return result;  
}


