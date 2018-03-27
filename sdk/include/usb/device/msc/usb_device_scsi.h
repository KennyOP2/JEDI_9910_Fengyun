/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as OTG USB Device SCSI Mode file.
 * Date: 2007/10/17
 *
 * @author Jack Chain
 * @version 0.01
 */

#ifndef USB_DEVICE_SCSI_H
#define USB_DEVICE_SCSI_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Type Definition
//=============================================================================
//=============================================================================
//                              Constant Definition
//=============================================================================

/*--------------------------------------------
 * Direction of transmit
 *-------------------------------------------*/
#define USB_TO_SCSI      2
#define SCSI_TO_USB      1
#define NO_TRANSMIT      0

/*--------------------------------------------
 * SCSI commands
 *-------------------------------------------*/
#define TEST_UNIT_READY               0x00
#define REQUEST_SENSE                 0x03
#define FORMAT_UNIT                   0x04
#define READ6                         0x08
#define WRITE6                        0x0A
#define SEEK                          0x0B
#define INQUIRY                       0x12
#define MODE_SELECT                   0x15
#define RELEASE                       0x17
#define MODE_SENSE6                   0x1A
#define START_STOP_UNIT               0x1B
#define SEND_DIAGNOSTIC               0x1D
#define READ_FORMAT_CAPACITY          0x23
#define READ_CAPACITY                 0x25
#define READ10                        0x28
#define MODE_SELECT10                 0x55
#define MODE_SENSE10                  0x5A
#define PREVENT_ALLOW_MEDIUM_REMOVAL  0x1E
#define REZERO_UNIT                   0x01
#define SEND_DIAGNOSTIC               0x1D
#define VERIFY                        0x2F
#define WRITE10                       0x2A
#define WRITE_AND_VERIFY              0x2E
#define READ12                        0xA8
#define WRITE12                       0xAA

/*--------------------------------------------
 * Sense key
 *-------------------------------------------*/
#define NO_SENSE                0x00
#define RECOVERED_ERROR         0x01
#define NOT_READY               0x02
#define MEDIUM_ERROR            0x03
#define HARDWARE_ERROR          0x04
#define ILLEGAL_REQUEST         0x05
#define UNIT_ATTENTION          0x06
#define DATA_PROTECT            0x07
#define BLANK_CHECK             0x08
#define VENDER_SPECIFIC         0x09
#define COPY_ABORTED            0x0a
#define ABORTED_COMMAND         0x0b
#define EQUAL                   0x0c
#define VOLUME_OVERFLOW         0x0d
#define MISCOMPARE              0x0e

/*--------------------------------------------
 * data length of the table
 *-------------------------------------------*/
#define INQUIRY_LENGTH          36     /*36Byte*/
#define	MODE_SENSE_LENGTH       72     /*24Byte*/
#define	MODE_SENSE6_LENGTH      4     /*4Byte*/
#define	MODE_SENSE10_LENGTH     28     /*28Byte*/
#define	MODE_SELECT_LENGTH      24     /*24Byte*/
#define	MODE_SELECT10_LENGTH    28     /*28Byte*/
#define REQUEST_SENSE_LENGTH    18     /*18Byte*/
#define READ_FORM_CAPA_LENGTH   12     /*20Byte*/
#define READ_CAPACITY_LENGTH    8     /*8Byte*/

//=============================================================================
//                              Type Definition
//=============================================================================

typedef struct _SENSE_DATA {
    MMP_UINT8    sense_key;
    MMP_UINT8    asc;
    MMP_UINT8    ascq;
} SENSE_DATA;


//=============================================================================
//                              Function Declaration
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
USB_DEVICE_SCSI_Initialize(void);

//=========================================================================
// The function which terminate scsi device
//
// Arguments:
// Return values:
//      TRUE        - normal ending
//      FALSE       - command not to be supporting
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_Terminate(void);

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
USB_DEVICE_SCSI_Command(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Receive(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Verify(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_TestUnitReady(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Seek(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_StartStopUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_PreventAllowMediumRemoval(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_RezeroUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_SendDiagnostic(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_RequestSense(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32* plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Inquiry(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Read6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Read10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Read12(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_ModeSense6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_ModeSense10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_ReadFormatCapacity(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_ReadCapacity(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_FormatUnit(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Write6(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Write10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_Write12(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_ModeSelect(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

//=========================================================================
// The processing of MODE SELECT command.
// MMP_INT USB_DEVICE_SCSI_ModeSelect10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag)
//
// Arguments:
//        I : MMP_UINT8*   ScsiCommandBuf  - CBWCB at the time of the UFI protocol
//        I : MMP_UINT8*   pbData          - EndPoint buffer
//        I : MMP_UINT32*  plDataSize      - input/output data size
//        I : MMP_UINT8    TransFlag       - direction of the transfer
// Return values:
//        DEV_OK        - normal ending
//        DEV_ERROR     - extraordinary ending
//        DEV_ERR_WRITE - writing data to the UFI occurred
// Overview:
//        It uses this command for the USB host
//        to set a device parameter on the USB device.
//=========================================================================
MMP_INT
USB_DEVICE_SCSI_ModeSelect10(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);

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
USB_DEVICE_SCSI_WriteVerify(MMP_UINT8 *ScsiCommandBuf, MMP_UINT8 *pbData, MMP_UINT32 *plDataSize, MMP_UINT8 TransFlag);


#ifdef __cplusplus
}
#endif

#endif
