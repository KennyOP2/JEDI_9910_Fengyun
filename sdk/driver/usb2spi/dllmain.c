// spi_driver.cpp : Defines the entry point for the DLL application.
//

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include "usb2spi\usb2spi.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    int result;
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            result = usb2spi_Open();
            break;
        case DLL_THREAD_ATTACH:
            //result = usb2spi_Open();
           break;
        case DLL_THREAD_DETACH:
            //usb2spi_Close();
            break;
        case DLL_PROCESS_DETACH:
            usb2spi_Close();
            break;

    }
    return TRUE;
}

