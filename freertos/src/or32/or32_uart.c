/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Software UART library
 *
 * @author Kuoping Hsu
 * @date 2007.02.15.
 * @version 1.0
 *
 */

/////////////////////////////////////////////////////////////////
//              Include File & Constat Declaration
/////////////////////////////////////////////////////////////////
#include "or32.h"
#include "mmio.h"
#include "or32_uart.h"
#include "spr_defs.h"

#define HWUART

#ifdef HWUART
	#define HWUART_UARTPORT 1   // 0: uart1, 1: uart2
	// NOTE[20120924]: only verified "1: uart2", "0: uart1" may not work
#else
    #define TICKS_COMPENSATE 0

/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////
    static int uartDelayTicks;

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
    int uart_tx_port;
    int uart_tx_dport;
    int gpio_sel_port;
    int uart_tx_sel_msk;
    int uart_tx_sel;
    int uart_tx_pin;
    int uart_tx_ddr;
    int dport_active_high;
#endif
    static int init = 0;

/////////////////////////////////////////////////////////////////
//                      Private Function
/////////////////////////////////////////////////////////////////
#ifdef HWUART
static void UART_Init( unsigned int port, unsigned int baudrate, unsigned int parity, unsigned int num, unsigned int len, unsigned int f_baudrate)
{
    unsigned int lcr;

    lcr = AHB_MMIO_Read(port + 0x0c);
    lcr &= ~0x80;
    /* Set DLAB=1 */
    AHB_MMIO_Write(port + 0x0c, 0x80);
    /* Set baud rate */
    AHB_MMIO_Write(port + 0x4, ((baudrate & 0xf00) >> 8));
    AHB_MMIO_Write(port + 0x0, (baudrate & 0xff));

    /*Set fraction rate*/
    AHB_MMIO_Write(port + 0x10, (f_baudrate & 0xf));

	//clear orignal parity setting
    lcr &= 0xc0;
    if (num==2)
        lcr|=0x4;
    len-=5;
    lcr|=len;

    AHB_MMIO_Write(port + 0x0c, lcr);
}

static void UART_SetMode(unsigned int port, unsigned int mode)
{
	unsigned int mdr;

    mdr = AHB_MMIO_Read(port + 0x20);
    mdr &= ~0x03;
    AHB_MMIO_Write(port + 0x20, mdr | mode);
}

static void UART_SetFifoCtrl(unsigned int port, unsigned int level_tx, unsigned int level_rx, unsigned int resettx, unsigned int resetrx)  //V1.20//ADA10022002
{
    char fcr = 0;

    fcr |= 0x1;

    if (resettx)
        fcr|=0x4;

    if (resetrx)
        fcr|=0x2;

    AHB_MMIO_Write(port + 0x08, fcr);
}
#else // HWUART
static void UART_TIMER_START (void) {
    /* Disable tick timer exception recognition */
#ifdef ENABLE_ONFLY
    mtspr(SPR_SR, mfspr(SPR_SR) & ~(SPR_SR_TEE));
#else
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
#endif

    /* stop timer */
    mtspr(SPR_TTMR2, 0);

    /* Reset counter */
    mtspr(SPR_TTCR2, 0);

    /* single run mode and disable interrupt */
    mtspr(SPR_TTMR2, SPR_TTMR_SR | SPR_TTMR_PERIOD);
}

static void UART_TIMER_END (void) {
    /* stop timer */
    mtspr(SPR_TTMR2, 0);

    /* Reset counter */
    mtspr(SPR_TTCR2, 0);
}

static void UART_DELAY (int k) {
    int ticks;
    do {
        ticks = mfspr(SPR_TTCR2); // get timer
    } while (ticks < k);
}

static void send_uart (char uartTxData) {
    int i;
    int k = 0;

    // timer start
    UART_TIMER_START();

    // start bit
    CLEAR_TX_PIN();
    UART_DELAY(k+=uartDelayTicks);

    // 8-bits data
    for (i=0; i<8; i++) {
        if (uartTxData & 0x1) SET_TX_PIN();
        else                  CLEAR_TX_PIN();
        uartTxData = uartTxData >> 1;
        UART_DELAY(k+=uartDelayTicks);
    }

    // stop bit
    SET_TX_PIN();
    UART_DELAY(k+=uartDelayTicks);

    // timer end
    UART_TIMER_END();
}
#endif

void uart_init(void) {    
#ifdef HWUART   
    unsigned int int_div = 0;
    unsigned int total_div = 0;
    unsigned int f_div = 0;
    unsigned int data;
    unsigned int uart_gpio_pin = 0;

    total_div = (80000000/BAUDRATE);
    int_div = (total_div >> 4);
    f_div = total_div & 0xF;

    // pad select
    if (HWUART_UARTPORT == 0)
    {
    	data = AHB_MMIO_Read(0xDE000000 + 0x90);
    	data |= (3<<0) | (3<<1);
    	AHB_MMIO_Write(0xDE000000 + 0x90, data);
    }
    else
    {
        
#if defined(IT9913_128LQFP)
        uart_gpio_pin = 20; 
        
#elif defined (IT9919_144TQFP)
    #if defined(REF_BOARD_AVSENDER)
        uart_gpio_pin = 20; //for dexatek
    #elif defined (REF_BOARD_CAMERA)
        uart_gpio_pin = 28;
    #else
        uart_gpio_pin = 30; 
    #endif
             
#else
        uart_gpio_pin = 30;
#endif   
     
        // set gpio mode
        if (uart_gpio_pin < 16)
        {
            data = AHB_MMIO_Read(0xDE000000 + 0x90);
            data &= (0x0 << (uart_gpio_pin * 2));
            AHB_MMIO_Write(0xDE000000 + 0x90, data);
        }
        else if (uart_gpio_pin < 32)
        {
            data = AHB_MMIO_Read(0xDE000000 + 0x94);
            data &= (0x0 << ((uart_gpio_pin - 16) * 2));
            AHB_MMIO_Write(0xDE000000 + 0x94, data);
        }
        else if (uart_gpio_pin < 48)
        {
            data = AHB_MMIO_Read(0xDE000000 + 0x98);
            data &= (0x0 << ((uart_gpio_pin - 32) * 2));
            AHB_MMIO_Write(0xDE000000 + 0x98, data);
        }
        else
        {
            data = AHB_MMIO_Read(0xDE000000 + 0x9C);
            data &= (0x0 << ((uart_gpio_pin - 48) * 2));
            AHB_MMIO_Write(0xDE000000 + 0x9C, data);
        }
	
	    if (uart_gpio_pin < 32)
	    {
            // set gpio to be uart
            AHB_MMIO_Write(0xDE000000 + 0xD8, 1<<uart_gpio_pin);
    
            // set gpio direction
            data = AHB_MMIO_Read(0xDE000000 + 0x8);
            data |= (0x1<<uart_gpio_pin);
            AHB_MMIO_Write(0xDE000000 + 0x8, data);  
        }
        else
        {
            // set gpio to be uart
            AHB_MMIO_Write(0xDE000000 + 0xDC, 1<<(uart_gpio_pin-31));
    
            // set gpio direction
            data = AHB_MMIO_Read(0xDE000000 + 0xC);
            data |= (0x1<<(uart_gpio_pin-31));
            AHB_MMIO_Write(0xDE000000 + 0xC, data);              
        }      
    }
		
    // power on clk
//    data = AHB_MMIO_Read(0xc0000020);
//    data |= 0x80000000;
//    AHB_MMIO_Write(0xc0000020, data);

	if (HWUART_UARTPORT == 0)
	{            
	    UART_SetMode(0xDE600000, 0x0);
	    UART_Init(0xDE600000, int_div, 0, 0, 8, f_div);
		UART_SetFifoCtrl(0xDE600000, 1, 1 ,1 ,1);
	}
	else
	{
	    UART_SetMode(0xDE700000, 0x0);
	    UART_Init(0xDE700000, int_div, 0, 0, 8, f_div);
	    UART_SetFifoCtrl(0xDE700000, 1, 1 ,1 ,1);
	}

	return;
		
#else // HWUART
    int hz;

    // calculate DelayTicks
    hz = or32_getSysCLK(); // Get system clock
    uartDelayTicks = (hz/BAUDRATE) - TICKS_COMPENSATE;

    uart_tx_port      = (USE_GPIO_N < 32 ? 0xDE000000 : 0xDE000040);               // GPIO data output register
    uart_tx_dport     = (USE_GPIO_N < 32 ? 0xDE000008 : 0xDE000048);               // GPIO direction register
    gpio_sel_port     = (0xDE000090+(USE_GPIO_N/16)*4);               // GPIO select register
    uart_tx_sel_msk   = (3 << ((USE_GPIO_N%16) * 2 )); // GPIO(n) Select Pins
    uart_tx_sel       = (0 << ((USE_GPIO_N%16) * 2 )); // GPIO(n) Select Pins
    uart_tx_pin       = (1 << USE_GPIO_N);        // GPIO(n) Output Pins                   
    uart_tx_ddr       = (1 << USE_GPIO_N);        // GPIO(n) Output enable Pins of I/O direction
    dport_active_high = 1;                                                  // Set 0 as output port

    GPIO_INIT();
    ENABLE_TX_PIN();
    SET_TX_PIN();
#endif    
}

/////////////////////////////////////////////////////////////////
//                      Public Function
/////////////////////////////////////////////////////////////////
void or32_uart_putchar(char ch) {	
    if (!init) {
        uart_init();
        for (;init<200;) init++;
        // NOTE[20120924]: replace with better way to wait ready
    }

#ifdef HWUART
    unsigned int data;
    
    do {
        if (HWUART_UARTPORT == 0)
        {
        	data = AHB_MMIO_Read(0xDE600000+0x14);
        }
        else
        {
        	data = AHB_MMIO_Read(0xDE700000+0x14);
        }
    } while (!((data & 0x20) == 0x20));

    if (HWUART_UARTPORT == 0)
    {
        AHB_MMIO_Write(0xDE600000, ch);
    }
    else
    {
        AHB_MMIO_Write(0xDE700000, ch);
    }
#else	    
    send_uart(ch);
#endif    
}

// getchar(), do not support currently
//char or32_uart_getchar(void) {
//    return 0;
//}

// flush_input_buffer(), do not support currently
//void or32_flush_input_buffer(void) {
//    return ;
//}

// kbhit(), do not support currently
//char or32_kbhit(void) {
//    return 0;
//}
