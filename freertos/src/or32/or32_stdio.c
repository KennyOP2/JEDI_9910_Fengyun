/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Function body for newlib porting.
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

#include "ticktimer.h"
#include "or32.h"

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
int en_pbuf   = 0;              // enable print buffer
int pbuf_size = 4*1024;         // size of print buffer

int en_uprint = 1;              // enable UART print
int uprint_gpio_group = 0;      // 0 for DGPIO (on AMBA), 1 for GPIO (on LCD)
int uprint_gpio_pin   = 7;      // GPIO number
int uprint_baud_rate  = 115200; // baud rate

extern unsigned char *__dbgmsg_buf_ptr;
extern int __dbgmsg_buf_len;
extern int __dbgmsg_idx_ptr;

/////////////////////////////////////////////////////////////////
//                      Private Function
/////////////////////////////////////////////////////////////////
/*
 * outbyte -- shove a byte out the serial port. We wait till the byte
 */
PROFILE_FUNC int outbyte(unsigned char byte) {
    DISABLE_PICMR_HOST_INTERRUPT();
    if (en_pbuf) {
        if (__dbgmsg_buf_ptr == NULL) {
            if ((__dbgmsg_buf_ptr = (void *)malloc(pbuf_size)) == NULL) {
                return 0;
            }
            __dbgmsg_buf_len = pbuf_size;
        }

        if (__dbgmsg_idx_ptr < pbuf_size)
            __dbgmsg_buf_ptr[__dbgmsg_idx_ptr++] = byte;
        else
            __dbgmsg_idx_ptr = 0;
    }

    if (en_uprint) {
        or32_uart_putchar(byte);
    }
    ENABLE_PICMR_HOST_INTERRUPT();

    return byte;
}

/////////////////////////////////////////////////////////////////
//                      Global Function
/////////////////////////////////////////////////////////////////
/*
 * Enable the print buffer and setting the parameter
 */
void
or32_enPrintBuffer(
    int enable,
    int size)
{
    en_pbuf   = enable;
    pbuf_size = size;
    if (en_pbuf) en_uprint = 0;
}

/*
 * Enable the UART print and setting the parameter
 */
void
or32_enUartPrint(
    int enable,
    int gpio_group,
    int gpio_pin,
    int baud_rate)
{
    en_uprint         = enable;
    uprint_gpio_group = gpio_group;
    uprint_gpio_pin   = gpio_pin;
    uprint_baud_rate  = baud_rate;
    if (en_uprint) en_pbuf = 0;
}
