/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file TS_Mux.c
 * Mux bitstream to ts format
 *
 * @author Steven Hsiao
 * @version 0.01
 */

#include "pal/pal.h"
#include "crc.h"
#include "ts_mux.h"
#include "mmp_dma.h"
#include "psi_si_table_mgr.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define TS_WRITE_BUFFER_SIZE                (188 * 100 * 4)
#define TS_PRIOR_HEADER                     (4)

#define SIT_PID                             0x77
#define CRC_LEN                             4

#define TS_HEADER_LEN                       4
#define PSI_PRIOR_HEADER_LEN                3
#define SIT_HEADER_LEN                      20
#define NULL_PACKET_HEADER_LEN              6
#define POINTER_FIELD_LEN                   1
#define PES_PAYLOAD_LEN                     (TS_PACKET_LEN - TS_HEADER_LEN)

#define AC3_DESCRIPTOR_LEN                  3
#define AAC_DESCRIPTOR_LEN                  4
#define DTS_DESCRIPTOR_LEN                  7
#define PES_HEADER_LEN_WITHOUT_PRIVATE_DATA 14
#define PES_HEADER_LEN_WITH_PRIVATE_DATA    31

#define ADJUST_PCR_OFFSET                   90 * 1000 // 1 second
#define EXTRA_PCR_INJECT_COUNT              0xFFFFFFFF
#define SKT_PID                             0x88
#define SKT_TABLE_ID                        0x88
#define SKT_HEADER_LEN                      6
#define PUBLIC_KEY_LEN                      2
#define SESSION_KEY_LEN                     2

//=============================================================================
//                              Macro Definition
//=============================================================================

#define Assign_PID(pBuffer, pid)      { pBuffer[1] = ((pBuffer[1] & 0xE0) | (pid >> 8)); pBuffer[2] = (pid & 0xFF); }
#define UpdateCc(pBuffer, cc)         {(pBuffer[3] = (pBuffer[3] & 0xF0) | cc); if (++cc > 0xF) {cc = 0; } }
#define UpdateRepeatCc(pBuffer, cc)   {--cc; cc &= 0xF; (pBuffer[3] = (pBuffer[3] & 0xF0) | cc); if (++cc > 0xF) {cc = 0; }}
#define UpdateBufferIndex(ptTsHandle) { ptTsHandle->tsWriteIndex += 188; if (ptTsHandle->tsWriteIndex >= ptTsHandle->bufferSize) { if (ptTsHandle->pfCallback) (*ptTsHandle->pfCallback)(ptTsHandle); ptTsHandle->tsWriteIndex = 0; } }

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

/** DMA width normal mode */
static MMP_UINT32 gDmaAttribList[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)0,
    MMP_DMA_ATTRIB_NONE
};

static MMP_UINT8  gpTsDefaultHeader[] =
{
    0x47, 0x0, 0x0, 0x10
};

static MMP_UINT8  gpTsDefaultStartIndHeader[] =
{
    0x47, 0x40, 0x0, 0x10
};

static MMP_UINT8  gpTsAdaptationHeader[] =
{
    0x47, 0x40, 0x0, 0x30
};
static MMP_UINT8  gpTsNoPayloadAdaptationHeader[] =
{
    0x47, 0x00, 0x0, 0x20
};

static MMP_UINT8  gpNullPktHeader[NULL_PACKET_HEADER_LEN] =
{
    0x47, 0x5F, 0xFF, 0x20, 0xB7, 0x0
};

static MMP_UINT8  gpDefaultPesHeader[PES_HEADER_LEN_WITHOUT_PRIVATE_DATA] =
{
    0x0, 0x0, 0x1, 0xE0, 0x0, 0x0, 0x84, 0x80, 0x5, 0x21, 0x0, 0x1, 0x0, 0x1
};

static MMP_UINT8  gpPrivateDataPesHeader[PES_HEADER_LEN_WITH_PRIVATE_DATA] =
{
    0x0, 0x0, 0x1, 0xE0, 0x0, 0x0, 0x84, 0x81, 0x16, 0x21, 0x0, 0x1, 0x0, 0x1, 0x8E,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

static MMP_UINT8  gpDefaultSitHeader[SIT_HEADER_LEN] =
{
    0x77, 0xB0, 0x00, 0xFF, 0xFF, 0xC1, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00
};

#ifdef AV_SENDER_SECURITY_MODE
static MMP_UINT8 gpDefaultSktHeader[SIT_HEADER_LEN] =
{
    0x88, 0xB0, 0x0, 0xC1, 0x0, 0x0
};
#endif

//=============================================================================
//                              Private Function Definition
//=============================================================================

static void
_AppendAdaptationField(
    MMP_UINT8 *pBuffer,
    MMP_BOOL bPCR,
    MMP_UINT32 adaptationLen,
    MMP_BOOL bDiscontinuity,
    MMP_BOOL bIdr)
{
    MMP_UINT32 skipLen = 0;
    PalMemset(pBuffer, 0x0, adaptationLen);
    pBuffer[0] = (MMP_UINT8) (adaptationLen - 1);
    if (bPCR)
    {
        if (bDiscontinuity)
        {
            pBuffer[1] = 0x90;
        }
        else
        {
            pBuffer[1] = 0x10;
        }
        if (bIdr)
        {
            pBuffer[1] |= 0x60;
        }
        skipLen = 6;
    }
    if (adaptationLen > 1)
    {
        if (adaptationLen - 2 - skipLen)
        {
            PalMemset(&pBuffer[2 + skipLen], 0xFF, adaptationLen - 2 - skipLen);
        }
    }
}

static void
_InsertPesPts(
    MMP_UINT8 *pPesBuffer,
    PTS_TIME *pPts)
{
    MMP_UINT32 bit30_32 = ((pPts->timeHigh & 0x1) << 2) | ((pPts->timeLow >> 30) & 0x3);
    MMP_UINT32 bit15_29 = ((pPts->timeLow >> 15) & 0x7FFF);
    MMP_UINT32 bit0_14  = (pPts->timeLow & 0x7FFF);

    pPesBuffer[9]  = (MMP_UINT8) (0x21 | (bit30_32 << 1));
    pPesBuffer[10] = (MMP_UINT8) (bit15_29 >> 7);
    pPesBuffer[11] = (MMP_UINT8) ((bit15_29 & 0x7F) << 1) | 0x1;
    pPesBuffer[12] = (MMP_UINT8) (bit0_14 >> 7);
    pPesBuffer[13] = (MMP_UINT8) ((bit0_14 & 0x7F) << 1) | 0x1;
}

static void
_InsertPCR(
    MMP_UINT8 *pAdaptationBuffer,
    PTS_TIME *pPts)
{
    MMP_UINT32 timeLow  = pPts->timeLow;
    MMP_UINT32 timeHigh = pPts->timeHigh;

    if (timeLow < ADJUST_PCR_OFFSET)
    {
        if (timeHigh == 0)
        {
            timeHigh = 1;
        }
        else
        {
            timeHigh = 0;
        }
    }
    timeLow             -= ADJUST_PCR_OFFSET;

    pAdaptationBuffer[2] = ((MMP_UINT8) (timeHigh << 7) | (MMP_UINT8) (timeLow >> 25));
    pAdaptationBuffer[3] = ((MMP_UINT8) (timeLow >> 17)) & 0xFF;
    pAdaptationBuffer[4] = ((MMP_UINT8) (timeLow >> 9)) & 0xFF;
    pAdaptationBuffer[5] = ((MMP_UINT8) (timeLow >> 1)) & 0xFF;
    pAdaptationBuffer[6] = (((MMP_UINT8) (timeLow & 0x1)) << 7) | 0x7E;
    pAdaptationBuffer[7] = 0x0;
}

static void
_IncrementPCR(
    PTS_TIME *pPts,
    MMP_UINT32 incrementMs)
{
    MMP_UINT32 timeLow        = pPts->timeLow;
    MMP_UINT32 timeHigh       = pPts->timeHigh;
    MMP_UINT32 incrementValue = incrementMs * 90;

    if (pPts->timeLow > (0xFFFFFFFF - incrementValue))
    {
        pPts->timeHigh = (pPts->timeHigh + 1) & 0x1;
    }
    pPts->timeLow += incrementValue;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

void
TsMuxUpdateTimeStamp(
    PTS_TIME *ptPts,
    MMP_UINT32 timeStamp)
{
    MMP_UINT64 pts = (MMP_UINT64) timeStamp * 90;
    pts            %= 0x200000000ULL;
    ptPts->timeHigh = (MMP_UINT32) ((pts >> 32) & 0x1);
    ptPts->timeLow  = (MMP_UINT32) pts;
}

void
TsMuxCreatePatTable(
    TS_OUT_HANDLE *ptTsHandle)
{
    PsiSiMgrGetPat(ptTsHandle->pPatTsPacketBuffer);
}

void
TsMuxCreatePmtTable(
    TS_OUT_HANDLE *ptTsHandle)
{
    MMP_UINT32         i          = 0;
    SERVICE_ENTRY_INFO *ptService = MMP_NULL;
    ptTsHandle->serviceCount = PsiSiMgrGetServiceCount();
    for (i = 0; i < ptTsHandle->serviceCount; i++)
    {
        ptService = &ptTsHandle->ptServiceArray[i];
        PsiSiMgrGetEsPid(i, &ptService->videoPid, &ptService->audioPid);
        PsiSiMgrGetPmt(i, ptService->pPmtTsPacketBuffer);
    }
}

void
TsMuxCreateSdtTable(
    TS_OUT_HANDLE *ptTsHandle)
{
    if (ptTsHandle->pSdtTsPacketBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle->pSdtTsPacketBuffer);
        ptTsHandle->pSdtTsPacketBuffer = MMP_NULL;
    }
    ptTsHandle->sdtTsPacketLen     = PsiSiMgrGetSdtLen();
    ptTsHandle->pSdtTsPacketBuffer = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, PsiSiMgrGetSdtLen());
    if (MMP_NULL == ptTsHandle->pSdtTsPacketBuffer)
    {
        ptTsHandle->sdtTsPacketLen = 0;
    }
    else
    {
        PsiSiMgrGetSdt(ptTsHandle->pSdtTsPacketBuffer);
    }
}

void
TsMuxCreateNitTable(
    TS_OUT_HANDLE *ptTsHandle)
{
    if (ptTsHandle->pNitTsPacketBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle->pNitTsPacketBuffer);
        ptTsHandle->pNitTsPacketBuffer = MMP_NULL;
    }
    ptTsHandle->nitTsPacketLen     = PsiSiMgrGetNitLen();
    ptTsHandle->pNitTsPacketBuffer = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, PsiSiMgrGetNitLen());
    if (MMP_NULL == ptTsHandle->pNitTsPacketBuffer)
    {
        ptTsHandle->nitTsPacketLen = 0;
    }
    else
    {
        PsiSiMgrGetNit(ptTsHandle->pNitTsPacketBuffer);
    }
}

// Source Information Table
//input_source
//0 - HDMI
//1 - Analog
//2 - Sensor
//15 - Unknown

//interlace_mode
//0 - Progressive Mode
//1 - Interlace Mode
//audio_codec
//0 - Mpeg Audio
//1 - AAC
//
// source_information_section() {
//      table_id                        8
//      section_syntax_indicator        1
//      '0'                             1
//      reserved                        2
//      section_length                 12
//      reserved                       18
//      version_number                  5
//      current_next_indicator          1
//      section_number                  8
//      last_section_number             8
//      reserved                        1
//      skipframe                       1
//      valid_resoultion                1
//      input_lock                      1
//      input_source                    4
//      width                          16
//      height                         16
//      encoder_info                   16
//      reserved                        4
//      audio_codec                     4
//      sampling_rate                  32
//      CRC_32                         32
// }
void
TsMuxCreateSitTable(
    TS_OUT_HANDLE *ptTsHandle,
    TX_ENCODE_INFO *ptTxEncodeInfo)
{
    MMP_UINT8  *pSitBuffer         = MMP_NULL;
    MMP_UINT8  *pSitTsPacketBuffer = ptTsHandle->pSitTsPacketBuffer;
    MMP_UINT32 sectionLen          = (SIT_HEADER_LEN - PSI_PRIOR_HEADER_LEN) + CRC_LEN;
    MMP_UINT32 sitLen              = sectionLen + PSI_PRIOR_HEADER_LEN;
    MMP_UINT32 stuffLen            = TS_PACKET_LEN - TS_HEADER_LEN - sitLen - POINTER_FIELD_LEN;
    MMP_UINT32 crc                 = 0;
    MMP_UINT32 frameRateInt        = 0;

    //dbg_msg(DBG_MSG_TYPE_INFO, "sit version: %u\n", ptTsHandle->sitVersion);
    //dbg_msg(DBG_MSG_TYPE_INFO, "bInterlace: %u\n", ptTxEncodeInfo->bInterlace);
    //dbg_msg(DBG_MSG_TYPE_INFO, "bValidResolution: %u\n", ptTxEncodeInfo->bValidResolution);
    //dbg_msg(DBG_MSG_TYPE_INFO, "bVideoLock: %u\n", ptTxEncodeInfo->bVideoLock);
    //dbg_msg(DBG_MSG_TYPE_INFO, "inputsource: %u\n", ptTxEncodeInfo->videoInputSource);
    //dbg_msg(DBG_MSG_TYPE_INFO, "width: %u\n", ptTxEncodeInfo->width);
    //dbg_msg(DBG_MSG_TYPE_INFO, "height: %u\n", ptTxEncodeInfo->height);
    //dbg_msg(DBG_MSG_TYPE_INFO, "framerate: %u\n", ptTxEncodeInfo->frameRate);
    //dbg_msg(DBG_MSG_TYPE_INFO, "audioCodec: %u\n", ptTxEncodeInfo->audioCodec);
    //dbg_msg(DBG_MSG_TYPE_INFO, "samplingRate: %u\n", ptTxEncodeInfo->samplingRate);

    PalMemset(pSitTsPacketBuffer, 0x0, TS_PACKET_LEN - stuffLen);
    PalMemset(&pSitTsPacketBuffer[TS_PACKET_LEN - stuffLen], 0xFF, stuffLen);
    PalMemcpy(pSitTsPacketBuffer, gpTsDefaultStartIndHeader, TS_PRIOR_HEADER);
    Assign_PID(pSitTsPacketBuffer, SIT_PID);
    pSitBuffer             = pSitTsPacketBuffer + TS_PRIOR_HEADER + POINTER_FIELD_LEN;
    PalMemcpy(pSitBuffer, gpDefaultSitHeader, SIT_HEADER_LEN);
    // Add section length
    pSitBuffer[1]          = (pSitBuffer[1] & 0xF0) | ((sectionLen >> 8) & 0x0F);
    pSitBuffer[2]          = (sectionLen & 0xFF);
    // Update sit version
    pSitBuffer[5]          = (0xC1 | ptTsHandle->sitVersion << 1);
    ptTsHandle->sitVersion = ((ptTsHandle->sitVersion + 1) & 0x1F);

    //interlace_mode
    if (ptTxEncodeInfo->bSkipFrame)
    {
        pSitBuffer[8] |= (0x1 << 6);
    }
    else
    {
        pSitBuffer[8] &= ~(0x1 << 6);
    }

    //valid_resolution
    if (ptTxEncodeInfo->bValidResolution)
    {
        pSitBuffer[8] |= (0x1 << 5);
    }
    else
    {
        pSitBuffer[8] &= ~(0x1 << 5);
    }
    //input_lock
    if (ptTxEncodeInfo->bVideoLock)
    {
        pSitBuffer[8] |= (0x1 << 4);
    }
    else
    {
        pSitBuffer[8] &= ~(0x1 << 4);
    }
    // input_source
    pSitBuffer[8] |= (ptTxEncodeInfo->videoInputSource & 0x0F);

    //resolution
    //width
    pSitBuffer[9]                    = (MMP_UINT8) ((ptTxEncodeInfo->width & 0xFF00) >> 8);
    pSitBuffer[10]                   = (MMP_UINT8) (ptTxEncodeInfo->width & 0xFF);

    //height
    pSitBuffer[11]                   = (MMP_UINT8) ((ptTxEncodeInfo->height & 0xFF00) >> 8);
    pSitBuffer[12]                   = (MMP_UINT8) (ptTxEncodeInfo->height & 0xFF);

    //encoder_info
    pSitBuffer[13]                   = (MMP_UINT8) (ptTxEncodeInfo->encoderInfo >> 8);
    pSitBuffer[14]                   = (MMP_UINT8) (ptTxEncodeInfo->encoderInfo & 0xFF);

    //audioCodec
    pSitBuffer[15]                  |= ptTxEncodeInfo->audioCodec & 0xF;

    //audio sampling rate
    pSitBuffer[16]                   = (MMP_UINT8) ((ptTxEncodeInfo->samplingRate & 0xFF000000) >> 24);
    pSitBuffer[17]                   = (MMP_UINT8) ((ptTxEncodeInfo->samplingRate & 0x00FF0000) >> 16);
    pSitBuffer[18]                   = (MMP_UINT8) ((ptTxEncodeInfo->samplingRate & 0x0000FF00) >> 8);
    pSitBuffer[19]                   = (MMP_UINT8) (ptTxEncodeInfo->samplingRate & 0x000000FF);

    crc                              = CalcCRC32(pSitBuffer, sitLen - 4);
    pSitBuffer[sitLen - CRC_LEN]     = (crc >> 24);
    pSitBuffer[sitLen - CRC_LEN + 1] = ((crc >> 16) & 0xFF);
    pSitBuffer[sitLen - CRC_LEN + 2] = ((crc >> 8) & 0xFF);
    pSitBuffer[sitLen - CRC_LEN + 3] = (crc & 0xFF);

    //{
    //    MMP_UINT32 i = 0;
    //    dbg_msg(DBG_MSG_TYPE_INFO, "sit table\n");
    //    for (i = 0; i < 188; i++)
    //    {
    //        dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", pSitTsPacketBuffer[i]);
    //    }
    //    dbg_msg(DBG_MSG_TYPE_INFO, "\n");
    //}
}

#ifdef AV_SENDER_SECURITY_MODE
MMP_BOOL
TsMuxCreateSktTable(
    TS_OUT_HANDLE *ptTsHandle)
{
    MMP_UINT32          bufferSize                   = 0;
    MMP_UINT32          i                            = 0;
    MMP_UINT32          sectionCount                 = 0;
    MMP_UINT8           *pBuffer                     = MMP_NULL;
    MMP_UINT32          lastSectionCount             = 0;
    MMP_UINT32          currentSection               = 0;
    TS_DEVICE_KEY_STORE *ptKeyStore                  = MMP_NULL;
    MMP_UINT8           *pSktSectionBuffer           = MMP_NULL;
    MMP_UINT8           *pSktTsPacketBuffer          = MMP_NULL;
    MMP_UINT32          sectionLen                   = 0;
    MMP_UINT32          sktLen                       = 0;
    MMP_UINT32          stuffLen                     = 0;
    MMP_UINT32          crc                          = 0;
    MMP_UINT8           pTmpEncryptSessionKeyBuf[32] = { 0 };
    PUBLIC_KEY          tPublicKey                   = { 0 };
    ptTsHandle->sktSectionCount = 0;

    if (TsSecurityGetKeyStore(&ptKeyStore))
    {
        for (i = 0; i < ptKeyStore->keyCount; i++)
        {
            if (ptKeyStore->pptKeyEntry[i]->keySize && ptKeyStore->pptKeyEntry[i]->bValid)
            {
                ptTsHandle->sktSectionCount++;
            }
        }
        if (0 == ptTsHandle->sktSectionCount)
        {
            return MMP_FALSE;
        }
        if (ptTsHandle->pSktTsPacketBuffer)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle->pSktTsPacketBuffer);
        }
        ptTsHandle->pSktTsPacketBuffer = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, TS_PACKET_LEN * ptTsHandle->sktSectionCount);
        if (MMP_NULL == ptTsHandle->pSktTsPacketBuffer)
        {
            return MMP_FALSE;
        }
        lastSectionCount = (ptTsHandle->sktSectionCount - 1);
        if (MMP_NULL == pTmpEncryptSessionKeyBuf)
        {
            return MMP_FALSE;
        }
        {
            MMP_UINT32 j = 0;
            dbg_msg(DBG_MSG_TYPE_INFO, "data session key\n");
            for (j = 0; j < 16; j++)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", ptTsHandle->pSessionKey[j]);
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "\n");
        }
        for (i = 0; i < ptKeyStore->keyCount; i++)
        {
            if (ptKeyStore->pptKeyEntry[i]->keySize && ptKeyStore->pptKeyEntry[i]->bValid)
            {
                sectionLen         = (SKT_HEADER_LEN - PSI_PRIOR_HEADER_LEN) + PUBLIC_KEY_LEN + SESSION_KEY_LEN + ptKeyStore->pptKeyEntry[i]->keySize + ptTsHandle->sessionKeyLen + CRC_LEN;
                sktLen             = sectionLen + PSI_PRIOR_HEADER_LEN;
                stuffLen           = TS_PACKET_LEN - TS_HEADER_LEN - sktLen - POINTER_FIELD_LEN;
                pSktTsPacketBuffer = &ptTsHandle->pSktTsPacketBuffer[TS_PACKET_LEN * currentSection];
                //PalMemset(pSktTsPacketBuffer, 0x0, TS_PACKET_LEN);
                PalMemset(pSktTsPacketBuffer, 0x0, TS_PACKET_LEN - stuffLen);
                PalMemset(&pSktTsPacketBuffer[TS_PACKET_LEN - stuffLen], 0xFF, stuffLen);
                PalMemcpy(pSktTsPacketBuffer, gpTsDefaultStartIndHeader, TS_PRIOR_HEADER);
                Assign_PID(pSktTsPacketBuffer, SKT_PID);
                pSktSectionBuffer                                      = pSktTsPacketBuffer + TS_PRIOR_HEADER + POINTER_FIELD_LEN;
                PalMemcpy(pSktSectionBuffer, gpDefaultSktHeader, SKT_HEADER_LEN);
                PalMemcpy(&pSktSectionBuffer[SKT_HEADER_LEN + PUBLIC_KEY_LEN + SESSION_KEY_LEN], ptKeyStore->pptKeyEntry[i]->pKeyBuffer, ptKeyStore->pptKeyEntry[i]->keySize);
                tPublicKey.modulusLen                                  = ptKeyStore->pptKeyEntry[i]->keySize - 4;
                tPublicKey.pModulus                                    = ptKeyStore->pptKeyEntry[i]->pKeyBuffer;
                tPublicKey.pubExponentLen                              = 4;
                tPublicKey.pPubExponent                                = &ptKeyStore->pptKeyEntry[i]->pKeyBuffer[tPublicKey.modulusLen];
                TsSecurityEncryptSessionKey(&tPublicKey, ptTsHandle->pSessionKey, ptTsHandle->sessionKeyLen, pTmpEncryptSessionKeyBuf);
                PalMemcpy(&pSktSectionBuffer[SKT_HEADER_LEN + PUBLIC_KEY_LEN + SESSION_KEY_LEN + ptKeyStore->pptKeyEntry[i]->keySize], pTmpEncryptSessionKeyBuf, ptTsHandle->sessionKeyLen);
                pSktSectionBuffer[SKT_HEADER_LEN]                      = (tPublicKey.modulusLen + tPublicKey.pubExponentLen) >> 8;
                pSktSectionBuffer[SKT_HEADER_LEN + 1]                  = ((tPublicKey.modulusLen + tPublicKey.pubExponentLen) & 0xFF);
                pSktSectionBuffer[SKT_HEADER_LEN + PUBLIC_KEY_LEN]     = ptTsHandle->sessionKeyLen >> 8;
                pSktSectionBuffer[SKT_HEADER_LEN + PUBLIC_KEY_LEN + 1] = (ptTsHandle->sessionKeyLen & 0xFF);
                pSktSectionBuffer[1]                                   = (pSktSectionBuffer[1] & 0xF0) | ((sectionLen >> 8) & 0x0F);
                pSktSectionBuffer[2]                                   = (sectionLen & 0xFF);
                pSktSectionBuffer[4]                                   = (MMP_UINT8) currentSection;
                pSktSectionBuffer[5]                                   = (MMP_UINT8) lastSectionCount;
                crc                                                    = CalcCRC32(pSktSectionBuffer, sktLen - 4);
                pSktSectionBuffer[sktLen - CRC_LEN]                    = (crc >> 24);
                pSktSectionBuffer[sktLen - CRC_LEN + 1]                = ((crc >> 16) & 0xFF);
                pSktSectionBuffer[sktLen - CRC_LEN + 2]                = ((crc >> 8) & 0xFF);
                pSktSectionBuffer[sktLen - CRC_LEN + 3]                = (crc & 0xFF);
                dbg_msg(DBG_MSG_TYPE_INFO, "crc: 0x%X\n", crc);
                {
                    MMP_UINT32 j = 0;
                    dbg_msg(DBG_MSG_TYPE_INFO, "section data\n");
                    for (j = 0; j < sectionLen; j++)
                    {
                        dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pSktSectionBuffer[j]);
                    }
                    dbg_msg(DBG_MSG_TYPE_INFO, "\n");
                }
                currentSection++;
            }
        }
    }
}

void
TsMuxGenerateSktTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    MMP_UINT32 i             = 0;
    MMP_UINT8  *pSktTsBuffer = MMP_NULL;
    for (i = 0; i < ptTsHandle->sktSectionCount; i++)
    {
        pSktTsBuffer = &ptTsHandle->pSktTsPacketBuffer[TS_PACKET_LEN * i];
        UpdateCc(pSktTsBuffer, ptTsHandle->sktCc);
        PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], pSktTsBuffer, TS_PACKET_LEN);
        UpdateBufferIndex(ptTsHandle);
    }
}
#endif

void
TsMuxGeneratePatTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    UpdateCc(ptTsHandle->pPatTsPacketBuffer, ptTsHandle->patCc);
    PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], ptTsHandle->pPatTsPacketBuffer, TS_PACKET_LEN);
    UpdateBufferIndex(ptTsHandle);
}

void
TsMuxGeneratePmtTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    MMP_UINT32         i          = 0;
    SERVICE_ENTRY_INFO *ptService = MMP_NULL;
    for (i = 0; i < ptTsHandle->serviceCount; i++)
    {
        ptService = &ptTsHandle->ptServiceArray[i];
        UpdateCc(ptService->pPmtTsPacketBuffer, ptService->pmtCc);
        PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], ptService->pPmtTsPacketBuffer, TS_PACKET_LEN);
        UpdateBufferIndex(ptTsHandle);
    }
}

void
TsMuxGenerateSdtTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    MMP_UINT32 packetCount = ptTsHandle->sdtTsPacketLen / TS_PACKET_LEN;
    MMP_UINT32 i           = 0;
    MMP_UINT8  *pBuffer    = MMP_NULL;;
    for (i = 0; i < packetCount; i++)
    {
        pBuffer = &ptTsHandle->pSdtTsPacketBuffer[TS_PACKET_LEN * i];
        UpdateCc(pBuffer, ptTsHandle->sdtCc);
        PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], &ptTsHandle->pSdtTsPacketBuffer[TS_PACKET_LEN * i], TS_PACKET_LEN);
        UpdateBufferIndex(ptTsHandle);
    }
}

void
TsMuxGenerateNitTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    MMP_UINT32 packetCount = ptTsHandle->nitTsPacketLen / TS_PACKET_LEN;
    MMP_UINT32 i           = 0;
    MMP_UINT8  *pBuffer    = MMP_NULL;;
    for (i = 0; i < packetCount; i++)
    {
        pBuffer = &ptTsHandle->pNitTsPacketBuffer[TS_PACKET_LEN * i];
        UpdateCc(pBuffer, ptTsHandle->nitCc);
        PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], &ptTsHandle->pNitTsPacketBuffer[TS_PACKET_LEN * i], TS_PACKET_LEN);
        UpdateBufferIndex(ptTsHandle);
    }
}

void
TsMuxGenerateSitTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    UpdateCc(ptTsHandle->pSitTsPacketBuffer, ptTsHandle->sitCc);
    PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], ptTsHandle->pSitTsPacketBuffer, TS_PACKET_LEN);
    UpdateBufferIndex(ptTsHandle);
}

void
TsMuxGeneratePesTsPacket(
    TS_OUT_HANDLE *ptTsHandle,
    MMP_UINT32 serviceIndex,
    MMP_BOOL bVideo,
    MMP_UINT8 *pEsBuffer,
    MMP_UINT32 esPacketLen,
    MMP_BOOL bDiscontinuity)
{
    MMP_UINT32         i                   = 0;
    MMP_UINT32         loopTimes           = 0;
    MMP_UINT32         headerPacketLen     = 0;
    MMP_UINT32         adaptationLen       = 8;
    MMP_UINT8          *pPesTsPacketBuffer = MMP_NULL;
    MMP_UINT8          *pPesBuffer         = MMP_NULL;
    MMP_UINT32         esBufferIndex       = 0;
    MMP_UINT32         remainPayloadSize   = 0;
    //FILE*      ptPesFile = ptTsHandle->pWriteFILE;
    MMP_UINT16         pid                 = 0;
    MMP_UINT8          *pCc                = 0;
    PTS_TIME           *ptTimestamp        = MMP_NULL;
    MMP_UINT32         pesPacketLen        = 0;
    MMP_BOOL           bIdr                = MMP_FALSE;
    SERVICE_ENTRY_INFO *ptService          = &ptTsHandle->ptServiceArray[serviceIndex];
    MMP_DMA_CONTEXT    tDmaContext         = { 0 };
    MMP_UINT32         injectPcrCount      = 0;
    MMP_UINT32         extraPcrInjectCount = 0;
    MMP_UINT32         pcrIncrementTime    = 0;
    mmpDmaCreateContext(&tDmaContext);
    ptTsHandle->pesTsSize = 0;

    if (bVideo)
    {
        pid         = ptService->videoPid;
        pCc         = &ptService->videoCc;
        ptTimestamp = &ptService->tVideoTimestamp;
        if (bVideo & 0x80000000)
        {
            bIdr = MMP_TRUE;
        }
    }
    else
    {
        pid          = ptService->audioPid;
        pCc          = &ptService->audioCc;
        ptTimestamp  = &ptService->tAudioTimestamp;
        pesPacketLen = esPacketLen;
    }

    if (ptTsHandle->bEnableSecurityMode)
    {
        headerPacketLen = TS_PACKET_LEN - TS_HEADER_LEN - adaptationLen - PES_HEADER_LEN_WITH_PRIVATE_DATA;
        if (pesPacketLen)
        {
            pesPacketLen += (PES_HEADER_LEN_WITH_PRIVATE_DATA - 6);
        }
    }
    else
    {
        headerPacketLen = TS_PACKET_LEN - TS_HEADER_LEN - adaptationLen - PES_HEADER_LEN_WITHOUT_PRIVATE_DATA;
        if (pesPacketLen)
        {
            pesPacketLen += (PES_HEADER_LEN_WITHOUT_PRIVATE_DATA - 6);
        }
    }

    if (headerPacketLen > esPacketLen)
    {
        if (ptTsHandle->bEnableSecurityMode)
        {
            adaptationLen   = TS_PACKET_LEN - TS_HEADER_LEN - PES_HEADER_LEN_WITH_PRIVATE_DATA - esPacketLen;
            headerPacketLen = TS_PACKET_LEN - TS_HEADER_LEN - adaptationLen - PES_HEADER_LEN_WITH_PRIVATE_DATA;
        }
        else
        {
            adaptationLen   = TS_PACKET_LEN - TS_HEADER_LEN - PES_HEADER_LEN_WITHOUT_PRIVATE_DATA - esPacketLen;
            headerPacketLen = TS_PACKET_LEN - TS_HEADER_LEN - adaptationLen - PES_HEADER_LEN_WITHOUT_PRIVATE_DATA;
        }
    }

    // Generate PES header packet first.
    pPesTsPacketBuffer = (MMP_UINT8 *) &ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex];
    //PalMemset(pPesTsPacketBuffer, 0x0, TS_PACKET_LEN);
    PalMemcpy(pPesTsPacketBuffer, gpTsAdaptationHeader, TS_PRIOR_HEADER);
    Assign_PID(pPesTsPacketBuffer, pid);
    UpdateCc(pPesTsPacketBuffer, *pCc);
    if (bVideo)
    {
        _AppendAdaptationField(&pPesTsPacketBuffer[TS_HEADER_LEN], MMP_TRUE, adaptationLen, bDiscontinuity, bIdr);
        _InsertPCR(&pPesTsPacketBuffer[TS_HEADER_LEN], ptTimestamp);
    }
    else
    {
        _AppendAdaptationField(&pPesTsPacketBuffer[TS_HEADER_LEN], MMP_FALSE, adaptationLen, MMP_FALSE, MMP_FALSE);
    }
    pPesBuffer = pPesTsPacketBuffer + TS_PRIOR_HEADER + adaptationLen;

    // Update CTR nonce counter into private data section of PES header.
    if (ptTsHandle->bEnableSecurityMode)
    {
        PalMemcpy(pPesBuffer, gpPrivateDataPesHeader, PES_HEADER_LEN_WITH_PRIVATE_DATA);
        // Update counter
#ifdef AV_SENDER_SECURITY_MODE
        PalMemcpy(&pPesBuffer[15], ptTsHandle->pInitVector, 16);
#endif
    }
    else
    {
        PalMemcpy(pPesBuffer, gpDefaultPesHeader, PES_HEADER_LEN_WITHOUT_PRIVATE_DATA);
    }

    // Update audio length and audio type if the bitstream is audio
    if (MMP_FALSE == bVideo)
    {
        // update stream id
        pPesBuffer[3] = 0xC0;
        // update packet length
        pPesBuffer[4] = ((pesPacketLen >> 8) & 0xFF);
        pPesBuffer[5] = (pesPacketLen & 0xFF);
    }

    // Update pts
    _InsertPesPts(pPesBuffer, ptTimestamp);

    if (ptTsHandle->bEnableSecurityMode)
    {
        PalMemcpy(&pPesBuffer[PES_HEADER_LEN_WITH_PRIVATE_DATA], &pEsBuffer[esBufferIndex], headerPacketLen);
    }
    else
    {
        PalMemcpy(&pPesBuffer[PES_HEADER_LEN_WITHOUT_PRIVATE_DATA], &pEsBuffer[esBufferIndex], headerPacketLen);
    }
    esBufferIndex        += headerPacketLen;
    UpdateBufferIndex(ptTsHandle);
    loopTimes             = (esPacketLen - esBufferIndex) / PES_PAYLOAD_LEN;
    //fwrite(pPesTsPacketBuffer, 1, TS_PACKET_LEN, ptPesFile);
    // Rest packet without adaptation and pes header.
    ptTsHandle->pesTsSize = loopTimes * 188 + 188;
    {
#if 0
        MMP_UINT32 testClock = PalGetClock();
        MMP_UINT32 timeCount = 0;
#endif

        extraPcrInjectCount = loopTimes / EXTRA_PCR_INJECT_COUNT;
        if (extraPcrInjectCount)
        {
            pcrIncrementTime = ptService->frameTime / (extraPcrInjectCount + 1);
            //dbg_msg(DBG_MSG_TYPE_INFO, "extraCount: %u, incrTime: %u\n", extraPcrInjectCount, pcrIncrementTime);
        }

        while (esPacketLen - esBufferIndex >= PES_PAYLOAD_LEN)
        {
            if (bVideo && injectPcrCount == EXTRA_PCR_INJECT_COUNT)
            {
                pPesTsPacketBuffer     = (MMP_UINT8 *) &ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex];
                PalMemcpy(pPesTsPacketBuffer, gpTsAdaptationHeader, TS_PRIOR_HEADER);
                pPesTsPacketBuffer[1] &= (~0x40);
                Assign_PID(pPesTsPacketBuffer, pid);
                UpdateCc(pPesTsPacketBuffer, *pCc);
                _AppendAdaptationField(&pPesTsPacketBuffer[TS_HEADER_LEN], MMP_TRUE, 8, MMP_FALSE, MMP_FALSE);
                _IncrementPCR(ptTimestamp, pcrIncrementTime);
                _InsertPCR(&pPesTsPacketBuffer[TS_HEADER_LEN], ptTimestamp);
                PalMemcpy(&pPesTsPacketBuffer[8 + TS_HEADER_LEN], &pEsBuffer[esBufferIndex], PES_PAYLOAD_LEN - 8);
                injectPcrCount = 0;
                esBufferIndex += PES_PAYLOAD_LEN - 8;
                UpdateBufferIndex(ptTsHandle);
                continue;
            }

            if (bVideo)
            {
                injectPcrCount++;
            }

            pPesTsPacketBuffer = (MMP_UINT8 *) &ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex];
            //PalMemset(pPesTsPacketBuffer, 0x0, TS_PACKET_LEN);
            PalMemcpy(pPesTsPacketBuffer, gpTsDefaultHeader, TS_PRIOR_HEADER);
            Assign_PID(pPesTsPacketBuffer, pid);
            UpdateCc(pPesTsPacketBuffer, *pCc);
            gDmaAttribList[3] = (MMP_UINT32) &pEsBuffer[esBufferIndex];
            gDmaAttribList[5] = (MMP_UINT32) &pPesTsPacketBuffer[TS_PRIOR_HEADER];
            gDmaAttribList[7] = PES_PAYLOAD_LEN;
            mmpDmaWaitIdle(tDmaContext);
            mmpDmaSetAttrib(tDmaContext, gDmaAttribList);
            mmpDmaFire(tDmaContext);
            //PalMemcpy(&pPesTsPacketBuffer[TS_PRIOR_HEADER], &pEsBuffer[esBufferIndex], PES_PAYLOAD_LEN);
            esBufferIndex += PES_PAYLOAD_LEN;
            //fwrite(pPesTsPacketBuffer, 1, TS_PACKET_LEN, ptPesFile);
            UpdateBufferIndex(ptTsHandle);
        }
#if 0
        timeCount = PalGetDuration(testClock);
        if (timeCount > 10)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "loop: %u, %u ms\n", loopTimes, timeCount);
        }
#endif
        mmpDmaWaitIdle(tDmaContext);
        mmpDmaDestroyContext(tDmaContext);
    }

    // Add Remain Byes
    pPesTsPacketBuffer = (MMP_UINT8 *) &ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex];
    //PalMemset(pPesTsPacketBuffer, 0x0, TS_PACKET_LEN);
    remainPayloadSize  = esPacketLen - esBufferIndex;
    if (remainPayloadSize)
    {
        adaptationLen         = TS_PACKET_LEN - TS_HEADER_LEN - remainPayloadSize;
        PalMemcpy(pPesTsPacketBuffer, gpTsAdaptationHeader, TS_PRIOR_HEADER);
        pPesTsPacketBuffer[1] = 0;
        Assign_PID(pPesTsPacketBuffer, pid);
        UpdateCc(pPesTsPacketBuffer, *pCc);
        _AppendAdaptationField(&pPesTsPacketBuffer[TS_HEADER_LEN], MMP_FALSE, adaptationLen, MMP_FALSE, MMP_FALSE);
        PalMemcpy(&pPesTsPacketBuffer[TS_HEADER_LEN + adaptationLen], &pEsBuffer[esBufferIndex], remainPayloadSize);
        UpdateBufferIndex(ptTsHandle);
        ptTsHandle->pesTsSize += 188;
    }
#if 0
    ptTsHandle->bForceUpdate = MMP_TRUE;
    if (ptTsHandle->pfCallback)
    {
        (*ptTsHandle->pfCallback)(ptTsHandle);
    }
    ptTsHandle->bForceUpdate = MMP_FALSE;
    ptTsHandle->pesTsSize    = 0;
#endif
}

void
TsMuxGenerateNullTsPacket(
    TS_OUT_HANDLE *ptTsHandle)
{
    PalMemcpy(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex], gpNullPktHeader, NULL_PACKET_HEADER_LEN);
    PalMemset(&ptTsHandle->pTsWriteBuffer[ptTsHandle->tsWriteIndex + NULL_PACKET_HEADER_LEN], 0xFF, TS_PACKET_LEN - NULL_PACKET_HEADER_LEN);
    UpdateBufferIndex(ptTsHandle);
}

void
TsMuxRemoveService(
    TS_OUT_HANDLE *ptTsHandle)
{
    PalMemset(ptTsHandle->ptServiceArray, 0x0, sizeof(ptTsHandle->ptServiceArray));
    ptTsHandle->serviceCount = 0;
}

MMP_BOOL
TsMuxCreateHandle(
    TS_OUT_HANDLE **pptTsHandle,
    BUFFER_FULL_CALLBACK pfCallback,
    MMP_BOOL bExternalBuffer,
    MMP_BOOL bEnableSecurityMode)
{
    TS_OUT_HANDLE *ptHandle = MMP_NULL;

    if (MMP_NULL == pptTsHandle)
    {
        return MMP_FALSE;
    }

    ptHandle = (TS_OUT_HANDLE *) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_OUT_HANDLE));
    if (MMP_NULL == ptHandle)
    {
        return MMP_FALSE;
    }
    PalMemset(ptHandle, 0x0, sizeof(TS_OUT_HANDLE));

    if (MMP_FALSE == bExternalBuffer)
    {
        ptHandle->bufferSize     = TS_WRITE_BUFFER_SIZE;
        ptHandle->pTsWriteBuffer = (MMP_UINT8 *) PalHeapAlloc(PAL_HEAP_DEFAULT, TS_WRITE_BUFFER_SIZE);

        if (MMP_NULL == ptHandle->pTsWriteBuffer)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptHandle);
            return MMP_FALSE;
        }
    }

    ptHandle->pfCallback = pfCallback;
    //if (bEnableSecurityMode)
    //{
    //    mmpDpuGenerateRandomData(ptHandle->pSessionKey, 16);
    //}
    ptHandle->bEnableSecurityMode = bEnableSecurityMode;
    ptHandle->bExternalBuffer     = bExternalBuffer;
    *pptTsHandle                  = ptHandle;
    return MMP_TRUE;
}

void
TsMuxTerminateHandle(
    TS_OUT_HANDLE *ptTsHandle)
{
    if (MMP_NULL == ptTsHandle)
    {
        return;
    }

    if (ptTsHandle->pTsWriteBuffer && MMP_FALSE == ptTsHandle->bExternalBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle->pTsWriteBuffer);
    }

    if (ptTsHandle->pSdtTsPacketBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle->pSdtTsPacketBuffer);
    }

    if (ptTsHandle->pNitTsPacketBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle->pNitTsPacketBuffer);
    }

    TsMuxRemoveService(ptTsHandle);
    PalHeapFree(PAL_HEAP_DEFAULT, ptTsHandle);
}

void
TsMuxSetTsBuffer(
    TS_OUT_HANDLE *ptTsHandle,
    MMP_UINT8 *pTsBuffer,
    MMP_UINT32 bufferSize)
{
    if (MMP_NULL == ptTsHandle)
    {
        return;
    }

    ptTsHandle->bufferSize     = bufferSize;
    ptTsHandle->pTsWriteBuffer = pTsBuffer;
}