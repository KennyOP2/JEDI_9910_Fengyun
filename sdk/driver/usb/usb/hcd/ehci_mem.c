/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * ehci memory related. This file is part of ehci-hcd.c
 *
 * @author Irene Lin
 */

#define EHCI_QH_ALIGN          32
#define EHCI_QH_SIZE           ((sizeof(struct ehci_qh) + (EHCI_QH_ALIGN-1)) & ~(EHCI_QH_ALIGN-1))
#define EHCI_QH_TOTAL_SIZE     (EHCI_QH_NUM*EHCI_QH_SIZE+EHCI_QH_ALIGN)

#define EHCI_QTD_ALIGN         32
#define EHCI_QTD_SIZE          ((sizeof(struct ehci_qtd) + (EHCI_QTD_ALIGN-1)) & ~(EHCI_QTD_ALIGN-1))
#define EHCI_QTD_TOTAL_SIZE    (EHCI_QTD_NUM*EHCI_QTD_SIZE+EHCI_QTD_ALIGN)

/** periodic table (frame list) */
#define EHCI_PT_ALIGN          0x1000 // 4k
#define EHCI_PT_TOTAL_SIZE(x)  ((x)*sizeof(MMP_UINT32) + EHCI_PT_ALIGN)

#define EHCI_20KBUF_SIZE       (5*4*1024)
#define EHCI_20KBUF_ALIGN      0x1000//4
#define EHCI_20KBUF_TOTAL_SIZE  (EHCI_20KBUF_NUM*EHCI_20KBUF_SIZE+EHCI_20KBUF_ALIGN)

static void dumpVram(MMP_UINT8* addr, MMP_UINT32 size)
{
    MMP_UINT32 i = 0;
    MMP_UINT8* data = (MMP_UINT8*)SYS_Malloc(size);
    HOST_ReadBlockMemory((MMP_UINT32)data, (MMP_UINT32)addr, size);

    LOG_DATA " \n" LOG_END
    LOG_DATA " addr = 0x%08X \n", addr LOG_END
    for(i=0; i<size; i++)
    {
        if(!(i % 4))
            LOG_DATA " \n" LOG_END
        LOG_DATA " %02X ", data[i] LOG_END
    }
    LOG_DATA " \n" LOG_END
    for(i=0; i<500000; i++) {}
}

static void dumpSram(MMP_UINT8* addr, MMP_UINT32 size)
{
    MMP_UINT32 i = 0;

    LOG_DATA " \n" LOG_END
    for(i=0; i<size; i++)
    {
        if(!(i % 4))
            LOG_DATA " \n" LOG_END
        LOG_DATA " %02X ", addr[i] LOG_END
    }
    LOG_DATA " \n" LOG_END
}

static void dumpReg(struct ehci_hcd* ehci)
{
    MMP_UINT32 size = sizeof(struct ehci_regs)/4;
    MMP_UINT32* reg = (MMP_UINT32*)&ehci->regs;
    MMP_UINT32 i = 0;
    MMP_UINT32 value = 0;

    LOG_DATA " \n EHCI registers: \n" LOG_END
    for(i=0; i<size; i++)
    {
        AHB_ReadRegister(reg[i], &value);
        LOG_DATA "reg 0x%08X = %08X \n", reg[i], value LOG_END
    }
    LOG_DATA " \n" LOG_END
    for(i=0; i<500000; i++) {}
}

/* 
 * Allocator / cleanup for the per device structure
 * Called by hcd init / removal code
 */
static struct usb_hcd* ehci_hcd_alloc(void)
{
    struct ehci_hcd *ehci;

    ehci = (struct ehci_hcd*)SYS_Malloc(sizeof(struct ehci_hcd));
    if(ehci != 0) 
    {
        memset(ehci, 0, sizeof(struct ehci_hcd));
        return &ehci->hcd;
    }
    return MMP_NULL;
}

static void ehci_hcd_free(struct usb_hcd *hcd)
{
    if(hcd)
        SYS_Free((void*)(hcd_to_ehci(hcd)));
}

static MMP_INT ehci_mem_init(struct ehci_hcd *ehci)
{
    MMP_INT result = 0;

    /** NOTE!!! These memory will never be released! here I will align the pool base  */

    /* QH for control/bulk/intr transfers */
    ehci->qh_pool = (MMP_UINT8*)MEM_Allocate(EHCI_QH_TOTAL_SIZE, MEM_USER_USBEX, MEM_USAGE_QH_EX);
    if(!ehci->qh_pool)
    {
        result = ERROR_USB_ALLOC_QH_FAIL;
        goto end;
    }
    ehci->qh_pool = (MMP_UINT8*)(((MMP_UINT32)ehci->qh_pool + (EHCI_QH_ALIGN-1)) & ~(EHCI_QH_ALIGN-1));

    /* QTDs for control/bulk/intr transfers */
    ehci->qtd_pool = (MMP_UINT8*)MEM_Allocate(EHCI_QTD_TOTAL_SIZE, MEM_USER_USBEX, MEM_USAGE_QTD_EX);
    if(!ehci->qtd_pool)
    {
        result = ERROR_USB_ALLOC_QTD_FAIL;
        goto end;
    }
    ehci->qtd_pool = (MMP_UINT8*)(((MMP_UINT32)ehci->qtd_pool + (EHCI_QTD_ALIGN-1)) & ~(EHCI_QTD_ALIGN-1));

    /* Hardware periodic table */
    ehci->periodic = (MMP_UINT32*)MEM_Allocate(EHCI_PT_TOTAL_SIZE(ehci->periodic_size), MEM_USER_USBEX, MEM_USAGE_PERIODIC_EX);
    if(!ehci->periodic)
    {
        result = ERROR_USB_ALLOC_PERIODIC_TABLE_FAIL;
        goto end;
    }
    ehci->periodic = (MMP_UINT32*)(((MMP_UINT32)ehci->periodic + (EHCI_PT_ALIGN-1)) & ~(EHCI_PT_ALIGN-1));
    { /** initialize the periodic schedule frame list */
        MMP_UINT i;
        MMP_UINT32 value = EHCI_LIST_END;
        for(i=0; i<ehci->periodic_size; i++)
            HOST_WriteBlockMemory((MMP_UINT32)ehci->periodic+i*4, (MMP_UINT32)&value, sizeof(MMP_UINT32));
    }

    /* software shadow of hardware table */
    ehci->pshadow = malloc(ehci->periodic_size * sizeof(void*));
    if(!ehci->pshadow)
    {
        result = ERROR_USB_ALLOC_PT_SHADOW_FAIL;
        goto end;
    }
    memset((void*)ehci->pshadow, 0, ehci->periodic_size*sizeof(void*));

#if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
    /* 4k buffer for QTD */
    ehci->page_pool = (MMP_UINT8*)MEM_Allocate(EHCI_20KBUF_TOTAL_SIZE, MEM_USER_USBEX, MEM_USAGE_4KBUF_EX);
    if(!ehci->page_pool)
    {
        result = ERROR_USB_ALLOC_4KBUF_FAIL;
        goto end;
    }
    ehci->page_pool = (MMP_UINT8*)(((MMP_UINT32)ehci->page_pool + (EHCI_20KBUF_ALIGN-1)) & ~(EHCI_20KBUF_ALIGN-1));
#endif

end:
    if(result)
        LOG_ERROR " ehci_mem_init() return error code 0x%08X, driver index 0x%08X \n", result, ehci->hcd.index LOG_END
    return result;
}

static struct ehci_qtd* ehci_qtd_alloc(struct ehci_hcd* ehci)
{
    struct ehci_qtd* qtd = MMP_NULL;
    MMP_UINT32 i=0;

    ithEnterCritical();
    for(i=0; i<EHCI_QTD_NUM; i++)
    {
        if(ehci->qtd_manage[i] == EHCI_MEM_FREE)
        {
            ehci->qtd_manage[i] = EHCI_MEM_USED;
            qtd = (struct ehci_qtd*)(ehci->qtd_pool+i*EHCI_QTD_SIZE);
            HOST_SetBlockMemory((MMP_UINT32)qtd, 0x0, EHCI_QTD_SIZE);
            break;
        }
    }

    if(qtd)
    {
        MMP_UINT32 list_end = EHCI_LIST_END;
        struct list_head*    qtd_list_tmp;

        VMEM_STRUCT_W(ehci_qtd, qtd, qtd_addr, (MMP_UINT8*)qtd);
        VMEM_STRUCT_W(ehci_qtd, qtd, hw_next, list_end);
        VMEM_STRUCT_W(ehci_qtd, qtd, hw_alt_next, list_end);
        VMEM_STRUCT_R_ADDR(ehci_qtd, qtd, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        INIT_LIST_HEAD_VMEM(qtd_list_tmp);
        VMEM_STRUCT_W(ehci_qtd, qtd, buf_index, i);
    }
    ithExitCritical();

    return qtd;
}

static void ehci_qtd_free(struct ehci_hcd* ehci, struct ehci_qtd* qtd)
{
    MMP_UINT32 index = 0;

    ithEnterCritical();

    VMEM_STRUCT_R(ehci_qtd, qtd, buf_index, &index);
    if(ehci->qtd_manage[index] == EHCI_MEM_FREE)
        LOG_ERROR " qtd double free!!! index = %d \n", index LOG_END

    ehci->qtd_manage[index] = EHCI_MEM_FREE;

    ithExitCritical();
}

static struct ehci_qh* ehci_qh_alloc(struct ehci_hcd* ehci)
{
    struct ehci_qh* qh = MMP_NULL;
    MMP_UINT32 i=0;

    ithEnterCritical();

    for(i=0; i<EHCI_QH_NUM; i++)
    {
        if(ehci->qh_manage[i] == EHCI_MEM_FREE)
        {
            ehci->qh_manage[i] = EHCI_MEM_USED;
            qh = (struct ehci_qh*)(ehci->qh_pool+i*EHCI_QH_SIZE);
            HOST_SetBlockMemory((MMP_UINT32)qh, 0x0, EHCI_QH_SIZE);
            break;
        }
    }

    if(qh)
    {
        struct list_head*    qtd_list_tmp;
        MMP_UINT32 one = 1;

        VMEM_STRUCT_W(ehci_qh, qh, qh_addr, (MMP_UINT8*)qh);
        VMEM_STRUCT_W(ehci_qh, qh, refcount, one);
        VMEM_STRUCT_R_ADDR(ehci_qh, qh, qtd_list, (MMP_UINT8**)&qtd_list_tmp);
        INIT_LIST_HEAD_VMEM(qtd_list_tmp);
        VMEM_STRUCT_W(ehci_qh, qh, buf_index, i);
    }

    ithExitCritical();

    return qh;
}

static struct ehci_qh* qh_get(struct ehci_qh* qh)
{
    MMP_UINT32 tmp_value;
    
    ithEnterCritical();

    VMEM_STRUCT_R(ehci_qh, qh, refcount, &tmp_value);
    tmp_value++;
    VMEM_STRUCT_W(ehci_qh, qh, refcount, tmp_value);
    //LOG_DEBUG " qh 0x%08X refcount++ %d \n", qh, tmp_value LOG_END

    ithExitCritical();

    return qh;
}

static void qh_put(struct ehci_hcd* ehci, struct ehci_qh* qh)
{
    MMP_UINT32 tmp_value;

    ithEnterCritical();

    VMEM_STRUCT_R(ehci_qh, qh, refcount, &tmp_value);
    tmp_value--;
    //LOG_DEBUG " qh 0x%08X refcount-- %d \n", qh, tmp_value LOG_END

    if(tmp_value==0)
    {
        MMP_UINT32 index = 0;

        VMEM_STRUCT_R(ehci_qh, qh, buf_index, &index);
        if(ehci->qh_manage[index] == EHCI_MEM_FREE)
            LOG_ERROR " qh double free!!! index = %d \n", index LOG_END

        ehci->qh_manage[index] = EHCI_MEM_FREE;
        LOG_DEBUG " free qh 0x%08X \n", qh LOG_END
    }
    else
    {
        VMEM_STRUCT_W(ehci_qh, qh, refcount, tmp_value);
    }

    ithExitCritical();
}


#if !defined(__FREERTOS__) && !defined(__OPENRTOS__)
static MMP_UINT8* ehci_20kbuf_alloc(struct ehci_hcd *ehci)
{
    MMP_UINT8* addr = MMP_NULL;
    MMP_UINT32 i=0;

    for(i=0; i<EHCI_20KBUF_NUM; i++)
    {
        if(ehci->page_manage[i] == EHCI_MEM_FREE)
        {
            ehci->page_manage[i] = EHCI_MEM_USED;
            addr = (ehci->page_pool+i*EHCI_20KBUF_SIZE);
            HOST_SetBlockMemory((MMP_UINT32)addr, 0x0, EHCI_20KBUF_SIZE);
            LOG_DEBUG " ehci_20kbuf_alloc() index %d \n", i LOG_END
            break;
        }
    }
    if(i == EHCI_20KBUF_NUM)
    {
        LOG_ERROR " ehci_20kbuf_alloc() fail \n" LOG_END
        while(1);
    }
    return addr;
}

static MMP_INT ehci_20kbuf_free(struct ehci_hcd *ehci, MMP_UINT8* addr)
{
    MMP_INT result = 0;
    MMP_INT index = 0;

    index = ((MMP_UINT32)addr - (MMP_UINT32)ehci->page_pool)/EHCI_20KBUF_SIZE;
    LOG_DEBUG " ehci_20kbuf_free() index %d \n", index LOG_END
    if(index > EHCI_20KBUF_NUM) 
    {
        result = ERROR_USB_FREE_ONE_20KBUF_FAIL;
        LOG_ERROR " (0x%08X - 0x%08X)/EHCI_4KBUF_SIZE = %d \n", addr, ehci->page_pool, index LOG_END
        goto end;
    }
    ehci->page_manage[index] = EHCI_MEM_FREE;

end:
    if(result)
        LOG_ERROR " ehci_20kbuf_free() return error code 0x%08X \n", result LOG_END
    return result;
}
#endif // #if !defined(__FREERTOS__) && !defined(__OPENRTOS__)

