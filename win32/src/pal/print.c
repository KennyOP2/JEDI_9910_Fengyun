#include "config.h"
#include "pal/pal.h"
#include <stdarg.h>
#include <stdio.h>

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
