#include <stdio.h>
#include "pal/pal.h"
#include "ssp/ssp_reg.h"
#include "nor.h"
#include "mmp_nor.h"
#include "mem/mem.h"
#include "host/host.h"
#include "host/ahb.h"
#include "host/gpio.h"
#include "sys/sys.h"
#include "mmp_spi.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define NOR_BUS            SPI_1
#define INVALID_SIZE       (-1)
#define SSP_POLLING_COUNT  (0x100000)
#define WRITE_DATA_COUNTS  8
#define WRITE_DATA_BYTES   28           //according to fifo length
#ifdef _WIN32
    #define PER_READ_BYTES 28           //14*2 (fifo size - input size)
#else
    #define PER_READ_BYTES 60           //15*4 (fifo size - input size)
#endif
#define PER_DMA_BYTES      2048

#ifdef __FREERTOS__
    #define WriteReg       AHB_WriteRegister
    #define ReadReg        AHB_ReadRegister
#else
    #define WriteReg       HOST_WriteRegister
    #define ReadReg        HOST_ReadRegister
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    MMP_UINT16 sectorNum;
    MMP_UINT32 startAddr;
    MMP_UINT32 endAddr;
} NOR_ADDR_MAP;

typedef struct
{
    MMP_UINT16 bytesPerPage;
    MMP_UINT16 pagesPerSector;
    MMP_INT32  bytesPerSector;
    MMP_UINT16 sectorsPerBlock;
    MMP_UINT16 totalBlocks;
} NOR_ADDR_CONTEXT;

typedef enum
{
    AMIC_A25L032,
    ATMEL_AT26DF161,
    ATMEL_AT26D321,
    EON_EN25P32,
    EON_EN25B16,
    EON_EN25B32,
    EON_EN25B64,
    EON_EN25F16,
    EON_EN25F32,
    EON_EN25Q16,
    EON_EN25Q32A,
    EON_EN25F80,
    EON_EN25Q80A,
    EON_EN25Q64,
    EON_EN25QH32,
    ES_ES25M16A,
    ESMT_F25L16A,
    ESMT_F25L32QA,
    MX_25L1605A,
    MX_25L3205D,
    MX_25L3235D,
    MX_25L1635D,
    MX_25L6445E,
    MX_25L3206E,
    NUMON_M25P20,
    NUMON_M25P32,
    SPAN_S25FL016A,
    SPAN_S25FL032A,
    SST_25VF016B,
    WIN_W25X16A,
    WIN_W25X32V,
    WIN_W25Q32BV,
    WIN_W25Q64BV,
    GD_GD25Q32,
    ESMT_F25L32PA,
    PMC_PM25LQ032C,
    UNKNOW_VENDOR = 0xFFFF
} NOR_VENDOR_ID;

typedef struct
{
    MMP_UINT8     manufatureID;
    MMP_UINT16    deviceID;
    MMP_UINT8     deviceID2;
    MMP_UINT8     *deviceName;
    NOR_VENDOR_ID vendorID;
} NOR_VENDOR_CONTEXT;

typedef enum
{
    BIG_ENDIAN,
    LITTLE_ENDIAN
} ENDIAN_TYPE;

//=============================================================================
//                              Global Data Definition
//=============================================================================
SPI_CONTEXT        g_SpiContext;
NOR_ADDR_CONTEXT   g_norAddrMap;
MMP_UINT16         g_initCount;
NOR_VENDOR_ID      g_vendor             = UNKNOW_VENDOR;
static ENDIAN_TYPE g_endian;
MMP_MUTEX          gNorMutex            = MMP_NULL;

NOR_VENDOR_CONTEXT nor_support_vendor[] = {
    {0x37, 0x3016, 0x15, "AMIC__A25L032", AMIC_A25L032},
    {0x1F, 0x4600, 0xFF, "ATMEL_AT26DF161", ATMEL_AT26DF161},
    {0x1F, 0x4700, 0xFF, "ATMEL__AT26D321", ATMEL_AT26D321},
    {0x1C, 0x2016, 0x15, "EON__EN25P32", EON_EN25P32},
    {0x1C, 0x2015, 0x34, "EON__EN25B16", EON_EN25B16},
    {0x1C, 0x2016, 0x35, "EON__EN25B32", EON_EN25B32},
    {0x1C, 0x2017, 0x36, "EON__EN25B64", EON_EN25B64},
    {0x1C, 0x3115, 0x14, "EON__EN25F16", EON_EN25F16},
    {0x1C, 0x3116, 0x15, "EON__EN25F32", EON_EN25F32},
    {0x1C, 0x3015, 0x14, "EON_EN25Q16", EON_EN25Q16},
    {0x1C, 0x3016, 0x15, "EON_EN25Q32A", EON_EN25Q32A},
    {0x1C, 0x3114, 0x13, "EON_EN25F80", EON_EN25F80},
    {0x1C, 0x3014, 0x13, "EON_EN25Q80A", EON_EN25Q80A},
    {0x1C, 0x3017, 0x16, "EON_EN25Q64",  EON_EN25Q64},
    {0x1C, 0x7016, 0x15, "EON_EN25QH32", EON_EN25QH32},
    {0x4A, 0x3215, 0x14, "ES__ES25M16A", ES_ES25M16A},
    {0x8C, 0x2015, 0x14, "ESMT_F25L16PA", ESMT_F25L16A},
    {0x8C, 0x4116, 0x15, "ESMT_F25L32QA", ESMT_F25L32QA},
    {0xC2, 0x2015, 0x14, "MX__25L1605A", MX_25L1605A},
    {0xC2, 0x2016, 0x15, "MX__25L3205D", MX_25L3205D},
    {0xC2, 0x5E16, 0x5E, "MX__25L3235D", MX_25L3235D},
    {0xC2, 0x2415, 0x24, "MX__25L1635D", MX_25L1635D},
    {0xC2, 0x2017, 0x16, "MX_25L6445E", MX_25L6445E},
    {0xC2, 0x2016, 0x15, "MX_25L3206E", MX_25L3206E},
    {0x20, 0x2012, 0xFF, "NUMON_M25P20", NUMON_M25P20},
    {0x20, 0x2016, 0xFF, "NUMON_M25P32", NUMON_M25P32},
    {0x01, 0x0214, 0xFF, "SPAN_S25FL016A", SPAN_S25FL016A},
    {0x01, 0x0215, 0xFF, "SPAN_S25FL032A", SPAN_S25FL032A},
    {0xBF, 0x2541, 0x41, "SST_25VF016B", SST_25VF016B},
    {0xEF, 0x3015, 0x14, "WIN__W25X16A", WIN_W25X16A},
    {0xEF, 0x3016, 0x15, "WIN_W25X32V", WIN_W25X32V},
    {0xEF, 0x4016, 0x15, "WIN_W25Q32BV", WIN_W25Q32BV},
    {0xC8, 0x4016, 0x15, "GD_GD25Q32", GD_GD25Q32},
    {0x8C, 0x2016, 0x15, "ESMT_F25L32PA",    ESMT_F25L32PA},
    {0x7F, 0x9D46, 0x15, "PMC_PM25LQ032C", PMC_PM25LQ032C},
    {0xEF, 0x4017, 0x16, "WIN__W25Q64BV", WIN_W25Q64BV}
};

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_NorLock()
{
    if (gNorMutex)
    {
        PalWaitMutex(gNorMutex, PAL_MUTEX_INFINITE);
    }
}

static void
_NorUnlock()
{
    if (gNorMutex)
    {
        PalReleaseMutex(gNorMutex);
    }
}

static MMP_INT
NorSendCommand(
    FLASH_CMD_TYPE cmdType,
    MMP_UINT       arg,
    MMP_UINT       data,
    MMP_UINT       size)
{
    MMP_INT   result;
    MMP_UINT8 cmd;
    MMP_UINT8 inputData[4];
    switch (cmdType)
    {
    case ERASE_ALL:
        //C7h/ 60h
        cmd          = ERASE_BULK_CMD;
        inputData[0] = cmd;
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;

    case WRITE_EN:
        //06h
        cmd          = WRITE_EN_CMD;
        inputData[0] = cmd;
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;

    case WRITE_DIS:
        //04h
        cmd          = WRITE_DIS_CMD;
        inputData[0] = cmd;
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;

    case WRITE_STATUS_EN:
        cmd          = WRITE_STATUS_EN_CMD;
        inputData[0] = cmd;
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 1, 0, 0, 8);
        break;

    case WRITE_STATUS:
        //01h S7-S0
        cmd          = WRITE_STATUS_CMD;
        inputData[0] = cmd;
        inputData[1] = arg;
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 2, 0, 0, 16);
        break;

    case ERASE_SECTOR:
        //20h A23-A16 A15-A8 A7-A0
        cmd          = ERASE_SECOTR_CMD;
        inputData[0] = cmd;
        inputData[1] = arg >> 16;
        inputData[2] = arg >> 8;
        inputData[3] = arg;
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 16);
#else
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 32);
#endif
        break;

    case ERASE_BLOCK:
        //D8h/ 52h A23-A16 A15-A8 A7-A0
        cmd          = ERASE_BLOCK_CMD;
        inputData[0] = cmd;
        inputData[1] = (MMP_UINT8)(arg >> 16);
        inputData[2] = (MMP_UINT8)(arg >> 8);
        inputData[3] = (MMP_UINT8)arg;
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 16);
#else
        result       = mmpSpiPioWrite(NOR_BUS, inputData, 4, 0, 0, 32);
#endif
        break;

    case AUTO_WRITE:
        break;

    case AUTO_WRITE_DATA_ONLY:
        break;

    case WRITE:
        //02h A23-A16 A15-A8 A7-A0 D7-D0 (Next byte) continuous
        cmd          = PAGE_PROGRAM_CMD;
        inputData[0] = cmd;
        inputData[1] = (MMP_UINT8)(arg >> 16);
        inputData[2] = (MMP_UINT8)(arg >> 8);
        inputData[3] = (MMP_UINT8)arg;
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (size >= 32 /*g_norAddrMap.bytesPerPage*/ || (size & 1))
            result = mmpSpiDmaWrite(NOR_BUS, inputData, 4, (void *)data, size, 8);
        else
            result = mmpSpiPioWrite(NOR_BUS, inputData, 4, (void *)data, size, 16);
        //result = mmpSpiPioWrite(NOR_BUS, inputData, 4, (void*)data, size, 8);
#else
        result = mmpSpiPioWrite(NOR_BUS, inputData, 4, (void *)data, size, 32);
#endif
        break;

    case READ:
        //03h A23-A16 A15-A8 A7-A0 (D7-D0) (Next byte) continuous
        cmd          = READ_CMD;
        inputData[0] = cmd;
        inputData[1] = arg >> 16;
        inputData[2] = arg >> 8;
        inputData[3] = arg;
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        if (size >= 16 || (size & 1))
        {
            result = mmpSpiDmaRead(NOR_BUS, inputData, 4, (void *)data, size, 8);
        }
        else
        {
            result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void *)data, size, 16);
        }
#else
        result = mmpSpiPioRead(NOR_BUS, inputData, 4, (void *)data, size, 32);
#endif
        break;

    case FAST_READ:
        //0Bh A23-A16 A15-A8 A7-A0 dummy (D7-D0) (Next Byte) continuous
        //TODO
        cmd          = FAST_READ_CMD;
        inputData[0] = cmd;
        inputData[1] = arg >> 16;
        inputData[2] = arg >> 8;
        inputData[3] = arg;
#if defined(_WIN32) || (defined(__OPENRTOS__) || defined(__FREERTOS__))
        result       = mmpSpiPioRead(NOR_BUS, inputData, 4, (void *)data, size, 16);
#else
        result       = mmpSpiPioRead(NOR_BUS, inputData, 4, (void *)data, size, 32);
#endif
        break;

    case READ_STATUS:
        //05h (S7-S0)(1)
        cmd          = READ_STATUS_CMD;
        inputData[0] = cmd;
        mmpSpiPioRead(NOR_BUS, inputData, 1, (void *)data, 1, 8);
        break;

    case READ_ID:
        //9Fh (M7-M0) (ID15-ID8) (ID7-ID0)
        cmd          = JEDEC_READ_ID;
        inputData[0] = cmd;
        result       = mmpSpiPioRead(NOR_BUS, inputData, 1, (void *)data, 5, 8);
        break;

    case READ_DEVICE:
        //90h dummy dummy 00h (M7-M0) (ID7-ID0)
        cmd          = READ_ID_CMD;
        inputData[0] = cmd;
        inputData[1] = arg >> 16;
        inputData[2] = arg >> 8;
        inputData[3] = arg;
        result       = mmpSpiPioRead(NOR_BUS, inputData, 4, (void *)data, 2, 8);
        break;

    default:
        break;
    }

    return 0;
}

static MMP_UINT
NorCheckReady(
    void)
{
    MMP_UINT32 i;
    MMP_UINT   state = NOR_ERROR_STATUS_BUSY;
    MMP_UINT8  temp;

    i = SSP_POLLING_COUNT;
    while (--i)
    {
        NorSendCommand(READ_STATUS, 0, (MMP_UINT)&temp, 1);
        if (temp & NOR_DEVICE_BUSY)
        {
            continue;
        }
        else
        {
            state = NOR_DEVICE_READY;
            goto done;
        }
    }

    state = NOR_ERROR_DEVICE_TIMEOUT;

done:
    return state;
}

static MMP_UINT
NorCheckReadyEx(
    void)
{
    MMP_UINT32 i;
    MMP_UINT   state = NOR_ERROR_STATUS_BUSY;
    MMP_UINT8  temp;

    i = SSP_POLLING_COUNT;
    while (--i)
    {
        NorSendCommand(READ_STATUS, 0, (MMP_UINT)&temp, 1);
        if (temp & NOR_DEVICE_BUSY)
        {
            MMP_Sleep(100);
            continue;
        }
        else
        {
            state = NOR_DEVICE_READY;
            goto done;
        }
    }

    state = NOR_ERROR_DEVICE_TIMEOUT;

done:
    return state;
}

static NOR_VENDOR_ID
Nor_ReadID(
    void)
{
    MMP_UINT16    i;
    MMP_UINT8     manufatureID = 0;
    MMP_UINT16    deviceID     = 0;
    MMP_UINT8     deviceID2    = 0;
    MMP_UINT8     data[8];
    NOR_VENDOR_ID vendorID     = UNKNOW_VENDOR;

    NorSendCommand(READ_ID, 0, (MMP_UINT)data, 0);
    manufatureID = (MMP_UINT8)(data[0]);
    deviceID     = (MMP_UINT16)((data[1] << 8) | data[2]);
    if (manufatureID == 0x1F)   // ATMEL manufacture ID
    {
        deviceID2 = 0xFF;
    }
    else if (manufatureID == 0x01) // SPANSION manufacture ID
    {
        deviceID2 = 0xFF;
    }
    else if (manufatureID == 0x20) // Numonyx manufacture ID
    {
        deviceID2 = 0xFF;
    }
    else
    {
        NorSendCommand(READ_DEVICE, 0, (MMP_UINT)data, 2);
        deviceID2 = (MMP_UINT8)(data[1]);
    }

    printf("manufatureID = 0x%X, deviceID = 0x%X, deviceID2 = 0x%X\n", manufatureID, deviceID, deviceID2);

    for (i = 0; i < (sizeof(nor_support_vendor) / sizeof(nor_support_vendor[0])); ++i)
    {
        if (manufatureID == nor_support_vendor[i].manufatureID)
        {
            if (deviceID == nor_support_vendor[i].deviceID)
            {
                if (deviceID2 == nor_support_vendor[i].deviceID2)
                {
                    vendorID = nor_support_vendor[i].vendorID;
                    printf("[NOR][INFO] DEVICE IS %s \n", nor_support_vendor[i].deviceName);
                    break;
                }
            }
        }
    }

    if (vendorID == UNKNOW_VENDOR)
    {
        printf("[NOR][ERROR] DEVICE UNKNOW  \n");
    }

    return vendorID;
}

static MMP_RESULT
InitNorFlash(
    void)
{
    MMP_RESULT result = 0;

    g_vendor = Nor_ReadID();
    if (g_vendor == UNKNOW_VENDOR)
    {
        result = 1;
        goto end;
    }

    switch (g_vendor)
    {
    case NUMON_M25P20:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 4;
        break;

    case ESMT_F25L16A:
    case ESMT_F25L32QA:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 32;
        break;

    case EON_EN25P32:
    case NUMON_M25P32:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 64;
        break;

    case EON_EN25B16:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 32;      //31x64k + 1x32k + 1x16k + 1x8k + 2x4k
        break;

    case MX_25L1605A:
    case MX_25L1635D:
    case WIN_W25X16A:
    case ATMEL_AT26DF161:
    case SPAN_S25FL016A:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 32;
        break;

    case EON_EN25F16:
    case EON_EN25Q16:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 32;      //31x64k + 1x32k + 1x16k + 1x8k + 2x4k
        break;

    case EON_EN25B32:
    case EON_EN25F32:
    case EON_EN25Q32A:
    case EON_EN25QH32:
    case MX_25L3235D:
    case MX_25L3205D:
    case SPAN_S25FL032A:
    case WIN_W25X32V:
    case WIN_W25Q32BV:
    case GD_GD25Q32:
    case PMC_PM25LQ032C:
    case ESMT_F25L32PA:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 64;
        break;

    case SST_25VF016B:
        g_norAddrMap.bytesPerPage    = 512;
        g_norAddrMap.pagesPerSector  = 128;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 32;
        break;

    case AMIC_A25L032:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 64;
        break;

    case ATMEL_AT26D321:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 64;
        break;

    case ES_ES25M16A:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 32;
        break;

    case EON_EN25B64:
    case EON_EN25Q64:
    case WIN_W25Q64BV:
    case MX_25L6445E:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 128;
        break;

    case EON_EN25F80:
    case EON_EN25Q80A:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 256;
        g_norAddrMap.bytesPerSector  = 64 * 1024;
        g_norAddrMap.sectorsPerBlock = 1;
        g_norAddrMap.totalBlocks     = 16;
        break;

    case MX_25L3206E:
        g_norAddrMap.bytesPerPage    = 256;
        g_norAddrMap.pagesPerSector  = 16;
        g_norAddrMap.bytesPerSector  = 4 * 1024;
        g_norAddrMap.sectorsPerBlock = 16;
        g_norAddrMap.totalBlocks     = 64;
        break;

    default:
        break;
    }

end:
    return result;
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
MMP_RESULT
FTSPI_Read(
    void     *pdes,
    MMP_UINT addr,
    MMP_INT  size)
{
    MMP_UINT8  *pbuf   = (MMP_UINT8 *)(pdes);
    MMP_RESULT result  = 0;
    MMP_UINT   perSize = 0;

#if defined(DTV_SD1_ENABLE) && !defined(DTV_SET_SD1_INTERNAL)
    //set GPIO to Mode1
    ithGpioSetMode(9, ITH_GPIO_MODE1);
    ithGpioSetMode(10, ITH_GPIO_MODE1);
    ithGpioSetMode(11, ITH_GPIO_MODE1);
#endif

    while (size > 0)
    {
#if 0
        perSize = (size > PER_READ_BYTES) ? (PER_READ_BYTES) : size;
#endif
        perSize = (size > PER_DMA_BYTES) ? (PER_DMA_BYTES) : size;
        NorSendCommand(READ, addr, (MMP_UINT)pbuf, perSize);
        size   -= perSize;
        addr   += perSize;
        pbuf   += perSize;
    }

    return result;
}

MMP_RESULT
FTSPI_Write(
    void     *psrc,
    MMP_UINT addr,
    MMP_INT  size)
{
    MMP_INT32  i           = 0;

    MMP_UINT8  temp;
    MMP_UINT8  *pbuf       = (MMP_UINT8 *)(psrc);
    MMP_UINT16 data        = 0;
    MMP_RESULT result      = 0;

    MMP_INT32  numOfSector = (size / g_norAddrMap.bytesPerSector);
    MMP_INT    wOffset     = size % (g_norAddrMap.bytesPerSector);
    MMP_INT    wSize       = (size < g_norAddrMap.bytesPerSector) ? (size) : (g_norAddrMap.bytesPerSector);
    MMP_INT    addrOffset  = addr % (g_norAddrMap.bytesPerSector);
    MMP_UINT   addrTemp    = addr;
    MMP_UINT   perSize;

#if defined(DTV_SD1_ENABLE) && !defined(DTV_SET_SD1_INTERNAL)
    //set GPIO to Mode1
    ithGpioSetMode(9, ITH_GPIO_MODE1);
    ithGpioSetMode(10, ITH_GPIO_MODE1);
    ithGpioSetMode(11, ITH_GPIO_MODE1);
#endif

    if (wOffset)
    {
        numOfSector++;
    }

    // Unlock write protected
    NorSendCommand(WRITE_EN, 0, 0, 0);
    NorSendCommand(WRITE_STATUS, 0, 0, 0);
    result = NorCheckReady();
    if (result)
    {
        goto done;
    }

    NorSendCommand(READ_STATUS, 0, (MMP_UINT)&temp, 1);
    if ((temp & NOR_WRITE_PROTECT) != 0)
    {
        result = NOR_ERROR_STATUS_PROTECT;
        printf("[ERROR] NOR_STATUS_PROTECT \n");
        goto done;
    }

    // Erase sectors
    for (i = 0; i < numOfSector; ++i)
    {
        //EON EN25B16 sector 0 devide 5 partial sectors
        if ((g_vendor == EON_EN25B16 || g_vendor == EON_EN25B32 || g_vendor == EON_EN25B64) && (addr == 0))
        {
            MMP_INT32 j;
            MMP_INT32 bytesPerSector = 0;

            for (j = 0; j < 5; ++j)
            {
                switch (j)
                {
                case 0:
                    bytesPerSector = 0x1000;
                    break;

                case 1:
                    bytesPerSector = 0x1000;
                    break;

                case 2:
                    bytesPerSector = 0x2000;
                    break;

                case 3:
                    bytesPerSector = 0x4000;
                    break;

                case 4:
                    bytesPerSector = 0x8000;
                    break;
                }

                // Write enable
                NorSendCommand(WRITE_EN, 0, 0, 0);
                // Erase
                NorSendCommand(ERASE_BLOCK, addrTemp, 0, 0);

                // Wait erase finished
                result = NorCheckReadyEx();
                if (result)
                {
                    goto done;
                }
                addrTemp += bytesPerSector;
            }
        }
        else
        {
            // Write enable
            NorSendCommand(WRITE_EN, 0, 0, 0);
            // Erase
            NorSendCommand(ERASE_BLOCK, addrTemp, 0, 0);

            // Wait erase finished
            result = NorCheckReadyEx();
            if (result)
            {
                goto done;
            }
            addrTemp += g_norAddrMap.bytesPerSector;
        }
    }

    // Write state
    {
        MMP_UINT bufAddr = (MMP_UINT)pbuf;
        addrOffset = addr % (g_norAddrMap.bytesPerPage);
        addrTemp   = addr;
        while (size)
        {
            if (addrOffset)
            {
                wSize      = g_norAddrMap.bytesPerPage - addrOffset;
                if (wSize > size)
                    wSize = size;
                addrOffset = 0;
            }
            else
            {
                wSize = (size < g_norAddrMap.bytesPerPage) ? (size) : (g_norAddrMap.bytesPerPage);
            }
            //test only
            //if(wSize == g_norAddrMap.bytesPerPage)
            if (wSize >= 32)
            {
                //write enable
                NorSendCommand(WRITE_EN, 0, 0, 0);
                NorSendCommand(WRITE, addrTemp, bufAddr, wSize);
                result = NorCheckReady();
                if (result)
                {
                    goto done;
                }
                bufAddr  += wSize;
                addrTemp += wSize;
                size     -= wSize;
            }
            else
            {
                for (i = 0; i < wSize; i += WRITE_DATA_BYTES)
                {
                    if ((wSize - i) >= WRITE_DATA_BYTES)
                    {
                        perSize = WRITE_DATA_BYTES;
                    }
                    else
                    {
                        perSize = wSize - i;
                    }

                    // Write enable
                    NorSendCommand(WRITE_EN, 0, 0, 0);
                    //32 bytes each wrtie cmd
                    NorSendCommand(WRITE, addrTemp, bufAddr, perSize);

                    bufAddr  += perSize;
                    addrTemp += perSize;

                    result    = NorCheckReady();
                    if (result)
                    {
                        goto done;
                    }
                }
                size -= wSize;
            }
        }
    }

done:
    // Write disable
    NorSendCommand(WRITE_DIS, 0, 0, 0);

    if (result == 0)
    {
        result = NorCheckReady();
    }

    if (result)
        printf("[ERROR] result = %x \n", result);

    return result;
}

MMP_RESULT
FTSPI_WriteWithoutErase(
    void     *psrc,
    MMP_UINT addr,
    MMP_INT  size)
{
    MMP_INT32  i      = 0;
    MMP_UINT8  temp   = 0;
    MMP_UINT16 *pbuf  = (MMP_UINT16 *)(psrc);
    MMP_UINT16 data   = 0;
    MMP_RESULT result = 0;
    MMP_UINT   perSize;

    //TODO
    MMP_INT32  numOfSector = (size / g_norAddrMap.bytesPerSector);
    MMP_INT    wOffset     = size % (g_norAddrMap.bytesPerSector);
    MMP_INT    wSize       = (size < g_norAddrMap.bytesPerSector) ? (size) : (g_norAddrMap.bytesPerSector);
    MMP_INT    addrOffset  = addr % (g_norAddrMap.bytesPerSector);
    MMP_UINT   addrTemp    = addr;

    if (wOffset)
    {
        numOfSector++;
    }

    // Unlock write protected
    NorSendCommand(WRITE_EN, 0, 0, 0);
    NorSendCommand(WRITE_STATUS, 0, 0, 0);
    result = NorCheckReady();
    if (result)
    {
        goto done;
    }

    NorSendCommand(READ_STATUS, 0, (MMP_UINT)&temp, 1);
    if ((temp & NOR_WRITE_PROTECT) != 0)
    {
        result = NOR_ERROR_STATUS_PROTECT;
        printf("[ERROR] NOR_STATUS_PROTECT \n");
        goto done;
    }

    // Write state
    {
        MMP_UINT bufAddr = (MMP_UINT)pbuf;
        addrOffset = addr % (g_norAddrMap.bytesPerPage);
        addrTemp   = addr;
        while (size)
        {
            if (addrOffset)
            {
                wSize      = g_norAddrMap.bytesPerPage - addrOffset;
                if (wSize > size)
                    wSize = size;
                addrOffset = 0;
            }
            else
            {
                wSize = (size < g_norAddrMap.bytesPerPage) ? (size) : (g_norAddrMap.bytesPerPage);
            }

            //test only
            //if(wSize == g_norAddrMap.bytesPerPage)
            if (wSize >= 32)
            {
                //write enable
                NorSendCommand(WRITE_EN, 0, 0, 0);
                NorSendCommand(WRITE, addrTemp, bufAddr, wSize);
                result = NorCheckReady();
                if (result)
                {
                    goto done;
                }
                bufAddr  += wSize;
                addrTemp += wSize;
                size     -= wSize;
            }
            else
            {
                for (i = 0; i < wSize; i += WRITE_DATA_BYTES)
                {
                    if ((wSize - i) >= WRITE_DATA_BYTES)
                    {
                        perSize = WRITE_DATA_BYTES;
                    }
                    else
                    {
                        perSize = wSize - i;
                    }

                    // Write enable
                    NorSendCommand(WRITE_EN, 0, 0, 0);
                    //32 bytes each wrtie cmd
                    NorSendCommand(WRITE, addrTemp, bufAddr, perSize);

                    bufAddr  += perSize;
                    addrTemp += perSize;

                    result    = NorCheckReady();
                    if (result)
                    {
                        goto done;
                    }
                }
                size -= wSize;
            }
        }
    }

done:
    // Write disable
    NorSendCommand(WRITE_DIS, 0, 0, 0);

    if (result == 0)
    {
        result = NorCheckReady();
    }
    if (result)
        printf("[ERROR] result = %x \n", result);

    return result;
}

MMP_RESULT
NorBulkErase(
    void)
{
    MMP_RESULT result = 0;
    MMP_UINT32 j;
    MMP_UINT8  temp;
    _NorLock();
    // Unlock write protected
    NorSendCommand(WRITE_EN, 0, 0, 0);

    // Erase
    NorSendCommand(ERASE_ALL, 0, 0, 0);

    // Wait erase finished
    j = 0x800000;
    do
    {
        NorSendCommand(READ_STATUS, 0, (MMP_UINT)&temp, 1);
    } while ((temp & NOR_DEVICE_BUSY) && (--j));

    if (j == 0)
        return NOR_ERROR_DEVICE_TIMEOUT;

    // Write disable
    NorSendCommand(WRITE_DIS, 0, 0, 0);
    _NorUnlock();
    return result;
}

MMP_RESULT
NorInitial(
    void)
{
    MMP_RESULT result = 0;

    union
    {
        MMP_UINT  dwData;
        MMP_UINT8 byteData[4];
    } pat;

    if (g_initCount == 0)
    {
        pat.dwData = 0x12345678;
        if (pat.byteData[0] != (pat.dwData & 0xFF))
        {
            g_endian = BIG_ENDIAN;
        }
        else
        {
            g_endian = LITTLE_ENDIAN;
        }

        result = mmpSpiInitialize(NOR_BUS);
        if (result == 0)
        {
            result = InitNorFlash();
        }
    }

    g_initCount++;
    if (gNorMutex)
    {
        PalDestroyMutex(gNorMutex);
    }
    gNorMutex = PalCreateMutex(MMP_NULL);
    return result;
}

MMP_RESULT
NorTerminate(
    void)
{
    MMP_RESULT result = 0;

    if (--g_initCount == 0)
    {
        mmpSpiTerminate(NOR_BUS);
    }
    if (gNorMutex)
    {
        PalDestroyMutex(gNorMutex);
        gNorMutex = MMP_NULL;
    }
    return result;
}

MMP_RESULT
NorRead(
    void     *pdes,
    MMP_UINT addr,
    MMP_INT  size)
{
    MMP_RESULT result;
    MMP_UINT8  *norTempBuf = MMP_NULL;
    MMP_UINT32 des         = (MMP_UINT32)pdes;
    _NorLock();
    if (des & 0x3)
    {
        norTempBuf = (MMP_UINT8 *)MEM_Allocate(size, MEM_USER_NOR);
        if (!norTempBuf)
        {
            result = -1;
            goto end;
        }

        result = FTSPI_Read(norTempBuf, addr, size);
        if (!result)
            memcpy(pdes, norTempBuf, size);

        MEM_Release(norTempBuf);
    }
    else
    {
        result = FTSPI_Read(pdes, addr, size);
    }

end:
    _NorUnlock();
    return result;
}

MMP_RESULT
NorWrite(
    void     *psrc,
    MMP_UINT addr,
    MMP_INT  size)
{
    MMP_RESULT result;
    _NorLock();
    result = FTSPI_Write(psrc, addr, size);
    _NorUnlock();
    return result;
}

MMP_RESULT
NorWriteWithoutErase(
    void     *psrc,
    MMP_UINT addr,
    MMP_INT  size)
{
    MMP_RESULT result;
    _NorLock();
    result = FTSPI_WriteWithoutErase(psrc, addr, size);
    _NorUnlock();
    return result;
}

MMP_UINT32
NorCapacity(
    void)
{
    return (MMP_UINT32)(g_norAddrMap.bytesPerSector * g_norAddrMap.sectorsPerBlock \
                        * g_norAddrMap.totalBlocks);
}

MMP_UINT32
NorGetAttitude(
    NOR_ATTITUDE atti)
{
    MMP_UINT32 data = 0;

    switch (atti)
    {
    case NOR_ATTITUDE_ERASE_UNIT:
        data = (MMP_UINT32)g_norAddrMap.bytesPerSector;
        break;

    case NOR_ATTITUDE_DEVICE_COUNT:
        data = sizeof(nor_support_vendor) / sizeof(nor_support_vendor[0]);
        break;

    case NOR_ATTITUDE_CURRENT_DEVICE_ID:
        data = g_vendor;
        break;

    case NOR_ATTITUDE_PAGE_SIZE:
        data = (MMP_UINT32)g_norAddrMap.bytesPerPage;
        break;

    case NOR_ATTITUDE_PAGE_PER_SECTOR:
        data = (MMP_UINT32)g_norAddrMap.pagesPerSector;
        break;

    case NOR_ATTITUDE_SECTOR_PER_BLOCK:
        data = (MMP_UINT32)g_norAddrMap.sectorsPerBlock;
        break;

    case NOR_ATTITUDE_BLOCK_SIZE:
        data = (MMP_UINT32)g_norAddrMap.totalBlocks;
        break;

    case NOR_ATTITUDE_TOTAL_SIZE:
        data = (MMP_UINT32)(g_norAddrMap.totalBlocks * g_norAddrMap.sectorsPerBlock * g_norAddrMap.bytesPerSector);
        break;

    default:
        break;
    }

    return data;
}

MMP_UINT8 *
NorGetDeviceName(
    MMP_UINT8 num)
{
    MMP_UINT8 *ptr = MMP_NULL;
    MMP_UINT8 i;

    for (i = 0; i < sizeof(nor_support_vendor) / sizeof(nor_support_vendor[0]); ++i)
    {
        if (num == nor_support_vendor[i].vendorID)
        {
            ptr = nor_support_vendor[i].deviceName;
            break;
        }
    }

    return ptr;
}

void
NorGetBuildInfo(
    MMP_UINT8 *version,
    MMP_UINT8 *date)
{
    MMP_UINT8 *ptr;
    MMP_UINT8 cnt = 0;

    *version = BUILD_VERSION;
    ptr      = __DATE__;
    cnt      = strlen(ptr);
    memcpy(date, ptr, cnt);
}