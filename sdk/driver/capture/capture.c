#include "host/host.h"
#include "math.h"
#include "mem/mem.h"
#include "intr/intr.h"

#include "capture/capture.h"
#include "capture/capture_hw.h"
#include "capture/capture_util.h"
#include "capture/capture_reg.h"
#include "mmp_capture.h"

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
/*
Input   Output  Ratio(In/Out)
1088    1080    1.007407407
1920    1920    1
1280    1920    0.666666667
576     1080    0.533333333
480     1080    0.444444444
720     1920    0.375
640     1920    0.333333333
288     1080    0.266666667
352     1920    0.183333333
*/
#define     WEIGHT_NUM 9
static MMP_FLOAT   ScaleRatio[WEIGHT_NUM] = {(1088.0f/1080), (1920.0f/1920), (1280.0f/1920), (576.0f/1080), (480.0f/1080), (720.0f/1920), (640.0f/1920), (288.0f/1080), (352.0f/1920)};
static MMP_UINT16  WeightMatInt[WEIGHT_NUM][CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];

MMP_UINT32 CapMemBuffer[CAPTURE_MEM_BUF_COUNT] = { 0 };
static MMP_UINT16 CapMemTable[CAPTURE_MEM_BUF_COUNT][2] = {
    {0,     0                                                         },//Y0
    {0,     CAP_INPUT_MAX_HEIGHT * 3                                  },//UV0
    {0,     CAP_INPUT_MAX_HEIGHT                                      },//Y1
    {0,     CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1)    },//UV1
    {0,     CAP_INPUT_MAX_HEIGHT * 2                                  },//Y2
    {0,     CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1) * 2} //UV2
};
//   CAP_MEM_BUF_PITCH
//     ----------
// 720 |   Y0   |
//     ----------
// 720 |   Y1   |
//     ----------
// 720 |   Y2   |
//     ----------
// 360 |   UV0  |
//     ----------
// 360 |   UV1  |
//     ----------
// 360 |   UV2  |
//     ----------

//=============================================================================
//                Private Function Definition
//=============================================================================
    
#if defined (SENSOR_DEV)

    #if defined(IT9919_144TQFP)    
        #if defined (IPASION_IP2986)
            #include "IT9919_144TQFP/io_sensor_ipasion_ip2986.c"        
        #endif      
    #else //IT9917_176TQFP            
        #if defined(PIXELPLUS_PH1100K)
            #include "IT9917_176TQFP/io_sensor_pixelplus_ph1100k.c"      
        #elif defined (APTINA_MT9M034)
            #include "IT9917_176TQFP/io_sensor_aptina_mt9m034.c"
        #endif          
    #endif

#else      
    
    #if defined (IT9913_128LQFP)
        #include "IT9913_128LQFP/io_video.c" 
    #elif defined (IT9919_144TQFP)
        #include "IT9919_144TQFP/io_video.c"                   
    #else
        #include "IT9917_176TQFP/io_video.c"   
    #endif 
           
#endif

//=============================================================================
/**
* Calculate Scale Factor
*/
//=============================================================================
static MMP_FLOAT
_Cap_ScaleFactor(
    MMP_UINT16  Input,
    MMP_UINT16  Output)
{
    return (MMP_FLOAT) (((MMP_INT) (16384.0f*Input/(MMP_FLOAT)Output))/16384.0f);
}

//=============================================================================
/**
* Select Color Matrix 
*/
//=============================================================================
static void
_Cap_ColorMatrix(
    MMP_BOOL isBT709)
{
    if (isBT709 == MMP_TRUE)
    {
        /* RGB to YUV REG */   
        Capctxt->RGBtoYUVFun._11 = 0x0037;
        Capctxt->RGBtoYUVFun._12 = 0x00B7;
        Capctxt->RGBtoYUVFun._13 = 0x0012;
        Capctxt->RGBtoYUVFun._21 = 0x07e2;
        Capctxt->RGBtoYUVFun._22 = 0x079b;
        Capctxt->RGBtoYUVFun._23 = 0x0083;
        Capctxt->RGBtoYUVFun._31 = 0x0083;
        Capctxt->RGBtoYUVFun._32 = 0x0789;
        Capctxt->RGBtoYUVFun._33 = 0x07f4;
        Capctxt->RGBtoYUVFun.ConstY = 0x0000;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;    
    }
    else
    {
        /* RGB to YUV REG */   
        Capctxt->RGBtoYUVFun._11 = 0x004D;
        Capctxt->RGBtoYUVFun._12 = 0x0096;
        Capctxt->RGBtoYUVFun._13 = 0x001D;
        Capctxt->RGBtoYUVFun._21 = 0x07d4;
        Capctxt->RGBtoYUVFun._22 = 0x07a9;
        Capctxt->RGBtoYUVFun._23 = 0x0083;
        Capctxt->RGBtoYUVFun._31 = 0x0083;
        Capctxt->RGBtoYUVFun._32 = 0x0792;
        Capctxt->RGBtoYUVFun._33 = 0x07eb;
        Capctxt->RGBtoYUVFun.ConstY = 0x0000;
        Capctxt->RGBtoYUVFun.ConstU = 0x0080;
        Capctxt->RGBtoYUVFun.ConstV = 0x0080;      
    }        
}

//=============================================================================
/**
 * Create weighting for the matrix of scaling.
*/
//=============================================================================
static void
_CAP_CreateWeighting(
    MMP_FLOAT   scale,
    MMP_UINT8   taps,
    MMP_UINT8   tapSize,
    MMP_FLOAT   weightMatrix[][CAP_SCALE_TAP])
{
    MMP_UINT8   i, j;
    MMP_FLOAT   WW;
    MMP_FLOAT   W[32];
    MMP_INT16   point;
    MMP_UINT8   adjust;
    MMP_INT16   sharp;
    MMP_UINT8   method;
    MMP_FLOAT   precision = 64.0f;
    MMP_FLOAT   fscale = 1.0f;

    //Kevin:TODO temp solution
    adjust = 0;
    method = 11;
    sharp = 0;

    switch(sharp)
    {
        case 4:     fscale = 0.6f;  break;
        case 3:     fscale = 0.7f;  break;
        case 2:     fscale = 0.8f;  break;
        case 1:     fscale = 0.9f;  break;
        case 0:     fscale = 1.0f;  break;
        case -1:    fscale = 1.1f;  break;
        case -2:    fscale = 1.2f;  break;
        case -3:    fscale = 1.3f;  break;
        case -4:    fscale = 1.4f;  break;
        default:    fscale = 1.5f;  break;
    };

    if (adjust == 0)
    {
        if (scale < 1.0f)
        {
            scale = fscale;
        }
        else
        {
            scale *= fscale;
        }
    }
    else if (adjust == 1)
    {
        //Last update (2002/04/24) by WKLIN]
        // For Low Pass
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale > 1.0f)
        {
            scale *= 1.1f;
        }
    }
    else if (adjust == 2)
    {
        //Last update (2003/08/17) by WKLIN in Taipei
        //For including more high frequency details
        if (scale < 1.0f)
        {
            scale = 0.9f;
        }
        else if (scale > 1.0f)
        {
            scale *= 0.9f;
        }
    }
    else if (adjust == 3)
    {
        //Last update (2003/09/10) by WKLIN in Hsin-Chu
        //For excluding more high frequency details
        if (scale < 1.0f)
        {
            scale = 1.2f;
        }
        else if (scale >= 1.0f)
        {
            scale *= 1.3f;
        }
    }

    if (method == 10)
    {
        //Sinc
        for (i=0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1) - 1;
            for (j = 0; j < taps; j++)
            {
                W[j] = sinc( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  WeightMat[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }

        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 11)
    {
        //rcos
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW =0.0;
            point = (MMP_INT) (taps>>1)-1;
            for (j=0; j < taps; j++)
            {
                W[j] = rcos( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];
            }

            //for (j=0; j< taps; j++)
            //  weightMatrix[i][j] = ((int)(W[j]/WW*precision + 0.5 ))/precision;

            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }

        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else if (method == 12)
    {
        // Catmull-Rom Cubic interpolation
        for (i = 0; i <= (tapSize>>1); i++)
        {
            WW = 0.0f;
            point = (MMP_INT)(taps>>1)-1;
            for (j = 0; j < taps; j++)
            {
                W[j] = cubic01( ((MMP_FLOAT)point + i/((MMP_FLOAT)tapSize)) / (MMP_FLOAT)scale);
                point--;
                WW += W[j];

                //cap_msg_ex(CAP_MSG_TYPE_ERR, "i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
                //  i, W[j], point, WW);
            }


            //Changed: 2004/02/24
            weightMatrix[i][taps-1] = 1.0;
            for (j = 0; j < taps-1; j++)
            {
                weightMatrix[i][j] = ((MMP_INT)(W[j]/WW*precision + 0.5)) / precision;
                weightMatrix[i][taps-1] -= weightMatrix[i][j];
            }
        }
        for (i = ((tapSize>>1)+1); i < tapSize; i++)
        {
            for (j = 0; j < taps; j++)
            {
                weightMatrix[i][j] = weightMatrix[tapSize-i][taps-1-j];
            }
        }
    }
    else
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * CAP memory Initialize.
 */
//=============================================================================
MMP_RESULT 
Cap_Memory_Initialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT32 startaddr;
    MMP_UINT32 i;
    MMP_UINT32 offset = 0;
    MMP_UINT16 data;
    MMP_UINT32 size = CAP_MEM_BUF_HEIGHT * CAP_MEM_BUF_PITCH + CAP_MEM_BUF_PITCH - 1;

    if (CapMemBuffer[0])
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " Init Conflict %s (%d) error\n", __FUNCTION__, __LINE__);
        goto end;
    }

    startaddr = (MMP_UINT32) MEM_Allocate(size, MEM_USER_VIDEO);

    if (startaddr == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " create memory pool fail %s (%d) error\n", __FUNCTION__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;       
    }
        
    startaddr -= (MMP_UINT32)HOST_GetVramBaseAddress();
    //startaddr = ((startaddr + CAP_MEM_BUF_PITCH - 1) >> 11) << 11;

    CapMemBuffer[0] = startaddr;
    //printf("CapMemBuffer[0] addr = 0x%x\n", CapMemBuffer[0]);

    for (i = 1; i < CAPTURE_MEM_BUF_COUNT; i++)
    {
        offset = CapMemTable[i][0] + CapMemTable[i][1] * CAP_MEM_BUF_PITCH;
        CapMemBuffer[i] = offset + CapMemBuffer[0];
        //printf("CapMemBuffer[%d] addr = 0x%x\n", i , CapMemBuffer[i]);
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);
        
    return (MMP_RESULT)result;
           
}
//=============================================================================
/**
 * CAP memory clear.
 */
//=============================================================================
MMP_RESULT 
Cap_Memory_Clear(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT32 i;
    MMP_UINT8 *pAddr;
        
    if (CapMemBuffer[0] == 0x0)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " clear memory fail %s (%d) error\n", __FUNCTION__, __LINE__);
        result = MMP_RESULT_ERROR;
        goto end;
    }    
    
    for (i = 0; i < CAPTURE_MEM_BUF_COUNT; i++)
    {
        pAddr = (MMP_UINT8 *)CapMemBuffer[i];        
        if (i & 0x1)
            memset(pAddr, 0x80, (CAP_MEM_BUF_PITCH * (CAP_INPUT_MAX_HEIGHT >> 1)));
        else
            memset(pAddr, 0x10, (CAP_MEM_BUF_PITCH * CAP_INPUT_MAX_HEIGHT));
    }    
        
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);
        
    return (MMP_RESULT)result;    
}

//=============================================================================
/**
 * Update CAP device.
 */
//=============================================================================
MMP_RESULT
Cap_Update_Reg(
    void)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32 index;

    if(Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
            
    switch (INPUT_DEVICE)
    {
#ifdef COMPOSITE_DEV        
        case MMP_CAP_DEV_ADV7180:
            _ADV7180_Input_Data_Pin();
            break;
#endif
    
#ifdef COMPONENT_DEV    
        case MMP_CAP_DEV_CAT9883:
           _CAT9883_Input_Data_Pin();
            break;
#endif
          
#ifdef SENSOR_DEV
        case MMP_CAP_DEV_SENSOR:
            _SENSOR_Input_Data_Pin();
            break;
#else         
        case MMP_CAP_DEV_HDMIRX:
            _HDMIRX_Input_Data_Pin();
            break;
#endif          
        default:
            cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
            result = MMP_RESULT_ERROR;
            break;
    }
            
    // Update Onfly or Memeory mode    
    if (Capctxt->EnableOnflyMode == MMP_TRUE)   
        Cap_Set_ISP_HandShaking(ONFLY_MODE, 0);
    else
    {
        Cap_Set_ISP_HandShaking(MEMORY_MODE, 0);
        Cap_Set_Interrupt_Mode(CAP_INT_MODE_ERR_FRAMEEND);
    }

    if (mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX && mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
        _Cap_ColorMatrix(MMP_FALSE); //BT601
    else if (Capctxt->ininfo.capwidth >= 1280)
        _Cap_ColorMatrix(MMP_TRUE); //BT709
    else
       _Cap_ColorMatrix(MMP_FALSE); //BT601
    
    // Update Capture Interrupt
    Cap_Set_Enable_Interrupt(Capctxt->EnableInterrupt);
    
    // Update Capture Write Buffer Address
    Cap_Set_Buffer_addr_Reg(CapMemBuffer, Capctxt->outinfo.OutAddrOffset);   
                        
    // Update Input Pin mux
    Cap_Set_Input_Pin_Mux_Reg(&Capctxt->inmux_info);

    // Update Output Pin mux    
    Cap_Set_Output_Pin_Mux_Reg(&Capctxt->outpin_info);
    
    // Update IO Parameter
    Cap_Set_IO_Mode_Reg(&Capctxt->iomode_info);
    
    // Update Input Parameter
    Cap_Set_Input_Data_Info_Reg(&Capctxt->ininfo);
    
    // Update Hor Scale
    Cap_Set_HorScale_Width_Reg(&Capctxt->outinfo);

    Cap_Set_Enable_Reg(&Capctxt->funen);

    // Update RGB to YUV Matrix
    Cap_SetRGBtoYUVMatrix_Reg(&Capctxt->RGBtoYUVFun);    
        
    // Update Color Format    
    Cap_Set_Color_Format_Reg(&Capctxt->YUVinfo);

    // Update Scale Matrix
    Capctxt->ScaleFun.HCI = _Cap_ScaleFactor(Capctxt->ininfo.ROIWidth, Capctxt->outinfo.OutWidth);
    CAP_SetScaleParam_Reg(&Capctxt->ScaleFun);

    for(index = 0; index < WEIGHT_NUM; index++)
    {
        if(Capctxt->ScaleFun.HCI >= ScaleRatio[index])
        {
            Cap_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
            break;
        }
        else if(index == WEIGHT_NUM - 1)
            Cap_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
    }

    // Update Error Handle mode
#ifdef SENSOR_DEV   
    Cap_Set_Error_Handleing(0x8064);
    Cap_Set_Wait_Error_Reset();
#else   
    Cap_Set_Error_Handleing(0xF7FF);
    Cap_Set_Wait_Error_Reset();
#endif  
   
    // Update Skip Pattern
    Cap_Set_Skip_Pattern_Reg(Capctxt->Skippattern, Capctxt->SkipPeriod);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);
        
    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Update CAP device.
 */
//=============================================================================
MMP_RESULT
Cap_SetSkipMode(
    CAP_SKIP_MODE mode)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT32  SkipPeriod;

    if(Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }
   
    if (mode == CAPTURE_SKIP_BY_TWO)
    {
        Capctxt->skip_mode = CAPTURE_SKIP_BY_TWO;
        Capctxt->Skippattern = 0xAAAA;
        Capctxt->SkipPeriod = 0xF;
    } 
    else  if (mode == CAPTURE_SKIP_BY_THREE) 
    {
    	Capctxt->skip_mode = CAPTURE_SKIP_BY_THREE;
    	Capctxt->Skippattern = 0x0003;
    	Capctxt->SkipPeriod = 0x2;
    } 
    else if (mode == CAPTURE_SKIP_30FPS_TO_25FPS)
    {
    	Capctxt->skip_mode = CAPTURE_SKIP_30FPS_TO_25FPS;
    	Capctxt->Skippattern = 0x0001;
    	Capctxt->SkipPeriod = 0x5;       
    }
    else if (mode == CAPTURE_SKIP_60FPS_TO_25FPS)
    {
    	Capctxt->skip_mode = CAPTURE_SKIP_60FPS_TO_25FPS;
    	Capctxt->Skippattern = 0x05ad;
    	Capctxt->SkipPeriod = 0xb;          
    }
    else  
    {
        Capctxt->skip_mode = CAPTURE_NO_DROP;
        Capctxt->Skippattern = 0;
        Capctxt->SkipPeriod = 0xF;
    }
  
    // Update Skip Pattern
    Cap_Set_Skip_Pattern_Reg(Capctxt->Skippattern, Capctxt->SkipPeriod);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) error\n", __FUNCTION__, __LINE__);
        
    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * CAP default value initialization.
 */
//=============================================================================
MMP_RESULT
Cap_Initialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    MMP_UINT16  i, j, index;
    MMP_FLOAT   WeightMat[CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];
    MMP_UINT16 data;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        goto end;
    }

    /* RGB to YUV REG */   
    Capctxt->RGBtoYUVFun._11 = 0x004D;
    Capctxt->RGBtoYUVFun._12 = 0x0096;
    Capctxt->RGBtoYUVFun._13 = 0x001D;
    Capctxt->RGBtoYUVFun._21 = 0x07AA;
    Capctxt->RGBtoYUVFun._22 = 0x07b6;
    Capctxt->RGBtoYUVFun._23 = 0x0083;
    Capctxt->RGBtoYUVFun._31 = 0x0083;
    Capctxt->RGBtoYUVFun._32 = 0x0793;
    Capctxt->RGBtoYUVFun._33 = 0x07ec;
    Capctxt->RGBtoYUVFun.ConstY = 0x0000;
    Capctxt->RGBtoYUVFun.ConstU = 0x0080;
    Capctxt->RGBtoYUVFun.ConstV = 0x0080;

    Capctxt->ininfo.PitchY = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.PitchUV = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS;

    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnCSFun = MMP_FALSE;     
    Capctxt->funen.EnInBT656 = MMP_FALSE;
    
    Capctxt->funen.EnUseExtDE = MMP_FALSE;
    Capctxt->funen.EnUseExtVRst = MMP_FALSE;
    Capctxt->funen.EnUseExtHRst = MMP_FALSE;
    
    Capctxt->funen.EnNoHSyncForSensor = MMP_FALSE;  

    Capctxt->funen.EnProgressiveToField = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;
            
    Capctxt->funen.EnYPbPrTopVSMode = MMP_FALSE;            
    Capctxt->funen.EnDlyVS = MMP_FALSE;  
    
    Capctxt->funen.EnPort1UV2LineDS = MMP_TRUE;
                    
    /* Scale */
    Capctxt->ScaleFun.HCI = 0.0f;

    //Initial Scale Weight Matrix
    for(index = 0; index < WEIGHT_NUM; index++)
    {
        _CAP_CreateWeighting(ScaleRatio[index], CAP_SCALE_TAP, CAP_SCALE_TAP_SIZE, WeightMat);

          for(j = 0; j < CAP_SCALE_TAP_SIZE; j++)
              for(i = 0; i < CAP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT16)MMP_FLOATToFix(WeightMat[j][i], 1, 6);
    }

    switch (INPUT_DEVICE)
    {
#ifdef COMPOSITE_DEV        
        case MMP_CAP_DEV_ADV7180:
            _ADV7180_Setting();
            break;
#endif
    
#ifdef COMPONENT_DEV    
        case MMP_CAP_DEV_CAT9883:
            _CAT9883_Setting();
            break;
#endif
           
#ifdef SENSOR_DEV
        case MMP_CAP_DEV_SENSOR:
            _SENSOR_Setting();  
            break;      
#else          
        case MMP_CAP_DEV_HDMIRX:
        #if !defined (EXTERNAL_HDMIRX) && defined(HDMI_LOOPTHROUGH)
            _HDMIRX_LoopThrough_Setting();
        #else
            _HDMIRX_Setting();
        #endif    
            break;
#endif          
        default:
            cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
            result = MMP_RESULT_ERROR;
            break;
    }    
    
    Cap_Set_EnableFrameRate_Reg();
    
    Cap_Set_Output_Driving_Strength_Reg(0x01);
    
end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) Capture Initialize Fail\n", __FUNCTION__, __LINE__);
        
    return result;
}


