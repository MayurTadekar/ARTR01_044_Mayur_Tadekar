/**************************************************
 * S.G.M.P
 * @file: VK.c
 * @brief: Vulkan Sine Wave
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: Sunday 2026-08-09 12:27:32 PM
 **************************************************/

// HEADERS
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "VK.h"

// Vulkan Headers
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h> //  We must tell the header the platform we are on

// GLM related macros and headers files
// GLM follows column major order just like OpenGL
// BUT unlike OpenGL, GLM's matrix arrays are 4x4 2D arrays not 1D arrays of 16 elements. VVVIMP
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_0_TO_1
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// OpenGL Headers
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/opencl.h>
#include <CL/cl_ext.h>  // for cl_mem_device_handle_list_khr & cl_khr_device_handle_list_khr_end

// OpenCL Import Lib
#pragma comment(lib, "opencl.lib")

// MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define NEWLINEINFILE (fprintf(gpLogFile, "\n"))

#define WINDOW_TITLE "MST: Vulkan-OpenCL Interop - Sine Wave"

// GLOBAL VARIABLES
HWND ghwnd;
BOOL bFullscreen = FALSE;
BOOL bActiveWindow = TRUE;
FILE *gpLogFile = NULL;
DWORD dwStyle;
BOOL bEscapeKeyPressed = FALSE;
WINDOWPLACEMENT wpPrev;

const char *gpszAppName = "ARTR";

//  Step 02 - Instance Extension Related Variables
//  Extension Needed
//  VK_KHR_SURFACE_EXTENSION_NAME
//  VK_KHR_WIN32_SURFACE_EXTENSION_NAME
//  VK_EXT_DEBUG_REPORT_EXTENSION_NAME
uint32_t enabledInstanceExtensionCount = 0;
const char *enabledInstanceExtensionNames_array[3]; // 2 cha 3 kela

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
const char *enabledDeviceExtensionNames_array[2];

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
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

// Step 13 - Swapchain Images and ImageView
// For Color Images
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

// For Depth Image
VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
VkImageView vkImageView_depth = VK_NULL_HANDLE;

// Step 14 - Command Pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

// Step 15 - Command Buffer
// VkCommandBuffer *vkCommandBuffer_array = NULL;

// Step 16 - RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

// Step 17 - Frame Buffers
VkFramebuffer *vkFramebuffer_array = NULL;

// Step 18 - Fences and Semaphores
VkSemaphore vkSemaphore_backbuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_rendercomplete = VK_NULL_HANDLE;
VkFence *vkFence_array = NULL;

// Step 19 - Build Command Buffer
VkClearColorValue vkClearColorValue;
VkClearDepthStencilValue vkClearDepthStencilValue;

// Step 20 - Render
BOOL bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX; //  or UINT_MAX

// Step 21 - Validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char *enabledValidationLayerName_array[1]; //  for VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

// Step 22 - Vertex Buffer
typedef struct
{

    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} VertexData;

// Uniform related Buffers
typedef struct
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    glm::vec4 color;
} MyUniformData;

typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} UniformData;

UniformData uniformData;

// Step 23 - Shaders
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

// Step 24 - DescriptorSetLayout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

// Step 25 - Pipeline Layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

// Descriptor Pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

// Descriptor Set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

// Step 26 Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

// Sine Wave Related

unsigned int meshWidth = 1024;
unsigned int meshHeight = 1024;

// float pos_1024_graphics[1024][1024][4];
// VertexData vertex_positon_1024x1024_graphics;
// VkCommandBuffer* vkCommandBuffer_for_1024x1024_graphics_array = null;

// float pos_1024_graphics[1024][1024][4];
// VertexData vertex_positon_1024x1024_graphics;
// VkCommandBuffer* vkCommandBuffer_for_1024x1024_graphics_array = null;

// float pos_1024_graphics[1024][1024][4];
// VertexData vertex_positon_1024x1024_graphics;
// VkCommandBuffer* vkCommandBuffer_for_1024x1024_graphics_array = null;

// float pos_512_graphics[512][512][4];
// VertexData vertex_positon_512x512_graphics;
// VkCommandBuffer* vkCommandBuffer_for_512x512_graphics_array = null;

float pos_1024_graphics[1024][1024][4];
VertexData vertex_positon_1024x1024_graphics;
VkCommandBuffer *vkCommandBuffer_for_1024x1024_graphics_array = NULL;
BOOL bMesh1024Chosen = TRUE;
char colorFromKey = 'O';
float animationTime = 0.0f;

// OpenCL Related Variables
cl_int oclResult;
cl_platform_id oclPlatformID;
cl_device_id oclDeviceID;
cl_context oclContext;
cl_command_queue oclCommandQueue;
cl_program oclProgram;
cl_kernel oclKernel;
cl_mem posOpenCL = NULL;
VkExternalMemoryHandleTypeFlagBits vkExternalMemoryHandleTypeFlagBits;
VertexData vertexDataExternal;
BOOL bOnGpu = FALSE;

// FUNCTION SIGNATURE
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// FUNCTIONS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{

    // FUNCTION SIGNATURE
    void Uninitialize(void);
    void Update(void);
    VkResult Display(void);
    VkResult Initialize(void);

    // VARIABLES
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[255];
    WNDCLASSEX wndcls;
    RECT rcDesktop;
    BOOL bDone = FALSE;
    VkResult vkResult = VK_SUCCESS;

    // CODE
    gpLogFile = fopen("VK_log.txt", "w");
    if (0 == gpLogFile)
    {
        MessageBox(NULL, TEXT("Log File Cannot Be opened"), TEXT("ERROR"), MB_ICONERROR);
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "WndProc()   =>  program started successfully\n");
        // LOG_ENTRY("WinMain", "program started successfully");
        // LOG_START("WinMain()");
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

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                          szAppName,
                          TEXT(WINDOW_TITLE),
                          WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          rcDesktop.right / 2 - WIN_WIDTH / 2,
                          rcDesktop.bottom / 2 - WIN_HEIGHT / 2,
                          WIN_WIDTH,
                          WIN_HEIGHT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ghwnd = hwnd;

    vkResult = Initialize();

    if (VK_SUCCESS != vkResult)
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

    while (FALSE == bDone)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
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

                if (TRUE == bEscapeKeyPressed)
                {
                    bDone = TRUE;
                }

                Update();
                vkResult = Display();
                if (VK_FALSE != vkResult && VK_SUCCESS != vkResult)
                {
                    fprintf(gpLogFile, "WinMain()   =>  call to display failed\n");
                    bDone = TRUE;
                }
            }
        }
    }

    Uninitialize();
    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

    // FUNCTION SIGNATURE
    void Uninitialize(void);
    void ToggleFullscreen(void);
    VkResult Resize(int, int);

    // CODE
    switch (iMsg)
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
        switch (LOWORD(wParam))
        {
        case 0x1B: //  VK_ESCAPEKEY
            bEscapeKeyPressed = TRUE;
            break;

        case 52: // 4
            bMesh1024Chosen = TRUE;
            break;
        }
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case 'f':
        case 'F':
            ToggleFullscreen();
            break;

        case 'r':
        case 'R':
            colorFromKey = 'R';
            break;

        case 'g':
        case 'G':
            colorFromKey = 'G';
            break;

        case 'b':
        case 'B':
            colorFromKey = 'B';
            break;

        case 'c':
        case 'C':
            colorFromKey = 'C';
            break;

        case 'm':
        case 'M':
            colorFromKey = 'M';
            break;

        case 'y':
        case 'Y':
            colorFromKey = 'Y';
            break;

        case 'w':
        case 'W':
            colorFromKey = 'W';
            break;

        case 'k':
        case 'K':
            colorFromKey = 'K';
            break;

        case 'o':
        case 'O':
            colorFromKey = 'O';
            break;

        case 'p':
        case 'P':
            bOnGpu = FALSE;
            break;

        case 'h':
        case 'H':
            bOnGpu = TRUE;
            break;
        }
        break;
    }
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{

    // VARIABLES
    MONITORINFO mi;

    memset((void *)&mi, 0, sizeof(MONITORINFO));
    mi.cbSize = sizeof(MONITORINFO);

    // CODE
    if (!bFullscreen)
    {

        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (WS_OVERLAPPEDWINDOW & dwStyle)
        {

            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & (~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd,
                             HWND_TOP,
                             mi.rcMonitor.left,
                             mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left,
                             mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOZORDER | SWP_FRAMECHANGED);
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
    VkResult CreateVulkanInstance(void);
    VkResult GetSupportedSurface(void);
    VkResult GetPhysicalDevice(void);
    VkResult PrintVkInfo(void);
    VkResult CreateVulkanDevice(void);
    void GetDeviceQueue(void);
    VkResult CreateSwapchain(VkBool32);
    VkResult CreateImagesAndImageViews(void);
    VkResult CreateCommandPool(void);
    // VkResult CreateCommandBuffers(void);
    VkResult CreateCommandBuffers(VkCommandBuffer **);
    void InitializeSineWaveArrays(int, int);
    // VkResult CreateVertexBuffer(void);
    VkResult CreateVertexBuffer(unsigned int, unsigned int, VertexData *);
    VkResult CreateUniformBuffer(void);
    VkResult CreateShaders(void);
    VkResult CreateDescriptorSetLayout(void);
    VkResult CreatePipelineLayout(void);
    VkResult CreateDescriptorPool(void);
    VkResult CreateDescriptorSet(void);
    VkResult CreateRenderPass(void);
    VkResult CreatePipeline(void);
    VkResult CreateFrameBuffers(void);
    VkResult CreateSemaphores(void);
    VkResult CreateFences(void);
    VkResult BuildCommandBuffers(void);

    // OpenCL related function declarations
    cl_int initialize_openCL(void);
    VkResult CreateExternalVertexBuffer(unsigned int, unsigned int, VertexData *);

    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (TRUE == bValidation)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()    ||> VALIDATION \n");
    }

    fprintf(gpLogFile, "Initialize()    >>>  function started\n");

    //  Get Vulkan Instance
    vkResult = CreateVulkanInstance();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()   =>  CreateVulkanInstance() failed\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()   =>  CreateVulkanInstance() successful\n");
    }

    //  Create Vulkan Presentation Surface
    vkResult = GetSupportedSurface();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()   =>  GetSupportedSurface() failed\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()   =>  GetSupportedSurface() successful\n");
    }

    //  Step 05 - Physical Device - Enumerate & Select Physical Device and its Queue Family Index
    vkResult = GetPhysicalDevice();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  GetPhysicalDevice() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  GetPhysicalDevice() successful\n");
    }

    //  Step 06 - Print VK Info
    vkResult = PrintVkInfo();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  PrintVkInfo() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  PrintVkInfo() successful\n");
    }

    // Step 08 - Vulkan Device
    vkResult = CreateVulkanDevice();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVulkanDevice() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVulkanDevice() successful\n");
    }

    GetDeviceQueue();

    // OpenCL Initialization
    oclResult = initialize_openCL();
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  initialize_openCL() failed %d\n", oclResult);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  initialize_openCL() successful\n");
    }

    // Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateSwapchain() failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateSwapchain() successful\n");
    }

    // Swapchain Images and ImageViews
    vkResult = CreateImagesAndImageViews();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateImagesAndImageViews() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateImagesAndImageViews() successful\n");
    }

    // Create Command Pool
    vkResult = CreateCommandPool();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateCommandPool() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateCommandPool() successful\n");
    }

    // Create Command Buffers
    vkResult = CreateCommandBuffers(&vkCommandBuffer_for_1024x1024_graphics_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateCommandBuffers() failed %d for 1024 x 1024\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateCommandBuffers() successful for 1024 x 1024\n");
    }

    // Initialize our PositionArray
    InitializeSineWaveArrays(1024, 1024);

    // Create Vertex Buffer
    memset((void *)&vertex_positon_1024x1024_graphics, 0, sizeof(VertexData));
    vkResult = CreateVertexBuffer(1024, 1024, &vertex_positon_1024x1024_graphics);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVertexBuffer() failed %d for 1024 x 1024\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVertexBuffer() successful for 1024 x 1024\n");
    }

    // Create External Vertex Buffer
    memset((void*)&vertexDataExternal, 0, sizeof(VertexData));
    vkResult = CreateExternalVertexBuffer(1024, 1024, &vertexDataExternal);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateExternalVertexBuffer() failed %d for 1024 x 1024\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateExternalVertexBuffer() successful for 1024 x 1024\n");
    }

    // Create Uniform Buffer
    vkResult = CreateUniformBuffer();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateUniformBuffer() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateUniformBuffer() successful\n");
    }

    // Create Shader
    vkResult = CreateShaders();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateShaders() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateShaders() successful\n");
    }

    // Create Descriptor SetLayout
    vkResult = CreateDescriptorSetLayout();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateDescriptorSetLayout() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateDescriptorSetLayout() successful\n");
    }

    // Create Pipeline Layout
    vkResult = CreatePipelineLayout();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreatePipelineLayout() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreatePipelineLayout() successful\n");
    }

    // Create Descriptor Pool
    vkResult = CreateDescriptorPool();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateDescriptorPool() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateDescriptorPool() successful\n");
    }

    // Create Descriptor Set
    vkResult = CreateDescriptorSet();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateDescriptorSet() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateDescriptorSet() successful\n");
    }

    // Create RenderPass
    vkResult = CreateRenderPass();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateRenderPass() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateRenderPass() successful\n");
    }

    // Create Pipeline
    vkResult = CreatePipeline();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreatePipeline() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreatePipeline() successful\n");
    }

    // Create Frame Buffer
    vkResult = CreateFrameBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateFrameBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateFrameBuffers() successful\n");
    }

    // Create Semaphores
    vkResult = CreateSemaphores();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateSemaphores() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateSemaphores() successful\n");
    }

    // Create Semaphores
    vkResult = CreateFences();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateFences() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateFences() successful\n");
    }

    memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));

    // FINALLY GIVING BLUE COLOR - Analogous to glClearColor;
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 0.0f;
    vkClearColorValue.float32[3] = 1.0f;

    memset((void *)&vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));

    // Set Default Clear Depth
    vkClearDepthStencilValue.depth = 1.0f; // float

    // Set Default Stensil Value
    vkClearDepthStencilValue.stencil = 0; // uint32_t;

    // Build Command Buffers
    vkResult = BuildCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  BuildCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  BuildCommandBuffers() successful\n");
    }

    // Initialization is Completed
    bInitialized = TRUE;

    fprintf(gpLogFile, "Initialize()    -->  function completed\n");
    return (vkResult);
}

cl_int initialize_openCL(void)
{
    // Function Declaration
    BOOL doesOpenCLPlatformSupportsRequiredExtensions(cl_platform_id);

    // Check for presence OpenCL Device
    // If Absent, no point in proceeding further
    cl_uint platform_count = 0;
    cl_platform_id *oclPlatformIDs = NULL;
    cl_uint deviceCount = 0;
    cl_device_id *oclDeviceIDs = NULL;
    
    // Get Number of OpenCL Supported Platforms
    oclResult = clGetPlatformIDs(0, NULL, &platform_count);
    if(CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  clGetPlatformIDs() failed %d\n", oclResult);
        return (oclResult);
    }
    else if(0 == platform_count)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  No OpenCL platforms found\n");
        return (CL_PLATFORM_NOT_FOUND_KHR);
    }
    else
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Number of OpenCL platforms found: %u\n", platform_count);
    }

    oclPlatformIDs = (cl_platform_id *)malloc(sizeof(cl_platform_id) * platform_count);
    if (NULL == oclPlatformIDs)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Memory allocation for oclPlatformIDs failed\n");
        return (CL_OUT_OF_HOST_MEMORY);
    }

    // Now Get PlatformIDs in allocated oclPlatformIDs array
    oclResult = clGetPlatformIDs(platform_count, oclPlatformIDs, NULL);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  clGetPlatformIDs() failed %d\n", oclResult);
        return (oclResult);
    }

    int openclPlatformWithGPUDeviceFound = -1;
    // Loop through the opencl platform array to find a platform with a GPU device with required OpenCL extensions
    for (uint32_t le = 0; le < platform_count; le++)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Checking platform %u\n", le);
        
        // Get Number Of Device for the current platform
        oclResult = clGetDeviceIDs(oclPlatformIDs[le], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceCount);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "initialize_openCL()    =>  clGetDeviceIDs() failed for platform %u with error %d\n", le, oclResult);
            continue;
        }
        else if (0 == deviceCount)
        {
            fprintf(gpLogFile, "initialize_openCL()    =>  No GPU devices found for platform %u\n", le);
            continue;
        }

        // So now we have ith platform with one GPU device
        // Now see if this GPU device supports the required OpenCL extensions
        // cl_khr_external_memory
        // cl_khr_device_uuid
        // cl_khr_external_semaphore
        
        if ( FALSE == doesOpenCLPlatformSupportsRequiredExtensions(oclPlatformIDs[le]) )
        {
            fprintf(gpLogFile, "initialize_openCL()    =>  Platform %u does not support required OpenCL extensions\n", le);
            continue;
        }

        // Now we have correct platform with required GPU device and OpenCL extensions
        oclPlatformID = oclPlatformIDs[le];

        // Print selected platforms properties
        size_t infoSize;
        char* oclPlatformInfo = NULL;
        clGetPlatformInfo(oclPlatformID, CL_PLATFORM_NAME, 0, NULL, &infoSize);
        oclPlatformInfo = (char*)malloc(infoSize);
        clGetPlatformInfo(oclPlatformID, CL_PLATFORM_NAME, infoSize, oclPlatformInfo, NULL);
        fprintf(gpLogFile, "initialize_openCL()    =>  Selected platform name: %s\n", oclPlatformInfo);
        free(oclPlatformInfo);
        
        openclPlatformWithGPUDeviceFound = le;
        break;
    }

    if (-1 == openclPlatformWithGPUDeviceFound)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  No suitable OpenCL platform with required GPU device and extensions found\n");
        return(-32);
    }


    // Now allocate memory to hold one or more GPU Devices in found platform
    oclDeviceIDs = (cl_device_id*)malloc(deviceCount * sizeof(cl_device_id));
    if (NULL == oclDeviceIDs)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Failed to allocate memory for OpenCL device IDs\n");
        return(-33);
    }

    // Get the GPU device IDs for the selected platform
    oclResult = clGetDeviceIDs(oclPlatformID, CL_DEVICE_TYPE_GPU, deviceCount, oclDeviceIDs, NULL);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  clGetDeviceIDs() failed for selected platform with error %d\n", oclResult);
        return(-34);
    }

    // Get Vulkan Compatible UUID of Device
    VkPhysicalDeviceIDProperties vkPhysicalDeviceIDProperties;
    memset((void *)&vkPhysicalDeviceIDProperties, 0, sizeof(VkPhysicalDeviceIDProperties));

    vkPhysicalDeviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
    vkPhysicalDeviceIDProperties.pNext = NULL;
    
    VkPhysicalDeviceProperties2 vkPhysicalDeviceProperties2;
    memset((void *)&vkPhysicalDeviceProperties2, 0, sizeof(VkPhysicalDeviceProperties2));
    
    vkPhysicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkPhysicalDeviceProperties2.pNext = &vkPhysicalDeviceIDProperties;

    vkGetPhysicalDeviceProperties2(vkPhysicalDevice_selected, &vkPhysicalDeviceProperties2);

    cl_uchar cl_UUID[CL_UUID_SIZE_KHR];
    int vulkanOpenCLInteropDeviceFound = -1;
    BOOL isEqualUUID = TRUE;
    for (int ole = 0; ole < deviceCount; ++ole)
    {
        // Get OpenCL capable device's UUID from OpenCL itself
        clGetDeviceInfo(oclDeviceIDs[ole], CL_DEVICE_UUID_KHR, CL_UUID_SIZE_KHR, &cl_UUID, NULL);
        
        // Now compare OpenCL device's UUID with Vulkan device's UUID to find interop capable device
        for( uint32_t ile = 0; ile < CL_UUID_SIZE_KHR; ++ile)
        {
            if(cl_UUID[ile] != vkPhysicalDeviceIDProperties.deviceUUID[ile])
            {
                isEqualUUID = FALSE;
                break;
            }
        }
        // Go for next device if available
        if(FALSE == isEqualUUID)
        {
            isEqualUUID = TRUE; // Reset for next device
            continue;
        }

        // otherwise device is found
        oclDeviceID = oclDeviceIDs[ole];
        size_t infoSize;
        char* oclDeviceInfo = NULL;
        clGetDeviceInfo(oclDeviceID, CL_DEVICE_NAME, 0, NULL, &infoSize);
        oclDeviceInfo = (char*)malloc(infoSize * sizeof(char));
        clGetDeviceInfo(oclDeviceID, CL_DEVICE_NAME, infoSize, oclDeviceInfo, NULL);
        fprintf(gpLogFile, "initialize_openCL()    =>  Vulkan-OpenCL Interop GPU Device Name: %s\n", oclDeviceInfo);
        free(oclDeviceInfo);

        vulkanOpenCLInteropDeviceFound = 1;
        break;
    }

    free(oclDeviceIDs);
    oclDeviceIDs = NULL;

    // If no such Vulkan-OpenCL Interop capable device is found
    if (-1 == vulkanOpenCLInteropDeviceFound)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  No Vulkan-OpenCL Interop capable device found\n");
        return(-2); // Standard error code for no Vulkan-OpenCL Interop capable device found
    }

    // Create OpenCL context for the interop device
    oclContext = clCreateContext(NULL, 1, &oclDeviceID, NULL, NULL, &oclResult);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Failed to create OpenCL context for Vulkan-OpenCL Interop device\n");
        return(oclResult); // Standard error code for OpenCL context creation failure
    }

    // Create Command Queue
    oclCommandQueue = clCreateCommandQueueWithProperties(oclContext, oclDeviceID, NULL, &oclResult);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Failed to create OpenCL command queue for Vulkan-OpenCL Interop device\n");
        return(oclResult); // Standard error code for OpenCL command queue creation failure
    }

    // Create OpenCL Program from OpenCL source code
    const char* oclSourceCode = 
    "__kernel void sineWave_Kernel(__global float4* pos, unsigned int mesh_width, unsigned int mesh_height, float animationTime)   "   \
    "{\n" \
        "unsigned int x = get_global_id(0);\n" \
        "unsigned int y = get_global_id(1);\n" \

        "float u = (float)x / (float)mesh_width;\n" \
        "float v = (float)y / (float)mesh_height;\n" \

        "u = u * 2.0f - 1.0f;\n" \
        "v = v * 2.0f - 1.0f;\n" \

        "float frequency = 4.0f;\n" \

        "float w = sin(u * frequency + animationTime ) * cos(v * frequency + animationTime) * 0.5f;\n" \

        "pos[y * mesh_width + x] = (float4)(u, w, v, 1.0f);\n" \
    "}";

    oclProgram = clCreateProgramWithSource(oclContext, 1, (const char**)&oclSourceCode, NULL, &oclResult);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Failed to create OpenCL program from source code\n");
        return(oclResult);
    }

    // Build OpenCL Program
    oclResult = clBuildProgram(oclProgram, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Failed to build OpenCL program\n");
    
        // Get build log size
        size_t logSize;
        clGetProgramBuildInfo(oclProgram, oclDeviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* buildLog = (char*)malloc(logSize);
        clGetProgramBuildInfo(oclProgram, oclDeviceID, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
        fprintf(gpLogFile, "Build Log:\n%s\n", buildLog);
        free(buildLog);
    
        return(oclResult);
    }

    // Create OpenCL Kernel
    oclKernel = clCreateKernel(oclProgram, "sineWave_Kernel", &oclResult);
    if (CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "initialize_openCL()    =>  Failed to create OpenCL kernel\n");
        return(oclResult);
    }

    vkExternalMemoryHandleTypeFlagBits = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    fprintf(gpLogFile, "initialize_openCL()    =>  OpenCL kernel created successfully\n");

    return(CL_SUCCESS);
}

BOOL doesOpenCLPlatformSupportsRequiredExtensions(cl_platform_id oclPlatformID)
{

    // Variable Definition
    size_t extensionSize;
    char* oclPlatformExtensions = NULL;

    // Code

    // List given platform's extensions
    clGetPlatformInfo(oclPlatformID, CL_PLATFORM_EXTENSIONS, 0, NULL, &extensionSize);
    oclPlatformExtensions = (char*)malloc(extensionSize * sizeof(char));
    clGetPlatformInfo(oclPlatformID, CL_PLATFORM_EXTENSIONS, extensionSize, oclPlatformExtensions, NULL);

    char* oclPlatformExtensions_Copy_For_Strtok = NULL;
    oclPlatformExtensions_Copy_For_Strtok = (char*)malloc(extensionSize * sizeof(char));
    strcpy(oclPlatformExtensions_Copy_For_Strtok, oclPlatformExtensions);

    // Find out how many extensions are supported
    char* token = strtok(oclPlatformExtensions_Copy_For_Strtok, " ");
    int le = 0;
    int extensionCount = 0;
    while (token != NULL)
    {
        le++;
        token = strtok(NULL, " ");
    }
    extensionCount = le;
    
    fprintf(gpLogFile, "Platform supports %d extensions\n", extensionCount);

    // Create array of lengths of names of each found extension
    int* extensionNameLengths = (int*)malloc(extensionCount * sizeof(int));
    strcpy(oclPlatformExtensions_Copy_For_Strtok, oclPlatformExtensions);
    token = strtok(oclPlatformExtensions_Copy_For_Strtok, " ");
    le = 0;
    while (token != NULL)
    {
        extensionNameLengths[le] = strlen(token);
        token = strtok(NULL, " ");
        le++;
    }

    // Accordingly allocate a string such that it will hold string w.r.t extension count and each string will be of length as per extensionNameLengths
    char** clExtensionNames_array = NULL;
    clExtensionNames_array = (char**)malloc(extensionCount * sizeof(char*));
    for(int le = 0; le < extensionCount; le++)
    {
        clExtensionNames_array[le] = (char*)malloc((extensionNameLengths[le] + 1) * sizeof(char));
    }
    
    // Copy extension names into allocated array
    strcpy(oclPlatformExtensions_Copy_For_Strtok, oclPlatformExtensions);
    token = strtok(oclPlatformExtensions_Copy_For_Strtok, " ");
    le = 0;
    while (token != NULL)
    {
        strcpy(clExtensionNames_array[le], token);
        token = strtok(NULL, " ");
        le++;
    }

    // Free temporary memory
    
    if(oclPlatformExtensions_Copy_For_Strtok)
    {
        free(oclPlatformExtensions_Copy_For_Strtok);
        oclPlatformExtensions_Copy_For_Strtok = NULL;
    }

    if(extensionNameLengths)
    {
        free(extensionNameLengths);
        extensionNameLengths = NULL;
    }

    // Print all supported OpenCL extension names
    for(int le = 0; le < extensionCount; le++)
    {
        fprintf(gpLogFile, "OpenCL Extension %d: %s\n", le, clExtensionNames_array[le]);
    }

    // Now compare in extension array to check for required OpenCL extensions for Vulkan-OpenCL interop
    int clKHRDeviceUUIDExtensionFound = 0;
    int clKHRExternalMemoryExtensionFound = 0;
    for(int le = 0; le < extensionCount; le++)
    {
        // For "cl_khr_device_uuid"
        if(0 == strcmp(clExtensionNames_array[le], "cl_khr_device_uuid"))
        {
            clKHRDeviceUUIDExtensionFound = 1;
        }

        // For "cl_khr_external_memory"
        if(0 == strcmp(clExtensionNames_array[le], "cl_khr_external_memory"))
        {
            clKHRExternalMemoryExtensionFound = 1;
        }
    }
    
    fprintf(gpLogFile, "cl_khr_device_uuid extension found: %s\n", clKHRDeviceUUIDExtensionFound ? "YES" : "NO");
    fprintf(gpLogFile, "cl_khr_external_memory extension found: %s\n", clKHRExternalMemoryExtensionFound ? "YES" : "NO");

    // Free allocated memory for extension names
    for(int le = 0; le < extensionCount; le++)
    {
        if(clExtensionNames_array[le])
        {
            free(clExtensionNames_array[le]);
            clExtensionNames_array[le] = NULL;
        }
    }
    if(clExtensionNames_array)
    {
        free(clExtensionNames_array);
        clExtensionNames_array = NULL;
    }

    BOOL bResult = FALSE;

    if(1 == clKHRDeviceUUIDExtensionFound && 1 == clKHRExternalMemoryExtensionFound)
    {
        bResult = TRUE;
    }
    
    return (bResult);
}

VkResult Resize(int width, int height)
{
    // Function Declaration
    VkResult CreateSwapchain(VkBool32);
    VkResult CreateImagesAndImageViews(void);
    VkResult CreateCommandPool(void);
    VkResult CreatePipelineLayout(void);
    VkResult CreateRenderPass(void);
    VkResult CreatePipeline(void);
    VkResult CreateFrameBuffers(void);
    VkResult BuildCommandBuffers(void);
    VkResult CreateCommandBuffers(VkCommandBuffer * *vkCommandBuffer_array);
    // VkResult CreateCommandBuffers(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "Resize()    >>>  function started\n");

    if (height < 0)
        height = 1;

    // Check bInitialized
    if (FALSE == bInitialized)
    {
        fprintf(gpLogFile, "Resize()    =>  Initialization yet not completed or failed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // As recreation of swapchain is needed we are going to repeat many steps of initialize again hence set bInitialized to false again
    bInitialized = FALSE;

    // Set global winWidth and winHeight variables
    winWidth = width;
    winHeight = height;

    // 01. Swapchain
    // 02. Swapchain Images and View
    // 03. RenderPass
    // 04. FrameBuffer
    // 05. Pipeline
    // 06. PipelineLayout
    // 07. CommandBuffers
    // 08. Build CommandBuffers

    // Wait for device to complete inhand task
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpLogFile, "Resize()  =>  vkDeviceWaitIdle() is done\n");
    }

    // Check presence of swapchain
    if (VK_NULL_HANDLE == vkSwapchainKHR)
    {
        fprintf(gpLogFile, "Resize()  =>  swapchain is NULL, cannot proceed further \n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Destoy Frame Buffer
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[le], NULL);
        fprintf(gpLogFile, "Resize()  =>  vkDestroyFramebuffer() successful for %d\n", le);
    }

    if (vkFramebuffer_array)
    {
        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
        fprintf(gpLogFile, "Resize()  =>  vkFramebuffer_array freed successfully\n");
    }

    // Destroy RenderPass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkDestroyRenderPass() successful\n");
    }

    // Destroy Pipeline
    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkDestroyPipeline() successful\n");
    }

    // Destroy Pipeline Layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkDestroyPipelineLayout() successful\n");
    }

    // Destroy Command Buffers
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_for_1024x1024_graphics_array[le]);
        fprintf(gpLogFile, "Resize()  =>  vkFreeCommandBuffers() successful for %d\n", le);
    }

    if (vkCommandBuffer_for_1024x1024_graphics_array)
    {
        free(vkCommandBuffer_for_1024x1024_graphics_array);
        vkCommandBuffer_for_1024x1024_graphics_array = NULL;
        fprintf(gpLogFile, "Resize()  =>  vkCommandBuffer_for_1024x1024_graphics_array freed successfully\n");
    }

    // For Depth
    if (vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkDestroyImageView() successful for vkImageView_depth\n");
    }

    if (vkDeviceMemory_depth)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
        vkDeviceMemory_depth = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkFreeMemory() successful for vkDeviceMemory_depth\n");
    }

    if (vkImage_depth)
    {
        vkDestroyImage(vkDevice, vkImage_depth, NULL);
        vkImage_depth = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkDestroyImage() successful for vkImage_depth\n");
    }

    // Destroy Swapchain Images and Image Views

    // Free swapchain image views
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[le], NULL);
        fprintf(gpLogFile, "Resize()  =>  vkDestroyImageView() successful for %d\n", le);
    }

    if (swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpLogFile, "Resize()  =>  swapchainImageView_array freed successfully\n");
    }

    // Free swapchain images
    // for(uint32_t le = 0; le < swapchainImageCount; ++le)
    // {
    //     vkDestroyImage(vkDevice, swapchainImage_array[le], NULL);
    //     fprintf(gpLogFile, "Resize()  =>  vkDestroyImage() successful for %d\n", le);
    //     fclose(gpLogFile);
    // }

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpLogFile, "Resize()  =>  swapchainImage_array freed successfully\n");
    }

    // Destroy Swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Resize()  =>  vkDestroySwapchainKHR() successful\n");
    }

    /****** RECREATE FOR RESIZE ******/

    // Create Swapchain

    // Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreateSwapchain() failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Create Swapchain and ImageViews

    // Swapchain Images and ImageViews
    vkResult = CreateImagesAndImageViews();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreateImagesAndImageViews() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Command Buffers
    vkResult = CreateCommandBuffers(&vkCommandBuffer_for_1024x1024_graphics_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreateCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Pipeline Layout
    vkResult = CreatePipelineLayout();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreatePipelineLayout() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create RenderPass
    vkResult = CreateRenderPass();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreateRenderPass() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Pipeline
    vkResult = CreatePipeline();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreatePipeline() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Frame Buffer
    vkResult = CreateFrameBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  CreateFrameBuffers() failed %d\n", vkResult);
        return (vkResult);
    }

    // Build Command Buffers
    vkResult = BuildCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Resize()    =>  BuildCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }

    bInitialized = TRUE;
    fprintf(gpLogFile, "Resize()    -->  function completed\n");
    return (vkResult);
}

VkResult Display(void)
{
    // 11 June 2025 (Wednesday) 02:08:24 AM === MI DISPLAY MADHYE AATA AALO FIRST TIME IN VULKAN

    // Function Declaration
    VkResult UpdateUniformBuffer(void);
    VkResult BuildCommandBuffers(void);
    VkResult PrepareSinWaveForCPU(unsigned int, unsigned int, float);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkCommandBuffer *vkCommandBuffer_array = NULL;

    // Code

    // if control comes here before initialization is completed then return
    if (FALSE == bInitialized)
    {
        fprintf(gpLogFile, "Display()   => Initialization not completed\n");
        return ((VkResult)VK_FALSE);
    }

    if( TRUE == bOnGpu )
    {

        // fprintf(gpLogFile, "Display()   =>  On GPU\n");

        // Set OpenCL kernels 1st Argument i.e 0th
        oclResult = clSetKernelArg(oclKernel, 0, sizeof(cl_mem), (void *)&posOpenCL);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clSetKernelArg() failed for argument 0\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // Set OpenCL kernels 2nd Argument i.e 1st
        oclResult = clSetKernelArg(oclKernel, 1, sizeof(cl_uint), (void *)&meshWidth);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clSetKernelArg() failed for argument 1\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // Set OpenCL kernels 3rd Argument i.e 2nd
        oclResult = clSetKernelArg(oclKernel, 2, sizeof(cl_uint), (void *)&meshHeight);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clSetKernelArg() failed for argument 2\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // Set OpenCL kernels 4th Argument i.e 3rd
        oclResult = clSetKernelArg(oclKernel, 3, sizeof(float), (void *)&animationTime);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clSetKernelArg() failed for argument 3\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // Map Vulkan buffer for writing before executing OpenCL kernel
        clEnqueueAcquireExternalMemObjectsKHR_fn clEnqueueAcquireExternalMemObjectsKHR = NULL;
        clEnqueueAcquireExternalMemObjectsKHR = (clEnqueueAcquireExternalMemObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(oclPlatformID, "clEnqueueAcquireExternalMemObjectsKHR");
        if (NULL == clEnqueueAcquireExternalMemObjectsKHR)
        {
            fprintf(gpLogFile, "Display()   =>  clGetExtensionFunctionAddressForPlatform() failed for clEnqueueAcquireExternalMemObjectsKHR\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        oclResult = clEnqueueAcquireExternalMemObjectsKHR(oclCommandQueue, 1, &posOpenCL, 0, NULL, NULL);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clEnqueueAcquireExternalMemObjectsKHR() failed\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // Execute OpenCL kernel
        size_t globalWorkSize[2] = {meshWidth, meshHeight};
        oclResult = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clEnqueueNDRangeKernel() failed\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // Release Vulkan buffer after executing OpenCL kernel
        clEnqueueReleaseExternalMemObjectsKHR_fn clEnqueueReleaseExternalMemObjectsKHR = NULL;
        clEnqueueReleaseExternalMemObjectsKHR = (clEnqueueReleaseExternalMemObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(oclPlatformID, "clEnqueueReleaseExternalMemObjectsKHR");
        if (NULL == clEnqueueReleaseExternalMemObjectsKHR)
        {
            fprintf(gpLogFile, "Display()   =>  clGetExtensionFunctionAddressForPlatform() failed for clEnqueueReleaseExternalMemObjectsKHR\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        oclResult = clEnqueueReleaseExternalMemObjectsKHR(oclCommandQueue, 1, &posOpenCL, 0, NULL, NULL);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clEnqueueReleaseExternalMemObjectsKHR() failed\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }

        // End of OpenCL-Vulkan interop section
        oclResult = clFinish(oclCommandQueue);
        if (CL_SUCCESS != oclResult)
        {
            fprintf(gpLogFile, "Display()   =>  clFinish() failed\n");
            return (VK_ERROR_INITIALIZATION_FAILED);
        }
    }
    else
    {
        if (TRUE == bMesh1024Chosen)
        {
            PrepareSinWaveForCPU(1024, 1024, animationTime);
        }
    }

    // if(TRUE == bMesg128Chosen)
    // {
    //     vkCommandBuffer_array = vkCommandBuffer_for_128x128_graphics_array;
    // } else
    if (TRUE == bMesh1024Chosen)
    {
        vkCommandBuffer_array = vkCommandBuffer_for_1024x1024_graphics_array;
    }

    // Build Command Buffers
    vkResult = BuildCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   =>  BuildCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Display()   =>  BuildCommandBuffers() successful\n");
    }

    // Acquire index of next swapchain image

    /**
     *  If function does not gives an image then the
     *  function will return VK_NOT_READY
     */
    vkResult = vkAcquireNextImageKHR(vkDevice,
                                     vkSwapchainKHR,
                                     UINT64_MAX,             //  HERE WE WAIT FOR SWAPCHAIN TO GIVE IMAGE nanoseconds
                                     vkSemaphore_backbuffer, //  WATING FOR A QUEUE TO RELEASE THE IMAGE HELD BY ANOTHER QUEUE DEMANDED BY SWAPCHAIN
                                     VK_NULL_HANDLE,         //  NOT STOPING THE HOST ON DEVICE SO NOT USING FENCE
                                     &currentImageIndex);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   => vkAcquireNextImageKHR() failed\n");
        return (vkResult);
    }

    // Use Fence to allow host to wait of completion of execution of previous command buffer
    vkResult = vkWaitForFences(vkDevice,
                               1,
                               &vkFence_array[currentImageIndex],
                               VK_TRUE,
                               UINT64_MAX);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   => vkWaitForFences() failed\n");
        return (vkResult);
    }

    // Now ready the fences for execution of next command buffer
    vkResult = vkResetFences(vkDevice,
                             1,
                             &vkFence_array[currentImageIndex]);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   => vkWaitForFences() failed\n");
        return (vkResult);
    }

    // One of the member of VkSubmitInfo struct requires array of pipeline stages
    // We have only one of Completion of color attachment output
    // still we need 1 member array
    const VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // declare, memset, initialize VkSubmitInfo
    VkSubmitInfo vkSubmitInfo;
    memset((void *)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDestStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backbuffer;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_for_1024x1024_graphics_array[currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_rendercomplete;

    // now submit above work to the queue
    vkResult = vkQueueSubmit(vkQueue,
                             1,
                             &vkSubmitInfo,
                             vkFence_array[currentImageIndex]);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   => vkQueueSubmit() failed\n");
        return (vkResult);
    }

    // we are going to present rendered image after declaring an initializing VkPresentInfoKHR struct
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void *)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_rendercomplete;

    // Now present the queue
    vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   => vkQueueSubmit() failed\n");
        return (vkResult);
    }

    vkResult = UpdateUniformBuffer();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Display()   => UpdateUniformBuffer() failed\n");
        return (vkResult);
    }

    vkDeviceWaitIdle(vkDevice); //  VALIDATION ||> CASE 01
    return (vkResult);
}

void Update(void)
{
    // Code
    animationTime += 0.005;
}

void Uninitialize(void)
{
    // Function declarations
    cl_int uninitialize_opencl(void);

    // Code
    fprintf(gpLogFile, "Uninitialize()  >>>  function started\n");

    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpLogFile, "Uninitialize()  =>  vkDeviceWaitIdle() is done\n");
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyFence(vkDevice, vkFence_array[le], NULL);
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyFence() successful for %d\n", le);
    }

    if (vkFence_array)
    {
        free(vkFence_array);
        vkFence_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFence_array freed successfully\n");
    }

    if (vkSemaphore_rendercomplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_rendercomplete, NULL);
        vkSemaphore_rendercomplete = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySemaphore() successful for vkSemaphore_rendercomplete\n");
    }

    if (vkSemaphore_backbuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backbuffer, NULL);
        vkSemaphore_backbuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySemaphore() successful for vkSemaphore_backbuffer\n");
    }

    // Frame Buffer
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[le], NULL);
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyFramebuffer() successful for %d\n", le);
    }

    if (vkFramebuffer_array)
    {
        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFramebuffer_array freed successfully\n");
    }

    // Render Pass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyRenderPass() successful\n");
    }

    // Descriptor Pool
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        // Note: Descriptor Sets are automatically freed when pool is destroyed
        vkDescriptorSet = VK_NULL_HANDLE; // Resetting to NULL as it is not
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyDescriptorPool() successful for vkDescriptorPool and vkDescriptorSet\n");
    }

    // Pipeline Layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyPipelineLayout() successful\n");
    }

    if (vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyDescriptorSetLayout() successful\n");
    }

    // Pipeline
    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyPipeline() successful\n");
    }

    // Destroy Shader Fragment

    // Fragment Shader
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyShaderModule() successful for vkShaderModule_fragment_shader\n");
    }

    // Vertex Shader
    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyShaderModule() successful for vkShaderModule_vertex_shader\n");
    }

    // Destroy Uniform Buffer
    if (uniformData.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
        uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for uniformData.vkDeviceMemory\n");
    }

    if (uniformData.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
        uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for uniformData.vkBuffer\n");
    }

    // Vertex Buffer
    if (vertex_positon_1024x1024_graphics.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertex_positon_1024x1024_graphics.vkDeviceMemory, NULL);
        vertex_positon_1024x1024_graphics.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertex_positon_1024x1024_graphics.vkDeviceMemory\n");
    }

    if (vertex_positon_1024x1024_graphics.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertex_positon_1024x1024_graphics.vkBuffer, NULL);
        vertex_positon_1024x1024_graphics.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertex_positon_1024x1024_graphics.vkBuffer\n");
    }

    // Uniform Buffer
    if (uniformData.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
        uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for uniformData.vkDeviceMemory\n");
    }

    if (uniformData.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
        uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for uniformData.vkBuffer\n");
    }

    // Uninitialize OpenCL
    oclResult = uninitialize_opencl();
    if( CL_SUCCESS != oclResult )
    {
        fprintf(gpLogFile, "Uninitialize()  =>  uninitialize_opencl() failed\n");
        return;
    }
    else
    {
        fprintf(gpLogFile, "Uninitialize()  =>  uninitialize_opencl() successful\n");
    }

    if(vertexDataExternal.vkDeviceMemory)
    {
        // Free external vertex data
        vkFreeMemory(vkDevice, vertexDataExternal.vkDeviceMemory, NULL);
        vertexDataExternal.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertexDataExternal.vkDeviceMemory\n");
    }

    if(vertexDataExternal.vkBuffer)
    {
        // Free external vertex buffer
        vkDestroyBuffer(vkDevice, vertexDataExternal.vkBuffer, NULL);
        vertexDataExternal.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertexDataExternal.vkBuffer\n");
    }

    // Descriptor Pool (Note: Descriptor Sets are automatically freed when pool is destroyed)
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyDescriptorPool() successful\n");
    }

    // Command Buffers
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_for_1024x1024_graphics_array[le]);
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeCommandBuffers() successful for %d\n", le);
    }

    if (vkCommandBuffer_for_1024x1024_graphics_array)
    {
        free(vkCommandBuffer_for_1024x1024_graphics_array);
        vkCommandBuffer_for_1024x1024_graphics_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  vkCommandBuffer_for_1024x1024_graphics_array freed successfully\n");
    }

    // Destroy Command Pool
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyCommandPool() successful\n");
    }

    // For Depth
    if (vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImageView() successful for vkImageView_depth\n");
    }

    if (vkDeviceMemory_depth)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
        vkDeviceMemory_depth = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vkDeviceMemory_depth\n");
    }

    if (vkImage_depth)
    {
        vkDestroyImage(vkDevice, vkImage_depth, NULL);
        vkImage_depth = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImage() successful for vkImage_depth\n");
    }

    // Free swapchain image views
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[le], NULL);
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImageView() successful for %d\n", le);
    }

    if (swapchainImageView_array)
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
    //     fclose(gpLogFile);
    // }

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  swapchainImage_array freed successfully\n");
    }

    // Destroy Swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySwapchainKHR() successful\n");
    }

    // No need to Destroy/Uninitialize Device Queue

    // Destroy Vulkan Device
    if (vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroy() successful \n");
    }

    // No Need to Destroy Selected Physical Device as it gets destroy with the Logical Device

    // Destroy Surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL); //  the vkDestroySurfaceKHR() is generic and not platform specific
        vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySurfaceKHR() successful\n");
    }

    if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyDebugReportCallbackEXT_fnptr() successful\n");
    }

    // Destroy Vulkan Instance
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyInstance() successful\n");
    }

    if (bFullscreen)
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle & WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
        ShowCursor(TRUE);
        bFullscreen = FALSE;
    }

    if (gpLogFile)
    {
        fprintf(gpLogFile, "Uninitialize()  -->  program terminated successfully\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
}

cl_int uninitialize_opencl(void)
{
    // Code

    if(posOpenCL)
    {
        clReleaseMemObject(posOpenCL);
        posOpenCL = NULL;
    }

    if(oclKernel)
    {
        clReleaseKernel(oclKernel);
        oclKernel = NULL;
    }

    if(oclProgram)
    {
        clReleaseProgram(oclProgram);
        oclProgram = NULL;
    }

    if(oclCommandQueue)
    {
        clReleaseCommandQueue(oclCommandQueue);
        oclCommandQueue = NULL;
    }

    if(oclContext)
    {
        clReleaseContext(oclContext);
        oclContext = NULL;
    }

    return(CL_SUCCESS);
}

/*****************************************************************************************************************************\
                                        VULKAN RELATED FUNCTIONS
\*****************************************************************************************************************************/

//  STEP 03
VkResult CreateVulkanInstance(void)
{
    // Function Declaration
    VkResult FillInstanceExtensionNames(void);
    VkResult FillValidationLayerNames(void);
    VkResult CreateValidationCallbackFunction(void);

    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;
    VkApplicationInfo vkApplicationInfo;
    VkInstanceCreateInfo vkInstanceCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateVulkanInstance()    >>>  function started\n");

    memset((void *)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
    memset((void *)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

    // Step 01 Fill and Initialize required extension names & count global variables
    vkResult = FillInstanceExtensionNames();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  FillInstanceExtensionNames() failed\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  FillInstanceExtensionNames() successful\n");
    }

    // Fill Validation Layer
    if (TRUE == bValidation)
    {
        vkResult = FillValidationLayerNames();
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVulkanInstance()   =>  FillValidationLayerNames() failed\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVulkanInstance()   =>  FillValidationLayerNames() successful\n");
        }
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

    if (TRUE == bValidation)
    {
        vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerName_array;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    // Step 04: Call vkCreateInstance() to get VkInstance in global variable & do error checking
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
    if (VK_ERROR_INCOMPATIBLE_DRIVER == vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> incompatible driver(%d)\n", vkResult);
        return (vkResult);
    }
    else if (VK_ERROR_EXTENSION_NOT_PRESENT == vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> extension not present(%d)\n", vkResult);
        return (vkResult);
    }
    else if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> unknown reason(%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanInstance()   =>  vkCreateInstance() successful\n");
    }

    // DO FOR VALIDATION CALLBACKS
    if (TRUE == bValidation)
    {
        vkResult = CreateValidationCallbackFunction();
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVulkanInstance()   =>  CreateValidationCallbackFunction() failed\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVulkanInstance()   =>  CreateValidationCallbackFunction() successful\n");
        }
    }

    fprintf(gpLogFile, "CreateVulkanInstance()    -->  function completed\n");

    return (vkResult);
}

// STEP 02
VkResult FillInstanceExtensionNames(void)
{
    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;
    uint32_t instanceExtensionCount = 0;
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    char **instanceExtensionNames_array = NULL;
    VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 win32SurfaceExtensionFound = VK_FALSE;
    VkBool32 debugReportExtensionFound = VK_FALSE;

    // Code
    fprintf(gpLogFile, "FillInstanceExtensionNames()  >>>  function started\n");

    // Step 01: Find how many instance extensions are supported by the Vulkan Driver for this version & keep the count in the local variable

    // First Vulkan API
    vkResult = vkEnumerateInstanceExtensionProperties(NULL,                    // Layer Name : कोणत्या layer ची extensions हवी आहे NULL -> सर्वच हवे आहे
                                                      &instanceExtensionCount, // Count : ExtensionCount
                                                      NULL);                   // Extension च्या properties चा array (Parameterized Return Value)
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() failed - first call\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() successful - first call\n");
    }

    //  Step 02: Allocate & Fill struct VkExtensionProperties array corresponding to above count;
    vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkExtensionProperties_array)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  Out of memory while allocating vkExtensionProperties_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL,
                                                      &instanceExtensionCount,
                                                      vkExtensionProperties_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() failed - second call\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() successful - second call\n");
    }

    //  Step 03: Fill & Display a local string array of extension names obtained from VkExtensionProperties array
    instanceExtensionNames_array = (char **)malloc(sizeof(char *) * instanceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == instanceExtensionNames_array)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  Out of memory while allocating instanceExtensionNames_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    //  Allocate memory for each string member
    for (uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        instanceExtensionNames_array[le] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == instanceExtensionNames_array[le])
        {
            fprintf(gpLogFile, "FillInstanceExtensionNames()   =>  Out of memory while allocating instanceExtensionNames_array[%d]\n", le);
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        memcpy(instanceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  vulkan instance extension name = %s\n", instanceExtensionNames_array[le]);
    }

    //  Step 04: As not required henceforth, free the allocated VkExtensionProperties Array;
    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    //  Step 05: Find whether above extension names contained our required 2 extensions viz, VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    for (uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        // VK_KHR_SURFACE_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_KHR_SURFACE_EXTENSION_NAME))
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        // VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
        {
            win32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }

        // VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            debugReportExtensionFound = VK_TRUE;
            if (TRUE == bValidation)
            {
                enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
            else
            {
                // Array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
            }
        }
    }

    // Step 06: As not needed henceforth, free the local string and the array
    // Free each string
    for (uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        free(instanceExtensionNames_array[le]);
        instanceExtensionNames_array[le] = NULL;
    }
    //  Free the array
    free(instanceExtensionNames_array);
    instanceExtensionNames_array = NULL;

    //  Step 07: Print whether out Vulkan Driver supports our required extension names of not.
    if (VK_FALSE == vulkanSurfaceExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_SURFACE_EXTENSION_NAME found successfully\n");
    }

    if (VK_FALSE == win32SurfaceExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_WIN32_SURFACE_EXTENSION_NAME found successfully\n");
    }

    if (VK_FALSE == debugReportExtensionFound)
    {
        if (TRUE == bValidation)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
            fprintf(gpLogFile, "FillInstanceExtensionNames()    ||>  validation is on but VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "FillInstanceExtensionNames()    ||>  vailidation is off and VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found\n");
        }
    }
    else
    {
        if (TRUE == bValidation)
        {
            fprintf(gpLogFile, "FillInstanceExtensionNames()    ||>  validation is on and VK_EXT_DEBUG_REPORT_EXTENSION_NAME found successfully\n");
        }
        else
        {
            fprintf(gpLogFile, "FillInstanceExtensionNames()    ||>  validation is off and VK_EXT_DEBUG_REPORT_EXTENSION_NAME found successfully\n");
        }
    }

    //  Step 08: Print only enabled extension names
    for (uint32_t le = 0; le < enabledInstanceExtensionCount; ++le)
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  Enabled vulkan instance extension name = %s\n", enabledInstanceExtensionNames_array[le]);
    }

    fprintf(gpLogFile, "FillInstanceExtensionNames()    -->  function completed\n");

    return (vkResult);
}

VkResult FillValidationLayerNames(void)
{
    //  Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;
    VkLayerProperties *vkLayerProperties_array = NULL;
    char **validationLayerNames_array = NULL;
    VkBool32 validationLayerFound = VK_FALSE;

    // Code
    fprintf(gpLogFile, "FillValidationLayerNames()    >>>  function started\n");

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() failed - first call\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() successful - first call\n");
    }

    vkLayerProperties_array = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * validationLayerCount);
    // FOR THE SAKE OF BREVITY

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() failed - second call\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() successful - second call\n");
    }

    validationLayerNames_array = (char **)malloc(sizeof(char *) * validationLayerCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == validationLayerNames_array)
    {
        fprintf(gpLogFile, "FillValidationLayerNames()   =>  Out of memory while allocating validationLayerNames_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    //  Allocate memory for each string member
    for (uint32_t le = 0; le < validationLayerCount; ++le)
    {
        validationLayerNames_array[le] = (char *)malloc(sizeof(char) * (strlen(vkLayerProperties_array[le].layerName) + 1));
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == validationLayerNames_array[le])
        {
            fprintf(gpLogFile, "FillValidationLayerNames()   =>  Out of memory while allocating validationLayerNames_array[%d]\n", le);
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        memcpy(validationLayerNames_array[le], vkLayerProperties_array[le].layerName, strlen(vkLayerProperties_array[le].layerName) + 1);

        fprintf(gpLogFile, "FillValidationLayerNames()    =>  vulkan instance layer name = %s\n", validationLayerNames_array[le]);
    }

    if (vkLayerProperties_array)
    {
        free(vkLayerProperties_array);
        vkLayerProperties_array = NULL;
    }

    for (uint32_t le = 0; le < validationLayerCount; ++le)
    {
        if (0 == strcmp(validationLayerNames_array[le], "VK_LAYER_KHRONOS_validation"))
        {
            validationLayerFound = VK_TRUE;
            enabledValidationLayerName_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }

    for (uint32_t le = 0; le < validationLayerCount; ++le)
    {
        free(validationLayerNames_array[le]);
        validationLayerNames_array[le] = NULL;
    }
    //  Free the array
    free(validationLayerNames_array);
    validationLayerNames_array = NULL;

    if (VK_FALSE == validationLayerFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        fprintf(gpLogFile, "FillValidationLayerNames()    =>  VK_LAYER_KHRONOS_validation not found\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillValidationLayerNames()    =>  VK_LAYER_KHRONOS_validation found successfully\n");
    }

    if (TRUE == bValidation)
    {
        for (uint32_t le = 0; le < enabledValidationLayerCount; ++le)
        {
            fprintf(gpLogFile, "FillValidationLayerNames()    ||>  Enabled vulkan instance layer name = %s\n", enabledValidationLayerName_array[le]);
        }
    }

    fprintf(gpLogFile, "FillValidationLayerNames()    -->  function completed\n");
    return (vkResult);
}

VkResult CreateValidationCallbackFunction(void)
{
    // Function Declaration
    // Callback Function
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT,
                                                       VkDebugReportObjectTypeEXT,
                                                       uint64_t,
                                                       size_t,
                                                       int32_t,
                                                       const char *,
                                                       const char *,
                                                       void *);

    //  Variable Definition
    VkResult vkResult = VK_SUCCESS;
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;

    // Code
    fprintf(gpLogFile, "CreateValidationCallbackFunction()    >>>  function started\n");

    // Get the required function pointers
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
    if (NULL == vkCreateDebugReportCallbackEXT_fnptr)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpLogFile, "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() failed to get function pointer for vkCreateDebugReportCallbackEXT\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() successful to get function pointer for vkCreateDebugReportCallbackEXT\n");
    }

    vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    if (NULL == vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpLogFile, "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() failed to get function pointer for vkDestroyDebugReportCallbackEXT\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() successful to get function pointer for vkDestroyDebugReportCallbackEXT\n");
    }

    // Get the Vulkan Debug Report callback object
    memset((void *)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = DebugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateValidationCallbackFunction()   =>  vkCreateDebugReportCallbackEXT_fnptr() failed\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateValidationCallbackFunction()   =>  vkCreateDebugReportCallbackEXT_fnptr() successful\n");
    }

    fprintf(gpLogFile, "CreateValidationCallbackFunction()    -->  function completed\n");
    return (vkResult);
}

//  STEP 04
VkResult GetSupportedSurface(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;

    // Code
    fprintf(gpLogFile, "GetSupportedSurface()    >>>  function started\n");

    memset((void *)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetSupportedSurface()   =>  vkCreateWin32SurfaceKHR() failed\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetSupportedSurface()   =>  vkCreateWin32SurfaceKHR() successful\n");
    }

    fprintf(gpLogFile, "GetSupportedSurface()    -->  function completed\n");

    return (vkResult);
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
    fprintf(gpLogFile, "GetPhysicalDevice() >>>  function started\n");

    // Step 02: call vkEnumeratePhysicalDevices() function to get Physical device count.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() failed - first call - %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == physicalDeviceCount)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()     =>  physicalDeviceCount is 0\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() successful - first call\n");
    }

    // Step 03: Allocate VkPhysicalDevice array according to above count
    vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkPhysicalDevice_array)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  Out of memory while allocating vkPhysicalDevice_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    // Step 04: Call vkEnumeratePhysicalDevice() to fill above array
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() failed - second call - %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() successful - second call\n");
    }

    // Step 05: Start a loop using physicalDeviceCount & physicalDevice array
    for (uint32_t ole = 0; ole < physicalDeviceCount; ++ole)
    {
        // SubStep a: Declare a local variable to hold queue count
        uint32_t queueCount = 0;

        // SubStep b: Call vkGetPhysicalDeviceQueueFamilyProperties() to initialize above queue count variable.
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[ole], &queueCount, NULL); // If physical device present then it must have atleast 1 queue family

        // SubStep c: Allocate VkQueueFamilyProperties array according to above count
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queueCount);
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == vkQueueFamilyProperties_array)
        {
            fprintf(gpLogFile, "GetPhysicalDevice()   =>  Out of memory while allocating vkQueueFamilyProperties_array\n");
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        // SubStep d: call vkGetPhysicalDeviceQueueFamilyProperties
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[ole], &queueCount, vkQueueFamilyProperties_array);

        // SubStep e: Declare VkBool32 array & allocate it using above queue count.
        isQueueSurfaceSupported_array = (VkBool32 *)malloc(sizeof(VkBool32) * queueCount);
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == isQueueSurfaceSupported_array)
        {
            free(vkQueueFamilyProperties_array); // Clean up previous allocation
            fprintf(gpLogFile, "GetPhysicalDevice()   =>  Out of memory while allocating isQueueSurfaceSupported_array\n");
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        // SubStep f: Start a nested loop & fill above VkBool32 type array by calling vkGetPhysicalDeviceSurfaceSupportKHR()
        for (uint32_t ile = 0; ile < queueCount; ++ile)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[ole], ile, vkSurfaceKHR, &isQueueSurfaceSupported_array[ile]);
        }

        // SubStep g: Start another nested loop & check whether physical device in its array with its queue family has graphics bit or not.
        for (uint32_t ile = 0; ile < queueCount; ++ile)
        {
            if (vkQueueFamilyProperties_array[ile].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (VK_TRUE == isQueueSurfaceSupported_array[ile])
                {
                    vkPhysicalDevice_selected = vkPhysicalDevice_array[ole];
                    graphicsQueueFamilyIndex_selected = ile;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }

        //  SubStep h: free the queue-family array and VkBool32 array
        if (isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(gpLogFile, "GetPhysicalDevice() =>  isQueueSurfaceSupported_array successfully freed\n");
        }

        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gpLogFile, "GetPhysicalDevice() =>  vkQueueFamilyProperties_array successful freed\n");
        }

        // SubStep i:
        if (VK_TRUE == bFound)
        {
            break;
        }
    }

    // Step 06: Do error checking according to value of bfound
    if (VK_TRUE == bFound)
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  successful in getting Physical Device with graphics enabled\n");
    }
    else
    {
        if (vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
            fprintf(gpLogFile, "GetPhysicalDevice() =>  vkPhysicalDevice_array successfully freed\n");
        }

        fprintf(gpLogFile, "GetPhysicalDevice() => failed to select PhysicalDevice \n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Step 07: memset the global vkPhysicalDeviceMemoryProperties
    memset((void *)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    // Step 08: Initialize above structure by using vkGetPhysicalDeviceMemoryProperties()
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties); //  This is the Good Place to do this.

    // Step 09: Declare a local structure VkPhysicalDeviceFeatures, memset it, & initialize it by calling vkGetPhysicalDeviceFeatures()
    memset((void *)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

    // Step 10: By using tessellationShader member of above structure, check ifselected devices supportsd TessellationShader
    if (vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device support Tessellation Shader\n");
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device does not support Tessellation Shader\n");
    }

    // Step 11: By using geometryShader member of above structure, check ifselected devices supportsd GeometryShader;
    if (vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device support Geometry Shader\n");
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevice() =>  selected physical device does not support Geometry Shader\n");
    }

    fprintf(gpLogFile, "GetPhysicalDevice()    -->  function completed\n");
    return (vkResult);
}

VkResult PrintVkInfo(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "PrintVkInfo()    >>>  function started\n");

    NEWLINEINFILE;
    fprintf(gpLogFile, "**************************************************************************\n");
    fprintf(gpLogFile, "                     Vulkan Info\n");
    fprintf(gpLogFile, "**************************************************************************\n");
    NEWLINEINFILE;

    for (uint32_t le = 0; le < physicalDeviceCount; ++le)
    {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void *)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[le], &vkPhysicalDeviceProperties);

        uint32_t majorVersion = VK_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

        // API Version
        fprintf(gpLogFile, "API Version = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

        // Device Name
        fprintf(gpLogFile, "Device Name = %s\n", vkPhysicalDeviceProperties.deviceName);

        // Device Type
        switch (vkPhysicalDeviceProperties.deviceType)
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
    fprintf(gpLogFile, "**************************************************************************\n");
    NEWLINEINFILE;

    // Free Physical Device
    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(gpLogFile, "PrintVkInfo() =>  vkPhysicalDevice_array successfully freed\n");
    }

    fprintf(gpLogFile, "PrintVkInfo()    -->  function completed\n");
    return (vkResult);
}

// STEP 07
VkResult FillDeviceExtensionNames(void)
{
    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;
    uint32_t deviceExtensionCount = 0;
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    char **deviceExtensionNames_array = NULL;
    VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;
    VkBool32 vulkanExternalMemoryWin32ExtensionFound = VK_FALSE;

    // Code
    fprintf(gpLogFile, "FillDeviceExtensionNames()  >>>  function started\n");

    // First Vulkan API
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    NULL);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() failed - first call\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() successful - first call\n");
    }

    vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkExtensionProperties_array)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  Out of memory while allocating vkExtensionProperties_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() failed - second call\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() successful - second call\n");
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  number of found device extension count = %u\n", deviceExtensionCount);
    }

    deviceExtensionNames_array = (char **)malloc(sizeof(char *) * deviceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == deviceExtensionNames_array)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  Out of memory while allocating deviceExtensionNames_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    //  Allocate memory for each string member
    for (uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        deviceExtensionNames_array[le] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == deviceExtensionNames_array[le])
        {
            fprintf(gpLogFile, "FillDeviceExtensionNames()   =>  Out of memory while allocating deviceExtensionNames_array[%d]\n", le);
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        memcpy(deviceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  vulkan device extension name = %s\n", deviceExtensionNames_array[le]);
    }

    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    for (uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        // VK_KHR_SWAPCHAIN_EXTENSION_NAME if macro for "VK_KHR_swapchain"
        if (0 == strcmp(deviceExtensionNames_array[le], VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }

        // VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME is macro for "VK_KHR_external_memory_win32_name"
        if (0 == strcmp(deviceExtensionNames_array[le], VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME))
        {
            vulkanExternalMemoryWin32ExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
        }
    }

    // Free each string
    for (uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        free(deviceExtensionNames_array[le]);
        deviceExtensionNames_array[le] = NULL;
    }
    //  Free the array
    free(deviceExtensionNames_array);
    deviceExtensionNames_array = NULL;

    if (VK_FALSE == vulkanSwapchainExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  VK_KHR_SWAPCHAIN_EXTENSION_NAME found successfully\n");
    }

    if (VK_FALSE == vulkanExternalMemoryWin32ExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME found successfully\n");
    }

    for (uint32_t le = 0; le < enabledDeviceExtensionCount; ++le)
    {
        fprintf(gpLogFile, "FillDeviceExtensionNames()    =>  Enabled vulkan device extension name = %s\n", enabledDeviceExtensionNames_array[le]);
    }

    fprintf(gpLogFile, "FillDeviceExtensionNames()    -->  function completed\n");

    return (vkResult);
}

// Step 08
VkResult CreateVulkanDevice(void)
{
    // Function Declaration
    VkResult FillDeviceExtensionNames(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkDeviceCreateInfo vkDeviceCreateInfo;

    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo; //  NEW CODE ADDITION
    float queuePriorities[1];

    // Code
    fprintf(gpLogFile, "CreateVulkanDevice()  >>>  function started\n");

    //  Step 07 - Device Extensions

    // Step 01 & Step 02
    vkResult = FillDeviceExtensionNames();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  FillDeviceExtensionNames() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  FillDeviceExtensionNames() successful\n");
    }

    //  Step 03
    memset((void *)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    memset((void *)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo)); //  NEW CODE ADDITION

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
    vkDeviceCreateInfo.enabledLayerCount = 0;      //  THIS IS DEPRECATED, KEPT HERE BECAUSE SIR SAID SO
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL; //  THIS IS DEPRECATED, KEPT HERE BECAUSE SIR SAID SO
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    // Step 05
    vkResult = vkCreateDevice(vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  vkCreateDevice() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVulkanDevice()    =>  vkCreateDevice() successful\n");
    }

    fprintf(gpLogFile, "CreateVulkanDevice()  -->  function completed\n");
    return (vkResult);
}

void GetDeviceQueue(void)
{
    // Code
    fprintf(gpLogFile, "GetDeviceQueue()  >>>  function started\n");

    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

    if (VK_NULL_HANDLE == vkQueue)
    {
        fprintf(gpLogFile, "GetDeviceQueue()  =>  vkGetDeviceQueue() resulted null for vkQueue\n");
        return;
    }
    else
    {
        fprintf(gpLogFile, "GetDeviceQueue()  =>  vkGetDeviceQueue() successful\n");
    }

    fprintf(gpLogFile, "GetDeviceQueue()  -->  function completed\n");
}

VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t formatCount = 0;
    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array;

    // Code
    fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()  >>>  function started\n");

    // Get the count of supported surface color formats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed first call with %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == formatCount)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed first call %d that resulted in 0 formatCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() first call successful with %d format counts\n", formatCount);
    }

    vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkSurfaceFormatKHR_array)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  Out of memory while allocating vkSurfaceFormatKHR_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkSurfaceFormatKHR_array successfully allocated\n");
    }

    //  Filling the array
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed second call with %d\n", vkResult);
        return (vkResult);
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
    if (1 == formatCount && VK_FORMAT_UNDEFINED == vkSurfaceFormatKHR_array[0].format)
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

    if (vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace() =>  vkSurfaceFormatKHR_array successfully freed\n");
    }

    fprintf(gpLogFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace()  -->  function completed\n");
    return (vkResult);
}

VkResult GetPhysicalDevicePresentMode(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t presentModeCount = 0;
    VkPresentModeKHR *vkPresentModeKHR_array = NULL;

    // Code
    fprintf(gpLogFile, "GetPhysicalDevicePresentMode()  >>>  function started\n");

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed first call with %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == presentModeCount)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed first call %d that resulted in 0 presentModeCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() first call successful with %d presentModeCount\n", presentModeCount);
    }

    vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkPresentModeKHR_array)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  Out of memory while allocating vkPresentModeKHR_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed second call with %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() second call successful with %d presentModeCount\n", presentModeCount);
    }

    // MY LOOP TO JUST LOG AND SEE PRESENT MODE OF MY PHYSICAL DEVICE
    for (uint32_t le = 0; le < presentModeCount; ++le)
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  found %d present mode\n", vkPresentModeKHR_array[le]);
    }

    // Decide the Presentation Mode
    for (uint32_t le = 0; le < presentModeCount; ++le)
    {
        if (VK_PRESENT_MODE_MAILBOX_KHR == vkPresentModeKHR_array[le])
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    if (VK_PRESENT_MODE_MAILBOX_KHR != vkPresentModeKHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  selected VK_PRESENT_MODE_FIFO_KHR for present mode\n");
    }
    else
    {
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode()    =>  selected VK_PRESENT_MODE_MAILBOX_KHR for present mode\n");
    }

    if (vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gpLogFile, "GetPhysicalDevicePresentMode() =>  vkPresentModeKHR_array successfully freed\n");
    }

    fprintf(gpLogFile, "GetPhysicalDevicePresentMode()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateSwapchain(VkBool32 vSync)
{
    // Function Declarations
    VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult GetPhysicalDevicePresentMode(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    uint32_t testingNROfSwapchainImages = 0;
    uint32_t desiredNROfSwapchainImages = 0;
    VkImageUsageFlags vkImageUsageFlags;
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;

    // Code
    fprintf(gpLogFile, "CreateSwapchain()  >>>  function started\n");

    // ColorFormat and ColorSpace
    vkResult = GetPhysicalDeviceSurfaceFormatAndColorSpace();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDeviceSurfaceFormatAndColorSpace() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDeviceSurfaceFormatAndColorSpace() successful\n");
    }

    // Get Physical Device Surface Capabilities
    memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceKHR));

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkGetPhysicalDeviceSurfaceCapabilitiesKHR() successful\n");
    }

    fprintf(gpLogFile, "CreateSwapchain()    =>  minImageCount %d\n", vkSurfaceCapabilitiesKHR.minImageCount);
    fprintf(gpLogFile, "CreateSwapchain()    =>  maxImageCount %d\n", vkSurfaceCapabilitiesKHR.maxImageCount);

    // findout desired swapchain image count
    testingNROfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNROfSwapchainImages)
    {
        desiredNROfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNROfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }

    // choose size of swapchain image
    memset((void *)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        fprintf(gpLogFile, "CreateSwapchain()    =>  swapchain image width x height = %d x %d\n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    else
    {
        // ifsurface size is already defined then swapchain image size must match withit
        VkExtent2D vkExtent2D;

        memset((void *)&vkExtent2D, 0, sizeof(vkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.minImageExtent.width %d\n", vkSurfaceCapabilitiesKHR.minImageExtent.width);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.maxImageExtent.width %d\n", vkSurfaceCapabilitiesKHR.maxImageExtent.width);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkExtent2D.width %d\n", vkExtent2D.width);
        fprintf(gpLogFile, "CreateSwapchain()    =>  selected width = %d\n", glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)));

        vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));

        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.minImageExtent.height %d\n", vkSurfaceCapabilitiesKHR.minImageExtent.height);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.maxImageExtent.height %d\n", vkSurfaceCapabilitiesKHR.maxImageExtent.height);
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkExtent2D.height %d\n", vkExtent2D.height);
        fprintf(gpLogFile, "CreateSwapchain()    =>  selected height = %d\n", glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)));

        vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
    }

    // Set swapchain image usage flag
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VIMP must be given
    // VK_IMAGE_USAGE_TRANSFER_SRC_BIT : this is optional but is given everytime for texture, FBO, compute
    vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // Whether to consider Pre-transform/Flipping or not
    if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  in ifvkSurfaceTransformFlagBitsKHR = %d\n", vkSurfaceTransformFlagBitsKHR);
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
        // FOR MY LOGGING
        fprintf(gpLogFile, "CreateSwapchain()    =>  in else vkSurfaceTransformFlagBitsKHR = %d\n", vkSurfaceTransformFlagBitsKHR);
    }

    // Present Mode
    vkResult = GetPhysicalDevicePresentMode();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDevicePresentMode() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  GetPhysicalDevicePresentMode() successful\n");
    }

    // Initialize VkSwapchainCreateInfoKHR struct
    memset((void *)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNROfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkCreateSwapchainKHR() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSwapchain()    =>  vkCreateSwapchainKHR() successful\n");
    }

    fprintf(gpLogFile, "CreateSwapchain()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateImagesAndImageViews(void)
{
    // Function Declarations
    VkResult GetSupportedDepthFormat(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkImageViewCreateInfo vkImageViewCreateInfo;
    VkImageCreateInfo vkImageCreateInfo;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    VkMemoryRequirements vkMemoryRequirements;

    // Code
    fprintf(gpLogFile, "CreateImagesAndImageViews()  >>>  function started\n");

    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed first call with %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == swapchainImageCount)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed first call %d that resulted in 0 swapchainImageCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() first call successful with %d swapchainImageCount\n", swapchainImageCount);
    }

    swapchainImage_array = (VkImage *)malloc(sizeof(VkImage) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == swapchainImage_array)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  malloc failed for swapchainImage_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed second call with %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() second successful with %d swapchainImageCount\n", swapchainImageCount);
    }

    swapchainImageView_array = (VkImageView *)malloc(sizeof(VkImageView) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == swapchainImageView_array)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  malloc failed for swapchainImageView_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    memset((void *)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

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
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[le];

        vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImageViews() failed for iteration %d. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImageViews() successful for iteration %d \n", le);
        }
    }

    // For Depth Image
    vkResult = GetSupportedDepthFormat();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  GetSupportedDepthFormat() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  GetSupportedDepthFormat() successful\n");
    }

    // For Depth Image, Initialize VkImageCreateInfo
    memset((void *)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = vkFormat_depth;
    vkImageCreateInfo.extent.width = winWidth;
    vkImageCreateInfo.extent.height = winHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    // vkImageCreateInfo.pNext = NULL;
    // vkImageCreateInfo.flags = 0;
    // vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    // vkImageCreateInfo.format = vkFormat_depth;
    // vkImageCreateInfo.extent.width = windowWidth;
    // vkImageCreateInfo.extent.height = windowHeight;
    // vkImageCreateInfo.extent.depth = 1;
    // vkImageCreateInfo.mipLevels = 1;
    // vkImageCreateInfo.arrayLayers = 1;
    // vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    // vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    // vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // vkImageCreateInfo.queueFamilyIndexCount = 0;
    // vkImageCreateInfo.pQueueFamilyIndices = NULL;
    // vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_depth);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImage() failed (%d) for depth image\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImage() successful for depth image\n");
    }

    // Memory Requirements for Image
    // Get Image Memory Requirements
    vkGetImageMemoryRequirements(vkDevice, vkImage_depth, &vkMemoryRequirements);

    // Allocate Required Memory
    memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    {
        if (1 == (vkMemoryRequirements.memoryTypeBits & 1))
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = le;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_depth);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkAllocateMemory() successful \n");
    }

    // Bind Image to Memory
    vkResult = vkBindImageMemory(vkDevice, vkImage_depth, vkDeviceMemory_depth, 0);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkBindImageMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkBindImageMemory() successful \n");
    }

    // Create Image View for Above Depth Image
    memset((void *)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_depth;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_depth;

    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkImageView_depth);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImageView() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateImagesAndImageViews()    =>  vkCreateImageView() successful \n");
    }

    fprintf(gpLogFile, "CreateImagesAndImageViews()  -->  function completed\n");
    return (vkResult);
}

VkResult GetSupportedDepthFormat(void)
{
    // Code
    VkResult vkResult = VK_SUCCESS;
    VkFormat vkFormat_depth_array[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM};

    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  function started\n");

    // Options to Select depth format
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  Options for Depth Format\n");
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  VK_FORMAT_D32_SFLOAT_S8_UINT %d\n", VK_FORMAT_D32_SFLOAT_S8_UINT);
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  VK_FORMAT_D32_SFLOAT %d\n", VK_FORMAT_D32_SFLOAT);
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  VK_FORMAT_D24_UNORM_S8_UINT %d\n", VK_FORMAT_D24_UNORM_S8_UINT);
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  VK_FORMAT_D16_UNORM_S8_UINT %d\n", VK_FORMAT_D16_UNORM_S8_UINT);
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  VK_FORMAT_D16_UNORM %d\n", VK_FORMAT_D16_UNORM);

    for (UINT le = 0; le < sizeof(vkFormat_depth_array) / sizeof(vkFormat_depth_array[0]); le++)
    {
        VkFormatProperties vkFormatProperties;

        memset(&vkFormatProperties, 0, sizeof(VkFormatProperties));

        vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice_selected, vkFormat_depth_array[le], &vkFormatProperties);

        if (vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            vkFormat_depth = vkFormat_depth_array[le];
            vkResult = VK_SUCCESS;
            break;
        }
    }

    // Selected Depth Format
    fprintf(gpLogFile, "GetSupportedDepthFormat()  >>>  Selected Depth Format: %d\n", vkFormat_depth);

    fprintf(gpLogFile, "GetSupportedDepthFormat()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateCommandPool(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateCommandPool()  >>>  function started\n");

    memset((void *)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

    vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateCommandPool()    =>  vkCreateCommandPool() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateCommandPool()    =>  vkCreateCommandPool() successful\n");
    }

    fprintf(gpLogFile, "CreateCommandPool()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateCommandBuffers(VkCommandBuffer **ppVkCommandBuffer_array)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkCommandBuffer *vkCommandBuffer_array = NULL;
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;

    // Code
    fprintf(gpLogFile, "CreateCommandBuffers()  >>>  function started\n");

    memset((void *)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    vkCommandBuffer_array = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkCommandBuffer_array)
    {
        fprintf(gpLogFile, "CreateCommandBuffers()    =>  malloc failed for vkCommandBuffer_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateCommandBuffers()    =>  vkAllocateCommandBuffers() failed for iteration %d. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateCommandBuffers()    =>  vkAllocateCommandBuffers() successful for iteration %d \n", le);
        }
    }

    *ppVkCommandBuffer_array = vkCommandBuffer_array;

    fprintf(gpLogFile, "CreateCommandBuffers()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateVertexBuffer(unsigned int meshWidth, unsigned int meshHeight, VertexData *pVertexData)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    // Position
    VertexData vertexData_position;
    unsigned int size = meshWidth * meshHeight * 4 * sizeof(float);
    float triangle_position[] = {0.0f, 1.0f, 0.0f,   // Top Vertex
                                 -1.0f, -1.0f, 0.0f, // Bottom Left Vertex
                                 1.0f, -1.0f, 0.0f}; // Bottom Right Vertex

    VkBufferCreateInfo vkBufferCreateInfo;
    VkMemoryRequirements vkMemoryRequirements;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    void *data = NULL;

    // Code
    fprintf(gpLogFile, "CreateVertexBuffer()  >>>  function started\n");

    memset((void *)&vertexData_position, 0, sizeof(VertexData));
    memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
    vkBufferCreateInfo.size = size;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() successful \n");
    }

    memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

    memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // Initialia value before loop

    for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    {
        if (1 == (vkMemoryRequirements.memoryTypeBits & 1))
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = le;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() successful \n");
    }

    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() successful \n");
    }

    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() successful \n");
    }

    if (1024 == meshWidth && 1024 == meshHeight)
    {
        memcpy(data, pos_1024_graphics, size);
    }

    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

    *pVertexData = vertexData_position;

    fprintf(gpLogFile, "CreateVertexBuffer()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateExternalVertexBuffer(unsigned int meshWidth, unsigned int meshHeight, VertexData *pVertexData)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    // Position
    VertexData vertexData_position;
    unsigned int size = meshWidth * meshHeight * 4 * sizeof(float);

    VkBufferCreateInfo vkBufferCreateInfo;
    VkExternalMemoryBufferCreateInfo vkExternalMemoryBufferCreateInfo;
    VkMemoryRequirements vkMemoryRequirements;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;

    // Code
    fprintf(gpLogFile, "CreateExternalVertexBuffer()  >>>  function started\n");

    // Initialize External Memory Buffer Info
    memset((void *)&vertexData_position, 0, sizeof(VertexData));
    memset((void *)&vkExternalMemoryBufferCreateInfo, 0, sizeof(VkExternalMemoryBufferCreateInfo));

    vkExternalMemoryBufferCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    vkExternalMemoryBufferCreateInfo.pNext = NULL;
    vkExternalMemoryBufferCreateInfo.handleTypes = vkExternalMemoryHandleTypeFlagBits;

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = &vkExternalMemoryBufferCreateInfo;
    vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
    vkBufferCreateInfo.size = size;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkCreateBuffer() successful \n");
    }

    memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

    // Prepare for Exportable memory allocation
    VkExportMemoryAllocateInfoKHR vkExportMemoryAllocateInfoKHR;
    memset((void *)&vkExportMemoryAllocateInfoKHR, 0, sizeof(VkExportMemoryAllocateInfoKHR));
    vkExportMemoryAllocateInfoKHR.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    vkExportMemoryAllocateInfoKHR.pNext = NULL;
    vkExportMemoryAllocateInfoKHR.handleTypes = vkExternalMemoryHandleTypeFlagBits;

    memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = &vkExportMemoryAllocateInfoKHR;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // Initialia value before loop

    for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    {
        if (1 == (vkMemoryRequirements.memoryTypeBits & 1))
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = le;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkAllocateMemory() successful \n");
    }

    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkBindBufferMemory() successful \n");
    }

    // vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    // if (VK_SUCCESS != vkResult)
    // {
    //     fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
    //     return (vkResult);
    // }
    // else
    // {
    //     fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkMapMemory() successful \n");
    // }

    // if (1024 == meshWidth && 1024 == meshHeight)
    // {
    //     memcpy(data, pos_1024_graphics, size);
    // }

    // vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

    // COMPLETELY NEW CODE

    HANDLE hMemoryWin32Handle = NULL;

    VkMemoryGetWin32HandleInfoKHR vkMemoryGetWin32HandleInfoKHR;
    memset((void *)&vkMemoryGetWin32HandleInfoKHR, 0, sizeof(VkMemoryGetWin32HandleInfoKHR));
    vkMemoryGetWin32HandleInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
    vkMemoryGetWin32HandleInfoKHR.pNext = NULL;
    vkMemoryGetWin32HandleInfoKHR.memory = vertexData_position.vkDeviceMemory;
    vkMemoryGetWin32HandleInfoKHR.handleType = vkExternalMemoryHandleTypeFlagBits;

    PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(vkDevice, "vkGetMemoryWin32HandleKHR");
    if(NULL == vkGetMemoryWin32HandleKHR)
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkGetMemoryWin32HandleKHR() not found\n");
        return(VK_ERROR_INITIALIZATION_FAILED);
    }

    // Call above function using function pointer
    vkResult = vkGetMemoryWin32HandleKHR(vkDevice, &vkMemoryGetWin32HandleInfoKHR, &hMemoryWin32Handle);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkGetMemoryWin32HandleKHR() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  vkGetMemoryWin32HandleKHR() successful \n");
    }

    // Map above external buffer's memory into OpenCL to get the device pointer from OpenCL
    cl_mem_properties oclMemoryProperties[] = {
        CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KHR, (cl_mem_properties)hMemoryWin32Handle,
        // List Start
        CL_MEM_DEVICE_HANDLE_LIST_KHR, (cl_mem_properties)oclDeviceID,
        CL_MEM_DEVICE_HANDLE_LIST_END_KHR, 0
    };

    // Create OpenCL Compatible External Buffer from above Vulkan Buffer
    posOpenCL = clCreateBufferWithProperties(   oclContext, 
                                                oclMemoryProperties, 
                                                CL_MEM_READ_WRITE, 
                                                vkMemoryRequirements.size, 
                                                NULL,
                                                &oclResult);
    if(CL_SUCCESS != oclResult)
    {
        fprintf(gpLogFile, "CreateExternalVertexBuffer()    =>  clCreateBufferWithProperties() failed (%d)\n", oclResult);
        return(VK_ERROR_INITIALIZATION_FAILED);
    }

    // Close the handle
    CloseHandle(hMemoryWin32Handle);
    hMemoryWin32Handle = NULL;

    *pVertexData = vertexData_position;

    return(vkResult);
}

void InitializeSineWaveArrays(int meshWidth, int meshHeight)
{
    // Code
    for (int ole = 0; ole < meshWidth; ++ole)
    {
        for (int mle = 0; mle < meshHeight; ++mle)
        {
            for (int ile = 0; ile < 4; ++ile)
            {
                pos_1024_graphics[ole][mle][ile] = (float)ile / (float)meshWidth;
            }
        }
    }
}

VkResult CreateUniformBuffer(void)
{
    // Function Declarations
    VkResult UpdateUniformBuffer(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkBufferCreateInfo vkBufferCreateInfo;
    VkMemoryRequirements vkMemoryRequirements;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;

    // Code
    fprintf(gpLogFile, "CreateUniformBuffer()  >>>  function started\n");

    // Create Uniform Buffer
    memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    memset((void *)&uniformData, 0, sizeof(MyUniformData));
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkCreateBuffer() successful \n");
    }

    // Get Buffer Memory Requirements
    vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);

    // Allocate Required Memory
    memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    {
        if (1 == (vkMemoryRequirements.memoryTypeBits & 1))
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = le;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkAllocateMemory() successful \n");
    }

    // Bind Buffer to Memory
    vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkBindBufferMemory() successful \n");
    }

    // Update Uniform Buffer
    vkResult = UpdateUniformBuffer();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  UpdateUniformBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  UpdateUniformBuffer() successful \n");
    }

    // // Map Memory and Copy Data
    // void *data = NULL;
    // vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    // if(VK_SUCCESS != vkResult)
    // {
    //     fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
    //     return (vkResult);
    // }

    // memcpy(data, &myUniformData, sizeof(MyUniformData));
    // vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);

    fprintf(gpLogFile, "CreateUniformBuffer()  -->  function completed\n");
    return (vkResult);
}

VkResult UpdateUniformBuffer(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    void *data = NULL;
    MyUniformData myUniformData;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);

    // Code

    // Initialize Uniform Data
    memset((void *)&myUniformData, 0, sizeof(MyUniformData));

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));

    myUniformData.modelMatrix = modelMatrix;
    myUniformData.viewMatrix = viewMatrix;

    // Create Perspective Projection Matrix
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;
    
    // For Color
    if('K' == colorFromKey)
    {
        // This is the only case where background changes
        myUniformData.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        if('R' == colorFromKey)
        {
            myUniformData.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
        else if ('G' == colorFromKey)
        {
            myUniformData.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        else if ('B' == colorFromKey)
        {
            myUniformData.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        }
        else if ('C' == colorFromKey)
        {
            myUniformData.color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        }
        else if ('M' == colorFromKey)
        {
            myUniformData.color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
        }
        else if ('Y' == colorFromKey)
        {
            myUniformData.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        }
        else if ('W' == colorFromKey)
        {
            myUniformData.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
        else if ('O' == colorFromKey)
        {
            myUniformData.color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
        }
        else
        {
            myUniformData.color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
        }
    }

    // Map Memory and Copy Data
    vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "UpdateUniformBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(MyUniformData));

    vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);

    return (vkResult);
}

VkResult CreateShaders(void)
{
    // Variables Definition
    VkResult vkResult = VK_SUCCESS;

    // For Vertex Shader
    const char *szFileName = "shader.vert.spv";
    FILE *fp = NULL;
    size_t size = 0;
    char *shaderData = NULL;
    size_t retVal = 0;
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateShaders()  >>>  function started\n");

    // For Vertex Shader
    fp = fopen(szFileName, "rb");
    if (NULL == fp)
    {
        fprintf(gpLogFile, "CreateShaders()  -->  failed to open vertex shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateShaders()  -->  successful to open vertex shader spv file\n");
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    if (0 == size)
    {
        fprintf(gpLogFile, "CreateShaders()  -->  size is 0 of vertex shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    fseek(fp, 0L, SEEK_SET);

    shaderData = (char *)malloc(sizeof(char) * size);

    retVal = fread(shaderData, size, 1, fp);
    if (1 != retVal)
    {
        fprintf(gpLogFile, "CreateShaders()  -->  failed to read vertex shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateShaders()  -->  successful to read vertex shader spv file\n");
    }

    fclose(fp);

    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //  RESERVED so must be 0
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateShaders()    =>  vkCreateShaderModule() failed for vertex shader (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateShaders()    =>  vkCreateShaderModule() successful for vertex shader \n");
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        fprintf(gpLogFile, "CreateShaders()    =>  vertex shader data successfully freed \n");
    }

    fprintf(gpLogFile, "CreateShaders()    =>  vertex shader module successfully created \n");

    // For fragment shader
    szFileName = "shader.frag.spv";

    fp = NULL;
    fp = fopen(szFileName, "rb");
    if (NULL == fp)
    {
        fprintf(gpLogFile, "CreateShaders()  -->  failed to open fragment shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateShaders()  -->  successful to open fragment shader spv file\n");
    }

    fseek(fp, 0L, SEEK_END);
    size = 0;
    size = ftell(fp);
    if (0 == size)
    {
        fprintf(gpLogFile, "CreateShaders()  -->  size is 0 of fragment shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    fseek(fp, 0L, SEEK_SET);

    shaderData = NULL;
    shaderData = (char *)malloc(sizeof(char) * size);

    retVal = 0;
    retVal = fread(shaderData, size, 1, fp);
    if (1 != retVal)
    {
        fprintf(gpLogFile, "CreateShaders()  -->  failed to read fragment shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateShaders()  -->  successful to read fragment shader spv file\n");
    }

    fclose(fp);

    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //  RESERVED so must be 0
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateShaders()    =>  vkCreateShaderModule() failed for fragment shader (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateShaders()    =>  vkCreateShaderModule() successful for fragment shader \n");
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        fprintf(gpLogFile, "CreateShaders()    =>  fragment shader data successfully freed \n");
    }

    fprintf(gpLogFile, "CreateShaders()    =>  fragment shader module successfully created \n");

    fprintf(gpLogFile, "CreateShaders()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateDescriptorSetLayout(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;

    // Initialize Descriptor Set Binding
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;

    // Code
    fprintf(gpLogFile, "CreateDescriptorSetLayout()  >>>  function started\n");

    memset((void *)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));

    vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //  This is the type of descriptor we are going to use
    vkDescriptorSetLayoutBinding.binding = 0;                                        //  This is actually the 0 binding point of shader
    vkDescriptorSetLayoutBinding.descriptorCount = 1;
    vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;

    memset((void *)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0; // RESERVED
    vkDescriptorSetLayoutCreateInfo.bindingCount = 1;
    vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;

    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateDescriptorSetLayout()    =>  vkCreateDescriptorSetLayout() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateDescriptorSetLayout()    =>  vkCreateDescriptorSetLayout() successful \n");
    }

    fprintf(gpLogFile, "CreateDescriptorSetLayout()  -->  function completed\n");
    return (vkResult);
}

VkResult CreatePipelineLayout(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;

    // Code
    fprintf(gpLogFile, "CreatePipelineLayout()  >>>  function started\n");

    memset((void *)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreatePipelineLayout()    =>  vkCreatePipelineLayout() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreatePipelineLayout()    =>  vkCreatePipelineLayout() successful \n");
    }

    fprintf(gpLogFile, "CreatePipelineLayout()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateDescriptorPool(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkDescriptorPoolSize vkDescriptorPoolSize;
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateDescriptorPool()  >>>  function started\n");

    // Before Creating Actual Descriptor Pool. Vulkan Expects Descriptor pool size.
    memset((void *)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));

    vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize.descriptorCount = 1;

    // Create The Pool
    memset((void *)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = 1;
    vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
    vkDescriptorPoolCreateInfo.maxSets = 1;

    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateDescriptorPool()    =>  vkCreateDescriptorPool() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateDescriptorPool()    =>  vkCreateDescriptorPool() successful \n");
    }

    fprintf(gpLogFile, "CreateDescriptorPool()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateDescriptorSet(void)
{
    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    VkWriteDescriptorSet vkWriteDescriptorSet;

    // Code
    fprintf(gpLogFile, "CreateDescriptorSet()  >>>  function started\n");

    memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() successful \n");
    }

    // Describe if we want Buffer as Uniform or Image as Uniform

    // Specify Buffer Info
    memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);

    /**
     *
     * Now update the above descriptor set to the shader
     *
     * We can write or copy to shader
     *
     * We will prefer write descriptor set
     *
     */

    // Now connect the descriptor set to the actual buffer using write descriptor set
    memset((void *)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));

    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.pNext = NULL;
    vkWriteDescriptorSet.dstSet = vkDescriptorSet;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet.pImageInfo = NULL;
    vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet.pTexelBufferView = NULL;
    vkWriteDescriptorSet.dstBinding = 0;

    // ISKA KATA HUA PAANI BHI NAHI MANGTA HAI
    vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

    fprintf(gpLogFile, "CreateDescriptorSet()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateRenderPass(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    // VkAttachmentDescription vkAttachmentDescription_array[1];
    VkAttachmentDescription vkAttachmentDescription_array[2]; // For Color and Depth
    VkAttachmentReference vkAttachmentReference_color;
    VkAttachmentReference vkAttachmentReference_depth;
    VkSubpassDescription vkSubpassDescription;
    VkRenderPassCreateInfo vkRenderPassCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateRenderPass()  >>>  function started\n");

    memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * ARRAYSIZE(vkAttachmentDescription_array));

    // For Color
    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // For Depth
    vkAttachmentDescription_array[1].flags = 0;
    vkAttachmentDescription_array[1].format = vkFormat_depth;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // For Color
    memset((void *)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_color.attachment = 0; //  index of attachment of above array
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // For Depth
    memset((void *)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_depth.attachment = 1; //  index of attachment of above array
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    // vkSubpassDescription.colorAttachmentCount = ARRAYSIZE(vkAttachmentDescription_array);
    vkSubpassDescription.colorAttachmentCount = 1; // This count should be the count of vkAttachmentReference used for color
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;

    memset((void *)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;

    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateRenderPass()    =>  vkCreateRenderPass() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateRenderPass()    =>  vkCreateRenderPass() successful \n");
    }

    fprintf(gpLogFile, "CreateRenderPass()  -->  function completed\n");
    return (vkResult);
}

VkResult CreatePipeline(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;

    // Code
    fprintf(gpLogFile, "CreatePipeline()  >>>  function started\n");

    // VkVertexInputBindingDescription
    memset((void *)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * ARRAYSIZE(vkVertexInputBindingDescription_array));
    vkVertexInputBindingDescription_array[0].binding = 0;
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 4;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // VkVertexInputAttributeDescription
    memset((void *)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * ARRAYSIZE(vkVertexInputAttributeDescription_array));

    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;

    // Vertex Input State
    memset((void *)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    // Input Assembly State
    memset((void *)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    //  vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = ;

    // Rasterization/Rasterizer State
    memset((void *)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

    // Color Blend State
    memset((void *)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; //  NO VALIDATION | NO ERROR | NO OUTPUT =

    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;

    memset((void *)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

    // Depth Stencil State
    memset((void *)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));

    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back;
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

    // Viewport Scissor State
    memset((void *)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;

    memset((void *)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0;
    vkViewport.y = 0;
    vkViewport.width = (float)vkExtent2D_swapchain.width;
    vkViewport.height = (float)vkExtent2D_swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    vkPipelineViewportStateCreateInfo.viewportCount = 1; // i.e we can specify multiple viewports as array -> MultiViewPort Assignment
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;

    memset((void *)&vkRect2D_scissor, 0, sizeof(VkRect2D));
    vkRect2D_scissor.offset.x = 0;
    vkRect2D_scissor.offset.y = 0;
    vkRect2D_scissor.extent.width = vkExtent2D_swapchain.width;
    vkRect2D_scissor.extent.height = vkExtent2D_swapchain.height;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;

    // Depth Stencil State
    // As we dont have depth yet, we can omit this state

    // Dynamic State
    // We dont have any dynamic state

    // Multi Sample State
    memset((void *)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // If this is not specified then it will give validation error

    // Shader State
    memset((void *)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

    // Vertex Shader
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL; // This is very important and if NULL not specified then we get Validation Error
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

    // Fragment Shader
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL; // This is very important and if NULL not specified then we get Validation Error
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main";
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;

    // Tessellation State
    // We Dont have tessellation shaders so we can omit this state

    // As Pipelines are created from Pipeline Cache, so now we will create pipeline objects
    memset((void *)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;

    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreatePipeline()    =>  vkCreatePipelineCache() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreatePipeline()    =>  vkCreatePipelineCache() successful \n");
    }

    // Create Graphics Pipeline
    memset((void *)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0;
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

    // Now create the pipeline
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);
    if (VK_SUCCESS != vkResult)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        fprintf(gpLogFile, "CreatePipeline()  -->  vkDestroyPipelineCache() successful\n");

        fprintf(gpLogFile, "CreatePipeline()    =>  vkCreateGraphicsPipelines() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreatePipeline()    =>  vkCreateGraphicsPipelines() successful \n");
    }

    // We are done using vkPipelineCache so destroy it
    if (vkPipelineCache)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        fprintf(gpLogFile, "CreatePipeline()  -->  vkDestroyPipelineCache() successful\n");
    }

    fprintf(gpLogFile, "CreatePipeline()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateFrameBuffers(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    // VkFramebufferCreateInfo vkFramebufferCreateInfo;
    // VkImageView vkImageView_attachment_array[1];

    // Code
    fprintf(gpLogFile, "CreateFrameBuffers()  >>>  function started\n");

    // memset((void *)vkImageView_attachment_array, 0, ARRAYSIZE(vkImageView_attachment_array));

    // memset((void *)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

    // vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    // vkFramebufferCreateInfo.pNext = NULL;
    // vkFramebufferCreateInfo.flags = 0;
    // vkFramebufferCreateInfo.renderPass = vkRenderPass;
    // vkFramebufferCreateInfo.attachmentCount = ARRAYSIZE(vkImageView_attachment_array);
    // vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
    // vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
    // vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
    // vkFramebufferCreateInfo.layers = 1; //  VALIDATION ||> CASE 02

    vkFramebuffer_array = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkFramebuffer_array)
    {
        fprintf(gpLogFile, "CreateFrameBuffers()    =>  malloc failed for vkFramebuffer_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        // VkImageView vkImageView_attachment_array[1];
        VkImageView vkImageView_attachment_array[2]; // For Color and Depth Attachments
        VkFramebufferCreateInfo vkFramebufferCreateInfo;

        memset((void *)vkImageView_attachment_array, 0, ARRAYSIZE(vkImageView_attachment_array));
        memset((void *)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = NULL;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = vkRenderPass;
        vkFramebufferCreateInfo.attachmentCount = ARRAYSIZE(vkImageView_attachment_array);
        vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
        vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
        vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
        vkFramebufferCreateInfo.layers = 1; //  VALIDATION ||> CASE 02

        vkImageView_attachment_array[0] = swapchainImageView_array[le];
        vkImageView_attachment_array[1] = vkImageView_depth;

        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateFrameBuffers()    =>  vkCreateFramebuffer() failed for iteration %d. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateFrameBuffers()    =>  vkCreateFramebuffer() successful for iteration %d \n", le);
        }
    }

    fprintf(gpLogFile, "CreateFrameBuffers()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateSemaphores(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateSemaphores()  >>>  function started\n");

    memset((void *)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;
    vkSemaphoreCreateInfo.flags = 0; //  MUST BE 0, BECAUSE OF RESERVED

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backbuffer);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSemaphores()    =>  vkCreateSemaphore() failed for vkSemaphore_backbuffer %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSemaphores()    =>  vkCreateSemaphore() successful for vkSemaphore_backbuffer \n");
    }

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_rendercomplete);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateSemaphores()    =>  vkCreateSemaphore() failed for vkSemaphore_rendercomplete %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateSemaphores()    =>  vkCreateSemaphore() successful for vkSemaphore_rendercomplete \n");
    }

    fprintf(gpLogFile, "CreateSemaphores()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateFences(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkFenceCreateInfo vkFenceCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateFences()  >>>  function started\n");

    memset((void *)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkFence_array = (VkFence *)malloc(sizeof(VkFence) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkFence_array)
    {
        fprintf(gpLogFile, "CreateFences()    =>  malloc failed for vkFence_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateFences()    =>  vkCreateFence() failed for iteration %d (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateFences()    =>  vkCreateFence() successful for iteration %d \n", le);
        }
    }

    fprintf(gpLogFile, "CreateFences()  -->  function completed\n");
    return (vkResult);
}

VkResult BuildCommandBuffers(void)
{
    // Function Declaration
    VkResult PrepareSinWaveForCPU(unsigned int, unsigned int, float);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkCommandBuffer *vkCommandBuffer_array = NULL;

    // Code
    // fprintf(gpLogFile, "BuildCommandBuffers()  >>>  function started\n");

    if(TRUE == bMesh1024Chosen)
    {
        // PrepareSinWaveForCPU(1024, 1024, animationTime);
        vkCommandBuffer_array = vkCommandBuffer_for_1024x1024_graphics_array;
    }

    // Loop Per Swapchain Image
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        // Reset Command Buffers
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[le], 0); // 0 means -> Dont release the resources created by command pool for this command buffers
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "BuildCommandBuffers()    =>  vkResetCommandBuffer() failed for %d buffer. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "BuildCommandBuffers()    =>  vkResetCommandBuffer() successful for %d buffer\n", le);
        }

        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void *)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0; // 0=> i. Use only Primary Command Buffers and ii. We are not going to use this buffer simultaneously between multiple threads

        // Begin Command Buffer Recording
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[le], &vkCommandBufferBeginInfo);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "BuildCommandBuffers()    =>  vkBeginCommandBuffer() failed for %d buffer. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "BuildCommandBuffers()    =>  vkBeginCommandBuffer() successful for %d buffer\n", le);
        }

        // Set Clear Values
        VkClearValue vkClearValue_array[2];
        memset((void *)vkClearValue_array, 0, sizeof(VkClearValue) * ARRAYSIZE(vkClearValue_array));

        if('K' == colorFromKey)
        {
            VkClearColorValue vkClearColorValue_white = {1.0f, 1.0f, 1.0f, 1.0f};
            vkClearValue_array[0].color = vkClearColorValue_white;
        }
        else
        {
            vkClearValue_array[0].color = vkClearColorValue;
        }
        
        vkClearValue_array[1].depthStencil = vkClearDepthStencilValue;

        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void *)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[le];

        vkCmdBeginRenderPass(vkCommandBuffer_array[le], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // Bind with the pipeline
            vkCmdBindPipeline(vkCommandBuffer_array[le], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

            // Bind Descriptor Set to Pipeline
            vkCmdBindDescriptorSets(vkCommandBuffer_array[le],
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    vkPipelineLayout,
                                    0, // firstSet
                                    1, // descriptorSetCount
                                    &vkDescriptorSet,
                                    0,     // dynamicOffsetCount
                                    NULL); // pDynamicOffsets

            // Bind with the Vertex Buffer
            VkDeviceSize vkDeviceSize_offset_array[1];

            memset((void *)vkDeviceSize_offset_array, 0, sizeof(VkDeviceSize) * ARRAYSIZE(vkDeviceSize_offset_array));

            vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 0, 1, &vertex_positon_1024x1024_graphics.vkBuffer, vkDeviceSize_offset_array);

            //******************************************************************************************************
            // HERE WE SHOULD CALL VULKAN DRAWING FUNCTION
            //******************************************************************************************************

            /*********** BEGIN DRAWING ***********/

            if(TRUE == bMesh1024Chosen)
            {
                if(FALSE == bOnGpu)
                {
                    vkCmdBindVertexBuffers( vkCommandBuffer_array[le], 
                                            0, 
                                            1, 
                                            &vertex_positon_1024x1024_graphics.vkBuffer, 
                                            vkDeviceSize_offset_array);
                }
                else
                {
                    vkCmdBindVertexBuffers( vkCommandBuffer_array[le], 
                                            0, 
                                            1, 
                                            &vertexDataExternal.vkBuffer, 
                                            vkDeviceSize_offset_array);   
                }

                vkCmdDraw(  vkCommandBuffer_array[le], 
                            1024 * 1024, 
                            1, 
                            0, 
                            0);
            }

            /************ END DRAWING ************/
        }
        vkCmdEndRenderPass(vkCommandBuffer_array[le]);

        // End Command Buffer Recording
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "BuildCommandBuffers()    =>  vkEndCommandBuffer() failed for %d buffer. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "BuildCommandBuffers()    =>  vkEndCommandBuffer() successful for %d buffer\n", le);
        }
    }

    fprintf(gpLogFile, "BuildCommandBuffers()  -->  function completed\n");
    return (vkResult);
}

VkResult PrepareSinWaveForCPU(unsigned int width, unsigned int height, float animTime)
{
    // Function 
    void FillSinWaveArraysForCpu(unsigned int, unsigned int, float);

    // Local Variable
    VkResult vkResult = VK_SUCCESS;
    void* data = NULL;
    unsigned int size = width * height * 4 * sizeof(float);

    // Code

    // Fill Sine Wave Array
    FillSinWaveArraysForCpu(width, height, animTime);

    // Map the buffers
    if(1024 == width && 1024 == height)
    {
        vkResult = vkMapMemory(vkDevice, vertex_positon_1024x1024_graphics.vkDeviceMemory, 0, size, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "PrepareSinWaveForCPU()    =>  vkMapMemory() failed. (%d)\n", vkResult);
            return (vkResult);
        }

        memcpy(data, pos_1024_graphics, size);

        vkUnmapMemory(vkDevice, vertex_positon_1024x1024_graphics.vkDeviceMemory);
    }

    return (vkResult);
}

void FillSinWaveArraysForCpu(unsigned int width, unsigned int height, float animTime)
{
    // Code
    for(int ole = 0; ole < (int)width; ole++)
    {
        for(int mle = 0; mle < (int)height; mle++)
        {
            for(int ile = 0; ile < (int)4; ile++)
            {
                float amplitude = 0.5f;
                float u = (float)ole / (float)width;
                float v = (float)mle / (float)height;
                u = u * 2.0f - 1.0f;
                v = v * 2.0f - 1.0f;
                float frequency = 4.0f;
                float w = sinf(frequency * u + animTime) * cosf(frequency * v + animTime);
                if(0 == ile)
                {
                    if(1024 == width && 1024 == height)
                    {
                        pos_1024_graphics[ole][mle][ile] = u;
                    }
                }
                if(1 == ile)
                {
                    if(1024 == width && 1024 == height)
                    {
                        pos_1024_graphics[ole][mle][ile] = w * amplitude;
                    }
                }
                if(2 == ile)
                {
                    if(1024 == width && 1024 == height)
                    {
                        pos_1024_graphics[ole][mle][ile] = v;
                    }
                }
                if(3 == ile)
                {
                    if(1024 == width && 1024 == height)
                    {
                        pos_1024_graphics[ole][mle][ile] = 1.0f;
                    }
                }
            }
        }
    }
}


/***********************************************************************************\
                                ALWAYS LAST FUNCTION
\***********************************************************************************/
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT,
                                                   VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
                                                   uint64_t object,
                                                   size_t location,
                                                   int32_t messageCode,
                                                   const char *pLayerPrefix,
                                                   const char *pMessage,
                                                   void *pUserData)
{
    // Code
    // fprintf(gpLogFile, "DebugReportCallback()  ||>  function started\n");

    fprintf(gpLogFile, "MST_VALIDATION : DebugReportCallback()  ||>     %s %d = %s\n", pLayerPrefix, messageCode, pMessage);

    // fprintf(gpLogFile, "DebugReportCallback()  ||>  function completed\n");

    return (VK_FALSE);
}

/***********************************************************************************\
\***********************************************************************************/
