/****************************************************************************
 *
 *            Copyright (c) 2006-2008 by HCC Embedded
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

/****************************************************************************
 *
 * Includes
 *
 ***************************************************************************/

#include "spi.h"
#include "adf.h"
#include "../mlayer.h"
#include <string.h>
#if _HCC_MDEBUG_ENABLED_
#include "../../../src_int/prgmacro.h"
#endif
#include "../mdebug.h"

/****************************************************************************
 *
 * Structure
 *
 ***************************************************************************/

typedef struct 
{
	unsigned char flash[ADF_WHOLE_PAGE_SIZE*ADF_REAL_PAGE_COUNT];
	unsigned char buf1[ADF_WHOLE_PAGE_SIZE];
	unsigned char buf2[ADF_WHOLE_PAGE_SIZE];
	unsigned char cmd;
	int cnt,addr_cnt;
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long all_addr;
	unsigned long *wearacc;
} M_DEV;

/****************************************************************************
 *
 * Static variables
 *
 ***************************************************************************/

static M_DEV m_devs[ADF_NUM_OF_DEVICES];
static M_DEV *dev_sel=0;

static unsigned long gl_wearacc[ADF_MAX_BLOCK];
static int initonce=0;

/****************************************************************************
 *
 * ll_getwear
 *
 * function for windows based test enviroment for getting wear of a block
 *
 * INPUTS
 *
 * pba - physical block address
 * dest - where to store wear information
 *
 ***************************************************************************/

void ll_getwear(t_ba pba, unsigned long *dest) 
{
	*dest=gl_wearacc[pba];
}

/****************************************************************************
 *
 * myassert
 *
 * If program found fatal error, then it calls this function
 *
 * INPUTS
 *
 * s - contains the string of the error
 *
 ***************************************************************************/

static void myassert(char *s) 
{
	for (;;);
}

/****************************************************************************
 *
 * spi_init
 *
 * initialize simulated spi
 *
 ***************************************************************************/

void spi_init() 
{
	int a;
	dev_sel=0;

	if (!initonce) 
	{
		(void)memset(gl_wearacc,0xff,sizeof(gl_wearacc));
		for (a=0; a<ADF_NUM_OF_DEVICES; a++) 
		{
			M_DEV *dev=&m_devs[a];

			(void)memset(dev->flash,0xff,ADF_WHOLE_PAGE_SIZE*ADF_REAL_PAGE_COUNT);

			dev->wearacc=&gl_wearacc[ADF_MAX_BLOCK_PER_DEVICE*(long)a];
		}
		initonce=1;
	}
}

/****************************************************************************
 *
 * memset_l
 *
 * fast memory setting (32bit)
 *
 * INPUTS
 *
 * dest - what to fill
 * data - long data to be filled
 * size - size in bytes to fill
 *
 ***************************************************************************/

static void memset_l(void *dest, long data,long size) 
{
	long *d=(long*)dest;
	size>>=2;
	data=(long)(0xffffffffUL);
	while (size--) 
	{
		*d++=data;
	}
}


/****************************************************************************
 *
 * memcpy_l
 *
 * fast copy memory (32bits)
 *
 * INPUTS
 *
 * dest - destination address
 * sou - source address
 * size - number of bytes to copy
 *
 ***************************************************************************/

static void memcpy_l(void *dest, void *sou,long size) 
{
	long *d=(long*)dest;
	long *s=(long*)sou;
	size>>=2;
	while (size--) 
	{
		*d++=*s++;
	}
}


/****************************************************************************
 *
 * spi_rx_buffer
 *
 * Receive data into buffer
 *
 * INPUTS
 *
 * buffer - where to store data
 * read - number of bytes to read
 *
 ***************************************************************************/

void spi_rx_buffer(unsigned char *dst,int read) 
{
	long *dl=(long*)dst;
	long *sl=(long*)(&dev_sel->buf1[dev_sel->byte_addr]);
	unsigned char *sst;

	if (dev_sel->byte_addr+read>ADF_WHOLE_PAGE_SIZE) myassert("spi_fast_read_buf1");

	dev_sel->byte_addr+=read;

	while (read>3) 
	{
		*dl++=*sl++;
		read-=4;
	}

	if (!read) return;

	dst=(unsigned char*)dl;
	sst=(unsigned char*)sl;

	while (read--) 
	{
		*dst++=*sst++;
	}
}

/****************************************************************************
 *
 * spi_tx_buffer
 *
 * Send buffer to serial
 *
 * INPUTS
 *
 * buffer - datas what to send
 * write - number of bytes to write
 *
 ***************************************************************************/

void spi_tx_buffer(unsigned char *src,int write) 
{
	long *dl=(long*)(&dev_sel->buf1[dev_sel->byte_addr]);
	long *sl=(long*)src;
	unsigned char *sst;

	if (dev_sel->byte_addr+write>ADF_WHOLE_PAGE_SIZE) myassert("spi_fast_write_buf1");

	dev_sel->byte_addr+=write;

	while (write>3) 
	{
		*dl++=*sl++;
		write-=4;
	}

	if (!write) return;

	src=(unsigned char*)dl;
	sst=(unsigned char*)sl;

	while (write--) 
	{
		*src++=*sst++;
	}
}

/****************************************************************************
 *
 * spi_rx_8
 *
 * Receive 8 bits
 *
 * RETURNS
 *
 * received data
 *
 ***************************************************************************/

unsigned char spi_rx_8() 
{
	if (!dev_sel) 
	{
		myassert("spi_rx_8");
		return 0xff;
	}

	switch (dev_sel->cmd) 
	{
	case ADF_STATUS:
		return (ADF_ID<<2) | ADF_BREADY;

	case ADF_READ:
		{
			unsigned char ch=dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE+dev_sel->byte_addr];
			if (dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE+dev_sel->byte_addr > ADF_REAL_PAGE_COUNT*ADF_WHOLE_PAGE_SIZE) myassert("spi_rx_8 ADF_READ");
			dev_sel->byte_addr++;
			return ch;
		}

	case ADF_READ_BUF1:
		if (dev_sel->byte_addr>=ADF_WHOLE_PAGE_SIZE) myassert("spi_rx_8 ADF_READ_BUF1");
		return dev_sel->buf1[dev_sel->byte_addr++];
	case ADF_READ_BUF2:
		if (dev_sel->byte_addr>=ADF_WHOLE_PAGE_SIZE) myassert("spi_rx_8 ADF_READ_BUF2");
		return dev_sel->buf2[dev_sel->byte_addr++];

	default:
		return 0xff;
	}
}

/****************************************************************************
 *
 * spi_tx_8
 *
 * Send  8 bits on spi
 *
 * INPUTS
 *
 * data - send data to spi
 *
 ***************************************************************************/

#if ADF_PAGE_SIZE==256
#define GET_SPARE_AREA2(_buf_) ((ST_SPARE*)(((unsigned char*)(_buf_))-ADF_WHOLE_PAGE_SIZE+gl_pagesize))
#endif

void spi_tx_8(unsigned char data) 
{
	if (!dev_sel) 
	{
		myassert("spi_tx_8");
		return;
	}

	if (!dev_sel->cmd) 
	{
		dev_sel->cmd=data;
		dev_sel->cnt=0;
		dev_sel->all_addr=0;
		dev_sel->addr_cnt=0;
		return;
	}

	if (dev_sel->addr_cnt<3) 
	{
		dev_sel->addr_cnt++;
		dev_sel->all_addr<<=8;
		dev_sel->all_addr|=data;

		if (dev_sel->addr_cnt!=3) return;
		else 
		{
			dev_sel->page_addr=dev_sel->all_addr >> ADF_BYTE_ADDRESS_WIDTH;
			dev_sel->byte_addr=dev_sel->all_addr & ((1 << ADF_BYTE_ADDRESS_WIDTH) -1);
		}
	}

	switch (dev_sel->cmd) 
	{
	case ADF_STATUS:
			break;
	case ADF_READ:
			break;
	case ADF_READ_BUF1:
	case ADF_READ_BUF2:
			break;
	case ADF_WRITE_BUF1:
			if (dev_sel->byte_addr>=ADF_WHOLE_PAGE_SIZE) myassert("spi_tx_8 ADF_WRITE_BUF1");
			if (dev_sel->cnt) dev_sel->buf1[dev_sel->byte_addr++]=data;
			break;
	case ADF_WRITE_BUF2:
 			if (dev_sel->byte_addr>=ADF_WHOLE_PAGE_SIZE) myassert("spi_tx_8 ADF_WRITE_BUF2");
			if (dev_sel->cnt) dev_sel->buf2[dev_sel->byte_addr++]=data;
			break;
	case ADF_PROGERASE_BUF1:
			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_PROGERASE_BUF1");
			(void)memcpy_l(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],dev_sel->buf1,ADF_WHOLE_PAGE_SIZE);
			break;
	case ADF_PROGERASE_BUF2:
 			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_PROGERASE_BUF2");
			(void)memcpy_l(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],dev_sel->buf2,ADF_WHOLE_PAGE_SIZE);
			break;
	case ADF_PROG_BUF1:
			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_PROG_BUF1");
			(void)memcpy_l(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],dev_sel->buf1,ADF_WHOLE_PAGE_SIZE);

			if ((dev_sel->page_addr % ADF_PAGES_PER_BLOCK)==0) 
			{
#if ADF_PAGE_SIZE==512
				dev_sel->wearacc[dev_sel->page_addr/ADF_PAGES_PER_BLOCK]=
				GET_SPARE_AREA(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE])->wear;
#else
				if ((dev_sel->page_addr/ADF_PAGES_PER_BLOCK) & 1) 
				{
					dev_sel->wearacc[dev_sel->page_addr/ADF_PAGES_PER_BLOCK/2]=
					GET_SPARE_AREA2(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE])->wear;
				}
#endif
			}

			break;

	case ADF_PROG_BUF2:
 			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_PROGERASE_BUF2");
			(void)memcpy_l(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],dev_sel->buf2,ADF_WHOLE_PAGE_SIZE);

			if ((dev_sel->page_addr % ADF_PAGES_PER_BLOCK)==0) 
			{
#if ADF_PAGE_SIZE==512
				dev_sel->wearacc[dev_sel->page_addr/ADF_PAGES_PER_BLOCK]=
				GET_SPARE_AREA(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE])->wear;
#else
				if ((dev_sel->page_addr/ADF_PAGES_PER_BLOCK) & 1) 
				{
					dev_sel->wearacc[dev_sel->page_addr/ADF_PAGES_PER_BLOCK/2]=
					GET_SPARE_AREA2(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE])->wear;
				}
#endif
			}

			break;

	case ADF_ERASE_PAGE:
 			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_ERASE_PAGE");
			(void)memset_l(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],0xff,ADF_WHOLE_PAGE_SIZE);
			break;

	case ADF_ERASE_BLOCK:
 			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_ERASE_BLOCK");
			(void)memset_l(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],0xff,ADF_WHOLE_PAGE_SIZE*ADF_PAGES_PER_BLOCK);

			if ((dev_sel->page_addr % ADF_PAGES_PER_BLOCK)==0) 
			{
#if ADF_PAGE_SIZE==512
				dev_sel->wearacc[dev_sel->page_addr/ADF_PAGES_PER_BLOCK]=
				GET_SPARE_AREA(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE])->wear;
#else
				if ((dev_sel->page_addr/ADF_PAGES_PER_BLOCK) & 1) 
				{
					dev_sel->wearacc[dev_sel->page_addr/ADF_PAGES_PER_BLOCK/2]=
					GET_SPARE_AREA2(&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE])->wear;
				}
#endif
			}

#if _HCC_MDEBUG_ENABLED_
	cnt_increase(CNT_TERASES);
#endif
			break;
	case ADF_READ_MAIN2BUF1:
 			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_READ_MAIN2BUF1");
			(void)memcpy_l(dev_sel->buf1,&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],ADF_WHOLE_PAGE_SIZE);
			break;
	case ADF_READ_MAIN2BUF2:
 			if (dev_sel->page_addr>=ADF_REAL_PAGE_COUNT) myassert("spi_tx_8 ADF_READ_MAIN2BUF2");
			(void)memcpy_l(dev_sel->buf2,&dev_sel->flash[dev_sel->page_addr*ADF_WHOLE_PAGE_SIZE],ADF_WHOLE_PAGE_SIZE);
			break;
	default:
		return;
	}

	dev_sel->cnt++;
}

/****************************************************************************
 *
 * spi_cs_lo
 *
 * setting given chip select to low
 *
 * INPUTS
 *
 * cs - chip select number
 *
 ***************************************************************************/

void spi_cs_lo(unsigned char cs) 
{
	if (cs<ADF_NUM_OF_DEVICES) 
	{
		dev_sel=&m_devs[cs];
		dev_sel->cmd=0;
	}
	else 
	{
		dev_sel=0;
		myassert("spi_cs_lo: invalid cs");
	}
}

/****************************************************************************
 *
 * spi_cs_hi
 *
 * setting given chip select to high
 *
 * INPUTS
 *
 * cs - chip select number
 *
 ***************************************************************************/

void spi_cs_hi(unsigned char cs) 
{
	if (cs<ADF_NUM_OF_DEVICES) 
	{
		m_devs[cs].cmd=0;
	}
	else 
	{
		myassert("spi_cs_hi: invalid cs");
	}
	dev_sel=0;
}

/****************************************************************************
 *
 * end of spi_pc.c
 *
 ***************************************************************************/

