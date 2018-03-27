#ifndef _UDEFS_F_H_
#define _UDEFS_F_H_

/****************************************************************************
 *
 *            Copyright (c) 2003-2007 by HCC Embedded
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

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * enable this if CAPI (Common API) is used
 *
 ***************************************************************************/
#define FN_CAPI_USED 0

/****************************************************************************
 *
 * OEM name
 *
 ***************************************************************************/
#define OEM_NAME "MSDOS5.0"
/*#define OEM_NAME "EFFSFAT"*/

/****************************************************************************
 *
 * CAPI selected includes
 *
 ***************************************************************************/

#if FN_CAPI_USED
#include "../../fw_port.h"
#else

/****************************************************************************
 *
 *	if Unicode is used then comment in HCC_UNICODE define
 *
 ***************************************************************************/
#define HCC_UNICODE

#ifndef HCC_UNICODE
#define F_LONGFILENAME 0 /*  0 - 8+3 names   1 - long file names   */
#define W_CHAR char
#else
#define F_LONGFILENAME 1 /* don't change it, because unicode version alvays uses long file name */
#define W_CHAR wchar
#endif

#ifdef HCC_UNICODE
#ifdef _WIN32
typedef unsigned short wchar;
#else
typedef unsigned int wchar;
#endif
#endif

/****************************************************************************
 *
 * volumes definitions
 *
 ***************************************************************************/

#define FN_MAXVOLUME    10  /* maximum number of volumes */
#define FN_MAXTASK      12  /* maximum number of task */
#define FN_MAXPATH      512 /* maximum allowed filename or pathname */
#define FN_CURRDRIVE     -1 /* setting the current drive at startup (-1 means no default current drive)*/

#define FN_MUTEX_TYPE unsigned long

/* select path separator */
#if 1
#define F_SEPARATORCHAR '/'
#else
#define F_SEPARATORCHAR '\\'
#endif

/****************************************************************************
 *
 * Last error usage
 *
 ***************************************************************************/

#if 0
/* simple asignment */
#define F_SETLASTERROR(ec) (fm->lasterror=(ec))
#define F_SETLASTERROR_NORET(ec) (fm->lasterror=(ec))
#elif 1
/* function calls used for it */
#define F_SETLASTERROR(ec) fn_setlasterror(fm,ec)
#define F_SETLASTERROR_NORET(ec) fn_setlasterror_noret(fm,ec)
#elif 0
/* no last error is used (save code space) */
#define F_SETLASTERROR(ec) (ec)
#define F_SETLASTERROR_NORET(ec)
#endif


/****************************************************************************
 *
 * Close bracket for non CAPI
 *
 ***************************************************************************/

#endif  /* FN_CAPI_USED */

/****************************************************************************
 *
 * Common defines (for non CAPI and CAPI)
 *
 ***************************************************************************/

#define F_MAXFILES  10      /* maximum number of files */

#define F_MAXSEEKPOS 256    /* number of division of fast seeking */

/****************************************************************************
 *
 * functions definitions
 *
 ***************************************************************************/

/* Use internal mem functions (memcpy,memset) or switch to library functions */
/*#define INTERNAL_MEMFN*/

/* Use malloc for cache items */
#define USE_MALLOC

#ifdef USE_MALLOC
#include "sys/sys.h"
#define __malloc(x) SYS_Malloc(x)		/* normally use malloc from library */
#define __free(x) SYS_Free(x)		    /* normally use free from library */
#endif

/* maximum supported sector size */
#define F_MAX_SECTOR_SIZE 512

/* Enable FAT caching */
#define FATCACHE_ENABLE
#if F_LONGFILENAME
#define DIRCACHE_ENABLE
#endif

/* define of allocation of faster searching mechanism */
#ifdef USE_MALLOC
//#define FATBITFIELD_ENABLE
#endif

#ifdef FATCACHE_ENABLE
#define FATCACHE_BLOCKS 1
#define FATCACHE_READAHEAD 128	/* max. 256 */
#endif

#if F_LONGFILENAME
#ifdef DIRCACHE_ENABLE
#define DIRCACHE_SIZE 16	/* max. 32 (<=max. cluster size) */
#endif
#endif

#define WR_DATACACHE_SIZE 256	/* min. 1 !!!! */

//#define FORCE_TASK_SWITCH
#ifdef FORCE_TASK_SWITCH
// 1. Let fat function call sleep to force task switch
// to another task to prevent video/audio thread starving issue.
// Note by Steven 2010/7/16.
// 2. The value shouble be (power of two) - 1 because the counter function
// use mask to check the sleep timing.
// 3. 0x7F (511), so every 128 times access, force task switch.
#define FORCE_SLEEP_PERIOD  0x7F
#endif

#ifdef INTERNAL_MEMFN
#define _memcpy(d,s,l) _f_memcpy(d,s,l)
#define _memset(d,c,l) _f_memset(d,c,l)
#else
#include <string.h>
#define _memcpy(d,s,l) memcpy(d,s,l)
#define _memset(d,c,l) memset(d,c,l)
#endif

#ifdef USE_MALLOC
#include <stdlib.h>
#endif

/****************************************************************************
 *
 * Last access date
 *
 ***************************************************************************/

#define F_UPDATELASTACCESSDATE 0
/* it defines if a file is opened for read to update lastaccess time */

/****************************************************************************
 *
 * Opened file size
 *
 ***************************************************************************/

#define F_FINDOPENFILESIZE 1
/* set F_FINDOPENFILESIZE to 0 if filelength needs to return with 0 for an opened file  */
/* other case filelength functions can return with opened file length also */

/****************************************************************************
 *
 * Removing enterFS function
 *
 * Backward compatibility for f_enterFS. If F_REMOVEENTERFS is set to 1 then
 * no f_enterFS call is necessary to be called. When this is enabled then
 * some function will returns with F_ERR_TASKNOTFOUND instead of
 * F_ERR_NOMOREENTRY or F_ERR_ALLOCATION. Also some error codes won't be
 * signaled with f_getlasterror. Don't call f_enterFS when F_REMOVEENTERFS
 * is set to 1. Normal setting of F_REMOVEENTERFS is 0. Please change
 * this only for backward compatibility if necessary, other case add
 * f_enterFS calls into your code.
 *
 ***************************************************************************/

#define F_REMOVEENTERFS 0

/****************************************************************************
 *
 * closing bracket for C++
 *
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of udefs_f.h
 *
 ***************************************************************************/

#endif /* _UDEFS_F_H_ */

