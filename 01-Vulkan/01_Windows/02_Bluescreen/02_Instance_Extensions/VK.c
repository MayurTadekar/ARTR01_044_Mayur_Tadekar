/**************************************************
 * S.G.M.P
 * @file: VK.c
 * @brief: Step02 Instance Extensions
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @Copyright ©2020-2023 MarshallingVoid
 * @date: 04 June 2025 (Wednesday) 09:32:30 PM
 **************************************************/
 

// HEADERS
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "VK.h"

// Vulkan Headers
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>  //  We must tell the header the platform we are on

// Vulkan Related Libraries
#pragma comment(lib, "vulkan-1.lib")


// MACROS
#define     WIN_WIDTH       800
#define     WIN_HEIGHT      600

// GLOBAL VARIABLES
HWND    ghwnd;
BOOL    bFullscreen = FALSE;
BOOL    bActiveWindow = TRUE;
FILE    *gpLogFile = NULL;
DWORD   dwStyle;
BOOL    bEscapeKeyPressed = FALSE;
WINDOWPLACEMENT wpPrev;

const char* gpszAppName = "ARTR";

// Instance Extension Related Variables
uint32_t enabledInstanceExtensionCount = 0;
const char* enabledInstanceExtensionNames_array[2]; 

//  Extension Needed
//  VK_KHR_SURFACE_EXTENSION_NAME
//  VK_KHR_WIN32_SURFACE_EXTENSION_NAME


// FUNCTION SIGNATURE
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// FUNCTIONS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) 
{

    // FUNCTION SIGNATURE
    VkResult Initialize(void);
    void Uninitialize(void);
    void Display(void);
    void Update(void);

    // VARIABLES
    HWND        hwnd;
    MSG         msg;
    TCHAR       szAppName[255];
    WNDCLASSEX  wndcls;
    RECT        rcDesktop;
    BOOL        bDone = FALSE;
    VkResult    vkResult = VK_SUCCESS;

    // CODE
    gpLogFile = fopen("VK_log.txt", "w");
    if( 0 == gpLogFile) 
    {
        MessageBox(NULL, TEXT("Log File Cannot Be opened"), TEXT("ERROR"), MB_ICONERROR);
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "WndProc()   =>  Program Started Successfully\n");
    }

    wsprintf(szAppName, TEXT("%s"), gpszAppName);

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

    vkResult = Initialize();

    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "WinMain()   =>  Initialize() Failed\n");
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }
    else
    {
        fprintf(gpLogFile, "WinMain()   =>  Initialize() Successful\n");
    }

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

VkResult Initialize(void) 
{
    // Function Declaration
    VkResult FillInstanceExtensionNames(void);

    // Variable Declaration
    VkResult vkResult;

    // Code
    vkResult = FillInstanceExtensionNames();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()   =>  FillInstanceExtensionNames() Failed\n");
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }
    else
    {
        fprintf(gpLogFile, "Initialize()   =>  FillInstanceExtensionNames() Successful\n");
    }



    return(vkResult);
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
        fprintf(gpLogFile, "Uninitialize()  =>  Program Terminated Successfully\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
}

/*****************************************************************************************************************************\
                                        VULKAN RELATED FUNCTIONS
\*****************************************************************************************************************************/

VkResult FillInstanceExtensionNames(void)
{
    // Variable Declaration
    VkResult    vkResult = VK_SUCCESS;
    uint32_t    instanceExtensionCount = 0;
    VkExtensionProperties   *vkExtensionProperties_array = NULL;
    char        **instanceExtensionNames_array = NULL;
    VkBool32    vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32    win32SurfaceExtensionFound = VK_FALSE;

    // Code
    fprintf(gpLogFile, "FillInstanceExtensionNames()  =>  function started\n");


    // Step 01: Find how many instance extensions are supported by the Vulkan Driver for this version & keep the count in the local variable

    // First Vulkan API
    vkResult = vkEnumerateInstanceExtensionProperties(  NULL,                       // Layer Name : कोणत्या layer ची extensions हवी आहे NULL -> सर्वच हवे आहे 
                                                        &instanceExtensionCount,    // Count : ExtensionCount
                                                        NULL);                      // Extension च्या properties चा array (Parameterized Return Value)
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() Failed - First Call\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() Successful - First Call\n");
    }

    //  Step 02: Allocate & Fill struct VkExtensionProperties array corresponding to above count;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    vkResult = vkEnumerateInstanceExtensionProperties(  NULL,
                                                        &instanceExtensionCount,
                                                        vkExtensionProperties_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() Failed - Second Call\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() Successful - Second Call\n");
    }

    //  Step 03: Fill & Display a local string array of extension names obtained from VkExtensionProperties array
    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);

    //  FOR THE SAKE OF BREVITY

    //  Allocate memory for each string member
    for(uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        instanceExtensionNames_array[le] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
    
        memcpy(instanceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  Vulkan Extension Name = %s\n", instanceExtensionNames_array[le]);
    }

    //  Step 04: As not required henceforth, free the allocated VkExtensionProperties Array;
    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    //  Step 05: Find whether above extension names contained our required 2 extensions viz, VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    for(uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        if(0 == strcmp(instanceExtensionNames_array[le], VK_KHR_SURFACE_EXTENSION_NAME))
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        if(0 == strcmp(instanceExtensionNames_array[le], VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
        {
            win32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
    }

    // Step 06: As not needed henceforth, free the local string and the array
    // Free each string
    for(uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        free(instanceExtensionNames_array[le]);
        instanceExtensionNames_array[le] = NULL;
    }
    //  Free the array
    free(instanceExtensionNames_array);
    instanceExtensionNames_array = NULL;

    //  Step 07: Print whether out Vulkan Driver supports our required extension names of not.
    if(VK_FALSE == vulkanSurfaceExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;  //  Return hardcoded failure
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_SURFACE_EXTENSION_NAME found successfully\n");
    }

    if(VK_FALSE == win32SurfaceExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;  //  Return hardcoded failure
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_WIN32_SURFACE_EXTENSION_NAME found successfully\n");
    }

    //  Step 08: Print only enabled extension names
    for(uint32_t le = 0; le < enabledInstanceExtensionCount; ++le)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  Enabled vulkan instance extension name = %s\n", enabledInstanceExtensionNames_array[le]);
    }


    fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  function completed\n");
    return(vkResult);
}
