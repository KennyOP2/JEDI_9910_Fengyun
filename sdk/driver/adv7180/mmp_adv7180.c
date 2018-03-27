#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"
#include "host/ahb.h"
#include "mmp_iic.h"
#include "sys/sys.h"

#include "adv7180/adv7180.h"
#include "mmp_adv7180.h"

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
static void* gtADV7180Semaphore = MMP_NULL;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Device initialization.
 */
//=============================================================================
MMP_BOOL
mmpADV7180Initialize(
    MMP_BOOL isSVideoMode)
{
    if (gtADV7180Semaphore == MMP_NULL)
        gtADV7180Semaphore = SYS_CreateSemaphore(1, "MMP_ADV7180");

    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);
            
    ADV7180_PowerDown(MMP_FALSE);
    
    if (isSVideoMode)
        ADV7180Initial(ADV7180_INPUT_SVIDEO);
    else
        ADV7180Initial(ADV7180_INPUT_CVBS);
        
    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore);         
}

//=============================================================================
/**
* Device Terminate
*/
//=============================================================================
void
mmpADV7180Terminate(
    void)
{
    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);
            
    ADV7180_PowerDown(MMP_TRUE);

    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore);    

    if (gtADV7180Semaphore)
    {
        SYS_DeleteSemaphore(gtADV7180Semaphore);
        gtADV7180Semaphore = MMP_NULL;
    }
}

//=============================================================================
/**
 * Device Output Pin Tri-State.
 */
//=============================================================================
void
mmpADV7180OutputPinTriState(
    MMP_BOOL flag)
{
    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);

    if (flag == MMP_TRUE)
        Set_ADV7180_Tri_State_Enable();
    else
        Set_ADV7180_Tri_State_Disable();

    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore);
}

//=============================================================================
/**
 * Device Signal State.
 */
//=============================================================================
MMP_BOOL
mmpADV7180IsSignalStable(
    void)
{
    MMP_BOOL isStable;

    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);

    isStable = ADV7180_IsStable();

    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore);

    return isStable;
}

//=============================================================================
/**
* Device property.
*/
//=============================================================================
ADV7180_API MMP_UINT32
mmpADV7180GetProperty(
    MMP_ADV7180_PROPERTY    property)
{
    MMP_UINT32 value;

    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);

    Get_Auto_Detection_Result();

    switch (property)
    {
    case ADV7180_HEIGHT:
            value = ADV7180_InHeight;
        break;
    case ADV7180_WIDTH:
            value = ADV7180_InWidth;
        break;
    case ADV7180_FRAMERATE:
            value = ADV7180_InFrameRate;
        break;
    case ADV7180_IS_INTERLACE:
            value = 1;
        break;
    default:
        break;
    }

    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore);

    return value;
}

//=============================================================================
/**
* Device power down.
*/
//=============================================================================
void
mmpADV7180PowerDown(
    MMP_BOOL enable)
{
    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);
            
    ADV7180_PowerDown(enable);

    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore);     
}

//=============================================================================
/**
* Is SVideo Input for hauppauge.
*/
//=============================================================================
MMP_BOOL
mmpADV7180IsSVideoInput(
    void)
{
    MMP_BOOL isSVideo;
    
    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);
    
    isSVideo = ADV7180_IsSVideoInput();
            
    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore); 
            
    return isSVideo;
}

//=============================================================================
/**
* Device is power down.
*/
//=============================================================================
MMP_BOOL
mmpADV7180IsPowerDown(
    void)
{
    MMP_BOOL isPowerDown;
    
    if (gtADV7180Semaphore)
        SYS_WaitSemaphore(gtADV7180Semaphore);   
        
    isPowerDown = ADV7180_IsPowerDown(); 
    
    if (gtADV7180Semaphore)
        SYS_ReleaseSemaphore(gtADV7180Semaphore); 
            
    return isPowerDown;
}
