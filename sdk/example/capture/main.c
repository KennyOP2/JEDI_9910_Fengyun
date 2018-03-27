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

#include "it6607/it6607.h"
#include "pal/timer.h"
#include "mmp_types.h"
#include "mmp_isp.h"



#include "mmp_iic.h"
#include "mmp_capture.h"
#include "mmp_it6607.h"

#if 0
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

MMP_RESULT DoTest(void)
{
    MMP_RESULT result = 0;
    PAL_CLOCK_T			hdmirxClock    	= 0;
    MMP_ISP_SEQUENCE_SHARE *ispctxt = MMP_NULL;
    MMP_CAP_SHARE *capctxt = MMP_NULL;
    MMP_UINT16 data;
    MMP_UINT16 Width, Height;
    static MMP_BOOL Cap_Fired = MMP_FALSE;
    MMP_CAP_ERROR_CODE errcode;
    static MMP_BOOL bSignal;
    MMP_BOOL bChangeMode, bOldsignal;
    MMP_INT Cap_Fire_Flag = 0;

    printf("### Do Test Start ###\n");


    ithIntrInit();

    /* ======= initial ======== */
#if 1
    result = mmpIicInitialize(0, 0,0,0,0,0, 0);
    if(result)
    {
        result = -1;
        //goto end;
        printf("IIC init error\n");
    }
#endif
    mmpIT6607Initialize();
    //mmpHdmiRxinit();

    //Cap_Init();
    capctxt = calloc (1, sizeof (MMP_CAP_SHARE));

    capctxt->In_Width = 720;
    capctxt->In_Height = 480;
    capctxt->In_PitchY = 2048;
    capctxt->In_PitchUV = 2048;

    printf("======= Capture init =========\n");
    result = mmpCapInitialize();
    if (result != MMP_SUCCESS)
    {
        printf("mmp cap initialize fail\n");
    }

    result = mmpCapFunEnable(MMP_CAP_DEMODE | MMP_CAP_CSFUN);


    result = mmpCapProcess(capctxt);

#if 1  //ISP Init


        //for sequence process
        //ispctxt->EnCapOnflyMode = MMP_TRUE;
        //ispctxt->EnOnflyInFieldMode = MMP_FALSE;

        mmpIspInitialize();
        //mmpIspEnable(MMP_ISP_REMAP_ADDRESS);

#endif


    for (;;)
        {
        data = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);

        errcode = (data&0x0F00) >> 8;
		if (errcode)
        {

            printf("!!!! Capture Error code = 0x%x !!!!\n", errcode);
	        mmpCapReset();
	        Cap_Fire_Flag = 0;
        }
		if (mmpIsCapFire() && data&0x000F)
        {
            printf("!!!! Capture fire but capture idle !!!!!\n");
            mmpCapReset();
            Cap_Fire_Flag = 0;
        }

		if(mmpIT6607LoopProcess())
        {
		    if (Cap_Fire_Flag == 0)
        {
		        mmpCapGetSignalInfo();
		        Cap_Fire_Flag = 1;
        }
    }
		else
            Cap_Fire_Flag = 0;

		MMP_Sleep(100);


    }





  end :
    if (capctxt)
    free(capctxt);

    if (ispctxt)
    free(ispctxt);

    return result;
}

int  main(int argc, char** argv)
{
#if 0
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "capture_test_main",
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

#if 0
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;

    result = DoTest();
}
#endif
