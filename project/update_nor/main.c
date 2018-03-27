#include "mmp_types.h"
//#include "freertos.h"
//#include "task.h"
#include "pal\thread.h"
#include "mmp_usbex.h"
#include "core_interface.h"
#include "pal/pal.h"


#define ROM_OFFSET (2 * 1024 * 1024)
#define NOR_COMPARE_DATA_ADDR (16 * 1024 * 1024)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
static MMP_INT
Initialize(
    void)
{
    MMP_INT result = 0;
    MMP_UINT32 romSize = 0;
    MMP_UINT8* pRomAddr = (MMP_UINT8*) ROM_OFFSET;
    MMP_UINT8* pReadBuffer = MMP_NULL;
    MMP_UINT32 checkClock = 0;
    MMP_UINT16          chipPacketType = 0;

    HOST_ReadRegister(0x0000, &chipPacketType);
#if 0
    chipPacketType = (chipPacketType >> 12);

#if defined (IT9917_176TQFP)
    if (chipPacketType != 0x5)
    {
        result = 1;
        PalAssert("Chip packet type error (config = IT9917_176TQFP)\n");
        goto end;        
    }
#elif defined (IT9913_128LQFP)
    if (chipPacketType != 0x0)
    {
        result = 1;
        PalAssert("Chip packet type error (config = IT9913_128LQFP)\n");
        goto end;        
    }
#elif defined (IT9919_144TQFP)
    if (chipPacketType != 0x4 && chipPacketType != 0x2)
    {
        result = 1;
        PalAssert("Chip packet type error (config = IT9919_144TQFP)\n");
        goto end;        
    }
#endif 
#endif
    printf("SDK Version: %d.%d\n", SDK_MAJOR_VERSION, SDK_MINOR_VERSION);
    // wait for creating tasks, may reduce or remove later
    ithIntrInit();
    mmpDmaInitialize();
    HOST_WriteRegister( 0x7c90, 0x4000);
    HOST_WriteRegister( 0x7c92, 0x0054);    
    NorInitial();
    printf("Nor Initial\n");
    romSize = (MMP_UINT32) ((pRomAddr[3] << 24) | (pRomAddr[2] << 16) | (pRomAddr[1] << 8) | pRomAddr[0]);
    printf("romSize: %u bytes\n", romSize);
    printf("Start update firmware to nor, size: %u\n", romSize);
    checkClock = PalGetClock();
    NorWrite(&pRomAddr[4], 0, romSize);
    printf("Total Update time: %u ms\n", PalGetDuration(checkClock));
    pReadBuffer = (MMP_UINT8*) NOR_COMPARE_DATA_ADDR;
    printf("Nor update is done, start reading rom from nor\n");
    NorRead(pReadBuffer, 0, romSize);
    printf("Start comparing data\n");
    if (0 == memcmp(pReadBuffer, &pRomAddr[4], romSize))
    {
        printf("comparison is success\n");
    }
    else
    {
        printf("comparison is failed\n");
    }
    while (1);
end:
    return result;
}

static MMP_INT
Terminate(
    void)
{
    MMP_INT result = 0;

    coreTerminate();

    return result;
}

static MMP_INT
MainLoop(
    void)
{
    MMP_INT result = 0;
    for (;;)
    {
        PalSleep(33);
    }

    return result;
}

int appmain(void)
{
    MMP_INT result = 0;

    result = Initialize();
    if (result)
        goto end;

    result = MainLoop();
    if (result)
        goto end;

    result = Terminate();
    if (result)
        goto end;

end:
    return result;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
