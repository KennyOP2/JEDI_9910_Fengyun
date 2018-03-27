#ifndef _FTL_DEFS_H_
#define _FTL_DEFS_H_

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

/******************************************************************************
 *
 * Opening bracket for C++ compatibility
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 *
 *  Remove comment if FAT THIN or FAT STHIN is used
 *
 *****************************************************************************/

/* #define USE_FATTHIN */

/******************************************************************************
 *
 *  set USE_ONLY512 to 1 if only 512 bytes page driver is used in MLayer
 *  set USE_ONLY2048 to 1 if only 2048 bytes page driver is used in MLayer
 *  set USE_ONLY4096 to 1 if only 4096 bytes page driver is used in MLayer
 *  set USE_ONLY8192 to 1 if only 8192 bytes page driver is used in MLayer
 *  Only one of above can be set to 1 or none of them
 *  if no USE_ONLYxxx is used then check MAX_DATA_SIZE value below
 *
 *****************************************************************************/


#define USE_ONLY8192 0	//It need to be set as 512B/sector in device mode
#define USE_ONLY4096 0
#define USE_ONLY2048 0
#define USE_ONLY512 0



/******************************************************************************
 *
 *  if no USE_ONLYxxx is used then check MAX_DATA_SIZE value below
 *
 *****************************************************************************/
#ifdef LARGEBLOCK_8KB 
	#define MAX_DATA_SIZE   8192
#elif LARGEBLOCK_4KB 
	#define MAX_DATA_SIZE   4096	//for compatibility of linux OS
#elif LARGEBLOCK_2KB
	#define MAX_DATA_SIZE   2048	//for compatibility of linux OS
#else
	#define MAX_DATA_SIZE   512
#endif

//#elif USE_ONLY8192
//#define MAX_DATA_SIZE   8192
/* here can be set the maximum available DATA size if USE_ONLYxxx is not defined */
/******************************************************************************
 *
 * Set t_bit type according if bit type is available (e.g.MSC51)
 *
 *****************************************************************************/

#if 1

typedef unsigned char t_bit;

#else

typedef bit t_bit;

#endif

/******************************************************************************
 *
 * FTL low level variable definitions (see spec. for detail)
 *
 *****************************************************************************/

#define MAX_BLOCK_AVAILABLE				0x8000 //16384 /* maximum number of block is available */
#define MAX_PAGE_PER_BLOCK_AVAILABLE	128	  /* maximum number of pages available */

#define MAX_FREE_BLOCK_AVAILABLE		40
#define MAX_LOG_BLOCK_AVAILABLE			4

#define MAX_NUM_OF_DIF_MAPBLOCK			4	 /* number of different map from 1-16 */
#define MAX_MAPBLOCK_SHADOW				3	 /* number of shadow map block per different maps */

#define MAX_CACHEFRAG 1			/* number of cacheable fragments optimum is 4 <depending on RAM>, minimum is 1*/

#define MAX_RESERVEDBLOCKS	0	/* keep blocks reserved from the begining (from block 0) */

/******************************************************************************
 *
 *  Set _HCC_MDEBUG_ENABLED_ value to 1 if debug prints are enabled
 *  or set it to back to 0 <default> if no debug print requested
 *
 *****************************************************************************/

#define _HCC_MDEBUG_ENABLED_ 0

/******************************************************************************
 *
 * FTL_DELETE_CONTENT is for erasing block which are not used anymore
 * this #define defines also an interface for FAT/SAFEFAT filesystem
 * for supporting f_deletecontent function which removes also the data
 * from the file when file is deleted
 *
 * set FTL_DELETE_CONTENT to 1 if above needs to be supported
 * When FTL_DELETE_CONTENT is enabled and only 512 bytes page is used then
 * set MAX_FREE_BLOCK_AVAILABLE to 27 (it cannot be more)
 *
 *****************************************************************************/

#define FTL_DELETE_CONTENT 0

/******************************************************************************
 *
 * definitions for FTL DRIVER don't modify it
 *
 *****************************************************************************/

#if ((USE_ONLY512 + USE_ONLY2048 + USE_ONLY4096 + USE_ONLY8192) > 1)
#error "Only one USE_ONLYxxx can be set at once"
#endif

#if (USE_ONLY512 || USE_ONLY2048 || USE_ONLY4096 || USE_ONLY8192)

/* if special USE_ONLYxxx is specified then no hlayer is used */
/* but ftl_xxx used directly ml_xxx */

#ifndef _MLAYER_H_
#include "ftl/mlayer.h"
#endif

#define ftl_open ml_open
#define ftl_read ml_read
#define ftl_write ml_write
#define ftl_close ml_close
#define ftl_getmaxsector ml_getmaxsector

#define FTL_READ ML_READ
#define FTL_WRITE ML_WRITE

/* definition for high level function drivers */
#define ftl_init ml_init
#define ftl_format ml_format

#else

/* hlayer is used only if device setting is not only 512 bytes */
/* but conversation from any to 512 is needed */

#ifndef _HLAYER_H_
#include "hlayer.h"		
#endif

#define ftl_open hl_open
#define ftl_read hl_read
#define ftl_write hl_write
#define ftl_close hl_close
#define ftl_getmaxsector hl_getmaxsector

#define FTL_READ HL_READ
#define FTL_WRITE HL_WRITE

/* definition for high level function drivers */
#define ftl_init hl_init
#define ftl_format hl_format

#endif


/******************************************************************************
 *
 * Closing bracket for C++
 *
 *****************************************************************************/

#ifdef __cplusplus
}
#endif

/******************************************************************************
 *
 *  End of ftl_defs.h
 *
 *****************************************************************************/

#endif /* _FTL_DEFS_H_ */
