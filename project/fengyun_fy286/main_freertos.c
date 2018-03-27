//#include "spr_defs.h"
#include "mmp_types.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "utility.h"
//#include "mmp_intr.h"
#ifdef HAVE_FAT
#include "common/fat.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
////#define USE_PRINT_BUF
//#define USE_UART_PRINT
//
//#define PRINT_BUF_SIZE  (4*1024)    // the size of print buffer
////#ifdef EV_BOARD
//#define UART_GPIO_GROUP (0)         // 1 for GPIO (share with LCD),
//                                    // 0 for DGPIO (on APB)
//#define UART_GPIO_N     (7)         // GPIO number
////#else
////#define UART_GPIO_GROUP (1)         // 1 for GPIO (share with LCD),
//                                    // 0 for DGPIO (on APB)
////#define UART_GPIO_N     (3)         // GPIO number
////#endif
//#define UART_BAUDRATE   (115200)    // baud rate

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_INT
PalInitialize(
    void);

int appmain(void);

//=============================================================================
//                              Public Function Definition
//=============================================================================
portTASK_FUNCTION(task_main, params)
{
#ifdef HAVE_FAT
    f_enterFS();
#endif

    appmain();
}

int main(int argc, char** argv)
{
    signed portBASE_TYPE ret = pdFAIL;

    // get booting time
    //int boot_time = mfspr(SPR_TTCR2);
    mtspr(SPR_TTMR2, 0);    // disable timer

    // setup print port
    // !!NOTICE!! it must setup the print port before print message
//    #if defined(USE_PRINT_BUF)
//    PalEnablePrintBuffer(MMP_TRUE, PRINT_BUF_SIZE);
//    #else
//    PalEnablePrintBuffer(MMP_FALSE, PRINT_BUF_SIZE);
//    #endif
//
//    #if defined(USE_UART_PRINT)
//    PalEnableUartPrint(MMP_TRUE, UART_GPIO_GROUP, UART_GPIO_N, UART_BAUDRATE);
//    #else
//    PalEnableUartPrint(MMP_FALSE, UART_GPIO_GROUP, UART_GPIO_N, UART_BAUDRATE);
//    #endif

    // print booting time
    //printf("Booting spend %.4f seconds.\n", (float)boot_time/PalGetSysClock());

    // set memory write protection address from 0 to 0x60,
    PalSetWatchPoint((void*)0x00, (void*)0x60);
	
	// initialize interrupt
    //mmpIntrInitialize();
    
    #ifdef HAVE_FAT
    // init FAT file system
    f_init();
    #endif

    // main task
    ret = xTaskCreate(task_main, "task_main",
        10 * 1024,
        NULL, tskIDLE_PRIORITY + 2, NULL);
    if (pdFAIL == ret) {
        PalExit();
    }

    // audio task
    //if (MMP_FALSE == mmpAudioInitializeTask()) {
    //    PalExit();
    //}

    // initialize PAL
    // all tasks will be initialized in this function
    if (PalInitialize()) {
        PalExit();
    }
    
    vTaskStartScheduler();
    return 0;
}
