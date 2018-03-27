/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for OpenRISC
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __SYS_H__
#define __SYS_H__

#include "spr_defs.h"
#include "or32.h"

void report_error(const char *errstr);
void ic_enable(void);
void ic_disable(void);
void dc_enable(void);
void dc_disable(void);
void exit(int exitno);
void ic_invalidate(void);
void dc_invalidate(void);

#define DMA_INT_MASK  (1<<2)
#define HOST_INT_MASK (1<<3)
#define TICK_INT_MASK (1<<4)
#define UART_INT_MASK (1<<5)

/////////////////////////////////////////////////////////////////
// Inline Function
/////////////////////////////////////////////////////////////////
static __inline void CLI(void) {
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_IEE);
}

static __inline void SEI(void) {
    mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);
}
#endif /* __SYS_H__ */

