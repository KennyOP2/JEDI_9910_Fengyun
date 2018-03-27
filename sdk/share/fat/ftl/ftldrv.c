/****************************************************************************
 *
 *            Copyright (c) 2005-2008 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 110
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

/******************************************************************************
 *
 * Includes
 *
 *****************************************************************************/

#include "ftldrv.h"

//#include "../../../dpf/config.h"
#include <pal/pal.h>

/* Byte per sector is selected according to used page size in FTL */
#if USE_ONLY512
#define FTLDRV_BPS 512
#elif USE_ONLY2048
#define FTLDRV_BPS 2048
#elif USE_ONLY4096
#define FTLDRV_BPS 4096
#elif USE_ONLY8192
#define FTLDRV_BPS 8192
#else
#define FTLDRV_BPS 512
#endif

#define ENABLE_FTL_SEMAPHORE

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
#define MAX_PRINT_SIZE (5*1024*1024)
static unsigned long nand_read_total_size = 0;   
static unsigned long nand_read_total_time = 0;
static unsigned long nand_read_total_count = 0;
static double nand_read_max_perf = 0;
static double nand_read_min_perf = 1000000;
static double nand_read_avg_perf = 0;
static unsigned long nand_write_total_size = 0;   
static unsigned long nand_write_total_time = 0;
static unsigned long nand_write_total_count = 0;
static double nand_write_max_perf = 0;
static double nand_write_min_perf = 1000000;
static double nand_write_avg_perf = 0;
#endif

#ifdef	ENABLE_FTL_SEMAPHORE
static void    *FTL_Semaphore = MMP_NULL;
#endif

/****************************************************************************
 *
 * ftl_readmultiplesector
 *
 * read multiple sectors from drive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer where to store data
 * sector - where to read data from
 * cnt - number of sectors to read
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

#ifndef USE_FATTHIN
static int ftl_readmultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt)
{
	unsigned char *ptr=(unsigned char *)data;
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
    MMP_ULONG starttime = 0;
    MMP_ULONG duration = 0;
    double temp_perf = 0;
    unsigned long temp_cnt = cnt;
    
    starttime = PalGetClock();
#endif
  	#ifdef	ENABLE_FTL_SEMAPHORE
  	SYS_WaitSemaphore(FTL_Semaphore);
	#endif	

/* fnPr("readmultiplesector %d cnt %d\n",sector,cnt); */

	if (ftl_open(sector,(unsigned long)cnt,FTL_READ)) 
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	while (cnt--)
	{
		if (ftl_read(ptr))
		{
			(void)ftl_close();
			#ifdef	ENABLE_FTL_SEMAPHORE
			SYS_ReleaseSemaphore(FTL_Semaphore);
			#endif
			return 1;
		}

		ptr+=FTLDRV_BPS;
	}

#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
    duration = PalGetDuration(starttime);
    if (duration > 0)
    {
        nand_read_total_time += duration;
        nand_read_total_size  += (FTLDRV_BPS * temp_cnt);
    }
    if (nand_read_total_size > MAX_PRINT_SIZE)
    {
        temp_perf = ((((double)nand_read_total_size/1024)/(double)nand_read_total_time) * 1000);
        if (temp_perf > nand_read_max_perf) nand_read_max_perf = temp_perf;
        if (temp_perf < nand_read_min_perf) nand_read_min_perf = temp_perf;
        nand_read_avg_perf = (nand_read_avg_perf * nand_read_total_count + temp_perf )/(nand_read_total_count + 1); 
        nand_read_total_count++;
        printf("[NAND read perforamnce] read rate = %-7.0f KB/sec!!\n",temp_perf);
        printf("[NAND read perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n",nand_read_max_perf,nand_read_min_perf,nand_read_avg_perf);
        nand_read_total_time = 0;
        nand_read_total_size  = 0;
    }
#endif

	if (ftl_close()) 
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}
		
	#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_ReleaseSemaphore(FTL_Semaphore);
	#endif
	
	return 0;
}
#endif

/****************************************************************************
 *
 * ftl_readsector
 *
 * read sector from drive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer where to store data
 * sector - where to read data from
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

#ifdef USE_FATTHIN
unsigned char ftl_readsector(void *data, unsigned long sector)
#else
static int ftl_readsector(F_DRIVER *driver,void *data, unsigned long sector)
#endif
{
/*  fnPr("readsector: %d\n",sector); */
/*  if (((long)data)&3) fnPr("FTL: not aligned read!\n"); */
  	#ifdef	ENABLE_FTL_SEMAPHORE
  	SYS_WaitSemaphore(FTL_Semaphore);
	#endif

	if (ftl_open(sector,1,FTL_READ)) 
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	if (ftl_read((unsigned char *)data))
	{
		(void)ftl_close();
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	if (ftl_close()) 
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_ReleaseSemaphore(FTL_Semaphore);
	#endif

	return 0;
}

/****************************************************************************
 *
 * ftl_writemultiplesector
 *
 * write multiple sectors into drive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer
 * sector - where to write data
 * cnt - number of sectors to write
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

#ifndef USE_FATTHIN
static int ftl_writemultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt)
{
	unsigned char *ptr=(unsigned char *)data;
	
  	#ifdef	ENABLE_FTL_SEMAPHORE
  	SYS_WaitSemaphore(FTL_Semaphore);
	#endif	
	
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
    MMP_ULONG starttime = 0;
    MMP_ULONG duration = 0;
    double temp_perf = 0;
    unsigned long temp_cnt = cnt;
    
    starttime = PalGetClock();
#endif

/* fnPr("writemultiplesector %d cnt %d\n",sector,cnt); */

	if (ftl_open(sector,(unsigned long)cnt,FTL_WRITE)) 
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	while (cnt--)
	{
		if (ftl_write(ptr))
		{
			(void)ftl_close();
			#ifdef	ENABLE_FTL_SEMAPHORE
			SYS_ReleaseSemaphore(FTL_Semaphore);
			#endif
			return 1;
		}

		ptr+=FTLDRV_BPS;
	}
	
#ifdef  ENABLE_MEASURE_STORAGE_PERFORMANCE
    duration = PalGetDuration(starttime);
    if (duration > 0)
    {
        nand_write_total_time += duration;
        nand_write_total_size  += (FTLDRV_BPS * temp_cnt);
    }
    if (nand_write_total_size > (500*1024))
    {
        temp_perf = ((((double)nand_write_total_size/1024)/(double)nand_write_total_time) * 1000);
        if (temp_perf > nand_write_max_perf) nand_write_max_perf = temp_perf;
        if (temp_perf < nand_write_min_perf) nand_write_min_perf = temp_perf;
        nand_write_avg_perf = (nand_write_avg_perf * nand_write_total_count + temp_perf )/(nand_write_total_count + 1); 
        nand_write_total_count++;
        printf("[NAND write perforamnce] write rate = %-7.0f KB/sec!!\n",temp_perf);
        printf("[NAND write perforamnce] MAX rate = %-7.0f MIN rate = %-7.0f AVG rate = %-7.0f\n",nand_write_max_perf,nand_write_min_perf,nand_write_avg_perf);
        nand_write_total_time = 0;
        nand_write_total_size  = 0;
    }
#endif

	if (ftl_close())
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}
		
	#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_ReleaseSemaphore(FTL_Semaphore);
	#endif
	return 0;
}
#endif

/****************************************************************************
 *
 * ftl_writesector
 *
 * write sectors into drive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer
 * sector - where to write data
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

#ifdef USE_FATTHIN
unsigned char ftl_writesector(void *data, unsigned long sector)
#else
static int ftl_writesector(F_DRIVER *driver,void *data, unsigned long sector)
#endif
{
/*  fnPr("writesector: %d\n",sector); */
/*  if (((long)data)&3) fnPr("FTL: not aligned write!\n"); */
  	#ifdef	ENABLE_FTL_SEMAPHORE
  	SYS_WaitSemaphore(FTL_Semaphore);
	#endif

	if (ftl_open(sector,1,FTL_WRITE))
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	if (ftl_write((unsigned char *)data))
	{
		(void)ftl_close();
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}

	if (ftl_close())
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
 		return 1;
 	}

	#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_ReleaseSemaphore(FTL_Semaphore);
	#endif

	return 0;
}

/****************************************************************************
 *
 * ftl_getphy
 *
 * determinate drive physicals
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

#ifdef USE_FATTHIN
unsigned char ftl_getphy(F_PHY *phy)
#else
static int ftl_getphy(F_DRIVER *driver, F_PHY *phy)
#endif
{
	phy->number_of_sectors=ftl_getmaxsector();

	phy->bytes_per_sector=FTLDRV_BPS;
		   
	return 0;
}

/****************************************************************************
 *
 * ftl_release
 *
 * deletes drive
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

#ifndef USE_FATTHIN
static void ftl_release (F_DRIVER *driver)
{
}
#endif

/****************************************************************************
 *
 * ftl_ioctl
 *
 * io control functions
 *
 * INPUTS
 *
 * driver - driver structure
 * iparam - as input parameter
 * oparam - as output parameter
 *
 * RETURNS
 *
 * 0 if success or other if any error
 *
 ***************************************************************************/

#ifndef USE_FATTHIN
#if FTL_DELETE_CONTENT
static int ftl_ioctl (F_DRIVER *driver, unsigned long msg, void *iparam, void *oparam)
{
	#ifdef	ENABLE_FTL_SEMAPHORE    
  	SYS_WaitSemaphore(FTL_Semaphore);
	#endif
	
	switch (msg)
	{
	case F_IOCTL_MSG_ENDOFDELETE:
		/* flush all log block */
		if (ml_flushlogblock())
		{
			#ifdef	ENABLE_FTL_SEMAPHORE
			SYS_ReleaseSemaphore(FTL_Semaphore);
			#endif
			return 1;
		}
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 0;

	case F_IOCTL_MSG_MULTIPLESECTORERASE:
		{
			ST_IOCTL_MULTIPLESECTORERASE *p=(ST_IOCTL_MULTIPLESECTORERASE*)iparam;
			unsigned long cou=p->sector_num;

			/* open FTL for write multiple sectors */
			if (ftl_open(p->start_sector,cou,FTL_WRITE))
			{
				#ifdef	ENABLE_FTL_SEMAPHORE
				SYS_ReleaseSemaphore(FTL_Semaphore);
				#endif
				return 1;
			}

			while(cou--)
			{
				/* write sectors with the same data */
				if (ftl_write((unsigned char *)p->one_sector_databuffer))
				{
					#ifdef	ENABLE_FTL_SEMAPHORE
					SYS_ReleaseSemaphore(FTL_Semaphore);
					#endif
					return 1;
				}
			}

			/* close ftl */
			if (ftl_close())
			{
				#ifdef	ENABLE_FTL_SEMAPHORE
				SYS_ReleaseSemaphore(FTL_Semaphore);
				#endif
				return 1;
			}
		}
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 0;

	default:
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}
}
#endif
#endif

/****************************************************************************
 *
 * ftl_initfunc (in FAT_THIN)
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 *
 *
 * f_ftldrvinit (in FAT)
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameter
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

#ifdef USE_FATTHIN

unsigned char ftl_initfunc()
{
	return (int)ftl_init();
}

#else

F_DRIVER *ftl_initfunc(unsigned long driver_param)
{
	#ifdef	ENABLE_FTL_SEMAPHORE
	if(!FTL_Semaphore)
	{
	    FTL_Semaphore = SYS_CreateSemaphore(1, "MMP_FTL");

    	if(!FTL_Semaphore)
    	{
    	    //create NAND flash semaphore fail;
            //result=LL_ERROR;
    	    goto end;
    	}
    }
	#endif
	(void)memset (&ftl_driver,0,sizeof(ftl_driver));

	ftl_driver.readsector=ftl_readsector;
	ftl_driver.writesector=ftl_writesector;
	ftl_driver.readmultiplesector=ftl_readmultiplesector;
	ftl_driver.writemultiplesector=ftl_writemultiplesector;
	ftl_driver.getphy=ftl_getphy;
	ftl_driver.release=ftl_release;

#if FTL_DELETE_CONTENT
	ftl_driver.ioctl=ftl_ioctl;
#endif

#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_WaitSemaphore(FTL_Semaphore);
#endif

	if (ftl_init())
	{
		#ifdef	ENABLE_FTL_SEMAPHORE	
		if(FTL_Semaphore)
		{
   	    	SYS_ReleaseSemaphore(FTL_Semaphore);
   	    }   	    
   	    #endif
   	    
		return 0;
	}	
end:
	#ifdef	ENABLE_FTL_SEMAPHORE
    if(FTL_Semaphore)
    {
        SYS_ReleaseSemaphore(FTL_Semaphore);
    }
    #endif

	return &ftl_driver;
}

#endif

int
mmpNandGetCapacity(unsigned long* lastBlockId,
                   unsigned long* blockLength)
{
	*lastBlockId = ftl_getmaxsector();
	//*blockLength = 512;
	*blockLength = FTLDRV_BPS;
	
	return 0;
}
         
int 
mmpNandReadSector(unsigned long blockId,
                  unsigned long sizeInByte,
                  unsigned char* srcBuffer)
{
  unsigned char *ptr=(unsigned char *)srcBuffer;

  #ifdef	ENABLE_FTL_SEMAPHORE
  SYS_WaitSemaphore(FTL_Semaphore);
  #endif

#if 0
#if defined(__FREERTOS__) && defined(FTL_CLEAN_CACHE)
    #ifdef FTL_USE_DC_INVALIDATE
	ithInvalidateDCache();
	#else
	ithInvalidateDCacheRange(srcBuffer, F_DEF_SECTOR_SIZE * sizeInByte);
	#endif
#endif
#else
	#if defined(__FREERTOS__)
		#if defined(__OPENRTOS__)
		ithInvalidateDCacheRange(srcBuffer, FTLDRV_BPS * sizeInByte);
        #elif defined(__FREERTOS__)
						dc_invalidate();
        #endif
	#endif
#endif

	if (ftl_open(blockId,sizeInByte,FTL_READ)) 
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1; 
	}

	while (sizeInByte--) 
	{
		if (ftl_read(ptr)) 
		{
			(void)ftl_close();
			#ifdef	ENABLE_FTL_SEMAPHORE
			SYS_ReleaseSemaphore(FTL_Semaphore);
			#endif
			return 1;
		}

		//ptr+=F_DEF_SECTOR_SIZE;
		ptr+=FTLDRV_BPS;
	}

	if (ftl_close())
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		
		return 1;
	}
	
	#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_ReleaseSemaphore(FTL_Semaphore);
	#endif
	
	return 0;
}
                  
int 
mmpNandWriteSector(unsigned long blockId,
                  unsigned long sizeInByte,
                  unsigned char* destBuffer)
{
  unsigned char *ptr=(unsigned char *)destBuffer;

  #ifdef	ENABLE_FTL_SEMAPHORE
  SYS_WaitSemaphore(FTL_Semaphore);
  #endif 

#if 0
#if defined(__FREERTOS__) && defined(FTL_CLEAN_CACHE)
    #ifdef FTL_USE_DC_INVALIDATE
	ithInvalidateDCache();
	#else
	ithInvalidateDCacheRange(destBuffer, F_DEF_SECTOR_SIZE * sizeInByte);
	#endif
#endif
#else
	#if defined(__FREERTOS__)
        #if defined(__OPENRTOS__)
			ithInvalidateDCacheRange(destBuffer, FTLDRV_BPS * sizeInByte);
        #elif defined(__FREERTOS__)
							dc_invalidate();
        #endif

	#endif
#endif

	if (ftl_open(blockId,sizeInByte,FTL_WRITE))
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1; 
	}

	while (sizeInByte--) 
	{
		if (ftl_write(ptr)) 
		{
			(void)ftl_close();
			#ifdef	ENABLE_FTL_SEMAPHORE
			SYS_ReleaseSemaphore(FTL_Semaphore);
			#endif
			return 1;
		}

		ptr+=FTLDRV_BPS;
	}

	if (ftl_close())
	{
		#ifdef	ENABLE_FTL_SEMAPHORE
		SYS_ReleaseSemaphore(FTL_Semaphore);
		#endif
		return 1;
	}
	
	#ifdef	ENABLE_FTL_SEMAPHORE
	SYS_ReleaseSemaphore(FTL_Semaphore);
	#endif
	
	return 0;
}     

/******************************************************************************
 *
 *  End of ftldrv.c
 *
 *****************************************************************************/

