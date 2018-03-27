#include "config.h"
#include "pal/pal.h"

#if defined(__OR32__)
#  include "or32.h"
#endif

#define MEM_DEBUG_REG1  0x03d8
#define MEM_DEBUG_REG2  0x03da
#define MEM_DEBUG_REG3  0x03dc
#define MEM_DEBUG_REG4  0x03de

MMP_BOOL
PalSetWatchPoint(
    void   *top,
    void   *buttom)
{
    if ((int)top > (int)buttom)
        return MMP_FALSE;

    HOST_WriteRegister (MEM_DEBUG_REG1,((((int)top)>> 2)&0xffff));      // TOP address [17:2]
    HOST_WriteRegister (MEM_DEBUG_REG2,((((int)top)>>18)&0x00ff));      // TOP address [25:18]
    HOST_WriteRegister (MEM_DEBUG_REG3,((((int)buttom)>> 2)&0xffff));   // BOTTOM address [17:2]
    HOST_WriteRegister (MEM_DEBUG_REG4,((1 << 15) |                     // D[15] enable memory monitor
                                        (2 << 13) |                     // D[14:13] = 10 to detect write request
                                        ((((int)buttom)>>18)&0x00ff))); // D[7:0] BOTTOM address [25:18]

    mtspr(SPR_DMR2, mfspr(SPR_DMR2) | (1<<(13+10)));            // enable external watch point

    return MMP_TRUE;
}

void
PalInvalidateCache(
    void   *ptr,
    MMP_INT len)
{
    or32_invalidate_cache(ptr, len);
}

void
PalFlushCache(
    void)
{
    dc_invalidate();
}

MMP_INT
PalGetBusClock(
    void)
{
    return or32_getBusCLK();
}

MMP_INT
PalGetSysClock(
    void)
{
    return or32_getSysCLK();
}

MMP_INT
PalGetMemClock(
    void)
{
    return or32_getMemCLK();
}

void
PalDoze(
    void)
{
    or32_doze();
}

void
PalDisableInterrupts(
    void)
{
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_IEE);
}

void
PalEnableInterrupts(
    void)
{
    mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);
}
