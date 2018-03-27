#ifndef ROM_PARSER_H
#define ROM_PARSER_H

#include "pal/pal.h"
#include "core_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct ROM_INFO_TAG
{
    MMP_UINT32  dataOffset;
    MMP_UINT32  dataSize;
    MMP_UINT32  binSize;
    MMP_UINT32  crc32;
    MMP_UINT32  version;
} ROM_INFO;

//=============================================================================
//                              Function Declaration
//=============================================================================
CORE_API MMP_RESULT
romParser_GetRomInfo(
    MMP_UINT8*  pBuffer,
    MMP_UINT32  bufSize,
    ROM_INFO*   pRomInfo);

#ifdef __cplusplus
}
#endif

#endif // end of ROM_PARSER_H