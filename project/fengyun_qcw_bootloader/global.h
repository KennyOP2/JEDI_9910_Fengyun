#include "pal/pal.h"

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================
#define BREAK_ON_ERROR(exp) if (exp) { trac(""); break; }
#define MALLOC(size)        PalHeapAlloc(PAL_HEAP_DEFAULT, (size))
#define FREE(ptr)           {PalHeapFree(PAL_HEAP_DEFAULT, (void*)(ptr)); (ptr) = MMP_NULL;}

#ifdef __cplusplus
}
#endif
#endif
