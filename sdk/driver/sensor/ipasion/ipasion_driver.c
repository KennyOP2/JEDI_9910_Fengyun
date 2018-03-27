
#include "mmp_types.h"
#include "pal/timer.h"

#include "mmp_iic.h"

#include "sensor/ipasion/ipasion_io.h"       
#include "sensor/ipasion/ipasion_driver.h"   
         
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

//=============================================================================
//                Private Function Definition
//=============================================================================
static void 
_IPASION_ISP_Initialize(
    void)
{
    IPASION_WriteI2C_Byte(0xF026, 0x70);
    IPASION_WriteI2C_Byte(0xF028, 0x01);
    IPASION_WriteI2C_Byte(0xF029, 0x03);
    IPASION_WriteI2C_Byte(0xF02A, 0x03);
    IPASION_WriteI2C_Byte(0xF02B, 0xFE);
    IPASION_WriteI2C_Byte(0xF02C, 0xFC);
    IPASION_WriteI2C_Byte(0xF023, 0x32);
    IPASION_WriteI2C_Byte(0xF022, 0x0A);
    IPASION_WriteI2C_Byte(0xF021, 0x3F);
    IPASION_WriteI2C_Byte(0xF020, 0x2E);
    
    IPASION_WriteI2C_Byte(0xF01F, 0x37);
    IPASION_WriteI2C_Byte(0xF01E, 0x3D);
    IPASION_WriteI2C_Byte(0xF004, 0x08);
    IPASION_WriteI2C_Byte(0xF04C, 0x05);
    IPASION_WriteI2C_Byte(0xF05B, 0x01);
    IPASION_WriteI2C_Byte(0xF05C, 0x01);
    IPASION_WriteI2C_Byte(0xFE81, 0x40);
    IPASION_WriteI2C_Byte(0xFE7C, 0x03);
    IPASION_WriteI2C_Byte(0xFE31, 0x03);
    IPASION_WriteI2C_Byte(0xF073, 0x03);
    
    IPASION_WriteI2C_Byte(0xF074, 0xE8);
    IPASION_WriteI2C_Byte(0xF075, 0x03);
    IPASION_WriteI2C_Byte(0xF076, 0xE8);
    IPASION_WriteI2C_Byte(0xF07A, 0x32);
    IPASION_WriteI2C_Byte(0xF07B, 0x00);
    IPASION_WriteI2C_Byte(0xF07C, 0x00);
    IPASION_WriteI2C_Byte(0xF07D, 0x02);
    IPASION_WriteI2C_Byte(0xF07E, 0x24);
    IPASION_WriteI2C_Byte(0xF07F, 0x03);
    IPASION_WriteI2C_Byte(0xF080, 0x06);
    
    IPASION_WriteI2C_Byte(0xF081, 0x04);
    IPASION_WriteI2C_Byte(0xF024, 0x63);
    IPASION_WriteI2C_Byte(0xF004, 0x08);
    IPASION_WriteI2C_Byte(0xF077, 0x14);
    IPASION_WriteI2C_Byte(0xF079, 0x05);
    IPASION_WriteI2C_Byte(0xF078, 0x6E);
    IPASION_WriteI2C_Byte(0xF088, 0x60);
    IPASION_WriteI2C_Byte(0xF089, 0x60);
    IPASION_WriteI2C_Byte(0xF08A, 0x20);
    IPASION_WriteI2C_Byte(0xF08B, 0x20);
    
    IPASION_WriteI2C_Byte(0xF004, 0x08);
    IPASION_WriteI2C_Byte(0xF063, 0x00);
    IPASION_WriteI2C_Byte(0xF064, 0x00);
    IPASION_WriteI2C_Byte(0xFEA9, 0x00);
    IPASION_WriteI2C_Byte(0xFEA7, 0x00);
    IPASION_WriteI2C_Byte(0xF0D9, 0x04);
    IPASION_WriteI2C_Byte(0xF0DA, 0x04);
    IPASION_WriteI2C_Byte(0xF0DB, 0x01);
    
    MMP_Sleep(200);
          
}

//The default FPS settings of iP2986 is 30fps at max. and 15fps at min.. 
//If you want to keep it in 30fps, following the steps as below when initializing.
static void
_IPASION_ISP_720P30FPS(
    void)
{
    IPASION_WriteI2C_Byte(0xF044, 0x1E);
    IPASION_WriteI2C_Byte(0xF045, 0x1E);
    IPASION_WriteI2C_Byte(0xF005, 0x0A);
    
    MMP_Sleep(200);    
}
//=============================================================================
//                Public Function Definition
//=============================================================================
void 
mmpIpasionInitialize(
    void)
{
    _IPASION_ISP_Initialize();
    
    _IPASION_ISP_720P30FPS();   

}


    



