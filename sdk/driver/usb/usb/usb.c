/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * USB helper routines for real drivers.
 *
 * @author Irene Lin
 */
/* 
 * NOTE! This is not actually a driver at all, rather this is
 * just a collection of helper routines that implement the
 * generic USB things that the real drivers can use..
 *
 * Think of this as a "USB library" rather than anything else.
 * It should be considered a slave, with no callbacks. Callbacks
 * are evil.
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define GET_TIMEOUT     1200
#define SET_TIMEOUT     1500

//=============================================================================
//                   Gloabl Data
//=============================================================================
/*
 * We have a per-interface "registered driver" list.
 */
LIST_HEAD(usb_driver_list);

//=============================================================================
//                   Private function definition
//=============================================================================
MMP_INT find_next_zero_bit(MMP_UINT32* map, MMP_UINT8 startBit, MMP_UINT8 endBit)
{
    MMP_INT i = 0;
    for(i=startBit; i<=endBit; i++)
    {
        if((((*map)>>i) & 1) == 0)
        {
            (*map) |= (1<<i);
            return i;
        }
    }
    ithPrintf(" find_next_zero_bit(0x%08X, %d, %d) not find! \n", (*map), startBit, endBit);
    return -1;
}

void clear_bit(MMP_UINT32* map, MMP_UINT8 i)
{
    (*map) &= ~(1<<i);
}

//=============================================================================
/**
 * Periodic scheduler - bandwidth related
 */
//=============================================================================
/** claim bandwidth */
void usb_claim_bandwidth(struct usb_device *dev, struct urb *urb, MMP_INT bustime, MMP_INT isoc)
{
	dev->bus->bandwidth_allocated += bustime;
	if(isoc)
		dev->bus->bandwidth_isoc_reqs++;
	else
		dev->bus->bandwidth_int_reqs++;
	urb->bandwidth = bustime;

	LOG_INFO " bandwidth alloc increased by %d to %d for %d requesters \n",
		bustime,
		dev->bus->bandwidth_allocated,
		dev->bus->bandwidth_int_reqs + dev->bus->bandwidth_isoc_reqs LOG_END
}

/**
 * usb_release_bandwidth():
 *
 * called to release a pipe's bandwidth (in microseconds)
 */
void usb_release_bandwidth(struct usb_device* dev, struct urb* urb, MMP_INT isoc)
{
	dev->bus->bandwidth_allocated -= urb->bandwidth;
	if(isoc)
		dev->bus->bandwidth_isoc_reqs--;
	else
		dev->bus->bandwidth_int_reqs--;

	LOG_INFO "bandwidth alloc reduced by %d to %d for %d requesters \n",
		urb->bandwidth,
		dev->bus->bandwidth_allocated,
		dev->bus->bandwidth_int_reqs + dev->bus->bandwidth_isoc_reqs LOG_END

	urb->bandwidth = 0;
}

//=============================================================================
/**
 *	usb_register - register a USB driver
 *	@new_driver: USB operations for the driver
 *
 *	Registers a USB driver with the USB core.  The list of unattached
 *	interfaces will be rescanned whenever a new driver is added, allowing
 *	the new driver to attach to any recognized devices.
 *	Returns a negative error code on failure and 0 on success.
 */
//=============================================================================
MMP_INT usb_register(struct usb_driver *new_driver)
{
    LOG_INFO " registered new driver: %s \n", new_driver->name LOG_END

    if(!new_driver->serialize)
    {
        LOG_ERROR " driver %s NOT initial mutex! \n", new_driver->name LOG_END
        return -1;
    }

    /* Add it to the list of known drivers */
    list_add_tail(&new_driver->driver_list, &usb_driver_list);

    return 0;
}

//=============================================================================
//                   Bus Function Definition - for HC driver use
//=============================================================================
/**
 *	usb_alloc_bus - creates a new USB host controller structure
 *	@op: pointer to a struct usb_operations that this bus structure should use
 *
 *	Creates a USB host controller bus structure with the specified 
 *	usb_operations and initializes all the necessary internal objects.
 *	(For use only by USB Host Controller Drivers.)
 *
 *	If no memory is available, NULL is returned.
 *
 *	The caller should call usb_free_bus() when it is finished with the structure.
 */
struct usb_bus *usb_alloc_bus(struct usb_operations *op)
{
    struct usb_bus* bus;

    bus = (struct usb_bus*)SYS_Malloc(sizeof(struct usb_bus));
    if(!bus)
        return MMP_NULL;

    memset(bus, 0, sizeof(struct usb_bus));

    bus->op = op;
    bus->root_device = MMP_NULL;
    bus->hcpriv = MMP_NULL;
    bus->busnum = -1;
    bus->bandwidth_allocated = 0;
    bus->bandwidth_int_reqs  = 0;
    bus->bandwidth_isoc_reqs = 0;

    return bus;
}

/**
 *	usb_free_bus - frees the memory used by a bus structure
 *	@bus: pointer to the bus to free
 *
 *	(For use only by USB Host Controller Drivers.)
 *  It may never be free.
 */
void usb_free_bus(struct usb_bus *bus)
{
    if(!bus)
        return;

    SYS_Free((void*)bus);
}


//=============================================================================
//                   USB Device Function Definition - for HC driver use
//=============================================================================
/*
 * Only HC's should call usb_alloc_dev and usb_free_dev directly
 */
struct usb_device* usb_alloc_dev(struct usb_device* parent, struct usb_bus* bus)
{
    struct usb_device* dev = MMP_NULL;

    dev = (struct usb_device*)SYS_Malloc(sizeof(struct usb_device));
    if(!dev)
        goto end;

    memset(dev, 0, sizeof(struct usb_device));

    dev->bus = bus;
    dev->parent = parent;
    dev->bus->op->allocate(dev);

end:
    return dev;
}

void usb_destroy_configuration(struct usb_device *dev);

void usb_free_dev(struct usb_device* dev)
{
    if(dev->serialize)
    {
        SYS_DeleteSemaphore(dev->serialize);
        dev->serialize = MMP_NULL;
    }

    dev->bus->op->deallocate(dev);
    usb_destroy_configuration(dev);
    SYS_Free((void*)dev);
}

//=============================================================================
//                   Enumerate related functions
//=============================================================================
#include "usb_enumerate.c"
#include "usb_driver.c"


//=============================================================================
//                   USB Transaction related function Definition
//=============================================================================
/**
 *	usb_alloc_urb - creates a new urb for a USB driver to use
 *	@iso_packets: number of iso packets for this urb
 *
 *	Creates an urb for the USB driver to use and returns a pointer to it.
 *	If no memory is available, NULL is returned.
 *
 *	If the driver want to use this urb for interrupt, control, or bulk
 *	endpoints, pass '0' as the number of iso packets.
 *
 *	The driver should call usb_free_urb() when it is finished with the urb.
 */
urb_t* usb_alloc_urb(MMP_INT iso_packets)
{
    MMP_INT result = 0;
    urb_t* urb = MMP_NULL;

    if(iso_packets)
    {
        urb = (urb_t*)SYS_Malloc(sizeof(urb_t) + iso_packets * sizeof(iso_packet_descriptor_t));
        if(!urb) 
        {
            LOG_ERROR " usb_alloc_urb() alloc system memory fail! \n" LOG_END
            goto end;
        }
        memset(urb, 0, sizeof(*urb));
        urb->type = URB_BUF_ISO;
        /** ISO TODO: urb->lock */
    }
    else
    {
        result = urbBufGet(&urb);
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR " usb_alloc_urb() has error code 0x%08X \n", result LOG_END
    return urb;
}

/**
 *	usb_free_urb - frees the memory used by a urb
 *	@urb: pointer to the urb to free
 *
 *	If an urb is created with a call to usb_create_urb() it should be
 *	cleaned up with a call to usb_free_urb() when the driver is finished
 *	with it.
 */
void usb_free_urb(urb_t* urb)
{
    MMP_INT result = 0;
    if(urb)
    {
        if(urb->type == URB_BUF_ISO)
        {
            SYS_Free((void*)urb);
        }
        else
            result = urbBufFree(urb);
    }

    if(result)
        LOG_ERROR " usb_free_urb() has error code 0x%08X \n", result LOG_END
}

MMP_INT usb_submit_urb(urb_t *urb)
{
    MMP_INT result = 0;

    if(urb && urb->dev && urb->dev->bus && urb->dev->bus->op)
        result = urb->dev->bus->op->submit_urb(urb);
    else
        result = -ENODEV;

    if(result)
        LOG_ERROR " usb_submit_urb() return error code %d, urb 0x%08X \n", result, urb LOG_END
    return result;
}

MMP_INT usb_unlink_urb(urb_t *urb)
{
    MMP_INT result = 0;

    if(urb && urb->dev && urb->dev->bus && urb->dev->bus->op)
        result = urb->dev->bus->op->unlink_urb(urb);
    else
        result = -ENODEV;

    if(result)
    {
        //LOG_ERROR " usb_unlink_urb() return error code %d, urb 0x%08X \n", result, urb LOG_END
        ithPrintf("#");
    }
    return result;
}

MMP_INT usb_dev_exist(struct usb_device* usb_dev)
{
    MMP_INT result = 0;

    if(usb_dev && usb_dev->bus && usb_dev->bus->op)
        result = usb_dev->bus->op->dev_exist(usb_dev);
    else
        result = 0;

    return result;
}


/*-------------------------------------------------------------------*
 * completion handler for compatibility wrappers (sync control/bulk) *
 *-------------------------------------------------------------------*/

static void usb_api_blocking_completion(urb_t *urb)
{
    MMP_EVENT urbEvent = urb->context;
    urb->reserved = 1;
    SYS_SetEvent(urbEvent);
}

/** Starts urb and waits for completion or timeout */
static MMP_INT usb_start_wait_urb(urb_t *urb, MMP_INT timeout, MMP_INT* actual_length)
{ 
    MMP_INT result = 0;
    MMP_EVENT urbEvent = MMP_NULL;
    
    urbEvent = SYS_CreateEvent();
    urb->context = urbEvent;
    result = usb_submit_urb(urb);
    if(result)
        goto end;

    result = SYS_WaitEvent(urbEvent, timeout);
    if(result && !urb->reserved)  /** wait event timeout */
    {
        if(urb->status != -EINPROGRESS) /* No callback?!! */
        {
            LOG_ERROR " raced timeout: pipe 0x%08X, status 0x%08X, timeout valut %d \n", urb->pipe, (-urb->status), timeout LOG_END
            result = urb->status;
        } 
        else 
        {
            LOG_ERROR " usb_control/bulk_msg: timeout (%d) \n", timeout LOG_END
            LOG_ERROR " urb 0x%08X, cookies 0x%X, type %d, pipe 0x%08X, len: %d/%d \r\n", urb, urb->cookies, urb->type, urb->pipe, urb->actual_length, urb->transfer_buffer_length LOG_END
            usb_unlink_urb(urb);  // remove urb safely
            result = -ETIMEDOUT;
        }
    }
    else
    {
        result = urb->status;
    }

    if(actual_length)
        (*actual_length) = urb->actual_length;

end:
    if(urbEvent)
        SYS_DelEvent(urbEvent);

    if(urb)
    {
        #if defined(HANDLE_HW_HANG)
        // +wlHsu
        if(urb->status == -EPROTO)
        {
            sdk_msg_ex(SDK_MSG_TYPE_ERROR, "\nUSB H/W crazy (err: EPROTO)!!");
            result = -ERR_HW_HANG;
        }
        // -wlHsu
        #endif
        
        usb_free_urb(urb);
    }
        
    if(result)
        LOG_ERROR " usb_start_wait_urb() return error code %d \n", (result<0)?(-result):result LOG_END
    return result;
}

/*-------------------------------------------------------------------*/
/** Starts urb and busy waits for completion or timeout */
static MMP_INT usb_start_busy_wait_urb(urb_t *urb, MMP_INT timeout, MMP_INT* actual_length)
{ 
    MMP_INT result = 0;
    struct usb_hcd*	hcd;
    struct ehci_hcd* ehci;
    
    if(!urb->dev || !urb->dev->bus)
    {
        result = -ENODEV;
        goto end;
    }

    result = usb_submit_urb(urb);
    if(result)
        goto end;

    hcd = urb->dev->bus->hcpriv;
    ehci = hcd_to_ehci(hcd);
	while(!urb->reserved)
	{
	    if(ehci->tasklet)
	        ehci_tasklet(ehci);
#if !defined(USB_IRQ_ENABLE)
        ehci_irq(hcd);
#endif
	}
	
    if(result && !urb->reserved)  /** wait event timeout */
    {
        if(urb->status != -EINPROGRESS) /* No callback?!! */
        {
            LOG_ERROR " raced timeout: pipe 0x%08X, status 0x%08X, timeout valut %d \n", urb->pipe, (-urb->status), timeout LOG_END
            result = urb->status;
        } 
        else 
        {
            LOG_ERROR " usb_control/bulk_msg: timeout \n" LOG_END
            usb_unlink_urb(urb);  // remove urb safely
            result = -ETIMEDOUT;
        }
    }
    else
    {
        result = urb->status;
    }

    if(actual_length)
        (*actual_length) = urb->actual_length;

end:
    if(urb)
        usb_free_urb(urb);
        
    if(result)
        LOG_ERROR " usb_start_wait_urb() return error code %d \n", (result<0)?(-result):result LOG_END
    return result;
}

/*-------------------------------------------------------------------*/
MMP_INT usb_internal_control_msg(
    struct usb_device* usb_dev, 
    MMP_UINT32  pipe, 
    devrequest* cmd,  
    void*       data, 
    MMP_INT     len, 
    MMP_INT     timeout)
{
    urb_t *urb;
    MMP_INT result = 0;
    MMP_INT length = 0;

    urb = usb_alloc_urb(0);
    if(!urb)
    {
        result = ERROR_USB_ALLOC_SYS_MEM_FAIL;
        goto end;
    }
  
    FILL_CONTROL_URB(urb, usb_dev, pipe, (MMP_UINT8*)cmd, data, len,
           usb_api_blocking_completion, 0);

    result = usb_start_wait_urb(urb, timeout, &length);

end:
    if(result < 0)
    {
        LOG_ERROR " usb_internal_control_msg() return error code %d \n", result LOG_END
        return result;
    }
    else
    {
        return length;
    }
}

#if defined(USB_LOGO_TEST)
MMP_INT usb_internal_control_msg_step(
    struct usb_device* usb_dev, 
    MMP_UINT32  pipe, 
    devrequest* cmd,  
    void*       data, 
    MMP_INT     len, 
    MMP_INT     timeout,
    MMP_INT     step)
{
    urb_t *urb;
    MMP_INT result = 0;
    MMP_INT length = 0;

    urb = usb_alloc_urb(0);
    if(!urb)
    {
        result = ERROR_USB_ALLOC_SYS_MEM_FAIL;
        goto end;
    }
  
    FILL_CONTROL_URB(urb, usb_dev, pipe, (MMP_UINT8*)cmd, data, len,
           usb_api_blocking_completion, 0);

    urb->transfer_flags |= (USB_SINGLE_STEP_0 << step);

    result = usb_start_wait_urb(urb, timeout, &length);

end:
    if(result < 0)
    {
        LOG_ERROR " usb_internal_control_msg_step() return error code %d \n", result LOG_END
        return result;
    }
    else
    {
        return length;
    }
}
#endif

/**
 *	usb_control_msg - Builds a control urb, sends it off and waits for completion
 *	@dev: pointer to the usb device to send the message to
 *	@pipe: endpoint "pipe" to send the message to
 *	@request: USB message request value
 *	@requesttype: USB message request type value
 *	@value: USB message value
 *	@index: USB message index value
 *	@data: pointer to the data to send
 *	@size: length in bytes of the data to send
 *	@timeout: time to wait for the message to complete before timing out (if 0 the wait is forever)
 *
 *	This function sends a simple control message to a specified endpoint
 *	and waits for the message to complete, or timeout.
 *	
 *	If successful, it returns 0, othwise a non-zero error number.
 *
 *	Don't use this function from within an interrupt context, like a
 *	bottom half handler.  If you need a asyncronous message, or need to send
 *	a message from within interrupt context, use usb_submit_urb()
 */
MMP_INT usb_control_msg(
    struct usb_device* dev, 
    MMP_UINT32 pipe, 
    MMP_UINT8  request, 
    MMP_UINT8  requesttype,
    MMP_UINT16 value, 
    MMP_UINT16 index, 
    void*      data, 
    MMP_UINT16 size, 
    MMP_INT    timeout)
{
    MMP_INT result = 0;
    devrequest* dr = (devrequest*)SYS_Malloc(sizeof(devrequest));

    if(!dr)
    {
        result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
        goto end;
    }

    dr->requesttype = requesttype;
    dr->request = request;
    dr->value = cpu_to_le16(value);
    dr->index = cpu_to_le16(index);
    dr->length = cpu_to_le16(size);

    result = usb_internal_control_msg(dev, pipe, dr, data, size, timeout);

end:
    if(dr)
        SYS_Free((void*)dr);
    if(result < 0)
        LOG_ERROR " usb_control_msg() return error code 0x%08X \n", (-result) LOG_END
    return result;
}


/**
 *	usb_bulk_msg - Builds a bulk urb, sends it off and waits for completion
 *	@usb_dev: pointer to the usb device to send the message to
 *	@pipe: endpoint "pipe" to send the message to
 *	@data: pointer to the data to send
 *	@len: length in bytes of the data to send
 *	@actual_length: pointer to a location to put the actual length transferred in bytes
 *	@timeout: time to wait for the message to complete before timing out (if 0 the wait is forever)
 *
 *	This function sends a simple bulk message to a specified endpoint
 *	and waits for the message to complete, or timeout.
 *	
 *	If successful, it returns 0, othwise a negative error number.
 *	The number of actual bytes transferred will be plaed in the 
 *	actual_timeout paramater.
 *
 *	Don't use this function from within an interrupt context, like a
 *	bottom half handler.  If you need a asyncronous message, or need to
 *	send a message from within interrupt context, use usb_submit_urb()
 */
MMP_INT usb_bulk_msg(struct usb_device *usb_dev, MMP_UINT32 pipe, 
            void *data, MMP_INT len, MMP_INT *actual_length, MMP_INT timeout)
{
    MMP_INT result = 0;
    urb_t *urb;

    if(len < 0)
    {
        result = -EINVAL;
        goto end;
    }

    urb = usb_alloc_urb(0);
    if(!urb)
    {
        result = -ENOMEM;
        goto end;
    }

    FILL_BULK_URB(urb, usb_dev, pipe, data, len, usb_api_blocking_completion, 0);

    result = usb_start_wait_urb(urb, timeout, actual_length);

end:
    if(result < 0)
        LOG_ERROR " usb_bulk_msg() return error code 0x%08X \n", (-result) LOG_END
    return result;
}

MMP_INT usb_bulk_busy_msg(struct usb_device *usb_dev, MMP_UINT32 pipe, 
            void *data, MMP_INT len, MMP_INT *actual_length, MMP_INT timeout)
{
    MMP_INT result = 0;
    urb_t *urb;

    if(len < 0)
    {
        result = -EINVAL;
        goto end;
    }

    urb = usb_alloc_urb(0);
    if(!urb)
    {
        result = -ENOMEM;
        goto end;
    }

    FILL_BULK_URB(urb, usb_dev, pipe, data, len, usb_api_blocking_completion, 0);
    result = usb_start_busy_wait_urb(urb, timeout, actual_length);

end:
    if(result < 0)
        LOG_ERROR " usb_bulk_msg() return error code 0x%08X \n", (-result) LOG_END
    return result;
}


//=============================================================================
//                   USB Connect related Function Definition - for HC driver use
//=============================================================================

/*
 * Something got disconnected. Get rid of it, and all of its children.
 */
void usb_disconnect(struct usb_device **pdev)
{
    struct usb_device * dev = *pdev;
    MMP_INT i;

    if(!dev)
        return;

    *pdev = MMP_NULL;

    LOG_INFO "USB disconnect on device %d \n", dev->devnum LOG_END

    if(dev->actconfig) 
    {
        for(i=0; i<dev->actconfig->bNumInterfaces; i++) 
        {
            struct usb_interface *usb_interface = &dev->actconfig->usb_interface[i];
            struct usb_driver *driver = usb_interface->driver;
            if(driver) 
            {
                SYS_WaitSemaphore(driver->serialize);
                driver->disconnect(dev, usb_interface->private_data);
                SYS_ReleaseSemaphore(driver->serialize);
                if(usb_interface->driver)
                    usb_driver_release_interface(driver, usb_interface);
            }
        }
    }

    /* Free up all the children.. */
    for(i=0; i<USB_MAXCHILDREN; i++) 
    {
        struct usb_device **child = dev->children + i;
        if(*child)
            usb_disconnect(child);
    }

    /* Free the device number and remove the /proc/bus/usb entry */
    if(dev->devnum > 0) 
        clear_bit(&dev->bus->devmap, dev->devnum);

    /* Free up the device itself */
    usb_free_dev(dev);
}

/*
 * Connect a new USB device. This basically just initializes
 * the USB device information and sets up the topology - it's
 * up to the low-level driver to reset the port and actually
 * do the setup (the upper levels don't know how to do that).
 *
 * Actually it should call by HUB thread.
 */
void usb_connect(struct usb_device* dev)
{
    dev->descriptor.bMaxPacketSize0 = 8;  /* Start off at 8 bytes  */
#if !defined(MS_ENUMERATE)
    dev->devnum = find_next_zero_bit(&dev->bus->devmap, 1, 31);
#else
    dev->devnum = 0;
#endif

    dev->serialize = SYS_CreateSemaphore(1, MMP_NULL);
    if(!dev->serialize)
        LOG_ERROR " usb_connect() create mutex fail! \n" LOG_END
}

/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
MMP_INT usb_new_device(struct usb_device *dev)
{
    MMP_INT result = 0;

    /* We read the first 8 bytes from the device descriptor to get to */
    /* the bMaxPacketSize0 field. Then we set the maximum packet size */
    /* for the control pipe, and retrieve the rest */
#if defined(WR_MAX_PACKET_LENGTH)
    if(dev->speed == USB_SPEED_LOW)
    {
        dev->epmaxpacketin [0] = 8;
        dev->epmaxpacketout[0] = 8;
    }
    else
    {
        dev->epmaxpacketin [0] = 32;
        dev->epmaxpacketout[0] = 32;
    }
#else
    dev->epmaxpacketin [0] = 8;
    dev->epmaxpacketout[0] = 8;
#endif

#if !defined(MS_ENUMERATE)
#if 1 
/**
 * Workaround Transcend JF V33/8GB 
 *  Vendor information: JetFlashTranscend 8GB   8.07
 *  Product identification: Transcend 8GB   8.07
 *  Product revision level: 8.07
 */
re_try:
    result = usb_set_address(dev);
    if(result)
    {
        {
            struct usb_hcd*	hcd = dev->bus->hcpriv;
            result = ehci_port_reset(hcd_to_ehci(hcd));
            if(result)
            {
                LOG_ERROR " port reset fail....\n" LOG_END
                //while(1);
                result = ERROR_USB_SERIOUS_ERROR;
                goto end;
            }
            goto re_try;
        }
    }
#else
    result = usb_set_address(dev);
    if(result)
        goto end;
#endif

    MMP_Sleep(10);	/* Let the SET_ADDRESS settle */
#endif

    result = usb_get_descriptor(dev, USB_DT_DEVICE, 0, &dev->descriptor, 8);
    if(result < 8) 
    {
        if(result < 0)
            LOG_ERROR "USB device not responding, giving up (error=%d) \n", result LOG_END
        else
            LOG_ERROR "USB device descriptor short read (expected %i, got %i) \n", 8, result LOG_END
        goto end;
    }
    dev->epmaxpacketin [0] = dev->descriptor.bMaxPacketSize0;
    dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

#if defined(MS_ENUMERATE)
    {
        struct usb_hcd*	hcd = dev->bus->hcpriv;
        result = ehci_port_reset(hcd_to_ehci(hcd));
        if(result)
        {
            LOG_ERROR " port reset fail....\n" LOG_END
            while(1);
        }
    }
#endif

    result = usb_get_device_descriptor(dev);
    if(result < (MMP_INT)sizeof(dev->descriptor)) 
    {
        if(result < 0)
            LOG_ERROR "unable to get device descriptor (error=%d) \n", result LOG_END
        else
            LOG_ERROR "USB device descriptor short read (expected %Zi, got %i) \n", sizeof(dev->descriptor), result LOG_END
        goto end;
    }

#if defined(MS_ENUMERATE)
    dev->devnum = find_next_zero_bit(&dev->bus->devmap, 1, 31);
    if(!dev->devnum)
    {
        result = ERROR_USB_NOT_FIND_DEVCE_ID;
        goto end;
    }
    result = usb_set_address(dev);
    if(result)
        goto end;

    MMP_Sleep(10);	/* Let the SET_ADDRESS settle */
#endif
    result = usb_get_configuration(dev);
    if(result < 0) 
    {
        LOG_ERROR "unable to get device %d configuration (error=%d) \n", dev->devnum, result LOG_END
        goto end;
    }

    /* we set the default configuration here */
    result = usb_set_configuration(dev, dev->config[0].bConfigurationValue);
    if(result) 
    {
        LOG_ERROR "failed to set device %d default configuration (result=%d) \n", dev->devnum, result LOG_END
        goto end;
    }

    LOG_INFO "new device strings: Mfr=%d, Product=%d, SerialNumber=%d \n",
        dev->descriptor.iManufacturer, dev->descriptor.iProduct, dev->descriptor.iSerialNumber LOG_END

    if(dev->descriptor.iManufacturer)
        usb_show_string(dev, "Manufacturer", dev->descriptor.iManufacturer);
    if(dev->descriptor.iProduct)
        usb_show_string(dev, "Product", dev->descriptor.iProduct);
    if(dev->descriptor.iSerialNumber)
        usb_show_string(dev, "SerialNumber", dev->descriptor.iSerialNumber);

    /* find drivers willing to handle this device */
    result = usb_find_drivers(dev);

end:
    if(result)
    {
        LOG_ERROR " usb_new_device() return error code 0x%08X \n", ((result<0)?(-result):result) LOG_END
        if(dev->serialize)
        {
            SYS_DeleteSemaphore(dev->serialize);
            dev->serialize = MMP_NULL;
        }
        clear_bit(&dev->bus->devmap, dev->devnum);
        dev->devnum = -1;
    }
    return result;
}



//=============================================================================
//                   Some Public Functions
//=============================================================================

/** HID Class: Class-Specific Requests */
/** 
 * Get_Report Request
 * The Get_Report request allows the host to receive a report via the control pipe.
 */
MMP_INT usb_get_report(struct usb_device *dev, MMP_INT ifnum, MMP_UINT8 type, MMP_UINT8 id, void* buf, MMP_INT size)
{
    return usb_control_msg(dev, 
                           usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_REPORT, 
                           USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (type << 8) + id, 
                           ifnum, 
                           buf, 
                           size, 
                           GET_TIMEOUT);
}

/** 
 * Set_Protocol Request
 * The Set_Protocol switches between the boot protocol and the report protocol (or vice versa).
 */
MMP_INT usb_set_protocol(struct usb_device* dev, MMP_INT ifnum, MMP_INT protocol)
{
    return usb_control_msg(dev, 
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_PROTOCOL, 
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           protocol, 
                           ifnum, 
                           MMP_NULL, 
                           0, 
                           SET_TIMEOUT);
}

/** 
 * Set_Idle Request
 * The Set_Idle request silences a particular report on the Interrupt In pipe until a new
 * event occurs or the specified amount of time passes.
 */
MMP_INT usb_set_idle(struct usb_device *dev, MMP_INT ifnum, MMP_INT duration, MMP_INT report_id)
{
    return usb_control_msg(dev, 
                           usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_IDLE, 
                           USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (duration << 8) | report_id, 
                           ifnum, 
                           MMP_NULL, 
                           0, 
                           SET_TIMEOUT);
}

/*
 * endp: endpoint number in bits 0-3;
 *	     direction flag in bit 7 (1 = IN, 0 = OUT)
 */
MMP_INT usb_clear_halt(struct usb_device *dev, MMP_INT pipe)
{
    MMP_INT result = 0;
    MMP_UINT8 status[2] = {0};
    MMP_UINT16 endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

    result = usb_control_msg(dev, 
                             usb_sndctrlpipe(dev, 0),
                             USB_REQ_CLEAR_FEATURE, 
                             USB_RECIP_ENDPOINT, 
                             0, 
                             endp, 
                             MMP_NULL, 
                             0, 
                             SET_TIMEOUT);
    /* don't clear if failed */
    if (result < 0)
        return result;

    result = usb_control_msg(dev, 
                             usb_rcvctrlpipe(dev, 0),
                             USB_REQ_GET_STATUS, 
                             USB_DIR_IN | USB_RECIP_ENDPOINT, 
                             0, 
                             endp,
                             status, 
                             sizeof(status), 
                             SET_TIMEOUT);
    if (result < 0)
        return result;

    if (status[0] & 1)
        return -EPIPE;		/* still halted */

    usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

    /* toggle is reset on clear */
    usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);

    return 0;
}

