#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
    #if defined(RTC_EXPORTS)
        #define RTC_API __declspec(dllexport)
    #else
        #define RTC_API __declspec(dllimport)
    #endif
#else
    #define RTC_API     extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */
//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

typedef unsigned char BYTE;

RTC_API BYTE
mmpExRtcIICRead(BYTE RegAddr);

RTC_API MMP_RESULT
mmpExRtcIICWrite(BYTE RegAddr, BYTE d);

RTC_API MMP_RESULT
mmpExRtcInitialize(
    void);

RTC_API MMP_RESULT
mmpExRtcTerminate(
    void);

RTC_API MMP_RESULT
mmpExRtcGetTime(
    BYTE *hour,
    BYTE *min,
    BYTE *sec);

RTC_API MMP_RESULT
mmpExRtcSetTime(
    BYTE hour,
    BYTE min,
    BYTE sec);

RTC_API MMP_RESULT
mmpExRtcSetDate(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day);

RTC_API MMP_RESULT
mmpExRtcGetDate(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day);

RTC_API MMP_RESULT
mmpExRtcSetDateTime(
    MMP_UINT year,
    MMP_UINT month,
    MMP_UINT day,
    MMP_UINT hour,
    MMP_UINT min,
    MMP_UINT sec);

RTC_API MMP_RESULT
mmpExRtcGetDateTime(
    MMP_UINT *year,
    MMP_UINT *month,
    MMP_UINT *day,
    MMP_UINT *hour,
    MMP_UINT *min,
    MMP_UINT *sec);

RTC_API MMP_RESULT
mmpExRtcSetAlarm(
    BYTE hour,
    BYTE min,
    BYTE sec);

RTC_API MMP_RESULT
mmpExRtcGetAlarm(
    BYTE *pHour,
    BYTE *pMin,
    BYTE *pSec);