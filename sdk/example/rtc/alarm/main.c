/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "pal/pal.h"
#include "mmp_rtc.h"


#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "host/ahb.h"
#endif


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

MMP_RESULT DoTest(void)
{
    MMP_RESULT result;
    MMP_UINT hour, min, sec;
    
    result = mmpRtcInitialize();
    if(result)
    {
        result = -1;
        goto end;    
    }
    result = mmpRtcSetTime(23, 59, 51);
    PalAssert(result == MMP_RESULT_SUCCESS);

    PalSleep(2000);

    result = mmpRtcSetAlarm(0, 0, 2);
    PalAssert(result == MMP_RESULT_SUCCESS);

    mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_SEC);
    mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_MIN);
    mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_HOUR);
    mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_DAY);
    mmpRtcEnableInterrupt(MMP_RTC_INTR_ALARM);
    
    while(1)
    {
        if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_SEC))
        {
            printf("every sec intrrupt occured!\n");
    
            result = mmpRtcGetTime(&hour, &min, &sec);
            PalAssert(result == MMP_RESULT_SUCCESS);
            printf("hour=%d,min=%d,sec=%d\n", hour, min, sec);
        }
    
        if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_MIN))
        {
            printf("every min intrrupt occured!\n");
    
            result = mmpRtcGetTime(&hour, &min, &sec);
            PalAssert(result == MMP_RESULT_SUCCESS);
            printf("hour=%d,min=%d,sec=%d\n", hour, min, sec);
        }
    
        if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_HOUR))
        {
            printf("every hour intrrupt occured!\n");
    
            result = mmpRtcGetTime(&hour, &min, &sec);
            PalAssert(result == MMP_RESULT_SUCCESS);
            printf("hour=%d,min=%d,sec=%d\n", hour, min, sec);
        }
    
        if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_DAY))
        {
            printf("every day intrrupt occured!\n");
    
            result = mmpRtcGetTime(&hour, &min, &sec);
            PalAssert(result == MMP_RESULT_SUCCESS);
            printf("hour=%d,min=%d,sec=%d\n", hour, min, sec);
        }
    
        if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_ALARM))
        {
            printf("every alarm intrrupt occured!\n");
    
            result = mmpRtcGetTime(&hour, &min, &sec);
            PalAssert(result == MMP_RESULT_SUCCESS);
            printf("hour=%d,min=%d,sec=%d\n", hour, min, sec);
        }
          
    }
end:
    return result;
}

int  main(int argc, char** argv)
{
   
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "alarmtest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#endif
    DoTest();
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;
    DoTest();
}
#endif


