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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "spr_defs.h"

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION            0
#ifdef IDLE_TASK
#define configUSE_IDLE_HOOK             1
#else
#define configUSE_IDLE_HOOK             0
#endif
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              ( ( unsigned portLONG ) 66000000 ) /* = 66.000MHz clk gen */
#define configTICK_RATE_HZ              ( ( portTickType ) 1000 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned portSHORT ) 2048 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 256 * 1024 ) )
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         0
#define configUSE_CO_ROUTINES           0
#define configUSE_MUTEXES               1
#define configCHECK_FOR_STACK_OVERFLOW  0
#define configUSE_RECURSIVE_MUTEXES     1
#define configQUEUE_REGISTRY_SIZE       0
#define configUSE_COUNTING_SEMAPHORES   0
#define configUSE_NEWLIB_REENTRANT      0
#define configUSE_TRACE_FACILITY        0
#define configUSE_DUMP_FACILITY         0
#define configGENERATE_RUN_TIME_STATS   1

#define configMAX_PRIORITIES            ( ( unsigned portBASE_TYPE ) 5 )
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskRestart                    1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 0
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1

#if ( configGENERATE_RUN_TIME_STATS == 1 )
extern void vConfigureTimerForRunTimeStats( void );
extern unsigned int vGetRunTimeCounterValue( void );
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() vGetRunTimeCounterValue()
#endif

#endif /* FREERTOS_CONFIG_H */

