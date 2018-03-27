/*******************************************************************************
*
*  h e a d e r s
*
*******************************************************************************/
#include <stdio.h>
#include "mmp.h"
#include "mmp_types.h"
#include "pal/file.h"
#include "pal/timer.h"
#include "mmp_usbex.h"
#include "mmp_watchdog.h"
#include "core_interface.h"
#include "pal/thread.h"
#include "usbd/inc/it_usbd.h"
#include "usbd/inc/it_usbd_property.h"
#include "FreeRTOS.h"
#include "task.h"
#include "msg_route.h"
#include "config.h"

class it_usbd_pcgrabber : public it_usbd_pcgrabber_base
{
private:
    void fill_buffer();
    int m_is_upgrading;
    unsigned char* m_firmware_image;
    unsigned int m_firmware_length;
    unsigned int m_audio_source;
    unsigned int m_video_source;

public:
    it_usbd_pcgrabber()
        : m_is_upgrading(0)
        , m_firmware_image(0)
        , m_firmware_length(0)
        , m_audio_source(0)
        , m_video_source(::CAPTURE_VIDEO_SOURCE_HDMI)
    {
        printf("+++%s()\n", __FUNCTION__);
        printf("---%s()\n", __FUNCTION__);
    }

    int  main();
    void upgrader_task();
    void start_encoder(int is_true);

    // overriding
    void start();
    void stop();
    void advance_buffer(int len);
    char *get_buffer(int& len);
    void set_source(int audio_source, int video_source);
    int is_upgrading();
    void upgrade(void *image, unsigned int length);
    void set_pc_mode(int enable)
    {
        printf("+++%s(%d->%d)\n", __FUNCTION__, it_usbd_property::m_pc_mode, enable);
        SendAsyncMsg(enable ? MSG_DEVICE_MODE : MSG_HOST_MODE, 0, 0, 0);
        printf("---%s(%d->%d)\n", __FUNCTION__, it_usbd_property::m_pc_mode, enable);
    }

    unsigned short get_usb_vid()
    {
        return 0x048D;      // ITE
        //return 0x2040;  // HPG
        //return 0x1B80;  // ROXIO
    }

    unsigned short get_usb_pid()
    {
        return 0x0281;  //FY 
        //return 0xF800;
        //return 0xE012;
    }

    const char *get_usb_manufacturer_string()
    {
        return "www.ezcap.com";  //FY
        //return "Hauppauge Computer Works, Inc.";
        //return "Roxio Inc.";
    }

    const char *get_usb_product_string()
    {
        return "ezcap";
        //return "HD-PVR Rocket";
        //return "GameCAP HD PRO";
    }

    const char *get_usb_serial_number_string()
    {
        static char serial_number_string[256];

        serial_number_string[0]   = 0;
        coreGetUsbSerialNumber((char *) serial_number_string);
        serial_number_string[253] = 0;
        return serial_number_string;

        return "ITE"                        "."
               //return "HPG"                        "."
               //return "ROXIO"                      "."
               IT_USBD_STRINGIZE(CUSTOMER_CODE)     "."
               IT_USBD_STRINGIZE(PROJECT_CODE)      "."
               IT_USBD_STRINGIZE(SDK_MAJOR_VERSION) "."
               IT_USBD_STRINGIZE(SDK_MINOR_VERSION) "."
               IT_USBD_STRINGIZE(BUILD_NUMBER);
    }
};

extern int error_select2;
extern int error_select3;
#define IT_SELECT2(i, n0, n1)     (((i) == 0) ? (n0) : (((i) == 1) ? (n1) : error_select2))
#define IT_SELECT3(i, n0, n1, n2) (((i) == 0) ? (n0) : (((i) == 1) ? (n1) : (((i) == 2) ? (n2) : error_select3)))
unsigned long ts_num_packets_padding_to = 0;
#define IT_CONFIG_VIDEO_WIDTH   IT_SELECT2(1, 1280,  1920)
#define IT_CONFIG_VIDEO_HEIGHT  IT_SELECT2(1,  720,  1080)
#define IT_CONFIG_VIDEO_BITRATE IT_SELECT3(2, 13600, 16000, 18000)
#define IT_CONFIG_AUDIO_CODEC   IT_SELECT2(1, MPEG_AUDIO_ENCODER, AAC_AUDIO_ENCODER)
#define IT_CONFIG_ENABLESTOP    1

//static class it_usbd_pcgrabber _it_usbd_pcgrabber;
class it_usbd_pcgrabber      *it_usbd_pcgrabber      = NULL;
class it_usbd_pcgrabber_base *it_usbd_pcgrabber_base = NULL;

/*******************************************************************************
*
*  f u n c t i o n   p r o t o t y p e s
*
*******************************************************************************/
extern "C" void malloc_stats();

/*******************************************************************************
*
*  g l o b a l   v a r i a b l e s
*
*******************************************************************************/
unsigned long long        ts_num_null_packets       = 0;
unsigned long long        ts_num_null_packets_last  = 0;
static unsigned long long ts_bytes_read             = 0;
static unsigned int       ts_bytes_read0            = 0;
unsigned int              ts_t0_get_buf             = 0;
unsigned int              ts_t0_read0               = 0;
unsigned int              it_usb_read_bytes         = 0;
unsigned int              it_usb_read_time          = 0;

static int                it_usbd_ts_reader_ts_stop = 1;

//#define                         ITCONFIG_INITONLY
//#define                         ITCONFIG_USB_XFERPATTERN
//#define                         ITCONFIG_SAVE_INMAIN
//#define                         ITCONFIG_SAVE_TS
//#define                         ITCONFIG_SAVE_PATTERN
//#define                         ITCONFIG_USB_STEMXFER
#define                         ITCONFIG_USB_MAXSIZE   (256 * 1024)
#define                         ITCONFIG_XFER_HASHSIZE (0x100000 * 16)
static char               *buffer     = 0;
static unsigned int       buffer_size = 0;
static unsigned int       buffer_rptr = 0;
static unsigned long long synced      = 0;

#if defined(ITCONFIG_USB_XFERPATTERN) || defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
//#define                         TSBUFFER_SIZE   (188 * 1000)
    #define                         TSBUFFER_SIZE  (188 * 1394)
    #define                         TSBUFFER_COUNT (8)
//static char                     _buffer[TSBUFFER_SIZE * TSBUFFER_COUNT];
static char         *_buffer  = NULL;
static unsigned int file_wptr = 0;
#endif

#if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
    #define                         TIMES_DATA 1
static int      times_data     = TIMES_DATA;
static MMP_UINT *dummyfilename = (MMP_UINT *) L"C:/DUMMY.DAT";
static PAL_FILE *dummyfile     = NULL;
#endif

#if defined(ITCONFIG_SAVE_TS)
static MMP_UINT *tsfilename = (MMP_UINT *) L"C:/XX_TS_CLONE.ts";
static PAL_FILE *tsfile     = NULL;
#endif

#if defined(ITCONFIG_SAVE_PATTERN)
static MMP_UINT *patfilename = (MMP_UINT *) L"C:/XX_PAT_CLONE.ts";
static PAL_FILE *patfile     = NULL;
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//int
//it_ts_file_get_source()
//{
//    return ts_source;
//}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// TEST
//extern "C" void
//vTaskGetRunTimeStats(void*);

//extern "C" void
//vTaskGetRunCodeStackStats(void*);
// TEST

portTASK_FUNCTION(upgrader_task, pvParameters)
{
    ( void ) pvParameters;

    it_usbd_pcgrabber->upgrader_task();
}

class it_usbd_pcgrabber_base *
    it_usbd_pcgrabber_base::init_instance()
{
    static class it_usbd_pcgrabber it_usbd_pcgrabber;
    ::it_usbd_pcgrabber      = &it_usbd_pcgrabber;
    ::it_usbd_pcgrabber_base = &it_usbd_pcgrabber;
    return &it_usbd_pcgrabber;
}

void
it_usbd_pcgrabber::upgrader_task()
{
    it_auto_log auto_log(__FUNCTION__, NULL);

    printf("%s(): upgrading...\n", __FUNCTION__);
    m_is_upgrading    = 1;
    coreFirmwareUpgrade((MMP_UINT8 *) m_firmware_image, m_firmware_length);
    m_firmware_image  = 0;
    m_firmware_length = 0;
    m_is_upgrading    = 0;
    for (int i = 6; i > 0; i--)
    {
        printf("\r%s(): wait for rebooting... %d ", __FUNCTION__, i);
        fflush(stdout);
        PalSleep(1000);
    }
    printf("\n\n");
    mmpWatchDogEnable(1);
    printf("%s(): force rebooting...\n", __FUNCTION__);
    while (1);
}

extern MMP_BOOL gbDeviceMode;

#ifdef __cplusplus
extern "C" {
#endif

extern void
_DeviceModeProcess(
    void);

extern void
_HostModeProcess(
    void);
#ifdef __cplusplus
}
#endif

static void
_ProcessMsg(
    MSG_OBJECT *ptMsg)
{
    switch (ptMsg->name)
    {
    case MSG_CONFIG_STORE:
        :: config_store();
        break;

    case MSG_SERIAL_NUMBER_STORE:
        :: serial_number_store( );
        break;

    case MSG_DEVICE_MODE:
        :: _DeviceModeProcess();
        break;

    case MSG_HOST_MODE:
        :: _HostModeProcess();
        break;

    case MSG_START_ENCODER:
        it_usbd_pcgrabber->start_encoder(1);
        break;

    case MSG_STOP_ENCODER:
        it_usbd_pcgrabber->start_encoder(0);
        break;
    }
}

int
it_usbd_pcgrabber::main()
{
    it_auto_log  auto_log(__FUNCTION__, NULL);
#if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
    unsigned int rptr;
    int          nwrite;
    int          wlen;
#endif
    MMP_UINT8    *ts_buffer;
    MMP_UINT32   ts_buffer_size;
    MMP_UINT32   ts_wptr;
    MMP_UINT32   ts_wptr0 = (unsigned int) -1;
    unsigned int t0_ts_file;
    unsigned int t0_ts_buf;
    unsigned int t0_debug_ptr;
    unsigned int t0_debug_msg;
    unsigned int t_now;
    PROCESS_MSG  oldMsgProcessRoutine;

#if defined(IT_CONFIG_ENABLESTOP)
    coreApiStop();
#endif

    // TEST
#if defined(ITCONFIG_USB_XFERPATTERN) || defined(ITCONFIG_SAVE_PATTERN)
    #define CHUNKSIZE (32 * 1024)

    coreGetTsMuxBufferInfo((MMP_UINT8 **) &buffer, (MMP_UINT32 *) &buffer_size);
    _buffer = (char *) malloc(buffer_size);
    if (!_buffer)
    {
        printf("[X] unable to malloc(%d)\nSystem halted!\n", buffer_size);
        while (1);
    }

    int i;
    for (i = 0; i < buffer_size / CHUNKSIZE; i++)
    {
        memset(_buffer + i * CHUNKSIZE, i, CHUNKSIZE);
    }
    memset(_buffer + i * CHUNKSIZE, 0xff, buffer_size - i * CHUNKSIZE);
#endif
    // TEST

#if defined(ITCONFIG_INITONLY)
    return 0;
#endif

    t0_ts_file                  = t0_ts_buf = t0_debug_ptr = t0_debug_msg = PalGetClock() - 30 * 1000;

    coreInitVideoEnPara();
    it_usbd_property::m_pc_mode = !0;

    oldMsgProcessRoutine        = ::GetDelayedMessagesProcessRoutine();
    ::SetDelayedMessagesProcessRoutine(::_ProcessMsg);

    while (gbDeviceMode && it_usbd_property::m_pc_mode)
    {
        ::SendDelayedMessages();
        mmpWatchDogRefreshTimer();

        if (PalGetDuration(t0_ts_file) > 8)
        {
            t0_ts_file = PalGetClock();
        }

        if (PalGetDuration(t0_ts_buf) > 16)
        {
            t0_ts_buf = PalGetClock();
            coreGetTsMuxBufferInfo(&ts_buffer, &ts_buffer_size);
            ts_wptr   = coreGetTsMuxBufferWriteIndex();
        }
#if (1)
        if ((PalGetDuration(t0_debug_ptr) > 60 * 1000) ||
            0 /*|| (ts_wptr != ts_wptr0)*/)
        {
            t0_debug_ptr = PalGetClock();
            t_now        = t0_debug_ptr / 1000;
            printf("tsinfo@%02d:%02d:%02d:%02d PTR:%08X SZ:%08X(%d) W:%08X(%d) R:%08X(%d)\n",
                   t_now / (24 * 60 * 60),
                   t_now / (60 * 60) % 24,
                   t_now / 60 % 60,
                   t_now % 60,
                   ts_buffer, ts_buffer_size, ts_buffer_size, ts_wptr, ts_wptr, buffer_rptr, buffer_rptr);
            ts_wptr0 = ts_wptr;
        }

        if (PalGetDuration(t0_debug_msg) > 60 * 1000)
        {
            t0_debug_msg = PalGetClock();
    #if (0)
            vTaskGetRunTimeStats((char *)0);
            PalSleep(3);
            vTaskGetRunCodeStackStats((char *)0);
            PalSleep(3);
            malloc_stats();
            PalSleep(3);
    #endif
        }
#endif
#if defined(ITCONFIG_SAVE_INMAIN)
    #if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
        rptr = buffer_rptr;
        if (file_wptr <= rptr)
        {
            wlen = rptr - file_wptr;
        }
        else
        {
            wlen = buffer_size - file_wptr;
        }

        if (wlen)
        {
            if (wlen > ITCONFIG_USB_MAXSIZE)
            {
                wlen = ITCONFIG_USB_MAXSIZE;
            }

        #if defined(ITCONFIG_SAVE_TS)
            if (tsfile)
            {
                nwrite = PalFileWrite(buffer + file_wptr, 1, wlen, tsfile, NULL, NULL);
                printf("nwrite(tsfile)=%d\n", nwrite);
            }
        #endif

        #if defined(ITCONFIG_SAVE_PATTERN)
            if (patfile)
            {
                nwrite = PalFileWrite(_buffer + file_wptr, 1, wlen, patfile, NULL, NULL);
                printf("nwrite(patfile)=%d\n", nwrite);
            }
        #endif

            file_wptr += wlen;

            if (file_wptr > buffer_size)
            {
                printf("[X] file_wptr=%d\n", file_wptr);
            }

            if (file_wptr == buffer_size)
            {
                file_wptr = 0;
            }
        }
    #endif
#endif

        PalSleep(1);
    }
    ::SetDelayedMessagesProcessRoutine(oldMsgProcessRoutine);      // resto re message process routine
    it_usbd_ts_reader_ts_stop   = 1;
    it_usbd_property::m_pc_mode = 0;

    ts_num_packets_padding_to   = 0;
    return 0;
}

void
it_usbd_pcgrabber::start_encoder(
    int is_true)
{
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    unsigned long              t = PalGetClock();
    unsigned long              d;
    VIDEO_ENCODER_PARAMETER    vencoder_param;
    CORE_AUDIO_ENCODE_PARA     aencoder_param;
    VIDEO_ENCODER_UPDATE_FLAGS encoder_update_flags;

    printf("+++%s(%s)@%d, m_pc_mode=%d\n", __FUNCTION__, is_true ? "TRUE" : "FALSE", PalGetClock(), it_usbd_property::m_pc_mode);

    #if defined(IT_CONFIG_ENABLESTOP)
    if (is_true)
    {
        if (it_usbd_ts_reader_ts_stop)
        {
            TS_MUXER_PARAMETER muxer_param;

            memset(&muxer_param, 0, sizeof(muxer_param));
            coreTsRemoveServices();
            coreTsUpdateCountryId(CORE_COUNTRY_TAIWAN);
            coreTsUpdateModulationParameter(
                887000,
                6000,
                CONSTELATTION_64QAM,
                CODE_RATE_7_8,
                GUARD_INTERVAL_1_32);
            muxer_param.audioEncoderType = (AUDIO_ENCODER_TYPE) it_usbd_property::m_audio_codec;
            coreTsInsertService(
                it_usbd_property::m_ts_program_number,
                it_usbd_property::m_ts_pmt_pid,
                it_usbd_property::m_ts_video_pid,
                H264_VIDEO_STREAM,
                it_usbd_property::m_ts_audio_pid,
                (muxer_param.audioEncoderType == MPEG_AUDIO_ENCODER) ? MPEG_AUDIO : AAC,
                (MMP_UINT8 *) it_usbd_property::m_ts_provider_name,
                strlen(it_usbd_property::m_ts_provider_name),
                (MMP_UINT8 *) it_usbd_property::m_ts_service_name,
                strlen(it_usbd_property::m_ts_service_name));
            coreTsUpdateTable();

            muxer_param.bEnableSecuirtyMode = MMP_FALSE;
            muxer_param.bAddStuffData       = MMP_TRUE;
            coreSetMuxerParameter(&muxer_param);

            coreGetTsMuxBufferInfo((MMP_UINT8 **) &buffer, (MMP_UINT32 *) &buffer_size);

            memset(&aencoder_param, 0, sizeof(aencoder_param));
            aencoder_param.audioEncoderType = muxer_param.audioEncoderType;
            aencoder_param.bitRate          = it_usbd_property::m_audio_bitrate;
            //coreSetAudioEncodeParameter(&aencoder_param);

            encoder_update_flags            = (VIDEO_ENCODER_UPDATE_FLAGS)(
                0 |
                VIDEO_ENCODER_FLAGS_UPDATE_WIDTH_HEIGHT |
                VIDEO_ENCODER_FLAGS_UPDATE_BITRATE |
                (it_usbd_property::m_video_framerate ? VIDEO_ENCODER_FLAGS_UPDATE_FRAME_RATE : 0) |
                ((it_usbd_property::m_video_gop_size != -1) ? VIDEO_ENCODER_FLAGS_UPDATE_GOP_SIZE : 0) |
                0);

            //coreSetAspectRatio((INPUT_ASPECT_RATIO) it_usbd_property::m_video_aspect_ratio);
            //coreSetMicInVolStep(it_usbd_property::m_mic_volume);
            //coreSetLineInBoost((LINE_BOOST) it_usbd_property::m_linein_boost);

            vencoder_param.EnWidth     = it_usbd_property::m_video_dimension_width;
            vencoder_param.EnHeight    = it_usbd_property::m_video_dimension_height;
            vencoder_param.EnBitrate   = it_usbd_property::m_video_bitrate;
            vencoder_param.EnFrameRate = it_usbd_property::m_video_framerate;
            vencoder_param.EnGOPSize   = it_usbd_property::m_video_gop_size;

            printf(
                "[!] %dx%d@%d, br=%d, gop=%d\n",
                vencoder_param.EnWidth, vencoder_param.EnHeight,
                vencoder_param.EnFrameRate,
                vencoder_param.EnBitrate,
                vencoder_param.EnGOPSize);

            //coreSetVideoEnPara((CAPTURE_VIDEO_SOURCE) m_video_source, encoder_update_flags, coreGetInputSrcInfo(), &vencoder_param);
            printf(
                "[!] coreSetVideoEnPara(%d, %08X)\n",
                m_video_source,
                encoder_update_flags);

            coreSetCaptureSource((CAPTURE_VIDEO_SOURCE) m_video_source);
            coreApiPlay();

            ts_num_packets_padding_to = it_usbd_property::m_video_bitrate * 1000 * it_usbd_property::m_ts_stuffing_ratio / (8 * 188 * 30 * 100);
        }
    }
    else
    {
        if (!it_usbd_ts_reader_ts_stop)
        {
            coreApiStop();
        }
    }
    #endif

    it_usbd_ts_reader_ts_stop = !is_true;

    d                         = PalGetDuration(t);
    printf("---%s(%s) takes %dms @ %d\n", __FUNCTION__, is_true ? "TRUE" : "FALSE", d, PalGetClock());
#endif
}

void
it_usbd_pcgrabber::start()
{
    it_auto_log auto_log(__FUNCTION__, NULL);
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    static int  id = 0;

    //start_encoder(1);
    SendAsyncMsg(MSG_START_ENCODER, 0, 0, 0);

    ts_bytes_read     = 0;
    ts_bytes_read0    = 0;
    it_usb_read_bytes = 0;
    synced            = 0;

    #if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
    PAL_FILE *tmpfile;

    ts_t0_get_buf = PalGetClock();
    ts_t0_read0   = PalGetClock();
    buffer_rptr   = 0;
    file_wptr     = 0;
    times_data    = TIMES_DATA;

        #if defined(ITCONFIG_SAVE_TS)
    if ((tmpfile = tsfile))
    {
        tsfile = NULL;
        PalFileClose(tmpfile, NULL, NULL);
    }

    tsfilename[3] = '0' + ((id / 10) % 10);
    tsfilename[4] = '0' + (id % 10);
    tsfile        = PalWFileOpen(tsfilename, PAL_FILE_WB, NULL, NULL);
    printf("%s() tsfile=%08X\n", __FUNCTION__, tsfile);

    //if (tsfile1)
    //{
    //    PalFileClose(tsfile1, NULL, NULL);
    //}

    //tsfilename1[3] = '0' + ((id / 10) % 10);
    //tsfilename1[4] = '0' + (id % 10);
    //tsfile1 = PalWFileOpen(tsfilename1 , PAL_FILE_WB, NULL, NULL);
    //printf("%s() tsfile1=%08X\n", __FUNCTION__, tsfile1);
        #endif

        #if defined(ITCONFIG_SAVE_PATTERN)
    if ((tmpfile = patfile))
    {
        patfile = NULL;
        PalFileClose(tmpfile, NULL, NULL);
    }

    // TEST
    //patfile = PalWFileOpen((MMP_UINT*) L"C:/clone_pat_00.ts", PAL_FILE_WB, NULL, NULL);
    //PalFileWrite(_buffer, 1, sizeof(_buffer), patfile, NULL, NULL);
    //PalFileClose(patfile, NULL, NULL);
    // TEST

    patfilename[3] = '0' + ((id / 10) % 10);
    patfilename[4] = '0' + (id % 10);
    patfile        = PalWFileOpen(patfilename, PAL_FILE_WB, NULL, NULL);
    printf("%s() patfile=%08X\n", __FUNCTION__, patfile);
        #endif

    if (dummyfile)
    {
        PalFileClose(dummyfile, NULL, NULL);
    }
    dummyfile = PalWFileOpen(dummyfilename, PAL_FILE_WB, NULL, NULL);
    printf("%s() dummyfile=%08X\n", __FUNCTION__, dummyfile);

    id++;
    #elif defined(ITCONFIG_USB_XFERPATTERN)
    buffer_rptr = 0;
    #else
    //buffer_rptr = (coreGetTsMuxBufferWriteIndex() + buffer_size * 8 / 8) % buffer_size;
    buffer_rptr = coreGetTsMuxBufferWriteIndex();
    printf("[!] buffer_rptr=%08X\n", buffer_rptr);
    #endif

    ts_num_null_packets      = 0;
    ts_num_null_packets_last = 0;
#endif
}

void
it_usbd_pcgrabber::stop()
{
    it_auto_log auto_log(__FUNCTION__, NULL);

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    #if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
    PAL_FILE *tmpfile;
    #endif

    // start_encoder(0);
    SendAsyncMsg(MSG_STOP_ENCODER, 0, 0,  0);

    #if defined(ITCONFIG_SAVE_TS)
    if ((tmpfile = tsfile))
    {
        tsfile = NULL;
        printf("%s() tsfile=%08X\n", __FUNCTION__, tmpfile);
        PalFileClose(tmpfile, NULL, NULL);
    }

    //if (tsfile1)
    //{
    //    printf("%s() tsfile1=%08X\n", __FUNCTION__, tsfile1);
    //    PalFileClose(tsfile1, NULL, NULL);
    //    tsfile1 = NULL;
    //}
    #endif
    #if defined(ITCONFIG_SAVE_PATTERN)
    if ((tmpfile = patfile))
    {
        patfile = NULL;
        printf("%s() patfile=%08X\n", __FUNCTION__, tmpfile);
        PalFileClose(tmpfile, NULL, NULL);
    }
    #endif
    #if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
    if (dummyfile)
    {
        printf("%s() dummyfile=%08X\n", __FUNCTION__, dummyfile);
        PalFileClose(dummyfile, NULL, NULL);
        dummyfile = NULL;
    }
    #endif
#endif
}

char *
it_usbd_pcgrabber::get_buffer(
    int&            len)
{
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    unsigned int buffer_wptr;

    if (it_usbd_ts_reader_ts_stop)
    {
        len = 0;
        return NULL;
    }

    if (!buffer || !buffer_size)
    {
        len = 0;
        return NULL;
    }

    buffer_wptr = coreGetTsMuxBufferWriteIndex();

    if (buffer_rptr <= buffer_wptr)
    {
        len = buffer_wptr - buffer_rptr;
    }
    else
    {
        len = buffer_size - buffer_rptr;
    }

    if (len)
    {
        ts_t0_get_buf = PalGetClock();
    }
    else if (PalGetDuration(ts_t0_get_buf) > 10000)
    {
        printf("[X] No available data for more than 10 seconds!\n");
        ts_t0_get_buf = PalGetClock();
    }
    //printf("getbuffer()=%d\n", len);
    #if defined(ITCONFIG_USB_STEMXFER)
    if (len)
    {
        if (len > ITCONFIG_USB_MAXSIZE)
        {
            len = ITCONFIG_USB_MAXSIZE;
        }
        it_usbd_pcgrabber->advance_buffer(len);
    }
    len = 0;
    return NULL;
    #elif defined(ITCONFIG_USB_XFERPATTERN)
    return _buffer + buffer_rptr;
    #else
    // TEST
    if (0 && !synced && len)
    {
        printf("[!] ts_bytes_read: %lld, buffer_rptr:%d, buffer_wptr:%d @ %d\n", ts_bytes_read % buffer_size, buffer_rptr, buffer_wptr, PalGetClock());
        for (int i = 0; i < len - 188 * 3; i++)
        {
            if (buffer[(buffer_rptr + i) % buffer_size] == 0x47 &&
                buffer[(buffer_rptr + i + 188) % buffer_size] == 0x47 &&
                buffer[(buffer_rptr + i + 188 * 2) % buffer_size] == 0x47)
            {
                synced = ts_bytes_read + i;
                printf("[!] synced@%lld\n", synced);
                break;
            }
        }
    }
    // TEST
    return buffer + buffer_rptr;
    #endif
#else
    len = 0;
    return NULL;
#endif
}

void
it_usbd_pcgrabber::fill_buffer()
{
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    // do nothing
#endif
}

void
it_usbd_pcgrabber::advance_buffer(
    int len)
{
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    MMP_UINT32 ts_wptr;
    int        nwrite;

    if (!buffer || !buffer_size)
    {
        return;
    }

    ts_wptr = coreGetTsMuxBufferWriteIndex();

    #if !defined(ITCONFIG_SAVE_INMAIN)
        #if defined(ITCONFIG_SAVE_TS)
    if (tsfile && len)
    {
        nwrite = PalFileWrite(buffer + buffer_rptr, 1, len, tsfile, NULL, NULL);
        printf("nwrite(tsfile)=%d\n", nwrite);
    }

    //if (tsfile1 && len)
    //{
    //    nwrite = PalFileWrite(buffer + buffer_rptr, 1, len, tsfile1, NULL, NULL);
    //    printf("nwrite(tsfile1)=%d\n", nwrite);
    //}
        #endif
        #if defined(ITCONFIG_SAVE_PATTERN)
    if (patfile && len)
    {
        nwrite = PalFileWrite(_buffer + buffer_rptr, 1, len, patfile, NULL, NULL);
        printf("nwrite(patfile)=%d\n", nwrite);
    }
        #endif

        #if defined(ITCONFIG_SAVE_TS) || defined(ITCONFIG_SAVE_PATTERN)
    if (!--times_data)
    {
        times_data = TIMES_DATA;
    }
    else
    {
        return;
    }
        #endif
    #endif

    // TEST
    //printf("[!] buffer_rptr %d=>%d +%d\n", buffer_rptr, (buffer_rptr + len) % buffer_size, len);
    // TEST

    // TEST
    if (0 && synced)
    {
        for (int i = (synced % buffer_size); i < buffer_rptr + len; i += 188, synced += 188)
        {
            if (buffer[i] != 0x47)
            {
                printf("[X] !synced@%lld\n", synced);
                synced = 0;
                break;
            }
        }
    }
    // TEST
    buffer_rptr += len;

    if (buffer_rptr > buffer_size)
    {
        printf("[X] buffer_rptr=%d\n", buffer_rptr);
    }

    if (buffer_rptr >= buffer_size)
    {
        buffer_rptr = 0;
    }

    ts_bytes_read     += len;
    ts_bytes_read0    += len;
    it_usb_read_bytes += len;

    if (ts_bytes_read0 > ITCONFIG_XFER_HASHSIZE)
    {
        ts_bytes_read0          -= ITCONFIG_XFER_HASHSIZE;
        //printf("TS05+%5d@%d: 0x%llX  0x%08X%08X bytes read!\r\n", PalGetDuration(ts_t0_read0), PalGetClock(), ts_bytes_read, (unsigned int) (ts_bytes_read >> 32), (unsigned int) (ts_bytes_read & 0x00000000ffffffffUL));
        printf("TS05+%5d@%d: 0x%llX bytes read, %llu (%+llu) null packets!\r\n", PalGetDuration(ts_t0_read0), PalGetClock(), ts_bytes_read, ts_num_null_packets, ts_num_null_packets - ts_num_null_packets_last);
        ts_num_null_packets_last = ts_num_null_packets;
        ts_t0_read0              = PalGetClock();
    }
#endif
}

void
it_usbd_pcgrabber::set_source(
    int audio_source,
    int video_source)
{
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    switch (video_source)
    {
    default:
        printf("[X] %s(%d, %d)\n", __FUNCTION__, audio_source, video_source);
        break;

    case it_usbd_property::VIDEO_SOURCE_HDMI:
        if (audio_source == it_usbd_property::AUDIO_SOURCE_HDMI)
        {
            printf("%s() ==> %s:%d\n", __FUNCTION__, IT_USBD_STRINGIZE(CAPTURE_VIDEO_SOURCE_HDMI), CAPTURE_VIDEO_SOURCE_HDMI);
            m_video_source = CAPTURE_VIDEO_SOURCE_HDMI;
        }
        else
        {
            printf("%s() ==> %s:%d\n", __FUNCTION__, IT_USBD_STRINGIZE(CAPTURE_VIDEO_SOURCE_DVI), CAPTURE_VIDEO_SOURCE_DVI);
            m_video_source = CAPTURE_VIDEO_SOURCE_DVI;
        }
        break;

    case it_usbd_property::VIDEO_SOURCE_YPBPR:
        printf("%s() ==> %s:%d\n", __FUNCTION__, IT_USBD_STRINGIZE(CAPTURE_VIDEO_SOURCE_YPBPR), CAPTURE_VIDEO_SOURCE_YPBPR);
        m_video_source = CAPTURE_VIDEO_SOURCE_YPBPR;
        break;

    case it_usbd_property::VIDEO_SOURCE_SVIDEO:
        printf("%s() ==> %s:%d\n", __FUNCTION__, IT_USBD_STRINGIZE(CAPTURE_VIDEO_SOURCE_SVIDEO), CAPTURE_VIDEO_SOURCE_SVIDEO);
        m_video_source = CAPTURE_VIDEO_SOURCE_SVIDEO;
        break;

    case it_usbd_property::VIDEO_SOURCE_COMPOSITE:
        printf("%s() ==> %s:%d\n", __FUNCTION__, IT_USBD_STRINGIZE(CAPTURE_VIDEO_SOURCE_CVBS), CAPTURE_VIDEO_SOURCE_CVBS);
        m_video_source = CAPTURE_VIDEO_SOURCE_CVBS;
        break;

    case it_usbd_property::VIDEO_SOURCE_DSUB:
        printf("%s() ==> %s:%d\n", __FUNCTION__, IT_USBD_STRINGIZE(CAPTURE_VIDEO_SOURCE_VGA), CAPTURE_VIDEO_SOURCE_VGA);
        m_video_source = CAPTURE_VIDEO_SOURCE_VGA;
        break;
    }
#endif
}

int
it_usbd_pcgrabber::is_upgrading()
{
    return m_is_upgrading;
}

void
it_usbd_pcgrabber::upgrade(void *image, unsigned int length)
{
    portBASE_TYPE xReturn;

    if (image && length && !m_is_upgrading)
    {
        m_is_upgrading    = 1;

        m_firmware_image  = (unsigned char *) image;
        m_firmware_length = length;

        xReturn           = xTaskCreate(
            ::upgrader_task,
            ( signed portCHAR * ) "IT_USBD_UPGRADER",
            1024,
            ( void * ) NULL,
            tskIDLE_PRIORITY + 1,
            ( xTaskHandle * ) NULL);

        if (xReturn != pdPASS)
        {
            printf("[X] Can't create task to upgrade firmware!!!\n");
            m_firmware_image  = 0;
            m_firmware_length = 0;
            m_is_upgrading    = 0;
        }
    }
}

extern "C" int
it_usbd_main()
{
    it_auto_log auto_log(__FUNCTION__, NULL);
    printf("it_usbd_main()@%d\n", PalGetClock());
#if (0)
    try{
        static class it_usbd_pcgrabber _x_it_usbd_pcgrabber;
        static class it_usbd_pcgrabber *x_it_usbd_pcgrabber = new class it_usbd_pcgrabber ();
    }
    catch (...)
    {
        printf("[X] %s\n", __FUNCTION__);
    }
#endif

    printf("it_usbd_pcgrabber=%08X %08X\n", it_usbd_pcgrabber, &it_usbd_pcgrabber);
    printf("it_usbd_pcgrabber_base=%08X %08X\n", it_usbd_pcgrabber_base, &it_usbd_pcgrabber_base);
    return it_usbd_pcgrabber->main();
}