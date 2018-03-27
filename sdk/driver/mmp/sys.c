/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file sys.c
 *
 * @author Vincent Lee
 */

#include "sys/sys.h"

//=============================================================================
//                              Function Definition
//=============================================================================

void 
MMP_Sleep(
    MMP_UINT32 ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    #if 1
    if (ms == 0)
    {
        taskYIELD();
        return;
    }

    while (ms >= 500) {
        vTaskDelay(timer_ms_to_xticks(500));
        ms -= 500;
    }
    vTaskDelay(timer_ms_to_xticks(ms));
    #else
    #if defined(__DEBUG__)
    timer_sleep(timer_ms_to_xticks(ms)/8);
    #endif
    timer_sleep(timer_ms_to_xticks(ms));
    #endif
#endif
}

void MMP_USleep(MMP_UINT32 us)
{
#if defined(__FREERTOS__)
    if (us == 0)
    {
        taskYIELD();
    }
    else
    {
        vTaskDelay(timer_us_to_xticks(us));
    }
#elif defined(WIN32)
    Sleep(1);
#endif
}

SYS_CLOCK_T
SYS_GetClock(
    void)
{
#if defined(__FREERTOS__)
    return xTaskGetTimerCount();
#elif defined(WIN32)
    return timeGetTime();
#else
    return 0;
#endif
}

#define SYS_CLOCKS_PER_SEC 1000

MMP_ULONG
SYS_GetDuration(
    SYS_CLOCK_T clock)
{
#if defined(__FREERTOS__)
    return (xTaskGetTimerCount() - clock);
#elif defined(WIN32)
    SYS_CLOCK_T currClock = timeGetTime();
    MMP_ULONG duration;

    if (currClock >= clock)
    {
        duration = currClock - clock;
    }
    else
    {
        // Overflow
        duration = ((SYS_CLOCK_T) -1) - clock + currClock;
    }

    return duration * 1000 / SYS_CLOCKS_PER_SEC;
#else
    return 0;
#endif
}


//=============================================================================
//                              Function Definition - memory
//=============================================================================

void*
SYS_Malloc(
    MMP_UINT32 size)
{
    PRECONDITION(size != 0);
    return malloc(size);
}

void*
SYS_Realloc(
    void* address,
    MMP_UINT32 size)
{
    PRECONDITION(address != NULL);
	PRECONDITION(size != 0);
    return realloc(address,size);
}

void
SYS_Free(
    void* address)
{
    PRECONDITION(address != NULL);
    free(address);
}

void
SYS_CleanMemory(
	void* dest,
	MMP_UINT32 size)
{
    PRECONDITION(dest != NULL);
    PRECONDITION(size != 0);
    memset(dest, 0, (int)size);
}

void
SYS_MemorySet(
	void* dest,
	MMP_UINT8 data,
    MMP_UINT32 size)
{
    PRECONDITION(dest != NULL);
    PRECONDITION(size != 0);
	memset(dest, data, (int)size);
}

void
SYS_Memcpy(
    void* dest,
    const void* src,
    MMP_UINT32 size)
{
    PRECONDITION(src != NULL);
    PRECONDITION(dest != NULL);
    PRECONDITION(size != 0);
	memcpy(dest, src, (int)size);
}

//=============================================================================
//                              Function Definition - event
//=============================================================================

void*
SYS_CreateEvent(
    void)
{
#ifdef WIN32
    HANDLE hEvent;

    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL)
	{
        MMP_DbgPrint(("Error create event failed\n"));
		return NULL;
	}
	return (void*) hEvent;
#else
    xSemaphoreHandle sem;
    vSemaphoreCreateBinary(sem);
    if (!xSemaphoreTake(sem, portMAX_DELAY))
        return 0;
    else
        return (void*)sem;
#endif
}

MMP_BOOL
SYS_DelEvent(
    void* event)
{
#ifdef WIN32
    CloseHandle((HANDLE)event);
#else
    vQueueDelete(event);
#endif
    return MMP_TRUE;
}

MMP_BOOL
SYS_SetEvent(
    void* event)
{
#ifdef WIN32
    SetEvent((HANDLE*) event);
#else
    signed portBASE_TYPE ret = pdFAIL;
    PRECONDITION(event);
    ret = xSemaphoreGive((xSemaphoreHandle)event);
    if (errQUEUE_FULL != ret && pdPASS != ret)
        SYS_PANIC();
#endif
    return MMP_TRUE;
}

MMP_BOOL
SYS_SetEventFromIsr(
    void* event)
{
#ifdef WIN32
    SetEvent((HANDLE*) event);
#else
	static portBASE_TYPE gxHigerPriorityTaskWoken;
    signed portBASE_TYPE ret = pdFAIL;
    PRECONDITION(event);
    ret = xSemaphoreGiveFromISR((xSemaphoreHandle)event, &gxHigerPriorityTaskWoken);
    if (errQUEUE_FULL != ret && pdPASS != ret)
        SYS_PANIC();
#endif
    return MMP_TRUE;
}

void
SYS_WaitForEventForever(
    void* event)
{
#ifdef WIN32
    WaitForSingleObject((HANDLE)event, INFINITE);
#else
    PRECONDITION(event);
    while (!xSemaphoreTake((xSemaphoreHandle) event, portMAX_DELAY));
#endif
}

MMP_INT
SYS_WaitEvent(
    void* event,
    MMP_ULONG timeout)
{
    MMP_INT result=0;
#if defined(__FREERTOS__)
    if (timeout == SYS_EVENT_INFINITE)
        SYS_WaitForEventForever(event);
    else
        result = !xSemaphoreTake( (xSemaphoreHandle)event, timer_ms_to_xticks(timeout) );
#endif

#if defined(_WIN32_WCE) || defined (WIN32)
    result = WaitForSingleObject(event, timeout);
    if (result == WAIT_TIMEOUT)
    {
       	printf("WAIT EVENT TIMEOUT\r\n");
    }
    else if (result == WAIT_FAILED)
    {
        printf("WaitForSingleObject FAIL");
    }
    else
    {
        result = 0;
    }
#endif

    return result;
}

//=============================================================================
//                              Function Definition - semaphore
//=============================================================================

void*
SYS_CreateSemaphore(
	MMP_UINT32 initialCount, void* SemaphoreName)
{
#ifdef WIN32
	wchar_t wbuf[256];
	mbstowcs(wbuf, (char*)SemaphoreName, 256);
    return (void*) CreateSemaphore(NULL, initialCount, initialCount, wbuf);
#else
    if (initialCount > 1) return NULL;

    xSemaphoreHandle semaphore;
    vSemaphoreCreateBinary(semaphore);

    if (0 == initialCount) SYS_WaitSemaphore(semaphore);

    return (void*) semaphore;
#endif
}

void
SYS_ReleaseSemaphore(
    void* semaphore)
{
#ifdef WIN32
    ReleaseSemaphore((HANDLE)semaphore, 1, NULL);
#else
    if (pdTRUE != xSemaphoreGive(semaphore))
        printf("release semaphore 0x%X fail!!\n", semaphore);
#endif
}

void
SYS_WaitSemaphore(
    void* semaphore)
{
    PRECONDITION(semaphore != NULL);
#ifdef WIN32
    WaitForSingleObject((HANDLE) semaphore, INFINITE);
#else
    {
        int i = 0;
        while (1)
        {
            if (xSemaphoreTake(semaphore, (portTickType) portMAX_DELAY) == pdTRUE)
                break;

            if (i++ >= 10000)
            {
                printf("wait semaphore 0x%X timeout!!\n", semaphore);
                break;
            }
        };
    }
#endif
}

void
SYS_DeleteSemaphore(
    void* semaphore)
{
    PRECONDITION(semaphore != NULL);
#ifdef WIN32
    CloseHandle((HANDLE) semaphore);
#else
    vQueueDelete(semaphore);
#endif
}


//=============================================================================
//                              Function Definition - Task relaed
//=============================================================================

void*
SYS_CreateTaskThread(
    void (*t_entry)(),
    void* parameter,
    void* stack_address,
    MMP_UINT32 stack_size,
    MMP_UINT8 priority,
    MMP_UINT32 time_slice)
{
#if defined (WIN32)
    HANDLE hThread;
    int nPriority;;
    if(stack_address) SYS_Free(stack_address);

    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)t_entry, (LPVOID)parameter, 0, NULL);
    if (hThread == NULL)
    {
        MMP_DbgPrint(("Error create thread failed\n"));
        return NULL;
    }
	priority = (priority >> 5);
	if(priority == 4) {
		nPriority = (THREAD_PRIORITY_NORMAL);
	} else if(priority > 4) {
		nPriority = (THREAD_PRIORITY_BELOW_NORMAL);
	} else {
		nPriority = (THREAD_PRIORITY_ABOVE_NORMAL);
	}
	SetThreadPriority(hThread, nPriority);
//	SetThreadPriority(hThread, (THREAD_PRIORITY_ABOVE_NORMAL));

    return (void*) hThread;
#endif
}

MMP_BOOL
SYS_DeleteTaskThread(
    void* t_pointer)
{
#if defined (WIN32)
    CloseHandle((HANDLE)t_pointer);
#endif
    return MMP_TRUE;
}

MMP_API MMP_UINT
SYS_GetTaskThread(void)
{
#if defined (WIN32)
    return GetCurrentThreadId();
#endif
}
