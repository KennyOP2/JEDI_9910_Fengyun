#ifndef __ISP_TYPES_H_6LX4PNNH_236F_H9EQ_5J77_Q9PJXPQ2OI5F__
#define __ISP_TYPES_H_6LX4PNNH_236F_H9EQ_5J77_Q9PJXPQ2OI5F__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "mmp_isp.h"
//=============================================================================
//                              Constant Definition
//=============================================================================
#define ISP_SCALE_TAP               4
#define ISP_SCALE_TAP_SIZE          8
#define ISP_SCALE_MAX_BLOCK_WIDTH   512
#define ISP_SCALE_MAX_BLOCK_HEIGHT  60
#define ISP_SCLAE_MAX_LINE_BUFF_LEN 1920

#define FULLHD_WIDTH                1920
#define FULLHD_HEIGHT               1080
#define HD_WIDTH                    1280
#define HD_HEIGHT                   720
#define PAL_WIDTH                   720
#define PAL_HEIGHT                  576

#define VIDEO_BLANK_WIDTH           16
#define VIDEO_BLANK_HEIGHT          16
#define BLANK_BUFFER_INDEX          (4)

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 *  Debug message
 */

typedef enum _ISP_MSG_TYPE
{
    ISP_MSG_TYPE_ERR          = (0x1 << 0),   
} ISP_MSG_TYPE;


#ifdef _MSC_VER // WIN32
    #ifndef trac
    #define trac(string, ...)               do{ printf(string, __VA_ARGS__); \
                                                 printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define isp_msg(type, string, ...)      ((void)((type) ? printf(string, __VA_ARGS__) : MMP_NULL))
    #define isp_msg_ex(type, string, ...)   do{ if(type){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)  

#else
    #ifndef trac
    #define trac(string, args...)               do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define isp_msg(type, string, args...)      ((void)((type) ? printf(string, ## args) : MMP_NULL))
    #define isp_msg_ex(type, string, args...)   do{ if(type){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)    
#endif

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  ISP Update Flags
 */
typedef enum ISP_FLAGS_UPDATE_TAG
{
    ISP_FLAGS_UPDATE_InputParameter         = (0x00000001 << 0),
    ISP_FLAGS_UPDATE_InputBuf               = (0x00000001 << 1),
    ISP_FLAGS_UPDATE_InputAddr              = (0x00000001 << 2),
    ISP_FLAGS_UPDATE_DeInterlaceParam       = (0x00000001 << 3),
    ISP_FLAGS_UPDATE_JpegEncode             = (0x00000001 << 4),    
    ISP_FLAGS_UPDATE_CCMatrix               = (0x00000001 << 5),
    ISP_FLAGS_UPDATE_ScaleParam             = (0x00000001 << 6),
    ISP_FLAGS_UPDATE_ScaleMatrixH           = (0x00000001 << 7),
    ISP_FLAGS_UPDATE_ScaleMatrixV           = (0x00000001 << 8),
    ISP_FLAGS_UPDATE_FrameFun0              = (0x00000001 << 9),
    ISP_FLAGS_UPDATE_RGBtoYUVMatrix         = (0x00000001 << 10),
    ISP_FLAGS_UPDATE_OutParameter           = (0x00000001 << 11),
    ISP_FLAGS_UPDATE_OutBufInfo             = (0x00000001 << 12),
    ISP_FLAGS_UPDATE_OutAddress             = (0x00000001 << 13),
    ISP_FLAGS_UPDATE_FrmMatrix              = (0x00000001 << 14),
    ISP_FLAGS_UPDATE_RemapAddr              = (0x00000001 << 15),
    ISP_FLAGS_UPDATE_Interrupt              = (0x00000001 << 16)

} ISP_FLAGS_UPDATE;

/************************************************************************
* \brief
*   type define base on isp specification
 ***********************************************************************/
typedef enum ISP_DEINTER_MODE_TAG
{
    DEINTER3D = 0, 
    DEINTER2D = 1

} ISP_DEINTER_MODE;

typedef enum ISP_FRAMEFUN_FORMAT_TAG
{
    CARGB565 = 0, 
    ARGB4444 = 1

} ISP_FRAMEFUN_FORMAT;

typedef enum ISP_OUT_FORMAT_TAG
{
    NVMode = 0, 
    YUVPlane = 1
    
} ISP_OUT_FORMAT;

typedef enum ISP_YUV_PLANE_FORMAT_TAG
{
    YUV422 = 0, 
    YUV420 = 1, 
    YUV444 = 2, 
    YUV422R = 3

} ISP_YUV_PLANE_FORMAT;

typedef enum ISP_NV_FORMAT_TAG
{
    NV12 = 0, 
    NV21 = 1

} ISP_NV_FORMAT;

//=============================================================================
//                              Structure Definition
//=============================================================================

//Input Buffer Struct
typedef struct ISP_INPUT_INFO_TAG
{
    MMP_BOOL                EnableInYUV255Range;    
    MMP_BOOL                EnableReadMemoryMode;
    MMP_BOOL                DisableCaptureCtrl;
    MMP_UINT16              InputBufferNum;
    ISP_NV_FORMAT           NVFormat;
    MMP_UINT8*              AddrY[5];
    MMP_UINT8*              AddrUV[5];
    MMP_UINT8*              AddrYp;
    MMP_UINT16              PitchY;
    MMP_UINT16              PitchUV;
    MMP_UINT16              SrcWidth;
    MMP_UINT16              SrcHeight; 
    MMP_BOOL                EnableCCFun;
    
} ISP_INPUT_INFO;

//Deinterlace
typedef struct ISP_DEINTERLACE_CTRL_TAG
{
    MMP_BOOL                Enable;
    MMP_BOOL                EnableOutMotion;
    ISP_DEINTER_MODE        DeinterMode;
    MMP_BOOL                EnSrcBottomFieldFirst;
    MMP_BOOL                EnDeinterBottomField;
    MMP_BOOL                EnSrcLPF;
    MMP_BOOL                EnLummaEdgeDetect;
    MMP_BOOL                EnChromaEdgeDetect;
    MMP_BOOL                UVRepeatMode;
    MMP_BOOL                Disable30MotionDetect;

    //For 3D Parameter
    MMP_UINT16              MDThreshold_High;
    MMP_UINT16              MDThreshold_Low;
    MMP_UINT16              MDThreshold_Step;
    MMP_BOOL                EnLPFBlend;
    MMP_BOOL                EnLPFWeight;
    MMP_BOOL                EnLPFStaticPixel;
    MMP_BOOL                DisableMV_A;
    MMP_BOOL                DisableMV_B;
    MMP_BOOL                DisableMV_C;
    MMP_BOOL                DisableMV_D;
    MMP_BOOL                DisableMV_E;
    MMP_BOOL                DisableMV_F;
    MMP_BOOL                DisableMV_G;    

    //For 2D Parameter
    MMP_UINT16              D2EdgeBlendWeight;
    MMP_UINT16              D2OrgBlendWeight;
    
    //LowLevelEdge
    MMP_BOOL                EnLowLevelEdge;
    MMP_BOOL                EnLowLevelOutside;
    MMP_UINT16              LowLevelMode;
    MMP_UINT16              LowLevelBypassBlend;
    MMP_UINT16              LowLevelPosX;
    MMP_UINT16              LowLevelPosY;
    MMP_UINT16              LowLevelWidth;
    MMP_UINT16              LowLevelHeight;    
    
    //A2ECO fro UV 2D interpolation mode
    MMP_BOOL                EnUV2DMethod;

} ISP_DEINTERLACE_CTRL;

//Jpeg Encode
typedef struct ISP_JEPG_ENCODE_TAG
{
    MMP_BOOL                EnableJPEGEncode;
    MMP_UINT16              TotalSliceNum;
   
} ISP_JEPG_ENCODE_CTRL;

//Color correction matrix
typedef struct ISP_COLOR_CORRECTION_TAG
{
    MMP_INT32               OffsetR;
    MMP_INT32               OffsetG;
    MMP_INT32               OffsetB;
    MMP_INT32               _11;
    MMP_INT32               _12;
    MMP_INT32               _13;
    MMP_INT32               _21;
    MMP_INT32               _22;
    MMP_INT32               _23;
    MMP_INT32               _31;
    MMP_INT32               _32;
    MMP_INT32               _33;
    MMP_INT32               DeltaR;
    MMP_INT32               DeltaG;
    MMP_INT32               DeltaB;

} ISP_COLOR_CORRECTION;

//Scale Function
typedef struct ISP_SCALE_CTRL_TAG
{
    MMP_FLOAT               HCI;
    MMP_FLOAT               VCI;
    MMP_FLOAT               WeightMatX[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
    MMP_FLOAT               WeightMatY[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
    
} ISP_SCALE_CTRL;

//Frame Function
typedef struct ISP_FRMFUN_CTRL_TAG
{
    MMP_BOOL                Enable;
    MMP_BOOL                EnableGobang;
    MMP_BOOL                EnableFieldMode;
    MMP_BOOL                EnableRGB2YUV;
    MMP_UINT8*              Addr;
    MMP_UINT16              Width;
    MMP_UINT16              Height;
    MMP_UINT16              Pitch;
    MMP_UINT16              StartX;
    MMP_UINT16              StartY;
    MMP_UINT16              ColorKeyR;
    MMP_UINT16              ColorKeyG;
    MMP_UINT16              ColorKeyB;
    MMP_UINT16              ConstantAlpha;
    ISP_FRAMEFUN_FORMAT     Format;
            
} ISP_FRMFUN_CTRL;

//Transfer Matrix RGB to YUV
typedef struct ISP_RGB_TO_YUV_TAG
{
    MMP_UINT16              _11;
    MMP_UINT16              _12;
    MMP_UINT16              _13;
    MMP_UINT16              _21;
    MMP_UINT16              _22;
    MMP_UINT16              _23;
    MMP_UINT16              _31;
    MMP_UINT16              _32;
    MMP_UINT16              _33;
    MMP_UINT16              ConstY;
    MMP_UINT16              ConstU;
    MMP_UINT16              ConstV;

} ISP_RGB_TO_YUV;

//Output
typedef struct ISP_OUTPUT_INFO_TAG
{
    MMP_UINT16              SWWrFlipNum;
    MMP_BOOL                EnableSWFlipMode;
    MMP_UINT16              OutputBufferNum;
    MMP_BOOL                EnableFieldMode;
    MMP_UINT16              EngineDelay;
    MMP_BOOL                EnableUVBiDownsample;
    MMP_BOOL                EnableRemapYAddr;
    MMP_BOOL                EnableRemapUVAddr;
    MMP_BOOL                EnableSWCtrlRdAddr;
    MMP_BOOL                DisableOutMatrix;
    
    ISP_OUT_FORMAT          OutFormat;
    ISP_YUV_PLANE_FORMAT    PlaneFormat;
    ISP_NV_FORMAT           NVFormat;
    
    MMP_UINT8*              AddrY[5];
    MMP_UINT8*              AddrU[5];
    MMP_UINT8*              AddrV[5];
    MMP_UINT16              Width;
    MMP_UINT16              Height;
    MMP_UINT16              PitchY;
    MMP_UINT16              PitchUV;

} ISP_OUTPUT_INFO;

//Remap Addr
typedef struct ISP_REMAP_ADDR_TAG
{
    MMP_UINT16              Addr_03;
    MMP_UINT16              Addr_04;
    MMP_UINT16              Addr_05;
    MMP_UINT16              Addr_06;
    MMP_UINT16              Addr_07;
    MMP_UINT16              Addr_08;
    MMP_UINT16              Addr_09;
    MMP_UINT16              Addr_10;
    MMP_UINT16              Addr_11;
    MMP_UINT16              Addr_12;
    MMP_UINT16              Addr_13;  
    MMP_UINT16              Addr_14;  
    MMP_UINT16              Addr_15;
    MMP_UINT16              Addr_16;
    MMP_UINT16              Addr_17;
    MMP_UINT16              Addr_18;
    MMP_UINT16              Addr_19;
    MMP_UINT16              Addr_20;
    MMP_UINT16              Addr_21;
    MMP_UINT16              Addr_22;
    MMP_UINT16              Addr_23;
    MMP_UINT16              Addr_24;
    MMP_UINT16              Addr_25;
    MMP_UINT16              Addr_26;
    MMP_UINT16              Addr_27;
    MMP_UINT16              Addr_28;
    MMP_UINT16              Addr_29;
    MMP_UINT16              Addr_30;
    MMP_UINT16              Addr_31;  

} ISP_REMAP_ADDR;

typedef struct ISP_COLOR_CTRL_TAG
{
    MMP_INT32           brightness;
    MMP_FLOAT           contrast;
    MMP_INT32           hue;
    MMP_FLOAT           saturation;
    MMP_INT32           colorEffect[2];

} ISP_COLOR_CTRL;

/**
 * ISP Data
 */
typedef struct ISP_CONTEXT_TAG
{
    //Update Flag
    ISP_FLAGS_UPDATE        UpdateFlags;

    //Input Information
    ISP_INPUT_INFO          InInfo;

    //Deinterlace
    ISP_DEINTERLACE_CTRL    DeInterlace;

    //Jpeg encode
    ISP_JEPG_ENCODE_CTRL    JpegEncode;
    
    //Color Correction Function
    ISP_COLOR_CORRECTION    CCFun;

    //Scale Function
    ISP_SCALE_CTRL          ScaleFun;

    //Frame Function
    ISP_FRMFUN_CTRL         FrameFun0;
    ISP_RGB_TO_YUV          FrmMatrix;

    //RGB2YUV Function
    ISP_RGB_TO_YUV          RGB2YUVFun;

    //Output Information
    ISP_OUTPUT_INFO         OutInfo;
    
    //Remap Luma and Chroma Addr
    ISP_REMAP_ADDR          RemapYAddr;
    ISP_REMAP_ADDR          RemapUVAddr;
    
    //Interrupt
    MMP_BOOL                EnableInterrupt;
    MMP_UINT16              InterruptMode;
    
    //color contrl
    ISP_COLOR_CTRL          ColorCtrl;

} ISP_CONTEXT;


extern ISP_CONTEXT* ISPctxt;


//=============================================================================
//                Global Data Definition
//=============================================================================


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

