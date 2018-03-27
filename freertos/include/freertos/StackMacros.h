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

#ifndef STACK_MACROS_H
#define STACK_MACROS_H

/*
 * Call the stack overflow hook function if the stack of the task being swapped
 * out is currently overflowed, or looks like it might have overflowed in the
 * past.
 *
 * Setting configCHECK_FOR_STACK_OVERFLOW to 1 will cause the macro to check
 * the current stack state only - comparing the current top of stack value to
 * the stack limit.  Setting configCHECK_FOR_STACK_OVERFLOW to greater than 1
 * will also cause the last few stack bytes to be checked to ensure the value
 * to which the bytes were set when the task was created have not been
 * overwritten.  Note this second test does not guarantee that an overflowed
 * stack will always be recognised.
 */

/*-----------------------------------------------------------*/

#if( configCHECK_FOR_STACK_OVERFLOW == 0 )

    /* FreeRTOSConfig.h is not set to check for stack overflows. */
    #define taskFIRST_CHECK_FOR_STACK_OVERFLOW()
    #define taskSECOND_CHECK_FOR_STACK_OVERFLOW()

#endif /* configCHECK_FOR_STACK_OVERFLOW == 0 */
/*-----------------------------------------------------------*/

#if( configCHECK_FOR_STACK_OVERFLOW == 1 )

    /* FreeRTOSConfig.h is only set to use the first method of
    overflow checking. */
    #define taskSECOND_CHECK_FOR_STACK_OVERFLOW()

#endif
/*-----------------------------------------------------------*/

#if( ( configCHECK_FOR_STACK_OVERFLOW > 0 ) && ( portSTACK_GROWTH < 0 ) )

    /* Only the current stack state is to be checked. */
    #define taskFIRST_CHECK_FOR_STACK_OVERFLOW()                                                        \
    {                                                                                                   \
    extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );      \
                                                                                                        \
        /* Is the currently saved stack pointer within the stack limit? */                              \
        if( pxCurrentTCB->pxTopOfStack <= pxCurrentTCB->pxStack )                                       \
        {                                                                                               \
            vApplicationStackOverflowHook( ( xTaskHandle ) pxCurrentTCB, pxCurrentTCB->pcTaskName );    \
        }                                                                                               \
    }

#endif /* configCHECK_FOR_STACK_OVERFLOW == 1 */
/*-----------------------------------------------------------*/

#if( ( configCHECK_FOR_STACK_OVERFLOW > 0 ) && ( portSTACK_GROWTH > 0 ) )

    /* Only the current stack state is to be checked. */
    #define taskFIRST_CHECK_FOR_STACK_OVERFLOW()                                                        \
    {                                                                                                   \
    extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );      \
                                                                                                        \
        /* Is the currently saved stack pointer within the stack limit? */                              \
        if( pxCurrentTCB->pxTopOfStack >= pxCurrentTCB->pxEndOfStack )                                  \
        {                                                                                               \
            vApplicationStackOverflowHook( ( xTaskHandle ) pxCurrentTCB, pxCurrentTCB->pcTaskName );    \
        }                                                                                               \
    }

#endif /* configCHECK_FOR_STACK_OVERFLOW == 1 */
/*-----------------------------------------------------------*/

#if( ( configCHECK_FOR_STACK_OVERFLOW > 1 ) && ( portSTACK_GROWTH < 0 ) )

    #define taskSECOND_CHECK_FOR_STACK_OVERFLOW()                                                                                                   \
    {                                                                                                                                               \
    extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );                                                  \
    static const unsigned portCHAR ucExpectedStackBytes[] = {   tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE };   \
                                                                                                                                                    \
                                                                                                                                                    \
        /* Has the extremity of the task stack ever been written over? */                                                                           \
        if( memcmp( ( void * ) pxCurrentTCB->pxStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) != 0 )                      \
        {                                                                                                                                           \
            vApplicationStackOverflowHook( ( xTaskHandle ) pxCurrentTCB, pxCurrentTCB->pcTaskName );                                                \
        }                                                                                                                                           \
    }

#endif /* #if( configCHECK_FOR_STACK_OVERFLOW > 1 ) */
/*-----------------------------------------------------------*/

#if( ( configCHECK_FOR_STACK_OVERFLOW > 1 ) && ( portSTACK_GROWTH > 0 ) )

    #define taskSECOND_CHECK_FOR_STACK_OVERFLOW()                                                                                                   \
    {                                                                                                                                               \
    extern void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );                                                  \
    portCHAR *pcEndOfStack = ( portCHAR * ) pxCurrentTCB->pxEndOfStack;                                                                             \
    static const unsigned portCHAR ucExpectedStackBytes[] = {   tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                                tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE };   \
                                                                                                                                                    \
                                                                                                                                                    \
        pcEndOfStack -= sizeof( ucExpectedStackBytes );                                                                                             \
                                                                                                                                                    \
        /* Has the extremity of the task stack ever been written over? */                                                                           \
        if( memcmp( ( void * ) pcEndOfStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) != 0 )                               \
        {                                                                                                                                           \
            vApplicationStackOverflowHook( ( xTaskHandle ) pxCurrentTCB, pxCurrentTCB->pcTaskName );                                                \
        }                                                                                                                                           \
    }

#endif /* #if( configCHECK_FOR_STACK_OVERFLOW > 1 ) */
/*-----------------------------------------------------------*/

#endif /* STACK_MACROS_H */

