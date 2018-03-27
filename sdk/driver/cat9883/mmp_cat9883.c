#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"
#include "host/ahb.h"
#include "mmp_iic.h"
#include "sys/sys.h"

#include "cat9883/cat9883_adc.h"
#include "mmp_cat9883.h"
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
static void* gtCAT9883Semaphore = MMP_NULL;

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
mmpCAT9883Initialize(
    void)
{
    if (gtCAT9883Semaphore == MMP_NULL)
        gtCAT9883Semaphore = SYS_CreateSemaphore(1, "MMP_CAT9883");

    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);
            
    CAT9883_PowerDown(MMP_FALSE);
    CAT9883CInitial();

    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);     
}

//=============================================================================
/**
* Device Terminate
*/
//=============================================================================
void
mmpCAT9883Terminate(
    void)
{
    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);
            
    CAT9883_PowerDown(MMP_TRUE);
    
    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);    
        
    if (gtCAT9883Semaphore)
    {
        SYS_DeleteSemaphore(gtCAT9883Semaphore);
        gtCAT9883Semaphore = MMP_NULL;
    }        
}

//=============================================================================
/**
 * Device Output Pin Tri-State.
 */
//=============================================================================
void
mmpCAT9883OutputPinTriState(
    MMP_BOOL flag)
{
    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);

    if (flag == MMP_TRUE)
        Set_CAT9883_Tri_State_Enable();
    else
        Set_CAT9883_Tri_State_Disable();

    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);
}

//=============================================================================
/**
 * Device Signal State.
 */
//=============================================================================
MMP_BOOL
mmpCAT9883IsSignalStable(
    MMP_BOOL isFrameRateCheck)
{
    MMP_BOOL value;
    MMP_BOOL stable;

    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);

    stable = Mode_stable(isFrameRateCheck);

    if(!stable)
    {
        if (isFrameRateCheck)
            mmpCapTurnOnClock(MMP_FALSE);

        ADC_Mode_Change();

        if (isFrameRateCheck)
            mmpCapTurnOnClock(MMP_TRUE);

        value = MMP_FALSE;
    }
    else
        value = MMP_TRUE;

    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);

    return value;
}

//=============================================================================
/**
* Device property.
*/
//=============================================================================
CAT9883_API MMP_UINT32
mmpCAT9883GetProperty(
    MMP_CAT9883_PROPERTY    property)
{
    MMP_UINT32 value;

    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);

    switch (property)
    {
    case CAT9883_HEIGHT:
        value = CAT9883_InHeight;
        break;
    case CAT9883_WIDTH:
        value = CAT9883_InWidth;
        break;
    case CAT9883_FRAMERATE:
        value = CAT9883_InFrameRate;
        break;
    case CAT9883_IS_INTERLACE:
        value = CAT9883_InIsInterlace;
        break;
    case CAT9883_IS_TV_MODE:
        value = CAT9883_InIsTVMode;
    default:
        break;
    }

    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);

    return value;
}

//=============================================================================
/**
* Device power down.
*/
//=============================================================================
MMP_BOOL
mmpCAT9883PowerDown(
    MMP_BOOL enable)
{
    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);
            
    CAT9883_PowerDown(enable);
    
    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);     
}

//=============================================================================
/**
* Is NTSC or PAL input for hauppauge
*/
//=============================================================================
MMP_BOOL
mmpCAT9883IsNTSCorPAL(
    void)
{
    MMP_BOOL isNTSCorPAL;
    
    if (gtCAT9883Semaphore)
        SYS_WaitSemaphore(gtCAT9883Semaphore);
        
    isNTSCorPAL = CAT9883_IsNTSCorPAL();

    if (gtCAT9883Semaphore)
        SYS_ReleaseSemaphore(gtCAT9883Semaphore);   
                    
    return isNTSCorPAL;
}

