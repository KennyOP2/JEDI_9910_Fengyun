#include <stdio.h>
#include "mmp.h"
#include "host/host.h"
//#include "host/host_hw.h"
#include "mem/mem.h"
#include "mmp_types.h"
//#include "config/config.h"
#include "pal/pal.h"
#include "mmc_smedia.h"
#include "common/port_f.h"

/** card driver **/
#include "mmp_dma.h"
#include "mmp_sd.h"
#ifdef DTV_MS_ENABLE
#include "mmp_mspro.h"
#endif

#ifdef DTV_USB_ENABLE
    #include "mmp_usbex.h"
    #include "mmp_msc.h"
#endif

#ifdef DTV_CF_ENABLE
    #include "mmp_cf.h"
#endif
#ifdef DTV_xD_ENABLE
    #include "xd/xd_mlayer.h"
    #include "xd/xd.h"
#endif


MMP_UINT32 GetHostBaseAddr(void);

#ifdef DTV_SD1_ENABLE
    static t_mmc_dsc sd_dsc;        /* current SD card descriptor */
    static F_DRIVER sd_drv;
    static unsigned long sd_number_of_sectors = 0;
    
    static unsigned long sd_sector_size = 512;
#endif

#ifdef DTV_SD2_ENABLE
    static t_mmc_dsc sd2_dsc;        /* current SD2 card descriptor */
    static F_DRIVER sd2_drv;
    static unsigned long sd2_number_of_sectors = 0;
    
    static unsigned long sd2_sector_size = 512;
#endif

#ifdef DTV_MMC_ENABLE
    static t_mmc_dsc mmc_dsc;       /* current MMC card descriptor */
    static F_DRIVER mmc_drv;
    static unsigned long mmc_number_of_sectors = 0;
    
    static unsigned long mmc_sector_size = 512;
#endif

#ifdef DTV_MS_ENABLE
    static t_mmc_dsc ms_dsc;        /* current MS card descriptor */
    static F_DRIVER ms_drv;
    static unsigned long ms_number_of_sectors = 0;
    
    static unsigned long ms_sector_size = 512;
#endif

#ifdef DTV_USB_ENABLE
    static t_mmc_dsc usb0_dsc;      /* current USB0 card descriptor */
    static F_DRIVER usb0_drv;
    static unsigned long usb0_number_of_sectors = 0;
    
    static t_mmc_dsc usb1_dsc;      /* current USB1 card descriptor */
    static F_DRIVER usb1_drv;
    static unsigned long usb1_number_of_sectors = 0;
    
    static t_mmc_dsc usb2_dsc;      /* current USB2 card descriptor */
    static F_DRIVER usb2_drv;
    static unsigned long usb2_number_of_sectors = 0;
    
    static t_mmc_dsc usb3_dsc;      /* current USB3 card descriptor */
    static F_DRIVER usb3_drv;
    static unsigned long usb3_number_of_sectors = 0;
    
    static t_mmc_dsc usb4_dsc;      /* current USB4 card descriptor */
    static F_DRIVER usb4_drv;
    static unsigned long usb4_number_of_sectors = 0;
    
    static t_mmc_dsc usb5_dsc;      /* current USB5 card descriptor */
    static F_DRIVER usb5_drv;
    static unsigned long usb5_number_of_sectors = 0;
    
    static t_mmc_dsc usb6_dsc;      /* current USB6 card descriptor */
    static F_DRIVER usb6_drv;
    static unsigned long usb6_number_of_sectors = 0;
    
    static t_mmc_dsc usb7_dsc;      /* current USB7 card descriptor */
    static F_DRIVER usb7_drv;
    static unsigned long usb7_number_of_sectors = 0;
    
    static unsigned long usb0_sector_size = 512;
    static unsigned long usb1_sector_size = 512;
    static unsigned long usb2_sector_size = 512;
    static unsigned long usb3_sector_size = 512;
    static unsigned long usb4_sector_size = 512;
    static unsigned long usb5_sector_size = 512;
    static unsigned long usb6_sector_size = 512;
    static unsigned long usb7_sector_size = 512;    
    
    extern USB_DEVICE_INFO usb_device[2]; /** 0: For Host+Device, 1: For Host only */
#endif

#ifdef DTV_CF_ENABLE
    static t_mmc_dsc cf_dsc;        /* current CF card descriptor */
    static F_DRIVER cf_drv;
    static unsigned long cf_number_of_sectors = 0;
    
    static unsigned long cf_sector_size = 512;
#endif

#ifdef DTV_xD_ENABLE
    static t_mmc_dsc xd_dsc;        /* current xD card descriptor */
    static F_DRIVER xd_drv;
    static unsigned long xd_number_of_sectors = 0;
    
    static unsigned long xd_sector_size = 512;
#endif


// [S069@20070720]: To improve mov/avi playback performance
#if     (S069_PERFORMANCE_CODE)
    #define PalSleep(n)     PalSleep(((n) == 0) ? 1 : (n))
#endif  /* (S069_PERFORMANCE_CODE) */
// ~[S069@20070720]: To improve mov/avi playback performance~

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
    #define MAX_PRINT_SIZE (5*1024*1024)
    static unsigned long sd_read_total_size = 0;
    static unsigned long sd_read_total_time = 0;
    static unsigned long sd_read_total_count = 0;
    static double sd_read_max_perf = 0;
    static double sd_read_min_perf = 1000000;
    static double sd_read_avg_perf = 0;
    #if defined(DTV_SD2_ENABLE)
    static unsigned long sd2_read_total_size = 0;
    static unsigned long sd2_read_total_time = 0;
    static unsigned long sd2_read_total_count = 0;
    static double sd2_read_max_perf = 0;
    static double sd2_read_min_perf = 1000000;
    static double sd2_read_avg_perf = 0;
    #endif
    static unsigned long mmc_read_total_size = 0;
    static unsigned long mmc_read_total_time = 0;
    static unsigned long mmc_read_total_count = 0;
    static double mmc_read_max_perf = 0;
    static double mmc_read_min_perf = 1000000;
    static double mmc_read_avg_perf = 0;
    //static unsigned long cf_read_total_size = 0;
    //static unsigned long cf_read_total_time = 0;
    //static unsigned long cf_read_total_count = 0;
    //static double cf_read_max_perf = 0;
    //static double cf_read_min_perf = 1000000;
    //static double cf_read_avg_perf = 0;
    static unsigned long ms_read_total_size = 0;
    static unsigned long ms_read_total_time = 0;
    static unsigned long ms_read_total_count = 0;
    static double ms_read_max_perf = 0;
    static double ms_read_min_perf = 1000000;
    static double ms_read_avg_perf = 0;
    //static unsigned long xd_read_total_size = 0;
    //static unsigned long xd_read_total_time = 0;
    //static unsigned long xd_read_total_count = 0;
    //static double xd_read_max_perf = 0;
    //static double xd_read_min_perf = 1000000;
    //static double xd_read_avg_perf = 0;
    static unsigned long usb0_read_total_size = 0;
    static unsigned long usb0_read_total_time = 0;
    static unsigned long usb0_read_total_count = 0;
    static double usb0_read_max_perf = 0;
    static double usb0_read_min_perf = 1000000;
    static double usb0_read_avg_perf = 0;
    static unsigned long usb1_read_total_size = 0;
    static unsigned long usb1_read_total_time = 0;
    static unsigned long usb1_read_total_count = 0;
    static double usb1_read_max_perf = 0;
    static double usb1_read_min_perf = 1000000;
    static double usb1_read_avg_perf = 0;
    static unsigned long usb2_read_total_size = 0;
    static unsigned long usb2_read_total_time = 0;
    static unsigned long usb2_read_total_count = 0;
    static double usb2_read_max_perf = 0;
    static double usb2_read_min_perf = 1000000;
    static double usb2_read_avg_perf = 0;
    static unsigned long usb3_read_total_size = 0;
    static unsigned long usb3_read_total_time = 0;
    static unsigned long usb3_read_total_count = 0;
    static double usb3_read_max_perf = 0;
    static double usb3_read_min_perf = 1000000;
    static double usb3_read_avg_perf = 0;
    static unsigned long usb4_read_total_size = 0;
    static unsigned long usb4_read_total_time = 0;
    static unsigned long usb4_read_total_count = 0;
    static double usb4_read_max_perf = 0;
    static double usb4_read_min_perf = 1000000;
    static double usb4_read_avg_perf = 0;
    static unsigned long usb5_read_total_size = 0;
    static unsigned long usb5_read_total_time = 0;
    static unsigned long usb5_read_total_count = 0;
    static double usb5_read_max_perf = 0;
    static double usb5_read_min_perf = 1000000;
    static double usb5_read_avg_perf = 0;
    static unsigned long usb6_read_total_size = 0;
    static unsigned long usb6_read_total_time = 0;
    static unsigned long usb6_read_total_count = 0;
    static double usb6_read_max_perf = 0;
    static double usb6_read_min_perf = 1000000;
    static double usb6_read_avg_perf = 0;
    static unsigned long usb7_read_total_size = 0;
    static unsigned long usb7_read_total_time = 0;
    static unsigned long usb7_read_total_count = 0;
    static double usb7_read_max_perf = 0;
    static double usb7_read_min_perf = 1000000;
    static double usb7_read_avg_perf = 0;
#endif

#if defined(__FREERTOS__)
    /* FIXME: Can not link FREERTOS include files because queue.h is conflict */
    void dc_invalidate(void);
    //static MMP_UINT32 vrambuffer[MAX_WRITE_SECTOR_CNT * 512 / sizeof (MMP_UINT32)];
#endif


//static int smedia_ms_initcard(F_DRIVER *driver, t_mmc_dsc *dsc, unsigned long *number_of_sectors)
//{
//    MMP_UINT result = MMP_RESULT_SUCCESS;
//    result = mmpMsproInitialize();
//
//    if (!result)
//    {
//        dsc->initok = 1;
//        return MMC_NO_ERROR;
//    }
//    else
//    {
//        dsc->initok = 0;
//        return MMC_ERR_CMD;
//    }
//}

#ifdef DTV_SD1_ENABLE
/****************************************************************************
 *
 * sd_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int sd_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpSdReadMultipleSector(sector, 1, data);

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * sd_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as sd_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int sd_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

	ret  = mmpSdReadMultipleSector(sector, cnt, data);

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		sd_read_total_time += duration;
		sd_read_total_size  += (sd_sector_size * cnt);
	}
	if (sd_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)sd_read_total_size / 1024) / (double)sd_read_total_time) * 1000);
		if (temp_perf > sd_read_max_perf) sd_read_max_perf = temp_perf;
		if (temp_perf < sd_read_min_perf) sd_read_min_perf = temp_perf;
		sd_read_avg_perf = (sd_read_avg_perf * sd_read_total_count + temp_perf ) / (sd_read_total_count + 1);
		sd_read_total_count++;
		printf("[SD read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[SD read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", sd_read_max_perf, sd_read_min_perf, sd_read_avg_perf);
		sd_read_total_time = 0;
		sd_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * sd_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int sd_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpSdWriteMultipleSector(sector, 1, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * sd_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as sd_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int sd_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpSdWriteMultipleSector(sector, cnt, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * sd_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long sd_getstatus(F_DRIVER *driver)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	// TODO:
	return 0;
}

/****************************************************************************
 *
 * sd_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int sd_getphy(F_DRIVER *driver, F_PHY *phy)
{
	MMP_INT result = 0;
	MMP_UINT32 sectors = 0;
	MMP_UINT32 blockSize = 0;

	result = mmpSdGetCapacity(&sectors, &blockSize);
	if (result)
		goto end;

	sd_sector_size = 512;
	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors   = sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

end:
	return result;
}

/****************************************************************************
 *
 * sd_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void sd_release(F_DRIVER *driver)
{
	mmpSdTerminate();
	(void)_memset(&sd_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&sd_drv, 0, sizeof(sd_drv));
	sd_number_of_sectors = 0;
}

/****************************************************************************
 *
 * sd_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *sd_initfunc(unsigned long driver_param)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
	(void)_memset(&sd_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&sd_drv, 0, sizeof(sd_drv));

	//ret  = mmpSdInitialize();
	//if (ret != MMP_RESULT_SUCCESS)
	//{
	//sd_dsc.initok = 0;
	//return 0;
	//}
	//else
	{
		sd_drv.readsector = sd_readsector;
		sd_drv.writesector = sd_writesector;
		sd_drv.readmultiplesector = sd_readmultiplesector;
		sd_drv.writemultiplesector = sd_writemultiplesector;
		sd_drv.getstatus = sd_getstatus;
		sd_drv.getphy = sd_getphy;
		sd_drv.release = sd_release;
		sd_drv.user_data = 0x0002;
		sd_dsc.initok = 1;

		return (&sd_drv);
	}
}
#endif

#ifdef DTV_SD2_ENABLE
/****************************************************************************
 *
 * sd2_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int sd2_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpSd2ReadMultipleSector(sector, 1, data);

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * sd2_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as sd2_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int sd2_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

	ret  = mmpSd2ReadMultipleSector(sector, cnt, data);

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		sd2_read_total_time += duration;
		sd2_read_total_size  += (sd2_sector_size * cnt);
	}
	if (sd2_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)sd2_read_total_size / 1024) / (double)sd2_read_total_time) * 1000);
		if (temp_perf > sd2_read_max_perf) sd2_read_max_perf = temp_perf;
		if (temp_perf < sd2_read_min_perf) sd2_read_min_perf = temp_perf;
		sd2_read_avg_perf = (sd2_read_avg_perf * sd2_read_total_count + temp_perf ) / (sd2_read_total_count + 1);
		sd2_read_total_count++;
		printf("[SD2 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[SD2 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", sd2_read_max_perf, sd2_read_min_perf, sd2_read_avg_perf);
		sd2_read_total_time = 0;
		sd2_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * sd2_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int sd2_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpSd2WriteMultipleSector(sector, 1, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * sd2_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as sd2_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int sd2_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpSd2WriteMultipleSector(sector, cnt, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * sd2_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long sd2_getstatus(F_DRIVER *driver)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	// TODO:
	return 0;
}

/****************************************************************************
 *
 * sd2_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int sd2_getphy(F_DRIVER *driver, F_PHY *phy)
{
	MMP_INT result = 0;
	MMP_UINT32 sectors = 0;
	MMP_UINT32 blockSize = 0;

	result = mmpSd2GetCapacity(&sectors, &blockSize);
	if (result)
		goto end;

	sd2_sector_size = 512;
	phy->number_of_cylinders = 0;
	phy->sector_per_track    = 63;
	phy->number_of_heads     = 255;
	phy->number_of_sectors   = sectors;
	phy->media_descriptor    = 0xf8; /* fixed drive */
    phy->bytes_per_sector    = 512;

end:
	return result;
}

/****************************************************************************
 *
 * sd2_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void sd2_release(F_DRIVER *driver)
{
	mmpSd2Terminate();
	(void)_memset(&sd2_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&sd2_drv, 0, sizeof(sd2_drv));
	sd2_number_of_sectors = 0;
}

/****************************************************************************
 *
 * sd2_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *sd2_initfunc(unsigned long driver_param)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
	(void)_memset(&sd2_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&sd2_drv, 0, sizeof(sd2_drv));

	//ret  = mmpSd2Initialize();
	//if (ret != MMP_RESULT_SUCCESS)
	//{
	//	sd2_dsc.initok = 0;
	//	return 0;
	//}
	//else
	{
		sd2_drv.readsector = sd2_readsector;
		sd2_drv.writesector = sd2_writesector;
		sd2_drv.readmultiplesector = sd2_readmultiplesector;
		sd2_drv.writemultiplesector = sd2_writemultiplesector;
		sd2_drv.getstatus = sd2_getstatus;
		sd2_drv.getphy = sd2_getphy;
		sd2_drv.release = sd2_release;
		sd2_drv.user_data = 0x0002;
		sd2_dsc.initok = 1;

		return (&sd2_drv);
	}
}
#endif

#ifdef DTV_CF_ENABLE
/****************************************************************************
 *
 * cf_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int cf_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpCfReadMultipleSector(sector, 1, data);

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * cf_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as cf_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int cf_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

	ret  = mmpCfReadMultipleSector(sector, cnt, data);

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		cf_read_total_time += duration;
		cf_read_total_size  += (cf_sector_size * cnt);
	}
	if (cf_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)cf_read_total_size / 1024) / (double)cf_read_total_time) * 1000);
		if (temp_perf > cf_read_max_perf) cf_read_max_perf = temp_perf;
		if (temp_perf < cf_read_min_perf) cf_read_min_perf = temp_perf;
		cf_read_avg_perf = (cf_read_avg_perf * cf_read_total_count + temp_perf ) / (cf_read_total_count + 1);
		cf_read_total_count++;
		printf("[CF read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[CF read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", cf_read_max_perf, cf_read_min_perf, cf_read_avg_perf);
		cf_read_total_time = 0;
		cf_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * cf_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int cf_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpCfWriteMultipleSector(sector, 1, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * cf_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as cf_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int cf_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpCfWriteMultipleSector(sector, cnt, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * cf_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long cf_getstatus(F_DRIVER *driver)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	// TODO:
	return 0;
}

/****************************************************************************
 *
 * cf_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int cf_getphy(F_DRIVER *driver, F_PHY *phy)
{
	MMP_INT result = 0;
	MMP_UINT32 sectors = 0;
	MMP_UINT32 blockSize = 0;

	result = mmpCfGetCapacity(&sectors, &blockSize);
	if (result)
		goto end;

	cf_sector_size = blockSize;
	phy->number_of_cylinders = 0;
	phy->sector_per_track    = 63;
	phy->number_of_heads     = 255;
	phy->number_of_sectors   = sectors;
	phy->media_descriptor    = 0xf8; /* fixed drive */

end:
	return result;
}

/****************************************************************************
 *
 * cf_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void cf_release(F_DRIVER *driver)
{
	mmpCfTerminate();
	(void)_memset(&cf_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&cf_drv, 0, sizeof(cf_drv));
	cf_number_of_sectors = 0;
}

/****************************************************************************
 *
 * cf_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *cf_initfunc(unsigned long driver_param)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
	(void)_memset(&cf_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&cf_drv, 0, sizeof(cf_drv));

	ret  = mmpCfInitialize();
	if (ret != MMP_RESULT_SUCCESS)
	{
		cf_dsc.initok = 0;
		return 0;
	}
	else
	{
		cf_drv.readsector = cf_readsector;
		cf_drv.writesector = cf_writesector;
		cf_drv.readmultiplesector = cf_readmultiplesector;
		cf_drv.writemultiplesector = cf_writemultiplesector;
		cf_drv.getstatus = cf_getstatus;
		cf_drv.getphy = cf_getphy;
		cf_drv.release = cf_release;
		cf_drv.user_data = 0x0008;
		cf_dsc.initok = 1;

		return (&cf_drv);
	}
}
#endif

#ifdef DTV_MS_ENABLE
/****************************************************************************
 *
 * ms_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int ms_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpMsproReadMultipleSector(sector, 1, data);

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * ms_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as ms_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int ms_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

	ret  = mmpMsproReadMultipleSector(sector, cnt, data);

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		ms_read_total_time += duration;
		ms_read_total_size  += (ms_sector_size * cnt);
	}
	if (ms_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)ms_read_total_size / 1024) / (double)ms_read_total_time) * 1000);
		if (temp_perf > ms_read_max_perf) ms_read_max_perf = temp_perf;
		if (temp_perf < ms_read_min_perf) ms_read_min_perf = temp_perf;
		ms_read_avg_perf = (ms_read_avg_perf * ms_read_total_count + temp_perf ) / (ms_read_total_count + 1);
		ms_read_total_count++;
		printf("[MS read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[MS read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", ms_read_max_perf, ms_read_min_perf, ms_read_avg_perf);
		ms_read_total_time = 0;
		ms_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * ms_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int ms_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpMsproWriteMultipleSector(sector, 1, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * ms_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int ms_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret  = mmpMsproWriteMultipleSector(sector, cnt, data);
	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_WRITE;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * ms_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long ms_getstatus(F_DRIVER *driver)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	// TODO:
	return 0;
}

/****************************************************************************
 *
 * ms_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int ms_getphy(F_DRIVER *driver, F_PHY *phy)
{
	MMP_INT result = 0;
	MS_PRO_CARD_ATTRIB attrib = {0};

	result = mmpMsproGetAttrib(&attrib);
	if (result)
		goto end;

	ms_sector_size = 512;
	phy->number_of_cylinders = attrib.numCylinders;
	phy->number_of_heads     = attrib.numHeads;
	phy->sector_per_track    = attrib.numSectorPerTrack;
	phy->number_of_sectors   = attrib.numSectors;
	phy->media_descriptor    = attrib.mediaDescriptor;

end:
	return result;
}

/****************************************************************************
 *
 * ms_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void ms_release(F_DRIVER *driver)
{
	mmpMsproTerminate();

	(void)_memset(&ms_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&ms_drv, 0, sizeof(ms_drv));
	ms_number_of_sectors = 0;
}

/****************************************************************************
 *
 * ms_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *ms_initfunc(unsigned long driver_param)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
	(void)_memset(&ms_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&ms_drv, 0, sizeof(ms_drv));

	//ret  = mmpMsproInitialize();
	//if (ret != MMP_RESULT_SUCCESS)
	//{
	//ms_dsc.initok = 0;
	//return 0;
	//}
	//else
	{
		ms_drv.readsector = ms_readsector;
		ms_drv.writesector = ms_writesector;
		ms_drv.readmultiplesector = ms_readmultiplesector;
		ms_drv.writemultiplesector = ms_writemultiplesector;
		ms_drv.getstatus = ms_getstatus;
		ms_drv.getphy = ms_getphy;
		ms_drv.release = ms_release;
		ms_drv.user_data = 0x0010;

		ms_dsc.initok = 1;
		return (&ms_drv);
	}
}
#endif

#ifdef DTV_xD_ENABLE
/****************************************************************************
 *
 * xd_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int xd_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

	ret =  xddrv_readsector(driver, data, sector);

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * xd_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as xd_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int xd_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

	ret =  xddrv_readmultiplesector(driver, data, sector, cnt) ;

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		xd_read_total_time += duration;
		xd_read_total_size  += (xd_sector_size * cnt);
	}
	if (xd_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)xd_read_total_size / 1024) / (double)xd_read_total_time) * 1000);
		if (temp_perf > xd_read_max_perf) xd_read_max_perf = temp_perf;
		if (temp_perf < xd_read_min_perf) xd_read_min_perf = temp_perf;
		xd_read_avg_perf = (xd_read_avg_perf * xd_read_total_count + temp_perf ) / (xd_read_total_count + 1);
		xd_read_total_count++;
		printf("[xD read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[xD read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", xd_read_max_perf, xd_read_min_perf, xd_read_avg_perf);
		xd_read_total_time = 0;
		xd_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 *
 * xd_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int xd_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
	if ( xddrv_writesector(driver, data, sector) )
		return 1;
	return 0;
}

/****************************************************************************
 * xd_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int xd_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	if ( xddrv_writemultiplesector(driver, data, sector, cnt) )
		return 1;
	return 0;
}

/****************************************************************************
 *
 * xd_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long xd_getstatus(F_DRIVER *driver)
{
	MMP_UINT32 numOfBlocks = 0;

	if ( xd_dsc.initok )
	{
		return 0;
	}
	else
	{
		if ( !XD_Initialize(&numOfBlocks) )
		{
			xd_dsc.initok = 1;
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

/****************************************************************************
 *
 * xd_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int xd_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (xd_dsc.initok == 0)
	{
		return MMC_ERR_NOTINITIALIZED;
	}

	{
		F_PHY xd_phy;
		xddrv_getphy(driver, &xd_phy);
		xd_number_of_sectors = xd_phy.number_of_sectors;
	}

	xd_sector_size = 512;

	phy->number_of_cylinders = 0;
	phy->sector_per_track    = 63;
	phy->number_of_heads     = 255;
	phy->number_of_sectors   = xd_number_of_sectors;
	phy->media_descriptor    = 0xf8; /* fixed drive */

	if (xd_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}
}

/****************************************************************************
 *
 * xd_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void xd_release(F_DRIVER *driver)
{
	xddrv_release(driver);

	(void)_memset(&xd_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&xd_drv, 0, sizeof(xd_drv));
	xd_number_of_sectors = 0;
}

/****************************************************************************
 *
 * xd_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *xd_initfunc(unsigned long driver_param)
{
	(void)_memset(&xd_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&xd_drv, 0, sizeof(xd_drv));

	xd_drv.readsector          = xd_readsector;
	xd_drv.writesector         = xd_writesector;
	xd_drv.readmultiplesector  = xd_readmultiplesector;
	xd_drv.writemultiplesector = xd_writemultiplesector;
	xd_drv.getstatus           = xd_getstatus;
	xd_drv.getphy              = xd_getphy;
	xd_drv.release             = xd_release;
	xd_drv.user_data           = 0x0020;

	//if ( f_xdinit(driver_param) != 0 )
	//{
	//return 0;
	//}
	//else
	//{
	xd_dsc.initok = 1;
	//}

	return &xd_drv;
}
#endif

#ifdef DTV_USB_ENABLE

/****************************************************************************
 *
 * usb0_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb0_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 0, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 0, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}
}

/****************************************************************************
 * usb0_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb0_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 0, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 0, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb0_read_total_time += duration;
		usb0_read_total_size  += (usb0_sector_size * cnt);
	}
	if (usb0_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb0_read_total_size / 1024) / (double)usb0_read_total_time) * 1000);
		if (temp_perf > usb0_read_max_perf) usb0_read_max_perf = temp_perf;
		if (temp_perf < usb0_read_min_perf) usb0_read_min_perf = temp_perf;
		usb0_read_avg_perf = (usb0_read_avg_perf * usb0_read_total_count + temp_perf ) / (usb0_read_total_count + 1);
		usb0_read_total_count++;
		printf("[USB0 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB0 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb0_read_max_perf, usb0_read_min_perf, usb0_read_avg_perf);
		usb0_read_total_time = 0;
		usb0_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb0_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb0_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 0, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 0, sector, 1, data);
#endif
}

/****************************************************************************
 * usb0_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb0_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 0, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 0, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb0_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb0_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb0_dsc.initok)
		return 0;
	else
	{

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
        if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 0))
#else
        if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[0].ctxt, 0))
#endif

        {
        usb0_dsc.initok = 1;
        return 0;
        }
        else
        return F_ST_MISSING;
        	}
}

/****************************************************************************
 *
 * usb0_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb0_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb0_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 0, &usb0_number_of_sectors, &usb0_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 0, &usb0_number_of_sectors, &usb0_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb0_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb0_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb0_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb0_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb0_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb0_drv, 0, sizeof(usb0_drv));
	usb0_number_of_sectors = 0;
	
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 0);
#else
	mmpMscTerminate(usb_device[0].ctxt, 0);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb0_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb0_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb0_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb0_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb0_drv, 0, sizeof(usb0_drv));

	usb0_drv.readsector = usb0_readsector;
	usb0_drv.writesector = usb0_writesector;
	usb0_drv.readmultiplesector = usb0_readmultiplesector;
	usb0_drv.writemultiplesector = usb0_writemultiplesector;
	usb0_drv.getstatus = usb0_getstatus;
	usb0_drv.getphy = usb0_getphy;
	usb0_drv.release = usb0_release;
	usb0_drv.user_data = 0x0100;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 0);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 0);
#endif

	if (!result)
	{
		usb0_dsc.initok = 1;
	}

	return &usb0_drv;
}



/****************************************************************************
 *
 * usb1_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb1_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 1, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 1, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb1_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb1_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 1, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 1, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb1_read_total_time += duration;
		usb1_read_total_size  += (usb1_sector_size * cnt);
	}
	if (usb1_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb1_read_total_size / 1024) / (double)usb1_read_total_time) * 1000);
		if (temp_perf > usb1_read_max_perf) usb1_read_max_perf = temp_perf;
		if (temp_perf < usb1_read_min_perf) usb1_read_min_perf = temp_perf;
		usb1_read_avg_perf = (usb1_read_avg_perf * usb1_read_total_count + temp_perf ) / (usb1_read_total_count + 1);
		usb1_read_total_count++;
		printf("[USB1 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB1 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb1_read_max_perf, usb1_read_min_perf, usb1_read_avg_perf);
		usb1_read_total_time = 0;
		usb1_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb1_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb1_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 1, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 1, sector, 1, data);
#endif
}

/****************************************************************************
 * usb1_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb1_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 1, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 1, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb1_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb1_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb1_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 1))
#else
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 1))
#endif
		{
			usb1_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb1_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb1_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb1_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 1, &usb1_number_of_sectors, &usb1_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 1, &usb1_number_of_sectors, &usb1_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb1_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb1_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb1_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb1_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb1_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb1_drv, 0, sizeof(usb1_drv));
	usb1_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 1);
#else
	mmpMscTerminate(usb_device[0].ctxt, 1);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb1_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb1_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb1_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb1_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb1_drv, 0, sizeof(usb1_drv));

	usb1_drv.readsector = usb1_readsector;
	usb1_drv.writesector = usb1_writesector;
	usb1_drv.readmultiplesector = usb1_readmultiplesector;
	usb1_drv.writemultiplesector = usb1_writemultiplesector;
	usb1_drv.getstatus = usb1_getstatus;
	usb1_drv.getphy = usb1_getphy;
	usb1_drv.release = usb1_release;
	usb1_drv.user_data = 0x0200;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 1);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 1);
#endif

	if (!result)
	{
		usb1_dsc.initok = 1;
	}

	return &usb1_drv;
}

/****************************************************************************
 *
 * usb2_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb2_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 2, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 2, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb2_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb2_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 2, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 2, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb2_read_total_time += duration;
		usb2_read_total_size  += (usb2_sector_size * cnt);
	}
	if (usb2_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb2_read_total_size / 1024) / (double)usb2_read_total_time) * 1000);
		if (temp_perf > usb2_read_max_perf) usb2_read_max_perf = temp_perf;
		if (temp_perf < usb2_read_min_perf) usb2_read_min_perf = temp_perf;
		usb2_read_avg_perf = (usb2_read_avg_perf * usb2_read_total_count + temp_perf ) / (usb2_read_total_count + 1);
		usb2_read_total_count++;
		printf("[USB2 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB2 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb2_read_max_perf, usb2_read_min_perf, usb2_read_avg_perf);
		usb2_read_total_time = 0;
		usb2_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb2_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb2_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 2, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 2, sector, 1, data);
#endif
}

/****************************************************************************
 * usb2_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb2_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 2, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 2, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb2_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb2_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb2_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 2))
#else
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 2))
#endif
		{
			usb2_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb2_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb2_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb2_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 2, &usb2_number_of_sectors, &usb2_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 2, &usb2_number_of_sectors, &usb2_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb2_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb2_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb2_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb2_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb2_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb2_drv, 0, sizeof(usb2_drv));
	usb2_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 2);
#else
	mmpMscTerminate(usb_device[0].ctxt, 2);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb2_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb2_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb2_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb2_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb2_drv, 0, sizeof(usb2_drv));

	usb2_drv.readsector = usb2_readsector;
	usb2_drv.writesector = usb2_writesector;
	usb2_drv.readmultiplesector = usb2_readmultiplesector;
	usb2_drv.writemultiplesector = usb2_writemultiplesector;
	usb2_drv.getstatus = usb2_getstatus;
	usb2_drv.getphy = usb2_getphy;
	usb2_drv.release = usb2_release;
	usb2_drv.user_data = 0x0400;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 2);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 2);
#endif

	if (!result)
	{
		usb2_dsc.initok = 1;
	}

	return &usb2_drv;
}


/****************************************************************************
 *
 * usb3_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb3_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 3, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 3, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb3_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb3_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 3, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 3, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb3_read_total_time += duration;
		usb3_read_total_size  += (usb3_sector_size * cnt);
	}
	if (usb3_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb3_read_total_size / 1024) / (double)usb3_read_total_time) * 1000);
		if (temp_perf > usb3_read_max_perf) usb3_read_max_perf = temp_perf;
		if (temp_perf < usb3_read_min_perf) usb3_read_min_perf = temp_perf;
		usb3_read_avg_perf = (usb3_read_avg_perf * usb3_read_total_count + temp_perf ) / (usb3_read_total_count + 1);
		usb3_read_total_count++;
		printf("[USB3 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB3 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb3_read_max_perf, usb3_read_min_perf, usb3_read_avg_perf);
		usb3_read_total_time = 0;
		usb3_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb3_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb3_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 3, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 3, sector, 1, data);
#endif
}

/****************************************************************************
 * usb3_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb3_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 3, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 3, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb3_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb3_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb3_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 3))
#else
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 3))
#endif
		{
			usb3_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb3_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb3_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb3_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 3, &usb3_number_of_sectors, &usb3_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 3, &usb3_number_of_sectors, &usb3_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb3_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb3_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb3_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb3_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb3_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb3_drv, 0, sizeof(usb3_drv));
	usb3_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 3);
#else
	mmpMscTerminate(usb_device[0].ctxt, 3);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb3_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb3_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb3_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb3_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb3_drv, 0, sizeof(usb3_drv));

	usb3_drv.readsector = usb3_readsector;
	usb3_drv.writesector = usb3_writesector;
	usb3_drv.readmultiplesector = usb3_readmultiplesector;
	usb3_drv.writemultiplesector = usb3_writemultiplesector;
	usb3_drv.getstatus = usb3_getstatus;
	usb3_drv.getphy = usb3_getphy;
	usb3_drv.release = usb3_release;
	usb3_drv.user_data = 0x0800;
	
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 3);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 3);
#endif

	if (!result)
	{
		usb3_dsc.initok = 1;
	}

	return &usb3_drv;
}


/****************************************************************************
 *
 * usb4_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb4_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 4, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 4, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb4_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb4_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 4, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 4, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb4_read_total_time += duration;
		usb4_read_total_size  += (usb4_sector_size * cnt);
	}
	if (usb4_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb4_read_total_size / 1024) / (double)usb4_read_total_time) * 1000);
		if (temp_perf > usb4_read_max_perf) usb4_read_max_perf = temp_perf;
		if (temp_perf < usb4_read_min_perf) usb4_read_min_perf = temp_perf;
		usb4_read_avg_perf = (usb4_read_avg_perf * usb4_read_total_count + temp_perf ) / (usb4_read_total_count + 1);
		usb4_read_total_count++;
		printf("[USB4 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB4 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb4_read_max_perf, usb4_read_min_perf, usb4_read_avg_perf);
		usb4_read_total_time = 0;
		usb4_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb4_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb4_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 4, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 4, sector, 1, data);
#endif
}

/****************************************************************************
 * usb4_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb4_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 4, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 4, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb4_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb4_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb4_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 4))
#else
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 4))
#endif
		{
			usb4_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb4_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb4_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb4_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 4, &usb4_number_of_sectors, &usb4_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 4, &usb4_number_of_sectors, &usb4_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb4_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb4_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb4_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb4_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb4_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb4_drv, 0, sizeof(usb4_drv));
	usb4_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 4);
#else
	mmpMscTerminate(usb_device[0].ctxt, 4);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb4_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb4_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb4_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb4_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb4_drv, 0, sizeof(usb4_drv));

	usb4_drv.readsector = usb4_readsector;
	usb4_drv.writesector = usb4_writesector;
	usb4_drv.readmultiplesector = usb4_readmultiplesector;
	usb4_drv.writemultiplesector = usb4_writemultiplesector;
	usb4_drv.getstatus = usb4_getstatus;
	usb4_drv.getphy = usb4_getphy;
	usb4_drv.release = usb4_release;
	usb4_drv.user_data = 0x1000;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 4);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 4);
#endif

	if (!result)
	{
		usb4_dsc.initok = 1;
	}

	return &usb4_drv;
}


/****************************************************************************
 *
 * usb5_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb5_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 5, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 5, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb5_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb5_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 5, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 5, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb5_read_total_time += duration;
		usb5_read_total_size  += (usb5_sector_size * cnt);
	}
	if (usb5_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb5_read_total_size / 1024) / (double)usb5_read_total_time) * 1000);
		if (temp_perf > usb5_read_max_perf) usb5_read_max_perf = temp_perf;
		if (temp_perf < usb5_read_min_perf) usb5_read_min_perf = temp_perf;
		usb5_read_avg_perf = (usb5_read_avg_perf * usb5_read_total_count + temp_perf ) / (usb5_read_total_count + 1);
		usb5_read_total_count++;
		printf("[USB5 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB5 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb5_read_max_perf, usb5_read_min_perf, usb5_read_avg_perf);
		usb5_read_total_time = 0;
		usb5_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb5_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb5_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 5, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 5, sector, 1, data);
#endif
}

/****************************************************************************
 * usb5_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb5_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 5, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 5, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb5_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb5_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb5_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		//if (MMC_NO_ERROR == mmpUsbHost_getstatus(5))
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 5))
#else
		//if (MMC_NO_ERROR == mmpOtgHost_getstatus(5))
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 5))
#endif
		{
			usb5_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb5_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb5_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb5_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 5, &usb5_number_of_sectors, &usb5_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 5, &usb5_number_of_sectors, &usb5_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb5_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb5_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb5_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb5_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb5_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb5_drv, 0, sizeof(usb5_drv));
	usb5_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 5);
#else
	mmpMscTerminate(usb_device[0].ctxt, 5);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb5_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb5_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb5_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb5_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb5_drv, 0, sizeof(usb5_drv));

	usb5_drv.readsector = usb5_readsector;
	usb5_drv.writesector = usb5_writesector;
	usb5_drv.readmultiplesector = usb5_readmultiplesector;
	usb5_drv.writemultiplesector = usb5_writemultiplesector;
	usb5_drv.getstatus = usb5_getstatus;
	usb5_drv.getphy = usb5_getphy;
	usb5_drv.release = usb5_release;
	usb5_drv.user_data = 0x2000;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 5);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 5);
#endif

	if (!result)
	{
		usb5_dsc.initok = 1;
	}

	return &usb5_drv;
}

/****************************************************************************
 *
 * usb6_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb6_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 6, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 6, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb6_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb6_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 6, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 6, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb6_read_total_time += duration;
		usb6_read_total_size  += (usb6_sector_size * cnt);
	}
	if (usb6_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb6_read_total_size / 1024) / (double)usb6_read_total_time) * 1000);
		if (temp_perf > usb6_read_max_perf) usb6_read_max_perf = temp_perf;
		if (temp_perf < usb6_read_min_perf) usb6_read_min_perf = temp_perf;
		usb6_read_avg_perf = (usb6_read_avg_perf * usb6_read_total_count + temp_perf ) / (usb6_read_total_count + 1);
		usb6_read_total_count++;
		printf("[USB6 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB6 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb6_read_max_perf, usb6_read_min_perf, usb6_read_avg_perf);
		usb6_read_total_time = 0;
		usb6_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb6_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb6_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 6, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 6, sector, 1, data);
#endif
}

/****************************************************************************
 * usb6_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb6_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 6, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 6, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb6_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb6_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb6_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 6))
#else
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 6))
#endif
		{
			usb6_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb6_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb6_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb6_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 6, &usb6_number_of_sectors, &usb6_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 6, &usb6_number_of_sectors, &usb6_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb6_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb6_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb6_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb6_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb6_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb6_drv, 0, sizeof(usb6_drv));
	usb6_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 6);
#else
	mmpMscTerminate(usb_device[0].ctxt, 6);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb6_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb6_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb6_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb6_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb6_drv, 0, sizeof(usb6_drv));

	usb6_drv.readsector = usb6_readsector;
	usb6_drv.writesector = usb6_writesector;
	usb6_drv.readmultiplesector = usb6_readmultiplesector;
	usb6_drv.writemultiplesector = usb6_writemultiplesector;
	usb6_drv.getstatus = usb6_getstatus;
	usb6_drv.getphy = usb6_getphy;
	usb6_drv.release = usb6_release;
	usb6_drv.user_data = 0x4000;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 6);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 6);
#endif

	if (!result)
	{
		usb6_dsc.initok = 1;
	}

	return &usb6_drv;
}

/****************************************************************************
 *
 * usb7_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int usb7_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 7, sector, 1, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 7, sector, 1, data);
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 * usb7_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb7_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
	MMP_RESULT ret = MMP_RESULT_SUCCESS;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	MMP_ULONG starttime = 0;
	MMP_ULONG duration = 0;
	double temp_perf = 0;

	starttime = PalGetClock();
#endif

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, 7, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, 7, sector, cnt, data);
#endif

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
	duration = PalGetDuration(starttime);
	if (duration > 0)
	{
		usb7_read_total_time += duration;
		usb7_read_total_size  += (usb7_sector_size * cnt);
	}
	if (usb7_read_total_size > MAX_PRINT_SIZE)
	{
		temp_perf = ((((double)usb7_read_total_size / 1024) / (double)usb7_read_total_time) * 1000);
		if (temp_perf > usb7_read_max_perf) usb7_read_max_perf = temp_perf;
		if (temp_perf < usb7_read_min_perf) usb7_read_min_perf = temp_perf;
		usb6_read_avg_perf = (usb6_read_avg_perf * usb6_read_total_count + temp_perf ) / (usb7_read_total_count + 1);
		usb6_read_total_count++;
		printf("[USB7 read perforamnce] read rate = %-7.0f KB/sec!!\n", temp_perf);
		printf("[USB7 read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n", usb7_read_max_perf, usb7_read_min_perf, usb7_read_avg_perf);
		usb7_read_total_time = 0;
		usb7_read_total_size  = 0;
	}
#endif

	if (ret != MMP_RESULT_SUCCESS)
	{
		return F_ERR_READ;
	}
	else
	{
		return F_NO_ERROR;
	}

}

/****************************************************************************
 *
 * usb7_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int usb7_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 7, sector, 1, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 7, sector, 1, data);
#endif
}

/****************************************************************************
 * usb7_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb7_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	return mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, 7, sector, cnt, data);
#else
	return mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, 7, sector, cnt, data);
#endif
}

/****************************************************************************
 *
 * usb7_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long usb7_getstatus(F_DRIVER *driver)
{
	//unsigned long ptr = driver->user_data;
	/* TODO: handle F_ST_CHANGED */

	if (usb7_dsc.initok)
		return 0;
	else
	{
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 7))
#else
		if (MMC_NO_ERROR == mmpMscGetStatus2(usb_device[1].ctxt, 7))
#endif
		{
			usb7_dsc.initok = 1;
			return 0;
		}
		else
			return F_ST_MISSING;
	}
}

/****************************************************************************
 *
 * usb7_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int usb7_getphy(F_DRIVER *driver, F_PHY *phy)
{
	if (usb7_dsc.initok == 0) return MMC_ERR_NOTINITIALIZED;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, 7, &usb7_number_of_sectors, &usb7_sector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, 7, &usb7_number_of_sectors, &usb7_sector_size);
#endif

	phy->number_of_cylinders = 0;
	phy->sector_per_track = 63;
	phy->number_of_heads = 255;
	phy->number_of_sectors = usb7_number_of_sectors;
	phy->media_descriptor = 0xf8; /* fixed drive */

	if (usb7_number_of_sectors)
	{
		return MMC_NO_ERROR;
	}
	else
	{
		return MMC_ERR_NOTINITIALIZED;
	}

}

/****************************************************************************
 *
 * usb7_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void usb7_release(F_DRIVER *driver)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb7_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb7_drv, 0, sizeof(usb7_drv));
	usb7_number_of_sectors = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, 7);
#else
	mmpMscTerminate(usb_device[0].ctxt, 7);
#endif

	if (result)
		goto end;

end:
	if (result)
		sdk_msg(SDK_MSG_TYPE_ERROR, "usb7_release() has error code 0x%08X \n", result );
}

/****************************************************************************
 *
 * usb7_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *usb7_initfunc(unsigned long driver_param)
{
	MMP_RESULT result = MMP_RESULT_SUCCESS;
	(void)_memset(&usb7_dsc, 0, sizeof(t_mmc_dsc));
	(void)_memset(&usb7_drv, 0, sizeof(usb7_drv));

	usb7_drv.readsector = usb7_readsector;
	usb7_drv.writesector = usb7_writesector;
	usb7_drv.readmultiplesector = usb7_readmultiplesector;
	usb7_drv.writemultiplesector = usb7_writemultiplesector;
	usb7_drv.getstatus = usb7_getstatus;
	usb7_drv.getphy = usb7_getphy;
	usb7_drv.release = usb7_release;
	usb7_drv.user_data = 0x8000;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	result = mmpMscInitialize(usb_device[1].ctxt, 7);
#else
	result = mmpMscInitialize(usb_device[0].ctxt, 7);
#endif


	if (!result)
	{
		usb7_dsc.initok = 1;
	}

	return &usb7_drv;
}
#endif

#if 0

// The template of device 
static t_mmc_dsc temp1_dsc;     /* current TEMP1 card descriptor */
static F_DRIVER temp1_drv;
static unsigned long temp1_number_of_sectors = 0;

/****************************************************************************
 *
 * temp1_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

int temp1_readsector(F_DRIVER *driver, void *data, unsigned long sector)
{

}

/****************************************************************************
 * temp1_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as temp1_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int temp1_readmultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{

}

/****************************************************************************
 *
 * temp1_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
int temp1_writesector(F_DRIVER *driver, void *data, unsigned long sector)
{

}

/****************************************************************************
 * temp1_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int temp1_writemultiplesector(F_DRIVER *driver, void *data, unsigned long sector, int cnt)
{

}

/****************************************************************************
 *
 * temp1_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/
static long temp1_getstatus(F_DRIVER *driver)
{

}

/****************************************************************************
 *
 * temp1_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/
static int temp1_getphy(F_DRIVER *driver, F_PHY *phy)
{

}

/****************************************************************************
 *
 * temp1_release
 *
 * Deletes a previously initialized driver
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void temp1_release(F_DRIVER *driver)
{

}

/****************************************************************************
 *
 * temp1_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameters
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *temp1_initfunc(unsigned long driver_param)
{

}
#endif

