/****************************************************************************
 *
 *            Copyright (c) 2003-2008 by HCC Embedded
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
#include "include/AT91SAM7SE512.h"

/****************************************************************************
 *
 * Special version for SAM7SEx and RM9260
 *
 * using hw ecc for data area (2KB)
 * and sw ecc for the remaining 12 bytes
 * data in a page:  ddddddd esssc
 * where:
 *  d - 2k data on main area
 *  e - hw ecc 4 bytes on spare start
 *  s - spare data 12 bytes
 *  c - sw ecc 4 bytes on spare
 *
 * flash r/b pin must be connected to pb19
 *
 ***************************************************************************/

/****************************************************************************
 *
 * Flash Device specific definitions
 *
 * k9f2g08u0m Samsung
 *
 ***************************************************************************/

#define BLOCK_NUM    2048   /* 2048 block all */
#define PAGE_SIZE    2048   /* page size is 2048 byte */
#define PAGEPERBLOCK 64     /* number of pages in a block */

#define SPARE_SIZE   MAX_SPARE_SIZE /* WARNING: spare size is 16 bytes always */

/* select sw ecc for 12bytes of spare area */
#define ECC 1

/****************************************************************************
 *
 * HW specific definitions
 *
 ***************************************************************************/

/* base addresses */
/* atmel at92sam7se-ek is used */

#define NANDDATAB   (*(volatile unsigned char*)0x40000000)
#define NANDDATAL   (*(volatile unsigned long*)0x40000000)
#define NANDCMD     (*(volatile unsigned char*)(0x40000000 + (1<<22)))
#define NANDADDR    (*(volatile unsigned char*)(0x40000000 + (1<<21)))

/* command of device */

#define NCMD_READ      0x00
#define NCMD_READ2     0x30

#define NCMD_READID    0x90

#define NCMD_RESET     0xFF

#define NCMD_PAGEPRG   0x80
#define NCMD_PAGEPRG2  0x10

#define NCMD_BLKERASE  0x60
#define NCMD_BLKERASE2 0xD0

#define NCMD_READST    0x70

#define NCMD_COPYS     0x35
#define NCMD_COPYD     0x85

/*  status bits  */

#define ST_ERROR 0x01
#define ST_READY 0x40
#define ST_WPROT 0x80

/****************************************************************************
 *
 * NANDwaitrb
 *
 * Wait until RB (ready/busy) signal goes high
 *
 ***************************************************************************/

static void NANDwaitrb()
{
	while (!(*AT91C_PIOB_PDSR & AT91C_PIO_PB19));
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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	unsigned long *rp=(unsigned long*)buffer;
	unsigned char iserased=1;
	int a;

#if ECC
	unsigned long ecc=0,eccori,ecchi,ecclo;
	unsigned long num=0x0000ffff; /* 24 bit ecc  */
#endif

	pagenum+=ppo;

	NANDCMD  = NCMD_READ;
	NANDADDR = 0;
	NANDADDR = 0;
	NANDADDR = (unsigned char)(pagenum & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);
	NANDCMD  = NCMD_READ2;

	NANDwaitrb();

	/* reading main area */
	for (a=0; a<PAGE_SIZE; a+=4)
	{  /* get whole page data */
		register unsigned long data=NANDDATAL;
		if (data!=0xffffffffUL) iserased=0;
		*rp++=data;
	}

	/* reading hwecc */
	{
		unsigned long hwecc=NANDDATAL;
	   	if (hwecc!=0xffffffffUL) iserased=0;
	}
	
	/* check/correct hwECC */
	if (!iserased)
	{
	   	unsigned char sr=(*AT91C_HECC_SR) & 0x7;
	
		if (sr & AT91C_ECC_MULERR)
		{
		  	/* multiple error cannot be recovered */
			return LL_ERROR;
		}
		
	   	if (sr & AT91C_ECC_RECERR)
	   	{
			if (sr & AT91C_ECC_ECCERR)
			{
				/* error in ECC no need to correct anything */
			}
			else
			{
				/* 1 bit error, let's correct it */
				unsigned long reg=*AT91C_HECC_PR;
				unsigned char bit= (unsigned char)(1 << (reg&0x0f));
	  			unsigned long addr=( reg >> 4)&0xfff;
				buffer[addr]^=bit;
		  	}
	   	}
	}

	/* reading spare area */	
	for (a=0; a<SPARE_SIZE-4; a+=4)
	{  /* get whole page data */
		register unsigned long data=NANDDATAL;
		if (data!=0xffffffffUL) iserased=0;

		*rp++=data;
		
#if ECC
		{
			int b;
			for (b=0; b<32; b++)
			{   /* calculate sw ECC */
				if (data&1) ecc^=num;
				num+=0x0000ffff;
				data>>=1;
			}
		}
#endif
	}
	
#if ECC
	eccori=NANDDATAL;  /* get ecc from 1st 32bit in spare area */

	if (eccori!=0xffffffffUL) iserased=0;
#endif

	if (iserased) return LL_ERASED;
	
#if ECC	
	ecc^=eccori;
	if (!ecc) return LL_OK; /* no bit error */

	ecchi=ecc>>16;
	ecclo=ecc&0x0ffff;

	if ( (ecchi + ecclo) !=0x0ffffUL )
	{
		/* ECC Error */
		return LL_ERROR; /* ecc error */
	}

	{	/* recovering 1 bit error */
		int addr=(int)(ecchi>>3);
		unsigned char bit =(unsigned char)(1 << (ecchi&7));
		buffer [PAGE_SIZE+addr]^=bit;
	}
#endif
	
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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	unsigned long *rp=(unsigned long*)buffer;
	unsigned char iserased=1;
	int a;

#if ECC
	unsigned long ecc=0,eccori,ecchi,ecclo;
	unsigned long num=0x0000ffff; /* 24 bit ecc  */
#endif

	pagenum+=ppo;

	NANDCMD  = NCMD_READ;
	NANDADDR = 0;
	NANDADDR = 0;
	NANDADDR = (unsigned char)(pagenum & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);
	NANDCMD  = NCMD_READ2;

	NANDwaitrb();

	/* reading main area */
	for (a=0; a<PAGE_SIZE; a+=4)
	{  /* get whole page data */
		register unsigned long data=NANDDATAL;
		if (data!=0xffffffffUL) iserased=0;
		
		if (index==LL_RP_1STHALF)
		{
			if (a<PAGE_SIZE/2) *rp++=data;
		}
		else if (index==LL_RP_2NDHALF)
		{
			if (a>=PAGE_SIZE/2) *rp++=data;
		}
		else if (index==LL_RP_DATA)
		{
			*rp++=data;
		}
	}

	/* reading hwecc */
	{
		unsigned long hwecc=NANDDATAL;
	   	if (hwecc!=0xffffffffUL) iserased=0;
	}
	
	/* check/correct hwECC */
	if (!iserased)
	{
	   	unsigned char sr=(*AT91C_HECC_SR) & 0x7;
	
		if (sr & AT91C_ECC_MULERR)
		{
		  	/* multiple error cannot be recovered */
			return LL_ERROR;
		}
		
	   	if (sr & AT91C_ECC_RECERR)
	   	{
			if (sr & AT91C_ECC_ECCERR)
			{
				/* error in ECC no need to correct anything */
			}
			else
			{
				/* 1 bit error, let's correct it */
				unsigned long reg=*AT91C_HECC_PR;
				unsigned char bit= (unsigned char)(1 << (reg&0x0f));
	  			unsigned long addr=( reg >> 4)&0xfff;
				
				if (index==LL_RP_1STHALF)
				{
					if (addr<PAGE_SIZE/2) buffer[addr]^=bit;
				}
				else if (index==LL_RP_2NDHALF)
				{
					if (addr>=PAGE_SIZE/2) buffer[addr-(PAGE_SIZE/2)]^=bit;
				}
				if (index==LL_RP_DATA)
				{
					buffer[addr]^=bit;
				}
				
		  	}
	   	}
	}

	/* reading spare area */	
	for (a=0; a<SPARE_SIZE-4; a+=4)
	{  /* get whole page data */
		register unsigned long data=NANDDATAL;
		if (data!=0xffffffffUL) iserased=0;

		if (index==LL_RP_SPARE)
		{
			*rp++=data;
		}
		
#if ECC
		{
			int b;
			for (b=0; b<32; b++)
			{   /* calculate sw ECC */
				if (data&1) ecc^=num;
				num+=0x0000ffff;
				data>>=1;
			}
		}
#endif
	}
	
#if ECC
	eccori=NANDDATAL;  /* get ecc from 1st 32bit in spare area */

	if (eccori!=0xffffffffUL) iserased=0;
#endif

	if (iserased) return LL_ERASED;
	
#if ECC	
	ecc^=eccori;
	if (!ecc) return LL_OK; /* no bit error */

	ecchi=ecc>>16;
	ecclo=ecc&0x0ffff;

	if ( (ecchi + ecclo) !=0x0ffffUL )
	{
		/* ECC Error */
		return LL_ERROR; /* ecc error */
	}

	{	/* recovering 1 bit error */
		int addr=(int)(ecchi>>3);
		unsigned char bit =(unsigned char)(1 << (ecchi&7));
		
		if (index==LL_RP_SPARE)
		{
			buffer [PAGE_SIZE+addr]^=bit;
		}
	}
#endif
	
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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	long io;
	int a;
	unsigned long *rp;

#if ECC
	unsigned long ecc,num;
	ecc=0;
	num=0x0000ffff; /* 24 bit ecc  */
#endif

	pagenum+=ppo;

	NANDCMD  = NCMD_PAGEPRG;
	NANDADDR = 0;
	NANDADDR = 0;
	NANDADDR = (unsigned char)(pagenum & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);
	
	rp=(unsigned long*)buffer;

	/* writing main area */
	for (a=0; a<PAGE_SIZE; a+=4)
	{  /* get whole page data */
		unsigned long rdata=*rp++;

		NANDDATAL = rdata;
	}
	
	/* write hwecc pr and npr register */	
	{
		unsigned long pr=*AT91C_HECC_PR;
		unsigned long npr=*AT91C_HECC_NPR;
		

		NANDDATAB = (unsigned char)(pr);
		NANDDATAB = (unsigned char)((pr)>>8);

		NANDDATAB = (unsigned char)(npr);
		NANDDATAB = (unsigned char)((npr)>>8);

	}

	/* writing spare area */
	rp=(unsigned long*)sparebuffer;
	for (a=0; a<SPARE_SIZE-4; a+=4)
	{
		register unsigned long rdata=*rp++;
		NANDDATAL = rdata;
#if ECC
		{
			int b;
			for (b=0; b<32; b++)
			{       /* calculate ECC */
				if (rdata&1) ecc^=num;
				num+=0x0000ffff;
				rdata>>=1;
			}
		}
#endif
	}

#if ECC
	/* write sw ecc */
	NANDDATAL = ecc;
#endif

	NANDCMD = NCMD_PAGEPRG2;
	NANDwaitrb();

	NANDCMD = NCMD_READST;
	io=NANDDATAB; /* holding status */

	if (io & ST_ERROR)
	{
/* fnPr("ERROR: ll_write 1\n");*/
		return LL_ERROR;
	}

	if (!(io & ST_READY))
	{
/* fnPr("ERROR: ll_write 2\n");*/
		return LL_ERROR;
	}

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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	long io;
	int a;
	unsigned long *rp;

#if ECC
	unsigned long ecc,num;
	ecc=0;
	num=0x0000ffff; /* 24 bit ecc  */
#endif

	pagenum+=ppo;

	NANDCMD  = NCMD_PAGEPRG;
	NANDADDR = 0;
	NANDADDR = 0;
	NANDADDR = (unsigned char)(pagenum & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);
	
	/* writing 1st half of main area */
	rp=(unsigned long*)buffer0;
	for (a=0; a<PAGE_SIZE/2; a+=4)
	{  /* get whole page data */
		unsigned long rdata=*rp++;

		NANDDATAL = rdata;
	}

	/* writing 2nd half of main area */
	rp=(unsigned long*)buffer1;
	for (a=0; a<PAGE_SIZE/2; a+=4)
	{  /* get whole page data */
		unsigned long rdata=*rp++;

		NANDDATAL = rdata;
	}
	
	/* write hwecc pr and npr register */	
	{
		unsigned long pr=*AT91C_HECC_PR;
		unsigned long npr=*AT91C_HECC_NPR;
		

		NANDDATAB = (unsigned char)(pr);
		NANDDATAB = (unsigned char)((pr)>>8);

		NANDDATAB = (unsigned char)(npr);
		NANDDATAB = (unsigned char)((npr)>>8);

	}

	/* writing spare area */
	rp=(unsigned long*)(buffer1+(PAGE_SIZE/2));
	for (a=0; a<SPARE_SIZE-4; a+=4)
	{
		register unsigned long rdata=*rp++;
		NANDDATAL = rdata;
#if ECC
		{
			int b;
			for (b=0; b<32; b++)
			{       /* calculate ECC */
				if (rdata&1) ecc^=num;
				num+=0x0000ffff;
				rdata>>=1;
			}
		}
#endif
	}

#if ECC
	/* write sw ecc */
	NANDDATAL = ecc;
#endif

	NANDCMD = NCMD_PAGEPRG2;
	NANDwaitrb();

	NANDCMD = NCMD_READST;
	io=NANDDATAB; /* holding status */

	if (io & ST_ERROR)
	{
/* fnPr("ERROR: ll_write 1\n");*/
		return LL_ERROR;
	}

	if (!(io & ST_READY))
	{
/* fnPr("ERROR: ll_write 2\n");*/
		return LL_ERROR;
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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	long io;

/*    fnPr("Erase: %d\n",block);    */

	NANDCMD = NCMD_BLKERASE;
	NANDADDR = (unsigned char)(pagenum & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);	
	NANDCMD  = NCMD_BLKERASE2;
	NANDwaitrb();

	NANDCMD = NCMD_READST;
	io=NANDDATAB; /* holding status */

	if (io & ST_ERROR)
	{
/* fnPr("ERROR: ll_erase 1\n");*/
		return LL_ERROR;
	}

	if (!(io & ST_READY))
	{
/* fnPr("ERROR: ll_erase 2\n");*/
		return LL_ERROR;
	}

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
	long pagenum=((long)pba)*PAGEPERBLOCK;
	unsigned long data;
	int a;

	for (a=0; a<2; a++,pagenum++)
	{

   		NANDCMD  = NCMD_READ;
		NANDADDR = 0;
		NANDADDR = 8; /* this bit signals that spare area reading */
		NANDADDR = (unsigned char)(pagenum & 0xff);
		NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
		NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);		
   		NANDCMD  = NCMD_READ2;

   		NANDwaitrb();

		data=NANDDATAL;

		if (data!=0xffffffffUL)
		{
/* fnPr("%l is bad block\n",pba); */
			return 1; /* signal BAD */
		}
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

	pagenum+=ppo;

  	NANDCMD  = NCMD_READ;
	NANDADDR = (unsigned char)(sparepos + 4); /* shifted with 4 bytes (hwecc at spare start)*/
	NANDADDR = 8; /* this bit signal that spare area reading */
	NANDADDR = (unsigned char)(pagenum & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 8) & 0xff);
	NANDADDR = (unsigned char)((pagenum >> 16) & 0xff);	
	NANDCMD  = NCMD_READ2;

	NANDwaitrb();
	*ch=NANDDATAB;

/* fnPr("%b ",*ch); */

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

	NANDCMD  = NCMD_READID;
	NANDADDR = 0;

	manID=NANDDATAB;
	devID=NANDDATAB;

/* fnPr("ManID %w DevID %w\n",manID,devID); */

	if ((manID==0x00ec) && (devID==0x00da))
	{
   		gl_numofblocks=BLOCK_NUM;
   		gl_pageperblock=PAGEPERBLOCK;
   		gl_pagesize=PAGE_SIZE;

		*AT91C_HECC_CR = 0x01;  //reset it
		*AT91C_HECC_MR = 0x02; //2112 bytes per page
		
   		return LL_OK; /* ok k9f1g08u0m */
	}

	return LL_ERROR; /*  unknown type  */
}

/****************************************************************************
 *
 *  end of k9f2g08u0m.c
 *
 ***************************************************************************/

