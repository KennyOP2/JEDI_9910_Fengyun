#ifndef _SPI_H_
#define _SPI_H_

/****************************************************************************
 *
 *            Copyright (c) 2006-2008 by HCC Embedded
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
 * Open bracket for C++ compatibility
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Externed functions
 *
 ***************************************************************************/

extern void spi_init(void);

extern void spi_cs_lo(unsigned char cs);
extern void spi_cs_hi(unsigned char cs);

extern unsigned char spi_rx_8(void);
extern void spi_tx_8(unsigned char data);

extern void spi_rx_buffer(unsigned char *buffer,int read);
extern void spi_tx_buffer(unsigned char *buffer,int write);

/****************************************************************************
 *
 * Close bracket for C++
 *
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of spi.h
 *
 ***************************************************************************/

#endif /* _SPI_H_ */

