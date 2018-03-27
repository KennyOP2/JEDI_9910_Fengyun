#include "config.h"
#include "pal/pal.h"
#include <stdlib.h>

MMP_INT
PalRand(
    void)
{
    return rand();
}

void
PalSrand(
    MMP_UINT seed)
{
    srand(seed);
}