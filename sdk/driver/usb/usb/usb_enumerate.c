/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * This file is part of usb.c 
 * These codes used for enumerate new usb device.
 *
 * @author Irene Lin
 */


/*-------------------------------------------------------------------*/

static MMP_INT usb_parse_endpoint(struct usb_endpoint_descriptor *endpoint, MMP_UINT8 *buffer, MMP_INT size)
{
    MMP_INT result = 0;
    struct usb_descriptor_header *header=MMP_NULL;
    MMP_UINT8 *begin;
    MMP_INT parsed = 0, len, numskipped;

    header = (struct usb_descriptor_header *)buffer;

    /* Everything should be fine being passed into here, but we sanity */
    /*  check JIC */
    if(header->bLength > size) 
    {
        LOG_ERROR "ran out of descriptors parsing \n" LOG_END
        result = -ERROR_USB_INVALID_DESCRIPTOR_LEN;
        goto end;
    }
        
    if(header->bDescriptorType != USB_DT_ENDPOINT) 
    {
        LOG_WARNING "unexpected descriptor 0x%X, expecting endpoint descriptor, type 0x%X \n",
            endpoint->bDescriptorType, USB_DT_ENDPOINT LOG_END
        result = parsed;
        goto end;
    }

    if(header->bLength == USB_DT_ENDPOINT_AUDIO_SIZE)
        memcpy(endpoint, buffer, USB_DT_ENDPOINT_AUDIO_SIZE);
    else
        memcpy(endpoint, buffer, USB_DT_ENDPOINT_SIZE);
    #if defined(DUMP_DEVICE_INFO)
    {
        MMP_UINT32 j;
        MMP_UINT8* data = (MMP_UINT8*)endpoint;
        LOG_DATA "\n Endpoint Descriptor:\n" LOG_END
        for(j=0; j<USB_DT_ENDPOINT_SIZE; j++)
            LOG_DATA " %02X", data[j] LOG_END
        LOG_DATA " \n" LOG_END
    }
    #endif
    
    endpoint->wMaxPacketSize = le16_to_cpu(endpoint->wMaxPacketSize);

    buffer += header->bLength;
    size -= header->bLength;
    parsed += header->bLength;

    /* Skip over the rest of the Class Specific or Vendor Specific */
    /*  descriptors */
    begin = buffer;
    numskipped = 0;
    while(size >= sizeof(struct usb_descriptor_header)) 
    {
        header = (struct usb_descriptor_header *)buffer;

        if(header->bLength < 2) 
        {
            LOG_ERROR "invalid descriptor length of %d \n", header->bLength LOG_END
            result = -ERROR_USB_INVALID_DESCRIPTOR_LEN;
            goto end;
        }

        /* If we find another "proper" descriptor then we're done  */
        if ((header->bDescriptorType == USB_DT_ENDPOINT) ||
            (header->bDescriptorType == USB_DT_INTERFACE) ||
            (header->bDescriptorType == USB_DT_CONFIG) ||
            (header->bDescriptorType == USB_DT_DEVICE))
            break;

        LOG_WARNING "skipping descriptor 0x%X \n",	header->bDescriptorType LOG_END
        numskipped++;

        buffer += header->bLength;
        size -= header->bLength;
        parsed += header->bLength;
    }
    if(numskipped)
        LOG_WARNING "skipped %d class/vendor specific endpoint descriptors \n", numskipped LOG_END

    /* Copy any unknown descriptors into a storage area for drivers */
    /*  to later parse */
    len = (MMP_INT)(buffer - begin);
    if(!len) 
    {
        endpoint->extra = MMP_NULL;
        endpoint->extralen = 0;
        result = parsed;
        goto end;
    }

    endpoint->extra = SYS_Malloc(len);
    if(!endpoint->extra) 
    {
        LOG_ERROR "couldn't allocate memory for endpoint extra descriptors \n" LOG_END
        endpoint->extralen = 0;
        result = parsed;
        goto end;
    }

    memcpy(endpoint->extra, begin, len);
    endpoint->extralen = len;
    result = parsed;

end:
    if(result < 0)
        LOG_ERROR " usb_parse_endpoint() return error code 0x%08X \n", (-result) LOG_END

    return result;
}

static MMP_INT usb_parse_interface(struct usb_interface *usb_interface, MMP_UINT8 *buffer, MMP_INT size)
{
    MMP_INT i, len, numskipped, retval, parsed = 0, result=0;
    struct usb_descriptor_header *header=MMP_NULL;
    struct usb_interface_descriptor *ifp=MMP_NULL;
    MMP_UINT8* begin=MMP_NULL;

    usb_interface->act_altsetting = 0;
    usb_interface->num_altsetting = 0;
    usb_interface->max_altsetting = USB_ALTSETTINGALLOC;

    usb_interface->altsetting = SYS_Malloc(sizeof(struct usb_interface_descriptor) * usb_interface->max_altsetting);
    if(!usb_interface->altsetting) 
    {
        LOG_ERROR "couldn't kmalloc interface->altsetting \n" LOG_END
        result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
        goto end;
    }

    while(size > 0) 
    {
        if(usb_interface->num_altsetting >= usb_interface->max_altsetting) 
        {
            void *ptr;
            MMP_INT oldmas;

            oldmas = usb_interface->max_altsetting;
            usb_interface->max_altsetting += USB_ALTSETTINGALLOC;
            if(usb_interface->max_altsetting > USB_MAXALTSETTING) 
            {
                LOG_ERROR "too many alternate settings (max %d) \n", USB_MAXALTSETTING LOG_END
                result = -ERROR_USB_TOO_MANY_ALTERNATE_SETTING;
                goto end;
            }

            ptr = usb_interface->altsetting;
            usb_interface->altsetting = SYS_Malloc(sizeof(struct usb_interface_descriptor) * usb_interface->max_altsetting);
            if(!usb_interface->altsetting) 
            {
                LOG_ERROR "couldn't kmalloc interface->altsetting \n" LOG_END
                usb_interface->altsetting = ptr;
                result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
                goto end;
            }
            memcpy(usb_interface->altsetting, ptr, sizeof(struct usb_interface_descriptor) * oldmas);
            SYS_Free(ptr);
        }

        ifp = usb_interface->altsetting + usb_interface->num_altsetting;
        usb_interface->num_altsetting++;

        memcpy(ifp, buffer, USB_DT_INTERFACE_SIZE);
        #if defined(DUMP_DEVICE_INFO)
        {
            MMP_UINT32 j;
            MMP_UINT8* data = (MMP_UINT8*)ifp;
            LOG_DATA "\n Interface Descriptor: %d\n", (usb_interface->num_altsetting-1) LOG_END
            for(j=0; j<USB_DT_INTERFACE_SIZE; j++)
                LOG_DATA " %02X", data[j] LOG_END
            LOG_DATA " \n" LOG_END
        }
        #endif

        /* Skip over the interface */
        buffer += ifp->bLength;
        parsed += ifp->bLength;
        size -= ifp->bLength;

        begin = buffer;
        numskipped = 0;

        /* Skip over any interface, class or vendor descriptors */
        while(size >= sizeof(struct usb_descriptor_header)) 
        {
            header = (struct usb_descriptor_header *)buffer;
            if(header->bLength < 2) 
            {
                LOG_ERROR "invalid descriptor length of %d \n", header->bLength LOG_END
                result = -ERROR_USB_INVALID_DESCRIPTOR_LEN;
                goto end;
            }

            /* If we find another "proper" descriptor then we're done  */
            if ((header->bDescriptorType == USB_DT_INTERFACE) ||
                (header->bDescriptorType == USB_DT_ENDPOINT) ||
                (header->bDescriptorType == USB_DT_CONFIG) ||
                (header->bDescriptorType == USB_DT_DEVICE))
                break;

            numskipped++;

            buffer += header->bLength;
            parsed += header->bLength;
            size -= header->bLength;
        }

        if(numskipped)
            LOG_WARNING "skipped %d class/vendor specific interface descriptors \n", numskipped LOG_END

        /* Copy any unknown descriptors into a storage area for */
        /*  drivers to later parse */
        len = (MMP_INT)(buffer - begin);
        if(!len) 
        {
            ifp->extra = MMP_NULL;
            ifp->extralen = 0;
        } 
        else 
        {
            ifp->extra = SYS_Malloc(len);
            if(!ifp->extra) 
            {
                LOG_ERROR "couldn't allocate memory for interface extra descriptors \n" LOG_END
                ifp->extralen = 0;
                result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
                goto end;
            }
            memcpy(ifp->extra, begin, len);
            ifp->extralen = len;
            #if defined(DUMP_DEVICE_INFO)
            {
                MMP_INT j;
                MMP_UINT8* data = (MMP_UINT8*)ifp->extra;
                LOG_DATA "\n Interface Extra Descriptor: \n" LOG_END
                for(j=0; j<len; j++)
                    LOG_DATA " %02X", data[j] LOG_END
                LOG_DATA " \n" LOG_END
            }
            #endif
        }

        /* Did we hit an unexpected descriptor? */
        header = (struct usb_descriptor_header *)buffer;
        if ((size >= sizeof(struct usb_descriptor_header)) &&
            ((header->bDescriptorType == USB_DT_CONFIG) ||
             (header->bDescriptorType == USB_DT_DEVICE)))
        {
            result = parsed;
            goto end;
        }

        if(ifp->bNumEndpoints > USB_MAXENDPOINTS) 
        {
            LOG_ERROR " too many endpoints %d \n", ifp->bNumEndpoints LOG_END
            result = -ERROR_USB_TOO_MANY_ENDPOINT;
            goto end;
        }

        ifp->endpoint = (struct usb_endpoint_descriptor*)SYS_Malloc(ifp->bNumEndpoints * sizeof(struct usb_endpoint_descriptor));
        if(!ifp->endpoint) 
        {
            result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
            goto end;
        }

        memset(ifp->endpoint, 0, ifp->bNumEndpoints * sizeof(struct usb_endpoint_descriptor));
    
        for(i=0; i<ifp->bNumEndpoints; i++) 
        {
            header = (struct usb_descriptor_header *)buffer;

            if(header->bLength > size) 
            {
                LOG_ERROR "ran out of descriptors parsing \n" LOG_END
                result = -ERROR_USB_INVALID_DESCRIPTOR_LEN;
                goto end;
            }
        
            retval = usb_parse_endpoint(ifp->endpoint + i, buffer, size);
            if(retval < 0)
            {
                result = retval;
                goto end;
            }

            buffer += retval;
            parsed += retval;
            size -= retval;
        }

        /* We check to see if it's an alternate to this one */
        ifp = (struct usb_interface_descriptor *)buffer;
        if (size < USB_DT_INTERFACE_SIZE ||
            ifp->bDescriptorType != USB_DT_INTERFACE ||
            !ifp->bAlternateSetting)
        {
            result = parsed;
            goto end;
        }
    }

end:
    if(result<0)
        LOG_ERROR " usb_parse_interface() reutrn error code 0x%08X \n", (-result) LOG_END
    return result;
}

MMP_INT usb_parse_configuration(struct usb_config_descriptor *config, MMP_UINT8* buffer)
{
    MMP_INT size=0, i=0, retval=0, result=0;
    struct usb_descriptor_header *header = MMP_NULL;

    memcpy(config, buffer, USB_DT_CONFIG_SIZE);
    #if defined(DUMP_DEVICE_INFO)
    {
        MMP_UINT32 j;
        MMP_UINT8* data = (MMP_UINT8*)config;
        LOG_DATA "\n Config Descriptor:\n" LOG_END
        for(j=0; j<USB_DT_CONFIG_SIZE; j++)
            LOG_DATA " %02X", data[j] LOG_END
        LOG_DATA " \n" LOG_END
    }
    #endif
    config->wTotalLength = le16_to_cpu(config->wTotalLength);
    size = config->wTotalLength;

    if(config->bNumInterfaces > USB_MAXINTERFACES) 
    {
        result = -ERROR_USB_TOO_MANY_INTERFACES;
        goto end;
    }

    config->usb_interface = (struct usb_interface*)SYS_Malloc(config->bNumInterfaces * sizeof(struct usb_interface));
    if(!config->usb_interface) 
    {
        result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
        goto end;
    }

    memset(config->usb_interface, 0, config->bNumInterfaces * sizeof(struct usb_interface));

    buffer += config->bLength;
    size -= config->bLength;
    
    config->extra = MMP_NULL;
    config->extralen = 0;

    for(i=0; i<config->bNumInterfaces; i++) 
    {
        MMP_INT numskipped=0, len=0;
        MMP_UINT8* begin=MMP_NULL;

        /* Skip over the rest of the Class Specific or Vendor */
        /*  Specific descriptors */
        begin = buffer;
        numskipped = 0;
        while(size >= sizeof(struct usb_descriptor_header)) 
        {
            header = (struct usb_descriptor_header *)buffer;

            if((header->bLength > size) || (header->bLength < 2)) 
            {
                LOG_ERROR "invalid descriptor length of %d, size %d \n", header->bLength, size LOG_END
                result = -ERROR_USB_INVALID_DESCRIPTOR_LEN;
                goto end;
            }

            /* If we find another "proper" descriptor then we're done  */
            if ((header->bDescriptorType == USB_DT_ENDPOINT) ||
                (header->bDescriptorType == USB_DT_INTERFACE) ||
                (header->bDescriptorType == USB_DT_CONFIG) ||
                (header->bDescriptorType == USB_DT_DEVICE))
                break;

            LOG_WARNING "skipping descriptor 0x%X \n", header->bDescriptorType LOG_END
            numskipped++;

            buffer += header->bLength;
            size -= header->bLength;
        }
        if(numskipped)
            LOG_WARNING "skipped %d class/vendor specific endpoint descriptors \n", numskipped LOG_END

        /* Copy any unknown descriptors into a storage area for */
        /*  drivers to later parse */
        len = (MMP_INT)(buffer - begin);
        if(len) 
        {
            if(config->extralen) 
            {
                LOG_WARNING "extra config descriptor \n" LOG_END
            } 
            else 
            {
                config->extra = SYS_Malloc(len);
                if(!config->extra) 
                {
                    LOG_ERROR "couldn't allocate memory for config extra descriptors \n" LOG_END
                    config->extralen = 0;
                    result = -ERROR_USB_ALLOC_SYS_MEM_FAIL;
                    goto end;
                }

                memcpy(config->extra, begin, len);
                config->extralen = len;
            }
        }

        retval = usb_parse_interface(config->usb_interface + i, buffer, size);
        if(retval < 0)
        {
            result = retval;
            goto end;
        }

        buffer += retval;
        size -= retval;
    }
    result = size;

end:
    if(result<0)
        LOG_ERROR " usb_parse_configuration() return error code 0x%08X \n", (-result) LOG_END
    return result;
}

void usb_destroy_configuration(struct usb_device *dev)
{
    MMP_INT c, i, j, k;

    if(!dev->config)
        return;

    if(dev->rawdescriptors) 
    {
        for(i=0; i<dev->descriptor.bNumConfigurations; i++)
            SYS_Free(dev->rawdescriptors[i]);

        SYS_Free(dev->rawdescriptors);
    }

    for(c=0; c<dev->descriptor.bNumConfigurations; c++) 
    {
        struct usb_config_descriptor *cf = &dev->config[c];

        if(!cf->usb_interface)
            break;

        for(i=0; i<cf->bNumInterfaces; i++) 
        {
            struct usb_interface *ifp =	&cf->usb_interface[i];
                
            if(!ifp->altsetting)
                break;

            for(j=0; j<ifp->num_altsetting; j++) 
            {
                struct usb_interface_descriptor *as = &ifp->altsetting[j];
                    
                if(as->extra)
                    SYS_Free(as->extra);

                if(!as->endpoint)
                    break;
                    
                for(k=0; k<as->bNumEndpoints; k++) 
                {
                    if(as->endpoint[k].extra)
                        SYS_Free(as->endpoint[k].extra);
                }	
                SYS_Free(as->endpoint);
            }
            SYS_Free(ifp->altsetting);
        }
        SYS_Free(cf->usb_interface);
    }
    SYS_Free(dev->config);
}


//=============================================================================
//                   Enumerate related functions
//=============================================================================

void usb_set_maxpacket(struct usb_device *dev)
{
    MMP_INT i, b;

    for(i=0; i<dev->actconfig->bNumInterfaces; i++) 
    {
        struct usb_interface *ifp = dev->actconfig->usb_interface + i;
        struct usb_interface_descriptor *as = ifp->altsetting + ifp->act_altsetting;
        struct usb_endpoint_descriptor *ep = as->endpoint;
        MMP_INT e;

        for(e=0; e<as->bNumEndpoints; e++) 
        {
            b = ep[e].bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            if((ep[e].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_CONTROL) 
            {	/* Control => bidirectional */
                dev->epmaxpacketout[b] = ep[e].wMaxPacketSize;
                dev->epmaxpacketin [b] = ep[e].wMaxPacketSize;
            }
            else if(usb_endpoint_out(ep[e].bEndpointAddress)) 
            {
                if(ep[e].wMaxPacketSize > dev->epmaxpacketout[b])
                    dev->epmaxpacketout[b] = ep[e].wMaxPacketSize;
            }
            else 
            {
                if(ep[e].wMaxPacketSize > dev->epmaxpacketin [b])
                    dev->epmaxpacketin [b] = ep[e].wMaxPacketSize;
            }
        }
    }
}

MMP_INT usb_set_address(struct usb_device *dev)
{
    LOG_DEBUG " usb_set_address() devnum %d \n", dev->devnum LOG_END
    return usb_control_msg(dev, 
                           usb_snddefctrl(dev), 
                           USB_REQ_SET_ADDRESS,
                           0, 
                           dev->devnum, 
                           0, 
                           NULL, 
                           0, 
                           SET_TIMEOUT);
}

MMP_INT usb_get_descriptor(struct usb_device *dev, MMP_UINT8 type, MMP_UINT8 index, void *buf, MMP_INT size)
{
    MMP_INT i = 5;
    MMP_INT result = 0;
    
    LOG_DEBUG " usb_get_descriptor() type 0x%02X, index 0x%02X \n", type, index LOG_END

    memset(buf,0,size);	// Make sure we parse really received data

    while(i--) 
    {
        result = usb_control_msg( dev, 
                                  usb_rcvctrlpipe(dev, 0),
                                  USB_REQ_GET_DESCRIPTOR, 
                                  USB_DIR_IN,
                                  ((type << 8) + index), 
                                  0, 
                                  buf, 
                                  size, 
                                  GET_TIMEOUT);
        if((result > 0) || (result == -EPIPE))
            break;	/* retry if the returned length was 0; flaky device */
    }
    return result;
}

MMP_INT usb_get_device_descriptor(struct usb_device *dev)
{
    MMP_INT result = usb_get_descriptor(dev, USB_DT_DEVICE, 0, &dev->descriptor, sizeof(dev->descriptor));

    #if defined(DUMP_DEVICE_INFO)
    {
        MMP_UINT8* data = (MMP_UINT8*)&dev->descriptor;
        MMP_UINT32 i=0;

        LOG_DATA " \n Device Descriptor: \n" LOG_END
        for(i=0; i<sizeof(dev->descriptor); i++)
            LOG_DATA " %02X", data[i] LOG_END
        LOG_DATA " \n\n" LOG_END
    }
    #endif

    if(result >= 0) 
    {
        dev->descriptor.bcdUSB    = le16_to_cpu(dev->descriptor.bcdUSB);
        dev->descriptor.idVendor  = le16_to_cpu(dev->descriptor.idVendor);
        dev->descriptor.idProduct = le16_to_cpu(dev->descriptor.idProduct);
        dev->descriptor.bcdDevice = le16_to_cpu(dev->descriptor.bcdDevice);
    }
    return result;
}

MMP_INT usb_get_configuration(struct usb_device *dev)
{
    MMP_INT result=0;
    MMP_UINT cfgno=0, length=0;
    MMP_UINT8 *buffer = MMP_NULL;
    MMP_UINT8 *bigbuffer = MMP_NULL;
    struct usb_config_descriptor *desc=MMP_NULL;

    if(dev->descriptor.bNumConfigurations > USB_MAXCONFIG) 
    {
        LOG_ERROR "too many configurations \n" LOG_END
        result = -EINVAL;
        goto end;
    }

    if(dev->descriptor.bNumConfigurations < 1) 
    {
        LOG_ERROR "not enough configurations \n" LOG_END
        result = -EINVAL;
        goto end;
    }

    dev->config = (struct usb_config_descriptor*)SYS_Malloc(dev->descriptor.bNumConfigurations * sizeof(struct usb_config_descriptor));
    if(!dev->config) 
    {
        LOG_ERROR "out of memory => dev->config \n" LOG_END
        result = -ENOMEM;
        goto end;
    }
    SYS_MemorySet(dev->config, 0, dev->descriptor.bNumConfigurations * sizeof(struct usb_config_descriptor));

    dev->rawdescriptors = (MMP_UINT8**)SYS_Malloc(sizeof(MMP_UINT8*) * dev->descriptor.bNumConfigurations);
    if(!dev->rawdescriptors) 
    {
        LOG_ERROR "out of memory => dev->rawdescriptors \n" LOG_END
        result = -ENOMEM;
        goto end;
    }

    buffer = SYS_Malloc(8);
    if(!buffer) 
    {
        LOG_ERROR "unable to allocate memory for configuration descriptors \n" LOG_END
        result = -ENOMEM;
        goto end;
    }
    desc = (struct usb_config_descriptor *)buffer;

    for(cfgno=0; cfgno<dev->descriptor.bNumConfigurations; cfgno++) 
    {
        /* We grab the first 8 bytes so we know how long the whole configuration is */
        result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 8);
        if(result < 8) 
        {
            if(result < 0)
                LOG_ERROR "unable to get descriptor \n" LOG_END
            else 
            {
                LOG_ERROR "config descriptor too short (expected %i, got %i) \n", 8, result LOG_END
                result = -EINVAL;
            }
            goto end;
        }

        /* Get the full buffer */
        length = le16_to_cpu(desc->wTotalLength);
        bigbuffer = SYS_Malloc(length);
        if(!bigbuffer) 
        {
            LOG_ERROR "unable to allocate memory for configuration descriptors \n" LOG_END
            result = -ENOMEM;
            goto end;
        }

        /* Now that we know the length, get the whole thing */
        result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, bigbuffer, length);
        if(result < 0) 
        {
            LOG_ERROR "couldn't get all of config descriptors \n" LOG_END
            SYS_Free(bigbuffer);
            goto end;
        }	
        if(result < (MMP_INT)length) 
        {
            LOG_ERROR "config descriptor too short (expected %i, got %i) \n", length, result LOG_END
            result = -EINVAL;
            SYS_Free(bigbuffer);
            goto end;
        }

        dev->rawdescriptors[cfgno] = bigbuffer;
        #if defined(DUMP_DEVICE_INFO)
        {
            MMP_UINT32 j;
            MMP_UINT8* data = (MMP_UINT8*)bigbuffer;
            LOG_DATA "\n Raw Configuration Descriptor: %d\n", cfgno LOG_END
            for(j=0; j<length; j++)
            {
                LOG_DATA " %02X", data[j] LOG_END
                if(!((j+1)%16))
                    LOG_DATA " \n" LOG_END
            }
            LOG_DATA " \n" LOG_END
        }
        #endif

        result = usb_parse_configuration(&dev->config[cfgno], bigbuffer);
        if(result > 0)
            LOG_WARNING "descriptor data left \n" LOG_END
        else if(result < 0) 
        {
            result = -EINVAL;
            goto end;
        }
    }

end:
    if(buffer)
        SYS_Free(buffer);

    if(result < 0)
    {
        dev->descriptor.bNumConfigurations = cfgno;
        LOG_ERROR " usb_get_configuration() return error code 0x%08X\n", (-result) LOG_END
    }
    return result;
}

MMP_INT usb_set_configuration(struct usb_device *dev, MMP_INT configuration)
{
    MMP_INT i, result;
    struct usb_config_descriptor *cp = MMP_NULL;

    for(i=0; i<dev->descriptor.bNumConfigurations; i++) 
    {
        if(dev->config[i].bConfigurationValue == configuration) 
        {
            cp = &dev->config[i];
            break;
        }
    }
    if(!cp) 
    {
        LOG_ERROR "selecting invalid configuration %d \n", configuration LOG_END
        result = -EINVAL;
        goto end;
    }

    result = usb_control_msg(dev, 
                             usb_sndctrlpipe(dev, 0),
                             USB_REQ_SET_CONFIGURATION, 
                             0, 
                             configuration, 
                             0, 
                             MMP_NULL, 
                             0, 
                             SET_TIMEOUT);
    if(result < 0)
        goto end;

    dev->actconfig = cp;
    dev->toggle[0] = 0;
    dev->toggle[1] = 0;
    usb_set_maxpacket(dev);

end:
    if(result < 0)
        LOG_ERROR " usb_set_configuration() return error code 0x%08X \n", (-result) LOG_END
    return result;
}

MMP_INT usb_get_string(struct usb_device *dev, MMP_UINT16 langid, MMP_UINT8 index, void *buf, MMP_INT size)
{
    return usb_control_msg( dev, 
                            usb_rcvctrlpipe(dev, 0),
                            USB_REQ_GET_DESCRIPTOR, 
                            USB_DIR_IN,
                            (USB_DT_STRING << 8) + index, 
                            langid, 
                            buf, 
                            size, 
                            GET_TIMEOUT);
}

/*
 * usb_string:
 *	returns string length (> 0) or error (< 0)
 */
MMP_INT usb_string(struct usb_device *dev, MMP_INT index, MMP_UINT8 *buf, MMP_INT size)
{
    MMP_INT result = 0;
    MMP_UINT8 *tbuf = MMP_NULL;
    MMP_UINT u, idx;

    if(size <= 0 || !buf || !index)
    {
        result = -EINVAL;
        goto end;
    }

    buf[0] = 0;
    tbuf = SYS_Malloc(256);
    if(!tbuf)
    {
        result = -ENOMEM;
        goto end;
    }

    /* get langid for strings if it's not yet known */
    if(!dev->have_langid) 
    {
        result = usb_get_string(dev, 0, 0, tbuf, 4);
        if(result < 0) 
        {
            LOG_ERROR "error getting string descriptor 0 (error=%d) \n", result LOG_END
            goto end;
        } 
        else if(tbuf[0] < 4) 
        {
            LOG_ERROR "string descriptor 0 too short \n" LOG_END
            result = -EINVAL;
            goto end;
        } 
        else 
        {
            dev->have_langid = -1;
            dev->string_langid = tbuf[2] | (tbuf[3]<< 8);
                /* always use the first langid listed */
            LOG_INFO "USB device number %d default language ID 0x%x \n", dev->devnum, dev->string_langid LOG_END
        }
    }

    /*
     * Just ask for a maximum length string and then take the length
     * that was returned.
     */
    result = usb_get_string(dev, dev->string_langid, index, tbuf, 255);
    if(result < 0)
        goto end;

    size--;		/* leave room for trailing NULL char in output buffer */
    for(idx=0, u=2; u<(MMP_UINT)result; u+=2) 
    {
        if(idx >= (MMP_UINT)size)
            break;
        if(tbuf[u+1])			/* high byte */
            buf[idx++] = '?';  /* non-ASCII character */
        else
            buf[idx++] = tbuf[u];
    }
    buf[idx] = 0;
    result = idx;

end:
    if(tbuf)
        SYS_Free(tbuf);

    if(result < 0)
        LOG_ERROR " usb_string() return error code 0x%08X \n", (-result) LOG_END
    return result;
}

void usb_show_string(struct usb_device *dev, MMP_UINT8 *id, MMP_INT index)
{
    MMP_UINT8 *buf=MMP_NULL;

    if(!index)
        return;
    if(!(buf = SYS_Malloc(256)))
        return;
    if(usb_string(dev, index, buf, 256) > 0)
        LOG_DATA "%s: %s\n", id, buf LOG_END

    SYS_Free(buf);
}


