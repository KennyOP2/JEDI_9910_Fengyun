#include "sys/sys.h"
#include "host/ahb.h"
#include "mmp_exrtc.h"
#include "mmp_iic.h"

#define EXRTCSEC         0x0
#define EXRTCMIN         0x1
#define EXRTCHOUR        0x2
#define EXRTCWEEK        0x3
#define EXRTCDATE        0x4
#define EXRTCMONTH       0x5
#define EXRTCYEAR        0x6
#define EXRTCALARM1SEC   0x7
#define EXRTCALARM1MIN   0x8
#define EXRTCALARM1HOURS 0x9
#define EXRTCALARM1DAYS  0xA
#define EXRTCALARM2MIN   0xB
#define EXRTCALARM2HOURS 0xC
#define EXRTCALARM2DAYS  0xD

#define TWENTYFOUR_HR    (0 << 6)
#define TWELVE_HR        (1 << 6)

static BYTE ExRtc_IICADDR = 0xD0 >> 1;

BYTE
mmpExRtcIICRead(BYTE RegAddr)
{
    BYTE     d;
    MMP_BOOL flag;

    mmpIicLockModule();
    if (0 != (flag = mmpIicReceiveData(IIC_MASTER_MODE, ExRtc_IICADDR, RegAddr, &d, 1)))
    {
        printf("External RTC I2C Read Fail !!! , reg = %02x\n", RegAddr);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return d;
}

MMP_RESULT
mmpExRtcIICWrite(BYTE RegAddr, BYTE d)
{
    MMP_BOOL flag;
    mmpIicLockModule();
    if (0 != (flag = mmpIicSendData(IIC_MASTER_MODE, ExRtc_IICADDR, RegAddr, &d, 1)))
    {
        printf("External RTC I2C Write error, reg = %02x val =%02x\n", RegAddr, d);
        mmpIicGenStop();
    }
    mmpIicReleaseModule();
    return flag;
}

static MMP_UINT g(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    if (month <= 2)
        month += 13;
    else
        month += 1;

    return month;
}

static MMP_UINT f(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    if (month <= 2)
        year -= 1;

    return year;
}

static MMP_UINT setN(MMP_UINT year, MMP_UINT month, MMP_UINT day)
{
    return 1461 * f(year, month, day) / 4 + 153 * g(year, month, day) / 5 + day;
}

static MMP_UINT is_leap(MMP_UINT year)
{
    if (year % 100 == 0)
    {
        return year % 400 == 0;
    }

    return year % 4 == 0;
}

static MMP_UINT ExRtcBcdCovertoDec(BYTE BCD)
{
    MMP_UINT Dec = 0;

    if (BCD & 0x40)
        Dec += 40;

    if (BCD & 0x20)
        Dec += 20;

    if (BCD & 0x10)
        Dec += 10;

    if (BCD & 0x08)
        Dec += 8;

    if (BCD & 0x04)
        Dec += 4;

    if (BCD & 0x02)
        Dec += 2;

    if (BCD & 0x1)
        Dec += 1;

    return Dec;
}

static BYTE ExRtcDecConvertoBcd(MMP_UINT dec)
{
    BYTE bcd = 0;

    if (dec / 80)
    {
        bcd |= (1 << 7);
        dec -= 80;
    }
    if (dec / 40)
    {
        bcd |= (1 << 6);
        dec -= 40;
    }
    if (dec / 20)
    {
        bcd |= (1 << 5);
        dec -= 20;
    }
    if (dec / 10)
    {
        bcd |= (1 << 4);
        dec -= 10;
    }
    if (dec / 8)
    {
        bcd |= (1 << 3);
        dec -= 8;
    }
    if (dec / 4)
    {
        bcd |= (1 << 2);
        dec -= 4;
    }
    if (dec / 2)
    {
        bcd |= (1 << 1);
        dec -= 2;
    }
    if (dec / 1)
    {
        bcd |= (1 << 0);
        dec -= 1;
    }

    return bcd;
}

MMP_RESULT
mmpExRtcInitialize(
    void)
{
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcTerminate(
    void)
{
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcGetTime(
    BYTE *hour,
    BYTE *min,
    BYTE *sec)
{
    if (hour)
        *hour = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCHOUR));

    if (min)
        *min = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCMIN));

    if (sec)
        *sec = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCSEC));

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcSetTime(
    BYTE hour,
    BYTE min,
    BYTE sec)
{
    mmpExRtcIICWrite(EXRTCHOUR, ExRtcDecConvertoBcd(hour));
    mmpExRtcIICWrite(EXRTCMIN,  ExRtcDecConvertoBcd(min));
    mmpExRtcIICWrite(EXRTCSEC, ExRtcDecConvertoBcd(sec));
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcSetDate(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day)
{
    if (year >= 2000)
        year -= 2000;

    mmpExRtcIICWrite(EXRTCYEAR, ExRtcDecConvertoBcd(year));
    mmpExRtcIICWrite(EXRTCMONTH, ExRtcDecConvertoBcd(month));
    mmpExRtcIICWrite(EXRTCDATE, ExRtcDecConvertoBcd(day));

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcGetDate(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day)
{
    BYTE m;

    *year = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCYEAR));

    m     = mmpExRtcIICRead(EXRTCMONTH);
    if (m & 0x80)
        *year += 100;

    *year += 2000;

    *month = ExRtcBcdCovertoDec(m);

    *day   = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCDATE));

    //printf("year=%d, month=%d, day=%d\n", *year, *month, *day);
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcSetDateTime(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day,
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec)
{
    if (year >= 2000)
        year -= 2000;

    mmpExRtcIICWrite(EXRTCYEAR, ExRtcDecConvertoBcd(year));
    mmpExRtcIICWrite(EXRTCMONTH, ExRtcDecConvertoBcd(month));
    mmpExRtcIICWrite(EXRTCDATE, ExRtcDecConvertoBcd(day));
    mmpExRtcIICWrite(EXRTCHOUR, ExRtcDecConvertoBcd(hour));
    mmpExRtcIICWrite(EXRTCMIN,  ExRtcDecConvertoBcd(min));
    mmpExRtcIICWrite(EXRTCSEC,  ExRtcDecConvertoBcd(sec));

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcGetDateTime(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day,
    MMP_UINT *hour,
    MMP_UINT *min,
    MMP_UINT *sec)
{
    BYTE m;

    *year = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCYEAR));

    m     = mmpExRtcIICRead(EXRTCMONTH);
    if (m & 0x80)
        *year += 100;

    *year += 2000;
    *month = ExRtcBcdCovertoDec(m);
    *day   = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCDATE));

    if (hour)
        *hour = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCHOUR));

    if (min)
        *min = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCMIN));

    if (sec)
        *sec = ExRtcBcdCovertoDec(mmpExRtcIICRead(EXRTCSEC));

    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
mmpExRtcSetAlarm(
    BYTE hour,
    BYTE min,
    BYTE sec)
{
    /* TODO*/
}

MMP_RESULT
mmpExRtcGetAlarm(
    BYTE *pHour,
    BYTE *pMin,
    BYTE *pSec)
{
    /* TODO*/
}