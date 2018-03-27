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

/*
 * The simplest possible implementation of pvPortMalloc(). Use newlib
 * malloc & free function call.
 *
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

//#include "mem/memleak.h"

/* Setup the correct byte alignment mask for the defined byte alignment. */
#if portBYTE_ALIGNMENT == 4
    #define heapBYTE_ALIGNMENT_MASK ( ( size_t ) 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
    #define heapBYTE_ALIGNMENT_MASK ( ( size_t ) 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
    #define heapBYTE_ALIGNMENT_MASK ( ( size_t ) 0x0000 )
#endif

#ifndef heapBYTE_ALIGNMENT_MASK
    #error "Invalid portBYTE_ALIGNMENT definition"
#endif

/* Allocate the memory for the heap defined in the load script.
   configTOTAL_HEAP_SIZE is ignored.
*/

void *pvPortMalloc( size_t xWantedSize )
{
void *pvReturn = NULL;

    /* Ensure that blocks are always aligned to the required number of bytes. */
    #if portBYTE_ALIGNMENT != 1
    if( xWantedSize & heapBYTE_ALIGNMENT_MASK )
    {
        /* Byte alignment required. */
        xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & heapBYTE_ALIGNMENT_MASK ) );
    }
    #endif

    vTaskSuspendAll();
    {
        pvReturn = malloc(xWantedSize);
        if (NULL == pvReturn) taskSOFTWARE_BREAKPOINT();
    }
    xTaskResumeAll();

    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
    free( pv );
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
    /* intentionally left blank. */
}

