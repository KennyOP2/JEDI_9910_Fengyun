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

class it_usbd_pcgrabber : public it_usbd_pcgrabber_base
{
private:
    void start_encoder(int is_true);
    void fill_buffer();
    int           m_is_upgrading;
    unsigned char *m_firmware_image;
    unsigned int  m_firmware_length;
    unsigned int  m_audio_source;
    unsigned int  m_video_source;

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
        SendDelayedMsg(enable ? MSG_DEVICE_MODE : MSG_HOST_MODE, 100, 0, 0, 0);
        printf("---%s(%d->%d)\n", __FUNCTION__, it_usbd_property::m_pc_mode, enable);
    }

    unsigned short get_usb_vid()
    {
        return 0x048D;  // ITE
        //return 0x2040;  // HPG
        //return 0x1B80;  // ROXIO
    }

    unsigned short get_usb_pid()
    {
        return 0x9910;
        //return 0xF800;
        //return 0xE012;
    }

    const char *get_usb_manufacturer_string()
    {
        //return "ITE TECH. INC.";
        return "Fengyun Computer Works, Inc.";
        //return "Roxio Inc.";
    }

    const char *get_usb_product_string()
    {
        //return "IT9910 Grabber Device (HD)";
        return "ezcap HD Capture";
        //return "GameCAP HD PRO";
    }

    const char *get_usb_serial_number_string()
    {
        static char serial_number_string[256];

        serial_number_string[0]   = 0;
        coreGetUsbSerialNumber((char *) serial_number_string);
        serial_number_string[253] = 0;
        return serial_number_string;

        //return "ITE"                        "."
        return "HPG"                        "."
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
#define IT_CONFIG_VIDEO_WIDTH   IT_SELECT2(1, 1280,  1920)
#define IT_CONFIG_VIDEO_HEIGHT  IT_SELECT2(1,  720,  1080)
#define IT_CONFIG_VIDEO_BITRATE IT_SELECT3(2, 13600, 16000, 18000)
#define IT_CONFIG_AUDIO_CODEC   IT_SELECT2(1, MPEG_AUDIO_ENCODER, AAC_AUDIO_ENCODER)

//static class it_usbd_pcgrabber _it_usbd_pcgrabber;
class it_usbd_pcgrabber      *it_usbd_pcgrabber                = NULL;
class it_usbd_pcgrabber_base *it_usbd_pcgrabber_base           = NULL;

#if (CONFIG_HAVE_USBD)
unsigned char                it_usbd_null_packets_16[188 * 16] =
{
    0x47, 0x5F, 0xFF, 0x20, 0xB7, 0x00, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
};
#endif

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
#define                         ITCONFIG_USB_MAXSIZE   (256 * 1024)
#define                         ITCONFIG_XFER_HASHSIZE (0x100000 * 16)
static char               *buffer     = 0;
static unsigned int       buffer_size = 0;
static unsigned int       buffer_rptr = 0;
static unsigned long long synced      = 0;

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

int
it_usbd_pcgrabber::main()
{
    it_auto_log auto_log(__FUNCTION__, NULL);

    // coreInitVideoEnPara();
    it_usbd_property::m_pc_mode = !0;
    while (gbDeviceMode && it_usbd_property::m_pc_mode)
    {
        //::SendDelayedMessages();
        mmpWatchDogRefreshTimer();

        PalSleep(1);
    }
    it_usbd_property::m_pc_mode = 0;

    return 0;
}

void
it_usbd_pcgrabber::start_encoder(
    int is_true)
{}

void
it_usbd_pcgrabber::start()
{
    it_auto_log auto_log(__FUNCTION__, NULL);
}

void
it_usbd_pcgrabber::stop()
{
    it_auto_log auto_log(__FUNCTION__, NULL);

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    start_encoder(0);
#endif
}

char *
it_usbd_pcgrabber::get_buffer(
    int& len)
{
    len = 0;
    return NULL;
}

void
it_usbd_pcgrabber::fill_buffer()
{}

void
it_usbd_pcgrabber::advance_buffer(
    int len)
{}

void
it_usbd_pcgrabber::set_source(
    int audio_source,
    int video_source)
{}

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