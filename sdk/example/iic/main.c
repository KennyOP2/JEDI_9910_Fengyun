/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#include "stdio.h"
#include "pal/pal.h"
#include "mmp.h"
#include "mmp_iic.h"
#include "host/ahb.h"

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#endif

#define TOUCH_DEVICE_ID 0x48

#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

MMP_RESULT DoTest(void)
{
    MMP_RESULT result;
    MMP_UINT  i; 
    MMP_UINT8 cmd, measureX;
	MMP_UINT32 regVal = 0;
	AHB_WriteRegisterMask(0x7848, (0x1 << 17 | 0x1 << 15), (0x1 << 17 | 0x1 << 15));
	AHB_ReadRegister(0x7848, &regVal);
	printf("(%d), 0x7848: 0x%X\n", __LINE__, regVal);
	AHB_WriteRegisterMask(0x7870, (0x0 << 17 | 0x1 << 16), (0x0 << 17 | 0x1 << 16));
	AHB_ReadRegister(0x7870, &regVal);
	printf("(%d), 0x7870: 0x%X\n", __LINE__, regVal);

	result = mmpIicInitialize(CONTROLLER_MODE, 0,0,0,0,0, 0);
    if(result)
    {
        result = -1;
        goto end;    
    }
    cmd = 0x82;     //active x driver
    result = mmpIicSendDataEx(IIC_MASTER_MODE, TOUCH_DEVICE_ID, &cmd,1); 
    if(result)
        printf("[I2C][WRTIE] ERROR = %x \n", result);
    
    //measure x
    cmd = 0xC2;
    result = mmpIicReceiveData(IIC_MASTER_MODE, TOUCH_DEVICE_ID , cmd, &measureX, 1);
    if(result)
        printf("[I2C][READ] ERROR = %x \n", result);
   
end:
    return result;
}

MMP_RESULT DoTest2(void)
{
	#define SLAVE_ADDR	(0x98>>1)
	
    MMP_RESULT result  = 0;
	MMP_UINT8  regAddr = 0;
	MMP_UINT8  buf[8]  = {0};

	result = mmpIicInitialize(CONTROLLER_MODE, 0,0,0,0,0, 0);
    if(result)
    {
        result = -1;
        goto end;    
    }

	/*
	regAddr = 0xCE;	// HDMI REG_INT_MASK1
    result = mmpIicReceiveData(IIC_MASTER_MODE, SLAVE_ADDR, regAddr, buf, 1);
    if(result)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("Origin value of 0x%X is 0x%X\n", regAddr, buf[0]);
	}
	*/
	
    regAddr = 0x09;	// HDMI REG_INT_MASK1
	buf[0] = 0x09;
	//result = mmpIicSendData(IIC_MASTER_MODE, SLAVE_ADDR, regAddr, buf, 1);
	result = mmpIicSendDataEx(IIC_MASTER_MODE, SLAVE_ADDR, &regAddr, 1); 
    if(result)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("Write 0x%X to regAddr 0x%X\n", buf[0], regAddr);
	}
    
    //measure x
    regAddr = 0xC6;	// HDMI REG_INT_MASK1
    result = mmpIicReceiveData(IIC_MASTER_MODE, SLAVE_ADDR, regAddr, buf, 1);
    if(result)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("The value read from 0x%X is 0x%X\n", regAddr, buf[0]);
	}
   
end:
    return result;
}

/* Test Interrupt */
MMP_RESULT DoTest3(void)
{
	#define SLAVE_ADDR	0x48
	
    MMP_RESULT result  = 0;
	MMP_UINT8  regAddr = 0;
	MMP_UINT8  buf[8]  = {0};

	/* Enable interrupt in AMBA */
	AHB_WriteRegisterMask(0xDE200000 + 0x04, (1 << 19), (1 << 19));
	AHB_WriteRegisterMask(0xDE200000 + 0x24, (1 << 19), (1 << 19));

	result = mmpIicInitialize(CONTROLLER_MODE, 0,0,0,0,0, 0);
    if(result)
    {
        result = -1;
        goto end;    
    }
	
    regAddr = 0x82;	// HDMI REG_INT_MASK1
	buf[0] = 0xF8;
	result = mmpIicSendDataEx(IIC_MASTER_MODE, SLAVE_ADDR, &regAddr, 1); 
    if(result)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("Write 0x%X to regAddr 0x%X\n", buf[0], regAddr);
	}
    
    //measure x
    regAddr = 0xC6;	// HDMI REG_INT_MASK1
    result = mmpIicReceiveData(IIC_MASTER_MODE, SLAVE_ADDR, regAddr, buf, 1);
    if(result)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("The value read from 0x%X is 0x%X\n", regAddr, buf[0]);
	}
   
end:
    return result;
}

/* Test IIC slave */
MMP_RESULT DoTest4(void)
{	
    MMP_RESULT result = 0;

    printf("[IIC_TEST]DoTest4().\n");
    
	result = mmpIicInitialize(CONTROLLER_MODE, 0,0,0,0,0, 0);
	if ( result )
	{
		printf("[IIC_TEST]mmpIicInitialize() fail, code = %d\n", result);
	}

	while( 1 )
	{
		#define INPUT_BUF_LENGTH	512
		
		MMP_UINT8  inutBuffer[INPUT_BUF_LENGTH] = {0xFF};
		MMP_UINT32 i;
		
		result = mmpIicSlaveRead(inutBuffer, INPUT_BUF_LENGTH);
		if ( result )
		{
			printf("[IIC_TEST]Receive data......count = %d\n", result);
			for ( i = 0; i < result ; i++ )
			{
				printf("0x%02X ", inutBuffer[i]);
			}
			printf("\n");
		}
	}
}

int  main(int argc, char** argv)
{
#if defined(__FREERTOS__)
    signed portBASE_TYPE ret = pdFAIL;

    //HOST_GetChipVersionfromReg();

    ret = xTaskCreate(main_task_func, "iictest_main",
        configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
    if (pdFAIL == ret) {
        printf(" ERROR: Failed to create main task!!\n");
        return 1;
    }
    
    //RISC hw uart test
	//PalEnableUartPrint(MMP_TRUE,0,7,115200);
    //RISC printbuffer
    PalEnablePrintBuffer(MMP_TRUE,4*1024); 

    vTaskStartScheduler();
#endif
    DoTest4();
}

#if defined(__FREERTOS__)
portTASK_FUNCTION(main_task_func, params)
{
    MMP_INT result = 0;
    DoTest4();
}
#endif

