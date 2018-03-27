/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for OR32 UART function.
 *
 * @author Kuoping Hsu
 * @date 2007.02.15.
 * @version 1.0
 *
 */

#ifndef __OR32_UART_H__
#define __OR32_UART_H__

/////////////////////////////////////////////////////////////////
//              Global Variable
/////////////////////////////////////////////////////////////////
extern int en_uprint;
extern int uprint_gpio_group;
extern int uprint_gpio_pin;
extern int uprint_baud_rate;

/////////////////////////////////////////////////////////////////
//              Include File & Constat Declaration
/////////////////////////////////////////////////////////////////
// Baud rate defination,
#define BAUDRATE        uprint_baud_rate

// GPIO Setting
#define USE_UART_GPIO   uprint_gpio_group           // 0: use DGPIO (on APB), 1: use GPIO (on LCD)
#define USE_GPIO_N      uprint_gpio_pin

/***********************************************************************************/
// GPIO Mapping Table
#define UART_TX_PORT            uart_tx_port        // ((USE_UART_GPIO) ? (USE_GPIO_N/3*2+0x0100)     : 0x68000000);     // GPIO data output register
#define UART_TX_DPORT           uart_tx_dport       // ((USE_UART_GPIO) ? (USE_GPIO_N/3*2+0x0100)     : 0x68000008);     // GPIO direction register
#define GPIO_SEL_PORT           gpio_sel_port       // ((USE_UART_GPIO) ? (USE_GPIO_N/3*2+0x0100)     : 0x68000048);     // GPIO select register
#define UART_TX_SEL_MSK         uart_tx_sel_msk     // ((USE_UART_GPIO) ? (3 << ((USE_GPIO_N%3)*2+9)) : (3 << (USE_GPIO_N * 2 ))); // GPIO(n) Select Pins
#define UART_TX_SEL             uart_tx_sel         // ((USE_UART_GPIO) ? (2 << ((USE_GPIO_N%3)*2+9)) : (0 << (USE_GPIO_N * 2 ))); // GPIO(n) Select Pins
#define UART_TX_PIN             uart_tx_pin         // ((USE_UART_GPIO) ? (1 << (USE_GPIO_N%3+3))     : (1 << USE_GPIO_N));    // GPIO(n) Output Pins
#define UART_TX_DDR             uart_tx_ddr         // ((USE_UART_GPIO) ? (1 << (USE_GPIO_N%3))       : (1 << USE_GPIO_N));    // GPIO(n) Output enable Pins of I/O direction
#define DPORT_ACTIVE_HIGH       dport_active_high   // ((USE_UART_GPIO) ? 0 : 1);                                              // Set 0 as output port

#if 0
#define IO_WriteMask(addr, data, mask) \
                                if (USE_UART_GPIO == 1) HOST_WriteRegisterMask((short)(addr), (short)(data), (short)(mask)); \
                                else AHB_WriteRegisterMask((addr), (data), (mask))
#else // optimize for speed
#define IO_WriteMask(addr, data, mask) \
                                if (USE_UART_GPIO == 1) *(volatile short*)(0x50000000+addr)= ((*(volatile short*)(0x50000000+addr) & ~(short)(mask)) | (short)(data)); \
                                else *(volatile int*)(addr)= ((*(volatile int*)(addr) & ~(int)(mask)) | (int)(data))
#endif

#define GPIO_INIT()             IO_WriteMask(GPIO_SEL_PORT, UART_TX_SEL, UART_TX_SEL_MSK)
#define SET_TX_PIN()            IO_WriteMask(UART_TX_PORT, UART_TX_PIN, UART_TX_PIN)
#define CLEAR_TX_PIN()          IO_WriteMask(UART_TX_PORT, 0, UART_TX_PIN)
#define ENABLE_TX_PIN()         if (DPORT_ACTIVE_HIGH == 1) { IO_WriteMask(UART_TX_DPORT, UART_TX_DDR, UART_TX_DDR); } \
                                else { IO_WriteMask(UART_TX_DPORT, 0, UART_TX_DDR); }

/////////////////////////////////////////////////////////////////
//                      Function Declaration
/////////////////////////////////////////////////////////////////
void or32_uart_putchar(char ch);
//char or32_uart_getchar(void);
//void or32_flush_input_buffer(void);
//char or32_kbhit(void);

#endif // __OR32_UART_H__
