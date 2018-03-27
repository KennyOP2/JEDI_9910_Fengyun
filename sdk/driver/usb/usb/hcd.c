/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * USB Host Controller Driver framework.
 *
 * @author Irene Lin
 */
/*
 * USB Host Controller Driver framework
 *
 * Plugs into usbcore (usb_bus) and lets HCDs share code, minimizing
 * HCD-specific behaviors/bugs.
 *
 * This does error checks, tracks devices and urbs, and delegates to a
 * "hc_driver" only for code (and data) that really needs to know about
 * hardware differences.  That includes root hub registers, i/o queues,
 * and so on ... but as little else as possible.
 *
 * Shared code includes most of the "root hub" code (these are emulated,
 * though each HC's hardware works differently) and PCI glue, plus request
 * tracking overhead.  The HCD code should only block on spinlocks or on
 * hardware handshaking; blocking on software events (such as other kernel
 * threads releasing resources, or completing actions) is all generic.
 *
 * Happens the USB 2.0 spec says this would be invisible inside the "USBD",
 * and includes mostly a "HCDI" (HCD Interface) along with some APIs used
 * only by the hub driver ... and that neither should be seen or used by
 * usb client device drivers.
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"

//=============================================================================
//                              Global Data Definition
//=============================================================================
struct usb_hcd* hcd0 = MMP_NULL;
struct usb_hcd* hcd1 = MMP_NULL;

//=============================================================================
//                              Public Function Definition
//=============================================================================
static struct usb_operations hcd_operations;

MMP_INT usb_hcd_probe(struct hc_driver* driver, MMP_UINT32 index)
{
    MMP_INT result = 0;
    struct usb_bus* bus;
    struct usb_hcd* hcd;

    /** alloc ehci_hcd */
    hcd = driver->hcd_alloc();
    if(!hcd)
    {
        result = ERROR_USB_ALLOC_HCD_FAIL;
        goto end;
    }
    /** init hcd */
    hcd->driver = driver;
    hcd->index = index;
	_spin_lock_init(&hcd->hcd_data_lock);
    if(!(index & 0x1)) 
    {   /** usb 0 */
        hcd->iobase = USB0_BASE;
    }
    else /** usb 1 */
    {
        hcd->iobase = USB1_BASE;
    }
    LOG_INFO " driver index = 0x%08X, hcd->iobase = 0x%08X \n", hcd->index, hcd->iobase LOG_END

    /** alloc bus */
    bus = usb_alloc_bus(&hcd_operations);
    if(!bus)
    {
        result = ERROR_USB_ALLOC_BUS_FAIL;
        goto end;
    }
    hcd->bus = bus;
    bus->hcpriv = (void*)hcd;
    if(!(hcd->index & 0x1)) 
    {   /** usb 0 */
        bus->busnum = 0;
        hcd0 = hcd;
    }
    else
    {
        bus->busnum = 1;
        hcd1 = hcd;
    }
    
    /** ehci start */
    result = driver->start(hcd);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR " usb_hcd_probe() return error code 0x%08X, hcd index = 0x%08X \n", result, hcd->index LOG_END
    return result;
}


MMP_INT hcd_alloc_dev(struct usb_device *udev)
{
    MMP_INT result = 0;
    struct hcd_dev*	dev = MMP_NULL;
    struct usb_hcd*	hcd = MMP_NULL;
    LOG_ENTER " hcd_alloc_dev() udev = 0x%08X \n", udev LOG_END

    hcd = udev->bus->hcpriv;

    dev = (struct hcd_dev*)SYS_Malloc(sizeof(struct hcd_dev));
    if(!dev)
    {
        result = ERROR_USB_ALLOC_HCD_DEV_FAIL;
        goto end;
    }
    memset(dev, 0, sizeof(struct hcd_dev));
    INIT_LIST_HEAD(&dev->urb_list);

    _spin_lock_irqsave(&hcd->hcd_data_lock);
    udev->hcpriv = dev;
    _spin_unlock_irqrestore(&hcd->hcd_data_lock);

end:
    if(result)
        LOG_ERROR " hcd_alloc_dev() return error code 0x%08X \n", result LOG_END
    LOG_LEAVE " hcd_alloc_dev() udev = 0x%08X \n", udev LOG_END
    return result;
}


MMP_INT hcd_free_dev(struct usb_device *udev)
{
    MMP_INT result = 0;
    struct hcd_dev*	dev;
    struct usb_hcd*	hcd;
    LOG_ENTER " hcd_free_dev() udev = 0x%08X \n", udev LOG_END

    if(!udev || !udev->hcpriv)
    {
        LOG_ERROR " hcd_free_dev() udev is NULL! \n" LOG_END
    }

    if(!udev->bus || !udev->bus->hcpriv)
    {
        LOG_ERROR " hcd_free_dev() bus is NULL! \n" LOG_END
    }

    dev = udev->hcpriv;
    hcd = udev->bus->hcpriv;

    /* device driver problem with refcounts? */
    if(!list_empty(&dev->urb_list)) 
    {
        LOG_ERROR " Free busy device, device number %d (device driver bug!) \n", udev->devnum LOG_END
        result = ERROR_USB_FREE_NON_EMPTY_URB_DEVICE;
        goto end;
    }

    hcd->driver->free_config(hcd, udev);

    _spin_lock_irqsave(&hcd->hcd_data_lock);
    udev->hcpriv = MMP_NULL;
    _spin_unlock_irqrestore(&hcd->hcd_data_lock);
 
    SYS_Free((void*)dev);

end:
    if(result)
        LOG_ERROR " hcd_free_dev() return error code 0x%08X \n", result LOG_END
    LOG_LEAVE " hcd_free_dev() udev = 0x%08X \n", udev LOG_END
    return result;
}


MMP_INT hcd_get_frame_number(struct usb_device *udev)
{
    MMP_INT result = 0;
    LOG_ENTER " hcd_get_frame_number() udev = 0x%08X \n", udev LOG_END

    if(result)
        LOG_ERROR " hcd_get_frame_number() return error code 0x%08X \n", result LOG_END
    LOG_LEAVE " hcd_get_frame_number() udev = 0x%08X \n", udev LOG_END
    return result;
}

/*-------------------------------------------------------------------------*/
extern void usb_release_bandwidth(struct usb_device* dev, struct urb* urb, MMP_INT isoc);

static void urb_unlink(struct urb *urb)
{
    struct usb_device*	dev;
    struct usb_hcd* hcd;
#if defined(MS_ENUMERATE)
    if(!urb->dev || !urb->dev->bus || urb->dev->devnum < 0)
#else
    if(!urb->dev || !urb->dev->bus || urb->dev->devnum <= 0)
#endif
    {
        LOG_ERROR " urb_unlink() error code ENODEV \n" LOG_END
        LOG_ERROR " urb 0x%08X, cookies 0x%X, type %d, pipe 0x%08X, len: %d/%d \r\n", urb, urb->cookies, urb->type, urb->pipe, urb->actual_length, urb->transfer_buffer_length LOG_END
        while(1);
    }
    hcd = urb->dev->bus->hcpriv;

    /* Release any periodic transfer bandwidth */
    if(urb->bandwidth)
        usb_release_bandwidth(urb->dev, urb, usb_pipeisoc(urb->pipe));

    /* clear all state linking urb to this dev (and hcd) */
    _spin_lock_irqsave(&hcd->hcd_data_lock);
    list_del_init(&urb->urb_list);
    dev = urb->dev;
    urb->dev = MMP_NULL;
    _spin_unlock_irqrestore(&hcd->hcd_data_lock);
}


/* may be called in any context with a valid urb->dev usecount */
/* caller surrenders "ownership" of urb */
MMP_INT hcd_submit_urb(struct urb* urb)
{
    MMP_INT result = 0;
    struct usb_hcd*	hcd;
    struct hcd_dev*	dev;
    MMP_UINT32  pipe, temp, max;
    LOG_ENTER " hcd_submit_urb() urb = 0x%08X \n", urb LOG_END

    if(!urb || urb->hcpriv || !urb->complete)
    {
        LOG_ERROR " 1\n" LOG_END
        result = -EINVAL;
        goto end;
    }
    urb->status = -EINPROGRESS;
    urb->actual_length = 0;
    urb->bandwidth = 0;
    INIT_LIST_HEAD(&urb->urb_list);

#if defined(MS_ENUMERATE)
    if(!urb->dev || !urb->dev->bus || urb->dev->devnum < 0)
#else
    if(!urb->dev || !urb->dev->bus || urb->dev->devnum <= 0)
#endif
    {
        LOG_ERROR " 2\n" LOG_END
        result = -ENODEV;
        goto end;
    }
    hcd = urb->dev->bus->hcpriv;
    dev = urb->dev->hcpriv;
    if(!hcd || !dev)
    {
        LOG_ERROR " 3\n" LOG_END
        result = -ENODEV;
        goto end;
    }

    /* can't submit new urbs when quiescing, halted, ... */
    if(/*hcd->state == USB_STATE_QUIESCING ||*/ !HCD_IS_RUNNING(hcd->state))
    {
        LOG_ERROR " 4\n" LOG_END
        result = -ESHUTDOWN;
        goto end;
    }
    pipe = urb->pipe;
    temp = usb_pipetype(urb->pipe);
    if(usb_endpoint_halted(urb->dev, usb_pipeendpoint(pipe), usb_pipeout(pipe)))
    {
        LOG_ERROR " 5\n" LOG_END
        result = -EPIPE;
        goto end;
    }

    /* FIXME there should be a sharable lock protecting us against
     * config/altsetting changes and disconnects, kicking in here.
     */

    /* Sanity check, so HCDs can rely on clean data */
    max = usb_maxpacket(urb->dev, pipe, usb_pipeout(pipe));
    if(max <= 0) 
    {
        LOG_ERROR " bogus endpoint (bad maxpacket) 6" LOG_END
        result = -EINVAL;
        goto end;
    }

    /* "high bandwidth" mode, 1-3 packets/uframe? */
    if(urb->dev->speed == USB_SPEED_HIGH) 
    {
        MMP_INT	mult;
        switch (temp) 
        {
        case PIPE_ISOCHRONOUS:
        case PIPE_INTERRUPT:
            mult = 1 + ((max >> 11) & 0x03);
            max &= 0x03ff;
            max *= mult;
        }
    }

    /* periodic transfers limit size per frame/uframe */
    switch (temp) 
    {
    case PIPE_ISOCHRONOUS: 
#if 0
        // ISO TODO
        {
            MMP_INT	n, len;

            if(urb->number_of_packets <= 0)	
            {
                LOG_ERROR " 7\n" LOG_END
                result = -EINVAL;
                goto end;
            }
            for(n=0; n<urb->number_of_packets; n++) 
            {
                len = urb->iso_frame_desc[n].length;
                if(len < 0 || len > max) 
                {
                    LOG_ERROR " 8\n" LOG_END
                    result = -EINVAL;
                    goto end;
                }
            }
        }
#endif
        break;
    case PIPE_INTERRUPT:
        if(urb->transfer_buffer_length > (MMP_INT)max)
        {
            LOG_ERROR " 9\n" LOG_END
            result = -EINVAL;
            goto end;
        }
    }
    /** check buffer length */
    if(urb->transfer_buffer_length < 0)
    {
        LOG_ERROR " 10\n" LOG_END
        result = -EINVAL;
        goto end;
    }

    if(urb->next) 
    {
        LOG_ERROR " use explicit queuing not urb->next 10\n" LOG_END
        result = -EINVAL;
        goto end;
    }

    /*
     * Force periodic transfer intervals to be legal values that are
     * a power of two (so HCDs don't need to).
     *
     * FIXME want bus->{intr,iso}_sched_horizon values here.  Each HC
     * supports different values... this uses EHCI/UHCI defaults (and
     * EHCI can use smaller non-default values).
     */
    switch(temp) 
    {
    case PIPE_ISOCHRONOUS:
    case PIPE_INTERRUPT:
        /* too small? */
        if(urb->interval <= 0)
        {
            LOG_ERROR " 11\n" LOG_END
            result = -EINVAL;
            goto end;
        }
        /* too big? */
        switch(urb->dev->speed) 
        {
        case USB_SPEED_HIGH:	/* units are microframes */
            urb->interval = urb->interval * 8;  // enlarge to us??

            // NOTE usb handles 2^15
            if(urb->interval > (1024 * 8))
                urb->interval = 1024 * 8;
            temp = 1024 * 8;
            break;
        case USB_SPEED_FULL:	/* units are frames/msec */
        case USB_SPEED_LOW:
            urb->interval = urb->interval * 8;  // enlarge to us??
            if(temp == PIPE_INTERRUPT) 
            {
                if(urb->interval > 255)
                {
                    LOG_ERROR " 12\n" LOG_END
                    result = -EINVAL;
                    goto end;
                }
                // NOTE ohci only handles up to 32
                temp = 128;
            } 
            else 
            {
                if(urb->interval > 1024)
                    urb->interval = 1024;
                // NOTE usb and ohci handle up to 2^15
                temp = 1024;
            }
            break;
        default:
            {
                LOG_ERROR " 13\n" LOG_END
                result = -EINVAL;
                goto end;
            }
        }
        /* power of two? */
        while((MMP_INT)temp > urb->interval)
            temp >>= 1;
        urb->interval = temp;
    }


    /*
     * FIXME:  make urb timeouts be generic, keeping the HCD cores
     * as simple as possible.
     */

    // NOTE:  a generic device/urb monitoring hook would go here.
    // hcd_monitor_hook(MONITOR_URB_SUBMIT, urb)
    // It would catch submission paths for all urbs.

    /*
     * Atomically queue the urb,  first to our records, then to the HCD.
     * Access to urb->status is controlled by urb->lock ... changes on
     * i/o completion (normal or fault) or unlinking.
     */
    _spin_lock_irqsave(&hcd->hcd_data_lock);
    if(HCD_IS_RUNNING(hcd->state) /*&& hcd->state != USB_STATE_QUIESCING*/) 
    {
        list_add(&urb->urb_list, &dev->urb_list);
    } 
    else 
    {
        INIT_LIST_HEAD(&urb->urb_list);
        LOG_ERROR " 14\n" LOG_END
        result = -ESHUTDOWN;
    }
    _spin_unlock_irqrestore(&hcd->hcd_data_lock);
    if(result)
        goto end;

    result = hcd->driver->urb_enqueue(hcd, urb);
    /* urb->dev got nulled if hcd called giveback for us
     * NOTE: ref to urb->dev is a race without (2.5) refcounting,
     * unless driver only returns status when it didn't giveback 
     */
    if(result && urb->dev)
        urb_unlink(urb);

end:
    if(result)
        LOG_ERROR " hcd_submit_urb() return error code 0x%08X \n", result LOG_END
    LOG_LEAVE " hcd_submit_urb() urb = 0x%08X \n", urb LOG_END
    return result;
}


/*-------------------------------------------------------------------------*/
extern MMP_INT find_next_zero_bit(MMP_UINT32* map, MMP_UINT8 startBit, MMP_UINT8 endBit);
extern void clear_bit(MMP_UINT32* map, MMP_UINT8 i);

struct completion_splice {		// modified urb context:
    /* did we complete? */
    MMP_EVENT	done;
    /* original urb data */
    void (*complete)(struct urb *);
    void *context;
};

static void unlink_complete(struct urb *urb)
{
    struct completion_splice	*splice;

    LOG_DEBUG " unlink_complete() urb 0x%08X \n", urb LOG_END

    splice = (struct completion_splice *)urb->context;

    /* issue original completion call */
    urb->complete = splice->complete;
    urb->context = splice->context;
    urb->complete(urb);

    /* then let the synchronous unlink call complete */
    SYS_SetEvent(splice->done);
}

/*
 * called in any context; note ASYNC_UNLINK restrictions
 *
 * caller guarantees urb won't be recycled till both unlink()
 * and the urb's completion function return
 */
MMP_INT hcd_unlink_urb(struct urb* urb)
{
    MMP_INT result = 0;
    struct hcd_dev *dev;
    struct usb_hcd *hcd = 0;
    struct completion_splice	splice;
    MMP_EVENT urbEvent = MMP_NULL;

    LOG_DEBUG " hcd_unlink_urb() urb = 0x%08X \n", urb LOG_END

    if(!urb)
        return -EINVAL;

    /*
     * we contend for urb->status with the hcd core,
     * which changes it while returning the urb.
     *
     * Caller guaranteed that the urb pointer hasn't been freed, and
     * that it was submitted.  But as a rule it can't know whether or
     * not it's already been unlinked ... so we respect the reversed
     * lock sequence needed for the usb_hcd_giveback_urb() code paths
     * (urb lock, then hcd_data_lock) in case some other CPU is now
     * unlinking it.
     */
     
    _spin_lock_irqsave(&urb->lock);
    _spin_lock(&hcd->hcd_data_lock); // It should be here?
    if(!urb->hcpriv || urb->transfer_flags & USB_TIMEOUT_KILLED) 
    {
        result = -EINVAL;
        goto done;
    }

    if(!urb->dev || !urb->dev->bus) 
    {
        result = -ENODEV;
        goto done;
    }

    /* giveback clears dev; non-null means it's linked at this level */
    dev = urb->dev->hcpriv;
    hcd = urb->dev->bus->hcpriv;
    if(!dev || !hcd) 
    {
        result = -ENODEV;
        goto done;
    }
	
    //_spin_lock(&hcd->hcd_data_lock); // Irene: maybe has problem!?

    /* For non-periodic transfers, any status except -EINPROGRESS means
     * the HCD has already started to unlink this URB from the hardware.
     * In that case, there's no more work to do.
     *
     * For periodic transfers, this is the only way to trigger unlinking
     * from the hardware.  Since we (currently) overload urb->status to
     * tell the driver to unlink, error status might get clobbered ...
     * unless that transfer hasn't yet restarted.  One such case is when
     * the URB gets unlinked from its completion handler.
     *
     * FIXME use an URB_UNLINKED flag to match URB_TIMEOUT_KILLED
     */
    switch(usb_pipetype(urb->pipe)) 
    {
    case PIPE_CONTROL:
    case PIPE_BULK:
        if(urb->status != -EINPROGRESS) 
        {
            result = -EINVAL;
            //goto done0;
            goto done;
        }
    }

    /* maybe set up to block on completion notification */
    if((urb->transfer_flags & USB_TIMEOUT_KILLED))
        urb->status = -ETIMEDOUT;
    else if(!(urb->transfer_flags & USB_ASYNC_UNLINK)) 
    {
        LOG_DEBUG " sync unlink \n" LOG_END
        urbEvent = SYS_CreateEvent();
        splice.done = urbEvent;
        splice.complete = urb->complete;
        splice.context = urb->context;
        urb->complete = unlink_complete;
        urb->context = &splice;
        urb->status = -ENOENT;
    } 
    else 
    {
        /* asynchronous unlink */
        urb->status = -ECONNRESET;
    }
	
    _spin_unlock(&hcd->hcd_data_lock);
    _spin_unlock_irqrestore(&urb->lock);

    // TODO?? rh_timer
    result = hcd->driver->urb_dequeue(hcd, urb);
	
    // FIXME:  if result and we tried to splice, whoa!!
    if(result && urb->status == -ENOENT) 
        LOG_ERROR "whoa! result %d", result LOG_END

    /* block till giveback, if needed */
    if(!(urb->transfer_flags & (USB_ASYNC_UNLINK|USB_TIMEOUT_KILLED))
            && HCD_IS_RUNNING (hcd->state)
            && !result) 
    {
        LOG_DEBUG " bus %d: wait for giveback urb 0x%08X \n", hcd->bus->busnum, urb LOG_END
        result = SYS_WaitEvent(splice.done, UNLINK_TIMEOUT);
        if(result)
        {
            result = -ERROR_USB_SYNC_UNLINK_TIMEOUT;
            goto end;
        }
    } 
    else if((urb->transfer_flags & USB_ASYNC_UNLINK) && result == 0) 
    {
        result = -EINPROGRESS;
        goto end;
    }
    goto end;

//done0:
//    _spin_unlock(&hcd->hcd_data_lock);
done:
    _spin_unlock(&hcd->hcd_data_lock); // It should be here ?
    _spin_unlock_irqrestore(&urb->lock);

end:
    if(urbEvent)
        SYS_DelEvent(urbEvent);
    if(result)
        LOG_ERROR " hcd_unlink_urb() return error code %d \n", result LOG_END

    LOG_LEAVE " hcd_unlink_urb() urb = 0x%08X \n", urb LOG_END
    return result;
}


MMP_INT hcd_dev_exist(struct usb_device *udev)
{
    MMP_INT result = 0;
    struct usb_hcd*	hcd;

    if(!udev || !udev->hcpriv)
    {
        LOG_ERROR " hcd_dev_exist() udev is NULL! \n" LOG_END
    }

    if(!udev->bus || !udev->bus->hcpriv)
    {
        LOG_ERROR " hcd_dev_exist() bus is NULL! \n" LOG_END
    }

    hcd = udev->bus->hcpriv;
    return hcd->driver->dev_exist(hcd);
}


static struct usb_operations hcd_operations = 
{
    hcd_alloc_dev,
    hcd_free_dev,
    hcd_get_frame_number,
    hcd_submit_urb,
    hcd_unlink_urb,
    hcd_dev_exist
};


/*-------------------------------------------------------------------------*/
/**
 * usb_hcd_giveback_urb - return URB from HCD to device driver
 * @hcd: host controller returning the URB
 * @urb: urb being returned to the USB device driver.
 * Context: in_interrupt()
 *
 * This hands the URB from HCD to its USB device driver, using its
 * completion function.  The HCD has freed all per-urb resources
 * (and is done using urb->hcpriv).  It also released all HCD locks;
 * the device driver won't cause deadlocks if it resubmits this URB,
 * and won't confuse things by modifying and resubmitting this one.
 * Bandwidth and other resources will be deallocated.
 *
 * HCDs must not use this for periodic URBs that are still scheduled
 * and will be reissued.  They should just call their completion handlers
 * until the urb is returned to the device driver by unlinking.
 *
 * NOTE that no urb->next processing is done, even for isochronous URBs.
 * ISO streaming functionality can be achieved by having completion handlers
 * re-queue URBs.  Such explicit queuing doesn't discard error reports.
 */
void usb_hcd_giveback_urb(struct usb_hcd *hcd, struct urb *urb)
{
    urb_unlink(urb);

    // NOTE:  a generic device/urb monitoring hook would go here.
    // hcd_monitor_hook(MONITOR_URB_FINISH, urb, dev)
    // It would catch exit/unlink paths for all urbs, but non-exit
    // completions for periodic urbs need hooks inside the HCD.
    // hcd_monitor_hook(MONITOR_URB_UPDATE, urb, dev)

    LOG_INFO " usb_hcd_giveback_urb() urb %08X \n", urb LOG_END

    if(urb->status)
    {
        LOG_ERROR " usb_hcd_giveback_urb() urb %08X, status %d, actual_length (0x%08X/0x%08X)\r\n", 
            urb, urb->status, urb->actual_length, urb->transfer_buffer_length LOG_END
    }

    /* pass ownership to the completion handler */
    urb->complete(urb);
}

