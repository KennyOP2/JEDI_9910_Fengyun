/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * SMedia USB HCD Driver API header file.
 *
 * @author Irene Lin
 */

#ifndef USB_EX_H
#define USB_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__FREERTOS__) || defined(__OPENRTOS__)
#define USB_PACKED      __attribute__ ((__packed__))
#else
#define USB_PACKED
#endif

#include "mmp_usbex.h"
#include "list.h"
#include "../usb_port.h"

//=============================================================================
//                              USB Constants
//=============================================================================

/* USB constants */
    
/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			    1
#define USB_CLASS_COMM			    2
#define USB_CLASS_HID			    3
#define USB_CLASS_PHYSICAL		    5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		    7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			    9
#define USB_CLASS_CDC_DATA		    0x0a
#define USB_CLASS_CSCID		        0x0b /* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d /* content security */
#define USB_CLASS_APP_SPEC		    0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

/*
 * USB types
 */
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03

/*
 * USB directions
 */
#define USB_DIR_OUT			0
#define USB_DIR_IN			0x80

/*
 * Descriptor types
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05

#define USB_DT_HID			    (USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT			(USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL			(USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB			    (USB_TYPE_CLASS | 0x09)

/*
 * Descriptor sizes per descriptor type
 */
#define USB_DT_DEVICE_SIZE		    18
#define USB_DT_CONFIG_SIZE		    9
#define USB_DT_INTERFACE_SIZE		9
#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE		7
#define USB_DT_HID_SIZE			    9

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3

/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_UNDEF_0                        0xf0
#define USB_PID_OUT                            0xe1
#define USB_PID_ACK                            0xd2
#define USB_PID_DATA0                          0xc3
#define USB_PID_PING                           0xb4	/* USB 2.0 */
#define USB_PID_SOF                            0xa5
#define USB_PID_NYET                           0x96	/* USB 2.0 */
#define USB_PID_DATA2                          0x87	/* USB 2.0 */
#define USB_PID_SPLIT                          0x78	/* USB 2.0 */
#define USB_PID_IN                             0x69
#define USB_PID_NAK                            0x5a
#define USB_PID_DATA1                          0x4b
#define USB_PID_PREAMBLE                       0x3c	/* Token mode */
#define USB_PID_ERR                            0x3c	/* USB 2.0: handshake mode */
#define USB_PID_SETUP                          0x2d
#define USB_PID_STALL                          0x1e
#define USB_PID_MDATA                          0x0f	/* USB 2.0 */

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS		    0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		    0x03
#define USB_REQ_SET_ADDRESS		    0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		    0x0C

/*
 * HID requests
 */
#define USB_REQ_GET_REPORT		    0x01
#define USB_REQ_GET_IDLE		    0x02
#define USB_REQ_GET_PROTOCOL		0x03
#define USB_REQ_SET_REPORT		    0x09
#define USB_REQ_SET_IDLE		    0x0A
#define USB_REQ_SET_PROTOCOL		0x0B



//=============================================================================
//                              USB Descriptor
//=============================================================================
typedef struct USB_PACKED {
    MMP_UINT8  requesttype;
    MMP_UINT8  request;
    MMP_UINT16 value;
    MMP_UINT16 index;
    MMP_UINT16 length;
} devrequest;


/*
 * This is a USB device descriptor.
 *
 * USB device information
 */

/* Everything but the endpoint maximums are aribtrary */
#define USB_MAXCONFIG		    8
#define USB_ALTSETTINGALLOC     4
#define USB_MAXALTSETTING	    128  /* Hard limit */
#define USB_MAXINTERFACES	    32
#define USB_MAXENDPOINTS	    32

/* All standard descriptors have these 2 fields in common */
struct usb_descriptor_header {
    MMP_UINT8  bLength;
    MMP_UINT8  bDescriptorType;
} USB_PACKED;

/* Device descriptor */
struct usb_device_descriptor {
    MMP_UINT8  bLength;
    MMP_UINT8  bDescriptorType;
    MMP_UINT16 bcdUSB;
    MMP_UINT8  bDeviceClass;
    MMP_UINT8  bDeviceSubClass;
    MMP_UINT8  bDeviceProtocol;
    MMP_UINT8  bMaxPacketSize0;
    MMP_UINT16 idVendor;
    MMP_UINT16 idProduct;
    MMP_UINT16 bcdDevice;
    MMP_UINT8  iManufacturer;
    MMP_UINT8  iProduct;
    MMP_UINT8  iSerialNumber;
    MMP_UINT8  bNumConfigurations;
} USB_PACKED;

/* Endpoint descriptor */
struct usb_endpoint_descriptor {
    MMP_UINT8  bLength;
    MMP_UINT8  bDescriptorType;
    MMP_UINT8  bEndpointAddress;
    MMP_UINT8  bmAttributes;
    MMP_UINT16 wMaxPacketSize;
    MMP_UINT8  bInterval;
    MMP_UINT8  bRefresh;
    MMP_UINT8  bSynchAddress;

    MMP_UINT8* extra;   /* Extra descriptors */
    MMP_INT    extralen;
} ;

/* Interface descriptor */
struct usb_interface_descriptor {
    MMP_UINT8  bLength;
    MMP_UINT8  bDescriptorType;
    MMP_UINT8  bInterfaceNumber;
    MMP_UINT8  bAlternateSetting;
    MMP_UINT8  bNumEndpoints;
    MMP_UINT8  bInterfaceClass;
    MMP_UINT8  bInterfaceSubClass;
    MMP_UINT8  bInterfaceProtocol;
    MMP_UINT8  iInterface;

    struct usb_endpoint_descriptor *endpoint;

    MMP_UINT8* extra;   /* Extra descriptors */
    MMP_INT    extralen;
};

struct usb_interface {
    struct usb_interface_descriptor *altsetting;

    MMP_INT act_altsetting;		/* active alternate setting */
    MMP_INT num_altsetting;		/* number of alternate settings */
    MMP_INT max_altsetting;     /* total memory allocated */
 
    struct usb_driver *driver;	/* driver */
    void*  private_data;
};

/* Configuration descriptor information.. */
struct usb_config_descriptor {
    MMP_UINT8  bLength;
    MMP_UINT8  bDescriptorType;
    MMP_UINT16 wTotalLength;
    MMP_UINT8  bNumInterfaces;
    MMP_UINT8  bConfigurationValue;
    MMP_UINT8  iConfiguration;
    MMP_UINT8  bmAttributes;
    MMP_UINT8  MaxPower;

    struct usb_interface* usb_interface;

    MMP_UINT8* extra;   /* Extra descriptors */
    MMP_INT    extralen;
};

/* String descriptor */
struct usb_string_descriptor {
    MMP_UINT8  bLength;
    MMP_UINT8  bDescriptorType;
    MMP_UINT16 wData[1];
} USB_PACKED;



//=============================================================================
//                              USB Driver Related
//=============================================================================
struct usb_device;

/*
 * Device table entry for "new style" table-driven USB drivers.
 * User mode code can read these tables to choose which modules to load.
 * Declare the table as __devinitdata, and as a MODULE_DEVICE_TABLE.
 *
 * With a device table provide bind() instead of probe().  Then the
 * third bind() parameter will point to a matching entry from this
 * table.  (Null value reserved.)
 * 
 * Terminate the driver's table with an all-zeroes entry.
 * Init the fields you care about; zeroes are not used in comparisons.
 */
#define USB_DEVICE_ID_MATCH_VENDOR		    0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		    0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO		    0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI		    0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200

#define USB_DEVICE_ID_MATCH_DEVICE		        (USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)
#define USB_DEVICE_ID_MATCH_DEV_RANGE		    (USB_DEVICE_ID_MATCH_DEV_LO | USB_DEVICE_ID_MATCH_DEV_HI)
#define USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION	(USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_DEV_RANGE)
#define USB_DEVICE_ID_MATCH_DEV_INFO \
    (USB_DEVICE_ID_MATCH_DEV_CLASS | USB_DEVICE_ID_MATCH_DEV_SUBCLASS | USB_DEVICE_ID_MATCH_DEV_PROTOCOL)
#define USB_DEVICE_ID_MATCH_INT_INFO \
    (USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS | USB_DEVICE_ID_MATCH_INT_PROTOCOL)
#define USB_INTERFACE_CLASS(cl) \
    USB_DEVICE_ID_MATCH_INT_CLASS, 0,0,0,0,0,0,0,cl,0,0

/* Some useful macros */
#define USB_DEVICE(vend,prod) \
    USB_DEVICE_ID_MATCH_DEVICE, vend, prod, 0,0,0,0,0,0,0,0
#define USB_DEVICE_INFO(cl,sc,pr) \
    USB_DEVICE_ID_MATCH_DEV_INFO, 0,0,0,0,cl,sc,pr,0,0,0
#define USB_INTERFACE_INFO(cl,sc,pr) \
    USB_DEVICE_ID_MATCH_INT_INFO, 0,0,0,0,0,0,0,cl,sc,pr

struct usb_device_id {
    /* This bitmask is used to determine which of the following fields
     * are to be used for matching.
     */
    MMP_UINT16		match_flags;

    /*
     * vendor/product codes are checked, if vendor is nonzero
     * Range is for device revision (bcdDevice), inclusive;
     * zero values here mean range isn't considered
     */
    MMP_UINT16		idVendor;
    MMP_UINT16		idProduct;
    MMP_UINT16		bcdDevice_lo, bcdDevice_hi;

    /*
     * if device class != 0, these can be match criteria;
     * but only if this bDeviceClass value is nonzero
     */
    MMP_UINT8		bDeviceClass;
    MMP_UINT8		bDeviceSubClass;
    MMP_UINT8		bDeviceProtocol;

    /*
     * if interface class != 0, these can be match criteria;
     * but only if this bInterfaceClass value is nonzero
     */
    MMP_UINT8		bInterfaceClass;
    MMP_UINT8		bInterfaceSubClass;
    MMP_UINT8		bInterfaceProtocol;
};

#define USB_DRIVER_FLAG_OPEN_BY_HCD		    0x0001

struct usb_driver {
    const char *name;
    MMP_UINT32 flags;
    MMP_MUTEX  serialize;

    void *(*probe)(
        struct usb_device *dev,		/* the device */
        MMP_UINT intf,			/* what interface */
        const struct usb_device_id *id	/* from id_table */
        );
    MMP_INT (*open)(void *);
    void (*disconnect)(struct usb_device *, void *);

    /* ioctl -- userspace apps can talk to drivers through usbdevfs */
    MMP_INT (*ioctl)(struct usb_device *dev, MMP_UINT32 code, void *buf);

    /* support for "new-style" USB hotplugging
     * binding policy can be driven from user mode too
     */
    const struct usb_device_id *id_table;

    struct list_head driver_list;
};



//=============================================================================
//                     USB Request Packet(URB) Related
//=============================================================================
/*
 * urb->transfer_flags:
 */
#define USB_DISABLE_SPD         0x0001
#define USB_ISO_ASAP            0x0002
#define USB_ASYNC_UNLINK        0x0008
#define USB_QUEUE_BULK          0x0010
#define USB_NO_FSBR		        0x0020
#define USB_ZERO_PACKET         0x0040  // Finish bulk OUTs always with zero length packet
#define URB_NO_INTERRUPT	    0x0080	/* HINT: no non-error interrupt needed */
                    /* ... less overhead for QUEUE_BULK */
#define USB_TIMEOUT_KILLED	    0x1000	// only set by HCD!
#if defined(USB_LOGO_TEST)
#define USB_SINGLE_STEP_0	    0x10000000	// only for HOST logo use
#define USB_SINGLE_STEP_1	    0x20000000	// only for HOST logo use
#define USB_SINGLE_STEP_2	    0x40000000	// only for HOST logo use
#endif

typedef struct
{
    MMP_UINT32 offset;
    MMP_UINT32 length;		// expected length
    MMP_UINT32 actual_length;
    MMP_UINT32 status;
} iso_packet_descriptor_t, *piso_packet_descriptor_t;

struct urb;
typedef void (*usb_complete_t)(struct urb *);

typedef struct urb
{
#define URB_BUF_COOKIE      0x541123 
    MMP_UINT32          cookies; 
    MMP_INT             type;
    _spinlock_t         lock;		    // lock for the URB
    void*               hcpriv;	        // private data for host controller
    struct list_head    urb_list;	    // list pointer to all active urbs 
    struct urb*         next;		    // pointer to next URB	
    struct usb_device*  dev;		    // pointer to associated USB device
    MMP_UINT32          pipe;	   	    // pipe information
    MMP_INT             status;		    // returned status
    MMP_UINT32          transfer_flags;	        // USB_DISABLE_SPD | USB_ISO_ASAP | etc.
    void*               transfer_buffer;		// associated data buffer
    MMP_INT             transfer_buffer_length;	// data buffer length
    MMP_INT             actual_length;          // actual data buffer length	
    MMP_INT             bandwidth;			    // bandwidth for this transfer request (INT or ISO)
    MMP_UINT8*          setup_packet;	        // setup packet (control only)
    //
    MMP_INT             start_frame;		// start frame (iso/irq only)
    MMP_INT             number_of_packets;	// number of packets in this request (iso)
    MMP_INT             interval;           // polling interval (irq only)
    MMP_INT             error_count;		// number of errors in this transfer (iso only)
    MMP_INT             timeout;			// timeout (in jiffies)
    //
    void*               context;			// context for completion routine
    usb_complete_t      complete;	        // pointer to completion routine
    MMP_UINT32          reserved;
    //
#if 0 // ISO TODO 
    iso_packet_descriptor_t iso_frame_desc[1];
#endif
} urb_t, *purb_t;

/**
 * FILL_CONTROL_URB - macro to help initialize a control urb
 * @URB: pointer to the urb to initialize.
 * @DEV: pointer to the struct usb_device for this urb.
 * @PIPE: the endpoint pipe
 * @SETUP_PACKET: pointer to the setup_packet buffer
 * @TRANSFER_BUFFER: pointer to the transfer buffer
 * @BUFFER_LENGTH: length of the transfer buffer
 * @COMPLETE: pointer to the usb_complete_t function
 * @CONTEXT: what to set the urb context to.
 *
 * Initializes a control urb with the proper information needed to submit
 * it to a device.  This macro is depreciated, the usb_fill_control_urb()
 * function should be used instead.
 */
#define FILL_CONTROL_URB(URB,DEV,PIPE,SETUP_PACKET,TRANSFER_BUFFER,BUFFER_LENGTH,COMPLETE,CONTEXT) \
    do {\
    (URB)->dev=DEV;\
    (URB)->pipe=PIPE;\
    (URB)->setup_packet=SETUP_PACKET;\
    (URB)->transfer_buffer=TRANSFER_BUFFER;\
    (URB)->transfer_buffer_length=BUFFER_LENGTH;\
    (URB)->complete=COMPLETE;\
    (URB)->context=CONTEXT;\
    } while (0)

/**
 * FILL_BULK_URB - macro to help initialize a bulk urb
 * @URB: pointer to the urb to initialize.
 * @DEV: pointer to the struct usb_device for this urb.
 * @PIPE: the endpoint pipe
 * @TRANSFER_BUFFER: pointer to the transfer buffer
 * @BUFFER_LENGTH: length of the transfer buffer
 * @COMPLETE: pointer to the usb_complete_t function
 * @CONTEXT: what to set the urb context to.
 *
 * Initializes a bulk urb with the proper information needed to submit it
 * to a device.  This macro is depreciated, the usb_fill_bulk_urb()
 * function should be used instead.
 */
#define FILL_BULK_URB(URB,DEV,PIPE,TRANSFER_BUFFER,BUFFER_LENGTH,COMPLETE,CONTEXT) \
    do {\
    (URB)->dev=DEV;\
    (URB)->pipe=PIPE;\
    (URB)->transfer_buffer=TRANSFER_BUFFER;\
    (URB)->transfer_buffer_length=BUFFER_LENGTH;\
    (URB)->complete=COMPLETE;\
    (URB)->context=CONTEXT;\
    } while (0)
    
/**
 * FILL_INT_URB - macro to help initialize a interrupt urb
 * @URB: pointer to the urb to initialize.
 * @DEV: pointer to the struct usb_device for this urb.
 * @PIPE: the endpoint pipe
 * @TRANSFER_BUFFER: pointer to the transfer buffer
 * @BUFFER_LENGTH: length of the transfer buffer
 * @COMPLETE: pointer to the usb_complete_t function
 * @CONTEXT: what to set the urb context to.
 * @INTERVAL: what to set the urb interval to.
 *
 * Initializes a interrupt urb with the proper information needed to submit
 * it to a device.  This macro is depreciated, the usb_fill_int_urb()
 * function should be used instead.
 */
#define FILL_INT_URB(URB,DEV,PIPE,TRANSFER_BUFFER,BUFFER_LENGTH,COMPLETE,CONTEXT,INTERVAL) \
    do {\
    (URB)->dev=DEV;\
    (URB)->pipe=PIPE;\
    (URB)->transfer_buffer=TRANSFER_BUFFER;\
    (URB)->transfer_buffer_length=BUFFER_LENGTH;\
    (URB)->complete=COMPLETE;\
    (URB)->context=CONTEXT;\
    (URB)->interval=INTERVAL;\
    (URB)->start_frame=-1;\
    } while (0)

#define FILL_CONTROL_URB_TO(a,aa,b,c,d,e,f,g,h) \
    do {\
    (a)->dev=aa;\
    (a)->pipe=b;\
    (a)->setup_packet=c;\
    (a)->transfer_buffer=d;\
    (a)->transfer_buffer_length=e;\
    (a)->complete=f;\
    (a)->context=g;\
    (a)->timeout=h;\
    } while (0)

#define FILL_BULK_URB_TO(a,aa,b,c,d,e,f,g) \
    do {\
    (a)->dev=aa;\
    (a)->pipe=b;\
    (a)->transfer_buffer=c;\
    (a)->transfer_buffer_length=d;\
    (a)->complete=e;\
    (a)->context=f;\
    (a)->timeout=g;\
    } while (0)
 
/**
 * usb_fill_control_urb - initializes a control urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @setup_packet: pointer to the setup_packet buffer
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 *
 * Initializes a control urb with the proper information needed to submit
 * it to a device.
 */
static MMP_INLINE void usb_fill_control_urb (struct urb *urb,
                     struct usb_device *dev,
                     MMP_UINT32 pipe,
                     MMP_UINT8* setup_packet,
                     MMP_UINT8* transfer_buffer,
                     MMP_INT buffer_length,
                     usb_complete_t complete,
                     void* context)
{
    urb->dev = dev;
    urb->pipe = pipe;
    urb->setup_packet = setup_packet;
    urb->transfer_buffer = transfer_buffer;
    urb->transfer_buffer_length = buffer_length;
    urb->complete = complete;
    urb->context = context;
}

/**
 * usb_fill_bulk_urb - macro to help initialize a bulk urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 *
 * Initializes a bulk urb with the proper information needed to submit it
 * to a device.
 */
static MMP_INLINE void usb_fill_bulk_urb (struct urb *urb,
                      struct usb_device *dev,
                      MMP_UINT32 pipe,
                      void *transfer_buffer,
                      MMP_INT buffer_length,
                      usb_complete_t complete,
                      void *context)
                      
{
    urb->dev = dev;
    urb->pipe = pipe;
    urb->transfer_buffer = transfer_buffer;
    urb->transfer_buffer_length = buffer_length;
    urb->complete = complete;
    urb->context = context;
}
    
/**
 * usb_fill_int_urb - macro to help initialize a interrupt urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 * @interval: what to set the urb interval to.
 *
 * Initializes a interrupt urb with the proper information needed to submit
 * it to a device.
 */
static MMP_INLINE void usb_fill_int_urb (struct urb *urb,
                     struct usb_device *dev,
                     MMP_UINT32 pipe,
                     void *transfer_buffer,
                     MMP_INT buffer_length,
                     usb_complete_t complete,
                     void *context,
                     MMP_INT interval)
{
    urb->dev = dev;
    urb->pipe = pipe;
    urb->transfer_buffer = transfer_buffer;
    urb->transfer_buffer_length = buffer_length;
    urb->complete = complete;
    urb->context = context;
    urb->interval = interval;
    urb->start_frame = -1;
}

USBEX_API MMP_INT usb_register(struct usb_driver *new_driver);
USBEX_API const struct usb_device_id* usb_match_id(struct usb_device *dev, 
                                                   struct usb_interface *usb_interface,
                                                   const struct usb_device_id *id);

USBEX_API purb_t usb_alloc_urb(MMP_INT iso_packets);
USBEX_API void usb_free_urb (purb_t purb);
USBEX_API MMP_INT usb_submit_urb(purb_t purb);
USBEX_API MMP_INT usb_unlink_urb(purb_t purb);
USBEX_API MMP_INT usb_internal_control_msg(struct usb_device *usb_dev, MMP_UINT32 pipe, devrequest *cmd,  void *data, MMP_INT len, MMP_INT timeout);
USBEX_API MMP_INT usb_bulk_msg(struct usb_device *usb_dev, MMP_UINT32 pipe, void *data, MMP_INT len, MMP_INT *actual_length, MMP_INT timeout);
USBEX_API MMP_INT usb_bulk_busy_msg(struct usb_device *usb_dev, MMP_UINT32 pipe, void *data, MMP_INT len, MMP_INT *actual_length, MMP_INT timeout);
USBEX_API MMP_INT usb_control_msg(
                        struct usb_device* dev, 
                        MMP_UINT32 pipe, 
                        MMP_UINT8  request, 
                        MMP_UINT8  requesttype,
                        MMP_UINT16 value, 
                        MMP_UINT16 index, 
                        void*      data, 
                        MMP_UINT16 size, 
                        MMP_INT    timeout);
USBEX_API MMP_INT usb_dev_exist(struct usb_device* usb_dev);

//=============================================================================
//                     USB Bus Related
//=============================================================================

struct usb_operations {
    MMP_INT (*allocate)(struct usb_device *);
    MMP_INT (*deallocate)(struct usb_device *);
    MMP_INT (*get_frame_number) (struct usb_device *usb_dev);
    MMP_INT (*submit_urb) (struct urb* purb);
    MMP_INT (*unlink_urb) (struct urb* purb);
    MMP_INT (*dev_exist)(struct usb_device *);
};

/*
 * Allocated per bus we have
 */
struct usb_bus {
    MMP_INT             busnum;			/* Bus number (in order of reg) */

    struct usb_operations *op;          /* Operations (specific to the HC) */
    struct usb_device*  root_device;    /* Root device */
    void*               hcpriv;         /* Host Controller private data */
    MMP_UINT32          devmap;           /* Device map */

    MMP_INT             bandwidth_allocated;  /* on this Host Controller; */
                                              /* applies to Int. and Isoc. pipes; */
                                              /* measured in microseconds/frame; */
                                              /* range is 0..900, where 900 = */
                                              /* 90% of a 1-millisecond frame */
    MMP_INT             bandwidth_int_reqs;	  /* number of Interrupt requesters */
    MMP_INT             bandwidth_isoc_reqs;  /* number of Isoc. requesters */
};

struct usb_bus *usb_alloc_bus(struct usb_operations *);
void usb_free_bus(struct usb_bus *);
struct usb_device* usb_alloc_dev(struct usb_device* parent, struct usb_bus* bus);
void usb_free_dev(struct usb_device* dev);
void usb_connect(struct usb_device* dev);
void usb_disconnect(struct usb_device **pdev);


//=============================================================================
//                     USB Device Related
//=============================================================================
/*
 * As of USB 2.0, full/low speed devices are segregated into trees.
 * One type grows from USB 1.1 host controllers (OHCI, UHCI etc).
 * The other type grows from high speed hubs when they connect to
 * full/low speed devices using "Transaction Translators" (TTs).
 *
 * TTs should only be known to the hub driver, and high speed bus
 * drivers (only EHCI for now).  They affect periodic scheduling and
 * sometimes control/bulk error recovery.
 */
struct usb_tt {
    struct usb_device	*hub;	/* upstream highspeed hub */
    int			multi;	/* true means one TT per port */
};

/* This is arbitrary.
 * From USB 2.0 spec Table 11-13, offset 7, a hub can
 * have up to 255 ports. The most yet reported is 10.
 */
#define USB_MAXCHILDREN		(4)

struct usb_device {
    MMP_INT         devnum;			/* Device number on USB bus */
    MMP_INT         type;

    enum {
        USB_SPEED_UNKNOWN = 0,			/* enumerating */
        USB_SPEED_LOW, USB_SPEED_FULL,	/* usb 1.1 */
        USB_SPEED_HIGH				    /* usb 2.0 */
    } speed;

    struct usb_tt*	tt; 		/* low/full speed dev, highspeed hub */
    MMP_INT		    ttport;		/* device port on that tt hub */

    MMP_MUTEX       serialize;

    MMP_UINT32  toggle[2];		/* one bit for each endpoint ([0] = IN, [1] = OUT) */
    MMP_UINT32  halted[2];		/* endpoint halts; one bit per endpoint # & direction; */
                                /* [0] = IN, [1] = OUT */
    MMP_INT     epmaxpacketin[16];		/* INput endpoint specific maximums */
    MMP_INT     epmaxpacketout[16];		/* OUTput endpoint specific maximums */

    struct usb_device*  parent;
    struct usb_bus*     bus;		/* Bus we're part of */

    struct usb_device_descriptor  descriptor;   /* Descriptor */
    struct usb_config_descriptor* config;	    /* All of the configs */
    struct usb_config_descriptor* actconfig;    /* the active configuration */

    MMP_UINT8** rawdescriptors;		/* Raw descriptors for each config */

    MMP_INT     have_langid;		/* whether string_langid is valid yet */
    MMP_INT     string_langid;		/* language ID for strings */
  
    void*       hcpriv;			/* Host Controller private data */
    
    /*
     * Child devices - these can be either new devices
     * (if this is a hub device), or different instances
     * of this same device.
     *
     * Each instance needs its own set of data structures.
     */
    MMP_INT maxchild;			/* Number of ports if hub */
    struct usb_device* children[USB_MAXCHILDREN];

    USB_DEVICE_INFO device_info[5];
    MMP_INT         driverNum;
};


//=============================================================================
//                     Pipe Related
//=============================================================================
/*
 * Calling this entity a "pipe" is glorifying it. A USB pipe
 * is something embarrassingly simple: it basically consists
 * of the following information:
 *  - device number (7 bits)
 *  - endpoint number (4 bits)
 *  - current Data0/1 state (1 bit)
 *  - direction (1 bit)
 *  - speed (1 bit)
 *  - pipe type (2 bits: control, interrupt, bulk, isochronous)
 *
 * That's 18 bits. Really. Nothing more. And the USB people have
 * documented these eighteen bits as some kind of glorious
 * virtual data structure.
 *
 * Let's not fall in that trap. We'll just encode it as a simple
 * unsigned int. The encoding is:
 *
 *  - direction:	bit 7		(0 = Host-to-Device [Out], 1 = Device-to-Host [In])
 *  - device:		bits 8-14
 *  - endpoint:		bits 15-18
 *  - Data0/1:		bit 19
 *  - speed:		bit 26		(0 = Full, 1 = Low Speed)
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt, 10 = control, 11 = bulk)
 *
 * Why? Because it's arbitrary, and whatever encoding we select is really
 * up to us. This one happens to share a lot of bit positions with the UHCI
 * specification, so that much of the uhci driver can just mask the bits
 * appropriately.
 *
 * NOTE:  there's no encoding (yet?) for a "high speed" endpoint; treat them
 * like full speed devices.
 */

#define PIPE_ISOCHRONOUS		0
#define PIPE_INTERRUPT			1
#define PIPE_CONTROL			2
#define PIPE_BULK			    3

#define usb_maxpacket(dev, pipe, out)	(out \
                ? (dev)->epmaxpacketout[usb_pipeendpoint(pipe)] \
                : (dev)->epmaxpacketin [usb_pipeendpoint(pipe)] )
#define usb_packetid(pipe)	(((pipe) & USB_DIR_IN) ? USB_PID_IN : USB_PID_OUT)

#define usb_pipeout(pipe)	    ((((pipe) >> 7) & 1) ^ 1)
#define usb_pipein(pipe)	    (((pipe) >> 7) & 1)
#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipe_endpdev(pipe)	(((pipe) >> 8) & 0x7ff)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)
#define usb_pipedata(pipe)	    (((pipe) >> 19) & 1)
#define usb_pipeslow(pipe)	    (((pipe) >> 26) & 1)
#define usb_pipetype(pipe)	    (((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	    (usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	    (usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	    (usb_pipetype((pipe)) == PIPE_BULK)

#define PIPE_DEVEP_MASK		0x0007ff00

/* The D0/D1 toggle bits */
#define usb_gettoggle(dev, ep, out)         (((dev)->toggle[out] >> ep) & 1)
#define	usb_dotoggle(dev, ep, out)          ((dev)->toggle[out] ^= (1 << ep))
#define usb_settoggle(dev, ep, out, bit)    ((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << ep)) | ((bit) << ep))

/* Endpoint halt control/status */
#define usb_endpoint_out(ep_dir)	        (((ep_dir >> 7) & 1) ^ 1)
#define usb_endpoint_halt(dev, ep, out)     ((dev)->halted[out] |= (1 << (ep)))
#define usb_endpoint_running(dev, ep, out)  ((dev)->halted[out] &= ~(1 << (ep)))
#define usb_endpoint_halted(dev, ep, out)   ((dev)->halted[out] & (1 << (ep)))

static MMP_INLINE MMP_UINT32 __create_pipe(struct usb_device *dev, MMP_UINT32 endpoint)
{
    return (dev->devnum << 8) | (endpoint << 15) | ((dev->speed == USB_SPEED_LOW) << 26);
}

static MMP_INLINE MMP_UINT32 __default_pipe(struct usb_device *dev)
{
    return ((dev->speed == USB_SPEED_LOW) << 26);
}

/* Create various pipes... */
#define usb_sndctrlpipe(dev,endpoint)	((PIPE_CONTROL << 30) | __create_pipe(dev,endpoint))
#define usb_rcvctrlpipe(dev,endpoint)	((PIPE_CONTROL << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndisocpipe(dev,endpoint)	((PIPE_ISOCHRONOUS << 30) | __create_pipe(dev,endpoint))
#define usb_rcvisocpipe(dev,endpoint)	((PIPE_ISOCHRONOUS << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev,endpoint)	((PIPE_BULK << 30) | __create_pipe(dev,endpoint))
#define usb_rcvbulkpipe(dev,endpoint)	((PIPE_BULK << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev,endpoint)	((PIPE_INTERRUPT << 30) | __create_pipe(dev,endpoint))
#define usb_rcvintpipe(dev,endpoint)	((PIPE_INTERRUPT << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_snddefctrl(dev)		((PIPE_CONTROL << 30) | __default_pipe(dev))
#define usb_rcvdefctrl(dev)		((PIPE_CONTROL << 30) | __default_pipe(dev) | USB_DIR_IN)


/*
 * Send and receive control messages..
 */
MMP_INT usb_new_device(struct usb_device *dev);
USBEX_API MMP_INT usb_string(struct usb_device *dev, MMP_INT index, MMP_UINT8 *buf, MMP_INT size);
USBEX_API MMP_INT usb_clear_halt(struct usb_device *dev, MMP_INT pipe);
/** 
 * HID Class-Specific Request 
 */
USBEX_API MMP_INT usb_get_report(struct usb_device *dev, MMP_INT ifnum, MMP_UINT8 type, MMP_UINT8 id, void* buf, MMP_INT size);
USBEX_API MMP_INT usb_set_protocol(struct usb_device* dev, MMP_INT ifnum, MMP_INT protocol);
USBEX_API MMP_INT usb_set_idle(struct usb_device *dev, MMP_INT ifnum, MMP_INT duration, MMP_INT report_id);




#ifdef __cplusplus
}
#endif

#endif 
