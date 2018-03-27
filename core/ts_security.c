#include "polarssl/aes.h"
#include "polarssl\sha2.h"
#include "polarssl\rsa.h"
#include "polarssl\md.h"
#include "pal/pal.h"
#include "mmp_dpu.h"
#include "ts_security.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
#define MAX_KEY_COUNT       16
#define TS_PACKET_SIZE      188
//JEDI

static TS_DEVICE_KEY_STORE* gptKeyStore = MMP_NULL;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
MMP_INT
_TsSecurityGenerateRadom(
    void* rng_state,
    unsigned char *output,
    size_t len)
{
    MMP_UINT32 data = 0;
    size_t i;

    mmpDpuGenerateRandomData((MMP_UINT8*) &data, 4);
    for( i = 0; i < len; ++i )
        output[i] = (MMP_UINT8) data;    
    return 0;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

//Castor 3
MMP_BOOL
TsSecurityGenerateKey(
    MMP_UINT32 keyLen,
    PRIVATE_KEY* ptPrivateKey,
    PUBLIC_KEY*  ptPublicKey)
{
    rsa_context tRsaContext = { 0 };
    void* pRand;
    MMP_UINT16 exponent = 0;
    MMP_INT result = 0;
    // Initialize the RNG state
    // Generate RSA key
    rsa_init(&tRsaContext, RSA_PKCS_V15, 0);
    result = rsa_gen_key(&tRsaContext, _TsSecurityGenerateRadom, pRand, keyLen, 65537);
    if (result)
    {
        dbg_msg(DBG_MSG_TYPE_INFO, "gen key fail: 0x%X\n", -result);
    }
    ptPublicKey->modulusLen = tRsaContext.N.n * 4;
    ptPublicKey->pModulus = PalHeapAlloc(PAL_HEAP_DEFAULT, ptPublicKey->modulusLen);
    mpi_write_binary(&tRsaContext.N, ptPublicKey->pModulus, (size_t) ptPublicKey->modulusLen);
    ptPublicKey->pubExponentLen = tRsaContext.E.n * 4;
    ptPublicKey->pPubExponent = PalHeapAlloc(PAL_HEAP_DEFAULT, ptPublicKey->pubExponentLen);
    mpi_write_binary(&tRsaContext.E, ptPublicKey->pPubExponent, (size_t) ptPublicKey->pubExponentLen);
    ptPrivateKey->pLen = tRsaContext.P.n * 4;
    ptPrivateKey->pP = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptPrivateKey->pLen);
    mpi_write_binary(&tRsaContext.P, ptPrivateKey->pP, (size_t) ptPrivateKey->pLen);
    ptPrivateKey->qLen = tRsaContext.Q.n * 4;
    ptPrivateKey->pQ = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptPrivateKey->qLen);
    mpi_write_binary(&tRsaContext.Q, ptPrivateKey->pQ, (size_t) ptPrivateKey->qLen);
    ptPrivateKey->dModP_1Len = tRsaContext.DP.n * 4;
    ptPrivateKey->pDModP_1 = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptPrivateKey->dModP_1Len);
    mpi_write_binary(&tRsaContext.DP, ptPrivateKey->pDModP_1, (size_t) ptPrivateKey->dModP_1Len);
    ptPrivateKey->dModQ_1Len = tRsaContext.DQ.n * 4;
    ptPrivateKey->pDModQ_1 = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptPrivateKey->dModQ_1Len);
    mpi_write_binary(&tRsaContext.DQ, ptPrivateKey->pDModQ_1, (size_t) ptPrivateKey->dModQ_1Len);
    ptPrivateKey->q_1ModPLen = tRsaContext.QP.n * 4;
    ptPrivateKey->pQ_1ModP = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptPrivateKey->q_1ModPLen);
    mpi_write_binary(&tRsaContext.QP, ptPrivateKey->pQ_1ModP, (size_t) ptPrivateKey->q_1ModPLen);
#if 0
    {
      
        char* keyN      = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        char* keyE      = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        char* keyP      = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        char* keyQ      = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        char* keyDModP  = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        char* keyDModQ  = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        char* key1ModP  = PalHeapAlloc(PAL_HEAP_DEFAULT, keyLen);
        
        mpi_write_binary(&tRsaContext.N, keyN, (size_t) 32);
        mpi_write_binary(&tRsaContext.E, keyE, (size_t) 4);
        mpi_write_binary(&tRsaContext.P, keyP, (size_t) 16);
        mpi_write_binary(&tRsaContext.Q, keyQ, (size_t) 16);
        mpi_write_binary(&tRsaContext.DP, keyDModP, (size_t) 16);
        mpi_write_binary(&tRsaContext.DQ, keyDModQ, (size_t) 16);
        mpi_write_binary(&tRsaContext.QP, key1ModP, (size_t) 16);

        MMP_UINT32 i = 0;
        MMP_UINT8* pBuffer = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "N\n");
        pBuffer = (MMP_UINT8*) keyN;
        for (i = 0; i < tRsaContext.N.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");        
        dbg_msg(DBG_MSG_TYPE_INFO, "E\n");
        pBuffer = (MMP_UINT8*) keyE;        
        for (i = 0; i < tRsaContext.E.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "P\n");
        pBuffer = (MMP_UINT8*) keyP;        
        for (i = 0; i < tRsaContext.P.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");        
        dbg_msg(DBG_MSG_TYPE_INFO, "Q\n");
        pBuffer = (MMP_UINT8*) keyQ;        
        for (i = 0; i < tRsaContext.Q.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "DP\n");
        pBuffer = (MMP_UINT8*) keyDModP;        
        for (i = 0; i < tRsaContext.DP.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");        
        dbg_msg(DBG_MSG_TYPE_INFO, "DQ\n");
        pBuffer = (MMP_UINT8*) keyDModQ;        
        for (i = 0; i < tRsaContext.DQ.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "QP\n");
        pBuffer = (MMP_UINT8*) key1ModP;        
        for (i = 0; i < tRsaContext.QP.n * 4; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pBuffer[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");                                                 
    }
#endif
    {
        MMP_UINT32 i = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pModulus[] =\n{\n");
        for (i = 0; i < ptPublicKey->modulusLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPublicKey->pModulus[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pPubExponent[] =\n{\n");
        for (i = 0; i < ptPublicKey->pubExponentLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPublicKey->pPubExponent[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pP[] =\n{\n");
        for (i = 0; i < ptPrivateKey->pLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPrivateKey->pP[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pQ[] =\n{\n");
        for (i = 0; i < ptPrivateKey->qLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPrivateKey->pQ[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pDModP_1[] =\n{\n");
        for (i = 0; i < ptPrivateKey->dModP_1Len; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPrivateKey->pDModP_1[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pDModQ_1[] =\n{\n");
        for (i = 0; i < ptPrivateKey->dModQ_1Len; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPrivateKey->pDModQ_1[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "static MMP_UINT8 pQ_1ModP[] =\n{\n");
        for (i = 0; i < ptPrivateKey->q_1ModPLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X, ", ptPrivateKey->pQ_1ModP[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n};\n");
    }

    rsa_free(&tRsaContext);
    return MMP_TRUE;
}

MMP_BOOL
TsSecurityDestroyKey(
    PUBLIC_KEY*  ptPublicKey,
    PRIVATE_KEY* ptPrivateKey)
{
    if (ptPublicKey->pModulus)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPublicKey->pModulus);
    }
    if (ptPublicKey->pPubExponent)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPublicKey->pPubExponent);        
    }
    PalMemset(ptPublicKey, 0x0, sizeof(PUBLIC_KEY));
    
    if (ptPrivateKey->pP)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPrivateKey->pP);
    }
    if (ptPrivateKey->pQ)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPrivateKey->pQ);
    }
    if (ptPrivateKey->pDModP_1)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPrivateKey->pDModP_1);
    }
    if (ptPrivateKey->pDModQ_1)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPrivateKey->pDModQ_1);
    }
    if (ptPrivateKey->pQ_1ModP)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptPrivateKey->pQ_1ModP);
    }
    PalMemset(ptPrivateKey, 0x0, sizeof(PRIVATE_KEY));
}

void
TsSecurityDecryptSessionKey(
    PUBLIC_KEY*  ptPublicKey,
    PRIVATE_KEY* ptPrivateKey,
    MMP_UINT8*   pEncryptSessionKey,
    MMP_UINT32   bufferSize,
    MMP_UINT8*   pOutSessionKey)
{
    MMP_UINT32 outLen = ptPublicKey->modulusLen;
    rsa_context tRsaContext = { 0 };
    int result = 0;
    rsa_init(&tRsaContext, RSA_PKCS_V15, 0);
    tRsaContext.len = ptPublicKey->modulusLen;
    mpi_read_binary( &tRsaContext.N , ptPublicKey->pModulus, ptPublicKey->modulusLen);
    mpi_read_binary( &tRsaContext.E , ptPublicKey->pPubExponent, ptPublicKey->pubExponentLen);
    mpi_read_binary( &tRsaContext.P , ptPrivateKey->pP, ptPrivateKey->pLen);
    mpi_read_binary( &tRsaContext.Q , ptPrivateKey->pQ, ptPrivateKey->qLen);
    mpi_read_binary( &tRsaContext.DP , ptPrivateKey->pDModP_1, ptPrivateKey->dModP_1Len);
    mpi_read_binary( &tRsaContext.DQ , ptPrivateKey->pDModQ_1, ptPrivateKey->dModQ_1Len);
    mpi_read_binary( &tRsaContext.QP , ptPrivateKey->pQ_1ModP, ptPrivateKey->q_1ModPLen);
    mpi_inv_mod(&tRsaContext.D, &tRsaContext.E, &tRsaContext.N);    
    result = rsa_pkcs1_decrypt(&tRsaContext,
                       RSA_PRIVATE,
                       &outLen,
                       pEncryptSessionKey,
                       pOutSessionKey,
                       tRsaContext.len);
#if 1
    {
        MMP_UINT32 i = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "encrypted session key is\n");
        for (i = 0; i < bufferSize; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pEncryptSessionKey[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");        
        dbg_msg(DBG_MSG_TYPE_INFO, "result: 0x%X, decrypted session key is\n", -result);
        for (i = 0; i < bufferSize; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pOutSessionKey[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
    }
#endif
    rsa_free(&tRsaContext);
}

// JEDI
MMP_BOOL
TsSecurityCretateKeyStore(
    MMP_UINT32              keyStoreCount)
{
    TS_DEVICE_KEY_ENTRY** pptKeyArray = MMP_NULL;
    MMP_UINT8* pArrayBuffer = MMP_NULL;
    MMP_UINT32 i = 0;
    MMP_BOOL   bResult = MMP_TRUE;

    if (keyStoreCount)
    {
        gptKeyStore = (TS_DEVICE_KEY_STORE*) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_DEVICE_KEY_STORE));
        if (MMP_NULL == gptKeyStore)
        {
            bResult = MMP_FALSE;
            goto end;
        }
        gptKeyStore->pptKeyEntry = (TS_DEVICE_KEY_ENTRY**) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_DEVICE_KEY_ENTRY*) * keyStoreCount);
        if (MMP_NULL == gptKeyStore->pptKeyEntry)
        {
            bResult = MMP_FALSE;
            goto end;
        }
        pArrayBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_DEVICE_KEY_ENTRY) * keyStoreCount);
        if (MMP_NULL == pArrayBuffer)
        {
            bResult = MMP_FALSE;
            goto end;
        }
        PalMemset(pArrayBuffer, 0x0, sizeof(TS_DEVICE_KEY_ENTRY) * keyStoreCount);
        for (i = 0; i < keyStoreCount; i++)
        {      
            gptKeyStore->pptKeyEntry[i] = (TS_DEVICE_KEY_ENTRY*) &pArrayBuffer[i * sizeof(TS_DEVICE_KEY_ENTRY)];
        }
        gptKeyStore->keyCount = keyStoreCount;
    }
end:
    if (MMP_FALSE == bResult)
    {
        if (gptKeyStore->pptKeyEntry)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, gptKeyStore->pptKeyEntry);
        }
        if (gptKeyStore)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, gptKeyStore);
        }
        if (pArrayBuffer)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, pArrayBuffer);
        }
        gptKeyStore = MMP_NULL;
    }
    return bResult;
}
    
void
TsSecurityDestroyKeyStore(
    void)
{
    MMP_UINT32 i = 0;
    TS_DEVICE_KEY_ENTRY* ptEntry = MMP_NULL;
    if (gptKeyStore)
    {
        for (i = 0; i < gptKeyStore->keyCount; i++)
        {
            ptEntry = gptKeyStore->pptKeyEntry[i];
            if (ptEntry->pKeyBuffer)
            {
                PalHeapFree(PAL_HEAP_DEFAULT, ptEntry->pKeyBuffer);
            }
        }

        if (gptKeyStore->pptKeyEntry[0])
        {
                PalHeapFree(PAL_HEAP_DEFAULT, gptKeyStore->pptKeyEntry[0]);
        }
        PalHeapFree(PAL_HEAP_DEFAULT, gptKeyStore);
    }
    gptKeyStore = MMP_NULL;
}

MMP_BOOL
TsSecurityGetKeyStore(
    TS_DEVICE_KEY_STORE**  pptKeyStore)
{
    if (MMP_NULL == gptKeyStore)
    {
        return MMP_FALSE;
    }
    *pptKeyStore = gptKeyStore;
    return MMP_TRUE;
}

MMP_UINT32
TsSecurityGetKeyStoreCount(
    void)
{
    MMP_UINT32 keyCount = 0;
    MMP_UINT32 i = 0;
    TS_DEVICE_KEY_ENTRY* ptEntry = MMP_NULL;

    if (MMP_NULL == gptKeyStore)
    {
        return 0;
    }
    for (i = 0; i < gptKeyStore->keyCount; i++)
    {
        ptEntry = gptKeyStore->pptKeyEntry[i];
        if (MMP_TRUE  == ptEntry->bValid)
        {
            keyCount++;
        }
    }
    return keyCount;
}

MMP_BOOL
TsSecurityInsertPublicKey(
    PUBLIC_KEY*          ptPublicKey)
{
    MMP_UINT32 i = 0;
    TS_DEVICE_KEY_ENTRY* ptEntry = MMP_NULL;
 
    if (MMP_NULL == gptKeyStore)
    {
        return MMP_FALSE;
    }

    for (i = 0; i < gptKeyStore->keyCount; i++)
    {
        ptEntry = gptKeyStore->pptKeyEntry[i];
        if (MMP_TRUE  == ptEntry->bValid)
        {
            // Entry is existed.
            if ((0 == PalMemcmp(ptEntry->pKeyBuffer, ptPublicKey->pModulus, ptPublicKey->modulusLen))
             && (0 == PalMemcmp(&ptEntry->pKeyBuffer[ptPublicKey->modulusLen], ptPublicKey->pPubExponent, ptPublicKey->pubExponentLen)))
            {
                dbg_msg(DBG_MSG_TYPE_INFO, "entry is inserted already\n");
                return MMP_TRUE;
            }
        }
    }

    for (i = 0; i < gptKeyStore->keyCount; i++)
    {
        ptEntry = gptKeyStore->pptKeyEntry[i];
        if (MMP_FALSE  == ptEntry->bValid)
        {
            ptEntry->keySize = ptPublicKey->modulusLen + ptPublicKey->pubExponentLen;
            ptEntry->pKeyBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, ptEntry->keySize);
            PalMemcpy(ptEntry->pKeyBuffer, ptPublicKey->pModulus, ptPublicKey->modulusLen);
            PalMemcpy(&ptEntry->pKeyBuffer[ptPublicKey->modulusLen], ptPublicKey->pPubExponent, ptPublicKey->pubExponentLen);
            ptEntry->bValid = MMP_TRUE;
            return MMP_TRUE;
        }
    }
    return MMP_FALSE;
}

MMP_BOOL
TsSecurityInsertKeyBuffer(
    MMP_UINT8*          pKeyBuffer,
    MMP_UINT32          keySize)
{
    MMP_UINT32 i = 0;
    TS_DEVICE_KEY_ENTRY* ptEntry = MMP_NULL;
 
    if (MMP_NULL == gptKeyStore)
    {
        return MMP_FALSE;
    }
    for (i = 0; i < gptKeyStore->keyCount; i++)
    {
        ptEntry = gptKeyStore->pptKeyEntry[i];
        if (MMP_TRUE  == ptEntry->bValid && (keySize == ptEntry->keySize))
        {
            // Entry is existed.
            if (0 == PalMemcmp(ptEntry->pKeyBuffer, pKeyBuffer, keySize))
            {
                return MMP_TRUE;
            }
        }
    }

    for (i = 0; i < gptKeyStore->keyCount; i++)
    {
        ptEntry = gptKeyStore->pptKeyEntry[i];
        if (MMP_FALSE  == ptEntry->bValid)
        {
            ptEntry->pKeyBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, keySize);
            PalMemcpy(ptEntry->pKeyBuffer, pKeyBuffer, keySize);
            ptEntry->keySize = keySize;
            ptEntry->bValid = MMP_TRUE;
            return MMP_TRUE;
        }
    }
    return MMP_FALSE;
}

void
TsSecurityGenerateSessionKey(
    MMP_UINT8*  pSessionKey,
    MMP_UINT32  bufferSize)
{
    mmpDpuGenerateRandomData(pSessionKey, bufferSize);
#if 0
    {
        MMP_UINT32 i = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "plain session key is\n");
        for (i = 0; i < bufferSize; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pSessionKey[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
    }
#endif
}

void
TsSecurityEncryptSessionKey(
    PUBLIC_KEY* ptPublicKey,
    MMP_UINT8*  pSessionKey,
    MMP_UINT32  bufferSize,
    MMP_UINT8*  pOutSessionKey)
{
    rsa_context tRsaContext = { 0 };
    int result = 0;

    rsa_init(&tRsaContext, RSA_PKCS_V15, 0);
    tRsaContext.len = ptPublicKey->modulusLen;
#if 0
    {
        MMP_UINT32 i = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "pModulus\n");
        for (i = 0; i < ptPublicKey->modulusLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", ptPublicKey->pModulus[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
        dbg_msg(DBG_MSG_TYPE_INFO, "pPubExponent\n");
        for (i = 0; i < ptPublicKey->pubExponentLen; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", ptPublicKey->pPubExponent[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
    }
#endif
    mpi_read_binary( &tRsaContext.N , ptPublicKey->pModulus, ptPublicKey->modulusLen);
    mpi_read_binary( &tRsaContext.E , ptPublicKey->pPubExponent, ptPublicKey->pubExponentLen);
    result = rsa_pkcs1_encrypt(&tRsaContext,
                      &_TsSecurityGenerateRadom,
                      0,
                      RSA_PUBLIC,
                      16,
                      pSessionKey,
                      pOutSessionKey);
#if 0
    {
        MMP_UINT32 i = 0;
        dbg_msg(DBG_MSG_TYPE_INFO, "result: 0x%X, encrypted session key is\n", -result);
        for (i = 0; i < bufferSize; i++)
        {
            dbg_msg(DBG_MSG_TYPE_INFO, "0x%02X ", pOutSessionKey[i]);
        }
        dbg_msg(DBG_MSG_TYPE_INFO, "\n");
    }
#endif
    rsa_free(&tRsaContext);
}
