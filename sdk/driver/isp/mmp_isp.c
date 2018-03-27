#include "host/host.h"
#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"

#include "isp/isp_config.h"
#include "isp/isp_types.h"
#include "isp/isp_reg.h"
#include "isp/isp_hw.h"
#include "isp/isp.h"
#include "mmp_isp.h"
#include "intr/intr.h"

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
ISP_CONTEXT* ISPctxt = MMP_NULL;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * ISP context initialization.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
MMP_RESULT
mmpIspInitialize(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        ISPctxt = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(ISP_CONTEXT));
        if (!ISPctxt)
        {
            result = ISP_ERR_CONTEXT_ALLOC_FAIL;
            goto end;
        }
    }

    ISP_PowerUp();

    PalMemset((void*)ISPctxt, 0, sizeof(ISP_CONTEXT));

    ISP_ContextInitialize(MMP_TRUE);

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP terminate.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API when leaving ISP module.
 */
//=============================================================================
MMP_RESULT
mmpIspTerminate(
    void)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        return (MMP_RESULT)result;
    }

    //
    // Disable ISP engine
    //
    result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    if (ISPctxt->EnableInterrupt == MMP_TRUE)
    {
        result = ISP_WaitInterruptIdle();
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }
        
        mmpIspDisableInterrupt();
    }

    ISP_PowerDown();

    PalMemset((void*)ISPctxt, 0, sizeof(ISP_CONTEXT));

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP context reset.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
MMP_RESULT
mmpIspContextReset(
    void)
{   
    //Update Flag
    ISPctxt->UpdateFlags = 0xFFFFFFFF;
   
    //Input Information
    PalMemset((void*)(&(ISPctxt->InInfo)), 0, sizeof(ISP_INPUT_INFO));
           
    //Deinterlace
    ISPctxt->DeInterlace.Enable = MMP_FALSE;

    //Jpeg encode
    PalMemset((void*)(&(ISPctxt->JpegEncode)), 0, sizeof(ISP_JEPG_ENCODE_CTRL));

    //Output Information
    PalMemset((void*)(&(ISPctxt->OutInfo)), 0, sizeof(ISP_OUTPUT_INFO));

    //Interrupt
    ISPctxt->EnableInterrupt = MMP_FALSE;    

    //Context Init
    ISP_ContextInitialize(MMP_FALSE);
}

//=============================================================================
/**
 * Enable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspEnable(
    MMP_ISP_CAPS    cap)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch (cap)
    {
    case MMP_ISP_DEINTERLACE:
        ISPctxt->DeInterlace.Enable = MMP_TRUE;
        ISPctxt->DeInterlace.UVRepeatMode = MMP_TRUE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_LOWLEVELEDGE:
        ISPctxt->DeInterlace.EnLowLevelEdge = MMP_TRUE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_FRAME_FUNCTION_0:
        ISPctxt->FrameFun0.Enable = MMP_TRUE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_FrameFun0;
        break;

    case MMP_ISP_REMAP_ADDRESS:
        ISPctxt->OutInfo.EnableRemapYAddr = MMP_TRUE;
        ISPctxt->OutInfo.EnableRemapUVAddr = MMP_TRUE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;
        break;

    case MMP_ISP_INTERRUPT:
        ISPctxt->EnableInterrupt = MMP_TRUE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_Interrupt;
        break;

    case MMP_ISP_DEINTER_FIELD_TOP:
        if(ISPctxt->DeInterlace.EnSrcBottomFieldFirst == 0)
            ISPctxt->DeInterlace.EnDeinterBottomField = 1;
        else
            ISPctxt->DeInterlace.EnDeinterBottomField = 0;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_DEINTER_FIELD_BOTTOM:
        if(ISPctxt->DeInterlace.EnSrcBottomFieldFirst == 0)
            ISPctxt->DeInterlace.EnDeinterBottomField = 0;
        else
            ISPctxt->DeInterlace.EnDeinterBottomField = 1;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;
        
    default:
        result = ISP_ERR_NO_MATCH_ENABLE_TYPE;
        break;
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Disable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspDisable(
    MMP_ISP_CAPS cap)
{
    ISP_RESULT   result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch (cap)
    {
    case MMP_ISP_DEINTERLACE:
        ISPctxt->DeInterlace.Enable = MMP_FALSE;
        ISPctxt->DeInterlace.UVRepeatMode = MMP_FALSE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_LOWLEVELEDGE:
        ISPctxt->DeInterlace.EnLowLevelEdge = MMP_FALSE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_FRAME_FUNCTION_0:
        ISPctxt->FrameFun0.Enable = MMP_FALSE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_FrameFun0;
        break;

    case MMP_ISP_REMAP_ADDRESS:
        ISPctxt->OutInfo.EnableRemapYAddr = MMP_FALSE;
        ISPctxt->OutInfo.EnableRemapUVAddr = MMP_FALSE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;
        break;

    case MMP_ISP_INTERRUPT:
        ISPctxt->EnableInterrupt = MMP_FALSE;
        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_Interrupt;

    default:
        result = ISP_ERR_NO_MATCH_ENABLE_TYPE;
        break;
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Query ISP capability.
 *
 * @param cap       Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_TRUE if function enabled, MMP_FALSE if function disable.
 */
//=============================================================================
MMP_BOOL
mmpIspQuery(
    MMP_ISP_CAPS    cap)
{
    MMP_BOOL   result = MMP_FALSE;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch(cap)
    {
    case MMP_ISP_DEINTERLACE:
        result = ISPctxt->DeInterlace.Enable;
        break;

    case MMP_ISP_LOWLEVELEDGE:
        result = ISPctxt->DeInterlace.EnLowLevelEdge;
        break;

    case MMP_ISP_FRAME_FUNCTION_0:
        result = ISPctxt->FrameFun0.Enable;
        break;

    case MMP_ISP_REMAP_ADDRESS:
        result = ISPctxt->OutInfo.EnableRemapYAddr;
        break;

    case MMP_ISP_INTERRUPT:
        result = ISPctxt->EnableInterrupt;
        break;

    default:
        result = MMP_FALSE;
        break;
    }

end:
    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Set Remap Address Parameter
 * @param MMP_ISP_REMAP_ADDR
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspSetRemapAddr(
    MMP_ISP_REMAP_ADDR    *data)
{
    ISP_RESULT         result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    ISPctxt->RemapYAddr.Addr_03 = data->YAddr_03;
    ISPctxt->RemapYAddr.Addr_04 = data->YAddr_04;
    ISPctxt->RemapYAddr.Addr_05 = data->YAddr_05;
    ISPctxt->RemapYAddr.Addr_06 = data->YAddr_06;
    ISPctxt->RemapYAddr.Addr_07 = data->YAddr_07;
    ISPctxt->RemapYAddr.Addr_08 = data->YAddr_08;
    ISPctxt->RemapYAddr.Addr_09 = data->YAddr_09;
    ISPctxt->RemapYAddr.Addr_10 = data->YAddr_10;
    ISPctxt->RemapYAddr.Addr_11 = data->YAddr_11;
    ISPctxt->RemapYAddr.Addr_12 = data->YAddr_12;
    ISPctxt->RemapYAddr.Addr_13 = data->YAddr_13;
    ISPctxt->RemapYAddr.Addr_14 = data->YAddr_14;
    ISPctxt->RemapYAddr.Addr_15 = data->YAddr_15;
    ISPctxt->RemapYAddr.Addr_16 = data->YAddr_16;
    ISPctxt->RemapYAddr.Addr_17 = data->YAddr_17;
    ISPctxt->RemapYAddr.Addr_18 = data->YAddr_18;
    ISPctxt->RemapYAddr.Addr_19 = data->YAddr_19;
    ISPctxt->RemapYAddr.Addr_20 = data->YAddr_20;
    ISPctxt->RemapYAddr.Addr_21 = data->YAddr_21;
    ISPctxt->RemapYAddr.Addr_22 = data->YAddr_22;
    ISPctxt->RemapYAddr.Addr_23 = data->YAddr_23;
    ISPctxt->RemapYAddr.Addr_24 = data->YAddr_24;
    ISPctxt->RemapYAddr.Addr_25 = data->YAddr_25;
    ISPctxt->RemapYAddr.Addr_26 = data->YAddr_26;
    ISPctxt->RemapYAddr.Addr_27 = data->YAddr_27;
    ISPctxt->RemapYAddr.Addr_28 = data->YAddr_28;
    ISPctxt->RemapYAddr.Addr_29 = data->YAddr_29;
    ISPctxt->RemapYAddr.Addr_30 = data->YAddr_30;
    ISPctxt->RemapYAddr.Addr_31 = data->YAddr_31;

    ISPctxt->RemapUVAddr.Addr_03 = data->UVAddr_03;
    ISPctxt->RemapUVAddr.Addr_04 = data->UVAddr_04;
    ISPctxt->RemapUVAddr.Addr_05 = data->UVAddr_05;
    ISPctxt->RemapUVAddr.Addr_06 = data->UVAddr_06;
    ISPctxt->RemapUVAddr.Addr_07 = data->UVAddr_07;
    ISPctxt->RemapUVAddr.Addr_08 = data->UVAddr_08;
    ISPctxt->RemapUVAddr.Addr_09 = data->UVAddr_09;
    ISPctxt->RemapUVAddr.Addr_10 = data->UVAddr_10;
    ISPctxt->RemapUVAddr.Addr_11 = data->UVAddr_11;
    ISPctxt->RemapUVAddr.Addr_12 = data->UVAddr_12;
    ISPctxt->RemapUVAddr.Addr_13 = data->UVAddr_13;
    ISPctxt->RemapUVAddr.Addr_14 = data->UVAddr_14;
    ISPctxt->RemapUVAddr.Addr_15 = data->UVAddr_15;
    ISPctxt->RemapUVAddr.Addr_16 = data->UVAddr_16;
    ISPctxt->RemapUVAddr.Addr_17 = data->UVAddr_17;
    ISPctxt->RemapUVAddr.Addr_18 = data->UVAddr_18;
    ISPctxt->RemapUVAddr.Addr_19 = data->UVAddr_19;
    ISPctxt->RemapUVAddr.Addr_20 = data->UVAddr_20;
    ISPctxt->RemapUVAddr.Addr_21 = data->UVAddr_21;
    ISPctxt->RemapUVAddr.Addr_22 = data->UVAddr_22;
    ISPctxt->RemapUVAddr.Addr_23 = data->UVAddr_23;
    ISPctxt->RemapUVAddr.Addr_24 = data->UVAddr_24;
    ISPctxt->RemapUVAddr.Addr_25 = data->UVAddr_25;
    ISPctxt->RemapUVAddr.Addr_26 = data->UVAddr_26;
    ISPctxt->RemapUVAddr.Addr_27 = data->UVAddr_27;
    ISPctxt->RemapUVAddr.Addr_28 = data->UVAddr_28;
    ISPctxt->RemapUVAddr.Addr_29 = data->UVAddr_29;
    ISPctxt->RemapUVAddr.Addr_30 = data->UVAddr_30;
    ISPctxt->RemapUVAddr.Addr_31 = data->UVAddr_31;

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_RemapAddr;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP Motion Detection Process
 *
 * @param data  MMP_ISP_SINGLE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspMotionProcess(
    const MMP_ISP_SINGLE_SHARE     *data)
{
    ISP_RESULT  result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //
    // Set Input Address, Width, Height and Pitch
    //
    ISPctxt->InInfo.AddrY[0]    = (MMP_UINT8*)data->In_AddrY;
    ISPctxt->InInfo.AddrUV[0]   = (MMP_UINT8*)data->In_AddrUV;
    ISPctxt->InInfo.AddrYp      = (MMP_UINT8*)data->In_AddrYp;

    // width must be 2 alignment
    ISPctxt->InInfo.SrcWidth    = (data->In_Width >> 2) << 2;
    ISPctxt->InInfo.SrcHeight   = data->In_Height;

    ISPctxt->InInfo.PitchY      = data->In_PitchY;
    ISPctxt->InInfo.PitchUV     = data->In_PitchUV;

    // Set Input Format
    result = ISP_SetInputFormat(data->In_Format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    // Set Output Address, Width, Height and Pitch
    //
    ISPctxt->OutInfo.AddrY[0]   = (MMP_UINT8*)data->Out_AddrY;
    ISPctxt->OutInfo.AddrU[0]   = (MMP_UINT8*)data->Out_AddrU;
    ISPctxt->OutInfo.AddrV[0]   = (MMP_UINT8*)data->Out_AddrV;

    // width must be 2 alignment
    ISPctxt->OutInfo.Width      = (data->Out_Width >> 2) << 2;
    ISPctxt->OutInfo.Height     = data->Out_Height;

    ISPctxt->OutInfo.PitchY     = data->Out_PitchY;
    ISPctxt->OutInfo.PitchUV    = data->Out_PitchUV;

    // Set Output Format
    result = ISP_SetOutputFormat(data->Out_Format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Set single porcess
    ISPctxt->InInfo.EnableReadMemoryMode = MMP_TRUE;
    ISPctxt->InInfo.DisableCaptureCtrl = MMP_TRUE;
    ISPctxt->InInfo.InputBufferNum = 0;

    ISPctxt->OutInfo.OutputBufferNum = 0;
    ISPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_TRUE;
    ISPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;
    ISPctxt->OutInfo.EnableFieldMode = MMP_FALSE;
    ISPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;

    ISPctxt->JpegEncode.EnableJPEGEncode = MMP_FALSE;

    // Enable Motion Detection Setting
    ISP_EnableMotionDetectionParameter();

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputAddr;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutBufInfo;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_JpegEncode;

    // check isp engine idle
    result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Update parameter
    result = ISP_Update();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // ISP_LogReg();
    ISP_DriverFire_Reg();

end:
    ISP_DisableMotionDetectionParameter();

    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP Single Process
 *
 * @param data  MMP_ISP_SINGLE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspSingleProcess(
    const MMP_ISP_SINGLE_SHARE     *data)
{
    ISP_RESULT         result = ISP_SUCCESS;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //
    // Set Input Address, Width, Height and Pitch
    //
    ISPctxt->InInfo.AddrY[0]    = (MMP_UINT8*)data->In_AddrY;
    ISPctxt->InInfo.AddrUV[0]   = (MMP_UINT8*)data->In_AddrUV;
    ISPctxt->InInfo.AddrYp      = (MMP_UINT8*)data->In_AddrYp;

    // width must be 2 alignment
    ISPctxt->InInfo.SrcWidth  = (data->In_Width >> 2) << 2;
    ISPctxt->InInfo.SrcHeight = (data->In_Height >> 2) << 2;

    ISPctxt->InInfo.PitchY  = data->In_PitchY;
    ISPctxt->InInfo.PitchUV = data->In_PitchUV;

    // Set Input Format
    result = ISP_SetInputFormat(data->In_Format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    // Set Output Address, Width, Height and Pitch
    //
    ISPctxt->OutInfo.AddrY[0] = (MMP_UINT8*)data->Out_AddrY;
    ISPctxt->OutInfo.AddrU[0] = (MMP_UINT8*)data->Out_AddrU;
    ISPctxt->OutInfo.AddrV[0] = (MMP_UINT8*)data->Out_AddrV;

    // width must be 2 alignment
    ISPctxt->OutInfo.Width  = (data->Out_Width >> 2) << 2;
    ISPctxt->OutInfo.Height = data->Out_Height;

    ISPctxt->OutInfo.PitchY  = data->Out_PitchY;
    ISPctxt->OutInfo.PitchUV = data->Out_PitchUV;

    // Set Output Format
    result = ISP_SetOutputFormat(data->Out_Format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Set single porcess
    ISPctxt->InInfo.EnableReadMemoryMode = MMP_TRUE;
    ISPctxt->InInfo.DisableCaptureCtrl = MMP_TRUE;
    ISPctxt->InInfo.InputBufferNum = 0;

    ISPctxt->OutInfo.OutputBufferNum = 0;
    ISPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_TRUE;
    ISPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;
    ISPctxt->OutInfo.EnableFieldMode = MMP_FALSE;
    ISPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;

    ISPctxt->InterruptMode = 0x2;

    ISPctxt->JpegEncode.EnableJPEGEncode = MMP_FALSE;

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputAddr;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutBufInfo;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_JpegEncode;

    // Update parameter
    result = ISP_Update();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // ISP_LogReg();
    ISP_DriverFire_Reg();

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP Sequence Process.
 * @param data MMP_ISP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspSequenceProcess(
    const MMP_ISP_SEQUENCE_SHARE     *data)
{
    ISP_RESULT      result = ISP_SUCCESS;
    MMP_UINT8       i;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    if (ISPctxt->OutInfo.Width == 0 || ISPctxt->OutInfo.Height == 0)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    // Set Input Address, Width, Height and Pitch
    //
    if (data->In_BufferNum == 0 && data->EnCapOnflyMode == MMP_FALSE)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }
    else if (data->EnCapOnflyMode == MMP_FALSE)
    {
        ISPctxt->InInfo.InputBufferNum = data->In_BufferNum - 1;

        for (i = 0; i < 5; i++)
        {
            if (i < data->In_BufferNum)
            {
                ISPctxt->InInfo.AddrY[i] = (MMP_UINT8*)data->In_AddrY[i];
                ISPctxt->InInfo.AddrUV[i] = (MMP_UINT8*)data->In_AddrUV[i];
            }
            else
            {
                ISPctxt->InInfo.AddrY[i] = 0x0;
                ISPctxt->InInfo.AddrUV[i] = 0x0;
            }
        }
        ISPctxt->InInfo.AddrYp = 0x0;

        ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputAddr;
    }

    //width must be 2 alignment
    ISPctxt->InInfo.SrcWidth  = (data->In_Width >> 2) << 2;
    ISPctxt->InInfo.SrcHeight = data->In_Height;

    ISPctxt->InInfo.PitchY  = data->In_PitchY;
    ISPctxt->InInfo.PitchUV = data->In_PitchUV;

    //Set Input Format
    result = ISP_SetInputFormat(data->In_Format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //Set sqeuence porcess

    if (data->EnCapOnflyMode == MMP_TRUE)
        ISPctxt->InInfo.EnableReadMemoryMode = MMP_FALSE;
    else
        ISPctxt->InInfo.EnableReadMemoryMode = MMP_TRUE;

    ISPctxt->InInfo.DisableCaptureCtrl = MMP_FALSE;

    ISPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_FALSE;
    ISPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;

    if (data->EnOnflyInFieldMode == MMP_TRUE)
        ISPctxt->OutInfo.EnableFieldMode = MMP_TRUE;
    else
        ISPctxt->OutInfo.EnableFieldMode = MMP_FALSE;

    ISPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;

    ISPctxt->JpegEncode.EnableJPEGEncode = MMP_FALSE;

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_JpegEncode;

    //check isp engine idle
    result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }
    
    ISPctxt->UpdateFlags = 0xFFFFFFFF;
    
    //Update parameter
    result = ISP_Update();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // ISP_LogReg();
    ISP_RefreshFire_Reg();

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
* Set Sequence Output Parameter
* @param MMP_ISP_SEQUENCE_SHARE
* @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
*/
//=============================================================================
MMP_RESULT
mmpIspSetSequenceOutputInfo(
    const MMP_ISP_SEQUENCE_SHARE    *data)
{
    ISP_RESULT  result = ISP_SUCCESS;
    MMP_UINT8   i;

    if (ISPctxt == MMP_NULL)
    {
        result = ISP_ERR_NOT_INITIALIZE;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //
    //Set Output Address, Width, Height and Pitch
    //
    if (data->Out_BufferNum == 0)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }
    else
    {
        ISPctxt->OutInfo.OutputBufferNum = data->Out_BufferNum - 1;

        for (i = 0; i < 5; i++)
        {
            if (i < data->Out_BufferNum)
            {
                ISPctxt->OutInfo.AddrY[i] = (MMP_UINT8*)data->Out_AddrY[i];
                ISPctxt->OutInfo.AddrU[i] = (MMP_UINT8*)data->Out_AddrU[i];
                ISPctxt->OutInfo.AddrV[i] = (MMP_UINT8*)data->Out_AddrV[i];
            }
            else
            {
                ISPctxt->OutInfo.AddrY[i] = 0x0;
                ISPctxt->OutInfo.AddrU[i] = 0x0;
                ISPctxt->OutInfo.AddrV[i] = 0x0;
            }
        }
    }

    //width must be 2 alignment
    ISPctxt->OutInfo.Width  = (data->Out_Width >> 2) << 2;
    ISPctxt->OutInfo.Height = data->Out_Height;

    ISPctxt->OutInfo.PitchY  = data->Out_PitchY;
    ISPctxt->OutInfo.PitchUV = data->Out_PitchUV;

    //Set Output Format
    result = ISP_SetOutputFormat(data->Out_Format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutBufInfo;
    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Set frame function background image information & color key.  (For Direct Assign VRAM address. Ex.2D input)
 *
 * @param baseAddr      base address of the background image buffer.
 * @param startX        x position of the background image.
 * @param startY        y position of the background image.
 * @param width         width of the background image.
 * @param height        height of the background image.
 * @param colorKeyR     color key for R channel.
 * @param colorKeyG     color key for G channel.
 * @param colorKeyB     color key for B channel.
 * @param constantAlpha constant Alpha Value.
 * @param format        format of the picture & color key. only support RGB 888, RGB565
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @see mmpIspEnable() mmpIspDisable()
 */
//=============================================================================
MMP_RESULT
mmpIspSetFrameFunction(
    void*                   vramAddr,
    MMP_UINT                startX,
    MMP_UINT                startY,
    MMP_UINT                width,
    MMP_UINT                height,
    MMP_UINT                pitch,
    MMP_UINT                colorKeyR,
    MMP_UINT                colorKeyG,
    MMP_UINT                colorKeyB,
    MMP_UINT                constantAlpha,
    MMP_PIXEL_FORMAT        format)
{
    ISP_RESULT          result = ISP_SUCCESS;
    ISP_FRMFUN_CTRL     *pIspFrameFunc0 = &ISPctxt->FrameFun0;

    pIspFrameFunc0->EnableRGB2YUV = MMP_TRUE;

    if (width < 16 || height < 16)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err, width(%d) < 16 or height(%d) < 16 !!", width, height);
        result = ISP_ERR_INVALID_PARAM;
        goto end;
    }

    startX -= (startX & 0x1); // startX%2;
    startY -= (startY & 0x1); // startY%2;
    width  -= (width & 0x1);  // width%2;
    height -= (height & 0x1); // height%2;

    switch (format)
    {
    case MMP_PIXEL_FORMAT_ARGB4444:
        pIspFrameFunc0->Format = ARGB4444;
        break;

    case MMP_PIXEL_FORMAT_RGB565:
        pIspFrameFunc0->Format = CARGB565;
        break;

    default :
        result = ISP_ERR_NO_MATCH_OUTPUT_FORMAT;
        goto end;
        break;
    }

    pIspFrameFunc0->ColorKeyR = (MMP_UINT16) colorKeyR;
    pIspFrameFunc0->ColorKeyG = (MMP_UINT16) colorKeyG;
    pIspFrameFunc0->ColorKeyB = (MMP_UINT16) colorKeyB;
    pIspFrameFunc0->ConstantAlpha = (MMP_UINT16) constantAlpha;
    pIspFrameFunc0->StartX = (MMP_UINT16) startX;
    pIspFrameFunc0->StartY = (MMP_UINT16) startY;
    pIspFrameFunc0->Width  = (MMP_UINT16) width;
    pIspFrameFunc0->Height = (MMP_UINT16) height;

    pIspFrameFunc0->Pitch  = (MMP_UINT16) pitch;
    pIspFrameFunc0->Addr   = (MMP_UINT8*) ((MMP_UINT) vramAddr);

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_FrameFun0;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Wait ISP Engine Idle
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspWaitEngineIdle(
    void)
{
    return ISP_WaitEngineIdle();
}

//=============================================================================
/**
 * Is ISP Engine Idle
 */
//=============================================================================
MMP_BOOL
mmpIspIsEngineIdle(
    void)
{
    return ISP_IsEngineIdle();
}

//=============================================================================
/**
 * Wait ISP Interrupt Idle
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspWaitInterruptIdle(
    void)
{
    return ISP_WaitInterruptIdle();
}

//=============================================================================
/**
 * Clear ISP Interrupt
 */
//=============================================================================
void
mmpIspClearInterrupt(
    void)
{
    ISP_ClearInterrupt_Reg();
}

//=============================================================================
/**
 * ISP Write Buffer Index
 * @return index number
 */
//=============================================================================
MMP_UINT16
mmpIspReturnWrBufIndex(
    void)
{
    return ISP_RetrunWrBufIndex_Reg();
}

//=============================================================================
/**
 * mmpIspRegisterIRQ.
 */
//=============================================================================
void
mmpIspRegisterIRQ(
    Isp_handler isphandler)
{
    // Initialize ISP IRQ
    ithIntrDisableIrq(ITH_INTR_ISP);
    ithIntrClearIrq(ITH_INTR_ISP);

    #if defined (__FREERTOS__)
    // register NAND Handler to IRQ
    ithIntrRegisterHandlerIrq(ITH_INTR_ISP, isphandler, MMP_NULL);
    #endif // defined (__FREERTOS__)

    // set IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_ISP, ITH_INTR_EDGE);

    // set IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_ISP, ITH_INTR_HIGH_RISING);

    // Enable IRQ
    ithIntrEnableIrq(ITH_INTR_ISP);
}

//=============================================================================
/**
 * Disable ISP Interrupt.
 */
//=============================================================================
MMP_RESULT
mmpIspDisableInterrupt(
    void)
{
    HOST_WriteRegisterMask(ISP_REG_SET50E, ((0x0 & ISP_BIT_ISP_INTERRUPT_EN) << ISP_SHT_ISP_INTERRUPT_EN), (ISP_BIT_ISP_INTERRUPT_EN << ISP_SHT_ISP_INTERRUPT_EN));     
    
    if (ISPctxt != MMP_NULL)
        ISPctxt->EnableInterrupt = MMP_FALSE;       
}

//=============================================================================
/**
 * mmpIspResetEngine
 */
//=============================================================================
MMP_RESULT
mmpIspResetEngine(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    HOST_ISP_Reset();

    return result;
}


//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
void
mmpIspSetColorCtrl(
    const MMP_ISP_COLOR_CTRL *data)
{
#if defined (USE_COLOR_EFFECT)
    if (data->brightness > 127)
        ISPctxt->ColorCtrl.brightness = 127;
    else if (data->brightness < -128)
        ISPctxt->ColorCtrl.brightness = -128;
    else
        ISPctxt->ColorCtrl.brightness = data->brightness;

    if (data->contrast > 4.0)
        ISPctxt->ColorCtrl.contrast = 4.0;
    else if (data->contrast < 0.0)
        ISPctxt->ColorCtrl.contrast = 0.0;
    else
        ISPctxt->ColorCtrl.contrast = data->contrast;

    if (data->hue > 359)
        ISPctxt->ColorCtrl.hue = 359;
    else if (data->hue < 0)
        ISPctxt->ColorCtrl.hue = 0;
    else
        ISPctxt->ColorCtrl.hue = data->hue;

    if (data->saturation > 4.0)
        ISPctxt->ColorCtrl.saturation = 4.0;
    else if (data->saturation < 0.0)
        ISPctxt->ColorCtrl.saturation = 0.0;
    else
        ISPctxt->ColorCtrl.saturation = data->saturation;

    ISP_SetColorCorrMatrix(
        &ISPctxt->CCFun,
        ISPctxt->ColorCtrl.brightness,
        ISPctxt->ColorCtrl.contrast,
        ISPctxt->ColorCtrl.hue,
        ISPctxt->ColorCtrl.saturation,
        ISPctxt->ColorCtrl.colorEffect);

    ISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_CCMatrix;
#endif
}

//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
void
mmpIspGetColorCtrl(
    MMP_ISP_COLOR_CTRL *data)
{
#if defined (USE_COLOR_EFFECT)
    data->brightness    = ISPctxt->ColorCtrl.brightness;
    data->contrast      = ISPctxt->ColorCtrl.contrast;
    data->hue           = ISPctxt->ColorCtrl.hue;
    data->saturation    = ISPctxt->ColorCtrl.saturation;
#endif
}

//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
void
mmpIspOnflyUpdateColorMatrix(
    void)
{
#if defined (USE_COLOR_EFFECT)
    ISP_UpdateColorMatrix();
#endif
}
