///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Main.c>
//   @author jau-chih.tseng@ite.com.tw
//   @date   2012/05/16
//   @fileversion: IT6604_SAMPLE_3.11
//******************************************/
#include "mmp_types.h"
#include "pal/timer.h"

#include "hdmirx/it6604/config.h"
#include "hdmirx/it6604/typedef.h"
#include "hdmirx/it6604/debug.h"
#include "hdmirx/it6604/it6604.h"
#include "hdmirx/it6604/io.h"
#include "hdmirx/it6604/mmp_it6604.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
//#define HDMI_DUMP_DEBUG

#define HPDOFF_MIN_PERIOD 2000

static PAL_CLOCK_T  prevTickCount = 0;
static BOOL         RXHPD_ENABLE  = TRUE;
static unsigned int HPDOff_TickCount;
static BOOL         bSignal;
//=============================================================================
//                Private Function Definition
//=============================================================================
static BOOL IsTimeOut(unsigned int x)
{
    if (PalGetDuration(prevTickCount) >= x)
    {
        prevTickCount = PalGetClock();
        return TRUE;
    }
    return FALSE;
}

static void hdmirx_clear_hpd()
{
    if (RXHPD_ENABLE == TRUE)
    {
        HPDOff_TickCount = PalGetClock();
        Turn_HDMIRX(OFF);
        RXHPD_ENABLE     = FALSE;
    }
}

static void hdmirx_set_hpd()
{
    if (RXHPD_ENABLE == FALSE)
    {
        while (PalGetDuration(HPDOff_TickCount) < HPDOFF_MIN_PERIOD)
        {
            //HDMIRX_DEBUG_PRINTF("HPDOff_TickCount = %d, ucTickCount = %d\r",HPDOff_TickCount,
            //PalGetClock()) ;
        }
        //HDMIRX_DEBUG_PRINTF("\n") ;

        Turn_HDMIRX(ON);
        RXHPD_ENABLE = TRUE;
    }
}

static int dump_vidmode()
{
#ifdef  HDMI_DUMP_DEBUG
    unsigned int HActive, VActive;
    unsigned int HTotal, VTotal;
    ULONG        HFreq, VFreq;
    ULONG        PCLK;
    int          xCntSum, xCntCnt;

    xCntCnt  = 0;
    xCntSum  = 0;

    xCntCnt++;
    xCntSum += getHDMIRXxCnt();

    HActive  = getHDMIRXHorzActive();
    VActive  = getHDMIRXVertActive();
    HTotal   = getHDMIRXHorzTotal();
    VTotal   = getHDMIRXVertTotal();

    PCLK     = 27000L * 128;
    PCLK    *= (ULONG) xCntCnt;
    PCLK    /= (ULONG) xCntSum;
    PCLK    *= 1000L;                         // 100*PCLK ;

    HFreq    = PCLK / (ULONG)HTotal;          // HFreq
    VFreq    = (HFreq * 100) / (ULONG)VTotal; // VFreq * 100

    HDMIRX_DEBUG_PRINTF(("\n========================================================================\n"));
    HDMIRX_DEBUG_PRINTF(("%s mode\n", IsHDMIRXHDMIMode() ? "HDMI" : "DVI"));
    HDMIRX_DEBUG_PRINTF(("Mode - %dx%d@", HActive, VActive));
    HDMIRX_DEBUG_PRINTF(("%ld.%02ldHz", VFreq / 100, VFreq % 100));
    switch (getHDMIRXOutputColorDepth())
    {
    case 0x7: HDMIRX_DEBUG_PRINTF(("@48bits")); break;
    case 0x6: HDMIRX_DEBUG_PRINTF(("@36bits")); break;
    case 0x5: HDMIRX_DEBUG_PRINTF(("@30bits")); break;
    case 0x4: HDMIRX_DEBUG_PRINTF(("@24bits")); break;
    default: HDMIRX_DEBUG_PRINTF(("@No def(24bits)")); break;
    }
    HDMIRX_DEBUG_PRINTF((", PCLK = %ld.%02ldMHz", PCLK / 1000000, (PCLK / 10000) % 100));
    PCLK  = xCntSum;
    PCLK *= 100;
    PCLK /= xCntCnt;
    PCLK -= (ULONG)(xCntSum / xCntCnt) * 100;
    HDMIRX_DEBUG_PRINTF((",xCnt= %d.%02ld\n", xCntSum / xCntCnt, PCLK));

    HDMIRX_DEBUG_PRINTF(("<%4dx%4d>,", HTotal, VTotal));

    HDMIRX_DEBUG_PRINTF(("H:(%d,%d,%d),"
                         , getHDMIRXHorzFrontPorch()
                         , getHDMIRXHorzSyncWidth()
                         , getHDMIRXHorzBackPorch()));

    HDMIRX_DEBUG_PRINTF(("V:(%d,%d,%d), "
                         , getHDMIRXVertFrontPorch()
                         , getHDMIRXVertSyncWidth()
                         , getHDMIRXVertSyncBackPorch()));

    HDMIRX_DEBUG_PRINTF(("VSyncToDE = %d\n", getHDMIRXVertSyncToDE()));
    HDMIRX_DEBUG_PRINTF(("========================================================================\n"));
    HDMIRX_DEBUG_PRINTF(("HDCP %s\n", IsHDCPOn() ? "ON" : "OFF"));
    HDMIRX_DEBUG_PRINTF(("========================================================================\n"));
#endif
}

static int dump_audSts()
{
#ifdef HDMI_DUMP_DEBUG
    BYTE audio_status;

    HDMIRX_DEBUG_PRINTF(("\n========================================================================\n"));
    audio_status = getHDMIRXAudioStatus();
    if ( (audio_status & T_AUDIO_MASK) != T_AUDIO_OFF)
    {
        BYTE ch[5];
        if (audio_status == T_AUDIO_HBR)
        {
            HDMIRX_DEBUG_PRINTF(("Audio input is HBR (High Bit Rate) audio.\n"));
        }
        else if (audio_status == T_AUDIO_DSD)
        {
            HDMIRX_DEBUG_PRINTF(("Audio input is DSD (One Bit Audio) audio.\n"));
        }
        else
        {
            if ( (audio_status & T_AUDIO_NLPCM) == T_AUDIO_NLPCM)
            {
                HDMIRX_DEBUG_PRINTF(("Audio input is IEC 61937 compressed audio.\n"));
            }
            else
            {
                HDMIRX_DEBUG_PRINTF(("Audio input is IEC 60958 linear PCM audio.\n"));
            }

            HDMIRX_DEBUG_PRINTF(("layout %d, ", (audio_status & F_AUDIO_LAYOUT_1) ? 1 : 0));
            HDMIRX_DEBUG_PRINTF(("%d source\n", (unsigned int)audio_status & 7));

            getHDMIRXAudioChannelStatus(ch);
            HDMIRX_DEBUG_PRINTF(("Channel Status: %02X %02X %02X %02X %02X\n", (int)ch[0], (int)ch[1], (int)ch[2], (int)ch[3], (int)ch[4]));
        }
    }
    else
    {
        HDMIRX_DEBUG_PRINTF(("No Audio.\n"));
    }
    HDMIRX_DEBUG_PRINTF(("========================================================================\n"));
#endif
}

static void dump_InfoFrame()
{
#ifdef HDMI_DUMP_DEBUG
    BYTE INFOFRAME[31];
    int  i;

    if (GetAVIInfoFrame(INFOFRAME) )
    {
        HDMIRX_DEBUG_PRINTF(("GetAVIInfoFrame():"));
        for (i = 0; i <= (3 + (INFOFRAME[2] & 0x1F)); i++)
        {
            HDMIRX_DEBUG_PRINTF((" %02X", (int)INFOFRAME[i]));
        }
        HDMIRX_DEBUG_PRINTF(("\n"));
    }
    else
    {
        HDMIRX_DEBUG_PRINTF(("Cannot get AVI Infoframe()\n"));
    }

    if (GetAudioInfoFrame(INFOFRAME) )
    {
        HDMIRX_DEBUG_PRINTF(("GetAudioInfoFrame():"));
        for (i = 0; i <= (3 + (INFOFRAME[2] & 0x1F)); i++)
        {
            HDMIRX_DEBUG_PRINTF((" %02X", (int)INFOFRAME[i]));
        }
        HDMIRX_DEBUG_PRINTF(("\n"));
    }
    else
    {
        HDMIRX_DEBUG_PRINTF(("Cannot get audio infoframe.\n"));
    }

    if (GetVENDORSPECInfoFrame(INFOFRAME) )
    {
        HDMIRX_DEBUG_PRINTF(("GetVENDORSPECInfoFrame():"));
        for (i = 0; i <= (3 + (INFOFRAME[2] & 0x1F)); i++)
        {
            HDMIRX_DEBUG_PRINTF((" %02X", (int)INFOFRAME[i]));
        }
        HDMIRX_DEBUG_PRINTF(("\n"));
    }
    else
    {
        HDMIRX_DEBUG_PRINTF(("Cannot GetVENDORSPECInfoFrame()\n"));
    }
#endif
}

MMP_UINT32
_getPCLK(
    void)
{
    unsigned int xCntSum, xCntCnt, PCLK;

    xCntCnt  = 0;
    xCntSum  = 0;

    xCntCnt++;
    xCntSum += getHDMIRXxCnt();

    PCLK     = 27000L * 128;
    PCLK    *= (unsigned int) xCntCnt;
    PCLK    /= (unsigned int) xCntSum;
    PCLK    *= 1000L; // 100*PCLK;

    return PCLK;
}

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Device Initialize
 */
//=============================================================================
void
mmpIT6604Initialize(
    void)
{
    gtTriStateEnable = 0;

    IT6604_PowerDown(MMP_FALSE);

    hdmirx_clear_hpd();

    EnableHDMIRXVideoOutput(VIDEO_AUTO);
    SelectHDMIPort(CAT_HDMI_PORTA);

    InitHDMIRX(TRUE);
    hdmirx_set_hpd();

    bSignal = 0;
}

//=============================================================================
/**
 * Device Output Pin Tri-State.
 */
//=============================================================================
void
mmpIT6604OutputPinTriState(
    MMP_BOOL flag)
{
    if (flag == MMP_TRUE)
        Set_IT6604_Tri_State_Enable();
    else
        Set_IT6604_Tri_State_Disable();
}

//=============================================================================
/**
 * Device Signal State
 */
//=============================================================================
MMP_BOOL
mmpIT6604IsSignalStable(
    void)
{
    BOOL bOldSignal, bChangeMode;

    bOldSignal  = bSignal;
    bSignal     = CheckHDMIRX();

#if 0
    bChangeMode = (bSignal != bOldSignal);
    if (bChangeMode)
    {
        // if Changed Mode ...
        if (bSignal)
        {
            // if signal is TRUE , then ...
            dump_vidmode();
            if (IsHDMIRXHDMIMode() )
                dump_InfoFrame();
            dump_audSts();
        }
        bChangeMode = FALSE;          // clear bChange Mode action
    }
#endif

    if (bSignal)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

//=============================================================================
/**
 * * Device property.
 */
//=============================================================================
MMP_UINT32
mmpIT6604GetProperty(
    MMP_IT6604_PROPERTY property)
{
    MMP_UINT32 value = 0;
    MMP_UINT8  SamplingFreq;
    MMP_UINT8  Channelstatus;
    MMP_UINT8  audio_status;
    MMP_UINT8  INFOFRAME[31];
    MMP_UINT32 aspectRatio;
    MMP_UINT32 width, height;

    switch (property)
    {
    case IT6604_IS_DVI_MODE:
        value = (IsHDMIRXHDMIMode() == TRUE) ? 0 : 1;
        break;

    case IT6604_CHECK_ENGINE_IDLE:
        value = 0;
        break;

    case IT6604_ASPECT_RATIO:
        GetAudioInfoFrame(INFOFRAME);
        aspectRatio = (INFOFRAME[5] & (0x3 << 4)) >> 4;
        if (aspectRatio == 0x0 || aspectRatio == 0x3)
        {
            width  = getHDMIRXHorzActive();
            height = getHDMIRXVertActive();

            if (width >= 1280)
                value = IT6604_PAR_16_9;
            else
                value = IT6604_PAR_4_3;
        }
        else if (aspectRatio == 0x1) //4:3
            value = IT6604_PAR_4_3;
        else if (aspectRatio == 0x2) //16:9
            value = IT6604_PAR_16_9;
        break;

    case IT6604_OUTPUT_VIDEO_MODE:
        value = getHDMIRXOutputColorMode();
        break;

    case IT6604_PCLK:
        value = _getPCLK();
        break;

    case IT6604_HEIGHT:
        value = (MMP_UINT16)getHDMIRXVertActive();
        break;

    case IT6604_WIDTH:
        value = (MMP_UINT16)getHDMIRXHorzActive();
        break;

    case IT6604_HTOTAL:
        value = (MMP_UINT16)getHDMIRXHorzTotal();
        break;

    case IT6604_VTOTAL:
        value = (MMP_UINT16)getHDMIRXVertTotal();
        break;

    case IT6604_COLOR_DEPTH:
        switch (getHDMIRXOutputColorDepth())
        {
        case 0x7: value = 48; break;
        case 0x6: value = 36; break;
        case 0x5: value = 30; break;
        case 0x4: value = 24; break;
        default:  value = 24; break;
        }
        break;

    case IT6604_IS_INTERLACE:
        value = IsHDMIRXInterlace();
        break;

    case IT6604_IS_VID_STABLE:
        value = IsSCDT();
        break;

    case IT6604_IS_AUDIO_ON:
        value = IsHDMIRX_AudioReady();
        break;

    case IT6604_IS_AUDIO_RESET:
        value = gIsAudioReset;
        break;

    case IT6604_AUDIO_SAMPLERATE:
        if (getHDMIRXAudioInfo(&SamplingFreq, &Channelstatus))
        {
            switch (SamplingFreq)
            {
            case AUDFS_32KHz:
                value = 32000;
                break;

            case AUDFS_44p1KHz:
                value = 44100;
                break;

            case AUDFS_48KHz:
                value = 48000;
                break;

            case AUDFS_88p2KHz:
                value = 88000;
                break;

            case AUDFS_96KHz:
                value = 96000;
                break;

            case AUDFS_176p4KHz:
                value = 176000;
                break;

            case AUDFS_192KHz:
                value = 192000;
                break;

            default:
                value = 0;
                break;
            }
        }
        else
        {
            value = 0;
        }
        break;

    case IT6604_AUDIO_CHANNEL_NUMBER:
        if (IsHDMIRXHDMIMode())
        {
            value = HDMIRXAudioChannelNum();
            /* Try to get Audio channel from Audio Info Frame */
            if (!value)
            {
                //while (!getHDMIRX_AudioInfoFrame((Audio_InfoFrame *)INFOFRAME))
                //{
                //printf("############\n");
                //PalSleep(100);
                //}
                if (GetAudioInfoFrame(INFOFRAME))
                {
                    /* Ref. CEA-861-D Table 17 */
                    switch (INFOFRAME[4] & 0x7)
                    {
                    case 0x0:
                        value = 0;
                        break;

                    case 0x1:
                        value = 2;
                        break;

                    case 0x2:
                        value = 3;
                        break;

                    case 0x3:
                        value = 4;
                        break;

                    case 0x4:
                        value = 5;
                        break;

                    case 0x5:
                        value = 6;
                        break;

                    case 0x6:
                        value = 7;
                        break;

                    case 0x7:
                        value = 8;
                        break;

                    default:
                        value = 0;
                        break;
                    }
                }
            }
        }
        else
        {
            value = 0;
        }
        break;

    case IT6604_AUDIO_MODE:
        audio_status = getHDMIRXAudioStatus();
        if ((audio_status & T_AUDIO_MASK) != T_AUDIO_OFF)
        {
            if (audio_status == T_AUDIO_HBR)
            {
                value = IT6604_AUDIO_HBR;
                printf("Audio input is HBR (High Bit Rate) audio.\n");
            }
            else if (audio_status == T_AUDIO_DSD)
            {
                value = IT6604_AUDIO_DSD;
                printf("Audio input is DSD (One Bit Audio) audio.\n");
            }
            else
            {
                if ((audio_status & T_AUDIO_NLPCM) == T_AUDIO_NLPCM)
                {
                    value = IT6604_AUDIO_NLPCM;
                    printf("Audio input is IEC 61937 compressed audio.\n");
                }
                else
                {
                    value = IT6604_AUDIO_LPCM;
                    printf("Audio input is IEC 60958 linear PCM audio.\n");
                }
            }
        }
        else
        {
            value = IT6604_AUDIO_OFF;
            printf("No Audio.\n");
        }

    case IT6604_CHECK_PLG5V_PWR:
        value = CheckPlg5VPwr();
        break;

    default:
        break;
    }

    return value;
}

//=============================================================================
/**
 * * Device set property.
 */
//=============================================================================
void
mmpIT6604SetProperty(
    MMP_IT6604_PROPERTY property, MMP_UINT32 value)
{
    switch (property)
    {
    case IT6604_IS_AUDIO_RESET:
        gIsAudioReset = value;

    default:
        break;
    }
}

//=============================================================================
/**
 * * Check HDCP.
 */
//=============================================================================
MMP_BOOL
mmpIT6604IsHDCPOn(
    void)
{
    return IsHDCPOn();
}

//=============================================================================
/**
 * * Get DE Timing.
 */
//=============================================================================
void
mmpIT6604GetDETiming(
    MMP_IT6604_DETIMING *pDETiming)
{
    MMP_UINT32 HSync      = getHDMIRXHorzSyncWidth();
    MMP_UINT32 HBackPorch = getHDMIRXHorzBackPorch();
    MMP_UINT32 HActive    = getHDMIRXHorzActive();
    MMP_UINT32 VSync      = getHDMIRXVertSyncWidth();
    MMP_UINT32 VBackPorch = getHDMIRXVertSyncBackPorch();
    MMP_UINT32 VActive    = getHDMIRXVertActive();

    pDETiming->HDES = HSync + HBackPorch - 2;
    pDETiming->HDEE = HSync + HBackPorch - 2 + HActive;

    pDETiming->VDES = VSync + VBackPorch - 1;
    pDETiming->VDEE = VSync + VBackPorch - 1 + VActive;

    //printf("---HDES = %d, HDEE = %d, VDES = %d, VDEE = %d---\n", pDETiming->HDES, pDETiming->HDEE, pDETiming->VDES, pDETiming->VDEE);
}

//=============================================================================
/**
 * Device power down
 */
//=============================================================================
MMP_BOOL
mmpIT6604PowerDown(
    MMP_BOOL enable)
{
    IT6604_PowerDown(enable);
}

//=============================================================================
/**
 * HDCP ON/OFF
 */
//=============================================================================
void
mmpIT6604DisableHDCP(
    MMP_BOOL flag)
{
    return;
}