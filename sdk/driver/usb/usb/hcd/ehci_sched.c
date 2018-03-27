/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * This file is part of ehci-hcd.c 
 *
 * @author Irene Lin
 */

/*-------------------------------------------------------------------------*/
/*
 * EHCI scheduled transaction support:  interrupt, iso, split iso
 * These are called "periodic" transactions in the EHCI spec.
 *
 * Note that for interrupt transfers, the QH/QTD manipulation is shared
 * with the "asynchronous" transaction support (control/bulk transfers).
 * The only real difference is in how interrupt transfers are scheduled.
 * We get some funky API restrictions from the current URB model, which
 * works notably better for reading transfers than for writing.  (And
 * which accordingly needs to change before it'll work inside devices,
 * or with "USB On The Go" additions to USB 2.0 ...)
 */
/*-------------------------------------------------------------------------*/

extern void usb_claim_bandwidth(struct usb_device *dev, struct urb *urb, MMP_INT bustime, MMP_INT isoc);
static int ehci_get_frame(struct usb_hcd *hcd);


#define GET_FRAME(f, r)    HOST_ReadBlockMemory((MMP_UINT32)(r), (MMP_UINT32)ehci->periodic+(f)*4, sizeof(MMP_UINT32))
#define SET_FRAME(f, v)    HOST_WriteBlockMemory((MMP_UINT32)ehci->periodic+(f)*4, (MMP_UINT32)&(v), sizeof(MMP_UINT32))



/*-------------------------------------------------------------------------*/

static void enable_periodic(struct ehci_hcd* ehci)
{
    MMP_UINT32 cmd = 0;
    MMP_UINT32 reg = 0;
#if defined(ALPHA_SDK)
    struct timeval startT, endT;
#else
    SYS_CLOCK_T lastTime;
#endif

    LOG_INFO " @@ enable_periodic @@\n" LOG_END

    /**
     * did clearing PSE did take effect yet?
     * takes effect only at frame boundaries...
     */
#if defined(ALPHA_SDK)
    gettimeofday(&startT, NULL);

    AHB_ReadRegister(ehci->regs.status, &reg);
    while(reg & STS_PSS)
    {
        gettimeofday(&endT, NULL);
        if(itpTimevalDiff(&startT, &endT) > 500)
#else
    lastTime = SYS_GetClock();

    AHB_ReadRegister(ehci->regs.status, &reg);
    while(reg & STS_PSS)
    {
        if(SYS_GetDuration(lastTime) > 500)
#endif
        {
            LOG_ERROR " enable_periodic() wait PSS ready timeout!\n" LOG_END
            while(1);
        }
        MMP_USleep(20);
        AHB_ReadRegister(ehci->regs.status, &reg);
    }

    AHB_ReadRegister(ehci->regs.command, &cmd);
    if(!(cmd & CMD_RUN))
        LOG_ERROR " HC NOT RUNNING!!!!!!! \n" LOG_END
    cmd |= CMD_PSE;
    AHB_WriteRegister(ehci->regs.command, cmd);

    ehci->hcd.state = USB_STATE_RUNNING;

    /** make sure tasklet scans these */
    AHB_ReadRegister(ehci->regs.frame_index, &reg);
    ehci->next_uframe = reg % (ehci->periodic_size << 3);
    LOG_INFO " ehci->next_uframe = 0x%08X\n", ehci->next_uframe LOG_END
}

static void disable_periodic(struct ehci_hcd* ehci)
{
    MMP_UINT32 cmd = 0;
    MMP_UINT32 reg = 0;
#if defined(ALPHA_SDK)
    struct timeval startT, endT;
#else
    SYS_CLOCK_T lastTime;
#endif

    LOG_INFO " @@ disable_periodic @@\n" LOG_END

    /* did setting PSE not take effect yet?
     * takes effect only at frame boundaries...
     */
#if defined(ALPHA_SDK)
    gettimeofday(&startT, NULL);

    AHB_ReadRegister(ehci->regs.status, &reg);
    while(!(reg & STS_PSS))
    {
        gettimeofday(&endT, NULL);
        if(itpTimevalDiff(&startT, &endT) > 500)
#else
    lastTime = SYS_GetClock();

    AHB_ReadRegister(ehci->regs.status, &reg);
    while(!(reg & STS_PSS))
    {
        if(SYS_GetDuration(lastTime) > 500)
#endif
        {
            LOG_ERROR " disable_periodic() PSE not take effect yet?\n" LOG_END
            while(1);
        }
        MMP_USleep(20);
        AHB_ReadRegister(ehci->regs.status, &reg);
    }

    AHB_ReadRegister(ehci->regs.command, &cmd);
    cmd = cmd & ~CMD_PSE;
    AHB_WriteRegister(ehci->regs.command, cmd);

    ehci->next_uframe = -1;
}

/*-------------------------------------------------------------------------*/

/**
 * periodic_next_shadow - return "next" pointer on shadow list
 * @periodic: host pointer to qh/itd/sitd
 * @tag: hardware tag for type of this record
 */
static ehci_shadow
periodic_next_shadow(ehci_shadow periodic, MMP_INT tag)
{
    ehci_shadow next = MMP_NULL;

    switch(le32_to_cpu(tag))
    {
    case Q_TYPE_QH:
        {
            struct ehci_qh* qh = (struct ehci_qh*)periodic;
            VMEM_STRUCT_R(ehci_qh, qh, qh_next, &next);
            return next;
        }
#if 0
    case Q_TYPE_FSTN:
        return &periodic->fstn->fstn_next;
    case Q_TYPE_ITD:
        return &periodic->itd->itd_next;
#ifdef have_split_iso
    case Q_TYPE_SITD:
        return &periodic->sitd->sitd_next;
#endif /* have_split_iso */
#endif
    }

    LOG_ERROR " periodic_next_shadow() BAD shadow %X tag %d \n", periodic, tag LOG_END
    return MMP_NULL;
}

/**
 * returns true after successful unlink 
 * caller must hold ehci->lock
 */
static MMP_BOOL periodic_unlink(struct ehci_hcd* ehci, MMP_UINT32 frame, void* ptr)
{
#if 1
    MMP_UINT32 hw_p;    /** we use value */
    ehci_shadow	here = ehci->pshadow[frame]; /** we use value */
    struct ehci_qh* qh;
    ehci_shadow next;

    GET_FRAME(frame, &hw_p);

    /** find predecessor of "ptr"; hw and shadow lists are in sync */
    while(here && here != ptr) 
    {
        qh = (struct ehci_qh*)here;
        here = periodic_next_shadow(here, Q_NEXT_TYPE(hw_p));
        VMEM_STRUCT_R(ehci_qh, qh, hw_next, &hw_p);
    }
    /** an interrupt entry (at list end) could have been shared */
    if(!here) 
    {
        LOG_ERROR " entry %X no longer on frame [%d] \n", ptr, frame LOG_END
        return MMP_FALSE;
    }
    LOG_INFO " periodic unlink %X from frame %d \n", ptr, frame LOG_END

    /**
     * update hardware list ... HC may still know the old structure, so
     * don't change hw_next until it'll have purged its cache
     */
    qh = (struct ehci_qh*)here;
    next = periodic_next_shadow(here, Q_NEXT_TYPE(hw_p));
    VMEM_STRUCT_R(ehci_qh, qh, hw_next, &hw_p);
    SET_FRAME(frame, hw_p);

    /** unlink from shadow list; HCD won't see old structure again */
    ehci->pshadow[frame] = next;

    return MMP_TRUE;
#else
    union ehci_shadow	*prev_p = &ehci->pshadow [frame];
    u32			*hw_p = &ehci->periodic [frame];
    union ehci_shadow	here = *prev_p;
    union ehci_shadow	*next_p;

  DBG_HOST_EHCI("### >>> Enter ehci-sched.c file --> periodic_unlink function \n");

    /* find predecessor of "ptr"; hw and shadow lists are in sync */
    while (here.ptr && here.ptr != ptr) {
        prev_p = periodic_next_shadow (prev_p, Q_NEXT_TYPE (*hw_p));
        hw_p = &here.qh->hw_next;
        here = *prev_p;
    }
    /* an interrupt entry (at list end) could have been shared */
    if (!here.ptr) {
        dbg ("entry %p no longer on frame [%d]", ptr, frame);
        return 0;
    }
    // vdbg ("periodic unlink %p from frame %d", ptr, frame);

    /* update hardware list ... HC may still know the old structure, so
     * don't change hw_next until it'll have purged its cache
     */
    next_p = periodic_next_shadow (&here, Q_NEXT_TYPE (*hw_p));
    *hw_p = here.qh->hw_next;

    /* unlink from shadow list; HCD won't see old structure again */
    *prev_p = *next_p;
    next_p->ptr = 0;

    return 1;
#endif
}

static void intr_deschedule(struct ehci_hcd* ehci,
                            MMP_UINT32	frame,
                            struct ehci_qh*	qh,
                            MMP_UINT32	period) 
{
    MMP_UINT32 tmp_value;

    LOG_INFO " intr_deschedule() ehci %X, frame %d, qh %X, period %d \n", ehci, frame, qh, period LOG_END
  
    period >>= 3;		// FIXME microframe periods not handled yet

    _spin_lock_irqsave(&ehci->lock);

    do {
        periodic_unlink(ehci, frame, qh);
        qh_put(ehci, qh);
        frame += period;
    } while(frame < ehci->periodic_size);

    tmp_value = QH_STATE_UNLINK;
    VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
    tmp_value = 0;
    VMEM_STRUCT_W(ehci_qh, qh, qh_next, (void*)tmp_value);
    ehci->periodic_urbs--;

    /** maybe turn off periodic schedule */
    if(!ehci->periodic_urbs)
        disable_periodic(ehci);
    else
        LOG_INFO "periodic schedule still enabled \n" LOG_END

    _spin_unlock_irqrestore(&ehci->lock);

    /*
     * If the hc may be looking at this qh, then delay a uframe
     * (yeech!) to be sure it's done.
     * No other threads may be mucking with this qh.
     */
    if(((ehci_get_frame(&ehci->hcd) - frame) % period) == 0)
        MMP_USleep(125);

    tmp_value = QH_STATE_IDLE;
    VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
    tmp_value = EHCI_LIST_END;
    VMEM_STRUCT_W(ehci_qh, qh, hw_next, tmp_value);

    VMEM_STRUCT_R(ehci_qh, qh, refcount, &tmp_value);
    LOG_INFO " descheduled qh %p, per = %d frame = %d count = %d, urbs = %d \n",
        qh, period, frame, tmp_value, ehci->periodic_urbs LOG_END
}

/*-------------------------------------------------------------------------*/

/**
 * how many of the uframe's 125 usecs are allocated? 
 */
static MMP_UINT32
periodic_usecs(struct ehci_hcd* ehci, 
               MMP_UINT32 frame, 
               MMP_UINT32 uframe)
{
    MMP_UINT32  hw_p; // = &ehci->periodic [frame];
    void**	    q = &ehci->pshadow[frame];
    MMP_UINT32	usecs = 0;
    MMP_UINT32  tmp_value = 0;

    GET_FRAME(frame, &hw_p);
    while((*q)) 
    {
        LOG_INFO " periodic_usecs@ Q_NEXT_TYPE(hw_p) = %d \n", Q_NEXT_TYPE(hw_p) LOG_END
        switch(Q_NEXT_TYPE(hw_p)) 
        {
        case Q_TYPE_QH:
            /* is it in the S-mask? */
            #if 1
            {
                struct ehci_qh* qh = (struct ehci_qh*)(*q);
                VMEM_STRUCT_R(ehci_qh, qh, hw_info2, &tmp_value);
                if(tmp_value & cpu_to_le32(1 << uframe))
                {
                    VMEM_STRUCT_R(ehci_qh, qh, usecs, &tmp_value);
                    usecs += tmp_value;
                }
                VMEM_STRUCT_R_ADDR(ehci_qh, qh, qh_next, q);
            }
            #else
            if(q->qh->hw_info2 & cpu_to_le32 (1 << uframe))
                usecs += q->qh->usecs;
            q = &q->qh->qh_next;
            #endif
            break;
        #if 0
        case Q_TYPE_FSTN:
            /* for "save place" FSTNs, count the relevant INTR
             * bandwidth from the previous frame
             */
            if (q->fstn->hw_prev != EHCI_LIST_END) {
                dbg ("not counting FSTN bandwidth yet ...");
            }
            q = &q->fstn->fstn_next;
            break;
        case Q_TYPE_ITD:
            /* NOTE the "one uframe per itd" policy */
            if (q->itd->hw_transaction [uframe] != 0)
                usecs += q->itd->usecs;
            q = &q->itd->itd_next;
            break;
        #endif
#ifdef have_split_iso
        case Q_TYPE_SITD:
            temp = q->sitd->hw_fullspeed_ep &
                __constant_cpu_to_le32 (1 << 31);

            // FIXME:  this doesn't count data bytes right...

            /* is it in the S-mask?  (count SPLIT, DATA) */
            if (q->sitd->hw_uframe & cpu_to_le32 (1 << uframe)) {
                if (temp)
                    usecs += HS_USECS (188);
                else
                    usecs += HS_USECS (1);
            }

            /* ... C-mask?  (count CSPLIT, DATA) */
            if (q->sitd->hw_uframe &
                    cpu_to_le32 (1 << (8 + uframe))) {
                if (temp)
                    usecs += HS_USECS (0);
                else
                    usecs += HS_USECS (188);
            }
            q = &q->sitd->sitd_next;
            break;
#endif /* have_split_iso */
        default:
            LOG_ERROR " periodic_usecs() error type! \n" LOG_END
            while(1);
        }
    }

    LOG_DEBUG " periodic_usecs() frame %d, uframe %d => usecs %d \n", frame, uframe, usecs LOG_END

    return usecs;
}


static MMP_INT check_period(struct ehci_hcd* ehci, 
                            MMP_UINT32	frame,
                            MMP_INT		uframe,
                            MMP_UINT32	period,
                            MMP_UINT32	usecs) 
{
    LOG_DEBUG " check_period() enter \n" LOG_END

    /**
     * 80% periodic == 100 usec/uframe available
     * convert "usecs we need" to "max already claimed" 
     */
    usecs = 100 - usecs;

    do 
    {
        MMP_UINT32	claimed;

        // FIXME delete when intr_submit handles non-empty queues
        // this gives us a one intr/frame limit (vs N/uframe)
        if(ehci->pshadow[frame])
        {
            LOG_WARNING " this gives us a one intr/frame limit (vs N/uframe)\n" LOG_END
            return 0;
        }

        claimed = periodic_usecs(ehci, frame, uframe);
        if(claimed > usecs)
            return 0;

    // FIXME update to handle sub-frame periods ??
    } while((frame += period) < ehci->periodic_size);

    return 1; /** success! */
}

/*-------------------------------------------------------------------------*/

static MMP_INT intr_submit(struct ehci_hcd* ehci,
                           struct urb*		urb,
                           struct list_head* qtd_list) 
{
    MMP_INT		result = 0;
    MMP_INT		epnum, period;
    MMP_UINT32  tmp_value=0;
    struct list_head* tmp_list_head=MMP_NULL;
    struct list_head* tmp_list_head1=MMP_NULL;
    MMP_UINT32	usecs = 0;
    struct ehci_qh*	qh = MMP_NULL;
    struct hcd_dev* dev = MMP_NULL;
    MMP_BOOL    head_in_vram = MMP_FALSE;

    LOG_ENTER " intr_submit() ehci 0x%08X, urb 0x%08X \n", ehci, urb LOG_END

    /** get endpoint and transfer data */
    epnum = usb_pipeendpoint(urb->pipe);
    if(usb_pipein(urb->pipe))
        epnum |= 0x10;

    /*
     * NOTE: current completion/restart logic doesn't handle more than
     * one qtd in a periodic qh ... 16-20 KB/urb is pretty big for this.
     * such big requests need many periods to transfer.
     *
     * FIXME want to change hcd core submit model to expect queuing
     * for all transfer types ... not just ISO and (with flag) BULK.
     * that means: getting rid of this check; handling the "interrupt
     * urb already queued" case below like bulk queuing is handled (no
     * errors possible!); and completly getting rid of that annoying
     * qh restart logic.  simpler/smaller overall, and more flexible.
     */
    VMEM_STRUCT_R(list_head, qtd_list, next, &tmp_list_head);
    VMEM_STRUCT_R(list_head, qtd_list, prev, &tmp_list_head1);
    if(unlikely(tmp_list_head != tmp_list_head1)) 
    {
        LOG_ERROR " Only one intr qtd per urb allowed!! => unlikely(qtd_list->next != qtd_list->prev) \n" LOG_END
        result = -EINVAL;
        goto done;
    }

    usecs = HS_USECS(urb->transfer_buffer_length);

    /* FIXME handle HS periods of less than 1 frame. */
    period = urb->interval >> 3;
    if(period < 1) // never??
    {
        LOG_ERROR " intr period %d uframes, NYET! => intr_submit (period < 1)\n", urb->interval LOG_END
        result = -EINVAL;
        goto done;
    }

    _spin_lock_irqsave(&ehci->lock);

    /* get the qh (must be empty and idle) */
    dev = (struct hcd_dev *)urb->dev->hcpriv;
    qh = (struct ehci_qh *)dev->ep[epnum];

    if(qh) 
    {
        LOG_DEBUG " intr_submit() has qh %X \n", qh LOG_END
        LOG_ERROR " intr_submit() has qh %X ====> TODO ###### \n" LOG_END
        while(1);
#if 0
        /* only allow one queued interrupt urb per EP */
        if(unlikely(qh->qh_state != QH_STATE_IDLE
                || !list_empty (&qh->qtd_list))) 
        {
            LOG_ERROR " interrupt urb already queued! \n" LOG_END
            result = -EBUSY;
        } 
        else 
        {
            LOG_ERROR " intr_submit function(maybe reset) \n" LOG_END

            /* maybe reset hardware's data toggle in the qh */
            if (unlikely (!usb_gettoggle (urb->dev, epnum & 0x0f,
                    !(epnum & 0x10)))) {
                qh->hw_token |=
                    __constant_cpu_to_le32 (QTD_TOGGLE);
                usb_settoggle (urb->dev, epnum & 0x0f,
                    !(epnum & 0x10), 1);
            }
            /* trust the QH was set up as interrupt ... */
            list_splice (qtd_list, &qh->qtd_list);
            qh_update (qh, list_entry (qtd_list->next,
                        struct ehci_qtd, qtd_list));
            qtd_list = &qh->qtd_list;
        }
#endif
    } 
    else 
    {
        LOG_DEBUG " intr_submit() new qh \n" LOG_END

        qh = ehci_qh_make(ehci, urb, qtd_list);
        if(likely(qh != 0)) 
        {
            dev->ep[epnum] = qh;
            //qtd_list = &qh->qtd_list;
            VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&qtd_list);
            head_in_vram = MMP_TRUE;
        } 
        else
            result = -ENOMEM;
    }

    /** Schedule this periodic QH. */
    if(likely(result == 0)) 
    {
        MMP_UINT32 frame = (MMP_UINT32)period;

        tmp_value = EHCI_LIST_END;
        VMEM_STRUCT_W(ehci_qh, qh, hw_next, tmp_value);
        VMEM_STRUCT_W(ehci_qh, qh, usecs, usecs);

        urb->hcpriv = qh_get(qh);
        result = -ENOSPC;

        /** pick a set of schedule slots, link the QH into them */
        do 
        {
            MMP_INT	uframe;

            /* pick a set of slots such that all uframes have
             * enough periodic bandwidth available.
             *
             * FIXME for TT splits, need uframes for start and end.
             * FSTNs can put end into next frame (uframes 0 or 1).
             */
            frame--;
            for(uframe=0; uframe<8; uframe++) 
            {
                if(check_period(ehci, frame, uframe, period, usecs) != 0)
                    break;
            }
            if(uframe == 8)
                continue;

            /** QH will run once each period, starting there  */
            urb->start_frame = frame;
            result = 0;

            /** set S-frame mask */
            VMEM_STRUCT_R(ehci_qh, qh, hw_info2, &tmp_value);
            tmp_value |= cpu_to_le32(1 << uframe);
            VMEM_STRUCT_W(ehci_qh, qh, hw_info2, tmp_value);

            /** stuff into the periodic schedule */
            tmp_value = QH_STATE_LINKED;
            VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
            LOG_INFO " qh %X usecs %d period %d starting frame %d uframe %d \n", qh, usecs, period, frame, uframe LOG_END

            do 
            {
                if(unlikely(ehci->pshadow[frame] != 0)) 
                {
                    // FIXME -- just link toward the end, before any qh with a shorter period,
                    // AND handle it already being (implicitly) linked into this frame
                    // AS WELL AS updating the check_period() logic
                    LOG_ERROR " ehci->pshadow[%d] != 0 ##### Bug!!! #####\n", frame LOG_END
                    while(1);
                } 
                else 
                {
                    ehci->pshadow[frame] = qh_get(qh);
                    tmp_value = QH_NEXT(qh);
                    SET_FRAME(frame, tmp_value);
                }
                frame += period;
            } while(frame < ehci->periodic_size);
#if 0
    if(0)
    {
        dumpVram((MMP_UINT8*)qh, sizeof(struct ehci_qh));
    }
#endif

            /** update bandwidth utilization records (for usbfs) */
            usb_claim_bandwidth(urb->dev, urb, usecs/period, 0);

            /** maybe enable periodic schedule processing */
            if(!ehci->periodic_urbs++)
                enable_periodic(ehci);
            break;
        } while(frame);
    }
    _spin_unlock_irqrestore(&ehci->lock);

done:
    if(result)
        qtd_list_free(ehci, urb, qtd_list, head_in_vram);

    return result;
}


static void
intr_complete(struct ehci_hcd* ehci,
              MMP_UINT32	   frame,
              struct ehci_qh*  qh)		/* caller owns ehci->lock ... */
{
    struct ehci_qtd* qtd;
    struct urb*	     urb;
    MMP_INT		unlinking;
    MMP_UINT32  tmp_value;
    struct list_head *qtd_list, *tmp_list_head;

    LOG_DEBUG " intr_complete() ehci %X, frame %d, qh %X \n", ehci, frame, qh LOG_END

    VMEM_STRUCT_R(ehci_qh, qh, hw_token, &tmp_value);
    tmp_value = le32_to_cpu(tmp_value);
    if(likely(tmp_value & QTD_STS_ACTIVE))
        return;

    VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&qtd_list);
    if(unlikely(LIST_EMPTY_VMEM(qtd_list))) 
    {
        LOG_ERROR " interrupt qh %X no TDs??\n" LOG_END
        return;
    }
    
    VMEM_STRUCT_R(list_head, qtd_list, next, &tmp_list_head);
    qtd = list_entry(tmp_list_head, struct ehci_qtd, qtd_list);
    VMEM_STRUCT_R(ehci_qtd, qtd, urb, &urb);
    unlinking = (urb->status == -ENOENT) || (urb->status == -ECONNRESET);

    /** call any completions, after patching for reactivation */
    //_spin_unlock_irqrestore(&ehci->lock);
    /** NOTE:  currently restricted to one qtd per qh! */
    if(qh_completions (ehci, qh, 0) == 0)
        urb = 0;
    //_spin_lock_irqsave(&ehci->lock);

    /** never reactivate requests that were unlinked ... */
    if(likely(urb != 0)) 
    {
        if(unlinking
            || urb->status == -ECONNRESET
            || urb->status == -ENOENT
            // || (urb->dev == MMP_NULL)
            || ehci->hcd.state == USB_STATE_HALT)
            urb = 0;
        // FIXME look at all those unlink cases ... we always
        // need exactly one completion that reports unlink.
        // the one above might not have been it!
    }

    /** normally reactivate */
    if(likely(urb != 0)) 
    {
        urb->status = -EINPROGRESS;
        urb->actual_length = 0;

        /** patch qh and restart */
        qh_update(qh, qtd);
    }
}

/*-------------------------------------------------------------------------*/

static void scan_periodic(struct ehci_hcd* ehci)
{
    MMP_UINT32	frame, clock, now_uframe, mod;
    MMP_UINT32  tmp_value;

    LOG_DEBUG " scan_periodic() \n" LOG_END

    mod = ehci->periodic_size << 3;
    _spin_lock_irqsave(&ehci->lock);

    /**
     * When running, scan from last scan point up to "now"
     * else clean up by scanning everything that's left.
     * Touches as few pages as possible:  cache-friendly.
     * Don't scan ISO entries more than once, though.
     */
    frame = ehci->next_uframe >> 3;
    if(HCD_IS_RUNNING(ehci->hcd.state))
        AHB_ReadRegister(ehci->regs.frame_index, &now_uframe);
    else
        now_uframe = (frame << 3) - 1;
    now_uframe %= mod;
    clock = now_uframe >> 3;

    for(;;) 
    {
        void*  q;
        void** q_p;
        MMP_UINT32	type, hw_p/* *hw_p */;
        MMP_UINT32	uframes;

restart:
        /** scan schedule to _before_ current frame index */
        if(frame == clock)
            uframes = now_uframe & 0x07;
        else
            uframes = 8;

        q_p = &ehci->pshadow[frame];
        //hw_p = &ehci->periodic[frame]; /** original is pointer */
        GET_FRAME(frame, &hw_p);         /** this hw_p is value, not pointer */
        q = (*q_p);
        type = Q_NEXT_TYPE(hw_p);

        /** scan each element in frame's queue for completions */
        while(q != 0) 
        {
            MMP_INT	 last;
            //MMP_UINT uf;
            void*	 temp;

            //switch(type) 
            switch(le32_to_cpu(type)) 
            {
            case Q_TYPE_QH:
                {
                    struct ehci_qh* qh = (struct ehci_qh*)q;
                    VMEM_STRUCT_R(ehci_qh, qh, hw_next, &tmp_value);
                    last = (tmp_value == EHCI_LIST_END);
                    VMEM_STRUCT_R(ehci_qh, qh, qh_next, &temp);
                    type = Q_NEXT_TYPE(tmp_value);
                    intr_complete(ehci, frame, qh_get(qh));
                    qh_put(ehci, qh);
                    q = temp;
                }
                break;
#if 0
            case Q_TYPE_FSTN:
                last = (q.fstn->hw_next == EHCI_LIST_END);
                /* for "save place" FSTNs, look at QH entries
                 * in the previous frame for completions.
                 */
                if (q.fstn->hw_prev != EHCI_LIST_END) {
                    dbg ("ignoring completions from FSTNs");
                }
                type = Q_NEXT_TYPE (q.fstn->hw_next);
                q = q.fstn->fstn_next;
                break;
            case Q_TYPE_ITD:
                last = (q.itd->hw_next == EHCI_LIST_END);

                /* Unlink each (S)ITD we see, since the ISO
                 * URB model forces constant rescheduling.
                 * That complicates sharing uframes in ITDs,
                 * and means we need to skip uframes the HC
                 * hasn't yet processed.
                 */
                for (uf = 0; uf < uframes; uf++) {
                    if (q.itd->hw_transaction [uf] != 0) {
                        temp = q;
                        *q_p = q.itd->itd_next;
                        *hw_p = q.itd->hw_next;
                        type = Q_NEXT_TYPE (*hw_p);

                        /* might free q.itd ... */
                        flags = itd_complete (ehci,
                            temp.itd, uf, flags);
                        break;
                    }
                }
                /* we might skip this ITD's uframe ... */
                if (uf == uframes) {
                    q_p = &q.itd->itd_next;
                    hw_p = &q.itd->hw_next;
                    type = Q_NEXT_TYPE (q.itd->hw_next);
                }

                q = *q_p;
                break;
#ifdef have_split_iso
            case Q_TYPE_SITD:
                last = (q.sitd->hw_next == EHCI_LIST_END);
                flags = sitd_complete (ehci, q.sitd, flags);
                type = Q_NEXT_TYPE (q.sitd->hw_next);

                // FIXME unlink SITD after split completes
                q = q.sitd->sitd_next;
                break;
#endif /* have_split_iso */
#endif
            default:
                LOG_ERROR " corrupt type %d frame %d shadow %X \n",	type, frame, q LOG_END
                last = 1;
                q = 0;
            }

            /* did completion remove an interior q entry? */
            if(unlikely(q==0 && !last))
                goto restart;
        }

        /* stop when we catch up to the HC */

        // FIXME:  this assumes we won't get lapped when
        // latencies climb; that should be rare, but...
        // detect it, and just go all the way around.
        // FLR might help detect this case, so long as latencies
        // don't exceed periodic_size msec (default 1.024 sec).

        // FIXME:  likewise assumes HC doesn't halt mid-scan

        if(frame == clock) 
        {
            MMP_UINT32 now;

            if(!HCD_IS_RUNNING(ehci->hcd.state))
                break;
            ehci->next_uframe = now_uframe;
            AHB_ReadRegister(ehci->regs.frame_index, &now);
            now = now % mod;
            if(now_uframe == now)
                break;

            /** rescan the rest of this frame, then ... */
            now_uframe = now;
            clock = now_uframe >> 3;
        } 
        else
            frame = (frame + 1) % ehci->periodic_size;
    } 

    _spin_unlock_irqrestore(&ehci->lock);
}
