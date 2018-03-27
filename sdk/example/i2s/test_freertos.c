/* sy.chuang, 2012-0827, ITE Tech. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

xTaskHandle maintask_handle = NULL;
portTASK_FUNCTION_PROTO(maintask_func, params);

void i2s_main(void);

int main(void)
{
	signed portBASE_TYPE ret = pdFAIL;

	ret = xTaskCreate(maintask_func, "test_i2s_main", configMINIMAL_STACK_SIZE * 2,
	NULL, tskIDLE_PRIORITY + 1, &maintask_handle);

	if(pdFAIL == ret) {
		printf("ERROR# Failed to create main task !\n");
		return 1;
	}

	vTaskStartScheduler();

	return 0;
}

portTASK_FUNCTION(maintask_func, params)
{
	i2s_main();
}

