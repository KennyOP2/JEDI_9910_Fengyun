add_definitions(
    -DHAVE_MMAP=0
    -DINTERNAL_NEWLIB
    -DDEFINE_MALLOC
    -DDEFINE_FREE
    -DDEFINE_REALLOC
    -DDEFINE_CALLOC
    -DDEFINE_CFREE
    -DDEFINE_MEMALIGN
    -DDEFINE_VALLOC
    -DDEFINE_PVALLOC
    -DDEFINE_MALLINFO
    -DDEFINE_MALLOC_STATS
    -DDEFINE_MALLOC_USABLE_SIZE
    -DDEFINE_MALLOPT
    )

add_library(malloc STATIC
    calloc.c
    malign.c
    malloc.c
    mallocr.c
    msize.c
    mstats.c
    mtrim.c
    realloc.c
    valloc.c
    )
