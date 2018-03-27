#ifndef __STORAGE_MGR_H_NNZC4CRJ_L3BL_OZQE_1C1F_YCJJN22K9YOV__
#define __STORAGE_MGR_H_NNZC4CRJ_L3BL_OZQE_1C1F_YCJJN22K9YOV__

#include "mmp_types.h"
#if defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS)
#include "ntfs-3g/inc/sxa.h"
#endif /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================
//                              Global Data Definition
//=============================================================================
    /** Use storage_thread polling card or main thread polling card **/
#define ENABLE_STORAGE_THREAD_POLLING

    /** special Card Volume Definitions **/
//#if !defined(_WIN32)
//    #define PTP_VOLUME                   L"PTP:/"
//#else
//    #define DTV_FILE_TDEFAULT            L"/smedia/"
//    #define DATA_VOLUME                  DTV_FILE_TDEFAULT  L"smediadata/"
//    #define NAND_VOLUME                  DTV_FILE_TDEFAULT  L"NAND/"
//#endif

#define DTV_PATH                         PAL_T("/")
#define DTV_ALL_SEARCH                   PAL_T("*.*")

#define DTV_MAX_VERIFY_DEVICE_COUNT         10 //200
#define DTV_MAX_VERIFY_DEVICE_DURATION      10000 // ms (10 seconds)

#ifdef DTV_680_8M
        /** MAX Volume Number **/
    #define DTV_VOLUME_NUM                      4

    /** The Foder Of Icon Resource In Application **/
    //#define RESOURCE_TDEFAULT                PAL_T("A:/smediadata/")

    //#define DTV_MAX_CAMERA_TABLE_COUNT          100   // for EXIF info table
    #define DTV_MAX_DIR_COUNT                   100     // the max amount of directory in storage mgr
    #define DTV_MAX_FILE_COUNT                  1000    // the max amount of file in storage mgr
    #define DTV_SUB_FILE_TABLE_COUNT            100     // the unit of allocated file table when file table full
    #define DTV_SUB_DIR_COUNT                   10      // the unit of allocated directory table when directory table full
    #define DTV_MAX_DIR_LEVEL                   8
    #define DTV_RESERVE_VOLUME_NUM              0       // reserve 0 volumes for internal device
#else
        /** MAX Volume Number **/
    #define DTV_VOLUME_NUM                      8

        /** The Foder Of Icon Resource In Application **/
    //#define RESOURCE_TDEFAULT                PAL_T("A:/smediadata/")

    //#define DTV_MAX_CAMERA_TABLE_COUNT          100   // for EXIF info table
    #define DTV_MAX_DIR_COUNT                   1000    // the max amount of directory in storage mgr
    #define DTV_MAX_FILE_COUNT                  10000   // the max amount of file in storage mgr
    #define DTV_SUB_FILE_TABLE_COUNT            100     // the unit of allocated file table when file table full
    #define DTV_SUB_DIR_COUNT                   10      // the unit of allocated directory table when directory table full
    #define DTV_MAX_DIR_LEVEL                   8

    #if defined(DTV_RESOURCE_AT_INTERNAL)
        #define DTV_RESERVE_VOLUME_NUM              1       // reserve 1 volumes for resource at internal device
    #elif defined(DTV_INTERNAL_DEV_ENABLE)
        #define DTV_RESERVE_VOLUME_NUM              0       // reserve 2 volumes for internal device
    #else
        #define DTV_RESERVE_VOLUME_NUM              2       // reserve 2 volumes for internal device
    #endif
#endif

#define MAX_FILE_FULL_PATH_LEN      512 //256

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct _SF_POS
{
    union
    {
        struct
        {
            unsigned long cluster;      /* which cluster is used */
            unsigned long prevcluster;  /* previous cluster for bad block handling */
            unsigned long sectorbegin;  /* calculated sector start */
            unsigned long sector;       /* current sector */
            unsigned long sectorend;    /* last saector position of the cluster */
            unsigned long pos;          /* current position */
        } fat_f_pos;

#if defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS)
        struct sxa_ntfs_pos ntfs_f_pos;
#endif /* defined(__FREERTOS__) && defined(CONFIG_HAVE_NTFS) */
    };
} SF_POS;

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================
/** Error code **/
typedef enum STORAGE_ERR_TAG
{
    STORAGE_ERR_OK                  = 0,
    STORAGE_ERR_CREATE_THREAD_FAIL,
    STORAGE_ERR_DESTROY_THREAD_FAIL,
    STORAGE_ERR_INTERRUPT,
    STORAGE_ERR_OUT_MAX_FILE_COUNT,
} STORAGE_ERR;

/** Card Event Type **/
typedef enum DTV_CARD_EVENT_TAG
{
    DTV_CARD_NOCHANGE = 0,
    DTV_CARD_INSERT,
    DTV_CARD_REMOVE,
    DTV_CARD_FAIL_CARD,
    DTV_CARD_USB_UNSUPPORT
} DTV_CARD_EVENT;

/** Search File Type **/
typedef enum DTV_FILE_TYPE_TAG
{
    DTV_FILE_TYPE_JPG = 0,
    DTV_FILE_TYPE_AUDIO,
    DTV_FILE_TYPE_VIDEO,
    DTV_FILE_TYPE_RECORD,
    DTV_FILE_TYPE_FW,
    DTV_FILE_TYPE_VOICE,
    DTV_FILE_TYPE_COUNT,
    DTV_FILE_TYPE_DIR
} DTV_FILE_TYPE;

/* Search string definitions*/
typedef enum DTV_EXT_SEARCH_TYPE_TAG
{
    // photo
    DTV_EXT_SEARCH_JPG     = 0,
    DTV_EXT_SEARCH_JPEG,
    DTV_EXT_SEARCH_BMP,
//    DTV_EXT_SEARCH_GIF,
    DTV_EXT_SEARCH_PHOTO_COUNT,

    // audio
    DTV_EXT_SEARCH_MP3,
    //DTV_EXT_SEARCH_M4A,
    //DTV_EXT_SEARCH_AAC,
#if defined(MUSIC_PLAYER_SUPPORT_WAV)
    DTV_EXT_SEARCH_WAV,
#endif

#if defined(HAVE_WMA)
    DTV_EXT_SEARCH_WMA,
    //DTV_EXT_SEARCH_ASF,
#endif

    DTV_EXT_SEARCH_AUDIO_COUNT,

    // video
    DTV_EXT_SEARCH_3GP,
    DTV_EXT_SEARCH_MP4,
    DTV_EXT_SEARCH_AVI,
    DTV_EXT_SEARCH_MOV,
    DTV_EXT_SEARCH_MPG,
    DTV_EXT_SEARCH_MPEG,
    DTV_EXT_SEARCH_VOB,
    DTV_EXT_SEARCH_DAT,
    DTV_EXT_SEARCH_MKV,
    DTV_EXT_SEARCH_TS,
    DTV_EXT_SEARCH_TP,
    DTV_EXT_SEARCH_TRP,
#ifdef ENABLE_FLV_FILEFORMAT
    DTV_EXT_SEARCH_FLV,
#endif
    DTV_EXT_SEARCH_VIDEO_COUNT,

    // record
    DTV_EXT_SEARCH_REC,
    DTV_EXT_SEARCH_RECORD_COUNT,

    // firmware file
    DTV_EXT_SEARCH_IMG,
    DTV_EXT_SEARCH_FW_COUNT,

    // voice
    DTV_EXT_SEARCH_AMR,

    DTV_EXT_SEARCH_COUNT
} DTV_EXT_SEARCH_TYPE;

typedef enum DTV_USB_SPEED_TYPE_TAG
{
    USB_UNKNOWN = 0,
    USB_1_1,
    USB_2_0
} DTV_USB_SPEED_TYPE;

static MMP_WCHAR *FileExtArray[DTV_EXT_SEARCH_COUNT - 4] =
{
    // photo
    PAL_T("*.jpg"),     // DTV_EXT_SEARCH_JPG     = 0,
    PAL_T("*.jpeg"),    // DTV_EXT_SEARCH_JPEG,
    PAL_T("*.bmp"),     // DTV_EXT_SEARCH_BMP,
    //PAL_T("*.gif"),   // DTV_EXT_SEARCH_GIF,
    /** DTV_EXT_SEARCH_PHOTO_COUNT = don't care, **/

    // audio
    PAL_T("*.mp3"),     // DTV_EXT_SEARCH_MP3,
    //PAL_T("*.m4a"),   // DTV_EXT_SEARCH_M4A,
    //PAL_T("*.aac"),   // DTV_EXT_SEARCH_AAC,
#if defined(MUSIC_PLAYER_SUPPORT_WAV)
    PAL_T("*.wav"),   // DTV_EXT_SEARCH_WAV,
#endif

#if defined(HAVE_WMA)
    PAL_T("*.wma"),     // DTV_EXT_SEARCH_WMA,
    //PAL_T("*.asf"),  // DTV_EXT_SEARCH_ASF,
#endif
    /** DTV_EXT_SEARCH_AUDIO_COUNT = don't care, **/

    // video
    PAL_T("*.3gp"),     // DTV_EXT_SEARCH_3GP,
    PAL_T("*.mp4"),     // DTV_EXT_SEARCH_MP4,
    PAL_T("*.avi"),     // DTV_EXT_SEARCH_AVI,
    PAL_T("*.mov"),     // DTV_EXT_SEARCH_MOV,
    PAL_T("*.mpg"),     // DTV_EXT_SEARCH_MPG,
    PAL_T("*.mpeg"),    // DTV_EXT_SEARCH_MPEG
    PAL_T("*.vob"),     // DTV_EXT_SEARCH_VOB,
    PAL_T("*.dat"),     // DTV_EXT_SEARCH_DAT
    PAL_T("*.mkv"),     // DTV_EXT_SEARCH_MKV,
    PAL_T("*.ts"),      // DTV_EXT_SEARCH_TS
    PAL_T("*.tp"),      // DTV_EXT_SEARCH_TP,
    PAL_T("*.trp"),      // DTV_EXT_SEARCH_TRP,
#ifdef ENABLE_FLV_FILEFORMAT
    PAL_T("*.flv"),     // DTV_EXT_SEARCH_FLV,
#endif
    /** DTV_EXT_SEARCH_VIDEO_COUNT = don't care, **/

    // record
    PAL_T("*.rec"),     // DTV_EXT_SEARCH_REC,
    /** DTV_EXT_SEARCH_RECORD_COUNT = don't card, **/

    // firmware file
    PAL_T("*.img"),     // DTV_EXT_SEARCH_IMG,
    /** DTV_EXT_SEARCH_FW_COUNT = don't card, **/

    // voice
    PAL_T("*.amr"),     // DTV_EXT_SEARCH_AMR,
};

typedef enum FILE_SYSTEM_FORMAT_TAG
{
    FAT_FILE_SYSTEM = 0,
    NTFS_FILE_SYSTEM
} FILE_SYSTEM_FORMAT;

//=============================================================================
//                Public Function Definition
//=============================================================================
STORAGE_ERR
storageMgrInitialize(
    MMP_BOOL    bFirstInit);

STORAGE_ERR
storageMgrTerminate(
    void);

void
storageMgrStartSearch(
    MMP_BOOL           bRecursiveSearch,
    PAL_DEVICE_TYPE    cardType,
    MMP_INT            partitionIdx,
    MMP_WCHAR          *startPath);

void
storageMgrInterruptSearch(
    void);

void
storageMgrClearDataBase(
    MMP_UINT volume);

void
storageMgrGetCardEvent(
    DTV_CARD_EVENT   *cardEvent,
    MMP_INT          *cardId);

MMP_BOOL
storageMgrGetUSBEvent(
    MMP_INT  cardId);


void
storageMgrGetClientEvent(
    DTV_CARD_EVENT   *cardEvent,
    MMP_INT          *cardId);

MMP_INT
storageMgrFormatVolume(
    MMP_INT     volume);

MMP_INT
storageMgrDeleteDir(
    MMP_WCHAR*       searchpath);

///////////////////////////////////////////////////////////////
// Get ( Set ) Function Declaration
//////////////////////////////////////////////////////////////
MMP_BOOL
storageMgrCheckFileType(
    DTV_FILE_TYPE    extType,
    MMP_WCHAR*       filename);

MMP_BOOL
storageMgrIsVolumeReady(
    PAL_DEVICE_TYPE    cardType,
    MMP_INT            partitionIdx);

MMP_BOOL
storageMgrIsCardInsert(
    PAL_DEVICE_TYPE    cardType);

MMP_BOOL
storageMgrGetClientStatus(
    void);

//void
//storageMgrSetClientStatus(
//    MMP_BOOL bClientHandle);

PAL_DEVICE_TYPE
storageMgrGetVolumeCard(
    MMP_INT     volume,
    MMP_INT     *partitionIdx);

MMP_INT
storageMgrGetCardVolume(
    PAL_DEVICE_TYPE    cardType,
    MMP_INT            partitionIdx);

MMP_BOOL
storageMgrIsSearchRecursive(
    MMP_INT          volume);

MMP_INT
storageMgrGetVolumeCountInCard(
    PAL_DEVICE_TYPE    cardType);

MMP_INT
storageMgrGetVolumeInfo(
    MMP_INT          volume,
    DTV_FILE_TYPE    fileType,
    MMP_UINT*        dirCount,
    MMP_UINT*        fileCount,
    MMP_BOOL*        volumeUsed,
    MMP_BOOL*        databaseStatus);

MMP_INT
storageMgrGetFileInfo(
    MMP_INT          volume,
    DTV_FILE_TYPE    fileType,
    MMP_UINT         fileIndex,
    MMP_WCHAR        *fPathName,
    MMP_UINT32       *fSize,
    MMP_UINT32       *fDateTime,
    MMP_WCHAR        *fLastName,
    SF_POS           *currFilePos);

MMP_INT
storageMgrGetDirInfo(
    MMP_INT          volume,
    DTV_FILE_TYPE    fileType,
    MMP_UINT         currDirIndex,
    MMP_WCHAR        *dirPath,
    MMP_WCHAR        *dirName,
    SF_POS           *currDirPos);

void
storageMgrUsbSuspendMode(
    MMP_BOOL        bEnable);

DTV_USB_SPEED_TYPE
storageMgrGetUsbSpeedType(
    void);

FILE_SYSTEM_FORMAT
storageMgrGetVolumeFileSystemFormat(
    MMP_INT            volume);

MMP_INT
storageMgrGetVolumeNumber(
    MMP_WCHAR* pathname);

#ifdef __cplusplus
}
#endif

#endif
