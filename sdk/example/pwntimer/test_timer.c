/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/sdk/example/pwntimer/test_timer.c
 *
 * @author Evan Chang
 * @version 1.0.0
 */


#include "stdio.h"
#if defined(CASTOR3_ALPHA)
#if defined(__OPENRTOS__)
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#else // !defined(__OPENRTOS__)
#include <windows.h>
#endif // defined(__OPENRTOS__)
#include "ite/ith.h"
#else // !defined(CASTOR3_ALPHA)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#else // !defined(__OPENRTOS__)
#include "host/ahb.h"
#include "mmp_timer.h"
#include "pal/pal.h"
#include "host/gpio.h"				// PWM, use Gpio
#include "timer/timer_reg.h"
#if defined(__FREERTOS__)
#include "intr/intr.h"
#endif // defined(__FREERTOS__)
#endif // defined(__OPENRTOS__)
#endif // defined(CASTOR3_ALPHA)


#if defined(MM9070) || defined(MM9910)
#if defined(__OPENRTOS__) || defined(__FREERTOS__)
#define TIMER_IRQ_ENABLE        1
#else // !defined(__OPENRTOS__) && !defined(__FREERTOS__)
#define TIMER_IRQ_ENABLE        0
#endif // defined(__OPENRTOS__)
static int clk = 0;
static int cnt = 0;
#endif

#if !defined(CASTOR3_ALPHA) && !defined(__OPENRTOS__)
typedef enum
{
    ITH_TIMER1 = 0,
    ITH_TIMER2 = 1,
    ITH_TIMER3 = 2,
    ITH_TIMER4 = 3,
    ITH_TIMER5 = 4,
    ITH_TIMER6 = 5
} ITHTimer;
#endif


#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif


#if defined(CASTOR3_ALPHA)
#if defined(__OPENRTOS__)
unsigned int PalGetClock(void)
{
    return xTaskGetTickCount();
}

unsigned long PalGetDuration(unsigned int clock)
{
    return (xTaskGetTickCount() - clock) / portTICK_RATE_MS;
}
#else // !defined(__OPENRTOS__)
unsigned int PalGetClock(void)
{
    return timeGetTime();
}

unsigned long PalGetDuration(unsigned int clock)
{
    return (timeGetTime() - clock);
}
#endif // defined(__OPENRTOS__)
#endif // defined(CASTOR3_ALPHA)


void timer_isr(void* data)
{
#if TIMER_IRQ_ENABLE
    static uint32_t cnt = 0;
    uint32_t timer = (uint32_t)data;
    
#if defined(__OPENRTOS__)
    ithPrintf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, PalGetDuration(clk), ithTimerGetIntrState(), ++cnt);
    //printf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, PalGetDuration(clk), ithTimerGetIntrState(), ++cnt);
    clk = PalGetClock();
    ithTimerClearIntr(timer);
#else
    MMP_UINT32 state;
    MMP_UINT32 duration = clk;
    
    clk = PalGetClock();
    duration = clk - duration;
    AHB_ReadRegister(TIMER_BASE + 0x7C, &state);
    printf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, duration, state, ++cnt);    
    AHB_WriteRegisterMask(TIMER_BASE + 0x7C, 0x7 << (timer * 4), 0x7 << (timer * 4));    
#endif
#endif
}

void test_pwmtimer(void)
{
#if defined(__OPENRTOS__) || defined(CASTOR3_ALPHA)
	/*int i;
	uint32_t TmCounter, TmLoad,
		 TmMatch1, TmMatch2,
		 TmCr,
		 gpioPad, gpioPinDir;

	ithBacklightInit(ITH_TIMER1, 100000, 100);
	ithBacklightReset();

	for(i = 1; i <= 10; i++)
	{
		ithBacklightSetDutyCycle(i * 10);

		TmCounter = ithReadRegA(ITH_TIMER_BASE + ITH_TIMER1_CNT_REG);
		TmLoad = ithReadRegA(ITH_TIMER_BASE + ITH_TIMER1_LOAD_REG);
		TmMatch1 = ithReadRegA(ITH_TIMER_BASE + ITH_TIMER1_MATCH1_REG);
		TmMatch2 = ithReadRegA(ITH_TIMER_BASE + ITH_TIMER1_MATCH2_REG);
		TmCr = ithReadRegA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG);
		gpioPad = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_MODE_REG);
		gpioPinDir = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_PINDIR_REG);
                                                                                      
		ithPrintf("GPIO Pad(0x%X): 0x%X\n", ITH_GPIO_BASE + ITH_GPIO1_MODE_REG, gpioPad);       
		ithPrintf("GPIO Pin Dir(0x%X): 0x%X\n", ITH_GPIO_BASE + ITH_GPIO1_PINDIR_REG, gpioPinDir);
		ithPrintf("TmCounter(0x%X): 0x%X\n", ITH_TIMER_BASE + ITH_TIMER1_CNT_REG, TmCounter);   
		ithPrintf("TmLoad(0x%X): 0x%X\n", ITH_TIMER_BASE + ITH_TIMER1_LOAD_REG, TmLoad);         
		ithPrintf("TmMatch1(0x%X): 0x%X\n", ITH_TIMER_BASE + ITH_TIMER1_MATCH1_REG, TmMatch1);     
		ithPrintf("TmMatch2(0x%X): 0x%X\n", ITH_TIMER_BASE + ITH_TIMER1_MATCH2_REG, TmMatch2);
		ithPrintf("TmCr(0x%X): 0x%X\n\n", ITH_TIMER_BASE + ITH_TIMER1_CR_REG, TmCr);

#if defined(WIN32)
		Sleep(5000);
#else
		sleep(5);
#endif
	}

	ithBacklightTurnOff();*/

	uint32_t mode;
	uint32_t value;
	uint32_t mask;

	// Setting Gpio pin 25 to PWM mode, PWM2, Timer3
	mode = 2;
	mask = 0x3;
	value = mode << ((25 - 16) * 2);
	mask = mask << ((25 - 16) * 2);
	ithWriteRegMaskA(ITH_GPIO_BASE + 0x94, value, mask);
	ithTimerReset(ITH_TIMER3);

	// Setting Gpio pin 26 PWM mode, PWM0, Timer1
	mode = 1;
	mask = 0x3;
	value = mode << ((26 - 16) * 2);
	mask = mask << ((26 - 16) * 2);
	ithWriteRegMaskA(ITH_GPIO_BASE + 0x94, value, mask);
	ithTimerReset(ITH_TIMER1);

	// Setting Match1, Match2
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_MATCH1_REG + 0x10 * ITH_TIMER1, 0);
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_MATCH2_REG + 0x10 * ITH_TIMER1, 1);

	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_MATCH1_REG + 0x10 * ITH_TIMER3, 0);
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_MATCH2_REG + 0x10 * ITH_TIMER3, 4);

	// Timer auto reload TIMR_T1LOAD_REG
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_LOAD_REG + 0x10 * ITH_TIMER1, 1);
	ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_LOAD_REG + 0x10 * ITH_TIMER3, 7);

	// Timer down count
	ithTimerCtrlDisable(ITH_TIMER1, ITH_TIMER_UPCOUNT);
	ithTimerCtrlDisable(ITH_TIMER3, ITH_TIMER_UPCOUNT);

	// Timer periodic mode
	//mmpTimerCtrlEnable(MMP_TIMER_1, MMP_TIMER_PERIODIC); //periodic mode will reset when match Match2
	//mmpTimerCtrlEnable(MMP_TIMER_3, MMP_TIMER_PERIODIC);

	// PWM Enable
	ithTimerCtrlEnable(ITH_TIMER1, ITH_TIMER_PWM);
	ithTimerCtrlEnable(ITH_TIMER3, ITH_TIMER_PWM);

	// Timer Enable
	ithTimerCtrlEnable(ITH_TIMER1, ITH_TIMER_EN);
	ithTimerCtrlEnable(ITH_TIMER3, ITH_TIMER_EN);

	while(1);

	// PWM Disable
	ithTimerCtrlDisable(ITH_TIMER1, ITH_TIMER_PWM);
	ithTimerCtrlDisable(ITH_TIMER3, ITH_TIMER_PWM);

	// Timer Disable
	ithTimerCtrlDisable(ITH_TIMER1, ITH_TIMER_EN);
	ithTimerCtrlDisable(ITH_TIMER3, ITH_TIMER_EN);
#else // !defined(__OPENRTOS__) && !defined(CASTOR3_ALPHA)
	MMP_UINT32 mode;
	MMP_UINT32 value;
	MMP_UINT32 mask;

	// Setting Gpio pin 25 to PWM mode, PWM2, Timer3
	mode = 2;
	mask = 0x3;
	value = mode << ((25 - 16) * 2);
	mask = mask << ((25 - 16) * 2);
	AHB_WriteRegisterMask(GPIO_BASE + 0x94, value, mask);
	mmpTimerResetTimer(MMP_TIMER_3);

	// Setting Gpio pin 26 PWM mode, PWM0, Timer1
	mode = 1;
	mask = 0x3;
	value = mode << ((26 - 16) * 2);
	mask = mask << ((26 - 16) * 2);
	AHB_WriteRegisterMask(GPIO_BASE + 0x94, value, mask);
	mmpTimerResetTimer(MMP_TIMER_1);

	// Setting Match1, Match2
	AHB_WriteRegister(TIMER_BASE_REG + TIMR_T1MAT1_REG + 0x10 * MMP_TIMER_1, 0);
	AHB_WriteRegister(TIMER_BASE_REG + TIMR_T1MAT2_REG + 0x10 * MMP_TIMER_1, 1);

	AHB_WriteRegister(TIMER_BASE_REG + TIMR_T1MAT1_REG + 0x10 * MMP_TIMER_3, 0);
	AHB_WriteRegister(TIMER_BASE_REG + TIMR_T1MAT2_REG + 0x10 * MMP_TIMER_3, 4);

	// Timer auto reload TIMR_T1LOAD_REG
	AHB_WriteRegister(TIMER_BASE_REG + TIMR_T1LOAD_REG + 0x10 * MMP_TIMER_1, 1);
	AHB_WriteRegister(TIMER_BASE_REG + TIMR_T1LOAD_REG + 0x10 * MMP_TIMER_3, 7);

	// Timer down count
	mmpTimerCtrlDisable(MMP_TIMER_1, MMP_TIMER_UPCOUNT);
	mmpTimerCtrlDisable(MMP_TIMER_3, MMP_TIMER_UPCOUNT);

	// Timer periodic mode
	//mmpTimerCtrlEnable(MMP_TIMER_1, MMP_TIMER_PERIODIC); //periodic mode will reset when match Match2
	//mmpTimerCtrlEnable(MMP_TIMER_3, MMP_TIMER_PERIODIC);

	// PWM Enable
	mmpTimerCtrlEnable(MMP_TIMER_1, MMP_TIMER_PWM);
	mmpTimerCtrlEnable(MMP_TIMER_3, MMP_TIMER_PWM);

	// Timer Enable
	mmpTimerCtrlEnable(MMP_TIMER_1, MMP_TIMER_EN);
	mmpTimerCtrlEnable(MMP_TIMER_3, MMP_TIMER_EN);

	while(1);

	// PWM Disable
	mmpTimerCtrlDisable(MMP_TIMER_1, MMP_TIMER_PWM);
	mmpTimerCtrlDisable(MMP_TIMER_3, MMP_TIMER_PWM);

	// Timer Disable
	mmpTimerCtrlDisable(MMP_TIMER_1, MMP_TIMER_EN);
	mmpTimerCtrlDisable(MMP_TIMER_3, MMP_TIMER_EN);
#endif
}


#if defined(CASTOR3_ALPHA)
void* DoTest(void* arg)
#else
void DoTest(void)
#endif
{
#if defined(MM9070) || defined(MM9910)
#if defined(__OPENRTOS__) || defined(CASTOR3_ALPHA)
    /* Basic test*/
    uint32_t status;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t us = 0;

    for (us=10000; us<=1000000; us*=10)
    {
        // Interrupt test
        ithPrintf("\n###### %dms timeout testing ######\n", us/1000);

        for(i=ITH_TIMER1; i<=ITH_TIMER6; i++)
        {
			// timer used in
			// 1: backlight
			// 2: no use
			// 3: print_buffer
			// 4: RTC in alpha
			// 5: no use
			// 6: Operation System
            if (/*i == ITH_TIMER1 || i == USEDTIMER || */i == ITH_TIMER6)
                continue;

            ithTimerReset(i);
#if TIMER_IRQ_ENABLE
            {
                // Initialize Timer IRQ
                ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
                ithIntrClearIrq(ITH_INTR_TIMER1 + i);

                #if defined (__OPENRTOS__)
                // register Timer Handler to IRQ
                ithIntrRegisterHandlerIrq(ITH_INTR_TIMER1 + i, timer_isr, (void *)i);
                #endif // defined (__OPENRTOS__)

                // set Timer IRQ to edge trigger
                ithIntrSetTriggerModeIrq(ITH_INTR_TIMER1 + i, ITH_INTR_EDGE);

                // set Timer IRQ to detect rising edge
                ithIntrSetTriggerLevelIrq(ITH_INTR_TIMER1 + i, ITH_INTR_HIGH_RISING);

                // Enable Timer IRQ
                ithIntrEnableIrq(ITH_INTR_TIMER1 + i);
            }
#endif // TIMER_IRQ_ENABLE

            ithTimerSetTimeout(i, us);
            clk = PalGetClock();
            ithTimerEnable(i);

#if !TIMER_IRQ_ENABLE
            j=0;
            //clock = PalGetClock();

            while(j++ < 100)
            {
                status = ithTimerGetIntrState();
                //if(status)
                {
                    printf("[TIMER][%d]intr gap time = %d ms status = 0x%x \n",i+1, PalGetDuration(clk), status);
                    //clock = PalGetClock();

                }
                clk = PalGetClock();
                //PalSleep(1);
            }
#else // TIMER_IRQ_ENABLE
            #if defined(CASTOR3_ALPHA)
            usleep(us*10);
            #else
            PalSleep(us/100); // goal: trigger 10 times
            #endif
            /*while(j<10000000)
            {
                printf("+test ... ");
                PalSleep(1000);
                printf("-test %d\n", j);
                j++;
            }*/
#endif // !TIMER_IRQ_ENABLE

            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithPrintf("###### Timer%d leaving\n", i+1);
            ithTimerDisable(i);
            ithTimerReset(i);

            ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
            ithIntrClearIrq(ITH_INTR_TIMER1 + i);
	    cnt = 0;
        }
    }
#else // !defined(__OPENRTOS__)
    /* Basic test*/
    int status;
    int i = 0;
    int j = 0;
    int us = 0;
    int clock = 0;

#if defined(__FREERTOS__) && defined(MM9910)
    //PalEnablePrintBuffer(MMP_TRUE,4*1024);
#endif
    ithIntrInit();

    for (us=10000; us<=1000000; us*=10)
    {
        // Interrupt test
        printf("\n###### %dms timeout testing ######\n", us/1000);

        for(i=ITH_TIMER1; i<=ITH_TIMER6; i++)
        {
            // timer used in
			// 1: backlight
			// 2: no use
			// 3: print_buffer
			// 4: RTC in alpha
			// 5: no use
			// 6: Operation System
            if (/*i == ITH_TIMER1 || i == USEDTIMER || */i == ITH_TIMER6)
                continue;
                
            mmpTimerResetTimer(i);

#if TIMER_IRQ_ENABLE
            {
                // Initialize Timer IRQ
                ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
                ithIntrClearIrq(ITH_INTR_TIMER1 + i);

                #if defined (__FREERTOS__)
                // register Timer Handler to IRQ
                ithIntrRegisterHandlerIrq(ITH_INTR_TIMER1 + i, timer_isr, (void*)i);
                #endif // defined (__FREERTOS__)

                // set Timer IRQ to edge trigger
                ithIntrSetTriggerModeIrq(ITH_INTR_TIMER1 + i, ITH_INTR_EDGE);

                // set Timer IRQ to detect rising edge
                ithIntrSetTriggerLevelIrq(ITH_INTR_TIMER1 + i, ITH_INTR_HIGH_RISING);

                // Enable Timer IRQ
                ithIntrEnableIrq(ITH_INTR_TIMER1 + i);
            }
#endif // TIMER_IRQ_ENABLE

            mmpTimerSetTimeOut(i, us);
            clk = PalGetClock();
            mmpTimerCtrlEnable(i, MMP_TIMER_EN);

#if !TIMER_IRQ_ENABLE
            j=0;
            clock = PalGetClock();

            while(j++ < 10)
            {
                status = mmpTimerReadIntr();
                if(status)
                {
                    printf("[TIMER][%d]intr gap time = %d ms status = 0x%x \n",i+1, PalGetDuration(clock), status);
                    clock = PalGetClock();

                }
                clock = PalGetClock();
                PalSleep(us/1000); // goal: trigger 10 times
            }
#else
            PalSleep(us/100); // goal: trigger 10 times
#endif // !TIMER_IRQ_ENABLE

            printf("###### Timer%d leaving\n", i+1);
            mmpTimerCtrlDisable(i, MMP_TIMER_EN);
            mmpTimerResetTimer(i);

#if TIMER_IRQ_ENABLE
            ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
            ithIntrClearIrq(ITH_INTR_TIMER1 + i);
#endif // !TIMER_IRQ_ENABLE
        }
    }
#endif // defined(__OPENRTOS__)
#else // !defined(MM9070) && !defined(MM9910)
    int status;
    int i = 0;
    int j = 0;
    int clock = 0;

    mmpTimerReset();
    
    printf("\n###### 1ms timeout testing ######\n");
    //1ms intr
    for(i=0; i<1; ++i)
    {
        mmpTimerSetTimeOut(i, 2000);
        //mmpTimerCtrlEnable(i, MMP_TIMER_EXTCLK); //[Evan, 2011/10/07]
        mmpTimerCtrlEnable(i, MMP_TIMER_EN);

        j=0;
        while(j++ < 100)
        {
            status = mmpTimerReadIntr();
            //if(status)
            {
                printf("[TIMER][%d]intr gap time = %d ms status = 0x%x \n",i+1, PalGetDuration(clock), status);
                //clock = PalGetClock();
                
            }
            clock = PalGetClock();
//          PalSleep(2);
            
        }
        mmpTimerCtrlDisable(i, MMP_TIMER_EN);
    }

    while(1);
    printf("\n###### 10ms timeout testing ######\n");
    //10ms intr
    for(i=0; i<1; ++i)
    {
        mmpTimerSetTimeOut(i, 10000);
        mmpTimerCtrlEnable(i, MMP_TIMER_EN);
        clock = PalGetClock();
        j=0;
        while(j++ < 10)
        {
            status = mmpTimerReadIntr();
            if(status)
            {
                printf("[TIMER][%d]intr gap time = %d ms\n",i+1, PalGetDuration(clock));
                clock = PalGetClock();
            }
            PalSleep(1);
        }
        mmpTimerCtrlDisable(i, MMP_TIMER_EN);
    }

    printf("\n###### 100ms timeout testing ######\n");

    //100ms intr
    for(i=0; i<1; ++i)
    {
        
        mmpTimerSetTimeOut(i, 100000);
        mmpTimerCtrlEnable(i, MMP_TIMER_EN);
        clock = PalGetClock();
        j=0;
        while(j++ < 100)
        {
            status = mmpTimerReadIntr();
            if(status)
            {
                printf("[TIMER][%d]intr gap time = %d ms\n",i+1, PalGetDuration(clock));
                clock = PalGetClock();
            }
            PalSleep(1);
        }
        mmpTimerCtrlDisable(i, MMP_TIMER_EN);
    }

    printf("\n###### 1000ms timout testing ######\n");

    //1s intr
    for(i=0; i<1; ++i)
    {
        mmpTimerSetTimeOut(i, 1000000);
        mmpTimerCtrlEnable(i, MMP_TIMER_EN);
        clock = PalGetClock();
        j=0;
        while(j++ < 1000)
        {
            status = mmpTimerReadIntr();
            if(status)
            {
                printf("[TIMER][%d]intr gap time = %d ms\n",i+1, PalGetDuration(clock));
                clock = PalGetClock();
            }
            PalSleep(10);
        }
        mmpTimerCtrlDisable(i, MMP_TIMER_EN);
    }
#endif // defined(MM9070) || defined(MM9910)

#if defined(CASTOR3_ALPHA)
	test_pwmtimer();
#endif

	while(1);
}

#define TEST_STACK_SIZE 1024000
int main(void)
{
#if defined(CASTOR3_ALPHA) && defined(__OPENRTOS__)
	pthread_t task;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	attr.stacksize = TEST_STACK_SIZE;

	pthread_create(&task, &attr, DoTest, NULL);

	/* Now all the tasks have been started - start the scheduler. */
	vTaskStartScheduler();

	/* Should never reach here! */
	return 0;
#endif

#if defined(__FREERTOS__)
	signed portBASE_TYPE ret = pdFAIL;

	//HOST_GetChipVersionfromReg();

	ret = xTaskCreate(main_task_func, "pwmtest_main",
			configMINIMAL_STACK_SIZE * 2,
			NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
	if (pdFAIL == ret) {
		printf(" ERROR: Failed to create main task!!\n");
		return 1;
	}

	vTaskStartScheduler();
#endif

#if defined(CASTOR3_ALPHA)
	DoTest(NULL);
#else
	DoTest();
#endif
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    DoTest();
}
#endif
