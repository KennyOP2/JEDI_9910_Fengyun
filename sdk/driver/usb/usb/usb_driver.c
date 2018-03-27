/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * This file is part of usb.c 
 * These codes used for finding usb driver.
 *
 * @author Irene Lin
 */


/*-------------------------------------------------------------------*/
/*
 * This is intended to be used by usb device drivers that need to
 * claim more than one interface on a device at once when probing
 * (audio and acm are good examples).  No device driver should have
 * to mess with the internal usb_interface or usb_device structure
 * members.
 */
void usb_driver_claim_interface(struct usb_driver *driver, struct usb_interface *iface, void* priv)
{
    if(!iface || !driver)
        return;

    LOG_INFO " %s driver claimed interface %p \n", driver->name, iface LOG_END

    iface->driver = driver;
    iface->private_data = priv;
}

/*
 * This should be used by drivers to check other interfaces to see if
 * they are available or not.
 */
MMP_INT usb_interface_claimed(struct usb_interface *iface)
{
    if(!iface)
        return 0;

    return (iface->driver != MMP_NULL);
}

/*
 * This should be used by drivers to release their claimed interfaces
 */
void usb_driver_release_interface(struct usb_driver *driver, struct usb_interface *iface)
{
    LOG_INFO " %s driver release interface %p \n", driver->name, iface LOG_END

    /* this should never happen, don't release something that's not ours */
    if(!iface || iface->driver != driver)
        return;

    iface->driver = MMP_NULL;
    iface->private_data = MMP_NULL;
}


/**
 * usb_match_id - find first usb_device_id matching device or interface
 * @dev: the device whose descriptors are considered when matching
 * @interface: the interface of interest
 * @id: array of usb_device_id structures, terminated by zero entry
 *
 * usb_match_id searches an array of usb_device_id's and returns
 * the first one matching the device or interface, or null.
 * This is used when binding (or rebinding) a driver to an interface.
 * Most USB device drivers will use this indirectly, through the usb core,
 * but some layered driver frameworks use it directly.
 * These device tables are exported with MODULE_DEVICE_TABLE, through
 * modutils and "modules.usbmap", to support the driver loading
 * functionality of USB hotplugging.
 *
 * What Matches:
 *
 * The "match_flags" element in a usb_device_id controls which
 * members are used.  If the corresponding bit is set, the
 * value in the device_id must match its corresponding member
 * in the device or interface descriptor, or else the device_id
 * does not match.
 *
 * "driver_info" is normally used only by device drivers,
 * but you can create a wildcard "matches anything" usb_device_id
 * as a driver's "modules.usbmap" entry if you provide an id with
 * only a nonzero "driver_info" field.  If you do this, the USB device
 * driver's probe() routine should use additional intelligence to
 * decide whether to bind to the specified interface.
 * 
 * What Makes Good usb_device_id Tables:
 *
 * The match algorithm is very simple, so that intelligence in
 * driver selection must come from smart driver id records.
 * Unless you have good reasons to use another selection policy,
 * provide match elements only in related groups, and order match
 * specifiers from specific to general.  Use the macros provided
 * for that purpose if you can.
 *
 * The most specific match specifiers use device descriptor
 * data.  These are commonly used with product-specific matches;
 * the USB_DEVICE macro lets you provide vendor and product IDs,
 * and you can also match against ranges of product revisions.
 * These are widely used for devices with application or vendor
 * specific bDeviceClass values.
 *
 * Matches based on device class/subclass/protocol specifications
 * are slightly more general; use the USB_DEVICE_INFO macro, or
 * its siblings.  These are used with single-function devices
 * where bDeviceClass doesn't specify that each interface has
 * its own class. 
 *
 * Matches based on interface class/subclass/protocol are the
 * most general; they let drivers bind to any interface on a
 * multiple-function device.  Use the USB_INTERFACE_INFO
 * macro, or its siblings, to match class-per-interface style 
 * devices (as recorded in bDeviceClass).
 *  
 * Within those groups, remember that not all combinations are
 * meaningful.  For example, don't give a product version range
 * without vendor and product IDs; or specify a protocol without
 * its associated class and subclass.
 */   
const struct usb_device_id *
usb_match_id(struct usb_device *dev, struct usb_interface *usb_interface,
         const struct usb_device_id *id)
{
    struct usb_interface_descriptor	*intf = 0;

    /* proc_connectinfo in devio.c may call us with id == NULL. */
    if(id == MMP_NULL)
        return MMP_NULL;

    /* It is important to check that id->driver_info is nonzero,
       since an entry that is all zeroes except for a nonzero
       id->driver_info is the way to create an entry that
       indicates that the driver want to examine every
       device and interface. */
    for(; id->idVendor || id->bDeviceClass || id->bInterfaceClass; id++) 
    {
        if((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
            id->idVendor != dev->descriptor.idVendor)
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
            id->idProduct != dev->descriptor.idProduct)
            continue;

        /* No need to test id->bcdDevice_lo != 0, since 0 is never
           greater than any unsigned number. */
        if((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
           (id->bcdDevice_lo > dev->descriptor.bcdDevice))
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
           (id->bcdDevice_hi < dev->descriptor.bcdDevice))
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
           (id->bDeviceClass != dev->descriptor.bDeviceClass))
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
           (id->bDeviceSubClass!= dev->descriptor.bDeviceSubClass))
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
           (id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
            continue;

        intf = &usb_interface->altsetting[usb_interface->act_altsetting];

        if((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
           (id->bInterfaceClass != intf->bInterfaceClass))
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
           (id->bInterfaceSubClass != intf->bInterfaceSubClass))
            continue;

        if((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
           (id->bInterfaceProtocol != intf->bInterfaceProtocol))
            continue;

        return id;
    }

    return MMP_NULL;
}

/*
 * This entrypoint gets called for each new device.
 *
 * We now walk the list of registered USB drivers,
 * looking for one that will accept this interface.
 *
 * The probe return value is changed to be a private pointer.  This way
 * the drivers don't have to dig around in our structures to set the
 * private pointer if they only need one interface. 
 *
 * Returns: 0 if a driver accepted the interface, -1 otherwise
 */
static MMP_INT usb_find_interface_driver(struct usb_device *dev, MMP_UINT ifnum)
{
    MMP_INT result=0;
    struct list_head *tmp;
    struct usb_interface *usb_interface;
    void *usb_private;
    const struct usb_device_id *id;
    struct usb_driver *driver;
    MMP_INT i;

    if((!dev) || (ifnum >= dev->actconfig->bNumInterfaces)) 
    {
        LOG_ERROR "bad find_interface_driver params, dev %p, ifnum %d \n", dev, ifnum LOG_END
        result = -ERROR_USB_BAD_PARAMS;
        return result;
    }

    SYS_WaitSemaphore(dev->serialize);

    usb_interface = dev->actconfig->usb_interface + ifnum;

    if(usb_interface_claimed(usb_interface))
    {
        result = -1;
        goto end;
    }

    usb_private = MMP_NULL;
    for(tmp = usb_driver_list.next; tmp != &usb_driver_list;) 
    {
        driver = list_entry(tmp, struct usb_driver, driver_list);
        tmp = tmp->next;

        id = driver->id_table;
        /* new style driver? */
        if(id) 
        {
            for(i=0; i<usb_interface->num_altsetting; i++) 
            {
                usb_interface->act_altsetting = i;
                id = usb_match_id(dev, usb_interface, id);
                if(id) 
                {
                    SYS_WaitSemaphore(driver->serialize);
                    usb_private = driver->probe(dev,ifnum,id);
                    SYS_ReleaseSemaphore(driver->serialize);
                    if(usb_private != MMP_NULL)
                        break;
                }
            }

            /* if driver not bound, leave defaults unchanged */
            if(usb_private == MMP_NULL)
                usb_interface->act_altsetting = 0;
        } 
        else 
        { /* "old style" driver */
            SYS_WaitSemaphore(driver->serialize);
            usb_private = driver->probe(dev, ifnum, MMP_NULL);
            SYS_ReleaseSemaphore(driver->serialize);
        }

        /* probe() may have changed the config on us */
        usb_interface = dev->actconfig->usb_interface + ifnum;
        if(usb_private) 
        {
            usb_driver_claim_interface(driver, usb_interface, usb_private);
            dev->device_info[dev->driverNum].type = dev->type;
            dev->device_info[dev->driverNum].ctxt = usb_private;
            dev->driverNum++;
            goto end;
        }
    }

    if(!dev->driverNum)
    {
        result = -1;
    }

end:
    SYS_ReleaseSemaphore(dev->serialize); 
    return result;
}

#if defined(USB_LOGO_TEST)

static struct usb_device_id id_hub[] = 
{
  {USB_INTERFACE_CLASS(USB_CLASS_HUB)},
  {USB_INTERFACE_CLASS(0)}
};

static MMP_BOOL usb_is_interface_hub(struct usb_device *dev, MMP_UINT ifnum)
{
    MMP_BOOL isHub = MMP_FALSE;
    struct usb_interface *usb_interface;
    MMP_INT i;

    SYS_WaitSemaphore(dev->serialize);

    usb_interface = dev->actconfig->usb_interface + ifnum;

    for(i=0; i<usb_interface->num_altsetting; i++) 
    {
        usb_interface->act_altsetting = i;
        if(usb_match_id(dev, usb_interface, &id_hub))
        {
            isHub = MMP_TRUE;
            break;
        }
    }

end:
    SYS_ReleaseSemaphore(dev->serialize); 
    return isHub;
}
#endif

/*
 * This entrypoint gets called for each new device.
 *
 * All interfaces are scanned for matching drivers.
 */
static MMP_INT usb_find_drivers(struct usb_device *dev)
{
    MMP_INT result = 0;
    MMP_UINT ifnum;
    MMP_UINT rejected = 0;
    MMP_UINT claimed = 0;

    for(ifnum=0; ifnum < dev->actconfig->bNumInterfaces; ifnum++) 
    {
        /* if this interface hasn't already been claimed */
        if(!usb_interface_claimed(dev->actconfig->usb_interface + ifnum)) 
        {
            if(usb_find_interface_driver(dev, ifnum))
                rejected++;
            else
                claimed++;
        }
    }

    LOG_INFO " rejected %d, claimed %d \n", rejected, claimed LOG_END
    if(rejected)
        LOG_WARNING " %d unhandled interfaces on device \n", rejected LOG_END

    if(!claimed) 
    {
        LOG_ERROR "USB device %d (vend/prod 0x%x/0x%x) is not claimed by any active driver!! \n",
            dev->devnum,
            dev->descriptor.idVendor,
            dev->descriptor.idProduct LOG_END
        result = ERROR_USB_NO_USB_DRIVER_SUPPORT;

        #if defined(USB_LOGO_TEST)
        if(usb_is_interface_hub(dev, 0) == MMP_TRUE)
            result = HUB_ERROR;
        #endif
    }

    return result;
}


