/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia DMA Driver API header file.
 *
 * @author Irene Lin
 */
#ifndef MMP_DMA_H
#define MMP_DMA_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if defined(_WIN32) || defined(_WIN32_WCE)

	#if defined(DMA_EXPORTS)
		#define DMA_API __declspec(dllexport)
	#else
		#define DMA_API __declspec(dllimport)
	#endif
#else
	#define DMA_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */


//=============================================================================
//                              Include Files
//=============================================================================
#include "mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MMP_DMA_ATTRIB_TAG
{
    MMP_DMA_ATTRIB_DMA_TYPE               = 0,
    MMP_DMA_ATTRIB_SRC_ADDR               = 1,
    MMP_DMA_ATTRIB_DST_ADDR               = 2,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE     = 3,
    /** 
     * For MMP_DMA_TYPE_MEM_TO_MS and MMP_DMA_TYPE_MS_TO_MEM with DMA normal use.
     * Size not change, but set again. 
     */
    MMP_DMA_ATTRIB_SIZE_SET_AGAIN         = 4,
    /** For Memory <--> Engine with hardware handshaking mode. */
    MMP_DMA_ATTRIB_HW_HANDSHAKING         = 5,
    /** 
     * Byte unit. 
     * Only 1/2/4 bytes can be set. 
     */
    MMP_DMA_ATTRIB_SRC_TX_WIDTH           = 6,  
    MMP_DMA_ATTRIB_DST_TX_WIDTH           = 7,  
    /** 
     * "source transfer width" unit. 
     *  Only 1/4/8/16/32/64/128/256 can be set. 
     */
    MMP_DMA_ATTRIB_SRC_BURST_SIZE         = 8,  
	MMP_DMA_ATTRIB_LLD_ADDR               = 9,  
	/** priority: 0~3, 0:lowest priority, 3:highest priority */
	MMP_DMA_ATTRIB_PRIORITY               = 10,
	/*
	FIFO threshold value 
	*/
	MMP_DMA_ATTRIB_FIFO_TH				  = 11,
	MMP_DMA_ATTRIB_NONE                   = 0xFFFFFFFF
} MMP_DMA_ATTRIB;

typedef enum MMP_DMA_TYPE_TAG
{
    MMP_DMA_TYPE_MEM_TO_MEM               = 1,
    /** 
     * It's 4-bytes memset. MMP_DMA_ATTRIB_SRC_ADDR must fill with 4-btyes pattern.
     */
    MMP_DMA_TYPE_MEM_SET                  = 2,
    MMP_DMA_TYPE_MEM_TO_MS                = 3,
    MMP_DMA_TYPE_MS_TO_MEM                = 4,
    MMP_DMA_TYPE_MEM_TO_CF                = 5,
    MMP_DMA_TYPE_CF_TO_MEM                = 6,
    MMP_DMA_TYPE_MEM_TO_SD                = 7,
    MMP_DMA_TYPE_SD_TO_MEM                = 8,
    MMP_DMA_TYPE_MEM_TO_XD                = 9,
    MMP_DMA_TYPE_XD_TO_MEM                = 10,
    MMP_DMA_TYPE_MEM_TO_SPDIF             = 11,
    MMP_DMA_TYPE_SPI_TO_MEM               = 12,
    MMP_DMA_TYPE_MEM_TO_SPI               = 13,
	MMP_DMA_TYPE_SPI2_TO_MEM              = 14,
	MMP_DMA_TYPE_MEM_TO_SPI2              = 15,
    MMP_DMA_TYPE_APB_TO_SPI               = 16,
    MMP_DMA_TYPE_UART_TO_MEM              = 17,
    MMP_DMA_TYPE_MEM_TO_UART              = 18,
	MMP_DMA_TYPE_UART2_TO_MEM             = 19,
	MMP_DMA_TYPE_MEM_TO_UART2             = 20,
	MMP_DMA_TYPE_IRDA_TO_MEM			  = 21,
	MMP_DMA_TYPE_MEM_TO_IRDA			  = 22,
    MMP_DMA_TYPE_IR_TO_MEM   			  = 23,
    MMP_DMA_TYPE_MEM_TO_IR   			  = 24
} MMP_DMA_TYPE;

/* dma transfer width */
#define DMA_TX_WIDTH_8     0
#define DMA_TX_WIDTH_16    1
#define DMA_TX_WIDTH_32    2

/** dma address control */
#define DMA_ADDR_CTRL_INC   0
#define DMA_ADDR_CTRL_DEC   1
#define DMA_ADDR_CTRL_FIX   2


/** dma linked list descriptor */
typedef struct {
    MMP_UINT32 srcAddr;
    MMP_UINT32 dstAddr;
    MMP_UINT32 llp;
#define LLD_SRC_WIDTH_SHT       25
#define LLD_DST_WIDTH_SHT       22
#define LLD_SRC_ADDR_CTRL_SHT   20
#define LLD_DST_ADDR_CTRL_SHT   18
#define LLD_SRC_SEL				17
#define LLD_DST_SEL				16

#define LLD_INTR_TC_MSK			(0x1<<28)

    MMP_UINT32 control;
    MMP_UINT32 txSize;
} MMP_DMA_LLD;


//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef void*   MMP_DMA_CONTEXT;

#define DMA_IRQ_FLAGS_TC    (0x00000001 << 0)
#define DMA_IRQ_FLAGS_ERR   (0x00000001 << 1)
#define DMA_IRQ_FLAGS_ABT   (0x00000001 << 2)
#define DMA_IRQ_SUCCESS     DMA_IRQ_FLAGS_TC
typedef void (*dmaIntrHandler)(void* arg, MMP_UINT32 status);



//=============================================================================
//                              Function Declaration
//=============================================================================
/** @defgroup group15 SMedia DMA Driver API
 *  The DMA module API.
 *  @{
 */

/**
 * DAM Controller Initialization.
 *
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpDmaCreateContext()
 */
DMA_API MMP_INT mmpDmaInitialize(void);

/**
 * DAM Controller Create channel context.
 *
 * @param dmaCtxt        DMA channel context.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpDmaDestroyContext()
 */
DMA_API MMP_INT mmpDmaCreateContext(MMP_DMA_CONTEXT* dmaCtxt);

/**
* DAM Controller reset the channel's software context.
*
* @param dmaCtxt        DMA channel context.
*
* @see mmpDmaCreateContext()
*/
DMA_API MMP_INT mmpDmaResetContext(MMP_DMA_CONTEXT dmaCtxt);

/**
 * DAM Controller Destroy channel context.
 *
 * @param dmaCtxt        DMA channel context.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpDmaCreateContext()
 */
DMA_API MMP_INT mmpDmaDestroyContext(MMP_DMA_CONTEXT dmaCtxt);

/**
 * Set channel attribute.
 *
 *
 * @param dmaCtxt        DMA channel context.
 * @param attribList     DMA Channel attribute.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpDmaCreateContext()
 */
DMA_API MMP_INT mmpDmaSetAttrib(MMP_DMA_CONTEXT dmaCtxt, const MMP_UINT32*  attribList);

/**
 * Fire related channel.
 *
 * @param dmaCtxt        DMA channel context.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpDmaWaitIdle()
 */            
DMA_API MMP_INT mmpDmaFire(MMP_DMA_CONTEXT dmaCtxt);

/**
 * Wait channel idle. 
 *
 * @param dmaCtxt        DMA channel context.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpDmaFire()
 */                            
DMA_API MMP_INT mmpDmaWaitIdle(MMP_DMA_CONTEXT dmaCtxt);

DMA_API MMP_INT mmpDmaBusyWaitIdle(MMP_DMA_CONTEXT dmaCtxt);

DMA_API MMP_INT mmpDmaRegisterIsr(MMP_DMA_CONTEXT dmaCtxt, dmaIntrHandler handler, void* arg);

DMA_API void mmpDmaDumpReg(MMP_DMA_CONTEXT   dmaCtxt);

//@}

#ifdef __cplusplus
}
#endif

#endif /* MMP_DMA_H */
