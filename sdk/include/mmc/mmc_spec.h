//*****************************************************************************
// Name: mmc_Spec.h
//
// Description:
//     MMC spec head file
//
// Author: Peter Lin
//  Date: 2004/03/25
//
// Copyright(c)2003-2005 Silicon Integrated Systems Corp. All rights reserved.
//*****************************************************************************

#ifndef __MMC_SPEC_H
#define __MMC_SPEC_H

#include "mmp_types.h"

typedef struct _MMC_CID
{
    MMP_UINT8    MID;        // Manufacturer ID          D[127:120]
    MMP_UINT16    OID;        // OEM/Application ID       D[119:104]
    MMP_UINT8    PNM[6];     // Product name             D[103:56]
    MMP_UINT8    PRV;        // Product revision         D[55:48]
    MMP_UINT8    PSN[4];     // Product serial number    D[47:16]
    MMP_UINT8    MDT;        // Manufacturer date        D[15:8]
    MMP_UINT8    CRC;    // CRC7 checksum            D[7:1]
    MMP_UINT8    X ;    // Not used, always 1       D[0:0]
} MMC_CID_DATA, *PMMC_CID_DATA; 

typedef struct _MMC_CSD
{
    MMP_UINT8    CSDstructure    ;    // CSD Structure                D[127:126]
    MMP_UINT8    specVer         ;    // System Spec Version          D[125:122]
    MMP_UINT8    reserved1       ;    // reserved                     D[121:120]
    MMP_UINT8    TAAC;                   // Data read access-time-1      D[119:112]
    MMP_UINT8    NSAC;                   // Data read access-time-2      D[111:104]
                                    // in CLK cycles (NSAC * 100)
    MMP_UINT8    transSpeed;             // Max data transfer rate       D[103:96]
    MMP_UINT16    CCC            ;    // Card command classes         D[95:84]
    MMP_UINT8    readBlkLen      ;     // Max read data block LEN      D[83:80]
    MMP_UINT8    readBlkPartial   ;     // Partial BLKs for Rd allowed   D[79:79]
    MMP_UINT8    writeBlkMisAlign;     // write BLK misalignment       D[78:78]
    MMP_UINT8    readBlkMisAlign ;     // read BLK misalignment        D[77:77]
    MMP_UINT8    DSRImp          ;     // DSR implemented              D[76:76]
    MMP_UINT8    reserved2       ;     // reserved                     D[75:74]
    MMP_UINT16    cSize          ;    // Device size                  D[73:62]
    MMP_UINT8    VDDRdCurrMin    ;     // Max Rd current @Vdd min      D[61:59]
    MMP_UINT8    VDDRdCurrMax    ;     // Max Rd current @Vdd max      D[58:56]
    MMP_UINT8    VDDWrCurrMin    ;     // Max Wr current @Vdd min      D[55:53]
    MMP_UINT8    VDDWrCurrMax    ;     // Max Wr current @Vdd max      D[52:50]
    MMP_UINT8    cSizeMult       ;     // device size multiplier       D[49:47]

    union 
    {
        struct 
        {
            MMP_UINT8    eraseGrpSize    ;     // Erase group size             D[46:42]
            MMP_UINT8    eraseGrpMult    ;     // Erase group size multiplier  D[41:37]
            MMP_UINT8    wpGrpSize       ;     // Wr protect group size        D[36:32]
            MMP_UINT8    wpGrpEnable     ;     // Wr protect group enable      D[31:31]
   
        }MMC_ERASE;    

        struct 
        {
            MMP_UINT8    eraseBlkEn      ;     // Erase single block enable    D[46:46]
            MMP_UINT8    sectorSize      ;     // Erase sector size            D[45:39]
            MMP_UINT8    wpGrpSize       ;     // Wr protect group size        D[38:32]
            MMP_UINT8    wpGrpEnable     ;     // Wr protect group enable      D[31:31]      
        }SD_ERASE;    
    
    }ERASE;    
    
    MMP_UINT8    defultECC       ;     // Manufacturer default ECC     D[30:29]
    MMP_UINT8    r2wFactor       ;     // Write speed factor           D[28:26]
    MMP_UINT8    wrBlkLen        ;     // Max write data BLK LEN       D[25:22]
    MMP_UINT8    wrBlkPartial    ;     // Partial BLKs for Wr allowed  D[21:21]
    MMP_UINT8    reserved3       ;     // reserved                     D[20:17]
    MMP_UINT8    contentProtApp  ;     // Content protect application  D[16:16]
    MMP_UINT8    fileFormatGrp   ;     // File format group            D[15:15]
    MMP_UINT8    copy            ;     // Copy flag (OTP)              D[14:14]
    MMP_UINT8    permWrProt      ;     // Permanent Wr protection      D[13:13]
    MMP_UINT8    tmpWrProt       ;     // Temporary Wr protection      D[12:12]
    MMP_UINT8    fileFormat      ;     // File format                  D[11:10]
    MMP_UINT8    ECC             ;     // ECC Code                     D[9:8]
    MMP_UINT8    CRC             ;     // CRC                          D[7:1]
    MMP_UINT8    X               ;     // Not used, always 1           D[0:0]
} MMC_CSD_DATA, *PMMC_CSD_DATA; 

typedef struct _MMC_CARD_STATUS
{
   MMP_UINT8    OutOfRange  ;    // CMD's argument out of range    D[31:31]
    MMP_UINT8    AddrError   ;    // Address misaligned block len   D[30:30]
    MMP_UINT8    BlkLenErr   ;    // Xferred block len not allowed  D[29:29]
    MMP_UINT8    EraseSeqErr ;    // Error occurred in seq of erase D[28:28]
    MMP_UINT8    EraseParam  ;    // Invalid erase group            D[27:27]
    MMP_UINT8    WpViolation ;    // Write to protected block       D[26:26]
    MMP_UINT8    CardLocked  ;    // Card is locked by host         D[25:25]
    MMP_UINT8    LockULFail  ;    // Seq or pwd err in lock/unlock  D[24:24]
    MMP_UINT8    CmdCRCErr   ;    // Pre-Cmd CRC Err                D[23:23]
    MMP_UINT8    IllegalCmd  ;    // Cmd illegal for card state     D[22:22]
    MMP_UINT8    CardEccFail ;    // Card ECC failed to correct dataD[21:21]
    MMP_UINT8    CCError     ;    // Internal card controller error D[20:20]
    MMP_UINT8    UnknowError ;    // General or unknown err         D[19:19]
    MMP_UINT8    Underrun    ;    // Card failed to sustain data    D[18:18]
                                //  xfer in stream read
    MMP_UINT8    Overrun     ;    // Card failed to sustain data    D[17:17]
                                //  programming in stream write
    MMP_UINT8    CIDCSDOverWr;    // Reference to spec              D[16:16]
    MMP_UINT8    WpEraseSkip ;    // Only partial space erased      D[15:15]
                                //  due to protected blocks
    MMP_UINT8    ECCDisabled ;    // No internal ECC                D[14:14]
    MMP_UINT8    EraseReset  ;    // Erase seq cleared before exec  D[13:13]
                                //  b/c out of erase seq cmd rec
    MMP_UINT8    CurrentState;    // State of card when receiving   D[12:9]
                                //  Cmd.
                                //  0 = idle
                                //  1 = ready
                                //  2 = ident
                                //  3 = stby
                                //  4 = tran
                                //  5 = data
                                //  6 = rcv
                                //  7 = prg
                                //  8 = dis
                                //  9-15 = reserved
    MMP_UINT8    ReadyForData;    // Buffer empty signalling        D[8:8]
                                //  on the bus
    MMP_UINT8    AppCMD      ;    // Card will expect ACMD          D[5:5]
} MMC_CARD_STATUS, *PMMC_CARD_STATUS;
 
#if 0 // backup
typedef struct _MMC_CSD
{
    MMP_UINT8    CSDstructure    : 2;    // CSD Structure                D[127:126]
    MMP_UINT8    specVer         : 4;    // System Spec Version          D[125:122]
    MMP_UINT8    reserved1       : 2;    // reserved                     D[121:120]
    MMP_UINT8    TAAC;                   // Data read access-time-1      D[119:112]
    MMP_UINT8    NSAC;                   // Data read access-time-2      D[111:104]
                                    // in CLK cycles (NSAC * 100)
    MMP_UINT8    transSpeed;             // Max data transfer rate       D[103:96]
    MMP_UINT16    CCC             :12;    // Card command classes         D[95:84]
    MMP_UINT8    readBlkLen      :4;     // Max read data block LEN      D[83:80]
    MMP_UINT8    readBlkPatial   :1;     // Patial BLKs for Rd allowed   D[79:79]
    MMP_UINT8    writeBlkMisAlign:1;     // write BLK misalignment       D[78:78]
    MMP_UINT8    readBlkMisAlign :1;     // read BLK misalignment        D[77:77]
    MMP_UINT8    DSRImp          :1;     // DSR implemented              D[76:76]
    MMP_UINT8    reserved2       :2;     // reserved                     D[75:74]
    MMP_UINT16    cSize           :12;    // Device size                  D[73:62]
    MMP_UINT8    VDDRdCurrMin    :3;     // Max Rd current @Vdd min      D[61:59]
    MMP_UINT8    VDDRdCurrMax    :3;     // Max Rd current @Vdd max      D[58:56]
    MMP_UINT8    VDDWrCurrMin    :3;     // Max Wr current @Vdd min      D[55:53]
    MMP_UINT8    VDDWrCurrMax    :3;     // Max Wr current @Vdd max      D[52:50]
    MMP_UINT8    cSizeMult       :3;     // device size multiplier       D[49:47]
    MMP_UINT8    eraseGrpSize    :5;     // Erase group size             D[46:42]
    MMP_UINT8    eraseGrpMult    :5;     // Erase group size multiplier  D[41:37]
    MMP_UINT8    wpGrpSize       :5;     // Wr protect group size        D[36:32]
    MMP_UINT8    wpGrpEnable     :1;     // Wr protect group enable      D[31:31]
    MMP_UINT8    defultECC       :2;     // Manufacturer default ECC     D[30:29]
    MMP_UINT8    r2wFactor       :3;     // Write speed factor           D[28:26]
    MMP_UINT8    wrBlkLen        :4;     // Max write data BLK LEN       D[25:22]
    MMP_UINT8    wrBlkPartial    :1;     // Partial BLKs for Wr allowed  D[21:21]
    MMP_UINT8    reserved3       :4;     // reserved                     D[20:17]
    MMP_UINT8    contentProtApp  :1;     // Content protect application  D[16:16]
    MMP_UINT8    fileFormatGrp   :1;     // File format group            D[15:15]
    MMP_UINT8    copy            :1;     // Copy flag (OTP)              D[14:14]
    MMP_UINT8    permWrProt      :1;     // Permanent Wr protection      D[13:13]
    MMP_UINT8    tmpWrProt       :1;     // Temporary Wr protection      D[12:12]
    MMP_UINT8    fileFormat      :2;     // File format                  D[11:10]
    MMP_UINT8    ECC             :2;     // ECC Code                     D[9:8]
    MMP_UINT8    CRC             :7;     // CRC                          D[7:1]
    MMP_UINT8    X               :1;     // Not used, always 1           D[0:0]
} MMC_CSD_DATA, *PMMC_CSD_DATA; 

typedef struct _MMC_CARD_STATUS
{
    MMP_UINT8    OutOfRange  : 1;    // CMD's argument out of range    D[31:31]
    MMP_UINT8    AddrError   : 1;    // Address misaligned block len   D[30:30]
    MMP_UINT8    BlkLenErr   : 1;    // Xferred block len not allowed  D[29:29]
    MMP_UINT8    EraseSeqErr : 1;    // Error occurred in seq of erase D[28:28]
    MMP_UINT8    EraseParam  : 1;    // Invalid erase group            D[27:27]
    MMP_UINT8    WpViolation : 1;    // Write to protected block       D[26:26]
    MMP_UINT8    CardLocked  : 1;    // Card is locked by host         D[25:25]
    MMP_UINT8    LockULFail  : 1;    // Seq or pwd err in lock/unlock  D[24:24]
    MMP_UINT8    CmdCRCErr   : 1;    // Pre-Cmd CRC Err                D[23:23]
    MMP_UINT8    IllegalCmd  : 1;    // Cmd illegal for card state     D[22:22]
    MMP_UINT8    CardEccFail : 1;    // Card ECC failed to correct dataD[21:21]
    MMP_UINT8    CCError     : 1;    // Internal card controller error D[20:20]
    MMP_UINT8    UnknowError : 1;    // General or unknown err         D[19:19]
    MMP_UINT8    Underrun    : 1;    // Card failed to sustain data    D[18:18]
                                //  xfer in stream read
    MMP_UINT8    Overrun     : 1;    // Card failed to sustain data    D[17:17]
                                //  programming in stream write
    MMP_UINT8    CIDCSDOverWr: 1;    // Reference to spec              D[16:16]
    MMP_UINT8    WpEraseSkip : 1;    // Only partial space erased      D[15:15]
                                //  due to protected blocks
    MMP_UINT8    ECCDisabled : 1;    // No internal ECC                D[14:14]
    MMP_UINT8    EraseReset  : 1;    // Erase seq cleared before exec  D[13:13]
                                //  b/c out of erase seq cmd rec
    MMP_UINT8    CurrentState: 4;    // State of card when receiving   D[12:9]
                                //  Cmd.
                                //  0 = idle
                                //  1 = ready
                                //  2 = ident
                                //  3 = stby
                                //  4 = tran
                                //  5 = data
                                //  6 = rcv
                                //  7 = prg
                                //  8 = dis
                                //  9-15 = reserved
    MMP_UINT8    ReadyForData: 1;    // Buffer empty signalling        D[8:8]
                                //  on the bus
    MMP_UINT8    AppCMD      : 1;    // Card will expect ACMD          D[5:5]

} MMC_CARD_STATUS, *PMMC_CARD_STATUS; 
#endif //#if 0

#endif //__MMC_SPEC_H
