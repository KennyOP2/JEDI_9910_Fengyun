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

#ifndef PROJDEFS_H
#define PROJDEFS_H

/* Defines the prototype to which task functions must conform. */
typedef void (*pdTASK_CODE)( void * );

#define pdTRUE      ( 1 )
#define pdFALSE     ( 0 )

#define pdPASS                                  ( 1 )
#define pdFAIL                                  ( 0 )
#define errQUEUE_EMPTY                          ( 0 )
#define errQUEUE_FULL                           ( 0 )

/* Error definitions. */
#define errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY   ( -1 )
#define errNO_TASK_TO_RUN                       ( -2 )
#define errQUEUE_BLOCKED                        ( -4 )
#define errQUEUE_YIELD                          ( -5 )

#endif /* PROJDEFS_H */

