/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * USB Host Controller Driver (usb_hcd) framework.
 *
 * @author Irene Lin
 */
#ifndef USB_EX_HCD_H
#define USB_EX_HCD_H
/*-------------------------------------------------------------------------*/
/*
 * USB Host Controller Driver (usb_hcd) framework
 *
 * Since "struct usb_bus" is so thin, you can't share much code in it.
 * This framework is a layer over that, and should be more sharable.
 */
/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include "usb/usb/list.h"


//=============================================================================
//                              usb_hcd struct
//=============================================================================
struct usb_hcd {	/* usb_bus.hcpriv points to this */
    /*
     * housekeeping
     */
    struct usb_bus*		bus;		/* hcd is-a bus */

    //struct timer_list	rh_timer;	/* drives root hub */
    //struct list_head	dev_list;	/* devices on this bus */

    /*
     * hardware info/state
     */
    struct hc_driver*	driver;	/* hw-specific hooks */
    MMP_UINT32          iobase;

    MMP_INT			    state;
#define	__ACTIVE		0x01
#define	__SLEEPY		0x02
#define	__SUSPEND		0x04
#define	__TRANSIENT		0x80

#define	USB_STATE_HALT		0
#define	USB_STATE_RUNNING	(__ACTIVE)
#define	USB_STATE_READY		(__ACTIVE|__SLEEPY)
#define	USB_STATE_QUIESCING	(__SUSPEND|__TRANSIENT|__ACTIVE)
#define	USB_STATE_RESUMING	(__SUSPEND|__TRANSIENT)
#define	USB_STATE_SUSPENDED	(__SUSPEND)

#define	HCD_IS_RUNNING(state) ((state) & __ACTIVE)
#define	HCD_IS_SUSPENDED(state) ((state) & __SUSPEND)

    MMP_UINT32          connect;
    _spinlock_t         hcd_data_lock;
    MMP_UINT32	        index;
};

struct hcd_dev {	/* usb_device.hcpriv points to this */
    //struct list_head	dev_list;	/* on this hcd */
    struct list_head	urb_list;	/* pending on this dev */

    /* per-configuration HC/HCD state, such as QH or ED */
    void			*ep[32];
};



//=============================================================================
//                              hc_driver struct
//=============================================================================

/* each driver provides one of these, and hardware init support */

#define USB_HAS_DEVICE_MODE     0x80000000

struct hc_driver {
    MMP_INT 	flags;
#define	HCD_USB11	0x0010		/* USB 1.1 */
#define	HCD_USB2	0x0020		/* USB 2.0 */

    /* called to init HCD and root hub */
    MMP_INT	(*start) (struct usb_hcd *hcd);

    #if defined(USB_PM)
    /* called after all devices were suspended */
    MMP_INT	(*suspend) (struct usb_hcd *hcd, MMP_UINT32 state);

    /* called before any devices get resumed */
    MMP_INT	(*resume) (struct usb_hcd *hcd);
    #endif
    /* cleanly make HCD stop writing memory and doing I/O */
    MMP_INT	(*stop) (struct usb_hcd *hcd);

    /* return current frame number */
    MMP_INT	(*get_frame_number) (struct usb_hcd *hcd);

    /* memory lifecycle */
    struct usb_hcd*	(*hcd_alloc) (void);
    void		(*hcd_free) (struct usb_hcd *hcd);

    /* manage i/o requests, device state */
    MMP_INT	(*urb_enqueue) (struct usb_hcd *hcd, struct urb *urb);
    MMP_INT	(*urb_dequeue) (struct usb_hcd *hcd, struct urb *urb);

    // frees configuration resources -- allocated as needed during
    // urb_enqueue, and not freed by urb_dequeue
    void	(*free_config) (struct usb_hcd *hcd, struct usb_device *dev);
    MMP_INT	(*dev_exist) (struct usb_hcd *hcd);

    #if defined(USB_ROOT_HUB)
    /* root hub support */
    MMP_INT		(*hub_status_data) (struct usb_hcd *hcd, char *buf);
    MMP_INT		(*hub_control) (struct usb_hcd *hcd,
                MMP_UINT16 typeReq, MMP_UINT16 wValue, MMP_UINT16 wIndex,
                char *buf, MMP_UINT16 wLength);
    #endif
};


/*-------------------------------------------------------------------------*/

/**
 * Generic bandwidth allocation constants/support
 */
#define FRAME_TIME_USECS	1000L
#define BitTime(bytecount)  (7 * 8 * bytecount / 6)  /* with integer truncation */
		/* Trying not to use worst-case bit-stuffing
                   of (7/6 * 8 * bytecount) = 9.33 * bytecount */
		/* bytecount = data payload byte count */

#define NS_TO_US(ns)	((ns + 500L) / 1000L)
			/* convert & round nanoseconds to microseconds */
#if 0
extern void usb_claim_bandwidth (struct usb_device *dev, struct urb *urb,
		int bustime, int isoc);
extern void usb_release_bandwidth (struct usb_device *dev, struct urb *urb,
		int isoc);

/*
 * Full/low speed bandwidth allocation constants/support.
 */
#define BW_HOST_DELAY	1000L		/* nanoseconds */
#define BW_HUB_LS_SETUP	333L		/* nanoseconds */
                        /* 4 full-speed bit times (est.) */

#define FRAME_TIME_BITS         12000L		/* frame = 1 millisecond */
#define FRAME_TIME_MAX_BITS_ALLOC	(90L * FRAME_TIME_BITS / 100L)
#define FRAME_TIME_MAX_USECS_ALLOC	(90L * FRAME_TIME_USECS / 100L)

extern int usb_check_bandwidth (struct usb_device *dev, struct urb *urb);
#endif

/*
 * Ceiling microseconds (typical) for that many bytes at high speed
 * ISO is a bit less, no ACK ... from USB 2.0 spec, 5.11.3 (and needed
 * to preallocate bandwidth)
 */
#define USB2_HOST_DELAY	5	/* nsec, guess */
#define HS_USECS(bytes) NS_TO_US ( ((55 * 8 * 2083)/1000) \
	+ ((2083UL * (3167 + BitTime (bytes)))/1000) \
	+ USB2_HOST_DELAY)
#if 0
#define HS_USECS_ISO(bytes) NS_TO_US ( ((long)(38 * 8 * 2.083)) \
	+ ((2083UL * (3167 + BitTime (bytes)))/1000) \
	+ USB2_HOST_DELAY)

extern long usb_calc_bus_time (int speed, int is_input,
			int isoc, int bytecount);
#endif




extern struct usb_hcd* hcd0;
extern struct usb_hcd* hcd1;

//=============================================================================
//                              Public functions
//=============================================================================
struct ehci_hcd;

MMP_INT ehci_hcd_init(MMP_UINT32 index);
void usb_hcd_giveback_urb(struct usb_hcd *hcd, struct urb *urb);
void ehci_tasklet(struct ehci_hcd* ehci);


#ifdef __cplusplus
}
#endif

#endif // ifndef USB_EX_HCD_H


