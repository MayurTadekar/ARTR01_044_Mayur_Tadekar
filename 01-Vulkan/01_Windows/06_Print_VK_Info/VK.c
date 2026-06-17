/**************************************************
 * S.G.M.P
 * @file: VK.c
 * @brief: Step 06 Print Vulkan Info
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @Copyright ©2020-2025 MarshallingVoid
 * @date: 08 June 2025 (Sunday) 01:12:12 PM
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
#define     NEWLINEINFILE   (fprintf(gpLogFile, "\n"))

// GLOBAL VARIABLES
HWND    ghwnd;
BOOL    bFullscreen = FALSE;
BOOL    bActiveWindow = TRUE;
FILE    *gpLogFile = NULL;
DWORD   dwStyle;
BOOL    bEscapeKeyPressed = FALSE;
WINDOWPLACEMENT wpPrev;

const char* gpszAppName = "ARTR";

//  Extension Needed
//  VK_KHR_SURFACE_EXTENSION_NAME
//  VK_KHR_WIN32_SURFACE_EXTENSION_NAME

//  Step 02 - Instance Extension Related Variables
uint32_t enabledInstanceExtensionCount = 0;
const char* enabledInstanceExtensionNames_array[2]; 

//  Step 03 - Vulkan Instance 
VkInstance vkInstance = VK_NULL_HANDLE;

//  Step 04 - Vulkan Presentation Surface Object
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

// Step 05 - Vulkan Physical Device Related Global Variables
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

// Step 06 - Print Vulkan Info
uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

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
        fprintf(gpLogFile, "WndProc()   =>  program started successfully\n");
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
        fprintf(gpLogFile, "WinMain()   =>  Initialize() failed\n");
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }
    else
    {
        fprintf(gpLogFile, "WinMain()   =>  Initialize() successful\n");
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
    VkResult    CreateVulkanInstance(void);
    VkResult    GetSupportedSurface(void);
    VkResult    GetPhysicalDevice(void);
    VkResult    PrintVkInfo(void);

    // Variable Declaration
    VkResult    vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "Initialize()    =>  function started\n");


    //  Get Vulkan Instance
    vkResult = CreateVulkanInstance();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()   =>  CreateVulkanInstance() failed\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()   =>  CreateVulkanInstance() successful\n");
    }
    
    
    //  Create Vulkan Presentation Surface
    vkResult = GetSupportedSurface();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()   =>  GetSupportedSurface() failed\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()   =>  GetSupportedSurface() successful\n");
    }

    //  Step 05 - Physical Device - Enumerate & Select Physical Device and its Queue Family Index
    vkResult = GetPhysicalDevice();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  GetPhysicalDevice() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  GetPhysicalDevice() successful\n");
    }

    //  Step 06 - Print VK Info
    vkResult = PrintVkInfo();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  PrintVkInfo() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  PrintVkInfo() successful\n");
    }

    fprintf(gpLogFile, "Initialize()    =>  function completed\n");
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
    fprintf(gpLogFile, "Uninitialize()  =>  function started\n");

    // No Need to Destroy Selected Physical Device

    // Destroy Surface
    if(vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);    //  the vkDestroySurfaceKHR() is generic and not platform specific
        vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySurfaceKHR() successful\n");
    }

    // Destroy Vulkan Instance
    if(vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyInstance() successful\n");
    }

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
        fprintf(gpLogFile, "Uninitialize()  =>  program terminated successfully\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
 
    fprintf(gpLogFile, "Uninitialize()  =>  function completed\n");
}

/*****************************************************************************************************************************\
                                        VULKAN RELATED FUNCTIONS
\*****************************************************************************************************************************/


// STEP 02
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
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() failed - first call\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() successful - first call\n");
    }

    //  Step 02: Allocate & Fill struct VkExtensionProperties array corresponding to above count;
    vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    vkResult = vkEnumerateInstanceExtensionProperties(  NULL,
                                                        &instanceExtensionCount,
                                                        vkExtensionProperties_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() failed - second call\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() successful - second call\n");
    }

    //  Step 03: Fill & Display a local string array of extension names obtained from VkExtensionProperties array
    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);

    //  FOR THE SAKE OF BREVITY

    //  Allocate memory for each string member
    for(uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        instanceExtensionNames_array[le] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
    
        memcpy(instanceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  vulkan extension name = %s\n", instanceExtensionNames_array[le]);
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

//  STEP 03
VkResult CreateVulkanInstance(void)
{
    // Function Declaration
    VkResult FillInstanceExtensionNames(void);

    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;
    VkApplicationInfo   vkApplicationInfo;
    VkInstanceCreateInfo vkInstanceCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateVulkanInstance()    =>  function started\n");

   
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

    // Step 01 Fill and Initialize required extension names & count global variables
    vkResult = FillInstanceExtensionNames();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  FillInstanceExtensionNames() failed\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  FillInstanceExtensionNames() successful\n");
    }

    // Step 02 Initialize struct VkApplicationInfo
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

    // Step 03  Initialize struct VkInstanceCreateInfo by using information from Step 01 and Step 02

    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    // Step 04: Call vkCreateInstance() to get VkInstance in global variable & do error checking
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
    if(VK_ERROR_INCOMPATIBLE_DRIVER == vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> incompatible driver(%d)\n", vkResult);
        return(vkResult);
    }
    else if(VK_ERROR_EXTENSION_NOT_PRESENT == vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> extension not present(%d)\n", vkResult);
        return(vkResult);
    }
    else if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> unknown reason(%d)\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() successful\n");
    }


    fprintf(gpLogFile, "CreateVulkanInstance()    =>  function completed\n");
    

    return(vkResult);
}

//  STEP 04
VkResult GetSupportedSurface(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkWin32SurfaceCreateInfoKHR  vkWin32SurfaceCreateInfoKHR;

    // Code
    fprintf(gpLogFile, "GetSupportedSurface()    =>  function started\n");

    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetSupportedSurface()   =>  vkCreateWin32SurfaceKHR() failed\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetSupportedSurface()   =>  vkCreateWin32SurfaceKHR() successful\n");
    }

    fprintf(gpLogFile, "GetSupportedSurface()    =>  function completed\n");
    
    
    return(vkResult);
}


//  Step 05
VkResult GetPhysicalDevice(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkBool32 bFound = VK_FALSE;
    VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
    VkBool32 *isQueueSurfaceSupported_array = NULL;
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;

    // Code
    fprintf(gpLogFile, "GetPhysicalDevice() =>  function started\n");

    // Step 02: call vkEnumeratePhysicalDevices() function to get Physical device count.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() failed - first call - %d\n", vkResult);
        return(vkResult);
    }
    else if(0 == physicalDeviceCount)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()     =>  physicalDeviceCount is 0\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() successful - first call\n");
    }

    // Step 03: Allocate VkPhysicalDevice array according to above count
    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    // Step 04: Call vkEnumeratePhysicalDevice() to fill above array
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() failed - second call - %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() successful - second call\n");
    }

    // Step 05: Start a loop using physicalDeviceCount & physicalDevice array
    for(uint32_t ole = 0; ole < physicalDeviceCount; ++ole)
    {
        // SubStep a: Declare a local variable to hold queue count
        uint32_t queueCount = 0;

        // SubStep b: Call vkGetPhysicalDeviceQueueFamilyProperties() to initialize above queue count variable.
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[ole], &queueCount, NULL);    // If physical device present then it must have atleast 1 queue family

        // SubStep c: Allocate VkQueueFamilyProperties array according to above count
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);

        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

        // SubStep d: call vkGetPhysicalDeviceQueueFamilyProperties
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[ole], &queueCount, vkQueueFamilyProperties_array);

        // SubStep e: Declare VkBool32 array & allocate it using above queue count.
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);

        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

        // SubStep f: Start a nested loop & fill above VkBool32 type array by calling vkGetPhysicalDeviceSurfaceSupportKHR()
        for(uint32_t ile = 0; ile < queueCount; ++ile)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[ole], ile, vkSurfaceKHR, &isQueueSurfaceSupported_array[ile]);
        }

        // SubStep g: Start another nested loop & check whether physical device in its array with its queue family has graphics bit or not.
        for(uint32_t ile = 0; ile < queueCount; ++ile)
        {
            if(vkQueueFamilyProperties_array[ile].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if(VK_TRUE == isQueueSurfaceSupported_array[ile])
                {
                    vkPhysicalDevice_selected = vkPhysicalDevice_array[ole];
                    graphicsQueueFamilyIndex_selected = ile;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }

        //  SubStep h: free the queue-family array and VkBool32 array
        if(isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(gpLogFile, "GetPhysicalDevice() =>  isQueueSurfaceSupported_array successfully freed\n");
        }

        if(vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gpLogFile, "GetPhysicalDevice() =>  vkQueueFamilyProperties_array successful freed\n");
        }   

        // SubStep i:
        if(VK_TRUE == bFound)
        {
            break;
        }
    }

    // Step 06: Do error checking according to value of bfound 
    if(VK_TRUE == bFound)
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  successful in getting Physical Device with graphics enabled\n");
    }
    else
    {
        if(vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
            fprintf(gpLogFile, "GetPhysicalDevice() =>  vkPhysicalDevice_array successfully freed\n");
        }
        
        fprintf(gpLogFile, "GetPhysicalDevice() => failed to select PhysicalDevice \n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);        
    }

    // Step 07: memset the global vkPhysicalDeviceMemoryProperties
    memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    // Step 08: Initialize above structure by using vkGetPhysicalDeviceMemoryProperties()   
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);  //  This is the Good Place to do this.

    // Step 09: Declare a local structure VkPhysicalDeviceFeatures, memset it, & initialize it by calling vkGetPhysicalDeviceFeatures()
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

    // Step 10: By using tessellationShader member of above structure, check if selected devices supportsd TessellationShader
    if(vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device support Tessellation Shader\n");
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device does not support Tessellation Shader\n");
    }

    // Step 11: By using geometryShader member of above structure, check if selected devices supportsd GeometryShader;
    if(vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device support Geometry Shader\n");
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device does not support Geometry Shader\n");
    }

    fprintf(gpLogFile, "GetPhysicalDevice()    =>  function completed\n");
    return(vkResult);
}

VkResult PrintVkInfo(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "PrintVkInfo()    =>  function started\n");

    fprintf(gpLogFile, "**************************************************************************\n");
    fprintf(gpLogFile, "                     Vulkan Info\n");
    fprintf(gpLogFile, "**************************************************************************\n");

    for(uint32_t le = 0; le < physicalDeviceCount; ++le)
    {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[le], &vkPhysicalDeviceProperties);

        uint32_t majorVersion = VK_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
    
        // API Version
        fprintf(gpLogFile, "API Version = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

        // Device Name
        fprintf(gpLogFile, "Device Name = %s\n", vkPhysicalDeviceProperties.deviceName);

        // Device Type
        switch(vkPhysicalDeviceProperties.deviceType)
        {
        
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                fprintf(gpLogFile, "Device Type = Integrated GPU(iGPU)\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                fprintf(gpLogFile, "Device Type = Discrete GPU(dGPU)\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                fprintf(gpLogFile, "Device Type = Virtual GPU(vGPU)\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                fprintf(gpLogFile, "Device Type = CPU\n");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                fprintf(gpLogFile, "Device Type = OTHER\n");
                break;            

            default:
                fprintf(gpLogFile, "Device Type = UNKNOWN\n");
                break;
        }

        // Vendor ID
        fprintf(gpLogFile, "Vendor ID = 0x%04X\n", vkPhysicalDeviceProperties.vendorID);

        // Device ID
        fprintf(gpLogFile, "Device ID = 0x%04X\n", vkPhysicalDeviceProperties.deviceID);

    }

    // Free Physical Device
    if(vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(gpLogFile, "PrintVkInfo() =>  vkPhysicalDevice_array successfully freed\n");
    }

    NEWLINEINFILE;

    fprintf(gpLogFile, "PrintVkInfo()    =>  function completed\n");

    return(vkResult);
}
