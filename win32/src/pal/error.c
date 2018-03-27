#include "config.h"
#include "pal/pal.h"
#include <windows.h>

void __cdecl _assert(void *, void *, unsigned);

MMP_UINT32  enable_DbgMsgFlag = 0x1;
MMP_UINT32  enable_SdkMsgFlag = 0x1;

void
PalAssertFail(
	const MMP_CHAR* exp,
	const MMP_CHAR* file,
	MMP_UINT line)
{
    _assert((void*) exp, (void*) file, line);
}

MMP_CHAR*
PalGetErrorString(
    MMP_INT errnum)
{
    static TCHAR buf[256];

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errnum,
		0,
        buf,
        256,
        NULL);

    return buf;
}

void
PalExit(
    MMP_INT status)
{
    exit(status);
}
