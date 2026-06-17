/**************************************************
 * S.G.M.P
 * @file: VK.c
 * @brief: Implementation of First Window Boiler Plate Code for ARTR
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @Copyright ©2020-2023 MarshallingVoid
 * @date: 04 June 2025 (Wednesday) 09:06:39 PM
 **************************************************/
 

// HEADERS
#include    <Windows.h>
#include    <stdio.h>
#include    <stdbool.h>

#include    "VK.h"

// MACROS
#define     WIN_WIDTH       800
#define     WIN_HEIGHT      600

// GLOBAL VARIABLES
HWND    ghwnd;
BOOL    bFullscreen = FALSE;
BOOL    bActiveWindow = TRUE;
FILE    *gpLogFile = NULL;
DWORD   dwStyle;
BOOL    bDone = FALSE;
BOOL    bEscapeKeyPressed = FALSE;
WINDOWPLACEMENT wpPrev;

// FUNCTION SIGNATURE
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// FUNCTIONS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) 
{

    // FUNCTION SIGNATURE
    int Initialize(void);
    void Uninitialize(void);
    void Display(void);
    void Update(void);

    // VARIABLES
    HWND        hwnd;
    MSG         msg;
    TCHAR       szAppName[] = TEXT("CVKWindow");
    WNDCLASSEX  wndcls;
    RECT        rcDesktop;

    // CODE
    gpLogFile = fopen("VK_log.txt", "w");
    if( 0 == gpLogFile) 
    {
        MessageBox(NULL, TEXT("Log File Cannot Be opened"), TEXT("ERROR"), MB_ICONERROR);
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "WinMain() => Program Started Successfully\n");
    }

    wndcls.cbSize = sizeof(WNDCLASSEX);
    wndcls.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndcls.cbClsExtra = 0;
    wndcls.cbWndExtra = 0;
    wndcls.lpfnWndProc = WndProc;
    wndcls.lpszClassName = szAppName;
    wndcls.lpszMenuName = NULL;
    wndcls.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wndcls.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wndcls.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndcls.hInstance = hInstance;
    wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndcls);

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);

    hwnd = CreateWindowEx(  WS_EX_APPWINDOW,
                            szAppName,
                            TEXT("MST: Vulkan"),
                            WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                            rcDesktop.right/2 - WIN_WIDTH/2,
                            rcDesktop.bottom/2 - WIN_HEIGHT/2,
                            WIN_WIDTH,
                            WIN_HEIGHT,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);

    ghwnd = hwnd;

    Initialize();

    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    while( FALSE == bDone )
    {
        if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            if( WM_QUIT == msg.message)
            {
                bDone = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (TRUE == bActiveWindow)
            {

                if(TRUE == bEscapeKeyPressed)
                {
                    bDone = TRUE;
                }

                Update();
                Display();
            }
        }
    } 

    Uninitialize();
    return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{

    // FUNCTION SIGNATURE
    void Uninitialize(void);
    void Resize(int, int);
    void ToggleFullscreen(void);
    
    // CODE
    switch( iMsg ) 
    {
        
        case WM_CREATE:
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
        
        break;

        case WM_CLOSE:
                Uninitialize();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SETFOCUS:
            bActiveWindow = TRUE;
            break;
        
        case WM_KILLFOCUS:
            bActiveWindow = FALSE;
            break;

        case WM_SIZE:
            Resize(LOWORD(lParam), HIWORD(lParam));
            break;
        
        case WM_KEYDOWN:
            switch(LOWORD(wParam))
            {
                case 0x1B:  //  VK_ESCAPEKEY
                    bEscapeKeyPressed = TRUE;
                    break;
            }
            break;

        case WM_CHAR:
            switch(wParam)
            {
                case 'f':
                case 'F':
                    ToggleFullscreen();
                    break;
            }
            break;
    }   
    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void) 
{
    
    // VARIABLES
    MONITORINFO mi;

    memset((void*)&mi, 0, sizeof(MONITORINFO));
    mi.cbSize = sizeof(MONITORINFO);

    // CODE
    if( !bFullscreen )
    {

        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if( WS_OVERLAPPEDWINDOW & dwStyle)
        {

            if( GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi) )
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & (~WS_OVERLAPPEDWINDOW));
                SetWindowPos( ghwnd,
                              HWND_TOP,
                              mi.rcMonitor.left,
                              mi.rcMonitor.top,
                              mi.rcMonitor.right - mi.rcMonitor.left,
                              mi.rcMonitor.bottom - mi.rcMonitor.top,
                              SWP_NOZORDER | SWP_FRAMECHANGED
                            );
            }
        }
        ShowCursor(FALSE);
        bFullscreen = TRUE;

    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle & WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
        ShowCursor(TRUE);
        bFullscreen = FALSE;
    }
}

int Initialize(void) 
{
    return(0);
}

void Resize(int width, int height) 
{
    // Code
}

void Display(void) 
{
    // Code
}

void Update(void)
{
    // Code
}

void Uninitialize(void) 
{
    // Code

    if( bFullscreen )
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle & WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
        ShowCursor(TRUE);
        bFullscreen = FALSE;
    }

    if( gpLogFile) 
    {
        fprintf(gpLogFile, "Uninitialize() => Program Terminated Successfully\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
}
