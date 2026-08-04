/**************************************************
 * S.G.M.P
 * @file: vk.cpp
 * @brief: Linux - Vulkan Smiley
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: 04 August 2026 (Tuesday) 07:55:00 PM
 **************************************************/

// Standard C library headers
#include <stdio.h>  // for file IO
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()
#include <stdarg.h> // for va_list, va_start, va_end

// Xlib headers
#include <X11/Xlib.h>   // X Window System
#include <X11/Xutil.h>  // for visual info
#include <X11/XKBlib.h> // for keyboard input handling (XkbKeyCodeToKeysym) (KeyCode to KeySym conversion)
#include <X11/keysym.h> // for key symbols
#include <X11/Xatom.h>  // for XA_ATOM

// Vulkan Headers
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h> //  We must tell the header the platform we are on

// header file for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM related macros and headers files
// GLM follows column major order just like OpenGL
// BUT unlike OpenGL, GLM's matrix arrays are 4x4 2D arrays not 1D arrays of 16 elements. VVVIMP
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_0_TO_1
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Macros
#define     WIN_WIDTH   800
#define     WIN_HEIGHT  600
#define     fprintf(stream, ...) mylog((stream), __VA_ARGS__)
#define     ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))

static int mylog(FILE *stream, const char *format, ...)
{
    va_list args;
    int written;

    va_start(args, format);
    written = vfprintf(stream, format, args);
    va_end(args);

    if (stream)
    {
        fflush(stream);
    }

    return written;
}

// Global variables
const char* gpszAppName = "ARTR"; // Application name
const char* gpszWindowTitle = "MST: Linux - Vulkan Smiley"; // Window title

// Global variables for X Window System
Display* gpDisplay = NULL; // Pointer to the X display
XVisualInfo* gpXVisualInfo = NULL; // Pointer to the X visual info
Colormap gColormap; // Colormap for the window
Window gWindow; // Window handle

// Global variables for window dimensions
int giWindowWidth = WIN_WIDTH; // Window width
int giWindowHeight = WIN_HEIGHT; // Window height

// Global flags
Bool gbActiveWindow = False; // Active window flag
Bool gbEscapeKeyPressed = False; // Escape key pressed flag
Bool gbFullscreen = False; // Fullscreen flag
Bool gbWindowMinimized = False; // Window minimized flag

// Global variable for log file
FILE* gpLogFile = NULL; // Log file pointer


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
const char *enabledDeviceExtensionNames_array[1];

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

// Step 13 - Swapchain Images and ImageViews
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
VkCommandBuffer *vkCommandBuffer_array = NULL;

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
Bool bInitialized = False;
uint32_t currentImageIndex = UINT32_MAX; //  or UINT_MAX

// Step 21 - Validation
Bool bValidation = True; // This should be consistent with X11 Bool type which uses True/False
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

// Triangle Vertex Data
VertexData vertexData_position_pyramid;
VertexData vertexData_texcoords_pyramid;

// Rectangle Vertex Data
VertexData vertexData_position_cube;
VertexData vertexData_texcoords_cube;

// Uniform related Buffers
typedef struct
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
} MyUniformData;

typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} UniformData;

UniformData uniformData_pyramid;
UniformData uniformData_cube;

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
VkDescriptorSet vkDescriptorSet_pyramid = VK_NULL_HANDLE;
VkDescriptorSet vkDescriptorSet_cube = VK_NULL_HANDLE;

// Step 26 Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

// For Rotation
float rotate_angle = 0.0f;

// For Texture related operations
VkImage vkImage_texture_stone = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture_stone = VK_NULL_HANDLE;
VkImageView vkImageView_texture_stone = VK_NULL_HANDLE;
VkSampler vkSampler_texture_stone = VK_NULL_HANDLE;

VkImage vkImage_texture_smiley = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture_smiley = VK_NULL_HANDLE;
VkImageView vkImageView_texture_smiley = VK_NULL_HANDLE;
VkSampler vkSampler_texture_smiley = VK_NULL_HANDLE;

// Entry point function
int main(int argc, char* argv[])
{
    // Function declarations
    void ToggleFullscreen(void);
    Bool isWindowMinimized(void);
    VkResult initialize(void);
    VkResult resize(int, int);
    VkResult display(void);
    void update(void);
    void uninitialize(void);

    // Local Variables
    XVisualInfo xVisualInfo; // Visual info structure
    int iNumFBConfigs = 0; // Number of framebuffer configurations
    XSetWindowAttributes windowAttributes; // Window attributes structure
    int defaultScreen; // Default screen number
    int defaultDepth; // Default depth of the screen
    int styleMask; // Style mask for window attributes
    Atom windowManagerDeleteAtom; // Atom for window manager delete message
    int screenWidth; // Screen width
    int screenHeight; // Screen height
    XEvent event; // X event structure
    KeySym keySym; // Key symbol
    char keys[26]; // Array to hold key input
    Bool bDone = False; // Loop control flag

    // Code
    gpLogFile = fopen("log.txt", "w");
    if (NULL == gpLogFile)
    {
        printf("main() -> Log file creation failed. Exiting...\n");
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "main() -> Log file created successfully.\n");
    }

    // Open Display
    gpDisplay = XOpenDisplay(NULL);
    if (NULL == gpDisplay)
    {
        fprintf(gpLogFile, "main() -> XOpenDisplay() failed. Exiting...\n");
        uninitialize();
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "main() -> XOpenDisplay() succeeded. Display: %s\n", XDisplayName(NULL));
    }

    // Get default screen and depth
    defaultScreen = XDefaultScreen(gpDisplay);
    defaultDepth = XDefaultDepth(gpDisplay, defaultScreen);

    // Initialize local XVisualInfo
    memset(&xVisualInfo, 0, sizeof(XVisualInfo));
    xVisualInfo.screen = defaultScreen;

    gpXVisualInfo = XGetVisualInfo(gpDisplay, VisualScreenMask, &xVisualInfo, &iNumFBConfigs);
    if (NULL == gpXVisualInfo)
    {
        fprintf(gpLogFile, "main() -> XGetVisualInfo() failed. Exiting...\n");
        uninitialize();
        exit(1);
    }   
    else
    {
        fprintf(gpLogFile, "main() -> XGetVisualInfo() succeeded. Visual ID: 0x%lx\n", (unsigned long)gpXVisualInfo->visualid);
    }
    fprintf(gpLogFile, "main() -> Found no of matching FBConfigs: %d\n", iNumFBConfigs);

    // Create Colormap
    gColormap = XCreateColormap(gpDisplay, XRootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone);
    if (0 == gColormap)
    {
        fprintf(gpLogFile, "main() -> XCreateColormap() failed. Exiting...\n");
        uninitialize();
        exit(1);
    }

    // Initialize window attributes
    memset((void*)&windowAttributes, 0, sizeof(XSetWindowAttributes));
    
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(gpDisplay, defaultScreen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = gColormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | FocusChangeMask | VisibilityChangeMask | PropertyChangeMask;
    
    // Set style mask for window attributes 
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    // Create Window
    gWindow = XCreateWindow(gpDisplay, 
                            XRootWindow(gpDisplay, gpXVisualInfo->screen),
                            0,
                            0,
                            WIN_WIDTH,
                            WIN_HEIGHT,
                            0,
                            gpXVisualInfo->depth,
                            InputOutput,
                            gpXVisualInfo->visual,
                            styleMask,
                            &windowAttributes);
    if(0 == gWindow)
    {
        fprintf(gpLogFile, "main() -> XCreateWindow() failed. Exiting...\n");
        uninitialize();
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "main() -> XCreateWindow() succeeded. Window ID: 0x%lx\n", (unsigned long)gWindow);
    }

    // Set window title
    XStoreName(gpDisplay, gWindow, gpszWindowTitle);

    // Prepare to handle window close event from window manager
    windowManagerDeleteAtom = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDeleteAtom, 1);

    // Map the window to make it visible
    XMapWindow(gpDisplay, gWindow);

    // // Get screen dimensions
    // screenWidth = XWidthOfScreen(XScreenOfDisplay(gpDisplay, gpXVisualInfo->screen));
    // screenHeight = XHeightOfScreen(XScreenOfDisplay(gpDisplay, gpXVisualInfo->screen));

    // // Center the window on the screen
    // XMoveWindow(gpDisplay, 
    //             gWindow,
    //             (screenWidth / 2 - WIN_WIDTH / 2),
    //             (screenHeight / 2 - WIN_HEIGHT / 2));

    // Initialize Vulkan here
    VkResult vkResult = initialize();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "main() -> initialize() failed with error code: %d. Exiting...\n", vkResult);
        uninitialize();
        exit(1);
    }
    else
    {
        fprintf(gpLogFile, "main() -> initialize() succeeded.\n");
    }

    // Event loop
    while (False == bDone)
    {
        while(XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);
            switch(event.type)
            {
                case MapNotify:
                    break;

                case FocusIn:   // WM_SETFOCUS
                    gbActiveWindow = True;
                    break;

                case FocusOut:  // WM_KILLFOCUS 
                    gbActiveWindow = False;
                    break;

                case KeyPress:
                    keySym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                    switch(keySym)
                    {
                        case XK_Escape:
                            gbEscapeKeyPressed = True;
                            bDone = True;
                            break;
                    }

                    XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                    switch(keys[0])
                    {
                        case 'F':
                        case 'f':
                            if(gbFullscreen == False)
                            {
                                ToggleFullscreen();
                                gbFullscreen = True;
                            }
                            else
                            {
                                ToggleFullscreen();
                                gbFullscreen = False;
                            }

                        default:
                            break;
                    }
                    break;

                    case ConfigureNotify:   //  WM_SIZE
                        giWindowWidth = event.xconfigure.width;
                        giWindowHeight = event.xconfigure.height;
                        vkResult = resize(giWindowWidth, giWindowHeight);
                        if (VK_SUCCESS != vkResult)
                        {
                            fprintf(gpLogFile, "main() -> resize() failed with error code: %d. Exiting...\n", vkResult);
                            uninitialize();
                            exit(1);
                        }
                        break;

                    case PropertyNotify:  // WM_ACTIVATE
                        if(True == isWindowMinimized())
                        {
                            gbWindowMinimized = True;
                            fprintf(gpLogFile, "main() -> Window is minimized.\n");
                        }
                        else
                        {
                            gbWindowMinimized = False;
                            fprintf(gpLogFile, "main() -> Window is not minimized.\n");
                        }
                        break;

                    case DestroyNotify:  // WM_DESTROY
                        break;

                    case 33:    // WM_CLOSE
                        bDone = True;
                        break;

                    default:
                        break;
            }
        }

        if (True == gbActiveWindow)
        {
            if(True == gbEscapeKeyPressed)
            {
                bDone = True;
            }

            if (False == gbWindowMinimized)
            {
                update();
                vkResult = display();
                if (VK_FALSE != vkResult && VK_SUCCESS != vkResult)
                {
                    fprintf(gpLogFile, "main() -> display() failed with error code: %d. Exiting...\n", vkResult);
                    uninitialize();
                    exit(1);
                }
            }
        }
    }
    
    uninitialize();

    return(0);
}

// Function to toggle fullscreen mode
void ToggleFullscreen(void) 
{
    // Variable Declarations
    Atom    wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    Atom    fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    XEvent  event = {0};

    // Code
    event.type = ClientMessage;
    event.xclient.window = gWindow;
    event.xclient.message_type = wm_state;
    event.xclient.format = 32;
    event.xclient.data.l[0] = gbFullscreen ? 0 : 1;
    event.xclient.data.l[1] = fullscreen;

    XSendEvent( gpDisplay,
                RootWindow(gpDisplay, gpXVisualInfo->screen),
                False,
                StructureNotifyMask,
                &event);
}

// Function to check if the window is minimized
Bool isWindowMinimized(void)
{
    // Function declarations
    void uninitialize(void);
    
    // Local Variable Declarations
    Bool windowMinimized = False;
    int iResult = -1;

    Atom returned_property_type = None;
    int returned_property_format = -1;
    unsigned long number_of_returned_items = 0;
    unsigned long number_of_bytes_remained = 0;
    Atom* returned_property_data_array = NULL;

    Atom wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", True);
    if(None == wm_state)
    {
        fprintf(gpLogFile, "isWindowMinimized() -> XInternAtom() failed for _NET_WM_STATE. Exiting...\n");
        uninitialize();
        exit(1);
    }

    Atom wm_state_hidden = XInternAtom(gpDisplay, "_NET_WM_STATE_HIDDEN", True);
    if(None == wm_state_hidden)
    {
        fprintf(gpLogFile, "isWindowMinimized() -> XInternAtom() failed for _NET_WM_STATE_HIDDEN. Exiting...\n");
        uninitialize();
        exit(1);
    }

    iResult = XGetWindowProperty(gpDisplay,
                                 gWindow,
                                 wm_state,
                                 0L,
                                 1024L,
                                 False,
                                 XA_ATOM,
                                 &returned_property_type,
                                 &returned_property_format,
                                 &number_of_returned_items,
                                 &number_of_bytes_remained,
                                 (unsigned char**)&returned_property_data_array);
    if(Success != iResult || NULL == returned_property_data_array)
    {
        if(NULL != returned_property_data_array)
        {
            XFree(returned_property_data_array);
            returned_property_data_array = NULL;
        }
        return(False);
    }
    else
    {
        // Loop the returned property data array to check if _NET_WM_STATE_HIDDEN is present
        for(unsigned long le = 0; le < number_of_returned_items; le++)
        {
            // Check whether the returned array contains _NET_WM_STATE_HIDDEN atom
            if(wm_state_hidden == returned_property_data_array[le])
            {
                windowMinimized = True;
                break;
            }
        }
    }
    
    if(NULL != returned_property_data_array)
    {
        XFree(returned_property_data_array);
        returned_property_data_array = NULL;
    }
    
    return(windowMinimized);
}

VkResult initialize(void)
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
    VkResult CreateCommandBuffers(void);
    VkResult CreateVertexBuffer(void);
    VkResult CreateTexture(const char *, VkImage *, VkDeviceMemory *, VkImageView *, VkSampler *);
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

    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (True == bValidation)
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
    vkResult = CreateCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateCommandBuffers() successful\n");
    }

    // Create Vertex Buffer
    vkResult = CreateVertexBuffer();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVertexBuffer() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateVertexBuffer() successful\n");
    }

    // Create Stone Texture
    // vkResult = CreateTexture("./Stone.png", &vkImage_texture_stone, &vkDeviceMemory_texture_stone, &vkImageView_texture_stone, &vkSampler_texture_stone);
    // if (VK_SUCCESS != vkResult)
    // {
    //     fprintf(gpLogFile, "Initialize()    =>  CreateTexture() failed %d for %s\n", vkResult, "./Stone.png");
    //     return (vkResult);
    // }
    // else
    // {
    //     fprintf(gpLogFile, "Initialize()    =>  CreateTexture() successful for %s\n", "./Stone.png");
    // }

    // Create Kundali Texture
    vkResult = CreateTexture("./Smiley.png", &vkImage_texture_smiley, &vkDeviceMemory_texture_smiley, &vkImageView_texture_smiley, &vkSampler_texture_smiley);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateTexture() failed %d for %s\n", vkResult, "./Kundali.png");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  CreateTexture() successful for %s\n", "./Kundali.png");
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

    vkClearDepthStencilValue.depth = 1.0f;
    vkClearDepthStencilValue.stencil = 0;

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
    bInitialized = True;

    fprintf(gpLogFile, "Initialize()    -->  function completed\n");
    return (vkResult);
}

VkResult resize(int width, int height)
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
    VkResult CreateCommandBuffers(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "Resize()    >>>  function started\n");

    if (height < 0)
        height = 1;

    // Check bInitialized
    if (False == bInitialized)
    {
        fprintf(gpLogFile, "Resize()    =>  Initialization yet not completed or failed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // As recreation of swapchain is needed we are going to repeat many steps of initialize again hence set bInitialized to false again
    bInitialized = False;

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
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[le]);
        fprintf(gpLogFile, "Resize()  =>  vkFreeCommandBuffers() successful for %d\n", le);
    }

    if (vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpLogFile, "Resize()  =>  vkCommandBuffer_array freed successfully\n");
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
    //     fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImage() successful for %d\n", le);
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
    vkResult = CreateCommandBuffers();
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

    bInitialized = True;
    fprintf(gpLogFile, "Resize()    -->  function completed\n");
    return (vkResult);
}

VkResult display(void)
{
    // 11 June 2025 (Wednesday) 02:08:24 AM === MI DISPLAY MADHYE AATA AALO FIRST TIME IN VULKAN

    // Function Declaration
    VkResult UpdateUniformBuffer(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code

    // ifcontrol comes here before initialization is completed then return
    if (False == bInitialized)
    {
        fprintf(gpLogFile, "Display()   => Initialization not completed\n");
        return ((VkResult)False);
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
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
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

void update(void)
{
    // Code
    rotate_angle += 0.5f; // Increment the rotation angle for animation
    if(rotate_angle >= 360.0f)
    {
        rotate_angle -= 360.0f; // Keep the angle within [0, 360) range
    }
}

void uninitialize(void)
{
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
        vkDescriptorSet_pyramid = VK_NULL_HANDLE; // Resetting to NULL as it is not used anymore
        vkDescriptorSet_cube = VK_NULL_HANDLE; // Resetting to NULL as it is not used anymore
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
    if (uniformData_pyramid.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData_pyramid.vkDeviceMemory, NULL);
        uniformData_pyramid.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for uniformData_pyramid.vkDeviceMemory\n");
    }

    if (uniformData_pyramid.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData_pyramid.vkBuffer, NULL);
        uniformData_pyramid.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for uniformData_pyramid.vkBuffer\n");
    }

    // Destroy Uniform Buffer
    if (uniformData_cube.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData_cube.vkDeviceMemory, NULL);
        uniformData_cube.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for uniformData_cube.vkDeviceMemory\n");
    }

    if (uniformData_cube.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData_cube.vkBuffer, NULL);
        uniformData_cube.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for uniformData_cube.vkBuffer\n");
    }

    // Destroy Stone texture resources.
    if (vkSampler_texture_stone)
    {
        vkDestroySampler(vkDevice, vkSampler_texture_stone, NULL);
        vkSampler_texture_stone = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySampler() successful for vkSampler_texture_stone\n");
    }

    if (vkImageView_texture_stone)
    {
        vkDestroyImageView(vkDevice, vkImageView_texture_stone, NULL);
        vkImageView_texture_stone = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImageView() successful for vkImageView_texture_stone\n");
    }

    if (vkImage_texture_stone)
    {
        vkDestroyImage(vkDevice, vkImage_texture_stone, NULL);
        vkImage_texture_stone = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImage() successful for vkImage_texture_stone\n");
    }

    if (vkDeviceMemory_texture_stone)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_texture_stone, NULL);
        vkDeviceMemory_texture_stone = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vkDeviceMemory_texture_stone\n");
    }

    // Destroy Kundali texture resources.
    if (vkSampler_texture_smiley)
    {
        vkDestroySampler(vkDevice, vkSampler_texture_smiley, NULL);
        vkSampler_texture_smiley = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroySampler() successful for vkSampler_texture_smiley\n");
    }

    if (vkImageView_texture_smiley)
    {
        vkDestroyImageView(vkDevice, vkImageView_texture_smiley, NULL);
        vkImageView_texture_smiley = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImageView() successful for vkImageView_texture_smiley\n");
    }

    if (vkImage_texture_smiley)
    {
        vkDestroyImage(vkDevice, vkImage_texture_smiley, NULL);
        vkImage_texture_smiley = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyImage() successful for vkImage_texture_smiley\n");
    }

    if (vkDeviceMemory_texture_smiley)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_texture_smiley, NULL);
        vkDeviceMemory_texture_smiley = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vkDeviceMemory_texture_smiley\n");
    }

    // Destroy Color Buffer
    if (vertexData_texcoords_cube.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_texcoords_cube.vkDeviceMemory, NULL);
        vertexData_texcoords_cube.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertexData_texcoords_cube.vkDeviceMemory\n");
    }

    if (vertexData_texcoords_cube.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_texcoords_cube.vkBuffer, NULL);
        vertexData_texcoords_cube.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_texcoords_cube.vkBuffer\n");
    }

    // Vertex Buffer
    if (vertexData_position_cube.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_cube.vkDeviceMemory, NULL);
        vertexData_position_cube.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertexData_position_cube.vkDeviceMemory\n");
    }

    if (vertexData_position_cube.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_cube.vkBuffer, NULL);
        vertexData_position_cube.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_position_cube.vkBuffer\n");
    }

    // Destroy Color Buffer
    if (vertexData_texcoords_pyramid.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_texcoords_pyramid.vkDeviceMemory, NULL);
        vertexData_texcoords_pyramid.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertexData_texcoords_pyramid.vkDeviceMemory\n");
    }

    if (vertexData_texcoords_pyramid.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_texcoords_pyramid.vkBuffer, NULL);
        vertexData_texcoords_pyramid.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_texcoords_pyramid.vkBuffer\n");
    }

    // Vertex Buffer
    if (vertexData_position_pyramid.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_pyramid.vkDeviceMemory, NULL);
        vertexData_position_pyramid.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertexData_position_pyramid.vkDeviceMemory\n");
    }

    if (vertexData_position_pyramid.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_pyramid.vkBuffer, NULL);
        vertexData_position_pyramid.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_position_pyramid.vkBuffer\n");
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
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[le]);
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeCommandBuffers() successful for %d\n", le);
    }

    if (vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpLogFile, "Uninitialize()  =>  vkCommandBuffer_array freed successfully\n");
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

    if (True == gbFullscreen)
    {
        ToggleFullscreen();
        gbFullscreen = False;
    }

    if (gpLogFile)
    {
        fprintf(gpLogFile, "Uninitialize()  -->  program terminated successfully\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
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
    if (True == bValidation)
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

    if (True == bValidation)
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
    if (True == bValidation)
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
    VkBool32 xlibSurfaceExtensionFound = VK_FALSE;
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
        if (0 == strcmp(instanceExtensionNames_array[le], VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
        {
            xlibSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
        }

        // VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            debugReportExtensionFound = VK_TRUE;
            if (True == bValidation)
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

    if (VK_FALSE == xlibSurfaceExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_XLIB_SURFACE_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "FillInstanceExtensionNames()    =>  VK_KHR_XLIB_SURFACE_EXTENSION_NAME found successfully\n");
    }

    if (VK_FALSE == debugReportExtensionFound)
    {
        if (True == bValidation)
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
        if (True == bValidation)
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

    if (True == bValidation)
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
    VkXlibSurfaceCreateInfoKHR vkXlibSurfaceCreateInfoKHR;

    // Code
    fprintf(gpLogFile, "GetSupportedSurface()    >>>  function started\n");

    memset((void *)&vkXlibSurfaceCreateInfoKHR, 0, sizeof(VkXlibSurfaceCreateInfoKHR));

    vkXlibSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    vkXlibSurfaceCreateInfoKHR.pNext = NULL;
    vkXlibSurfaceCreateInfoKHR.flags = 0;
    vkXlibSurfaceCreateInfoKHR.dpy = gpDisplay; // Display pointer from X
    vkXlibSurfaceCreateInfoKHR.window = gWindow; // Window handle from X

    vkResult = vkCreateXlibSurfaceKHR(vkInstance, &vkXlibSurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "GetSupportedSurface()   =>  vkCreateXlibSurfaceKHR() failed\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "GetSupportedSurface()   =>  vkCreateXlibSurfaceKHR() successful\n");
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

    fprintf(gpLogFile, "\n");
    fprintf(gpLogFile, "**************************************************************************\n");
    fprintf(gpLogFile, "                     Vulkan Info\n");
    fprintf(gpLogFile, "**************************************************************************\n");
    fprintf(gpLogFile, "\n");

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

    fprintf(gpLogFile, "\n");
    fprintf(gpLogFile, "**************************************************************************\n");
    fprintf(gpLogFile, "\n");

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
        if (0 == strcmp(deviceExtensionNames_array[le], VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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

    winWidth = vkExtent2D_swapchain.width;
    winHeight = vkExtent2D_swapchain.height;

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
    vkImageCreateInfo.extent.width = vkExtent2D_swapchain.width;
    vkImageCreateInfo.extent.height = vkExtent2D_swapchain.height;
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

    for (uint32_t le = 0; le < sizeof(vkFormat_depth_array) / sizeof(vkFormat_depth_array[0]); le++)
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

VkResult CreateCommandBuffers(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
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

    fprintf(gpLogFile, "CreateCommandBuffers()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateVertexBuffer(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // float pyramid_position[] = {
    //     // Front Face
    //     0.0f, 1.0f, 0.0f,
    //     -1.0f, -1.0f, 1.0f,
    //     1.0f, -1.0f, 1.0f,

    //     // Right Face
    //     0.0f, 1.0f, 0.0f,
    //     1.0f, -1.0f, 1.0f,
    //     1.0f, -1.0f, -1.0f,

    //     // Back Face
    //     0.0f, 1.0f, 0.0f,
    //     1.0f, -1.0f, -1.0f,
    //     -1.0f, -1.0f, -1.0f,

    //     // Left Face
    //     0.0f, 1.0f, 0.0f,
    //     -1.0f, -1.0f, -1.0f,
    //     -1.0f, -1.0f, 1.0f
    // };

    // float pyramid_texcoords[] = {
    //         0.5f,
    //         1.0f,
    //         0.0f,
    //         0.0f,
    //         1.0f,
    //         0.0f,

    //         0.5f,
    //         1.0f,
    //         0.0f,
    //         0.0f,
    //         1.0f,
    //         0.0f,

    //         0.5f,
    //         1.0f,
    //         0.0f,
    //         0.0f,
    //         1.0f,
    //         0.0f,

    //         0.5f,
    //         1.0f,
    //         0.0f,
    //         0.0f,
    //         1.0f,
    //         0.0f,
    // };

    float cube_position[] = {
        // Front
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,

        // Back
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,

        // Right
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,

        // Left
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,

        // Top
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,

        // Bottom
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
    };

    float cube_texcoords[] = {
        // FRONT
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        0.0f,

        // BACK
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        0.0f,

        // RIGHT
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,

        // LEFT
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,

        // TOP
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,

        // BOTTOM
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f
    };

    VkBufferCreateInfo vkBufferCreateInfo;
    VkMemoryRequirements vkMemoryRequirements;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    void *data = NULL;

    // Code
    fprintf(gpLogFile, "CreateVertexBuffer()  >>>  function started\n");


    // // Create Vertex Buffer for Position for triangle
    // {
    //     memset((void *)&vertexData_position_pyramid, 0, sizeof(VertexData));
    //     memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    //     vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //     vkBufferCreateInfo.pNext = NULL;
    //     vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
    //     vkBufferCreateInfo.size = sizeof(pyramid_position);
    //     vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    //     vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position_pyramid.vkBuffer);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d) for Position\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() successful for Position\n");
    //     }

    //     memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    //     vkGetBufferMemoryRequirements(vkDevice, vertexData_position_pyramid.vkBuffer, &vkMemoryRequirements);

    //     memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    //     vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //     vkMemoryAllocateInfo.pNext = NULL;
    //     vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //     vkMemoryAllocateInfo.memoryTypeIndex = 0; // Initialia value before loop

    //     for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    //     {
    //         if (1 == (vkMemoryRequirements.memoryTypeBits & 1))
    //         {
    //             if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    //             {
    //                 vkMemoryAllocateInfo.memoryTypeIndex = le;
    //                 break;
    //             }
    //         }

    //         vkMemoryRequirements.memoryTypeBits >>= 1;
    //     }

    //     vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_pyramid.vkDeviceMemory);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d) for Position\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() successful for Position\n");
    //     }

    //     vkResult = vkBindBufferMemory(vkDevice, vertexData_position_pyramid.vkBuffer, vertexData_position_pyramid.vkDeviceMemory, 0);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d) for Position\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() successful for Position\n");
    //     }

    //     vkResult = vkMapMemory(vkDevice, vertexData_position_pyramid.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() failed (%d) for Position\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() successful for Position\n");
    //     }

    //     memcpy(data, pyramid_position, sizeof(pyramid_position));

    //     vkUnmapMemory(vkDevice, vertexData_position_pyramid.vkDeviceMemory);
    // }

    // // Create Vertex Buffer for Color for triangle
    // {
    //     memset((void *)&vertexData_texcoords_pyramid, 0, sizeof(VertexData));
    //     memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    //     vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //     vkBufferCreateInfo.pNext = NULL;
    //     vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
    //     vkBufferCreateInfo.size = sizeof(pyramid_texcoords);
    //     vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    //     vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_texcoords_pyramid.vkBuffer);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d) for Color\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() successful for Color\n");
    //     }

    //     memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    //     vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoords_pyramid.vkBuffer, &vkMemoryRequirements);

    //     memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    //     vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //     vkMemoryAllocateInfo.pNext = NULL;
    //     vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //     vkMemoryAllocateInfo.memoryTypeIndex = 0; // Initialia value before loop

    //     for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    //     {
    //         if (1 == (vkMemoryRequirements.memoryTypeBits & 1))
    //         {
    //             if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    //             {
    //                 vkMemoryAllocateInfo.memoryTypeIndex = le;
    //                 break;
    //             }
    //         }

    //         vkMemoryRequirements.memoryTypeBits >>= 1;
    //     }

    //     vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoords_pyramid.vkDeviceMemory);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d) for Color\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() successful for Color\n");
    //     }

    //     vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoords_pyramid.vkBuffer, vertexData_texcoords_pyramid.vkDeviceMemory, 0);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d) for Color\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() successful for Color\n");
    //     }

    //     vkResult = vkMapMemory(vkDevice, vertexData_texcoords_pyramid.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    //     if (VK_SUCCESS != vkResult)
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() failed (%d) for Color\n", vkResult);
    //         return (vkResult);
    //     }
    //     else
    //     {
    //         fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() successful for Color\n");
    //     }

    //     memcpy(data, pyramid_texcoords, sizeof(pyramid_texcoords));

    //     vkUnmapMemory(vkDevice, vertexData_texcoords_pyramid.vkDeviceMemory);
    // }


    // Create Vertex Buffer for Position for rectangle
    {
        memset((void *)&vertexData_position_cube, 0, sizeof(VertexData));
        memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
        vkBufferCreateInfo.size = sizeof(cube_position);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position_cube.vkBuffer);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d) for Position\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() successful for Position\n");
        }

        memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, vertexData_position_cube.vkBuffer, &vkMemoryRequirements);

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

        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_cube.vkDeviceMemory);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d) for Position\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() successful for Position\n");
        }

        vkResult = vkBindBufferMemory(vkDevice, vertexData_position_cube.vkBuffer, vertexData_position_cube.vkDeviceMemory, 0);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d) for Position\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() successful for Position\n");
        }

        vkResult = vkMapMemory(vkDevice, vertexData_position_cube.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() failed (%d) for Position\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() successful for Position\n");
        }

        memcpy(data, cube_position, sizeof(cube_position));

        vkUnmapMemory(vkDevice, vertexData_position_cube.vkDeviceMemory);
    }

    // Create Vertex Buffer for Color for rectangle
    {
        memset((void *)&vertexData_texcoords_cube, 0, sizeof(VertexData));
        memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
        vkBufferCreateInfo.size = sizeof(cube_texcoords);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_texcoords_cube.vkBuffer);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d) for Color\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkCreateBuffer() successful for Color\n");
        }

        memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoords_cube.vkBuffer, &vkMemoryRequirements);

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

        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoords_cube.vkDeviceMemory);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d) for Color\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkAllocateMemory() successful for Color\n");
        }

        vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoords_cube.vkBuffer, vertexData_texcoords_cube.vkDeviceMemory, 0);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d) for Color\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkBindBufferMemory() successful for Color\n");
        }

        vkResult = vkMapMemory(vkDevice, vertexData_texcoords_cube.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() failed (%d) for Color\n", vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpLogFile, "CreateVertexBuffer()    =>  vkMapMemory() successful for Color\n");
        }

        memcpy(data, cube_texcoords, sizeof(cube_texcoords));

        vkUnmapMemory(vkDevice, vertexData_texcoords_cube.vkDeviceMemory);
    }

    fprintf(gpLogFile, "CreateVertexBuffer()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateTexture(const char *textureFileName, VkImage *vkImage, VkDeviceMemory *vkDeviceMemory, VkImageView *vkImageView, VkSampler *vkSampler)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "CreateTexture()  >>>  function started\n");

    // STEP 1: Get image info like width, height, no of channels and actual image data using function from stb_image.h
    FILE *fp = NULL;
    fp = fopen(textureFileName, "rb");
    if (NULL == fp)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  fopen() Failed to open texture file: %s\n", textureFileName);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    uint8_t *imageData = NULL;
    int imageWidth, imageHeight, imageChannels;
    imageData = stbi_load_from_file(fp, &imageWidth, &imageHeight, &imageChannels, STBI_rgb_alpha);
    if (NULL == imageData || 0 >= imageWidth || 0 >= imageHeight || 0 >= imageChannels)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  stbi_load_from_file() Failed to load texture file: %s\n", textureFileName);
        fclose(fp);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    // VkDeviceSize is uint64_t
    VkDeviceSize imageSize = imageWidth * imageHeight * 4; // 4 for RGBA

    // STEP 2: Put above image data into a staging buffer
    VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void *)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = imageSize;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // Because this is our SOURCE to copy to vkImage.
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vkBuffer_stagingBuffer);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkCreateBuffer() failed (%d) for staging buffer\n", vkResult);
        stbi_image_free(imageData);
        fclose(fp);
        return (vkResult);
    }

    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void *)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);

    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void *)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;

    for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; le++)
    {
        if ((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = le;
                break;
            }
        }

        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkAllocateMemory() failed (%d) for staging buffer\n", vkResult);
        stbi_image_free(imageData);
        fclose(fp);
        return (vkResult);
    }

    vkResult = vkBindBufferMemory(vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkBindBufferMemory() failed (%d) for staging buffer\n", vkResult);
        stbi_image_free(imageData);
        fclose(fp);
        return (vkResult);
    }

    void *data = NULL;

    vkResult = vkMapMemory(vkDevice, vkDeviceMemory_stagingBuffer, 0, imageSize, 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkMapMemory() failed (%d) for staging buffer\n", vkResult);
        stbi_image_free(imageData);
        fclose(fp);
        return (vkResult);
    }

    memcpy(data, imageData, imageSize);

    vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer);

    // SubStep: As Copying of image data is already done into staging buffer, we can free the actual image data given by stb
    stbi_image_free(imageData);
    imageData = NULL;
    fprintf(gpLogFile, "CreateTexture()  >>>  image data freed successfully\n");

    // Step 3: Create "DEVICE ONLY VISIBLE", empty but enough sized image equal to size of image (imgWidth * imgHeight)
    VkImageCreateInfo vkImageCreateInfo;
    memset((void *)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Here ther canbe be SRGB but for portability on phones and desktops UNORM
    vkImageCreateInfo.extent.width = imageWidth;
    vkImageCreateInfo.extent.height = imageHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, vkImage);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkCreateImage() failed (%d)\n", vkResult);
        return (vkResult);
    }

    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void *)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(vkDevice, *vkImage, &vkMemoryRequirements_image);

    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements_image.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t le = 0; le < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++le)
    {
        if (1 == (vkMemoryRequirements_image.memoryTypeBits & 1))
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[le].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = le;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, vkDeviceMemory);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }

    vkBindImageMemory(vkDevice, *vkImage, *vkDeviceMemory, 0);

    // STEP 4: Send "IMAGE LAYOUT TRANSITION" to Vulkan/GPU before copying image data to empty vkImage using pipeline barrier

    // 4.1 Allocate Command Buffer
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_Transition_Image_Layout;
    memset((void *)&vkCommandBufferAllocateInfo_Transition_Image_Layout, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.pNext = NULL;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;

    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_Transition_Image_Layout, &vkCommandBuffer_Transition_Image_Layout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkAllocateCommandBuffers() failed (%d) for Transition_Image_Layout\n", vkResult);
        return (vkResult);
    }

    // 4.2 Begin Command Buffer
    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_Transition_Image_Layout;
    memset((void *)&vkCommandBufferBeginInfo_Transition_Image_Layout, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_Transition_Image_Layout.pNext = NULL;
    vkCommandBufferBeginInfo_Transition_Image_Layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_Transition_Image_Layout, &vkCommandBufferBeginInfo_Transition_Image_Layout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkBeginCommandBuffer() failed (%d) for Transition_Image_Layout\n", vkResult);
        return (vkResult);
    }

    // 4.3 Proper Operations - HERE WE DO PIPELINE BARRIER
    VkPipelineStageFlags vkPipelineStageFlags_Source = 0;
    VkPipelineStageFlags vkPipelineStageFlags_Destination = 0;
    VkImageMemoryBarrier vkImageMemoryBarrier;
    memset((void *)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = *vkImage;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;

    if (vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
        fprintf(gpLogFile, "CreateImage()    =>  Transitioning from undefined to transfer dst optimal\n");
    }
    else if (vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        fprintf(gpLogFile, "CreateImage()    =>  Transitioning from transfer dst optimal to shader read only optimal\n");
    }
    else
    {
        fprintf(gpLogFile, "CreateImage()    =>  Unsupported Texture Layout Transition\n");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    vkCmdPipelineBarrier(vkCommandBuffer_Transition_Image_Layout,
                         vkPipelineStageFlags_Source,
                         vkPipelineStageFlags_Destination,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         &vkImageMemoryBarrier);

    // 4.4 End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Transition_Image_Layout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImage()    =>  vkEndCommandBuffer() failed (%d) for Transition_Image_Layout\n", vkResult);
        return (vkResult);
    }

    // 4.5 Submit Command Buffer to Queue
    VkSubmitInfo vkSubmitInfo_Transition_Layout_Image;
    memset(&vkSubmitInfo_Transition_Layout_Image, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_Transition_Layout_Image.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_Transition_Layout_Image.pNext = NULL;
    vkSubmitInfo_Transition_Layout_Image.commandBufferCount = 1;
    vkSubmitInfo_Transition_Layout_Image.pCommandBuffers = &vkCommandBuffer_Transition_Image_Layout;

    vkResult = vkQueueSubmit(vkQueue,
                             1,
                             &vkSubmitInfo_Transition_Layout_Image,
                             VK_NULL_HANDLE);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImage()    =>  vkQueueSubmit() failed (%d) for Transition_Image_Layout\n", vkResult);
        return (vkResult);
    }

    // 4.6 Wait for Queue to Comaplete operation of submission
    vkResult = vkQueueWaitIdle(vkQueue);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImage()    =>  vkQueueWaitIdle() failed (%d) for Transition_Image_Layout\n", vkResult);
        return (vkResult);
    }

    // 4.7 Free Command Buffer after work done
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Transition_Image_Layout);
    vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;

    // STEP 5: Now actually copy tha Image Data from STEP2 to vkImage STEP3

    // 5.1 Allocate Command Buffer
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_Buffer_To_Image_Copy;
    memset((void *)&vkCommandBufferAllocateInfo_Buffer_To_Image_Copy, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.pNext = NULL;
    vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer vkCommandBuffer_Buffer_To_Image_Copy = VK_NULL_HANDLE;

    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_Buffer_To_Image_Copy, &vkCommandBuffer_Buffer_To_Image_Copy);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkAllocateCommandBuffers() failed (%d) for Buffer_To_Image_Copy\n", vkResult);
        return (vkResult);
    }

    // 5.2 Begin Command Buffer
    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_Buffer_To_Image_Copy;
    memset((void *)&vkCommandBufferBeginInfo_Buffer_To_Image_Copy, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_Buffer_To_Image_Copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_Buffer_To_Image_Copy.pNext = NULL;
    vkCommandBufferBeginInfo_Buffer_To_Image_Copy.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_Buffer_To_Image_Copy, &vkCommandBufferBeginInfo_Buffer_To_Image_Copy);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkBeginCommandBuffer() failed (%d) for Buffer_To_Image_Copy\n", vkResult);
        return (vkResult);
    }

    // 5.3 Copy Image Data
    VkBufferImageCopy vkBufferImageCopy;
    memset((void *)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));
    vkBufferImageCopy.bufferOffset = 0;
    vkBufferImageCopy.bufferRowLength = 0;
    vkBufferImageCopy.bufferImageHeight = 0;
    vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopy.imageSubresource.mipLevel = 0;
    vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
    vkBufferImageCopy.imageSubresource.layerCount = 1;
    vkBufferImageCopy.imageOffset.x = 0;
    vkBufferImageCopy.imageOffset.y = 0;
    vkBufferImageCopy.imageOffset.z = 0;
    vkBufferImageCopy.imageExtent.width = imageWidth;
    vkBufferImageCopy.imageExtent.height = imageHeight;
    vkBufferImageCopy.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(vkCommandBuffer_Buffer_To_Image_Copy,
                           vkBuffer_stagingBuffer,
                           *vkImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &vkBufferImageCopy);

    // 5.4 End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Buffer_To_Image_Copy);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkEndCommandBuffer() failed (%d) for Buffer_To_Image_Copy\n", vkResult);
        return (vkResult);
    }

    // 5.5 Submit Command Buffer to Queue
    VkSubmitInfo vkSubmitInfo_Buffer_To_Image_Copy;
    memset(&vkSubmitInfo_Buffer_To_Image_Copy, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_Buffer_To_Image_Copy.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_Buffer_To_Image_Copy.pNext = NULL;
    vkSubmitInfo_Buffer_To_Image_Copy.commandBufferCount = 1;
    vkSubmitInfo_Buffer_To_Image_Copy.pCommandBuffers = &vkCommandBuffer_Buffer_To_Image_Copy;

    vkResult = vkQueueSubmit(vkQueue,
                             1,
                             &vkSubmitInfo_Buffer_To_Image_Copy,
                             VK_NULL_HANDLE);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkQueueSubmit() failed (%d) for Buffer_To_Image_Copy\n", vkResult);
        return (vkResult);
    }

    // 5.6 Wait for Queue to Comaplete operation of submission
    vkResult = vkQueueWaitIdle(vkQueue);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkQueueWaitIdle() failed (%d) for Buffer_To_Image_Copy\n", vkResult);
        return (vkResult);
    }

    // 5.7 Free Command Buffer after work done
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Buffer_To_Image_Copy);
    vkCommandBuffer_Buffer_To_Image_Copy = VK_NULL_HANDLE;

    // STEP 6: Now again do "Image Layout Transition" similar to STEP 4 for correct reading/writing of data for shaders

    // 6.1 Allocate Command Buffer
    memset((void *)&vkCommandBufferAllocateInfo_Transition_Image_Layout, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.pNext = NULL;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_Transition_Image_Layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;

    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_Transition_Image_Layout, &vkCommandBuffer_Transition_Image_Layout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkAllocateCommandBuffers() failed (%d) for Transition_Image_Layout 2nd time\n", vkResult);
        return (vkResult);
    }

    // 6.2 Begin Command Buffer
    memset((void *)&vkCommandBufferBeginInfo_Transition_Image_Layout, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_Transition_Image_Layout.pNext = NULL;
    vkCommandBufferBeginInfo_Transition_Image_Layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_Transition_Image_Layout, &vkCommandBufferBeginInfo_Transition_Image_Layout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()  >>>  vkBeginCommandBuffer() failed (%d) for Transition_Image_Layout 2nd time\n", vkResult);
        return (vkResult);
    }

    // 6.3 Proper Operations - HERE WE DO PIPELINE BARRIER
    memset((void *)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = *vkImage;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;

    if (vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
        fprintf(gpLogFile, "CreateImage()    =>  Transitioning from undefined to transfer dst optimal 2nd time\n");
    }
    else if (vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        fprintf(gpLogFile, "CreateImage()    =>  Transitioning from transfer dst optimal to shader read only optimal 2nd time\n");
    }
    else
    {
        fprintf(gpLogFile, "CreateImage()    =>  Unsupported Texture Layout Transition\n");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    vkCmdPipelineBarrier(vkCommandBuffer_Transition_Image_Layout,
                         vkPipelineStageFlags_Source,
                         vkPipelineStageFlags_Destination,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         &vkImageMemoryBarrier);

    // 6.4 End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Transition_Image_Layout);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImage()    =>  vkEndCommandBuffer() failed (%d) for Transition_Image_Layout 2nd time\n", vkResult);
        return (vkResult);
    }

    // 6.5 Submit Command Buffer to Queue
    memset(&vkSubmitInfo_Transition_Layout_Image, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_Transition_Layout_Image.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_Transition_Layout_Image.pNext = NULL;
    vkSubmitInfo_Transition_Layout_Image.commandBufferCount = 1;
    vkSubmitInfo_Transition_Layout_Image.pCommandBuffers = &vkCommandBuffer_Transition_Image_Layout;

    vkResult = vkQueueSubmit(vkQueue,
                             1,
                             &vkSubmitInfo_Transition_Layout_Image,
                             VK_NULL_HANDLE);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImage()    =>  vkQueueSubmit() failed (%d) for Transition_Image_Layout 2nd time\n", vkResult);
        return (vkResult);
    }

    // 6.6 Wait for Queue to Comaplete operation of submission
    vkResult = vkQueueWaitIdle(vkQueue);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateImage()    =>  vkQueueWaitIdle() failed (%d) for Transition_Image_Layout 2nd time\n", vkResult);
        return (vkResult);
    }

    // 6.7 Free Command Buffer after work done
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Transition_Image_Layout);
    vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;

    // STEP 7: Now Staging Buffer is not needed hence release it's data & it's memory
    if (vkBuffer_stagingBuffer)
    {
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL);
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    }

    if (vkDeviceMemory_stagingBuffer)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL);
        vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    }

    // STEP 8: Create Image View
    // Create Image View for Above Depth Image
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void *)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = *vkImage;

    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, vkImageView);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()    =>  vkCreateImageView() failed (%d) for STEP 8\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateTexture()    =>  vkCreateImageView() successful for STEP 8\n");
    }

    // STEP 9: Create Texture Sampler for above Image
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void *)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));

    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
    vkSamplerCreateInfo.maxAnisotropy = 16;
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    vkSamplerCreateInfo.compareEnable = VK_FALSE;
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    vkResult = vkCreateSampler(vkDevice, &vkSamplerCreateInfo, NULL, vkSampler);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateTexture()    =>  vkCreateSampler() failed (%d) for STEP 9\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateTexture()    =>  vkCreateSampler() successful for STEP 9\n");
    }

    return (vkResult);
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

    // Triangle
    memset((void *)&uniformData_pyramid, 0, sizeof(UniformData));
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData_pyramid.vkBuffer);
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
    vkGetBufferMemoryRequirements(vkDevice, uniformData_pyramid.vkBuffer, &vkMemoryRequirements);

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

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_pyramid.vkDeviceMemory);
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
    vkResult = vkBindBufferMemory(vkDevice, uniformData_pyramid.vkBuffer, uniformData_pyramid.vkDeviceMemory, 0);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateUniformBuffer()    =>  vkBindBufferMemory() successful \n");
    }

    // Rectangle
    memset((void *)&uniformData_cube, 0, sizeof(UniformData));
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData_cube.vkBuffer);
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
    vkGetBufferMemoryRequirements(vkDevice, uniformData_cube.vkBuffer, &vkMemoryRequirements);

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

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_cube.vkDeviceMemory);
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
    vkResult = vkBindBufferMemory(vkDevice, uniformData_cube.vkBuffer, uniformData_cube.vkDeviceMemory, 0);
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
    glm::mat4 translateMatrix = glm::mat4(1.0f);
    glm::mat4 rotateX = glm::mat4(1.0f);
    glm::mat4 rotateY = glm::mat4(1.0f);
    glm::mat4 rotateZ = glm::mat4(1.0f);
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);

    // Code

    // Triangle
    vkResult = VK_SUCCESS;
    data = NULL;
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    perspectiveProjectionMatrix = glm::mat4(1.0f);

    // Initialize Uniform Data
    memset((void *)&myUniformData, 0, sizeof(MyUniformData));

    translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, -6.0f));

    rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(rotate_angle), glm::vec3(0.0f, 1.0f, 0.0f));

    modelMatrix = translateMatrix * rotateY;

    myUniformData.modelMatrix = modelMatrix;
    myUniformData.viewMatrix = viewMatrix;

    // Create Perspective Projection Matrix
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    // Map Memory and Copy Data
    vkResult = vkMapMemory(vkDevice, uniformData_pyramid.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "UpdateUniformBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(MyUniformData));

    vkUnmapMemory(vkDevice, uniformData_pyramid.vkDeviceMemory);

    // RECTANGLE
    vkResult = VK_SUCCESS;
    data = NULL;
    translateMatrix = glm::mat4(1.0f);
    rotateX = glm::mat4(1.0f);
    rotateY = glm::mat4(1.0f);
    rotateZ = glm::mat4(1.0f);
    scaleMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    perspectiveProjectionMatrix = glm::mat4(1.0f);

    // Initialize Uniform Data
    memset((void *)&myUniformData, 0, sizeof(MyUniformData));

    translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -6.0f));

    modelMatrix = translateMatrix * rotateX * rotateY * rotateZ * scaleMatrix;

    myUniformData.modelMatrix = modelMatrix;
    myUniformData.viewMatrix = viewMatrix;

    // Create Perspective Projection Matrix
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    // Map Memory and Copy Data
    vkResult = vkMapMemory(vkDevice, uniformData_cube.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "UpdateUniformBuffer()    =>  vkMapMemory() failed (%d) for rectangle\n", vkResult);
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(MyUniformData));

    vkUnmapMemory(vkDevice, uniformData_cube.vkDeviceMemory);

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
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[2];

    // Code
    fprintf(gpLogFile, "CreateDescriptorSetLayout()  >>>  function started\n");
    
    memset((void *)&vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * 2);

    // For Uniform Buffer
    vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //  This is the type of descriptor we are going to use
    vkDescriptorSetLayoutBinding_array[0].binding = 0;                                        //  This is actually the 0 binding point of shader
    vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL;

    // For Texture Sampler
    vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //  This is the type of descriptor we are going to use
    vkDescriptorSetLayoutBinding_array[1].binding = 1;                                               //  This is actually the 1 binding point of shader
    vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL;

    memset((void *)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0; // RESERVED
    vkDescriptorSetLayoutCreateInfo.bindingCount = ARRAYSIZE(vkDescriptorSetLayoutBinding_array);
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;

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
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateDescriptorPool()  >>>  function started\n");

    // Before Creating Actual Descriptor Pool. Vulkan Expects Descriptor pool size.
    memset((void *)&vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * ARRAYSIZE(vkDescriptorPoolSize_array));

    // For Uniform Buffer
    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1;

    // For Texture Sampler
    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize_array[1].descriptorCount = 1;

    // Create The Pool
    memset((void *)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;
    vkDescriptorPoolCreateInfo.maxSets = 2;

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
    VkDescriptorImageInfo vkDescriptorImageInfo;
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    // Code
    fprintf(gpLogFile, "CreateDescriptorSet()  >>>  function started\n");

    // TRIANGLE
    // memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    // vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // vkDescriptorSetAllocateInfo.pNext = NULL;
    // vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    // vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    // vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    // vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_pyramid);
    // if (VK_SUCCESS != vkResult)
    // {
    //     fprintf(gpLogFile, "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() failed (%d) for TRIANGLE\n", vkResult);
    //     return (vkResult);
    // }
    // else
    // {
    //     fprintf(gpLogFile, "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() successful for TRIANGLE\n");
    // }

    // // Describe if we want Buffer as Uniform or Image as Uniform

    // // Specify Buffer Info
    // memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    // vkDescriptorBufferInfo.buffer = uniformData_pyramid.vkBuffer;
    // vkDescriptorBufferInfo.offset = 0;
    // vkDescriptorBufferInfo.range = sizeof(MyUniformData);

    // // Texture Sampler Info
    // memset((void *)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    // vkDescriptorImageInfo.sampler = vkSampler_texture_stone;
    // vkDescriptorImageInfo.imageView = vkImageView_texture_stone;
    // vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // /**
    //  *
    //  * Now update the above descriptor set to the shader
    //  *
    //  * We can write or copy to shader
    //  *
    //  * We will prefer write descriptor set
    //  *
    //  */

    // // Now connect the descriptor set to the actual buffer using write descriptor set
    // memset((void *)&vkWriteDescriptorSet_array[0], 0, sizeof(VkWriteDescriptorSet) * ARRAYSIZE(vkWriteDescriptorSet_array));

    // vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // vkWriteDescriptorSet_array[0].pNext = NULL;
    // vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_pyramid;
    // vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    // vkWriteDescriptorSet_array[0].descriptorCount = 1;
    // vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    // vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    // vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    // vkWriteDescriptorSet_array[0].dstBinding = 0;

    // vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // vkWriteDescriptorSet_array[1].pNext = NULL;
    // vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_pyramid;
    // vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    // vkWriteDescriptorSet_array[1].descriptorCount = 1;
    // vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    // vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    // vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    // vkWriteDescriptorSet_array[1].dstBinding = 1;

    // // ISKA KATA HUA PAANI BHI NAHI MANGTA HAI
    // vkUpdateDescriptorSets(vkDevice, ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    // RECTANGLE
    memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_cube);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() failed (%d) for RECTANGLE\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() successful for RECTANGLE\n");
    }

    // Describe if we want Buffer as Uniform or Image as Uniform

    // Specify Buffer Info
    memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_cube.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);

    // Texture Sampler Info
    memset((void *)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.sampler = vkSampler_texture_smiley;
    vkDescriptorImageInfo.imageView = vkImageView_texture_smiley;
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
    memset((void *)&vkWriteDescriptorSet_array[0], 0, sizeof(VkWriteDescriptorSet));

    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].pNext = NULL;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_cube;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0;

    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].pNext = NULL;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_cube;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1;

    // ISKA KATA HUA PAANI BHI NAHI MANGTA HAI
    vkUpdateDescriptorSets(vkDevice, ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    fprintf(gpLogFile, "CreateDescriptorSet()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateRenderPass(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkAttachmentDescription vkAttachmentDescription_array[2];
    VkAttachmentReference vkAttachmentReference_color;
    VkAttachmentReference vkAttachmentReference_depth;
    VkSubpassDescription vkSubpassDescription;
    VkRenderPassCreateInfo vkRenderPassCreateInfo;

    // Code
    fprintf(gpLogFile, "CreateRenderPass()  >>>  function started\n");

    memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * ARRAYSIZE(vkAttachmentDescription_array));

    // Color Attachment
    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth Attachment
    vkAttachmentDescription_array[1].flags = 0;
    vkAttachmentDescription_array[1].format = vkFormat_depth;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Color Attachment Reference
    memset((void *)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_color.attachment = 0; //  index of attachment of above array
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    memset((void *)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_depth.attachment = 1; //  index of attachment of above array
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1;//ARRAYSIZE(vkAttachmentDescription_array);
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
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2];
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2];
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

    // Position
    vkVertexInputBindingDescription_array[0].binding = 0;
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; // Position
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Color
    vkVertexInputBindingDescription_array[1].binding = 1;
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 2; // Color
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // VkVertexInputAttributeDescription
    memset((void *)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * ARRAYSIZE(vkVertexInputAttributeDescription_array));
    
    // Position
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;

    // Texture Coordinates
    vkVertexInputAttributeDescription_array[1].binding = 1;
    vkVertexInputAttributeDescription_array[1].location = 1;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;

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
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //  vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = ;

    // Rasterization/Rasterizer State
    memset((void *)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
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
        
        VkImageView vkImageView_attachment_array[2];
        VkFramebufferCreateInfo vkFramebufferCreateInfo;
    
        memset((void *)vkImageView_attachment_array, 0, sizeof(VkImageView) * ARRAYSIZE(vkImageView_attachment_array));
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
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    fprintf(gpLogFile, "BuildCommandBuffers()  >>>  function started\n");

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

        vkClearValue_array[0].color = vkClearColorValue;
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
            VkDeviceSize vkDeviceSize_offset_position[1];
            VkDeviceSize vkDeviceSize_offset_texcoords[1];

            // // Triangle
            // {
            //     // Bind Descriptor Set to Pipeline
            //     vkCmdBindDescriptorSets(vkCommandBuffer_array[le],
            //                             VK_PIPELINE_BIND_POINT_GRAPHICS,
            //                             vkPipelineLayout,
            //                             0, // firstSet
            //                             1, // descriptorSetCount
            //                             &vkDescriptorSet_pyramid,
            //                             0,     // dynamicOffsetCount
            //                             NULL); // pDynamicOffsets

            //     // Bind with the Vertex Position Buffer
            //     memset((void *)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * ARRAYSIZE(vkDeviceSize_offset_position));
            //     vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 0, 1, &vertexData_position_pyramid.vkBuffer, vkDeviceSize_offset_position);

            //     // Bind with the Vertex Color Buffer
            //     memset((void *)vkDeviceSize_offset_texcoords, 0, sizeof(VkDeviceSize) * ARRAYSIZE(vkDeviceSize_offset_texcoords));
            //     vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 1, 1, &vertexData_texcoords_pyramid.vkBuffer, vkDeviceSize_offset_texcoords);

            //     //******************************************************************************************************
            //     // HERE WE SHOULD CALL VULKAN DRAWING FUNCTION
            //     //******************************************************************************************************

            //     /*********** BEGIN DRAWING ***********/

            //     vkCmdDraw(vkCommandBuffer_array[le], 12, 1, 0, 0);

            //     /************ END DRAWING ************/
            // }
            
            // Rectangle
            {
                // Bind Descriptor Set to Pipeline
                vkCmdBindDescriptorSets(vkCommandBuffer_array[le],
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        vkPipelineLayout,
                                        0, // firstSet
                                        1, // descriptorSetCount
                                        &vkDescriptorSet_cube,
                                        0,     // dynamicOffsetCount
                                        NULL); // pDynamicOffsets

                // Bind with the Vertex Position Buffer
                memset((void *)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 0, 1, &vertexData_position_cube.vkBuffer, vkDeviceSize_offset_position);

                // Bind with the Vertex Color Buffer
                memset((void *)vkDeviceSize_offset_texcoords, 0, sizeof(VkDeviceSize) * ARRAYSIZE(vkDeviceSize_offset_texcoords));
                vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 1, 1, &vertexData_texcoords_cube.vkBuffer, vkDeviceSize_offset_texcoords);

                //******************************************************************************************************
                // HERE WE SHOULD CALL VULKAN DRAWING FUNCTION
                //******************************************************************************************************

                /*********** BEGIN DRAWING ***********/

                vkCmdDraw(vkCommandBuffer_array[le], 6, 1, 0, 0);

                /************ END DRAWING ************/
            }
            
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
