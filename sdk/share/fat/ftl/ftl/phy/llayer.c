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

/****************************************************************************
 *
 * LLAYER for PC simulation, devices for max 1GB
 *
 ***************************************************************************/

#include "llayer.h"
#if _HCC_MDEBUG_ENABLED_
#include "../../../src_int/prgmacro.h"
#endif
#include "../mdebug.h"
#include <string.h>
#include <stdlib.h>

#define SMEDIA_NANDFLASH_DRIVER 1

#if SMEDIA_NANDFLASH_DRIVER
#include "nandflash/nfdrv.h"
#include "mmp_types.h"

#endif

#if !SMEDIA_NANDFLASH_DRIVER

#define MAX_DLEN (gl_pagesize + MAX_SPARE_SIZE)
#define MAX_BLOCK_SIZE (MAX_DLEN*gl_pageperblock)
#define MAX_SIZE (gl_numofblocks * MAX_BLOCK_SIZE)

int allocated=0;
static unsigned char *gl_data=0; /* [MAX_SIZE*MLAYER_SETS];	*/
static unsigned long *gl_wear=0; /* [MAX_BLOCK*MLAYER_SETS];*/

static int onceinit=0;
#endif

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
#if !SMEDIA_NANDFLASH_DRIVER
	gl_numofblocks=1024;//2048;//1024;//100;//2048;//16384;//1024;
	gl_pageperblock=64;//32;//64;//32;
	gl_pagesize=512; //4096;//2048;//512;//2048;//512;

	gl_free_block=MAX_FREE_BLOCK_AVAILABLE;
	gl_log_block=MAX_LOG_BLOCK_AVAILABLE;

	if (!allocated) 
	{
		gl_data=malloc(MAX_SIZE);
		allocated=1;
	}
	if (!gl_wear) 
	{
		gl_wear=malloc(sizeof(long)*gl_numofblocks);
	}
	if ((!gl_data) || (!gl_wear)) return 1;

	if (!onceinit) 
	{
		(void)memset(gl_data,0xff,MAX_SIZE);
		onceinit=1;
	}

	return 0;
#else
	MMP_UINT8 result;

	gl_free_block=MAX_FREE_BLOCK_AVAILABLE;
	gl_log_block=MAX_LOG_BLOCK_AVAILABLE;
	
	result = mmpNFInitialize(&gl_numofblocks, &gl_pageperblock, &gl_pagesize);

    return result;
#endif
}

/****************************************************************************
 *
 * ll_getwear
 *
 * subfunction for windows, displaying wear information
 *
 * INPUTS
 *
 * RETURNS
 *
 ***************************************************************************/

void ll_getwear(t_ba blk, unsigned long *dest) 
{
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long pos=blk;

	if (!gl_data) 
	{
		*dest=0;
		return;
	}

	pos*=MAX_BLOCK_SIZE;

	*dest=GET_SPARE_AREA(&gl_data[pos])->wear;
#else
#endif
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
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long pos=pba;
	DEBPR2("ll_erase pba %d, %08x\n",pba,pba);

	if (pba>=gl_numofblocks) return LL_ERROR;

	pos*=MAX_BLOCK_SIZE;

	(void)memset(gl_data+pos,0xff,MAX_BLOCK_SIZE);
#if _HCC_MDEBUG_ENABLED_
	cnt_increase(CNT_TERASES);
#endif
	return LL_OK;
#else
    return (mmpNFErase(pba));
#endif
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

unsigned char ll_write(t_ba pba,t_po ppo, unsigned char *buffer, unsigned char *sparebuffer) 
{
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long page=ppo;
	unsigned long pos=pba;
	unsigned char *dest;
	unsigned long a;

	dest=gl_data;
	DEBPR3("ll_write pba %d, %08x ppo %d\n",pba,pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	pos*=MAX_BLOCK_SIZE;
	page*=MAX_DLEN;

	dest+=(pos+page);

	if (!ppo) 
	{
		unsigned long newwear=GET_SPARE_AREA(buffer)->wear;
		if (gl_wear[pba]+1!=newwear) 
		{
		 	a=10;
		}
		gl_wear[pba]=newwear;
	}

	for (a=0; a<gl_pagesize; a++) 
	{
		char ch=*dest;
		char ch2=*buffer++;
		*dest++=(unsigned char)(ch&ch2);
	}

	for (a=0; a<MAX_SPARE_SIZE; a++) 
	{
		char ch=*dest;
		char ch2=*sparebuffer++;
		*dest++=(unsigned char)(ch&ch2);
	}


	return LL_OK;
#else
    return mmpNFWrite(pba, ppo, buffer, sparebuffer);
#endif
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
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long page=ppo;
	unsigned long pos=pba;
	unsigned char *dest;
	unsigned long a;

	dest=gl_data;
	DEBPR3("ll_writedouble pba %d, %08x ppo %d\n",pba,pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	pos*=MAX_BLOCK_SIZE;
	page*=MAX_DLEN;

	dest+=(pos+page);

	for (a=0; a<gl_pagesize/2; a++) 
	{
		char ch=*dest;
		char ch2=*buffer0++;
		*dest++=(unsigned char)(ch&ch2);
	}

	for (; a<MAX_DLEN; a++) 
	{
		char ch=*dest;
		char ch2=*buffer1++;
		*dest++=(unsigned char)(ch&ch2);
	}


	if (!ppo) 
	{
		unsigned long newwear=GET_SPARE_AREA((gl_data+pos+page))->wear;
		if (gl_wear[pba]+1!=newwear) 
		{
		 	a=10;
		}
		gl_wear[pba]=newwear;
	}

	return LL_OK;
#else
    return (mmpNFWriteDouble(pba, ppo, buffer0, buffer1));
#endif
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
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long page=ppo;
	unsigned long pos=pba;
	unsigned char *sou;
	unsigned long a;
	char iserased=1;

	sou=gl_data;

	DEBPR3("ll_read pba %d, %08x ppo %d\n",pba,pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	pos*=MAX_BLOCK_SIZE;
	page*=MAX_DLEN;

	sou+=(pos+page);

	for (a=0; a<MAX_DLEN; a++) 
	{ /* we should go throught on all data+spare*/
		unsigned char ch=*sou++;
		if (ch!=0xff) iserased=0; /* simple erase chk */

		if (buffer) *buffer++=ch; /* if there is given buffer then store the data into it */
	}

	if (iserased) return LL_ERASED;	 /* if flasg is still set, returns with erased page */

	return LL_OK;
#else
    return mmpNFRead(pba, ppo, buffer);
#endif
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
 *         LL_RP_DATA-data only
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
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long page=ppo;
	unsigned long pos=pba;
	unsigned char *sou;
	unsigned long a;
	char iserased=1;

	sou=gl_data;

	DEBPR3("ll_readpart pba %d, %08x ppo %d\n",pba,pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	pos*=MAX_BLOCK_SIZE;
	page*=MAX_DLEN;

	sou+=(pos+page);

	for (a=0; a<MAX_DLEN; a++) 
	{ /* we should go throught on all data+spare*/
		unsigned char ch=*sou++;
		if (ch!=0xff) iserased=0; /* simple erase chk */

		if (index==LL_RP_1STHALF) 
		{ /* if 1st half is requested */
			if (a<gl_pagesize/2) *buffer++=ch;
		}
		else if (index==LL_RP_2NDHALF) 
		{ /* 2nd half is requested */
			if (a>=gl_pagesize/2 && a<gl_pagesize) *buffer++=ch;
		}
		else if (index==LL_RP_DATA) 
		{
			if (a<gl_pagesize) *buffer++=ch;
		}
		else if (index==LL_RP_SPARE) 
		{ /* spare only is requested */
			if (a>=gl_pagesize) *buffer++=ch;
		}
		else return LL_ERROR;
	}

	if (iserased) return LL_ERASED;	 /* if flasg is still set, returns with erased page */

	return LL_OK;
#else
    return mmpNFReadPart(pba, ppo, buffer, index);
#endif
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
#if !SMEDIA_NANDFLASH_DRIVER
	/* in the simulation we simulate only 2 badblock 10 and 200 only for test purpose */
	if (pba==10 || pba==200) return 1; /* signal as bad block */

	return 0; /* no bad block signalled */
#else
    return mmpNFIsBadBlock(pba);
#endif
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

unsigned char ll_readonebyte(t_ba pba, t_po ppo, unsigned char sparepos,unsigned char *ch) 
{
#if !SMEDIA_NANDFLASH_DRIVER
	unsigned long page=ppo;
	unsigned long pos=pba;
	unsigned char *sou;

	sou=gl_data;

/*	DEBPR3("ll_readonebyte pba %d, %08x ppo %d\n",pba,pba,ppo); */

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	pos*=MAX_BLOCK_SIZE;
	page*=MAX_DLEN;

	sou+=(pos+page);

	sou+=gl_pagesize; /* goto spare area*/
	sou+=sparepos;	  /* jump tu position in spare */

	*ch=*sou; /* store 1 character, no ECC required */

	return LL_OK;
#else
    return mmpNFReadOneByte(pba, ppo, sparepos, ch);
#endif
}

/****************************************************************************
 *
 * end of llayer.c
 *
 ***************************************************************************/
