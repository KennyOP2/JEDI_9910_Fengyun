#ifndef PSI_SI_TABLE_MGR_H
#define PSI_SI_TABLE_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core_interface.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Public Function Declarition
//=============================================================================

MMP_BOOL
PsiSiMgrInit(
    void);

void
PsiSiMgrTerminate(
    void);

MMP_BOOL
PsiSiMgrUpdateTSID(
    MMP_UINT32 tsId);

MMP_BOOL
PsiSiMgrUpdateNetworkName(
    MMP_UINT8* pNetworkName,
    MMP_UINT32 nameLen);

MMP_BOOL
PsiSiMgrUpdateNetworkId(
    MMP_UINT32 networkId);

MMP_BOOL
PsiSiMgrUpdateONID(
    MMP_UINT32 onId);

MMP_BOOL
PsiSiMgrUpdateServiceListDescriptor(
    MMP_UINT32 serviceId,
    MMP_UINT32 serviceType);
    
MMP_BOOL
PsiSiMgrUpdateCountryId(
    CORE_COUNTRY_ID countryId);

MMP_BOOL
PsiSiMgrUpdateLCN(
    MMP_UINT32 serviceId,
    MMP_UINT32 lcn);

MMP_BOOL
PsiSiMgrUpdateModulationParameter(
    MMP_UINT32          frequency,
    MMP_UINT32          bandwidth,
    CONSTELLATION_MODE  constellation,
    CODE_RATE_MODE      codeRate,
    GUARD_INTERVAL_MODE guardInterval);

MMP_BOOL
PsiSiMgrGenerateTablePacket(
    void);

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
    MMP_UINT32      serviceNameLen);

MMP_BOOL
PsiSiMgrUpdateServiceName(
    MMP_UINT16      programNumber,
    MMP_UINT8*      pServiceProviderName,
    MMP_UINT32      providerNameLen,    
    MMP_UINT8*      pServiceName,
    MMP_UINT32      serviceNameLen);

MMP_BOOL
PsiSiMgrUpdateServiceProgramNumber(
    MMP_UINT32      serviceIndex,
    MMP_UINT16      programNumber);
    
MMP_BOOL
PsiSiMgrUpdateServicePmtPid(
    MMP_UINT16      programNumber,
    MMP_UINT16      pmtPid);

MMP_BOOL
PsiSiMgrUpdateServiceVideoInfo(
    MMP_UINT16          programNumber,
    MMP_UINT16          videoPid,
    VIDEO_STREAM_TYPE   videoStreamType);

MMP_BOOL
PsiSiMgrUpdateServiceAudioInfo(
    MMP_UINT16          programNumber,
    MMP_UINT16          audioPid,
    VIDEO_STREAM_TYPE   audioStreamType);

MMP_BOOL
PsiSiMgrUpdateAudioEncodeType(
    AUDIO_STREAM_TYPE   audioStreamType);

MMP_UINT32
PsiSiMgrGetPatLen(
    void);

MMP_UINT32
PsiSiMgrGetPmtLen(
    void);

MMP_UINT32
PsiSiMgrGetSdtLen(
    void);

MMP_UINT32
PsiSiMgrGetNitLen(
    void);

MMP_BOOL
PsiSiMgrGetPat(
    MMP_UINT8* pBuffer);

MMP_BOOL
PsiSiMgrGetPmt(
    MMP_UINT32 index,
    MMP_UINT8* pBuffer);

MMP_BOOL
PsiSiMgrGetSdt(
    MMP_UINT8* pBuffer);

MMP_BOOL
PsiSiMgrGetNit(
    MMP_UINT8* pBuffer);

MMP_UINT32
PsiSiMgrGetServiceCount(
    void);

MMP_BOOL
PsiSiMgrGetEsPid(
    MMP_UINT32  serviceIndex,
    MMP_UINT16* pVideoPid,
    MMP_UINT16* pAudioPid);

void
PsiSiMgrRemoveServices(
    void);

#ifdef __cplusplus
}
#endif

#endif
