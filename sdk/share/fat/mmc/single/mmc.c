/****************************************************************************
 *
 *            Copyright (c) 2003 by HCC Embedded 
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
 * Budapest 1132
 * Victor Hugo Utca 11-15
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#include "mmc.h"

#define tNCR	(8+1)			/* command to response time */
#define tNCX	(8+1)			/* response to CSD time */
#define tNRC	(1+1)			/* response to next cmd time */
#define tNWR	(1+1)			/* response to data (write) */
#define tNBR	(1+1)			/* stop transition to busy time */
static unsigned long tNAC=0;		/* READ: response to data time (varies upon card) */
static unsigned long tBUSY=0;		/* WRITE: maximum write time */


#define CMD_GO_IDLE_STATE		0
#define CMD_SEND_OP_COND		1
#define CMD_SEND_CSD			9
#define CMD_STOP_TRANS			12
#define CMD_SEND_STATUS			13
#define CMD_SET_BLOCKLEN		16
#define CMD_READ_SINGLE_BLOCK		17
#define CMD_READ_MULTIPLE_BLOCK		18
#define CMD_SET_BLOCK_COUNT		23
#define CMD_WRITE_SINGLE_BLOCK		24
#define CMD_WRITE_MULTIPLE_BLOCK	25
#define CMD_READ_OCR			58
#define CMD_CRC_ON_OFF			59


#define BIT_IDLE_STATE			1
#define BIT_ILLEGAL_COMMAND		4



/* redefine CS high functions, add 8 bits after CS high */ 
#define SPI_CS_HI { spi_cs_hi(); spi_tx1(0xff); }
#define SPI_CS_LO spi_cs_lo()


static t_mmc_dsc mmc_dsc;		/* current MMC/SD card descriptor */
static F_DRIVER mmc_drv;

#if USE_CRC
/****************************************************************************
 * crc16
 * calculates a CRC value
 ***************************************************************************/
static unsigned long CRCtbl[256];
static unsigned long crc16( unsigned long crc, unsigned char c)
{
  int a;
  crc=crc^((unsigned long)c<<8UL);
  for(a=0;a<8;a++) 
  {
    if(crc&0x8000) crc=(crc<<1)^0x1021;
              else crc<<=1;
  }
  return crc;
}
#endif



/****************************************************************************
 *
 * spiWaitStartBit
 *
 * Wait data start bit on spi and send one more clock
 *
 * RETURNS
 *
 * 0 - if received
 * other if time out
 *
 ***************************************************************************/
static int spiWaitStartBit (unsigned long a) 
{
   while (a--) 
   {
     if (spi_rx1()!=0xff) return 0;
   }
   return 1;
}

/****************************************************************************
 *
 * spiWaitBusy
 *
 * Wait until SPI datain line goes hi
 *
 * RETURNS
 *
 * 0 - if ok
 * other if time out
 *
 ***************************************************************************/
static int spiWaitBusy(void) 
{
   unsigned long a=tBUSY;
   while(a--)
   {
     if (spi_rx1()==0xff) return 0; 
   }
   return 1;
}

/****************************************************************************
 *
 * mmc_cmd
 *
 * Send command to SPI, it adds startbit and stopbit and calculates crc7 also
 * sent, and Wait response start bit on spi and r1 response
 *
 * INPUTS
 *
 * cmd - command to send
 * data - argument of the command
 *
 * RETURNS
 *
 * r1 if received or 0xff if any error
 *
 ***************************************************************************/
static unsigned char mmc_cmd(unsigned long cmd,unsigned long data) 
{
   register int a;
   register unsigned char tmp=0;

   cmd|=0xffffff40;
   spi_tx4(cmd);

#if USE_CRC
   for (a=0;a<8;a++) 
   {     /* send start and cmd */
     if (cmd&0x80) tmp^=0x40;
     cmd<<=1;
     if (tmp&0x40) tmp=(tmp<<1)^0x09;
              else tmp=tmp<<1;
   }
#endif

   spi_tx4(data);  /* send data */
#if USE_CRC
   for (a=0;a<32;a++) 
   {   /* send 32bit argument */
     if (data & 0x80000000) tmp^=0x40;
     data<<=1;
     if(tmp&0x40) tmp=(tmp<<1)^0x09;
             else tmp=tmp<<1;
   }

   tmp<<=1;  /* set tmp into correct position */
   tmp|=1;   /* add endbit   */

   spi_tx1(tmp); /* send tmp and endbit */
#else
   spi_tx1(0x95);
#endif

   for (a=0;a<tNCR;a++)
   {
     tmp=spi_rx1();
     if (tmp!=0xff) return tmp;
   }

   return 0xff;
}


/****************************************************************************
 *
 * mmc_read
 *
 * Getting a datablock from SD
 *
 * INPUTS
 *
 * driver - driver structure
 * dwaddr - long pointer where to store received data (512bytes)
 * addr - address where to start read card, it has to be sector aligned
 * cnt - number of sectors
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error (crc,timeouts)
 *
 ***************************************************************************/

static int mmc_read(F_DRIVER *driver,unsigned char *dwaddr,unsigned long addr,int cnt) 
{
#if USE_CRC
   register unsigned short crc;
#endif
   int multi;
   unsigned char r1;

   if (!get_cd()) return MMC_ERR_NOTPLUGGED; /* card missing */ 

   SPI_CS_LO;

   if (cnt>1) multi=1; else multi=0;

   if (multi && mmc_dsc.bcs)
   {
     r1=mmc_cmd(CMD_SET_BLOCK_COUNT,cnt);
     if (r1)
     {
       SPI_CS_HI;
       if (r1&BIT_IDLE_STATE) mmc_dsc.initok=0;
       return MMC_ERR_CMD;
     }
   }

   r1=mmc_cmd(multi?CMD_READ_MULTIPLE_BLOCK:CMD_READ_SINGLE_BLOCK,addr);
   if (r1) 
   {
     SPI_CS_HI;
     if (r1&BIT_IDLE_STATE) mmc_dsc.initok=0;
     return MMC_ERR_CMD;
   }
   
   for (;cnt;cnt--)
   {
      register unsigned long a;
#if USE_CRC
      crc=0;
#endif

      for (a=0;a<tNAC && (r1=spi_rx1())==0xff;a++);
      if (r1!=0xfe)
      {
         SPI_CS_HI;
         return MMC_ERR_STARTBIT;
      }

      spi_rx512(dwaddr);
#if USE_CRC
      for (a=0;a<512;a++) crc=(crc<<8)^CRCtbl[((crc>>8)^(*dwaddr++))&0xff];
#else
      dwaddr+=512;
#endif

      {
#if USE_CRC
        unsigned short rcrc=spi_rx1();
		rcrc<<=8;
        rcrc|=spi_rx1();
        if (crc!=rcrc)
        {
          SPI_CS_HI;
          return MMC_ERR_CRC;
        }
#else
        (void)spi_rx1(); /* crc lo */
        (void)spi_rx1(); /* crc hi */
#endif
      }
   }

   if (multi && mmc_dsc.bcs==0)
   {
     if (mmc_cmd(CMD_STOP_TRANS,0)) (void)spiWaitBusy();
   }

   SPI_CS_HI;

   return MMC_NO_ERROR;
}


/****************************************************************************
 *
 * mmc_write
 *
 * Write data into SD
 *
 * INPUTS
 *
 * driver - driver structure
 * dwaddr - long pointer where original data is (512bytes)
 * addr - where to store in the card (address has to be sector aligned)
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
static int mmc_write(F_DRIVER *driver,unsigned char *dwaddr,unsigned long addr,int cnt) 
{
#if USE_CRC
   register unsigned short crc;
#endif
   int multi;
   unsigned char r1;

   if (get_cd()==0) return MMC_ERR_NOTPLUGGED;/* card is missing */
   if (get_wp()) return MMC_ERR_WRITEPROTECT; /* write protected!! */

   SPI_CS_LO;

   if (cnt>1) multi=1; else multi=0;

   if (multi && mmc_dsc.bcs)
   {
     r1=mmc_cmd(CMD_SET_BLOCK_COUNT,cnt);
     if (r1)
     {
       SPI_CS_HI;
       if (r1&BIT_IDLE_STATE) mmc_dsc.initok=0;
       return MMC_ERR_CMD;
     }
   }

   r1=mmc_cmd(multi?CMD_WRITE_MULTIPLE_BLOCK:CMD_WRITE_SINGLE_BLOCK,addr);
   if (r1) 
   {
     SPI_CS_HI;
     if (r1&BIT_IDLE_STATE) mmc_dsc.initok=0;
     return MMC_ERR_CMD;
   }

   for (;cnt;cnt--)
   {
      register unsigned int a;

      if (multi) spi_tx2(0xfffc); else spi_tx2(0xfffe);

      spi_tx512(dwaddr);
#if USE_CRC
      crc=0;
      for (a=0;a<512;a++) crc=(crc<<8)^CRCtbl[((crc>>8)^(*dwaddr++))&0xff];
      spi_tx2(crc);
#else
      dwaddr+=512;
      spi_tx2(0);
#endif

      for (a=0;a<tNCR;a++)
      {
        if (spi_rx1()==0xe5) break;
      }
      if (a==tNCR)
      {
        SPI_CS_HI;
        return MMC_ERR_WRITE;
      }
      
      spi_tx4(0xffffffff);
      if (spiWaitBusy()) 
      {
         SPI_CS_HI;
         return MMC_ERR_BUSY;
      }
   }

   if (multi)
   {
     if (mmc_dsc.bcs) spi_tx2(0xffff); else spi_tx2(0xfffd);
     spi_tx4(0xffffffff);
     if (spiWaitBusy())
     {
       SPI_CS_HI;
       return MMC_ERR_BUSY;
     }
   }

   SPI_CS_HI;
   
   return MMC_NO_ERROR;
}

/****************************************************************************
 *
 * mmc_readsector
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

int mmc_readsector(F_DRIVER *driver,void *data,unsigned long sector) 
{
   return mmc_read(driver,data,sector*512,1);
}

/****************************************************************************
 * mmc_readmultiplesector
 * Reads multiple sectors
 *
 * INPUT - same as mmc_readsector
 *         cnt - number of sectors to read
 ***************************************************************************/
int mmc_readmultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
   return mmc_read(driver,data,sector*512,cnt);
}

/****************************************************************************
 *
 * mmc_writesector
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
int mmc_writesector(F_DRIVER *driver,void *data,unsigned long sector) 
{
   return mmc_write(driver,data,sector*512,1);
}

/****************************************************************************
 * mmc_writemultiplesector
 * Writes multiple sectors
 *
 * INPUT - same as mmc_writesector
 *         cnt - number of sectors to write
 ***************************************************************************/
int mmc_writemultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
   return mmc_write(driver,data,sector*512,cnt);
}

/****************************************************************************
 *
 * _get_csd_bits
 *
 * Gets specified bits from bitspace g_mmc_type.CSD
 * Bits described in MMC specification. (e.g. CSD_STRUCTURE: st=126, len=2)
 * 
 * INPUT: st - start bit (max. 127)
 *        len- length of data needed in bits.
 * OUTPUT:decoded value
 *
 ***************************************************************************/
unsigned short _get_csd_bits (unsigned char *src, unsigned char st, unsigned char len)
{
  unsigned short dst;
  unsigned char tmp,t_left,t_act;

  dst=0;

  tmp=127-(st+len-1);
  src+=(tmp>>3);
  tmp&=7;
  t_left=8-tmp;

  t_act=*src++;
  for (;tmp;tmp--) t_act<<=1;
  while (len)
  {
    dst<<=1;
    if (t_left==0)
    {
      t_left=8;
      t_act=*src++;
    }
    if (t_act&0x80) dst|=1;
    t_act<<=1;
    --t_left;
    --len;
  }

  return(dst);
}


/****************************************************************************
 *
 * mmc_getcsd
 *
 * Get Card Specific Data
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
static int mmc_getcsd (void) 
{
  unsigned char a;
  unsigned char *csd=mmc_dsc.CSD;
#if USE_CRC
  unsigned short crc=0;
#endif

  SPI_CS_LO;
  if (mmc_cmd(CMD_SEND_CSD,0)) 
  {
    SPI_CS_HI;
    return MMC_ERR_CMD; /*r1 any error*/
  }

  if (spiWaitStartBit(tNCX)) 
  {
    SPI_CS_HI;
    return MMC_ERR_STARTBIT;
  }

  for (a=0;a<16;*(csd+a++)=spi_rx1()); 

#if USE_CRC
  for (a=0;a<16;a++) crc=(crc<<8)^CRCtbl[(crc>>8)^(csd[a])];
  {
    unsigned short tmp=0;
    tmp=spi_rx1()<<8;
    tmp|=spi_rx1();
    SPI_CS_HI;
    if (crc!=tmp) return MMC_ERR_CRC; 
  }
#else
  spi_tx2(0xffff);
  SPI_CS_HI;
#endif

  mmc_dsc.TRANSPEED=_get_csd_bits(csd,96,8);
  mmc_dsc.R_BL_LEN=_get_csd_bits(csd,80,4);
  mmc_dsc.CSIZE=_get_csd_bits(csd,62,12);
  mmc_dsc.CSIZE_M=_get_csd_bits(csd,47,3);
  mmc_dsc.NSAC=_get_csd_bits(csd,104,8);
  mmc_dsc.TAAC=_get_csd_bits(csd,112,8);
  mmc_dsc.R2W=_get_csd_bits(csd,26,3);

  {
    int i;
    unsigned long br;
    unsigned char m[15]={10,12,13,15,20,25,30,35,40,45,50,55,60,70,80};
    
    br=1;
    for (i=mmc_dsc.TRANSPEED&7;i;br*=10,i--);
    br*=m[((mmc_dsc.TRANSPEED>>3)&0xf)-1]; 
    spi_set_baudrate(br*10000);	/* pass speed in Hz */

    {
      unsigned long div=100;
      br=spi_get_baudrate();
      for (i=7-((mmc_dsc.TAAC)&7);i;div*=10,i--);
      div*=10;
      div/=m[(((mmc_dsc.TAAC)>>3)&0xf)-1]; 
      tNAC=(10*(((br+div-1)/div)+100*(mmc_dsc.NSAC))+1)/8;
      tBUSY=tNAC*(1<<(mmc_dsc.R2W));
    }
  }

  return MMC_NO_ERROR;
}



      
/****************************************************************************
 *
 * mmc_initcard
 *
 * inti Card protocols via spi
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/
static int mmc_initcard(void) 
{
   long a;
   long retry;
   unsigned long ocr;
   int ret;
   unsigned char r1;

   mmc_dsc.initok=0;
   spi_set_baudrate(100000); /* set baudrate here to 100kHz */   

   for (a=0; a<1000; a++) spi_tx4(0xffffffff); /* wait some */

   for (retry=0;retry<100;retry++) 
   {
      SPI_CS_LO;
      if (mmc_cmd(CMD_GO_IDLE_STATE,0)==BIT_IDLE_STATE) break;
      SPI_CS_HI;
   }
   SPI_CS_HI;
   if (retry==100) return MMC_ERR_INIT;

   for (retry=0,a=1;retry<5000 && a;retry++)
   {
     SPI_CS_LO;
     a=mmc_cmd(CMD_SEND_OP_COND,0);
     SPI_CS_HI;
   }
   if (retry==5000) return MMC_ERR_INIT;

   /* read OCR */
   SPI_CS_LO;
   if (mmc_cmd(CMD_READ_OCR,0))
   { 
     SPI_CS_HI;
     return MMC_ERR_CMD;
   }
   ocr=spi_rx1()<<24;
   ocr|=(spi_rx1()<<16);
   ocr|=(spi_rx1()<<8);
   ocr|=spi_rx1();
   SPI_CS_HI;
   if ((ocr&~(1<<7))==0x80ff8000)
   {
     if (ocr&(1<<7)) mmc_dsc.cardtype=1;
                else mmc_dsc.cardtype=0;
   }
   else return MMC_ERR_INIT;

   SPI_CS_LO;
   if (mmc_cmd(CMD_SET_BLOCKLEN,512)) 
   { /* set blk lenght 512 byte */
     SPI_CS_HI;
     return MMC_ERR_CMD;
   }
   SPI_CS_HI;

   SPI_CS_LO;
#if USE_CRC
   if (mmc_cmd(CMD_CRC_ON_OFF,1))
#else
   if (mmc_cmd(CMD_CRC_ON_OFF,0)) 
#endif
   {  /* set crc on/off */
     SPI_CS_HI;
     return MMC_ERR_CMD;
   }
   SPI_CS_HI;

   ret=mmc_getcsd(); 
   if (ret) return ret ;

   SPI_CS_LO;
   r1=mmc_cmd(CMD_SET_BLOCK_COUNT,1);
   SPI_CS_HI;
   if (r1==BIT_ILLEGAL_COMMAND) mmc_dsc.bcs=0; 
   else if (r1==0) mmc_dsc.bcs=1;
   else return MMC_ERR_CMD;
   mmc_dsc.initok=1;

   return MMC_NO_ERROR;
}

/****************************************************************************
 *
 * mmc_getstatus
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
static long mmc_getstatus(F_DRIVER *driver) 
{
   long state=0;

   if (get_wp()) state|=F_ST_WRPROTECT;
   if (get_cd()==0) 
   {
      mmc_dsc.initok=0;
      state|=F_ST_MISSING;
   }
   else 
   {
      if (mmc_dsc.initok)
      {
        unsigned short rc;
        SPI_CS_LO;
        rc=mmc_cmd(CMD_SEND_STATUS,0);
        rc<<=8;
        rc|=spi_rx1();
        SPI_CS_HI;
        if (rc&(BIT_IDLE_STATE<<8)) 
        {
          state|=F_ST_CHANGED;
          (void)mmc_initcard();
        }
      }
      else
      {
        (void)mmc_initcard();
      }
   }

   if (!mmc_dsc.initok) state|=F_ST_MISSING; /* card is not initialized */
   
   return state; 
}


/****************************************************************************
 *
 * mmc_getphy
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
static int mmc_getphy(F_DRIVER *driver,F_PHY *phy) 
{
   if (mmc_dsc.initok==0) return MMC_ERR_NOTINITIALIZED;
   phy->number_of_cylinders=0;
   phy->sector_per_track=63;
   phy->number_of_heads=255;
   phy->number_of_sectors=((1UL<<(mmc_dsc.CSIZE_M+2))*(1UL<<mmc_dsc.R_BL_LEN)*(mmc_dsc.CSIZE+1))/512UL;
   phy->media_descriptor=0xf0;

   return MMC_NO_ERROR;
}

/****************************************************************************
 * 
 * mmc_release
 * 
 * Deletes a previously initialized driver
 * 
 * INPUTS
 * 
 * driver - driver structure
 * 
 ***************************************************************************/

static void mmc_release(F_DRIVER *driver)
{
}
 
/****************************************************************************
 *
 * mmc_initfunc
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

F_DRIVER *mmc_initfunc(unsigned long driver_param) 
{
#if USE_CRC
   int a;
   for (a=0;a<256;a++) CRCtbl[a]=crc16(0,(unsigned char)a); 
#endif

   (void)_memset(&mmc_dsc,0,sizeof(t_mmc_dsc));
   (void)spi_init();

   (void)_memset(&mmc_drv,0,sizeof(mmc_drv));

   mmc_drv.readsector=mmc_readsector;
   mmc_drv.writesector=mmc_writesector;
   mmc_drv.readmultiplesector=mmc_readmultiplesector;
   mmc_drv.writemultiplesector=mmc_writemultiplesector;
   mmc_drv.getstatus=mmc_getstatus;
   mmc_drv.getphy=mmc_getphy;
   mmc_drv.release=mmc_release;

   return &mmc_drv;
}



