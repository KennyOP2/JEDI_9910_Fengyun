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
 * Includes
 *
 ***************************************************************************/

#include "mlayer.h"
#include "wear.h"

#if _HCC_MDEBUG_ENABLED_
#include "../../src_int/prgmacro.h"
#endif

/****************************************************************************
 *
 * global variable for wear_alloc function
 *
 ***************************************************************************/

WEAR_ALLOCSTRUCT gl_wear_allocstruct; /* this structure is filled when wear_alloc returns */

/****************************************************************************
 *
 * static variables
 *
 ***************************************************************************/

static ST_STATIC *gl_static;

#if FTL_DELETE_CONTENT
static unsigned long *gl_dynamic_wear_info; /* dynamic wear info points into saved area */
#else
static unsigned long gl_dynamic_wear_info[MAX_FREE_BLOCK_AVAILABLE]; /* dynamic wear info is here */
#endif

static unsigned char gl_dynamic_lock[(MAX_FREE_BLOCK_AVAILABLE+7) >> 3];/* bitfield for locking mechanism */

static const unsigned char gl_lookup_mask[8]= /* optimalized table for fast masking */
	{0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/****************************************************************************
 *
 * wear_init
 *
 * initiate wear leveling, this function must be called at power on
 *
 ***************************************************************************/

void wear_init(ST_STATIC *ptr) 
{
	unsigned char a;

	gl_static=ptr;

#if FTL_DELETE_CONTENT
	gl_dynamic_wear_info=gl_static->dynamic_wear_info;
#else
	for (a=0; a<gl_free_block; a++) 
	{
		gl_dynamic_wear_info[a]=WEAR_NA;
	}
#endif

	for (a=0; a<sizeof (gl_dynamic_lock); a++) 
	{
		gl_dynamic_lock[a]=0xff; /* set all bits to 1 means no free block available */
	}

	for (a=0; a<MAXSTATICWEAR; a++) 
	{
		gl_static->wear_info[a].lba=BLK_NA;
        gl_static->wear_info[a].wear=ml_ToLittleEndian32(WEAR_NA);
	}

	gl_static->lba=0; /* reset counter */
    gl_static->cnt=ml_ToLittleEndian32(2); /* wait cycle after power on */
	gl_static->dynamic_index=0; /* reset dynamic index */
}

/****************************************************************************
 *
 * wear_getstatic_lba
 *
 * retreive the current lba of the static wear
 *
 * RETURNS
 *
 * lba of static block
 *
 ***************************************************************************/

t_ba wear_getstatic_lba() 
{
	return ml_ToBigEndian32(gl_static->lba);
}

/****************************************************************************
 *
 * wear_setstatic_lba
 *
 * update the current lba of the static wear
 *
 * INPUTS
 *
 * lba - lba of static block
 *
 ***************************************************************************/

void wear_setstatic_lba(t_ba lba) 
{
	gl_static->lba = ml_ToLittleEndian32(lba);
}

/****************************************************************************
 *
 * dynamic_peak
 *
 * searching in dynamic area if there is any wear which is bigger than the
 * average
 *
 * RETURNS
 *
 * 0 - not found
 * other found a peek in dynamic area
 *
 ***************************************************************************/

static t_bit dynamic_peak() 
{
	unsigned long average=0;
	unsigned long avecou=0;
	unsigned char index;
	t_ba *free_table;

	free_table=gl_freetable;	       /* get free table */
	for (index=0; index<gl_free_block; index++,free_table++) 
	{
		if (!(ml_ToBigEndian32((*free_table))&FT_BAD)) 	  /* check if block is not used (free) */
		{
			if (!(gl_dynamic_lock[index>>3]&gl_lookup_mask[index&7]))  /* check if there is dynamic lock set */
			{
				unsigned long wear=gl_dynamic_wear_info[index];
				if (wear!=WEAR_NA) 
				{
					average+=wear;
					avecou++;
				}
			}
		}
	}

	if (!avecou) return 0;
	average/=avecou;

	free_table=gl_freetable;	       /* get free table */
	for (index=0; index<gl_free_block; index++, free_table++) 
	{
		if (!(ml_ToBigEndian32((*free_table))&FT_BAD)) 	  /* check if block is not used (free) */
		{
			if (!(gl_dynamic_lock[index>>3]&gl_lookup_mask[index&7]))  /* check if there is dynamic lock set */
			{
				unsigned long wear=gl_dynamic_wear_info[index];
				if (wear!=WEAR_NA) 
				{
					if (wear>average+((3*WEAR_STATIC_LIMIT)/4))  /* 75% of distance */
					{
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

/****************************************************************************
 *
 * wear_check_static
 *
 * checking if static wear is needed
 *
 * RETURN
 *
 * 0 - if not needed
 * 1 - if static wear necessary
 *
 ***************************************************************************/

t_bit wear_check_static() 
{
	unsigned char a;
	t_ba static_lba=BLK_NA;
	unsigned long static_wear=WEAR_NA;

	if ((!ml_ToBigEndian32(gl_static->cnt)) 
        || dynamic_peak()) 	/* search the lowest static */
	{
		for (a=0; a<MAXSTATICWEAR; a++) 
		{
			unsigned long wear = ml_ToBigEndian32(gl_static->wear_info[a].wear);
			t_ba     lba       = ml_ToBigEndian32(gl_static->wear_info[a].lba);

			if ((wear!=WEAR_NA) && (lba!=BLK_NA)) 
			{
				if (static_lba!=BLK_NA) 
				{
					if (wear>=static_wear) continue;	/* if its bigger continue */
				}

				static_lba=lba;
				static_wear=wear;
			}
		}
	}

    if (static_lba!=BLK_NA)
	{
		unsigned char *lookup_mask=(unsigned char *)gl_lookup_mask;
		unsigned char *dynamic_lock=gl_dynamic_lock;
		t_ba *free_table=gl_freetable;
		unsigned char find_index=INDEX_NA;
		unsigned long find_wear=WEAR_NA;

		for (a=0; a<gl_free_block; a++)
		{   /* search for the highest dynamic */
			if (!( (*dynamic_lock) & (*lookup_mask) ) )
			{ /* check if locked */
				if (!(ml_ToBigEndian32((*free_table)) & FT_BAD))
				{	/* if it is realy free */
					unsigned long wear=gl_dynamic_wear_info[a];

					if (wear!=WEAR_NA)
					{
						if (find_index==INDEX_NA)
						{
							find_index=a;
							find_wear=wear;
						}
						else if (wear>find_wear)
						{	/* if its bigger */
							find_index=a;
							find_wear=wear;
						}
					}
				}

				free_table++;
				lookup_mask++;

				if ((a&7)==7)
				{
					lookup_mask=(unsigned char*)gl_lookup_mask; /* reset look up */
					dynamic_lock++;				/* goto next bitfield; */
				}
			}
		}

		if (find_index!=INDEX_NA)
		{
			if (find_wear>static_wear+WEAR_STATIC_LIMIT)
			{
#if _HCC_MDEBUG_ENABLED_
				if (ml_ToBigEndian32(gl_static->cnt))
					cnt_increase(CNT_TSTATICPEAK);
#endif
				gl_wear_allocstruct.free_index=find_index;
				gl_wear_allocstruct.free_wear=find_wear+1; /* incremented by 1! */

				gl_wear_allocstruct.static_lba=static_lba;
				gl_wear_allocstruct.static_wear=static_wear;

				gl_static->cnt = ml_ToLittleEndian32(WEAR_STATIC_COUNT);
				return 1;	 /* static wear successfully */
			}
		}
	}
	else 
	{
		gl_static->cnt = ml_ToLittleEndian32(ml_ToBigEndian32(gl_static->cnt) - 1);
		//gl_static->cnt--;
	}

	gl_wear_allocstruct.free_index=INDEX_NA;

	gl_wear_allocstruct.static_lba=BLK_NA;
	gl_wear_allocstruct.static_wear=WEAR_NA;

	return 0;  /* no static wear needed */
}

/****************************************************************************
 *
 * wear_alloc
 *
 * allocates a block from free table list according to theirs wear level info
 * datas will be unchanged in all table (calling twice it returns with same
 * values) if static_lba is not equal with BLK_NA then static wear heandler
 * has to be called immediately. gl_wear_allocstruct holds all the
 * information
 *
 ***************************************************************************/

void wear_alloc()
{
	unsigned char index=gl_static->dynamic_index;  /* get last counter value */
	unsigned char num=(unsigned char)gl_free_block;/* get number of maximum cycles */
	t_ba *free_table=&gl_freetable[index];	       /* get current free table entry */

	while (num--)
	{	/* cycle for all entries */
		free_table++;					  /* goto next entry */
	    index++;						  /* goto next index */
		if (index>=gl_free_block)
		{
			index=0;					  /* counter can be overflow, independently */
			free_table=gl_freetable;	  /* get start of free table */
		}

		if (!(ml_ToBigEndian32((*free_table))&FT_BAD))
		{	  /* check if block is not used (free) */
			if (!(gl_dynamic_lock[index>>3]&gl_lookup_mask[index&7]))
			{ /* check if there is dynamic lock set */
				gl_static->dynamic_index=index;			/* restore index */
				gl_wear_allocstruct.free_index=index; /* store index into the alloc struct */
				gl_wear_allocstruct.free_wear=gl_dynamic_wear_info[index]+1; /* store increased value of wear */
				return;	 /* allocation is success */
			}
		}
	}

	gl_static->dynamic_index=index;					/* restore index */
 	gl_wear_allocstruct.free_index=INDEX_NA;
 	gl_wear_allocstruct.free_wear=WEAR_NA;		   /* fatal error! no free block in the table */
}

/****************************************************************************
 *
 * wear_updatedynamicinfo
 *
 * updating dynamic wear level info of an indexed block (free block)
 *
 * INPUTS
 *
 * index - index of the block in free table entry
 * wear - wear info of given block
 *
 ***************************************************************************/

void wear_updatedynamicinfo(unsigned char index, unsigned long wear) 
{
   if (index<gl_free_block) 
   {
		if (wear<WEAR_NA)  /* keep original data inside <e.g. for log,map blocks> */
		{
			gl_dynamic_wear_info[index]=wear;
		}

		gl_dynamic_lock[index>>3]|=gl_lookup_mask[index&7]; /* optimalized! */
	}
}


/****************************************************************************
 *
 * wear_releasedynamiclock
 *
 * Releasing newly added free blocks (enabled for allocation)
 * Must be called when a MAP dir or entry is stored
 *
 ***************************************************************************/

void wear_releasedynamiclock() 
{
	unsigned char index;

#if FTL_DELETE_CONTENT
	unsigned char maxindex=(unsigned char)gl_free_block;

	for (index=0; index<maxindex; index++) 
	{
		if (gl_dynamic_lock[index>>3] & gl_lookup_mask[index&7])
		{
			ml_preeraseblk(index);
		}
	}
#endif

	for (index=0; index<sizeof (gl_dynamic_lock); index++) 
	{
		/* set all bits to 0 */
		gl_dynamic_lock[index]=0; 
	}
}

/****************************************************************************
 *
 * wear_updatestaticinfo
 *
 * updating static wear info of a static block (used block)
 *
 * INPUTS
 *
 * lba - logical block number
 * wear - wear info of the given block
 *
 ***************************************************************************/

void wear_updatestaticinfo(t_ba lba, unsigned long wear) 
{
	unsigned long max_wear=WEAR_NA;
	unsigned char a,max_index=INDEX_NA;

	if (lba==BLK_NA) return;

	for (a=0; a<MAXSTATICWEAR; a++)    /* check if its already existed */
	{
		if (ml_ToBigEndian32(gl_static->wear_info[a].lba) == lba) 
		{
			gl_static->wear_info[a].wear = ml_ToLittleEndian32(wear);
			return;
		}
	}

	for (a=0; a<MAXSTATICWEAR; a++)    /* search maximum or empty entry */
	{
		unsigned long twear = ml_ToBigEndian32(gl_static->wear_info[a].wear);
		if (twear!=WEAR_NA) 
		{
			if (max_index!=INDEX_NA) 
			{
				if (twear<max_wear)
					continue;
			}

			max_wear=twear;
			max_index=a;
		}
		else  /* if entry is empty put immediatelly */
		{
			gl_static->wear_info[a].wear = ml_ToLittleEndian32(wear);
			gl_static->wear_info[a].lba  = ml_ToLittleEndian32(lba);
			return;
		}
	}

	if (max_index!=INDEX_NA) 
	{
		if (wear<max_wear) 
		{
			gl_static->wear_info[max_index].wear = ml_ToLittleEndian32(wear);
			gl_static->wear_info[max_index].lba  = ml_ToLittleEndian32(lba);
		}
	}
}

/****************************************************************************
 *
 * end of wear.c
 *
 ***************************************************************************/

