#include "config.h"
#include "pal/pal.h"
#include "mmp_nor.h"
#include "FreeRTOS.h"
#include "task.h"

MMP_UINT32  enable_DbgMsgFlag = 0x3; // default: enable ERROR
MMP_UINT32  enable_SdkMsgFlag = 0x1;

void
PalAssertFail(
    const MMP_CHAR *exp,
    const MMP_CHAR *file,
    MMP_UINT line)
{
#if 0
    MMP_UINT32 capacity          = 0;
    MMP_UINT32 sectionSize       = 0;
    MMP_UINT32 address           = 0;
    MMP_UINT8  pBuffer[8 * 1024] = { 0 };
#endif
    printf("Failed assertion: %s (in %s, line %u)\r\n", exp, file, line);
#if 0
    capacity    = NorCapacity();
    sectionSize = NorGetAttitude(NOR_ATTITUDE_ERASE_UNIT);
    address     = capacity - sectionSize - 64 * 1024 * 2;
    NorWrite(pBuffer, address, 8 * 1024);

    printf("nor flash erase finished\n");
    PalSleep(100);
#endif

    while (1);
}

MMP_CHAR *
PalGetErrorString(
    MMP_INT errnum)
{
    return MMP_NULL;
}

void
PalExit(
    MMP_INT status)
{
    printf("Program exit with status code: %d\r\n", status);

    taskSOFTWARE_BREAKPOINT();
}