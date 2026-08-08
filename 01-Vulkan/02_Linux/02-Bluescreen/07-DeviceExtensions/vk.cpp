/**************************************************
 * S.G.M.P
 * @file: vk.cpp
 * @brief: Linux - Vulkan Device Extensions
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: 03 August 2026 (Monday) 08:05:00 PM
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
const char* gpszWindowTitle = "MST: Linux - Vulkan Device Extensions"; // Window title

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

// VK_KHR_SWAPCHAIN_EXTENSION_NAME

// Step 07 - Device Extension
uint32_t enabledDeviceExtensionCount = 0;
const char* enabledDeviceExtensionNames_array[1];

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

VkResult Initialize(void) // Function Definition
{
    // Function Declaration
    VkResult    CreateVulkanInstance(void);
    VkResult    GetSupportedSurface(void);
    VkResult    GetPhysicalDevice(void);
    VkResult    PrintVkInfo(void);
    VkResult    FillDeviceExtensionNames(void);

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

    //  Step 07 - Device Extensions
    vkResult = FillDeviceExtensionNames();
    if(VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, "Initialize()    =>  FillDeviceExtensionNames() failed %d\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpLogFile, "Initialize()    =>  FillDeviceExtensionNames() successful\n");
    }

    fprintf(gpLogFile, "Initialize()    -->  function completed\n");
    return (vkResult);
}

VkResult resize(int width, int height)
{
    (void)width;
    (void)height;
    return (VK_SUCCESS);
}

VkResult display(void)
{
    return ((VkResult)VK_FALSE);
}

void update(void)
{
    // Code
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
    if (vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
        vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkFreeMemory() successful for vertexData_position.vkDeviceMemory\n");
    }

    if (vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
        vertexData_position.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpLogFile, "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_position.vkBuffer\n");
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
