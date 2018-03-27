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

    * RISC porting by SMedia Tech. Corp. 2008
*/

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  unsigned portLONG
#define portBASE_TYPE   long
typedef unsigned portLONG portTickType;
#define portMAX_DELAY ( portTickType ) 0xffffffff

/*-----------------------------------------------------------*/

/* Hardware specifics. */
#define portSTACK_GROWTH            ( -1 )
#define portTICK_RATE_MS            ( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          4

/*
 * These define the timer to use for generating the tick interrupt.
 * They are put in this file so they can be shared between "port.c"
 * and "portisr.c".
 */
#define portTIMER_REG_BASE_PTR      AT91C_BASE_TC0
#define portTIMER_CLK_ENABLE_BIT    AT91C_PS_TC0
#define portTIMER_AIC_CHANNEL       ( ( unsigned portLONG ) 4 )
/*-----------------------------------------------------------*/

/* Task utilities. */

/* FIXME: replace magic numbers. */
#if defined(RTOS_USE_ISR)

#define portDISABLE_INTERRUPTS() do { \
    asm volatile ("l.mfspr r31, r0,  17"  : : : "r31" ); \
    asm volatile ("l.andi  r31, r31, ~0x6" : : : "r31" ); \
    asm volatile ("l.mtspr r0,  r31, 17"); \
    } while (0)

#define portENABLE_INTERRUPTS() do { \
    asm volatile ("l.mfspr r31, r0,  17"  : : : "r31" ); \
    asm volatile ("l.ori   r31, r31, 0x6" : : : "r31" ); \
    asm volatile ("l.mtspr r0,  r31, 17"); \
    } while (0)

#else

#define portDISABLE_INTERRUPTS() do {} while(0)

#define portENABLE_INTERRUPTS()  do {} while(0)

#endif /* defined(RTOS_USE_ISR) */

extern unsigned portBASE_TYPE uxCriticalNesting;

#define portENTER_CRITICAL() do { \
    portDISABLE_INTERRUPTS(); \
    uxCriticalNesting ++; \
    } while (0)

#define portEXIT_CRITICAL() do { \
    uxCriticalNesting --; \
    if (0 == uxCriticalNesting) \
        portENABLE_INTERRUPTS(); \
    } while (0)

extern volatile signed portBASE_TYPE xSchedulerRunning;

void vTaskCheckDelayedTasks( void );

#define portYIELD() do { \
                        portENTER_CRITICAL(); \
                        if (xSchedulerRunning != pdFALSE) { \
                            vTaskCheckDelayedTasks(); \
                            asm volatile ( "l.sys  0xF" ); \
                            asm volatile ( "l.nop  0x0" ); \
                        } \
                        portEXIT_CRITICAL(); \
                    } while(0)

void vTaskSwitchContext();
#define portYIELD_FROM_ISR()         vTaskSwitchContext()

#define portSOFTWARE_BREAKPOINT() do { \
    asm volatile ( "l.trap 15" ); \
    asm volatile ( "l.nop  0" ); \
    } while (0)
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

