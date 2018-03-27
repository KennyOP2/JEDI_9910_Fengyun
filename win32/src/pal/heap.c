#include "config.h"
#include "pal/pal.h"

#define CRTDBG_MAP_ALLOC
#include <malloc.h>

void*
PalHeapAlloc(
    MMP_INT name,
    MMP_SIZE_T size)
{
	void* mem;

    LOG_ENTER "PalHeapAlloc(name=%d,size=%d)\r\n", name, size LOG_END

	PalAssert(size > 0);

	mem = malloc(size);
	PalAssert(mem);

#if 0 // FOR DEBUG ONLY
    if ((MMP_ULONG) mem == 0x011A8A88 || (MMP_ULONG) mem == 0x00FAFE48)
    {
     return mem;
    }
#endif

    LOG_LEAVE "PalHeapAlloc()=0x%X\r\n", mem LOG_END
    return mem;
}

void
PalHeapFree(
    MMP_INT name,
    void* ptr)
{
    LOG_ENTER "PalHeapFree(name=%d,ptr=0x%X)\r\n", name, ptr LOG_END

    free(ptr);

    LOG_LEAVE "PalHeapFree()\r\n" LOG_END
}
