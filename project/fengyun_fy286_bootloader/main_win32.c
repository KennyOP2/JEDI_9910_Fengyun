#include <crtdbg.h>
#include <windows.h>
#include <stdio.h>
//#include "config.h"
#include "pal/pal.h"
#include "host/host.h"

static HWND hWnd;
static UINT_PTR timer;

static void
_Verification(
    void);

static LRESULT CALLBACK
WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    BOOL result;
    DWORD error;

    switch (msg)
    {
    case WM_TIMER:
        //SendDelayedMessages();
        return 0;

    case WM_KEYDOWN:
        switch (wparam)
        {
        case VK_ESCAPE:
            result = DestroyWindow(hwnd);
            if (result == FALSE)
            {
                error = GetLastError();
                PalAssert(!"DestroyWindow FAIL");
            }
            return 0;

        default:
            //SendMsg(wparam, MMP_NULL, 0, 0);
            return 0;
        }
        break;

    case WM_CLOSE:
        result = DestroyWindow(hwnd);
        if (result == FALSE)
        {
            error = GetLastError();
            PalAssert(!"DestroyWindow FAIL");
        }
        return 0;

    case WM_DESTROY:
        result = KillTimer(hWnd, 0);
        if (result == FALSE)
        {
            result = GetLastError();
            PalAssert(!"KillTimer FAIL");
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

static MMP_INT
Initialize(
    void)
{
    WNDCLASS wc;
    BOOL result;
    DWORD error = 0;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    PalAssert(wc.hInstance);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    PalAssert(wc.hIcon);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    PalAssert(wc.hCursor);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ITE";

    result = RegisterClass(&wc);
    if (result == FALSE)
    {
        error = GetLastError();
        PalAssert(!"RegisterClass FAIL");
        goto end;
    }

    hWnd = CreateWindow(
        "ITE",
        "ITE Digital Television",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        NULL,
        NULL,
        wc.hInstance,
        NULL);
    if (hWnd == NULL)
    {
        error = GetLastError();
        PalAssert(!"CreateWindow FAIL");
        goto end;
    }

    ShowWindow(hWnd, SW_SHOW);

    result = UpdateWindow(hWnd);
    if (result == FALSE)
    {
        error = GetLastError();
        PalAssert(!"UpdateWindow FAIL");
        goto end;
    }

//    smtkCfgMgrInit();
//
//    error = smtkFontMgrInitialize();
//    if (error)
//        goto end;
//
//    error = smtkGuiMgrInitialize();
//    if (error)
//        goto end;
//
//    dbg_msg(DBG_MSG_TYPE_ERROR, "width: %d, height: %d\n", smtkGuiMgr.dispWidth, smtkGuiMgr.dispHeight);
//
//#ifdef TS_FILE
//    smtkDtvInit(DTV_FILE_MODE);
//#else
//    smtkDtvDemodInit();
//    smtkDtvInit(DTV_FREE_TO_AIR_MODE);
//#endif
//    InitDelayedMessages();
//    DtvWnd_Init();

    timer = SetTimer(hWnd, 0, 33, NULL);
    if (timer == 0)
    {
        error = GetLastError();
        PalAssert(!"SetTimer FAIL");
        goto end;
    }

end:
    return error;
}

static MMP_INT
Terminate(
    void)
{
    MMP_INT result = 0;

//    DtvWnd_Terminate();
//    TerminateDelayedMessages();
//    smtkDtvTerminate();
//
//#ifndef TS_FILE
//    smtkDtvDemodTerminate();
//#endif
//
//    result = smtkGuiMgrTerminate();
//    if (result)
//        goto end;
//
//    result = smtkFontMgrTerminate();
//    if (result)
//        goto end;
//
    timer = 0;
    hWnd = NULL;
//
//end:
    return result;
}

static MMP_INT
MainLoop(
    void)
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

int
main(
    void)
{
    MMP_INT result;

    result = Initialize();
    if (result)
        goto end;

    _Verification();

    result = MainLoop();
    if (result)
        goto end;

    result = Terminate();
    if (result)
        goto end;

end:
    // Debug memory leaks
    _CrtDumpMemoryLeaks();

    return result;
}

static void
_Verification(
    void)
{
    MMP_UINT16 value = 0;
    
    HOST_ReadRegister(0x2, &value);
    printf("chip id: 0x%x\n", value);    
    value = 0x300;
    HOST_WriteRegister(0x328, value);
    printf("memory test write: 0x%x\n", value);
    PalSleep(1);
    HOST_ReadRegister(0x328, &value);
    printf("memory test read: 0x%x\n", value);
    
    while(1);
}
