/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * EHCI hc_driver implementation.
 *
 * @author Irene Lin
 */
//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"
#if !defined(DISABLE_USB_DEVICE)
#include "usb/device/device.h"
#endif
#if defined(USB_IRQ_ENABLE) && defined(MM680)
#include "mmp_intr.h"
#endif

#if defined(CONFIG_HAVE_USBD)
#include "usbd/inc/it_usbd.h"
void*   p_it_usbd_handle = NULL;
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define DEVICE_OPEN_TIMEOUT    1000

//=============================================================================
//                              Macro Definition
//=============================================================================
//=============================================================================
//                              Global Data Definition
//=============================================================================
#if !defined(DISABLE_USB_DEVICE)
/** for device driver task use */
enum
{
    OTG_INFORM_FLAG_TERMINATE      = 0x00000001,
    OTG_INFORM_FLAG_DEVICE_CLOSE   = 0x00000002
};

typedef struct OTG_MGR_TAG
{
    MMP_EVENT           eventThreadToApi;
    MMP_EVENT           eventApiToThread;
    MMP_UINT            informFlag;
} OTG_MGR;

static OTG_MGR  otgMgr = {0};
extern OTG_DEVICE_GET_MAX_LUN_NUM   OTG_DEVICE_GetMaxLunNum;
extern OTG_DEVICE_INITIALIZE   OTG_DEVICE_Initialize;
extern OTG_DEVICE_TERMINATE    OTG_DEVICE_Terminate;
extern OTG_DEVICE_GET_CAPACITY OTG_DEVICE_GetCapacity;
extern OTG_DEVICE_READ_SECTOR  OTG_DEVICE_ReadSector;
extern OTG_DEVICE_WRITE_SECTOR OTG_DEVICE_WriteSector;
extern OTG_DEVICE_RESPONSE     OTG_DEVICE_Response;
extern OTG_DEVICE_INQUIRY      OTG_DEVICE_Inquiry;
extern OTG_DEVICE_IS_LOCK      OTG_DEVICE_IsLock;
extern OTG_DEVICE_EJECT        OTG_DEVICE_Eject;
#endif // #if !defined(DISABLE_USB_DEVICE)


#ifdef USB_LOGO_TEST
//2008.5.5 Jack add, for USB_LOGO_TEST USE, To Record Suspend Status
extern MMP_UINT8 status_suspend;
extern MMP_UINT8 timeout_times;
#endif

static struct usb_hcd* otg_hcd;
#ifdef USB_CHARGE_ENABLE
static MMP_BOOL isUSBChargeState;
#endif

#if defined(USB_IRQ_ENABLE)
#if !defined(MM680)
#include "usb_interrupt.c"
#endif
void*	isr_event;
#endif

//=============================================================================
//                              Private Function Definition
//=============================================================================
void* USBEX_ThreadFunc(void* data)
{
    struct ehci_hcd* ehci0 = MMP_NULL;
    struct ehci_hcd* ehci1 = MMP_NULL;

    if(hcd0)	ehci0 = hcd_to_ehci(hcd0);
    if(hcd1)    ehci1 = hcd_to_ehci(hcd1);

#if defined(USB_IRQ_ENABLE)
    isr_event = SYS_CreateEvent();
    while(1)
    {
        #if defined(POLLING_REMOVE)
        {
            MMP_INT res = 0;
            res = SYS_WaitEvent(isr_event, 30);
            if(res) /** timeout */
            {
                if(hcd0)	ehci_isDisconnect(hcd0);
                if(hcd1)	ehci_isDisconnect(hcd1);
            }
        }
        #else
        SYS_WaitForEventForever(isr_event);
        #endif

        if(hcd0 && ehci0->tasklet)
            ehci_tasklet(ehci0);

        if(hcd1 && ehci1->tasklet)
            ehci_tasklet(ehci1);
    }

#else
    while(1)
    {
        if(hcd0)
        {
            if(ehci0->tasklet)
                ehci_tasklet(ehci0);
            ehci_irq(hcd0);
        }
        if(hcd1)
        {
            if(ehci1->tasklet)
                ehci_tasklet(ehci1);
            ehci_irq(hcd1);
        }

        if(hcd0 && !hcd1)		{ if(!ehci0->tasklet) MMP_Sleep(1); }
        else if(!hcd0 && hcd1) 	{ if(!ehci1->tasklet) MMP_Sleep(1); }
        else					{ if(!ehci0->tasklet && !ehci1->tasklet) MMP_Sleep(1); }
    }
#endif
}

void* DEVICE_ThreadFunc(void* data)
{
#if !defined(DISABLE_USB_DEVICE)

#if defined(CONFIG_HAVE_USBD)
f_enterFS();
#endif

waitEvent:
    SYS_WaitForEventForever(otgMgr.eventApiToThread);

    while(1)
    {
        if(otgMgr.informFlag & OTG_INFORM_FLAG_TERMINATE)
        {
            goto end;
        }
        if(otgMgr.informFlag & OTG_INFORM_FLAG_DEVICE_CLOSE)
        {
            otgMgr.informFlag &= ~OTG_INFORM_FLAG_DEVICE_CLOSE;
            SYS_SetEvent(otgMgr.eventThreadToApi);
            goto waitEvent;
        }
        USB_DEVICE_Handler();
        MMP_Sleep(1);
    }

end:
    SYS_SetEvent(otgMgr.eventThreadToApi);
#endif // #if !defined(DISABLE_USB_DEVICE)
    return MMP_NULL;
}


//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_INT mmpUsbExInitialize(int usb_enable)
{
    MMP_INT result = 0;
    MMP_UINT16 value = 0;
    MMP_UINT32 usb0 = 0x0;
    MMP_UINT32 usb1 = 0x1;
    MMP_UINT16 packageType = 0;
    MMP_UINT32  usb0_en = usb_enable & (1 << 0);
    MMP_UINT32  usb1_en = usb_enable & (1 << 1);

    if(!usb_enable)
    {
        result = -1;
        goto end;
    }

#if defined(MM9910)
    if(usb0_en)
        HOST_WriteRegisterMask(0x904, 0x10, 0x10);  /** D[4]=1 : usb0 phy power on */
    if(usb1_en)
        HOST_WriteRegisterMask(0x90C, 0x10, 0x10);  /** D[4]=1 : usb1 phy power on */
    MMP_Sleep(10);

    if(0)
    {
        MMP_UINT32 cnt = 1000;
        if(usb0_en)
        {
            while(cnt)
            {
                HOST_ReadRegister(0x908, &value);
                if(value & 0x4000) break;
                cnt--;
            }
            if(cnt==0)  {  printf(" USB0 controller's input clock not work!!! \n\n");  while(1);  }
        }
        if(usb1_en)
        {
            cnt = 1000;
            while(cnt)
            {
                HOST_ReadRegister(0x910, &value);
                MMP_Sleep(1);
                if(value & 0x4000) break;
                cnt--;
            }
            if(cnt==0)  {  printf(" USB1 controller's input clock not work!!! \n\n");  while(1);  }
        }
        value = 0;
    }

    HOST_WriteRegister(0x46, 0x100A);
    MMP_Sleep(5);
    HOST_WriteRegister(0x46, 0x000A);
    MMP_Sleep(5);

    //HOST_WriteRegister(0x900, 0);
    //HOST_WriteRegisterMask(0x168c, (0x1<<9), (0x1<<9)); // for risc
    //HOST_WriteRegisterMask(0x900, (0x1<<8), (0x1<<8));  // for usb AMBA path

    #if !defined(DISABLE_USB_DEVICE)
    {
        ithGpioSetMode(4, 1); /* GPIO4: mode 1 */
        /** config usb */
        #if defined(USB0_OTG_USB1_HOST)
        HOST_WriteRegisterMask(0x900, 0x1, 0x1);
        usb0 |= USB_HAS_DEVICE_MODE; /** usb0 has device function */
        #else
        usb1 |= USB_HAS_DEVICE_MODE; /** usb1 has device function */
        #endif
    }
    #endif
#elif defined(MM9070)
    ithUsbEnableClock();
    if(usb0_en)
    {
        HOST_WriteRegisterMask(0x904, 0x40, 0x40);  /** D[6]=1 : usb0 PLL 30MHz */
        HOST_WriteRegisterMask(0x904, 0x10, 0x10);  /** D[4]=1 : usb0 phy power on */
    }
    if(usb1_en)
    {
        HOST_WriteRegisterMask(0x90C, 0x40, 0x40);  /** D[6]=1 : usb1 PLL 30MHz */
        HOST_WriteRegisterMask(0x90C, 0x10, 0x10);  /** D[4]=1 : usb1 phy power on */
    }
    MMP_Sleep(10); /** for usb controller 30MHz stable (from phy) */

    /** reset usb engine */
    HOST_WriteRegisterMask(0x0046, 0x1000, 0x1000);
    MMP_Sleep(5);
    HOST_WriteRegisterMask(0x0046, 0x0000, 0x1000);
    MMP_Sleep(5);

    /** test code */
    //HOST_WriteRegisterMask(0x168c, (0x1<<9), (0x1<<9)); // for risc
    HOST_WriteRegisterMask(0x900, (0x1<<8), (0x1<<8));  // for usb AMBA path

  #if !defined(DISABLE_USB_DEVICE)
    /** for Device mode setting */
    #if defined(ALPHA_SDK)
    ithGpioSetMode(8, ITH_GPIO_MODE1);
    #else
    AHB_WriteRegisterMask((GPIO_BASE+0x90), 0x10000, 0x30000); /** GPIO8 as ID pin */
    #endif

    /** config usb */
    #if defined(USB0_OTG_USB1_HOST)
    HOST_WriteRegisterMask(0x900, 0x1, 0x1);
    usb0 |= USB_HAS_DEVICE_MODE; /** usb0 has device function */
    #else
    usb1 |= USB_HAS_DEVICE_MODE; /** usb1 has device function */
    #endif
  #endif //#if !defined(DISABLE_USB_DEVICE)

    /** for performance issue */
    HOST_WriteRegister(0x914, 0x4627);
    HOST_WriteRegister(0x916, 0x8);
#endif

    #if defined(RUN_FPGA)
    USB_DEVICE_EnableHalfSpeedReg();
    #endif

    result = urbBufInitialize();
    if(result)
        goto end;

    /** usb host controller basic init */
    if(usb0_en)
    {
        result = ehci_hcd_init(usb0);
        if(result)
            goto end;
    }
    if(usb1_en)
    {
        result = ehci_hcd_init(usb1);
        if(result)
            goto end;
    }

    if(usb0 & USB_HAS_DEVICE_MODE)
        otg_hcd = hcd0;
    else
        otg_hcd = hcd1;

    /** This is device driver task, and it will never be destroyed. */
    #if !defined(DISABLE_USB_DEVICE)
    memset((void*)&otgMgr, 0, sizeof(OTG_MGR));
    otgMgr.eventApiToThread = SYS_CreateEvent();
    otgMgr.eventThreadToApi = SYS_CreateEvent();
    #endif

#if defined(IT9063)
    AHB_WriteRegisterMask((USB0_BASE+0x80), 0x1<<12, 0x1<<12);
#endif

#if defined(USB_IRQ_ENABLE)
    usbIntrEnable();
#endif

end:
    if(result)
        LOG_ERROR " mmpUsbExInitialize() return error code 0x%08X \n", result LOG_END
    return result;
}

MMP_UINT32 mmpUsbExCheckDeviceState(MMP_INT usb, MMP_INT* state, USB_DEVICE_INFO* device_info)
{
    MMP_INT result = 0;
    MMP_UINT32 temp = 0;
    struct ehci_hcd* ehci = MMP_NULL;
    struct usb_device* dev=MMP_NULL;
    struct usb_hcd* hcd = MMP_NULL;
    MMP_BOOL exit = (usb & 0x10) ? MMP_TRUE : MMP_FALSE; /** for suspend use */
    usb &= ~0x10;

    (*state) = 0;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    ehci = hcd_to_ehci(hcd);
    AHB_ReadRegister(ehci->regs.port_status[0], &temp);
    //if(temp & PORT_CONNECT) /** device present */
    if((temp & PORT_CONNECT) && (exit==MMP_FALSE)) /** device present */
    {
        if(!hcd->connect)
        {
            result = ehci_init(hcd, &dev);
            if(result)
                goto end;

            if(dev->driverNum)
            {
                (*state) = USB_DEVICE_STATE_CONNECT;
                memcpy((void*)device_info, &dev->device_info[0], sizeof(USB_DEVICE_INFO));
            }
        }
    }
    else
    {
        if(hcd->connect)
        {
            (*state) = USB_DEVICE_STATE_DISCONNECT;
            device_info->ctxt = MMP_NULL;
            device_info->type = hcd->driver->stop(hcd);
        }
    }

end:
    if(result)
        LOG_ERROR " mmpUsbExCheckDeviceState() return error code 0x%08X \n", ((result<0)? (-result) : result) LOG_END
    return result;
}

MMP_BOOL
mmpUsbExUsb0IsOtg(void)
{
    return (hcd0->index & USB_HAS_DEVICE_MODE) ? MMP_TRUE : MMP_FALSE;
}

/** Irene: get connected usb device speed routine. */
MMP_INT
mmpUsbExGetSpeed(MMP_INT usb)
{
    struct ehci_hcd* ehci = MMP_NULL;
    struct usb_hcd* hcd = MMP_NULL;
    MMP_INT speed;
    MMP_UINT32 value;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    if(hcd->connect)
    {
        ehci = hcd_to_ehci(hcd);
        AHB_ReadRegister(ehci->otg_regs.ctrl_status, &value);
        speed = HOST_SPEED(value);

        if(speed == FULL_SPEED)
            return USBEX_SPEED_FULL;
        else if(speed == LOW_SPEED)
            return USBEX_SPEED_LOW;
        else if(speed == HIGH_SPEED)
            return USBEX_SPEED_HIGH;
    }
end:
    return USBEX_SPEED_UNKNOWN;
}

//=============================================================================
/**
 * Return OTG Device mode device is connect or not
 *
 * @return MMP_TRUE if device is connect, return MMP_FALSE if device is not connect.
 * @see mmpOtgDeviceModeOpen()
 */
//=============================================================================
#if !defined(DISABLE_USB_DEVICE)
MMP_BOOL
mmpOtgIsDeviceMode(void)
{
#ifdef USB_LOGO_TEST
    if (status_suspend == 1)
    {
        if (timeout_times > 0)
        {
            timeout_times--;
            return MMP_TRUE;
        }
        else
        {
            status_suspend = 0;
            return MMP_FALSE;
        }
    }
    else
    {
        if(USB_OTG_GetCurrentIdReg() == USB_OTG_ID_B)
            return MMP_TRUE;
        else
            return MMP_FALSE;
    }
#else
    if(USB_OTG_GetCurrentIdReg() == USB_OTG_ID_B)
        return MMP_TRUE;
    else
        return MMP_FALSE;
#endif
}


//=============================================================================
/**
 * Setup OTG Device mode at open device connect
 *
 * @param attribList        OTG device mode need call back function pointers.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 * @see mmpOtgDeviceModeClose()
 */
//=============================================================================
MMP_INT
mmpOtgDeviceModeOpen(
    const MMP_ULONG*  attribList)
{
    MMP_INT result = 0;
    struct ehci_hcd* ehci;
    MMP_UINT32 status=0;
    MMP_UINT32 timeOut = 0;

    LOG_ENTER "[mmpOtgDeviceOpen] Enter \n" LOG_END

#if !defined(CONFIG_HAVE_USBD)
    if(!attribList)
    {
        result = ERROR_OTG_INIT_ATTRIB_WRONG;
        goto end;
    }

    /**
     * Get the storage device driver's call back function.
     */
    while(*attribList != MMP_OTG_ATTRIB_NONE)
    {
        switch(*attribList++)
        {
        case MMP_OTG_ATTRIB_DEVICE_INITIALIZE:
            OTG_DEVICE_Initialize = (OTG_DEVICE_INITIALIZE)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_TERMINATE:
            OTG_DEVICE_Terminate = (OTG_DEVICE_TERMINATE)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_GET_CAPACITY:
            OTG_DEVICE_GetCapacity = (OTG_DEVICE_GET_CAPACITY)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_READ_SECTOR:
            OTG_DEVICE_ReadSector = (OTG_DEVICE_READ_SECTOR)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_WRITE_SECTOR:
            OTG_DEVICE_WriteSector = (OTG_DEVICE_WRITE_SECTOR)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_GET_MAX_LUN_NUM:
            OTG_DEVICE_GetMaxLunNum = (OTG_DEVICE_GET_MAX_LUN_NUM)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_RESPONSE:
            OTG_DEVICE_Response = (OTG_DEVICE_RESPONSE)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_INQUIRY:
            OTG_DEVICE_Inquiry = (OTG_DEVICE_INQUIRY)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_IS_LOCK:
            OTG_DEVICE_IsLock = (OTG_DEVICE_IS_LOCK)*attribList++;
            break;
        case MMP_OTG_ATTRIB_DEVICE_EJECT:
            OTG_DEVICE_Eject = (OTG_DEVICE_EJECT)*attribList++;
            break;
        default:
            result = ERROR_OTG_INIT_ATTRIB_WRONG1;
            goto end;
        }
    };
#endif

    do
    {
        ehci = hcd_to_ehci(otg_hcd);
        USB_OTGC_A_Bus_Drop(); /** 2008-09-22 Irene: Faraday suggestion */
        // A => B
        if(ehci->otg_regs.curr_role == ROLE_HOST)
        {
            // If A-Vbus still valid
            AHB_ReadRegister(ehci->otg_regs.ctrl_status, &status);
            if(status & USB_OTG_MSK_HOST_VBUS_VALID)
            {
                MMP_UINT32 tmp=0;
                AHB_ReadRegister(ehci->regs.intr_enable, &tmp);
                // host fully close
                if(tmp & USB_HOST_MSK_INTERRUPT)
                {
                    if(HOST_SPEED(status)==FULL_SPEED) // Full speed
                        USB_EHCI_Host20_Close(1);
                    else
                        USB_EHCI_Host20_Close(0);
                }

                // wait role change ready
                result = USB_OTG_WaitChangeToDeviceRoleReg();
                if(result)
                    goto end;

                ehci->otg_regs.curr_role = ROLE_DEVICE;
            }
        }
        else
        {
            AHB_ReadRegister(USB_OTG_REG_CONTROL_STATUS, &status);
            if(status & USB_OTG_MSK_HOST_VBUS_VALID)
            {
#if defined(CONFIG_HAVE_USBD)
                if (!(p_it_usbd_handle = it_usbd_open()))
                {
                    result = ERROR_USB_DEVICE_OPEN_FAIL;
                    goto end;
                }
#else
                result = OTG_OTGP_main(1,1,0);
                if(result)
                    goto end;
#endif
                break;
            }
        }
        MMP_Sleep(1);
        timeOut++;
        if(timeOut > DEVICE_OPEN_TIMEOUT)
        {
            result = ERROR_USB_DEVICE_OPEN_FAIL;
            goto end;
        }
    }while(1);

    SYS_SetEvent(otgMgr.eventApiToThread);

    #if defined(USB_CHARGE_ENABLE)
    {
        MMP_BOOL charge;
        MMP_UINT32 timeout = 800; // 800 ms
        USB_DEVICE_SetIsUSBCharge(MMP_TRUE);

        do
        {
            MMP_Sleep(1);
            charge = USB_DEVICE_GetIsUSBCharge();
            timeout--;
            if(!timeout)
                break;
        } while(charge);

        if(timeout==0) /** It's chagrge mode or enter device mode fail! */
        {
            isUSBChargeState = MMP_TRUE;
            result = ERROR_USB_DEVICE_OPEN_FAIL;
        }
        else /** Enter normal device mode. */
        {
            isUSBChargeState = MMP_FALSE;
        }
    }
    #endif

end:
    LOG_LEAVE "[mmpOtgDeviceOpen] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpOtgDeviceOpen() return error code 0x%08X \n", result LOG_END

    return result;
}

#if defined(USB_CHARGE_ENABLE)
MMP_BOOL
mmpOtgisUSBChargeState(void)
{
   return isUSBChargeState;
}
#endif

MMP_INT
mmpOtgDeviceModeClose(void)
{
    MMP_INT result = 0;

    otgMgr.informFlag |= OTG_INFORM_FLAG_DEVICE_CLOSE;
    SYS_WaitForEventForever(otgMgr.eventThreadToApi);

#if defined(CONFIG_HAVE_USBD)
    it_usbd_close(p_it_usbd_handle);
    p_it_usbd_handle = NULL;
#else
    OTG_OTGP_Close();
#endif
    USB_OTGC_A_Bus_Drive(); /** 2008-09-22 Irene: Faraday suggestion */

    return 0;
}
#else  // #if !defined(DISABLE_USB_DEVICE)
MMP_BOOL mmpOtgIsDeviceMode(void)
{
    return MMP_FALSE;
}

MMP_INT mmpOtgDeviceModeOpen(
    const MMP_ULONG*  attribList)
{
    return 0;
}

#if defined(USB_CHARGE_ENABLE)
MMP_BOOL mmpOtgisUSBChargeState(void)
{
   return MMP_FALSE;
}
#endif

MMP_INT mmpOtgDeviceModeClose(void)
{
    return 0;
}
#endif  // #if !defined(DISABLE_USB_DEVICE)

#if defined(USB_LOGO_TEST)

MMP_INT
mmpUsbExPortControl(
    MMP_INT     usb,
    MMP_UINT32  ctrl)
{
    MMP_INT result=0;
    struct ehci_hcd* ehci = MMP_NULL;
    struct usb_hcd* hcd = MMP_NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    if(!hcd)
        goto end;

    ehci = hcd_to_ehci(hcd);

    switch(ctrl)
    {
    case USB_PORT_TEST_PACKET:
        AHB_WriteRegister((hcd->iobase+0x0118), 0x1);
        break;
    case USB_PORT_TEST_J_STATE:
    case USB_PORT_TEST_K_STATE:
    case USB_PORT_TEST_SE0_NAK:
        AHB_WriteRegisterMask(ehci->regs.port_status[0], (ctrl<<PORT_TEST_SHT), PORT_TEST_MSK);
        break;
    case USB_PORT_TEST_FORCE_EN:
        AHB_WriteRegisterMask(ehci->regs.port_status[0], (ctrl<<PORT_TEST_SHT), PORT_TEST_MSK);
        MMP_Sleep(100);
        AHB_WriteRegisterMask(ehci->regs.port_status[0], 0x0, PORT_TEST_MSK);
        break;
    }

end:
    return result;
}

extern MMP_INT usb_internal_control_msg_step(
    struct usb_device* usb_dev,
    MMP_UINT32  pipe,
    devrequest* cmd,
    void*       data,
    MMP_INT     len,
    MMP_INT     timeout,
    MMP_INT     step);

MMP_INT
mmpUsbExDeviceControl(
    void*       dev,
    MMP_UINT32  ctrl,
    MMP_UINT32  step,
    MMP_UINT8*  data)
{
    MMP_INT result=0;
    struct usb_device* usb_dev = (struct usb_device*)dev;
    devrequest dr = {0};

    switch(ctrl)
    {
    case USB_DEV_CTRL_SINGLE_STEP_GET_DEV:
        {
            if((step<0) || (step>2))
                return -1;

            if(step==0)
            {
                dr.requesttype = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
                dr.request = USB_REQ_GET_DESCRIPTOR;
                dr.value = cpu_to_le16(USB_DT_DEVICE<< 8);
                dr.index = 0;
                dr.length = cpu_to_le16(0x12);
            }
            result = usb_internal_control_msg_step(usb_dev,
                                                   usb_rcvctrlpipe(usb_dev, 0),
                                                   &dr,
                                                   data,
                                                   0x12,
                                                   500,
                                                   step);
            if(result > 0)
                result = 0;
        }
        break;
    case USB_DEV_CTRL_SINGLE_STEP_SET_FEATURE:
        {
            if((step<0) || (step>1))
                return -1;

            if(step==0)
            {
                dr.requesttype = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
                dr.request = USB_REQ_SET_FEATURE;
                dr.value = cpu_to_le16(0x2);    /** Test mode (statndard feature selectors) */
                dr.index = cpu_to_le16(0x3<<8); /** Test_SE0_NAK (Test Mode Selectors) */
                dr.length = 0;
            }
            result = usb_internal_control_msg_step(usb_dev,
                                                   usb_sndctrlpipe(usb_dev, 0),
                                                   &dr,
                                                   MMP_NULL,
                                                   0,
                                                   500,
                                                   step);
            if(result > 0)
                result = 0;
        }
        break;
    }

end:
    if(result < 0)
        LOG_ERROR " mmpUsbExDeviceControl() ctrl %d, step %d, result = %d \n", ctrl, step, result LOG_END
    return result;
}

MMP_BOOL
mmpUsbExIsDeviceConnect(MMP_INT usb)
{
    struct ehci_hcd* ehci = MMP_NULL;
    struct usb_hcd* hcd = MMP_NULL;
    MMP_UINT32 status = 0;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    ehci = hcd_to_ehci(hcd);
    AHB_ReadRegister(ehci->regs.port_status[0], &status);
    if(status & PORT_CONNECT)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

MMP_INT
mmpUsbExSuspend(MMP_INT usb)
{
    struct usb_hcd* hcd = MMP_NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    return hcd->driver->suspend(hcd, 0);
}

MMP_INT
mmpUsbExResume(MMP_INT usb)
{
    struct usb_hcd* hcd = MMP_NULL;

    if(usb == USB0)
        hcd = hcd0;
    else if(usb == USB1)
        hcd = hcd1;

    return hcd->driver->resume(hcd);
}


#endif


