/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file transport.c
 * Driver for USB Tuner compliant devices.
 *
 * @author Barry Wu
 */


#include "usb_mod.h"
#include "usb_mod_transport.h"
//#include "usb/ite_usb.h"

#define SEND_DIAGNOSTIC               0x1D

unsigned long gUsbCacheCount[2] = { 0 };

/**
 * This issues a CB[I] Reset to the device in question
 */
int 
usb_mod_CB_reset(
    struct usbtuner_data *ut)
{
    uint8_t cmd[12];
    int result = 0;

    LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END

    /** if the device was removed, then we're already reset */
    if(!ut->pusb_dev)
        goto end;

    memset(cmd, 0xFF, sizeof(cmd));
    cmd[0] = SEND_DIAGNOSTIC;
    cmd[1] = 4;
    result = usb_control_msg(ut->pusb_dev, 
                             usb_sndctrlpipe(ut->pusb_dev,0),
                             UT_CBI_ADSC, 
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0, 
                             ut->ifnum, 
                             cmd, 
                             sizeof(cmd), 
                             1000);
    if(result < 0) 
    {
        LOG_ERROR " CB[I] soft reset failed %d\n", result LOG_END
        result = ERROR_USB_TUNER_CBI_SOFT_RESET_FAIL;
        goto end;
    }

    /** long wait for reset */
    //MMP_Sleep(2000);

    LOG_INFO " CB_reset: clearing endpoint halt\n" LOG_END
    usb_mod_clear_halt(ut->pusb_dev, usb_rcvbulkpipe(ut->pusb_dev, ut->ep_in));
    usb_mod_clear_halt(ut->pusb_dev, usb_sndbulkpipe(ut->pusb_dev, ut->ep_out));
    LOG_INFO " CB_reset done\n" LOG_END

end:
    LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    return result;
}

int usb_mod_CBI_transport(struct usbtuner_data *ut)
{
   return;
}

uint32_t usb_mod_CBI_sendcmd(struct usbtuner_data* ut, uint8_t* buffer, uint16_t size)
{
    int result = 0;
    uint32_t pipe = 0;
    int actual_len = 0;
    
    LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END
    //printf("Trans >>> EP IN: %d,EP OUT: %d\n", ut->ep_in, ut->ep_out);
    
    pipe = usb_sndbulkpipe(ut->pusb_dev, ut->ep_out);
    result = usb_bulk_msg(ut->pusb_dev, pipe, buffer, size, &actual_len, UT_BULK_TIMEOUT);

    if( result != 0 )
    {
        LOG_ERROR " transport erroe = 0x%x \n", result LOG_END
    }
    LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    return result;
}

/**
 * This is a version of usb_clear_halt() that doesn't read the status from
 * the device -- this is because some devices crash their internal firmware
 * when the status is requested after a halt
 */
int usb_mod_clear_halt(struct usb_device *dev, int pipe)
{
    int result = 0;
    uint8_t status[2] = {0};
    uint16_t endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

    LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END
    
    result = usb_control_msg(dev, 
                             usb_sndctrlpipe(dev, 0),
                             USB_REQ_CLEAR_FEATURE, 
                             USB_RECIP_ENDPOINT, 
                             0, 
                             endp, 
                             0, 
                             0, 
                             2000);
    /* don't clear if failed */
    if (result < 0)
        return result;

    usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

    /* toggle is reset on clear */
    usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);

    LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    return 0;
}

uint32_t usb_mod_CBI_receivecmd(struct usbtuner_data* ut, uint8_t* rcvbyteBuf, uint16_t size)
{
    int result = 0;
    uint32_t pipe = 0;
    int actual_len = 0;

    LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END
    //printf("Rcv >>> EP IN: %d,EP OUT: %d\n", ut->ep_in, ut->ep_out);

    pipe = usb_rcvbulkpipe(ut->pusb_dev, ut->ep_in);
    result = usb_bulk_msg(ut->pusb_dev, pipe, rcvbyteBuf, size, &actual_len, UT_BULK_TIMEOUT);
 
    if(result != 0)
    {
        LOG_ERROR " transport erroe = 0x%x \n", result LOG_END
    }
    LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    return result;
}

uint32_t usb_mod_CBI_transferData(struct usbtuner_data* ut, uint8_t* buffer, uint32_t size)
{
    int result = 0;
    uint32_t pipe = 0;
    int actual_len = 0;

    LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END    
    //printf("DataRcv >>> EP IN: %d,EP OUT: %d\n", ut->ep_in, ut->ep_out);
    pipe = usb_sndbulkpipe(ut->pusb_dev, 6);
    result = usb_bulk_msg(ut->pusb_dev, pipe, buffer, size, &actual_len, UT_BULK_TIMEOUT);
    
    if(result != 0)
    {
        LOG_ERROR " transport erroe = 0x%x \n", result LOG_END
    }
    LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    return result;
}

static void
usb_mod_CBI_AsyncCallback(
    urb_t* urb)
{
    USB_CALLBACK_CONTEXT* pCallbackContext = 0;

    //if (gUsbCacheCount[0]++ > 150)
    //{
    //    printf("URB callback!!\n");
    //    gUsbCacheCount[0] = 0;
    //}
    if (urb)
    {
        LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END
        
        pCallbackContext = (USB_CALLBACK_CONTEXT*) urb->context;
        if (pCallbackContext->pfSelfCallback)
        {
            if (gUsbCacheCount[1]++ > 100)
            {
                printf("updatebuf callback idx %d!!\n", gUsbCacheCount[1]);
                gUsbCacheCount[1] = 0;
            }
            //else
            //    printf("updatebuf callback idx %d!!\n", gUsbCacheCount[1]);
            (pCallbackContext->pfSelfCallback)((void*)pCallbackContext);
        }
        usb_free_urb(urb);
        LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    }
}


uint32_t usb_mod_CBI_AsyncReceiveData(struct usbtuner_data* ut, uint8_t demod_addr, uint8_t* buffer, uint32_t size, USB_CALLBACK_CONTEXT* pCallbackContext)
{
    int result = 0;
    uint32_t pipe = 0;
    int actual_len = 0, ep_data;
    urb_t *urb;
    //printf("DataRcv >>> EP IN: %d,EP OUT: %d\n", ut->ep_in, ut->ep_out);

    LOG_INFO "[enter] %s()\n", __FUNCTION__ LOG_END

    if (demod_addr == 0x01)
        ep_data = 5;
    else 
        ep_data = 4;
        
    pipe = usb_rcvbulkpipe(ut->pusb_dev, 4);

    if(size < 0)
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

    FILL_BULK_URB(urb, ut->pusb_dev, pipe, buffer, size, usb_mod_CBI_AsyncCallback, pCallbackContext);
    result = usb_submit_urb(urb);
end:
    if(result < 0)
        LOG_ERROR " usb_bulk_msg() return error code 0x%08X \n", (-result) LOG_END
    
    LOG_INFO "[leave] %s()\n", __FUNCTION__ LOG_END
    return result;
}
