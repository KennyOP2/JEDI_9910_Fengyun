
#if defined(USB_IRQ_ENABLE)
#if defined(__OPENRTOS__)
#include "ite/ith.h"
#elif defined(__FREERTOS__)
#include "intr/intr.h"
#define ithPrintf   printf
#endif


static MMP_INLINE void otg_isr(struct ehci_hcd* ehci)
{
    MMP_UINT32 status;
    AHB_ReadRegister(ehci->otg_regs.intr_status, &status);
    AHB_WriteRegister(ehci->otg_regs.intr_status, status);
    //ithPrintf(" otg intr: 0x%08X \n", status);
}

static MMP_INLINE void device_isr(void)
{
    //ithPrintf(" #### device intr \n");
}

static void usb_isr(void* arg)
{
    struct usb_hcd* hcd = (struct usb_hcd*)arg;
    struct ehci_hcd* ehci = hcd_to_ehci(hcd);
    MMP_UINT32 status, status1;

    AHB_ReadRegister(ehci->global_regs.intr_status, &status);
    AHB_WriteRegister(ehci->global_regs.intr_status, status);
    if(status & G_HC_INTR)
    {
        AHB_ReadRegister(ehci->regs.status, &status1);
        if(status1 & STS_PCD)
        {
            AHB_ReadRegister(ehci->regs.port_status[0], &status1);
            AHB_WriteRegister(ehci->regs.port_status[0], status1);
        }
        ehci_irq(hcd);
    }
    if(status & G_OTG_INTR)
    {
        otg_isr(ehci);
        //ithPrintf(" usb: 0x%X \n", status);
    }
    if(status & G_DEVICE_INTR)
        device_isr();
}

static MMP_INLINE void usbIntrEnable(void)
{
    MMP_UINT32 mask = G_HC_INTR|G_OTG_INTR|G_DEVICE_INTR;
    MMP_UINT32 intr_en = G_HC_INTR|G_OTG_INTR; // just disable device mode interrupt
    struct ehci_hcd* ehci = 0;

	/** register interrupt handler to interrupt mgr */
	if(hcd0)
	{
		ehci = hcd_to_ehci(hcd0);
		ithIntrRegisterHandlerIrq(ITH_INTR_USB0, usb_isr, (void*)hcd0);
		ithIntrSetTriggerModeIrq(ITH_INTR_USB0, ITH_INTR_LEVEL);
		ithIntrSetTriggerLevelIrq(ITH_INTR_USB0, ITH_INTR_LOW_FALLING);
		ithIntrEnableIrq(ITH_INTR_USB0);
		/** enable usb0 interrupt */
		AHB_WriteRegisterMask(ehci->global_regs.intr_mask, ~intr_en, mask);
	}
	if(hcd1)
	{
		ehci = hcd_to_ehci(hcd1);
		/** register interrupt handler to interrupt mgr */
		ithIntrRegisterHandlerIrq(ITH_INTR_USB1, usb_isr, (void*)hcd1);
		ithIntrSetTriggerModeIrq(ITH_INTR_USB1, ITH_INTR_LEVEL);
		ithIntrSetTriggerLevelIrq(ITH_INTR_USB1, ITH_INTR_LOW_FALLING);
		ithIntrEnableIrq(ITH_INTR_USB1);
		/** enable usb1 interrupt */
		AHB_WriteRegisterMask(ehci->global_regs.intr_mask, ~intr_en, mask);
	}
}

#else

#define usbIntrEnable()

#endif
