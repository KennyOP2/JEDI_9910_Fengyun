#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#endif
#include "pal/pal.h"
#include "host/host.h"
#include "host/ahb.h"
#include "sys/sys.h"
#include "mem/mem.h"
#include "pal/thread.h"

//#include "capture/capture.h"
#include "hdmitx/hdmitx.h"
#include "hdmitx/hdmitx_sys.h"


//#include "it6607/it6607_main.h"
//#include "it6607/it6607.h"

//#include "intr/intr.h"


#include "pal/timer.h"
#include "mmp_types.h"


#include "mmp_iic.h"

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

MMP_RESULT DoTest(void)
{
    MMP_RESULT result = 0;
    PAL_CLOCK_T			hdmirxClock    	= 0;


    //ithIntrInit();

    /* ======= initial ======== */
    result = mmpIicInitialize(0, 0,0,0,0,0, 0);
    if(result)
    {
        result = -1;
        //goto end;
        printf("IIC init error\n");
        while(1);
    }

    HDMITX_ChangeDisplayOption(HDMI_480p60, HDMI_RGB444) ;
    printf("HDMITX init start\n");
    InitHDMITX();
    //printf("it6607 init end\n");
    //mmpHdmiRxinit();

    //Cap_Init();

    for (;;)
    {

        if (PalGetDuration(hdmirxClock) > 20)
		{
			HDMITX_DevLoopProc();
			//IT6607_LoopProcess();
			hdmirxClock = PalGetClock();

		}
    }





    return result;
}

int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "HDMITX_test_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }
    vTaskStartScheduler();
#else
    DoTest();
#endif
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;

    result = DoTest();
}
#endif

