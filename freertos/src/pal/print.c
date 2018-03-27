#include "config.h"
#include "pal/pal.h"
#include <stdarg.h>
#include <stdio.h>

#include "common/fat.h"
#include "common/common.h"

void
or32_enPrintBuffer(
    MMP_BOOL   enable,
    MMP_UINT32 size);

void
or32_enUartPrint(
    MMP_BOOL   enable,
    MMP_UINT32 gpio_group,
    MMP_UINT32 gpio_pin,
    MMP_UINT32 baud_rate);

void
PalEnablePrintBuffer(
    MMP_BOOL   enable,
    MMP_UINT32 size)
{
    #if defined(__OR32__)
    or32_enPrintBuffer(enable, size);
    #endif
}

void
PalEnableUartPrint(
    MMP_BOOL   enable,
    MMP_UINT32 gpio_group,
    MMP_UINT32 gpio_pin,
    MMP_UINT32 baud_rate)
{
    #if defined(__OR32__)
    or32_enUartPrint(enable, gpio_group, gpio_pin, baud_rate);
    #endif // __OR32__
}

MMP_INT
PalPrintf(
    const MMP_CHAR* format,
    ...)
{
    va_list ap;
    MMP_CHAR buf[384];
    MMP_INT result;

	va_start(ap, format);
    result = vsprintf(buf, format, ap);
	va_end(ap);

    if (result >= 0)
    {
        printf(buf);
    }
    return result;
}
