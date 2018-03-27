/****************************************************************************
 *
 *            Copyright (c) 2003-2007 by HCC Embedded
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

#include "test_f.h"
#include "testdrv_f.h"
#include "common/debug.h"

/****************************************************************************
 *
 * defines
 *
 ***************************************************************************/

#define MEM_LONG_ACCESS 1		/* set this value to 1 if 32bit access available */

#define RAM_TEST_8GB_DRIVE	0	/* allow or not allow 8GB test drive (default is 0) */

#if RAM_TEST_8GB_DRIVE
#define RAMDRV0_SIZE 0x8000000	/* 128MB defintion for size of ramdrive0 for testing of 8GB */
#else
#define RAMDRV0_SIZE 0x100000	/* 1MB defintion for size of ramdrive0 */
#endif

#if TEST_FULL_COVERAGE
#define RAMDRV1_SIZE 0x800000   /* 8MB defintion for size of ramdrive1 */
#define RAMDRV2_SIZE 0x4000000  /* 64MB defintion for size of ramdrive2 */
#define RAMDRV3_SIZE 0x2000000  /* 32MB defintion for size of ramdrive3 multiple partitions */
#define RAMDRV_CNT	 4			/* number of ramdrives defined */
#else
#define RAMDRV_CNT	 1			/* number of ramdrives defined */
#endif


/****************************************************************************
 *
 * static structure
 *
 ***************************************************************************/

typedef struct
{
	char *ramdrv;
	unsigned long maxsector;
	int use;
	F_DRIVER *driver;
} S_TESTRAMDRV;

/****************************************************************************
 *
 * static variables
 *
 ***************************************************************************/

static int gl_errors[TESTDRV_ERRMAX];

static char g_ramdrv0[RAMDRV0_SIZE];
#if TEST_FULL_COVERAGE
static char g_ramdrv1[RAMDRV1_SIZE];
static char g_ramdrv2[RAMDRV2_SIZE];
static char g_ramdrv3[RAMDRV3_SIZE];
#endif

static F_DRIVER g_testdrivers[RAMDRV_CNT];

static S_TESTRAMDRV g_ramdrv[RAMDRV_CNT] =
{
  { g_ramdrv0, (RAMDRV0_SIZE/F_DEF_SECTOR_SIZE), 0,&g_testdrivers[0] },
#if TEST_FULL_COVERAGE
  { g_ramdrv1, (RAMDRV1_SIZE/F_DEF_SECTOR_SIZE), 0,&g_testdrivers[1] },
  { g_ramdrv2, (RAMDRV2_SIZE/F_DEF_SECTOR_SIZE), 0,&g_testdrivers[2] },
  { g_ramdrv3, (RAMDRV3_SIZE/F_DEF_SECTOR_SIZE), 0,&g_testdrivers[3] },
#endif
};


/****************************************************************************
 *
 * test_ram_readmultiplesector
 *
 * read multiple sectors from ramdrive
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
static int test_ram_readmultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt)
{
	S_TESTRAMDRV *p=(S_TESTRAMDRV *)(driver->user_ptr);
	char *d=(char*)data;
	long len;
	char *s;

	DEBPR2("rd %08x %04x\n",sector,cnt)

	if (gl_errors[TESTDRV_RD]) /* check if error is set */
	{
		DEBPR0("failed\n");
		gl_errors[TESTDRV_RD]--; /* decrease counter */
		return TEST_RAM_ERR_SECTOR; /* generate error */
	}

#if RAM_TEST_8GB_DRIVE
	if (sector>=p->maxsector) return 0; /* don't use any ram of simulated 8GB drive */
#else
	if (sector>=p->maxsector) return TEST_RAM_ERR_SECTOR;
#endif

	s=p->ramdrv;
	s+=sector*F_DEF_SECTOR_SIZE;
	len=F_DEF_SECTOR_SIZE*(long)cnt;


#if MEM_LONG_ACCESS
	/* check if size, destination and source is aligned to 32bit access */
	if ((!(len&3)) && (!(((long)d)&3)) && (!(((long)s)&3)))
	{
		long *dd=(long*)d;
		long *ss=(long*)s;
		len>>=2;
		while (len--)
		{
			*dd++=*ss++;
		}

		return TEST_RAM_NO_ERROR;
	}
#endif

	while (len--)
	{
		*d++=*s++;
	}

	return TEST_RAM_NO_ERROR;
}

/****************************************************************************
 *
 * test_ram_readsector
 *
 * read sector from ramdrive
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

static int test_ram_readsector(F_DRIVER *driver,void *data, unsigned long sector)
{
	return test_ram_readmultiplesector(driver,data,sector,1);
}

/****************************************************************************
 *
 * test_ram_writemultiplesector
 *
 * write multiple sectors into ramdrive
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

static int test_ram_writemultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt)
{
	long len;
	char *s=(char*)data;
	char *d;
	S_TESTRAMDRV *p=(S_TESTRAMDRV *)(driver->user_ptr);

	DEBPR2("wr %08x %04x\n",sector,cnt)

	if (gl_errors[TESTDRV_WR]) /* check if error is set */
	{
		DEBPR0("failed\n");
		gl_errors[TESTDRV_WR]--; /* decrease counter */
		return TEST_RAM_ERR_SECTOR; /* generate error */
	}

#if RAM_TEST_8GB_DRIVE
	if (sector>=p->maxsector) return 0; /* don't use any ram of simulated 8GB drive */
#else
	if (sector>=p->maxsector) return TEST_RAM_ERR_SECTOR;
#endif

	d=p->ramdrv;
	d+=sector*F_DEF_SECTOR_SIZE;
	len=F_DEF_SECTOR_SIZE*(long)cnt;

#if MEM_LONG_ACCESS
	/* check if size, destination and source is aligned to 32bit access */
	if ((!(len&3)) && (!(((long)d)&3)) && (!(((long)s)&3)))
	{
		long *dd=(long*)d;
		long *ss=(long*)s;
		len>>=2;
		while (len--)
		{
			*dd++=*ss++;
		}
		return TEST_RAM_NO_ERROR;
	}
#endif

	while (len--)
	{
		*d++=*s++;
	}

	return TEST_RAM_NO_ERROR;
}

/****************************************************************************
 *
 * test_ram_writesector
 *
 * write sectors into ramdrive
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

static int test_ram_writesector(F_DRIVER *driver,void *data, unsigned long sector)
{
	return test_ram_writemultiplesector(driver,data,sector,1);
}

/****************************************************************************
 *
 * test_ram_getphy
 *
 * determinate ramdrive physicals
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

static int test_ram_getphy(F_DRIVER *driver,F_PHY *phy)
{
	S_TESTRAMDRV *p=(S_TESTRAMDRV *)(driver->user_ptr);

#if RAM_TEST_8GB_DRIVE
	phy->number_of_sectors=16777216; /* simulated 8GB sector numbers */
#else
	phy->number_of_sectors=p->maxsector;
#endif

	p=p; /* just for Lint that 'p' must be used */

	return TEST_RAM_NO_ERROR;
}

/****************************************************************************
 *
 * test_ram_release
 *
 * release a drive
 *
 * INPUTS
 *
 * driver - driver structure
 *
 ***************************************************************************/

static void test_ram_release (F_DRIVER *driver)
{
	S_TESTRAMDRV *p=(S_TESTRAMDRV *)(driver->user_ptr);

	if (p>=g_ramdrv && p<g_ramdrv+RAMDRV_CNT)
	{
		p->use=0;
	}
}

/****************************************************************************
 *
 * test_ram_getstatus
 *
 * returns if missing or changed media. In this test driver this is only
 * for test purpose
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * return with state
 *
 ***************************************************************************/

static long test_ram_getstatus(F_DRIVER *driver)
{
	if (gl_errors[TESTDRV_STATE]) /* check if error is set */
	{
		DEBPR0("failed\n");
		gl_errors[TESTDRV_STATE]--;			/* decrease counter */
		return F_ST_MISSING|F_ST_CHANGED;	/* generate error */
	}

	return 0; /* state ok */
}

/****************************************************************************
 *
 * f_testramdrvinit
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

F_DRIVER *f_testramdrvinit(unsigned long driver_param)
{
	S_TESTRAMDRV *p;
	unsigned int i;

	if (driver_param==F_AUTO_ASSIGN)
	{
		p=g_ramdrv;
		for (i=0;i<RAMDRV_CNT;i++,p++)
		{
			if (!p->use) break;
		}
	}
	else
	{
		p=&g_ramdrv[driver_param];
	}

	if (p<g_ramdrv || p>=g_ramdrv+RAMDRV_CNT) return 0;
	if (p->use) return 0;

	p->driver->readsector=test_ram_readsector;
	p->driver->writesector=test_ram_writesector;
	p->driver->readmultiplesector=test_ram_readmultiplesector;
	p->driver->writemultiplesector=test_ram_writemultiplesector;
	p->driver->getphy=test_ram_getphy;
	p->driver->getstatus=test_ram_getstatus;
	p->driver->release=test_ram_release;
	p->driver->user_ptr=p;
	p->use=1;

	return p->driver;
}

void testdrv_set(int errnum, int errcou)
{
	gl_errors[errnum]=errcou;
}

/******************************************************************************
 *
 *  End of testdrv.c
 *
 *****************************************************************************/

