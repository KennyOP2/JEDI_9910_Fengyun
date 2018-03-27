#ifndef USB_PORT_H
#define USB_PORT_H

#include "portmacro.h"


#ifdef __cplusplus
extern "C" {
#endif

#define USBEX_API	extern
#if defined(MM9910)
#define ithPrintf	printf
#endif

#define ithEnterCritical		portENTER_CRITICAL
#define ithExitCritical			portEXIT_CRITICAL

/*
 * vmem api
 */
#define MEM_USAGE_DMA           0
#define MEM_USAGE_QH_EX         0
#define MEM_USAGE_QTD_EX        0
#define MEM_USAGE_PERIODIC_EX   0
#define MEM_USAGE_4KBUF_EX      0
#define MEM_Allocate(a,b,c)     MEM_Allocate(a,b)

/*
 * for spin lock
 */
typedef struct {
	volatile unsigned int lock;
} _spinlock_t;

#define _spin_lock_init(x)	
#define _spin_lock(x)					portENTER_CRITICAL()
#define _spin_unlock(x)					portEXIT_CRITICAL()
#define _local_irq_save()					
#define _local_irq_restore()				
#define _spin_lock_irqsave(lock)		do { _local_irq_save();  _spin_lock(lock); } while (0)
#define _spin_unlock_irqrestore(lock)	do { _spin_unlock(lock);  _local_irq_restore(); } while (0)


#define HOST_GetVramBaseAddress()       0
#define ithUsbEnableClock()				do { HOST_WriteRegisterMask(0x46, 0x1<<3, 0x1<<3); HOST_WriteRegisterMask(0x46, 0x1<<1, 0x1<<1); } while(0)

#ifdef __cplusplus
}
#endif

#endif


