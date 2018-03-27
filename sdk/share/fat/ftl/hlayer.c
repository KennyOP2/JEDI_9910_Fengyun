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
#define  ENABLE_CONTINUE_WRITE
#define  ENABLE_DMA_MEMCOPY
/****************************************************************************
 *
 *	includes
 *
 ***************************************************************************/

#include "hlayer.h"
#include <string.h>
#include "host/host.h"

#ifdef  ENABLE_DMA_MEMCOPY
#include "mmp_dma.h"
#endif

/****************************************************************************
 *
 *	if USE_ONLYxxx is defined then comment out interface of this layer
 *
 ***************************************************************************/

#if ((!USE_ONLY512) && (!USE_ONLY2048) && (!USE_ONLY4096)&& (!USE_ONLY8192))

/****************************************************************************
 *
 * static variables
 *
 ***************************************************************************/

static unsigned char hl_realbuff[MAX_DATA_SIZE+4];
static unsigned char* hl_buff=hl_realbuff;
static unsigned long hl_abspos;
static unsigned long hl_relpos;
static unsigned long hl_secnum;
static unsigned char hl_state;
static unsigned char hl_page_shift;
static unsigned char hl_page_mask;
static unsigned char hl_page_mul;

#ifdef  ENABLE_CONTINUE_WRITE
static unsigned char hl_laststate=HL_CLOSE;
static unsigned long hl_lastlba=0;
static unsigned long hl_lastlength=0;
static unsigned long hl_lastabspos=0;

static unsigned char DirectMemCpyFlag=0;
static unsigned char* hl_datap=NULL;
static unsigned char hl_ftlstatus=HL_CLOSE;
#endif

#ifdef  ENABLE_DMA_MEMCOPY
static MMP_DMA_CONTEXT g_FTL_hl_DmaContent = MMP_NULL;
#endif
/****************************************************************************
 *
 * hl_init
 *
 * Initialize this layer, this function must be called at the begining
 *
 * RETURNS
 *
 * 0 - if ok
 * other if any error
 *
 ***************************************************************************/

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
#ifdef  ENABLE_DMA_MEMCOPY
void Mem_DmaCopy(void* dst, void* src, MMP_UINT32 size);
#endif
////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
t_bit hl_init(void) 
{
	t_bit ret;
	hl_state=HL_INIT;
	hl_abspos=0xffffffff; /* set to init state */

	#ifdef  ENABLE_CONTINUE_WRITE
	hl_laststate = HL_CLOSE;
	#endif

	ret=ml_init();
	if (!ret) hl_state=HL_CLOSE;

	if (gl_pagesize>MAX_DATA_SIZE) 
	{
		hl_state=HL_INIT;
		return 1;	/* page size cannot be bigger than the maximum allowed */
	}

	if (gl_pagesize==1024) 
	{
		hl_page_shift=1;
		hl_page_mask=1;
		hl_page_mul=2;
	}
	else if (gl_pagesize==2048) 
	{
		hl_page_shift=2;
		hl_page_mask=3;
		hl_page_mul=4;
	}
	else if (gl_pagesize==4096) 
	{
		hl_page_shift=3;
		hl_page_mask=7;
		hl_page_mul=8;
	}
	else if (gl_pagesize==8192) 
	{
		hl_page_shift=4;
		hl_page_mask=15;
		hl_page_mul=16;
	}
	else if (gl_pagesize!=512) 
	{
		hl_state=HL_INIT;
		return 1;	/* page size cannot be other than 512,1024,2048,4096 or 8192 */
	}

	if ( ((long)(hl_buff)) & (sizeof(long)-1) )
	{
		hl_buff=(hl_buff+(4-( ((long)(hl_buff)) & (sizeof(long)-1) )));
	}

	#ifdef  ENABLE_DMA_MEMCOPY
	if ( g_FTL_hl_DmaContent == MMP_NULL )
    {
        mmpDmaInitialize();
        if ( mmpDmaCreateContext(&g_FTL_hl_DmaContent) )
        {
            printf("FTL_HL_Initialize: Initial DMA fail.\n");
            return 1;	
        }
    }
    #endif

	return ret;
}

/****************************************************************************
 *
 * hl_format
 *
 * Formatting the layer, this function must be called once at manufacturing
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit hl_format(void) 
{
	t_bit ret;

	hl_state=HL_INIT;
	hl_abspos=0xffffffff; /* set to init state */
	ret=ml_format();

	if (!ret) hl_state=HL_CLOSE;
	return ret;
}

/****************************************************************************
 *
 * hl_open
 *
 * Opening the chanel for read or write from a specified sector
 *
 * INPUTS
 *
 * sector - start of sector for read or write
 * secnum - number of sector will be read or written
 * mode - HL_READ open for read, HL_WRITE open for write
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit hl_open(unsigned long sector, unsigned long secnum, unsigned char mode) 
{
	#ifdef  ENABLE_CONTINUE_WRITE
	hl_lastlba=0;
	hl_lastlength=0;
	hl_lastabspos=0;

	if(DirectMemCpyFlag)
	{	
		printf("hlopen: err, DirectMemCpyFlag = %x\n",DirectMemCpyFlag);
		DirectMemCpyFlag = 0;	
	}

	if( mode==HL_WRITE )
	{
		hl_lastlba=sector;
		hl_lastlength=secnum;		
		hl_lastabspos=hl_abspos;
	}	
	
	if(hl_laststate==HL_WRITE)
	{
		if(mode!=HL_WRITE)
		{
			if (ml_open(hl_abspos,1,ML_WRITE)) return 1;
			if (ml_write(hl_buff)) return 1;
			if (ml_close()) return 1;
			hl_laststate=HL_CLOSE;
		}	
	}
	#endif
	
	if (gl_pagesize==512) 
	{
		if (mode==HL_READ) return ml_open(sector,secnum,ML_READ);
		if (mode==HL_WRITE) return ml_open(sector,secnum,ML_WRITE);
		return 1;
	}
	else if (hl_state==HL_CLOSE) 
	{
		unsigned long abspos=sector>>hl_page_shift;
		unsigned long relpos=sector&hl_page_mask;
		unsigned long tmpsecnum=secnum;

		hl_relpos=relpos;
		hl_secnum=secnum;

		if (mode==HL_READ) 
		{
			if (hl_abspos!=abspos) 
			{
				if (ml_open(abspos,1,ML_READ)) return 1;
				if (ml_read(hl_buff)) return 1;
				if (ml_close()) return 1;
				hl_abspos=abspos;
			}

			hl_state=HL_READ;
			return 0;
		}
		else if (mode==HL_WRITE) 
		{
			#ifdef  ENABLE_CONTINUE_WRITE
			if (hl_abspos!=abspos) 
			{
				if(hl_laststate==HL_WRITE)
				{
					if (ml_open(hl_abspos,1,ML_WRITE)) return 1;
					if (ml_write(hl_buff)) return 1;
					if (ml_close()) return 1;
					hl_laststate = HL_CLOSE;
				}				
			}
			#endif
			
			if (relpos || secnum<hl_page_mul) 
			{
				/* judge if it need readback src page to hl_buffer, SP!=0 or SN<4. */
				tmpsecnum+=relpos;
				if (hl_abspos!=abspos) 
				{
					if (ml_open(abspos,1,ML_READ)) return 1;
					if (ml_read(hl_buff)) return 1;
					if (ml_close()) return 1;
					hl_abspos=abspos;
				}
			}
			else hl_abspos=abspos;

			if (tmpsecnum&hl_page_mask) 
			{
				tmpsecnum-=tmpsecnum&hl_page_mask;/* cut the last, that will be written separately */
			}

			tmpsecnum>>=hl_page_shift;
			
			#ifdef  ENABLE_CONTINUE_WRITE
			if( ((hl_relpos+secnum)%hl_page_mul)==0 )
			{
				tmpsecnum--;
			}			
			#endif
			
			if (tmpsecnum) 
			{
				if (ml_open(abspos,tmpsecnum,ML_WRITE)) return 1;
			}
			#ifndef  ENABLE_CONTINUE_WRITE
			else 
			{
			 	if (ml_open(abspos,1,ML_WRITE)) return 1;
			}
			#endif

			hl_state=HL_WRITE;
			return 0;
		}
	}

	return 1;
}

/****************************************************************************
 *
 * hl_write
 *
 * Writing sector data
 *
 * INPUTS
 *
 * data - data pointer for an array which length is sector size
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit hl_write(unsigned char *datap) 
{
	if (gl_pagesize==512) 
	{
		return ml_write(datap);
	}
	else if (hl_state==HL_WRITE) 
	{
		if (!hl_secnum) 
		{
		 	hl_state=HL_ABORT;
		 	return 1;
		}

		if (hl_relpos>=hl_page_mul) 
		{
			hl_state=HL_ABORT;
			#ifdef  ENABLE_CONTINUE_WRITE
			if(DirectMemCpyFlag)
			{
				DirectMemCpyFlag = 0;

				if(!hl_datap)
				{
					printf("err, hl_datap=NULL\r\n");
					return 1;
				}
				
				if ( ml_write( hl_datap ) ) return 1;
				
				hl_datap=NULL;
			}
			else
			{
			if (ml_write(hl_buff)) return 1;
			}
			#else
			if (ml_write(hl_buff)) return 1;
			#endif

			hl_abspos++;

			if (hl_secnum<hl_page_mul) 
			{ /* read remaining */
				if (ml_close()) return 1;

				if (ml_open(hl_abspos,1,ML_READ)) return 1;
				if (ml_read(hl_buff)) return 1;
				if (ml_close()) return 1;

				#ifndef  ENABLE_CONTINUE_WRITE
				if (ml_open(hl_abspos,1,ML_WRITE)) return 1;
				#endif
			}

			#ifdef  ENABLE_CONTINUE_WRITE
			if(hl_secnum>hl_page_mul)
			{
				DirectMemCpyFlag = 1;	
				hl_datap = datap;
			}
			#endif

			hl_state=HL_WRITE;
			hl_relpos=0;
		}

		hl_secnum--;
		#ifdef  ENABLE_CONTINUE_WRITE
		if(!DirectMemCpyFlag)
		{
		(void)memcpy(hl_buff+hl_relpos*512,datap,512);
		}
		else
		{
			if( (hl_secnum+hl_relpos+1)<=hl_page_mul )
			{
				printf("Err,data does not in wbf,(%x,%x)\r\n",hl_relpos,hl_secnum+1);
			}
		}
		#else
		(void)memcpy(hl_buff+hl_relpos*512,datap,512);
		#endif

		hl_relpos++;
		return 0;
	}

	return 1;
}

/****************************************************************************
 *
 * hl_read
 *
 * read sector data
 *
 * INPUTS
 *
 * data - where to read a given sector
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit hl_read(unsigned char *datap) 
{
	if (gl_pagesize==512) 
	{
		return ml_read(datap);
	}
	else if (hl_state==HL_READ) 
	{
		if (!hl_secnum) 
		{
		 	hl_state=HL_ABORT;
		 	return 1;
		}
		hl_secnum--;

		if (hl_relpos>=hl_page_mul) 
		{
			hl_state=HL_ABORT;
			hl_abspos++;
			
			#ifdef  ENABLE_CONTINUE_WRITE
			if(hl_secnum>=hl_page_mul)
			{
				if (ml_open(hl_abspos,1,ML_READ)) return 1;
				if (ml_read(datap)) return 1;
				if (ml_close()) return 1;
				DirectMemCpyFlag = 1;
			}
			else
			{
				if (ml_open(hl_abspos,1,ML_READ)) return 1;
				if (ml_read(hl_buff)) return 1;
				if (ml_close()) return 1;
				DirectMemCpyFlag = 0;
			}
			#else
			if (ml_open(hl_abspos,1,ML_READ)) return 1;
			if (ml_read(hl_buff)) return 1;
			if (ml_close()) return 1;
			#endif
			hl_state=HL_READ;
			hl_relpos=0;
		}

		#ifdef  ENABLE_CONTINUE_WRITE
		if(!DirectMemCpyFlag)
		{
		(void)memcpy(datap,hl_buff+hl_relpos*512,512);
		}		
		#else		
		(void)memcpy(datap,hl_buff+hl_relpos*512,512);
		#endif
		hl_relpos++;
		return 0;
	}

	return 1;
}

/****************************************************************************
 *
 * hl_close
 *
 * closing sector reading or writing
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit hl_close(void) 
{
	if (gl_pagesize==512) 
	{
		#ifdef  ENABLE_CONTINUE_WRITE
		hl_ftlstatus = HL_CLOSE;
		#endif
		return ml_close();
	}
	else 
	{
		if (hl_state==HL_READ) 
		{
			hl_state=HL_CLOSE;
		#ifdef  ENABLE_CONTINUE_WRITE
			hl_ftlstatus = HL_CLOSE;
		#endif
			if (!hl_secnum) return 0;
		}
		else if (hl_state==HL_WRITE) 
		{
			#ifdef  ENABLE_CONTINUE_WRITE
			hl_laststate=HL_WRITE;
			hl_state=HL_CLOSE;	
			hl_ftlstatus = 0;

			if( hl_lastabspos != (hl_lastlba/4) )
			{					
				if(hl_lastlength<0x10)
				{
					hl_ftlstatus = HL_WRITE;
				}
			}			
			#else
			hl_state=HL_CLOSE;
			if (ml_write(hl_buff)) return 1;
			if (ml_close()) return 1;
			#endif		

			if (!hl_secnum) return 0;
		}

		hl_abspos=0xffffffff; /* set to init state buffer is not realiable */
	 	hl_state=HL_CLOSE;
	 	#ifdef  ENABLE_CONTINUE_WRITE
	 	hl_ftlstatus = HL_CLOSE;
	 	#endif
	}

	return 1;
}

/****************************************************************************
 *
 * hl_getmaxsector
 *
 * retreives the maximum number of sectors can be used in the system
 *
 * RETURNS
 *
 * maximum number of sectors
 *
 ***************************************************************************/

unsigned long hl_getmaxsector()
{
	if (gl_pagesize==512)
	{
		return ml_getmaxsector();
	}

	return ml_getmaxsector() << hl_page_shift;
}


/****************************************************************************
 *
 * Mem_DmaCopy
 *
 * retreives the maximum number of sectors can be used in the system
 *
 * RETURNS
 *
 * maximum number of sectors
 *
 ***************************************************************************/
#ifdef  ENABLE_DMA_MEMCOPY
void Mem_DmaCopy(void* dst, void* src, MMP_UINT32 size)
{
    const MMP_UINT32 attribList[] =
    {
        MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
        MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)src,
        MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)dst,
        MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)size,
        MMP_DMA_ATTRIB_NONE
    };
    
    mmpDmaSetAttrib(g_FTL_hl_DmaContent, attribList);
    mmpDmaFire(g_FTL_hl_DmaContent);
    
    if (mmpDmaWaitIdle(g_FTL_hl_DmaContent))
    {
        printf("hlDmaCopy: Copy Failed.\n");
    }
    
}
#endif
/****************************************************************************
 *
 *	end of USE_ONLYxxx
 *
 ***************************************************************************/

#endif	/* !USE_ONLY512 !USE_ONLY2048 !USE_ONLY4096*/

/****************************************************************************
 *
 * End of hlayer.c
 *
 ***************************************************************************/

