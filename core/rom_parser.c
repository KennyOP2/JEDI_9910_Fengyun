//=============================================================================
//                              Include Files
//=============================================================================
#include "rom_parser.h"

//=============================================================================
//                              Macro
//=============================================================================
#define BREAK_ON_ERROR(exp) if (exp) { trac(""); break; }

//=============================================================================
//                              Constant Definition
//=============================================================================
#define HEADER1 ((((int)'S')<<24)+(((int)'M')<<16)+(((int)'E')<<8)+(((int)'D')<<0))
#define HEADER2 ((((int)'I')<<24)+(((int)'A')<<16)+(((int)'0')<<8)+(((int)'2')<<0))

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
static MMP_INLINE MMP_UINT32
_GET32(
    MMP_UINT8* addr)
{
    return addr[0] << 24
         | addr[1] << 16
         | addr[2] <<  8
         | addr[3];
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

MMP_RESULT
romParser_GetRomInfo(
    MMP_UINT8*  pBuffer,
    MMP_UINT32  bufSize,
    ROM_INFO*   pRomInfo)
{
    MMP_RESULT  result = MMP_RESULT_ERROR;
    MMP_UINT32  header1 = _GET32(&pBuffer[0]);
    MMP_UINT32  header2 = _GET32(&pBuffer[4]);
    MMP_UINT32  headerSize  = _GET32(&pBuffer[12]);
    MMP_UINT32  dataSize    = _GET32(&pBuffer[16]);

    do
    {
        BREAK_ON_ERROR(!pBuffer);
        BREAK_ON_ERROR(!pRomInfo);
        BREAK_ON_ERROR(header1 != HEADER1);
        BREAK_ON_ERROR(header2 != HEADER2);
        BREAK_ON_ERROR(headerSize < 68);
        BREAK_ON_ERROR(headerSize > bufSize);
        pRomInfo->version       = _GET32(&pBuffer[20]);
        pRomInfo->crc32         = _GET32(&pBuffer[headerSize - 8]);
        pRomInfo->binSize       = _GET32(&pBuffer[headerSize - 4]);
        pRomInfo->dataOffset    = headerSize;
        pRomInfo->dataSize      = dataSize;
        result = MMP_SUCCESS;
    } while (0);
    return result;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================