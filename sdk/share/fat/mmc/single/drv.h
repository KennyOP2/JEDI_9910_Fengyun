#ifndef _DRV_H_
#define _DRV_H_

/****************************************************************************
 *
 *            Copyright (c) 2003 by HCC Embedded 
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
 * Budapest 1132
 * Victor Hugo Utca 11-15
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

#include "../../common/fat.h"
#include "mmc_dsc.h"


#define F_MMC_DRIVE0 0
#define F_MMC_DRIVE1 1
#define F_MMC_DRIVE2 2
#define F_MMC_DRIVE3 3

int spi_init (void);			/* init SPI */
void spi_set_baudrate (unsigned long);	/* set baudrate */
unsigned long spi_get_baudrate (void);	/* get baudrate */
void spi_tx1 (unsigned char);		/* transmit 1 byte */
void spi_tx2 (unsigned short);		/* transmit 2 bytes */
void spi_tx4 (unsigned long);		/* transmit 4 bytes */
void spi_tx512 (unsigned char *);	/* transmit 512 bytes */
unsigned char spi_rx1 (void);		/* receive 1 byte */
void spi_rx512 (unsigned char *);	/* receive 512 bytes */
void spi_cs_lo (void);			/* CS low */
void spi_cs_hi (void);			/* CS high */

int get_cd (void);		/* get Card Detect state */
int get_wp (void);		/* get Write Protect state */

#ifdef __cplusplus
}
#endif

#endif

