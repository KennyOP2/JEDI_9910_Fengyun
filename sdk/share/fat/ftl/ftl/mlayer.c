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

#include "mlayer.h"
#include "phy/llayer.h"
#include "wear.h"
#include "string.h"

/****************************************************************************
 *
 * Definitions for debug version
 *
 ***************************************************************************/

#define _HCC_MLAYER_C_
#include "mdebug.h"

#if _HCC_MDEBUG_ENABLED_
#include "../../src_int/prgmacro.h"
#endif

#if defined(__FREERTOS__)
#define _ENDIAN_TRANSFORM 1  // For big-endian processor
#else
#define _ENDIAN_TRANSFORM 0  // For little-endian processor
#endif

/****************************************************************************
 *
 *	global Variables
 *
 ***************************************************************************/

t_ba *gl_freetable;
unsigned long gl_numofblocks;
unsigned char gl_pageperblock;
unsigned long gl_pagesize;
unsigned long gl_reservedblocks;

unsigned long gl_free_block;
unsigned long gl_log_block;
t_frag gl_max_frag_per_mapblock; /* maximum number of frag per blk */


/****************************************************************************
 *
 *	static Variables
 *
 ***************************************************************************/

/* main data and system tmp buffer in long aligned area*/
static unsigned long ml_lbuffer[MAX_PAGE_SIZE/4]; 
static unsigned char *ml_buffer; /* main data and system tmp buffer */

static union
{
	unsigned char basebuff[(MAX_DATA_SIZE/2) + MAX_SPARE_SIZE + (MAX_DATA_SIZE/2)*MAX_CACHEFRAG];
/* This buffer is for system using */
/* in the 1st half (MAX_DATA_SIZE/2) there is static wear info, the freelog table and mapinfo blocks */
/* then MAX_SPARE_SIZE for spare area for runtime using */
/* then the 1st cache fragment area (MAX_DATA_SIZE/2) */
/* then remaning fragment cache buffer areas */
/* This buffer is used at startup also, so its size has to be a complete page size with spare */
	ST_STATIC staticbuff;
/* created union for aligned structure */
} ml_u;

static ST_LOG ml_log[MAX_LOG_BLOCK_AVAILABLE];
static ST_LOG *ml_curlog;
static unsigned char ml_logblocknum;
static unsigned char ml_logmerge;

static t_ba ml_lba;
static t_po ml_lpo;
static t_ba ml_pba;

static unsigned long ml_seccou;

static ST_FRAG ml_frag;

static unsigned char ml_state;

static ST_MAPINFO *ml_mapinfo;

static unsigned char ml_save_map_need;

static unsigned long ml_max_mappagecou_lo;
static unsigned long ml_max_mappagecou_hi; /* for searching the maximum! */

static ST_MAPBLOCK ml_mapblocks[MAX_NUM_OF_DIF_MAPBLOCK];

static ST_MAPBLOCK *ml_stepback;
static unsigned long ml_data_blk_count;

static ST_MAPDIR ml_mapdir[MAX_FRAGNUM_AVAILABLE];

/****************************************************************************
 *
 * ml_chk_frag
 *
 * function checks if requested fragment is in the cache area
 * and set ppba pointer its start address if found
 *
 * INPUTS
 *
 * frag - frag to check if its cached
 *
 * RETURNS
 *
 * 1 - if cached
 * 0 - if not
 *
 ***************************************************************************/

static t_bit ml_chk_frag(t_frag frag)
{
	unsigned char idx;

	for (idx=0; idx<MAX_CACHEFRAG; idx++)	/* run throught on cache area */
	{
		if (ml_frag.cached[idx]==frag)		/* if we found the frag in cache */
		{
			ml_frag.ppba=ml_frag.ppbas[idx];/* set global pointer onto it */
			return 1;						/* signal found */
		}
	}

	return 0;								/* not found */
}

/****************************************************************************
 *
 * ml_load_curr_frag
 *
 * load frag into RAM and set as current frag
 *
 * INPUTS
 *
 * frag - fragment number to load
 *
 * RETURNS
 *
 * zero if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_load_curr_frag(t_frag frag)
{
	ml_frag.current=frag;				/* set as current */

	if (!ml_chk_frag(ml_frag.current)) 	/* load if not cached */
	{
		unsigned char *buf;
		ST_MAPDIR *map;

		ml_frag.pos++;								   /* try to use next frag cache entry */
		if (ml_frag.pos>=MAX_CACHEFRAG) ml_frag.pos=0; /* check if reach the last entry */

		buf=(unsigned char*)ml_frag.ppbas[ml_frag.pos]; /* get cache entry buffer's */
		map=&ml_mapdir[ml_frag.current];				/* get current frag map pointer */

		if (ll_readpart(map->pba,map->ppo,buf,map->index)) { /* read that half part */
			ml_frag.cached[ml_frag.pos]=FRAG_NA; 		 /* if error then set cache entry to n/a */
			return 1;
		}

		ml_frag.cached[ml_frag.pos]=ml_frag.current; /* set cache field to current fragnumber*/
		ml_frag.ppba=(t_ba *)buf;		        	  /* set current fragment buffer ptr */
	}

	return 0;
}

/****************************************************************************
 *
 * ml_get_log2phy
 *
 * convert logical address to physical
 *
 * INPUTS
 *
 * lba - logical block address
 *
 * RETURNS
 *
 * physical address or BLK_NA if address is invalid
 *
 ***************************************************************************/

static t_ba ml_get_log2phy(t_ba lba)
{
	if (lba>=(gl_numofblocks-gl_free_block-gl_reservedblocks)) return BLK_NA; /* check if it is in valid range */

	if (ml_load_curr_frag((t_frag)(lba/MAX_FRAGSIZE))) return BLK_NA;  /* load current fragment */

	return ml_ToBigEndian32(ml_frag.ppba[lba%MAX_FRAGSIZE]); 			  /* return it's physical address */
}

/****************************************************************************
 *
 * ml_set_log2phy
 *
 * set logical block's physical address, 1st ml_get_log2phy function has to
 * be called to get the fragments into the cached area and ppba must be set
 *
 * INPUTS
 *
 * lba - logical block address
 * pba - physical block address
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_set_log2phy(t_ba lba,t_ba pba)
{
	unsigned short index;

	if (lba>=(gl_numofblocks-gl_free_block-gl_reservedblocks))	/* check if lba is valid */
	{
		return 1;
	}

	index=(unsigned short)(lba%MAX_FRAGSIZE);	/* get index in the fragment */
	ml_frag.ppba[index] = ml_ToLittleEndian32(pba);					/* set pba into the fragment */

	return 0;
}

/****************************************************************************
 *
 * ml_lowinit
 *
 * low level initialization, this must be called from ml_init and ml_format
 *
 * RETURN
 *
 * 0 - if ok
 * other if error
 *
 ***************************************************************************/

static t_bit ml_lowinit()
{
	int a;

	ml_buffer=(unsigned char *)ml_lbuffer; /* setting up tmp ptr from long aligned area */

	DEBOPEN;

	gl_free_block=MAX_FREE_BLOCK_AVAILABLE;
	gl_log_block=MAX_LOG_BLOCK_AVAILABLE;
	gl_reservedblocks=MAX_RESERVEDBLOCKS;

	ml_state=ML_INIT;

	gl_numofblocks=0;
	gl_pageperblock=0;
	gl_pagesize=0;

	if (ll_init()) return 1;
		
	if (gl_numofblocks==0) return 1;
	if (gl_pageperblock==0) return 1;
	if (gl_pagesize==0) return 1;

	if (gl_numofblocks>MAX_BLOCK_AVAILABLE) return 1;
	if (gl_pageperblock>MAX_PAGE_PER_BLOCK_AVAILABLE) return 1;
	if (gl_pagesize>MAX_DATA_SIZE) return 1;

	if (gl_free_block>MAX_FREE_BLOCK_AVAILABLE) return 1;
	if (gl_log_block>MAX_LOG_BLOCK_AVAILABLE) return 1;

	gl_max_frag_per_mapblock = (t_frag)((gl_numofblocks/ MAX_FRAGSIZE)/MAX_NUM_OF_DIF_MAPBLOCK);
	if (!gl_max_frag_per_mapblock) gl_max_frag_per_mapblock=2; /* max frag per block cannot be zero */
	if (gl_max_frag_per_mapblock & 1) gl_max_frag_per_mapblock++; /* max frag per block must be even number */

	ml_curlog=0;
	ml_logblocknum=0;
	ml_logmerge=0;

	(void)memset (ml_log,0xff,sizeof(ml_log)); /* reset all entries */

	/* allocate ptrs in the half page */
	wear_init(&ml_u.staticbuff); /* it uses this stored buffer */
	ml_mapinfo = (ST_MAPINFO *)(ml_u.basebuff+sizeof(ST_STATIC));
	gl_freetable=(t_ba *)(ml_u.basebuff+sizeof(ST_STATIC)+sizeof(ST_MAPINFO));

	/* check if data above is fit into a half page */
	{
		int freesize=(int)((long)(ml_u.basebuff+gl_pagesize/2)-(long)(&gl_freetable[gl_free_block]));
		if (freesize<0) /* this size must be positive, showing how many bytes are left there */
		{
			return 1; /* FATAL: freetable is not fit into a half page */
		}
	}

	/* check spare size */
	if (gl_pagesize/32<sizeof(ST_SPARE))
	{
		/* pagesize in a 512 data device cannot be more than 16 bytes */
		/* or pagesize in a 2048 data device cannot be more than 64 bytes */
		return 1; /* FATAL */
	}

	/* check necessary map shadow blocks */
	if ( ((gl_max_frag_per_mapblock/2)/gl_pageperblock)+1 > MAX_MAPBLOCK_SHADOW )
	{
		return 1; /* FATAL */
	}

	/* reset fragment cache */
	ml_frag.pos=0;
	for (a=0; a<MAX_CACHEFRAG;a++)
	{ /* set all cached frag entry to NA */
		ml_frag.cached[a]=FRAG_NA;
	}
	ml_frag.current=FRAG_NA;		 /* set current frag also NA */

	/* filling variables of mapblocks */
	for (a=0; a<MAX_NUM_OF_DIF_MAPBLOCK; a++)
	{
		ST_MAPBLOCK *mapblock=&ml_mapblocks[a];

		mapblock->last_pba   = BLK_NA;
		mapblock->last_ppo   = INDEX_NA;
		mapblock->block_type = (unsigned char)(BLK_TYPE_MAP_ORI+a);
		mapblock->mapdir     = &ml_mapdir[a*gl_max_frag_per_mapblock];
		mapblock->pba        = BLK_NA;
		mapblock->ppo        = INDEX_NA;
		mapblock->ref_count  = 0;
		mapblock->start_frag = (t_frag)(a*gl_max_frag_per_mapblock);
		mapblock->end_frag   = (t_frag)(mapblock->start_frag+gl_max_frag_per_mapblock);

		mapblock->shadowidxcou=0;
		(void)memset(mapblock->shadowidx,INDEX_NA,MAX_MAPBLOCK_SHADOW);
	}

	ml_max_mappagecou_lo=0;
	ml_max_mappagecou_hi=0;

	ml_save_map_need=0;

	(void)memset(ml_mapdir,0xff,sizeof(ml_mapdir));
	ml_data_blk_count=0;

	/* set all cache entry half buffer's */
	for (a=0; a<MAX_CACHEFRAG; a++)
	{
		unsigned char *buf=
		   ml_u.basebuff+(MAX_DATA_SIZE/2)+MAX_SPARE_SIZE+(MAX_DATA_SIZE/2)*a;
		   /* fragment cache buffers, use only half data size buffer, see base buff definition  */
		ml_frag.ppbas[a]=(t_ba *)buf; /* store its current buffer */
	}

	return 0;
}

/****************************************************************************
 *
 * ml_buildmap
 *
 * function for rebuilding mapdir
 *
 * INPUTS
 *
 * mapblock - map block to be build
 *
 * RETURNS
 *
 * 0 - if ok
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_buildmap(ST_MAPBLOCK *map)
{
	ST_SPARE *sptr=GET_SPARE_AREA(ml_buffer);
	unsigned char mappage;
	unsigned char cou;
	unsigned char index;
	unsigned char idxcou;
	ST_MAPDIR *mapdir;
	t_frag frag,fragcou;
	t_ba pba;
	t_po ppo;

	DEBPR0("ml_buildmap\n");

again: /* label for step back */

	/* set counter to not available */
	map->shadowidxcou=INDEX_NA;

	/* go through on all index entry */
	for (cou=0; cou<MAX_MAPBLOCK_SHADOW; cou++)
	{
		index=ml_mapinfo->shadowidx[cou];	/* get the index value */
		map->shadowidx[cou]=index;			/* copy back shadow index */

		if (index!=INDEX_NA)				/* check if valid */
		{
			map->shadowidxcou=cou;			/* update indexcou */
		}
	}

	/* check if we found proper shadow index counter */
	if (map->shadowidxcou==INDEX_NA)
	{
		return 1; /* fatal */
	}

	map->shadowidxcou++; /* goto the next entry */

	/* get the counter, if step back then it is also ok */
	map->mappagecou_hi = ml_ToBigEndian32(ml_mapinfo->mappagecou_hi);
	map->mappagecou_lo = ml_ToBigEndian32(ml_mapinfo->mappagecou_lo);

	/* check if this is the maximum counter */
	if (ml_ToBigEndian32(ml_mapinfo->mappagecou_hi) > ml_max_mappagecou_hi) /* check if high value is bigger */
	{
		ml_max_mappagecou_lo = ml_ToBigEndian32(ml_mapinfo->mappagecou_lo);	/* set new value of low */
		ml_max_mappagecou_hi = ml_ToBigEndian32(ml_mapinfo->mappagecou_hi); /* set new value of high */
	}
	else {
		/* check if high value is equal and low value is bigger */
		if (   ml_ToBigEndian32(ml_mapinfo->mappagecou_hi) == ml_max_mappagecou_hi 
            && ml_ToBigEndian32(ml_mapinfo->mappagecou_lo) > ml_max_mappagecou_lo)
		{
			ml_max_mappagecou_lo = ml_ToBigEndian32(ml_mapinfo->mappagecou_lo);	/* set new value of low */
			ml_max_mappagecou_hi = ml_ToBigEndian32(ml_mapinfo->mappagecou_hi);	/* set new value of high */
		}
	}

	mappage=0;		/* signal we didn't find any map page */
	idxcou=0;		/* reset index counter */

	index=map->shadowidx[idxcou];	/* get its value */

	/* 1st index cannot be not available */
	if (index==INDEX_NA)
	{
		return 1;
	}

	pba=(t_ba)(ml_ToBigEndian32(gl_freetable[index])&FT_ADDRMASK);	/* get pba from index */
	ppo=0;											/* ppo value starts from 0 */
	frag=map->start_frag;							/* initialize start fragment */

	/* collect original fragment's data */
	mapdir=&ml_mapdir[frag];						/* initialize mapdir ptr */
	fragcou=(t_frag)(gl_max_frag_per_mapblock/2);	/* 2 fragments per page */
	while (fragcou)
	{
		if (ppo==gl_pageperblock) 
		{
			idxcou++;								   /* goto next shadow index */
			if (idxcou==MAX_MAPBLOCK_SHADOW)
			{
				return 1; /* fatal, there are missing frags */
			}

			index=map->shadowidx[idxcou];	/* get its value */
			if (index==INDEX_NA)
			{  /* index cannot be not available until all frags is not collected */
				return 1;
			}

			pba=(t_ba)(ml_ToBigEndian32(gl_freetable[index])&FT_ADDRMASK);	/* get pba from index */
			ppo=0;											/* ppo value starts from 0 */
		}

		if (!ll_readpart(pba,ppo,(unsigned char*)sptr,LL_RP_SPARE)) /* read page's spare area */
		{
			if (sptr->u.map.block_type & BLK_MAPPAGE_FLAG) /* check page type */
			{
				if (!ppo)	/* 1st page is always map page other must be original until collected */
				{
					mapdir=&ml_mapdir[ml_ToBigEndian16(sptr->u.map.frag)];	/* get the mapdir from it */

					mapdir->pba=pba;			/* set fragment pba */
					mapdir->ppo=ppo;			/* set fragment ppo */
					mapdir->index=1;			/* set fragment index */

					mapdir=&ml_mapdir[frag];	/* initialize mapdir ptr back */

					mappage=1;					/* we found a mappage */
				}
				else
				{
					return 1; /* only original fragment can be here, mappage found */
				}
			}
			else 
			{
				if (ml_ToBigEndian16(sptr->u.map.frag) != frag) /* check stored fragment pair */
				{
					return 1; /* fatal, fragment is not equal */
				}

				mapdir->pba=pba;	/* set current pba into mapdir */
				mapdir->ppo=ppo;	/* set current ppo into mapdir */
				mapdir->index=1;	/* set 1st part index into mapdir */
				mapdir++;			/* go to next mapdir entry */

				mapdir->pba=pba;	/* set current pba into mapdir */
				mapdir->ppo=ppo;	/* set current ppo into mapdir */
				mapdir->index=0;	/* set 2nd part index into mapdir */
				mapdir++;			/* goto next mapdir entry */

				frag+=2;			/* go to next 2 fragment */
				fragcou--;			/* decrementing pair counter */
			}

			ppo++; /* goto next page */
		}
		else
		{
			/* broken mapdir, so step back */
			if (ml_stepback) /* check if there was any step back */
			{
				return 1; /* only once is allowed */
			}

			ml_stepback=map;		   /* set step back value to the current map block */

			/* check if pba and ppo valid for step back*/
			if (   ml_ToBigEndian32(ml_mapinfo->last_pba) == BLK_NA 
                || ml_mapinfo->last_ppo == INDEX_NA)
			{
				return 1; /* can't step back */
			}

			map->pba = ml_ToBigEndian32(ml_mapinfo->last_pba);	/* get last stored pba of mappage */
			map->ppo = ml_mapinfo->last_ppo;	/* get last stored ppo of mappage */

			/* read previous value into base buff */
			if (ll_readpart(map->pba,map->ppo,ml_u.basebuff,LL_RP_2NDHALF))
			{
				return 1; /* fatal */
			}

			map->last_pba=map->pba; /* update last information of pba */
			map->last_ppo=map->ppo;	/* update last information of ppo */

			map->ppo++; /* set next position */

			goto again;
		}
	}

	/* check and get fragments from mappage */
	for (;;)
	{
		if (ppo==gl_pageperblock) 
		{
			idxcou++;								   /* goto next shadow index */
			if (idxcou==MAX_MAPBLOCK_SHADOW) break; /* finished, no more shadow block */

			index=map->shadowidx[idxcou];	/* get its value */
			if (index==INDEX_NA) break;	/* finished, no more shadow block indexed */

			pba=(t_ba)(ml_ToBigEndian32(gl_freetable[index])&FT_ADDRMASK);	/* get pba from index */
			ppo=0;											/* ppo value starts from 0 */
		}

		if (!ll_readpart(pba,ppo,(unsigned char*)sptr,LL_RP_SPARE))	/* read spare area */
		{
			if (sptr->u.map.block_type & BLK_MAPPAGE_FLAG) /* check if it is a map page*/
			{
				mapdir=&ml_mapdir[ml_ToBigEndian16(sptr->u.map.frag)];	/* get the mapdir from it */

				mapdir->pba=pba;			/* set fragment pba */
				mapdir->ppo=ppo;			/* set fragment ppo */
				mapdir->index=1;			/* set fragment index */

				mappage=1;					/* we found a mappage */
			}
			else
			{
				if (ppo)
				{
					return 1; /* fatal only mappage can be here */
				}
			}
		}

		ppo++;	/* go to next page */
	}

	/* check if any mappage found */
	if (!mappage)
	{
		return 1; /* no mappage found, eg. FLT missing after format */
	}

	return 0; /* successfuly built */
}


/****************************************************************************
 *
 * check_reference_count
 *
 * function check and set the latest map reference counter
 *
 * INPUTS
 *
 * mapnum - which map need to be checked
 * ref_count - current reference counter
 * pba - current block physical address
 *
 ***************************************************************************/

static void check_reference_count(unsigned char mapnum,unsigned long ref_count,t_ba pba) 
{
	ST_MAPBLOCK *map=&ml_mapblocks[mapnum];

	DEBPR3("check_reference_count pba:%08x mapnum:%d refcou:%08x\n",pba,mapnum,ref_count);
	if (map->pba!=BLK_NA)  /* if we have already found map block */
	{
		/* check overflow as well, because 32 bit counter can overflow, distance check in range */
		if (ref_count>map->ref_count) 
		{
			if (ref_count-map->ref_count > 64) return;
		}
		else 
		{
			if (map->ref_count-ref_count < 64) return;
		}
	}

	DEBPR0("found this is the latest!\n");

	map->pba=pba;
	map->ref_count=ref_count;
}

/****************************************************************************
 *
 * ml_preeraseblk
 *
 * allocate a block from free table and erases it
 *
 * RETURNS
 *
 * index in free table or INDEX_NA if any error
 *
 ***************************************************************************/

#if FTL_DELETE_CONTENT
void ml_preeraseblk(unsigned char index)
{
	t_ba pba;

	pba=gl_freetable[index];

	if (pba & (FT_MAP | FT_LOG))
	{
		/* cannot be erased this is in use */
		return;
	}

	if (wll_erase(103,pba))
	{
		gl_freetable[index] |= FT_BAD;    /* signal it and set as BAD */
	}
}
#endif

/****************************************************************************
 *
 * ml_alloc
 *
 * allocate a block from free table and erases it
 *
 * RETURNS
 *
 * index in free table or INDEX_NA if any error
 *
 ***************************************************************************/

static unsigned char ml_alloc()
{
	unsigned char index;

	for (;;)
	{	/* loop for alloc */
		t_ba pba;

		wear_alloc();
		index=gl_wear_allocstruct.free_index;

		DEBPR1("ml_alloc index %d\n",index);

		if (index==INDEX_NA)
		{
		    return INDEX_NA; /* FATAL error no free blocks are available */
		}

		pba = ml_ToBigEndian32(gl_freetable[index]);

		DEBPR2("ml_alloc pba %d, %08x\n", pba,pba);

#if FTL_DELETE_CONTENT
		/* all block is erased in free table */
		return index;
#else
		if (wll_erase(100,pba) == LL_OK)
		{
			return index; /* erase is ok, so return with the index */
		}

        DEBPR1("ml_alloc: Erase of block %d failed\n", pba);

		gl_freetable[index] |= ml_ToLittleEndian32(FT_BAD);     /* signal it and locked as BAD */

		/* lets allocate another one from freetable*/
#endif
	}
}

/****************************************************************************
 *
 * get_mapblock
 *
 * Function for retreiving the map according fragment counter
 *
 * INPUTS
 *
 * frag - fragment number
 *
 * RETURNS
 *
 * map pointer where the fragment is or NULL if any error
 *
 ***************************************************************************/

ST_MAPBLOCK *get_mapblock(t_frag frag)
{
	int index=(int)(frag/gl_max_frag_per_mapblock);	/* calculate index */

	if (index<MAX_NUM_OF_DIF_MAPBLOCK) 			/* if index is in the range*/
	{
		return &ml_mapblocks[index];			/* return requested mapblock ptr */
	}

	return 0;									/* return invalid (not found) */
}

/****************************************************************************
 *
 * alloc_mapblock
 *
 * allocate a new map block
 *
 * INPUTS
 *
 * mapblock - which map block need to be allocated
 *
 * RETURNS
 *
 * 0 - if successfuly
 * other if any error
 *
 ***************************************************************************/

static t_bit alloc_mapblock(ST_MAPBLOCK *mapblock)
{
	DEBPR0("alloc_mapblock\n");

	if (mapblock->shadowidxcou!=MAX_MAPBLOCK_SHADOW) /* check if we reach the maximum */
	{
		unsigned char index = ml_alloc();	/* allocate a block */
		if (index==INDEX_NA) 
			return 1;		/* fatal */

		mapblock->pba = ml_ToBigEndian32(gl_freetable[index]);	/* set pba */
		mapblock->ppo = 0;					/* reset ppo */

		gl_freetable[index] |= ml_ToLittleEndian32(FT_MAP);		/* set MAP flag in FLT */

		wear_updatedynamicinfo(index,gl_wear_allocstruct.free_wear); /* update wear info */

		mapblock->ref_count++;				   /* increase counter */
		mapblock->ref_count&=255; 			   /* keep in range */

		mapblock->shadowidx[mapblock->shadowidxcou++]=index; /* put the index of the block back */

		DEBPR2("map_pba %08x flt %08x \n",mapblock->pba,gl_freetable[index])

		return 0; /* success */
	}

	return 1; /* FATAL */
}

/****************************************************************************
 *
 * write_frags
 *
 * subfunction for formatting to write current fragments in fragbuff
 *
 * INPUTS
 *
 * frag - current frag number
 *
 * RETURNS
 *
 * 0 - if successfuly
 * other if any error
 *
 ***************************************************************************/

static t_bit write_frags(t_frag frag, unsigned char *buff)
{
	ST_MAPBLOCK *mapblock=get_mapblock(frag);
	ST_SPARE *sptr=GET_SPARE_AREA(buff);
	ST_MAPDIR *mapdir;

	DEBPR0("write_frags\n");

	if (!mapblock) return 1;	/* FATAL, out of range */

	if (mapblock->ppo==gl_pageperblock) /* Check if there is any space */
	{
		if (alloc_mapblock(mapblock)) /* allocate another block */
		{
			return 1; /* FATAL */
		}
	}

	if (!mapblock->ppo) /* check if this is the 1st page of the mapblock */
	{
		sptr->wear            = ml_ToLittleEndian32(1);	/* here this is the 1st write of the block */
		sptr->u.map.ref_count = ml_ToLittleEndian32(mapblock->ref_count); /* write reference counter */
	}

	sptr->u.map.block_type = mapblock->block_type; /* signal original fragments here */
	sptr->u.map.frag       = ml_ToLittleEndian16(frag);	/* store the fragment number */

	if (wll_write(1,mapblock->pba,mapblock->ppo,buff,(unsigned char *)sptr)) return 1; /* fatal */

	DEBPR2("mapblock %02x frag %d (+1)\n",mapblock->block_type,frag);

	/* store mapdir info 2 frags are a pair in the fragment area */
	mapdir=&ml_mapdir[frag];	/* get its mapdir */
	mapdir->pba=mapblock->pba;
	mapdir->ppo=mapblock->ppo;
	mapdir->index=1;

	mapdir++;					/* goto the next mapdir (2nd frag) */
	mapdir->pba=mapblock->pba;
	mapdir->ppo=mapblock->ppo;
	mapdir->index=0;

	mapblock->ppo++;			/* goto next available */

	return 0;
}


/****************************************************************************
 *
 * get_fragment
 *
 * function copy the requested frag content to destionation buffer
 * if its cached then it copies other case it reads
 *
 * INPUTS
 *
 * frag - requested fragment
 * dest - destination buffer
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static void get_fragment(t_frag frag,unsigned char *dest) 
{
	if (ml_chk_frag(frag)) /* check if its cached */
	{
		(void)memcpy(dest,ml_frag.ppba,(int)gl_pagesize/2); /* simple copy */
	}
	else
	{
		ST_MAPDIR *mapdir=&ml_mapdir[frag]; /* getting mapdir */
		/* read requested part */
		if (ll_readpart(mapdir->pba,mapdir->ppo,dest,mapdir->index))
		{
			(void)memset(dest,0xff,(int)gl_pagesize/2); /* fills with ff-s if error */
		}
	}
}

/****************************************************************************
 *
 * store_mappage
 *
 * store a complete map page
 *
 * INPUTS
 *
 * mapblock - mapblock which contains fragments
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit store_mappage(ST_MAPBLOCK *mapblock)
{
	unsigned char cou;
	ST_SPARE *sptr;

	/* special spare_area pointer, only half buffer is used in write double! */
	sptr=((ST_SPARE*)(((unsigned char*)(ml_u.basebuff))+(gl_pagesize/2)));

	/* setting spare information about mappage */
	sptr->u.map.block_type = (unsigned char)(mapblock->block_type | BLK_MAPPAGE_FLAG); /* set map page here only! */
	sptr->u.map.frag       = ml_ToLittleEndian16(ml_frag.current);

	/* if we are in the 1st page then set special variable */
	if (!mapblock->ppo)
	{
		sptr->wear            = ml_ToLittleEndian32(gl_wear_allocstruct.free_wear); /* wear level information */
		sptr->u.map.ref_count = ml_ToLittleEndian32(mapblock->ref_count);	        /* reference counter */
	}

	/* update mapinfo about map blocks */
	for (cou=0; cou<MAX_MAPBLOCK_SHADOW; cou++)
	{
		ml_mapinfo->shadowidx[cou]=mapblock->shadowidx[cou]; /* copy shadow block's index */
	}

	/* update mapinfo about last success mappage */
	ml_mapinfo->last_pba = ml_ToLittleEndian32(mapblock->last_pba);
	ml_mapinfo->last_ppo = mapblock->last_ppo;

	/* increase mappage counter */
	ml_mapinfo->mappagecou_lo = ml_ToLittleEndian32(ml_ToBigEndian32(ml_mapinfo->mappagecou_lo)+1);
	//ml_mapinfo->mappagecou_lo++;
	if (!ml_ToBigEndian32(ml_mapinfo->mappagecou_lo))
	{
		ml_mapinfo->mappagecou_hi = ml_ToLittleEndian32(ml_ToBigEndian32(ml_mapinfo->mappagecou_hi)+1); /* 64bit counter */
		//ml_mapinfo->mappagecou_hi++; /* 64bit counter */
	}

	/* check if current fragment is cached (it has to be) */
	if (!ml_chk_frag(ml_frag.current))
	{
		return 1; /* fatal */
	}

	/* write 1st half of fragment, 2nd half of FLT in the map page */
	return wll_writedouble(30,mapblock->pba,mapblock->ppo,(unsigned char*)(ml_frag.ppba),ml_u.basebuff);
}

/****************************************************************************
 *
 * collect_frags
 *
 * function collects stored or cached fragments
 *
 * INPUTS
 *
 * mapblock - mapblock which contains fragments
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit collect_frags(ST_MAPBLOCK *mapblock)
{
	t_frag cou=(t_frag)(gl_max_frag_per_mapblock/2);/* 2 fragments per page */
	t_frag frag=mapblock->start_frag;	/* setting start fragment */
	unsigned char *dest;

	while (cou--)					/* collect all fragments */
	{
		if (mapblock->ppo==gl_pageperblock) /* Check if there is any space */
		{
			if (alloc_mapblock(mapblock)) /* allocate another block */
			{
				return 1; /* FATAL */
			}

			if (store_mappage(mapblock)) /* 1st page is always map page */
			{
				return 1;
			}

			mapblock->ppo++;	/* goto next page */
		}

		/* collect 1st half */
		dest=ml_buffer;				/* 1st half is for even frags */

		get_fragment(frag,dest);	/* getting fragment*/

		/* collect 2nd half */
		dest+=gl_pagesize/2;		/* 2nd half is for odd frags */

		get_fragment((t_frag)(frag+1),dest);	/* getting next fragment*/

		/* simple writes them */
		if (write_frags(frag,ml_buffer)) return 1;

		frag+=2;					/* 2 fragment per page */
	}

	return 0;
}

/****************************************************************************
 *
 * ml_save_map
 *
 * Save map informations (fragments, free table modification)
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_save_map()
{
	ST_MAPBLOCK  *mapblock=get_mapblock(ml_frag.current);	/* get current map block */
	unsigned char domapmerge=0;								/* normaly we don't need merge */

	DEBPR0("ml_save_map\n")

	if (!mapblock) 
		return 1; /* fatal */

	/* check if we at the end of the block */
	if (mapblock->ppo==gl_pageperblock)
	{
		if (mapblock->shadowidxcou<MAX_MAPBLOCK_SHADOW) /* check if we still have shadow block */
		{
			if (alloc_mapblock(mapblock))				/* allocate map block*/
			{
				return 1; /* fatal */
			}
		}
		else
		{
			domapmerge=1;	/* we need merge, because no more shadow block */
		}
	}

	/* check if we need merge */
	if (!domapmerge)
	{
		if (!store_mappage(mapblock)) /* simple store map page */
		{
			ST_MAPDIR *mapdir=&ml_mapdir[ml_frag.current]; /* get current frag mapdir */

			mapdir->pba=mapblock->pba;		/* set fragment pba */
			mapdir->ppo=mapblock->ppo;		/* set fragment ppo */
			mapdir->index=1;				/* set fragment index to 1 */

			mapblock->last_pba=mapblock->pba;	/* store last succes written mapblock pba */
			mapblock->last_ppo=mapblock->ppo;	/* store last succes written mapblock ppo */

			mapblock->ppo++;				/* goto next page available */

			wear_releasedynamiclock();		/* release dynamic lock */
			ml_save_map_need=0;				/* reset save_map_need flag */
			return 0;						/* return with success*/
		}
	}

	/* merge map blocks and create forever loop because of any error */
	for (;;)
	{
		unsigned char cou;

		/* go through on shadow blocks */
		for (cou=0; cou<MAX_MAPBLOCK_SHADOW;cou++)
		{
			unsigned char midx=mapblock->shadowidx[cou];/* get index value of map block */
			if (midx!=INDEX_NA)
			{
				gl_freetable[midx] &= ml_ToLittleEndian32(~FT_MAP);			/* remove FT_MAP flag from FLT */
				wear_updatedynamicinfo(midx,WEAR_NA);	/* entry is still locked in FLT */
				mapblock->shadowidx[cou]=INDEX_NA;		/* remove index from map block*/
			}
		}
		mapblock->shadowidxcou=0;	/* reset shadow index counter */

		if (alloc_mapblock(mapblock)) /* allocate new map block*/
		{
			return 1; /* fatal */
		}

		if (!store_mappage(mapblock)) /* 1st page is mapblock */
		{
			t_ba last_pba=mapblock->pba; /* temporarly store last succes written mapblock pba */
			t_po last_ppo=mapblock->ppo; /* temporarly store last succes written mapblock ppo */
			mapblock->ppo++;			 /* 1st page is the FLT */

			if (!collect_frags(mapblock))	/* collecting fragments */
			{								/* if success */
				mapblock->last_pba=last_pba;/* store last succes written mapblock pba */
				mapblock->last_ppo=last_ppo;/* store last succes written mapblock ppo */
				wear_releasedynamiclock();	/* release dynamic lock */
				ml_save_map_need=0;			/* reset save_map_need flag */
				return 0;					/* return with success*/
			}
		}
	}
}

/****************************************************************************
 *
 * ml_init
 *
 * Initialize this layer, this function must be called at the begining
 *
 * RETURNS
 *
 * 0 - if ok
 * other if any error
 *
 ***************************************************************************/

t_bit ml_init()
{
	unsigned long wear_sum=0;	/* summary of wears */
	unsigned long wear_count=0;	/* counting number of wears  */
	unsigned long wear_average=0;
	unsigned char idx;
	unsigned char cou;
	t_ba pba,lba;
	t_po po,lastpo;
	ST_SPARE *sptr;
	ST_LOG *log;

	DEBPR0("\n\nml_init\n");

	if (ml_lowinit())
	{
		printf("ml_init: ml_lowinit() Failed.\n");
		return 1;
	}

	sptr=GET_SPARE_AREA(ml_buffer);

/* search last map blk */
	for (pba=(t_ba)gl_reservedblocks; pba<gl_numofblocks; pba++)
	{
		unsigned char block_type;

		if (ll_readonebyte(pba,0,(unsigned char)((long)(&sptr->u.map.block_type)-(long)(sptr)),&block_type))
		{
			continue; /* pre selecting blocks */
		}

		block_type&=BLK_TYPE_MASK;
		if (block_type==BLK_TYPE_INV || (!block_type)) 
            continue; /* skip 2 bit errors, no map block */

		if (!ll_readpart(pba,0,(unsigned char*)sptr,LL_RP_SPARE))
		{
			if (ml_ToBigEndian32(sptr->wear) < WEAR_NA)
			{
				wear_count++;			/* increase counts */
				wear_sum += ml_ToBigEndian32(sptr->wear);	/* add to summary */
			}

			block_type=sptr->u.map.block_type;
			block_type &= BLK_TYPE_MAP_MASK; /* remove BLK_MAPPAGE_FLAG */

			if (block_type >= BLK_TYPE_MAP_ORI && block_type<(BLK_TYPE_MAP_ORI+MAX_NUM_OF_DIF_MAPBLOCK)) 
			{
				check_reference_count
					(
						(unsigned char)(block_type-BLK_TYPE_MAP_ORI),
						ml_ToBigEndian32(sptr->u.map.ref_count),
						pba
					);
			}
		}
	}

	ml_stepback=0; /* reset stepback */

	for (cou=0; cou<MAX_NUM_OF_DIF_MAPBLOCK; cou++)
	{    
		ST_MAPBLOCK *map=&ml_mapblocks[cou];
		if (map->pba==BLK_NA) 
			return 1; /* fatal, no map blk found */

/* search last erased only page */
		lastpo=gl_pageperblock;
		for (po=0;po<gl_pageperblock;po++)
		{
			int ret = ll_read(map->pba,(unsigned char)(gl_pageperblock-po-1),ml_buffer);
			if (ret==LL_ERASED) 
				lastpo=(unsigned char)(gl_pageperblock-po-1);
			else 
				break;
		}

/* search last po in block */
		map->ppo=INDEX_NA;
		for (po=0;po<lastpo;po++)
		{            
			int ret = ll_read(map->pba,po,ml_buffer);
			if (ret==LL_OK)
			{
				if (sptr->u.map.block_type & BLK_MAPPAGE_FLAG)
				{
					map->ppo=po;
				}
			}
		}

		if (map->ppo==INDEX_NA)
		{
			DEBPR0("ERROR: map->ppo==INDEX_NA\n");
			return 1; /* fatal, no map page in blk found */
		}

/* read it */
		if (ll_readpart(map->pba,map->ppo,ml_u.basebuff,LL_RP_2NDHALF))
			return 1; /* fatal */

		map->last_pba=map->pba; /* update last information */
		map->last_ppo=map->ppo;

		map->ppo=lastpo;		/* setup next useable page */

		if (ml_buildmap(map))
		{
			DEBPR1("ERROR: ml_buildmap at map: %d\n",cou);
			return 1; /* fatal */
		}
	}


	{
/* finding last fltable */
		ST_MAPBLOCK *mapcur=ml_mapblocks;
		unsigned long mappagecou_lo=mapcur->mappagecou_lo;
		unsigned long mappagecou_hi=mapcur->mappagecou_hi;
		ST_MAPBLOCK *map=mapcur;

		for (cou=1,mapcur++; cou<MAX_NUM_OF_DIF_MAPBLOCK;cou++,mapcur++)
		{
			if (mapcur->mappagecou_hi>mappagecou_hi)
			{
				mappagecou_lo=mapcur->mappagecou_lo;
				mappagecou_hi=mapcur->mappagecou_hi;
				map=mapcur;
			}
			else if (mapcur->mappagecou_hi==mappagecou_hi && mapcur->mappagecou_lo>mappagecou_lo)
			{
				mappagecou_lo=mapcur->mappagecou_lo;
				mappagecou_hi=mapcur->mappagecou_hi;
				map=mapcur;
			}
		}

		if (ll_readpart(map->last_pba,map->last_ppo,ml_u.basebuff,LL_RP_2NDHALF)) return 1; /* fatal */
		/* read the last freelog table */

		ml_mapinfo->mappagecou_lo = ml_ToLittleEndian32(ml_max_mappagecou_lo);
		ml_mapinfo->mappagecou_hi = ml_ToLittleEndian32(ml_max_mappagecou_hi); /* write back the maximum found */
	}

/* building log blocks */
	ml_logblocknum=0;
	log=ml_log;
	for (idx=0; idx<gl_free_block; idx++)
	{
		if ((ml_ToBigEndian32(gl_freetable[idx])&(FT_MAP|FT_LOG))==FT_LOG)
		{
			pba=(t_ba)(ml_ToBigEndian32(gl_freetable[idx]) & FT_ADDRMASK);

			log->lastppo=gl_pageperblock;
			log->pba=pba;
			log->switchable=1;
			log->index=idx;

			lastpo=gl_pageperblock;
			for (po=0;po<gl_pageperblock;po++)
			{ /* searching the last page from top */
				int ret = ll_read(pba,(unsigned char)(gl_pageperblock-po-1),ml_buffer);
				if (ret==LL_ERASED)
					lastpo=(unsigned char)(gl_pageperblock-po-1);
				else 
					break;
			}

			log->lastppo=lastpo;
			for (po=0; po<lastpo; po++)
			{	/* building log blk */
				int ret = ll_read(pba,po,ml_buffer);
				if (ret==LL_OK) 
				{
#if !defined(LARGEBLOCK_8KB) && !defined(LARGEBLOCK_4KB) && !defined(LARGEBLOCK_2KB) && defined(NF_HW_ECC) // Small block && SW-ECC
                    log->ppo[ml_ToBigEndian16(sptr->u.log.lpo)] = po;
#else
					log->ppo[sptr->u.log.lpo]=po;
#endif
					log->lba = ml_ToBigEndian32(sptr->u.log.lba);
#if !defined(LARGEBLOCK_8KB) && !defined(LARGEBLOCK_4KB) && !defined(LARGEBLOCK_2KB) && defined(NF_HW_ECC) // Small block && SW-ECC
                    if (po != ml_ToBigEndian16(sptr->u.log.lpo))
#else
					if (po!=sptr->u.log.lpo) 
#endif
						log->switchable=0;
					if (!po) 
						log->wear=ml_ToBigEndian32(sptr->wear);
				}
				else
				{
					log->switchable=0; /* if error or hole in the log block */
				}
			}

			if (log->lastppo!=0)
			{
				log++;
				ml_logblocknum++;
			}
			else
			{
				gl_freetable[idx] &= ml_ToLittleEndian32(~FT_LOG); /* nothing was written, so remove log block flag */
				log->lastppo=INDEX_NA;		/* reset data in not used log blk */
				log->pba=BLK_NA;
			}
		}
	}

	if (wear_count) wear_average=wear_sum/wear_count; /* calculate current average for dynamic */

#if (!FTL_DELETE_CONTENT)
/* fill dynamic wear info table */
	for (idx=0; idx<gl_free_block; idx++)
	{
		if ((ml_ToBigEndian32(gl_freetable[idx]) & FT_BAD) != FT_BAD)
		{
		 	pba=(t_ba)(ml_ToBigEndian32(gl_freetable[idx])&FT_ADDRMASK);
			if (ll_read(pba,0,ml_buffer)==LL_OK)
			{
				wear_updatedynamicinfo(idx,ml_ToBigEndian32(sptr->wear));
			}
			else
			{
				wear_updatedynamicinfo(idx,wear_average);
			}
		}
	}
#endif
	wear_releasedynamiclock();

/* counting blocks */
	for(lba=0; lba<gl_numofblocks; lba++)
	{
		pba=ml_get_log2phy(lba);
        if (pba==BLK_NA) 
			break;
        ml_data_blk_count++;
	}

/* check if step back is requested */
	if (ml_stepback)
	{
		ml_stepback->ppo=gl_pageperblock;
		ml_stepback->shadowidxcou=MAX_MAPBLOCK_SHADOW; /* force map merge */

		if (ml_load_curr_frag(ml_stepback->start_frag)) 
			return 1;

		if (ml_save_map()) 
			return 1;
	}

	ml_state=ML_CLOSE;

	return 0;
}

/****************************************************************************
 *
 * ml_getmaxsector
 *
 * retreives the maximum number of sectors can be used in the system
 *
 * RETURNS
 *
 * maximum number of sectors
 *
 ***************************************************************************/

unsigned long ml_getmaxsector() 
{
	return ml_data_blk_count*gl_pageperblock;
}

/****************************************************************************
 *
 * ml_format
 *
 * Formatting the layer, this function must be called once at manufacturing
 * This is important that this function can be called only once!
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_format() 
{
	unsigned short fidx;
	unsigned char index;
	unsigned char cou;
	t_frag frag;
	t_ba pba;
	t_ba *ppba;
	ST_SPARE *sptr;
	
	if (ml_lowinit()) 
	{
		return 1;
	}

	sptr=GET_SPARE_AREA(ml_buffer);	/* here we can set up a local spare ptr <gl_pagesize is requested for it>*/

/* allocating free table */
	for (index=0,pba=(t_ba)gl_reservedblocks; index<gl_free_block && pba<gl_numofblocks; pba++)
	{

		if (ll_isbadblock(pba)) continue; /* if a block is bad it is left out from the system */

		if (ll_read(pba,0,ml_buffer)!=LL_ERASED)  /* if blk is not an erased blk */
		{
			if (ll_erase(pba)) 
			{
				
				continue; /* cannot be erased so leave it out */
		}
		}

		gl_freetable[index] = ml_ToLittleEndian32(pba);
		wear_updatedynamicinfo(index,0);
		index++;
	}

	if (index!=gl_free_block) 
		return 1; /* FATAL error */
	wear_releasedynamiclock();

/* allocate MAP block */
	for (cou=0; cou<MAX_NUM_OF_DIF_MAPBLOCK; cou++)
	{
		ST_MAPBLOCK *mapblock=&ml_mapblocks[cou]; /* get the given map block */

		mapblock->shadowidxcou=0;	/* reset index */
		mapblock->ref_count=0;		/* reset reference counter */

		if (alloc_mapblock(mapblock)) /* allocate 1st block in map block */
		{
			return 1; /* fatal */
		}
	}

/* reset mapinfo */
	(void)memset (ml_mapinfo,0xff,sizeof(ST_MAPINFO));

/* create MAP dir from remaining blocks */
	ppba=(t_ba *)ml_buffer; /* ml_buffer data area is for fragments, spare is for reading new phy block*/
	fidx=0;
	frag=0;
	for (;pba<gl_numofblocks;pba++)
	{
		unsigned char block_type;

		if (ll_isbadblock(pba)) 
			continue; /* if a block is bad it is left out from the system */

		if (ll_readonebyte(pba,0,(unsigned char)((long)(&sptr->u.map.block_type)-(long)(sptr)),&block_type))
		{
			continue; /* pre selecting blocks, leave out where there is any error (NO ECC!) */
		}

		block_type&=BLK_TYPE_MASK;
		if (!(block_type==BLK_TYPE_INV || (!block_type)))
		{ /* if block_type valid map block, then erase it */
			if (ll_erase(pba)) 
				continue; /* cannot be erased so leave it out */
		}

		ml_data_blk_count++; /* counting data blocks */

		ppba[fidx++] = ml_ToLittleEndian32(pba);

		/* if reach the end of fragpair then write it */
		if (fidx==MAX_FRAGSIZE*2)
		{
			if (write_frags(frag,(unsigned char*)ppba)) 
				return 1; /* fatal */
			fidx=0;
			frag+=2;
		}
	}

/* fill remaining frag entries with BLK_NA */
	while (frag!=(gl_max_frag_per_mapblock*MAX_NUM_OF_DIF_MAPBLOCK))
	{
		while (fidx!=MAX_FRAGSIZE*2)
		{
			ppba[fidx++]=BLK_NA;
		}

		if (write_frags(frag,(unsigned char*)ppba)) 
			return 1; /* fatal */
		fidx=0;
		frag+=2;
	}

/* reset counters */
	ml_mapinfo->mappagecou_hi=0;
	ml_mapinfo->mappagecou_lo=0;

/* store FLT to the top of MAP BLKS */
	for (cou=0; cou<MAX_NUM_OF_DIF_MAPBLOCK; cou++)
	{
		/* setting global mapblock */
		if (ml_load_curr_frag(ml_mapblocks[cou].start_frag))
		{
			return 1; /* FATAL */
		}

		if (ml_save_map()) /* Store information */
		{
			return 1;
		}
	}

	ml_state=ML_CLOSE;

	return 0;
}

/****************************************************************************
 *
 * ml_findlog
 *
 * Searching for a log block (check if logical block is logged)
 * it sets ml_curlog to a log block or set it to NULL
 *
 * INPUTS
 *
 * lba - logical block address
 *
 * RETURNS
 *
 * 1 - if found
 * 0 - not found
 *
 ***************************************************************************/

static t_bit ml_findlog(t_ba lba)
{
	ST_LOG *log=ml_log;		/* initialize log ptr */
	unsigned long a;

	for (a=0; a<gl_log_block; a++,log++) /* count on all log block */
	{
		if (log->lba==lba)	/* check if lba is equal */
		{
			ml_curlog=log;	/* we found the log block so set the current */
			return 1;		/* return found */
		}
	}

	ml_curlog=0;	/* reset log block */
	return 0;		/* return not found */
}

/****************************************************************************
 *
 * ml_dostatic
 *
 * does static wear leveling, its read logical block, updates static wear
 * info, and if static request is coming, then copy the block
 *
 ***************************************************************************/

static void ml_dostatic()
{
	unsigned long wear;
	t_ba pba;
	t_ba static_lba=wear_getstatic_lba();

	/* get pba of next static lba */
	for (;;)
	{
		static_lba++;
		if (static_lba>=ml_data_blk_count) static_lba=0;

		pba=ml_get_log2phy(static_lba);
		if (pba!=BLK_NA) break;

		static_lba=0;
		pba=ml_get_log2phy(static_lba);
		if (pba!=BLK_NA) break;
	}

	/* writes back new static lba */
	wear_setstatic_lba(static_lba);

	if (!ll_read(pba,0,ml_buffer))
	{   /* get original */
	 	wear = ml_ToBigEndian32(GET_SPARE_AREA(ml_buffer)->wear);
		if (wear<WEAR_NA) 
			wear_updatestaticinfo(static_lba,wear);
		else 
			wear_updatestaticinfo(static_lba,0);
	}
	else 
		wear_updatestaticinfo(static_lba,0);

	if (wear_check_static())
	{
		t_po po;
		t_ba d_pba,lba;
		t_ba s_pba;
		unsigned char index=gl_wear_allocstruct.free_index;

		if (index==INDEX_NA) 
			return;

		d_pba = ml_ToBigEndian32(gl_freetable[index]);

#if FTL_DELETE_CONTENT
		/* all block is erased in free table */
#else
		if (wll_erase(101,d_pba))
		{
			gl_freetable[index] |= ml_ToLittleEndian32(FT_BAD);    /* signal it and set as BAD */
			return;
		}
#endif

#if _HCC_MDEBUG_ENABLED_
		cnt_increase(CNT_TSTATIC);
#endif

		lba=gl_wear_allocstruct.static_lba;
		s_pba=ml_get_log2phy(lba);
		if (s_pba==BLK_NA) return; /* nothing to do */

		for (po=0; po<gl_pageperblock; po++)
		{
			int       ret  = ll_read(s_pba,po,ml_buffer);
			ST_SPARE* sptr = GET_SPARE_AREA(ml_buffer);
			if (!po)
			{
				sptr->wear             = ml_ToLittleEndian32(gl_wear_allocstruct.free_wear);
				sptr->u.map.block_type = BLK_TYPE_DAT;
			}
			else 
			{
				if (ret) continue;
			}

			if (wll_write(10,d_pba,po,ml_buffer,(unsigned char*)sptr)) return; /* we can stop static swap */
		}

		(void)ml_set_log2phy(lba,d_pba);
		wear_updatestaticinfo(lba,gl_wear_allocstruct.free_wear);

		gl_freetable[index] = ml_ToLittleEndian32(s_pba);
		wear_updatedynamicinfo(index,gl_wear_allocstruct.static_wear);

		(void)ml_save_map();
	}
}


/****************************************************************************
 *
 * ml_alloclog
 *
 * alloc log block and initiate it
 *
 * INPUTS
 *
 * log - pointer where to alloc to
 * lba - which logical block is connected to log block
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_alloclog(ST_LOG *log,t_ba lba) 
{
	unsigned char index=ml_alloc();
	t_ba pba;

	if (index==INDEX_NA) return 1;

	pba = ml_ToBigEndian32(gl_freetable[index]);

	log->index=index;
	log->lastppo=0;
	log->lba=lba;
	log->pba=pba;
	log->wear=gl_wear_allocstruct.free_wear;
	log->switchable=1;

	gl_freetable[index] = ml_ToLittleEndian32((t_ba)(pba | FT_LOG));
	wear_updatedynamicinfo(index,log->wear); /* locked now */
	ml_logblocknum++;

	(void)memset(log->ppo,INDEX_NA,sizeof(log->ppo));

	ml_save_map_need=1;

	return 0;
}

/****************************************************************************
 *
 * ml_open
 *
 * Opening the chanel for read or write from a specified sector
 *
 * INPUTS
 *
 * sector - start of sector for read or write
 * secnum - number of sector will be read or written
 * mode - ML_READ open for read, ML_WRITE open for write
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_open(unsigned long sector, unsigned long secnum, unsigned char mode)
{
	if (ml_state==ML_INIT) return 1;

	ml_state=ML_CLOSE;
    ml_seccou=secnum;

	ml_lba=(t_ba)(sector/gl_pageperblock);
	ml_lpo=(t_po)(sector%gl_pageperblock);

	if (ml_lba>=ml_data_blk_count) return 1;
	if (ml_lpo>=gl_pageperblock) return 1;

	if (mode==ML_READ) ml_state=ML_PENDING_READ;
	else if (mode==ML_WRITE) ml_state=ML_PENDING_WRITE;
	else return 1;

	return 0;
}


/****************************************************************************
 *
 * ml_domerge
 *
 * does log block and logical block merge
 *
 * INPUTS
 *
 * log - log block pointer (it contains logical block number also)
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_domerge(ST_LOG *log) 
{
	unsigned long wear=WEAR_NA;
	unsigned char po,lpo,index;
	t_ba pba;
	t_ba orig_pba;

	ml_dostatic();

	ml_save_map_need=1;

	if (log->lastppo==gl_pageperblock && log->switchable)
	{ /*  check if switchable */

		orig_pba=ml_get_log2phy(log->lba);
		if (orig_pba==BLK_NA) return 1; /*  fatal */

		(void)ml_set_log2phy(log->lba,log->pba);		/* put log block to original */
		wear_updatestaticinfo(log->lba,log->wear);

		if (!ll_read(orig_pba,0,ml_buffer))
		{/* get original */
		 	wear = ml_ToBigEndian32(GET_SPARE_AREA(ml_buffer)->wear);
		}
		else wear=0;

		gl_freetable[log->index] = ml_ToLittleEndian32(orig_pba); /* release original */
		if (wear<WEAR_NA) 
			wear_updatedynamicinfo(log->index,wear);
		else 
			wear_updatedynamicinfo(log->index,0);

		ml_curlog->lba=BLK_NA;

		ml_logblocknum--;

		return 0;
	}
again:
	index=ml_alloc();
	if (index==INDEX_NA) return 1;
	pba = ml_ToBigEndian32(gl_freetable[index]);
	orig_pba=ml_get_log2phy(log->lba);
	if (orig_pba==BLK_NA) 
	{
		return 1; /* fatal */
	}

	for (po=0; po<gl_pageperblock; po++)
	{
		ST_SPARE *sptr=GET_SPARE_AREA(ml_buffer);

		lpo=log->ppo[po];
		if (lpo==INDEX_NA)
		{
/* 			if (po &&  */
			(void)ll_read(orig_pba,po,ml_buffer);
			/* ) continue; //get original */
			if (!po) 
				wear = ml_ToBigEndian32(GET_SPARE_AREA(ml_buffer)->wear);
		}
		else
		{
/* 			if (po &&  */
			(void)ll_read(log->pba,lpo,ml_buffer);
				/* ) continue;	 //get logged */
		}

		if (!po)
		{
			sptr->wear             = ml_ToLittleEndian32(gl_wear_allocstruct.free_wear);
			sptr->u.map.block_type = BLK_TYPE_DAT;

			if(sptr->u.log.lba == 0xFFFFFFFF)	
			{
				//printf("ml_DoMrg,pba=[%x],lba=%x!!\n",log->pba,log->lba);
				sptr->u.log.lba	= ml_ToLittleEndian32(log->lba);
			}
		}

		if (wll_write(20,pba,po,ml_buffer,(unsigned char*)sptr)) 
		{
			goto again;
		}
	}

	if (wear==WEAR_NA)
	{
		if (!ll_read(orig_pba,0,ml_buffer))
		{
			wear = ml_ToBigEndian32(GET_SPARE_AREA(ml_buffer)->wear);/* get original */
		}
		else
		{
			wear=0;
		}
	}

	gl_freetable[index] = ml_ToLittleEndian32(orig_pba); /* release original */
	if (wear<WEAR_NA) 
		wear_updatedynamicinfo(index,wear);
	else 
		wear_updatedynamicinfo(index,0);

	gl_freetable[log->index] &= ml_ToLittleEndian32(~FT_LOG);	 /* release old log block */
	wear_updatedynamicinfo(log->index,WEAR_NA);

	(void)ml_set_log2phy(log->lba,pba);		/* put new block to original */
	wear_updatestaticinfo(log->lba,gl_wear_allocstruct.free_wear);

	ml_curlog->lba=BLK_NA;

	ml_logblocknum--;

	return 0;
}

/****************************************************************************
 *
 * ml_releaselog
 *
 * releasing a log block for a new logical block
 *
 * INPUTS
 *
 * lba - logical block address for new log block
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_releaselog(t_ba lba) 
{
	/* first try to find empty log block (lba==BLK_NA) */
	if (ml_logblocknum < gl_log_block) 
	{
		if (ml_findlog(BLK_NA)) /* find a free */
		{
			return ml_alloclog(ml_curlog,lba);
		}
	}

	/* 2md try to find a switchable log block to release */
	{
		unsigned char logcou;
		ST_LOG *log=ml_log;

		for (logcou=0; logcou<gl_log_block; logcou++,log++)
		{
			/*  check if switchable */
			if (log->lastppo==gl_pageperblock && log->switchable)
			{
		ml_curlog=log;
		if (ml_domerge(log)) 
				{
			return 1;
				}

				return ml_alloclog(log,lba);
			}
		}
	}

	/* 3rd release next log block */
	{
		ST_LOG *log=&ml_log[ml_logmerge++];
		if (ml_logmerge>=gl_log_block) 			ml_logmerge=0;

		ml_curlog=log;
		if (ml_domerge(log)) 			return 1;
		return ml_alloclog(log,lba);
	}
}

/****************************************************************************
 *
 * ml_write
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

t_bit ml_write(unsigned char *datap) 
{
#if _HCC_MDEBUG_ENABLED_
	cnt_increase(CNT_TWRITES);
#endif

	if (ml_state==ML_PENDING_WRITE)
	{
		if (!ml_findlog(ml_lba))
		{
			if (ml_releaselog(ml_lba)) return 1;
		}
		ml_state=ML_WRITE;
	}

	if (ml_state==ML_WRITE)
	{
		ST_SPARE *sptr=GET_SPARE_AREA(ml_buffer);

		if (!ml_seccou--)
		{
			ml_state=ML_ABORT;
			return 1;
		}

		if (ml_lpo>=gl_pageperblock)
		{
			ml_lba++;
			if (ml_lba>=ml_data_blk_count)
			{
				ml_state=ML_CLOSE;
				return 1;
			}

			if (!ml_findlog(ml_lba))
			{
				if (ml_releaselog(ml_lba)) return 1;
			}
			ml_lpo=0;
		}

		if (ml_curlog->lastppo>=gl_pageperblock)
		{
			if (ml_domerge(ml_curlog)) return 1;
			if (ml_alloclog(ml_curlog,ml_lba)) return 1;		/* allocate a new log block */
		}

		if (!ml_curlog->lastppo)
		{
			sptr->wear             = ml_ToLittleEndian32(ml_curlog->wear);
			sptr->u.map.block_type = BLK_TYPE_DAT;
		}

		sptr->u.log.lba = ml_ToLittleEndian32(ml_lba);
#if !defined(LARGEBLOCK_8KB) && !defined(LARGEBLOCK_4KB) && !defined(LARGEBLOCK_2KB) && defined(NF_HW_ECC) // Small block && SW-ECC
        sptr->u.log.lpo = ml_ToLittleEndian16((unsigned short)ml_lpo);
#else
		sptr->u.log.lpo = ml_lpo;
#endif

		if (ml_lpo!=ml_curlog->lastppo) ml_curlog->switchable=0; /* not switchable from this point */

		ml_curlog->ppo[ml_lpo++]=ml_curlog->lastppo;

		/* check user data pointer alignment to 32 bit */
		if ( ((long)(datap)) & (sizeof(long)-1) )
		{
			(void)memcpy (ml_buffer,datap,(int)gl_pagesize);
			(void)wll_write(40,ml_curlog->pba,ml_curlog->lastppo++,ml_buffer,(unsigned char*)sptr);
		}
		else
		{
			/* datap is aligned to 32 bit */
			(void)wll_write(40,ml_curlog->pba,ml_curlog->lastppo++,datap,(unsigned char*)sptr);
		}

		if (ml_save_map_need)
		{
			if (ml_save_map()) return 1;
		}

		return 0;
	}

	return 1;
}

/****************************************************************************
 *
 * ml_read
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

t_bit ml_read(unsigned char *datap)
{
	if (ml_state==ML_PENDING_READ)
	{
		ml_pba=ml_get_log2phy(ml_lba);
		if (ml_pba==BLK_NA) return 1;

		(void)ml_findlog(ml_lba);
		ml_state=ML_READ;
	}

	if (ml_state==ML_READ)
	{
		int ret;

		if (!ml_seccou--)
		{
			ml_state=ML_ABORT;
			return 1;
		}

		if (ml_lpo>=gl_pageperblock)
		{
			ml_lba++;
			if (ml_lba>=ml_data_blk_count)
			{
				ml_state=ML_CLOSE;
				return 1;
			}
			ml_pba=ml_get_log2phy(ml_lba);
			if (ml_pba==BLK_NA) return 1;

			(void)ml_findlog(ml_lba);
			ml_lpo=0;
		}


		/* check whether user data pointer is aligned, if it is then read direct to there */
		if ( ((long)(datap)) & (sizeof(long)-1) )
		{
			/* read temporaly then copy to user data buffer */

			if (ml_curlog)
			{
				t_po po=ml_curlog->ppo[ml_lpo];
				if (po==INDEX_NA) 
					ret=ll_read(ml_pba,ml_lpo,ml_buffer);
				else 
					ret=ll_read(ml_curlog->pba,po,ml_buffer);
			}
			else
			{
				ret=ll_read(ml_pba,ml_lpo,ml_buffer);
			}

			(void)memcpy (datap,ml_buffer,(int)gl_pagesize);
		}
		else
		{
			/* datap is aligned to 32 bit */
			/* using ll_readpart LL_RP_DATA because user buffer has data size length buffer */

			if (ml_curlog)
			{
				t_po po=ml_curlog->ppo[ml_lpo];
				if (po==INDEX_NA) 
                    ret=ll_readpart(ml_pba,ml_lpo,datap,LL_RP_DATA);
				else
                    ret=ll_readpart(ml_curlog->pba,po,datap,LL_RP_DATA);
			}
			else
			{
				ret=ll_readpart(ml_pba,ml_lpo,datap,LL_RP_DATA);
			}
		}

		ml_lpo++;

		if (ret==LL_OK) return 0;
		else if (ret==LL_ERASED) return 0; /* erased */

		return 1;
	}

	return 1;
}

/****************************************************************************
 *
 * ml_close
 *
 * closing sector reading or writing
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_close() 
{
	t_bit ret=0;

	if (ml_state==ML_INIT) return 1;

	if (ml_seccou) ret=1;
	if (ml_state==ML_ABORT) ret=1;

	ml_state=ML_CLOSE;
	return ret;
}

/****************************************************************************
 *
 * ml_flushlogblock
 *
 * flush all log block, this is used for deleting content function
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

#if FTL_DELETE_CONTENT
t_bit ml_flushlogblock(void)
{
	unsigned char logcou;
	ST_LOG *log=ml_log;

	for (logcou=0; logcou<gl_log_block; logcou++,log++)
	{
		if (log->lba!=BLK_NA)
		{
			ml_curlog=log;
			if (ml_domerge(log)) 
			{
				return 1;
			}

			if (ml_save_map())
			{
				return 1;
			}
		}
	}

	return 0;
}
#endif

/****************************************************************************
 *
 * ml_ToBigEndian32
 *
 * Tranform 32 bits little-endian to big-endian
 *
 * RETURNS
 *
 * A 32 bits big-endian value
 *
 ***************************************************************************/

unsigned long
ml_ToBigEndian32(
	unsigned long value)
{
#if _ENDIAN_TRANSFORM
	unsigned short hi_part, lo_part;
	hi_part = value >> 16;
	lo_part = value & 0x0000FFFF;
	return ((ml_ToBigEndian16(lo_part) << 16) | ml_ToBigEndian16(hi_part));
#else
    return value;
#endif
}

/****************************************************************************
 *
 * ml_ToBigEndian16
 *
 * Tranform 16 bits little-endian to big-endian
 *
 * RETURNS
 *
 * A 16 bits big-endian value
 *
 ***************************************************************************/

unsigned short
ml_ToBigEndian16(
	unsigned short value)
{
#if _ENDIAN_TRANSFORM
	unsigned char hi_part, lo_part;
	hi_part = value >> 8;
	lo_part = value & 0x00FF;
	// Swap hi_part and lo_part
	return ((lo_part << 8) | hi_part);
#else
    return value;
#endif
}

/****************************************************************************
 *
 * ml_ToLittleEndian32
 *
 * Tranform 32 bits big-endian to little-endian
 *
 * RETURNS
 *
 * A 32 bits little-endian value
 *
 ***************************************************************************/

unsigned long
ml_ToLittleEndian32(
	unsigned long value)
{
#if _ENDIAN_TRANSFORM
	unsigned short hi_part, lo_part;
	hi_part = value >> 16;
	lo_part = value & 0x0000FFFF;
	return ((ml_ToLittleEndian16(lo_part) << 16) | ml_ToLittleEndian16(hi_part));
#else
    return value;
#endif
}

/****************************************************************************
 *
 * ml_ToLittleEndian16
 *
 * Tranform 16 bits big-endian to little-endian
 *
 * RETURNS
 *
 * A 16 bits little-endian value
 *
 ***************************************************************************/

unsigned short
ml_ToLittleEndian16(
	unsigned short value)
{
#if _ENDIAN_TRANSFORM
	unsigned char hi_part, lo_part;
	hi_part = value >> 8;
	lo_part = value & 0x00FF;
	// Swap hi_part and lo_part
	return ((lo_part << 8) | hi_part);
#else
    return value;
#endif
}

/****************************************************************************
 *
 * End of mlayer.c
 *
 ***************************************************************************/

