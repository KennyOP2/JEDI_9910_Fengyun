/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/sdk/example/rtc/datetime/main.c
 *
 * @author Evan Chang
 * @version 1.0.0
 */


#include "stdio.h"
#if defined(CASTOR3_ALPHA)
#include "ite/ith.h"
#else // !defined(CASTOR3_ALPHA)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#else // !defined(__OPENRTOS__)
#include "host/ahb.h"
#include "pal/pal.h"
#include "mmp_rtc.h"
#if defined(__FREERTOS__)
#include "intr/intr.h"
#endif // defined(__FREERTOS__)
#endif // defined(__OPENRTOS__)
#endif // defined(CASTOR3_ALPHA)


#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif


#if defined(MM9070) || defined(MM9910)
// Interrupt Request (IRQ)
#if defined(__OPENRTOS__) || defined(__FREERTOS__)
#define RTC_IRQ_ENABLE		1
#else // !defined(__OPENRTOS__) && !defined(__FREERTOS__)
#define RTC_IRQ_ENABLE		0
#endif // defined(__OPENRTOS__) || defined(__FREERTOS__)
#endif // defined(MM9070) || defined(MM9910)


// Should be change base on real case //Steven 2009/12/28
#if defined(_WIN32)
//#define EXTCLK        0x2000000     /* external clk 32Mhz */
//#define EXTCLK        0x1B00000    /* external clk 27Mhz */
#define EXTCLK          0x8000          /* external clk 32khz */
#elif defined(__FREERTOS__) || defined(__OPENRTOS__)
#define EXTCLK          0x8000          /* external clk 32Khz */
//#define EXTCLK        12000000       /* external clk 12Mhz */
#endif // _WIN32

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

#if defined(__OPENRTOS__) || defined(CASTOR3_ALPHA)
static unsigned int g(unsigned int year, unsigned int month, unsigned int day)
{
	if(month<=2)
		month+=13;
	else
		month+=1;

	return month;
}

static unsigned int f(unsigned int year, unsigned int month, unsigned int day)
{
	if(month<=2)
		year-=1;

	return year;
}

static unsigned long setN(unsigned int year, unsigned int month, unsigned int day)
{
	return 1461 * f(year, month, day) / 4 + 153 * g(year, month, day) / 5 + day;
}

static unsigned int is_leap(unsigned int year)
{
	if (year % 100 == 0)
	{
		return year % 400 == 0;
	}

	return year % 4 == 0;
}

static void setRtcTime(
		unsigned int year,
		unsigned int month,
		unsigned int day,
		unsigned int hour,
		unsigned int min,
		unsigned int sec)
{
	unsigned long days = setN(year, month, day) - setN(1970, 1, 1);
	days = days*24 + hour;
	days = days*60 + min;
	days = days*60 + sec;

	ithRtcSetTime(days);
}

static void getRtcTime(
		unsigned int* year,
		unsigned int* month,
		unsigned int* day,
		unsigned int* hour,
		unsigned int* min,
		unsigned int* sec)
{
	unsigned long days;
	unsigned int days_this_year, y = 1970, m = 0;
	unsigned int days_per_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

	days = ithRtcGetTime();

	*sec = days % 60;
	days /= 60;

	*min = days % 60;
	days /= 60;

	*hour = days % 24;
	days /= 24;

	while (days >= (days_this_year = is_leap(y) ? 366 : 365))
	{
		days -= days_this_year;
		y++;
	}

	if (is_leap(y))					/* leap year ? */
		days_per_month[1]++;

	while (days >= days_per_month[m])
	{
		days -= days_per_month[m++];
	}

	if (year)
		*year = y;

	if (month)
		*month = m + 1;

	if (day)
		*day = days + 1;
}
#endif // defined(__OPENRTOS__)

#if defined(MM9070) || defined(MM9910)
static void rtc_isr(void* data)
{
#if defined(__OPENRTOS__) || defined(CASTOR3_ALPHA)
	unsigned int year, mon, day, hour, min, sec;
	uint32_t state = ithRtcGetIntrState();

	if (state & (0x1<<ITH_RTC_SEC))
	{
		ithPrintf("[Rtc] every sec intrrupt occured!\n");

		getRtcTime(&year, &mon, &day, &hour, &min, &sec);
		ithPrintf("[Rtc] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
		ithRtcClearIntr(ITH_RTC_SEC);
	}

	if (state & (0x1<<ITH_RTC_MIN))
	{
		ithPrintf("[Rtc] every min intrrupt occured!\n");

		getRtcTime(&year, &mon, &day, &hour, &min, &sec);
		ithPrintf("[Rtc] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
		ithRtcClearIntr(ITH_RTC_MIN);
	}

	if (state & (0x1<<ITH_RTC_HOUR))
	{
		ithPrintf("[Rtc] every hour intrrupt occured!\n");

		getRtcTime(&year, &mon, &day, &hour, &min, &sec);
		ithPrintf("[Rtc] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
		ithRtcClearIntr(ITH_RTC_HOUR);
	}

	if (state & (0x1<<ITH_RTC_DAY))
	{
		ithPrintf("[Rtc] every day intrrupt occured!\n");

		getRtcTime(&year, &mon, &day, &hour, &min, &sec);
		ithPrintf("[Rtc] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
		ithRtcClearIntr(ITH_RTC_DAY);
	}

	if (state & (0x1<<ITH_RTC_ALARM))
	{
		ithPrintf("[Rtc] every alarm intrrupt occured!\n");

		getRtcTime(&year, &mon, &day, &hour, &min, &sec);
		ithPrintf("[Rtc] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
		ithRtcClearIntr(ITH_RTC_ALARM);
	}
#else // !defined(__OPENRTOS__)
	MMP_RESULT result;
	MMP_UINT hour, min, sec;
	MMP_UINT year, month, day;

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
#endif // defined(__OPENRTOS__)
}
#endif // defined(MM9070) || defined(MM9910)

#if defined(CASTOR3_ALPHA)
void* DoTest(void* arg)
#else
void DoTest(void)
#endif
{
#if defined(MM9070) || defined(MM9910)
#if defined(__OPENRTOS__) || defined(CASTOR3_ALPHA)

	/* Basic Test*/

	unsigned int year, mon, day, hour, min, sec;
	int cnt;uint32_t tmp;

	#if defined(WIN32)
	SpiOpen(2);
	#endif

	// test
	/*ithSetRegBitH(ITH_HOST_BASE + ITH_APB_CLK3_REG, 11);

	sleep(5);
	for(cnt=0; cnt<100; cnt++)
	{
	AHB_ReadRegister(0xDE50002C, &tmp);
	printf("[%d] AHB_ReadRegister [0x902C] 0x%08X\n", cnt, tmp);
	tmp = 0;
	if(cnt ==99)
		while(1);
	}
	printf("[Rtc1] Before pre-reset [0x902C] 0x%08X\n", ithReadRegA(ITH_RTC_BASE + ITH_RTC_CR_REG));
	printf("[Rtc2] Before pre-reset [0x902C] 0x%08X\n", ithReadRegA(ITH_RTC_BASE + ITH_RTC_CR_REG));*/

	// do initial and enable
	ithRtcInit(EXTCLK);
	ithRtcEnable();

	// set time
	setRtcTime(2007, 12, 31, 23, 59, 1);

	#if defined(CASTOR3_ALPHA)
	sleep(2);
	#else
	PalSleep(2000);
	#endif

	// get time
	for (cnt=0; cnt<100; cnt++)
	{
		getRtcTime(&year, &mon, &day, &hour, &min, &sec);
		printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
		#if defined(CASTOR3_ALPHA)
		sleep(1);
		#else
		PalSleep(1000);
		#endif
	}

	/* Interrupt Test*/
#if RTC_IRQ_ENABLE
	{
	// Initialize RTC IRQ
	ithIntrDisableIrq(ITH_INTR_RTC);
	ithIntrClearIrq(ITH_INTR_RTC);

	// register handler to IRQ
	ithIntrRegisterHandlerIrq(ITH_INTR_RTC, rtc_isr, NULL);

	// set RTC IRQ to edge trigger
	ithIntrSetTriggerModeIrq(ITH_INTR_RTC, ITH_INTR_EDGE);

	// set RTC IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(ITH_INTR_RTC, ITH_INTR_HIGH_RISING);
	}

	// Enable RTC IRQ
	ithIntrEnableIrq(ITH_INTR_RTC);
#endif // RTC_IRQ_ENABLE

	// set alarm 2 sec
	ithWriteRegA(ITH_RTC_BASE + 0x14 /*AlarmSecond*/, 2);
	ithWriteRegA(ITH_RTC_BASE + 0x18, 1);
	ithWriteRegA(ITH_RTC_BASE + 0x1C, 0);

	// enable every interrupt
	ithRtcCtrlEnable(ITH_RTC_INTR_SEC);
	ithRtcCtrlEnable(ITH_RTC_INTR_MIN);
	ithRtcCtrlEnable(ITH_RTC_INTR_HOUR);
	ithRtcCtrlEnable(ITH_RTC_INTR_DAY);
	ithRtcCtrlEnable(ITH_RTC_ALARM_INTR);

#if !RTC_IRQ_ENABLE
	while(1)
	{
		if (ithRtcGetIntrState() & (0x1<<ITH_RTC_SEC))
		{
			printf("every sec intrrupt occured!\n");

			getRtcTime(&year, &mon, &day, &hour, &min, &sec);
			printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
			ithRtcClearIntr(ITH_RTC_SEC);
		}

		if (ithRtcGetIntrState() & (0x1<<ITH_RTC_MIN))
		{
			printf("every min intrrupt occured!\n");

			getRtcTime(&year, &mon, &day, &hour, &min, &sec);
			printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
			ithRtcClearIntr(ITH_RTC_MIN);
		}

		if (ithRtcGetIntrState() & (0x1<<ITH_RTC_HOUR))
		{
			printf("every hour intrrupt occured!\n");

			getRtcTime(&year, &mon, &day, &hour, &min, &sec);
			printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
			ithRtcClearIntr(ITH_RTC_HOUR);
		}

		if (ithRtcGetIntrState() & (0x1<<ITH_RTC_DAY))
		{
			printf("every day intrrupt occured!\n");

			getRtcTime(&year, &mon, &day, &hour, &min, &sec);
			printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
			ithRtcClearIntr(ITH_RTC_DAY);
		}

		if (ithRtcGetIntrState() & (0x1<<ITH_RTC_ALARM))
		{
			printf("every alarm intrrupt occured!\n");

			getRtcTime(&year, &mon, &day, &hour, &min, &sec);
			printf("year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, mon, day, hour, min, sec);
			ithRtcClearIntr(ITH_RTC_ALARM);
		}
	}
#endif // !RTC_IRQ_ENABLE
#else // !defined(__OPENRTOS__) && !defined(CASTOR3_ALPHA)
	MMP_RESULT result;
	MMP_UINT hour, min, sec;
	MMP_UINT year, month, day;

#if defined(__FREERTOS__) && defined(MM9910)
    PalEnablePrintBuffer(MMP_TRUE,4*1024);
#endif

	// datetime test
	printf("[Rtc] initialize Rtc\n");
	result = mmpRtcInitialize();
	if(result)
	{
		result = -1;
		printf("[Rtc] init error\n");
	}
	result = mmpRtcSetDate(2007, 12, 31);
	PalAssert(result == MMP_RESULT_SUCCESS);

	PalSleep(2000);

	result = mmpRtcSetTime(23, 59, 1);
	PalAssert(result == MMP_RESULT_SUCCESS);

	PalSleep(2000);
	result = mmpRtcGetDate(&year, &month, &day);
	PalAssert(result == MMP_RESULT_SUCCESS);

	printf("[Rtc] year=%d,month=%d,day=%d\n", year, month, day);

	PalAssert(year == 2007);
	PalAssert(month == 12);
	PalAssert(day == 31);

	result = mmpRtcGetTime(&hour, &min, &sec);
	PalAssert(result == MMP_RESULT_SUCCESS);

	printf("[Rtc] hour=%d,min=%d,sec=%d\n", hour, min, sec);

	PalAssert(hour == 23);
	PalAssert(min == 59);
	PalAssert(sec >= 1);

	while(1)
	{
		MMP_RESULT result = mmpRtcGetDate(&year, &month, &day);
		PalAssert(result == MMP_RESULT_SUCCESS);

		result = mmpRtcGetTime(&hour, &min, &sec);
		PalAssert(result == MMP_RESULT_SUCCESS);

		printf("[Rtc] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n", year, month, day, hour, min, sec);
		PalSleep(1000);   
	}

	// alarm test
	result = mmpRtcInitialize();
	if(result)
	{
		result = -1;
		printf("[Rtc] init error\n");
	}
	result = mmpRtcSetTime(23, 59, 51);
	PalAssert(result == MMP_RESULT_SUCCESS);

	PalSleep(2000);

#if RTC_IRQ_ENABLE
	{
	// Initialize RTC IRQ
	ithIntrDisableIrq(ITH_INTR_RTC);
	ithIntrClearIrq(ITH_INTR_RTC);

	// register handler to IRQ
	ithIntrRegisterHandlerIrq(ITH_INTR_RTC, rtc_isr, NULL);

	// set RTC IRQ to edge trigger
	ithIntrSetTriggerModeIrq(ITH_INTR_RTC, ITH_INTR_EDGE);

	// set RTC IRQ to detect rising edge
	ithIntrSetTriggerLevelIrq(ITH_INTR_RTC, ITH_INTR_HIGH_RISING);
	}

	// Enable RTC IRQ
	ithIntrEnableIrq(ITH_INTR_RTC);
#endif // RTC_IRQ_ENABLE

	result = mmpRtcSetAlarm(0, 0, 2);
	PalAssert(result == MMP_RESULT_SUCCESS);

	//mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_SEC);
	mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_MIN);
	//mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_HOUR);
	//mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_DAY);
	//mmpRtcEnableInterrupt(MMP_RTC_INTR_ALARM);

#if !RTC_IRQ_ENABLE
	while(1)
	{
		if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_SEC))
		{
			printf("[Rtc] every sec intrrupt occured!\n");

			result = mmpRtcGetTime(&hour, &min, &sec);
			PalAssert(result == MMP_RESULT_SUCCESS);
			printf("[Rtc] hour=%d,min=%d,sec=%d\n", hour, min, sec);
		}

		if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_MIN))
		{
			printf("[Rtc] every min intrrupt occured!\n");

			result = mmpRtcGetTime(&hour, &min, &sec);
			PalAssert(result == MMP_RESULT_SUCCESS);
			printf("[Rtc] hour=%d,min=%d,sec=%d\n", hour, min, sec);
		}

		if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_HOUR))
		{
			printf("[Rtc] every hour intrrupt occured!\n");

			result = mmpRtcGetTime(&hour, &min, &sec);
			PalAssert(result == MMP_RESULT_SUCCESS);
			printf("[Rtc] hour=%d,min=%d,sec=%d\n", hour, min, sec);
		}

		if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_EVERY_DAY))
		{
			printf("[Rtc] every day intrrupt occured!\n");

			result = mmpRtcGetTime(&hour, &min, &sec);
			PalAssert(result == MMP_RESULT_SUCCESS);
			printf("[Rtc] hour=%d,min=%d,sec=%d\n", hour, min, sec);
		}

		if (mmpRtcIsInterruptOccured(MMP_RTC_INTR_ALARM))
		{
			printf("[Rtc] every alarm intrrupt occured!\n");

			result = mmpRtcGetTime(&hour, &min, &sec);
			PalAssert(result == MMP_RESULT_SUCCESS);
			printf("[Rtc] hour=%d,min=%d,sec=%d\n", hour, min, sec);
		}
	}
#endif // !RTC_IRQ_ENABLE
#endif // defined(__OPENRTOS__) || defined(CASTOR3_ALPHA)
#else // !defined(MM9070) && !defined(MM9910)
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
#endif // defined(MM9070) || defined(MM9910)

	while(1);
}

#define TEST_STACK_SIZE 1024000
int  main(int argc, char** argv)
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
#endif // defined(CASTOR3_ALPHA) && defined(__OPENRTOS__)

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

