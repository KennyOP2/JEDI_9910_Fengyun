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
 * EHCI hardware queue manipulation ... the core.  QH/QTD manipulation.
 *
 * Control, bulk, and interrupt traffic all use "qh" lists.  They list "qtd"
 * entries describing USB transactions, max 16-20kB/entry (with 4kB-aligned
 * buffers needed for the larger number).  We use one QH per endpoint, queue
 * multiple (bulk or control) urbs per endpoint.  URBs may need several qtds.
 * A scheduled interrupt qh always (for now) has one qtd, one urb.
 *
 * ISO traffic uses "ISO TD" (itd, and sitd) records, and (along with
 * interrupts) needs careful scheduling.  Performance improvements can be
 * an ongoing challenge.  That's in "ehci-sched.c".
 * 
 * USB 1.1 devices are handled (a) by "companion" OHCI or UHCI root hubs,
 * or otherwise through transaction translators (TTs) in USB 2.0 hubs using
 * (b) special fields in qh entries or (c) split iso entries.  TTs will
 * buffer low/full speed data so the host collects it at high speed.
 */
/*-------------------------------------------------------------------------*/

//=====================================================================
/** Function Name: qtd_fill
 * Description: fill a qtd, returning how much of the buffer we were able to queue up
 * Input: <1>.qtd: qTD Structure 
 *        <2>.buf: buffer address
 *        <3>.len: total length
 *        <4>.token: ioc + C_Page + Cerr + PIDCode
 */
//=====================================================================
static MMP_INT
qtd_fill(struct ehci_qtd* qtd, MMP_UINT8* buf, MMP_INT len, MMP_INT token)
{
    MMP_INT	i = 0;
    MMP_INT count = 0;
    MMP_UINT32 tmp_buf = (MMP_UINT32)buf - (MMP_UINT32)HOST_GetVramBaseAddress();
    MMP_UINT32 tmp_value;
    MMP_UINT32 offset;

    /* one buffer entry per 4K ... first might be short or unaligned */
    VMEM_STRUCT_W(ehci_qtd, qtd, hw_buf0, cpu_to_le32(tmp_buf));

    offset = count = 0x1000 - (tmp_buf & 0x0fff);	/* rest of that page */
    if(likely(len < count))		/* ... if needed */
        count = len;
    else 
    {
        tmp_buf +=  0x1000;
        tmp_buf &= ~0x0fff;

        /* per-qtd limit: from 16K to 20K (best alignment) */
        for(i=1; count<len && i<5; i++) 
        {
            switch(i) 
            {
            case 1:
                VMEM_STRUCT_W(ehci_qtd, qtd, hw_buf1, cpu_to_le32(tmp_buf));
                break;
            case 2:
                VMEM_STRUCT_W(ehci_qtd, qtd, hw_buf2, cpu_to_le32(tmp_buf));
                break;
            case 3:
                VMEM_STRUCT_W(ehci_qtd, qtd, hw_buf3, cpu_to_le32(tmp_buf));
                break;
            case 4:
                VMEM_STRUCT_W(ehci_qtd, qtd, hw_buf4, cpu_to_le32(tmp_buf));
                break;
            default:
                ithPrintf(" Fill qtd fail!!!! \n");
                break;
            };
            tmp_buf += 0x1000;
            if((count + 0x1000) < len)
            {
                count += 0x1000;
                /** Irene: let first qtd transfer size be multiple of 512, or first qtd will halted?? */
                if(offset && (i==4))
                    count &= ~0x1FF;
            }
            else
                count = len;
        }
    }
    //tmp_value = cpu_to_le32((count << 16) | token);
    //VMEM_STRUCT_W(ehci_qtd, qtd, hw_token, tmp_value);
    tmp_value = ((count << 16) | token);
    VMEM_STRUCT_W(ehci_qtd, qtd, hw_token, cpu_to_le32(tmp_value));
    VMEM_STRUCT_W(ehci_qtd, qtd, length, count);

    return count;
}

/* update halted (but potentially linked) qh */
static void qh_update(struct ehci_qh* qh, struct ehci_qtd* qtd)
{
    MMP_UINT32 tmp_value = 0;
    //MMP_UINT32 tmp_value1 = 0;
    MMP_UINT8* tmp_addr = MMP_NULL;
    LOG_DEBUG " qh_update() qh 0x%08X, qtd 0x%08X \n", qh, qtd LOG_END

    // Irene_20121121: has potential race condition.
    //VMEM_STRUCT_W(ehci_qh, qh, hw_current, tmp_value);

    VMEM_STRUCT_R(ehci_qtd, qtd, qtd_addr, &tmp_addr);
    tmp_value = QTD_NEXT(tmp_addr);
    VMEM_STRUCT_W(ehci_qh, qh, hw_qtd_next, tmp_value);

    tmp_value = EHCI_LIST_END;
    VMEM_STRUCT_W(ehci_qh, qh, hw_alt_next, tmp_value);

    /* HC must see latest qtd and qh data before we clear ACTIVE+HALT */
/** These codes will cause race condition!! */
#if defined(__FREERTOS__) || defined(__OPENRTOS__)
#if 0  
    tmp_value = (QTD_TOGGLE | QTD_STS_PING);
    tmp_value = cpu_to_le32(tmp_value);
    //VMEM_STRUCT_R(ehci_qh, qh, hw_token, &tmp_value1); /** for clear cache. NOT WORK!!! */
    qh->hw_token &= tmp_value;
#endif // #if 0
#else
    /** 
     * Irene NOTE: Important!!!!!!!!
     * If the usb device's respoinse is too slow, this token will error.... and then HC will error!~~~ 
     */
    #if 0
    #if 0
    VMEM_STRUCT_R(ehci_qh, qh, hw_token, &tmp_value);
    tmp_value = le32_to_cpu(tmp_value);
    tmp_value = tmp_value & (QTD_TOGGLE | QTD_STS_PING);
    VMEM_STRUCT_W(ehci_qh, qh, hw_token, cpu_to_le32(tmp_value));
    #else
    {
        MMP_UINT32 mask = cpu_to_le32((QTD_TOGGLE | QTD_STS_PING));
        VMEM_STRUCT_R(ehci_qh, qh, hw_token, &tmp_value);
        tmp_value &= mask;
        VMEM_STRUCT_W(ehci_qh, qh, hw_token, tmp_value);
    }
    #endif
    #endif
#endif
}


/*-------------------------------------------------------------------------*/
/** Copy the Status from token to urb */
static void qtd_copy_status(struct urb *urb, MMP_UINT8* buf_addr, MMP_INT length, MMP_UINT32 token)
{
    MMP_INT tx_length = length - QTD_LENGTH(token);
    #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
    struct usb_hcd* hcd = urb->dev->bus->hcpriv;
    struct ehci_hcd* ehci = hcd_to_ehci(hcd);

    if(likely(QTD_PID(token) == 2))
    {
        ehci_20kbuf_free(ehci, buf_addr);
    }
    #endif

    /* count IN/OUT bytes, not SETUP (even short packets) */
    if(likely(QTD_PID(token) != 2))
    {
        if(usb_pipein(urb->pipe) && tx_length)
        {
            #if defined(__FREERTOS__) || defined(__OPENRTOS__)
            ithInvalidateDCacheRange((void*)(buf_addr), tx_length);
            #else
            MMP_UINT32 dstAddr = (MMP_UINT32)urb->transfer_buffer + (MMP_UINT32)urb->actual_length;
            HOST_ReadBlockMemory(dstAddr, (MMP_UINT32)buf_addr, tx_length);
            #endif
        }
        #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
        if(length)
        {
            ehci_20kbuf_free(ehci, buf_addr);
        }
        #endif
        //LOG_DEBUG " qtd_copy_status() tx_length = 0x%X \n", tx_length LOG_END
        urb->actual_length += tx_length;
    }

    /* don't modify error codes */
    if(unlikely(urb->status == -EINPROGRESS && (token & QTD_STS_HALT))) 
    {
        if(token & QTD_STS_BABBLE) 
        {
            /* FIXME "must" disable babbling device's port too */
            urb->status = -EOVERFLOW;
        } 
        else if(token & QTD_STS_MMF) 
        {
            /* fs/ls interrupt xfer missed the complete-split */
            urb->status = -EPROTO;
        } 
        else if(token & QTD_STS_DBE) 
        {
            urb->status = (QTD_PID(token) == 1) /* IN ? */
                ? -ENOSR  /* hc couldn't read data */
                : -ECOMM; /* hc couldn't write data */
        } 
        else if(token & QTD_STS_XACT) 
        {
            /* timeout, bad crc, wrong PID, etc; retried */
            if(QTD_CERR(token))
                urb->status = -EPIPE;
            else 
            {
                LOG_ERROR " 3strikes \n" LOG_END
                urb->status = -EPROTO;
            }
        /* CERR nonzero + no errors + halt --> stall */
        } 
        else if(QTD_CERR(token))
            urb->status = -EPIPE;
        else	/* unknown */
            urb->status = -EPROTO;

        LOG_ERROR "ep %d-%s qtd token %08x --> status %d \n",
            /* devpath */
            usb_pipeendpoint(urb->pipe),
            usb_pipein (urb->pipe) ? "in" : "out",
            token, urb->status LOG_END

        /* stall indicates some recovery action is needed */
        if(urb->status == -EPIPE) 
        {
            MMP_INT	pipe = urb->pipe;

            if(!usb_pipecontrol(pipe))
            {
                usb_endpoint_halt(urb->dev, usb_pipeendpoint(pipe), usb_pipeout (pipe));
            }
            /** Irene TODO ?? */
            #if 0
            if(urb->dev->tt && !usb_pipeint (pipe)) 
            {
                LOG_ERROR "must CLEAR_TT_BUFFER, hub port %d%s addr %d ep %d",
                    urb->dev->ttport, /* devpath */
                    urb->dev->tt->multi ? "" : " (all-ports TT)",
                    urb->dev->devnum, usb_pipeendpoint (urb->pipe) LOG_END
                // FIXME something (khubd?) should make the hub
                // CLEAR_TT_BUFFER ASAP, it's blocking other
                // fs/ls requests... hub_tt_clear_buffer() ?
            }
            #endif
        }
    }
}

static void ehci_urb_complete(
    struct ehci_hcd* ehci,
    struct urb*		 urb)
{
    LOG_ENTER " ehci_urb_complete() ehci 0x%08X urb 0x%08X \n", ehci, urb LOG_END

    /* cleanse status if we saw no error */
    if(likely(urb->status == -EINPROGRESS)) 
    {
        if((urb->actual_length != urb->transfer_buffer_length) && (urb->transfer_flags & USB_DISABLE_SPD))
            urb->status = -EREMOTEIO;
        else
            urb->status = 0;
    }

    /* only report unlinks once */
    if(likely(urb->status != -ENOENT && urb->status != -ENOTCONN))
        urb->complete(urb);

    LOG_LEAVE " ehci_urb_complete() \n" LOG_END
}

/* urb->lock ignored from here on (hcd is done with urb) */
static void ehci_urb_done(
    struct ehci_hcd* ehci,
    struct urb* urb)
{
    LOG_ENTER " ehci_urb_done() ehci 0x%08X urb 0x%08X, \n", ehci, urb LOG_END

    if(likely(urb->hcpriv != 0)) 
    {
        qh_put(ehci, (struct ehci_qh*)urb->hcpriv);
        urb->hcpriv = 0;
    }

    if(likely(urb->status == -EINPROGRESS)) 
    {
        if((urb->actual_length != urb->transfer_buffer_length) && (urb->transfer_flags & USB_DISABLE_SPD))
            urb->status = -EREMOTEIO;
        else
            urb->status = 0;
    }

    /* hand off urb ownership */
    usb_hcd_giveback_urb(&ehci->hcd, urb);

    LOG_LEAVE " ehci_urb_done() \n" LOG_END
}

//=====================================================================
/*
 * Process completed qtds for a qh, issuing completions if needed.
 * When freeing:  frees qtds, unmaps buf, returns URB to driver.
 * When not freeing (queued periodic qh):  retain qtds, mapping, and urb.
 * Races up to qh->hw_current; returns number of urb completions.
 */
//=====================================================================
/* caller must own ehci->lock */
static MMP_INT
qh_completions(
    struct ehci_hcd*	ehci,
    struct ehci_qh*		qh,
    MMP_INT			    freeing)
{
    struct ehci_qtd		*qtd, *last;
    struct list_head	*next, *qtd_list, *tmp_list_head;
    MMP_UINT8 *addr;
    MMP_INT	unlink = 0, halted = 0;
    MMP_INT	retval = 0; /** urb done number */
    struct urb *urb, *last_urb;
    MMP_UINT32 tmp_hw_token, tmp_qh_state, tmp_value, addr1, addr2;

    #if 1 //#ifdef CONFIG_FARADAY_FEHCI
    MMP_INT orig_freeing = freeing;
    #endif

    LOG_ENTER " qh_completions() ehci 0x%08X, qh = 0x%08X \n", ehci, qh LOG_END

    VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&qtd_list);

    //_spin_lock_irqsave(&ehci->lock);
    if(unlikely(LIST_EMPTY_VMEM(qtd_list))) 
    {
        //_spin_unlock_irqrestore(&ehci->lock);
        goto end;
    }

    VMEM_STRUCT_R(list_head, qtd_list, next, &tmp_list_head);
    /* scan QTDs till end of list, or we reach an active one */
    for(qtd = list_entry(tmp_list_head, struct ehci_qtd, qtd_list), last=0, next=0; 
        next != qtd_list;
        last = qtd, qtd = list_entry(next, struct ehci_qtd, qtd_list))
    {
        MMP_UINT32 token = 0;

        VMEM_STRUCT_R(ehci_qtd, qtd, urb, &urb);

        /* clean up any state from previous QTD ...*/
        if(last) 
        {
            VMEM_STRUCT_R(ehci_qtd, last, urb, &last_urb);
            if(likely(last_urb != urb)) 
            {
                /* complete() can reenter this HCD */
                _spin_unlock_irqrestore(&ehci->lock);

                if(likely(freeing != 0))
                    ehci_urb_done(ehci, last_urb);
                else
                    ehci_urb_complete(ehci, last_urb);

                _spin_lock_irqsave(&ehci->lock);
                retval++;
            }

            /* qh overlays can have HC's old cached copies of
             * next qtd ptrs, if an URB was queued afterwards.
             */
            VMEM_STRUCT_R(ehci_qh, qh, hw_current, &tmp_value);
            addr = (MMP_UINT8*)(le32_to_cpu((MMP_UINT32)tmp_value) + (MMP_UINT32)HOST_GetVramBaseAddress());
            VMEM_STRUCT_R(ehci_qtd, last, hw_next, &addr1);
            VMEM_STRUCT_R(ehci_qh, qh, hw_qtd_next, &addr2);
            if(((MMP_UINT8*)last == addr) && (addr1 != addr2))
            {
                VMEM_STRUCT_W(ehci_qh, qh, hw_qtd_next, addr1);
                VMEM_STRUCT_R(ehci_qtd, last, hw_alt_next, &addr2);
                VMEM_STRUCT_W(ehci_qh, qh, hw_alt_next, addr2);
            }

            if(likely(freeing != 0))
                ehci_qtd_free(ehci, last);
            last = 0;
        }
        //next = qtd->qtd_list.next;
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&tmp_list_head);
        VMEM_STRUCT_R(list_head, tmp_list_head, next, &next);


        /* QTDs at tail may be active if QH+HC are running,
         * or when unlinking some urbs queued to this QH
         */
        VMEM_STRUCT_R(ehci_qtd, qtd, hw_token, &token);
        token = le32_to_cpu(token);
        {
            VMEM_STRUCT_R(ehci_qh, qh, hw_token, &tmp_hw_token);
            VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_qh_state);
            tmp_hw_token = le32_to_cpu(tmp_hw_token);

            halted = ( (halted) || 
                       (tmp_hw_token & QTD_STS_HALT) ||
                       (ehci->hcd.state == USB_STATE_HALT) ||
                       (tmp_qh_state == QH_STATE_IDLE) );
            if(halted)
            {
                LOG_DEBUG " halted!=0 => tmp_hw_token 0x%08X, hcd_state %d, qh_state %d \n", 
                    tmp_hw_token, ehci->hcd.state, tmp_qh_state LOG_END
                #if defined(DUMP_QH)
                {
                    ithPrintf(" qh = 0x%08X \n", qh);
                    dumpVram((void*)qh, sizeof(struct ehci_qh));
                    VMEM_STRUCT_R(ehci_qh, qh, hw_current, &tmp_value);
                    ithPrintf(" qh->hw_current = 0x%08X \n", tmp_value);
                    tmp_value = le32_to_cpu(tmp_value) + (MMP_UINT32)HOST_GetVramBaseAddress();
                    dumpVram((void*)tmp_value, sizeof(struct ehci_qtd));
                    dumpReg(ehci);
                }
                #endif
            }
        }


        /* fault: unlink the rest, since this qtd saw an error? */
        if(unlikely((token & QTD_STS_HALT) != 0)) 
        {
            LOG_ERROR " qh_completions() qtd halted! token 0x%08X \n", token LOG_END
            if(orig_freeing == 0)  //Interrupt, Keep qTD while removing Hub
               unlink = 1;  
            else
               freeing = unlink = 1;   			

            /* status copied below */

        }  /* QH halts only because of fault (above) or unlink (here). */
        else if(unlikely(halted != 0)) 
        {
            /* unlinking everything because of HC shutdown? */
            if(ehci->hcd.state == USB_STATE_HALT) 
            {
                LOG_ERROR " qh_completions() qh halted! USB_STATE_HALT\n" LOG_END
                freeing = unlink = 1;
            } /* explicit unlink, maybe starting here? */ 
            else if((tmp_qh_state == QH_STATE_IDLE)	&& 
                    (urb->status == -ECONNRESET	|| urb->status == -ENOENT)) 
            {
                LOG_ERROR " qh_completions() qh halted! QH_STATE_IDLE => unlink\n" LOG_END
                freeing = unlink = 1;
            } /* QH halted to unlink urbs _after_ this?  */ 
            else if(!unlink && (token & QTD_STS_ACTIVE) != 0) 
            {
                LOG_DEBUG " qh_completions() qh halted! QTD_STS_ACTIVE\n" LOG_END
                next = qtd_list;
                qtd = 0;
                continue;
            }

            /* unlink the rest?  once we start unlinking, after
             * a fault or explicit unlink, we unlink all later
             * urbs.  usb spec requires that.
             */
            if(unlink && urb->status == -EINPROGRESS)
            {
                urb->status = -ECONNRESET;
            }

        } 
        /* Else QH is active, so we must not modify QTDs
         * that HC may be working on.  No more qtds to check.
         */
        else if(unlikely(!unlink && (token & QTD_STS_ACTIVE) != 0)) 
        {
            LOG_DEBUG " qh_completions() still has qtd active! \n" LOG_END
            next = qtd_list;
            qtd = 0;
            continue;
        }

        VMEM_STRUCT_R(ehci_qtd, qtd, buf_addr, &addr);
        VMEM_STRUCT_R(ehci_qtd, qtd, length, &tmp_value);
        //_spin_lock(&urb->lock);
        qtd_copy_status(urb, addr, tmp_value, token);
        //_spin_unlock(&urb->lock);

        /*
         * NOTE:  this won't work right with interrupt urbs that
         * need multiple qtds ... only the first scan of qh->qtd_list
         * starts at the right qtd, yet multiple scans could happen
         * for transfers that are scheduled across multiple uframes. 
         * (Such schedules are not currently allowed!)
         */
        if(likely(freeing != 0))
        {
            //list_del(&qtd->qtd_list);
            struct list_head* tmp_qtd_list;
            VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&tmp_qtd_list);
            LIST_DEL_VMEM_VV(tmp_qtd_list);
        }
        else 
        {
            /**
             * restore everything the HC could change
             * from an interrupt QTD
             */
            MMP_UINT32 hw_token;
            MMP_UINT32 length;
            MMP_UINT32 hw_buf0;
            MMP_UINT8* buf_addr;

            /** restore hw_token and active qtd again */
            VMEM_STRUCT_R(ehci_qtd, qtd, hw_token, &hw_token);
            hw_token = le32_to_cpu(hw_token);
            VMEM_STRUCT_R(ehci_qtd, qtd, length, &length);

            hw_token = (hw_token & 0x8300) |
                       (length << 16) |
                       QTD_STS_ACTIVE |
                       (EHCI_TUNE_CERR << 10);
            VMEM_STRUCT_W(ehci_qtd, qtd, hw_token, cpu_to_le32(hw_token));

            /** restore buffer 0 offset */
            VMEM_STRUCT_R(ehci_qtd, qtd, hw_buf0, &hw_buf0);
            hw_buf0 = le32_to_cpu(hw_buf0);
            VMEM_STRUCT_R(ehci_qtd, qtd, buf_addr, &buf_addr);
            hw_buf0 = (hw_buf0 & ~0x0FFF) | (0x0FFF & ((MMP_UINT32)buf_addr-(MMP_UINT32)HOST_GetVramBaseAddress()));
            VMEM_STRUCT_W(ehci_qtd, qtd, hw_buf0, cpu_to_le32(hw_buf0));
        }

        if(urb->status == -EINPROGRESS)
        {
            LOG_DEBUG " qtd %08X ok, urb %08X, token %08X, len 0x%X \n",
                qtd, urb, token, urb->actual_length LOG_END
        }
        else
        {
            LOG_DEBUG "urb %08X status %d, qtd %08X, token %8X, len 0x%X \n",
                urb, urb->status, qtd, token,
                urb->actual_length LOG_END
        }
    }

    /* patch up list head? */
    if(unlikely(halted && !LIST_EMPTY_VMEM(qtd_list))) 
    {
        LOG_DEBUG " qh_completions() => halted && qtd_list not empty! \n" LOG_END
        VMEM_STRUCT_R(list_head, qtd_list, next, &tmp_list_head);
        qh_update(qh, list_entry(tmp_list_head, struct ehci_qtd, qtd_list));
    }
    //_spin_unlock_irqrestore(&ehci->lock);

    /* last urb's completion might still need calling */
    if(likely(last != 0)) 
    {
        VMEM_STRUCT_R(ehci_qtd, last, urb, &last_urb);
        if(likely(freeing != 0)) 
        {
            ehci_urb_done(ehci, last_urb);
            ehci_qtd_free(ehci, last);
        } 
        else
        {
            ehci_urb_complete(ehci, last_urb);
        }
        retval++;
    }

    /**
     * Irene: clear qh halt! 
     *        Originally it should be in qh_update()!!!!
     */
    if(tmp_hw_token & QTD_STS_HALT)
    {
        tmp_hw_token &= ~0xFE;
        tmp_hw_token &= ~0x80000000; /** clear data toggle bit */
        VMEM_STRUCT_W(ehci_qh, qh, hw_token, cpu_to_le32(tmp_hw_token));
    }

end:
    LOG_LEAVE " qh_completions() \n" LOG_END
    return retval;
}

//=====================================================================
/*
 * reverse of qh_urb_transaction:  free a list of TDs.
 * used for cleanup after errors, before HC sees an URB's TDs.
 */
//=====================================================================
static void qtd_list_free(
    struct ehci_hcd*	ehci,
    struct urb*		    urb,
    struct list_head*	qtd_list,
    MMP_BOOL head_in_vram)
{
    struct list_head	*entry, *temp;
    struct list_head*   qtd_list_tmp;

    LOG_DEBUG " qtd_list_free() ehci 0x%08X, urb 0x%08X \n", ehci, urb LOG_END

#if 0
    /** only head qtd_list is in system memory, others are in video memory */
    //for(entry=qtd_list->next, temp=entry->next; entry!=qtd_list; entry=temp, temp=entry->next)
    for(entry=qtd_list->next; entry!=qtd_list; )
    {
        struct ehci_qtd	*qtd;
        MMP_UINT8* addr;

        VMEM_STRUCT_R(list_head, entry, next, &temp);
        qtd = list_entry(entry, struct ehci_qtd, qtd_list);
        //list_del(&qtd->qtd_list);
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        LIST_DEL_VMEM_SV(qtd_list, qtd_list_tmp);
        VMEM_STRUCT_R(ehci_qtd, qtd, buf_addr, &addr);
        #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
        if(addr - (MMP_UINT32)HOST_GetVramBaseAddress())
            ehci_20kbuf_free(ehci, addr);
        #endif
        ehci_qtd_free(ehci, qtd);

        entry = temp;
    }
#else
    if(head_in_vram)
    { /** list head is also in video memory */
        VMEM_STRUCT_R(list_head, qtd_list, next, &entry);
    }
    else
    { /** only head qtd_list is in system memory, others are in video memory */
        entry = qtd_list->next;
    }

    //for(entry=qtd_list->next, temp=entry->next; entry!=qtd_list; entry=temp, temp=entry->next)
    for(; entry!=qtd_list; )
    {
        struct ehci_qtd	*qtd;
        MMP_UINT8* addr;

        VMEM_STRUCT_R(list_head, entry, next, &temp);
        qtd = list_entry(entry, struct ehci_qtd, qtd_list);
        #if 0
        list_del(&qtd->qtd_list);
        #else
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        if(head_in_vram)
            LIST_DEL_VMEM_VV(qtd_list_tmp);
        else
            LIST_DEL_VMEM_SV(qtd_list, qtd_list_tmp);
        #endif

        VMEM_STRUCT_R(ehci_qtd, qtd, buf_addr, &addr);
        #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
        if(addr - (MMP_UINT32)HOST_GetVramBaseAddress())
            ehci_20kbuf_free(ehci, addr);
        #endif
        ehci_qtd_free(ehci, qtd);

        entry = temp;
    }
#endif
}

//=====================================================================
/*
 * Hardware maintains data toggle (like OHCI) ... here we (re)initialize
 * the hardware data toggle in the QH, and set the pseudo-toggle in udev
 * so we can see if usb_clear_halt() was called.  NOP for control, since
 * we set up qh->hw_info1 to always use the QTD toggle bits. 
 */
//=====================================================================
static void
clear_toggle(struct usb_device* udev, MMP_INT ep, MMP_INT is_out, struct ehci_qh *qh)
{
    MMP_UINT32 tmp_value = 0;
    LOG_DEBUG "clear toggle, dev %d ep 0x%x-%s \n", udev->devnum, ep, (is_out ? "out" : "in") LOG_END

    VMEM_STRUCT_R(ehci_qh, qh, hw_token, &tmp_value);
    tmp_value = le32_to_cpu(tmp_value);
    tmp_value = tmp_value & ~QTD_TOGGLE;
    VMEM_STRUCT_W(ehci_qh, qh, hw_token, cpu_to_le32(tmp_value));

    usb_settoggle(udev, ep, is_out, 1);
}

//=====================================================================
/*
 * Each QH holds a qtd list; a QH is used for everything except iso.
 *
 * For interrupt urbs, the scheduler must set the microframe scheduling
 * mask(s) each time the QH gets scheduled.  For highspeed, that's
 * just one microframe in the s-mask.  For split interrupt transactions
 * there are additional complications: c-mask, maybe FSTNs.
 *
 * Would be best to create all qh's from config descriptors,
 * when each interface/altsetting is established.  Unlink
 * any previous qh and cancel its urbs first; endpoints are
 * implicitly reset then (data toggle too).
 * That'd mean updating how usbcore talks to HCDs. (2.5?)
 */
//=====================================================================
static struct ehci_qh*
ehci_qh_make(
    struct ehci_hcd*	ehci,
    struct urb*		    urb,
    struct list_head*	qtd_list) 
{
    MMP_INT result = 0;
    struct ehci_qh*	qh = MMP_NULL;
    MMP_UINT32		info1 = 0, info2 = 0;
    MMP_UINT32      tmp_value = 0;
    struct list_head*   qtd_list_tmp;
    //LOG_ENTER " ehci_qh_make() ehci 0x%08X, urb 0x%08X \n", ehci, urb LOG_END

    qh = ehci_qh_alloc(ehci);
    if(!qh)
    {
        result = ERROR_USB_ALLOC_ONE_QH_FAIL;
        goto end;
    }

    /*
     * init endpoint/device data for this QH
     */
    info1 |= usb_pipeendpoint(urb->pipe) << 8;
    info1 |= usb_pipedevice(urb->pipe) << 0;

    /* using TT? */ //TT=> Low/Full Speed Device connect to High Speed Hub
    switch(urb->dev->speed) 
    {
    case USB_SPEED_LOW:
        LOG_INFO "### Code Path = Low Speed (Use TT)\n" LOG_END
        info1 |= (1 << 12);	/* EPS "low" */
        /* FALL THROUGH */

    case USB_SPEED_FULL:
        /* EPS 0 means "full" */
        LOG_INFO "### Code Path = Full Speed(Use TT) ?? check code~~~~ \n" LOG_END
#if 0
        info1 |= (EHCI_TUNE_RL_TT << 28);
        if(usb_pipecontrol(urb->pipe)) 
        {
            info1 |= (1 << 27);	/* for TT */
            info1 |= 1 << 14;	/* toggle from qtd */
        }
        info1 |= usb_maxpacket(urb->dev, urb->pipe,	usb_pipeout(urb->pipe)) << 16;

        info2 |= (EHCI_TUNE_MULT_TT << 30);
        info2 |= urb->dev->ttport << 23;
        info2 |= urb->dev->tt->hub->devnum << 16;
#else
        if(usb_pipecontrol(urb->pipe)) 
        {
            info1 |= 1 << 14;	/* toggle from qtd */
        }
        info1 |= usb_maxpacket(urb->dev, urb->pipe,	usb_pipeout(urb->pipe)) << 16;
#endif
        /* NOTE:  if (usb_pipeint (urb->pipe)) { scheduler sets c-mask }
         * ... and a 0.96 scheduler might use FSTN nodes too
         */
        break;

    case USB_SPEED_HIGH:		/* no TT involved */
        LOG_INFO "### Code Path = High Speed \n" LOG_END
        info1 |= (2 << 12);	/* EPS "high" */
        info1 |= (EHCI_TUNE_RL_HS << 28);
        if(usb_pipecontrol(urb->pipe)) 
        {
            info1 |= 64 << 16;	/* usb2 fixed maxpacket */
            info1 |= 1 << 14;	/* toggle from qtd */
            info2 |= (EHCI_TUNE_MULT_HS << 30);
        } 
        else if(usb_pipebulk(urb->pipe)) 
        {
            info1 |= 512 << 16;	/* usb2 fixed maxpacket */
            info2 |= (EHCI_TUNE_MULT_HS << 30);
        } 
        else /** Interrupt pipe */
        {
            MMP_UINT32	temp;
            temp = usb_maxpacket(urb->dev, urb->pipe, usb_pipeout(urb->pipe));
            info1 |= (temp & 0x3ff) << 16;	/* maxpacket */
            /* HS intr can be "high bandwidth" */
            temp = 1 + ((temp >> 11) & 0x03);
            info2 |= temp << 30;		/* mult */
        }
        break;
    default:
        LOG_ERROR " ehci_qh_make() BUG!! \n" LOG_END
        while(1);
    }

    /* NOTE:  if (usb_pipeint (urb->pipe)) { scheduler sets s-mask } */
    tmp_value = QH_STATE_IDLE;
    VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
    VMEM_STRUCT_W(ehci_qh, qh, hw_info1, cpu_to_le32(info1));
    VMEM_STRUCT_W(ehci_qh, qh, hw_info2, cpu_to_le32(info2));

    /* initialize sw and hw queues with these qtds */
    //list_splice(qtd_list, &qh->qtd_list);
    VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
    LIST_SPLICE_VMEM(qtd_list, qtd_list_tmp);
    qh_update(qh, list_entry(qtd_list->next, struct ehci_qtd, qtd_list));

    /* initialize data toggle state */
    if(!usb_pipecontrol(urb->pipe))
    {
        clear_toggle(urb->dev,
                    usb_pipeendpoint(urb->pipe),
                    usb_pipeout(urb->pipe),
                    qh);
    }
#if 0
    if(0)
    {
        dumpVram((void*)qh, sizeof(struct ehci_qh));
    }
#endif

end:
    if(result)
        LOG_ERROR " ehci_qh_make() has error code 0x%08X \n", result LOG_END
    //LOG_LEAVE " ehci_qh_make() \n" LOG_END
    return qh;
}


//=====================================================================
/*
 * create a list of filled qtds for this URB; won't link into qh.
 */
//=====================================================================
static MMP_INT qh_urb_transaction(
    struct ehci_hcd*	ehci,
    struct urb*		    urb,
    struct list_head*	head) 
{
    MMP_INT result = 0;
    struct ehci_qtd*    qtd;
    struct ehci_qtd*    qtd_prev;
    struct list_head*   qtd_list_tmp;
    MMP_UINT32		token = 0, tmp;
    MMP_INT			len, maxpacket;
    MMP_UINT8*		buf = HOST_GetVramBaseAddress();
    #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
    MMP_INT         copy_len = 0;
    MMP_UINT8*      tmp_buf = MMP_NULL;
    #endif

    LOG_ENTER " qh_urb_transaction() ehci 0x%08X, urb 0x%08X \n", ehci, urb LOG_END

    /*
     * URBs map to sequences of QTDs:  one logical transaction
     */
    qtd = ehci_qtd_alloc(ehci);
    if(unlikely(!qtd))
    {
        result = ERROR_USB_ALLOC_ONE_QTD_FAIL;
        goto end;
    }

    qtd_prev = 0;
    //list_add_tail(&qtd->qtd_list, head);
    VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
    LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);
    VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);

    token = QTD_STS_ACTIVE;
    token |= (EHCI_TUNE_CERR << 10);

    if(usb_pipecontrol(urb->pipe)) 
    {
        MMP_UINT8* buf_addr = MMP_NULL;

        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        buf_addr = (MMP_UINT8*)urb->setup_packet;
        #if defined(DUMP_SETUP_CMD)
        LOG_DATA " setup: %02X %02X %02X %02X -  %02X %02X %02X %02X\n", buf_addr[0], buf_addr[1], buf_addr[2], buf_addr[3], buf_addr[4], buf_addr[5], buf_addr[6], buf_addr[7] LOG_END
        #endif
        #else
        buf_addr = ehci_20kbuf_alloc(ehci);
        HOST_WriteBlockMemory((MMP_UINT32)buf_addr, (MMP_UINT32)urb->setup_packet, 8);
        #endif
        /* control request data is passed in the "setup" pid */
        VMEM_STRUCT_W(ehci_qtd, qtd, buf_addr, buf_addr);

        /* SETUP pid */
        qtd_fill(qtd, buf_addr, sizeof(devrequest), token|(2 /* "setup" */ << 8));

        /* ... and always at least one more pid */
        token ^= QTD_TOGGLE;
        qtd_prev = qtd;
        qtd = ehci_qtd_alloc(ehci);
        if(unlikely(!qtd))
        {
            result = ERROR_USB_ALLOC_ONE_QTD_FAIL2;
            goto end;
        }
        VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
        tmp = QTD_NEXT(qtd);
        VMEM_STRUCT_W(ehci_qtd, qtd_prev, hw_next, tmp);
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);
    } 

    /*
     * data transfer stage:  buffer setup
     */
    len = urb->transfer_buffer_length;
    if(likely(len > 0)) 
    {
        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        buf = urb->transfer_buffer;
        #else
        tmp_buf = urb->transfer_buffer;
        buf = ehci_20kbuf_alloc(ehci);
        copy_len = (len > EHCI_20KBUF_SIZE) ? EHCI_20KBUF_SIZE : len;
        if(usb_pipeout(urb->pipe))
            HOST_WriteBlockMemory((MMP_UINT32)buf, (MMP_UINT32)tmp_buf, copy_len);
        tmp_buf += copy_len;
        #endif
    }

    if(!(buf - HOST_GetVramBaseAddress()) || usb_pipein(urb->pipe))
    {
        token |= (1 /* "in" */ << 8);
    }
    /* else it's already initted to "out" pid (0 << 8) */

    maxpacket = usb_maxpacket(urb->dev, urb->pipe, usb_pipeout(urb->pipe));

    /*
     * buffer gets wrapped in one or more qtds;
     * last one may be "short" (including zero len)
     * and may serve as a control status ack
     */
    for(;;) 
    {
        MMP_INT this_qtd_len;

        VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
        VMEM_STRUCT_W(ehci_qtd, qtd, buf_addr, buf);

        this_qtd_len = qtd_fill(qtd, buf, len, token);
        len -= this_qtd_len;
        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        buf += this_qtd_len;
        #else
        if(len > 0)
        {
            buf = ehci_20kbuf_alloc(ehci);
            copy_len = (len > EHCI_20KBUF_SIZE) ? EHCI_20KBUF_SIZE : len;
            if(usb_pipeout(urb->pipe))
                HOST_WriteBlockMemory((MMP_UINT32)buf, (MMP_UINT32)tmp_buf, copy_len);
            tmp_buf += copy_len;
        }
        #endif

        /* qh makes control packets use qtd toggle; maybe switch it */
        if((maxpacket & (this_qtd_len + (maxpacket - 1))) == 0)
            token ^= QTD_TOGGLE;

        if(likely(len <= 0))
            break;

        qtd_prev = qtd;
        qtd = ehci_qtd_alloc(ehci);
        if(unlikely(!qtd))
        {
            result = ERROR_USB_ALLOC_ONE_QTD_FAIL2;
            goto end;
        }
        VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
        tmp = QTD_NEXT(qtd);
        VMEM_STRUCT_W(ehci_qtd, qtd_prev, hw_next, tmp);
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);
    }

    /*
     * control requests may need a terminating data "status" ack;
     * bulk ones may need a terminating short packet (zero length).
     */
    if(likely((buf - HOST_GetVramBaseAddress()) != 0)) 
    {
        MMP_INT	one_more = 0;

        if(usb_pipecontrol(urb->pipe)) 
        {
            one_more = 1;
            token ^= 0x0100;	/* "in" <--> "out"  */
            token |= QTD_TOGGLE;	/* force DATA1 */
        } 
        else if(usb_pipebulk(urb->pipe)
                && (urb->transfer_flags & USB_ZERO_PACKET)
                && !(urb->transfer_buffer_length % maxpacket)) 
        {
            one_more = 1;
        }
        if(one_more) 
        {
            qtd_prev = qtd;
            qtd = ehci_qtd_alloc(ehci);
            if(unlikely(!qtd))
            {
                result = ERROR_USB_ALLOC_ONE_QTD_FAIL2;
                goto end;
            }
            VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
            tmp = QTD_NEXT(qtd);
            VMEM_STRUCT_W(ehci_qtd, qtd_prev, hw_next, tmp);
            VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
            LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);

            /* never any data in such packets */
            qtd_fill(qtd, HOST_GetVramBaseAddress()/*0*/, 0, token);
        }
    }

    /* by default, enable interrupt on urb completion */
    if(likely(!(urb->transfer_flags & URB_NO_INTERRUPT)))
    {
        VMEM_STRUCT_R(ehci_qtd, qtd, hw_token, &tmp);
        tmp |= cpu_to_le32(QTD_IOC);
        VMEM_STRUCT_W(ehci_qtd, qtd, hw_token, tmp);
    }
#if 0
    if(0)
    {
        dumpVram((MMP_UINT8*)qtd, sizeof(struct ehci_qtd));
    }
#endif
#if defined(DUMP_QTD)
    if(0)
    {
        struct list_head* tt = head->next;
        struct ehci_qtd* qtd_t = list_entry(tt, struct ehci_qtd, qtd_list);
        dumpVram((MMP_UINT8*)qtd_t, sizeof(struct ehci_qtd));

        VMEM_STRUCT_R(list_head, tt, next, &tt);
        qtd_t = list_entry(tt, struct ehci_qtd, qtd_list);
        dumpVram((MMP_UINT8*)qtd_t, sizeof(struct ehci_qtd));

        VMEM_STRUCT_R(list_head, tt, next, &tt);
        qtd_t = list_entry(tt, struct ehci_qtd, qtd_list);
        dumpVram((MMP_UINT8*)qtd_t, sizeof(struct ehci_qtd));
    }
#endif

end:
    if(result)
    {
        qtd_list_free(ehci, urb, head, 0); /** 0: head in system memory */
        LOG_ERROR " qh_urb_transaction() return error code 0x%08X \n", result LOG_END
    }
    LOG_LEAVE " qh_urb_transaction() \n" LOG_END
    return result;
}

#if defined(USB_LOGO_TEST)
static MMP_INT qh_urb_transaction_step(
    struct ehci_hcd*	ehci,
    struct urb*		    urb,
    struct list_head*	head) 
{
    MMP_INT result = 0;
    struct ehci_qtd*    qtd;
    struct ehci_qtd*    qtd_prev;
    struct list_head*   qtd_list_tmp;
    MMP_UINT32		token = 0, tmp;
    MMP_INT			len, maxpacket;
    MMP_UINT8*		buf = HOST_GetVramBaseAddress();
    #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
    MMP_INT         copy_len = 0;
    MMP_UINT8*      tmp_buf = MMP_NULL;
    #endif

    LOG_ENTER " qh_urb_transaction_step() ehci 0x%08X, urb 0x%08X \n", ehci, urb LOG_END

    /*
     * URBs map to sequences of QTDs:  one logical transaction
     */
    qtd = ehci_qtd_alloc(ehci);
    if(unlikely(!qtd))
    {
        result = ERROR_USB_ALLOC_ONE_QTD_FAIL;
        goto end;
    }

    qtd_prev = 0;
    //list_add_tail(&qtd->qtd_list, head);
    VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
    LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);
    VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);

    token = QTD_STS_ACTIVE;
    token |= (EHCI_TUNE_CERR << 10);

    if(urb->transfer_flags & USB_SINGLE_STEP_0)
        goto single_step0;
    if(urb->transfer_flags & USB_SINGLE_STEP_1)
        goto single_step1;
    if(urb->transfer_flags & USB_SINGLE_STEP_2)
        goto single_step2;

single_step0:

    if(usb_pipecontrol(urb->pipe)) 
    {
        MMP_UINT8* buf_addr = MMP_NULL;

        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        buf_addr = (MMP_UINT8*)urb->setup_packet;
        #if defined(DUMP_SETUP_CMD)
        LOG_DATA " setup: %02X %02X %02X %02X -  %02X %02X %02X %02X\n", buf_addr[0], buf_addr[1], buf_addr[2], buf_addr[3], buf_addr[4], buf_addr[5], buf_addr[6], buf_addr[7] LOG_END
        #endif
        #else
        buf_addr = ehci_20kbuf_alloc(ehci);
        HOST_WriteBlockMemory((MMP_UINT32)buf_addr, (MMP_UINT32)urb->setup_packet, 8);
        #endif
        /* control request data is passed in the "setup" pid */
        VMEM_STRUCT_W(ehci_qtd, qtd, buf_addr, buf_addr);

        /* SETUP pid */
        qtd_fill(qtd, buf_addr, sizeof(devrequest), token|(2 /* "setup" */ << 8));

        goto single_step_end;
#if 0
        /* ... and always at least one more pid */
        token ^= QTD_TOGGLE;
        qtd_prev = qtd;
        qtd = ehci_qtd_alloc(ehci);
        if(unlikely(!qtd))
        {
            result = ERROR_USB_ALLOC_ONE_QTD_FAIL2;
            goto end;
        }
        VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
        tmp = QTD_NEXT(qtd);
        VMEM_STRUCT_W(ehci_qtd, qtd_prev, hw_next, tmp);
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);
#endif
    } 

single_step1:
    if(urb->transfer_buffer_length==0)
        goto single_step2;

    token ^= QTD_TOGGLE;

    /*
     * data transfer stage:  buffer setup
     */
    len = urb->transfer_buffer_length;
    if(likely(len > 0)) 
    {
        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        buf = urb->transfer_buffer;
        #else
        tmp_buf = urb->transfer_buffer;
        buf = ehci_20kbuf_alloc(ehci);
        copy_len = (len > EHCI_20KBUF_SIZE) ? EHCI_20KBUF_SIZE : len;
        if(usb_pipeout(urb->pipe))
            HOST_WriteBlockMemory((MMP_UINT32)buf, (MMP_UINT32)tmp_buf, copy_len);
        tmp_buf += copy_len;
        #endif
    }

    if(!(buf - HOST_GetVramBaseAddress()) || usb_pipein(urb->pipe))
    {
        token |= (1 /* "in" */ << 8);
    }
    /* else it's already initted to "out" pid (0 << 8) */

    maxpacket = usb_maxpacket(urb->dev, urb->pipe, usb_pipeout(urb->pipe));

    /*
     * buffer gets wrapped in one or more qtds;
     * last one may be "short" (including zero len)
     * and may serve as a control status ack
     */
    for(;;) 
    {
        MMP_INT this_qtd_len;

        VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
        VMEM_STRUCT_W(ehci_qtd, qtd, buf_addr, buf);

        this_qtd_len = qtd_fill(qtd, buf, len, token);
        len -= this_qtd_len;
        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        buf += this_qtd_len;
        #else
        if(len > 0)
        {
            buf = ehci_20kbuf_alloc(ehci);
            copy_len = (len > EHCI_20KBUF_SIZE) ? EHCI_20KBUF_SIZE : len;
            if(usb_pipeout(urb->pipe))
                HOST_WriteBlockMemory((MMP_UINT32)buf, (MMP_UINT32)tmp_buf, copy_len);
            tmp_buf += copy_len;
        }
        #endif

        goto single_step_end;

#if 0
        /* qh makes control packets use qtd toggle; maybe switch it */
        if((maxpacket & (this_qtd_len + (maxpacket - 1))) == 0)
            token ^= QTD_TOGGLE;

        if(likely(len <= 0))
            break;

        qtd_prev = qtd;
        qtd = ehci_qtd_alloc(ehci);
        if(unlikely(!qtd))
        {
            result = ERROR_USB_ALLOC_ONE_QTD_FAIL2;
            goto end;
        }
        VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
        tmp = QTD_NEXT(qtd);
        VMEM_STRUCT_W(ehci_qtd, qtd_prev, hw_next, tmp);
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);
#endif
    }


single_step2:
    if((urb->transfer_buffer_length > 0) || usb_pipein(urb->pipe))
    {
        token |= (1 /* "in" */ << 8);
    }
    token ^= 0x0100;	/* "in" <--> "out"  */
    token |= QTD_TOGGLE;	/* force DATA1 */
    qtd_fill(qtd, HOST_GetVramBaseAddress()/*0*/, 0, token);

#if 0
    /*
     * control requests may need a terminating data "status" ack;
     * bulk ones may need a terminating short packet (zero length).
     */
    if(likely((buf - HOST_GetVramBaseAddress()) != 0)) 
    {
        MMP_INT	one_more = 0;

        if(usb_pipecontrol(urb->pipe)) 
        {
            one_more = 1;
            token ^= 0x0100;	/* "in" <--> "out"  */
            token |= QTD_TOGGLE;	/* force DATA1 */
        } 
        else if(usb_pipebulk(urb->pipe)
                && (urb->transfer_flags & USB_ZERO_PACKET)
                && !(urb->transfer_buffer_length % maxpacket)) 
        {
            one_more = 1;
        }
        if(one_more) 
        {
            qtd_prev = qtd;
            qtd = ehci_qtd_alloc(ehci);
            if(unlikely(!qtd))
            {
                result = ERROR_USB_ALLOC_ONE_QTD_FAIL2;
                goto end;
            }
            VMEM_STRUCT_W(ehci_qtd, qtd, urb, urb);
            tmp = QTD_NEXT(qtd);
            VMEM_STRUCT_W(ehci_qtd, qtd_prev, hw_next, tmp);
            VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
            LIST_ADD_TAIL_VMEM_SV(qtd_list_tmp, head);

            /* never any data in such packets */
            qtd_fill(qtd, HOST_GetVramBaseAddress()/*0*/, 0, token);
        }
    }
#endif

single_step_end:

    /* by default, enable interrupt on urb completion */
    if(likely(!(urb->transfer_flags & URB_NO_INTERRUPT)))
    {
        VMEM_STRUCT_R(ehci_qtd, qtd, hw_token, &tmp);
        tmp |= cpu_to_le32(QTD_IOC);
        VMEM_STRUCT_W(ehci_qtd, qtd, hw_token, tmp);
    }
#if 0
    if(0)
    {
        dumpVram((MMP_UINT8*)qtd, sizeof(struct ehci_qtd));
    }
#endif

end:
    if(result)
    {
        qtd_list_free(ehci, urb, head, 0); /** 0: head in system memory */
        LOG_ERROR " qh_urb_transaction() return error code 0x%08X \n", result LOG_END
    }
    LOG_LEAVE " qh_urb_transaction() \n" LOG_END
    return result;
}
#endif

//=====================================================================
/*
 * move qh (and its qtds) onto async queue; maybe enable queue.
 */
//=====================================================================
static void qh_link_async(struct ehci_hcd *ehci, struct ehci_qh *qh)
{
    MMP_INT result = 0;
    MMP_UINT32  tmp_value;
    MMP_UINT8*  tmp_addr;
    struct ehci_qh	*q;

    if(unlikely(!(q = ehci->async))) 
    {
        MMP_UINT32	cmd = 0;
#if defined(ALPHA_SDK)
        struct timeval startT, endT;

        AHB_ReadRegister(ehci->regs.command, &cmd);

        gettimeofday(&startT, NULL);
        /* in case a clear of CMD_ASE didn't take yet */
        AHB_ReadRegister(ehci->regs.status, &tmp_value);
        while(tmp_value & STS_ASS)
        {
            MMP_USleep(100);
            AHB_ReadRegister(ehci->regs.status, &tmp_value);
            gettimeofday(&endT, NULL);
            if(itpTimevalDiff(&startT, &endT) > 15)
#else
        SYS_CLOCK_T lastTime;

        AHB_ReadRegister(ehci->regs.command, &cmd);

        lastTime = SYS_GetClock();
        /* in case a clear of CMD_ASE didn't take yet */
        AHB_ReadRegister(ehci->regs.status, &tmp_value);
        while(tmp_value & STS_ASS)
        {
            MMP_USleep(100);
            AHB_ReadRegister(ehci->regs.status, &tmp_value);
            if(SYS_GetDuration(lastTime) > 15)
#endif
            {
                result = ERROR_USB_QH_LINK_FAIL1;
                LOG_ERROR " ERROR_USB_QH_LINK_FAIL1! \n" LOG_END
                while(1);
            }
        }

        VMEM_STRUCT_R(ehci_qh, qh, hw_info1, &tmp_value);
        tmp_value = le32_to_cpu(tmp_value) | QH_HEAD; /* [4.8] */
        VMEM_STRUCT_W(ehci_qh, qh, hw_info1, cpu_to_le32(tmp_value));
        VMEM_STRUCT_W(ehci_qh, qh, qh_next, (void*)qh);
        VMEM_STRUCT_R(ehci_qh, qh, qh_addr, &tmp_addr);
        tmp_value = QH_NEXT(tmp_addr);
        VMEM_STRUCT_W(ehci_qh, qh, hw_next, tmp_value);
        ehci->async = qh;
        #if 0
        {
            printf(" qh = 0x%08X \n", qh);
            dumpVram((void*)qh, sizeof(struct ehci_qh));
            VMEM_STRUCT_R(ehci_qh, qh, hw_qtd_next, &tmp_value);
            printf(" qh->hw_qtd_next = 0x%08X \n", tmp_value);
            tmp_value = le32_to_cpu(tmp_value) + (MMP_UINT32)HOST_GetVramBaseAddress();
            dumpVram((void*)tmp_value, sizeof(struct ehci_qtd));
            VMEM_STRUCT_R(ehci_qtd, tmp_value, hw_next, &tmp_value);
            printf(" 1'st qtd->hw_next = 0x%08X \n", tmp_value);
            tmp_value = le32_to_cpu(tmp_value) + (MMP_UINT32)HOST_GetVramBaseAddress();
            dumpVram((void*)tmp_value, sizeof(struct ehci_qtd));
        }
        #endif
        AHB_WriteRegister(ehci->regs.async_next, ((MMP_UINT32)qh - (MMP_UINT32)HOST_GetVramBaseAddress()));
        cmd |= (CMD_ASE|CMD_RUN);
        AHB_WriteRegister(ehci->regs.command, cmd);
        ehci->hcd.state = USB_STATE_RUNNING;
        LOG_DEBUG " qh_link_async() ehci 0x%08X, qh 0x%08X (QH_HEAD)\n", ehci, qh LOG_END
        /* qtd completions reported later by interrupt */
    } 
    else 
    {
        /* splice right after "start" of ring */
        VMEM_STRUCT_R(ehci_qh, qh, hw_info1, &tmp_value);
        tmp_value = le32_to_cpu(tmp_value);
        tmp_value = tmp_value & ~(QH_HEAD);
        VMEM_STRUCT_W(ehci_qh, qh, hw_info1, cpu_to_le32(tmp_value)); /* [4.8] */

        VMEM_STRUCT_R(ehci_qh, q, qh_next, &tmp_addr);
        VMEM_STRUCT_W(ehci_qh, qh, qh_next, (void*)tmp_addr);
        VMEM_STRUCT_R(ehci_qh, q, hw_next, &tmp_value);
        VMEM_STRUCT_W(ehci_qh, qh, hw_next, tmp_value);
        VMEM_STRUCT_W(ehci_qh, q, qh_next, qh);

        tmp_value = QH_NEXT(qh);
        VMEM_STRUCT_W(ehci_qh, q, hw_next, tmp_value); /** link to hw */
        /* qtd completions reported later by interrupt */
        LOG_DEBUG " qh_link_async() ehci 0x%08X, qh 0x%08X \n", ehci, qh LOG_END
    }

    tmp_value = QH_STATE_LINKED;
    VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
    //LOG_LEAVE " qh_link_async() \n" LOG_END
}


static MMP_INT
submit_async(
    struct ehci_hcd*    ehci,
    struct urb*		    urb,
    struct list_head*	qtd_list) 
{
    MMP_INT result = 0;
    struct ehci_qtd*	qtd;
    struct hcd_dev*		dev;
    MMP_INT 			epnum;
    struct ehci_qh*		qh = 0;
    MMP_UINT32          tmp_value=0;

    LOG_ENTER " submit_async() ehci 0x%08X, urb 0x%08X \n", ehci, urb LOG_END

    /** head is in system memory, other are in video memory */
    qtd = list_entry(qtd_list->next, struct ehci_qtd, qtd_list);
    dev = (struct hcd_dev*)urb->dev->hcpriv;
    epnum = usb_pipeendpoint(urb->pipe);
    if(usb_pipein(urb->pipe))
        epnum |= 0x10;

    LOG_INFO " bus %d: submit_async urb 0x%08X len %d ep %d-%s qtd 0x%08X [qh 0x%08X] \n",
        ehci->hcd.index, urb, urb->transfer_buffer_length,
        epnum & 0x0f, ((epnum & 0x10) ? "in" : "out"),
        qtd, (dev ? dev->ep[epnum] : (void *)~0)  LOG_END

    _spin_lock_irqsave(&ehci->lock);

    qh = (struct ehci_qh*)dev->ep[epnum];
    if(likely(qh != 0)) 
    {
        MMP_UINT32 hw_next = QTD_NEXT(qtd);
        struct list_head* tmp_list_head;

        /* maybe patch the qh used for set_address */
        VMEM_STRUCT_R(ehci_qh, qh, hw_info1, &tmp_value);
        tmp_value = le32_to_cpu(tmp_value);
        #if defined(MS_ENUMERATE)
        if(unlikely(((epnum & 0xF) == 0) && ((tmp_value & 0x7f) == 0)))
        #else
        if(unlikely((epnum == 0) && ((tmp_value & 0x7f) == 0)))
        #endif
        {   /* set device address */
            tmp_value |= usb_pipedevice(urb->pipe);
            VMEM_STRUCT_W(ehci_qh, qh, hw_info1, cpu_to_le32(tmp_value));
        }

        /* is an URB is queued to this qh already? */
        VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&tmp_list_head);
        if(unlikely(!LIST_EMPTY_VMEM(tmp_list_head))) 
        {
            struct ehci_qtd		*last_qtd;
            struct list_head* tmp_list_head;
            struct urb* last_urb;
            MMP_INT			short_rx = 0;
            MMP_UINT32 tmp_value, tmp_value1;
            MMP_UINT8* tmp_addr;

            LOG_DEBUG " submit_async() this qh has urbs(qtd not empty)! \n" LOG_END

            /* update the last qtd's "next" pointer */
            VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&tmp_list_head);
            VMEM_STRUCT_R(list_head, tmp_list_head, prev, &tmp_list_head);
            last_qtd = list_entry(tmp_list_head, struct ehci_qtd, qtd_list);
            VMEM_STRUCT_W(ehci_qtd, last_qtd, hw_next, hw_next);

            /* previous urb allows short rx? maybe optimize. */
            VMEM_STRUCT_R(ehci_qtd, last_qtd, urb, &last_urb);
            if(!(last_urb->transfer_flags & USB_DISABLE_SPD) && (epnum & 0x10)) 
            {
                // only the last QTD for now
                VMEM_STRUCT_W(ehci_qtd, last_qtd, hw_alt_next, hw_next);
                last_qtd->hw_alt_next = hw_next;
                short_rx = 1;
            }

            /* Adjust any old copies in qh overlay too.
             * Interrupt code must cope with case of HC having it
             * cached, and clobbering these updates.
             * ... complicates getting rid of extra interrupts!
             */
            VMEM_STRUCT_R(ehci_qh, qh, hw_current, &tmp_value);
            tmp_value = le32_to_cpu(tmp_value);
            VMEM_STRUCT_R(ehci_qtd, last_qtd, qtd_addr, &tmp_addr);
            tmp_value1 = (MMP_UINT32)tmp_addr - (MMP_UINT32)HOST_GetVramBaseAddress();
            if(tmp_value == tmp_value1)
            {
                VMEM_STRUCT_W(ehci_qh, qh, hw_qtd_next, hw_next);
                if(short_rx)
                {
                    //qh->hw_alt_next = hw_next | (qh->hw_alt_next & 0x1e);
                    MMP_UINT32 tmp;
                    VMEM_STRUCT_R(ehci_qh, qh, hw_alt_next, &tmp);
                    tmp = le32_to_cpu(tmp);
                    tmp = tmp & 0x1E;
                    tmp = cpu_to_le32(tmp);
                    tmp = hw_next | tmp;
                    VMEM_STRUCT_W(ehci_qh, qh, hw_alt_next, tmp);
                }
                LOG_DEBUG "queue to qh 0x%08X, patch \n", qh LOG_END
            }
        } 
        else  /* no URB queued */
        {
            LOG_DEBUG " submit_async() this qh has no urb! \n" LOG_END

            // FIXME:  how handle usb_clear_halt() for an EP with queued URBs?
            // usbcore may not let us handle that cleanly...
            // likely must cancel them all first!

            /* usb_clear_halt() means qh data toggle gets reset */
            if(usb_pipebulk(urb->pipe) && unlikely(!usb_gettoggle(urb->dev,	(epnum & 0x0f),	!(epnum & 0x10)))) 
            {
                clear_toggle(urb->dev, (epnum & 0x0f), !(epnum & 0x10), qh);
            }
            qh_update(qh, qtd);
        }
        //list_splice (qtd_list, qh->qtd_list.prev);
        VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&tmp_list_head);
        VMEM_STRUCT_R(list_head, tmp_list_head, prev, &tmp_list_head);
        LIST_SPLICE_VMEM(qtd_list, tmp_list_head);
    }
    else
    {
        /* can't sleep here, we have ehci->lock... */
        qh = ehci_qh_make(ehci, urb, qtd_list);
        if(likely(qh != 0))
            dev->ep[epnum] = qh;
    }


    /** The control endpoint's maximum packet size will be modified after fist "Get_Descriptor" request.
        There the maxp of qH should be updated here. */
    if(usb_pipecontrol(urb->pipe))
    {
        MMP_UINT32 maxp = (usb_maxpacket(urb->dev, urb->pipe, !(usb_pipein(urb->pipe)))<<16);
        VMEM_STRUCT_R(ehci_qh, qh, hw_info1, &tmp_value); 
        tmp_value = le32_to_cpu(tmp_value);
        tmp_value = (tmp_value & ~0x07FF0000) | maxp;
        VMEM_STRUCT_W(ehci_qh, qh, hw_info1, cpu_to_le32(tmp_value)); 
    }

    /* Control/bulk operations through TTs don't need scheduling,
     * the HC and TT handle it when the TT has a buffer ready. ??
     */
    if(likely(qh != 0)) 
    {
        urb->hcpriv = qh_get(qh);
        VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_value); 
        if(likely(tmp_value == QH_STATE_IDLE))
            qh_link_async(ehci, qh_get(qh));
    }
    #if defined(DUMP_QH)
    {
        ithPrintf(" qh = 0x%08X \n", qh);
        dumpVram((void*)qh, sizeof(struct ehci_qh));
        VMEM_STRUCT_R(ehci_qh, qh, hw_current, &tmp_value);
        ithPrintf(" qh->hw_current = 0x%08X \n", le32_to_cpu(tmp_value));
        tmp_value = le32_to_cpu(tmp_value) + (MMP_UINT32)HOST_GetVramBaseAddress();
        dumpVram((void*)tmp_value, sizeof(struct ehci_qtd));
        if(0)
        {
            MMP_UINT32 src_addr=tmp_value;
            MMP_UINT32 dst_addr=(MMP_UINT32)&tmp_value;
            HOST_ReadBlockMemory(dst_addr, src_addr, 4);
            tmp_value = le32_to_cpu(tmp_value) & 0xFFFFFFFE;
            if(tmp_value)
            {
                tmp_value = tmp_value + (MMP_UINT32)HOST_GetVramBaseAddress();
                dumpVram((void*)tmp_value, sizeof(struct ehci_qtd));
            }
            src_addr=tmp_value;
            HOST_ReadBlockMemory(dst_addr, src_addr, 4);
            tmp_value = le32_to_cpu(tmp_value) & 0xFFFFFFFE;
            if(tmp_value)
            {
                tmp_value = tmp_value + (MMP_UINT32)HOST_GetVramBaseAddress();
                dumpVram((void*)tmp_value, sizeof(struct ehci_qtd));
            }
        }
        dumpReg(ehci);
    }
    #endif
    _spin_unlock_irqrestore(&ehci->lock);

    if(unlikely(qh == 0)) 
    {
        qtd_list_free(ehci, urb, qtd_list, 0);  /** 0: head in system memory */
        result = -ENOMEM;
        goto end; 
    }

end:
    if(result)
    {
        LOG_ERROR " submit_async() return error code 0x%08X \n", result LOG_END
        LOG_ERROR " bus %d: submit_async urb 0x%08X len %d ep %d-%s qtd 0x%08X [qh 0x%08X] \n",
            ehci->hcd.index, urb, urb->transfer_buffer_length,
            epnum & 0x0f, ((epnum & 0x10) ? "in" : "out"),
            qtd, (dev ? dev->ep[epnum] : (void *)~0)  LOG_END
    }
    LOG_LEAVE " submit_async() \n" LOG_END
    return result;
}


/*-------------------------------------------------------------------------*/

/* the async qh for the qtds being reclaimed are now unlinked from the HC */
/* caller must not own ehci->lock */

static void end_unlink_async(struct ehci_hcd *ehci)
{
    struct ehci_qh	*qh = ehci->reclaim;
    MMP_UINT32 tmp_value;
    struct list_head* tmp_list_head;

    LOG_INFO " end_unlink_async() ehci 0x%08X, qh 0x%08X \n", ehci, qh LOG_END
  
    _spin_lock_irqsave(&ehci->lock);
    tmp_value = QH_STATE_IDLE;
    VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
    tmp_value = 0;
    VMEM_STRUCT_W(ehci_qh, qh, qh_next, (void*)tmp_value);

    qh_put(ehci, qh); // refcount from reclaim 
    ehci->reclaim = 0;
    ehci->reclaim_ready = 0;

    qh_completions(ehci, qh, 1);

    // unlink any urb should now unlink all following urbs, so that
    // relinking only happens for urbs before the unlinked ones.
    VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&tmp_list_head);
    if(!LIST_EMPTY_VMEM(tmp_list_head) && HCD_IS_RUNNING(ehci->hcd.state))
        qh_link_async(ehci, qh);
    else
        qh_put(ehci, qh);		// refcount from async list
    _spin_unlock_irqrestore(&ehci->lock);

    LOG_LEAVE " end_unlink_async() \n" LOG_END
}

/*-------------------------------------------------------------------------*/

/* makes sure the async qh will become idle */
/* caller must own ehci->lock */
static void start_unlink_async(struct ehci_hcd *ehci, struct ehci_qh *qh)
{
    MMP_UINT32	cmd = 0;
    struct ehci_qh	*prev, *tmp_qh;
    MMP_UINT32 tmp_value;
    MMP_UINT8* tmp_addr=0;

    LOG_ENTER " start_unlink_async() ehci 0x%08X, qh 0x%08X \n", ehci, qh LOG_END

    AHB_ReadRegister(ehci->regs.command, &cmd);

    VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_value);
    if(ehci->reclaim || !ehci->async || (tmp_value != QH_STATE_LINKED))
    {
        LOG_ERROR " BUG!!! start_unlink_async() ehci->reclaim = 0x%08X, ehci->async = 0x%08X, qh 0x%08X, qh_state %d \n",
            ehci->reclaim, ehci->async, qh, tmp_value LOG_END
        while(1);
    }

    tmp_value = QH_STATE_UNLINK;
    VMEM_STRUCT_W(ehci_qh, qh, qh_state, tmp_value);
    ehci->reclaim = qh = qh_get(qh);

    /* Remove the last QH (qhead)?  Stop async schedule first. */
    VMEM_STRUCT_R(ehci_qh, qh, qh_next, &tmp_addr);
    if(unlikely((qh == ehci->async) && ((struct ehci_qh*)tmp_addr == qh)))
    {
        LOG_DEBUG " start unlink last qh 0x%08X \n", qh LOG_END

        /* can't get here without STS_ASS set */
        if(ehci->hcd.state != USB_STATE_HALT) 
        {
            if(cmd & CMD_PSE)
            {
                tmp_value = cmd & ~CMD_ASE;
                AHB_WriteRegister(ehci->regs.command, tmp_value);
            }
            else 
            {
            #if defined(ALPHA_SDK)
                struct timeval startT, endT;
                
                ehci_ready(ehci);
                
                gettimeofday(&startT, NULL);
                AHB_ReadRegister(ehci->regs.status, &tmp_value);
                while(tmp_value & (STS_ASS | STS_PSS))
                {
                    gettimeofday(&endT, NULL);
                    if(itpTimevalDiff(&startT, &endT) > 500)
            #else
                SYS_CLOCK_T lastTime;
                
                ehci_ready(ehci);

                lastTime = SYS_GetClock();
                AHB_ReadRegister(ehci->regs.status, &tmp_value);
                while(tmp_value & (STS_ASS | STS_PSS))
                {
                    if(SYS_GetDuration(lastTime) > 500)
            #endif
                    {
                        LOG_ERROR " start_unlink_async() -> ehci_ready() fail! ehci 0x%08X, qh 0x%08X \n", ehci, qh LOG_END
                        while(1);
                    }
                    
                    {   /** Irene: for device is removed */
                        AHB_ReadRegister(ehci->regs.command, &tmp_value);
                        if(!(tmp_value & CMD_RUN))
                            break;
                    }
                    MMP_USleep(100);
                    AHB_ReadRegister(ehci->regs.status, &tmp_value);
                }
            }
        }

        tmp_value = 0;
        VMEM_STRUCT_W(ehci_qh, qh, qh_next, (void*)tmp_value);
        ehci->async = 0;

        ehci->reclaim_ready = 1;
        tasklet_schedule(ehci);
        #if defined(USB_IRQ_ENABLE)
        {
            extern void*	isr_event;
            SYS_SetEvent(isr_event);
        }
        #endif
        goto end;
    } 
#if 0
    if(unlikely(ehci->hcd.state == USB_STATE_HALT)) 
    {
        LOG_DEBUG " start unlink qh with HCD HALT! \n" LOG_END
        ehci->reclaim_ready = 1;
        tasklet_schedule(ehci);
        goto end;
    }
#endif
    LOG_INFO " start unlink qh 0x%08X from qh list! \n", qh LOG_END
    prev = ehci->async;
    #if 0
    while(prev->qh_next.qh != qh && prev->qh_next.qh != ehci->async)
        prev = prev->qh_next.qh;
    #else
    VMEM_STRUCT_R(ehci_qh, prev, qh_next, (void**)&tmp_qh);
    while((tmp_qh != qh) && (tmp_qh != ehci->async))
    {
        VMEM_STRUCT_R(ehci_qh, prev, qh_next, (void**)&prev);
        VMEM_STRUCT_R(ehci_qh, prev, qh_next, (void**)&tmp_qh);
    }
    #endif

    if(tmp_qh != qh)
    {
        LOG_ERROR " start_unlink_async() can't find the qh to be unlinked!! \n" LOG_END
        while(1);
    }

    VMEM_STRUCT_R(ehci_qh, qh, hw_info1, &tmp_value);
    tmp_value = le32_to_cpu(tmp_value);
    if(tmp_value & QH_HEAD)
    {
        LOG_DEBUG " unlink QH_HEAD \n" LOG_END
        ehci->async = prev;
        VMEM_STRUCT_R(ehci_qh, prev, hw_info1, &tmp_value);
        tmp_value = le32_to_cpu(tmp_value);
        tmp_value = tmp_value | QH_HEAD;
        VMEM_STRUCT_W(ehci_qh, prev, hw_info1, cpu_to_le32(tmp_value));
    }

    VMEM_STRUCT_R(ehci_qh, qh, hw_next, &tmp_value);
    VMEM_STRUCT_W(ehci_qh, prev, hw_next, tmp_value);
    VMEM_STRUCT_R(ehci_qh, qh, qh_next, &tmp_addr);
    VMEM_STRUCT_W(ehci_qh, prev, qh_next, tmp_addr);
#if defined(DUMP_QH)
    {
        LOG_DATA " unlink qh \n" LOG_END
        dumpVram((void*)qh, sizeof(struct ehci_qh));
        LOG_DATA " prev qh \n" LOG_END
        dumpVram((void*)prev, sizeof(struct ehci_qh));
    }
#endif

    if(unlikely(ehci->hcd.state == USB_STATE_HALT)) 
    {
        LOG_DEBUG " start unlink qh with HCD HALT! \n" LOG_END
        ehci->reclaim_ready = 1;
        _spin_unlock_irqrestore(&ehci->lock);
        end_unlink_async(ehci);
        _spin_lock_irqsave(&ehci->lock);
        goto end;
    }
    else
    {
        ehci->reclaim_ready = 0;
        cmd |= CMD_IAAD;
        AHB_WriteRegister(ehci->regs.command, cmd);
    }

end:
    LOG_LEAVE " start_unlink_async() \n" LOG_END
    return;
}

/*-------------------------------------------------------------------------*/
static void scan_async(struct ehci_hcd *ehci)
{
    struct ehci_qh*	qh;
    struct list_head* tmp_list_head;
    LOG_ENTER " scan_async() ehci 0x%08X \n", ehci LOG_END

    _spin_lock_irqsave(&ehci->lock);

rescan:
    qh = ehci->async;
    if(likely(qh != 0)) 
    {
        do 
        {
            /* clean any finished work for this qh */
            VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&tmp_list_head);
            if(!LIST_EMPTY_VMEM(tmp_list_head))
            {
                qh = qh_get(qh);
                //_spin_unlock_irqrestore(&ehci->lock);

                /* concurrent unlink could happen here */
                qh_completions(ehci, qh, 1);

                //_spin_lock_irqsave(&ehci->lock);
                qh_put(ehci, qh);
            }

            /* unlink idle entries (reduces BUS usage) */
            if(LIST_EMPTY_VMEM(tmp_list_head) && !ehci->reclaim) 
            {
                #if 0
                start_unlink_async(ehci, qh);
                #else
                struct ehci_qh* qh_next;
                VMEM_STRUCT_R(ehci_qh, qh, qh_next, &qh_next);
                if(qh_next != qh) 
                {
                    start_unlink_async(ehci, qh);
                } 
                else 
                {
                    // FIXME:  arrange to stop
                    // after it's been idle a while.
                    //LOG_DEBUG " scan_async() do what?? \n" LOG_END
                }
                #endif
            }

            VMEM_STRUCT_R(ehci_qh, qh, qh_next, &qh);
            if(!qh)		/* unlinked? */
                goto rescan;
        } while(qh != ehci->async);
    }

    _spin_unlock_irqrestore(&ehci->lock);
    LOG_LEAVE " scan_async() \n" LOG_END
}

