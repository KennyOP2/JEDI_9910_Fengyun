/*
    OpenRTOS V5.4.2 - Copyright (C) Wittenstein High Integrity Systems.

    OpenRTOS is distributed exclusively by Wittenstein High Integrity Systems,
    and is subject to the terms of the License granted to your organization,
    including its warranties and limitations on distribution.  It cannot be
    copied or reproduced in any way except as permitted by the License.

    Licenses are issued for each concurrent user working on a specified product
    line.

    WITTENSTEIN high integrity systems is a trading name of WITTENSTEIN
    aerospace & simulation ltd, Registered Office: Brown's Court, Long Ashton
    Business Park, Yanley Lane, Long Ashton, Bristol, BS41 9LB, UK.
    Tel: +44 (0) 1275 395 600, fax: +44 (0) 1275 393 630.
    E-mail: info@HighIntegritySystems.com
    Registered in England No. 3711047; VAT No. GB 729 1583 15

    http://www.HighIntegritySystems.com
*/

#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

/*
 * Include the generic headers required for the FreeRTOS port being used.
 */
#include <stddef.h>

/* Basic FreeRTOS definitions. */
#include "projdefs.h"

/* Application specific configuration options. */
#include "FreeRTOSConfig.h"

/* Definitions specific to the port being used. */
#include "portable.h"

/* Defines the prototype to which the application task hook function must
conform. */
typedef portBASE_TYPE (*pdTASK_HOOK_CODE)( void * );

#if ( configUSE_DUMP_FACILITY == 1 )
void traceTASK_SWITCHED_IN(void);
void traceTASK_SWITCHED_OUT(void);
void traceTASK_DELAY(void);

#define traceTASK_SWITCHED_IN()     portTASK_SWITCHED_IN()
#define traceTASK_SWITCHED_OUT()    portTASK_SWITCHED_OUT()
#define traceTASK_DELAY()           portTASK_DELAY()
#endif

/*
 * Check all the required application specific macros have been defined.
 * These macros are application specific and (as downloaded) are defined
 * within FreeRTOSConfig.h.
 */

#ifndef configUSE_PREEMPTION
    #error Missing definition:  configUSE_PREEMPTION should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_IDLE_HOOK
    #error Missing definition:  configUSE_IDLE_HOOK should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_TICK_HOOK
    #error Missing definition:  configUSE_TICK_HOOK should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_CO_ROUTINES
    #error  Missing definition:  configUSE_CO_ROUTINES should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_vTaskPrioritySet
    #error Missing definition:  INCLUDE_vTaskPrioritySet should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_uxTaskPriorityGet
    #error Missing definition:  INCLUDE_uxTaskPriorityGet should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_vTaskDelete
    #error Missing definition:  INCLUDE_vTaskDelete      should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_vTaskCleanUpResources
    #error Missing definition:  INCLUDE_vTaskCleanUpResources should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_vTaskSuspend
    #error Missing definition:  INCLUDE_vTaskSuspend     should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_vTaskDelayUntil
    #error Missing definition:  INCLUDE_vTaskDelayUntil should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef INCLUDE_vTaskDelay
    #error Missing definition:  INCLUDE_vTaskDelay should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_16_BIT_TICKS
    #error Missing definition:  configUSE_16_BIT_TICKS should be defined in FreeRTOSConfig.h as either 1 or 0.  See the Configuration section of the FreeRTOS API documentation for details.
#endif

#ifndef configUSE_APPLICATION_TASK_TAG
    #define configUSE_APPLICATION_TASK_TAG 0
#endif

#ifndef INCLUDE_uxTaskGetStackHighWaterMark
    #define INCLUDE_uxTaskGetStackHighWaterMark 0
#endif

#ifndef configUSE_RECURSIVE_MUTEXES
    #define configUSE_RECURSIVE_MUTEXES 0
#endif

#ifndef configUSE_MUTEXES
    #define configUSE_MUTEXES 0
#endif

#ifndef configUSE_COUNTING_SEMAPHORES
    #define configUSE_COUNTING_SEMAPHORES 0
#endif

#ifndef configUSE_ALTERNATIVE_API
    #define configUSE_ALTERNATIVE_API 0
#endif

#ifndef portCRITICAL_NESTING_IN_TCB
    #define portCRITICAL_NESTING_IN_TCB 0
#endif

#ifndef configMAX_TASK_NAME_LEN
    #define configMAX_TASK_NAME_LEN 16
#endif

#ifndef configIDLE_SHOULD_YIELD
    #define configIDLE_SHOULD_YIELD     1
#endif

#if configMAX_TASK_NAME_LEN < 1
    #undef configMAX_TASK_NAME_LEN
    #define configMAX_TASK_NAME_LEN 1
#endif

#ifndef INCLUDE_xTaskResumeFromISR
    #define INCLUDE_xTaskResumeFromISR 1
#endif

#ifndef INCLUDE_xTaskGetSchedulerState
    #define INCLUDE_xTaskGetSchedulerState 0
#endif

#if ( configUSE_MUTEXES == 1 )
    /* xTaskGetCurrentTaskHandle is used by the priority inheritance mechanism
    within the mutex implementation so must be available if mutexes are used. */
    #undef INCLUDE_xTaskGetCurrentTaskHandle
    #define INCLUDE_xTaskGetCurrentTaskHandle 1
#else
    #ifndef INCLUDE_xTaskGetCurrentTaskHandle
        #define INCLUDE_xTaskGetCurrentTaskHandle 0
    #endif
#endif

#ifndef portSET_INTERRUPT_MASK_FROM_ISR
    #define portSET_INTERRUPT_MASK_FROM_ISR() 0
#endif

#ifndef portCLEAR_INTERRUPT_MASK_FROM_ISR
    #define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusValue ) ( void ) uxSavedStatusValue
#endif

#ifndef configQUEUE_REGISTRY_SIZE
    #define configQUEUE_REGISTRY_SIZE 0
#endif

#if configQUEUE_REGISTRY_SIZE < 1
    #define configQUEUE_REGISTRY_SIZE 0
    #define vQueueAddToRegistry( xQueue, pcName )
    #define vQueueUnregisterQueue( xQueue )
#endif

/* Remove any unused trace macros. */
#ifndef traceSTART
    /* Used to perform any necessary initialisation - for example, open a file
    into which trace is to be written. */
    #define traceSTART()
#endif

#ifndef traceEND
    /* Use to close a trace, for example close a file into which trace has been
    written. */
    #define traceEND()
#endif

#ifndef traceTASK_SWITCHED_IN
    /* Called after a task has been selected to run.  pxCurrentTCB holds a pointer
    to the task control block of the selected task. */
    #define traceTASK_SWITCHED_IN()
#endif

#ifndef traceTASK_SWITCHED_OUT
    /* Called before a task has been selected to run.  pxCurrentTCB holds a pointer
    to the task control block of the task being switched out. */
    #define traceTASK_SWITCHED_OUT()
#endif

#ifndef traceBLOCKING_ON_QUEUE_RECEIVE
    /* Task is about to block because it cannot read from a
    queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
    upon which the read was attempted.  pxCurrentTCB points to the TCB of the
    task that attempted the read. */
    #define traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue )
#endif

#ifndef traceBLOCKING_ON_QUEUE_SEND
    /* Task is about to block because it cannot write to a
    queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
    upon which the write was attempted.  pxCurrentTCB points to the TCB of the
    task that attempted the write. */
    #define traceBLOCKING_ON_QUEUE_SEND( pxQueue )
#endif

#ifndef configCHECK_FOR_STACK_OVERFLOW
    #define configCHECK_FOR_STACK_OVERFLOW 0
#endif

/* The following event macros are embedded in the kernel API calls. */

#ifndef traceQUEUE_CREATE
    #define traceQUEUE_CREATE( pxNewQueue )
#endif

#ifndef traceQUEUE_CREATE_FAILED
    #define traceQUEUE_CREATE_FAILED()
#endif

#ifndef traceCREATE_MUTEX
    #define traceCREATE_MUTEX( pxNewQueue )
#endif

#ifndef traceCREATE_MUTEX_FAILED
    #define traceCREATE_MUTEX_FAILED()
#endif

#ifndef traceGIVE_MUTEX_RECURSIVE
    #define traceGIVE_MUTEX_RECURSIVE( pxMutex )
#endif

#ifndef traceGIVE_MUTEX_RECURSIVE_FAILED
    #define traceGIVE_MUTEX_RECURSIVE_FAILED( pxMutex )
#endif

#ifndef traceTAKE_MUTEX_RECURSIVE
    #define traceTAKE_MUTEX_RECURSIVE( pxMutex )
#endif

#ifndef traceCREATE_COUNTING_SEMAPHORE
    #define traceCREATE_COUNTING_SEMAPHORE()
#endif

#ifndef traceCREATE_COUNTING_SEMAPHORE_FAILED
    #define traceCREATE_COUNTING_SEMAPHORE_FAILED()
#endif

#ifndef traceQUEUE_SEND
    #define traceQUEUE_SEND( pxQueue )
#endif

#ifndef traceQUEUE_SEND_FAILED
    #define traceQUEUE_SEND_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE
    #define traceQUEUE_RECEIVE( pxQueue )
#endif

#ifndef traceQUEUE_PEEK
    #define traceQUEUE_PEEK( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE_FAILED
    #define traceQUEUE_RECEIVE_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_SEND_FROM_ISR
    #define traceQUEUE_SEND_FROM_ISR( pxQueue )
#endif

#ifndef traceQUEUE_SEND_FROM_ISR_FAILED
    #define traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE_FROM_ISR
    #define traceQUEUE_RECEIVE_FROM_ISR( pxQueue )
#endif

#ifndef traceQUEUE_RECEIVE_FROM_ISR_FAILED
    #define traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue )
#endif

#ifndef traceQUEUE_DELETE
    #define traceQUEUE_DELETE( pxQueue )
#endif

#ifndef traceTASK_CREATE
    #define traceTASK_CREATE( pxNewTCB )
#endif

#ifndef traceTASK_CREATE_FAILED
    #define traceTASK_CREATE_FAILED( pxNewTCB )
#endif

#ifndef traceTASK_DELETE
    #define traceTASK_DELETE( pxTaskToDelete )
#endif

#ifndef traceTASK_DELAY_UNTIL
    #define traceTASK_DELAY_UNTIL()
#endif

#ifndef traceTASK_DELAY
    #define traceTASK_DELAY()
#endif

#ifndef traceTASK_PRIORITY_SET
    #define traceTASK_PRIORITY_SET( pxTask, uxNewPriority )
#endif

#ifndef traceTASK_SUSPEND
    #define traceTASK_SUSPEND( pxTaskToSuspend )
#endif

#ifndef traceTASK_RESUME
    #define traceTASK_RESUME( pxTaskToResume )
#endif

#ifndef traceTASK_RESUME_FROM_ISR
    #define traceTASK_RESUME_FROM_ISR( pxTaskToResume )
#endif

#ifndef traceTASK_INCREMENT_TICK
    #define traceTASK_INCREMENT_TICK( xTickCount )
#endif

#ifndef configGENERATE_RUN_TIME_STATS
    #define configGENERATE_RUN_TIME_STATS 0
#endif

#if ( configGENERATE_RUN_TIME_STATS == 1 )

    #ifndef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
        #error If configGENERATE_RUN_TIME_STATS is defined then portCONFIGURE_TIMER_FOR_RUN_TIME_STATS must also be defined.  portCONFIGURE_TIMER_FOR_RUN_TIME_STATS should call a port layer function to setup a peripheral timer/counter that can then be used as the run time counter time base.
    #endif /* portCONFIGURE_TIMER_FOR_RUN_TIME_STATS */

    #ifndef portGET_RUN_TIME_COUNTER_VALUE
        #error If configGENERATE_RUN_TIME_STATS is defined then portGET_RUN_TIME_COUNTER_VALUE must also be defined.  portGET_RUN_TIME_COUNTER_VALUE should evaluate to the counter value of the timer/counter peripheral used as the run time counter time base.
    #endif /* portGET_RUN_TIME_COUNTER_VALUE */

#endif /* configGENERATE_RUN_TIME_STATS */

#ifndef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#endif

#ifndef configUSE_MALLOC_FAILED_HOOK
    #define configUSE_MALLOC_FAILED_HOOK 0
#endif

#endif /* INC_FREERTOS_H */

