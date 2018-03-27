/****************************************************************************
 *
 *            Copyright (c) 2003-2007 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 110
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#include "fat.h"
#include "port_f.h"
#ifdef __FREERTOS__
    #include "FreeRTOS.h"
    #include "task.h"
#endif // __FREERTOS__

#include "sys/sys.h"
#include "mmp_rtc.h"
#include "mmp_exrtc.h"
//static FN_MUTEX_TYPE theMutex;
//static int mutexCnt = 0;

/****************************************************************************
 *
 * f_getrand
 *
 * This function should be ported. It has to return a different 32bit
 * random number whenever it is called. Random number generator could be
 * get from system time, this algorithm below is just a simple random
 * number generator
 *
 * INPUTS
 *
 * rand - a number which could be used for random number generator
 *
 * RETURNS
 *
 * 32bit random number
 *
 ***************************************************************************/

static unsigned long dwrand = 0x729a8fb3;

unsigned long f_getrand(unsigned long rand)
{
    long a;

    dwrand ^= f_gettime();

    for (a = 0; a < 32; a++)
    {
        if (rand & 1)
        {
            dwrand ^= 0x34098bc2;
        }
        if (dwrand & 0x8000000)
        {
            dwrand <<= 1;
            dwrand  |= 1;
        }
        else
            dwrand <<= 1;
        rand >>= 1;
    }

    return dwrand;
}

/****************************************************************************
 *
 * f_getdate
 *
 * need to be ported depending on system, it retreives the
 * current date in DOS format
 *
 * RETURNS
 *
 * current date
 *
 ***************************************************************************/

unsigned short f_getdate(void)
{
    unsigned short   year   = 1980;
    unsigned short   month  = 1;
    unsigned short   day    = 1;
    MMP_INT          result = 0;

    //kenny patch RTC setting from USB
    extern MMP_UINT  gYear;
    extern MMP_UINT  gMonth;
    extern MMP_UINT  gDay;
    extern MMP_UINT8 gHour;
    extern MMP_UINT8 gMin;
    extern MMP_UINT8 gSec;

#if !defined(DISABLE_RTC)
    #ifdef __FREERTOS__
    MMP_UINT year1  = 1980;
    MMP_UINT month1 = 1;
    MMP_UINT day1   = 1;

        #ifdef EXTERNAL_RTC
    result = mmpExRtcGetDate(&year1, &month1, &day1);
    year   = (unsigned short)year1;
    month  = (unsigned short)month1;
    day    = (unsigned short)day1;
        #else
    //  result = mmpRtcGetDate(&year1,&month1,&day1);
    year   = (unsigned short)gYear;
    month  = (unsigned short)gMonth;
    day    = (unsigned short)gDay;
    //printf("year = %d,month = %d,day = %d\r\n",year,month,day);
        #endif
    #endif
#endif

    unsigned short pcdate = (unsigned short)((((year - 1980) << F_CDATE_YEAR_SHIFT) & F_CDATE_YEAR_MASK) |
                                             ((month << F_CDATE_MONTH_SHIFT) & F_CDATE_MONTH_MASK) |
                                             ((day << F_CDATE_DAY_SHIFT) & F_CDATE_DAY_MASK));
    return pcdate;
}

/****************************************************************************
 *
 * f_gettime
 *
 * need to be ported depending on system, it retreives the
 * current time in DOS format
 *
 * RETURNS
 *
 * current time
 *
 ***************************************************************************/

unsigned short f_gettime(void)
{
    unsigned short hour   = 12;
    unsigned short min    = 0;
    unsigned short sec    = 0;
    MMP_INT        result = 0;

#if !defined(DISABLE_RTC)
    #ifdef __FREERTOS__
    MMP_UINT         hour1 = 12;
    MMP_UINT         min1  = 0;
    MMP_UINT         sec1  = 0;
    unsigned char    h     = 0, m = 0, s = 0;

    //kenny patch RTC setting from USB
    extern MMP_UINT  gYear;
    extern MMP_UINT  gMonth;
    extern MMP_UINT  gDay;
    extern MMP_UINT8 gHour;
    extern MMP_UINT8 gMin;
    extern MMP_UINT8 gSec;

        #ifdef EXTERNAL_RTC
    result = mmpExRtcGetTime(&h, &m, &s);
    hour   = (unsigned short)h;
    min    = (unsigned short)m;
    sec    = (unsigned short)s;
        #else
    // result = mmpRtcGetTime(&hour1,&min1,&sec1);
    hour   = (unsigned short)gHour;
    min    = (unsigned short)gMin;
    sec    = (unsigned short)gSec;
    //printf("hour = %d,min = %d,sec = %d\r\n",hour,min,sec);
        #endif
    #endif
#endif

    unsigned short pctime = (unsigned short)(((hour << F_CTIME_HOUR_SHIFT) & F_CTIME_HOUR_MASK) |
                                             ((min << F_CTIME_MIN_SHIFT) & F_CTIME_MIN_MASK) |
                                             (((sec >> 1) << F_CTIME_SEC_SHIFT) & F_CTIME_SEC_MASK));
    return pctime;
}

/****************************************************************************
 *
 * f_mutex_create
 *
 * user function to create a mutex.
 *
 * RETURNS
 *   0 - success
 *   1 - error
 *
 ***************************************************************************/

#if (!FN_CAPI_USED)
int f_mutex_create(FN_MUTEX_TYPE *mutex)
{
    #ifdef WIN32
    const MMP_WCHAR *dpfSemaphoreName = L"DTV";
    #else
    void            *dpfSemaphoreName = MMP_NULL;
    #endif

    /*if(mutexCnt == 0)
       {
       theMutex = SYS_CreateSemaphore(1,dpfSemaphoreName);
       }
       mutexCnt++;

     * mutex=theMutex;*/

    *mutex = (FN_MUTEX_TYPE)SYS_CreateSemaphore(1, dpfSemaphoreName);
    return (*mutex) ? 0 : 1;
}
#endif

/****************************************************************************
 *
 * f_mutex_delete
 *
 * user function to delete a mutex.
 *
 * RETURNS
 *   0 - success
 *   1 - error
 *
 ***************************************************************************/

#if (!FN_CAPI_USED)
int f_mutex_delete(FN_MUTEX_TYPE *mutex)
{
    /* mutexCnt--;
       if(mutexCnt == 0)
       {
       SYS_DeleteSemaphore(theMutex);
       }
       mutex = MMP_NULL;*/

    SYS_DeleteSemaphore((void *) *mutex);
    return 0;
}
#endif

/****************************************************************************
 *
 * f_mutex_get
 *
 * user function to get a mutex.
 *
 * RETURNS
 *   0 - success
 *   1 - error
 *
 ***************************************************************************/

#if (!FN_CAPI_USED)
int f_mutex_get(FN_MUTEX_TYPE *mutex)
{
    SYS_WaitSemaphore((void *) *mutex);
    return 0;
}
#endif

/****************************************************************************
 *
 * f_mutex_put
 *
 * user function to release a mutex.
 *
 * RETURNS
 *   0 - success
 *   1 - error
 *
 ***************************************************************************/

#if (!FN_CAPI_USED)
int f_mutex_put(FN_MUTEX_TYPE *mutex)
{
    SYS_ReleaseSemaphore((void *) *mutex);
    return 0;
}
#endif

/****************************************************************************
 *
 * fn_gettaskID
 *
 * user function to get current task ID, valid return value must be get
 * from the current running task if its a multitask system, another case
 * this function can always returns with 1. Return value zero is not a valid
 * value.
 *
 * RETURNS
 *   task ID
 *
 ***************************************************************************/

#if (!FN_CAPI_USED)
    #ifndef _FN_GETTASKID_
long fn_gettaskID(void)
{
        #ifdef __FREERTOS__
    return (long) xTaskGetCurrentTaskHandle(); /* any value except 0 */
        #else
    return (long) SYS_GetTaskThread();         /* any value except 0 */
        #endif
}
    #endif
#endif

/****************************************************************************
 *
 * end of port_f.c
 *
 ***************************************************************************/