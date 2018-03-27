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
    MMP_UINT year, month, day, hour, min, sec;
    
    result = mmpRtcInitialize();
    if(result)
    {
        result = -1;
        goto end;    
    }
    result = mmpRtcSetDate(2007, 12, 31);
    PalAssert(result == MMP_RESULT_SUCCESS);

    PalSleep(2000);

    result = mmpRtcSetTime(23, 59, 1);
    PalAssert(result == MMP_RESULT_SUCCESS);

    PalSleep(2000);
    result = mmpRtcGetDate(&year, &month, &day);
    PalAssert(result == MMP_RESULT_SUCCESS);

    printf("year=%d,month=%d,day=%d\n", year, month, day);

    PalAssert(year == 2007);
    PalAssert(month == 12);
    PalAssert(day == 31);

    result = mmpRtcGetTime(&hour, &min, &sec);
    PalAssert(result == MMP_RESULT_SUCCESS);

    printf("hour=%d,min=%d,sec=%d\n", hour, min, sec);

    PalAssert(hour == 23);
    PalAssert(min == 59);
    PalAssert(sec >= 1);
    
    while(1)
    {
        MMP_RESULT result = mmpRtcGetDate(&year, &month, &day);
        PalAssert(result == MMP_RESULT_SUCCESS);

        result = mmpRtcGetTime(&hour, &min, &sec);
        PalAssert(result == MMP_RESULT_SUCCESS);

        printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, month, day, hour, min, sec);
        PalSleep(1000);   
    }
end:
    return result;
}

int  main(int argc, char** argv)
{
    
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "datetimetest_main",
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

