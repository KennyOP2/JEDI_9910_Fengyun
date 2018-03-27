#ifndef _FTLDRV_H_
#define _FTLDRV_H_

/****************************************************************************
 *
 *            Copyright (c) 2005-2009 by HCC Embedded
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
 * Includes
 *
 *****************************************************************************/

#include "ftl_defs.h"

// Awin@20080730
//#include "nandflash/configs.h"

/******************************************************************************
 *
 * Conditional Includes
 *
 *****************************************************************************/

#ifdef USE_FATTHIN
#include "../thin_usr.h"
#else
#include "common/fat.h"
#endif


/******************************************************************************
 *
 * Open bracket fot C++ compatibility
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 *	external function for FAT
 *	or USE_FATTHIN defined then for FAT THIN and/or FAT STHIN
 *
 *****************************************************************************/

#if defined(WIN32) && defined(NF_BUILD_DLL)
    
    DLLAPI int ftl_readsector(F_DRIVER *driver,void *data, unsigned long sector);
    DLLAPI int ftl_writesector(F_DRIVER *driver,void *data, unsigned long sector);
    DLLAPI int ftl_readmultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt);
    DLLAPI int ftl_writemultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt);
    DLLAPI int ftl_getphy(F_DRIVER *driver, F_PHY *phy);
    DLLAPI void ftl_release (F_DRIVER *driver);
    DLLAPI F_DRIVER *ftl_initfunc(unsigned long driver_param);

#else

    #ifndef USE_FATTHIN

    extern F_DRIVER *ftl_initfunc(unsigned long driver_param);

    #else

    extern unsigned char ftl_initfunc(void);
    extern unsigned char ftl_readsector(void *_data,unsigned long sector);
    extern unsigned char ftl_writesector(void *_data,unsigned long sector);
    #if F_FORMATTING
    extern unsigned char ftl_getphy(F_PHY *);
    #endif

    #define drv_readsector ftl_readsector
    #define drv_writesector ftl_writesector
    #define drv_initfunc ftl_initfunc

    #if F_FORMATTING
    #define drv_getphy ftl_getphy
    #endif

    #endif

#endif

extern int
mmpNandGetCapacity(unsigned long* lastBlockId,
                   unsigned long* blockLength);
                   
extern int 
mmpNandReadSector(unsigned long blockId,
                  unsigned long sizeInByte,
                  unsigned char* srcBuffer);
                  
extern int 
mmpNandWriteSector(unsigned long blockId,
                  unsigned long sizeInByte,
                  unsigned char* destBuffer); 

/******************************************************************************
 *
 * Includes and defines according if only 512 device is used or not
 *
 *****************************************************************************/

#ifndef USE_FATTHIN
static F_DRIVER ftl_driver;
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
 *  End of ftldrv.h
 *
 *****************************************************************************/

#endif /* _FTLDRV_H_ */
