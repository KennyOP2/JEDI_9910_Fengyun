/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * For OpenRISC test interrupt functions. Porting from ite/ith
 *
 * @author Evan Chang
 * @version 1.0
 */
#include "intr/intr.h"
//#include "spr_defs.h"


//#define ENABLE_PICMR_HOST_INTERRUPT()       mtspr(SPR_PICMR, mfspr(SPR_PICMR) |   SPR_PICMR_HOST)
//#define DISABLE_PICMR_HOST_INTERRUPT()      mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(SPR_PICMR_HOST))


static ithIntrHandler irqHandlerTable[64];
//static ithIntrHandler fiqHandlerTable[64];
static void* irqArgTable[64];
//static void* fiqArgTable[64];

static MMP_BOOL bInterruptControllerInit = MMP_FALSE;

static void intrDefaultHandler(void* arg)
{
    // DO NOTHING
}
    
MMP_BOOL ithIntrIsInit(void)
{
	return bInterruptControllerInit;
}

void ithIntrInit(void)
{
    int i;

	if (ithIntrIsInit())
		return;
	else
	{
		or32_installISR(OR32_ISR_INT, &isr_int_handle, ithIntrDoIrq);
		bInterruptControllerInit = MMP_TRUE;
	}
    
    for (i = 0; i < 64; ++i)
        irqHandlerTable[i] = /*fiqHandlerTable[i] = */intrDefaultHandler;
}

void ithIntrRegisterHandlerIrq(ITHIntr intr, ithIntrHandler handler, void* arg)
{
	DISABLE_PICMR_HOST_INTERRUPT();

	if (intr < 64)
	{
		irqHandlerTable[intr]   = handler ? handler : intrDefaultHandler;
		irqArgTable[intr]       = arg;
	}

	ENABLE_PICMR_HOST_INTERRUPT();
}

//void ithIntrDoIrq( void ) __attribute__((naked));
void ithIntrDoIrq(void)
{
#if defined(__OPENRTOS__)
	// save return address
	asm volatile ( "stmfd   sp!, {lr}" );
#endif
	// WARNING - Do not use local (stack) variables here.  Use globals if you must!

	uint32_t intrNum    = 0;
	uint32_t intrSrc1   = 0;
	uint32_t intrSrc2   = 0;

	AHB_ReadRegister(ITH_INTR_BASE + ITH_INTR_IRQ1_STATUS_REG, &intrSrc1);    // read irq1 source
	AHB_ReadRegister(ITH_INTR_BASE + ITH_INTR_IRQ2_STATUS_REG, &intrSrc2);    // read irq2 source

	// Test to see if there is a flag set, if not then don't do anything
	while (intrSrc1 != 0)
	{
		if ((intrSrc1 & 0xFFFF) == 0)
		{
			intrNum += 16;
			intrSrc1 >>= 16;
		}

		if ((intrSrc1 & 0xFF) == 0)
		{
			intrNum += 8;
			intrSrc1 >>= 8;
		}

		if ((intrSrc1 & 0xF) == 0)
		{
			intrNum += 4;
			intrSrc1 >>= 4;
		}

		if ((intrSrc1 & 0x3) == 0)
		{
			intrNum += 2;
			intrSrc1 >>= 2;
		}

		if ((intrSrc1 & 0x1) == 0)
		{
			intrNum += 1;
			intrSrc1 >>= 1;
		}

		AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, 0x1 << intrNum, 0x1 << intrNum); // clear irq1 source

		// Get the vector and process the interrupt
		irqHandlerTable[intrNum](irqArgTable[intrNum]);

		intrSrc1 &= ~1;
	}

	intrNum = 32;

	// Test to see if there is a flag set, if not then don't do anything
	while (intrSrc2 != 0)
	{
		if ((intrSrc2 & 0xFFFF) == 0)
		{
			intrNum += 16;
			intrSrc2 >>= 16;
		}

		if ((intrSrc2 & 0xFF) == 0)
		{
			intrNum += 8;
			intrSrc2 >>= 8;
		}

		if ((intrSrc2 & 0xF) == 0)
		{
			intrNum += 4;
			intrSrc2 >>= 4;
		}

		if ((intrSrc2 & 0x3) == 0)
		{
			intrNum += 2;
			intrSrc2 >>= 2;
		}

		if ((intrSrc2 & 0x1) == 0)
		{
			intrNum += 1;
			intrSrc2 >>= 1;
		}

		AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, 0x1 << intrNum, 0x1 << intrNum); // clear irq2 source

		// Get the vector and process the interrupt
		irqHandlerTable[intrNum](irqArgTable[intrNum]);

		intrSrc2 &= ~1;
	}
#if defined(__OPENRTOS__)
	// return
	asm volatile ( "ldmfd   sp!, {pc}" );
#endif
}

void ithIntrReset(void)
{
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, 0xFFFFFFFF);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGMODE_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGLEVEL_REG, 0);
#if !defined(MM9910)
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, 0xFFFFFFFF);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGMODE_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGLEVEL_REG, 0);
#endif
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, 0xFFFFFFFF);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGMODE_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGLEVEL_REG, 0);
#if !defined(MM9910)
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, 0xFFFFFFFF);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGMODE_REG, 0);
	AHB_WriteRegister(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGLEVEL_REG, 0);
#endif
}

void ithIntrSetTriggerModeIrq(ITHIntr intr, ITHIntrTriggerMode mode)
{
    if (mode)
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGMODE_REG, 0x1<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGMODE_REG, 0x1<<(intr - 32), 0x1<<(intr - 32));
    }
    else
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGMODE_REG, 0<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGMODE_REG, 0<<(intr - 32), 0x1<<(intr - 32));
    }
}

void ithIntrSetTriggerLevelIrq(ITHIntr intr, ITHIntrTriggerLevel level)
{
    if (level)
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGLEVEL_REG, 0x1<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGLEVEL_REG, 0x1<<(intr - 32), 0x1<<(intr - 32));
    }
    else
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGLEVEL_REG, 0<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGLEVEL_REG, 0<<(intr - 32), 0x1<<(intr - 32));
    }
}

void ithIntrSetTriggerModeFiq(ITHIntr intr, ITHIntrTriggerMode mode)
{
#if defined(MM9910)
#else
    if (mode)
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGMODE_REG, 0x1<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGMODE_REG, 0x1<<(intr - 32), 0x1<<(intr - 32));
    }
    else
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGMODE_REG, 0<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGMODE_REG, 0<<(intr - 32), 0x1<<(intr - 32));
    }
#endif
}

void ithIntrSetTriggerLevelFiq(ITHIntr intr, ITHIntrTriggerLevel level)
{
#if defined(MM9910)
#else
    if (level)
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGLEVEL_REG, 0x1<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGLEVEL_REG, 0x1<<(intr - 32), 0x1<<(intr - 32));
    }
    else
    {
        if (intr < 32)
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGLEVEL_REG, 0<<intr, 0x1<<intr);
        else
            AHB_WriteRegisterMask(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGLEVEL_REG, 0<<(intr - 32), 0x1<<(intr - 32));
    }
#endif
}
