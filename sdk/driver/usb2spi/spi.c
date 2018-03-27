// AfaUsb2Spi.cpp : Defines the entry point for the DLL application.
//
#include "usb2spi\FTCSPI.h"


#define MAX_SPI_93LC56B_CHIP_SIZE_IN_WORDS 128

#define MAX_FREQ_93LC56B_CLOCK_DIVISOR 3   // equivalent to 1.5MHz

const BYTE SPI_EWEN_CMD = '\x9F'; // set up write enable command
const BYTE SPI_EWDS_CMD = '\x87'; // set up write disable command
const BYTE SPI_ERAL_CMD = '\x97'; // set up erase all command

#define NUM_93LC56B_CMD_CONTOL_BITS 11
#define NUM_93LC56B_CMD_CONTOL_BYTES 2

#define NUM_93LC56B_CMD_DATA_BITS 16
#define NUM_93LC56B_CMD_DATA_BYTES 2

#define MAX_READ_DATA_WORDS_BUFFER_SIZE 65536    // 64k bytes

typedef WORD ReadDataWordBuffer[MAX_READ_DATA_WORDS_BUFFER_SIZE];
typedef ReadDataWordBuffer *PReadDataWordBuffer;
typedef struct SHARE_DATA_TAG
{
    FTC_HANDLE ftHandle;
    FTC_CHIP_SELECT_PINS ChipSelectsDisableStates;
    FTC_INPUT_OUTPUT_PINS HighInputOutputPins;
    FTC_HIGHER_OUTPUT_PINS HighPinsWriteActiveStates;
    FTC_INIT_CONDITION ReadStartCondition;
    FTC_WAIT_DATA_WRITE WaitDataWriteComplete;
    FTC_INIT_CONDITION WriteStartCondition;
}SHARE_DATA;


//CRITICAL_SECTION    CriticalSection;

FTC_HANDLE ftHandle;
FTC_CHIP_SELECT_PINS ChipSelectsDisableStates;
FTC_INPUT_OUTPUT_PINS HighInputOutputPins;
FTC_HIGHER_OUTPUT_PINS HighPinsWriteActiveStates;
FTC_INIT_CONDITION ReadStartCondition;
FTC_WAIT_DATA_WRITE WaitDataWriteComplete;
FTC_INIT_CONDITION WriteStartCondition;
static HANDLE hFileMapping = NULL;
static DWORD *mapAddr = NULL;


int SpiClose()
{    
    if (ftHandle)
    {
        SPI_Close(ftHandle);
    }
    //DeleteCriticalSection(&CriticalSection);
   
    return 0;
}


int SpiOpen(DWORD dwClockRate)
{
    FTC_STATUS Status = FTC_SUCCESS;
    DWORD dwNumDevices = 0;
    char szDeviceName[100];
    DWORD dwLocationID = 0;
    BOOL bPerformCommandSequence = false;
    DWORD dwClockFrequencyHz = 0;
    DWORD dwDividerValue;

    Status = SPI_GetNumDevices(&dwNumDevices);

    //Status = SPI_GetDllVersion(szDllVersion, 10);
    //InitializeCriticalSection(&CriticalSection);

    if ((Status == FTC_SUCCESS) /*&& (dwNumDevices > 0)*/)
    {
        if(dwNumDevices == 0)
        {
              Status = SPI_OpenEx(szDeviceName, dwLocationID, &ftHandle);
        }
        else if (dwNumDevices == 1)
        {
            Status = SPI_GetDeviceNameLocID(0, szDeviceName, 100, &dwLocationID);

            if (Status == FTC_SUCCESS)
            {
                Status = SPI_OpenEx(szDeviceName, dwLocationID, &ftHandle);
                //Status = SPI_Open(&ftHandle);
            }
        }
        else
        {
            if (dwNumDevices == 2)
            {
                Status = SPI_GetDeviceNameLocID(1, szDeviceName, 100, &dwLocationID);

                if (Status == FTC_SUCCESS)
                {
                    Status = SPI_OpenEx(szDeviceName, dwLocationID, &ftHandle);
                    //Status = SPI_Open(&ftHandle);
                }
            }
        }
    }


    if ((Status == FTC_SUCCESS) && (dwNumDevices > 0))
    {
        if (Status == FTC_SUCCESS)
            Status = SPI_GetClock(MAX_FREQ_93LC56B_CLOCK_DIVISOR, &dwClockFrequencyHz);

        if (Status == FTC_SUCCESS)
        {
            //Status = SPI_InitDevice(ftHandle, MAX_FREQ_93LC56B_CLOCK_DIVISOR); //65535
            switch(dwClockRate)
            {
            case 6:
                dwDividerValue = 0;
                break;
            case 3:
                dwDividerValue = 1;
                break;
            case 2:
                dwDividerValue = 2;
                break;
            default:
                dwDividerValue = 2;
                break;
            }
        
            Status = SPI_InitDevice(ftHandle, dwDividerValue); // 12M / ((1+x) * 2)

            bPerformCommandSequence = true;               
            if (bPerformCommandSequence == true)
            {
                if (Status == FTC_SUCCESS)
                Status = SPI_ClearDeviceCmdSequence(ftHandle);
            }

            if (Status == FTC_SUCCESS)
            {
                  // Must set the chip select disable states for all the SPI devices connected to the FT2232C dual device
                  ChipSelectsDisableStates.bADBUS3ChipSelectPinState = false;
                  ChipSelectsDisableStates.bADBUS4GPIOL1PinState = false;
                  ChipSelectsDisableStates.bADBUS5GPIOL2PinState = false;
                  ChipSelectsDisableStates.bADBUS6GPIOL3PinState = false;
                  ChipSelectsDisableStates.bADBUS7GPIOL4PinState = false;

                  HighInputOutputPins.bPin1InputOutputState = true;
                  HighInputOutputPins.bPin1LowHighState = true;
                  HighInputOutputPins.bPin2InputOutputState = true;
                  HighInputOutputPins.bPin2LowHighState = true;
                  HighInputOutputPins.bPin3InputOutputState = true;
                  HighInputOutputPins.bPin3LowHighState = true;
                  HighInputOutputPins.bPin4InputOutputState = true;
                  HighInputOutputPins.bPin4LowHighState = true;

                  Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);
            }

            if (Status == FTC_SUCCESS)
            {
                HighPinsWriteActiveStates.bPin1ActiveState = false;
                HighPinsWriteActiveStates.bPin1State = false;
                HighPinsWriteActiveStates.bPin2ActiveState = false;
                HighPinsWriteActiveStates.bPin2State = false;
                HighPinsWriteActiveStates.bPin3ActiveState = false;
                HighPinsWriteActiveStates.bPin3State = false;
                HighPinsWriteActiveStates.bPin4ActiveState = false;
                HighPinsWriteActiveStates.bPin4State = false;


                ReadStartCondition.bClockPinState = false;
                ReadStartCondition.bDataOutPinState = false;
                ReadStartCondition.bChipSelectPinState = true;
                ReadStartCondition.dwChipSelectPin = ADBUS3ChipSelect;

                WriteStartCondition.bClockPinState = false;
                WriteStartCondition.bDataOutPinState = false;
                WriteStartCondition.bChipSelectPinState = true;
                WriteStartCondition.dwChipSelectPin = ADBUS3ChipSelect;
            
                WaitDataWriteComplete.bWaitDataWriteComplete = false;
            }

        }
    }

    return 0;
}

FTC_STATUS SpiSetCs(int high)
{
    FTC_STATUS Status = FTC_SUCCESS;

#if (0)
    if (high)
    {
        HighInputOutputPins.bPin1LowHighState = true;
        Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);
        //SpiClose();
    }
    else
    {
        //SpiOpen(2);
        HighInputOutputPins.bPin1LowHighState = false;
        Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);
    }
#endif   
    return 0;
}

FTC_STATUS SpiRead(DWORD wrLen, PWriteControlByteBuffer pwrBuf, DWORD rdLen, PReadDataByteBuffer prdBuf)
{
    FTC_STATUS Status = FTC_SUCCESS;
    
    DWORD dwNumDataBytesReturned = 0;
    
    //EnterCriticalSection(&CriticalSection);

    Status = SPI_Read(ftHandle, &ReadStartCondition, true, false, wrLen*8,
                      pwrBuf, wrLen, true, true, rdLen*8,
                      prdBuf, &dwNumDataBytesReturned,
                      &HighPinsWriteActiveStates);

    //LeaveCriticalSection(&CriticalSection);

    return(Status);
}

FTC_STATUS SpiRead2(DWORD wrLen, DWORD wrLenBits, PWriteControlByteBuffer pwrBuf, DWORD rdLen, PReadDataByteBuffer prdBuf)
{
    FTC_STATUS Status = FTC_SUCCESS;
    
    DWORD dwNumDataBytesReturned = 0;
    
    //EnterCriticalSection(&CriticalSection);

    Status = SPI_Read(ftHandle, &ReadStartCondition, true, false, wrLenBits,
                      pwrBuf, wrLen, true, true, rdLen*8,
                      prdBuf, &dwNumDataBytesReturned,
                      &HighPinsWriteActiveStates);

    //LeaveCriticalSection(&CriticalSection);

    return(Status);
}

ULONG SpiWrite(DWORD ctrlLen, PWriteControlByteBuffer pCtrlBuf, DWORD dataLen, PWriteDataByteBuffer pDataBuf)
{
    FTC_STATUS Status = FTC_SUCCESS;
    
    //EnterCriticalSection(&CriticalSection);

    Status = SPI_Write(ftHandle, &WriteStartCondition, true, false,
                      ctrlLen*8, pCtrlBuf, ctrlLen,
                      true, dataLen*8, pDataBuf, dataLen, &WaitDataWriteComplete,
                      &HighPinsWriteActiveStates);

    //LeaveCriticalSection(&CriticalSection);
    
    return(Status);
}

FTC_STATUS SpiSetTrigger(void)
{
    FTC_STATUS Status = FTC_SUCCESS;

   
    HighInputOutputPins.bPin1LowHighState = true;
    Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);
      
    HighInputOutputPins.bPin1LowHighState = false;
    Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);
    Sleep(1);    
    HighInputOutputPins.bPin1LowHighState = true;
    Status = SPI_SetGPIOs(ftHandle, &ChipSelectsDisableStates, &HighInputOutputPins);

    return 0;
}