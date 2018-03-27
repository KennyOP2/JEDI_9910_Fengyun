#ifndef _WEAR_H_
#define _WEAR_H_

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
 * Opening bracket for C++ compatibility
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Static wear counter definitions
 *
 ***************************************************************************/

#define WEAR_STATIC_LIMIT  1024       /* minimum limit in static wear checking */
#define WEAR_STATIC_COUNT  1023       /* number of allocation when to check static */
#define MAXSTATICWEAR 8				  /* maximum deep of static wear leveling */

/****************************************************************************
 *
 * definition for non existed wear level information
 *
 ***************************************************************************/

#define WEAR_NA        0xfffffffeUL  /* if entry is not available */

/****************************************************************************
 *
 * structure for allocation
 *
 ***************************************************************************/

typedef struct 
{
	unsigned char free_index;        /* zero based index in freeblock table */
	unsigned long free_wear;         /* free block wear info */
	t_ba static_lba;				 /* logical block address */
	unsigned long static_wear;       /* static block wear info */
} WEAR_ALLOCSTRUCT;

extern WEAR_ALLOCSTRUCT gl_wear_allocstruct;

/****************************************************************************
 *
 * static wear leveling structure and variable
 *
 ***************************************************************************/

typedef struct 
{
	t_ba lba;
	unsigned long wear;
} STATIC_WEAR_INFO;

typedef struct 
{
	unsigned long cnt; /* counter for static wearing */
	t_ba lba;		   /* current lba for static wear */
	STATIC_WEAR_INFO wear_info[MAXSTATICWEAR];
	unsigned char dynamic_index; /* dynamic wear alloc index */

#if FTL_DELETE_CONTENT
	/* when FTL_DELETE_CONTENT is activated then dynamic wear info needs to be saved */
	unsigned long dynamic_wear_info[MAX_FREE_BLOCK_AVAILABLE];
#endif

} ST_STATIC;

/****************************************************************************
 *
 * wear functions
 *
 ***************************************************************************/

extern void wear_init(ST_STATIC *ptr);
extern t_bit wear_check_static(void);
extern void wear_alloc(void);
extern void wear_updatedynamicinfo(unsigned char index, unsigned long wear);
extern void wear_releasedynamiclock(void);
extern void wear_updatestaticinfo(t_ba lba, unsigned long wear);

extern t_ba wear_getstatic_lba(void);
extern void wear_setstatic_lba(t_ba lba);

/****************************************************************************
 *
 * Closing bracket for C++
 *
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of wear.c
 *
 ***************************************************************************/

#endif	/* _WEAR_H_ */
