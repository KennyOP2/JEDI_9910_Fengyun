#include "host/host.h"
#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"

#include "isp/isp_config.h"
#include "isp/isp_hw.h"
#include "isp/isp_reg.h"
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

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
ISP_LogReg(
    void)
{
    MMP_UINT16  reg, p;
    MMP_UINT    i, j, count;

    reg     = ISP_REG_BASE;
    count   = (0x06FE - reg) / sizeof(MMP_UINT16);

    j = 0;
    p = reg;
    //printf("\r\n");
    printf( "\n\t   0    2    4    6    8    A    C    E\r\n");

    for(i = 0; i < count; ++i)
    {
        MMP_UINT16 value = 0;

        HOST_ReadRegister(p, &value);
        if( j == 0 )
            printf("0x%04X:", p);

        printf(" %04X", value);

        if( j >= 7 )
        {
            printf("\r\n");
            j = 0;
        }
        else
            j++;

        p += 2;
    }

    if( j > 0 )
        printf("\r\n");

}
//=============================================================================
/**
 * Clear ISP Interrupt.
 */
//=============================================================================
void
ISP_ClearInterrupt_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
    
    Value = ((0x1 & ISP_BIT_ISP_INTERRUPT_CLEAR) << ISP_SHT_ISP_INTERRUPT_CLEAR);
            
    HOST_WriteRegister(ISP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Driver Fire ISP Engine.
 */
//=============================================================================
void
ISP_DriverFire_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
    
    Value = ((0x1 & ISP_BIT_DRIVER_FIRE_EN) << ISP_SHT_DRIVER_FIRE_EN);     
            
    HOST_WriteRegister(ISP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Driver Refresh ISP Parameter.
 */
//=============================================================================
void
ISP_RefreshFire_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
    
    Value = ((0x1 & ISP_BIT_ISP_REFRESH_PARM) << ISP_SHT_ISP_REFRESH_PARM);
            
    HOST_WriteRegister(ISP_REG_SET500, (MMP_UINT16)Value);
}

//=============================================================================
/**
* Driver Update ISP Parameter.
*/
//=============================================================================
void
ISP_UpdateFire_Reg(
    void)
{
    HOST_WriteRegister(ISP_REG_SET500, (MMP_UINT16)((0x1 & ISP_BIT_ISP_UPDATE_PARM_EN) << ISP_SHT_ISP_UPDATE_PARM_EN));
}

//=============================================================================
/**
* Set Input Format
*/
//=============================================================================
void
ISP_SetInputParameter_Reg(
    const ISP_INPUT_INFO    *pInInfo)
{
    MMP_UINT16  Value = 0;

    Value = ((pInInfo->EnableReadMemoryMode & ISP_BIT_READ_MEM_MODE_EN) << ISP_SHT_READ_MEM_MODE_EN) | 
            ((pInInfo->EnableInYUV255Range & ISP_BIT_IN_YUV255RANGE_EN) << ISP_SHT_IN_YUV255RANGE_EN) | 
            ((pInInfo->EnableCCFun & ISP_BIT_COLOR_CORRECT_EN) << ISP_SHT_COLOR_CORRECT_EN) |    
            ((pInInfo->NVFormat & ISP_BIT_IN_NV_FORMAT) << ISP_SHT_IN_NV_FORMAT) |
            ((0x0 & ISP_BIT_BYPASS_SCALE_EN) << ISP_SHT_BYPASS_SCALE_EN) |
            ((pInInfo->InputBufferNum & ISP_BIT_RDBUFFER_NUM) << ISP_SHT_RDBUFFER_NUM) |
            ((pInInfo->DisableCaptureCtrl & ISP_BIT_CAPTURE_CRTL_DISABLE) << ISP_SHT_CAPTURE_CRTL_DISABLE) |
            ((0x0 & ISP_BIT_SCENE_CHANGE_EN) << ISP_SHT_SCENE_CHANGE_EN);

    HOST_WriteRegister(ISP_REG_SET502,  (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set ISP input buffer relate parameters.
 */
//=============================================================================
void
ISP_SetInputBuf_Reg(
    const ISP_INPUT_INFO    *pInInfo)
{
    MMP_UINT16  Value = 0;

    HOST_WriteRegister(ISP_REG_INPUT_WIDTH, (MMP_UINT16)(pInInfo->SrcWidth & ISP_BIT_INPUT_WIDTH));
    HOST_WriteRegister(ISP_REG_INPUT_HEIGHT, (MMP_UINT16)(pInInfo->SrcHeight & ISP_BIT_INPUT_HEIGHT));

    HOST_WriteRegister(ISP_REG_INPUT_PITCH_Y, (MMP_UINT16)(pInInfo->PitchY & ISP_BIT_INPUT_PITCH_Y));
    HOST_WriteRegister(ISP_REG_INPUT_PITCH_UV, (MMP_UINT16)(pInInfo->PitchUV & ISP_BIT_INPUT_PITCH_UV));
}

//=============================================================================
/**
 * Set ISP input buffer address relate parameters
 */
//=============================================================================
void
ISP_SetInputAddr_Reg(
    const ISP_INPUT_INFO    *pInInfo)
{
    MMP_UINT32      Value = 0;
    MMP_UINT32      vramBaseAddr = (MMP_UINT32)HOST_GetVramBaseAddress();

    //CurFrame 0
    Value = (MMP_UINT32)pInInfo->AddrY[0] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y0L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[0] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV0L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    if (pInInfo->InputBufferNum != 0)
    {
    //CurFrame 1
    Value = (MMP_UINT32)pInInfo->AddrY[1] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y1L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y1H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[1] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV1L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV1H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    //CurFrame 2
    Value = (MMP_UINT32)pInInfo->AddrY[2] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y2L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y2H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[2] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV2L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV2H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    //CurFrame 3
    Value = (MMP_UINT32)pInInfo->AddrY[3] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y3L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y3H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[3] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV3L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV3H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
    
    //CurFrame 4
    Value = (MMP_UINT32)pInInfo->AddrY[4] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y4L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_Y4H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));

    Value = (MMP_UINT32)pInInfo->AddrUV[4] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV4L, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_UV4H, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
    }
    
    //PreFrame
    Value = (MMP_UINT32)pInInfo->AddrYp - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_YPL, (MMP_UINT16)(Value & ISP_BIT_INPUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_INPUT_ADDR_YPH, (MMP_UINT16)((Value >> 16) & ISP_BIT_INPUT_ADDR_H));
}

//=============================================================================
/**
* Set Deinterlace Parameter.
*/
//=============================================================================
void
ISP_SetDeInterlaceParam_Reg(
    const ISP_DEINTERLACE_CTRL  *pDeInterlace)
{
    MMP_UINT16  Value = 0;

    Value = ((pDeInterlace->EnableOutMotion & ISP_BIT_OUTMOTIONDETECT_EN) << ISP_SHT_OUTMOTIONDETECT_EN) |
            ((pDeInterlace->LowLevelBypassBlend & ISP_BIT_LOWLEVELBYPASSBLEND) << ISP_SHT_LOWLEVELBYPASSBLEND) |
            ((pDeInterlace->Disable30MotionDetect & ISP_BIT_30LOWLEVELEDGE_DISABLE) << ISP_SHT_30LOWLEVELEDGE_DISABLE) |
            ((pDeInterlace->EnLowLevelOutside & ISP_BIT_LOWLEVELOUTSIDE_EN) << ISP_SHT_LOWLEVELOUTSIDE_EN) |
            ((pDeInterlace->LowLevelMode & ISP_BIT_LOWLEVELMODE) << ISP_SHT_LOWLEVELMODE) |
            ((pDeInterlace->EnLowLevelEdge & ISP_BIT_LOWLEVELEDGE_EN) << ISP_SHT_LOWLEVELEDGE_EN) |
            ((pDeInterlace->UVRepeatMode & ISP_BIT_UVREPEAT_MODE) << ISP_SHT_UVREPEAT_MODE) |
            ((pDeInterlace->EnChromaEdgeDetect & ISP_BIT_CHROMA_EDGEDET_EN) << ISP_SHT_CHROMA_EDGEDET_EN) |
            ((pDeInterlace->EnLummaEdgeDetect & ISP_BIT_LUMA_EDGEDET_EN) << ISP_SHT_LUMA_EDGEDET_EN) |
            ((pDeInterlace->Enable & ISP_BIT_DEINTERLACE_EN) << ISP_SHT_DEINTERLACE_EN) |
            ((pDeInterlace->DeinterMode & ISP_BIT_2D_DEINTER_MODE_EN) << ISP_SHT_2D_DEINTER_MODE_EN) |
            ((pDeInterlace->EnSrcBottomFieldFirst & ISP_BIT_SRC_BOTTOM_FIELD_FIRST) << ISP_SHT_SRC_BOTTOM_FIELD_FIRST) |
            ((pDeInterlace->EnDeinterBottomField & ISP_BIT_DEINTER_BOTTOM_EN) << ISP_SHT_DEINTER_BOTTOM_EN) |
            ((pDeInterlace->EnSrcLPF & ISP_BIT_SRC_LPFITR_EN) << ISP_SHT_SRC_LPFITR_EN) |
            ((pDeInterlace->EnUV2DMethod & ISP_BIT_UV2D_METHOD_EN) << ISP_SHT_UV2D_METHOD_EN);
          
    HOST_WriteRegister(ISP_REG_SET504, (MMP_UINT16)Value);

    HOST_WriteRegister(ISP_REG_LOWLEVELEDGE_START_X, (MMP_UINT16)pDeInterlace->LowLevelPosX);
    HOST_WriteRegister(ISP_REG_LOWLEVELEDGE_START_Y, (MMP_UINT16)pDeInterlace->LowLevelPosY);
    HOST_WriteRegister(ISP_REG_LOWLEVELEDGE_WIDTH, (MMP_UINT16)pDeInterlace->LowLevelWidth);
    HOST_WriteRegister(ISP_REG_LOWLEVELEDGE_HEIGHT, (MMP_UINT16)pDeInterlace->LowLevelHeight);

    if(pDeInterlace->DeinterMode == DEINTER3D)
        ISP_Set3DDeInterlaceParm_Reg();
    else if(pDeInterlace->DeinterMode == DEINTER2D)
        ISP_Set2DDeInterlaceParam_Reg();
}


//=============================================================================
/**
* Set 3D-Deinterlace parameters.
*/
//=============================================================================
void
ISP_Set3DDeInterlaceParm_Reg(
    void)
{
    MMP_UINT16              Value = 0;
    ISP_DEINTERLACE_CTRL    *pDeInterlace = &ISPctxt->DeInterlace;

    //Parameter 1
    Value = ((pDeInterlace->MDThreshold_High & ISP_BIT_3D_MDTHRED_HIGH) << ISP_SHT_3D_MDTHRED_HIGH) |
            ((pDeInterlace->MDThreshold_Low & ISP_BIT_3D_MDTHRED_LOW) << ISP_SHT_3D_MDTHRED_LOW);

    HOST_WriteRegister(ISP_REG_3D_DEINTER_PARM_1, (MMP_UINT16)Value);

    //Parameter 2
    Value = ((pDeInterlace->DisableMV_A & ISP_BIT_DISABLE_MOTIONVALUE_A) << ISP_SHT_DISABLE_MOTIONVALUE_A) |
            ((pDeInterlace->DisableMV_B & ISP_BIT_DISABLE_MOTIONVALUE_B) << ISP_SHT_DISABLE_MOTIONVALUE_B) |
            ((pDeInterlace->DisableMV_C & ISP_BIT_DISABLE_MOTIONVALUE_C) << ISP_SHT_DISABLE_MOTIONVALUE_C) |
            ((pDeInterlace->DisableMV_D & ISP_BIT_DISABLE_MOTIONVALUE_D) << ISP_SHT_DISABLE_MOTIONVALUE_D) |
            ((pDeInterlace->DisableMV_E & ISP_BIT_DISABLE_MOTIONVALUE_E) << ISP_SHT_DISABLE_MOTIONVALUE_E) |
            ((pDeInterlace->DisableMV_F & ISP_BIT_DISABLE_MOTIONVALUE_F) << ISP_SHT_DISABLE_MOTIONVALUE_F) |
            ((pDeInterlace->DisableMV_G & ISP_BIT_DISABLE_MOTIONVALUE_G) << ISP_SHT_DISABLE_MOTIONVALUE_G) |
            ((pDeInterlace->EnLPFWeight & ISP_BIT_LPF_WEIGHT_EN) << ISP_SHT_LPF_WEIGHT_EN) |
            ((pDeInterlace->EnLPFBlend & ISP_BIT_LPF_BLEND_EN) << ISP_SHT_LPF_BLEND_EN) |
            ((pDeInterlace->MDThreshold_Step & ISP_BIT_3D_MDTHRED_STEP) << ISP_SHT_3D_MDTHRED_STEP);

    HOST_WriteRegister(ISP_REG_3D_DEINTER_PARM_2, (MMP_UINT16)Value);

    //Parameter 3
    Value = ((pDeInterlace->EnLPFStaticPixel & ISP_BIT_LPF_STATICPIXEL_EN) << ISP_SHT_LPF_STATICPIXEL_EN);

    HOST_WriteRegister(ISP_REG_3D_DEINTER_PARM_3, (MMP_UINT16)Value);

}

//=============================================================================
/**
* Set 2D-Deinterlace parameters.
*/
//=============================================================================
void
ISP_Set2DDeInterlaceParam_Reg(
    void)
{
    MMP_UINT16              Value = 0;
    ISP_DEINTERLACE_CTRL    *pDeInterlace = &ISPctxt->DeInterlace;

    Value = ((pDeInterlace->D2EdgeBlendWeight & ISP_BIT_2D_EDGE_WEIGHT) << ISP_SHT_2D_EDGE_WEIGHT) |
            ((pDeInterlace->D2OrgBlendWeight & ISP_BIT_2D_ORG_WEIGHT) << ISP_SHT_2D_ORG_WEIGHT);

    HOST_WriteRegister(ISP_REG_2D_DEINTER_PARM_1, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set Jpeg encode parameters
 */
//=============================================================================
void
ISP_SetJpegEncode_Reg(
    const ISP_JEPG_ENCODE_CTRL  *pJpegEncode)
{
    MMP_UINT16              Value = 0;
    
    Value = ((pJpegEncode->EnableJPEGEncode & ISP_BIT_JPEGDECODE_EN) << ISP_SHT_JPEGDECODE_EN) |
            ((pJpegEncode->TotalSliceNum & ISP_BIT_TOTALSLICENUM) << ISP_SHT_TOTALSLICENUM);
            
    HOST_WriteRegister(ISP_REG_SET506, (MMP_UINT16)Value);
}

//=============================================================================
/**
 * Set color correction matrix and constant
 */
//=============================================================================
void
ISP_SetCCMatrix_Reg(
    const ISP_COLOR_CORRECTION  *pColorCorrect)
{
    HOST_WriteRegister(ISP_REG_CC_IN_OFFSET_R, (MMP_UINT16)(pColorCorrect->OffsetR & ISP_BIT_IN_OFFSET));
    HOST_WriteRegister(ISP_REG_CC_IN_OFFSET_G, (MMP_UINT16)(pColorCorrect->OffsetG & ISP_BIT_IN_OFFSET));
    HOST_WriteRegister(ISP_REG_CC_IN_OFFSET_B, (MMP_UINT16)(pColorCorrect->OffsetB & ISP_BIT_IN_OFFSET));

    HOST_WriteRegister(ISP_REG_COL_COR_11, (MMP_UINT16)(pColorCorrect->_11 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_12, (MMP_UINT16)(pColorCorrect->_12 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_13, (MMP_UINT16)(pColorCorrect->_13 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_21, (MMP_UINT16)(pColorCorrect->_21 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_22, (MMP_UINT16)(pColorCorrect->_22 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_23, (MMP_UINT16)(pColorCorrect->_23 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_31, (MMP_UINT16)(pColorCorrect->_31 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_32, (MMP_UINT16)(pColorCorrect->_32 & ISP_BIT_COL_COR));
    HOST_WriteRegister(ISP_REG_COL_COR_33, (MMP_UINT16)(pColorCorrect->_33 & ISP_BIT_COL_COR));

    HOST_WriteRegister(ISP_REG_COL_COR_DELTA_R, (MMP_UINT16)(pColorCorrect->DeltaR & ISP_BIT_COL_CORR_DELTA));
    HOST_WriteRegister(ISP_REG_COL_COR_DELTA_G, (MMP_UINT16)(pColorCorrect->DeltaG & ISP_BIT_COL_CORR_DELTA));
    HOST_WriteRegister(ISP_REG_COL_COR_DELTA_B, (MMP_UINT16)(pColorCorrect->DeltaB & ISP_BIT_COL_CORR_DELTA));
}

//=============================================================================
/*
* Set Scale Factor
*/
//=============================================================================
void
ISP_SetScaleParam_Reg(
    const ISP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16  Value = 0;
    MMP_UINT32  HCI;
    MMP_UINT32  VCI;

    HCI = MMP_FLOATToFix(pScaleFun->HCI, 6, 14);
    VCI = MMP_FLOATToFix(pScaleFun->VCI, 6, 14);

    //HCI
    HOST_WriteRegister(ISP_REG_SCALE_HCI_L, (MMP_UINT16)(HCI & ISP_BIT_SCALE_L));
    HOST_WriteRegister(ISP_REG_SCALE_HCI_H, (MMP_UINT16)((HCI >> 16) & ISP_BIT_SCALE_H));

    //VCI
    HOST_WriteRegister(ISP_REG_SCALE_VCI_L, (MMP_UINT16)(VCI & ISP_BIT_SCALE_L));
    HOST_WriteRegister(ISP_REG_SCALE_VCI_H, (MMP_UINT16)((VCI >> 16) & ISP_BIT_SCALE_H));
}


//=============================================================================
/**
* Set Scale Horizontal Weight.
*/
//=============================================================================
void
ISP_SetScaleMatrixH_Reg(
    const ISP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16  Value;

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[0][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[1][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[2][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[3][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatX[4][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX4342, (MMP_UINT16)Value);
}

void
ISP_SetIntScaleMatrixH_Reg(
    MMP_UINT8  WeightMatX[][ISP_SCALE_TAP])
{
    MMP_UINT16  Value;

    Value = ((WeightMatX[0][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX0100, (MMP_UINT16)Value);

    Value = ((WeightMatX[0][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[0][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX0302, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX1110, (MMP_UINT16)Value);

    Value = ((WeightMatX[1][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[1][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX1312, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX2120, (MMP_UINT16)Value);

    Value = ((WeightMatX[2][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[2][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX2322, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX3130, (MMP_UINT16)Value);

    Value = ((WeightMatX[3][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[3][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX3332, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX4140, (MMP_UINT16)Value);

    Value = ((WeightMatX[4][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatX[4][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWX4342, (MMP_UINT16)Value);
}

//=============================================================================
/**
* Set Scale Vertical Weight.
*/
//=============================================================================
void
ISP_SetScaleMatrixV_Reg(
    const ISP_SCALE_CTRL    *pScaleFun)
{
    MMP_UINT16 Value;

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY0100, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[0][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY0302, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY1110, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[1][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY1312, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY2120, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[2][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY2322, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY3130, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[3][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY3332, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][0], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][1], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY4140, (MMP_UINT16)Value);

    Value = (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][2], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            (((MMP_UINT16)MMP_FLOATToFix(pScaleFun->WeightMatY[4][3], 1, 6) & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY4342, (MMP_UINT16)Value);
}

void
ISP_SetIntScaleMatrixV_Reg(
    MMP_UINT8  WeightMatY[][ISP_SCALE_TAP])
{
    MMP_UINT16 Value;

    Value = ((WeightMatY[0][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY0100, (MMP_UINT16)Value);

    Value = ((WeightMatY[0][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[0][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY0302, (MMP_UINT16)Value);

    Value = ((WeightMatY[1][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY1110, (MMP_UINT16)Value);

    Value = ((WeightMatY[1][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[1][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY1312, (MMP_UINT16)Value);

    Value = ((WeightMatY[2][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY2120, (MMP_UINT16)Value);

    Value = ((WeightMatY[2][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[2][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY2322, (MMP_UINT16)Value);

    Value = ((WeightMatY[3][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY3130, (MMP_UINT16)Value);

    Value = ((WeightMatY[3][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[3][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY3332, (MMP_UINT16)Value);

    Value = ((WeightMatY[4][0] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][1] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY4140, (MMP_UINT16)Value);

    Value = ((WeightMatY[4][2] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_L) |
            ((WeightMatY[4][3] & ISP_BIT_SCALEWEIGHT) << ISP_SHT_SCALEWEIGHT_H);
    HOST_WriteRegister(ISP_REG_SCALEWY4342, (MMP_UINT16)Value);
}


//=============================================================================
/**
* Frmfun RGB to YUV transfer matrix.
*/
//=============================================================================
void
ISP_SetFrmMatrix_Reg(
    const ISP_RGB_TO_YUV    *pMatrix)
{
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_11, (MMP_UINT16)(pMatrix->_11 & ISP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_12, (MMP_UINT16)(pMatrix->_12 & ISP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_13, (MMP_UINT16)(pMatrix->_13 & ISP_BIT_FRM_RGB2YUV));

    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_21, (MMP_UINT16)(pMatrix->_21 & ISP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_22, (MMP_UINT16)(pMatrix->_22 & ISP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_23, (MMP_UINT16)(pMatrix->_23 & ISP_BIT_FRM_RGB2YUV));

    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_31, (MMP_UINT16)(pMatrix->_31 & ISP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_32, (MMP_UINT16)(pMatrix->_32 & ISP_BIT_FRM_RGB2YUV));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_33, (MMP_UINT16)(pMatrix->_33 & ISP_BIT_FRM_RGB2YUV));

    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_CONST_Y, (MMP_UINT16)(pMatrix->ConstY & ISP_BIT_FRM_RGB2YUV_CONST));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_CONST_U, (MMP_UINT16)(pMatrix->ConstU & ISP_BIT_FRM_RGB2YUV_CONST));
    HOST_WriteRegister(ISP_REG_FRM_RGB2YUV_CONST_V, (MMP_UINT16)(pMatrix->ConstV & ISP_BIT_FRM_RGB2YUV_CONST));
}


//=============================================================================
/**
* Set Frame Function 0
*/
//=============================================================================
void
ISP_SetFrameFun0_Reg(
    const ISP_FRMFUN_CTRL   *pFrameFun)
{
    MMP_UINT32      Value = 0;
    MMP_UINT32      vramBaseAddr = (MMP_UINT32)HOST_GetVramBaseAddress();

    //Starting address
    Value = (MMP_UINT32)pFrameFun->Addr - vramBaseAddr; // byte align
    HOST_WriteRegister(ISP_REG_FRMFUN_0_ADDR_L, (MMP_UINT16)(Value & ISP_BIT_FRMFUN_ADDR_L));
    HOST_WriteRegister(ISP_REG_FRMFUN_0_ADDR_H, (MMP_UINT16)((Value >> 16) & ISP_BIT_FRMFUN_ADDR_H));

    //width, height, pitch
    HOST_WriteRegister(ISP_REG_FRMFUN_0_WIDTH,  (MMP_UINT16)(pFrameFun->Width & ISP_BIT_FRMFUN_WIDTH));
    HOST_WriteRegister(ISP_REG_FRMFUN_0_HEIGHT, (MMP_UINT16)(pFrameFun->Height & ISP_BIT_FRMFUN_HEIGHT));
    HOST_WriteRegister(ISP_REG_FRMFUN_0_PITCH,  (MMP_UINT16)(pFrameFun->Pitch & ISP_BIT_FRMFUN_PITCH));

    //start X/Y
    HOST_WriteRegister(ISP_REG_FRMFUN_0_START_X, (MMP_UINT16)(pFrameFun->StartX & ISP_BIT_FRMFUN_START_X));
    HOST_WriteRegister(ISP_REG_FRMFUN_0_START_Y, (MMP_UINT16)(pFrameFun->StartY & ISP_BIT_FRMFUN_START_Y));

    //color key
    Value = ((pFrameFun->ColorKeyR & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_R) |
            ((pFrameFun->ColorKeyG & ISP_BIT_FRMFUN_KEY) << ISP_SHT_FRMFUN_KEY_G);

    HOST_WriteRegister(ISP_REG_FRMFUN_0_KEY_RG, (MMP_UINT16)Value);
    HOST_WriteRegister(ISP_REG_FRMFUN_0_KEY_B,  (MMP_UINT16)(pFrameFun->ColorKeyB & ISP_BIT_FRMFUN_KEY));

    //Constant Alpha Value
    HOST_WriteRegister(ISP_REG_CONST_ALPHA_0, (MMP_UINT16)(pFrameFun->ConstantAlpha & ISP_BIT_CONST_ALPHA));


    //format ARGB4444 or Constant Alpha with RGB565
    if(pFrameFun->Format == ARGB4444)
    {
        Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                ((0x1 & ISP_BIT_FRMFUN_MODE) << ISP_SHT_FRMFUN_MODE);
    }
    else if(pFrameFun->Format == CARGB565)
    {
        if(pFrameFun->ConstantAlpha != 0)
        {
            Value = ((0x1 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE) << ISP_SHT_FRMFUN_MODE);
        }
        else
        {
            Value = ((0x0 & ISP_BIT_FRMFUN_ALPHA_BLEND_EN) << ISP_SHT_FRMFUN_ALPHA_BLEND_EN) |
                    ((0x0 & ISP_BIT_FRMFUN_MODE) << ISP_SHT_FRMFUN_MODE);

        }
    }

    //FrameFun Enable, BlendConst
    Value = Value |
            ((pFrameFun->Enable & ISP_BIT_FRMFUN_EN) << ISP_SHT_FRMFUN_EN) |
            ((pFrameFun->EnableFieldMode & ISP_BIT_FRMFUN_FIELDMODE_EN) << ISP_SHT_FRMFUN_FIELDMODE_EN) |
            ((pFrameFun->EnableGobang & ISP_BIT_FRMFUN_GOBANG_EN) << ISP_SHT_FRMFUN_GOBANG_EN) |
            ((pFrameFun->EnableRGB2YUV & ISP_BIT_FRMFUN_RGB2YUV_EN) << ISP_SHT_FRMFUN_RGB2YUV_EN);

    //Enable frame function, set format
    HOST_WriteRegister(ISP_REG_SET_FRMFUN_0, (MMP_UINT16)Value);
}

//=============================================================================
/**
* RGB to YUV transfer matrix.
*/
//=============================================================================
void
ISP_SetRGBtoYUVMatrix_Reg(
    const ISP_RGB_TO_YUV    *pRGBtoYUV)
{
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_11, (MMP_UINT16)(pRGBtoYUV->_11 & ISP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_12, (MMP_UINT16)(pRGBtoYUV->_12 & ISP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_13, (MMP_UINT16)(pRGBtoYUV->_13 & ISP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_21, (MMP_UINT16)(pRGBtoYUV->_21 & ISP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_22, (MMP_UINT16)(pRGBtoYUV->_22 & ISP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_23, (MMP_UINT16)(pRGBtoYUV->_23 & ISP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_31, (MMP_UINT16)(pRGBtoYUV->_31 & ISP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_32, (MMP_UINT16)(pRGBtoYUV->_32 & ISP_BIT_RGB_TO_YUV));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_33, (MMP_UINT16)(pRGBtoYUV->_33 & ISP_BIT_RGB_TO_YUV));

    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_CONST_Y, (MMP_UINT16)(pRGBtoYUV->ConstY & ISP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_CONST_U, (MMP_UINT16)(pRGBtoYUV->ConstU & ISP_BIT_RGB_TO_YUV_CONST));
    HOST_WriteRegister(ISP_REG_RGB_TO_YUV_CONST_V, (MMP_UINT16)(pRGBtoYUV->ConstV & ISP_BIT_RGB_TO_YUV_CONST));
}

//=============================================================================
/**
* Set Output Format
*/
//=============================================================================
void
ISP_SetOutParameter_Reg(
    const ISP_OUTPUT_INFO   *pOutInfo)
{

    MMP_UINT16  Value = 0;

    //Set ISP_REG_SET508
    
    Value = ((pOutInfo->EngineDelay & ISP_BIT_ENGINEDELAY) << ISP_SHT_ENGINEDELAY) |
            ((pOutInfo->OutputBufferNum & ISP_BIT_WRBUFFER_NUM) << ISP_SHT_WRBUFFER_NUM) |
            ((pOutInfo->EnableSWCtrlRdAddr & ISP_BIT_SWCTRL_RDADDR_EN) << ISP_SHT_SWCTRL_RDADDR_EN) |
            ((pOutInfo->EnableSWFlipMode & ISP_BIT_SWWRFLIP_EN) << ISP_SHT_SWWRFLIP_EN) |
            ((pOutInfo->SWWrFlipNum & ISP_BIT_SWWRFLIP_NUM) << ISP_SHT_SWWRFLIP_NUM);
 
    HOST_WriteRegister(ISP_REG_SET508,  (MMP_UINT16)Value); 
    Value = 0;


    //Set ISP_REG_SET50A

    Value = ((pOutInfo->EnableUVBiDownsample & ISP_BIT_OUT_BILINEAR_DOWNSAMPLE_EN) << ISP_SHT_OUT_BILINEAR_DOWNSAMPLE_EN) |
            ((pOutInfo->PlaneFormat & ISP_SHT_OUT_YUVPLANE_FORMAT) << ISP_BIT_OUT_YUVPLANE_FORMAT) |
            ((pOutInfo->NVFormat & ISP_BIT_OUT_NV_FORMAT) << ISP_SHT_OUT_NV_FORMAT) |
            ((pOutInfo->OutFormat & ISP_BIT_OUT_FORMAT) << ISP_SHT_OUT_FORMAT) |             
            ((pOutInfo->DisableOutMatrix & ISP_BIT_OUTMATRIX_DISABLE) << ISP_SHT_OUTMATRIX_DISABLE) |
            ((pOutInfo->EnableFieldMode & ISP_BIT_OUTPUT_FIELD_MODE) << ISP_SHT_OUTPUT_FIELD_MODE) |
            ((pOutInfo->EnableRemapUVAddr & ISP_BIT_REMAP_CHROMAADDR_EN) << ISP_SHT_REMAP_CHROMAADDR_EN) |
            ((pOutInfo->EnableRemapYAddr & ISP_BIT_REMAP_LUMAADDR_EN) << ISP_SHT_REMAP_LUMAADDR_EN);

    HOST_WriteRegister(ISP_REG_SET50A,  (MMP_UINT16)Value);
    Value = 0;
}

//=============================================================================
/**
 * Set Output Information
 */
//=============================================================================
void
ISP_SetOutBufInfo_Reg(
    const ISP_OUTPUT_INFO   *pOutInfo)
{
    //width, height, pitch
    HOST_WriteRegister(ISP_REG_OUT_WIDTH,  (MMP_UINT16)(pOutInfo->Width & ISP_BIT_OUT_WIDTH ));
    HOST_WriteRegister(ISP_REG_OUT_HEIGHT, (MMP_UINT16)(pOutInfo->Height & ISP_BIT_OUT_HEIGHT));
    HOST_WriteRegister(ISP_REG_OUT_Y_PITCH,  (MMP_UINT16)(pOutInfo->PitchY & ISP_BIT_OUT_PITCH));
    HOST_WriteRegister(ISP_REG_OUT_UV_PITCH,  (MMP_UINT16)(pOutInfo->PitchUV & ISP_BIT_OUT_PITCH));
}

//=============================================================================
/**
* Set Output Address
*/
//=============================================================================
void
ISP_SetOutAddress_Reg(
    const ISP_OUTPUT_INFO   *pOutInfo)
{
    MMP_UINT32  Value = 0;
    MMP_UINT32  vramBaseAddr = (MMP_UINT32)HOST_GetVramBaseAddress();

    // byte align
    //buffer address 0
    Value = (MMP_UINT32)pOutInfo->AddrY[0] - vramBaseAddr; 
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y0L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[0] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U0L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[0] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V0L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V0H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));
    
    
    if (pOutInfo->OutputBufferNum != 0)    
    {
    //buffer address 1
    Value = (MMP_UINT32)pOutInfo->AddrY[1] - vramBaseAddr; 
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y1L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y1H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[1] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U1L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U1H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[1] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V1L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V1H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));
    
    //buffer address 2
    Value = (MMP_UINT32)pOutInfo->AddrY[2] - vramBaseAddr; 
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y2L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y2H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[2] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U2L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U2H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[2] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V2L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V2H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));
    
    //buffer address 3
    Value = (MMP_UINT32)pOutInfo->AddrY[3] - vramBaseAddr; 
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y3L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y3H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[3] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U3L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U3H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[3] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V3L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V3H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));
    
    //buffer address 4
    Value = (MMP_UINT32)pOutInfo->AddrY[4] - vramBaseAddr; 
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y4L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_Y4H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrU[4] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U4L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_U4H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));

    Value = (MMP_UINT32)pOutInfo->AddrV[4] - vramBaseAddr;
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V4L, (MMP_UINT16)(Value & ISP_BIT_OUT_ADDR_L));
    HOST_WriteRegister(ISP_REG_OUT_ADDR_V4H, (MMP_UINT16)((Value >> 16) & ISP_BIT_OUT_ADDR_H));
    }
}

//=============================================================================
/**
* Set Remap Y Address
*/
//=============================================================================
void
ISP_SetRemapYAddress_Reg(
    const ISP_REMAP_ADDR   *pRemapAddr)
{
    MMP_UINT32  Value = 0;
    
    Value = ((pRemapAddr->Addr_04 & 0x3F) << 8) | (pRemapAddr->Addr_03 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_0403, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y)); 
    
    Value = ((pRemapAddr->Addr_06 & 0x3F) << 8) | (pRemapAddr->Addr_05 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_0605, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y)); 
        
    Value = ((pRemapAddr->Addr_08 & 0x3F) << 8) | (pRemapAddr->Addr_07 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_0807, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_10 & 0x3F) << 8) | (pRemapAddr->Addr_09 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_1009, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_12 & 0x3F) << 8) | (pRemapAddr->Addr_11 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_1211, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_14 & 0x3F) << 8) | (pRemapAddr->Addr_13 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_1413, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_16 & 0x3F) << 8) | (pRemapAddr->Addr_15 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_1615, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_18 & 0x3F) << 8) | (pRemapAddr->Addr_17 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_1817, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_20 & 0x3F) << 8) | (pRemapAddr->Addr_19 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_2019, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_22 & 0x3F) << 8) | (pRemapAddr->Addr_21 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_2221, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_24 & 0x3F) << 8) | (pRemapAddr->Addr_23 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_2423, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_26 & 0x3F) << 8) | (pRemapAddr->Addr_25 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_2625, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_28 & 0x3F) << 8) | (pRemapAddr->Addr_27 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_2827, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_30 & 0x3F) << 8) | (pRemapAddr->Addr_29 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_3029, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = (pRemapAddr->Addr_31 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_Y_XX31, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
}

//=============================================================================
/**
* Set Remap UV Address
*/
//=============================================================================
void
ISP_SetRemapUVAddress_Reg(
    const ISP_REMAP_ADDR   *pRemapAddr)
{
    MMP_UINT32  Value = 0;
    
    Value = ((pRemapAddr->Addr_04 & 0x3F) << 8) | (pRemapAddr->Addr_03 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_0403, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));    
    
    Value = ((pRemapAddr->Addr_06 & 0x3F) << 8) | (pRemapAddr->Addr_05 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_0605, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));    
        
    Value = ((pRemapAddr->Addr_08 & 0x3F) << 8) | (pRemapAddr->Addr_07 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_0807, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_10 & 0x3F) << 8) | (pRemapAddr->Addr_09 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_1009, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_12 & 0x3F) << 8) | (pRemapAddr->Addr_11 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_1211, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_14 & 0x3F) << 8) | (pRemapAddr->Addr_13 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_1413, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_16 & 0x3F) << 8) | (pRemapAddr->Addr_15 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_1615, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_18 & 0x3F) << 8) | (pRemapAddr->Addr_17 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_1817, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_20 & 0x3F) << 8) | (pRemapAddr->Addr_19 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_2019, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_22 & 0x3F) << 8) | (pRemapAddr->Addr_21 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_2221, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_24 & 0x3F) << 8) | (pRemapAddr->Addr_23 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_2423, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_26 & 0x3F) << 8) | (pRemapAddr->Addr_25 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_2625, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_28 & 0x3F) << 8) | (pRemapAddr->Addr_27 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_2827, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = ((pRemapAddr->Addr_30 & 0x3F) << 8) | (pRemapAddr->Addr_29 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_3029, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
    Value = (pRemapAddr->Addr_31 & 0x3F);
    HOST_WriteRegister(ISP_REG_MAPADR_UV_XX31, (MMP_UINT16)(Value & ISP_BIT_MAPADDR_Y));
    
}
    
//=============================================================================
/**
 * Wait ISP engine idle!  //for JPG module use
 */
//=============================================================================
MMP_RESULT
ISP_WaitEngineIdle(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    //
    //  Wait ISP engine idle!   0x6FC D[0]  0: idle, 1: busy
    //
    HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16*)&status);
    while( status & 0x0001 )
    {
        PalSleep(1);
        if( ++timeOut > 2000 )
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16*)&status);
    }

end:
    if( result )
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Is ISP engine idle! 
 */
//=============================================================================
MMP_BOOL
ISP_IsEngineIdle(
    void)
{
    MMP_UINT16  status = 0, status2 = 0;

    HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16*)&status);
    HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS_2, (MMP_UINT16*)&status2);
    
    if (((status & 0xFF0F) == 0x7504) && ((status2 & 0xFFFF) == 0x0000))
        return MMP_TRUE;
    else
        return MMP_FALSE;       
}

//=============================================================================
/**
 * Wait ISP interrupt idle!
 */
//=============================================================================
MMP_RESULT
ISP_WaitInterruptIdle(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    //
    //  Wait ISP interrupt idle!   0x6FE D[8]  0: idle, 1: busy
    //
    HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS_2, (MMP_UINT16*)&status);
    while( status & 0x0100 )
    {
        PalSleep(1);
        if( ++timeOut > 2000 )
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_INTERRUPT_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS_2, (MMP_UINT16*)&status);
    }

end:
    if( result )
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
* Set Interrupt Information
*/
//=============================================================================
void
ISP_SetInterruptParameter_Reg(
    const ISP_CONTEXT *pISPctxt)
{
    MMP_UINT16  Value = 0;
    
    //Set ISP_REG_SET50E
    Value = ((pISPctxt->EnableInterrupt & ISP_BIT_ISP_INTERRUPT_EN) << ISP_SHT_ISP_INTERRUPT_EN) |
            ((pISPctxt->InterruptMode & ISP_BIT_ISP_INTERRUPT_MODE) << ISP_SHT_ISP_INTERRUPT_MODE);
            
    HOST_WriteRegister(ISP_REG_SET50E, (MMP_UINT16)Value);
    //HOST_WriteRegisterMask(ISP_REG_SET50E, (pISPctxt->InterruptMode & ISP_BIT_ISP_INTERRUPT_MODE) << ISP_SHT_ISP_INTERRUPT_MODE, (ISP_BIT_ISP_INTERRUPT_MODE << ISP_SHT_ISP_INTERRUPT_MODE));
}

//=============================================================================
/**
 * Return ISP Write Buffer Index.
 */
//=============================================================================
MMP_UINT16
ISP_RetrunWrBufIndex_Reg(
    void)
{   
    MMP_UINT16  Value = 0;
                
    HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS, &Value);
    
    Value = (Value >> 4) & 0x7;
    
    return (MMP_UINT16)Value;
}

//=============================================================================
/**
 * Wait ISP change idle!
 */
//=============================================================================
MMP_RESULT
ISP_WaitISPChangeIdle(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;
    MMP_UINT16  status = 0;
    MMP_UINT16  timeOut = 0;

    //
    //  Wait ISP change idle!   0x6FC D[3]  0: idle, 1: busy
    //
    HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16*)&status);

    while( (status & 0x0008) )
    {
        PalSleep(1);

        if( ++timeOut > 2000 )
        {
            //ISP_LogReg();
            isp_msg_ex(ISP_MSG_TYPE_ERR, "ERROR_ISP_CHANGE_NOT_IDLE \n");
            result = ISP_ERR_NOT_IDLE;
            goto end;
        }
        HOST_ReadRegister(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16*)&status);
    }

end:
    if( result )
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0%x !", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP engine clock related.
 */
//=============================================================================
void
ISP_PowerUp(
    void)
{
   HOST_ISP_EnableClock();
   HOST_ISP_Reset();
}


void
ISP_PowerDown(
    void)
{
    HOST_ISP_Reset();
    HOST_ISP_DisableClock();
}
