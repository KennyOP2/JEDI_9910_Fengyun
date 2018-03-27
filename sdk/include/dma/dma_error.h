/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Use as DMA error code header file.
 *
 * @author Irene Lin
 */

#ifndef DMA_ERROR_H
#define DMA_ERROR_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_DMA_BASE                                  0x90000000 //(MMP_MODULE_DNA << MMP_ERROR_OFFSET)

#define ERROR_DMA_CREATE_SEMAPHORE_FAIL                      (ERROR_DMA_BASE + 0x0001)
#define ERROR_DMA_NO_AVAILABLE_CHANNEL                       (ERROR_DMA_BASE + 0x0002)
#define ERROR_DMA_UNKONWN_ATTRIB                             (ERROR_DMA_BASE + 0x0003)
#define ERROR_DMA_TYPE_NOT_SET                               (ERROR_DMA_BASE + 0x0004)
#define ERROR_DMA_SRC_ADDR_NOT_SET                           (ERROR_DMA_BASE + 0x0005)
#define ERROR_DMA_DST_ADDR_NOT_SET                           (ERROR_DMA_BASE + 0x0006)
#define ERROR_DMA_TOTAL_SIZE_NOT_SET                         (ERROR_DMA_BASE + 0x0007)
#define ERROR_DMA_WAIT_CHANNEL_READY_TIMEOUT                 (ERROR_DMA_BASE + 0x0008)
#define ERROR_DMA_BURST_SIZE_ERROR                           (ERROR_DMA_BASE + 0x0009)
#define ERROR_DMA_UNSUPPORT_HH_HANDSHAKING_ENGINE            (ERROR_DMA_BASE + 0x000A)
#define ERROR_DMA_LLD_ADDR_NOT_ALIGN						 (ERROR_DMA_BASE + 0x000B)



#ifdef __cplusplus
}
#endif

#endif
