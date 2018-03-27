#include "host/host.h"
#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"

#include "isp/isp_config.h"
#include "isp/isp_types.h"
#include "isp/isp_hw.h"
#include "isp/isp.h"
#include "isp/isp_util.h"

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
1920	720	    2.666666667
1080	480	    2.25
1080	576	    1.875
1280	720	    1.777777778
1920	1280	1.5
720	    576	    1.25
576     480	    1.2
1920    1920    1

480	    576	    0.833333333
576	    720	    0.8
1280    1920    0.666666667
720	    1280	0.5625
576     1080    0.533333333
480     1080    0.444444444
720     1920    0.375
*/
#define     WEIGHT_NUM 15
MMP_FLOAT   ScaleRatio[WEIGHT_NUM] = {(1920.0f/720), (1080.0f/480), (1080.0f/576), (1280.0f/720), (1920.0f/1280), (720.0f/576), (576.0f/480), (1920.0f/1920),
                                      (480.0f/576), (576.0f/720), (1280.0f/1920), (720.0f/1280), (576.0f/1080), (480.0f/1080), (720.0f/1920)};
MMP_UINT8   WeightMatInt[WEIGHT_NUM][ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
MMP_UINT8   MotionWeightMatInt_H[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
MMP_UINT8   MotionWeightMatInt_V[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];

MMP_UINT8   WeightMatInt_2TAP[WEIGHT_NUM][ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];

//=============================================================================
//                Private Function Definition
//=============================================================================
//=============================================================================
/**
* Calculate Scale Factor
*/
//=============================================================================
static MMP_FLOAT
_ISP_ScaleFactor(
    MMP_UINT16  Input,
    MMP_UINT16  Output)
{
    return (MMP_FLOAT) (((MMP_INT) (16384.0f*Input/(MMP_FLOAT)Output))/16384.0f);
}

//=============================================================================
/**
* Calculate ISP Deinterlace.
*/
//=============================================================================
static void
_ISP_Deinter_Param(
    ISP_DEINTERLACE_CTRL    *pDeInterlace)
{
    pDeInterlace->DeinterMode = DEINTER3D;
    pDeInterlace->EnableOutMotion = MMP_FALSE;
    pDeInterlace->Disable30MotionDetect = MMP_FALSE;

    pDeInterlace->EnSrcBottomFieldFirst = MMP_FALSE;
    pDeInterlace->EnDeinterBottomField = MMP_TRUE;

    pDeInterlace->EnChromaEdgeDetect = MMP_TRUE;
    pDeInterlace->EnLummaEdgeDetect = MMP_TRUE;
    pDeInterlace->EnSrcLPF = MMP_TRUE;

    pDeInterlace->UVRepeatMode = MMP_FALSE;

    pDeInterlace->EnLowLevelEdge = MMP_FALSE;
    pDeInterlace->LowLevelMode = 0;
    pDeInterlace->EnLowLevelOutside = MMP_FALSE;
    pDeInterlace->LowLevelBypassBlend = 0;

    pDeInterlace->LowLevelPosX = 0;
    pDeInterlace->LowLevelPosY = 425;
    pDeInterlace->LowLevelWidth = 704;
    pDeInterlace->LowLevelHeight = 55;
    
    if (pDeInterlace->DeinterMode == DEINTER3D)
    	pDeInterlace->EnUV2DMethod = MMP_TRUE;
    else
    	pDeInterlace->EnUV2DMethod = MMP_FALSE;
}

static void
_ISP_Deinter3D_Param(
    ISP_DEINTERLACE_CTRL    *pDeInterlace)
{
    MMP_UINT16      MDThreshold_High;
    MMP_UINT16      MDThreshold_Low;

    MDThreshold_Low = 8;
    MDThreshold_High = 16;

    pDeInterlace->MDThreshold_Low = MDThreshold_Low;
    pDeInterlace->MDThreshold_High = MDThreshold_High;
    pDeInterlace->MDThreshold_Step = (MMP_INT)((MMP_FLOAT)128.0f * 1.0f / (MDThreshold_High - MDThreshold_Low));

    pDeInterlace->EnLPFWeight = MMP_TRUE;
    pDeInterlace->EnLPFWeight = MMP_TRUE;
    pDeInterlace->EnLPFStaticPixel = MMP_TRUE;

    pDeInterlace->DisableMV_A = MMP_FALSE;
    pDeInterlace->DisableMV_B = MMP_FALSE;
    pDeInterlace->DisableMV_C = MMP_FALSE;
    pDeInterlace->DisableMV_D = MMP_FALSE;
    pDeInterlace->DisableMV_E = MMP_FALSE;
    pDeInterlace->DisableMV_F = MMP_FALSE;
    pDeInterlace->DisableMV_G = MMP_FALSE;  
}

static void
_ISP_Deinter2D_Param(
    ISP_DEINTERLACE_CTRL    *pDeInterlace)
{
    MMP_UINT16      EdgeBlendWeight;
    MMP_UINT16      OrgBlendWeight;

    EdgeBlendWeight = 8;
    OrgBlendWeight = 8;

    pDeInterlace->D2EdgeBlendWeight = (MMP_INT)((MMP_FLOAT)64.0f * EdgeBlendWeight / (EdgeBlendWeight + OrgBlendWeight));
    pDeInterlace->D2OrgBlendWeight = (MMP_INT)((MMP_FLOAT)64.0f * OrgBlendWeight / (EdgeBlendWeight + OrgBlendWeight));   
}

//=============================================================================
/**
* Calculate ISP Scaling Factor.
*/
//=============================================================================
static void
_ISP_CalScaleHCI_VCI(
    void)
{
    MMP_FLOAT   HCI;
    MMP_FLOAT   VCI;

    HCI = _ISP_ScaleFactor(ISPctxt->InInfo.SrcWidth, ISPctxt->OutInfo.Width);
    VCI = _ISP_ScaleFactor(ISPctxt->InInfo.SrcHeight, ISPctxt->OutInfo.Height);

    if(HCI != ISPctxt->ScaleFun.HCI)
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixH;

    if(VCI != ISPctxt->ScaleFun.VCI)
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixV;

    ISPctxt->ScaleFun.HCI = HCI;
    ISPctxt->ScaleFun.VCI = VCI;
}

//=============================================================================
/**
 * Create weighting for the matrix of scaling.
*/
//=============================================================================
static void
_ISP_CreateWeighting(
    MMP_FLOAT   scale,
    MMP_UINT8   taps,
    MMP_UINT8   tapSize,
    //MMP_FLOAT   weightMatrix[][ISP_SCALE_TAP])
    MMP_FLOAT   **weightMatrix)
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

                //printf("i:%2d   W=%6.3f   point=%2d    WW=%6.3f\n",
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
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() unknow error !\n", __FUNCTION__);
    }
}


//=============================================================================
/**
 * ISP update hardware register.
 */
//=============================================================================
static void
_ISP_UpdateHwReg(
    void)
{
    MMP_UINT16  index;

    //
    //Input Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputParameter)
        ISP_SetInputParameter_Reg(&ISPctxt->InInfo);

    //
    //Input Width, Height, Pitch
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputBuf)
        ISP_SetInputBuf_Reg(&ISPctxt->InInfo);

    //
    //Input Address
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputAddr)
        ISP_SetInputAddr_Reg(&ISPctxt->InInfo);

    //
    //Deinterlace Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_DeInterlaceParam)
        ISP_SetDeInterlaceParam_Reg(&ISPctxt->DeInterlace);

    //
    //Jpeg Encode Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_JpegEncode)
        ISP_SetJpegEncode_Reg(&ISPctxt->JpegEncode);

    //
    //Color Correction Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_CCMatrix)
    {
        ISP_SetCCMatrix_Reg(&ISPctxt->CCFun);
        ISPctxt->UpdateFlags &= (~ISP_FLAGS_UPDATE_CCMatrix);
    }

    //
    //Scale Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_ScaleParam)
    {
        _ISP_CalScaleHCI_VCI();
        ISP_SetScaleParam_Reg(&ISPctxt->ScaleFun);
    }

    //
    //Scale Horizontal Matrix
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_ScaleMatrixH)
    {
        if (ISPctxt->DeInterlace.EnableOutMotion)
        {
            ISP_SetIntScaleMatrixH_Reg(MotionWeightMatInt_H);
        }
        else
        {
        for(index = 0; index < WEIGHT_NUM; index++)
        {
            if(ISPctxt->ScaleFun.HCI >= ScaleRatio[index])
            {
                if (ISPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    ISP_SetIntScaleMatrixH_Reg(WeightMatInt_2TAP[index]);
                else
                ISP_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
                break;
            }
            else if(index == WEIGHT_NUM - 1)
            {
                if (ISPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    ISP_SetIntScaleMatrixH_Reg(WeightMatInt_2TAP[index]);
                else
                ISP_SetIntScaleMatrixH_Reg(WeightMatInt[index]);
        }
    }
    }
    }

    //
    //Scale Vertical Matrix
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_ScaleMatrixV)
    {
        if (ISPctxt->DeInterlace.EnableOutMotion)
        {
            ISP_SetIntScaleMatrixV_Reg(MotionWeightMatInt_V);
        }
        else
        {
        for(index = 0; index < WEIGHT_NUM; index++)
        {
            if(ISPctxt->ScaleFun.VCI >= ScaleRatio[index])
            {
                if (ISPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    ISP_SetIntScaleMatrixV_Reg(WeightMatInt_2TAP[index]);
                else 
                ISP_SetIntScaleMatrixV_Reg(WeightMatInt[index]);
                break;
            }
            else if(index == WEIGHT_NUM - 1)
            {
                if (ISPctxt->OutInfo.EnableFieldMode == MMP_TRUE)
                    ISP_SetIntScaleMatrixV_Reg(WeightMatInt_2TAP[index]);
                else
                ISP_SetIntScaleMatrixV_Reg(WeightMatInt[index]);
        }
    }
    }
    }

    //
    //YUV to RGB Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_FrmMatrix)
        ISP_SetFrmMatrix_Reg(&ISPctxt->FrmMatrix);

    //
    //Frame Function 0
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_FrameFun0)
        ISP_SetFrameFun0_Reg(&ISPctxt->FrameFun0);

    //
    //YUV to RGB Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_RGBtoYUVMatrix)
        ISP_SetRGBtoYUVMatrix_Reg(&ISPctxt->RGB2YUVFun);

    //
    //Output Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutParameter)
        ISP_SetOutParameter_Reg(&ISPctxt->OutInfo);

    //
    //Output Width, Height and Pitch
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutBufInfo)
        ISP_SetOutBufInfo_Reg(&ISPctxt->OutInfo);

    //
    //Output Address
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutAddress)
        ISP_SetOutAddress_Reg(&ISPctxt->OutInfo);

    //
    //Remap Address
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_RemapAddr)
    {
        ISP_SetRemapYAddress_Reg(&ISPctxt->RemapYAddr);
        ISP_SetRemapUVAddress_Reg(&ISPctxt->RemapUVAddr);
    }

    //
    //ISP Interrupt
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_Interrupt)
    {
        ISP_SetInterruptParameter_Reg(ISPctxt);
    }

    ISP_RefreshFire_Reg();
}

//=============================================================================
//                Public Function Definition
//=============================================================================


//=============================================================================
/**
 * ISP default value initialization.
 */
//=============================================================================
MMP_RESULT
ISP_ContextInitialize(
    MMP_BOOL initialMatrix)
{
    ISP_RESULT  result = ISP_SUCCESS;
    MMP_UINT16  i, j, index;
    MMP_FLOAT   **WeightMat;
    MMP_FLOAT   **WeightMat_2TAP;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    //Input format
    ISPctxt->InInfo.NVFormat = NV12;
    ISPctxt->InInfo.EnableInYUV255Range = MMP_FALSE;
    ISPctxt->InInfo.DisableCaptureCtrl = MMP_TRUE;
    ISPctxt->InInfo.InputBufferNum = 0;

    //Deinterlace Paramter
    _ISP_Deinter_Param(&ISPctxt->DeInterlace);
    _ISP_Deinter3D_Param(&ISPctxt->DeInterlace);
    _ISP_Deinter2D_Param(&ISPctxt->DeInterlace);

    //Color Correction
    ISPctxt->CCFun.OffsetR  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.OffsetG  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.OffsetB  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun._11      = MMP_FLOATToFix(1.0f, 4, 8);
    ISPctxt->CCFun._12      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._13      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._21      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._22      = MMP_FLOATToFix(1.0f, 4, 8);
    ISPctxt->CCFun._23      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._31      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._32      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._33      = MMP_FLOATToFix(1.0f, 4, 8);
    ISPctxt->CCFun.DeltaR   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.DeltaG   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.DeltaB   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->InInfo.EnableCCFun = MMP_TRUE;

    //Scale
    ISPctxt->ScaleFun.HCI = 0.0f;
    ISPctxt->ScaleFun.VCI = 0.0f;

    //FrmFun RGB to YUV   Input-> 0-255 Output-> 0-255
    ISPctxt->FrmMatrix._11 = 0x004D;
    ISPctxt->FrmMatrix._12 = 0x0096;
    ISPctxt->FrmMatrix._13 = 0x001D;
    ISPctxt->FrmMatrix._21 = 0x03d4;
    ISPctxt->FrmMatrix._22 = 0x03a9;
    ISPctxt->FrmMatrix._23 = 0x0083;
    ISPctxt->FrmMatrix._31 = 0x0083;
    ISPctxt->FrmMatrix._32 = 0x0392;
    ISPctxt->FrmMatrix._33 = 0x03eb;
    ISPctxt->FrmMatrix.ConstY = 0x0000;
    ISPctxt->FrmMatrix.ConstU = 0x0080;
    ISPctxt->FrmMatrix.ConstV = 0x0080;

    //Output format
    ISPctxt->OutInfo.OutFormat = NVMode;
    ISPctxt->OutInfo.NVFormat = NV12;
    ISPctxt->OutInfo.SWWrFlipNum = 0;
    ISPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;
    ISPctxt->OutInfo.OutputBufferNum = 0;
    ISPctxt->OutInfo.EnableFieldMode = MMP_FALSE;

    ISPctxt->OutInfo.EngineDelay = 0;  // 0 - 15

    ISPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;
    ISPctxt->OutInfo.EnableRemapYAddr = MMP_FALSE;
    ISPctxt->OutInfo.EnableRemapUVAddr = MMP_FALSE;
    ISPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_FALSE;
    ISPctxt->OutInfo.DisableOutMatrix = MMP_FALSE;

    if (ISPctxt->InInfo.EnableInYUV255Range)
    {
        //YUV 0-255 ---> 16-235
        ISPctxt->RGB2YUVFun._11 = 0x00DC;
        ISPctxt->RGB2YUVFun._12 = 0x0000;
        ISPctxt->RGB2YUVFun._13 = 0x0000;
        ISPctxt->RGB2YUVFun._21 = 0x0000;
        ISPctxt->RGB2YUVFun._22 = 0x00E1;
        ISPctxt->RGB2YUVFun._23 = 0x0000;
        ISPctxt->RGB2YUVFun._31 = 0x0000;
        ISPctxt->RGB2YUVFun._32 = 0x0000;
        ISPctxt->RGB2YUVFun._33 = 0x00E1;
        ISPctxt->RGB2YUVFun.ConstY = 0x0010;
        ISPctxt->RGB2YUVFun.ConstU = 0x0010;
        ISPctxt->RGB2YUVFun.ConstV = 0x0010;
    }
    else
    {
        //YUV 0-255 ---> 0-255
        ISPctxt->RGB2YUVFun._11 = 0x0100;
        ISPctxt->RGB2YUVFun._12 = 0x0000;
        ISPctxt->RGB2YUVFun._13 = 0x0000;
        ISPctxt->RGB2YUVFun._21 = 0x0000;
        ISPctxt->RGB2YUVFun._22 = 0x0100;
        ISPctxt->RGB2YUVFun._23 = 0x0000;
        ISPctxt->RGB2YUVFun._31 = 0x0000;
        ISPctxt->RGB2YUVFun._32 = 0x0000;
        ISPctxt->RGB2YUVFun._33 = 0x0100;
        ISPctxt->RGB2YUVFun.ConstY = 0x0000;
        ISPctxt->RGB2YUVFun.ConstU = 0x0000;
        ISPctxt->RGB2YUVFun.ConstV = 0x0000;                
    }

    //Type2 Frame based Tiled Map, Vertical Addressing Luma
    ISPctxt->RemapYAddr.Addr_03 = (0x0 << 5) | 11;
    ISPctxt->RemapYAddr.Addr_04 = (0x0 << 5) | 12;
    ISPctxt->RemapYAddr.Addr_05 = (0x0 << 5) | 13;
    ISPctxt->RemapYAddr.Addr_06 = (0x0 << 5) | 14;
    ISPctxt->RemapYAddr.Addr_07 = (0x0 << 5) | 3;
    ISPctxt->RemapYAddr.Addr_08 = (0x0 << 5) | 4;
    ISPctxt->RemapYAddr.Addr_09 = (0x0 << 5) | 5;
    ISPctxt->RemapYAddr.Addr_10 = (0x0 << 5) | 15;
    ISPctxt->RemapYAddr.Addr_11 = (0x0 << 5) | 6;
    ISPctxt->RemapYAddr.Addr_12 = (0x0 << 5) | 16;
    ISPctxt->RemapYAddr.Addr_13 = (0x0 << 5) | 7;
    ISPctxt->RemapYAddr.Addr_14 = (0x0 << 5) | 8;
    ISPctxt->RemapYAddr.Addr_15 = (0x0 << 5) | 9;
    ISPctxt->RemapYAddr.Addr_16 = (0x0 << 5) | 10;
    ISPctxt->RemapYAddr.Addr_17 = (0x0 << 5) | 17;
    ISPctxt->RemapYAddr.Addr_18 = (0x0 << 5) | 18;
    ISPctxt->RemapYAddr.Addr_19 = (0x0 << 5) | 19;
    ISPctxt->RemapYAddr.Addr_20 = (0x0 << 5) | 20;
    ISPctxt->RemapYAddr.Addr_21 = (0x0 << 5) | 21;
    ISPctxt->RemapYAddr.Addr_22 = (0x0 << 5) | 22;
    ISPctxt->RemapYAddr.Addr_23 = (0x0 << 5) | 23;
    ISPctxt->RemapYAddr.Addr_24 = (0x0 << 5) | 24;
    ISPctxt->RemapYAddr.Addr_25 = (0x0 << 5) | 25;
    ISPctxt->RemapYAddr.Addr_26 = (0x0 << 5) | 26;
    ISPctxt->RemapYAddr.Addr_27 = (0x0 << 5) | 27;
    ISPctxt->RemapYAddr.Addr_28 = (0x0 << 5) | 28;
    ISPctxt->RemapYAddr.Addr_29 = (0x0 << 5) | 29;
    ISPctxt->RemapYAddr.Addr_30 = (0x0 << 5) | 30;
    ISPctxt->RemapYAddr.Addr_31 = (0x0 << 5) | 31;

    //Type2 Frame based Tiled Map, Vertical Addressing Chroma
    ISPctxt->RemapUVAddr.Addr_03 = (0x0 << 5) | 11;
    ISPctxt->RemapUVAddr.Addr_04 = (0x0 << 5) | 12;
    ISPctxt->RemapUVAddr.Addr_05 = (0x0 << 5) | 13;
    ISPctxt->RemapUVAddr.Addr_06 = (0x0 << 5) | 14;
    ISPctxt->RemapUVAddr.Addr_07 = (0x0 << 5) | 3;
    ISPctxt->RemapUVAddr.Addr_08 = (0x0 << 5) | 4;
    ISPctxt->RemapUVAddr.Addr_09 = (0x0 << 5) | 5;
    ISPctxt->RemapUVAddr.Addr_10 = (0x0 << 5) | 16;
    ISPctxt->RemapUVAddr.Addr_11 = (0x1 << 5) | 6;
    ISPctxt->RemapUVAddr.Addr_12 = (0x1 << 5) | 15;
    ISPctxt->RemapUVAddr.Addr_13 = (0x0 << 5) | 7;
    ISPctxt->RemapUVAddr.Addr_14 = (0x0 << 5) | 8;
    ISPctxt->RemapUVAddr.Addr_15 = (0x0 << 5) | 9;
    ISPctxt->RemapUVAddr.Addr_16 = (0x0 << 5) | 10;
    ISPctxt->RemapUVAddr.Addr_17 = (0x0 << 5) | 17;
    ISPctxt->RemapUVAddr.Addr_18 = (0x0 << 5) | 18;
    ISPctxt->RemapUVAddr.Addr_19 = (0x0 << 5) | 19;
    ISPctxt->RemapUVAddr.Addr_20 = (0x0 << 5) | 20;
    ISPctxt->RemapUVAddr.Addr_21 = (0x0 << 5) | 21;
    ISPctxt->RemapUVAddr.Addr_22 = (0x0 << 5) | 22;
    ISPctxt->RemapUVAddr.Addr_23 = (0x0 << 5) | 23;
    ISPctxt->RemapUVAddr.Addr_24 = (0x0 << 5) | 24;
    ISPctxt->RemapUVAddr.Addr_25 = (0x0 << 5) | 25;
    ISPctxt->RemapUVAddr.Addr_26 = (0x0 << 5) | 26;
    ISPctxt->RemapUVAddr.Addr_27 = (0x0 << 5) | 27;
    ISPctxt->RemapUVAddr.Addr_28 = (0x0 << 5) | 28;
    ISPctxt->RemapUVAddr.Addr_29 = (0x0 << 5) | 29;
    ISPctxt->RemapUVAddr.Addr_30 = (0x0 << 5) | 30;
    ISPctxt->RemapUVAddr.Addr_31 = (0x0 << 5) | 31;

    //ISP Interrupt
    ISPctxt->EnableInterrupt = MMP_FALSE;
    ISPctxt->InterruptMode = 0x0;

    //Color Contrl
    ISPctxt->ColorCtrl.brightness = 0;
    ISPctxt->ColorCtrl.contrast = 1.0;
    ISPctxt->ColorCtrl.hue = 0;
    ISPctxt->ColorCtrl.saturation = 1.0;
    ISPctxt->ColorCtrl.colorEffect[0] = 0;
    ISPctxt->ColorCtrl.colorEffect[1] = 0;

    if (initialMatrix)
    {
    get_mem2Dfloat(&WeightMat, ISP_SCALE_TAP_SIZE, ISP_SCALE_TAP);
    get_mem2Dfloat(&WeightMat_2TAP, ISP_SCALE_TAP_SIZE, 2);

    //Initial Video Weight Matrix
    for(index = 0; index < WEIGHT_NUM; index++)
    {
        _ISP_CreateWeighting(ScaleRatio[index], ISP_SCALE_TAP, ISP_SCALE_TAP_SIZE, WeightMat);

        for(j = 0; j < ISP_SCALE_TAP_SIZE; j++)
            for(i = 0; i < ISP_SCALE_TAP; i++)
                WeightMatInt[index][j][i] = (MMP_UINT8)MMP_FLOATToFix(WeightMat[j][i], 1, 6);
    }

    //Initial Video Weight Matrix 2Tap
    for(index = 0; index < WEIGHT_NUM; index++)
    {
        _ISP_CreateWeighting(ScaleRatio[index], 2, ISP_SCALE_TAP_SIZE, WeightMat_2TAP);

        for(j = 0; j < ISP_SCALE_TAP_SIZE; j++)
            for(i = 0; i < ISP_SCALE_TAP; i++)
            {
                if (i == 1 || i == 2)
                    WeightMatInt_2TAP[index][j][i] = (MMP_UINT8)MMP_FLOATToFix(WeightMat_2TAP[j][i-1], 1, 6);
                else
                    WeightMatInt_2TAP[index][j][i] = 0x00;                  
            }
    }
    
    free_mem2Dfloat(WeightMat);
    free_mem2Dfloat(WeightMat_2TAP);
    }

    ISPctxt->UpdateFlags = 0xFFFFFFFF;
end:
    if(result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
* Set Motion Detection Parameter.
**/
//=============================================================================
void
ISP_EnableMotionDetectionParameter(
    void)
{
    //Enable Motion Detection
    ISPctxt->DeInterlace.Enable = MMP_TRUE;
    ISPctxt->DeInterlace.EnableOutMotion = MMP_TRUE;
    ISPctxt->DeInterlace.DeinterMode = DEINTER3D;
    ISPctxt->DeInterlace.EnUV2DMethod = MMP_FALSE;

    ISPctxt->DeInterlace.MDThreshold_Low = 12;
    ISPctxt->DeInterlace.MDThreshold_High = 12;
    ISPctxt->DeInterlace.MDThreshold_Step = (MMP_INT)((MMP_FLOAT)128.0f * 1.0f / (ISPctxt->DeInterlace.MDThreshold_High - ISPctxt->DeInterlace.MDThreshold_Low));

    ISPctxt->DeInterlace.EnLPFWeight = MMP_TRUE;
    ISPctxt->DeInterlace.EnLPFStaticPixel = MMP_FALSE;

    ISPctxt->DeInterlace.DisableMV_A = MMP_TRUE;
    ISPctxt->DeInterlace.DisableMV_B = MMP_TRUE;
    ISPctxt->DeInterlace.DisableMV_C = MMP_TRUE;
    ISPctxt->DeInterlace.DisableMV_D = MMP_FALSE;
    ISPctxt->DeInterlace.DisableMV_E = MMP_FALSE;
    ISPctxt->DeInterlace.DisableMV_F = MMP_FALSE;
    ISPctxt->DeInterlace.DisableMV_G = MMP_FALSE;

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;

    //Color Correction
    ISPctxt->CCFun.OffsetR  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.OffsetG  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.OffsetB  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun._11      = MMP_FLOATToFix(-1.0f, 4, 8);
    ISPctxt->CCFun._12      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._13      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._21      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._22      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._23      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._31      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._32      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._33      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun.DeltaR   = MMP_FLOATToFix(255.0f, 8, 0);
    ISPctxt->CCFun.DeltaG   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.DeltaB   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_CCMatrix;

    MotionWeightMatInt_H[0][0] = 0x3;
    MotionWeightMatInt_H[0][1] = 0x3;
    MotionWeightMatInt_H[0][2] = 0x3;
    MotionWeightMatInt_H[0][3] = 0x3;

    MotionWeightMatInt_H[1][0] = 0x3;
    MotionWeightMatInt_H[1][1] = 0x3;
    MotionWeightMatInt_H[1][2] = 0x3;
    MotionWeightMatInt_H[1][3] = 0x3;

    MotionWeightMatInt_H[2][0] = 0x3;
    MotionWeightMatInt_H[2][1] = 0x3;
    MotionWeightMatInt_H[2][2] = 0x3;
    MotionWeightMatInt_H[2][3] = 0x3;

    MotionWeightMatInt_H[3][0] = 0x3;
    MotionWeightMatInt_H[3][1] = 0x3;
    MotionWeightMatInt_H[3][2] = 0x3;
    MotionWeightMatInt_H[3][3] = 0x3;

    MotionWeightMatInt_H[4][0] = 0x3;
    MotionWeightMatInt_H[4][1] = 0x3;
    MotionWeightMatInt_H[4][2] = 0x3;
    MotionWeightMatInt_H[4][3] = 0x3;

    MotionWeightMatInt_V[0][0] = 0x40;
    MotionWeightMatInt_V[0][1] = 0x40;
    MotionWeightMatInt_V[0][2] = 0x40;
    MotionWeightMatInt_V[0][3] = 0x40;

    MotionWeightMatInt_V[1][0] = 0x40;
    MotionWeightMatInt_V[1][1] = 0x40;
    MotionWeightMatInt_V[1][2] = 0x40;
    MotionWeightMatInt_V[1][3] = 0x40;

    MotionWeightMatInt_V[2][0] = 0x40;
    MotionWeightMatInt_V[2][1] = 0x40;
    MotionWeightMatInt_V[2][2] = 0x40;
    MotionWeightMatInt_V[2][3] = 0x40;

    MotionWeightMatInt_V[3][0] = 0x40;
    MotionWeightMatInt_V[3][1] = 0x40;
    MotionWeightMatInt_V[3][2] = 0x40;
    MotionWeightMatInt_V[3][3] = 0x40;

    MotionWeightMatInt_V[4][0] = 0x40;
    MotionWeightMatInt_V[4][1] = 0x40;
    MotionWeightMatInt_V[4][2] = 0x40;
    MotionWeightMatInt_V[4][3] = 0x40;

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixH;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixV;

    //YUV 0-255 ---> 16-235
    ISPctxt->RGB2YUVFun._11 = 0x0016;
    ISPctxt->RGB2YUVFun._12 = 0x0000;
    ISPctxt->RGB2YUVFun._13 = 0x0000;
    ISPctxt->RGB2YUVFun._21 = 0x0000;
    ISPctxt->RGB2YUVFun._22 = 0x0000;
    ISPctxt->RGB2YUVFun._23 = 0x0000;
    ISPctxt->RGB2YUVFun._31 = 0x0000;
    ISPctxt->RGB2YUVFun._32 = 0x0000;
    ISPctxt->RGB2YUVFun._33 = 0x0000;
    ISPctxt->RGB2YUVFun.ConstY = 0x0000;
    ISPctxt->RGB2YUVFun.ConstU = 0x0000;
    ISPctxt->RGB2YUVFun.ConstV = 0x0000;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_RGBtoYUVMatrix;
}

//=============================================================================
/**
* Clear Motion Detection Parameter.
**/
//=============================================================================
void
ISP_DisableMotionDetectionParameter(
    void)
{
    //Disable Motion Detection
    ISPctxt->DeInterlace.Enable = MMP_FALSE;

    //Deinterlace Paramter
    _ISP_Deinter_Param(&ISPctxt->DeInterlace);
    _ISP_Deinter3D_Param(&ISPctxt->DeInterlace);
    _ISP_Deinter2D_Param(&ISPctxt->DeInterlace);

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixH;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleMatrixV;

    //Color Correction
    ISPctxt->CCFun.OffsetR  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.OffsetG  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.OffsetB  = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun._11      = MMP_FLOATToFix(1.0f, 4, 8);
    ISPctxt->CCFun._12      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._13      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._21      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._22      = MMP_FLOATToFix(1.0f, 4, 8);
    ISPctxt->CCFun._23      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._31      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._32      = MMP_FLOATToFix(0.0f, 4, 8);
    ISPctxt->CCFun._33      = MMP_FLOATToFix(1.0f, 4, 8);
    ISPctxt->CCFun.DeltaR   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.DeltaG   = MMP_FLOATToFix(0.0f, 8, 0);
    ISPctxt->CCFun.DeltaB   = MMP_FLOATToFix(0.0f, 8, 0);

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_CCMatrix;

    if (ISPctxt->InInfo.EnableInYUV255Range)
    {
        //YUV 0-255 ---> 16-235
        ISPctxt->RGB2YUVFun._11 = 0x00DC;
        ISPctxt->RGB2YUVFun._12 = 0x0000;
        ISPctxt->RGB2YUVFun._13 = 0x0000;
        ISPctxt->RGB2YUVFun._21 = 0x0000;
        ISPctxt->RGB2YUVFun._22 = 0x00E1;
        ISPctxt->RGB2YUVFun._23 = 0x0000;
        ISPctxt->RGB2YUVFun._31 = 0x0000;
        ISPctxt->RGB2YUVFun._32 = 0x0000;
        ISPctxt->RGB2YUVFun._33 = 0x00E1;
        ISPctxt->RGB2YUVFun.ConstY = 0x0010;
        ISPctxt->RGB2YUVFun.ConstU = 0x0010;
        ISPctxt->RGB2YUVFun.ConstV = 0x0010;
    }
    else
    {
        //YUV 0-255 ---> 0-255
        ISPctxt->RGB2YUVFun._11 = 0x0100;
        ISPctxt->RGB2YUVFun._12 = 0x0000;
        ISPctxt->RGB2YUVFun._13 = 0x0000;
        ISPctxt->RGB2YUVFun._21 = 0x0000;
        ISPctxt->RGB2YUVFun._22 = 0x0100;
        ISPctxt->RGB2YUVFun._23 = 0x0000;
        ISPctxt->RGB2YUVFun._31 = 0x0000;
        ISPctxt->RGB2YUVFun._32 = 0x0000;
        ISPctxt->RGB2YUVFun._33 = 0x0100;
        ISPctxt->RGB2YUVFun.ConstY = 0x0000;
        ISPctxt->RGB2YUVFun.ConstU = 0x0000;
        ISPctxt->RGB2YUVFun.ConstV = 0x0000;                
    }
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_RGBtoYUVMatrix;
}

//=============================================================================
/**
* Set isp input format.
**/
//=============================================================================
MMP_RESULT
ISP_SetInputFormat(
    MMP_ISP_INFORMAT    format)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if(ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }
    switch(format)
    {
        case MMP_ISP_IN_NV12:
            ISPctxt->InInfo.NVFormat = NV12;
            break;

        case MMP_ISP_IN_NV21:
            ISPctxt->InInfo.NVFormat = NV21;
            break;

        default:
            result = ISP_ERR_NO_MATCH_INPUT_FORMAT;
            break;
    }

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
end:

    if(result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}


//=============================================================================
/**
* Set isp output format.
**/
//=============================================================================
MMP_RESULT
ISP_SetOutputFormat(
    MMP_ISP_OUTFORMAT   format)
{
    ISP_RESULT   result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch(format)
    {
        case MMP_ISP_OUT_YUV422:
            ISPctxt->OutInfo.OutFormat = YUVPlane;
            ISPctxt->OutInfo.PlaneFormat = YUV422;
            break;

        case MMP_ISP_OUT_YUV420:
            ISPctxt->OutInfo.OutFormat = YUVPlane;
            ISPctxt->OutInfo.PlaneFormat = YUV420;
            break;

        case MMP_ISP_OUT_YUV444:
            ISPctxt->OutInfo.OutFormat = YUVPlane;
            ISPctxt->OutInfo.PlaneFormat = YUV444;
            break;

        case MMP_ISP_OUT_YUV422R:
            ISPctxt->OutInfo.OutFormat = YUVPlane;
            ISPctxt->OutInfo.PlaneFormat = YUV422R;
            break;

        case MMP_ISP_OUT_NV12:
            ISPctxt->OutInfo.OutFormat = NVMode;
            ISPctxt->OutInfo.NVFormat = NV12;
            break;

        case MMP_ISP_OUT_NV21:
            ISPctxt->InInfo.NVFormat = NVMode;
            ISPctxt->OutInfo.NVFormat = NV21;
            break;

        default:
            result = ISP_ERR_NO_MATCH_OUTPUT_FORMAT;
            break;
    }

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;

end:
    if(result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Update ISP device.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
ISP_Update(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if(ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    // Update ISP Scale Parameter
    if ((ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_InputBuf) ||
        (ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_OutBufInfo))
    {
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_ScaleParam;
    }

    // Update ISP hardware register
    if(ISPctxt->UpdateFlags)
        _ISP_UpdateHwReg();

    // Clear Update Flags
    if (ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_CCMatrix)
        ISPctxt->UpdateFlags = (0x0 | ISP_FLAGS_UPDATE_CCMatrix);
    else
    ISPctxt->UpdateFlags = 0x0;

end:

    if(result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
// brightness:     -128 ~ 127     default : 0
// contrast:       0.0 ~ 4.0      default : 1.0
// hue:            0 ~ 359        default : 0
// saturation:     0.0 ~ 4.0      default : 1.0
// colorEffect[2]: -128 ~ 128     default : 0, 0

// preOff:  S8
// M:       S4.8
// postOff: S8
*/
//=============================================================================
#if defined (USE_COLOR_EFFECT)
void
ISP_SetColorCorrMatrix(
    ISP_COLOR_CORRECTION  *pColorCorrect,
    MMP_INT32 brightness,
    MMP_FLOAT contrast,
    MMP_INT32 hue,
    MMP_FLOAT saturation,
    MMP_INT32 colorEffect[2])
{
    MMP_INT32 preOff[3];
    MMP_INT32 M[3][3];
    MMP_INT32 postOff[3];
    MMP_FLOAT cosTh, sinTh;

    preOff[0] = preOff[1] = preOff[2] = -128;

    M[0][0] = (int)(contrast * 256 + 0.5);
    M[0][1] = M[0][2] = 0;
    getSinCos(hue, &sinTh, &cosTh);
    M[1][0] = 0;
    M[1][1] = (int)(saturation * cosTh * 256 + 0.5);
    M[1][2] = (int)(saturation * -sinTh * 256 + 0.5);
    M[2][0] = 0;
    M[2][1] = (int)(saturation * sinTh * 256 + 0.5);
    M[2][2] = (int)(saturation * cosTh * 256 + 0.5);

    postOff[0] = (int)(contrast * brightness + 128.5);
    postOff[1] = colorEffect[0] + 128;
    postOff[2] = colorEffect[1] + 128;

    pColorCorrect->OffsetR  = preOff[0];
    pColorCorrect->OffsetG  = preOff[1];
    pColorCorrect->OffsetB  = preOff[2];
    pColorCorrect->_11      = M[0][0];
    pColorCorrect->_12      = M[0][1];
    pColorCorrect->_13      = M[0][2];
    pColorCorrect->_21      = M[1][0];
    pColorCorrect->_22      = M[1][1];
    pColorCorrect->_23      = M[1][2];
    pColorCorrect->_31      = M[2][0];
    pColorCorrect->_32      = M[2][1];
    pColorCorrect->_33      = M[2][2];
    pColorCorrect->DeltaR   = postOff[0];
    pColorCorrect->DeltaG   = postOff[1];
    pColorCorrect->DeltaB   = postOff[2];
}

//=============================================================================
/**
 * Update ISP color matrix device.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
ISP_UpdateColorMatrix(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if(ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        goto end;
    }

    //
    //Color Correction Parameter
    //
    if(ISPctxt->UpdateFlags & ISP_FLAGS_UPDATE_CCMatrix)
    {
        ISP_SetCCMatrix_Reg(&ISPctxt->CCFun);
        ISPctxt->UpdateFlags &= (~ISP_FLAGS_UPDATE_CCMatrix);
    }

end:
    if(result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}
#endif
