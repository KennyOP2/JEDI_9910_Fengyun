#ifndef __MMP_ISP_H_2XXFCCN9_G1EL_C42M_SEM9_S29A3L5FADVX__
#define __MMP_ISP_H_2XXFCCN9_G1EL_C42M_SEM9_S29A3L5FADVX__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp/isp_error.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(WIN32)

#if defined(ISP_EXPORTS)
#define ISP_API __declspec(dllexport)
#else
#define ISP_API __declspec(dllimport)
#endif

#else
#define ISP_API extern
#endif  //#if defined(WIN32)

//=============================================================================
//                              Constant Definition
//=============================================================================

/**
 *  JPEG/MPEG engine fire mode
 */
typedef enum TRIGGER_MODE_TAG
{
    TRIGGER_MODE_HW,
    TRIGGER_MODE_COMMAND
} TRIGGER_MODE;

/**
 * @remark When application enable or disable these capability during preview,
 * it must call mmpIspUpdate() to active it.
 *
 * @see mmpIspEnable mmpIspDisable
 */
typedef enum MMP_ISP_CAPS_TAG
{
    //Enable/disable deinterlace
    MMP_ISP_DEINTERLACE,

    //Enable/disable low leve ledge deinterlace
    MMP_ISP_LOWLEVELEDGE,

    //Enable/disable frame function
    MMP_ISP_FRAME_FUNCTION_0,

    //Enable/disable remap address
    MMP_ISP_REMAP_ADDRESS,

    //Enable/disable Interrupt
    MMP_ISP_INTERRUPT,
    
    //Top Field Deinter
    MMP_ISP_DEINTER_FIELD_TOP,
    
    //Bottom Field Deinter
    MMP_ISP_DEINTER_FIELD_BOTTOM
    
} MMP_ISP_CAPS;

// Input format
typedef enum MMP_ISP_INFORMAT_TAG
{
    MMP_ISP_IN_NV12,
    MMP_ISP_IN_NV21
} MMP_ISP_INFORMAT;

// Output format
typedef enum MMP_ISP_OUTFORMAT_TAG
{
    //ISP Output Format
    MMP_ISP_OUT_YUV422    = MMP_PIXEL_FORMAT_YUV422,
    MMP_ISP_OUT_YUV420    = MMP_PIXEL_FORMAT_YV12,
    MMP_ISP_OUT_YUV444    = 0x0101, // avoid mapping fail
    MMP_ISP_OUT_YUV422R   = 0x0102, // avoid mapping fail
    MMP_ISP_OUT_NV12      = 0x0103, // avoid mapping fail
    MMP_ISP_OUT_NV21      = 0x0104  // avoid mapping fail
} MMP_ISP_OUTFORMAT;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MMP_ISP_SINGLE_SHARE_TAG
{
    //Signal Process Input Parameter
    MMP_UINT32          In_AddrY;
    MMP_UINT32          In_AddrUV;
    MMP_UINT32          In_AddrYp;
    MMP_UINT16          In_Width;
    MMP_UINT16          In_Height;
    MMP_UINT16          In_PitchY;
    MMP_UINT16          In_PitchUV;
    MMP_ISP_INFORMAT    In_Format;

    //Signal Process Output Parameter
    MMP_UINT32          Out_AddrY;
    MMP_UINT32          Out_AddrU;
    MMP_UINT32          Out_AddrV;
    MMP_UINT16          Out_Width;
    MMP_UINT16          Out_Height;
    MMP_UINT16          Out_PitchY;
    MMP_UINT16          Out_PitchUV;
    MMP_ISP_OUTFORMAT   Out_Format;
} MMP_ISP_SINGLE_SHARE;

typedef struct MMP_ISP_SEQUENCE_SHARE_TAG
{
    //Input Parameter
    MMP_UINT32          In_AddrY[5];
    MMP_UINT32          In_AddrUV[5];
    MMP_UINT16          In_Width;
    MMP_UINT16          In_Height;
    MMP_UINT16          In_PitchY;
    MMP_UINT16          In_PitchUV;
    MMP_ISP_INFORMAT    In_Format;
    MMP_UINT16          In_BufferNum;

    //Output Parameter
    MMP_UINT32          Out_AddrY[5];
    MMP_UINT32          Out_AddrU[5];
    MMP_UINT32          Out_AddrV[5];
    MMP_UINT16          Out_Width;
    MMP_UINT16          Out_Height;
    MMP_UINT16          Out_PitchY;
    MMP_UINT16          Out_PitchUV;
    MMP_ISP_OUTFORMAT   Out_Format;
    MMP_UINT16          Out_BufferNum;

    //for sequence process
    MMP_BOOL            EnCapOnflyMode;
    MMP_BOOL            EnOnflyInFieldMode;
} MMP_ISP_SEQUENCE_SHARE;

typedef struct MMP_ISP_REMAP_ADDR_TAG
{
    MMP_UINT16          YAddr_03;
    MMP_UINT16          YAddr_04;
    MMP_UINT16          YAddr_05;
    MMP_UINT16          YAddr_06;
    MMP_UINT16          YAddr_07;
    MMP_UINT16          YAddr_08;
    MMP_UINT16          YAddr_09;
    MMP_UINT16          YAddr_10;
    MMP_UINT16          YAddr_11;
    MMP_UINT16          YAddr_12;
    MMP_UINT16          YAddr_13;
    MMP_UINT16          YAddr_14;
    MMP_UINT16          YAddr_15;
    MMP_UINT16          YAddr_16;
    MMP_UINT16          YAddr_17;
    MMP_UINT16          YAddr_18;
    MMP_UINT16          YAddr_19;
    MMP_UINT16          YAddr_20;
    MMP_UINT16          YAddr_21;
    MMP_UINT16          YAddr_22;
    MMP_UINT16          YAddr_23;
    MMP_UINT16          YAddr_24;
    MMP_UINT16          YAddr_25;
    MMP_UINT16          YAddr_26;
    MMP_UINT16          YAddr_27;
    MMP_UINT16          YAddr_28;
    MMP_UINT16          YAddr_29;
    MMP_UINT16          YAddr_30;
    MMP_UINT16          YAddr_31;

    MMP_UINT16          UVAddr_03;
    MMP_UINT16          UVAddr_04;
    MMP_UINT16          UVAddr_05;
    MMP_UINT16          UVAddr_06;
    MMP_UINT16          UVAddr_07;
    MMP_UINT16          UVAddr_08;
    MMP_UINT16          UVAddr_09;
    MMP_UINT16          UVAddr_10;
    MMP_UINT16          UVAddr_11;
    MMP_UINT16          UVAddr_12;
    MMP_UINT16          UVAddr_13;
    MMP_UINT16          UVAddr_14;
    MMP_UINT16          UVAddr_15;
    MMP_UINT16          UVAddr_16;
    MMP_UINT16          UVAddr_17;
    MMP_UINT16          UVAddr_18;
    MMP_UINT16          UVAddr_19;
    MMP_UINT16          UVAddr_20;
    MMP_UINT16          UVAddr_21;
    MMP_UINT16          UVAddr_22;
    MMP_UINT16          UVAddr_23;
    MMP_UINT16          UVAddr_24;
    MMP_UINT16          UVAddr_25;
    MMP_UINT16          UVAddr_26;
    MMP_UINT16          UVAddr_27;
    MMP_UINT16          UVAddr_28;
    MMP_UINT16          UVAddr_29;
    MMP_UINT16          UVAddr_30;
    MMP_UINT16          UVAddr_31;
} MMP_ISP_REMAP_ADDR;

typedef struct MMP_ISP_COLOR_CTRL_TAG
{
    MMP_INT32           brightness;
    MMP_FLOAT           contrast;
    MMP_INT32           hue;
    MMP_FLOAT           saturation;
} MMP_ISP_COLOR_CTRL;

typedef void (*Isp_handler)(void* arg);

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

//=============================================================================
/**
 * ISP initialization.
 *
 * @return ISP_SUCCESS if succeed, error codes of ISP_ERR_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 *
 * @see MMP_ISP_MODE
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspInitialize(
    void);

//=============================================================================
/**
 * ISP terminate.
 *
 * @return ISP_SUCCESS if succeed, error codes of ISP_ERR_ERROR otherwise.
 *
 * @remark Application must call this API when leaving ISP module.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspTerminate(
    void);

//=============================================================================
/**
 * ISP context reset.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspContextReset(
    void);
    
//=============================================================================
/**
 * Enable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return ISP_SUCCESS if succeed, error codes of ISP_ERR_ERROR otherwise.
 *
 * @see MMP_ISP_ATTRIBUTE mmpIspDisable
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspEnable(
    MMP_ISP_CAPS    cap);

//=============================================================================
/**
 * Disable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return ISP_SUCCESS if succeed, error codes of ISP_ERR_ERROR otherwise.
 *
 * @see MMP_ISP_ATTRIBUTE mmpIspEnable
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspDisable(
    MMP_ISP_CAPS    cap);

//=============================================================================
/**
 * Query ISP capability.
 *
 * @param cap       Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_TRUE if function enabled, MMP_FALSE if function disable.
 */
//=============================================================================
ISP_API MMP_BOOL
mmpIspQuery(
    MMP_ISP_CAPS    cap);

//=============================================================================
/**
 * Set Remap Address Parameter
 * @param MMP_ISP_REMAP_ADDR
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpIspSetRemapAddr(
    MMP_ISP_REMAP_ADDR  *data);

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
    const MMP_ISP_SINGLE_SHARE  *data);

//=============================================================================
/**
 * ISP Sequence Process.
 * @param data MMP_ISP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspSingleProcess(
    const MMP_ISP_SINGLE_SHARE  *data);

//=============================================================================
/**
 * Set Sequence Output Parameter
 * @param MMP_ISP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspSequenceProcess(
    const MMP_ISP_SEQUENCE_SHARE    *data);

//=============================================================================
/**
 * Set Sequence Output Parameter
 * @param MMP_ISP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspSetSequenceOutputInfo(
    const MMP_ISP_SEQUENCE_SHARE    *data);

//=============================================================================
/**
 * Set frame function background image information & color key.  (For Direct
 * Assign VRAM address. Ex.2D input)
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
ISP_API MMP_RESULT
mmpIspSetFrameFunction(
    void*                 vramAddr,
    MMP_UINT              startX,
    MMP_UINT              startY,
    MMP_UINT              width,
    MMP_UINT              height,
    MMP_UINT              pitch,
    MMP_UINT              colorKeyR,
    MMP_UINT              colorKeyG,
    MMP_UINT              colorKeyB,
    MMP_UINT              constantAlpha,
    MMP_PIXEL_FORMAT      format);

//=============================================================================
/**
* Wait ISP Engine Idle
* @return ISP_SUCCESS if succeed, error codes of ISP_ERR_ERROR otherwise.
*/
//=============================================================================
ISP_API MMP_RESULT
mmpIspWaitEngineIdle(
    void);

//=============================================================================
/**
 * Is ISP Engine Idle
 */
//=============================================================================
MMP_BOOL
mmpIspIsEngineIdle(
    void);
    
//=============================================================================
/**
 * Wait ISP Interrupt Idle
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspWaitInterruptIdle(
    void);
    
//=============================================================================
/**
 * Clear ISP Interrupt
 */
//=============================================================================
ISP_API void
mmpIspClearInterrupt(
    void);

//=============================================================================
/**
 * ISP Write Buffer Index
 * @return index number
 */
//=============================================================================
ISP_API MMP_UINT16
mmpIspReturnWrBufIndex(
    void);

//=============================================================================
/**
 * ISP Register IRQ
 */
//=============================================================================
ISP_API void
mmpIspRegisterIRQ(
    Isp_handler isphandler);

//=============================================================================
/**
 * Disable ISP Interrupt.
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspDisableInterrupt(
    void);

//=============================================================================
/**
 * mmpIspResetEngine
 */
//=============================================================================
ISP_API MMP_RESULT
mmpIspResetEngine(
    void);

//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
ISP_API void
mmpIspSetColorCtrl(
    const MMP_ISP_COLOR_CTRL     *data);

//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
ISP_API void
mmpIspGetColorCtrl(
    MMP_ISP_COLOR_CTRL     *data);

//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
ISP_API void
mmpIspOnflyUpdateColorMatrix(
    void);

#ifdef __cplusplus
}
#endif

#endif
