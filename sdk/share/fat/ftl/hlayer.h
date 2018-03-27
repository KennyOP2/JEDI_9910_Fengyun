#ifndef _HLAYER_H_
#define _HLAYER_H_

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
 *	includes
 *
 ***************************************************************************/

#ifndef _FTL_DEFS_H_
#include "ftl_defs.h"
#endif

// Awin@20080730
//#include "nandflash/configs.h"

/****************************************************************************
 *
 *	conditional includes
 *
 ***************************************************************************/

#ifndef _MLAYER_H_
#include "ftl/mlayer.h"
#endif

/****************************************************************************
 *
 *	Opening bracket for C++ compatibility
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 *	Functions
 *
 ***************************************************************************/

extern DLLAPI t_bit hl_init(void);
extern DLLAPI t_bit hl_format(void);

extern DLLAPI t_bit hl_open(unsigned long sector, unsigned long secnum, unsigned char mode);
extern DLLAPI t_bit hl_write(unsigned char *datap);
extern DLLAPI t_bit hl_read(unsigned char *datap);
extern DLLAPI t_bit hl_close(void);
extern DLLAPI unsigned long hl_getmaxsector(void);

/****************************************************************************
 *
 * defins for HLayer Modes for internally and for hl_open
 *
 ***************************************************************************/

enum {
	HL_CLOSE,
	HL_READ,
	HL_WRITE,
	HL_ABORT,
	HL_INIT
};

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
 * end of hlayer.h
 *
 ***************************************************************************/

#endif	/* _HLAYER_H_ */
