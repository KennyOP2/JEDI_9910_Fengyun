/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  usb_device_scsi.c otg usb device scsi command module Control
 *	Date: 2007/10/09
 *
 * @author Jack Chain
 * @version 0.1
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
//=============================================================================
//                              Macro Definition
//=============================================================================
//=============================================================================
//                              Private Structure Definition
//=============================================================================
//=============================================================================
//                              Global Data Definition
//=============================================================================

static SENSE_DATA    scsi_sense_data; // Sense data 
static MMP_UINT32    bulk_data_size; //Size of Bulk data
static MMP_UINT32    logicblock_size;
static MMP_UINT32    lastBlockId;
extern CBW tOTGCBW;

/*------------------------------------------------------------------
 * table kind (outside variable)
 *-----------------------------------------------------------------*/

static MMP_UINT8 INQUIRY_TABLE[INQUIRY_LENGTH]={
    0x00,               /*Qualifier, device type code*/
    0x80,               /*RMB, device type modification child*/
    0x00,               /*ISO Version, ECMA Version, ANSI Version*/
    0x01,               /*Response data form*/
    0x1F,               /*addition data length*/            
    0x00,0x00,0x00,     /*reserved*/
    'G','e','n','e','r','i','c',' ',                                    /*vender ID*/
    'S','t','o','r','a','g','e',' ','D','e','v','i','c','e',' ',' ',    /*product ID*/
    '1','.','0','0'                                                     /*Product Revision*/
};


//MMP_UINT8 MODE_SENSE6_TABLE[MODE_SENSE6_LENGTH]={
//	0x17,               /*length of the mode parameter*/
//	0x00,               /*medium type*/
//	0x00,               /*device peculiar parameter*/
//	0x08,               /*length of the block descriptor*/
//	0x00,               /*density code*/
//	0x00,0x00,0xC0,     /*number of the blocks*/
//	0x00,               /*Reserved*/
//	0x00,0x02,0x00,     /*length of the block*/
//	0x81,               /*PS, page code*/
//	0x0A,               /*length of the page*/
//	0x08,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00	/*mode parameter*/
//};

static MMP_UINT8 MODE_SENSE6_TABLE[MODE_SENSE6_LENGTH]={
	0x03,               /*length of the mode parameter*/
	0x00,               /*medium type*/
	0x00,               /*device peculiar parameter*/
	0x00,               /*length of the block descriptor*/
};

static MMP_UINT8 MODE_SENSE10_TABLE[MODE_SENSE10_LENGTH]={
	0x00,0x1A,          /*length of the mode parameter*/
	0x00,               /*medium type*/
	0x00,               /*device peculiar parameter*/
	0x00,0x00,          /*Reserved*/
	0x00,0x08,          /*length of the block descriptor*/
	0x00,               /*density code*/
	0x00,0x00,0xC0,     /*number of the blocks*/
	0x00,               /*Reserved*/
	0x00,0x02,0x00,     /*length of the block*/
	0x81,               /*PS, page code*/
	0x0A,               /*length of the page*/
	0x08,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00	/*mode parameter*/
};


static MMP_UINT8 MODE_SENSE_TABLE[MODE_SENSE_LENGTH]={
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*Mode Parameter Header:8 Byte*/
    0x01,0x0A,0x00,0xFF,0x00,0x00,0x00,0x00, /*Read-Write Error Recovery Page: 12 Byte*/
    0xFF,0x00,0x00,0x00,
    0x05,0x1E,0x00,0xFA,0x02,0x08,0x02,0x00, /*Flexible Disk Page: 32 Byte*/
    0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x05,0x30,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x01,0x68,0x00,0x00,
    0x1B,0x0A,0x00,0x01,0x00,0x00,0x00,0x00, /*Removable Block Access Capacities Page: 12 Byte*/
    0x00,0x00,0x00,0x00,
    0x1C,0x06,0x00,0x05,0x00,0x00,0x00,0x00  /*Timer and Protect Page: 8 Byte*/
};

static MMP_UINT8 MODE_SENSE_MASK[MODE_SENSE_LENGTH]={
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*Mode Parameter Header:8 Byte*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*Read-Write Error Recovery Page: 12 Byte*/
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*Flexible Disk Page: 32 Byte*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*Removable Block Access Capacities Page: 12 Byte*/
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  /*Timer and Protect Page: 8 Byte*/
};

static MMP_UINT8 MODE_SELECT_TABLE[MODE_SELECT_LENGTH]={
	0x17,               /*length of the mode parameter*/
	0x00,               /*medium type*/
	0x00,               /*device peculiar parameter*/
	0x08,               /*length of the block descriptor*/
	0x00,               /*density code*/
	0x00,0x00,0xC0,     /*number of the blocks*/
	0x00,               /*Reserved*/
	0x00,0x02,0x00,     /*length of the block*/
	0x01,               /*PS, page code*/
	0x0A,               /*length of the page*/
	0x08,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00	/*mode parameter*/
};

static MMP_UINT8 MODE_SELECT10_TABLE[MODE_SELECT10_LENGTH]={
	0x00,0x1A,          /*length of the mode parameter*/
	0x00,               /*medium type*/
	0x00,               /*device peculiar parameter*/
	0x00,0x00,          /*Reserved*/
	0x00,0x08,          /*length of the block descriptor*/
	0x00,               /*density code*/
	0x00,0x00,0xC0,     /*number of the blocks*/
	0x00,               /*Reserved*/
	0x01,               /*PS, page code*/
	0x00,0x02,0x00,     /*length of the block*/
	0x0A,               /*length of the page*/
	0x08,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00	/*mode parameter*/
};

static MMP_UINT8 READ_CAPACITY_TABLE[READ_CAPACITY_LENGTH]={    /*big endian*/
//	0x00,0x00,0x00,0xBF,            /*number of the outline reason blocks - 1*/
//	0x00,0x00,0x02,0x00             /*size of the data block(Byte?)*/
	0x00,0x00,0x1F,0xFF,            /*number of the outline reason blocks - 1*/
	0x00,0x00,0x02,0x00             /*size of the data block(Byte?)*/
};

static MMP_UINT8 READ_FORMAT_CAPACITY_TABLE[READ_FORM_CAPA_LENGTH]={
    0x00,0x00,0x00,0x08,                        /*Capacity List Header*/
    0x00,0x00,0x1F,0xFF,0x02,0x00,0x02,0x00    /*Current/Maximum Capacity Header*/
//    0x00,0x00,0x1F,0xFF,0x00,0x00,0x02,0x00     /*Formattable Capacity Descriptor*/
};

static MMP_UINT8 REQUEST_SENSE_TABLE[REQUEST_SENSE_LENGTH]={
    0x70,                   /*Error Code*/
    0x00,
    0x00,                   /*Sense Key*/
    0x00,0x00,0x00,0x00,    /*Information*/
    0x0A,                   /*Additional Sense Length*/
    0x00,0x00,0x00,0x00,
    0x00,                   /*Additional Sense Code*/
    0x00,                   /*Additional Sense Code Qualifier*/
    0x00,0x00,0x00,0x00
};


//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=========================================================================
/**
 * Dummy function
 */
//=========================================================================
MMP_INT
USB_DEVICE_GetMaxLunNum(
    MMP_UINT8* lunNum)
{
    LOG_INFO "USB_DEVICE_GetMaxLunNum() dummy function! \n" LOG_END
    return 0;
}

MMP_INT
USB_DEVICE_Initialize(
    MMP_UINT8 lun)
{
    LOG_INFO "USB_DEVICE_Initialize() dummy function! \n" LOG_END
    return 0;
}

MMP_INT
USB_DEVICE_Terminate(
    MMP_UINT8 lun)
{
    LOG_INFO "USB_DEVICE_Terminate() dummy function! \n" LOG_END
    return 0;
}

MMP_INT
USB_DEVICE_GetCapacity(
    MMP_UINT8 lun,
    MMP_UINT32* lastBlockId,
    MMP_UINT32* blockLength)
{
    LOG_INFO "USB_DEVICE_GetCapacity() dummy function! \n" LOG_END
    return 0;
}

MMP_INT
USB_DEVICE_ReadSector(
    MMP_UINT8 lun,
    MMP_UINT32  blockId, 
    MMP_UINT32  sizeInSector, 
    MMP_UINT16* srcBuffer)
{
    LOG_INFO "USB_DEVICE_ReadSector() dummy function! blockId = %d, sizeInSector = %d \n", blockId, sizeInSector LOG_END
    return 0;
}

MMP_INT
USB_DEVICE_WriteSector(
    MMP_UINT8 lun,
    MMP_UINT32  blockId, 
    MMP_UINT32  sizeInSector, 
    MMP_UINT16* dstBuffer)
{
    LOG_INFO "USB_DEVICE_WriteSector() dummy function! blockId = %d, sizeInSector = %d \n", blockId, sizeInSector LOG_END
    return 0;
}

MMP_OTG_DEVICE_STATUS
USB_DEVICE_Response(
    MMP_UINT8 lun)
{
    LOG_INFO "USB_DEVICE_Response() dummy function! \n" LOG_END
    return 0;
}

MMP_INT
USB_DEVICE_Inquiry(
    MMP_UINT8  lun,
    MMP_UINT8** inquiryData)
{
    LOG_INFO "USB_DEVICE_Inquiry() dummy function! \n" LOG_END
    return 0;
}

MMP_BOOL
USB_DEVICE_IsLock(
    MMP_UINT8  lun)
{
    LOG_INFO "USB_DEVICE_IsLock() dummy function! \n" LOG_END
    return MMP_FALSE;
}

MMP_INT
USB_DEVICE_Eject(
    MMP_UINT8  lun)
{
    LOG_INFO "USB_DEVICE_Eject() dummy function! \n" LOG_END
    return 0;
}

OTG_DEVICE_GET_MAX_LUN_NUM  OTG_DEVICE_GetMaxLunNum = USB_DEVICE_GetMaxLunNum;
OTG_DEVICE_INITIALIZE   OTG_DEVICE_Initialize  = USB_DEVICE_Initialize;
OTG_DEVICE_TERMINATE    OTG_DEVICE_Terminate   = USB_DEVICE_Terminate;
OTG_DEVICE_GET_CAPACITY OTG_DEVICE_GetCapacity = USB_DEVICE_GetCapacity;
OTG_DEVICE_READ_SECTOR  OTG_DEVICE_ReadSector  = USB_DEVICE_ReadSector;
OTG_DEVICE_WRITE_SECTOR OTG_DEVICE_WriteSector = USB_DEVICE_WriteSector;
OTG_DEVICE_RESPONSE     OTG_DEVICE_Response    = USB_DEVICE_Response;
OTG_DEVICE_INQUIRY      OTG_DEVICE_Inquiry     = USB_DEVICE_Inquiry;
OTG_DEVICE_IS_LOCK      OTG_DEVICE_IsLock      = USB_DEVICE_IsLock;
OTG_DEVICE_EJECT        OTG_DEVICE_Eject       = USB_DEVICE_Eject;


static MMP_INT _CheckCardResponse(void)
{
    MMP_INT result = 0;
    MMP_OTG_DEVICE_STATUS mediaStatus = 0;

    mediaStatus = OTG_DEVICE_Response(tOTGCBW.u8LUN);
    if(mediaStatus != MMP_OTG_MEDIA_READY)
    {
        if(mediaStatus == MMP_OTG_MEDIA_NOT_PRESENT)
        {
            scsi_sense_data.sense_key = NOT_READY;
            scsi_sense_data.asc       = 0x3A;
        }
        else if(mediaStatus == MMP_OTG_MEDIA_CHANGE)
        {
            scsi_sense_data.sense_key = UNIT_ATTENTION;
            scsi_sense_data.asc       = 0x28;
        }

        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_CARD_NO_RESPONSE;
        goto end;
    }

end:
    return result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
//=========================================================================
// The function which Init scsi device
//
// Arguments:
// Return values:
//      TRUE        - normal ending
//      FALSE       - command not to be supporting
// Overview:
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Initialize(void)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_Initialize]  Enter \n" LOG_END

    result = OTG_DEVICE_Initialize(tOTGCBW.u8LUN);

    LOG_LEAVE "[USB_DEVICE_SCSI_Initialize] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Initialize() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The function which terminate scsi device
//
// Arguments:
// Return values:
//      TRUE        - normal ending
//      FALSE       - command not to be supporting
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Terminate(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[USB_DEVICE_SCSI_Terminate]  Enter \n" LOG_END

    result = OTG_DEVICE_Terminate(tOTGCBW.u8LUN);

    LOG_LEAVE "[USB_DEVICE_SCSI_Terminate] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Terminate() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The function which begins the generation of the MMC command
// MMP_INT USB_DEVICE_SCSI_Command(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//      I : MMP_UINT8*      ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//      I : MMP_UINT8*      pbData         - EndPoint buffer
//      I : MMP_UINT32*     plDataSize      - input/output data size
//      I : MMP_UINT8       TransFlag      - direction of the transfer
// Return values:
//      TRUE        - normal ending
//      FALSE       - command not to be supporting
// Overview:
//         It analyzes a UFI command in CBWCB and
//      it summons processing every command.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Command(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_Command] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",
                ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END
    
    ///*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    //** It summons processing according to the contents of the command.
    //**::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    switch(ScsiCommandBuf[0]) 
    {
    // No data Access
    case VERIFY:
        LOG_CMD "[scsi_command] VERIFY() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Verify(ScsiCommandBuf, TransFlag);
        break;

    case TEST_UNIT_READY:
        LOG_CMD "[scsi_command] TEST_UNIT_READY() !!\n" LOG_END
        result = USB_DEVICE_SCSI_TestUnitReady(ScsiCommandBuf, TransFlag);
        break;

    case SEEK:
        LOG_CMD "[scsi_command] SEEK() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Seek(ScsiCommandBuf, TransFlag);
        break;

    case START_STOP_UNIT:
        LOG_CMD "[scsi_command] START_STOP_UNIT() !!\n" LOG_END
        result = USB_DEVICE_SCSI_StartStopUnit(ScsiCommandBuf, TransFlag);
        break;

    case PREVENT_ALLOW_MEDIUM_REMOVAL:
        LOG_CMD "[scsi_command] PREVENT_ALLOW_MEDIUM_REMOVAL() !!\n" LOG_END
        result = USB_DEVICE_SCSI_PreventAllowMediumRemoval(ScsiCommandBuf, TransFlag);
        break;

    case REZERO_UNIT:
        LOG_CMD "[scsi_command] REZERO_UNIT() !!\n" LOG_END
        result = USB_DEVICE_SCSI_RezeroUnit(ScsiCommandBuf, TransFlag);
        break;

    case SEND_DIAGNOSTIC:
        LOG_CMD "[scsi_command] SEND_DIAGNOSTIC() !!\n" LOG_END
        result = USB_DEVICE_SCSI_SendDiagnostic(ScsiCommandBuf, TransFlag);
        break;

    // Read Access
    case REQUEST_SENSE:
        LOG_CMD "[scsi_command] REQUEST_SENSE() !!\n" LOG_END
        result = USB_DEVICE_SCSI_RequestSense(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case INQUIRY:
        LOG_CMD "[scsi_command] INQUIRY() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Inquiry(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case READ6:
        LOG_CMD "[scsi_command] READ6() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Read6(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case READ10:
        LOG_CMD "[scsi_command] READ10() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Read10(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;
	
    case READ12:
        LOG_CMD "[scsi_command] READ12() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Read12(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;
	
    case MODE_SENSE10:
        LOG_CMD "[scsi_command] MODE_SENSE10() !!\n" LOG_END
        result = USB_DEVICE_SCSI_ModeSense10(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case MODE_SENSE6:
        LOG_CMD "[scsi_command] MODE_SENSE6() !!\n" LOG_END
        result = USB_DEVICE_SCSI_ModeSense6(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case READ_FORMAT_CAPACITY:
        LOG_CMD "[scsi_command] READ_FORMAT_CAPACITY() !!\n" LOG_END
        result = USB_DEVICE_SCSI_ReadFormatCapacity(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case READ_CAPACITY:
        LOG_CMD "[scsi_command] READ_CAPACITY() !!\n" LOG_END
        result = USB_DEVICE_SCSI_ReadCapacity(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;;

    // Write Access
    case FORMAT_UNIT:
    case WRITE6:
    case WRITE10:
    case WRITE12:
    case MODE_SELECT:
    case MODE_SELECT10:
    case WRITE_AND_VERIFY: 
        LOG_DEBUG "[scsi_command] FORMAT_UNIT, WRITE6,WRITE10,MODE_SELECT,MODE_SELECT10,WRITE_AND_VERIFY!!\n" LOG_END
        if (TransFlag == NO_TRANSMIT)
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x20; ///* Invalid command operation code 
            scsi_sense_data.ascq      = 0x00;
            result = ERROR_USB_DEVICE_SCSI_RECEIVE_UNSUPPORT_CMD;		
        }
        else
        {    	
        result = 0;		
        }
        break;

    default:
        LOG_ERROR "[USB_DEVICE_SCSI_Command] ERROR SCSI Command = 0x%4x\n",ScsiCommandBuf[0] LOG_END
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x20; ///* Invalid command operation code 
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_RECEIVE_UNSUPPORT_CMD;		
        break;
    }

    LOG_LEAVE "[USB_DEVICE_SCSI_Command] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Command() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The function which begins the generation of the MMC command
// MMP_INT USB_DEVICE_SCSI_Receive(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//      I : MMP_UINT8*      ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//      I : MMP_UINT8*      pbData         - EndPoint buffer
//      I : MMP_UINT32*     plDataSize      - input/output data size
//      I : MMP_UINT8       TransFlag      - direction of the transfer
// Return values:
//      TRUE        - normal ending
//      FALSE       - command not to be supporting
// Overview:
//         It analyzes a UFI command in CBWCB and
//      it summons processing every command.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Receive(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_Receive] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    ///*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    //** It summons processing according to the contents of the command.
    //**::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    switch(ScsiCommandBuf[0]) 
    {
    // No data Access
    case VERIFY:
    case TEST_UNIT_READY:
    case SEEK:
    case START_STOP_UNIT:
    case PREVENT_ALLOW_MEDIUM_REMOVAL:
    case REZERO_UNIT:
    case SEND_DIAGNOSTIC:
    //Read Access
    case REQUEST_SENSE:
    case INQUIRY:
    case READ6:
    case READ10:
    case READ12:
    case MODE_SENSE10:
    case MODE_SENSE6:
    case READ_FORMAT_CAPACITY:
    case READ_CAPACITY:
        LOG_DEBUG "[USB_DEVICE_SCSI_Receive] VERIFY, TEST_UNIT_READY,SEEK,START_STOP_UNIT,PREVENT_ALLOW_MEDIUM_REMOVAL,REZERO_UNIT Other Command!!\n" LOG_END
        result = 0;		  	
        break;
		
    //Write Access
    case FORMAT_UNIT:
        LOG_CMD "[scsi_receive] FORMAT_UNIT() !!\n" LOG_END
        result = USB_DEVICE_SCSI_FormatUnit(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case WRITE6:
        LOG_CMD "[scsi_receive] WRITE6() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Write6(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case WRITE10:
        LOG_CMD "[scsi_receive] WRITE10() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Write10(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case WRITE12:
        LOG_CMD "[scsi_receive] WRITE12() !!\n" LOG_END
        result = USB_DEVICE_SCSI_Write12(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case MODE_SELECT:
        LOG_CMD "[scsi_receive] MODE_SELECT() !!\n" LOG_END
        result = USB_DEVICE_SCSI_ModeSelect(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case MODE_SELECT10:
        LOG_CMD "[scsi_receive] MODE_SELECT10() !!\n" LOG_END
        result = USB_DEVICE_SCSI_ModeSelect10(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    case WRITE_AND_VERIFY:
        LOG_CMD "[scsi_receive] WRITE_AND_VERIFY() !!\n" LOG_END
        result = USB_DEVICE_SCSI_WriteVerify(ScsiCommandBuf, pbData, plDataSize, TransFlag);
        break;

    default:
        LOG_ERROR "[scsi_receive] ERROR SCSI Command = 0x%4x\n",ScsiCommandBuf[0] LOG_END
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x20; // Invalid command operation code 
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_RECEIVE_UNSUPPORT_CMD;		
    }

    LOG_LEAVE "[USB_DEVICE_SCSI_Receive] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Receive() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of VERIFY command.
// MMP_INT USB_DEVICE_SCSI_Verify(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*      ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8       TransFlag      - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_NODATA - mistake to the direction of the transfer
// Overview:
//        This command checks the logic data block
//        where an initiator was specified and reports the result.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Verify(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr = 0;
    MMP_UINT16 verifyLength = 0;
	
    LOG_ENTER "[USB_DEVICE_SCSI_Verify] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
        goto end;

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }

    //logic block address
    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                            + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                            + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                            + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    verifyLength = (((MMP_UINT16)ScsiCommandBuf[7] & 0xFF) <<8)
                    + (((MMP_UINT16)ScsiCommandBuf[8] & 0xFF) <<0);

    LOG_INFO " LogicalBlockAdr = %d, verifyLength = %d \n", LogicalBlockAdr, verifyLength LOG_END
    
    if((LogicalBlockAdr+verifyLength-1) > lastBlockId)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Verify]  It doesn't support verification with the data !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    if( (ScsiCommandBuf[1] & 0xF3) != 0x00 )
    { 
        // It doesn't support verification with the data 
        // which is sent from the host.                  
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Verify]  It doesn't support verification with the data !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //Number of block == 0
    if( ScsiCommandBuf[7] == 0 && ScsiCommandBuf[8] == 0 ) 
    {
        goto end;
    }

    //check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Verify] Leave\n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Verify() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the TEST UNIT READY command of the UFI.
// MMP_INT USB_DEVICE_SCSI_TestUnitReady(INT8U *ScsiCommandBuf, INT8U TransFlag)
//
// Arguments:
//        I : MMP_UINT8*      ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8       TransFlag      - direction of the data transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
// Overview:
//        This command reports the condition of the logical unit to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_TestUnitReady(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_TestUnitReady] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    ///*check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; ///*No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

    result = _CheckCardResponse();
    if(result)
        goto end;

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_TestUnitReady] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_TestUnitReady() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the SEEK10 command of the UFI.
// MMP_INT USB_DEVICE_SCSI_Seek(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*     ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8      TransFlag      - direction of the data transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
// Overview:
//        This command works in the seeking to the position
//        on the specified record medium.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Seek(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_Seek] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    if( (ScsiCommandBuf[1] & 0xE0) != 0x00 ) 
    {    ///*Logical Unit Number
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Seek]  It doesn't support verification with the data !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
    
    result = _CheckCardResponse();
    if(result)
        goto end;
    
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Seek] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Seek() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the START STOP UNIT command of the UFI.
// MMP_INT USB_DEVICE_SCSI_StartStopUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*       ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8        TransFlag      - direction of the data transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
// Overview:
//        This command makes it of it impossible for it to make
//        access to the medium with logical unit possible.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_StartStopUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_StartStopUnit] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
        goto end;

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    if( (ScsiCommandBuf[1] & 0xE0) != 0x00 ) 
    {    //Logical Unit Number
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_StartStopUnit]  It doesn't support verification with the data !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

    if(ScsiCommandBuf[4] & 0x02)
        result = OTG_DEVICE_Eject(tOTGCBW.u8LUN);

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_StartStopUnit] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_StartStopUnit() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the PREVENT_ALLOW_MEDIUM_REMOVAL command of the UFI.
// MMP_INT USB_DEVICE_SCSI_PreventAllowMediumRemoval(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*      ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8       TransFlag      - direction of the data transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
// Overview:
//        This command tells the UFI device to enable or disable  
//        the removal of the medium in the logical unit.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_PreventAllowMediumRemoval(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_PreventAllowMediumRemoval] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    LOG_INFO " PREVENT-ALLOW MEDIUM REMOVAL: prevent = 0x%X \n", ScsiCommandBuf[4] LOG_END
    if( ((ScsiCommandBuf[1] & 0xE0) != 0x00) ||
         (ScsiCommandBuf[4] & 0x01))
    {    //Logical Unit Number
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    //check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
    
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_PreventAllowMediumRemoval] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_PreventAllowMediumRemoval() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the REZERO UNIT command of the UFI.
// MMP_INT USB_DEVICE_SCSI_RezeroUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*     ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8      TransFlag      - direction of the data transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
// Overview:
//        This command positions the head of the drive to the cylinder 0.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_RezeroUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_RezeroUnit] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    if((ScsiCommandBuf[1] & 0xE0) != 0x00) 
    {    //Logical Unit Number
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_RezeroUnit]  It doesn't support verification with the data !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_RezeroUnit] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_RezeroUnit() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the SEND DIAGNOSTIC command of the UFI.
// MMP_INT USB_DEVICE_SCSI_SendDiagnostic(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*     ScsiCommandBuf - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8      TransFlag      - direction of the data transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
// Overview:
//        This command requests the UFI device to do a reset or 
//      perform a self-test.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_SendDiagnostic(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_SendDiagnostic] ScsiCommandBuf = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    if((ScsiCommandBuf[1] & 0xE0) != 0x00) 
    {    //Logical Unit Number
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_SendDiagnostic]  It doesn't support verification with the data !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //check to the direction of the transfer
    if(TransFlag != NO_TRANSMIT)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_SendDiagnostic] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_SendDiagnostic() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of the REQUEST SENSE command of the UFI.
// MMP_INT USB_DEVICE_SCSI_RequestSense(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32* plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*  pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERR_NODATA  - mistake to the direction of the transfer
//        DEV_ERR_READ    - reading data from the SCSI occurred
// Overview:
//        This command transfers the sense data
//        which the target maintains to the initiator and clears the sense data.
//
//        It handles the 1st byte and
//        the 5th byte(the control byte) of CDB by the 00h fixation.
//
//        When making the allocation length of CDB effective to 0 - 17 bytes and
//        in case of equal to or more than 18 byte value's being specified,
//        at 18 bytes, it breaks off a transfer.
//
//        When the allocation length is 0, in SCSI-2, it ends this command
//        without target's transferring sense data and because it assumes that
//        it makes the sense data to maintain at this time, too, invalid,
//        it follows this.
//
//        It ignores Flag bit and Link bit of the control byte.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_RequestSense(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32* plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_RequestSense] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    //length of the allocation of CDB
    bulk_data_size = (MMP_UINT32)ScsiCommandBuf[4];
    if(bulk_data_size > REQUEST_SENSE_LENGTH)
    {
        bulk_data_size = REQUEST_SENSE_LENGTH;
    }
    if(bulk_data_size > *plDataSize) 
    {
        bulk_data_size = *plDataSize;
    }
    *plDataSize = bulk_data_size;
    
    REQUEST_SENSE_TABLE[2]  = scsi_sense_data.sense_key & 0x0F;
    REQUEST_SENSE_TABLE[12] = scsi_sense_data.asc;
    REQUEST_SENSE_TABLE[13] = scsi_sense_data.ascq;
    memcpy(pbData, REQUEST_SENSE_TABLE, bulk_data_size);
    
    if(bulk_data_size == 0) 
    {
        if(TransFlag != NO_TRANSMIT)
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x00;
            scsi_sense_data.ascq      = 0x00;
            *plDataSize = 0;
            result = ERROR_USB_DEVICE_SCSI_DEVICE_ERROR;
            LOG_ERROR "[USB_DEVICE_SCSI_RequestSense] there is Not Transmit data !! result  = 0x%4x\n",result LOG_END
        }
        goto end;
    }
    
    if(TransFlag != SCSI_TO_USB) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_RequestSense] Leave x\n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_RequestSense() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of INQUIRY command.
// MMP_INT USB_DEVICE_SCSI_Inquiry(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK          - normal ending
//        DEV_ERROR       - extraordinary ending
//        DEV_ERR_READ    - reading data from the UFI occurred
// Overview:
//        This command reports composition information on the logical unit and
//        an attribute about the target and it to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Inquiry(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT8* inquiryData = MMP_NULL;

    LOG_ENTER "[USB_DEVICE_SCSI_Inquiry] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    switch(ScsiCommandBuf[1] & 0x01) 
    {    //EVPD
        case 0x00:  //Standard Inquiry Data
            if (ScsiCommandBuf[2] != 0)
            {
                scsi_sense_data.sense_key = ILLEGAL_REQUEST;
                scsi_sense_data.asc       = 0x24; //Invalid field in CDB
                scsi_sense_data.ascq      = 0x00;
            result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
                LOG_ERROR "[USB_DEVICE_SCSI_Inquiry] there is Not Transmit data !! result  = 0x%4x\n",result LOG_END
                goto end;
            }
            bulk_data_size = INQUIRY_LENGTH;
            break;
		
        case 0x01:  //Vital Product Data(VPD)
            break;

        default://Not supported
            break;
    }
	
    if(bulk_data_size > (MMP_UINT32)ScsiCommandBuf[4]) 
    {
        bulk_data_size = (MMP_UINT32)ScsiCommandBuf[4];
    }
    if(bulk_data_size > *plDataSize) 
    {
        bulk_data_size = *plDataSize;
    }
    *plDataSize = bulk_data_size;

    result = OTG_DEVICE_Inquiry(tOTGCBW.u8LUN, &inquiryData);
    if(result || inquiryData==MMP_NULL)
    {
        memcpy(pbData, INQUIRY_TABLE, bulk_data_size);
    }
    else
    {
        memcpy(pbData, inquiryData, bulk_data_size);
    }
    
    if(bulk_data_size == 0) 
    {
        if(TransFlag != NO_TRANSMIT) 
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x00;
            scsi_sense_data.ascq      = 0x00;
            *plDataSize = 0;
            result = ERROR_USB_DEVICE_SCSI_DEVICE_ERROR;
            LOG_ERROR "[USB_DEVICE_SCSI_Inquiry] there is Not Transmit data !! result  = 0x%4x\n",result LOG_END
        }
        goto end;
    }
    
    if(TransFlag != SCSI_TO_USB)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Inquiry] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Inquiry() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of READ6 command.
// MMP_INT USB_DEVICE_SCSI_Read6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_READ   - reading data from the UFI occurred
// Overview:
//        This command transfers data in the logic data block
//        in the specified range to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Read6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr;
    MMP_UINT32 bulk_sector_size;

    LOG_ENTER "[USB_DEVICE_SCSI_Read6] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    //logic block address
    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[1] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[3] & 0xFF);

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    //check to the direction of the transfer
    if(TransFlag != SCSI_TO_USB)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    if(ScsiCommandBuf[5] != 0x00)
    { 
        //Flag & Link
        // It doesn't support Flag bit and Link bit. 
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Read6] there is Not Support!! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    bulk_sector_size = (MMP_UINT32)ScsiCommandBuf[4];
    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_data_size = bulk_sector_size * logicblock_size;
	
    //check bulk_data_size is equal *plDataSize
    if(bulk_data_size > *plDataSize) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
    else
    {
        *plDataSize = bulk_data_size;
    }
	
    result = OTG_DEVICE_ReadSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Read6] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Read6() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of READ10 command.
// MMP_INT USB_DEVICE_SCSI_Read10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*    ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*    pbData          - EndPoint buffer
//        I : MMP_UINT32*   plDataSize      - input/output data size
//        I : MMP_UINT8     TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_READ   - reading data from the UFI occurred
// Overview:
//        This command transfers data in the logic data block
//        in the specified range to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Read10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr;
    MMP_UINT32 bulk_sector_size;

    LOG_ENTER "[USB_DEVICE_SCSI_Read10] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    //logic block address
    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                    + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
   
    //check to the direction of the transfer
    if(TransFlag != SCSI_TO_USB)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    if((ScsiCommandBuf[1] & 0xF9) != 0x00) 
    { //Flag & Link
        // It doesn't support Flag bit and Link bit. 
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Read10] there is Not Support!! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    // Transfer Length
    bulk_sector_size = (((MMP_UINT32)ScsiCommandBuf[7] & 0xFF) <<8)
                     +  ((MMP_UINT32)ScsiCommandBuf[8] & 0xFF); 
                     
    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_data_size = bulk_sector_size * logicblock_size;    
    if(bulk_data_size > *plDataSize)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
    else
    {
        *plDataSize = bulk_data_size;
    }
	
    LOG_INFO " LogicalBlockAdr = %d, sector_size = %d \n", LogicalBlockAdr, bulk_sector_size LOG_END

    result = OTG_DEVICE_ReadSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Read10] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Read10() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of READ12 command.
// MMP_INT USB_DEVICE_SCSI_Read12(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_READ   - reading data from the UFI occurred
// Overview:
//        This command transfers data in the logic data block
//        in the specified range to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Read12(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr;
    MMP_UINT32 bulk_sector_size;

    LOG_ENTER "[USB_DEVICE_SCSI_Read12] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    //logic block address
    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                    + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    //check to the direction of the transfer
    if(TransFlag != SCSI_TO_USB) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    if((ScsiCommandBuf[1] & 0xF9) != 0x00)
    { //Flag & Link
        // It doesn't support Flag bit and Link bit. 
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Read12] there is Not Support!! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    // Transfer Length
    bulk_sector_size = (((MMP_UINT32)ScsiCommandBuf[6] & 0xFF) <<24)
                     + (((MMP_UINT32)ScsiCommandBuf[7] & 0xFF) <<16)
                     + (((MMP_UINT32)ScsiCommandBuf[8] & 0xFF) <<8)
                     +  ((MMP_UINT32)ScsiCommandBuf[9] & 0xFF);

    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_data_size = bulk_sector_size * logicblock_size;    
    if(bulk_data_size > *plDataSize)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
    else
    {
        *plDataSize = bulk_data_size;
    }
	
    result = OTG_DEVICE_ReadSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Read12] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Read12() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of MODE SENSE command.
// MMP_INT USB_DEVICE_SCSI_ModeSense6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_READ   - reading data from the UFI occurred
// Overview:
//        This command transfers data in the logic data block
//        in the specified range to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ModeSense6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_ModeSense6] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    bulk_data_size  = MODE_SENSE6_LENGTH;
    if(bulk_data_size > *plDataSize) 
    {
        bulk_data_size = *plDataSize;
    }
    *plDataSize = bulk_data_size;    

    if(OTG_DEVICE_IsLock(tOTGCBW.u8LUN))
    {
        MODE_SENSE6_TABLE[2] |= 0x80;
    }
    else
    {
        MODE_SENSE6_TABLE[2] &= ~0x80;
    }
    memcpy(pbData, MODE_SENSE6_TABLE, bulk_data_size);
    
    if(bulk_data_size == 0) 
    {
        if(TransFlag != NO_TRANSMIT)
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x00;
            scsi_sense_data.ascq      = 0x00;
            result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
            LOG_ERROR "[USB_DEVICE_SCSI_ModeSense6] there is Not Read data !! result  = 0x%4x\n",result LOG_END
        }
        goto end;
    }
    
    if(TransFlag != SCSI_TO_USB) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_ModeSense6] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_ModeSense6() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of MODE SENSE command.
// MMP_INT USB_DEVICE_SCSI_ModeSense10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_READ   - reading data from the UFI occurred
// Overview:
//        This command transfers data in the logic data block
//        in the specified range to the initiator.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ModeSense10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_ModeSense10] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    bulk_data_size  = MODE_SENSE10_LENGTH;
    if(bulk_data_size > *plDataSize) 
    {
        bulk_data_size = *plDataSize;
    }
    *plDataSize = bulk_data_size;    
    memcpy(pbData, MODE_SENSE10_TABLE, bulk_data_size);
    
    if(bulk_data_size == 0) 
    {
        if(TransFlag != NO_TRANSMIT)
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x00;
            scsi_sense_data.ascq      = 0x00;
            result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        }
        goto end;
    }
    
    if(TransFlag != SCSI_TO_USB)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_ModeSense10] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_ModeSense10() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of READ FORMAT CAPACITIES command.
// MMP_INT USB_DEVICE_SCSI_ReadFormatCapacity(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK       - normal ending
//        DEV_ERROR    - extraordinary ending
//        DEV_ERR_READ - reading data from the UFI occurred
// Overview:
//        It uses this command to read the capacity of the storage device.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ReadFormatCapacity(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 lastBlockId = 0;
    MMP_UINT32 blockSize = 0;
    MMP_UINT32 blockNum = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_ReadFormatCapacity] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;    
    
    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }
    
    bulk_data_size  = READ_FORM_CAPA_LENGTH;
    if(bulk_data_size > *plDataSize) 
    {
        bulk_data_size = *plDataSize;
    }
    *plDataSize = bulk_data_size;    

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &blockSize);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    blockNum = lastBlockId + 1;
    READ_FORMAT_CAPACITY_TABLE[4] = (MMP_UINT8)((blockNum&0xFF000000)>>24);
    READ_FORMAT_CAPACITY_TABLE[5] = (MMP_UINT8)((blockNum&0x00FF0000)>>16);
    READ_FORMAT_CAPACITY_TABLE[6] = (MMP_UINT8)((blockNum&0x0000FF00)>>8);
    READ_FORMAT_CAPACITY_TABLE[7] = (MMP_UINT8)(blockNum&0x000000FF);
    READ_FORMAT_CAPACITY_TABLE[9] = (MMP_UINT8)((blockSize&0x00FF0000)>>16);
    READ_FORMAT_CAPACITY_TABLE[10] = (MMP_UINT8)((blockSize&0x0000FF00)>>8);
    READ_FORMAT_CAPACITY_TABLE[11] = (MMP_UINT8)(blockSize&0x000000FF);

    memcpy(pbData, READ_FORMAT_CAPACITY_TABLE, bulk_data_size);
    
    if(bulk_data_size == 0) 
    {
        if(TransFlag != NO_TRANSMIT) 
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x00;
            scsi_sense_data.ascq      = 0x00;
            result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        }
        goto end;
    }
    
    if(TransFlag != SCSI_TO_USB)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_ReadFormatCapacity] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_ReadFormatCapacity() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of READ CAPACITY command.
// MMP_INT USB_DEVICE_SCSI_ReadCapacity(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData         - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag      - direction of the transfer
// Return values:
//        DEV_OK       - normal ending
//        DEV_ERROR    - extraordinary ending
//        DEV_ERR_READ - reading data from the UFI occurred
// Overview:
//        It uses this command to read the capacity of the storage device.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ReadCapacity(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 lastBlockId;

    LOG_ENTER "[USB_DEVICE_SCSI_ReadCapacity] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END
    
    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;    
    
    if(TransFlag != SCSI_TO_USB)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }
    
    bulk_data_size  = READ_CAPACITY_LENGTH;
    if(bulk_data_size > *plDataSize) 
    {
        bulk_data_size = *plDataSize;
    }
    *plDataSize = bulk_data_size;  

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    READ_CAPACITY_TABLE[0] = (MMP_UINT8)((lastBlockId&0xFF000000)>>24);
    READ_CAPACITY_TABLE[1] = (MMP_UINT8)((lastBlockId&0x00FF0000)>>16);
    READ_CAPACITY_TABLE[2] = (MMP_UINT8)((lastBlockId&0x0000FF00)>>8);
    READ_CAPACITY_TABLE[3] = (MMP_UINT8)(lastBlockId&0x000000FF);
    READ_CAPACITY_TABLE[4] = (MMP_UINT8)((logicblock_size&0xFF000000)>>24);
    READ_CAPACITY_TABLE[5] = (MMP_UINT8)((logicblock_size&0x00FF0000)>>16);
    READ_CAPACITY_TABLE[6] = (MMP_UINT8)((logicblock_size&0x0000FF00)>>8);
    READ_CAPACITY_TABLE[7] = (MMP_UINT8)(logicblock_size&0x000000FF);
    memcpy(pbData, READ_CAPACITY_TABLE, bulk_data_size);
    
    if(bulk_data_size == 0)
    {
        if(TransFlag != NO_TRANSMIT)
        {
            scsi_sense_data.sense_key = ILLEGAL_REQUEST;
            scsi_sense_data.asc       = 0x00;
            scsi_sense_data.ascq      = 0x00;
            result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        }
        goto end;
    }
    
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_ReadCapacity] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_ReadCapacity() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of FORMAT UNIT command.
// MMP_INT USB_DEVICE_SCSI_FormatUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_WRITE  - writing data to the UFI occurred
// Overview:
//        This command physically format one track of diskette according
//      to the selected options.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_FormatUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr=0x001;
    MMP_UINT8  ParameterList[32];

    LOG_ENTER "[USB_DEVICE_SCSI_FormatUnit] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    //initialization of SENSE KEY
    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    if((ScsiCommandBuf[1] & 0xFF) != 0x17) 
    { 
         // LUN & FmtData & CmpList & Defect List Format
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_FormatUnit] there is Not Correct Data!! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    if((ScsiCommandBuf[7] == 0) && (ScsiCommandBuf[8] == 0)) 
        goto end;

    memcpy( ParameterList, pbData, *plDataSize);

    //check Defect List Header of parameter list
    if(((ParameterList[1] & 0x02) != 0x00) ||  ((ParameterList[3] & 0xFF) != 0x08))
    { 
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x26; //Invalid field in parameter list
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_PARAM_LIST;
        LOG_ERROR "[USB_DEVICE_SCSI_FormatUnit] Check Defect List Header of parameter list ERROR!! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //check Format Descriptor of parameter list
    if((ParameterList[4] != READ_FORMAT_CAPACITY_TABLE[12]) ||
       (ParameterList[5] != READ_FORMAT_CAPACITY_TABLE[13]) ||
       (ParameterList[6] != READ_FORMAT_CAPACITY_TABLE[14]) ||
       (ParameterList[7] != READ_FORMAT_CAPACITY_TABLE[15]) ||
       (ParameterList[9] != READ_FORMAT_CAPACITY_TABLE[17]) ||
       (ParameterList[10] != READ_FORMAT_CAPACITY_TABLE[18]) ||
         (ParameterList[11] != READ_FORMAT_CAPACITY_TABLE[19]) )
    { 
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x26; //Invalid field in parameter list
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_PARAM_LIST;
        LOG_ERROR "[USB_DEVICE_SCSI_FormatUnit] Check Format Descriptor of parameter list ERROR!! result  = 0x%4x\n",result LOG_END
        goto end;
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_FormatUnit] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_FormatUnit() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of WRITE6 command.
// MMP_INT USB_DEVICE_SCSI_Write6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_WRITE  - writing data to the UFI occurred
// Overview:
//        This command writes the data which was transferred from the initiator
//        in the logic data block in the specified range on the medium.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Write6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr;
    MMP_UINT32 bulk_sector_size;

    LOG_ENTER "[USB_DEVICE_SCSI_Write6] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    //logic block address
    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                    + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    //initialization of SENSE KEY
    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    if((ScsiCommandBuf[1] & 0xF9) != 0x00)
    { //Flag & Link
        // It doesn't support Flag bit and Link bit.  
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_Write6] It doesn't support Flag bit and Link bit.!! result  = 0x%4x\n",result LOG_END
        goto end;
    }
	
    bulk_data_size = *plDataSize;
    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_sector_size = bulk_data_size / logicblock_size;

    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
	
    result = OTG_DEVICE_WriteSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Write6] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Write6() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of WRITE10 command.
// MMP_INT USB_DEVICE_SCSI_Write10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_WRITE  - writing data to the UFI occurred
// Overview:
//        This command writes the data which was transferred from the initiator
//        in the logic data block in the specified range on the medium.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Write10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr = 0;
    MMP_UINT32 bulk_sector_size = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_Write10] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                    + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    //initialization of SENSE KEY
    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    // It doesn't support Flag bit and Link bit. 
    if(ScsiCommandBuf[9] != 0x00) 
    {   
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    // Transfer Length
    bulk_sector_size = (((MMP_UINT32)ScsiCommandBuf[7] & 0xFF) <<8)
                     +  ((MMP_UINT32)ScsiCommandBuf[8] & 0xFF); 
	
    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_data_size = bulk_sector_size * logicblock_size;
    if(bulk_data_size > *plDataSize)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
    else
    {
        *plDataSize = bulk_data_size;
    }
	
    LOG_INFO " LogicalBlockAdr = %d, sector_size = %d \n", LogicalBlockAdr, bulk_sector_size LOG_END

    result = OTG_DEVICE_WriteSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);
    
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Write10] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Write10() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of WRITE12 command.
// MMP_INT USB_DEVICE_SCSI_Write12(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_WRITE  - writing data to the UFI occurred
// Overview:
//        This command writes the data which was transferred from the initiator
//        in the logic data block in the specified range on the medium.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Write12(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr = 0;
    MMP_UINT32 bulk_sector_size = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_Write12] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    result = _CheckCardResponse();
    if(result)
    {
        *plDataSize = 0;
        goto end;
    }

    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                    + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    //initialization of SENSE KEY
    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;
    
    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    // It doesn't support Flag bit and Link bit. 
    if(ScsiCommandBuf[9] != 0x00) 
    {   
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    // Transfer Length
    bulk_sector_size = (((MMP_UINT32)ScsiCommandBuf[6] & 0xFF) <<24)
                     + (((MMP_UINT32)ScsiCommandBuf[7] & 0xFF) <<16)
                     + (((MMP_UINT32)ScsiCommandBuf[8] & 0xFF) <<8)
                     +  ((MMP_UINT32)ScsiCommandBuf[9] & 0xFF);
	
    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }

    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_data_size = bulk_sector_size * logicblock_size;
    if(bulk_data_size > *plDataSize)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
    else
    {
        *plDataSize = bulk_data_size;
    }
	
    LOG_INFO " LogicalBlockAdr = %d, sector_size = %d \n", LogicalBlockAdr, bulk_sector_size LOG_END

    result = OTG_DEVICE_WriteSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);
    
end:
    LOG_LEAVE "[USB_DEVICE_SCSI_Write12] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_Write12() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of MODE SELECT command.
// MMP_INT USB_DEVICE_SCSI_ModeSelect(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : INT8U*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : INT8U*   pbData          - EndPoint buffer
//        I : INT32U*  plDataSize      - input/output data size
//        I : INT8U    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK        - normal ending
//        DEV_ERROR     - extraordinary ending
//        DEV_ERR_WRITE - writing data to the UFI occurred
// Overview:
//        It uses this command for the USB host
//        to set a device parameter on the USB device.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ModeSelect(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_ModeSelect] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

    if(MODE_SELECT_LENGTH > *plDataSize) 
    {
        memcpy( MODE_SELECT_TABLE, pbData, *plDataSize);
    }
    else
    {
        memcpy( MODE_SELECT_TABLE, pbData, MODE_SELECT_LENGTH);
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_ModeSelect] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_ModeSelect() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================================
// The processing of MODE SELECT command.
// MMP_INT USB_DEVICE_SCSI_ModeSelect10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : INT8U*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : INT8U*   pbData          - EndPoint buffer
//        I : INT32U*  plDataSize      - input/output data size
//        I : INT8U    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK        - normal ending
//        DEV_ERROR     - extraordinary ending
//        DEV_ERR_WRITE - writing data to the UFI occurred
// Overview:
//        It uses this command for the USB host
//        to set a device parameter on the USB device.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ModeSelect10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;

    LOG_ENTER "[USB_DEVICE_SCSI_ModeSelect10] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }

    if(MODE_SELECT10_LENGTH > *plDataSize)
    {
        memcpy( MODE_SELECT10_TABLE, pbData, *plDataSize);
    }
    else 
    {
        memcpy( MODE_SELECT10_TABLE, pbData, MODE_SELECT10_LENGTH);
    }

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_ModeSelect10] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_ModeSelect10() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================================
// The processing of WRITE VERIFY command.
// MMP_INT USB_DEVICE_SCSI_WriteVerify(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : INT8U*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : INT8U*   pbData          - EndPoint buffer
//        I : INT32U*  plDataSize      - input/output data size
//        I : INT8U    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK         - normal ending
//        DEV_ERROR      - extraordinary ending
//        DEV_ERR_WRITE  - writing data to the UFI occurred
// Overview:
//        This command writes the data which was transferred from the initiator
//        in the logic data block in the specified range on the medium.
//
//        The writing data doesn't ask an intermediary to the Rx buffer
//        to transfer in DMA.    Therefore, in this program,
//        because there is not a comparative object,
//        it writes data onto the substitution memory only and
//        there is not comparison in it.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_WriteVerify(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
{
    MMP_INT result = 0;
    MMP_UINT32 LogicalBlockAdr;
    MMP_UINT32 bulk_sector_size;

    LOG_ENTER "[USB_DEVICE_SCSI_WriteVerify] ScsiCommandBuf = 0x%4x pbData = 0x%4x plDataSize = 0x%4x TransFlag = 0x%4x \n",ScsiCommandBuf ,pbData ,plDataSize ,TransFlag LOG_END

    //logic block address
    LogicalBlockAdr = (((MMP_UINT32)ScsiCommandBuf[2] & 0xFF) <<24)
                    + (((MMP_UINT32)ScsiCommandBuf[3] & 0xFF) <<16)
                    + (((MMP_UINT32)ScsiCommandBuf[4] & 0xFF) << 8)
                    + ( (MMP_UINT32)ScsiCommandBuf[5] & 0xFF);

    scsi_sense_data.sense_key = 0x00;
    scsi_sense_data.asc       = 0x00;
    scsi_sense_data.ascq      = 0x00;

    if(*plDataSize == 0 || (ScsiCommandBuf[7] | ScsiCommandBuf[8]) == 0) 
        goto end;

    //check to the direction of the transfer
    if(TransFlag != USB_TO_SCSI)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_SCSI_TX_DIR_ERROR;
        goto end;
    }
	
    if((ScsiCommandBuf[1] & 0xF3) != 0x00) 
    { // It doesn't support Flag bit and Link bit. 
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x24; //Invalid field in CDB
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        LOG_ERROR "[USB_DEVICE_SCSI_WriteVerify] It doesn't support Flag bit and Link bit. !! result  = 0x%4x\n",result LOG_END
        goto end;
    }

    //Transfer Length
    bulk_sector_size = (((MMP_UINT32)ScsiCommandBuf[7] & 0xFF) <<8)
                     +  ((MMP_UINT32)ScsiCommandBuf[8] & 0xFF); 
                     
    //check bulk_sector_size can not be zero
    if(bulk_sector_size == 0)
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
                     
    result = OTG_DEVICE_GetCapacity(tOTGCBW.u8LUN, &lastBlockId, &logicblock_size);
    if(result)
    {
        scsi_sense_data.sense_key = NOT_READY;
        scsi_sense_data.asc       = 0x3A;
        scsi_sense_data.ascq      = 0x00;
        result = ERROR_USB_DEVICE_GET_CAPACITY_FAIL;
        goto end;
    }
    bulk_data_size = bulk_sector_size * logicblock_size;
    //check bulk_data_size is equal *plDataSize
    if(bulk_data_size > *plDataSize) 
    {
        scsi_sense_data.sense_key = ILLEGAL_REQUEST;
        scsi_sense_data.asc       = 0x00; //No additional sense information
        scsi_sense_data.ascq      = 0x00;
        *plDataSize = 0;
        result = ERROR_USB_DEVICE_ILLEGAL_COMMAND;
        goto end;
    }
    else
    {
        *plDataSize = bulk_data_size;
    }
	
    result = OTG_DEVICE_WriteSector(tOTGCBW.u8LUN, LogicalBlockAdr, bulk_sector_size, (MMP_UINT16*)pbData);

end:
    LOG_LEAVE "[USB_DEVICE_SCSI_WriteVerify] Leave \n" LOG_END
    if(result)
        LOG_ERROR "USB_DEVICE_SCSI_WriteVerify() return error code 0x%08X \n", result LOG_END
    return result;
}

