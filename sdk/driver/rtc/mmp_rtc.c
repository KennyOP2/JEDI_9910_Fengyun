#include "sys/sys.h"
#include "host/ahb.h"
#include "mmp_rtc.h"

#if defined(__FREERTOS__) && defined(ENABLE_INTR)
    //#define RTC_IRQ_ENABLE
#endif

#if defined(RTC_IRQ_ENABLE)
#include "mmp_intr.h"
#endif

#ifdef _WIN32

#define CPE_RTC_BASE  0x9000	 	/*   Real Time Clock 		*/
#define CPE_GPIO_BASE 0x7C00
//#define EXTCLK        0x2000000     /* external clk 32Mhz */
//#define EXTCLK        0x1B00000     /* external clk 27Mhz */
#define EXTCLK		0x8000      /* external clk 32khz */
#else

#define CPE_RTC_BASE  0xDE500000	 	/*   Real Time Clock 		*/
#define CPE_GPIO_BASE 0xDE000000

// Should be change base on real case //Steven 2009/12/28
#define EXTCLK        0x8000        /* external clk 32Khz */
//#define EXTCLK        12000000      /* external clk 12Mhz */
//#define EXTCLK        27000000      /* external clk 27Mhz */ //TODO
#endif // _WIN32

 /* registers */
#define RtcSecond					0x0
#define RtcMinute					0x4
#define RtcHour						0x8
#define RtcDays						0xC
#define RtcWeek						0x10
#define AlarmSecond					0x14
#define AlarmMinute					0x18
#define AlarmHour					0x1C
#define AlarmDay					0x20
#define AlarmWeek					0x24
#define RtcRecord					0x28
#define RtcCR						0x2C
#define WRtcSecond					0x30
#define WRtcMinute					0x34
#define WRtcHour					0x38
#define WRtcDays					0x3C
#define WRtcWeek					0x40
#define IntrState					0x44
#define RtcDivide					0x48
#define RtcState					0x4C
#define RtcRevision					0x50


static void
ResetRTC(
    void)
{
	MMP_UINT32 days, hour, min, sec;
	MMP_UINT32 div, cr;
	MMP_UINT16 revisionId;

	// backup date and time
	AHB_ReadRegister(CPE_RTC_BASE + RtcDays, &days);
	AHB_ReadRegister(CPE_RTC_BASE + RtcHour, &hour);
	AHB_ReadRegister(CPE_RTC_BASE + RtcMinute, &min);
	AHB_ReadRegister(CPE_RTC_BASE + RtcSecond, &sec);
	AHB_ReadRegister(CPE_RTC_BASE + RtcDivide, &div);
	AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &cr);

	HOST_ReadRegister(0x04, &revisionId);
	if (revisionId == 0)
	{
		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, cr | (0x1 << 31));
		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, cr);

		div |= (0x1 << 31) /* DividerEnable */ | EXTCLK /* 0x8000:32kHz */;

		AHB_WriteRegister(CPE_RTC_BASE + RtcDivide, div);
	}
	else
	{
		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, cr | (0x1 << 31));
		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, cr);

		div |= (0x1 << 31) | (0x1 << 30) | EXTCLK;
		AHB_WriteRegister(CPE_RTC_BASE + RtcDivide, div);

		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, cr | (0x1 << 31));
		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, cr);
	}

	// restore date and time
	AHB_WriteRegister(CPE_RTC_BASE + WRtcDays, days);
	AHB_WriteRegister(CPE_RTC_BASE + WRtcHour, hour);
	AHB_WriteRegister(CPE_RTC_BASE + WRtcMinute, min);
	AHB_WriteRegister(CPE_RTC_BASE + WRtcSecond, sec);
}

#if defined(RTC_IRQ_ENABLE)
MMP_INT rtc_isr(void* data)
{
    MMP_INT result;
    MMP_UINT32 tmp;
	
	//TODO check intr status
	AHB_ReadRegister(CPE_RTC_BASE + IntrState, &tmp);
    tmp &= ~(MMP_RTC_INTR_EVERY_SEC);
   	AHB_WriteRegister(CPE_RTC_BASE + IntrState, tmp);

    return result;
}

#endif

MMP_RESULT
mmpRtcInitialize(
    void)
{
	MMP_UINT32 tmp = 0, tmp_old = 0;
	MMP_UINT16 revisionId;
	MMP_RESULT result = MMP_RESULT_SUCCESS;

	// get revision id
	HOST_ReadRegister(0x04, &revisionId);

	if (revisionId == 0)
	{
		// reset Rtc first
		//AHB_WriteRegister(CPE_RTC_BASE + RtcCR, 0x1 << 31);
		//AHB_WriteRegister(CPE_RTC_BASE + RtcCR, 0);
		
		ResetRTC();

#if defined(RTC_IRQ_ENABLE)
		//intr
		mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_SEC);
		result = mmpIntrRequestIrq(IRQ_RTC, rtc_isr, (LEVEL_TRIGGER|ACTIVE_HIGH), MMP_NULL);
		if(result)
			goto end;
		result = mmpIntrEnableIrq(IRQ_RTC, MMP_TRUE);
#endif
		mmpRtcEnable();

		while(1)
		{
			AHB_ReadRegister(CPE_RTC_BASE + RtcSecond, &tmp);
			if (tmp_old != tmp) break;
			tmp_old = tmp;
		}

		// switch EXTCLK 12M to 32k
		AHB_ReadRegister(CPE_RTC_BASE + RtcDivide, &tmp);
		tmp |= (0x1 << 30); // DividerSource, 0:12MHz 1:32kHz
		AHB_WriteRegister(CPE_RTC_BASE + RtcDivide, tmp);
	}
	else
	{
		// check initial bits, first boot?
		AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
		if ((tmp & 0xFFFF >> 9 & 0x33) != 0x33)
			ResetRTC();
		else
			return result;

#if defined(RTC_IRQ_ENABLE)
		//intr
		mmpRtcEnableInterrupt(MMP_RTC_INTR_EVERY_SEC);
		result = mmpIntrRequestIrq(IRQ_RTC, rtc_isr, (LEVEL_TRIGGER|ACTIVE_HIGH), MMP_NULL);
		if(result)
			goto end;
		result = mmpIntrEnableIrq(IRQ_RTC, MMP_TRUE);
#endif
		mmpRtcEnable();

		// write initial bits
		AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
		tmp |= 0x6600;
		AHB_WriteRegister(CPE_RTC_BASE + RtcCR, tmp);
	}
end:
	return result;
}

MMP_RESULT
mmpRtcTerminate(
    void)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    mmpRtcDisable();
    mmpRtcDisableInterrupt(MMP_RTC_INTR_EVERY_SEC);
    mmpRtcDisableInterrupt(MMP_RTC_INTR_EVERY_MIN);
    mmpRtcDisableInterrupt(MMP_RTC_INTR_EVERY_HOUR);
    mmpRtcDisableInterrupt(MMP_RTC_INTR_EVERY_DAY);
    mmpRtcDisableInterrupt(MMP_RTC_INTR_ALARM);

#if defined(RTC_IRQ_ENABLE)
   	result = mmpIntrEnableIrq(IRQ_RTC, MMP_FALSE);
#endif

    return result;
}

void
mmpRtcEnable(
    void)
{
    MMP_UINT32 tmp;
	AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
    tmp |= 0x01;  // set bit 0 RTC enable
    AHB_WriteRegister(CPE_RTC_BASE + RtcCR, tmp);
}

void
mmpRtcDisable(
    void)
{
#if defined(RTC_IRQ_ENABLE)
    MMP_UINT32 tmp;

    AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
    tmp &= ~(0x01);  // clear bit 0 RTC enable
    AHB_WriteRegister(CPE_RTC_BASE + RtcCR, tmp);
#else
	AHB_WriteRegister(CPE_RTC_BASE + RtcCR, 0);
#endif
}

void
mmpRtcEnableInterrupt(
    MMP_RTC_INTR_TYPE type)
{
    MMP_UINT32 tmp;

	AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
	tmp |= (0x01 << ((unsigned)type));
	AHB_WriteRegister(CPE_RTC_BASE + RtcCR, tmp);
}

void
mmpRtcDisableInterrupt(
    MMP_RTC_INTR_TYPE type)
{
	unsigned temp;
	MMP_UINT32 tmp;

	// clear bit n, but only 5 bit valid
	temp = 0x01 << ((unsigned) type);
	temp = ~ temp;
	temp &= 0x0000001f; // only left 5 bit

	AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
	tmp &= temp; // clear bit n
	AHB_WriteRegister(CPE_RTC_BASE + RtcCR, tmp);
}

MMP_BOOL
mmpRtcIsInterruptOccured(
    MMP_RTC_INTR_TYPE type)
{
    MMP_BOOL result;
    MMP_UINT32 tmp;

	AHB_ReadRegister(CPE_RTC_BASE + IntrState, &tmp);
    result = (tmp & (0x01 << ((unsigned)type - 1))) ? MMP_TRUE : MMP_FALSE;
    if (result)
    {
        tmp &= ~(0x01 << ((unsigned)type - 1)) & 0x0000001f;
        AHB_WriteRegister(CPE_RTC_BASE + IntrState, tmp);
        AHB_ReadRegister(CPE_RTC_BASE + IntrState, &tmp);
    }

    return result;
}

MMP_RESULT
mmpRtcSetAlarm(
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec)
{
    AHB_WriteRegister(CPE_RTC_BASE + AlarmHour, hour);
    AHB_WriteRegister(CPE_RTC_BASE + AlarmMinute, min);
    AHB_WriteRegister(CPE_RTC_BASE + AlarmSecond, sec);

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpRtcGetAlarm(
    MMP_UINT32* pHour,
    MMP_UINT32* pMin,
    MMP_UINT32* pSec)
{
    // ------------------------------------
    // Check Parameter
    if (   (pHour == MMP_NULL)
        || (pMin  == MMP_NULL)
        || (pSec  == MMP_NULL) )
    {
        return MMP_RESULT_ERROR;
    }

    // ------------------------------------
    // Action
    AHB_ReadRegister(CPE_RTC_BASE + AlarmHour,   pHour);
    AHB_ReadRegister(CPE_RTC_BASE + AlarmMinute, pMin);
    AHB_ReadRegister(CPE_RTC_BASE + AlarmSecond, pSec);

    return MMP_RESULT_SUCCESS;
}

static MMP_UINT g(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    if(month<=2)
       month+=13;
    else
       month+=1;

    return month;
}

static MMP_UINT f(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    if(month<=2)
       year-=1;

    return year;
}

static MMP_UINT setN(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    return 1461 * f(year, month, day) / 4 + 153 * g(year, month, day) / 5 + day;
}

MMP_RESULT
mmpRtcSetDate(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day)
{
    MMP_UINT32 days = setN(year, month, day) - setN(1970, 1, 1);
    MMP_UINT32 tmp;

    mmpRtcDisable();
    ResetRTC();
    AHB_WriteRegister(CPE_RTC_BASE + WRtcDays, days);
    AHB_WriteRegisterMask(CPE_RTC_BASE + RtcCR, 0x1 << 8, 0x1 << 8);
    mmpRtcEnable();

    do
    {
        PalSleep (1);
        AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
    } while (tmp & (0x1 << 8));

    return MMP_RESULT_SUCCESS;
}

static MMP_UINT is_leap(MMP_UINT year)
{
	if (year % 100 == 0)
    {
		return year % 400 == 0;
	}

	return year % 4 == 0;
}

MMP_RESULT
mmpRtcGetDate(
    MMP_UINT* year,
    MMP_UINT* month,
    MMP_UINT* day)
{
    MMP_UINT32 days;
    MMP_UINT days_this_year, y = 1970, m = 0;
    MMP_UINT days_per_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

	AHB_ReadRegister(CPE_RTC_BASE + RtcDays, &days);

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

	return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpRtcSetTime(
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec)
{
    MMP_UINT32 tmp;

    mmpRtcDisable();
    ResetRTC();
    AHB_WriteRegister(CPE_RTC_BASE + WRtcHour, hour);
    AHB_WriteRegister(CPE_RTC_BASE + WRtcMinute, min);
    AHB_WriteRegister(CPE_RTC_BASE + WRtcSecond, sec);
    AHB_WriteRegisterMask(CPE_RTC_BASE + RtcCR, 0x1 << 8, 0x1 << 8);
    mmpRtcEnable();

    do
    {
        PalSleep (1);
        AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
    } while (tmp & (0x1 << 8));

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpRtcSetDateTime(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day,
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec)
{
    MMP_UINT32 days = setN(year, month, day) - setN(1970, 1, 1);
    MMP_UINT32 tmp;

    AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);

    //RTC count loading
    if(tmp & (0x1 << 6))
    {
        return MMP_RESULT_ERROR;
    }
    mmpRtcDisable();
    ResetRTC();
    
    AHB_WriteRegister(CPE_RTC_BASE + WRtcDays, days);
    AHB_WriteRegister(CPE_RTC_BASE + WRtcHour, hour);
    AHB_WriteRegister(CPE_RTC_BASE + WRtcMinute, min);
    AHB_WriteRegister(CPE_RTC_BASE + WRtcSecond, sec);
    AHB_WriteRegisterMask(CPE_RTC_BASE + RtcCR, 0x1 << 8, 0x1 << 8);
    mmpRtcEnable();
    PalSleep (1);

    #if 0
    do
    {
        PalSleep (1);
        AHB_ReadRegister(CPE_RTC_BASE + RtcCR, &tmp);
    } while (tmp & (0x1 << 6));
    #endif
    
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpRtcGetTime(
    MMP_UINT* hour,
    MMP_UINT* min,
    MMP_UINT* sec)
{
    if (hour)
        AHB_ReadRegister(CPE_RTC_BASE + RtcHour, (MMP_UINT32*) hour);

    if (min)
        AHB_ReadRegister(CPE_RTC_BASE + RtcMinute, (MMP_UINT32*) min);

    if (sec)
        AHB_ReadRegister(CPE_RTC_BASE + RtcSecond, (MMP_UINT32*) sec);

    return MMP_RESULT_SUCCESS;
}

RTC_API MMP_RESULT
mmpRtcGetDateTimeOfDay(
    MMP_UINT* day,
    MMP_UINT* sec)
{
    MMP_UINT32 days;

	AHB_ReadRegister(CPE_RTC_BASE + RtcDays, &days);

    if (day)
        *day = days;

    if (sec)
    {
        MMP_UINT32 hour, min, s;

        AHB_ReadRegister(CPE_RTC_BASE + RtcHour, &hour);
        AHB_ReadRegister(CPE_RTC_BASE + RtcMinute, &min);
        AHB_ReadRegister(CPE_RTC_BASE + RtcSecond, &s);

        *sec = days * 24 * 60 * 60 + hour * 60 * 60 + min * 60 + s;
    }
    return MMP_RESULT_SUCCESS;
}

MMP_UINT32
mmpRtcConvert2Days(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day)
{
    MMP_UINT32 days = setN(year, month, day) - setN(1970, 1, 1);
    return days;
}

