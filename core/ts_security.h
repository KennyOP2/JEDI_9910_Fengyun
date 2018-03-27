/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file data_sync.h
 * @author
 * @version 0.1
 */

#ifndef TS_SECURITY_H
#define TS_SECURITY_H

#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define SKT_PRIOR_HEADER    6
#define PUBLIC_KEY_LEN      2
#define SESSION_KEY_LEN     2
#define CRC_SIZE            4
//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct PUBLIC_KEY_TAG
{
    MMP_UINT8* pModulus;
    MMP_INT32  modulusLen;
    MMP_UINT8* pPubExponent;
    MMP_INT32  pubExponentLen;
} PUBLIC_KEY;

typedef struct PRIVATE_KEY_TAG
{
    MMP_UINT8* pP;
    MMP_INT32  pLen;
    MMP_UINT8* pQ;
    MMP_INT32  qLen;
    MMP_UINT8* pDModP_1;
    MMP_INT32  dModP_1Len;
    MMP_UINT8* pDModQ_1;
    MMP_INT32  dModQ_1Len;
    MMP_UINT8* pQ_1ModP;
    MMP_INT32  q_1ModPLen;
} PRIVATE_KEY;

//format of SESSION_KEY_TABLE (SKT)
//table_id                  8
//section_syntax_indicator  1
//'0'                       1  
//reserved                  2
//section_length           12
//reserved                  2
//version_number            5
//current_next_indicator    1
//section_number            8
//last_section_number       8
//for (i=0;i<N;i++){
//  public_key_length      16
//  session_key_length     16
//  public_key_data
//  session_key_data
//}
//CRC_32

// Consist of n byte modulus and 4 byte exponent.
typedef struct TS_DEVICE_KEY_ENTRY_TAG
{
    MMP_UINT8* pKeyBuffer;
    MMP_UINT32 keySize;
    MMP_BOOL   bValid;
} TS_DEVICE_KEY_ENTRY;

typedef struct TS_DEVICE_KEY_STORE_TAG
{
    TS_DEVICE_KEY_ENTRY** pptKeyEntry;
    MMP_UINT32            keyCount;
} TS_DEVICE_KEY_STORE;

//=============================================================================
//                              Function  Definition
//=============================================================================

//Castor 3
MMP_BOOL
TsSecurityGenerateKey(
    MMP_UINT32 keyLen,
    PRIVATE_KEY* ptPrivateKey,
    PUBLIC_KEY*  ptPublicKey);

MMP_BOOL
TsSecurityDestroyKey(
    PUBLIC_KEY*  ptPublicKey,
    PRIVATE_KEY* ptPrivateKey);

void
TsSecurityDecryptSessionKey(
    PUBLIC_KEY*  ptPublicKey,
    PRIVATE_KEY* ptPrivateKey,
    MMP_UINT8*   pEncryptSessionKey,
    MMP_UINT32   bufferSize,
    MMP_UINT8*   pOutSessionKey);


// JEDI
MMP_BOOL
TsSecurityCretateKeyStore(
    MMP_UINT32              keyStoreCount);

void
TsSecurityDestroyKeyStore(
    void);

MMP_BOOL
TsSecurityGetKeyStore(
    TS_DEVICE_KEY_STORE**  pptKeyStore);

MMP_UINT32
TsSecurityGetKeyStoreCount(
    void);

MMP_BOOL
TsSecurityInsertPublicKey(
    PUBLIC_KEY*          ptPublicKey);

MMP_BOOL
TsSecurityInsertKeyBuffer(
    MMP_UINT8*          pKeyBuffer,
    MMP_UINT32          keySize);

void
TsSecurityGenerateSessionKey(
    MMP_UINT8*  pSessionKey,
    MMP_UINT32  bufferSize);
  
void
TsSecurityEncryptSessionKey(
    PUBLIC_KEY* ptPublicKey,
    MMP_UINT8*  pSessionKey,
    MMP_UINT32  bufferSize,
    MMP_UINT8*  pOutSessionKey);

#ifdef __cplusplus
}
#endif

#endif
