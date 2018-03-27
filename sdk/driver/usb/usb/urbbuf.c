/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * URB pool management.
 *
 * @author Irene Lin
 */

#include "usb/config.h"
#include "usb/usb/host.h"


static URBLIST_HEAD(urb) urbList; 
static struct urb urbPool[USBEX_URB_NUM];

MMP_INT urbBufInitialize(void) 
{ 
    static MMP_BOOL isInit = 0; 
    MMP_INT i;
    struct urb* urbHead = MMP_NULL; 

    if(isInit) 
        return 0;
    
    URBLIST_INIT(urbList); 

    for(i=0; i<USBEX_URB_NUM; i++) 
    { 
        urbHead = &urbPool[i]; 
        urbHead->cookies = URB_BUF_COOKIE; 
		_spin_lock_init(&urbHead->lock);
        URBLIST_INSERT(urbList, urbHead, next); 
    } 
    isInit = 1; 
    
    LOG_INFO " [URB] Total has %d urbs! \n", URBLIST_CNT(urbList) LOG_END
    return 0; 
} 

MMP_INT urbBufGet(struct urb** urb) 
{ 
    MMP_INT result = 0;

    URBLIST_REMOVE(urbList, (*urb), next); 
  
    if((*urb) == MMP_NULL) 
    {
        result = ERROR_USB_GET_URB_FAIL;
        ithPrintf("\n [URB] Not enough urb buffer!!! \n\n\n");
    }

    if(result)
        LOG_ERROR " urbBufGet() return error code 0x%08X \n", result LOG_END
    return result; 
} 

MMP_INT urbBufFree(struct urb* urb) 
{ 
    MMP_INT result = 0;
    if(urb == MMP_NULL) 
    { 
        result = ERROR_USB_FREE_NULL_URB;
        ithPrintf("\n [URB] Free null urb Buffer! \n\n\n"); 
        goto end;
    } 

    if(urb->cookies != URB_BUF_COOKIE) 
    { 
        result = ERROR_USB_URB_COOKIE_ERROR;
        ithPrintf("\n [URB] Cookie fail!! urb 0x%08X's cookies 0x%08X \n\n\n", urb, urb->cookies); 
        goto end;
    } 

    if(urb->type == URB_BUF_FREE) 
    { 
        result = ERROR_USB_URB_DOUBLE_FREE;
        ithPrintf("\n [URB] Double free!! urb 0x%08X \n\n\n", urb); 
        goto end;
    } 
    URBLIST_INSERT(urbList, urb, next); 

end:
    if(result)
        LOG_ERROR " urbBufFree() return error code 0x%08X \n", result LOG_END
    return result;
} 

MMP_UINT32 urbBufCount(void) 
{ 
    return URBLIST_CNT(urbList); 
} 
