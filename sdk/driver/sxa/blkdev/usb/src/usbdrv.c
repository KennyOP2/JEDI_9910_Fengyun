#include "pal/pal.h"
#include "buffer.h"

//#define SXA_DEBUG

#ifdef SXA_DEBUG

#include <stdio.h>
#include <stdlib.h>

static FILE *fp;
int usb_initfunc(int devnode)
{
	fp = fopen("D:\\workzone\\powei\\work\\buffer_cache\\sdb.img", "rb+");
	return 0;
}

int usb_release(int devnode)
{
	fclose(fp);
	return 0;
}

int usb_getphy(int devnode, unsigned int *psector_size, unsigned long long *pnum_sectors)
{
	*psector_size = SXA_SECTOR_SIZE;
    *pnum_sectors = 0x4000; // 8M byte
	return 0;
}

int usb_readmultiplesector(int devnode, void *data, unsigned long long sector, int cnt)
{
	int ret;
	ret = fseek(fp, sector*SXA_SECTOR_SIZE, SEEK_SET);
	ret = fread(data, 1, cnt*SXA_SECTOR_SIZE, fp);
	return 0;
}

int usb_writemultiplesector(int devnode, void *data, unsigned long long sector, int cnt)
{
	fseek(fp, sector*SXA_SECTOR_SIZE, SEEK_SET);
	fwrite(data, 1, cnt*SXA_SECTOR_SIZE, fp);
	return 0;
}

#else

#include "mmp_usbex.h"
#include "mmp_msc.h"

extern USB_DEVICE_INFO usb_device[2]; /** 0: For Host+Device, 1: For Host only */
static int initok[SXA_MAX_DEVICE] = {0};
/****************************************************************************
 *
 * usb_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 ***************************************************************************/
int usb_initfunc(int devnode)
{

	int ret = 0;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscInitialize(usb_device[1].ctxt, devnode);
#else
	ret = mmpMscInitialize(usb_device[0].ctxt, devnode);
#endif

	if (!ret)
	    initok[devnode] = 1;

    if (ret)
		ret = -1;

	return ret;
}


/****************************************************************************
 *
 * usb_release
 *
 * Deletes a previously initialized driver
 *
 ***************************************************************************/

int usb_release(int devnode)
{
	int ret = 0;
#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscTerminate(usb_device[1].ctxt, devnode);
#else
	mmpMscTerminate(usb_device[0].ctxt, devnode);
#endif
	initok[devnode] = 0;
	return ret;
}

/****************************************************************************
 *
 * usb_getphy
 *
 * determinate flash card physicals
 *
 ***************************************************************************/
int usb_getphy(int devnode, unsigned int *psector_size, unsigned long long *pnum_sectors)
{
	int ret=0;
	MMP_UINT32 sectorNum = 0;

	if (!initok[devnode]) 
		return -1;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	mmpMscGetCapacity(usb_device[1].ctxt, devnode, &sectorNum, (MMP_UINT32*)psector_size);
#else
	mmpMscGetCapacity(usb_device[0].ctxt, devnode, &sectorNum, (MMP_UINT32*)psector_size);
#endif
    *pnum_sectors = sectorNum;
	return ret;
}

/****************************************************************************
 * usb0_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as usb0_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int usb_readmultiplesector(int devnode, void *data, unsigned long long sector, int cnt)
{
	int ret = 0;

	if (!initok[devnode]) 
		return -1;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscReadMultipleSector((void*)usb_device[1].ctxt, devnode, sector, cnt, data);
#else
	ret = mmpMscReadMultipleSector((void*)usb_device[0].ctxt, devnode, sector, cnt, data);
#endif

	if (ret)
		ret = -1;

	return ret;
}

/****************************************************************************
 * usb0_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as ms_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int usb_writemultiplesector(int devnode, void *data, unsigned long long sector, int cnt)
{
	int ret = 0;

	if (!initok[devnode]) 
		return -1;

#ifdef SMTK_USB_HOST_NOT_ON_BOARD
	ret = mmpMscWriteMultipleSector((void*)usb_device[1].ctxt, devnode, sector, cnt, data);
#else
	ret = mmpMscWriteMultipleSector((void*)usb_device[0].ctxt, devnode, sector, cnt, data);
#endif
	
	if (ret)
		ret = -1;

	return ret;
}

#endif
