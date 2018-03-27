/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file sys.h
 *
 * @author Vincent Lee
 */

#ifndef SYS_H
#define SYS_H

#include "mmp_types.h"
#include "mmp.h"

#ifdef WIN32
    #include "Windows.h"
    #include "stdlib.h"
    #include "string.h"
    #include "stdio.h"
    #include <assert.h>
    #include <malloc.h>
#else
	#include <stdio.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <assert.h>
    #include "FreeRTOS.h"
    #include "semphr.h"
    #include "task.h"    
#endif

#ifdef __cplusplus
extern  "C" {
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

#ifdef WIN32
    #if defined(MMP_DEBUG)
        #define MMP_DbgPrint(_x_) printf _x_
        #define PRECONDITION assert
    #else
        #define MMP_DbgPrint(_x_) do {} while(0)
        #define PRECONDITION(x) do {} while(0)
    #endif
#else
    #if defined(__DEBUG__)
        #define MMP_DbgPrint(_x_) printf _x_
        #define PRECONDITION(x) if (!(x)) while(1);
    #elif defined(MMP_DEBUG)
        #define MMP_DbgPrint(_x_) printf _x_
        #define PRECONDITION(e) ((e) ? (void)0 : PalAssertFail(#e, __FILE__, __LINE__))
    #else
        #define MMP_DbgPrint(_x_) do {} while(0)
        #define PRECONDITION(x) do {} while(0)
    #endif
#endif

#define SYS_NOT_IMPLEMENTED(x) assert(0)
#define SYS_PANIC(x) assert(0)

//=============================================================================
//                              Function Declaration
//=============================================================================

MMP_API void
MMP_Sleep(
    MMP_UINT32 ms);

MMP_API void MMP_USleep(MMP_UINT32 us);

// time
typedef unsigned int SYS_CLOCK_T;

MMP_API SYS_CLOCK_T
SYS_GetClock(
    void);

MMP_API MMP_ULONG
SYS_GetDuration(
    SYS_CLOCK_T clock);

//=============================================================================
//                              Function Declaration - memory
//=============================================================================

MMP_API void*
SYS_Malloc(
    MMP_UINT32 size);

MMP_API void*
SYS_Realloc(
    void* address,
    MMP_UINT32 size);


MMP_API void
SYS_Free(
    void* address);

MMP_API void
SYS_CleanMemory(
	void* dest,
	MMP_UINT32 size);

MMP_API void
SYS_MemorySet(
	void* dest,
	MMP_UINT8 data,
    MMP_UINT32 size);

MMP_API void
SYS_Memcpy(
    void* dest,
    const void* src,
    MMP_UINT32 size);

//=============================================================================
//                              Function Declaration - event
//=============================================================================
#define SYS_EVENT_INFINITE    (0xFFFFFFFFul)

MMP_API void*
SYS_CreateEvent(
    void);

MMP_API MMP_BOOL
SYS_DelEvent(
    void* event);

MMP_API MMP_BOOL
SYS_SetEvent(
    void* event);

MMP_API MMP_BOOL
SYS_SetEventFromIsr(
    void* event);

MMP_API void
SYS_WaitForEventForever(
    void* event);

MMP_API MMP_INT
SYS_WaitEvent(
     void*  event,
     MMP_ULONG timeout);

//=============================================================================
//                              Function Declaration - semaphore
//=============================================================================

MMP_API void*
SYS_CreateSemaphore(
	MMP_UINT32 initialCount, 
	void* SemaphoreName);

MMP_API void
SYS_ReleaseSemaphore(
    void* semaphore);

MMP_API void
SYS_WaitSemaphore(
    void* semaphore);

MMP_API void
SYS_DeleteSemaphore(
    void* semaphore);


//=============================================================================
//                              Function Declaration - Task
//=============================================================================

MMP_API void*
SYS_CreateTaskThread(
    void (*t_entry)(),
    void* parameter,
    void* stack_address,
    MMP_UINT32 stack_size,
    MMP_UINT8 priority,
    MMP_UINT32 time_slice);

MMP_API MMP_BOOL
SYS_DeleteTaskThread(
    void* t_pointer);

MMP_API MMP_UINT
SYS_GetTaskThread(void);


#ifdef __cplusplus
}
#endif

#endif
