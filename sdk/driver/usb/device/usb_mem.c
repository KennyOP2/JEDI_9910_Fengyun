/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *    usb_memm.c USB memory manager
 *
 */

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_UINT8*   dmaAddr = MMP_NULL;

//=============================================================================
//                              Public Function Definition
//=============================================================================



//=============================================================================
/**
 * Allocate DMA buffer for Device DMA use.
 */
//=============================================================================
static MMP_INT
USB_DEVICE_AllocateDmaAddr(
    MMP_UINT8** addr,
    MMP_UINT32  size)
{
    MMP_INT     result = 0;
    MMP_UINT8   memoryStatus = 0;

    if(dmaAddr)
    {
        memoryStatus = MEM_Release((MMP_UINT8*)dmaAddr);
        if(memoryStatus != MEM_STATUS_SUCCESS)
        {
            result = ERROR_USB_RELEASE_DMA_BUFFER_FAIL;
            goto end;
        }
        dmaAddr = MMP_NULL;
    }

    dmaAddr = (MMP_UINT8*)MEM_Allocate((size+4), MEM_USER_USB_DEVICE, MEM_USAGE_DMA);
    if(dmaAddr == MMP_NULL)
    {
        result = ERROR_USB_ALLOCATE_DMA_BUFFER_FAIL;
        goto end;
    }
    (*addr) = (MMP_UINT8*)(((MMP_UINT32)dmaAddr + 3) & ~3);

end:
    if(result)
        LOG_ERROR "USB_DEVICE_AllocateDmaAddr() return error code 0x%08X \n", result LOG_END

    return result;
}

//=============================================================================
/**
 * Release DMA buffer.
 */
//=============================================================================
static MMP_INT
USB_DEVICE_ReleaseDmaAddr(
    void)
{
    MMP_INT     result = 0;
    MMP_UINT8   memoryStatus = 0;

    if(dmaAddr)
    {
        memoryStatus = MEM_Release((MMP_UINT8*)dmaAddr);
        if(memoryStatus != MEM_STATUS_SUCCESS)
        {
            result = ERROR_USB_RELEASE_DMA_BUFFER_FAIL;
            goto end;
        }
        dmaAddr = MMP_NULL;
    }

end:
    if(result)
        LOG_ERROR "USB_DEVICE_ReleaseDmaAddr() return error code 0x%08X \n", result LOG_END

    return result;
}
