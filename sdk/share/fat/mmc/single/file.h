/*
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * File functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef FILE_H
#define FILE_H

#include "mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constant definitions */
typedef void            SMTK_FILE;              /**< File handle */

#define SMTK_TEOF       (MMP_UINT16)(0xFFFF)    /**< End-of-file (Unicode) */

#define SMTK_FILE_TDEFAULT  L"/smedia/"
#define SMTK_FILE_RB        0                   /**< Opens for reading (binary) */
#define SMTK_FILE_WB        1                   /**< Opens for writing (binary) */

#ifdef SMTK_FILE_PAL

#define SMTK_NAND_VOLUME   L"A:/"
#define SMTK_SD_VOLUME     L"B:/"
#define SMTK_NOR1_VOLUME   L"C:/"
#define SMTK_CF_VOLUME     L"D:/"
#define SMTK_MS_VOLUME     L"E:/"
#define SMTK_xD_VOLUME     L"F:/"
#define SMTK_USB_0_VOLUME  L"I:/"
#define SMTK_USB_1_VOLUME  L"J:/"
#define SMTK_USB_2_VOLUME  L"K:/"
#define SMTK_USB_3_VOLUME  L"L:/"
#define SMTK_USB_4_VOLUME  L"M:/"
#define SMTK_USB_5_VOLUME  L"N:/"
#define SMTK_USB_6_VOLUME  L"O:/"
#define SMTK_USB_7_VOLUME  L"P:/"
#define SMTK_NAND1_VOLUME  L"G:/"
#define SMTK_NOR_VOLUME    L"H:/"

#ifdef  SMTK_DATA_IN_NOR
#define SMTK_SOURCE_DIR    L"H:/"
#define SMTK_VOICE_DIR     L"C:/VOICE/"
#define SMTK_VOICE_MKDIR   L"C:/VOICE"
#define SMTK_ENCODE_DIR    L"C:/FAVORITES/"
#define SMTK_ENCODE_MKDIR  L"C:/FAVORITES"
#define SMTK_BT_DIR        L"C:/BT/"
#define SMTK_BT_MKDIR      L"C:/BT"
#define SMTK_CONFIG_DIR    L"C:/CONFIG/"
#define SMTK_CONFIG_MKDIR  L"C:/CONFIG"
#define SMTK_DATA_PATH     L"H:/smediadata/"
#else
#define SMTK_SOURCE_DIR    L"A:/"
#define SMTK_VOICE_DIR     L"A:/VOICE/"
#define SMTK_VOICE_MKDIR   L"A:/VOICE"
#define SMTK_ENCODE_DIR    L"A:/FAVORITES/"
#define SMTK_ENCODE_MKDIR  L"A:/FAVORITES"
#define SMTK_BT_DIR        L"A:/BT/"
#define SMTK_BT_MKDIR      L"A:/BT"
#define SMTK_CONFIG_DIR    L"A:/CONFIG/"
#define SMTK_CONFIG_MKDIR  L"A:/CONFIG"
#define SMTK_DATA_PATH     L"G:/smediadata/"
#endif

#define SMTK_JPG_SEARCH   L"*.jpg"
#define SMTK_JPEG_SEARCH  L"*.jpeg"
#define SMTK_MP3_SEARCH   L"*.mp3"
#define SMTK_WMA_SEARCH   L"*.wma"
#define SMTK_3GP_SEARCH   L"*.3gp"
#define SMTK_MOV_SEARCH   L"*.mov"
#define SMTK_AVI_SEARCH   L"*.avi"
#define SMTK_MP4_SEARCH   L"*.mp4"
#define SMTK_MPG_SEARCH   L"*.mpg"
#define SMTK_AMR_SEARCH   L"*.amr"
#define SMTK_PATH         L"/"
#define SMTK_DIR          L"*.*"
#endif

#define SMTK_CARD_NUM        16

#define CARD_NORMAL          0x00
#define CARD_INSERT          0x01
#define CARD_REMOVE_CURRENT  0x02
#define CARD_REMOVE_MISC     0x03
#define CARD_CLIENT_MODE_IN  0x04
#define CARD_CLIENT_MODE_OUT 0x05

#define FILE_PATH_MAX_LEVEL 8

typedef enum SMTK_FILE_TYPE_TAG
{
    SMTK_FILE_JPG,
    SMTK_FILE_MP3,
    SMTK_FILE_MPEG,
    SMTK_FILE_AMR,

    SMTK_FILE_TYPE_COUNT
} SMTK_FILE_TYPE;

typedef struct SMTK_SEARCH_TAG
{
    MMP_WCHAR*  jpgsearchstring;
    MMP_WCHAR*  jpegsearchstring;
    MMP_WCHAR*  mp3searchstring;
    MMP_WCHAR*  the3gpsearchstring;
    MMP_WCHAR*  movsearchstring;
    MMP_WCHAR*  avisearchstring;
    MMP_WCHAR*  mp4searchstring;
    MMP_WCHAR*  mpgsearchstring;
    MMP_WCHAR*  amrsearchstring;
} SMTK_SEARCH;

typedef struct SMTK_FILE_TABLE_TAG
{
    MMP_WCHAR*  filename;
    MMP_ULONG   size;
    MMP_WCHAR*  lastname;
} SMTK_FILE_TABLE;



#ifdef SMTK_FILE_BROWSE_ENABLE

//file table temp size
#define JPG_TEMP_SIZE  10
#define MP3_TEMP_SIZE  5
#define MPEG_TEMP_SIZE 5
#define BGM_TEMP_SIZE  1

//file path max size
#define PATH_TABLE_MAX_SIZE 199

//file path table tag
typedef struct SMTK_FILE_PATH_TABLE_TAG
{
    MMP_WCHAR*  pathname;
    MMP_UINT    filecount[SMTK_FILE_TYPE_COUNT];
    MMP_WCHAR*  lastname;
    MMP_UINT    rootpathindex;
    MMP_UINT    pathlevel;
} SMTK_FILE_PATH_TABLE;


//jpg temp file table tag
typedef struct SMTK_JPG_FILE_TABLE_TAG
{
  SMTK_FILE_TABLE  temptable[3][JPG_TEMP_SIZE];
  //SMTK_FILE_TABLE prevtable[JPG_TEMP_SIZE];
  //SMTK_FILE_TABLE currtable[JPG_TEMP_SIZE];
  //SMTK_FILE_TABLE nexttable[JPG_TEMP_SIZE];
  MMP_UINT        prevtableid;
  MMP_UINT        currtableid;
  MMP_UINT        nexttableid;
  MMP_INT         currentid; //for current table
  MMP_INT         currentfileindex;
  MMP_UINT        changetable;
  //MMP_UINT        prevfileindex;
  //MMP_UINT        nextfileindex;
} SMTK_JPG_FILE_TABLE;

//mp3 temp file table tag
typedef struct SMTK_MP3_FILE_TABLE_TAG
{
  SMTK_FILE_TABLE  temptable[3][MP3_TEMP_SIZE];
  //SMTK_FILE_TABLE prevtable[MP3_TEMP_SIZE];
  //SMTK_FILE_TABLE currtable[MP3_TEMP_SIZE];
  //SMTK_FILE_TABLE nexttable[MP3_TEMP_SIZE];
  MMP_UINT        prevtableid;
  MMP_UINT        currtableid;
  MMP_UINT        nexttableid;
  MMP_INT         currentid; //for current table
  MMP_INT         currentfileindex;
  MMP_UINT        changetable;
  //MMP_UINT        prevfileindex;
  //MMP_UINT        nextfileindex;
} SMTK_MP3_FILE_TABLE;

//mpeg temp file table tag
typedef struct SMTK_MPEG_FILE_TABLE_TAG
{
  SMTK_FILE_TABLE  temptable[3][MPEG_TEMP_SIZE];
  //SMTK_FILE_TABLE prevtable[MPEG_TEMP_SIZE];
  //SMTK_FILE_TABLE currtable[MPEG_TEMP_SIZE];
  //SMTK_FILE_TABLE nexttable[MPEG_TEMP_SIZE];
  MMP_UINT        prevtableid;
  MMP_UINT        currtableid;
  MMP_UINT        nexttableid;
  MMP_INT         currentid; //for current table
  MMP_INT         currentfileindex;
  MMP_UINT        changetable;
  //MMP_UINT        prevfileindex;
  //MMP_UINT        nextfileindex;
} SMTK_MPEG_FILE_TABLE;

//background music temp file table tag
typedef struct SMTK_BGM_FILE_TABLE_TAG
{
  SMTK_FILE_TABLE  temptable[3][BGM_TEMP_SIZE];
  //SMTK_FILE_TABLE prevtable[BGM_TEMP_SIZE];
  //SMTK_FILE_TABLE currtable[BGM_TEMP_SIZE];
  //SMTK_FILE_TABLE nexttable[BGM_TEMP_SIZE];
  MMP_UINT        prevtableid;
  MMP_UINT        currtableid;
  MMP_UINT        nexttableid;
  MMP_INT         currentid; //for current table
  MMP_INT         currentfileindex;
  MMP_UINT        changetable;
  //MMP_UINT        prevfileindex;
  //MMP_UINT        nextfileindex;
} SMTK_BGM_FILE_TABLE;


//file mgr tag
typedef struct SMTK_FILE_MGR_TAG
{
#ifdef SMTK_FILE_PAL
    SMTK_FILE_PATH_TABLE*   pathTable;
    MMP_UINT                pathCount;
    MMP_UINT                fileCount[SMTK_FILE_TYPE_COUNT];
    SMTK_JPG_FILE_TABLE     jpgFileTable;
    SMTK_MP3_FILE_TABLE     mp3FileTable;
    SMTK_MPEG_FILE_TABLE    mpegFileTable;
    SMTK_BGM_FILE_TABLE     bgmFileTable;
    MMP_UINT                jpgCurrPathIndex;
    MMP_UINT                mp3CurrPathIndex;
    MMP_UINT                mpegCurrPathIndex;
    MMP_UINT                bgmCurrPathIndex;
    MMP_UINT                jpgTotalFileCount;
    MMP_UINT                mp3TotalFileCount;
    MMP_UINT                mpegTotalFileCount;
    MMP_UINT                bgmTotalFileCount;
    MMP_INT                 initStatus;
    MMP_INT                 jpgChangeStatus;
    MMP_INT                 mp3ChangeStatus;
    MMP_INT                 mpegChangeStatus;
    MMP_INT                 bgmChangeStatus;
    MMP_UINT                jpgCountStatus;   // 1 for non-update, 0 for general path
    MMP_UINT                mp3CountStatus;   // 1 for non-update, 0 for general path
    MMP_UINT                mpegCountStatus;  // 1 for non-update, 0 for general path
    MMP_UINT                bgmCountStatus;   // 1 for non-update, 0 for general path


    MMP_WCHAR           crrentVolume[sizeof(SMTK_NAND_VOLUME)+1];
    MMP_UINT16 cardType[SMTK_CARD_NUM];
    MMP_UINT16 writeable[SMTK_CARD_NUM];
#endif
    MMP_BOOL  cards[SMTK_CARD_NUM];
    MMP_INT   currentCard;
    MMP_INT   currentCardIndex;
    MMP_INT   focus;
    MMP_INT   sectorsPerCluster;
    MMP_UINT32          fileWriteIndex[SMTK_FILE_TYPE_COUNT];

      /**
     * Data for file decoder manager.
     */
    MMP_EVENT           eventMgrToThread;
    MMP_EVENT           eventThreadToMgr;

} SMTK_FILE_MGR;

#else //END OF SMTK_FILE_BROWSE_ENABLE

#define JPEG_MAX_FILE_NUM 999
#define MP3_MAX_FILE_NUM  30
#define MPEG_MAX_FILE_NUM 30
#define AMR_MAX_FILE_NUM  1

typedef struct SMTK_FILE_MGR_TAG
{
#ifdef SMTK_FILE_PAL
    SMTK_FILE_TABLE*    fileTables[SMTK_FILE_TYPE_COUNT];
    MMP_UINT            fileCount[SMTK_FILE_TYPE_COUNT];
    //MMP_UINT            fileTableId[SMTK_FILE_TYPE_COUNT];
    MMP_INT             initStatus;
    MMP_WCHAR           crrentVolume[sizeof(SMTK_NAND_VOLUME)+1];
    MMP_UINT16 cardType[SMTK_CARD_NUM];
    MMP_UINT16 writeable[SMTK_CARD_NUM];
#endif
    MMP_BOOL  cards[SMTK_CARD_NUM];
    MMP_INT   currentCard;
    MMP_INT   currentCardIndex;
    MMP_INT   focus;
    MMP_INT   sectorsPerCluster;
    MMP_UINT32          fileWriteIndex[SMTK_FILE_TYPE_COUNT];

      /**
     * Data for file decoder manager.
     */
    MMP_EVENT           eventMgrToThread;
    MMP_EVENT           eventThreadToMgr;

} SMTK_FILE_MGR;

#endif // END OF NOT SMTK_FILE_BROWSE_ENABLE


extern SMTK_FILE_MGR smtkFileMgr;


#ifdef SMTK_FILE_PAL
void
smtkFileMgrSetCurrentCard(
                 MMP_UINT16 card);

void smtkFileMgrSetCardInfo(
                 MMP_UINT16 card,
                 MMP_UINT16 cardtype,
                 MMP_UINT16 writeable);

void smtkFileMgrGetCardInfo(
                 MMP_UINT16 card,
                 MMP_UINT16* cardtype,
                 MMP_UINT16* writeable);

MMP_INT
smtkFileMgrInitVolume(
    MMP_UINT drvnumber,
    MMP_UINT cardtype);
    
MMP_INT32
smtkFileMgrPollingCards(
    void* cards);
                                  
#else
MMP_INT
smtkFileMgrGetCurrentCard(
     void);

MMP_INT
smtkFileMgrGetClusterSize(
     void);
#endif

MMP_INT
smtkFileMgrGetCardEvent(
     void);

MMP_INT
smtkFileMgrInitialize(
    void);

MMP_INT
smtkFileMgrTerminate(
    void);

void smtkFileMgrGetFileWriteIndex(
                 MMP_UINT16 filetype,
                 MMP_UINT32* index);

void smtkFileMgrSaveFileWriteIndex(
                 MMP_UINT16 filetype,
                 MMP_UINT32 index);


#ifdef SMTK_FILE_PAL

#ifdef SMTK_FILE_BROWSE_ENABLE

MMP_UINT
smtkFileMgrGetFileCount(
    MMP_UINT16 filetype);

MMP_UINT
smtkFileMgrGetFileName(
    MMP_UINT16 filetype,
    MMP_UINT16 findex,
    MMP_WCHAR* fname);

MMP_UINT
smtkFileMgrGetFileSize(
    MMP_UINT16 filetype,
    MMP_UINT16 findex,
    MMP_ULONG* fsize);

MMP_UINT
smtkFileMgrGetFileLastName(
    MMP_UINT16 filetype,
    MMP_UINT16 findex,
    MMP_WCHAR* flastname);

MMP_UINT
smtkFileMgrGotoPrevFile(
    MMP_UINT16 filetype);

MMP_UINT
smtkFileMgrGotoNextFile(
    MMP_UINT16 filetype);

MMP_UINT
smtkFileMgrJumpToFile(
    MMP_UINT16 filetype,
    MMP_UINT16 findex);

MMP_UINT
smtkFileMgrSetCurPath(
    MMP_UINT16 filetype,
    MMP_UINT16 pathindex);

MMP_UINT
smtkFileMgrGetCurPath(
    MMP_UINT16 filetype);

MMP_UINT
smtkFileMgrGetPrevLevelPath(
    MMP_UINT16 pathindex,
    MMP_UINT* pathtable);

MMP_UINT
smtkFileMgrGetNextLevelPath(
    MMP_UINT16 pathindex,
    MMP_UINT* pathtable);

MMP_UINT
smtkFileMgrGetFileCountInPath(
    MMP_UINT16 filetype,
    MMP_UINT16 pathindex);

MMP_UINT
smtkFileMgrCreateFilePathTable(
     void);

MMP_UINT
smtkFileMgrDestroyFilePathTable(
     void);

MMP_INT
smtkFileMgrCheckChangeStatus(
    void);

MMP_INT
smtkFileMgrCheckCardEvent(
     void);

#else

MMP_UINT
smtkFileMgrCreateFileTable(
     void);

MMP_UINT
smtkFileMgrGetFileTable(
    MMP_UINT16          filetype,
    SMTK_FILE_TABLE**    fileTable);

void
smtkFileMgrDestroyFileTable(
void);

#endif

MMP_INT
smtkFileMgrCheckInitializeStatus(
    void);

#else
MMP_UINT
smtkFileMgrCreateFileTable(
    MMP_UINT cardtype,
    MMP_UINT filetype,
    MMP_UINT limit,
    SMTK_FILE_TABLE** filetable,
    MMP_UINT* actualcount,
    MMP_UINT* tableid
);
#endif


#if !defined(SMTK_FILE_PAL)
MMP_UINT
smtkFileMgrDestroyFileTable(
    SMTK_FILE_TABLE* filetable,
    MMP_UINT         count);

SMTK_FILE*
smtkFileMgrOpen(
    const MMP_WCHAR* filename,
    MMP_UINT mode);

MMP_INT
smtkFileMgrClose(
    SMTK_FILE* stream);

MMP_UINT
smtkFileMgrRead(
    void* buffer,
    MMP_UINT size,
    MMP_UINT count,
    SMTK_FILE* stream);

MMP_UINT
smtkFileMgrWrite(
    const void* ptr,
    MMP_UINT size,
    MMP_UINT count,
    SMTK_FILE* stream);

MMP_UINT
smtkFileMgrSeek(
    SMTK_FILE* stream,
    MMP_UINT   offset);

MMP_UINT32
smtkFileMgrGetFileSize(
    SMTK_FILE* stream);


#endif

void nordrv_flush(void);

void private_nordrv_flush(void);


#ifdef __cplusplus
}
#endif

#endif /* FILE_H */
