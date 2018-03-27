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
 * LLAYER_F for PC simulation, bigger device than 1GB, this uses harddisk for
 * manipulation of blocks
 *
 ***************************************************************************/

#include "llayer.h"
#if _HCC_MDEBUG_ENABLED_
#include "../../../src_int/prgmacro.h"
#endif
#include "../mdebug.h"
#include <string.h>
#include <stdlib.h>

#include <direct.h>
#include <stdio.h>

#define MAX_DLEN (gl_pagesize + MAX_SPARE_SIZE)
#define MAX_BLOCK_SIZE (MAX_DLEN*gl_pageperblock)
#define MAX_SIZE (gl_numofblocks * MAX_BLOCK_SIZE)

/****************************************************************************
 *
 * definitions and structures for llcache
 *
 ***************************************************************************/

typedef struct 
{
	unsigned char modified;
	t_ba pba; /* which pba is it */
	unsigned char *dbuff; /* data buffer */
} ST_LLCACHE;

#define MAX_LLCACHE 512

static ST_LLCACHE gl_llcache[MAX_LLCACHE];
static long llcache_idx=0;
static int onceinit=0;

static unsigned long *gl_wear=0; /* [MAX_BLOCK*MLAYER_SETS];*/
static unsigned long *gl_wearacc=0; /* [MAX_BLOCK*MLAYER_SETS];*/
static unsigned char *gl_fileexist=0;


static char filename[32];
static char *dirname="c:\\blks";

/****************************************************************************
 *
 * init_llcache
 *
 * initialize cache
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static unsigned char init_llcache()
{
	long a;

	for (a=0; a<MAX_LLCACHE; a++)
	{
		gl_llcache[a].pba=BLK_NA;
		gl_llcache[a].modified=0;

		gl_llcache[a].dbuff=malloc(MAX_BLOCK_SIZE);
		if (gl_llcache[a].dbuff == 0) return 1;
	}

	return 0;
}


/****************************************************************************
 *
 * blockfilename
 *
 * retreive file name from pba
 *
 * INPUTS
 *
 * pba - physical block address
 *
 * RETURNS
 *
 * filename
 *
 ***************************************************************************/

static char *blockfilename(t_ba pba) 
{
	(void)sprintf (filename,"%s\\%08x.blk",dirname,pba);
	return filename;
}

/****************************************************************************
 *
 * cfopen
 *
 * open a file but, it checks internally if it exist without calling fopen
 *
 * INPUTS
 *
 * pba - physical block address
 * mode - file mode to be opened
 *
 * RETURNS
 *
 * 0 - if not exist
 * FILE pointer
 *
 ***************************************************************************/

static FILE *cfopen(t_ba pba, char *mode) 
{
	if (!gl_fileexist[pba]) return 0; /* not exist */
	return fopen(blockfilename(pba),mode);
}

/****************************************************************************
 *
 * find_llcache
 *
 * find a pba if it is cached
 *
 * INPUTS
 *
 * pba - physical block address
 *
 * RETURNS
 *
 * 0 - if not found
 * LL_CACHE entry
 *
 ***************************************************************************/

static ST_LLCACHE *find_llcache(t_ba pba) 
{
	ST_LLCACHE *l=gl_llcache;
	int a;

	for (a=0; a<MAX_LLCACHE; a++,l++) 
	{
		if (l->pba==pba) 
		{
			return l;
		}
	}
	return 0;
}

/****************************************************************************
 *
 * loadblk_llcache
 *
 * funtion to load block from file
 *
 * INPUTS
 *
 * l - cache entry where to load and which pba
 *
 ***************************************************************************/

static void loadblk_llcache(ST_LLCACHE *l) 
{
	FILE *file=cfopen(l->pba,"rb");
	 /* try to read from file */

	if (file) 
	{
		(void)fread(l->dbuff,1,MAX_BLOCK_SIZE,file);
		(void)fclose(file);
	}
	else 
	{
		(void)memset (l->dbuff,0xff,MAX_BLOCK_SIZE);
	}

}

/****************************************************************************
 *
 * saveblk_llcache
 *
 * save a cached entry into file
 *
 * INPUTS
 *
 * l - cache entry which contains the buffer and the pba to write
 *
 * RETURNS
 *
 * 0 - if success
 * other if any problem
 *
 ***************************************************************************/

static unsigned char saveblk_llcache(ST_LLCACHE *l) 
{
	FILE *file=fopen(blockfilename(l->pba),"wb+");

	if (!file) return 1;

	(void)fwrite(l->dbuff,1,MAX_BLOCK_SIZE,file);
	(void)fclose(file);

	gl_fileexist[l->pba]=1; /* now it is exist on HDD */
	l->modified=0;

	return 0;
}

/****************************************************************************
 *
 * get_llcache
 *
 * get an llcache entry, this must be called when a buffer is needed for a pba
 *
 * INPUTS
 *
 * pba - which pba needs buffer
 *
 * RETURNS
 *
 * LLCACHE entry if success
 * 0 - if any error
 *
 ***************************************************************************/

static ST_LLCACHE *get_llcache(t_ba pba) 
{
	ST_LLCACHE *l=find_llcache(pba);	/* 1st check if it is in */
	int a;
	if (l) return l;				/* return if it is */

	for (a=0; a<MAX_LLCACHE+1; a++) 
	{	/* +1 because we might not found entry which is not modified */
		llcache_idx++;
		if (llcache_idx>=MAX_LLCACHE) llcache_idx=0;

		l=&gl_llcache[llcache_idx];		/* get entry */
		if (!l->modified) break;		/* find 1st entry where we dont't have to save */
	}

	if (l->modified) 					   /* check if it is modified */
	{
		if (saveblk_llcache(l)) return 0;  /* save previous */
	}

	l->pba=pba;							   /* set new pba */
	loadblk_llcache(l);					   /* try to load it */

	return l;
}

/****************************************************************************
 *
 * ll_init
 *
 * low level init function, this is called from mlayer lowinit function once
 *
 * RETURNS
 *
 * 0 - if successfuly
 * other if any error
 *
 ***************************************************************************/

unsigned char ll_init() 
{
	unsigned long a;

	gl_numofblocks=8192*8 /1;//1024;
	gl_pageperblock=128;//32;
	gl_pagesize=2048;//512;

	gl_free_block=MAX_FREE_BLOCK_AVAILABLE;
	gl_log_block=MAX_LOG_BLOCK_AVAILABLE;

	if (!onceinit) 
	{
		_mkdir(dirname);

		gl_wear=malloc(sizeof(long)*gl_numofblocks);
		gl_wearacc=malloc(sizeof(long)*gl_numofblocks);
		gl_fileexist=malloc(gl_numofblocks);

		if (init_llcache()) return 1;

		if ((!gl_wear) || (!gl_wearacc) || (!gl_fileexist)) return 1;

		for (a=0; a<gl_numofblocks; a++) 
		{
			gl_fileexist[a]=0; /* at startup lets suppose all are not exist */
			/* so all are 0xff erased */
		}

		(void)memset(gl_wear,0xff,sizeof(long)*gl_numofblocks); /* set reset state */
		(void)memset(gl_wearacc,0xff,sizeof(long)*gl_numofblocks);

		onceinit=1;
	}

	if ((!gl_wear) || (!gl_wearacc) || (!gl_fileexist)) return 1;

	return 0;
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

void ll_getwear(t_ba pba, unsigned long *dest) 
{
	if (!gl_wearacc) 
	{
		*dest=0;
		return;
	}

	*dest=gl_wearacc[pba];
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
	DEBPR1("ll_erase pba %08x\n",pba);

	if (pba>=gl_numofblocks) return LL_ERROR;

#if _HCC_MDEBUG_ENABLED_
	cnt_increase(CNT_TERASES);
#endif

	{
		ST_LLCACHE *l=get_llcache(pba);
		if (!l) return LL_ERROR;

		gl_fileexist[pba]=0; /* not exist any more */
		(void)memset(l->dbuff,0xff,MAX_BLOCK_SIZE);
		gl_wearacc[l->pba]=GET_SPARE_AREA(l->dbuff)->wear;
		l->modified=1;
	}


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

unsigned char ll_write(t_ba pba,t_po ppo, unsigned char *buffer, unsigned char *sparebuffer) 
{
	unsigned long page=ppo;
	unsigned char *dest;
	unsigned long a;
	ST_LLCACHE *l=get_llcache(pba);

	if (!l) return LL_ERROR;

	dest=l->dbuff;

	DEBPR2("ll_write pba %08x ppo %d\n",pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	page*=MAX_DLEN;

	dest+=page;

//	if (!ppo) {
//		unsigned long newwear=GET_SPARE_AREA(buffer)->wear;
//		if (gl_wear[pba]+1!=newwear) {
//		 	a=10;
//		}
//		gl_wear[pba]=newwear;
//	}

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


	gl_wearacc[l->pba]=GET_SPARE_AREA(l->dbuff)->wear;
	l->modified=1;

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
	unsigned long page=ppo;
	unsigned char *dest;
	unsigned long a;
	ST_LLCACHE *l=get_llcache(pba);

	if (!l) return LL_ERROR;

	dest=l->dbuff;

	DEBPR2("ll_writedouble pba %08x ppo %d\n",pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	page*=MAX_DLEN;

	dest+=page;

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

	gl_wearacc[l->pba]=GET_SPARE_AREA(l->dbuff)->wear;
	l->modified=1;
//	if (!ppo) {
//		unsigned long newwear=GET_SPARE_AREA((gl_data+pos+page))->wear;
//		if (gl_wear[pba]+1!=newwear) {
//		 	a=10;
//		}
//		gl_wear[pba]=newwear;
//	}

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
	unsigned long page=ppo;
	unsigned char *sou;
	unsigned long a;
	char iserased=1;
	ST_LLCACHE *l=get_llcache(pba);

	if (!l) return LL_ERROR;

	sou=l->dbuff;

	DEBPR2("ll_read pba %08x ppo %d\n",pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	page*=MAX_DLEN;

	sou+=page;

	for (a=0; a<MAX_DLEN; a++)  /* we should go throught on all data+spare*/
	{
		unsigned char ch=*sou++;
		if (ch!=0xff) iserased=0; /* simple erase chk */

		if (buffer) *buffer++=ch; /* if there is given buffer then store the data into it */
	}

	if (iserased) return LL_ERASED;	 /* if flasg is still set, returns with erased page */

	return LL_OK;
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
	unsigned long page=ppo;
	unsigned char *sou;
	unsigned long a;
	char iserased=1;
	ST_LLCACHE *l=get_llcache(pba);
	if (!l) return LL_ERROR;

	sou=l->dbuff;

	DEBPR2("ll_readpart pba %08x ppo %d\n",pba,ppo);

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	page*=MAX_DLEN;

	sou+=page;

	for (a=0; a<MAX_DLEN; a++)  /* we should go throught on all data+spare*/
	{
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
	/* in the simulation we simulate only 2 badblock 10 and 200 only for test purpose */

	if (pba==10 || pba==200) return 1; /* signal as bad block */

	return 0; /* no bad block signalled */
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
/*	DEBPR3("ll_readonebyte pba %08x ppo %d\n",pba,ppo); */

	if (pba>=gl_numofblocks) return LL_ERROR;
	if (ppo>=gl_pageperblock) return LL_ERROR;

	if (gl_fileexist[pba] || find_llcache(pba)) 
	{ /* if exist or cached only */
		unsigned long page=ppo;
		unsigned char *sou;
		ST_LLCACHE *l=get_llcache(pba);
		if (!l) return LL_ERROR;

		sou=l->dbuff;

		page*=MAX_DLEN;
		sou+=page;		   /* calculate page position */

		sou+=gl_pagesize; /* goto spare area*/
		sou+=sparepos;	  /* jump tu position in spare */
		*ch=*sou; /* store 1 character, no ECC required */
	}
	else 
	{
		*ch=0xff; /* if not exist then it is erased so put 0xff safety */
	}

	return LL_OK;
}

/****************************************************************************
 *
 * end of llayer.c
 *
 ***************************************************************************/

