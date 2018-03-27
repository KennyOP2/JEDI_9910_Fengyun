#ifndef _MLAYER_H_
#define _MLAYER_H_

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

#ifndef _FTL_DEFS_H_
#include "../ftl_defs.h"
#endif

// Awin@20080730
//#include "nandflash/configs.h"
#define DLLAPI

#ifdef __cplusplus
extern "C" {
#endif

#define FTL_VERSION 146	/* Signaling version 1.46 */

#define MAX_SPARE_SIZE  sizeof(ST_SPARE)   /* this size of data is used from spare area only */

#if MAX_BLOCK_AVAILABLE>0x4000 /* if more than 16K block in the system */
#define BLK_NA         0xffffffffUL  /* if block is not available */
#else
#define BLK_NA         0xffff        /* if block is not available */
#endif
#define INDEX_NA       0xff          /* if index is not available */


#if MAX_BLOCK_AVAILABLE>0x4000 /* if more than 16K block in the system */
typedef unsigned long t_ba; /* typedef for block address */
typedef unsigned short t_frag;/* fragment type */
#define FRAG_NA 0xffff
#else
typedef unsigned short t_ba; /* typedef for block address */
typedef unsigned char t_frag;/* fragment type */
#define FRAG_NA 0xff
#endif
typedef unsigned char  t_po; /* typedef for page offset */

extern DLLAPI t_bit ml_init(void);
extern DLLAPI t_bit ml_format(void);

extern DLLAPI t_bit ml_open(unsigned long sector, unsigned long secnum, unsigned char mode);
extern DLLAPI t_bit ml_write(unsigned char *datap);
extern DLLAPI t_bit ml_read(unsigned char *datap);
extern DLLAPI t_bit ml_close(void);
extern DLLAPI unsigned long ml_getmaxsector(void);
unsigned long  ml_ToBigEndian32  (unsigned long  value);
unsigned short ml_ToBigEndian16  (unsigned short value);
unsigned long  ml_ToLittleEndian32(unsigned long  value);
unsigned short ml_ToLittleEndian16(unsigned short value);

#if FTL_DELETE_CONTENT
extern t_bit ml_flushlogblock(void);
#endif

extern unsigned long gl_numofblocks;
extern unsigned char gl_pageperblock;
extern unsigned long gl_pagesize;
extern unsigned long gl_free_block;
extern unsigned long gl_log_block;
extern unsigned long gl_reservedblocks;

extern t_frag gl_max_frag_per_mapblock;

#if MAX_FREE_BLOCK_AVAILABLE>254
#error MAX_FREE_BLOCK_AVAILABLE	cannot be more than 254
#endif

#if MAX_CACHEFRAG<1
#error MAX_CACHEFRAG cannot be less than 1
#endif

#if MAX_NUM_OF_DIF_MAPBLOCK<1 || MAX_NUM_OF_DIF_MAPBLOCK>16
#error MAX_NUM_OF_DIF_MAPBLOCK cannot be less than 1 or bigger than 16
#endif

#if MAX_FREE_BLOCK_AVAILABLE-(MAX_NUM_OF_DIF_MAPBLOCK*MAX_MAPBLOCK_SHADOW+1)-MAX_LOG_BLOCK_AVAILABLE < 8
#error MAX_FREE_BLOCK_AVAILABLE cannot hold this number of blocks
#endif

#if MAX_FREE_BLOCK_AVAILABLE*2>MAX_DATA_SIZE/2
#error MAX_FREE_BLOCK_AVAILABLE is to large
#endif

extern t_ba *gl_freetable;

#if FTL_DELETE_CONTENT
extern void ml_preeraseblk(unsigned char index);
#endif

#if MAX_BLOCK_AVAILABLE>0x4000 /* if more than 16K block in the system */
#define	FT_ADDRMASK ((t_ba)(0x3FFFFFFFUL))
#define	FT_MAP      0x80000000UL
#define	FT_LOG      0x40000000UL
#define FT_BAD	    (FT_MAP | FT_LOG)
#else
#define	FT_ADDRMASK ((t_ba)0x3FFF)
#define	FT_MAP      0x8000
#define	FT_LOG      0x4000
#define FT_BAD	    (FT_MAP | FT_LOG)
#endif

typedef struct 
{
	unsigned long wear; /* spare area 32-bit Wear Leveling Counter */

	union 
	{
		unsigned char dummy[8];			/* 8 bytes allocated for any structure below */

		struct 
		{
			unsigned char block_type;	/* <must be the 1st byte!>  block type + BLK_MAPPAGE_FLAG flag */
#if !defined(LARGEBLOCK_2KB) && defined(NF_HW_ECC) // Small block && SW-ECC
            unsigned short lpo;
#else
			t_po lpo;					/* logged page number */
#endif
			t_ba lba;					/* logged block blongs to this lba */
		} log;

		struct 
		{
			unsigned char block_type;	/* <must be the 1st byte!>  block type + BLK_MAPPAGE_FLAG flag */
			t_frag frag;				/* fragment number if mappage */
			unsigned long ref_count;    /* 32bit MAP block reference counter */
		} map;

	} u; /* None of the unioned structure size can be bigger than 8 bytes!! */

    unsigned long ecc;			/* space for ECC for lower layer calculation */
#if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
	unsigned long dummy1;
	unsigned long dummy2;
#endif
} ST_SPARE;

#define MAX_PAGE_SIZE (MAX_DATA_SIZE + MAX_SPARE_SIZE)

#define MAX_FRAGSIZE ((gl_pagesize/2)/sizeof (t_ba))

#if USE_ONLY512
#define MAX_FRAGNUM_AVAILABLE  (MAX_BLOCK_AVAILABLE / ((MAX_DATA_SIZE/2)/sizeof (t_ba)) )
#elif USE_ONLY2048
#define MAX_FRAGNUM_AVAILABLE  (MAX_BLOCK_AVAILABLE / ((MAX_DATA_SIZE/2)/sizeof (t_ba)) )
#elif USE_ONLY4096
#define MAX_FRAGNUM_AVAILABLE  (MAX_BLOCK_AVAILABLE / ((MAX_DATA_SIZE/2)/sizeof (t_ba)) )
#elif USE_ONLY8192
#define MAX_FRAGNUM_AVAILABLE  (MAX_BLOCK_AVAILABLE / ((MAX_DATA_SIZE/2)/sizeof (t_ba)) )
#else
/* 512 pagesize is used here because both (512/2048/4096) config can be used (hlayer needed)*/
#define MAX_FRAGNUM_AVAILABLE  (MAX_BLOCK_AVAILABLE / (512/2/sizeof (t_ba)))
#endif

/* block types definitions */
#define	BLK_TYPE_DAT  0x0f

#define BLK_TYPE_MAP_ORI 0x30  /* this is original map block value, no ecc required */

#define BLK_TYPE_MASK 0xf0 /* used for masking block type */
#define BLK_TYPE_INV  0xf0 /* block type cannot be 0xf0, in this case 2 bit error */

#define BLK_TYPE_MAP_MASK 0xf7 /* used for masking map block type */

#define BLK_MAPPAGE_FLAG 0x08  /* if page contains FLT, a fragment, and other information */
							   /* its not set when page contains original fragment info only */

#define GET_SPARE_AREA(_buf_) ((ST_SPARE*)(((unsigned char*)(_buf_))+gl_pagesize))

typedef struct 
{
	t_ba pba;
	t_po ppo;
	unsigned char index;
} ST_MAPDIR;



typedef struct 
{
	t_ba pba;		/* current map block address; BLK_NA if we don't have one (fatal error) */
	t_po ppo;		/* current page offset in map */
	t_ba last_pba;
	t_po last_ppo;				/*last good written map situated here */
	unsigned long ref_count;	/* last written counter in MAP block */
	t_frag start_frag;          /* start fragment number in this MAP */
	t_frag end_frag;		    /* end fragment number in this MAP */
	unsigned char block_type;   /*type in the spare area of this block*/

	ST_MAPDIR *mapdir;		   /* start entry in mg_mapdir*/
	unsigned char shadowidx[MAX_MAPBLOCK_SHADOW];  /* shadow block indexes */
	unsigned char shadowidxcou;	   /* current number of MAP blocks */
/* only at start up */
	unsigned long mappagecou_hi;
	unsigned long mappagecou_lo; /* searrching for the latest correct */
} ST_MAPBLOCK;

typedef struct 
{
	t_ba last_pba;
	t_po last_ppo;
	unsigned long mappagecou_hi;
	unsigned long mappagecou_lo; /* current counter */
	unsigned char shadowidx[MAX_MAPBLOCK_SHADOW]; /* mapblock shadow index in FLT */
} ST_MAPINFO;

typedef struct 
{
	t_frag cached[MAX_CACHEFRAG];
	t_ba *ppbas[MAX_CACHEFRAG];
	t_frag current;
	unsigned char pos;
	t_ba *ppba;
} ST_FRAG;

enum 
{
	ML_CLOSE,
	ML_PENDING_READ,
	ML_PENDING_WRITE,
	ML_READ,
	ML_WRITE,
	ML_ABORT,
	ML_INIT
};

typedef struct 
{
	unsigned long wear;
	t_ba lba;		/* which logical is this */
	t_ba pba;		/* what its phisical */
	unsigned char ppo[MAX_PAGE_PER_BLOCK_AVAILABLE];
	unsigned char lastppo;
	unsigned char index;
	unsigned char switchable;
} ST_LOG;

#ifdef __cplusplus
}
#endif


/****************************************************************************
 *
 * end of mlayer.h
 *
 ***************************************************************************/

#endif	/* _MLAYER_H_ */
