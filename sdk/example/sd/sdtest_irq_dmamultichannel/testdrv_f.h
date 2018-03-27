#ifndef _TESTDRV_F_H_
#define _TESTDRV_F_H_

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

/******************************************************************************
 *
 *	Opening bracket for C++ Compatibility
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Externed function
 *
 *****************************************************************************/

extern F_DRIVER *f_testramdrvinit(unsigned long driver_param);
#if TEST_FULL_COVERAGE
extern void testdrv_set(int errnum, int errcou);
#endif

/******************************************************************************
 *
 * Constants for f_initvolume user pointer parameter
 *
 *****************************************************************************/

#define F_TESTRAM_DRIVE0 0
#define F_TESTRAM_DRIVE1 1

/******************************************************************************
 *
 * enum for error cases
 *
 *****************************************************************************/

enum
{
/* 0 */ TESTDRV_RD,
/* 1 */ TESTDRV_WR,
/* 2 */ TESTDRV_STATE,
/* 3 */ TESTDRV_ERRMAX
};

/******************************************************************************
 *
 * return codes
 *
 *****************************************************************************/

enum
{
/* 0 */ TEST_RAM_NO_ERROR,
/* 1 */ TEST_RAM_ERR_SECTOR,
/* 2 */ TEST_RAM_ERR_NOTAVAILABLE
};

/******************************************************************************
 *
 *	Closing bracket for C++
 *
 *****************************************************************************/

#ifdef __cplusplus
}
#endif

/******************************************************************************
 *
 *  End of testdrv.c
 *
 *****************************************************************************/

#endif /* _TESTDRV_H_ */

