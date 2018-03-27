/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Function Body for SBRK.
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/unistd.h>

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
/* _heap_start, _heap_end is set in the linker command file */
extern caddr_t _heap_start;
extern caddr_t _heap_end;

/////////////////////////////////////////////////////////////////
//                      Public Function
/////////////////////////////////////////////////////////////////
/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
void* heap_ptr = NULL;

void* __attribute__((no_instrument_function))
_sbrk(ptrdiff_t __incr)
{
    void* base;

    if (heap_ptr == NULL) {
        heap_ptr = (void*) & _heap_start;
    }

    if ((void*)(heap_ptr + __incr) <= (void*)(&_heap_end)) {
        base = heap_ptr;
        heap_ptr += __incr;
        return (void*)(base);
    } else {
        errno = ENOMEM;
        return (void*)(NULL);
    }
}

