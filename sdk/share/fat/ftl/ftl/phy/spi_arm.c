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
 * Includes
 *
 ***************************************************************************/

#include "spi.h"

/****************************************************************************
 *
 * port defintions
 *
 * use one of this definitions for SPI where this is used on PORT_A or PORT_C
 *
 ***************************************************************************/

#define SPI_ON_PORT_A

//#define SPI_ON_PORT_C

/****************************************************************************
 *
 * port defintions
 *
 ***************************************************************************/

#define PORTA ((volatile unsigned long*)0xffb00020)
#define PORTB ((volatile unsigned long*)0xffb00024)
#define PORTC ((volatile unsigned long*)0xffb00028)

#define MODE(x) ((unsigned long)(x)<<24)
#define DIR(x)  ((unsigned long)(x)<<16)
#define CSF(x)  ((unsigned long)(x)<<8)
#define DATA(x) ((unsigned long)(x))

#define PBIT0 0x01UL
#define PBIT1 0x02UL
#define PBIT2 0x04UL
#define PBIT3 0x08UL
#define PBIT4 0x10UL
#define PBIT5 0x20UL
#define PBIT6 0x40UL
#define PBIT7 0x80UL

/****************************************************************************
 *
 * USED SPI port defintions
 *
 ***************************************************************************/

#ifdef SPI_ON_PORT_A
#define SPIPORT PORTA
#endif

#ifdef SPI_ON_PORT_C
#define SPIPORT PORTC
#endif

#define SPICS  PBIT0  //chip select 0
#define SPICS1 PBIT5  //chip select 1
#define SPICD  PBIT1  //card detect
#define SPIWP  PBIT2  //write protect
#define SPIDI  PBIT3  //host data in
#define SPICLK PBIT4  //clock
#define SPIDO  PBIT7  //host data out

/****************************************************************************
 *
 * Control lines definitions
 *
 ***************************************************************************/

#define SPI_CS0_LO   *SPIPORT &= ~DATA(SPICS)
#define SPI_CS0_HI   *SPIPORT |= DATA(SPICS)

#define SPI_CS1_LO   *SPIPORT &= ~DATA(SPICS1)
#define SPI_CS1_HI   *SPIPORT |= DATA(SPICS1)

#define SPI_DATA_LO *SPIPORT &= ~DATA(SPIDO)
#define SPI_DATA_HI *SPIPORT |= DATA(SPIDO)
#define SPI_CLK_LO  *SPIPORT &= ~DATA(SPICLK)
#define SPI_CLK_HI  *SPIPORT |= DATA(SPICLK)

#define SPI_DATA_IN ((*SPIPORT)&DATA(SPIDI))
#define SPI_CD_IN   ((*SPIPORT)&DATA(SPICD))
#define SPI_WP_IN   ((*SPIPORT)&DATA(SPIWP))


/****************************************************************************
 *
 * spi_init
 *
 * Init SPI ports, directions
 *
 * spics - output
 * spiwp - input
 * spidi - input
 * spiclk - output
 * spicd - input
 * spido - output
 *
 ***************************************************************************/

void spi_init() {

   *SPIPORT &= ~ ( MODE(SPICS | SPICS1 | SPICLK | SPIDO | SPIDI|SPICD|SPIWP) | DIR(SPIDI|SPICD|SPIWP) );
   *SPIPORT |= DIR(SPICS | SPICS1 | SPICLK | SPIDO);

#ifdef SPI_ON_PORT_C
   *SPIPORT |= CSF(SPICS | SPICS1 | SPICLK | SPIDO | SPIDI|SPICD|SPIWP); //additional portc settings
#endif

   SPI_CS0_HI;
   SPI_CS1_HI;
   SPI_CLK_LO;
}


/****************************************************************************
 *
 * spi_tx_8
 *
 * Send  8 bits on spi
 *
 * INPUTS
 *
 * data - send data to spi
 *
 ***************************************************************************/

void spi_tx_8(unsigned char data) {
register int a;
register int idata=data;

   for (a=0; a<8; a++) {

      if (idata & 0x80) SPI_DATA_HI;
      else SPI_DATA_LO;

      SPI_CLK_HI;

      idata<<=1;

      SPI_CLK_LO;
   }

}

/****************************************************************************
 *
 * spi_tx_buffer
 *
 * Send buffer to serial
 *
 * INPUTS
 *
 * buffer - datas what to send
 * write - number of bytes to write
 *
 ***************************************************************************/

void spi_tx_buffer(unsigned char *buffer,int write) {
   while (write--) {
      register int a;
      register int idata=*buffer++;

      for (a=0; a<8; a++) {

         if (idata & 0x80) SPI_DATA_HI;
         else SPI_DATA_LO;

         SPI_CLK_HI;

         idata<<=1;

         SPI_CLK_LO;
      }
   }

}

/****************************************************************************
 *
 * spi_rx_8
 *
 * Receive 8 bits
 *
 * RETURNS
 *
 * received data
 *
 ***************************************************************************/

unsigned char spi_rx_8() {
register int a;
register int data=0;

   for (a=0; a<8; a++) {

      SPI_DATA_HI;
      SPI_CLK_HI;

      data<<=1;
      if ( SPI_DATA_IN ) {
         data|=1;
      }

      SPI_CLK_LO;
   }

   return (unsigned char)data;
}


/****************************************************************************
 *
 * spi_rx_buffer
 *
 * Receive data into buffer
 *
 * INPUTS
 *
 * buffer - where to store data
 * read - number of bytes to read
 *
 ***************************************************************************/

void spi_rx_buffer(unsigned char *buffer,int read) {

   while (read--) {
      register int a;
      register int data=0;

      for (a=0; a<8; a++) {

         SPI_DATA_HI;
         SPI_CLK_HI;

         data<<=1;
         if ( SPI_DATA_IN ) {
            data|=1;
         }

         SPI_CLK_LO;
      }

      *buffer++=(unsigned char)data;
   }
}

/****************************************************************************
 *
 * spi_cs_lo
 *
 * setting given chip select to low
 *
 * INPUTS
 *
 * cs - chip select number
 *
 ***************************************************************************/

void spi_cs_lo(unsigned char cs) {
   if (!cs) SPI_CS0_LO;
   else if (cs==1) SPI_CS1_LO;
}

/****************************************************************************
 *
 * spi_cs_hi
 *
 * setting given chip select to high
 *
 * INPUTS
 *
 * cs - chip select number
 *
 ***************************************************************************/

void spi_cs_hi(unsigned char cs) {
   if (!cs) SPI_CS0_HI;
   else if (cs==1) SPI_CS1_HI;
}

/****************************************************************************
 *
 * end of spi_arm.c
 *
 ***************************************************************************/

