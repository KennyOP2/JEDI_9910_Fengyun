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
 * Interleaving Atmel DataFlash driver for FTL
 * Device page size is 512+16
 *
 ***************************************************************************/

/****************************************************************************
 *
 *	Includes
 *
 ***************************************************************************/

#include "adf.h"
#include "spi.h"
#include "llayer.h"

#if _HCC_MDEBUG_ENABLED_
#include "../../../src_int/prgmacro.h"
#endif
#include "../mdebug.h"

/****************************************************************************
 *
 *	This source code is for devices only which has 512 bytes per pages
 *
 ***************************************************************************/

#if ADF_PAGE_SIZE==512
#if ADF_NUM_OF_DEVICES&1
#error Number of devices has to be even for using of Interleaving
#endif

/****************************************************************************
 *
 * Static variables
 *
 ***************************************************************************/

static unsigned char mg_bwaitpending[ADF_NUM_OF_DEVICES];	/* information about wait request */

/****************************************************************************
 *
 * adf_cmd
 *
 * Will send a command and the address bytes trough the SPI to the flash
 * chip.
 *
 * INPUTS
 *
 * cmd - command byte to be sent
 * addr - address
 *
 * RETURNS
 *
 * none.
 *
 ***************************************************************************/

static void adf_cmd (unsigned char cmd, unsigned long addr)
{
	spi_tx_8(cmd);
	spi_tx_8((unsigned char)(addr>>16));
	spi_tx_8((unsigned char)(addr>>8));
	spi_tx_8((unsigned char)(addr));
}

/****************************************************************************
 *
 * adf_wait_ready
 *
 * Wait till the flash chip finiseh all operations (exits busy state).
 * It also controls chip selects
 *
 * INPUTS
 *
 * cs - current chip select
 *
 * RETURNS
 *
 * 0: if all ok
 * 1: if timeout passed
 *
 ***************************************************************************/

static unsigned char adf_wait_ready (unsigned char cs)
{
	unsigned char status;
	unsigned int tmout=60000;

	spi_cs_lo(cs);
	spi_tx_8(ADF_STATUS);

	do
	{
		status=spi_rx_8();

		if (!tmout--)
		{
			spi_cs_hi(cs);
			return 1;
		}
	} while (!(status & ADF_BREADY));

	spi_cs_hi(cs);

	return 0;
}

/****************************************************************************
 *
 * adf_wait_ready_request
 *
 * set a wait request to a chip select
 *
 * INPUTS
 *
 * cs - current chip select
 *
 ***************************************************************************/

static void adf_wait_ready_request(unsigned char cs) 
{
	mg_bwaitpending[cs]=1;
}

/****************************************************************************
 *
 * adf_wait_ready_check
 *
 * It check if wait was requested and then it waits until ready or TimeOut
 *
 * INPUTS
 *
 * cs - current chip select
 *
 * RETURNS
 *
 * 0: if all ok
 * 1: if timeout passed
 *
 ***************************************************************************/

static unsigned char adf_wait_ready_check(unsigned char cs)
{
	if (!mg_bwaitpending[cs]) return 0; /* if request is not pending return */

	mg_bwaitpending[cs]=0; /* remove wait pending */

	return adf_wait_ready(cs);
}

/****************************************************************************
 *
 * ll_init
 *
 * low level init function, this is called from mlayer lowinit function once
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * other if any error
 *
 ***************************************************************************/

unsigned char ll_init()
{
	unsigned char cs;
	DEBPR0("ll_init\n");

	gl_numofblocks=ADF_MAX_BLOCK >> 1;		  /* interleave half the block num      */
	gl_pageperblock=ADF_PAGES_PER_BLOCK << 1; /* and duplicate the pages per block  */
											  /* so the flash size will be the same */
	gl_pagesize=ADF_PAGE_SIZE;

	spi_init(); /* Call serial initialization */


	for (cs=0; cs<ADF_NUM_OF_DEVICES; cs++)
	{ /* Check ADF_IDs for all devices */
		unsigned char id;

		mg_bwaitpending[cs]=0; /* Clear wait pending bits */

		/* Read and check the type of the FLASH chip. */
		spi_cs_lo(cs);
		spi_tx_8(ADF_STATUS);
		id=spi_rx_8();
		spi_cs_hi(cs);

		if (((id>>2) & 0x0fu) != ADF_ID) return LL_ERROR;

		if (adf_wait_ready(cs)) return LL_ERROR;
	}

	return LL_OK;
}


/****************************************************************************
 *
 * ll_erase
 *
 * erase a block
 *
 * INPUTS
 *
 * pba - physical block address
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_erase(t_ba pba)
{
	unsigned char cs=(unsigned char)(pba/ADF_MAX_BLOCK_PER_DEVICE);
	unsigned long addr=(unsigned long)pba % ADF_MAX_BLOCK_PER_DEVICE;

	DEBPR2("ll_erase pba %d, %08x\n",pba,pba);

	addr <<=  ADF_BLOCK_ADDRESS_WIDTH;

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_ERASE_BLOCK, addr );
	spi_cs_hi(cs);
	adf_wait_ready_request(cs);

	cs+=ADF_NUM_OF_DEVICES >> 1; /* erase also its pair */

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_ERASE_BLOCK, addr );
	spi_cs_hi(cs);
	adf_wait_ready_request(cs);

	return LL_OK;
}

/****************************************************************************
 *
 * ll_write
 *
 * write a page
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - page data to be written 
 * sparebuffer - spare data to be written
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_write(t_ba pba,t_po ppo,unsigned char *buffer,unsigned char *sparebuffer)
{
	unsigned char cs=(unsigned char)(pba/ADF_MAX_BLOCK_PER_DEVICE);
	unsigned long addr=(unsigned long)pba % ADF_MAX_BLOCK_PER_DEVICE;

	DEBPR3("ll_write pba %d, %08x ppo %d\n",pba,pba,ppo);

	addr <<=  ADF_BLOCK_ADDRESS_WIDTH;
	addr += ((unsigned long)ppo) << (ADF_BYTE_ADDRESS_WIDTH - 1);

	if (ppo&1) cs+=ADF_NUM_OF_DEVICES >> 1; /* do interleaving */

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_WRITE_BUF1, 0);
	spi_tx_buffer(buffer,(int)(gl_pagesize));
	spi_tx_buffer(sparebuffer,(int)(MAX_SPARE_SIZE));
	spi_cs_hi(cs);

	spi_cs_lo(cs);
	adf_cmd(ADF_PROG_BUF1, addr);
	spi_cs_hi(cs);

	adf_wait_ready_request(cs);

	return LL_OK;
}

/****************************************************************************
 *
 * ll_writedouble
 *
 * write a page from 2 buffers
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - 1st half of page data to be written
 * buffer - 2nd half of page data + spare data to be written
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_writedouble(t_ba pba,t_po ppo, unsigned  char *buffer0,unsigned  char *buffer1)
{
	unsigned char cs=(unsigned char)(pba/ADF_MAX_BLOCK_PER_DEVICE);
	unsigned long addr=(unsigned long)pba % ADF_MAX_BLOCK_PER_DEVICE;

	DEBPR3("ll_writedouble pba %d, %08x ppo %d\n",pba,pba,ppo);

	addr <<=  ADF_BLOCK_ADDRESS_WIDTH;
	addr += ((unsigned long)ppo) << (ADF_BYTE_ADDRESS_WIDTH - 1);

	if (ppo&1) cs+=ADF_NUM_OF_DEVICES >> 1; /* do interleaving */

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_WRITE_BUF1, 0);
	spi_tx_buffer(buffer0,(int)(gl_pagesize/2));
	spi_tx_buffer(buffer1,(int)(gl_pagesize/2+MAX_SPARE_SIZE));
	spi_cs_hi(cs);

	spi_cs_lo(cs);
	adf_cmd(ADF_PROG_BUF1, addr);
	spi_cs_hi(cs);

	adf_wait_ready_request(cs);

	return LL_OK;
}

/****************************************************************************
 *
 * ll_read
 *
 * read a page
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - page data pointer where to store data (data+spare)
 *          if this data is zero (null) then there is no physical storing
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERASED - if page is empty
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_read(t_ba pba,t_po ppo, unsigned char *buffer)
{
	unsigned char cs=(unsigned char)(pba/ADF_MAX_BLOCK_PER_DEVICE);
	unsigned long addr=(unsigned long)pba % ADF_MAX_BLOCK_PER_DEVICE;
	int cou;

	DEBPR3("ll_read pba %d, %08x ppo %d\n",pba,pba,ppo);

	addr <<=  ADF_BLOCK_ADDRESS_WIDTH;
	addr += ((unsigned long)ppo) << (ADF_BYTE_ADDRESS_WIDTH - 1);

	if (ppo&1) cs+=ADF_NUM_OF_DEVICES >> 1; /* do interleaving */

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_READ_MAIN2BUF1,addr);
	spi_cs_hi(cs);
	if (adf_wait_ready(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_READ_BUF1,0);
	spi_tx_8(0xffu);
	spi_rx_buffer(buffer,(int)(gl_pagesize+MAX_SPARE_SIZE));
	spi_cs_hi(cs);;

	/* checking erased status */
	cou=(int)(gl_pagesize+MAX_SPARE_SIZE);
	while (cou--)
	{
		if (*buffer++!=0xff) return LL_OK;
	}

	return LL_ERASED;
}

/****************************************************************************
 *
 * ll_readpart
 *
 * readpart a page
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * buffer - page data pointer where to store data
 * index - which part need to be stored
 *         LL_RP_1STHALF-1st half
 *         LL_RP_2NDHALF-2nd half
 *         LL_RP_SPARE-spare only
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERASED - if page is empty
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_readpart(t_ba pba,t_po ppo, unsigned char *buffer,unsigned char index)
{
	unsigned char cs=(unsigned char)(pba/ADF_MAX_BLOCK_PER_DEVICE);
	unsigned long addr=(unsigned long)pba % ADF_MAX_BLOCK_PER_DEVICE;
	unsigned char iserased=1;
	int cou;

	DEBPR3("ll_readpart pba %d, %08x ppo %d\n",pba,pba,ppo);

	addr <<=  ADF_BLOCK_ADDRESS_WIDTH;
	addr += ((unsigned long)ppo) << (ADF_BYTE_ADDRESS_WIDTH - 1);

	if (ppo&1) cs+=ADF_NUM_OF_DEVICES >> 1; /* do interleaving */

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_READ_MAIN2BUF1,addr);
	spi_cs_hi(cs);

	if (adf_wait_ready(cs)) return LL_ERROR;

	if (index==LL_RP_1STHALF)
	{
		spi_cs_lo(cs);
		adf_cmd(ADF_READ_BUF1,0);
		spi_tx_8(0xffu);

		cou=(int)(gl_pagesize/2);
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
			*buffer++=ch;
		}
		cou=(int)(gl_pagesize/2+MAX_SPARE_SIZE);
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
		}

	    spi_cs_hi(cs);;
	}
	else if (index==LL_RP_2NDHALF)
	{
		spi_cs_lo(cs);
		adf_cmd(ADF_READ_BUF1,0);
		spi_tx_8(0xffu);

		cou=(int)(gl_pagesize/2);
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
		}
		cou=(int)(gl_pagesize/2);
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
			*buffer++=ch;
		}

		cou=MAX_SPARE_SIZE;
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
		}

		spi_cs_hi(cs);;
	}
	else if (index==LL_RP_SPARE)
	{
		spi_cs_lo(cs);
		adf_cmd(ADF_READ_BUF1,0);
		spi_tx_8(0xffu);

		cou=(int)(gl_pagesize);
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
		}

		cou=MAX_SPARE_SIZE;
		while(cou--)
		{
			unsigned char ch=spi_rx_8();
			if (ch!=0xff) iserased=0;
			*buffer++=ch;
		}

		spi_cs_hi(cs);;
	}
	else return LL_ERROR;

	if (iserased) return LL_ERASED;

	return LL_OK;
}

/****************************************************************************
 *
 * ll_isbadblock
 *
 * check if a block is manufactured as bad block
 *
 * INPUTS
 *
 * pba - physical block address
 *
 * RETURNS
 *
 * 0 - if not a bad block
 * any value (1) if it is a bad block
 *
 ***************************************************************************/

unsigned char ll_isbadblock(t_ba pba)
{
	return 0;  /* no bad block here */
}

/****************************************************************************
 *
 * ll_readonebyte
 *
 * readone byte from spare, NO ECC required, only read raw character from
 * spare from the given position.
 *
 * INPUTS
 *
 * pba - physical block address
 * ppo - physical page offset
 * sparepos - character position started in spare area
 * ch - where to put read character
 *
 * RETURNS
 *
 * LL_OK - if successfuly
 * LL_ERROR - if any error
 *
 ***************************************************************************/

unsigned char ll_readonebyte(t_ba pba, t_po ppo, unsigned char sparepos, unsigned char *ch)
{
	unsigned char cs=(unsigned char)(pba/ADF_MAX_BLOCK_PER_DEVICE);
	unsigned long addr=(unsigned long)pba % ADF_MAX_BLOCK_PER_DEVICE;

	addr <<=  ADF_BLOCK_ADDRESS_WIDTH;
	addr += ((unsigned long)ppo) << (ADF_BYTE_ADDRESS_WIDTH - 1);

	if (ppo&1) cs+=ADF_NUM_OF_DEVICES >> 1; /* do interleaving */

	if (adf_wait_ready_check(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_READ_MAIN2BUF1,addr);
	spi_cs_hi(cs);

	if (adf_wait_ready(cs)) return LL_ERROR;

	spi_cs_lo(cs);
	adf_cmd(ADF_READ_BUF1,gl_pagesize+sparepos);
	spi_tx_8(0xffu);
	*ch=spi_rx_8();
	spi_cs_hi(cs);;

	return LL_OK;
}

/****************************************************************************
 *
 *	end of page size checking
 *
 ***************************************************************************/

#endif /* ADF_PAGE_SIZE==512 */

/****************************************************************************
 *
 *	end of adf.c
 *
 ***************************************************************************/

