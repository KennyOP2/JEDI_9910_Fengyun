/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file host.c
 *
 * @author Vincent Lee
 */

#ifdef WIN32
    #include "host/host_memsetting.h"
    #include "host/host_bussetting.h"
    #include "usb2spi/usb2spi.h"
#endif

#include "sys/sys.h"
#include "host/host.h"
#include "host/gpio.h"
#include "host/ahb.h"
#include "mmp.h"
//#include "cmq/cmd_queue.h"
//#include "mmp_isp.h"
//#include "mmp_fpc.h"

#define CARD_ALWAYS_POWER_ON

//=============================================================================
//                              Global Data Definition
//=============================================================================

/* For MMIO interface */
#define MMIO_ADDR 0xC0000000

#ifdef WIN32
static MMP_UINT8 *gMemBaseAddr = MMP_NULL;
#endif

#define SD_IO_CNT                10

//#if 1

#if defined(IT9913_128LQFP)
    #define GPIO_CARD_POWER      17
    #define GPIO_CARD_DETECT     5
#elif defined (IT9919_144TQFP)
    #if defined(REF_BOARD_AVSENDER)
        #define GPIO_CARD_POWER  17 //for dexatek
        #define GPIO_CARD_DETECT 5  //for dexatek
    #elif defined (REF_BOARD_CAMERA)
        #define GPIO_CARD_POWER  MMP_POWER_ALWAYS_ON
        #define GPIO_CARD_DETECT MMP_CD_ALWAYS_OFF
    #else
        #define GPIO_CARD_POWER  44
        #define GPIO_CARD_DETECT 42
    #endif
#else
    #define GPIO_CARD_POWER      44
    #define GPIO_CARD_DETECT     42
#endif

MMP_UINT32 gpio_card_power_enable      = GPIO_CARD_POWER;
MMP_UINT32 gpio_card_detect            = GPIO_CARD_DETECT;
MMP_UINT32 gpio_sd_card_write_protect  = MMP_WP_ALWAYS_OFF;
MMP_UINT32 gpio_card2_power_enable     = MMP_POWER_ALWAYS_ON;
MMP_UINT32 gpio_card2_detect           = MMP_CD_ALWAYS_ON;
MMP_UINT32 gpio_sd_card2_write_protect = MMP_WP_ALWAYS_OFF;
MMP_UINT   sd_io[SD_IO_CNT]            = {6, 8, 9, 10, 11, 12, 38, 39, 40, 41};
MMP_UINT   sd2_io[SD_IO_CNT] = {7, 8, 9, 10, 11, 12, 38, 39, 40, 41};
//#else
//MMP_UINT32 gpio_card_power_enable       = 0;
//MMP_UINT32 gpio_card_detect             = 0;
//MMP_UINT32 gpio_sd_card_write_protect   = 0;
//MMP_UINT32 gpio_card2_power_enable      = 0;
//MMP_UINT32 gpio_card2_detect            = 0;
//MMP_UINT32 gpio_sd_card2_write_protect  = 0;
//MMP_UINT8  sd_io[SD_IO_CNT]  = {0};
//MMP_UINT8  sd2_io[SD_IO_CNT] = {0};
//#endif
//MMP_UINT32 gpio_cf_power_enable         = 12;
//MMP_UINT32 gpio_cf_detect               = 11;

//MMP_UINT32 gpio_2sd_switch              = 0;
//MMP_UINT32 gpio_ir                      = 0;
//MMP_UINT32 gpio_spdif                   = 0;
//MMP_UINT32 gpio_keypad_in               = 0;
//MMP_UINT32 gpio_fpc_ctl                 = 0;
//MMP_UINT32 gpio_fpc_clk                 = 0;
//MMP_UINT32 gpio_fpc_dat                 = 0;
//MMP_UINT32 gpio_iic                     = 0;
//MMP_UINT32 gpio_uart                    = 0;

//REG_SETTING reset_engine[] =
//{
//	  {MMP_HOST_CLOCK_REG_12,     MMP_HOST_RESET},        // MMP_RESET_HOST
//	  {MMP_MEM_CLOCK_REG_16,      MMP_MEM_RESET},         // MMP_RESET_MEM
//	  {MMP_AHB_CLOCK_REG_1A,      MMP_AHB_RESET},         // MMP_RESET_AHB
//	  {MMP_APB_CLOCK_REG_1E,      MMP_APB_RESET},         // MMP_RESET_APB
//	  {MMP_APB_CLOCK_REG_1E,      MMP_SDIP_RESET},        // MMP_RESET_SD_IP
//	  {MMP_APB_CLOCK_REG_1E,      MMP_UART_RESET},        // MMP_RESET_UART
//	  {MMP_2D_CLOCK_REG_26,       MMP_2D_RESET},          // MMP_RESET_2D
//    {MMP_2D_CLOCK_REG_26,       MMP_2DCMQ_RESET},       // MMP_RESET_2D_CMQ
//	  {MMP_LCD_CLOCK_REG_2A,      MMP_LCD_RESET},         // MMP_RESET_LCD
//    {MMP_LCD_CLOCK_REG_2A,      MMP_LCDGBL_RESET},      // MMP_RESET_LCD_GLOBAL
//    {MMP_TVE_CLOCK_REG_2E,      MMP_TVE_RESET},         // MMP_RESET_TV_ENCODER
//    {MMP_ISP_CLOCK_REG_32,      MMP_ISP_RESET},         // MMP_RESET_ISP
//    {MMP_ISP_CLOCK_REG_32,      MMP_ISPGBL_RESET},      // MMP_RESET_ISP_GLOBAL
//    {MMP_ISP_CLOCK_REG_32,      MMP_ISPCMQGBL_RESET},   // MMP_RESET_ISP_CMQ_GLOBAL
//    {MMP_VIDEO_CLOCK_REG_36,    MMP_VIDEO_RESET},       // MMP_RESET_VIDEO
//    {MMP_VIDEO_CLOCK_REG_36,    MMP_VIDEOGBL_RESET},    // MMP_RESET_VIDEO_GLOBAL
//    {MMP_VIDEO_CLOCK_REG_36,    MMP_JPEG_RESET},        // MMP_RESET_JPEG
//    {MMP_VIDEO_CLOCK_REG_36,    MMP_JPEGGBL_RESET},     // MMP_RESET_JPEG_GLOBAL
//    {MMP_AUDIO_CLOCK_REG_3E,    MMP_I2S_RESET},         // MMP_RESET_I2S
//    {MMP_AUDIO_CLOCK_REG_3E,    MMP_I2SGBL_RESET},      // MMP_RESET_I2S_GLOBAL
//    {MMP_AUDIO_CLOCK_REG_3E,    MMP_DAC_RESET},         // MMP_RESET_DAC
//    {MMP_PCR_CLOCK_REG_42,      MMP_PCR_RESET},         // MMP_RESET_PCR
//    {MMP_RISC_CLOCK_REG_44,     MMP_RISC0_RESET},       // MMP_RESET_RISC0
//    {MMP_RISC_CLOCK_REG_44,     MMP_JTAG_RESET},        // MMP_RESET_JTAG
//    {MMP_RISC_CLOCK_REG_44,     MMP_RISC1_RESET},       // MMP_RESET_RISC1
//    {MMP_USB_CLOCK_REG_46,      MMP_USB_RESET},         // MMP_RESET_USB
//    {MMP_TSI_CLOCK_REG_48,      MMP_TSI0_RESET},        // MMP_RESET_TSI0
//    {MMP_TSI_CLOCK_REG_48,      MMP_TSI1_RESET},        // MMP_RESET_TSI1
//    {MMP_DEMOD_CLOCK_REG_4A,    MMP_DEMOD_RESET},       // MMP_RESET_DEMODULATOR
//};

//=============================================================================
//                              Function Definition
//=============================================================================

static __inline void MMIO_Write(unsigned short addr, unsigned short data)
{
    *(volatile unsigned short *) (MMIO_ADDR + (unsigned int)addr) = data;
}

static __inline unsigned short MMIO_Read(unsigned short addr)
{
    return *(volatile unsigned short *) (MMIO_ADDR + (unsigned int)addr);
}

static __inline void MMIO_WriteMask(unsigned short addr, unsigned short data, unsigned short mask)
{
    MMIO_Write(addr, ((MMIO_Read(addr) & ~mask) | (data & mask)));
}

#if defined(__FREERTOS__)
static void swap_byte_order_and_copy_int(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    MMP_UINT32 i  = 0;
    MMP_UINT8  *s = (MMP_UINT8 *) srcAddress;
    MMP_UINT8  *d = (MMP_UINT8 *) destAddress;

    PRECONDITION(sizeInByte == 4 || sizeInByte == 2);
    PRECONDITION(0 != destAddress);
    PRECONDITION(destAddress % 2 == 0);

    /* check against overlap */
    PRECONDITION(s + sizeInByte <= d || d + sizeInByte <= s);

    /* swap byte order */
    s += (sizeInByte - 1);
    for (i = 0; i < sizeInByte; ++i)
        *d++ = *s--;
}

MMP_UINT8 *
HOST_GetVramBaseAddress(
    void)
{
    return 0;
}
#endif

void
HOST_ReadRegister(
    MMP_UINT16 destAddress,
    MMP_UINT16 *data)
{
#ifdef WIN32
    usb2spi_ReadRegister(destAddress, data);
#else
    PRECONDITION(data);
    *data = MMIO_Read(destAddress);
#endif
}

void
HOST_WriteRegister(
    MMP_UINT16 destAddress,
    MMP_UINT16 data)
{
#ifdef WIN32
    usb2spi_WriteRegister(destAddress, data);
#else
    PRECONDITION((destAddress % 2) == 0);
    MMIO_Write(destAddress, data);
#endif
}

void
HOST_WriteRegisterMask(
    MMP_UINT16 destAddress,
    MMP_UINT16 data,
    MMP_UINT16 mask)
{
    MMP_UINT16 oldValue = 0;

#ifdef WIN32
    usb2spi_ReadRegister(destAddress, &oldValue);
    data = (data & mask) | (oldValue & (~mask));
    usb2spi_WriteRegister(destAddress, data);
#else
    UNREFERENCED_VARIABLE(oldValue);
    MMIO_WriteMask(destAddress, data, mask);
#endif
}

void
HOST_SetBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT8 bytePattern,
    MMP_ULONG sizeInByte)
{
#if defined(WIN32)
    MMP_UINT32 i;
    MMP_UINT8  *srcAddress = SYS_Malloc(sizeInByte);
    for (i = 0; i < sizeInByte; ++i)
        *(srcAddress + i) = bytePattern;

    usb2spi_WriteMemory((destAddress - (MMP_UINT32) gMemBaseAddr), (MMP_UINT32)srcAddress, sizeInByte);
    SYS_Free(srcAddress);
#elif defined(__FREERTOS__)
    memset((void *)destAddress, (int)bytePattern, (size_t)sizeInByte);
#endif
}

void
HOST_WriteBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#ifdef WIN32
    usb2spi_WriteMemory((destAddress - (MMP_UINT32) gMemBaseAddr), srcAddress, sizeInByte);
#else
    PRECONDITION(sizeInByte % 2 == 0);
    PRECONDITION(destAddress % 2 == 0);
    memcpy((void *) destAddress, (const void *) srcAddress, sizeInByte);
#endif
}

void
HOST_ReadBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#ifdef WIN32
    usb2spi_ReadMemory((destAddress - (MMP_UINT32) gMemBaseAddr), srcAddress, sizeInByte);
#else
    memcpy((void *) destAddress, (const void *) srcAddress, sizeInByte);
    return;
#endif
}

void
HOST_ReadInt16ToEngine(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#if defined(__FREERTOS__)
    MMP_UINT32 i = 0;

    for (i = 0; i < (sizeInByte >> 1); i++)
    {
        swap_byte_order_and_copy_int(destAddress, srcAddress, 2);
        destAddress += 2;
        srcAddress  += 2;
    }
#else
    HOST_ReadBlockMemory(destAddress, srcAddress, sizeInByte);
#endif
}

MMP_UINT32
HOST_GetChipVersion(
    void)
{
    MMP_UINT16 chipId    = 0;
    MMP_UINT16 reversion = 0;

    HOST_ReadRegister(MMP_GENERAL_CONFIG_REG_02, &chipId);
    HOST_ReadRegister(MMP_GENERAL_CONFIG_REG_04, &reversion);
    return (MMP_UINT32) ((chipId << 16) | reversion);
}

//void
//HOST_ResetEngine(
//    MMP_UINT16 engine)
//{
//	if (engine > (sizeof(reset_engine)/sizeof(reset_engine[0])))
//		return;
//
//	HOST_WriteRegisterMask(reset_engine[engine].reg, 0xFFFF, reset_engine[engine].data);
//	MMP_Sleep(1);
//	HOST_WriteRegisterMask(reset_engine[engine].reg, 0x0000, reset_engine[engine].data);
//	MMP_Sleep(1);
//}

MMP_BOOL
HOST_IsEngineIdle(
    MMP_UINT16 reg,
    MMP_UINT16 mask,
    MMP_UINT16 condition)
{
    volatile MMP_UINT32 i            = 0;
    MMP_UINT16          registerData = 0;

    for (i = 0; i < 100000; i++)
    {
        HOST_ReadRegister(reg, &registerData);
        if ((registerData & mask) == condition)
            return MMP_TRUE;
    }
    return MMP_FALSE;
}

void
HOST_EnableInterrupt(
    MMP_UINT16 engine)
{
    HOST_WriteRegisterMask(MMP_GENERAL_INTERRUPT_REG_06, 0xFFFF, 1 << engine);
}

MMP_UINT16
HOST_GetInterruptStatus(
    MMP_UINT16 engine)
{
    MMP_UINT16 status = 0;

    HOST_ReadRegister(MMP_GENERAL_INTERRUPT_REG_0C, &status);
    return status;
}

//=============================================================================
//                              Function Definition - Win32 Only
//=============================================================================

#ifdef WIN32
    #if 0
void
HOST_ResetAllEngine(
    void)
{
    MMP_UINT i = 0;

    for (; i < MMP_RESET_COUNT; i++)
        HOST_ResetEngine(i);
}
    #endif

MMP_UINT8 *
HOST_GetVramBaseAddress(
    void)
{
    return gMemBaseAddr;
}

void
HOST_ReadOffsetBlockMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    HOST_ReadBlockMemory(destAddress, (MMP_UINT32) gMemBaseAddr + srcAddress, sizeInByte);
}

static void
HOST_SetMemoryBaseAddress(
    void)
{
    MMP_UINT16 memoryBaseHi = 0;
    MMP_UINT16 memoryBaseLo = 0;

    memoryBaseLo = (MMP_UINT16) gMemBaseAddr;
    HOST_WriteRegister(MMP_HOST_BUS_CONTROLLER_REG_204, memoryBaseLo);

    memoryBaseHi = (MMP_UINT16) ((MMP_UINT32) gMemBaseAddr >> (sizeof(MMP_UINT16) * 8));
    HOST_WriteRegister(MMP_HOST_BUS_CONTROLLER_REG_206, memoryBaseHi);
}

static void
HOST_MemorySettingInit()
{
    MMP_UINT i    = 0;
    MMP_UINT size = 0;

    size = sizeof(MemorySetting) / sizeof(MemorySetting[0]);

    for (i = 0; i < size; i++)
    {
        if (MemorySetting[i].reg == 0xFFFF)
            MMP_Sleep(MemorySetting[i].data);
        else
            HOST_WriteRegister(MemorySetting[i].reg, MemorySetting[i].data);
    }
}

static void
HOST_HostBusSettingInit()
{
    MMP_UINT i    = 0;
    MMP_UINT size = 0;

    size = sizeof(HostBusSetting) / sizeof(HostBusSetting[0]);
    for (i = 0; i < size; i++)
    {
        if (HostBusSetting[i].reg == 0xFFFF)
            MMP_Sleep(HostBusSetting[i].data);
        else
            HOST_WriteRegister(HostBusSetting[i].reg, HostBusSetting[i].data);
    }
}

void CreateStorageSemaphore();

MMP_RESULT
MMP_Initialize(
    MMP_UINT32 vramOffset)
{
    gMemBaseAddr = (MMP_UINT8 *) vramOffset;
    //HOST_ResetAllEngine();
    HOST_HostBusSettingInit();
    HOST_SetMemoryBaseAddress();
    HOST_MemorySettingInit();

    return MMP_RESULT_SUCCESS;
}
#endif

//=============================================================================
//                         Storage related feature
//=============================================================================

/** for all storage share pin use */
static void *storage_semaphore = MMP_NULL;

/* Get smaphore for all storage. */
void *HOST_GetStorageSemaphore(void)
{
    if (!storage_semaphore)
    {
        storage_semaphore = SYS_CreateSemaphore(1, "STORAGE");
        if (!storage_semaphore)
            printf(" @@ create storage semaphore fail! \n");
    }
    return storage_semaphore;
}

MMP_BOOL HOST_IsCardInserted(MMP_UINT32 card)
{
    MMP_BOOL   insert  = MMP_FALSE;
    MMP_UINT32 data    = 0;
    MMP_UINT32 gpio_cd = 0;

    switch (card)
    {
    case MMP_CARD_SD_I:
    case MMP_CARD_MS:
    case MMP_CARD_MS_mode4:
        gpio_cd = gpio_card_detect;
        break;

    case MMP_CARD_SD2:
        gpio_cd = gpio_card2_detect;
        break;

    default:
        break;
    }

    if (gpio_cd == MMP_CD_ALWAYS_ON)
    {
        insert = MMP_TRUE;
        goto end;
    }
    if ((gpio_cd == MMP_CD_ALWAYS_OFF) || (gpio_cd == 0))
    {
        insert = MMP_FALSE;
        goto end;
    }

    GPIO_Enable(gpio_cd);
    GPIO_SetMode(gpio_cd, GPIO_MODE_INPUT);
    data = GPIO_GetState(gpio_cd); // return GPIO_STATE_LO or GPIO_STATE_HI
    if (data == GPIO_STATE_HI)
        insert = MMP_FALSE;
    else
        insert = MMP_TRUE;

end:
    return insert;
}

MMP_BOOL mmpIsCardInserted(MMP_UINT32 card)
{
    return HOST_IsCardInserted(card);
}

MMP_BOOL HOST_IsCardLocked(MMP_UINT32 card)
{
    MMP_BOOL   lock    = MMP_FALSE;
    MMP_UINT32 data    = 0;
    MMP_UINT32 gpio_wp = MMP_WP_ALWAYS_OFF;

    switch (card)
    {
    case MMP_CARD_SD_I:
        gpio_wp = gpio_sd_card_write_protect;
        break;

    case MMP_CARD_SD2:
        gpio_wp = gpio_sd_card2_write_protect;
        break;

    default:
        break;
    }

    if (gpio_wp == MMP_WP_ALWAYS_ON)
    {
        lock = MMP_TRUE;
        goto end;
    }
    if (gpio_wp == MMP_WP_ALWAYS_OFF)
    {
        lock = MMP_FALSE;
        goto end;
    }

    GPIO_Enable(gpio_wp);
    GPIO_SetMode(gpio_wp, GPIO_MODE_INPUT);
    data = GPIO_GetState(gpio_wp); // return GPIO_STATE_LO or GPIO_STATE_HI
    if (data == GPIO_STATE_HI)
        lock = MMP_TRUE;
    else
        lock = MMP_FALSE;

end:
    return lock;
}

#if 0
typedef enum MMP_CARD_TAG
{
    MMP_CARD_SD_I,
    MMP_CARD_MS,
    MMP_CARD_SD2,
    MMP_CARD_CF,
    MMP_CARD_XD_IP,
    MMP_CARD_MS_mode4
} MMP_CARD;
#endif

void HOST_StorageIoUnSelect(MMP_UINT32 card)
{
    MMP_UINT8 i = 0;
    switch (card)
    {
    case MMP_CARD_SD_I:
        for (i = 0; i < SD_IO_CNT; i++)
            ithGpioCtrlDisable(sd_io[i], ITH_GPIO_PULL_ENABLE);             /* sd io pull disable */
        break;

    case MMP_CARD_SD2:
        for (i = 0; i < SD_IO_CNT; i++)
            ithGpioCtrlDisable(sd2_io[i], ITH_GPIO_PULL_ENABLE);             /* sd io pull disable */
        break;
    }
}

void HOST_CardPowerReset(MMP_UINT32 card, MMP_UINT32 sleeptime)
{
    MMP_UINT32 gpio_pe = MMP_POWER_ALWAYS_ON;

    HOST_StorageIoUnSelect(card);

    switch (card)
    {
    case MMP_CARD_SD_I:
    case MMP_CARD_MS:
    case MMP_CARD_MS_mode4:
        gpio_pe = gpio_card_power_enable;
        break;

    case MMP_CARD_SD2:
        gpio_pe = gpio_card2_power_enable;
        break;

    default:
        break;
    }

    if (gpio_pe == MMP_POWER_ALWAYS_ON)
        return;
    if (gpio_pe == MMP_POWER_ALWAYS_OFF)
        return;

    /** always power on */
    GPIO_Enable(gpio_pe);
    GPIO_SetMode(gpio_pe, GPIO_MODE_OUTPUT);
    GPIO_SetState(gpio_pe, GPIO_STATE_HI);  // power down
    MMP_Sleep(sleeptime);                   // for general case
    GPIO_SetState(gpio_pe, GPIO_STATE_LO);  // power up
}

void HOST_StorageIoSelect(MMP_UINT32 card)
{
    MMP_UINT8 i = 0;
    switch (card)
    {
    case MMP_CARD_SD_I:
        for (i = 0; i < SD_IO_CNT; i++)
        {
            ithGpioSetMode(sd_io[i], ITH_GPIO_MODE2);             /* switch I/O to SD/MMC controller */
            ithGpioCtrlEnable(sd_io[i], ITH_GPIO_PULL_ENABLE);    /* sd io pull up */
            ithGpioCtrlEnable(sd_io[i], ITH_GPIO_PULL_UP);
        }
        /** disable SD2 clock */
        break;

    case MMP_CARD_SD2:
        for (i = 0; i < SD_IO_CNT; i++)
        {
            ithGpioSetMode(sd2_io[i], ITH_GPIO_MODE2);             /* switch I/O to SD/MMC controller */
            ithGpioCtrlEnable(sd2_io[i], ITH_GPIO_PULL_ENABLE);    /* sd io pull up */
            ithGpioCtrlEnable(sd2_io[i], ITH_GPIO_PULL_UP);
        }
        break;
    }
}

void HOST_SetReserveIOInputMode(
    MMP_BOOL ebable)
{
    static MMP_UINT16 recordReg[16] = {0};
    static MMP_UINT16 record2Reg[8] = {0};
    MMP_UINT16        i, j;

    if (ebable == MMP_TRUE)
    {
        //SGPIO
        for (i = MMP_GENERAL_GPIO_REG_100, j = 0; i < MMP_GENERAL_GPIO_REG_110; i += 2, j++)
            HOST_ReadRegister(i, &recordReg[j]);

        for (i = MMP_GENERAL_GPIO_REG_100; i < MMP_GENERAL_GPIO_REG_110; i += 2)
            HOST_WriteRegisterMask(i, (2 << 13) | (2 << 13) | (2 << 9) | 7, (3 << 13) | (3 << 11) | (2 << 9) | 7);  /** pull high/down */
        //DGPIO
        HOST_ReadRegister(0x7808, &record2Reg[0]);
        HOST_ReadRegister(0x780a, &record2Reg[1]);
        HOST_ReadRegister(0x7848, &record2Reg[2]);
        HOST_ReadRegister(0x784A, &record2Reg[3]);
        HOST_ReadRegister(0x784C, &record2Reg[4]);
        HOST_WriteRegister(0x7848, (1 << 8)); //ir
        HOST_WriteRegister(0x784a, 0);
        HOST_WriteRegister(0x7808, (1 << 7)); //print msg
        HOST_WriteRegister(0x780a, 0);
        HOST_WriteRegisterMask(0x784c, 0, 0xFF);
    }
    else
    {
        //SGPIO
        for (i = MMP_GENERAL_GPIO_REG_100, j = 0; i < MMP_GENERAL_GPIO_REG_110; i += 2, j++)
            HOST_WriteRegister(i, recordReg[j]);
        //DGPIO
        HOST_WriteRegister(0x7848, record2Reg[2]);
        HOST_WriteRegister(0x784a, record2Reg[3]);
        HOST_WriteRegister(0x7808, record2Reg[0]);
        HOST_WriteRegister(0x780a, record2Reg[1]);
        HOST_WriteRegister(0x784C, record2Reg[4]);
    }
}

//=============================================================================
/**
 * Setting all Driver need GPIO
 *
 * @param attribList        GPIO Setting for Driver Need.
 * @return none
 */
//=============================================================================
void
mmpInitializeGPIO(MMP_ULONG *attribList)
{
    MMP_UINT32 gpioPin = 0;

    while (*attribList < MMP_GPIO_MAX)
    {
        switch (*attribList++)
        {
        case MMP_GPIO_CARD_POWER_ENABLE:
            gpio_card_power_enable = *attribList++;
#if defined(CARD_ALWAYS_POWER_ON)
            if ((gpio_card_power_enable != MMP_POWER_ALWAYS_ON) &&
                (gpio_card_power_enable != MMP_POWER_ALWAYS_OFF))
            {
                GPIO_Enable(gpio_card_power_enable);
                GPIO_SetMode(gpio_card_power_enable, GPIO_MODE_OUTPUT);
                GPIO_SetState(gpio_card_power_enable, GPIO_STATE_LO);      // power up
            }
#endif
            break;

        case MMP_GPIO_CARD_DETECT:
            gpio_card_detect = *attribList++;
            break;

        case MMP_GPIO_CARD_WP:
            gpio_sd_card_write_protect = *attribList++;
            break;

        case MMP_GPIO_CARD2_POWER_ENABLE:
            gpio_card2_power_enable = *attribList++;
#if defined(CARD_ALWAYS_POWER_ON)
            if ((gpio_card2_power_enable != MMP_POWER_ALWAYS_ON) &&
                (gpio_card2_power_enable != MMP_POWER_ALWAYS_OFF))
            {
                GPIO_Enable(gpio_card2_power_enable);
                GPIO_SetMode(gpio_card2_power_enable, GPIO_MODE_OUTPUT);
                GPIO_SetState(gpio_card2_power_enable, GPIO_STATE_LO);      // power up
            }
#endif
            break;

        case MMP_GPIO_CARD2_DETECT:
            gpio_card2_detect = *attribList++;
            break;

        case MMP_GPIO_CARD2_WP:
            gpio_sd_card2_write_protect = *attribList++;
            break;
            //case MMP_GPIO_2SD_SWITCH:
            //    gpio_2sd_switch = *attribList++;
            //    break;
            //case MMP_GPIO_IR:
            //    gpio_ir = *attribList++;
            //    break;
            //case MMP_GPIO_SPDIF:
            //    gpio_spdif = *attribList++;
            //    break;
            //case MMP_GPIO_KEYPAD_IN:
            //    gpio_keypad_in = *attribList++;
            //    break;
            //case MMP_GPIO_FPC_CTL:
            //    gpio_fpc_ctl = *attribList++;
            //    break;
            //case MMP_GPIO_FPC_CLK:
            //    gpio_fpc_clk = *attribList++;
            //    break;
            //case MMP_GPIO_FPC_DAT:
            //    gpio_fpc_dat = *attribList++;
            //    break;
            //case MMP_GPIO_IIC:
            //    gpio_iic = *attribList++;
            //    break;
            //case MMP_GPIO_UART:
            //    gpio_uart = *attribList++;
            //    break;
        }
    }
}