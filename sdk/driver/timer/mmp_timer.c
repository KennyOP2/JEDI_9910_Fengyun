/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Timer driver implementation.
 *
 * @author Sammy Chen
 */
//=============================================================================
//                              Include Files
//=============================================================================
#include "timer/timer_reg.h"
#include "mmp_timer.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================  
MMP_RESULT
mmpTimerReset(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_INT i;

    AHB_WriteRegister(TIMER_BASE_REG + TIMR_INTR_STATE_REG, 0x00777777);
    AHB_WriteRegister(TIMER_BASE_REG + TIMR_INTR_MASK_REG, 0);

    for (i = 0; i  <MMP_TIMER_COUNT; ++i)
    {
        AHB_WriteRegister(TIMER_BASE_REG + i*TIMER_MAP_OFFSET + TIMR_T1COUT_REG, 0);
        AHB_WriteRegister(TIMER_BASE_REG + i*TIMER_MAP_OFFSET + TIMR_T1LOAD_REG, 0);
        AHB_WriteRegister(TIMER_BASE_REG + i*TIMER_MAP_OFFSET + TIMR_T1MAT1_REG, 0);
        AHB_WriteRegister(TIMER_BASE_REG + i*TIMER_MAP_OFFSET + TIMR_T1MAT2_REG, 0);
        AHB_WriteRegister(TIMER_BASE_REG + i*TIMER_REG_OFFSET + TIMR_TM1CR_REG, 0);
    }
    
    return result;
}

MMP_RESULT mmpTimerResetTimer(MMP_TIMER_NUM timer)
{
    MMP_RESULT result = MMP_SUCCESS;

    AHB_WriteRegisterMask(TIMER_BASE_REG + TIMR_INTR_STATE_REG, 0x7 << (timer * 4), 0xF << (timer * 4));
	AHB_WriteRegisterMask(TIMER_BASE_REG + TIMR_INTR_MASK_REG, 0, 0xF << (timer * 4));
    
	AHB_WriteRegister(TIMER_BASE_REG + timer * 0x10 + TIMR_T1COUT_REG, 0);
	AHB_WriteRegister(TIMER_BASE_REG + timer * 0x10 + TIMR_T1LOAD_REG, 0);
	AHB_WriteRegister(TIMER_BASE_REG + timer * 0x10 + TIMR_T1MAT1_REG, 0);
	AHB_WriteRegister(TIMER_BASE_REG + timer * 0x10 + TIMR_T1MAT2_REG, 0);
    AHB_WriteRegister(TIMER_BASE_REG + timer * 0x04 + TIMR_TM1CR_REG, 0);

    return result;
}

MMP_RESULT
mmpTimerCtrlEnable(MMP_TIMER_NUM timer, MMPTIMERCTRL ctrl)
{
    MMP_RESULT result = MMP_SUCCESS;
    
    switch (ctrl)
    {
        case MMP_TIMER_EN:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_ENABLE, TIMER_BIT_ENABLE);
            break;
        case MMP_TIMER_EXTCLK:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_CLK_EXT, TIMER_BIT_CLK_EXT);
            break;
        case MMP_TIMER_UPCOUNT:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_UP_COUNT, TIMER_BIT_UP_COUNT);
            break;
        case MMP_TIMER_ONESHOT:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_ONE_SHOT_STOP, TIMER_BIT_ONE_SHOT_STOP);
            break;
        case MMP_TIMER_PERIODIC:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_PERIODIC_MODE, TIMER_BIT_PERIODIC_MODE);
            break;
        case MMP_TIMER_PWM:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_PWM_ENABLE, TIMER_BIT_PWM_ENABLE);
            break;
    }
    
    return result;
}

MMP_RESULT
mmpTimerCtrlDisable(MMP_TIMER_NUM timer, MMPTIMERCTRL ctrl)
{
    MMP_RESULT result = MMP_SUCCESS;
    
    switch (ctrl)
    {
        case MMP_TIMER_EN:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_DISABLE, TIMER_BIT_ENABLE);
            break;
        case MMP_TIMER_EXTCLK:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_CLK_PCLK, TIMER_BIT_CLK_EXT);
            break;
        case MMP_TIMER_UPCOUNT:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_DOWN_COUNT, TIMER_BIT_UP_COUNT);
            break;
        case MMP_TIMER_ONESHOT:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_WRAPPING, TIMER_BIT_ONE_SHOT_STOP);
            break;
        case MMP_TIMER_PERIODIC:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_FREE_MODE, TIMER_BIT_PERIODIC_MODE);
            break;
        case MMP_TIMER_PWM:
            AHB_WriteRegisterMask(TIMER_BASE_REG + timer*TIMER_REG_OFFSET + TIMR_TM1CR_REG, TIMER_BIT_PWM_DISABLE, TIMER_BIT_PWM_ENABLE);
            break;
    }
    
    return result;
}

MMP_RESULT
mmpTimerSetTimeOut(MMP_TIMER_NUM timer, MMP_INT us)
{
    MMP_INT64 count;

    count = (MMP_INT64)or32_getBusCLK()*us/1000000;
    AHB_WriteRegister(TIMER_BASE_REG + timer*TIMER_MAP_OFFSET + TIMR_T1COUT_REG, count);
    AHB_WriteRegister(TIMER_BASE_REG + timer*TIMER_MAP_OFFSET + TIMR_T1LOAD_REG, count);
}

MMP_UINT32
mmpTimerReadIntr(void)
{
    MMP_UINT32 data = 0;
    AHB_ReadRegister(TIMER_BASE_REG + TIMR_INTR_STATE_REG, &data);
	//clear intr
//    if(data)
//    {
        //TODO 
//        AHB_WriteRegister(TIMER_BASE_REG + TIMR_INTR_STATE_REG, data);
//    }
    return data;
}

MMP_UINT32
mmpTimerReadCounter(MMP_TIMER_NUM timer)
{
    MMP_UINT32 data = 0;
    AHB_ReadRegister(TIMER_BASE_REG + timer*TIMER_MAP_OFFSET + TIMR_T1COUT_REG, &data);
        
    return data;
}

MMP_RESULT
mmpTimerResetCounter(MMP_TIMER_NUM timer)
{   
    AHB_WriteRegister(TIMER_BASE_REG + timer*TIMER_MAP_OFFSET + TIMR_T1COUT_REG, 0);   
}
