/*
*
*/


typedef enum _ADV7180_INPUT_MODE
{
    ADV7180_INPUT_CVBS      = 0,
    ADV7180_INPUT_SVIDEO    = 1,
    ADV7180_INPUT_YPBPR     = 2,

}ADV7180_INPUT_MODE;


typedef enum _ADV7180_INPUT_STANDARD
{
    ADV7180_NTSM_M_J            = 0x0,
    ADV7180_NTSC_4_43           = 0x1,
    ADV7180_PAL_M               = 0x2,
    ADV7180_PAL_60              = 0x3,
    ADV7180_PAL_B_G_H_I_D       = 0x4,
    ADV7180_SECAM               = 0x5,
    ADV7180_PAL_COMBINATION_N   = 0x6,
    ADV7180_SECAM_525           = 0x7,

}ADV7180_INPUT_STANDARD;


MMP_UINT16 ADV7180_InWidth;
MMP_UINT16 ADV7180_InHeight;
MMP_UINT16 ADV7180_InFrameRate;


void Set_ADV7180_Tri_State_Enable();

void Set_ADV7180_Tri_State_Disable();

ADV7180_INPUT_STANDARD Get_Auto_Detection_Result();

void ADV7180Initial(ADV7180_INPUT_MODE mode);

void ADV7180_Input_Mode(ADV7180_INPUT_MODE mode);

MMP_BOOL ADV7180_IsStable();

void ADV7180_PowerDown(
    MMP_BOOL enable);
    
MMP_BOOL ADV7180_IsPowerDown();    

MMP_BOOL ADV7180_IsSVideoInput();
