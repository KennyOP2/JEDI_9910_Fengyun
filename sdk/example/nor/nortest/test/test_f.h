#ifndef _TEST_F_H_
#define _TEST_F_H_

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

/****************************************************************************
 *
 * Includes
 *
 ***************************************************************************/

#include "common/fat.h"
#include "common/common.h"
#include "chkdsk/chkdsk.h"
#if FN_CAPI_USED
#include "../../fw_file.h"
#endif

/****************************************************************************
 *
 * Open bracket for C++ compatibility
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Defines
 *
 ***************************************************************************/


#define TEST_FULL_COVERAGE  0 /* set this value to 1 if you want to check full coverage */
/* this test need more memory to run and it also checks bad block handling and failures on test drive */

#if	!TEST_FULL_COVERAGE
#define TEST_FAT_MEDIA F_FAT16_MEDIA /* defines that test uses this media type */
#else
#define TEST_FAT_MEDIA test_fat_media /* dynamic */
#endif

#define TEST_CHKDSK_ENABLED 0 /* set this value to 1 if you want to check disk test to be enabled */

/****************************************************************************
 *
 *  Exports
 *
 ***************************************************************************/

extern unsigned int f_dotest(void);

/****************************************************************************
 *
 *	Imports
 *
 *	see testport.c
 *
 ***************************************************************************/

extern int _f_poweron(void); /* use to build file system */
extern int _f_poweroff(void); /* use to delete file system */
extern void _f_dump(char *s); /* use to display text */
extern int _f_result(int line,long result); /* use to display test result */

/****************************************************************************
 *
 *  Closing bracket for C++
 *
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of test_f.h
 *
 ***************************************************************************/

#endif	/* _TEST_F_H_ */
