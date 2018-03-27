#ifndef _MMC_SMEDIA_H_
#define _MMC_SMEDIA_H_

#include "mmc/single/mmc.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DTV_SD1_ENABLE
    extern F_DRIVER *sd_initfunc(unsigned long driver_param);
#endif

#ifdef DTV_SD2_ENABLE
    extern F_DRIVER *sd2_initfunc(unsigned long driver_param);
#endif

#ifdef DTV_MS_ENABLE
    extern F_DRIVER *ms_initfunc(unsigned long driver_param);
#endif

#ifdef DTV_USB_ENABLE
    extern F_DRIVER *usb0_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb1_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb2_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb3_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb4_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb5_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb6_initfunc(unsigned long driver_param);
    extern F_DRIVER *usb7_initfunc(unsigned long driver_param);
#endif

#ifdef DTV_CF_ENABLE
    extern F_DRIVER *cf_initfunc(unsigned long driver_param);
#endif

#ifdef DTV_xD_ENABLE
    extern F_DRIVER *xd_initfunc(unsigned long driver_param);
#endif

//extern F_DRIVER *temp1_initfunc(unsigned long driver_param);


#ifdef __cplusplus
}
#endif

/******************************************************************************
 *
 * end of mmc_smedia.h
 *
 *****************************************************************************/

#endif /* _MMC_SMEDIA_H_ */

