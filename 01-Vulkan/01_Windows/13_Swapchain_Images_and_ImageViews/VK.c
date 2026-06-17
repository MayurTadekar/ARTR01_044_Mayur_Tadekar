/**************************************************
 * S.G.M.P
 * @file: VK.c
 * @brief: Step 13 Swapchain Images and ImageViews
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @Copyright ©2020-2025 MarshallingVoid
 * @date: 10 June 2025 (Tuesday) 03:11:23 AM
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

//  Step 02 - Instance Extension Related Variables
//  VK_KHR_SURFACE_EXTENSION_NAME
//  VK_KHR_WIN32_SURFACE_EXTENSION_NAME
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

// Step 07 - Device Extension
// VK_KHR_SWAPCHAIN_EXTENSION_NAME
uint32_t enabledDeviceExtensionCount = 0;
const char* enabledDeviceExtensionNames_array[1];

// Step 08 - Vulkan Device
VkDevice vkDevice = VK_NULL_HANDLE; //  Vulkan Device (Logical Device)

// Step 09 - Vulkan Device Queue
VkQueue vkQueue = VK_NULL_HANDLE;

// Step 10 - Color Format and Color Space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Step 11 - Presentation Mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

// Step 12 - Swapchain
int     winWidth = WIN_WIDTH;
int     winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_Swapchain;

// Step 13 - Swapchain Images and ImageViews
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

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
    VkResult    CreateVulkanDevice(void);
    void        GetDeviceQueue(void);
    VkResult    CreateSwapchain(VkBool32);
    VkResult    CreateImagesAndImageViews(void);

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

    // Step 08 - Vulkan Device
    vkResult = CreateVulkanDevice();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVulkanDevice() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVulkanDevice() successful\n");
    }

    GetDeviceQueue();

    // Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateSwapchain() failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateSwapchain() successful\n");
    }

    // Swapchain Images and ImageViews
    vkResult = CreateImagesAndImageViews();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateImagesAndImageViews() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateImagesAndImageViews() successful\n");
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

    if(vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpLogFile, "Uninitialize()  =>  vkDeviceWaitIdle() is done\n");
    }
    
    // Free swapchain image views
    for(uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[le], NULL);
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImageView() successful for %d\n", le);
    }

    if(swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  swapchainImageView_array freed successfully\n");
    }

    // Free swapchain images
    // for(uint32_t le = 0; le < swapchainImageCount; ++le)
    // {
    //     vkDestroyImage(vkDevice, swapchainImage_array[le], NULL);
    //     fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImage() successful for %d\n", le);
    // }

    if(swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  swapchainImage_array freed successfully\n");
    }

    // Destroy Swapchain
    if(vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySwapchainKHR() successful\n");
    }

    // No need to Destroy/Uninitialize Device Queue

    // Destroy Vulkan Device
    if(vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroy() successful \n");
    }

    // No Need to Destroy Selected Physical Device as it gets destroy with the Logical Device

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

        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  vulkan instance extension name = %s\n", instanceExtensionNames_array[le]);
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

    NEWLINEINFILE;

    // Free Physical Device
    if(vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(gpLogFile, "PrintVkInfo() =>  vkPhysicalDevice_array successfully freed\n");
    }

    fprintf(gpLogFile, "PrintVkInfo()    =>  function completed\n");

    return(vkResult);
}

// STEP 07
VkResult FillDeviceExtensionNames(void)
{
    // Variable Declaration
    VkResult    vkResult = VK_SUCCESS;
    uint32_t    deviceExtensionCount = 0;
    VkExtensionProperties   *vkExtensionProperties_array = NULL;
    char        **deviceExtensionNames_array = NULL;
    VkBool32    vulkanSwapchainExtensionFound = VK_FALSE;

    // Code
    fprintf(gpLogFile, "FillDeviceExtensionNames()  =>  function started\n");


    // First Vulkan API
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,                   
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    NULL);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() failed - first call\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() successful - first call\n");
    }

    vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,                   
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() failed - second call\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() successful - second call\n");
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  number of found device extension count = %u\n", deviceExtensionCount);
    }

    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);

    //  FOR THE SAKE OF BREVITY

    //  Allocate memory for each string member
    for(uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        deviceExtensionNames_array[le] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
    
        memcpy(deviceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  vulkan device extension name = %s\n", deviceExtensionNames_array[le]);
    }

    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    for(uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        if(0 == strcmp(deviceExtensionNames_array[le], VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }

    // Free each string
    for(uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        free(deviceExtensionNames_array[le]);
        deviceExtensionNames_array[le] = NULL;
    }
    //  Free the array
    free(deviceExtensionNames_array);
    deviceExtensionNames_array = NULL;

    if(VK_FALSE == vulkanSwapchainExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;  //  Return hardcoded failure
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  VK_KHR_SWAPCHAIN_EXTENSION_NAME found successfully\n");
    }

    for(uint32_t le = 0; le < enabledDeviceExtensionCount; ++le)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  Enabled vulkan device extension name = %s\n", enabledDeviceExtensionNames_array[le]);
    }

    fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  function completed\n");

    return(vkResult);
}

// Step 08
VkResult CreateVulkanDevice(void)
{
    // Function Declaration
    VkResult    FillDeviceExtensionNames(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkDeviceCreateInfo vkDeviceCreateInfo;

    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;        //  NEW CODE ADDITION
    float queuePriorities[1];

    // Code
    fprintf(gpLogFile, "CreateVulkanDevice()  =>  function started\n");

    //  Step 07 - Device Extensions

    // Step 01 & Step 02
    vkResult = FillDeviceExtensionNames();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  FillDeviceExtensionNames() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  FillDeviceExtensionNames() successful\n");
    }

    //  Step 03
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));    //  NEW CODE ADDITION

    queuePriorities[0] = 1.0f;

    //  NEW CODE ADDITION
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    //  Step 04
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0;           //  THIS IS DEPRECATED, KEPT HERE BECAUSE SIR SAID SO
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;      //  THIS IS DEPRECATED, KEPT HERE BECAUSE SIR SAID SO
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    // Step 05
    vkResult = vkCreateDevice(vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  vkCreateDevice() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  vkCreateDevice() successful\n");
    }

    fprintf(gpLogFile, "CreateVulkanDevice()  =>  function completed\n");
    return(vkResult);
}

void GetDeviceQueue(void)
{
    // Code
    fprintf(gpLogFile, "GetDeviceQueue()  =>  function started\n");

    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

    if(VK_NULL_HANDLE == vkQueue)
    {
        fprintf(gpLogFile, "GetDeviceQueue()  =>  vkGetDeviceQueue() resulted null for vkQueue\n");
        return;
    }
    else
    {
        fprintf(gpLogFile, "GetDeviceQueue()  =>  vkGetDeviceQueue() successful\n");
    }

    fprintf(gpLogFile, "GetDeviceQueue()  =>  function completed\n");
}

VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t formatCount = 0;
    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array;

    // Code
    fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()  =>  function started\n");

    // Get the count of supported surface color formats 
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed first call with %d\n", vkResult);
        return(vkResult);
    }
    else if(0 == formatCount)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed first call %d that resulted in 0 formatCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() first call successful with %d format counts\n", formatCount);
    }

    vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    //  Filling the array 
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed second call with %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() second call\n");
    }

    // MY LOOP TO JUST LOG AND SEE PRESENT MODE OF MY PHYSICAL DEVICE
    // for(uint32_t le = 0; le < formatCount; ++le)
    // {
    //     fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  found %d surface color format & %d color space\n", vkSurfaceFormatKHR_array[le].format, vkSurfaceFormatKHR_array[le].colorSpace);
    // }  

    //  Decide the surface color format first
    if(1 == formatCount && VK_FORMAT_UNDEFINED == vkSurfaceFormatKHR_array[0].format)
    {
        vkFormat_color = VK_FORMAT_B8G8R8_UNORM;
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  selected VK_FORMAT_B8G8R8_UNORM for surfaceColorFormat\n");
    }
    else
    {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  selected %d for surfaceColorFormat\n", vkFormat_color);
    }

    // Decide the color space
    vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
    fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  selected %d for color space\n", vkColorSpaceKHR);

    if(vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace() =>  vkSurfaceFormatKHR_array successfully freed\n");
    }

    fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()  =>  function completed\n");
    return(vkResult);
}

VkResult GetPhysicalDevicePresentMode(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t presentModeCount = 0;
    VkPresentModeKHR *vkPresentModeKHR_array = NULL;

    // Code
    fprintf(gpLogFile, "GetPhysicalDevicePresentMode()  =>  function started\n");

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed first call with %d\n", vkResult);
        return(vkResult);
    }
    else if(0 == presentModeCount)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed first call %d that resulted in 0 presentModeCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() first call successful with %d presentModeCount\n", presentModeCount);
    }

    vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * presentModeCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed second call with %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() second call successful with %d presentModeCount\n", presentModeCount);
    }

    // MY LOOP TO JUST LOG AND SEE PRESENT MODE OF MY PHYSICAL DEVICE
    for(uint32_t le = 0; le < presentModeCount; ++le)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  found %d present mode\n", vkPresentModeKHR_array[le]);
    }

    // Decide the Presentation Mode
    for(uint32_t le = 0; le < presentModeCount; ++le)
    {
        if( VK_PRESENT_MODE_MAILBOX_KHR == vkPresentModeKHR_array[le])
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    if(VK_PRESENT_MODE_MAILBOX_KHR != vkPresentModeKHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  selected VK_PRESENT_MODE_FIFO_KHR for present mode\n");
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  selected VK_PRESENT_MODE_MAILBOX_KHR for present mode\n");
    }

    if(vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode() =>  vkPresentModeKHR_array successfully freed\n");
    }        

    fprintf(gpLogFile, "GetPhysicalDevicePresentMode()  =>  function completed\n");
    return(vkResult);
}

VkResult CreateSwapchain(VkBool32 vSync)
{
    // Function Declarations
    VkResult    GetPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult    GetPhysicalDevicePresentMode(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS; 
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    uint32_t    testingNROfSwapchainImages = 0;
    uint32_t    desiredNROfSwapchainImages = 0;
    VkImageUsageFlags vkImageUsageFlags;
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR; 
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    
    // Code
    fprintf(gpLogFile, "CreateSwapchain()  =>  function started\n");

    // ColorFormat and ColorSpace
    vkResult = GetPhysicalDeviceSurfaceFormatAndColorSpace();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDeviceSurfaceFormatAndColorSpace() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDeviceSurfaceFormatAndColorSpace() successful\n");
    }

    // Get Physical Device Surface Capabilities
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceKHR));

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkGetPhysicalDeviceSurfaceCapabilitiesKHR() successful\n");
    }

    fprintf(gpLogFile, "CreateSwapchain()    =>  minImageCount %d\n", vkSurfaceCapabilitiesKHR.minImageCount);
    fprintf(gpLogFile, "CreateSwapchain()    =>  maxImageCount %d\n", vkSurfaceCapabilitiesKHR.maxImageCount);

    // findout desired swapchain image count
    testingNROfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    if(vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNROfSwapchainImages)
    {
        desiredNROfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNROfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }

    // choose size of swapchain image
    memset((void*)&vkExtent2D_Swapchain, 0, sizeof(VkExtent2D));

    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_Swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_Swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        fprintf(gpLogFile, "CreateSwapchain()    =>  swapchain image width x height = %d x %d\n", vkExtent2D_Swapchain.width, vkExtent2D_Swapchain.height);
    }
    else
    {
        // if surface size is already defined then swapchain image size must match withit
        VkExtent2D vkExtent2D;

        memset((void*)&vkExtent2D, 0, sizeof(vkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.minImageExtent.width %d\n", vkSurfaceCapabilitiesKHR.minImageExtent.width);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.maxImageExtent.width %d\n", vkSurfaceCapabilitiesKHR.maxImageExtent.width);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkExtent2D.width %d\n", vkExtent2D.width);
        fprintf(gpLogFile, "CreateSwapchain()    =>  selected width = %d\n", max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)));

        vkExtent2D_Swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));

        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.minImageExtent.height %d\n", vkSurfaceCapabilitiesKHR.minImageExtent.height);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.maxImageExtent.height %d\n", vkSurfaceCapabilitiesKHR.maxImageExtent.height);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkExtent2D.height %d\n", vkExtent2D.height);
        fprintf(gpLogFile, "CreateSwapchain()    =>  selected height = %d\n", max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)));

        vkExtent2D_Swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
    }

    // Set swapchain image usage flag
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VIMP must be given                 
    // VK_IMAGE_USAGE_TRANSFER_SRC_BIT : this is optional but is given everytime for texture, FBO, compute
    vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // Whether to consider Pre-transform/Flipping or not
    if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  in if vkSurfaceTransformFlagBitsKHR = %d\n", vkSurfaceTransformFlagBitsKHR);
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  in else vkSurfaceTransformFlagBitsKHR = %d\n", vkSurfaceTransformFlagBitsKHR);
    }

    // Present Mode
    vkResult = GetPhysicalDevicePresentMode();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDevicePresentMode() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDevicePresentMode() successful\n");
    }

    // Initialize VkSwapchainCreateInfoKHR struct
    memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNROfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_Swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_Swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkCreateSwapchainKHR() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkCreateSwapchainKHR() successful\n");
    }

    fprintf(gpLogFile, "CreateSwapchain()  =>  function completed\n");
    return(vkResult);
}

VkResult CreateImagesAndImageViews(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkImageViewCreateInfo vkImageViewCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateImagesAndImageViews()  =>  function started\n");

    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed first call with %d\n", vkResult);
        return(vkResult);
    }
    else if(0 == swapchainImageCount)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed first call %d that resulted in 0 swapchainImageCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() first call successful with %d swapchainImageCount\n", swapchainImageCount);
    }

    swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed second call with %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() second successful with %d swapchainImageCount\n", swapchainImageCount);
    }

    swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC

    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // Fill image view array using above struct
    for(uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[le];

        vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[le]);
        if(VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImageViews() failed for iteration %d. (%d)\n", le, vkResult);
            return(vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImageViews() successful for iteration %d \n", le);
        }
    }

    fprintf(gpLogFile, "CreateImagesAndImageViews()  =>  function completed\n");
    return(vkResult);
}
