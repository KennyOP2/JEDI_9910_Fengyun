/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */

#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef WIN32
#include <io.h>
#endif
#include "pal/pal.h"
#include "host/host.h"
#include "host/ahb.h"
#include "mmp_tsi.h"
#include "mmp_tso.h"
#include "testPattern1.h"
#include "sys/sys.h"
#include "mem/mem.h"
#include "mod_ctrl.h"
#include "mmp_iic.h"

/****************************************************************************/
/*                          MACRO for testing                               */
/****************************************************************************/

// Unmark NO_PCR_TEST ,and mark PCR_INSERT_TEST and WRONG_PCR_PID_INSERT_TEST for
// pure TS out test. Verification through comparing whether tso data is equal to
// tsi data.
#define NO_PCR_TEST

// Unmark PCR_INSERT_TEST ,and mark NO_PCR_TEST and WRONG_PCR_PID_INSERT_TEST for
// PCR insertion test. Verification through check whether the PCR value is
// incremental and other data section is same of output and input.
// tsi data.
//#define PCR_INSERT_TEST

// Unmark WRONG_PCR_PID_INSERT_TEST ,and mark NO_PCR_TEST and PCR_INSERT_TEST for
// PCR insertion test. Verification through check whether the PCR value is
// inserted abnormal of output and input.
//#define WRONG_PCR_PID_INSERT_TEST

// Enable Test pattern1
#define TEST_PATTERN1

// Init TSI output start port number 
#define TSI_OUT_GPIO_34

/* Extra code sections should be marked of TSI for TSO test because
   some workarounds for old TSI engine ,which cause abnormal behavior
   of TSO test.
   
function mmpTsiReceive:
mark the following code sections.
    // workaround: maybe tsi still keep data in hw buffer
    // and doesn't write to memory yet
    // so we just ignore the latest data to avoid reading wrong data
    //if (readSize > 32)
    //    readSize -= 32;
    //else
    //    readSize = 0;

and
    // workaround: maybe tsi still keep data in hw buffer
    // and doesn't write to memory yet
    // so we just ignore the latest data to avoid reading wrong data
    //if (tsiWritePtr < 32)
    //{
    //    if (readSize + tsiWritePtr > 32)
    //        readSize -= (32-tsiWritePtr);
    //    else
    //        readSize = 0;
    //}
*/

/****************************************************************************/
/*                          MACRO                                           */
/****************************************************************************/
#if defined(__FREERTOS__)
xTaskHandle main_task_handle = NULL;
portTASK_FUNCTION_PROTO(main_task_func, params);
#endif

static void
_Verification(
    void);

static MMP_INT
Initialize(
    void)
{
    //WNDCLASS wc;
    MMP_BOOL result;
    MMP_UINT32 error = 0;

end:
    return error;
}

static MMP_INT
Terminate(
    void)
{
    MMP_INT result = 0;

    return result;
}

static MMP_INT
MainLoop(
    void)
{
    return 1;
}

void DoTest(void)
{
    MMP_INT result;

    //result = Initialize();
    
    //if (result)
        //goto testend;

   PalEnablePrintBuffer(MMP_TRUE,4*1024); 
    _Verification();	//seems here has already fail? maybe the default setting has been changed
}

#if defined(__FREERTOS__)
int  main(int argc, char** argv)
{
	signed portBASE_TYPE ret = pdFAIL;

	ret = xTaskCreate(main_task_func, "tsotest_main",
		configMINIMAL_STACK_SIZE * 2,
		NULL, tskIDLE_PRIORITY + 1, &main_task_handle );
	if (pdFAIL == ret) {
		printf(" ERROR: Failed to create main task!!\n");
		return 1;
	}

	vTaskStartScheduler();

	DoTest();
}

portTASK_FUNCTION(main_task_func, params)
{
	DoTest();
}
#else
int
main(
    void)
{
    MMP_INT result;

    result = Initialize();
    if (result)
        goto end;

    _Verification();	//seems here has already fail? maybe the default setting has been changed

    result = MainLoop();
    if (result)
        goto end;

    result = Terminate();
    if (result)
        goto end;

end:
    // Debug memory leaks
    //_CrtDumpMemoryLeaks();

    return result;
}
#endif

static void
_Verification(
    void)
{
    MMP_UINT16  index = 0;
    MMP_UINT8*  pTestBuffer = MMP_NULL;
    MMP_UINT32  testIndex = 0;
    MMP_UINT32  result = 0;
   	MMP_UINT8*  pCurPos = MMP_NULL;
    MMP_INT32   waitSize = 0;
    MMP_INT32   actualSize = 0;
    MMP_UINT32  data = 0;
    MMP_UINT32  i = 0;
    MMP_UINT32  compareLoop = 0;
    MMP_UINT32  compareIndex = 0;
    MMP_UINT8*  pInputStart = MMP_NULL;
    MMP_UINT8*  pPatternStart = MMP_NULL;
    MMP_UINT32  inputPCR = 0;
    MMP_UINT32  patternPCR = 0;
    MMP_UINT32  adaptationFieldLen = 0;
    MMP_UINT32  portStart = 0;

    MMP_UINT32  writeSize = 0;
    MMP_UINT8*  pPatternInput = MMP_NULL;
    
    ChannelModulation      channelModulation;
        
    channelModulation.constellation = Constellation_64QAM;
    channelModulation.highCodeRate = CodeRate_7_OVER_8;
    channelModulation.interval = Interval_1_OVER_32;
    channelModulation.transmissionMode = TransmissionMode_8K;
    channelModulation.frequency = 557000;
    channelModulation.bandwidth = 6000;

#ifdef TEST_PATTERN1
    writeSize = sizeof(pTestPattern1)/6016 * 6016;
    pPatternInput = pTestPattern1;
#endif
    
#ifdef TSI_OUT_GPIO_34
		data = 0;
		// GPIO 34, 35, 36, 37 TSI
		//AHB_ReadRegister(GPIO_BASE + 0x98,&data);
		//data |= (0x2 << 4) | (0x2 << 6) | (0x2 << 8) | (0x2 << 10);
		//AHB_WriteRegister(GPIO_BASE + 0x98, data);
		portStart = 13;
#else
		// GPIO 13, 14, 15, 16 TSO
		AHB_ReadRegister(GPIO_BASE + 0x90,&data);
		data |= (0x2 << 26) | (0x2 << 28) | (0x2 << 30);
		AHB_WriteRegister(GPIO_BASE + 0x90,data);
		data = 0;
		AHB_ReadRegister(GPIO_BASE + 0x94,&data);
		data |= (0x2 << 0);
		AHB_WriteRegister(GPIO_BASE + 0x94,data);
		portStart = 34;
#endif
    //result = mmpTsiInitialize(0);	//Tsi0 base address = 0x1000
	//printf("TSI init: 0x%X..\n", result);
    //printf("TSI enable..\n");
    //mmpTsiEnable(0);
    //HOST_WriteRegister(0x100C, 0x50EF);
    result = mmpIicInitialize(CONTROLLER_MODE, 0, 0, 0, 0, 20, 1);
	if( result )   printf("mmpIicInitialize() err 0x%x !", result);

	//mmpIicLockModule();            
    result = mmpIicSetClockRate(200* 1024);
    if( result )   printf("curr iic clock %d !\n", result);

    ModCtrl_Init();

#ifdef NO_PCR_TEST
    result = mmpTsoInitialize(portStart, 0, 0, 0, 188 * 100, MMP_FALSE);
#elif defined (PCR_INSERT_TEST)
    result = mmpTsoInitialize(portStart, 3011, 32.768, 5000, 188 * 100, MMP_TRUE);
#elif defined (WRONG_PCR_PID_INSERT_TEST)
    result = mmpTsoInitialize(portStart, 3012, 32.768, 0, 188 * 100, MMP_TRUE);
#endif
    printf("TSO init: %u..\n", result);
    mmpTsoEnable();

    pTestBuffer = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT, 188 * 100);
    if (pTestBuffer)
    {
        PalMemset(pTestBuffer, 0x0, 188 * 100);
    }
    else
    {
        printf("Allocate comparison buffer is failed\n");
        goto end;
    }
    
    result = ModCtrl_setTXChannelModulation(&channelModulation);
    result = ModCtrl_acquireTxChannel(&channelModulation);
    //if (result)
    //    while(1);

    ModCtrl_setTxModeEnable();
    
    while(1)//for (index = 0; index < 100; index++)
    {
        mmpTsoWrite(pPatternInput, writeSize);
        waitSize = writeSize;
        do
        {
            MMP_INT32  usedLen = 0;
            MMP_UINT16 regVal = 0;
            HOST_ReadRegister(0x2120, &regVal);
            usedLen |= regVal;
            HOST_ReadRegister(0x2122, &regVal);
            usedLen |= ((regVal & 0x3F) << 16);
       		if (usedLen == 0)
       		    break;
      	} while (1);
      
  	    if (mmpTsoGetStatus() & (0x1 << 7 ))
  	    {
  	        printf("status: 0x%X, Abnoral End\n", mmpTsoGetStatus());
  	        goto end;
  	    }
        continue;
        testIndex = 0;
        PalMemset(pTestBuffer, 0x0, 188 * 100);
        do
        {
            actualSize = 0;
            //mmpTsiReceive(0, &pCurPos, (MMP_ULONG*) &actualSize);

            PalMemcpy(&pTestBuffer[testIndex], pCurPos, actualSize);
            testIndex += actualSize;
            waitSize -= actualSize;            
            PalSleep(10);
        } while (waitSize > 0);

        compareLoop = writeSize / 188;
        compareIndex = 0;
        for (i = 0; i < compareLoop; i++, compareIndex += 188)
        {
            if (PalMemcmp(&pTestBuffer[compareIndex], &pPatternInput[compareIndex], 188))
            {
                pInputStart = &pTestBuffer[compareIndex];
                pPatternStart = &pPatternInput[compareIndex];                
                // Prior TS header or data payload is different
                if (PalMemcmp(pInputStart, pPatternStart, 4)
                 || PalMemcmp(&pInputStart[12], &pPatternStart[12], 176))
                {
                    printf("Run[%u], Comparison is failed\n\n", index);
                    goto end;
                }
                
                // Check whether any adaptation field and packet start indicator
                if (((pInputStart[3] & 0x20)) && (pInputStart[1] & 0x40))
                {
                    adaptationFieldLen = pInputStart[4];
                    // adaptation field length is too short or PCR_flag is not on
                    if (adaptationFieldLen < 7 || !(pInputStart[5] & 0x10))
                    {
                        printf("Run[%u], Comparison is failed\n\n", index);
                        goto end;                        
                    }
                    inputPCR = ((pInputStart[6] << 24 | pInputStart[7] << 16 | pInputStart[8] << 8 | pInputStart[9]) << 1);
                    inputPCR |= (pInputStart[10] >> 7);
                    patternPCR = ((pPatternStart[6] << 24 | pPatternStart[7] << 16 | pPatternStart[8] << 8 | pPatternStart[9]) << 1);
                    patternPCR |= (pPatternStart[10] >> 7);
                    printf("input   6:0x%02X 7:0x%02X 8:0x%02X 9:0x%02X 10:0x%02X 11:0x%02X\n",
                           pInputStart[6],
                           pInputStart[7],
                           pInputStart[8],
                           pInputStart[9],
                           pInputStart[10],
                           pInputStart[11]);

                    printf("pattern 6:0x%02X 7:0x%02X 8:0x%02X 9:0x%02X 10:0x%02X 11:0x%02X\n",
                           pPatternStart[6],
                           pPatternStart[7],
                           pPatternStart[8],
                           pPatternStart[9],
                           pPatternStart[10],
                           pPatternStart[11]);
                                               
                    printf("input PCR: %u, pattern PCR: %u\n", inputPCR, patternPCR);
                }
            }
        }
        printf("Run[%u], Comparison is success\n\n", index);
    }

end:
	printf("END of TSOtest...\n");
    while(1);
}
