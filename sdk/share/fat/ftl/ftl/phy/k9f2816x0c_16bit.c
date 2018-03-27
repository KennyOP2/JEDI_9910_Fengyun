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

#include "llayer.h"
#include "../mdebug.h"


#define BLOCK_SIZE   0x4000 /* 16k */
#define BLOCK_NUM    1024   /* 1024 block all */
#define PAGE_SIZE    512    /* page size is 512 byte */
#define SPARE_SIZE   MAX_SPARE_SIZE /* WARNING: spare size is 16 bytes always */
#define PAGE_PER_BLK 32     /* 32 page per block */

/****************************************************************************
 *
 * static definitions
 *
 ***************************************************************************/


#define PAGEPERBLOCK (BLOCK_SIZE/PAGE_SIZE) /* number of pages in a block */

/****************************************************************************
 *
 * HW specific definitions
 *
 ***************************************************************************/

/* base addresses */

#define NANDCMD  (*(volatile unsigned long*)0x1000000)
#define NANDADDR (*(volatile unsigned long*)0x1000004)
#define NANDDATA (*(volatile unsigned long*)0x1000008)
#define NANDRB   (*(volatile unsigned long*)0x100000c)

/* command of device */

#define NCMD_READ1     0x00
#define NCMD_READ2     0x50
#define NCMD_READID    0x90
#define NCMD_RESET     0xFF
#define NCMD_PAGEPRG   0x80
#define NCMD_PAGEPRG2  0x10
#define NCMD_BLKERASE  0x60
#define NCMD_BLKERASE2 0xD0
#define NCMD_READST    0x70

/*  status bits  */

#define ST_ERROR 0x01
#define ST_READY 0x40
#define ST_WPROT 0x80

/****************************************************************************
 *
 * NANDcmd
 *
 * Send command to chip ( CLE-hi  ALE-lo WR-hi)
 *
 * INPUTS
 *
 * cmd - what byte command to send
 *
 ***************************************************************************/

static void NANDcmd(long cmd)
{
	NANDCMD = cmd | 0xff0000;
}

/****************************************************************************
 *
 * NANDaddr
 *
 * send address to chip ( CLE-lo ALE-hi WR-hi )
 *
 * INPUTS
 *
 * addr - page number
 *
 ***************************************************************************/

static void NANDaddr(long addr)
{
	addr&=0xff;
	NANDADDR = addr;
}

/****************************************************************************
 *
 * NANDwaitrb
 *
 * Wait until RB (ready/busy) signal goes high
 *
 ***************************************************************************/

static void NANDwaitrb()
{
	for (;;)
	{
		if (NANDRB & 1) return;
	}
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
	long pagenum=(long)pba*PAGEPERBLOCK+(long)ppo;
	unsigned long a;
	long b,num;
	unsigned long *rp=(unsigned long*)buffer;
	unsigned long ecc,eccori,ecchi,ecclo;
	unsigned char iserased=1;

	ecc=0;
	num=0x0000ffff; /* 24 bit ecc  */

	NANDcmd(NCMD_READ1);
	NANDaddr(0);
	NANDaddr(pagenum);
	NANDaddr(pagenum>> 8);
	NANDwaitrb();

	for (a=0; a<PAGE_SIZE+SPARE_SIZE-4; a+=4)
	{  /* get whole page data */
		register unsigned long data=NANDDATA & 0xffff;
		data<<=16;
		data|=NANDDATA & 0xffff; /* 2times 16bit */

		if (data!=0xffffffffUL) iserased=0;

		*rp++=data;
		for (b=0; b<32; b++)
		{       /* calculate ECC */
			if (data&1) ecc^=num;
			num+=0x0000ffff;
			data>>=1;
		}
	}

	eccori=NANDDATA & 0xffff;  /* get ecc from 1st 32bit in spare area */
	eccori<<=16;
	eccori|=NANDDATA & 0xffff; /* 2times 16bit */

	if (eccori!=0xffffffffUL) iserased=0;

	if (iserased) return LL_ERASED;

	ecc^=eccori;
	if (!ecc)
	{
/*      fnPr("ReadPage: ok pba %d ppo %d\n",pba,ppo); */
		return LL_OK; /* no bit error */
	}

	ecchi=ecc>>16;
	ecclo=ecc&0x0ffff;

	if ( (ecchi + ecclo) !=0x0ffffUL )
	{
/* fnPr("ERROR: ecc error\n");            */
		return LL_ERROR; /* ecc error */
	}

	buffer[ ecchi >> 3] ^= (1<< (ecchi&7) ); /* correcting error */

/*    fnPr("ReadPage: corrected pba %d ppo %d\n",pba,ppo); */
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
	long pagenum=(long)pba*PAGEPERBLOCK+(long)ppo;
	unsigned long a;
	long b,num;
	unsigned long *rp=(unsigned long*)buffer;
	unsigned long ecc,eccori,ecchi,ecclo;
	unsigned char iserased=1;

	ecc=0;
	num=0x0000ffff; /* 24 bit ecc  */

	NANDcmd(NCMD_READ1);
	NANDaddr(0);
	NANDaddr(pagenum);
	NANDaddr(pagenum>> 8);
	NANDwaitrb();

	for (a=0; a<PAGE_SIZE+SPARE_SIZE-4; a+=4)
	{  /* get whole page data */
		register unsigned long data=NANDDATA & 0xffff;
		data<<=16;
		data|=NANDDATA & 0xffff; /* 2times 16bit */

		if (data!=0xffffffffUL) iserased=0;

		if (index==LL_RP_1STHALF)
		{
			if (a<PAGE_SIZE/2) *rp++=data;
		}
		else if (index==LL_RP_2NDHALF)
		{
			if (a>=PAGE_SIZE/2 && a<PAGE_SIZE) *rp++=data;
		}
		else if (index==LL_RP_DATA)
		{
			if (a<PAGE_SIZE) *rp++=data;
		}
		else if (index==LL_RP_SPARE)
		{
			if (a>=PAGE_SIZE) *rp++=data;
		}
		else return LL_ERROR;

		for (b=0; b<32; b++)
		{       /* calculate ECC */
			if (data&1) ecc^=num;
			num+=0x0000ffff;
			data>>=1;
		}
	}

	eccori=NANDDATA & 0xffff;  /* get ecc from 1st 32bit in spare area */
	eccori<<=16;
	eccori|=NANDDATA & 0xffff; /* 2times 16bit */

	if (eccori!=0xffffffffUL) iserased=0;

	if (iserased) return LL_ERASED;

	ecc^=eccori;
	if (!ecc)
	{
/*      fnPr("ReadPage: ok pba %d ppo %d\n",pba,ppo); */
		return LL_OK; /* no bit error */
	}

	ecchi=ecc>>16;
	ecclo=ecc&0x0ffff;

	if ( (ecchi + ecclo) !=0x0ffffUL )
	{
/* fnPr("ERROR: ecc error\n");            */
		return LL_ERROR; /* ecc error */
	}

	a=(long)(ecchi>>3);

	if (index==LL_RP_1STHALF)
	{
		if (a<PAGE_SIZE/2)
		{
			buffer[ a ] ^= (1<< (ecchi&7) ); /* correcting error */
		}
	}
	else if (index==LL_RP_2NDHALF)
	{
		if (a>=PAGE_SIZE/2 && a<PAGE_SIZE)
		{
       		buffer[ a-(PAGE_SIZE/2)] ^= (1<< (ecchi&7) ); /* correcting error */
		}
	}
	else if (index==LL_RP_DATA)
	{
		if (a<PAGE_SIZE)
		{
       		buffer[ a ] ^= (1<< (ecchi&7) ); /* correcting error */
		}
	}
	else if (index==LL_RP_SPARE)
	{
		if (a>=PAGE_SIZE)
		{
       		buffer[ a-PAGE_SIZE] ^= (1<< (ecchi&7) ); /* correcting error */
		}
	}
	else return LL_ERROR;

/*    fnPr("ReadPage: corrected pba %d ppo %d\n",pba,ppo); */
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
	long pagenum=(long)pba*PAGEPERBLOCK+(long)ppo;
	long io;
	unsigned long a;
	long b;
	unsigned long ecc,num;
	unsigned long *rp;

	ecc=0;
	num=0x0000ffff; /* 24 bit ecc  */

	NANDcmd(NCMD_READ1);
	NANDcmd(NCMD_PAGEPRG);
	NANDaddr(0);
	NANDaddr(pagenum);
	NANDaddr(pagenum>> 8);

	rp=(unsigned long*)buffer;

	for (a=0; a<PAGE_SIZE; a+=4)
	{  /* write one page data */
		unsigned long rdata=*rp++;

		NANDDATA= (rdata>>16) & 0xffff;
		NANDDATA= rdata&0xffff; /* 2times 16bit */

		for (b=0; b<32; b++)
		{       /* calculate ECC */
			if (rdata&1) ecc^=num;
			num+=0x0000ffff;
			rdata>>=1;
		}

	}

	rp=(unsigned long*)sparebuffer;

	for (a=0; a<SPARE_SIZE-4; a+=4)
	{  /* write one page data */
		unsigned long rdata=*rp++;

		NANDDATA= (rdata>>16) & 0xffff;
		NANDDATA= rdata&0xffff; /* 2times 16bit */

		for (b=0; b<32; b++)
		{       /* calculate ECC */
			if (rdata&1) ecc^=num;
			num+=0x0000ffff;
			rdata>>=1;
		}

	}


	NANDDATA= (ecc>>16)& 0xffff;   /* write ecc also, but this is in spare area yet */
	NANDDATA= ecc & 0xffff;        /* 16bit */

	NANDcmd(NCMD_PAGEPRG2);
	NANDwaitrb();

	NANDcmd(NCMD_READST);
	io=NANDDATA; /* holding status */

	if (io & ST_ERROR)
	{
/*       fnPr("WritePage: error pba %d ppo %d\n",pba,ppo); */
		return LL_ERROR;
	}

	if (!(io & ST_READY))
	{
/*       fnPr("WritePage: error ready pba %d ppo %d\n",pba,ppo); */
		return LL_ERROR;
	}

/*    fnPr("WritePage: pba %d ppo %d\n",pba,ppo); */
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
	long pagenum=(long)pba*PAGEPERBLOCK+(long)ppo;
	long io;
	unsigned long a;
	long b;
	unsigned long ecc,num;
	unsigned long *rp=(unsigned long*)buffer0;
/* unsigned char *s=(unsigned char*)data; */

	ecc=0;
	num=0x0000ffff; /* 24 bit ecc  */

	NANDcmd(NCMD_READ1);
	NANDcmd(NCMD_PAGEPRG);
	NANDaddr(0);
	NANDaddr(pagenum);
	NANDaddr(pagenum>> 8);

	for (a=0; a<PAGE_SIZE/2; a+=4)
	{  /* write one page data */
		unsigned long rdata=*rp++;

		NANDDATA= (rdata>>16) & 0xffff;
		NANDDATA= rdata&0xffff; /* 2times 16bit */

		for (b=0; b<32; b++)
		{       /* calculate ECC */
			if (rdata&1) ecc^=num;
			num+=0x0000ffff;
			rdata>>=1;
		}
	}

	rp=(unsigned long*)buffer1;

	for (; a<PAGE_SIZE+SPARE_SIZE-4; a+=4)
	{  /* write one page data */
		unsigned long rdata=*rp++;

		NANDDATA= (rdata>>16) & 0xffff;
		NANDDATA= rdata&0xffff; /* 2times 16bit */

		for (b=0; b<32; b++)
		{       /* calculate ECC */
			if (rdata&1) ecc^=num;
			num+=0x0000ffff;
			rdata>>=1;
		}
	}

	NANDDATA= (ecc>>16)& 0xffff;   /* write ecc also, but this is in spare area yet */
	NANDDATA= ecc & 0xffff;        /* 16bit */

	NANDcmd(NCMD_PAGEPRG2);
	NANDwaitrb();

	NANDcmd(NCMD_READST);
	io=NANDDATA; /* holding status */

	if (io & ST_ERROR)
	{
/*       fnPr("WriteDoublePage: error pba %d ppo %d\n",pba,ppo); */
		return LL_ERROR;
	}

	if (!(io & ST_READY))
	{
/*       fnPr("WriteDoublePage: error ready pba %d ppo %d\n",pba,ppo); */
		return LL_ERROR;
	}

/*    fnPr("WriteDoublePage: pba %d ppo %d\n",pba,ppo); */
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
	long pagenum=(long)pba*PAGEPERBLOCK;
	long io;

/*    fnPr("Erase: %d\n",block);    */

	NANDcmd(NCMD_BLKERASE);
	NANDaddr(pagenum);
	NANDaddr(pagenum>> 8);

	NANDcmd(NCMD_BLKERASE2);
	NANDwaitrb();

	NANDcmd(NCMD_READST);
	io=NANDDATA; /* holding status */

	if (io & ST_ERROR)
	{
/*       fnPr("ErasePage: error pba %d \n",pba); */
		return LL_ERROR;
	}

	if (!(io & ST_READY))
	{
/*       fnPr("ErasePage: error ready pba %d \n",pba); */
		return LL_ERROR;
	}

/*    fnPr("ErasePage: pba %d \n",pba); */
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
	long pagenum=(long)pba*PAGEPERBLOCK;
	unsigned long data;
	int a;

	for (a=0; a<2; a++,pagenum++)
	{
		NANDcmd(NCMD_READ2);   /* read spare area */
		NANDaddr(0);
		NANDaddr(pagenum);
		NANDaddr(pagenum >> 8);
		NANDwaitrb();

		data=NANDDATA;
		data=NANDDATA;
		data=NANDDATA;
		data=NANDDATA;

		data=NANDDATA & 0xffff;
		data<<=16;
		data|=NANDDATA & 0xffff; /* 6th word contains no ffffffff if bad */

		if (data!=0xffffffffUL) return 1; /* signal BAD */
	}

	return 0;
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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	unsigned long data;

	pagenum+=ppo;

	NANDcmd(NCMD_READ2);   /* read spare area */
	NANDaddr(sparepos >> 1);
	NANDaddr(pagenum);
	NANDaddr(pagenum >> 8);
	NANDwaitrb();

	data=NANDDATA;

	if (sparepos&1) *ch=(unsigned char)(data&0xff);
	else *ch=(unsigned char)((data>>8)&0xff);

	return LL_OK;
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
	long manID, devID;

	NANDcmd(NCMD_READID);
	NANDaddr(0);

	manID=NANDDATA & 0xffff;
	devID=NANDDATA & 0xffff;

	if ((manID==0x00ec) && (devID==0x0053))
	{
   		gl_numofblocks=BLOCK_NUM;
   		gl_pageperblock=PAGE_PER_BLK;
   		gl_pagesize=PAGE_SIZE;

  		return 0; /* ok */
	}

	return 1; /*  unknown type  */
}

/****************************************************************************
 *
 *  end of K9F2816X0C_16bit.c
 *
 ***************************************************************************/




