/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Function body for OR32 ISR.
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#include <stdlib.h>

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include "spr_defs.h"
#include "isr.h"
#include "debug.h"

static void default_isr(void);

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
struct ISR_HANDLE _isr_root_handle[OR32_MAX_ISR] = {
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
  {default_isr, NULL},
};

/////////////////////////////////////////////////////////////////
//                      Public Function
/////////////////////////////////////////////////////////////////
/*
 * Install ISR function
 */
void or32_installISR(int isr, struct ISR_HANDLE *isr_handle, void isr_func(void)) {
    isr_handle->isr_func  = _isr_root_handle[isr].isr_func;
    isr_handle->next_node = _isr_root_handle[isr].next_node;
    _isr_root_handle[isr].isr_func  = isr_func;
    _isr_root_handle[isr].next_node = isr_handle;
}

/////////////////////////////////////////////////////////////////
//                      Private Function
/////////////////////////////////////////////////////////////////
static void default_isr(void) {
    // Empty Function
    PRINTF("Enter Defaut ISR\n");
}

