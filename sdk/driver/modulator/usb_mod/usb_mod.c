/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file usb_mod.c
 * Driver for USB Modulator(Eagle) compliant devices.
 *
 * @author Barry Wu
 */
//=============================================================================
//                              Include Files
//=============================================================================

#include "usb_mod.h"
#include "usb_mod_transport.h"
#include "mmp_modulator.h"
//#include "usb/ite_usb.h"
//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                   Gloabl Data
//=============================================================================
/** currently support 2 context for two USB host controller each */
static struct usbtuner_data ut_list[2];

static struct usb_device_id mod_usb_ids[] = 
{
    { USB_INTERFACE_INFO(USB_CLASS_VENDOR_SPEC, 0, 0) }
};


//=============================================================================
//                   Private function definition
//=============================================================================

//=============================================================================
//                   Public function definition
//=============================================================================
static void* usb_mod_probe(struct usb_device *dev, MMP_UINT intf, const struct usb_device_id *id)
{
    const int id_index = id - mod_usb_ids;
    struct usbtuner_data* ss = 0;
    struct usb_interface *usb_interface = dev->actconfig->usb_interface + intf;
    struct usb_interface_descriptor *altsetting = usb_interface->altsetting + usb_interface->act_altsetting;
    MMP_UINT8 i = 0;

    if(id_index >= sizeof(mod_usb_ids)/sizeof(mod_usb_ids[0]))
        goto end;

    LOG_INFO "......... USB Mod(Eagle) device detected!\n" LOG_END

    /** decide the context */
    if(!ut_list[0].pusb_dev)
        ss = &ut_list[0];
    else if(!ut_list[1].pusb_dev)
        ss = &ut_list[1];
    else
    {
        LOG_ERROR " No context available??? \n" LOG_END
        goto end;
    }
    
    dev->type = USB_DEVICE_TYPE_MSC;
    ss->pusb_dev = dev;
    
    /** Find the endpoints we need */
    for(i=0; i<altsetting->bNumEndpoints; i++) 
    {
        /* is it an BULK endpoint? */
        if((altsetting->endpoint[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) 
        {
            /* BULK in or out? */
            if(altsetting->endpoint[i].bEndpointAddress & USB_DIR_IN)
            {
                if(ss->ep_in == 0)
                    ss->ep_in = altsetting->endpoint[i].bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            }
            else
            {
                if(ss->ep_out== 0)
                    ss->ep_out = altsetting->endpoint[i].bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            }
        }

        /* is it an interrupt endpoint? */
        if((altsetting->endpoint[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) 
            ss->ep_int = &altsetting->endpoint[i];
    }
    printf("Endpoints: Bulk-in: %d, Bulk-out: %d, Interrupt: 0x%p (Period %d)\n",
          ss->ep_in, ss->ep_out, ss->ep_int, (ss->ep_int ? ss->ep_int->bInterval : 0));

    ss->subclass = id->bInterfaceSubClass;
    ss->protocol = id->bInterfaceProtocol;
    ss->ifnum = intf;
    ss->protocol_name = "0x00";

     /* 
     * Set the handler pointers based on the protocol
     */
    switch(ss->protocol)
    {
    case UT_PR_CB:
        ss->transport_name = "Control/Bulk";
        ss->max_lun = 0;
        break;
    case UT_PR_CBI:
        ss->transport_name = "Control/Bulk/Interrupt";
        ss->transport = usb_mod_CBI_transport;
        ss->transport_reset = usb_mod_CB_reset;
        ss->max_lun = 0;
        break;
    case UT_PR_BULK:
        ss->transport_name = "Bulk";
        break;
    default:
        goto error;
    }
    printf("\n Transport: %s \n", ss->transport_name);
    printf(" Protocol : %s \n", ss->protocol_name);
    printf(" Max Lun  : %d \n", ss->max_lun);

    ss->semaphore = (void*)SYS_CreateSemaphore(1, NULL);
    if(!ss->semaphore)
    {
        LOG_ERROR " usb_mod_probe() has error code %X \n" LOG_END
        goto error;
    }

    goto end;

error:
    memset((void*)ss, 0, sizeof(struct usbtuner_data));
    ss = 0;

end: 
    return (void*)ss;

}

static void usb_mod_disconnect(struct usb_device* dev, void* ptr)
{
    struct usbtuner_data* ss = 0;

    if(ut_list[0].pusb_dev == dev)
        ss = &ut_list[0];
    else if(ut_list[1].pusb_dev == dev)
        ss = &ut_list[1];
    else
        LOG_ERROR " usbmod_disconnect() no available context?? \n" LOG_END

    /** Do something.... */
    if(ss->semaphore)
    {
        /** wait ap finish the command */
        SYS_WaitSemaphore(ss->semaphore);
        SYS_ReleaseSemaphore(ss->semaphore);

        SYS_DeleteSemaphore(ss->semaphore);;
    }

    memset((void*)ss, 0, sizeof(struct usbtuner_data));
}

struct usb_driver usb_mod_driver = 
{
    "usb_mod_driver",   /** driver name */
    0,                  /** flag */
    NULL,               /** mutex : serialize */
    usb_mod_probe,      /** probe function */
    NULL,               /** open function */
    usb_mod_disconnect, /** disconnect function */
    NULL,               /** ioctl */
    mod_usb_ids,        /** id table */
    NULL,NULL           /** driver_list */
};

int iteUsbModDriverRegister(void)
{
    int rst = 0;
    
    INIT_LIST_HEAD(&usb_mod_driver.driver_list);
    usb_mod_driver.serialize = (void*)SYS_CreateSemaphore(1, NULL);

    rst = usb_register(&usb_mod_driver);
    return rst;
}


