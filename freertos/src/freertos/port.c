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

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "spr_defs.h"
#include "mmio.h"
#include "timer.h"

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

unsigned portBASE_TYPE uxCriticalNesting = 0;

void MMIO_Write(unsigned short addr, unsigned short data)
{
    *(volatile unsigned short *) (MMIO_ADDR + addr) = data;
}

unsigned int MMIO_Read(unsigned short addr)
{
    return *(volatile unsigned short *) (MMIO_ADDR + addr);
}

void MMIO_WriteMask(unsigned short addr, unsigned short data, unsigned short mask)
{
    MMIO_Write(addr, ((MMIO_Read(addr) & ~mask) | (data & mask)));
}

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
portSTACK_TYPE *pxOriginalTOS = pxTopOfStack;
int i = 0;

    pxTopOfStack += (CTXT_BYTES_OFFSET_FROM_SP / sizeof(*pxTopOfStack));

    /* The guard bytes can detect stack overflow. The context-saving and
       restoring code will check the existences of the guards in the debug
       build. However, the space of guards are still reserved in the release
       build (though they're not checked) because I don't want to modify the
       offset of the other contexts. They are not very large anyway
       (8 bytes totally).
    */
#if defined(__DEBUG__)
    *pxTopOfStack++ = ( portSTACK_TYPE ) CTXT_BEGIN_GUARD;
#else
    *pxTopOfStack++ = 0;
#endif
    *pxTopOfStack++ = ( portSTACK_TYPE ) pxCode;

    *pxTopOfStack++ = ( portSTACK_TYPE ) mfspr(SPR_SR);

    *pxTopOfStack++ = ( portSTACK_TYPE ) pxOriginalTOS;
    *pxTopOfStack++ = ( portSTACK_TYPE ) pvParameters; /* R3 */
    for (i = 4; i <= 30; ++ i)
        *pxTopOfStack++ = 0xACACAC00 | i; /* Magic number. Debugging only */

    /* R31 is usually not used, hopefully. Storing the entry point here may help
       to identify the owner of this context. */
    *pxTopOfStack++ = ( portSTACK_TYPE ) pxCode;

    *pxTopOfStack++ = uxCriticalNesting;
#if defined(__DEBUG__)
    *pxTopOfStack++ = ( portSTACK_TYPE ) CTXT_END_GUARD;
#else
    *pxTopOfStack++ = 0;
#endif

     return pxOriginalTOS;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
    timer_start(SPR_TTMR_PERIOD);

    /* Start the first task. */

    asm volatile ( "l.lwz   r1, 0(%0)" : : "r"(pxCurrentTCB) );
    asm volatile ( "l.addi  r1, r1, %0" : : "i"(CTXT_BYTES_OFFSET_FROM_SP) );

    asm volatile ( "l.lwz   r3, %0(r1)" : : "i"(CTXT_OFFSET_PC) );
    asm volatile ( "l.mtspr r0, r3, %0" : : "i"(SPR_EPCR_BASE) );
    asm volatile ( "l.lwz   r3, %0(r1)" : : "i"(CTXT_OFFSET_SR) );
    asm volatile ( "l.mtspr r0, r3, %0" : : "i"(SPR_ESR_BASE) );
    asm volatile ( "l.lwz   r3, 0x10(r1)" ); /* pvParameters */
    asm volatile ( "l.addi  r1, r1, %0" : : "i"(-CTXT_BYTES_OFFSET_FROM_SP) );
#if 0
    /* Here r2 is initialised to a value which is impossibly
       incorrect, and we expect the prologue the entry function should
       create the stack frame and initialise r2.
    */
    asm volatile ( "l.movhi r2, 0xA5A5" );
    asm volatile ( "l.ori   r2, r2, 0xA5A5" );
#endif
    asm volatile ( "l.rfe" );

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler() {}

/*
 * Lock routine called by Newlib on malloc / realloc / free entry to guarantee a
 * safe section as memory allocation management uses global data.
 * See the aforementioned details.
 */
void __malloc_lock(struct _reent *ptr)
{
	vTaskSuspendAll();
}

/*
 * Unlock routine called by Newlib on malloc / realloc / free exit to guarantee
 * a safe section as memory allocation management uses global data.
 * See the aforementioned details.
 */
void __malloc_unlock(struct _reent *ptr)
{
	xTaskResumeAll();
}
