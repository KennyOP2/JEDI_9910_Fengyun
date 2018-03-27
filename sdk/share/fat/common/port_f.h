#ifndef _PORT_F_H_
#define _PORT_F_H_

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

extern unsigned long f_getrand(unsigned long rand);
extern unsigned short f_getdate(void);
extern unsigned short f_gettime(void);

#if (!FN_CAPI_USED)
extern int f_mutex_get (FN_MUTEX_TYPE *);
extern int f_mutex_put (FN_MUTEX_TYPE *);
extern int f_mutex_create (FN_MUTEX_TYPE *);
extern int f_mutex_delete (FN_MUTEX_TYPE *);
extern long fn_gettaskID(void);
#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of port_f.h
 *
 ***************************************************************************/

#endif /* _PORT_F_H_ */

