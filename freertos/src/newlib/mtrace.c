/*
 * Copyright (c) 2010 ITE Tech. Corp. All Rights Reserved.
 */
/* @file
 * Function Body for malloc trace.
 *
 * @author Kuoping Hsu
 * @date 2010.07.30.
 * @version 1.0
 *
 */

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <malloc.h>
#include <FreeRTOS.h>

/////////////////////////////////////////////////////////////////
//                      Constant and Structur Declaration
/////////////////////////////////////////////////////////////////
#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif


/* This is part of newlib for malloc structur */
struct malloc_chunk
{
  INTERNAL_SIZE_T prev_size; /* Size of previous chunk (if free). */
  INTERNAL_SIZE_T size;      /* Size in bytes, including overhead. */
  struct malloc_chunk* fd;   /* double links -- used only if free. */
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk* mchunkptr;

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#define PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#define IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#define prev_chunk(p)\
   ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))


/* Treat space at ptr + offset as a chunk */

#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))

/* extract p's inuse bit */

#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#define prev_inuse(p)  ((p)->size & PREV_INUSE)

#ifndef M_MALLOC_CALLBACK
#define M_MALLOC_CALLBACK   -5
#endif

#ifndef M_FREE_CALLBACK
#define M_FREE_CALLBACK     -6
#endif

/////////////////////////////////////////////////////////////////
//                      Private Variable
/////////////////////////////////////////////////////////////////
static int malloc_reentry = 0;
static int free_reentry = 0;

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
extern void* heap_ptr;
extern caddr_t _heap_start;
extern caddr_t _heap_end;

/////////////////////////////////////////////////////////////////
//                      Private Function
/////////////////////////////////////////////////////////////////
static
void malloc_enter(int size, int ptr)
{
    if (malloc_reentry) return;
    malloc_reentry++;

    printf("[MTRACE] %dms: task '%s' malloc %08x with %d bytes\n",
           xTaskGetTimerCount(), vGetCurrentTaskName(), ptr, size);

    malloc_reentry--;
}

static
void free_enter(void *ptr)
{
    if (free_reentry) return;
    free_reentry++;

    printf("[MTRACE] %dms: task '%s' free %08x\n",
           xTaskGetTimerCount(), vGetCurrentTaskName(), (int)ptr);

    free_reentry--;
}

/////////////////////////////////////////////////////////////////
//                      Public Function
/////////////////////////////////////////////////////////////////
void mtrace_init(void)
{
    // CAUTION: Do not remove this print message.
    // On the first time call of printf function, the printf
    // function will invoke the malloc to obtains the buffer,
    // it will call the malloc call back function in the mean
    // time. To prevent the infinite recursive function call,
    // we should print a message to get the buffer before
    // enable the mtrace function.
    printf("[MTRACE] begin at %dms\n", xTaskGetTimerCount()); // DO NOT remove this message

    mallopt(M_MALLOC_CALLBACK, (int)(&malloc_enter));
    mallopt(M_FREE_CALLBACK, (int)(&free_enter));
}

void mtrace_dump(void)
{
    int idx;

    printf("[MTRACE] Memory dump\n");
    printf("[MTRACE] Heap start 0x%08x, end 0x%08x, use %d bytes\n",
           (int)&_heap_start, (int)&_heap_end, (int)heap_ptr - (int)&_heap_start);

    // Walk through the heap
    /*
    idx = (int)&_heap_start;
    while(idx < (int)&_heap_end)
    {
    }
    */
}

void mtrace_stop(void)
{
    printf("[MTRACE] end at %dms\n", xTaskGetTimerCount());
    mallopt(M_MALLOC_CALLBACK, 0);
    mallopt(M_FREE_CALLBACK, 0);
}

