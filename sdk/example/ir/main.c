/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "pal/pal.h"
#include "mmp.h"
#include "mmp_ir.h"
#include "host/ahb.h"

//#define IR_IRQ_ENABLE

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "intr/intr.h"
#endif

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

#ifdef IR_IRQ_ENABLE
static void
ir_isr(void* data)
{
	printf("[ir_isr_rx]\n");
    //CapGetSignal();
}

static void
ir_isr_tx(void* data)
{
	printf("[ir_isr_tx]\n");
    //CapGetSignal();
}
#endif

void DoTest(void)
{
//    MMP_INT result = 0;
	MMP_UINT32 key = 0;
//    MMP_UINT32 intr = 0;

    printf("[IR]DoTest\n");

#if defined(IR_IRQ_ENABLE)
    /* register interrupt handler to interrupt mgr */
    ithIntrDisableIrq(ITH_INTR_IR_RX);
    ithIntrClearIrq(ITH_INTR_IR_RX);

    ithIntrRegisterHandlerIrq(ITH_INTR_IR_RX, ir_isr, MMP_NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_IR_RX, ITH_INTR_EDGE);
    ithIntrSetTriggerLevelIrq(ITH_INTR_IR_RX, ITH_INTR_HIGH_RISING);
		
	/* Enable the interrupts. */
	ithIntrEnableIrq(ITH_INTR_IR_RX);

    //Tx
    ithIntrDisableIrq(ITH_INTR_IR_TX);
    ithIntrClearIrq(ITH_INTR_IR_TX);

    ithIntrRegisterHandlerIrq(ITH_INTR_IR_TX, ir_isr_tx, MMP_NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_IR_TX, ITH_INTR_EDGE);
    ithIntrSetTriggerLevelIrq(ITH_INTR_IR_TX, ITH_INTR_HIGH_RISING);
		
	/* Enable the interrupts. */
	ithIntrEnableIrq(ITH_INTR_IR_TX);

#endif

    mmpIrInitialize(0);

	while (1)
	{        
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x87);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0xb);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x16);
        AHB_WriteRegister(SIGNAL_CAP_BASE + CAP_SDR_REG, 0x290);
                    
		//key = mmpIrGetKey();
		//if (key)
        //{
        //    printf("key is: 0x%X\n", key);
        //}
        PalSleep(3000);
	}
}

int  main(int argc, char** argv)
{    
    int iStatReg,garbage,i,j;
    static MMP_UINT16 g_TxBuffer[34] = 
                   {0x87,0x16,0x16,0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,
                    0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,
                    0xb,0xb,0xb,0xb,0xb,0xb,0xb,0xb,0x16,0x16,0x16,0x290}; //34

    static MMP_UINT16 g_RxBuffer[34] = {0};

#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //PalEnablePrintBuffer(MMP_TRUE,4*1024); 
    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "irtest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );

    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }

    vTaskStartScheduler();

    //DoTest();

#else
	MMP_UINT32 key = 0;
    MMP_RESULT result = 0;
    MMP_UINT16 *txBuf = MMP_NULL;
    MMP_UINT16 *rxBuf = MMP_NULL; 

    result = mmpDmaInitialize();
    if(result)
        printf("DMA init fail\n");

    mmpIrInitialize(0);

    //for(i = 0; i < 68; ++i)
    //    g_TxBuffer[i] = (i & 0xFFF);
    

    for(j = 0; j < 1000; j++)
    {
        txBuf = malloc(34 * sizeof(MMP_UINT16));
        if (txBuf == 0)
        {
            printf("alloc fail\n");
        }
        rxBuf = malloc(34 * sizeof(MMP_UINT16));
        if(rxBuf == 0)
        {
            printf("alloc fail\n");
        }

        usb2spi_WriteMemory(txBuf, g_TxBuffer, 34 * sizeof(MMP_UINT16));
        //PalMemcpy(txBuf, g_TxBuffer, 1024 * sizeof(MMP_UINT16));

        for(i = 0; i < 34; ++i)
        {
            printf("0x%x,", g_TxBuffer[i]);      
        }
        printf("\n");

        mmpIrDmaWrite(txBuf, 68);   //68*2 
       
        while(1)
        {
            if (mmpIrDmaWaitIdle()== 0)
                break;
            printf("wait DMA Idle\n");
        }

        mmpIrDmaRead(g_RxBuffer, 68);

        usb2spi_ReadMemory(rxBuf, g_RxBuffer, 34 * sizeof(MMP_UINT16));

        for(i = 0; i < 34; ++i)
        {
            printf("0x%x,", rxBuf[i]);      
        }
        printf("\n");

        if (rxBuf[1]>0x92 || rxBuf[1]<0x7c)
            break;
        if (rxBuf[2]>0x18 || rxBuf[2]<0x14 
         || rxBuf[3]>0x18 || rxBuf[3]<0x14)
            break;
        if (rxBuf[4]>0xc || rxBuf[4]<0xa 
         || rxBuf[5]>0xc || rxBuf[5]<0xa
         || rxBuf[6]>0xc || rxBuf[6]<0xa
         || rxBuf[7]>0xc || rxBuf[7]<0xa
         || rxBuf[8]>0xc || rxBuf[8]<0xa
         || rxBuf[9]>0xc || rxBuf[9]<0xa
         || rxBuf[10]>0xc || rxBuf[10]<0xa
         || rxBuf[11]>0xc || rxBuf[11]<0xa)
            break;
        if (rxBuf[12]>0x18 || rxBuf[12]<0x14
         || rxBuf[13]>0x18 || rxBuf[13]<0x14
         || rxBuf[14]>0x18 || rxBuf[14]<0x14
         || rxBuf[15]>0x18 || rxBuf[15]<0x14
         || rxBuf[16]>0x18 || rxBuf[16]<0x14
         || rxBuf[17]>0x18 || rxBuf[17]<0x14
         || rxBuf[18]>0x18 || rxBuf[18]<0x14
         || rxBuf[19]>0x18 || rxBuf[19]<0x14
         || rxBuf[20]>0x18 || rxBuf[20]<0x14
         || rxBuf[21]>0x18 || rxBuf[21]<0x14
         || rxBuf[22]>0x18 || rxBuf[22]<0x14)
            break;
        if (rxBuf[23]>0xc || rxBuf[23]<0xa 
         || rxBuf[24]>0xc || rxBuf[24]<0xa
         || rxBuf[25]>0xc || rxBuf[25]<0xa
         || rxBuf[26]>0xc || rxBuf[26]<0xa
         || rxBuf[27]>0xc || rxBuf[27]<0xa
         || rxBuf[28]>0xc || rxBuf[28]<0xa
         || rxBuf[29]>0xc || rxBuf[29]<0xa
         || rxBuf[30]>0xc || rxBuf[30]<0xa)
            break;
        if (rxBuf[31]>0x18 || rxBuf[31]<0x14
         || rxBuf[32]>0x18 || rxBuf[32]<0x14
         || rxBuf[33]>0x18 || rxBuf[33]<0x14)
            break;
        printf("-----------------------------------------\n");

        //if (memcmp(&rxBuf[1+j], &g_TxBuffer[0], 34-j) == 0) //68-5
        //    printf("Data the same\n");
        //else
        //{
        //    printf("Data diff\n");
        //    break;
        //}

        free(rxBuf);
        free(txBuf);
        rxBuf = MMP_NULL;
        txBuf = MMP_NULL;
    }

	while (1)
	{        
		/*key = mmpIrGetKey();
		if (key)
			printf("key is: 0x%X\n", key);*/
		//PalSleep(500);
	}

#endif
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    printf("portTASK_FUNCTION\n");
    DoTest();
}
#endif
