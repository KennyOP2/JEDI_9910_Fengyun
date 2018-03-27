/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for OpenRISC function
 *
 * @author Kuoping Hsu
 * @date 2006.11.03.
 * @version 1.0
 *
 */

#ifndef __ISR_H__
#define __ISR_H__

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
//                      Data Type Definition
/////////////////////////////////////////////////////////////////
struct ISR_HANDLE {
   void (*isr_func)();  // function pointer for current ISR function
   struct ISR_HANDLE *next_node;
};

/////////////////////////////////////////////////////////////////
//                      Constant Definition
/////////////////////////////////////////////////////////////////
#define OR32_ISR_RESET          (0x1 - 1)
#define OR32_ISR_BUSERR         (0x2 - 1)
#define OR32_ISR_DPGFAULT       (0x3 - 1)
#define OR32_ISR_IPGFAULT       (0x4 - 1)
#define OR32_ISR_TT             (0x5 - 1)
#define OR32_ISR_ALIGN          (0x6 - 1)
#define OR32_ISR_ILL            (0x7 - 1)
#define OR32_ISR_INT            (0x8 - 1)
#define OR32_ISR_DTLBMISS       (0x9 - 1)
#define OR32_ISR_ITLBMISS       (0xa - 1)
#define OR32_ISR_RANG           (0xb - 1)
#define OR32_ISR_SYSCALL        (0xc - 1)
#define OR32_ISR_TRAP           (0xe - 1)

#define OR32_MAX_ISR            0xe

/////////////////////////////////////////////////////////////////
//                      Function Decleration
/////////////////////////////////////////////////////////////////
void or32_installISR(int isr, struct ISR_HANDLE *isr_handle, void isr_func(void));

/////////////////////////////////////////////////////////////////
//                      Macro Decleration
/////////////////////////////////////////////////////////////////
#define CALL_NEXT_ISR(isr) (isr).isr_func()

#ifdef __cplusplus
}
#endif

#endif // __ISR_H__

