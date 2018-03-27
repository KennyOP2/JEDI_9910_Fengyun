/*
 * Copyright (c) 2008 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Definitions used for the EHCI driver.
 *
 * @author Irene Lin
 */
#ifndef	USB_EHCI_HCD_H
#define	USB_EHCI_HCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../hcd.h"
#include "../otg/otg.h"


//===============================================================================
/** 
 * EHCI register interface, corresponds to 
 * Faraday FOTG210 Revision 1.3 specification 
 */
//===============================================================================

/* EHCI Section 2.2 Host Controller Capability Registers */
struct ehci_caps {
	MMP_UINT32		hcc_reg;        /* HC Capability Register - offset 0x0 */

	MMP_UINT32		hcs_params;     /* HCSPARAMS - offset 0x4 */
#define HCS_DEBUG_PORT(p)	(((p)>>20)&0xf)	/* bits 23:20, debug port? */
#define HCS_INDICATOR(p)	((p)&(1 << 16))	/* true: has port indicators */
#define HCS_N_CC(p)		    (((p)>>12)&0xf)	/* bits 15:12, #companion HCs */
#define HCS_N_PCC(p)		(((p)>>8)&0xf)	/* bits 11:8, ports per CC */
#define HCS_PORTROUTED(p)	((p)&(1 << 7))	/* true: port routing */ 
#define HCS_PPC(p)		    ((p)&(1 << 4))	/* true: port power control */ 
#define HCS_N_PORTS(p)		(((p)>>0)&0xf)	/* bits 3:0, ports on HC */

	MMP_UINT32		hcc_params;      /* HCCPARAMS - offset 0x8 */
#define HCC_EXT_CAPS(p)		(((p)>>8)&0xff)	/* for pci extended caps */
#define HCC_ISOC_CACHE(p)   ((p)&(1 << 7))  /* true: can cache isoc frame */
#define HCC_ISOC_THRES(p)   (((p)>>4)&0x7)  /* bits 6:4, uframes cached */
#define HCC_CANPARK(p)		((p)&(1 << 2))  /* true: can park on async qh */
#define HCC_PGM_FRAMELISTLEN(p) ((p)&(1 << 1))  /* true: periodic_size changes*/
#define HCC_64BIT_ADDR(p)       ((p)&(1))       /* true: can use 64-bit addr */
    #if 0
	MMP_UINT32		portroute [2];	 /* nibbles for routing - offset 0xC */
    #endif
};



/* EHCI Section 2.3 Host Controller Operational Registers */
struct ehci_regs {
	/* USBCMD: offset 0x00 */
	MMP_UINT32		command;
/* 23:16 is r/w intr rate, in microframes; default "8" == 1/msec */
#define CMD_PARK	(1<<11)		/* enable "park" on async qh */
#define CMD_PARK_CNT(c)	(((c)>>8)&3)	/* how many transfers to park for */
#define CMD_LRESET	(1<<7)		/* partial reset (no ports, etc) */
#define CMD_IAAD	(1<<6)		/* "doorbell" interrupt async advance */
#define CMD_ASE		(1<<5)		/* async schedule enable */
#define CMD_PSE  	(1<<4)		/* periodic schedule enable */
/* 3:2 is periodic frame list size */
#define CMD_RESET	(1<<1)		/* reset HC not bus */
#define CMD_RUN		(1<<0)		/* start/stop HC */

	/* USBSTS: offset 0x04 */
	MMP_UINT32		status;
#define STS_ASS		(1<<15)		/* Async Schedule Status */
#define STS_PSS		(1<<14)		/* Periodic Schedule Status */
#define STS_RECL	(1<<13)		/* Reclamation */
#define STS_HALT	(1<<12)		/* Not running (any reason) */
/* some bits reserved */
	/* these STS_* flags are also intr_enable bits (USBINTR) */
#define STS_IAA		(1<<5)		/* Interrupted on async advance */
#define STS_FATAL	(1<<4)		/* such as some PCI access errors */
#define STS_FLR		(1<<3)		/* frame list rolled over */
#define STS_PCD		(1<<2)		/* port change detect */
#define STS_ERR		(1<<1)		/* "error" completion (overflow, ...) */
#define STS_INT		(1<<0)		/* "normal" completion (short, ...) */

	/* USBINTR: offset 0x08 */
	MMP_UINT32		intr_enable;

	/* FRINDEX: offset 0x0C */
	MMP_UINT32		frame_index;	/* current microframe number */
    MMP_UINT32      reserved[1];
	/* PERIODICLISTBASE: offset 0x14 */
	MMP_UINT32		frame_list; 	/* points to periodic list */
	/* ASYNCICLISTADDR: offset 0x18 */
	MMP_UINT32		async_next;	/* address of next async queue head */

    MMP_UINT32      reserved2[1];

	/* PORTSC: offset 0x20 */
    MMP_UINT32      port_status[1];  /* up to N_PORTS */
/* 31:23 reserved */
#define PORT_WKOC_E	    (1<<22)		/* wake on overcurrent (enable) */
#define PORT_WKDISC_E	(1<<21)		/* wake on disconnect (enable) */
#define PORT_WKCONN_E	(1<<20)		/* wake on connect (enable) */
/* 19:16 for port testing */
#define PORT_TEST_SHT       16
#define PORT_TEST_MSK	    (0xF<<16)	/* test mode mask */
#define PORT_TEST_J_STATE	(0x1<<16)	/* Test J_STATE */
#define PORT_TEST_K_STATE	(0x2<<16)	/* Test K_STATE */
#define PORT_TEST_SE0_NAK	(0x3<<16)	/* Test SE0_NAK */
#define PORT_TEST_PACKET	(0x4<<16)	/* Test Packet */
#define PORT_TEST_FORCE_EN	(0x5<<16)	/* Test FORCE_ENABLE */
/* 9 reserved */
#define PORT_RESET	    (1<<8)		/* reset port */
#define PORT_SUSPEND	(1<<7)		/* suspend port */
#define PORT_RESUME	    (1<<6)		/* resume it */

#define PORT_PEC	    (1<<3)		/* port enable change */
#define PORT_PE		    (1<<2)		/* port enable */
#define PORT_CSC	    (1<<1)		/* connect status change */
#define PORT_CONNECT	(1<<0)		/* device connected */

    MMP_UINT32      reserved3[3];
    MMP_UINT32      hc_misc;
};



//===============================================================================
/** ehci_hcd->lock guards shared data against other CPUs:
 *   ehci_hcd:	async, reclaim, periodic (and shadow), ...
 *   hcd_dev:	ep[]
 *   ehci_qh:	qh_next, qtd_list
 *   ehci_qtd:	qtd_list
 *
 * Also, hold this lock when talking to HC registers or
 * when updating hw_* fields in shared qh/qtd/... structures.
 */
//===============================================================================
#define	EHCI_MAX_ROOT_PORTS	    15		/* see HCS_N_PORTS */
#define EHCI_QH_NUM             8// 4
#define EHCI_QTD_NUM            50
#define EHCI_20KBUF_NUM         15
#define EHCI_ITD_NUM            1   // ISO TODO
#define EHCI_SITD_NUM           1   // ISO TODO


struct ehci_hcd /* one per controller */
{			
    _spinlock_t         lock;

    /* async schedule support */
    struct ehci_qh*		async;
    struct ehci_qh*		reclaim;
    MMP_INT             reclaim_ready;

    /* periodic schedule support */
#define	DEFAULT_I_TDPS		1024		/* some HCs can do less */
    MMP_UINT            periodic_size;
    MMP_UINT32*         periodic;	    /* hw periodic table */
    MMP_UINT            i_thresh;	    /* uframes HC might cache */

    #if 0
    union ehci_shadow*	pshadow;	    /* mirror hw periodic table */
    #else
    void**        pshadow;	    /* mirror hw periodic table, we let it in system memory */
    #endif
    MMP_INT             next_uframe;	/* scan periodic, start here */
    MMP_UINT            periodic_urbs;	/* how many urbs scheduled? */

    /* per root hub port */
    MMP_ULONG   		reset_done [EHCI_MAX_ROOT_PORTS];

    /* glue to PCI and HCD framework */
    struct usb_hcd		hcd;
    struct ehci_caps	caps;
    struct ehci_regs	regs;
    struct otg_regs     otg_regs;
    struct global_regs  global_regs;
    MMP_UINT32          hc_caps;
    MMP_UINT32			hcs_params;	    /* cached register copy */
    MMP_UINT32          hcc_params;

    /* per-HC memory pools (could be per-PCI-bus, but ...) */
    MMP_UINT8*		    qh_pool;	    /* qh per active urb */
    MMP_UINT8*		    qtd_pool;	    /* one or more per qh */
    MMP_UINT8*		    itd_pool;	    /* itd per iso urb */
    MMP_UINT8*		    sitd_pool;	    /* sitd per split iso urb */

    MMP_UINT8           qh_manage[EHCI_QH_NUM];
    MMP_UINT8           qtd_manage[EHCI_QTD_NUM];
    MMP_UINT8           itd_manage[EHCI_ITD_NUM];
    MMP_UINT8           sitd_manage[EHCI_SITD_NUM];
#define EHCI_MEM_FREE   0
#define EHCI_MEM_USED   1

#if !defined(__FREERTOS__) && !defined(_OPENRTOS__)
    MMP_UINT8*          page_pool;
    MMP_UINT8           page_manage[EHCI_20KBUF_NUM];
#endif

    volatile MMP_UINT32          tasklet;
};

/* unwrap an HCD pointer to get an EHCI_HCD pointer */ 
#define hcd_to_ehci(hcd_ptr) list_entry(hcd_ptr, struct ehci_hcd, hcd)
#define tasklet_schedule(ehci)  (ehci->tasklet++)

/* NOTE:  urb->transfer_flags expected to not use this bit !!! */
#define EHCI_STATE_UNLINK	0x8000		/* urb being unlinked */




//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.5
 * QTD: describe data transfer components (buffer, direction, ...) 
 * See Fig 3-6 "Queue Element Transfer Descriptor Block Diagram".
 *
 * These are associated only with "QH" (Queue Head) structures,
 * used with control, bulk, and interrupt transfers.
 */
//===============================================================================
#define QTD_NEXT(addr)  cpu_to_le32(((MMP_UINT32)(addr) - (MMP_UINT32)HOST_GetVramBaseAddress()))

/* for periodic/async schedules and qtd lists, mark end of list */
#define	EHCI_LIST_END	cpu_to_le32(1) /* "null pointer" to hw */

struct ehci_qtd {
	/* first part defined by EHCI spec */
	MMP_UINT32			hw_next;        /* see EHCI 3.5.1 */
	MMP_UINT32			hw_alt_next;    /* see EHCI 3.5.2 */
	MMP_UINT32			hw_token;       /* see EHCI 3.5.3 */       
#define	QTD_TOGGLE	    (1 << 31)	    /* data toggle */
#define	QTD_LENGTH(tok)	(((tok)>>16) & 0x7fff)
#define	QTD_IOC		    (1 << 15)	/* interrupt on complete */
#define	QTD_CERR(tok)	(((tok)>>10) & 0x3)
#define	QTD_PID(tok)	(((tok)>>8) & 0x3)
#define	QTD_STS_ACTIVE	(1 << 7)	/* HC may execute this */
#define	QTD_STS_HALT	(1 << 6)	/* halted on error */
#define	QTD_STS_DBE	    (1 << 5)	/* data buffer error (in HC) */
#define	QTD_STS_BABBLE	(1 << 4)	/* device was babbling (qtd halted) */
#define	QTD_STS_XACT	(1 << 3)	/* device gave illegal response */
#define	QTD_STS_MMF	    (1 << 2)	/* incomplete split transaction */
#define	QTD_STS_STS	    (1 << 1)	/* split transaction state */
#define	QTD_STS_PING	(1 << 0)	/* issue PING? */
	MMP_UINT32			hw_buf0;     /* see EHCI 3.5.4 */
	MMP_UINT32			hw_buf1;     /* see EHCI 3.5.4 */
	MMP_UINT32			hw_buf2;     /* see EHCI 3.5.4 */
	MMP_UINT32			hw_buf3;     /* see EHCI 3.5.4 */
	MMP_UINT32			hw_buf4;     /* see EHCI 3.5.4 */

	/* the rest is HCD-private */
	MMP_UINT8*		    qtd_addr;		/* qtd address */
	struct list_head	qtd_list;		/* sw qtd list */

	struct urb*		    urb;			/* qtd's urb */
	MMP_UINT8*		    buf_addr;		/* buffer address */
	MMP_UINT32			length;			/* length of buffer */

    MMP_UINT32          buf_index;
};


/*-------------------------------------------------------------------------*/

/* type tag from {qh,itd,sitd,fstn}->hw_next */
#define Q_NEXT_TYPE(dma) ((dma) & cpu_to_le32(3 << 1))

/* values for that type tag */
#define Q_TYPE_ITD	    (0 << 1)
#define Q_TYPE_QH	    (1 << 1)
#define Q_TYPE_SITD 	(2 << 1)
#define Q_TYPE_FSTN 	(3 << 1)

/* next async queue entry, or pointer to interrupt/periodic QH */
#define	QH_NEXT(addr)	(cpu_to_le32((((MMP_UINT32)addr-(MMP_UINT32)HOST_GetVramBaseAddress())&~0x01f)|Q_TYPE_QH))

/*
 * Entries in periodic shadow table are pointers to one of four kinds
 * of data structure.  That's dictated by the hardware; a type tag is
 * encoded in the low bits of the hardware's periodic schedule.  Use
 * Q_NEXT_TYPE to get the tag.
 *
 * For entries in the async schedule, the type tag always says "qh".
 */
#if 0
union ehci_shadow 
{
	struct ehci_qh 		*qh;		/* Q_TYPE_QH */
	struct ehci_itd		*itd;		/* Q_TYPE_ITD */
	struct ehci_sitd	*sitd;		/* Q_TYPE_SITD */
	struct ehci_fstn	*fstn;		/* Q_TYPE_FSTN */
	void			*ptr;
};
#else
typedef void*   ehci_shadow;
#endif


//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.6
 * QH: describes control/bulk/interrupt endpoints
 * See Fig 3-7 "Queue Head Structure Layout".
 *
 * These appear in both the async and (for interrupt) periodic schedules.
 */
//===============================================================================
struct ehci_qh {
	/* first part defined by EHCI spec */
	MMP_UINT32			hw_next;	    /* see EHCI 3.6.1 */
	MMP_UINT32			hw_info1;       /* see EHCI 3.6.2 */
#define	QH_HEAD		0x00008000
	MMP_UINT32			hw_info2;       /* see EHCI 3.6.2 */
	MMP_UINT32			hw_current;	    /* qtd list - see EHCI 3.6.4 */
	
	/* qtd overlay (hardware parts of a struct ehci_qtd) */
	MMP_UINT32			hw_qtd_next;
	MMP_UINT32			hw_alt_next;
	MMP_UINT32			hw_token;
	MMP_UINT32			hw_buf0;
	MMP_UINT32			hw_buf1;
	MMP_UINT32			hw_buf2;
	MMP_UINT32			hw_buf3;
	MMP_UINT32			hw_buf4;

	/* the rest is HCD-private */
	MMP_UINT8*		    qh_addr;		/* address of qh */
    #if 1
    //void*           	qh_next;	    /* ptr to qh; or periodic */
    ehci_shadow        	qh_next;	    /* ptr to qh; or periodic */
    #else
    union ehci_shadow	qh_next;	    /* ptr to qh; or periodic */
    #endif
	struct list_head	qtd_list;	    /* sw qtd list */

	MMP_UINT32  		usecs;		    /* intr bandwidth */
	MMP_UINT32			qh_state;
#define	QH_STATE_LINKED		1		/* HC sees this */
#define	QH_STATE_UNLINK		2		/* HC may still see this */
#define	QH_STATE_IDLE		3		/* HC doesn't see this */

    MMP_UINT32          buf_index;
    MMP_UINT32          refcount;
};



//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.3
 * Fig 3-4 "Isochronous Transaction Descriptor (iTD)"
 *
 * Schedule records for high speed iso xfers
 */
//===============================================================================
struct ehci_itd {
	/* first part defined by EHCI spec */
	MMP_UINT32			hw_next;           /* see EHCI 3.3.1 */
	MMP_UINT32			hw_transaction [8]; /* see EHCI 3.3.2 */
#define EHCI_ISOC_ACTIVE        (1<<31)        /* activate transfer this slot */
#define EHCI_ISOC_BUF_ERR       (1<<30)        /* Data buffer error */
#define EHCI_ISOC_BABBLE        (1<<29)        /* babble detected */
#define EHCI_ISOC_XACTERR       (1<<28)        /* XactErr - transaction error */
#define	EHCI_ITD_LENGTH(tok)	(((tok)>>16) & 0x7fff)
#define	EHCI_ITD_IOC		    (1 << 15)	/* interrupt on complete */

	MMP_UINT32			hw_bufp [7];	/* see EHCI 3.3.3 */ 

	/* the rest is HCD-private */
	MMP_UINT8*		    itd_addr;	/* for this itd */
#if 1
	ehci_shadow	        itd_next;	/* ptr to periodic q entry */
#else
	union ehci_shadow	itd_next;	/* ptr to periodic q entry */
#endif

	struct urb*		    urb;
	struct list_head	itd_list;	/* list of urb frames' itds */
	MMP_UINT8*		    buf_addr;	/* frame's buffer address */

	/* for now, only one hw_transaction per itd */
	MMP_UINT32			transaction;
	MMP_UINT32			index;		/* in urb->iso_frame_desc */
	MMP_UINT32			uframe;		/* in periodic schedule */
	MMP_UINT32			usecs;
};


//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.4 
 * siTD, aka split-transaction isochronous Transfer Descriptor
 *       ... describe low/full speed iso xfers through TT in hubs
 * see Figure 3-5 "Split-transaction Isochronous Transaction Descriptor (siTD)
 */
//===============================================================================
struct ehci_sitd {
	/* first part defined by EHCI spec */
	MMP_UINT32			hw_next;
/* uses bit field macros above - see EHCI 0.95 Table 3-8 */
	MMP_UINT32			hw_fullspeed_ep;  /* see EHCI table 3-9 */
	MMP_UINT32          hw_uframe;        /* see EHCI table 3-10 */
    MMP_UINT32          hw_tx_results1;   /* see EHCI table 3-11 */
	MMP_UINT32          hw_tx_results2;   /* see EHCI table 3-12 */
	MMP_UINT32          hw_tx_results3;   /* see EHCI table 3-12 */
    MMP_UINT32          hw_backpointer;   /* see EHCI table 3-13 */
	MMP_UINT32			hw_buf_hi [2];	  /* Appendix B */

	/* the rest is HCD-private */
	MMP_UINT8*		    sitd_addr;
#if 1
	ehci_shadow	        sitd_next;	/* ptr to periodic q entry */
#else
	union ehci_shadow	sitd_next;	/* ptr to periodic q entry */
#endif
	struct urb*		    urb;
	MMP_UINT8*		    buf_addr;	/* buffer address */
};



//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.7
 * Periodic Frame Span Traversal Node (FSTN)
 *
 * Manages split interrupt transactions (using TT) that span frame boundaries
 * into uframes 0/1; see 4.12.2.2.  In those uframes, a "save place" FSTN
 * makes the HC jump (back) to a QH to scan for fs/ls QH completions until
 * it hits a "restore" FSTN; then it returns to finish other uframe 0/1 work.
 */
//===============================================================================
struct ehci_fstn {
	MMP_UINT32			hw_next;	/* any periodic q entry */
	MMP_UINT32			hw_prev;	/* qh or EHCI_LIST_END */

	/* the rest is HCD-private */
	MMP_UINT8*		    fstn_dma;
#if 1
	ehci_shadow	        fstn_next;	/* ptr to periodic q entry */
#else
	union ehci_shadow	fstn_next;	/* ptr to periodic q entry */
#endif
};


#define likely(x)	(x)
#define unlikely(x)	(x)
#if 1//defined(MS_ENUMERATE)
MMP_INT ehci_port_reset(struct ehci_hcd *ehci);
#endif
MMP_INT ehci_init(struct usb_hcd *hcd, struct usb_device** usb_device);
#if defined(MM680)
    #if defined(USB_IRQ_ENABLE)
    MMP_INT ehci_isDisconnect(struct usb_hcd* hcd);
    MMP_INT ehci_irq(void* hcd);
    #else
    void ehci_irq(struct usb_hcd* hcd);
    #endif 
#else // #if defined(MM680)
    #if defined(USB_IRQ_ENABLE)
    MMP_INT ehci_isDisconnect(struct usb_hcd* hcd);
    #endif
    void ehci_irq(struct usb_hcd* hcd);
#endif // #if defined(MM680)


#ifdef __cplusplus
}
#endif

#endif // #ifndef USB_EHCI_HCD_H

