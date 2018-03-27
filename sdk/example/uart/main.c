/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "pal/pal.h"
#include "mmp_uart.h"

//#define GPIO_BASE   0x7800
//#define UART1_BASE	
#include "host/ahb.h"
#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "host/ahb.h"
#endif

#ifdef __OPENRTOS__
#include "ite/ith.h"
#endif


#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

MMP_RESULT DoTest(void)
{
    MMP_RESULT result;
    MMP_UINT8 ch;
    MMP_UINT8 ch2 = 0x30;
	MMP_UINT32 regVal = 0;
	MMP_UINT32 addr = 0;
	int i, j = 0;
	MMP_UINT16 data;
	MMP_UINT uartport = UART1;
	MMP_UINT32 uartbase  = UART1_BASE;

#if defined(__FREERTOS__)
	//RISC hw uart test
	//PalEnableUartPrint(MMP_TRUE,0,7,115200);
    //RISC printbuffer
    PalEnablePrintBuffer(MMP_TRUE,4*1024); 
#endif

	printf("[UART]DoTest\n");

	mmpDmaInitialize();

    result = mmpUartInitialize(uartport, 115200, UART_PARITY_NONE, 0, 8);

	printf("mmpUartInitialize\n");

	//---------------------------------------------------------
    if(result)
    {
        result = -1;
        goto end;    
    }
	
    while(1)
    { 
        #if 0
		mmpUartSetLoopback(uartport, 1);
        mmpUartPutChar(uartport, ch2);
		#ifdef __OPENRTOS__
        AHB_ReadRegister(uartbase+0x5c,&data);
		#else
		HOST_ReadRegister(uartbase+0x5c,&data);
		#endif
		printf(" UART: FIFO = 0x%x \n", data);    
        ch2++;
        if(ch2 > 'z')
            ch2 = '0';         
        if(mmpUartGetChar(uartport, &ch))
        {
            printf(" UART: Get Ch = %c  hex = 0x%x \n", ch, ch);

        }
		#ifdef __OPENRTOS__
        AHB_ReadRegister(uartbase+0x5c,&data);
		#else
		HOST_ReadRegister(uartbase+0x5c,&data);
		#endif

		printf(" UART: FIFO = 0x%x \n", data);
        #else
        {
            #define SIZE 16
            char buf[SIZE];
			char buf2[SIZE];
			int error = 0;
#if 0
back:
			mmpUartSetLoopback(uartport, 1);
			for(i=0; i < 10; ++i)
			{
				mmpUartPutChar(uartport, ch2);

				if(mmpUartGetChar(uartport, &ch))
				{
					if(ch != ch2)
					{
						printf(" [ERROR][%d]UART: Get Ch = 0x%x Ch2 = 0x%x \n", i, ch, ch2);
						while(1);
					}
				}

				ch2++;
				if(ch2 > 'z')
					ch2 = '0';
			}

			for(;;)
			{
				if(mmpUartGetChar(uartport, &ch))
				{
					if(ch)
					{
						printf(" UART: Get Ch = 0x%x\n", ch);
					}
				}
			}
#endif
			//printf("UART: PIO TEST PASS\n");

			//if do dma test , please mask "goto back"
			//goto back;
            
            for(i=0; i < SIZE; ++i)
            {
                buf[i] = i & 0xFF;
            }
#if 1
			mmpUartSetLoopback(uartport, 1);

            mmpUartPutChars(uartport, buf, SIZE);

			printf("mmpUartPutChars\n");
#endif
           
			printf("mmpUartInterruptGetChars\n");
			//if(mmpUartInterruptGetChars(uartport, buf2, SIZE)) 
			if(mmpUartGetChars(uartport, buf2, SIZE))
			{
                for(i=0; i < SIZE; ++i)
                {
					
                    printf(" %02x", buf2[i]&0xff);
                    if(i%16 == 15)
                        printf("\n");
					
					if(buf[i] != buf2[i])
					{
						error = 1;
					}
					
                }
                printf("\n");
				if(error)
				{
					printf("UART COMP ERROR\n");
					//goto end;
				}
				else
					printf("UART PASS COUNT %d\n", j+1);
            }
            
        }
		#endif
		j++;
		PalSleep(1000);
	
     }
end:

    return result;
}

int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "uarttest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();
#else
    DoTest();
#endif
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;
    
    result = DoTest();
}
#endif

