#include "host/host.h"
#include "pal/pal.h"
#include "sys/sys.h"
#include "mmp_types.h"
#include "sys/sys.h"

#ifndef EXTERNAL_HDMIRX
    #include "hdmirx/it6607/mmp_it6607.h"
#else
    #include "hdmirx/it6604/mmp_it6604.h"
#endif

#include "mmp_hdmirx.h"

static MMP_BOOL gbDisableHdmiAccess = MMP_FALSE;

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
typedef void
(*DISABLE_HDCP)(
    MMP_BOOL flag);

typedef MMP_BOOL
(*POWER_DOWN)(
    MMP_BOOL enable);

typedef void
(*INITIALIZE)(
    void);

typedef void
(*OUTPUT_PIN_TRISTATE)(
    MMP_BOOL flag);

typedef MMP_BOOL
(*IS_SIGNAL_STABLE)(
    void);

typedef MMP_UINT32
(*GET_PROPERTY)(
    MMP_HDMIRX_PROPERTY property);

typedef void
(*SET_PROPERTY)(
    MMP_HDMIRX_PROPERTY property, MMP_UINT32 value);

typedef MMP_BOOL
(*IS_HDCP_ON)(
    void);

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct HDMI_RX_TAG
{
    INITIALIZE          initialize;
    POWER_DOWN          power_down;
    DISABLE_HDCP        disable_hdcp;
    OUTPUT_PIN_TRISTATE output_pin_tristate;
    IS_SIGNAL_STABLE    is_signal_stable;
    GET_PROPERTY        get_property;
    SET_PROPERTY        set_property;
    IS_HDCP_ON          is_hdcp_on;
} HDMI_RX;

//=============================================================================
//                Global Data Definition
//=============================================================================
static void *gtHDMIRXSemaphore = MMP_NULL;

const HDMI_RX gtHdmiRx           =
{
#ifndef EXTERNAL_HDMIRX
    mmpIT6607Initialize,
    mmpIT6607PowerDown,
    mmpIT6607DisableHDCP,
    mmpIT6607OutputPinTriState,
    mmpIT6607IsSignalStable,
    mmpIT6607GetProperty,
    mmpIT6607SetProperty,
    mmpIT6607IsHDCPOn
#else
    mmpIT6604Initialize,
    mmpIT6604PowerDown,
    mmpIT6604DisableHDCP,
    mmpIT6604OutputPinTriState,
    mmpIT6604IsSignalStable,
    mmpIT6604GetProperty,
    mmpIT6604SetProperty,
    mmpIT6604IsHDCPOn
#endif
};

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Device Initialize
 */
//=============================================================================
void
mmpHDMIRXInitialize(
    void)
{
    MMP_UINT16 chipPacketType = 0;

    if (gtHDMIRXSemaphore == MMP_NULL)
        gtHDMIRXSemaphore = SYS_CreateSemaphore(1, "MMP_HDMIRX");

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    HOST_ReadRegister(0x0000, &chipPacketType);
    chipPacketType = (chipPacketType >> 12);

    if (chipPacketType == 0x6) //IT9919-N
    {
        gtHdmiRx.disable_hdcp(MMP_TRUE);
    }
    else if (chipPacketType == 0x0 || chipPacketType == 0x2) //IT9917 or IT9919
    {
#ifndef EXTERNAL_HDMIRX
        gtHdmiRx.power_down(MMP_TRUE);
#endif
        goto lab_end;
    }

    gtHdmiRx.power_down(MMP_FALSE);
    gtHdmiRx.initialize();

lab_end:
    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);
}

//=============================================================================
/**
 * Device Terminate
 */
//=============================================================================
void
mmpHDMIRXTerminate(
    void)
{
    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    gtHdmiRx.power_down(MMP_TRUE);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    if (gtHDMIRXSemaphore)
    {
        SYS_DeleteSemaphore(gtHDMIRXSemaphore);
        gtHDMIRXSemaphore = MMP_NULL;
    }
}

//=============================================================================
/**
 * Device Output Pin Tri-State.
 */
//=============================================================================
void
mmpHDMIRXOutputPinTriState(
    MMP_BOOL flag)
{
    if (gbDisableHdmiAccess)
        return;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    gtHdmiRx.output_pin_tristate(flag);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);
}

//=============================================================================
/**
 * Device Signal State
 */
//=============================================================================
MMP_BOOL
mmpHDMIRXIsSignalStable(
    void)
{
    MMP_BOOL        bStableState;
    static MMP_UINT stableStateCount;

    if (gbDisableHdmiAccess)
        return MMP_TRUE;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    if (gtHdmiRx.is_signal_stable())
        stableStateCount++;
    else
        stableStateCount = 0;

    // we report the signal is stable only when we continously detect the signal
    // stable 3 times. 2 times is not safe.
    bStableState = (stableStateCount > 2);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    return bStableState;
}

//=============================================================================
/**
 * Device property.
 */
//=============================================================================
MMP_UINT32
mmpHDMIRXGetProperty(
    MMP_HDMIRX_PROPERTY property)
{
    MMP_UINT32 value;

    if (gbDisableHdmiAccess)
        return 0;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    value = gtHdmiRx.get_property(property);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    return value;
}

//=============================================================================
/**
 * Device set property.
 */
//=============================================================================
void
mmpHDMIRXSetProperty(
    MMP_HDMIRX_PROPERTY property, MMP_UINT32 value)
{
    if (gbDisableHdmiAccess)
        return;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    gtHdmiRx.set_property(property, value);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);
}

//=============================================================================
/**
 * Check HDCP.
 */
//=============================================================================
MMP_BOOL
mmpHDMIRXIsHDCPOn(
    void)
{
    MMP_UINT bHDCPOn;

    if (gbDisableHdmiAccess)
        return MMP_FALSE;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    bHDCPOn = gtHdmiRx.is_hdcp_on();

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    return bHDCPOn;
}

//=============================================================================
/**
 * Get DE Timing.
 */
//=============================================================================
void
mmpHDMIRXGetDETiming(
    MMP_HDMIRX_DETIMING *pDETiming)
{
    if (gbDisableHdmiAccess)
        return;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

#ifndef EXTERNAL_HDMIRX
    {
        MMP_IT6607_DETIMING DETable;
        mmpIT6607GetDETiming(&DETable);
        pDETiming->HDES = DETable.HDES;
        pDETiming->HDEE = DETable.HDEE;
        pDETiming->VDES = DETable.VDES;
        pDETiming->VDEE = DETable.VDEE;
    }
#else
    {
        MMP_IT6604_DETIMING DETable;
        mmpIT6604GetDETiming(&DETable);
        pDETiming->HDES = DETable.HDES;
        pDETiming->HDEE = DETable.HDEE;
        pDETiming->VDES = DETable.VDES;
        pDETiming->VDEE = DETable.VDEE;
    }
#endif

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);
}

//=============================================================================
/**
 * Device power down
 */
//=============================================================================
MMP_BOOL
mmpHDMIRXPowerDown(
    MMP_BOOL enable)
{
    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    gtHdmiRx.power_down(enable);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    return MMP_TRUE;
}

//=============================================================================
/**
 * Device power down
 */
//=============================================================================
void
mmpHDMIRXDisableHDCP(
    MMP_BOOL flag)
{
    if (gbDisableHdmiAccess)
        return;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

    gtHdmiRx.disable_hdcp(flag);

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);
}

MMP_BOOL
mmpHDMIRXIsDisableHDCP(
    void)
{
    MMP_BOOL flag = MMP_TRUE;

    if (gbDisableHdmiAccess)
        return MMP_TRUE;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

#ifndef EXTERNAL_HDMIRX
    flag = mmpIT6607IsDisableHDCP();
#else
    //flag = mmpIT6604IsDisableHDCP();
#endif

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    return flag;
}

MMP_BOOL
mmpHDMIRXIsHDCPKeyEmpty(
    void)
{
    MMP_BOOL flag = MMP_TRUE;

    if (gbDisableHdmiAccess)
        return MMP_FALSE;

    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

#ifndef EXTERNAL_HDMIRX
    flag = mmpIT6607IsHDCPKeyEmpty();
#else
    //flag = mmpIT6604IsHDCPKeyEmpty();
#endif

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);

    return flag;
}

void
mmpHDMIRXResetAudioFS(
    void)
{
    if (gtHDMIRXSemaphore)
        SYS_WaitSemaphore(gtHDMIRXSemaphore);

#ifndef EXTERNAL_HDMIRX
    mmpIT6607ResetAudioFS();
#else
    //mmpIT6604ResetAudioFS();
#endif

    if (gtHDMIRXSemaphore)
        SYS_ReleaseSemaphore(gtHDMIRXSemaphore);
}