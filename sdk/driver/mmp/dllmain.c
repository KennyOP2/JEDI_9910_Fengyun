#include <stdio.h>
#include <crtdbg.h>
#include <windows.h>
#include "mmp.h"
//#include "lcd/lcd_controller.h"
#include "host/host.h"
//#include "cmq/cmd_queue.h"

#if defined(_MSC_VER)
#pragma data_seg(".SHARED")
#endif
MMP_BOOL inited = MMP_FALSE;
#if defined(_MSC_VER)
#pragma data_seg()
#endif

#pragma comment(linker, "/section:.SHARED,RWS")

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        if (!inited)
        {
            if (MMP_FALSE == inited)
            {
                //MMP_Initialize(0);
                MEM_Initialize(0x2000000);
                //CmdQ_Initialize();
                //LCD_Initialize(0);
            }

            inited = MMP_TRUE;
        }
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
