/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file psi_si_table_mgr.c
 * Maintain PSI/SI table
 *
 * @author Steven Hsiao
 * @version 0.01
 */

#include "pal/pal.h"
#include "crc.h"
#include "core_interface.h"
#include "psi_si_table_mgr.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//#define DEBUG_TABLE

#define TS_PACKET_LEN                               188
#define TS_PRIOR_HEADER                             4
#define TS_MAX_PAYLOAD                              (TS_PACKET_LEN - TS_PRIOR_HEADER)
#define MAX_SERVICE_COUNT                           4
#define MAX_TS_ID                                   65535
#define MAX_SERVICE_ID                              65535
#define MAX_NETWORK_ID                              65535
#define MAX_ON_ID                                   65535
#define MAX_PROGRAM_NUMBER                          65535
#define MAX_PID                                     8191
#define MAX_E_BOOK_LCN                              1023
#define MAX_NORDIC_LCN                              16383
#define MIN_FREQUENCY                               50000
#define MAX_FREQUENCY                               950000
#define MAX_NAME_LEN                                255
#define MAX_SERVICE_TYPE                            0xFF
#define MAX_SI_SECTION_LEN                          1024
#define MAX_DESCRIPTOR_LEN                          255
                              
#define DEFAULT_TSID                                0x80
#define DEFAULT_COUNTRY_ID                          CORE_COUNTRY_TAIWAN
#define DEFAULT_NETWORK_ID                          0x3301
#define DEFAULT_ONID                                0x209E
#define DEFAULT_PDSD                                0x0
#define DEFAULT_LCN_TYPE                            NO_LCN
#define DEFAULT_NETWORKNAME                         "Private Network"

#define INIT_VERSION                                0xFFFFFFFF

#define PAT_PID                                     0
#define NIT_PID                                     0x10
#define SDT_PID                                     0x11
#define CRC_LEN                                     4

#define TERRESETRIAL_DELIVERY_SYSTEM_DESCRIPTOR_LEN 13
#define TS_HEADER_LEN                               4
#define PSI_PRIOR_HEADER_LEN                        3
#define PAT_HEADER_LEN                              8
#define PMT_HEADER_LEN                              12
#define SDT_HEADER_LEN                              11
#define NIT_HEADER_LEN                              18
#define NIT_TRANSPORT_STREAM_HEADER_LEN             8
#define NULL_PACKET_HEADER_LEN                      6
#define SDT_SERVICE_PRIOR_HEADER_LEN                5
#define POINTER_FIELD_LEN                           1

#define AC3_DESCRIPTOR_LEN                          3
#define AAC_DESCRIPTOR_LEN                          4
#define DTS_DESCRIPTOR_LEN                          7

#define SERVICE_DESCRIPTOR_LEN                      5
#define POINTER_FIELD_LEN                           1
#define PDSD_DESCRIPTOR_LEN                         6
#define LCN_CONTENT_LEN                             4      
#define TERRESTRIAL_DELIVERY_SYSTEM_LEN            13
#define DESCRIPTOR_HEADER_LEN                       2
#define SERVICE_LIST_CONTENT_LEN                    3

typedef enum LCN_TYPE_TAG
{
    NO_LCN = 0,
    LCN_E_BOOK,
    LCN_NORDIG,
} LCN_TYPE;

typedef enum TS_BANDWIDTH_TYPE_TAG
{
    TS_8M_BANDWIDTH = 0,
    TS_7M_BANDWIDTH = 1,
    TS_6M_BANDWIDTH = 2,
    TS_5M_BANDWIDTH = 3,
    TS_OTHER_BANDWIDTH = 4
} TS_BANDWIDTH_TYPE;

typedef enum TS_CONSTELLATION_TYPE_TAG
{
    TS_QPSK = 0,
    TS_16QAM = 1,
    TS_64QAM = 2,
    TS_OTHER_CONSTELLATION = 3
} TS_CONSTELLATION_TYPE;

typedef enum TS_CODE_RATE_TYPE_TAG
{
    TS_CODE_RATE_1_2 = 0,
    TS_CODE_RATE_2_3 = 1,
    TS_CODE_RATE_3_4 = 2,
    TS_CODE_RATE_5_6 = 3,
    TS_CODE_RATE_7_8 = 4,
    TS_OTHER_CODE_RATE = 7
} TS_CODE_RATE_TYPE;

typedef enum TS_GUARD_INTERVAL_TYPE_TAG
{
    TS_GUARD_INTERVAL_1_32 = 0,
    TS_GUARD_INTERVAL_1_16 = 1,
    TS_GUARD_INTERVAL_1_8 = 2,
    TS_GUARD_INTERVAL_1_4 = 3
} TS_GUARD_INTERVAL_TYPE;

//=============================================================================
//                              Macro Definition
//=============================================================================

#define Assign_PID(pBuffer, pid) { pBuffer[1] = ((pBuffer[1] & 0xE0) | (pid >> 8)); pBuffer[2] = (pid & 0xFF); }


//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct PMT_SERVICE_ENTRY_INFO_TAG
{
    MMP_UINT16  programNumber;
    MMP_UINT16  pmtPid;
    MMP_UINT16  videoPid;
    MMP_UINT16  videoStreamType;
    MMP_UINT16  audioPid;
    MMP_UINT16  audioStreamType;
    MMP_UINT8*  pServiceProviderName;
    MMP_UINT32  providerNameLen;
    MMP_UINT8*  pServiceName;
    MMP_UINT32  serviceNameLen;
    MMP_UINT32  serviceType; //ETSI EN 300 468 Table 79
    MMP_UINT32  lcn;
    MMP_UINT8   pTsPmtBuffer[TS_PACKET_LEN];
    MMP_UINT32  pmtVersion;
    MMP_BOOL    bUpdatePmtVersion;
} PMT_SERVICE_ENTRY_INFO;

typedef struct PSI_SI_TABLE_MGR_TAG
{
    CORE_COUNTRY_ID             countryId;
    MMP_UINT32                  tsId;
    MMP_UINT32                  onId;
    MMP_UINT32                  networkId;
    MMP_UINT32                  pdsd;
    MMP_UINT32                  frequency;
    MMP_UINT32                  bandwidth;
    CONSTELLATION_MODE          constellation;
    CODE_RATE_MODE              codeRate;
    GUARD_INTERVAL_MODE         guardInterval;
    LCN_TYPE                    lcnType;
    PMT_SERVICE_ENTRY_INFO      ptService[MAX_SERVICE_COUNT];
    MMP_UINT32                  serviceCount;
    MMP_UINT8*                  pNetworkName;
    MMP_UINT32                  networkNameLen;
    MMP_UINT8                   pTsPatBuffer[TS_PACKET_LEN];
    MMP_UINT32                  patVersion;
    MMP_BOOL                    bUpdatePatVersion;
    MMP_UINT8*                  pTsSdtBuffer;
    MMP_UINT32                  sdtVersion;
    MMP_BOOL                    bUpdateSdtVersion;
    MMP_UINT32                  sdtBufferLen;
    MMP_UINT8*                  pTsNitBuffer;
    MMP_UINT32                  nitVersion;
    MMP_BOOL                    bUpdateNitVersion;
    MMP_UINT32                  nitBufferLen;
    MMP_MUTEX                   pMutex;
} PSI_SI_TABLE_MGR;


//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

static MMP_UINT8 gpTsDefaultHeader[] =
{
    0x47, 0x0, 0x0, 0x10
};

static MMP_UINT8 gpTsDefaultStartIndHeader[] =
{
    0x47, 0x40, 0x0, 0x10
};

static MMP_UINT8 gpDefaultPatHeader[PAT_HEADER_LEN] =
{
    0x0, 0xB0, 0x00, 0x00, 0x80, 0xCD, 0x0, 0x0 
};

static MMP_UINT8 gpAc3Descriptor[AC3_DESCRIPTOR_LEN] =
{
    0x7A, 0x1, 0x0
};

static MMP_UINT8 gpAacDescriptor[AAC_DESCRIPTOR_LEN] =
{
    0x7C, 0x2, 0x0, 0x0
};

static MMP_UINT8 gpDtsDescriptor[DTS_DESCRIPTOR_LEN] =
{
    0x7B, 0x5, 0xD7, 0x60, 0x20, 0x01, 0xF8
};

static MMP_UINT8 gpDefaultPmtHeader[PMT_HEADER_LEN] =
{
    0x2, 0xB0, 0x17, 0x00, 0x01, 0xCD, 0x0, 0x0, 0xFF, 0xFE, 0xF0, 0x0
};

static MMP_UINT8 gpDefaultSdtHeader[SDT_HEADER_LEN] =
{
    0x42, 0xF0, 0x0, 0x00, 0x80, 0xC1, 0x0, 0x0, 0xF0, 0x0, 0xFF
};

static MMP_UINT8 gpDefaultNitHeader[NIT_HEADER_LEN] =
{
    0x40, 0xF0, 0x0, 0x12, 0x34, 0xC1, 0x0, 0x0, 0xF0, 0xD, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};


static PSI_SI_TABLE_MGR gtPsiSiTableMgr = { 0 };

//=============================================================================
//                              Private Function Definition
//=============================================================================

static void
_PsiSiTableMgrLock()
{
    if (gtPsiSiTableMgr.pMutex)
    {
        PalWaitMutex(gtPsiSiTableMgr.pMutex, PAL_MUTEX_INFINITE);
    }
}

static void
_PsiSiTableMgrUnLock()
{
    if (gtPsiSiTableMgr.pMutex)
    {
        PalReleaseMutex(gtPsiSiTableMgr.pMutex);
    }
}

static void
_PsiSiTableMgrDefaultSetting(
    void)
{
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;

    gtPsiSiTableMgr.tsId = DEFAULT_TSID;
    gtPsiSiTableMgr.countryId = DEFAULT_COUNTRY_ID;
    gtPsiSiTableMgr.networkId = DEFAULT_NETWORK_ID;
    gtPsiSiTableMgr.onId = DEFAULT_ONID;
    gtPsiSiTableMgr.pdsd = DEFAULT_PDSD;
    gtPsiSiTableMgr.lcnType = DEFAULT_LCN_TYPE;
    gtPsiSiTableMgr.patVersion = INIT_VERSION;
    gtPsiSiTableMgr.bUpdatePatVersion = MMP_TRUE;
    gtPsiSiTableMgr.sdtVersion = INIT_VERSION;
    gtPsiSiTableMgr.bUpdateSdtVersion = MMP_TRUE;
    gtPsiSiTableMgr.nitVersion = INIT_VERSION;
    gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;

    for (i = 0; i < MAX_SERVICE_COUNT; i++)
    {
        ptService = &gtPsiSiTableMgr.ptService[i];
        ptService->pmtVersion = INIT_VERSION;
        ptService->bUpdatePmtVersion = MMP_TRUE;
    }

    gtPsiSiTableMgr.networkNameLen = sizeof(DEFAULT_NETWORKNAME);
    gtPsiSiTableMgr.pNetworkName = PalHeapAlloc(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.networkNameLen);
    PalMemcpy(gtPsiSiTableMgr.pNetworkName, &DEFAULT_NETWORKNAME, gtPsiSiTableMgr.networkNameLen);
}

static void
_PsiSiTableMgrGeneratePAT(
    void)
{
    MMP_UINT8* pPatBuffer = MMP_NULL;
    MMP_UINT8* pPatTsPacketBuffer = gtPsiSiTableMgr.pTsPatBuffer;
    MMP_UINT32 sectionLen = (PAT_HEADER_LEN - PSI_PRIOR_HEADER_LEN) + 4 * gtPsiSiTableMgr.serviceCount + CRC_LEN;
    MMP_UINT32 patLen = sectionLen + PSI_PRIOR_HEADER_LEN;
    MMP_UINT32 stuffLen = TS_PACKET_LEN - TS_HEADER_LEN - patLen - POINTER_FIELD_LEN;
    MMP_UINT32 crc = 0;
    MMP_UINT32 i = 0;

    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;
    if (0 == gtPsiSiTableMgr.serviceCount)
    {
        return;
    }

    PalMemset(pPatTsPacketBuffer, 0x0, TS_PACKET_LEN - stuffLen);
    PalMemset(&pPatTsPacketBuffer[TS_PACKET_LEN - stuffLen], 0xFF, stuffLen);
    PalMemcpy(pPatTsPacketBuffer, gpTsDefaultStartIndHeader, TS_PRIOR_HEADER);
    Assign_PID(pPatTsPacketBuffer, PAT_PID);
    pPatBuffer = pPatTsPacketBuffer + TS_PRIOR_HEADER + POINTER_FIELD_LEN;
    PalMemcpy(pPatBuffer, gpDefaultPatHeader, PAT_HEADER_LEN);
    
    // section len
    pPatBuffer[1] = (pPatBuffer[1] & 0xF0) | ((sectionLen >> 8) & 0x0F);
    pPatBuffer[2] = (sectionLen & 0xFF);
    
    // transport stream id
    pPatBuffer[3] = ((gtPsiSiTableMgr.tsId >> 8) & 0xFF);
    pPatBuffer[4] = (gtPsiSiTableMgr.tsId & 0xFF);

    //Update version
    if (gtPsiSiTableMgr.bUpdatePatVersion)
    {         
        gtPsiSiTableMgr.patVersion = ((gtPsiSiTableMgr.patVersion + 1) & 0x1F);
        gtPsiSiTableMgr.bUpdatePatVersion = MMP_FALSE;
    }
    pPatBuffer[5] = (0xC1 | (gtPsiSiTableMgr.patVersion << 1));

    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        ptService = &gtPsiSiTableMgr.ptService[i];
        pPatBuffer[PAT_HEADER_LEN + i * 4] = (ptService->programNumber >> 8);
        pPatBuffer[PAT_HEADER_LEN + i * 4 + 1] = (ptService->programNumber & 0xFF);

        pPatBuffer[PAT_HEADER_LEN + i * 4 + 2] = 0xE0 | ((ptService->pmtPid >> 8) & 0xFF);
        pPatBuffer[PAT_HEADER_LEN + i * 4 + 3] = (ptService->pmtPid & 0xFF);
    }
    crc = CalcCRC32(pPatBuffer, patLen - 4);
    pPatBuffer[patLen - CRC_LEN] = (crc >> 24);
    pPatBuffer[patLen - CRC_LEN + 1] = ((crc >> 16) & 0xFF);
    pPatBuffer[patLen - CRC_LEN + 2] = ((crc >> 8) & 0xFF);
    pPatBuffer[patLen - CRC_LEN + 3] = (crc & 0xFF);

#ifdef DEBUG_TABLE
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "PAT TS Buffer\n");
            for (i = 0; i < TS_PACKET_LEN; i++)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", pPatTsPacketBuffer[i]);
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "\n\n");
        }
#endif
}

static void
_PsiSiTableMgrGeneratePMT(
    void)
{
    MMP_UINT8* pPmtTsPacketBuffer = MMP_NULL;
    MMP_UINT8* pPmtBuffer = MMP_NULL;
    MMP_UINT32 sectionLen = 0; // 2 *5 (Video + Audio)
    MMP_UINT32 pmtLen = 0;
    MMP_UINT32 stuffLen = 0;
    MMP_UINT32 crc = 0;
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;
    MMP_UINT32 audioDescriptorSize = 0; 
    MMP_UINT8* pAudioDescriptor = MMP_NULL;
    MMP_UINT16 audioStreamType = 0;
 
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        ptService = &gtPsiSiTableMgr.ptService[i];
        pPmtTsPacketBuffer = ptService->pTsPmtBuffer;
        PalMemset(pPmtTsPacketBuffer, 0x0, TS_PACKET_LEN - stuffLen);

        switch(ptService->audioStreamType)
        {
            case MPEG_AUDIO:
                audioStreamType = 0x03;
                break;
            case DOLBY_AC3:
                audioStreamType = 0xFF;
                audioDescriptorSize = AC3_DESCRIPTOR_LEN;
                pAudioDescriptor = gpAc3Descriptor;
                break;
            case AAC:
                audioStreamType = 0x0F;
                audioDescriptorSize = AAC_DESCRIPTOR_LEN;
                pAudioDescriptor = gpAacDescriptor;
                break;
            case DTS:
                audioStreamType = 0xFF;
                audioDescriptorSize = DTS_DESCRIPTOR_LEN;
                pAudioDescriptor = gpDtsDescriptor;
                break;
        }
        sectionLen = (PMT_HEADER_LEN - PSI_PRIOR_HEADER_LEN) + 2 * 5 + audioDescriptorSize + CRC_LEN; // 2 *5 (Video + Audio)
        pmtLen = sectionLen + PSI_PRIOR_HEADER_LEN;
        stuffLen = TS_PACKET_LEN - TS_HEADER_LEN - pmtLen - POINTER_FIELD_LEN;

        PalMemset(&pPmtTsPacketBuffer[TS_PACKET_LEN - stuffLen], 0xFF, stuffLen);
        PalMemcpy(pPmtTsPacketBuffer, gpTsDefaultStartIndHeader, TS_PRIOR_HEADER);
        Assign_PID(pPmtTsPacketBuffer, ptService->pmtPid);
        pPmtBuffer = pPmtTsPacketBuffer + TS_PRIOR_HEADER + POINTER_FIELD_LEN;
        PalMemcpy(pPmtBuffer, gpDefaultPmtHeader, PMT_HEADER_LEN);
        pPmtBuffer[1] |= ((sectionLen >> 8) & 0x0F);
        pPmtBuffer[2] = (sectionLen & 0xFF);

        // Program Number
        pPmtBuffer[3] = (MMP_UINT8) (ptService->programNumber >> 8);
        pPmtBuffer[4] = (MMP_UINT8) (ptService->programNumber & 0xFF);
        // version - 0 (MPEG Audio), 1(AC3), 2(AAC) 3(DTS) , current_next_indicator - 1
        if (ptService->bUpdatePmtVersion)
        {
            ptService->pmtVersion = ((ptService->pmtVersion + 1) & 0x1F);
            ptService->bUpdatePmtVersion = MMP_FALSE;
        }
        pPmtBuffer[5] = (0xC1 | (ptService->pmtVersion << 1));
        //switch(ptService->audioStreamType)
        //{
        //    case MPEG_AUDIO:
        //        pPmtBuffer[5] = 0xC1;
        //        break;
        //    case DOLBY_AC3:
        //        pPmtBuffer[5] = 0xC3;
        //        break;
        //    case AAC:
        //        pPmtBuffer[5] = 0xC5;
        //        break;
        //    case DTS:
        //        pPmtBuffer[5] = 0xC7;
        //        break;
        //}
        // section number - 0, last section number - 0
        pPmtBuffer[6] = 0x0;
        pPmtBuffer[7] = 0x0;

        // reserved , PCR_PID
        pPmtBuffer[8] = (MMP_UINT8) (0xE0 | (ptService->videoPid >> 8));
        pPmtBuffer[9] = (MMP_UINT8) (ptService->videoPid & 0xFF);

        // reserved, program_info_length
        pPmtBuffer[10] = 0xF0;
        pPmtBuffer[11] = 0x0;
        // Video ES
        // H264
        pPmtBuffer[PMT_HEADER_LEN] = 0x1B;
        pPmtBuffer[PMT_HEADER_LEN + 1] = (MMP_UINT8) (0xE0 | (ptService->videoPid >> 8));
        pPmtBuffer[PMT_HEADER_LEN + 2] = (MMP_UINT8) (ptService->videoPid & 0xFF);
        pPmtBuffer[PMT_HEADER_LEN + 3] = 0xF0;
        pPmtBuffer[PMT_HEADER_LEN + 4] = 0x00;

        // Audio ES
        pPmtBuffer[PMT_HEADER_LEN + 5] = (MMP_UINT8) audioStreamType;
        pPmtBuffer[PMT_HEADER_LEN + 6] = (MMP_UINT8) (0xE0 | (ptService->audioPid >> 8));
        pPmtBuffer[PMT_HEADER_LEN + 7] = (MMP_UINT8) (ptService->audioPid & 0xFF);
        pPmtBuffer[PMT_HEADER_LEN + 8] = 0xF0;
        pPmtBuffer[PMT_HEADER_LEN + 9] = (MMP_UINT8) audioDescriptorSize;

        if (audioDescriptorSize)
        {
            PalMemcpy(&pPmtBuffer[PMT_HEADER_LEN + 10], pAudioDescriptor, audioDescriptorSize);
        }

        crc = CalcCRC32(pPmtBuffer, pmtLen - 4);
        pPmtBuffer[pmtLen - CRC_LEN] = (crc >> 24);
        pPmtBuffer[pmtLen - CRC_LEN + 1] = ((crc >> 16) & 0xFF);
        pPmtBuffer[pmtLen - CRC_LEN + 2] = ((crc >> 8) & 0xFF);
        pPmtBuffer[pmtLen - CRC_LEN + 3] = (crc & 0xFF);
#ifdef DEBUG_TABLE
        {
            MMP_UINT32 j = 0;
            dbg_msg(DBG_MSG_TYPE_INFO, "PMT TS Buffer Service: %u\n", i);
            for (j = 0; j < TS_PACKET_LEN; j++)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", pPmtTsPacketBuffer[j]);
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "\n\n");
        }
#endif
    }
}

static void
_PsiSiTableMgrGenerateSDT(
    void)
{
    // Seperate each service as one section
    MMP_UINT32 i = 0;
    MMP_INT32  pSectionLen[MAX_SERVICE_COUNT] = { 0 };
    MMP_UINT8* pSectionBuffer[MAX_SERVICE_COUNT] = { 0 };
    MMP_UINT8* pCurSectionBuffer = MMP_NULL;
    MMP_UINT32 descriptorLen = 0;
    MMP_INT32  totalSectionLen = 0;
    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;
    MMP_UINT32 totalTsCount = 0;
    MMP_UINT32 crc = 0;
    MMP_UINT8* pTsBufferPos = MMP_NULL;

    totalTsCount = 0;
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        totalSectionLen = SDT_HEADER_LEN;
        ptService = &gtPsiSiTableMgr.ptService[i];
        totalSectionLen += SERVICE_DESCRIPTOR_LEN;
        totalSectionLen += SDT_SERVICE_PRIOR_HEADER_LEN;
        totalSectionLen += ptService->providerNameLen;
        totalSectionLen += ptService->serviceNameLen;
        totalSectionLen += CRC_LEN;
        pSectionLen[i] = totalSectionLen;

        pSectionBuffer[i] = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, totalSectionLen);
        if (MMP_NULL == pSectionBuffer[i])
        {
            goto end;
        }

        pCurSectionBuffer = pSectionBuffer[i];
        PalMemset(pCurSectionBuffer, 0x0, totalSectionLen);
        PalMemcpy(pCurSectionBuffer, gpDefaultSdtHeader, SDT_HEADER_LEN);

        //Update section length
        pCurSectionBuffer[1] |= (((totalSectionLen - 3) >> 8) & 0x0F);
        pCurSectionBuffer[2] = ((totalSectionLen - 3) & 0xFF);

        //Update transport stream id
        pCurSectionBuffer[3] = ((gtPsiSiTableMgr.tsId >> 8) & 0xFF);
        pCurSectionBuffer[4] = (gtPsiSiTableMgr.tsId & 0xFF);

        //Update version
        if (gtPsiSiTableMgr.bUpdateSdtVersion)
        {         
            gtPsiSiTableMgr.sdtVersion = ((gtPsiSiTableMgr.sdtVersion + 1) & 0x1F);
            gtPsiSiTableMgr.bUpdateSdtVersion = MMP_FALSE;
        }
        pCurSectionBuffer[5] = (0xC1 | (gtPsiSiTableMgr.sdtVersion << 1));

        //Update section number
        pCurSectionBuffer[6] = i;
        pCurSectionBuffer[7] = (gtPsiSiTableMgr.serviceCount - 1);

        //Update original network id
        pCurSectionBuffer[8] = ((gtPsiSiTableMgr.onId >> 8) & 0xFF);
        pCurSectionBuffer[9] = (gtPsiSiTableMgr.onId & 0xFF);

        pCurSectionBuffer += SDT_HEADER_LEN;

        //service info
        pCurSectionBuffer[0] = ((ptService->programNumber >> 8) & 0xFF);
        pCurSectionBuffer[1] = (ptService->programNumber & 0xFF);
        // reserved_future_use + EIT_schedule_flag + EIT_present_following_flag
        pCurSectionBuffer[2] = 0xFC;

        //running_status + free_CA_mode
        pCurSectionBuffer[3] = 0x80;

        //descriptor loop len
        descriptorLen = SERVICE_DESCRIPTOR_LEN;
        descriptorLen += ptService->providerNameLen;
        descriptorLen += ptService->serviceNameLen;        
        pCurSectionBuffer[3] |= ((descriptorLen >> 8) & 0x0F);
        pCurSectionBuffer[4] = (descriptorLen & 0xFF);

        //service descriptor
        pCurSectionBuffer += SERVICE_DESCRIPTOR_LEN;
        pCurSectionBuffer[0] = 0x48;
        pCurSectionBuffer[1] = ((descriptorLen - 2) & 0xFF);
        pCurSectionBuffer[2] = (ptService->serviceType & 0xFF);
        pCurSectionBuffer[3] = (ptService->providerNameLen & 0xFF);
        if (ptService->providerNameLen)
        {
            PalMemcpy(&pCurSectionBuffer[4], ptService->pServiceProviderName, ptService->providerNameLen);
        }
        pCurSectionBuffer += (4 + ptService->providerNameLen);
        pCurSectionBuffer[0] = (ptService->serviceNameLen & 0xFF);
        if (ptService->serviceNameLen)
        {
            PalMemcpy(&pCurSectionBuffer[1], ptService->pServiceName, ptService->serviceNameLen);
        }
        pCurSectionBuffer += (1 + ptService->serviceNameLen);

        crc = CalcCRC32(pSectionBuffer[i], totalSectionLen - CRC_LEN);
        pCurSectionBuffer[0] = ((crc >> 24) & 0xFF);
        pCurSectionBuffer[1] = ((crc >> 16) & 0xFF);
        pCurSectionBuffer[2] = ((crc >> 8) & 0xFF);
        pCurSectionBuffer[3] = (crc & 0xFF);

        // max packet payload of first packet is 183 bytes
        totalTsCount++;
        totalSectionLen -= (TS_MAX_PAYLOAD - POINTER_FIELD_LEN);
        if (totalSectionLen > 0)
        {
            totalTsCount += (totalSectionLen / TS_MAX_PAYLOAD);
            if (totalSectionLen % TS_MAX_PAYLOAD)
            {
                totalTsCount++;
            }
        }
    }

#ifdef DEBUG_TABLE
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        if (pSectionBuffer[i] && pSectionLen[i])
        {
            MMP_UINT32 j = 0;
            MMP_UINT8* pBuffer = pSectionBuffer[i];
            dbg_msg(DBG_MSG_TYPE_INFO, "SDT section: %u\n", i);
            for (j = 0; j < pSectionLen[i]; j++)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", pBuffer[j]);
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "\n\n");
        }
    }
#endif


    if (totalTsCount)
    {
        if (gtPsiSiTableMgr.pTsSdtBuffer)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.pTsSdtBuffer);
            gtPsiSiTableMgr.pTsSdtBuffer = MMP_NULL;
            gtPsiSiTableMgr.sdtBufferLen = MMP_NULL;
        }

        gtPsiSiTableMgr.sdtBufferLen = totalTsCount * TS_PACKET_LEN;
        gtPsiSiTableMgr.pTsSdtBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.sdtBufferLen);
        if (MMP_NULL == gtPsiSiTableMgr.pTsSdtBuffer || 0 == gtPsiSiTableMgr.sdtBufferLen)
        {
            goto end;
        }
        pTsBufferPos = gtPsiSiTableMgr.pTsSdtBuffer;

        for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
        {
            MMP_BOOL bFirstPacket = MMP_TRUE;
            MMP_INT32 remainSectionLen = pSectionLen[i];
            ptService = &gtPsiSiTableMgr.ptService[i];
            pCurSectionBuffer = pSectionBuffer[i];

            while (remainSectionLen > 0)
            {
                MMP_INT32 stuffLen = 0;
                MMP_INT32 payloadLen = 0;

                if (bFirstPacket)
                {
                    if (remainSectionLen >= (TS_MAX_PAYLOAD - POINTER_FIELD_LEN))
                    {
                        payloadLen = (TS_MAX_PAYLOAD - POINTER_FIELD_LEN);
                    }
                    else
                    {
                        payloadLen = remainSectionLen;
                        stuffLen = TS_MAX_PAYLOAD - POINTER_FIELD_LEN - payloadLen;
                    }
                    PalMemcpy(pTsBufferPos, gpTsDefaultStartIndHeader, TS_PRIOR_HEADER);
                    Assign_PID(pTsBufferPos, SDT_PID);
                    // pointer field
                    pTsBufferPos[TS_PRIOR_HEADER] = 0x0;
                    
                    //SDT section start
                    PalMemcpy(&pTsBufferPos[TS_PRIOR_HEADER + POINTER_FIELD_LEN], pCurSectionBuffer, payloadLen);
                    if (stuffLen)
                    {
                        PalMemset(&pTsBufferPos[TS_PRIOR_HEADER + POINTER_FIELD_LEN + payloadLen], 0xFF, stuffLen);
                    }
                    bFirstPacket = MMP_FALSE;
                }
                else
                {
                    if (remainSectionLen >= TS_MAX_PAYLOAD)
                    {
                        payloadLen = TS_MAX_PAYLOAD;
                    }
                    else
                    {
                        payloadLen = remainSectionLen;
                        stuffLen = TS_MAX_PAYLOAD - payloadLen;
                    }
                    PalMemcpy(pTsBufferPos, gpTsDefaultHeader, TS_PRIOR_HEADER);
                    Assign_PID(pTsBufferPos, SDT_PID);
                    PalMemcpy(&pTsBufferPos[TS_PRIOR_HEADER], pCurSectionBuffer, payloadLen);
                    if (stuffLen)
                    {
                        PalMemset(&pTsBufferPos[TS_PRIOR_HEADER + payloadLen], 0xFF, stuffLen);
                    }
                }

                remainSectionLen -= payloadLen;
                pCurSectionBuffer += payloadLen;
                pTsBufferPos += TS_PACKET_LEN;
            }
        }
    }
#ifdef DEBUG_TABLE
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "SDT TS Buffer\n");
        MMP_UINT32 j = 0;
        for (i = 0; i < gtPsiSiTableMgr.sdtBufferLen; i++, j++)
        {
            if (j == TS_PACKET_LEN)
            {
                j = 0;
                dbg_msg(DBG_MSG_TYPE_INFO, "\n");
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", gtPsiSiTableMgr.pTsSdtBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n\n");
    }
#endif
end:
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        if (pSectionBuffer[i])
        {
            PalHeapFree(PAL_HEAP_DEFAULT, pSectionBuffer[i]);
        }
    }
}

static void
_PsiSiTableMgrGenerateNIT(
    void)
{
    // Only one section of NIT
    MMP_UINT8* pNitTsPacketBuffer = MMP_NULL;
    MMP_UINT8* pNitBuffer = MMP_NULL;
    MMP_INT32  nitLen = 0;
    MMP_UINT32 stuffLen = 0;
    MMP_UINT32 crc = 0;
    MMP_UINT32 i = 0;
    MMP_UINT8* pCurSectionBuffer = MMP_NULL;
    MMP_UINT32 value = 0;
    MMP_UINT32 transportStreamLoopLen = 0;
    MMP_UINT32 lcnRelatedDescriptorLen = 0;
    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;
    MMP_UINT32 totalTsCount = 0;
    MMP_BOOL bFirstPacket = MMP_TRUE;
    MMP_INT32 remainSectionLen = 0;
    MMP_INT32 totalSectionLen = 0;
    MMP_UINT8* pTsBufferPos = MMP_NULL;

    nitLen = NIT_HEADER_LEN + CRC_LEN;

    // first loop
    // name descriptor
    if (gtPsiSiTableMgr.networkNameLen)
    {
        nitLen += (DESCRIPTOR_HEADER_LEN + gtPsiSiTableMgr.networkNameLen);
    }

    // second loop
    // terrestrial_delivery_system_descriptor + service_list_descriptor
    nitLen += TERRESTRIAL_DELIVERY_SYSTEM_LEN;
    nitLen += DESCRIPTOR_HEADER_LEN + (SERVICE_LIST_CONTENT_LEN * gtPsiSiTableMgr.serviceCount);
    // private data specifier descriptor + logical channel number descriptor
    switch (gtPsiSiTableMgr.lcnType)
    {
        case NO_LCN:
            lcnRelatedDescriptorLen = 0;
            break;
        case LCN_E_BOOK:
        case LCN_NORDIG:
            lcnRelatedDescriptorLen = PDSD_DESCRIPTOR_LEN;
            lcnRelatedDescriptorLen += (DESCRIPTOR_HEADER_LEN + LCN_CONTENT_LEN * gtPsiSiTableMgr.serviceCount);
            break;
    }
    
    nitLen += lcnRelatedDescriptorLen;

    pNitBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, nitLen);
    if (MMP_NULL == pNitBuffer)
    {
        return;
    }
    PalMemset(pNitBuffer, 0x0, nitLen);
    pCurSectionBuffer = pNitBuffer;
    PalMemcpy(pCurSectionBuffer, gpDefaultNitHeader, NIT_HEADER_LEN);

    // section length
    pCurSectionBuffer[1] |= (((nitLen - 3) >> 8) & 0x0F);
    pCurSectionBuffer[2] = ((nitLen - 3) & 0xFF);
    
    // network id
    pCurSectionBuffer[3] = ((gtPsiSiTableMgr.networkId >> 8) & 0xFF);
    pCurSectionBuffer[4] = (gtPsiSiTableMgr.networkId & 0xFF);
    
    //Update version
    if (gtPsiSiTableMgr.bUpdateNitVersion)
    {         
        gtPsiSiTableMgr.nitVersion = ((gtPsiSiTableMgr.nitVersion + 1) & 0x1F);
        gtPsiSiTableMgr.bUpdateNitVersion = MMP_FALSE;
    }
    pCurSectionBuffer[5] = (0xC1 | (gtPsiSiTableMgr.nitVersion << 1));

    // first loop network_descriptor_length
    if (gtPsiSiTableMgr.networkNameLen)
    {
        pCurSectionBuffer[8] |= (((DESCRIPTOR_HEADER_LEN + gtPsiSiTableMgr.networkNameLen) >> 8) & 0x0F);
        pCurSectionBuffer[9] = ((DESCRIPTOR_HEADER_LEN + gtPsiSiTableMgr.networkNameLen) & 0xFF);
        // network name descriptor
        pCurSectionBuffer[10] = 0x40;
        pCurSectionBuffer[11] = gtPsiSiTableMgr.networkNameLen;
        PalMemcpy(&pCurSectionBuffer[12], gtPsiSiTableMgr.pNetworkName, gtPsiSiTableMgr.networkNameLen);
        pCurSectionBuffer += (10 + (DESCRIPTOR_HEADER_LEN + gtPsiSiTableMgr.networkNameLen));
    }
    else
    {
        pCurSectionBuffer[9] = 0x0;
        pCurSectionBuffer += 10;
    }

    // reserved_future_use + transport_stream_loop_length
    pCurSectionBuffer[0] = 0xF0;
    transportStreamLoopLen = 6 + TERRESETRIAL_DELIVERY_SYSTEM_DESCRIPTOR_LEN 
                          + (DESCRIPTOR_HEADER_LEN + SERVICE_LIST_CONTENT_LEN * gtPsiSiTableMgr.serviceCount) 
                          + lcnRelatedDescriptorLen;
    pCurSectionBuffer[0] |= ((transportStreamLoopLen >> 8) & 0x0F);
    pCurSectionBuffer[1] = (transportStreamLoopLen & 0xFF);

    // transport stream info
    pCurSectionBuffer[2] = ((gtPsiSiTableMgr.tsId >> 8) & 0xFF);
    pCurSectionBuffer[3] = (gtPsiSiTableMgr.tsId & 0xFF);

    pCurSectionBuffer[4] = ((gtPsiSiTableMgr.onId >> 8) & 0xFF);
    pCurSectionBuffer[5] = (gtPsiSiTableMgr.onId & 0xFF);

    // transport_descriptors_length
    pCurSectionBuffer[6] = 0xF0;
    pCurSectionBuffer[6] |= (((transportStreamLoopLen - 6) >> 8) & 0x0F);
    pCurSectionBuffer[7] = ((transportStreamLoopLen - 6) & 0xFF);

    // second loop
    pCurSectionBuffer += 8;
    // terrestrial delivery system descriptor
    pCurSectionBuffer[0] = 0x5A;
    pCurSectionBuffer[1] = 11;
    // frequency 10Hz base, input is Khz base
    value = gtPsiSiTableMgr.frequency * 100;
    pCurSectionBuffer[2] = (value >> 24) & 0xFF;
    pCurSectionBuffer[3] = (value >> 16) & 0xFF;
    pCurSectionBuffer[4] = (value >> 8) & 0xFF;
    pCurSectionBuffer[5] = value & 0xFF; 

    // Bandwidth
    switch (gtPsiSiTableMgr.bandwidth)
    {
        case 5000:
            value = TS_5M_BANDWIDTH;
            break;
        case 6000:
            value = TS_6M_BANDWIDTH;
            break;
        case 7000:
            value = TS_7M_BANDWIDTH;
            break;
        case 8000:
            value = TS_8M_BANDWIDTH;
            break;
        default:
            value = TS_OTHER_BANDWIDTH;
            break;
    }

    //priority + Time_Slicing_indicator + MPE-FEC_indicator + reserved_future_used
    pCurSectionBuffer[6] = 0x1F;
    // bandwidth
    pCurSectionBuffer[6] |= ((value << 5) & 0xFF);

    // constellation
    switch (gtPsiSiTableMgr.constellation)
    {
        case CONSTELATTION_QPSK:
            value = TS_QPSK;
            break;
        case CONSTELATTION_16QAM:
            value = TS_16QAM;
            break;
        case CONSTELATTION_64QAM:
        default:
            value = TS_64QAM;
            break;
    }
    pCurSectionBuffer[7] = 0;
    // constellation
    pCurSectionBuffer[7] |= ((value << 6) & 0xC0);
    // code rate
    switch (gtPsiSiTableMgr.codeRate)
    {
        case CODE_RATE_1_2:
            value = TS_CODE_RATE_1_2;
            break;
        case CODE_RATE_2_3:
            value = TS_CODE_RATE_2_3;
            break;
        case CODE_RATE_3_4:
            value = TS_CODE_RATE_3_4;
            break;
        case CODE_RATE_5_6:
            value = TS_CODE_RATE_5_6;
            break;
        case CODE_RATE_7_8:
            value = TS_CODE_RATE_7_8;
            break;
        default:
            value = TS_OTHER_CODE_RATE;
            break;
    }
    // code_rate-HP_stream
    pCurSectionBuffer[7] |= (value & 0x7);
    // code_rate-LP_stream
    pCurSectionBuffer[8] = 0;
    pCurSectionBuffer[8] |= ((value << 5) & 0xE0);
    
    // guard_interval
    switch (gtPsiSiTableMgr.guardInterval)
    {
        case GUARD_INTERVAL_1_4:
            value = TS_GUARD_INTERVAL_1_4;
            break;
        case GUARD_INTERVAL_1_8:
            value = TS_GUARD_INTERVAL_1_8;
            break;
        case GUARD_INTERVAL_1_16:
            value = TS_GUARD_INTERVAL_1_16;
            break;
        case GUARD_INTERVAL_1_32:
        default:
            value = TS_GUARD_INTERVAL_1_32;
            break;
    }
    pCurSectionBuffer[8] |= ((value << 3) & 0x18);

    // transmission_mode - fixed 8K + other_frequency_flag
    pCurSectionBuffer[8] |= 0x2;
    
    // reserved_future_used
    pCurSectionBuffer[9] = pCurSectionBuffer[10] = pCurSectionBuffer[11] = pCurSectionBuffer[12] = 0xFF;

    // service_list_descriptor
    pCurSectionBuffer += TERRESETRIAL_DELIVERY_SYSTEM_DESCRIPTOR_LEN;
    pCurSectionBuffer[0] = 0x41;
    pCurSectionBuffer[1] = ((SERVICE_LIST_CONTENT_LEN * gtPsiSiTableMgr.serviceCount) & 0xFF);
    pCurSectionBuffer += DESCRIPTOR_HEADER_LEN;
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        ptService = &gtPsiSiTableMgr.ptService[i];
        pCurSectionBuffer[0] = ((ptService->programNumber >> 8) & 0xFF);
        pCurSectionBuffer[1] = (ptService->programNumber & 0xFF);
        pCurSectionBuffer[2] = (ptService->serviceType & 0xFF);
        pCurSectionBuffer += SERVICE_LIST_CONTENT_LEN;
    }
    //LCN related descriptors
    if (lcnRelatedDescriptorLen)
    {
        //private_data_specifier_descriptor
        pCurSectionBuffer[0] = 0x5F;
        pCurSectionBuffer[1] = 4;
        pCurSectionBuffer[2] = ((gtPsiSiTableMgr.pdsd >> 24) & 0xFF);
        pCurSectionBuffer[3] = ((gtPsiSiTableMgr.pdsd >> 16) & 0xFF);
        pCurSectionBuffer[4] = ((gtPsiSiTableMgr.pdsd >> 8) & 0xFF);
        pCurSectionBuffer[5] = (gtPsiSiTableMgr.pdsd & 0xFF);
        
        pCurSectionBuffer += PDSD_DESCRIPTOR_LEN;
        //logical_channel_descriptor
        pCurSectionBuffer[0] = 0x83;
        pCurSectionBuffer[1] = ((LCN_CONTENT_LEN * gtPsiSiTableMgr.serviceCount) & 0xFF);
        pCurSectionBuffer += DESCRIPTOR_HEADER_LEN;
        for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
        {
            ptService = &gtPsiSiTableMgr.ptService[i];
            pCurSectionBuffer[0] = ((ptService->programNumber >> 8) & 0xFF);
            pCurSectionBuffer[1] = (ptService->programNumber & 0xFF);
    
            switch(gtPsiSiTableMgr.lcnType)
            {
            case LCN_E_BOOK:
                pCurSectionBuffer[2] = 0xF8;
                pCurSectionBuffer[2] |= ((ptService->lcn >> 8) & 0x3);
                pCurSectionBuffer[3] = (ptService->lcn & 0xFF);
                break;
            case LCN_NORDIG:
                pCurSectionBuffer[2] = 0xC0;
                pCurSectionBuffer[2] |= ((ptService->lcn >> 8) & 0x3F);
                pCurSectionBuffer[3] = (ptService->lcn & 0xFF);
                break;
            }
            pCurSectionBuffer += LCN_CONTENT_LEN;
        }
    }
    
        crc = CalcCRC32(pNitBuffer, nitLen - CRC_LEN);
        pCurSectionBuffer[0] = ((crc >> 24) & 0xFF);
        pCurSectionBuffer[1] = ((crc >> 16) & 0xFF);
        pCurSectionBuffer[2] = ((crc >> 8) & 0xFF);
        pCurSectionBuffer[3] = (crc & 0xFF);

#ifdef DEBUG_TABLE
        if (pNitBuffer && nitLen)
        {
            MMP_UINT32 j = 0;
            MMP_UINT8* pBuffer = pNitBuffer;
            dbg_msg(DBG_MSG_TYPE_INFO, "NIT section\n");
            for (j = 0; j < nitLen; j++)
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", pBuffer[j]);
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "\n\n");
        }
#endif

    totalTsCount = 1;
    // max packet payload of first packet is 183 bytes
    totalSectionLen = nitLen;
    totalSectionLen -= (TS_MAX_PAYLOAD - POINTER_FIELD_LEN);
    
    if (totalSectionLen > 0)
    {
        totalTsCount += (totalSectionLen / TS_MAX_PAYLOAD);
        if (totalSectionLen % TS_MAX_PAYLOAD)
        {
            totalTsCount++;
        }
    }

    if (totalTsCount)
    {
        if (gtPsiSiTableMgr.pTsNitBuffer)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.pTsNitBuffer);
            gtPsiSiTableMgr.pTsNitBuffer = MMP_NULL;
            gtPsiSiTableMgr.nitBufferLen = MMP_NULL;
        }

        gtPsiSiTableMgr.nitBufferLen = totalTsCount * TS_PACKET_LEN;
        gtPsiSiTableMgr.pTsNitBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.nitBufferLen);
        if (MMP_NULL == gtPsiSiTableMgr.pTsNitBuffer || 0 == gtPsiSiTableMgr.nitBufferLen)
        {
            goto end;
        }

        pTsBufferPos = gtPsiSiTableMgr.pTsNitBuffer;
        remainSectionLen = nitLen;
        pCurSectionBuffer = pNitBuffer;
        while (remainSectionLen > 0)
        {
            MMP_INT32 stuffLen = 0;
            MMP_INT32 payloadLen = 0;

            if (bFirstPacket)
            {
                if (remainSectionLen >= (TS_MAX_PAYLOAD - POINTER_FIELD_LEN))
                {
                    payloadLen = (TS_MAX_PAYLOAD - POINTER_FIELD_LEN);
                }
                else
                {
                    payloadLen = remainSectionLen;
                    stuffLen = TS_MAX_PAYLOAD - POINTER_FIELD_LEN - payloadLen;
                }
                PalMemcpy(pTsBufferPos, gpTsDefaultStartIndHeader, TS_PRIOR_HEADER);
                Assign_PID(pTsBufferPos, NIT_PID);
                // pointer field
                pTsBufferPos[TS_PRIOR_HEADER] = 0x0;
                
                //NIT section start
                PalMemcpy(&pTsBufferPos[TS_PRIOR_HEADER + POINTER_FIELD_LEN], pCurSectionBuffer, payloadLen);
                if (stuffLen)
                {
                    PalMemset(&pTsBufferPos[TS_PRIOR_HEADER + POINTER_FIELD_LEN + payloadLen], 0xFF, stuffLen);
                }
                bFirstPacket = MMP_FALSE;
            }
            else
            {
                if (remainSectionLen >= TS_MAX_PAYLOAD)
                {
                    payloadLen = TS_MAX_PAYLOAD;
                }
                else
                {
                    payloadLen = remainSectionLen;
                    stuffLen = TS_MAX_PAYLOAD - payloadLen;
                }
                PalMemcpy(pTsBufferPos, gpTsDefaultHeader, TS_PRIOR_HEADER);
                Assign_PID(pTsBufferPos, NIT_PID);
                PalMemcpy(&pTsBufferPos[TS_PRIOR_HEADER], pCurSectionBuffer, payloadLen);
                if (stuffLen)
                {
                    PalMemset(&pTsBufferPos[TS_PRIOR_HEADER + payloadLen], 0xFF, stuffLen);
                }
            }

            remainSectionLen -= payloadLen;
            pCurSectionBuffer += payloadLen;
            pTsBufferPos += TS_PACKET_LEN;
        }
    }
#ifdef DEBUG_TABLE
    {
        MMP_UINT32 j = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "NIT TS Buffer\n");
        for (i = 0; i < gtPsiSiTableMgr.nitBufferLen; i++, j++)
        {
            if (j == TS_PACKET_LEN)
            {
                j = 0;
                dbg_msg(DBG_MSG_TYPE_INFO, "\n");
            }
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", gtPsiSiTableMgr.pTsNitBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n\n");
    }
#endif
end:
    if (pNitBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, pNitBuffer);
    }
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

MMP_BOOL
PsiSiMgrInit(
    void)
{
    if (gtPsiSiTableMgr.pMutex)
    {
        return MMP_TRUE;
    }
    PalMemset(&gtPsiSiTableMgr, 0x0, sizeof(PSI_SI_TABLE_MGR));

    gtPsiSiTableMgr.pMutex = PalCreateMutex(0);
    if (MMP_NULL == gtPsiSiTableMgr.pMutex)
    {
        return MMP_FALSE;
    }
    _PsiSiTableMgrLock();
    _PsiSiTableMgrDefaultSetting();
    _PsiSiTableMgrUnLock();
}

void
PsiSiMgrTerminate(
    void)
{
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;

    _PsiSiTableMgrLock();

    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        ptService = &gtPsiSiTableMgr.ptService[i];
        if (ptService->pServiceProviderName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptService->pServiceProviderName);
        }
        if (ptService->pServiceName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptService->pServiceName);
        }
    }

    if (gtPsiSiTableMgr.pNetworkName)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.pNetworkName);
    }

    if (gtPsiSiTableMgr.pTsSdtBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.pTsSdtBuffer);
    }

    if (gtPsiSiTableMgr.pTsNitBuffer)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.pTsNitBuffer);
    }

    _PsiSiTableMgrUnLock();
    if (gtPsiSiTableMgr.pMutex)
    {
        PalDestroyMutex(gtPsiSiTableMgr.pMutex);
        gtPsiSiTableMgr.pMutex = MMP_NULL;
    }
    
    PalMemset(&gtPsiSiTableMgr, 0x0, sizeof(PSI_SI_TABLE_MGR));
}

MMP_BOOL
PsiSiMgrUpdateTSID(
    MMP_UINT32 tsId)
{
    if (tsId > MAX_TS_ID)
    {
        return MMP_FALSE;
    }

    _PsiSiTableMgrLock();
    gtPsiSiTableMgr.tsId = tsId;
    gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
    gtPsiSiTableMgr.bUpdateSdtVersion = MMP_TRUE;
    gtPsiSiTableMgr.bUpdatePatVersion = MMP_TRUE;    
    _PsiSiTableMgrUnLock();
    return MMP_TRUE;
}

MMP_BOOL
PsiSiMgrUpdateNetworkName(
    MMP_UINT8* pNetworkName,
    MMP_UINT32 nameLen)
{
    MMP_BOOL bResult = MMP_TRUE;
    if (nameLen > MAX_NAME_LEN)
    {
        bResult = MMP_FALSE;
    }
    else
    {
        _PsiSiTableMgrLock();
        if (gtPsiSiTableMgr.pNetworkName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.pNetworkName);
            gtPsiSiTableMgr.pNetworkName = MMP_NULL;
            gtPsiSiTableMgr.networkNameLen = 0;
        }
        gtPsiSiTableMgr.networkNameLen = nameLen;
        if (gtPsiSiTableMgr.networkNameLen)
        {
            gtPsiSiTableMgr.pNetworkName = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, gtPsiSiTableMgr.networkNameLen);
            if (gtPsiSiTableMgr.pNetworkName)
            {
                PalMemcpy(gtPsiSiTableMgr.pNetworkName, pNetworkName, gtPsiSiTableMgr.networkNameLen);
                gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
            }
            else
            {
                gtPsiSiTableMgr.networkNameLen = 0;
                bResult = MMP_FALSE;
            }
        }
        _PsiSiTableMgrUnLock();
    }
    return bResult;
}

MMP_BOOL
PsiSiMgrUpdateNetworkId(
    MMP_UINT32 networkId)
{
    if (networkId > MAX_NETWORK_ID)
    {
        return MMP_FALSE;
    }
    _PsiSiTableMgrLock();
    gtPsiSiTableMgr.networkId = networkId;
    gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
    _PsiSiTableMgrUnLock();
    return MMP_TRUE;
}

MMP_BOOL
PsiSiMgrUpdateONID(
    MMP_UINT32 onId)
{
    if (onId > MAX_NETWORK_ID)
    {
        return MMP_FALSE;
    }
    _PsiSiTableMgrLock();
    gtPsiSiTableMgr.onId = onId;
    gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
    _PsiSiTableMgrUnLock();
    return MMP_TRUE;
}

MMP_BOOL
PsiSiMgrUpdateServiceListDescriptor(
    MMP_UINT32 serviceId,
    MMP_UINT32 serviceType)
{
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;
    MMP_BOOL bResult = MMP_FALSE;
    if (serviceType > MAX_SERVICE_TYPE)
    {
        return MMP_FALSE;
    }

    _PsiSiTableMgrLock();
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        if (gtPsiSiTableMgr.ptService[i].programNumber == serviceId)
        {
            ptHitService = &gtPsiSiTableMgr.ptService[i];
            break;
        }
    }
    
    if (ptHitService)
    {
        ptHitService->serviceType = serviceType;
        gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
        bResult = MMP_TRUE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}
    
MMP_BOOL
PsiSiMgrUpdateCountryId(
    CORE_COUNTRY_ID countryId)
{
    if (countryId < CORE_COUNTRY_LAST_ID)
    {
        _PsiSiTableMgrLock();
        gtPsiSiTableMgr.countryId = countryId;
        switch (countryId)
        {
            case CORE_COUNTRY_TAIWAN:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3301;
                gtPsiSiTableMgr.onId = 0x209E;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_DENMARK:
                gtPsiSiTableMgr.lcnType = LCN_NORDIG;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x20D0;
                gtPsiSiTableMgr.pdsd = 0x29;
                break;
            case CORE_COUNTRY_FINLAND:
                gtPsiSiTableMgr.lcnType = LCN_NORDIG;
                gtPsiSiTableMgr.networkId = 0x3301;
                gtPsiSiTableMgr.onId = 0x20F6;
                gtPsiSiTableMgr.pdsd = 0x29;
                break;
            case CORE_COUNTRY_NORWAY:
                gtPsiSiTableMgr.lcnType = LCN_NORDIG;
                gtPsiSiTableMgr.networkId = 0x3401;
                gtPsiSiTableMgr.onId = 0x2242;
                gtPsiSiTableMgr.pdsd = 0x29;
                break;
            case CORE_COUNTRY_SWEDEN:
                gtPsiSiTableMgr.lcnType = LCN_NORDIG;
                gtPsiSiTableMgr.networkId = 0x3101;
                gtPsiSiTableMgr.onId = 0x22F1;
                gtPsiSiTableMgr.pdsd = 0x29;
                break;
            case CORE_COUNTRY_GERMANY:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3001;
                gtPsiSiTableMgr.onId = 0x2114;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_UK:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3001;
                gtPsiSiTableMgr.onId = 0x233A;
                gtPsiSiTableMgr.pdsd = 0x233A;
                break;
            case CORE_COUNTRY_ITALY:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3001;
                gtPsiSiTableMgr.onId = 0x217C;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_AUSTRALIA:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x2024;
                gtPsiSiTableMgr.pdsd = 0x3200;
                break;
            case CORE_COUNTRY_NEW_ZEALAND:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3401;
                gtPsiSiTableMgr.onId = 0x222A;
                gtPsiSiTableMgr.pdsd = 0x37;
                break;
            case CORE_COUNTRY_FRANCE:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3301;
                gtPsiSiTableMgr.onId = 0x20FA;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_SPAIN:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3101;
                gtPsiSiTableMgr.onId = 0xA600;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_POLAND:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3401;
                gtPsiSiTableMgr.onId = 0x2268;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_CZECH:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3101;
                gtPsiSiTableMgr.onId = 0x20CB;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_NETHERLANDS:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3101;
                gtPsiSiTableMgr.onId = 0x2210;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_GREECE:
                // Can't find any network information of GREECE yet.
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3001;
                gtPsiSiTableMgr.onId = 0x3001;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_RUSSIA:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3501;
                gtPsiSiTableMgr.onId = 0x2283;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_SWITZERLAND:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x22F4;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_SLOVAK:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x22C1;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_SLOVENIA:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x22F4;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_HUNGARY:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3401;
                gtPsiSiTableMgr.onId = 0x3401;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_AUSTRIA:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3301;
                gtPsiSiTableMgr.onId = 0x2028;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_LATIVA:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3001;
                gtPsiSiTableMgr.onId = 0x21AC;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_ISRAEL:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x1111;
                gtPsiSiTableMgr.onId = 0xFF22;
                gtPsiSiTableMgr.pdsd = 0;
                break;
            case CORE_COUNTRY_CROATIA:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3101;
                gtPsiSiTableMgr.onId = 0x20BF;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_ESTONIA:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x20E9;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_PORTUGAL:
                gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
                gtPsiSiTableMgr.networkId = 0x3401;
                gtPsiSiTableMgr.onId = 0x22C8;
                gtPsiSiTableMgr.pdsd = 0x28;
                break;
            case CORE_COUNTRY_IRELAND:
                gtPsiSiTableMgr.lcnType = LCN_NORDIG;
                gtPsiSiTableMgr.networkId = 0x3201;
                gtPsiSiTableMgr.onId = 0x2174;
                gtPsiSiTableMgr.pdsd = 0x29;
                break;
            default:
                gtPsiSiTableMgr.lcnType = NO_LCN;
                gtPsiSiTableMgr.networkId = 0x3301;
                gtPsiSiTableMgr.onId = 0x209E;
                gtPsiSiTableMgr.pdsd = 0;
                break;
        }
        gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
        _PsiSiTableMgrUnLock();
        return MMP_TRUE;
    }
    else
    {
        return MMP_FALSE;
    }
}

MMP_BOOL
PsiSiMgrUpdateLCN(
    MMP_UINT32 serviceId,
    MMP_UINT32 lcn)
{
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;
    MMP_BOOL bResult = MMP_TRUE;

    _PsiSiTableMgrLock();
    switch (gtPsiSiTableMgr.lcnType)
    {
        case NO_LCN:
        {
            gtPsiSiTableMgr.lcnType = LCN_E_BOOK;
            gtPsiSiTableMgr.pdsd = 0x28;
            if (lcn > MAX_E_BOOK_LCN)
            {
                bResult = MMP_FALSE;
                goto end;
            }
            break;
        }
        case LCN_E_BOOK:
        {
            if (lcn > MAX_E_BOOK_LCN)
            {
                bResult = MMP_FALSE;
                goto end;
            }
            break;
        }
        case LCN_NORDIG:
        {
            if (lcn > MAX_NORDIC_LCN)
            {
                bResult = MMP_FALSE;
                goto end;
            }
            break;
        }
        default:
        {
            bResult = MMP_FALSE;
            goto end;
        }
    }

    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        if (gtPsiSiTableMgr.ptService[i].programNumber == serviceId)
        {
            ptHitService = &gtPsiSiTableMgr.ptService[i];
            break;
        }
    }
    
    if (ptHitService)
    {
        ptHitService->lcn = lcn;
        gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
    }
    else
    {
        bResult = MMP_FALSE;
    }
end:
    _PsiSiTableMgrUnLock();
    return bResult;
}


MMP_BOOL
PsiSiMgrUpdateModulationParameter(
    MMP_UINT32          frequency,
    MMP_UINT32          bandwidth,
    CONSTELLATION_MODE  constellation,
    CODE_RATE_MODE      codeRate,
    GUARD_INTERVAL_MODE guardInterval)
{
    if (frequency < MIN_FREQUENCY || frequency > MAX_FREQUENCY)
    {
        return MMP_FALSE;
    }
    
    if (bandwidth != 6000 && bandwidth != 7000 && bandwidth != 8000)
    {
        return MMP_FALSE;
    }
    
    if (constellation > CONSTELATTION_64QAM)
    {
        return MMP_FALSE;
    }

    if (codeRate > CODE_RATE_7_8)
    {
        return MMP_FALSE;
    }
    
    if (guardInterval > GUARD_INTERVAL_1_32)
    {
        return MMP_FALSE;
    }

    _PsiSiTableMgrLock();
    gtPsiSiTableMgr.frequency = frequency;
    gtPsiSiTableMgr.bandwidth = bandwidth;
    gtPsiSiTableMgr.constellation = constellation;
    gtPsiSiTableMgr.codeRate = codeRate;
    gtPsiSiTableMgr.guardInterval = guardInterval;
    gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
    _PsiSiTableMgrUnLock();
    return MMP_TRUE;
}

MMP_BOOL
PsiSiMgrGenerateTablePacket(
    void)
{
    _PsiSiTableMgrGeneratePAT();
    _PsiSiTableMgrGeneratePMT();
    _PsiSiTableMgrGenerateSDT();
    _PsiSiTableMgrGenerateNIT();
}

MMP_BOOL
PsiSiMgrInsertService(
    MMP_UINT16      programNumber,
    MMP_UINT16      pmtPid,
    MMP_UINT16      videoPid,
    MMP_UINT16      videoStreamType,
    MMP_UINT16      audioPid,
    MMP_UINT16      audioStreamType,
    MMP_UINT8*      pServiceProviderName,
    MMP_UINT32      providerNameLen,
    MMP_UINT8*      pServiceName,
    MMP_UINT32      serviceNameLen)
{
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;
    MMP_BOOL bResult = MMP_TRUE;

    _PsiSiTableMgrLock();
    if (gtPsiSiTableMgr.serviceCount >= MAX_SERVICE_COUNT)
    {
        bResult = MMP_FALSE;
        goto end;
    }

    if (providerNameLen > MAX_NAME_LEN || serviceNameLen > MAX_NAME_LEN ||
        (providerNameLen + serviceNameLen) > (MAX_DESCRIPTOR_LEN - 3))
    {
        bResult = MMP_FALSE;
        goto end;
    }

    ptHitService = &gtPsiSiTableMgr.ptService[gtPsiSiTableMgr.serviceCount];
    if (ptHitService->pServiceProviderName)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptHitService->pServiceProviderName);
        ptHitService->pServiceProviderName = MMP_NULL;
    }
    if (ptHitService->pServiceName)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptHitService->pServiceName);
        ptHitService->pServiceName = MMP_NULL;
    }
    ptHitService->programNumber =  programNumber;
    ptHitService->pmtPid = pmtPid;
    ptHitService->videoPid = videoPid;
    ptHitService->videoStreamType = videoStreamType;
    ptHitService->audioPid = audioPid;
    ptHitService->audioStreamType = audioStreamType;
    // service_type: digital television service
    ptHitService->serviceType = 0x1;

    ptHitService->providerNameLen = providerNameLen;
    if (ptHitService->providerNameLen)
    {
        ptHitService->pServiceProviderName = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptHitService->providerNameLen);
        if (MMP_NULL == ptHitService->pServiceProviderName)
        {
            bResult = MMP_FALSE;
            ptHitService->providerNameLen = 0;
            goto end;
        }
        PalMemcpy(ptHitService->pServiceProviderName, pServiceProviderName, ptHitService->providerNameLen);
    }

    ptHitService->serviceNameLen = serviceNameLen;
    if (ptHitService->serviceNameLen)
    {
        ptHitService->pServiceName = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptHitService->serviceNameLen);
        if (MMP_NULL == ptHitService->pServiceName)
        {
            if (ptHitService->pServiceProviderName)
            {
                PalHeapFree(PAL_HEAP_DEFAULT, ptHitService->pServiceProviderName);
                ptHitService->pServiceProviderName = MMP_NULL;
                ptHitService->providerNameLen = 0;
            }
            ptHitService->serviceNameLen = 0;
            bResult = MMP_FALSE;
            goto end;
        }
        PalMemcpy(ptHitService->pServiceName, pServiceName, ptHitService->serviceNameLen);
    }

    ptHitService->bUpdatePmtVersion = MMP_TRUE;
    gtPsiSiTableMgr.bUpdatePatVersion = MMP_TRUE;
    gtPsiSiTableMgr.serviceCount++;
    ptHitService->lcn = gtPsiSiTableMgr.serviceCount;

end:
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrUpdateServiceName(
    MMP_UINT16      programNumber,
    MMP_UINT8*      pServiceProviderName,
    MMP_UINT32      providerNameLen,    
    MMP_UINT8*      pServiceName,
    MMP_UINT32      serviceNameLen)
{
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;
    MMP_BOOL bResult = MMP_TRUE;

    if (providerNameLen > MAX_NAME_LEN || serviceNameLen > MAX_NAME_LEN || 
        (providerNameLen + serviceNameLen) > (MAX_DESCRIPTOR_LEN - 3))
    {
        return MMP_FALSE;
    }

    _PsiSiTableMgrLock();
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        if (gtPsiSiTableMgr.ptService[i].programNumber == programNumber)
        {
            ptHitService = &gtPsiSiTableMgr.ptService[i];
            break;
        }
    }
    
    if (ptHitService)
    {
        if (ptHitService->pServiceProviderName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptHitService->pServiceProviderName);
            ptHitService->pServiceProviderName = MMP_NULL;
            ptHitService->providerNameLen = 0;
        }

        if (ptHitService->pServiceName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptHitService->pServiceName);
            ptHitService->pServiceName = MMP_NULL;
            ptHitService->serviceNameLen = 0;
        }

        ptHitService->providerNameLen = providerNameLen;
        if (ptHitService->providerNameLen)
        {
            ptHitService->pServiceProviderName = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptHitService->providerNameLen);
            if (MMP_NULL == ptHitService->pServiceProviderName)
            {
                bResult = MMP_FALSE;
                ptHitService->providerNameLen = 0;
                goto end;
            }
            PalMemcpy(ptHitService->pServiceProviderName, pServiceProviderName, ptHitService->providerNameLen);
        }

        ptHitService->serviceNameLen = serviceNameLen;
        if (ptHitService->serviceNameLen)
        {
            ptHitService->pServiceName = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptHitService->serviceNameLen);
            if (MMP_NULL == ptHitService->pServiceName)
            {
                if (ptHitService->pServiceProviderName)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, ptHitService->pServiceProviderName);
                    ptHitService->pServiceProviderName = MMP_NULL;
                    ptHitService->providerNameLen = 0;
                }
                ptHitService->serviceNameLen = 0;
                bResult = MMP_FALSE;
                goto end;
            }
            PalMemcpy(ptHitService->pServiceName, pServiceName, ptHitService->serviceNameLen);
        }
        gtPsiSiTableMgr.bUpdateSdtVersion = MMP_TRUE;
    }
    else
    {
        bResult = MMP_FALSE;   
    }

end:
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrUpdateServiceProgramNumber(
    MMP_UINT32      serviceIndex,
    MMP_UINT16      programNumber)
{
    MMP_BOOL bResult = MMP_TRUE;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;
    
    _PsiSiTableMgrLock();
    if (serviceIndex < gtPsiSiTableMgr.serviceCount)
    {
        if (programNumber < MAX_PROGRAM_NUMBER)
        {
            ptHitService = &gtPsiSiTableMgr.ptService[serviceIndex];
            ptHitService->programNumber = programNumber;
            ptHitService->bUpdatePmtVersion = MMP_TRUE;
            gtPsiSiTableMgr.bUpdatePatVersion = MMP_TRUE;
            gtPsiSiTableMgr.bUpdateSdtVersion = MMP_TRUE;
            gtPsiSiTableMgr.bUpdateNitVersion = MMP_TRUE;
        }
        else
        {
            bResult = MMP_FALSE;
        }
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}
    
MMP_BOOL
PsiSiMgrUpdateServicePmtPid(
    MMP_UINT16      programNumber,
    MMP_UINT16      pmtPid)
{
    MMP_BOOL bResult = MMP_TRUE;
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;
    
    _PsiSiTableMgrLock();
    if (pmtPid < MAX_PID)
    {
        for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
        {
            if (gtPsiSiTableMgr.ptService[i].programNumber == programNumber)
            {
                ptHitService = &gtPsiSiTableMgr.ptService[i];
                break;
            }
        }
        
        if (ptHitService)
        {
            ptHitService->pmtPid = pmtPid;
            gtPsiSiTableMgr.bUpdatePatVersion = MMP_TRUE;
            ptHitService->bUpdatePmtVersion = MMP_TRUE;         
        }
        else
        {
            bResult = MMP_FALSE;
        }
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrUpdateServiceVideoInfo(
    MMP_UINT16          programNumber,
    MMP_UINT16          videoPid,
    VIDEO_STREAM_TYPE   videoStreamType)
{
    MMP_BOOL bResult = MMP_TRUE;
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;

    _PsiSiTableMgrLock();
    if (videoStreamType < LAST_VIDEO_STREAM_TYPE
     && videoPid < MAX_PID)
    {
        for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
        {
            if (gtPsiSiTableMgr.ptService[i].programNumber == programNumber)
            {
                ptHitService = &gtPsiSiTableMgr.ptService[i];
                break;
            }
        }
        
        if (ptHitService)
        {
            ptHitService->videoPid = videoPid;
            ptHitService->videoStreamType = videoStreamType;
            ptHitService->bUpdatePmtVersion = MMP_TRUE;         
        }
        else
        {
            bResult = MMP_FALSE;
        }
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrUpdateServiceAudioInfo(
    MMP_UINT16          programNumber,
    MMP_UINT16          audioPid,
    VIDEO_STREAM_TYPE   audioStreamType)
{
    MMP_BOOL bResult = MMP_TRUE;
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;

    _PsiSiTableMgrLock();
    if (audioStreamType < LAST_AUDIO_STREAM_TYPE
     && audioPid < MAX_PID)
    {
        for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
        {
            if (gtPsiSiTableMgr.ptService[i].programNumber == programNumber)
            {
                ptHitService = &gtPsiSiTableMgr.ptService[i];
                break;
            }
        }
        
        if (ptHitService)
        {
            ptHitService->audioPid = audioPid;
            ptHitService->audioStreamType = audioStreamType;
            ptHitService->bUpdatePmtVersion = MMP_TRUE;         
        }
        else
        {
            bResult = MMP_FALSE;
        }
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrUpdateAudioEncodeType(
    AUDIO_STREAM_TYPE   audioStreamType)
{
    MMP_UINT32 i = 0;
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        if (gtPsiSiTableMgr.ptService[i].audioStreamType != audioStreamType)
        {
            gtPsiSiTableMgr.ptService[i].audioStreamType = audioStreamType;
            gtPsiSiTableMgr.ptService[i].bUpdatePmtVersion = MMP_TRUE;
        }
    }
    return MMP_TRUE;
}

MMP_UINT32
PsiSiMgrGetPatLen(
    void)
{
    return TS_PACKET_LEN;
}

MMP_UINT32
PsiSiMgrGetPmtLen(
    void)
{
    return TS_PACKET_LEN;
}

MMP_UINT32
PsiSiMgrGetSdtLen(
    void)
{
    MMP_UINT32 tableLen = 0;
    _PsiSiTableMgrLock();
    tableLen = gtPsiSiTableMgr.sdtBufferLen;
    _PsiSiTableMgrUnLock();
    return tableLen;
}

MMP_UINT32
PsiSiMgrGetNitLen(
    void)
{
    MMP_UINT32 tableLen = 0;
    _PsiSiTableMgrLock();
    tableLen = gtPsiSiTableMgr.nitBufferLen;
    _PsiSiTableMgrUnLock();
    return tableLen;
}

MMP_BOOL
PsiSiMgrGetPat(
    MMP_UINT8* pBuffer)
{
    MMP_BOOL bResult = MMP_TRUE;
    _PsiSiTableMgrLock();
    if (pBuffer)
    {
        PalMemcpy(pBuffer, gtPsiSiTableMgr.pTsPatBuffer, TS_PACKET_LEN);
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrGetPmt(
    MMP_UINT32 index,
    MMP_UINT8* pBuffer)
{
    MMP_BOOL bResult = MMP_TRUE;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;

    _PsiSiTableMgrLock();
    if (index < gtPsiSiTableMgr.serviceCount)
    {
        ptHitService = &gtPsiSiTableMgr.ptService[index];
        PalMemcpy(pBuffer, ptHitService->pTsPmtBuffer, TS_PACKET_LEN);
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrGetSdt(
    MMP_UINT8* pBuffer)
{
    MMP_BOOL bResult = MMP_TRUE;
    _PsiSiTableMgrLock();
    if (gtPsiSiTableMgr.pTsSdtBuffer && pBuffer && gtPsiSiTableMgr.sdtBufferLen)
    {       
        PalMemcpy(pBuffer, gtPsiSiTableMgr.pTsSdtBuffer, gtPsiSiTableMgr.sdtBufferLen);
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_BOOL
PsiSiMgrGetNit(
    MMP_UINT8* pBuffer)
{
    MMP_BOOL bResult = MMP_TRUE;
    _PsiSiTableMgrLock();
    if (gtPsiSiTableMgr.pTsNitBuffer && pBuffer && gtPsiSiTableMgr.nitBufferLen)
    {
        PalMemcpy(pBuffer, gtPsiSiTableMgr.pTsNitBuffer, gtPsiSiTableMgr.nitBufferLen);
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

MMP_UINT32
PsiSiMgrGetServiceCount(
    void)
{
    MMP_UINT32 serviceCount = 0;
    _PsiSiTableMgrLock();
    serviceCount = gtPsiSiTableMgr.serviceCount;
    _PsiSiTableMgrUnLock();
    return serviceCount;
}

MMP_BOOL
PsiSiMgrGetEsPid(
    MMP_UINT32  serviceIndex,
    MMP_UINT16* pVideoPid,
    MMP_UINT16* pAudioPid)
{
    MMP_BOOL bResult = MMP_TRUE;
    PMT_SERVICE_ENTRY_INFO* ptHitService = MMP_NULL;

    _PsiSiTableMgrLock();
    if (serviceIndex < gtPsiSiTableMgr.serviceCount && pVideoPid && pAudioPid)
    {
        ptHitService = &gtPsiSiTableMgr.ptService[serviceIndex];
        *pVideoPid = ptHitService->videoPid;
        *pAudioPid = ptHitService->audioPid;
    }
    else
    {
        bResult = MMP_FALSE;
    }
    _PsiSiTableMgrUnLock();
    return bResult;
}

void
PsiSiMgrRemoveServices(
    void)
{
    MMP_UINT32 i = 0;
    PMT_SERVICE_ENTRY_INFO* ptService = MMP_NULL;

    _PsiSiTableMgrLock();
    for (i = 0; i < gtPsiSiTableMgr.serviceCount; i++)
    {
        ptService = &gtPsiSiTableMgr.ptService[i];
        if (ptService->pServiceProviderName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptService->pServiceProviderName);
            ptService->pServiceProviderName = MMP_NULL;
        }
        if (ptService->pServiceName)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptService->pServiceName);
            ptService->pServiceName = MMP_NULL;
        }
    }
    gtPsiSiTableMgr.serviceCount = 0;
    _PsiSiTableMgrUnLock();
}
