/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * EHCI hc_driver implementation.
 *
 * @author Irene Lin
 */
//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
/* magic numbers that can affect system performance */
#define EHCI_TUNE_CERR        3         /* 0-3 qtd retries; 0 == don't stop */
#define EHCI_TUNE_RL_HS       0         /* nak throttle; see 4.9 */
#define EHCI_TUNE_RL_TT       0
#define EHCI_TUNE_MULT_HS     0         /* 1-3 transactions/uframe; 4.10.3 */
#define EHCI_TUNE_MULT_TT     0

#if defined(USB_IRQ_ENABLE)
#define INTR_MASK         (STS_IAA | STS_FATAL | STS_ERR | STS_INT | STS_PCD)
#else
#define INTR_MASK         (STS_IAA | STS_FATAL | STS_ERR | STS_INT)
#endif
#define EHCI_HC_RESET_TIMEOUT 500

//=============================================================================
//                              Macro Definition
//=============================================================================
//=============================================================================
//                              Global Data Definition
//=============================================================================
//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_INT ehci_reset(struct ehci_hcd *ehci)
{
    MMP_INT        result = 0;
    MMP_UINT32     value = 0;
#if defined(ALPHA_SDK)
    struct timeval startT, endT;

    AHB_WriteRegisterMask(ehci->regs.command, CMD_RESET, CMD_RESET);
    gettimeofday(&startT, NULL);
    do
    {
        AHB_ReadRegister(ehci->regs.command, &value);
        gettimeofday(&endT, NULL);
        if (itpTimevalDiff(&startT, &endT) > EHCI_HC_RESET_TIMEOUT)
#else
    SYS_CLOCK_T lastTime;

    AHB_WriteRegisterMask(ehci->regs.command, CMD_RESET, CMD_RESET);
    lastTime = SYS_GetClock();
    do
    {
        AHB_ReadRegister(ehci->regs.command, &value);
        if (SYS_GetDuration(lastTime) > EHCI_HC_RESET_TIMEOUT)
#endif
        {
            result = ERROR_USB_HC_RESET_TIMEOUT;
            goto end;
        }
    } while (value & CMD_RESET);
    ehci->hcd.state = USB_STATE_HALT;
    MMP_Sleep(25);

end:
    if (result)
        LOG_ERROR " ehci_hc_reset() return error code 0x%08X, ehci index 0x%08X \n", result, ehci->hcd.index LOG_END
        return result;
    }

/* idle the controller (from running) */
static void ehci_ready(struct ehci_hcd *ehci)
{
    MMP_UINT32 command;

    if (!HCD_IS_RUNNING(ehci->hcd.state))
    {
        LOG_ERROR " ehci_ready() hcd is not running! \n" LOG_END
            while (1);
    }

    AHB_ReadRegister(ehci->regs.command, &command);
    command &= ~(CMD_ASE | CMD_IAAD | CMD_PSE);
    AHB_WriteRegister(ehci->regs.command, command);

    // hardware can take 16 microframes to turn off ...
    ehci->hcd.state = USB_STATE_READY;
}

static MMP_INT ehci_wait_ready(struct ehci_hcd *ehci)
{
    MMP_INT        result = 0;
    MMP_UINT32     tmp_value = 0;

#if defined(ALPHA_SDK)
    struct timeval startT, endT;

    gettimeofday(&startT, NULL);
    AHB_ReadRegister(ehci->regs.status, &tmp_value);
    while (tmp_value & (STS_ASS | STS_PSS))
    {
        gettimeofday(&endT, NULL);
        if (itpTimevalDiff(&startT, &endT) > 500)
#else
    SYS_CLOCK_T lastTime = SYS_GetClock();

    AHB_ReadRegister(ehci->regs.status, &tmp_value);
    while (tmp_value & (STS_ASS | STS_PSS))
    {
        if (SYS_GetDuration(lastTime) > 500)
#endif
        {
            result = ERROR_USB_WAIT_ASS_PSS_READY_TIMEOUT;
            goto end;
        }
        MMP_USleep(100);
        AHB_ReadRegister(ehci->regs.status, &tmp_value);
    }

end:
    if (result)
        LOG_ERROR " ehci_wait_ready() return error code 0x%08X \n", result LOG_END
        return result;
    }

#if 1//defined(MS_ENUMERATE)
MMP_INT ehci_port_reset(struct ehci_hcd *ehci)
#else
static MMP_INT ehci_port_reset(struct ehci_hcd *ehci)
#endif
{
    MMP_INT        result = 0;
    MMP_UINT32     value = 0;
#if defined(ALPHA_SDK)
    struct timeval startT, endT;
#else
    SYS_CLOCK_T    lastTime;
#endif

    /** run first */
    AHB_ReadRegister(ehci->regs.command, &value);
    value |= CMD_RUN;
    AHB_WriteRegister(ehci->regs.command, value);

    /** port reset */
    AHB_WriteRegisterMask(ehci->regs.port_status[0], PORT_RESET, PORT_RESET);
    MMP_Sleep(50);
    AHB_WriteRegisterMask(ehci->regs.port_status[0], 0x0, PORT_RESET);
#if defined(ALPHA_SDK)
    gettimeofday(&startT, NULL);
    do
    {
        AHB_ReadRegister(ehci->regs.port_status[0], &value);
        gettimeofday(&endT, NULL);
        if (itpTimevalDiff(&startT, &endT) > 500)
#else
    lastTime = SYS_GetClock();
    do
    {
        AHB_ReadRegister(ehci->regs.port_status[0], &value);
        if (SYS_GetDuration(lastTime) > 500)
#endif
        {
            result = ERROR_USB_PORT_RESET_FAIL;
            goto end;
        }
    } while (value & PORT_RESET);
    MMP_Sleep(5);

end:
    if (result)
        LOG_ERROR " ehci_port_reset() return error code 0x%08X, ehci index 0x%08X \n", result, ehci->hcd.index LOG_END
        return result;
    }

static MMP_INT ehci_get_speed(struct ehci_hcd *ehci)
{
    MMP_UINT32 value = 0;
    MMP_INT    speed = 0;

    AHB_ReadRegister(ehci->otg_regs.ctrl_status, &value);
    speed = HOST_SPEED(value);

    if (speed == FULL_SPEED)
    {
        LOG_DATA " Host Speed: Full Speed! \n" LOG_END
        return USB_SPEED_FULL;
    }
    else if (speed == LOW_SPEED)
    {
        LOG_DATA " Host Speed: Low Speed! \n" LOG_END
        return USB_SPEED_LOW;
    }
    else if (speed == HIGH_SPEED)
    {
        LOG_DATA " Host Speed: High Speed! \n" LOG_END
        return USB_SPEED_HIGH;
    }

    return USB_SPEED_UNKNOWN;
}

MMP_INT ehci_init(struct usb_hcd *hcd, struct usb_device **usb_device)
{
    MMP_INT           result = 0;
    struct ehci_hcd   *ehci = hcd_to_ehci(hcd);
    struct usb_device *udev = MMP_NULL;
    MMP_UINT32        temp = 0;
    MMP_INT           tmp_result = 0;

    ehci->i_thresh = 8;
    ehci->async = 0;
    ehci->reclaim = 0;
    ehci->next_uframe = -1;

    result = ehci_reset(ehci);
    if (result)
        goto end;

    /** enable interrupt */
    AHB_WriteRegister(ehci->regs.intr_enable, INTR_MASK);
    /** async schedule sleep time */
    AHB_WriteRegisterMask(ehci->regs.hc_misc, 0x3, 0x3);
    /** clear interrupt enables, set irq latency. and keeping default periodic framelist size. */
    AHB_ReadRegister(ehci->regs.command, &temp);
    temp &= 0xFF00FF;
#if defined(INTR_THRESHOLD)
    temp &= 0x0000FF;
    temp |= (INTR_THRESHOLD << 16);
#endif
    temp &= ~(CMD_IAAD | CMD_ASE | CMD_PSE);
    temp |= 0x100;  // mask 0x300
    AHB_WriteRegister(ehci->regs.command, temp);

    /** Is device presented on the port */
    AHB_ReadRegister(ehci->regs.port_status[0], &temp);
    if (!(temp & PORT_CONNECT))
    {
        LOG_INFO " Device NOT presented! ehci index = 0x%08X \n", ehci->hcd.index LOG_END
            goto end;
    }
    ehci->hcd.connect = 1;

    /** wire up the device */
    ehci->hcd.bus->root_device = udev = usb_alloc_dev(MMP_NULL, ehci->hcd.bus);
    if (!udev)
    {
        result = ERROR_USB_ALLOC_DEVICE_FAIL;
        goto end;
    }
    ehci->hcd.state = USB_STATE_READY;

    result = ehci_port_reset(ehci);
    if (result)
        goto end;

    usb_connect(udev);
#if !defined(MS_ENUMERATE)
    if (!udev->devnum)
    {
        result = ERROR_USB_NOT_FIND_DEVCE_ID;
        goto end;
    }
#endif
    udev->speed = ehci_get_speed(ehci);
    /** EOF 1 Timing Points */
    if (udev->speed == USB_SPEED_FULL) /** Fix "Full Speed" + "0xFF data" ==> transaction error */
        AHB_WriteRegisterMask(ehci->regs.hc_misc, 0xC, 0xC);
    else
        AHB_WriteRegisterMask(ehci->regs.hc_misc, 0x0, 0xC);
    if (tmp_result = usb_new_device(udev))
    {
        if (hcd->state == USB_STATE_RUNNING)
        {
            ehci_ready(ehci);
            result = ehci_wait_ready(ehci);
            if (result)
                goto end;
        }

        //ehci_reset(ehci); /** this will cause connect status bit fail! */
        hcd->bus->root_device = 0;
        usb_free_dev(udev);
#if defined(USB_LOGO_TEST)
        if (tmp_result == HUB_ERROR)
            result = HUB_ERROR;
        else
#endif
            result = -ENODEV;
        goto end;
    }
    (*usb_device) = udev;

end:
    if (result)
        LOG_ERROR " ehci_init() return error code 0x%08X \n", ((result < 0) ? (-result) : result)LOG_END
        return result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
#include "ehci_mem.c"
#include "ehci_q.c"
#include "ehci_sched.c"

static MMP_INT ehci_start(struct usb_hcd *hcd)
{
    MMP_INT         result = 0;
    MMP_UINT32      i = 0;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_UINT32      *reg = MMP_NULL;
    LOG_ENTER " ehci_start() hcd = 0x%08X \n", hcd LOG_END

        /** generate register offset */
        reg = (MMP_UINT32 *)&ehci->caps;
    for (i = 0; i < (sizeof(struct ehci_caps) / 4); i++, reg++)
        (*reg) = hcd->iobase + (i * 4);

    AHB_ReadRegister(ehci->caps.hcc_reg, &ehci->hc_caps);
    AHB_ReadRegister(ehci->caps.hcs_params, &ehci->hcs_params);
    AHB_ReadRegister(ehci->caps.hcc_params, &ehci->hcc_params);
    LOG_INFO " Reg 0x%08X = 0x%08X \n", ehci->caps.hcc_reg, ehci->hc_caps LOG_END
        LOG_INFO " Reg 0x%08X = 0x%08X \n", ehci->caps.hcs_params, ehci->hcs_params LOG_END
        LOG_INFO " Reg 0x%08X = 0x%08X \n", ehci->caps.hcc_params, ehci->hcc_params LOG_END
        reg = (MMP_UINT32 *)&ehci->regs;
    for (i = 0; i < (sizeof(struct ehci_regs) / 4); i++, reg++)
        (*reg) = hcd->iobase + (ehci->hc_caps & 0xFF) + (i * 4);
    reg = (MMP_UINT32 *)&ehci->otg_regs;
    for (i = 0; i < (sizeof(struct otg_regs) / 4); i++, reg++)
        (*reg) = hcd->iobase + 0x80 + (i * 4);
    ehci->otg_regs.curr_role = 0;
    reg = (MMP_UINT32 *)&ehci->global_regs;
    for (i = 0; i < (sizeof(struct global_regs) / 4); i++, reg++)
        (*reg) = hcd->iobase + 0xC0 + (i * 4);

    _spin_lock_init(&ehci->lock);

    /** Full speed Phy Reset Fail workaround solution 0x80 D[28] */
    //AHB_WriteRegisterMask(ehci->otg_regs.ctrl_status, 0x10000000, 0x10000000);
    USBEX_OTGSetup(ehci);

    /** memory init */
    /**
     * hw default: 1K periodic list heads, one per frame.
     * periodic_size can shrink by USBCMD update if hcc_params allows.
     */
    ehci->periodic_size = DEFAULT_I_TDPS;
    result = ehci_mem_init(ehci);
    if (result)
        goto end;

    /** Set periodic frame list base address */
    AHB_WriteRegister(ehci->regs.frame_list, ((MMP_UINT32)ehci->periodic - (MMP_UINT32)HOST_GetVramBaseAddress()));

end:
    if (result)
        LOG_ERROR " ehci_start() return error code 0x%08X \n", result LOG_END
        LOG_LEAVE " ehci_start() hcd = 0x%08X \n", hcd LOG_END
        return result;
}

#if defined(USB_PM)
static MMP_INT ehci_suspend(struct usb_hcd *hcd, MMP_UINT32 state)
{
    MMP_INT         result = 0;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_UINT32      reg = 0;
    LOG_ENTER " ehci_suspend() hcd = 0x%08X \n", hcd LOG_END

        /** Check port enable */
        AHB_ReadRegister(ehci->regs.port_status[0], &reg);
    if (!(reg & PORT_PE))
    {
        result = ERROR_USB_PORT_NOT_ENABLE;
        goto end;
    }

    /** Disable scheduler */
    if (hcd->state == USB_STATE_RUNNING)
        ehci_ready(ehci);
    /** Wait scheduler ready */
    result = ehci_wait_ready(ehci);
    if (result)
        goto end;

    /** Disable run/stop bit */
    AHB_WriteRegisterMask(ehci->regs.command, CMD_RUN, 0);

    /** Set suspend */
    AHB_WriteRegisterMask(ehci->regs.port_status[0], PORT_SUSPEND, PORT_SUSPEND);

end:
    if (result)
        LOG_ERROR " ehci_suspend() return error code 0x%08X \n", result LOG_END
        LOG_LEAVE " ehci_suspend() hcd = 0x%08X \n", hcd LOG_END
        return result;
}

static MMP_INT ehci_resume(struct usb_hcd *hcd)
{
    MMP_INT         result = 0;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_UINT32      reg = 0;
    LOG_ENTER " ehci_resume() hcd = 0x%08X \n", hcd LOG_END

        /** Ensure in suspend state */
        AHB_ReadRegister(ehci->regs.port_status[0], &reg);
    if ((reg & (PORT_PE | PORT_SUSPEND)) != (PORT_PE | PORT_SUSPEND))
    {
        LOG_ERROR " ehci_resume() port status %08X \n", reg LOG_END
            result = ERROR_USB_PORT_NOT_SUSPEND;
        goto end;
    }

    /** Force port resume */
    AHB_WriteRegisterMask(ehci->regs.port_status[0], PORT_RESUME, PORT_RESUME);
    MMP_Sleep(20);
    AHB_WriteRegisterMask(ehci->regs.port_status[0], 0, PORT_RESUME);

end:
    if (result)
        LOG_ERROR " ehci_resume() return error code 0x%08X \n", result LOG_END
        LOG_LEAVE " ehci_resume() hcd = 0x%08X \n", hcd LOG_END
        return result;
}
#endif

static MMP_INT ehci_stop(struct usb_hcd *hcd)
{
    MMP_INT type = 0;
    LOG_ENTER " ehci_stop() hcd = 0x%08X \n", hcd LOG_END

        if (hcd->bus->root_device)
        {
            type = hcd->bus->root_device->device_info[0].type;
            usb_disconnect(&hcd->bus->root_device);
        }

    hcd->connect = 0;

    LOG_LEAVE " ehci_stop() hcd = 0x%08X \n", hcd LOG_END
        return type;
}

static MMP_INT ehci_get_frame(struct usb_hcd *hcd)
{
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_UINT32      reg;

    AHB_ReadRegister(ehci->regs.frame_index, &reg);
    return (reg >> 3) % ehci->periodic_size;
}

/*-------------------------------------------------------------------------*/
/*
 * non-error returns are a promise to giveback() the urb later
 * we drop ownership so next owner (or urb unlink) can get it
 *
 * urb + dev is in hcd_dev.urb_list
 * we're queueing TDs onto software and hardware lists
 *
 * hcd-specific init for hcpriv hasn't been done yet
 *
 * NOTE:  EHCI queues control and bulk requests transparently, like OHCI.
 */
static MMP_INT ehci_urb_enqueue(struct usb_hcd *hcd, struct urb *urb)
{
    MMP_INT          result = 0;
    struct ehci_hcd  *ehci = hcd_to_ehci(hcd);
    struct list_head qtd_list;

    LOG_ENTER " ehci_urb_enqueue() hcd = 0x%08X, urb = 0x%08X \n", hcd, urb LOG_END

        urb->transfer_flags &= ~EHCI_STATE_UNLINK;
    INIT_LIST_HEAD(&qtd_list);
    switch (usb_pipetype(urb->pipe))
    {
    case PIPE_CONTROL:
    case PIPE_BULK:
#if defined(USB_LOGO_TEST)
        if (urb->transfer_flags & (USB_SINGLE_STEP_0 | USB_SINGLE_STEP_1 | USB_SINGLE_STEP_2))
            result = qh_urb_transaction_step(ehci, urb, &qtd_list);
        else
            result = qh_urb_transaction(ehci, urb, &qtd_list);

        if (result)
        {
            LOG_ERROR " 1 \n" LOG_END
                result = -ENOMEM;
            goto end;
        }
#else
        if (qh_urb_transaction(ehci, urb, &qtd_list))
        {
            LOG_ERROR " 1 \n" LOG_END
                result = -ENOMEM;
            goto end;
        }
#endif
        result = submit_async(ehci, urb, &qtd_list);
        if (result)
            goto end;
        break;

    case PIPE_INTERRUPT:
        if (qh_urb_transaction(ehci, urb, &qtd_list))
        {
            LOG_ERROR " 2 \n" LOG_END
                result = -ENOMEM;
            goto end;
        }
        result = intr_submit(ehci, urb, &qtd_list);
        if (result)
            goto end;
        break;

#if 0   // ISO TODO
    case PIPE_ISOCHRONOUS:
        if (urb->dev->speed == USB_SPEED_HIGH)
            return itd_submit(ehci, urb, mem_flags);
#ifdef have_split_iso
        else
            return sitd_submit(ehci, urb, mem_flags);
#else
        dbg("no split iso support yet");
        return -ENOSYS;
#endif     /* have_split_iso */
#endif
    default:   /* can't happen */
        LOG_ERROR " 3 \n" LOG_END
            result = -ENOSYS;
        goto end;
    }

end:
    if (result)
        LOG_ERROR " ehci_urb_enqueue() return error code 0x%08X, hcd index 0x%08X \n", result, hcd->index LOG_END
        LOG_LEAVE " ehci_urb_enqueue() hcd = 0x%08X \n", hcd LOG_END
        return result;
}

/* remove from hardware lists
 * completions normally happen asynchronously
 */
static MMP_INT ehci_urb_dequeue(struct usb_hcd *hcd, struct urb *urb)
{
    MMP_INT         result = 0;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    struct ehci_qh  *qh = (struct ehci_qh *)urb->hcpriv;
    MMP_UINT32      qh_state = 0;
    LOG_ENTER " ehci_urb_dequeue() \n" LOG_END

        VMEM_STRUCT_R(ehci_qh, qh, qh_state, &qh_state);
    LOG_INFO " bus %d dequeue urb 0x%08X qh state %d \n", hcd->bus->busnum, urb, qh_state LOG_END

        switch (usb_pipetype(urb->pipe))
    {
        case PIPE_CONTROL:
        case PIPE_BULK:
            _spin_lock_irqsave(&ehci->lock);
            if (ehci->reclaim)
            {
                LOG_DEBUG "dq: reclaim busy???? ~~~~~ \n " LOG_END
                    VMEM_STRUCT_R(ehci_qh, qh, qh_state, &qh_state);
                while (qh_state == QH_STATE_LINKED
                    && ehci->reclaim
                    && ehci->hcd.state != USB_STATE_HALT)
                {
                    _spin_unlock_irqrestore(&ehci->lock);
                    // yeech ... this could spin for up to two frames!
                    ithPrintf("wait for dequeue: index %d, state %d, reclaim %p, hcd state %d \n", ehci->hcd.index,
                        qh_state, ehci->reclaim, ehci->hcd.state);
                    MMP_USleep(100);
                    _spin_lock_irqsave(&ehci->lock);
                    VMEM_STRUCT_R(ehci_qh, qh, qh_state, &qh_state);

                    {
                        /** Irene: if device is removed, HC run bit will be disabled by HC.... ??
                            and then HC willn't trigger IAA interrupt.
                            and then it willn't run end_unlink_xxx ....*/
                        MMP_UINT32 value = 0;
                        AHB_ReadRegister(ehci->regs.command, &value);
                        if (((value & (CMD_RUN | CMD_IAAD)) == CMD_IAAD))
                        {
                            ehci->reclaim_ready = 1;
                            tasklet_schedule(ehci);
                        }
                    }
                }
            }
            if (qh_state == QH_STATE_LINKED)
            {
                LOG_DEBUG "dq: qh link => start unlink~~ \n " LOG_END
                    start_unlink_async(ehci, qh);
                {
                    /** Irene: if device is removed, HC run bit will be disabled by HC.... ??
                        and then HC willn't trigger IAA interrupt.
                        and then it willn't run end_unlink_xxx ....*/
                    MMP_UINT32 value = 0;
                    AHB_ReadRegister(ehci->regs.command, &value);
                    if (((value & (CMD_RUN | CMD_IAAD)) == CMD_IAAD))
                    {
                        ehci->reclaim_ready = 1;
                        tasklet_schedule(ehci);
                    }
                }
            }
            _spin_unlock_irqrestore(&ehci->lock);
            goto end;

        case PIPE_INTERRUPT:
            intr_deschedule(ehci, urb->start_frame, qh, urb->interval);
            if (ehci->hcd.state == USB_STATE_HALT)
                urb->status = -ESHUTDOWN;
            _spin_lock_irqsave(&ehci->lock);
            qh_completions(ehci, qh, 1);
            _spin_unlock_irqrestore(&ehci->lock);
            goto end;

        case PIPE_ISOCHRONOUS:
            // itd or sitd ...

            // wait till next completion, do it then.
            // completion irqs can wait up to 1024 msec,
            // TODO ISO
#if 0
            urb->transfer_flags |= EHCI_STATE_UNLINK;
#endif
            goto end;
    }
    result = -EINVAL;

end:
    if (result)
        LOG_ERROR " ehci_urb_dequeue() return error code %d \n", result LOG_END
        LOG_LEAVE " ehci_urb_dequeue() \n" LOG_END
        return result;
}

static void ehci_free_config(struct usb_hcd *hcd, struct usb_device *udev)
{
    MMP_INT         result = 0;
    struct hcd_dev  *dev = (struct hcd_dev *)udev->hcpriv;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_INT         i;

    LOG_ENTER " ehci_free_config() \n" LOG_END

        LOG_DEBUG "hcd %X: free_config devnum %d \n", hcd, udev->devnum LOG_END

        _spin_lock_irqsave(&ehci->lock);
    for (i = 0; i < 32; i++)
    {
        if (dev->ep[i])
        {
            struct ehci_qh   *qh;
            struct list_head *tmp_list_head = MMP_NULL;
            MMP_UINT32       tmp_value;

            /* dev->ep never has ITDs or SITDs */
            qh = (struct ehci_qh *)dev->ep[i];
            LOG_DEBUG "free_config, ep %d qh 0x%08X \n", i, qh LOG_END
                VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8 **)&tmp_list_head);
            if (!LIST_EMPTY_VMEM(tmp_list_head))
            {
                LOG_ERROR "ehci_free_config(): ep %d qh 0x%08X not empty! \n", i, qh LOG_END
            }
            dev->ep[i] = 0;

            /* wait_ms() won't spin here -- we're a thread */
            VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_value);
            while ((tmp_value == QH_STATE_LINKED)
                && ehci->reclaim
                && (ehci->hcd.state != USB_STATE_HALT))
            {
                _spin_unlock_irqrestore(&ehci->lock);
                MMP_Sleep(1);
                _spin_lock_irqsave(&ehci->lock);
            }

            VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_value);
            if (tmp_value == QH_STATE_LINKED)
            {
                start_unlink_async(ehci, qh);
                VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_value);
                while (tmp_value != QH_STATE_IDLE)
                {
                    _spin_unlock_irqrestore(&ehci->lock);
                    MMP_Sleep(1);
                    _spin_lock_irqsave(&ehci->lock);
                    VMEM_STRUCT_R(ehci_qh, qh, qh_state, &tmp_value);
                }
            }
            qh_put(ehci, qh);
        }
    }

    _spin_unlock_irqrestore(&ehci->lock);
    if (result)
        LOG_ERROR " ehci_free_config() return error code 0x%08X \n", result LOG_END
        LOG_LEAVE " ehci_free_config() \n" LOG_END
}

/** 1: exist, 0: not exist */
static MMP_INT ehci_dev_exist(struct usb_hcd *hcd)
{
    MMP_UINT32      status = 0;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);

    AHB_ReadRegister(ehci->regs.port_status[0], &status);
    if (status & PORT_CONNECT)
        return 1;
    else
        return 0;
}

static struct hc_driver ehci_driver =
{
    HCD_USB2,
    ehci_start,
#if defined(USB_PM)
    ehci_suspend,
    ehci_resume,
#endif
    ehci_stop,
    ehci_get_frame,
    ehci_hcd_alloc,
    ehci_hcd_free,
    ehci_urb_enqueue,
    ehci_urb_dequeue,
    ehci_free_config,
    ehci_dev_exist,
#if defined(USB_ROOT_HUB)
    ehci_hub_status_data,
    ehci_hub_control
#endif
};

extern MMP_INT usb_hcd_probe(struct hc_driver *driver, MMP_UINT32 index);

MMP_INT ehci_hcd_init(MMP_UINT32 index)
{
    return usb_hcd_probe(&ehci_driver, index);
}

/*-------------------------------------------------------------------------*/

/*
 * tasklet scheduled by some interrupts and other events
 * calls driver completion functions ... but not in_irq()
 */
void ehci_tasklet(struct ehci_hcd *ehci)
{
    LOG_ENTER " ehci_tasklet() ehci 0x%08X \n", ehci LOG_END

        ehci->tasklet = 0;

    if (ehci->reclaim_ready)
        end_unlink_async(ehci);

    scan_async(ehci);

    if (ehci->next_uframe != -1)
        scan_periodic(ehci);

    LOG_LEAVE " ehci_tasklet() \n" LOG_END
}

/*-------------------------------------------------------------------------*/
#if defined(MM680)

#if defined(USB_IRQ_ENABLE)

#include "mmp_intr.h"
extern void *isr_event;

MMP_INT ehci_irq(void *data)
{
    struct usb_hcd *hcd = (struct usb_hcd *)data;
#else
void ehci_irq(struct usb_hcd *hcd)
{
#endif
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_UINT32      status = 0, cmd;
    MMP_BOOL        bh = MMP_FALSE;

    AHB_ReadRegister(ehci->regs.status, &status);
    status &= INTR_MASK;
    if (!status)
    {
#if defined(QUICK_REMOVE)
        if (hcd->connect)
        {
            AHB_ReadRegister(ehci->regs.command, &cmd);
            if (!(cmd & CMD_RUN) && HCD_IS_RUNNING(ehci->hcd.state))
            {
                ehci->hcd.state = USB_STATE_HALT;
                if (cmd & CMD_IAAD)
                {
                    AHB_WriteRegisterMask(ehci->regs.command, 0, CMD_IAAD);
                    ehci->reclaim_ready = 1;
                }
                bh = MMP_TRUE;
                goto bh;
            }
        }
#endif
#if defined(USB_IRQ_ENABLE)
        return IRQ_NONE;
#else
        return;
#endif
    }

    LOG_ENTER " ehci_irq() ehci 0x%08X \n", ehci LOG_END
        AHB_WriteRegister(ehci->regs.status, status);

    /* INT, ERR, and IAA interrupt rates can be throttled */

    /* normal [4.15.1.2] or error [4.15.1.1] completion */
    if (likely((status & (STS_INT | STS_ERR)) != 0))
        bh = MMP_TRUE;

    /* complete the unlinking of some qh [4.15.2.3] */
    if (status & STS_IAA)
    {
        LOG_DEBUG " ehci_irq status = 0x%08X, STS_IAA ~~~ \n", status LOG_END
            ehci->reclaim_ready = 1;
        bh = 1;
    }

    /* Bus errors [4.15.2.4] */
    if (unlikely((status & STS_FATAL) != 0))
    {
        LOG_ERROR " hcd index %08X: fatal error, state %x  \n", ehci->hcd.index, ehci->hcd.state LOG_END
            ehci_reset(ehci);
        // Irene TODO !!!!! ??
        // generic layer kills/unlinks all urbs
        // then tasklet cleans up the rest
        bh = 1;
    }

    if (ehci->reclaim_ready)
        bh = 1;

bh:
    if (likely(bh == 1))
    {
#if defined(USB_IRQ_ENABLE)
        tasklet_schedule(ehci);
        SYS_SetEventFromIsr(isr_event);
#else
        tasklet_schedule(ehci);
#endif
    }

    LOG_LEAVE " ehci_irq() ehci 0x%08X \n", ehci LOG_END
#if defined(USB_IRQ_ENABLE)
        return IRQ_HANDLED;
#endif
}

#else // #if defined(MM680)

#if defined(USB_IRQ_ENABLE)
extern void *isr_event;
#endif

//#include "ite/itp.h"
void ehci_irq(struct usb_hcd *hcd)
{
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    MMP_UINT32      status = 0, cmd;
    MMP_BOOL        bh = MMP_FALSE;

    AHB_ReadRegister(ehci->regs.status, &status);
    AHB_WriteRegister(ehci->regs.status, status);
    status &= INTR_MASK;

#if !defined(USB_IRQ_ENABLE)
    if (!status) // for polling case without interrupt
    {
#if defined(QUICK_REMOVE)
        if (hcd->connect)
        {
            AHB_ReadRegister(ehci->regs.command, &cmd);
            if (!(cmd & CMD_RUN) && HCD_IS_RUNNING(ehci->hcd.state))
            {
                ehci->hcd.state = USB_STATE_HALT;
                if (cmd & CMD_IAAD)
                {
                    AHB_WriteRegisterMask(ehci->regs.command, 0, CMD_IAAD);
                    ehci->reclaim_ready = 1;
                }
                bh = MMP_TRUE;
                goto bh;
            }
        }
#endif
        return;
    }
#endif
    LOG_ENTER " ehci_irq() ehci 0x%08X \n", ehci LOG_END

        /* INT, ERR, and IAA interrupt rates can be throttled */

        /* normal [4.15.1.2] or error [4.15.1.1] completion */
        if (likely((status & (STS_INT | STS_ERR)) != 0))
            bh = MMP_TRUE;

    /* complete the unlinking of some qh [4.15.2.3] */
    if (status & STS_IAA)
    {
        LOG_DEBUG " ehci_irq status = 0x%08X, STS_IAA ~~~ \n", status LOG_END
            ehci->reclaim_ready = 1;
        bh = 1;
    }

    /* Bus errors [4.15.2.4] */
    if (unlikely((status & STS_FATAL) != 0))
    {
        ithPrintf("[USB] hcd index %08X: fatal error, state %x  \n", ehci->hcd.index, ehci->hcd.state);

        ehci_reset(ehci);
        // Irene TODO !!!!! ??
        // generic layer kills/unlinks all urbs
        // then tasklet cleans up the rest
        bh = 1;
    }

    if (ehci->reclaim_ready)
        bh = 1;

bh:
    if (likely(bh == 1))
    {
#if defined(USB_IRQ_ENABLE)
        tasklet_schedule(ehci);
        SYS_SetEventFromIsr(isr_event);
#if defined(__OPENRTOS__)
        ithYieldFromISR();
#else
        portYIELD_FROM_ISR();
#endif
#else
        tasklet_schedule(ehci);
#endif
#if defined(MM9070)
        ithFlushAhbWrap();
#endif
    }

    LOG_LEAVE " ehci_irq() ehci 0x%08X \n", ehci LOG_END
}

#endif // #if defined(MM680)

/*-------------------------------------------------------------------------*/
MMP_INT ehci_isDisconnect(struct usb_hcd *hcd)
{
    MMP_INT         res = 0;
    MMP_UINT32      cmd;
    struct ehci_hcd *ehci = hcd_to_ehci(hcd);
    if (hcd->connect)
    {
        AHB_ReadRegister(ehci->regs.command, &cmd);
        if (!(cmd & CMD_RUN) && HCD_IS_RUNNING(ehci->hcd.state))
        {
            ehci->hcd.state = USB_STATE_HALT;
            if (cmd & CMD_IAAD)
            {
                AHB_WriteRegisterMask(ehci->regs.command, 0, CMD_IAAD);
                ehci->reclaim_ready = 1;
            }
            tasklet_schedule(ehci);
            res = 1;
        }
    }
    return res;
}