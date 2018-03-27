#ifndef __SENSOR_DEVICE_TABLE_H__
#define __SENSOR_DEVICE_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

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
typedef struct CAP_SENSOR_TIMINFO_TABLE_TAG
{
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 FrameRate;
    MMP_UINT16 Skippattern;
    MMP_UINT16 ROIPosX;
    MMP_UINT16 ROIPosY;
    MMP_UINT16 ROIWidth;
    MMP_UINT16 ROIHeight;    
    MMP_UINT16 HPolarity:1;
    MMP_UINT16 VPolarity:1;     
        
}CAP_SENSOR_TIMINFO_TABLE;


static CAP_SENSOR_TIMINFO_TABLE SENSOR_TABLE = {
    //HActive, VActive,           FrameRate,     kipPattern, ROIX, ROIY, ROIW, ROIH,   Hpor,   Vpor  

#if defined (PIXELPLUS_PH1100K)
    1280,    720,    MMP_CAP_FRAMERATE_60HZ,         0x0000,    0,    0, 1280,  720,     0,      0     
#elif defined (IPASION_IP2986)    
    1280,    720,    MMP_CAP_FRAMERATE_30HZ,         0x0000,    0,    0, 1280,  720,     0,      0  
#elif defined (APTINA_MT9M034)     
    1280,    720,    MMP_CAP_FRAMERATE_60HZ,         0xAAAA,    0,    0, 1280,  720,     0,      0    
#else
       0,      0,                         0,              0,    0,    0,    0,    0,     0,      0
#endif     
               
};

//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================
  
    

#ifdef __cplusplus
}
#endif

#endif

