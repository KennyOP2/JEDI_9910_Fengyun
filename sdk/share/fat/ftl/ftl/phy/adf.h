#ifndef _ADF_H_
#define _ADF_H_

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
 * C++ opening bracket for compatibility
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Select one of theese chips
 *
 ***************************************************************************/

/*#define AT45DB11B*/     /* 1 Mbit chip uses adf256.c */
/*#define AT45DB21B*/     /* 2 Mbit chip uses adf256.c */
/*#define AT45DB41B*/     /* 4 Mbit chip uses adf256.c */
/*#define AT45DB81B*/     /* 8 Mbit chip uses adf256.c */
#define AT45DB161B    /* 16 Mbit chip uses adf.c */
/*#define AT45DB321B*/    /* 32 Mbit chip uses adf.c */
/*#define AT45DB642B*/    /* 64 Mbit chip uses adf.c */


/****************************************************************************
 *
 * Set this define according to how many device is used
 *
 ***************************************************************************/

#define ADF_NUM_OF_DEVICES		  2

/****************************************************************************
 *
 *	Chip definitions
 *
 ***************************************************************************/

#ifdef AT45DB11B     /* 1 Mbit chip */
#define ADF_PAGE_SIZE	          256u
#define ADF_SPARE_SIZE			  8u
#define ADF_REAL_PAGE_COUNT       512u
#define ADF_NUM_OF_SECTORS        3u
#define ADF_PAGES_PER_SECTOR      256u
#define ADF_BYTE_ADDRESS_WIDTH    9u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x03u

#elif defined AT45DB21B     /* 2 Mbit chip */
#define ADF_PAGE_SIZE	          256u
#define ADF_SPARE_SIZE			  8u
#define ADF_REAL_PAGE_COUNT       1024
#define ADF_NUM_OF_SECTORS        4u
#define ADF_PAGES_PER_SECTOR      512u
#define ADF_BYTE_ADDRESS_WIDTH    9u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x05u

#elif defined AT45DB41B     /* 4 Mbit chip */
#define ADF_PAGE_SIZE	          256u
#define ADF_SPARE_SIZE			  8u
#define ADF_REAL_PAGE_COUNT       2048u
#define ADF_NUM_OF_SECTORS        6u
#define ADF_PAGES_PER_SECTOR      512u
#define ADF_BYTE_ADDRESS_WIDTH    9u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x07u

#elif defined AT45DB81B     /* 8 Mbit chip */
#define ADF_PAGE_SIZE	          256u
#define ADF_SPARE_SIZE			  8u
#define ADF_REAL_PAGE_COUNT       4096u
#define ADF_NUM_OF_SECTORS        10u
#define ADF_PAGES_PER_SECTOR      512u
#define ADF_BYTE_ADDRESS_WIDTH    9u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x09u

#elif defined AT45DB161B    /* 16 Mbit chip */
#define ADF_PAGE_SIZE	          512u
#define ADF_SPARE_SIZE	          16u
#define ADF_REAL_PAGE_COUNT       4096u
#define ADF_NUM_OF_SECTORS        17u
#define ADF_PAGES_PER_SECTOR      256u
#define ADF_BYTE_ADDRESS_WIDTH    10u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x0bu

#elif defined AT45DB321B    /* 32 Mbit chip */
#define ADF_PAGE_SIZE	          512u
#define ADF_SPARE_SIZE	          16u
#define ADF_REAL_PAGE_COUNT       8192u
#define ADF_NUM_OF_SECTORS        17u
#define ADF_PAGES_PER_SECTOR      512u
#define ADF_BYTE_ADDRESS_WIDTH    10u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x0du

#elif defined AT45DB642B    /* 64 Mbit chip */
#define ADF_PAGE_SIZE	          1024u
#define ADF_SPARE_SIZE	          32u
#define ADF_REAL_PAGE_COUNT       8192u
#define ADF_NUM_OF_SECTORS        33u
#define ADF_PAGES_PER_SECTOR      256u
#define ADF_BYTE_ADDRESS_WIDTH    11u
#define ADF_PAGES_PER_BLOCK		  8u
#define ADF_PAGES_ADDRESS_WIDTH	  3u
#define ADF_ID					  0x0fu

#else
#error no device selected in the begining of adf.h
#endif

/****************************************************************************
 *
 * Command opcodes for the flash chip.
 *
 ***************************************************************************/

#define ADF_READ_CONT	      0xe8u
#define ADF_READ		      0xd2u
#define ADF_READ_BUF1	      0xd4u
#define ADF_READ_BUF2	      0xd6u
#define ADF_STATUS		      0xd7u
#define ADF_WRITE_BUF1	      0x84u
#define ADF_WRITE_BUF2	      0x87u
#define ADF_PROGERASE_BUF1	  0x83u
#define ADF_PROGERASE_BUF2	  0x86u
#define ADF_PROG_BUF1	      0x88u
#define ADF_PROG_BUF2	      0x89u
#define ADF_ERASE_PAGE	      0x81u
#define ADF_ERASE_BLOCK	      0x50u
#define ADF_READ_MAIN2BUF1	  0x53u
#define ADF_READ_MAIN2BUF2	  0x55u

/* READY x_bit in the satus register. */
#define ADF_BREADY		(1u<<7)

/****************************************************************************
 *
 * Some block and address precalculated definitions
 *
 ***************************************************************************/

/* define for block address shifting */
#define ADF_BLOCK_ADDRESS_WIDTH (ADF_BYTE_ADDRESS_WIDTH + ADF_PAGES_ADDRESS_WIDTH)

#if ADF_PAGE_SIZE==256
/* If pagesize is less than 256 then 2 pages will be used for storing data */
/* so block number per devices is divided by 2 */
/* Calculate max block from pages and number of devices */
#define ADF_MAX_BLOCK_PER_DEVICE ((ADF_REAL_PAGE_COUNT/2)/ADF_PAGES_PER_BLOCK)
#define ADF_MAX_BLOCK (ADF_MAX_BLOCK_PER_DEVICE*ADF_NUM_OF_DEVICES)
#else
/* Calculate max block from pages and number of devices */
#define ADF_MAX_BLOCK_PER_DEVICE (ADF_REAL_PAGE_COUNT/ADF_PAGES_PER_BLOCK)
#define ADF_MAX_BLOCK (ADF_MAX_BLOCK_PER_DEVICE*ADF_NUM_OF_DEVICES)
#endif

#define ADF_WHOLE_PAGE_SIZE (ADF_PAGE_SIZE+ADF_SPARE_SIZE)

/****************************************************************************
 *
 * C++ closing bracket
 *
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of adf.h
 *
 ***************************************************************************/

#endif /* _ADF_H_ */
