#ifndef TS_MUX_H
#define TS_MUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mmp_types.h"
#include "ts_security.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define TS_PACKET_LEN 188
#define MAX_SERVICE_COUNT 4

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef struct TS_OUT_HANDLE_TAG TS_OUT_HANDLE;
typedef void (*BUFFER_FULL_CALLBACK) (TS_OUT_HANDLE* ptDecoder);

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct PTS_TIME_TAG
{
    MMP_UINT32 timeHigh;
    MMP_UINT32 timeLow;
} PTS_TIME;

typedef struct SERVICE_ENTRY_INFO_TAG
{
    MMP_UINT16  videoPid;
    MMP_UINT16  audioPid;
    MMP_UINT8   pPmtTsPacketBuffer[TS_PACKET_LEN];
    PTS_TIME    tVideoTimestamp;
    PTS_TIME    tAudioTimestamp;
    MMP_UINT8   pmtCc;
    MMP_UINT8   videoCc;
    MMP_UINT8   audioCc;
    MMP_UINT8   frameTime;
} SERVICE_ENTRY_INFO;

typedef struct TX_ENCODE_INFO_TAG
{
    MMP_UINT32 width;
    MMP_UINT32 height;
    MMP_UINT32 bValidResolution;
    MMP_UINT32 bVideoLock;
    MMP_UINT32 videoInputSource;
    MMP_BOOL   bSkipFrame;
    MMP_UINT16 encoderInfo;
    MMP_UINT32 audioCodec;
    MMP_UINT32 samplingRate;
} TX_ENCODE_INFO;

struct TS_OUT_HANDLE_TAG
{
    MMP_UINT8*              pTsWriteBuffer;
    MMP_UINT32              bufferSize;
    MMP_UINT32              tsWriteIndex;
    MMP_UINT32              tsLastUpdateIndex;
    MMP_BOOL                bForceUpdate;
    MMP_UINT32              pesTsSize;
    FILE*                   pWriteFILE;
    BUFFER_FULL_CALLBACK    pfCallback;
    MMP_UINT32              frameCount;
    MMP_UINT8               pPatTsPacketBuffer[TS_PACKET_LEN];
    MMP_UINT8*              pSdtTsPacketBuffer;
    MMP_UINT32              sdtTsPacketLen;
    SERVICE_ENTRY_INFO      ptServiceArray[MAX_SERVICE_COUNT];
    MMP_UINT8*              pNitTsPacketBuffer;
    MMP_UINT32              nitTsPacketLen;
    MMP_UINT8               pSitTsPacketBuffer[TS_PACKET_LEN];
    MMP_BOOL                bEnableSecurityMode;
    MMP_BOOL                bExternalBuffer;
    MMP_UINT8               serviceCount;
    MMP_UINT8               patCc;
    MMP_UINT8               sdtCc;
    MMP_UINT8               nitCc;
    MMP_UINT8               sitCc;
    MMP_UINT8               sitVersion;
#ifdef AV_SENDER_SECURITY_MODE
    MMP_UINT8*              pSktTsPacketBuffer;
    TS_DEVICE_KEY_STORE*    ptKeyStore;
    MMP_UINT8*              pSessionKey;
    MMP_UINT8               pInitVector[16];
    MMP_UINT32              sessionKeyLen;
    MMP_UINT8               sktVersion;
    MMP_UINT8               sktCc;
    MMP_UINT8               sktSectionCount;
#endif
};

//=============================================================================
//                              Public Function Declarition
//=============================================================================

void
TsMuxUpdateTimeStamp(
    PTS_TIME*   ptPts,
    MMP_UINT32  timeStamp);

void
TsMuxCreatePatTable(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxCreatePmtTable(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxCreateSdtTable(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxCreateNitTable(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxCreateSitTable(
    TS_OUT_HANDLE*  ptTsHandle,
    TX_ENCODE_INFO* ptTxEncodeInfo);

void
TsMuxGeneratePatTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxGeneratePmtTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxGenerateSdtTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxGenerateNullTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxGenerateNitTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxGenerateSitTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

#ifdef AV_SENDER_SECURITY_MODE
void
TsMuxGenerateSktTsPacket(
    TS_OUT_HANDLE* ptTsHandle);

MMP_BOOL
TsMuxCreateSktTable(
    TS_OUT_HANDLE* ptTsHandle);
#endif

MMP_BOOL
TsMuxCreateHandle(
    TS_OUT_HANDLE**         pptTsHandle,
    BUFFER_FULL_CALLBACK    pfCallback,
    MMP_BOOL                bExternalBuffer,
    MMP_BOOL                bEnableSecurityMode);

void
TsMuxTerminateHandle(
    TS_OUT_HANDLE* ptTsHandle);

void
TsMuxSetTsBuffer(
    TS_OUT_HANDLE* ptTsHandle,
    MMP_UINT8*     pTsBuffer,
    MMP_UINT32     bufferSize);

void
TsMuxRemoveService(
    TS_OUT_HANDLE*  ptTsHandle);

#ifdef __cplusplus
}
#endif

#endif
