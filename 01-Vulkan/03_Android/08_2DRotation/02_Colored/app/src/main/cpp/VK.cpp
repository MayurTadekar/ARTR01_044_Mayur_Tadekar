/**************************************************
 * S.G.M.P
 * @file: VK.c
 * @brief: Vulkan Triangle Perspective Matrix Way
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: 02 July 2025 (Wednesday) 10:36:41 PM
 **************************************************/

// HEADERS android_headers
#include <android_native_app_glue.h> // Everything related with pure native activities needs this file, mainly entry point function android_main() and struct android_app
#include <android/log.h>			 // For android_log_print() -> Android che printf();

#include <memory.h> // for memset()
#include <math.h>

// Vulkan Headers 
// #define VK_USE_PLATFORM_WIN32_KHR --- android_platform
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h> //  We must tell the header the platform we are on

// GLM related macros and headers files
// GLM follows column major order just like OpenGL
// BUT unlike OpenGL, GLM's matrix arrays are 4x4 2D arrays not 1D arrays of 16 elements. VVVIMP
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_0_TO_1
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// MACROS
#define _ARRAYSIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define NEWLINEINFILE (__android_log_print(ANDROID_LOG_INFO, "MST:", "\n"))

#define WINDOW_TITLE "MST: Vulkan-AndroidNDK - Triangle Perspective Matrix Way"

// GLOBAL VARIABLES
// HWND ghwnd;
// bool bFullscreen = false;
// bool bActiveWindow = true;
// bool bEscapeKeyPressed = false;
// DWORD dwStyle;
// WINDOWPLACEMENT wpPrev;
// FILE *gpLogFile = NULL;

// android_global_variable
typedef struct
{
	struct android_app *app;
	bool bActive;
} Engine;


long touchStartTime = 0;
long pendingSingleTapTime = 0;
float touchStartX = 0.0f;
float touchStartY = 0.0f;
float lastTapX = 0.0f;
float lastTapY = 0.0f;

// Event Related Global Variables
bool bTouchDown = false;
bool bDragging = false;
bool bLongPressDetected = false;
bool bDoubleTapDetected = false;
bool bPendingSingleTap = false;

bool bDone = false;

ANativeWindow *androidNativeWindow = NULL;
AAssetManager *androidAssetManager = NULL;

const char *gpszAppName = "ARTR";

int winWidth = 0;
int winHeight = 0;

// Vulkan Related Global Variable
//  Step 02 - Instance Extension Related Variables
//  Extension Needed
//  VK_KHR_SURFACE_EXTENSION_NAME
//  VK_KHR_ANDROID_SURFACE_EXTENSION_NAME --- android_surface_extension
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
// int winWidth = WIN_WIDTH;
// int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

// Step 13 - Swapchain Images and ImageViews
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

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

// Step 20 - Render
bool bInitialized = false;
uint32_t currentImageIndex = UINT32_MAX; //  or UINT_MAX

// Step 21 - Validation
bool bValidation = true;
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

// Triangle
VertexData vertexData_position_triangle;
VertexData vertexData_color_triangle;

// Rectangle
VertexData vertexData_position_rectangle;
VertexData vertexData_color_rectangle;

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

UniformData uniformData_triangle;
UniformData uniformData_rectangle;

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
VkDescriptorSet vkDescriptorSet_triangle = VK_NULL_HANDLE;
VkDescriptorSet vkDescriptorSet_rectangle = VK_NULL_HANDLE;

// Step 26 Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

// Rotation Angle
float rotateAngle = 0.0f;

// // FUNCTION SIGNATURE
// LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// android_callback
void engine_handle_cmd(struct android_app *app, int32_t cmd);
int32_t engine_handle_input(struct android_app *app, AInputEvent *event);

// FUNCTIONS
void android_main(struct android_app *state)
{
	__android_log_print(ANDROID_LOG_INFO, "MST:", "Initializing Android app\n");

    // Function Declarations
    VkResult Display(void);
    void Update(void);

    // Variable Declarations
    VkResult vkResult = VK_SUCCESS;

	// Fullscreen and hide status bar
	JavaVM *vm = state->activity->vm; // Representing DALVIK in NDK
	JNIEnv *env = NULL;
	vm->AttachCurrentThread(&env, NULL);
	jobject activityObject = state->activity->clazz;
	
	__android_log_print(ANDROID_LOG_INFO, "MST:", "Initializing fullscreen mode\n");
	
	jclass activityClass = env->GetObjectClass(activityObject);
	jclass windowClass = env->FindClass("android/view/Window");
	jclass viewClass = env->FindClass("android/view/View");
	
	// Get Window Method
	jmethodID getWindowMethod = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
	jobject windowObject = env->CallObjectMethod(activityObject, getWindowMethod);
	
	jmethodID getDecorViewMethod = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
	jobject decorViewObject = env->CallObjectMethod(windowObject, getDecorViewMethod);

	// Get 8 view class static fields
	__android_log_print(ANDROID_LOG_INFO, "MST:", "Getting static fields\n");
	const int flag_SYSTEM_UI_FLAG_EMMERSIVE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_IMMERSIVE", "I"));
	__android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_FLAG_IMMERSIVE: %d\n", flag_SYSTEM_UI_FLAG_EMMERSIVE);
	// const int flag_SYSTEM_UI_LAYOUT_STABLE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_LAYOUT_STABLE", "I"));
	// __android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_LAYOUT_STABLE: %d\n", flag_SYSTEM_UI_LAYOUT_STABLE);
	// const int flag_SYSTEM_UI_LAYOUT_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_LAYOUT_HIDE_NAVIGATION", "I"));
	// __android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_LAYOUT_HIDE_NAVIGATION: %d\n", flag_SYSTEM_UI_LAYOUT_HIDE_NAVIGATION);
	// const int flag_SYSTEM_UI_LAYOUT_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_LAYOUT_FULLSCREEN", "I"));
	// __android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_LAYOUT_FULLSCREEN: %d\n", flag_SYSTEM_UI_LAYOUT_FULLSCREEN);
	// const int flag_SYSTEM_UI_HIDE_NAVIGATION = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_HIDE_NAVIGATION", "I"));
	// __android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_HIDE_NAVIGATION: %d\n", flag_SYSTEM_UI_HIDE_NAVIGATION);
	const int flag_SYSTEM_UI_FLAG_FULLSCREEN = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I"));
	__android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_FLAG_FULLSCREEN: %d\n", flag_SYSTEM_UI_FLAG_FULLSCREEN);
	const int flag_SYSTEM_UI_FLAG_LOW_PROFILE = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_LOW_PROFILE", "I"));
	__android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_FLAG_LOW_PROFILE: %d\n", flag_SYSTEM_UI_FLAG_LOW_PROFILE);
	// const int flag_SYSTEM_UI_FLAG_EMMERSIVE_STICKY = env->GetStaticIntField(viewClass, env->GetStaticFieldID(viewClass, "SYSTEM_UI_FLAG_EMMERSIVE_STICKY", "I"));
	// __android_log_print(ANDROID_LOG_INFO, "MST:", "SYSTEM_UI_FLAG_EMMERSIVE_STICKY: %d\n", flag_SYSTEM_UI_FLAG_EMMERSIVE_STICKY);

	__android_log_print(ANDROID_LOG_INFO, "MST:", "Setting system UI visibility\n");

	jmethodID setSystemUiVisibilityMethod = env->GetMethodID(viewClass, "setSystemUiVisibility", "(I)V");
	env->CallVoidMethod(decorViewObject, setSystemUiVisibilityMethod, 	flag_SYSTEM_UI_FLAG_EMMERSIVE | 
																		// flag_SYSTEM_UI_LAYOUT_STABLE |
																		// flag_SYSTEM_UI_LAYOUT_HIDE_NAVIGATION |
																		// flag_SYSTEM_UI_LAYOUT_FULLSCREEN |
																		// flag_SYSTEM_UI_HIDE_NAVIGATION |
																		flag_SYSTEM_UI_FLAG_FULLSCREEN |
																		flag_SYSTEM_UI_FLAG_LOW_PROFILE 
																		// | flag_SYSTEM_UI_FLAG_EMMERSIVE_STICKY
																	);

	// Change to Landscape Mode
	jclass activityInfoClass = env->FindClass("android/content/pm/ActivityInfo");
	__android_log_print(ANDROID_LOG_INFO, "MST:", "Finding activity info class\n");
	int flag_SCREEN_ORIENTATION_LANDSCAPE = env->GetStaticIntField(	activityInfoClass, env->GetStaticFieldID(activityInfoClass, "SCREEN_ORIENTATION_LANDSCAPE", "I"));

	jmethodID setRequestedOrientationMethod = env->GetMethodID(activityClass, "setRequestedOrientation", "(I)V");

	__android_log_print(ANDROID_LOG_INFO, "MST:", "Setting requested orientation to landscape\n");

	// Now actually call setRequestedOrientationMethod
	env->CallVoidMethod(activityObject, setRequestedOrientationMethod, flag_SCREEN_ORIENTATION_LANDSCAPE);

	__android_log_print(ANDROID_LOG_INFO, "MST:", "Fullscreen mode initialized\n");

	vm->DetachCurrentThread();
	
	// Code
	Engine engine;
	memset((void *)&engine, 0, sizeof(Engine));

	// Initialize State
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;

	engine.app = state;

	__android_log_print(ANDROID_LOG_INFO, "MST:", "android_main() started successfully\n");

	// Game Loop / Message Loop
	while (1)
	{
		int identifier;
		struct android_poll_source *source = NULL;

		while (0 <= (identifier = ALooper_pollOnce(engine.bActive ? 0 : -1, NULL, NULL, (void **)&source)))
		{
			// Process system events
			if (NULL != source)
			{
				source->process(state, source);
			}

			// Check when to exit
			if (0 != state->destroyRequested)
			{
				return;
			}
		}

		// Handle Long Press
		if ( true == bTouchDown && 
			 false == bDragging && 
			 false == bLongPressDetected && 
			 false == bDoubleTapDetected )
		{
			struct timespec ts;
			memset((void*)&ts, 0, sizeof(struct timespec));
			clock_gettime(CLOCK_MONOTONIC, &ts);

			long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
			
			if ( now - touchStartTime > 500 )
			{
				bLongPressDetected = true;
				__android_log_print(ANDROID_LOG_INFO, "MST:", "Long press detected\n");
			}
		}

		// Handle Single Tap
		if( true == bPendingSingleTap )
		{	
			struct timespec ts;
			memset((void*)&ts, 0, sizeof(struct timespec));
			clock_gettime(CLOCK_MONOTONIC, &ts);

			long now = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);

			if ( now - pendingSingleTapTime > 300 )
			{
				bPendingSingleTap = false;
				__android_log_print(ANDROID_LOG_INFO, "MST:", "Single tap detected\n");
			}
		}

        // Game Loop
        if ( true == engine.bActive && true == bInitialized)
        {
            // Display
            vkResult = Display();
            if ( VK_FALSE != vkResult && 
                 VK_SUCCESS != vkResult && 
                 VK_ERROR_OUT_OF_DATE_KHR != vkResult && 
                 VK_SUBOPTIMAL_KHR != vkResult )
            {
                __android_log_print(ANDROID_LOG_INFO, "MST:", "Failed to display frame\n");
            }

            // Update
            Update();
        }
	}
}

void engine_handle_cmd(struct android_app *app, int32_t cmd)
{
    // Function Declarations
    VkResult Initialize(void);
    void Uninitialize(void);

    // Variable Definitions
    VkResult vkResult = VK_SUCCESS;

	// Code
	Engine *engine = (Engine *)app->userData;

	switch (cmd)
	{
	case APP_CMD_SAVE_STATE:
		engine->bActive = false;
		break;

	case APP_CMD_INIT_WINDOW: // WM_CREATE
		if (NULL != engine->app->window)
		{
			androidNativeWindow = engine->app->window;
			
            androidAssetManager = engine->app->activity->assetManager;
            
            winWidth = ANativeWindow_getWidth(androidNativeWindow);
            winHeight = ANativeWindow_getHeight(androidNativeWindow);
            
            if ( false == bInitialized )
            {
                vkResult = Initialize();
                if ( VK_SUCCESS != vkResult )
                {
                    __android_log_print(ANDROID_LOG_INFO, "MST:", "Failed to initialize window\n");
                    return;
                }
                else
                {
                    __android_log_print(ANDROID_LOG_INFO, "MST:", "Window initialized\n");
                }
            }

            engine->bActive = true;

            __android_log_print(ANDROID_LOG_INFO, "MST:", "Window initialized\n");
		}
		else
		{
			androidNativeWindow = NULL;
			__android_log_print(ANDROID_LOG_INFO, "MST:", "Failed to initialize window\n");
		}
		break;

	case APP_CMD_TERM_WINDOW: // WM_DESTROY
        
        Uninitialize();

		__android_log_print(ANDROID_LOG_INFO, "MST:", "Window terminated\n");
		break;

	case APP_CMD_GAINED_FOCUS: // WM_SETFOCUS
		engine->bActive = true;
		__android_log_print(ANDROID_LOG_INFO, "MST:", "Window gained focus\n");
		break;

	case APP_CMD_LOST_FOCUS: // WM_KILLFOCUS
		engine->bActive = false;
		__android_log_print(ANDROID_LOG_INFO, "MST:", "Window lost focus\n");
		break;
	}
}

int32_t engine_handle_input(struct android_app *app, AInputEvent *event)
{
	// Code

	Engine *engine = (Engine *)app->userData;

	int32_t eventType = AInputEvent_getType(event);

	switch(eventType)
	{
		case AINPUT_EVENT_TYPE_MOTION:
		{
			// Handle motion events
			int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
		
			switch(action)
			{
				case AMOTION_EVENT_ACTION_DOWN:
				{
					touchStartX = AMotionEvent_getX(event, 0);
					touchStartY = AMotionEvent_getY(event, 0);
					struct timespec ts;
					memset((void*)&ts, 0, sizeof(struct timespec));
					clock_gettime(CLOCK_MONOTONIC, &ts);

					touchStartTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
					
					bTouchDown = true;
					bDragging = false;
					bLongPressDetected = false;
					bDoubleTapDetected = false;

					// Check whether the down action is for 2nd tap for double tap
					if ( true == bPendingSingleTap )
					{
						long timeSinceLastTap = touchStartTime - pendingSingleTapTime;
						float dx = touchStartX - lastTapX;
						float dy = touchStartY - lastTapY;
						float distance = sqrtf(dx * dx + dy * dy);
					
						if ( timeSinceLastTap < 300 && distance < 100.0f )
						{
							// Doubletap Detected
							bDoubleTapDetected = true;
							__android_log_print(ANDROID_LOG_INFO, "MST:", "Double tap detected\n");
						}
						else
						{
							// This is new touch but not for double tap
						}

						bPendingSingleTap = false;
					}
				}
				break;

				case AMOTION_EVENT_ACTION_MOVE:
				{
					float currentX = AMotionEvent_getX(event, 0);
					float currentY = AMotionEvent_getY(event, 0);

					float dx = currentX - touchStartX;
					float dy = currentY - touchStartY;
					float distance = sqrtf(dx * dx + dy * dy);

					if ( distance > 50.0f )
					{
						bDragging = true;
					}	
				}
				break;

				case AMOTION_EVENT_ACTION_UP:
				{
					struct timespec ts;
					memset((void*)&ts, 0, sizeof(struct timespec));
					clock_gettime(CLOCK_MONOTONIC, &ts);

					long currentTime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
					long duration = currentTime - touchStartTime;

					float endX = AMotionEvent_getX(event, 0);
					float endY = AMotionEvent_getY(event, 0);
					float dx = endX - touchStartX;
					float dy = endY - touchStartY;
					float distance = sqrtf(dx * dx + dy * dy);

					bTouchDown = false;

					if ( true == bDoubleTapDetected ) 
					{
						// This up belongs to 2nd tap of double tap which we already handled
					}
					else if ( true == bLongPressDetected )
					{
						// This up belongs to long press which we already handled
					}
					else if ( true == bDragging && 150.0f < distance )
					{
						// Swipe is detected
						__android_log_print(ANDROID_LOG_INFO, "MST:", "Swipe detected\n");
						ANativeActivity_finish(engine->app->activity);
					}
					else if ( 300 > duration && 50.0f > distance )
					{
						// This is means Short tap is detected and 2nd tap maybe pending.
						// This maybe up of single tap which we already handled
						bPendingSingleTap = true;
						pendingSingleTapTime = currentTime;
						lastTapX = endX;
						lastTapY = endY;
					}

					bDragging = false;
				}
				break;
			}
		}
		break;			
	}

	return (0);
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
    VkResult CreateCommandBuffers(void);
    VkResult CreateVertexBuffer(void);
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
    if (true == bValidation)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()    ||> VALIDATION \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    >>>  function started\n");

    //  Get Vulkan Instance
    vkResult = CreateVulkanInstance();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()   =>  CreateVulkanInstance() failed\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()   =>  CreateVulkanInstance() successful\n");
    }

    //  Create Vulkan Presentation Surface
    vkResult = GetSupportedSurface();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()   =>  GetSupportedSurface() failed\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()   =>  GetSupportedSurface() successful\n");
    }

    //  Step 05 - Physical Device - Enumerate & Select Physical Device and its Queue Family Index
    vkResult = GetPhysicalDevice();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  GetPhysicalDevice() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  GetPhysicalDevice() successful\n");
    }

    //  Step 06 - Print VK Info
    vkResult = PrintVkInfo();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  PrintVkInfo() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  PrintVkInfo() successful\n");
    }

    // Step 08 - Vulkan Device
    vkResult = CreateVulkanDevice();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateVulkanDevice() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateVulkanDevice() successful\n");
    }

    GetDeviceQueue();

    // Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateSwapchain() failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateSwapchain() successful\n");
    }

    // Swapchain Images and ImageViews
    vkResult = CreateImagesAndImageViews();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateImagesAndImageViews() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateImagesAndImageViews() successful\n");
    }

    // Create Command Pool
    vkResult = CreateCommandPool();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateCommandPool() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateCommandPool() successful\n");
    }

    // Create Command Buffers
    vkResult = CreateCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateCommandBuffers() successful\n");
    }

    // Create Vertex Buffer
    vkResult = CreateVertexBuffer();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateVertexBuffer() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateVertexBuffer() successful\n");
    }

    // Create Uniform Buffer
    vkResult = CreateUniformBuffer();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateUniformBuffer() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateUniformBuffer() successful\n");
    }

    // Create Shader
    vkResult = CreateShaders();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateShaders() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateShaders() successful\n");
    }

    // Create Descriptor SetLayout
    vkResult = CreateDescriptorSetLayout();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateDescriptorSetLayout() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateDescriptorSetLayout() successful\n");
    }

    // Create Pipeline Layout
    vkResult = CreatePipelineLayout();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreatePipelineLayout() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreatePipelineLayout() successful\n");
    }

    // Create Descriptor Pool
    vkResult = CreateDescriptorPool();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateDescriptorPool() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateDescriptorPool() successful\n");
    }

    // Create Descriptor Set
    vkResult = CreateDescriptorSet();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateDescriptorSet() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateDescriptorSet() successful\n");
    }

    // Create RenderPass
    vkResult = CreateRenderPass();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateRenderPass() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateRenderPass() successful\n");
    }

    // Create Pipeline
    vkResult = CreatePipeline();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreatePipeline() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreatePipeline() successful\n");
    }

    // Create Frame Buffer
    vkResult = CreateFrameBuffers();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateFrameBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateFrameBuffers() successful\n");
    }

    // Create Semaphores
    vkResult = CreateSemaphores();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateSemaphores() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateSemaphores() successful\n");
    }

    // Create Semaphores
    vkResult = CreateFences();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateFences() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  CreateFences() successful\n");
    }

    memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));

    // FINALLY GIVING BLUE COLOR - Analogous to glClearColor;
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 0.0f;
    vkClearColorValue.float32[3] = 1.0f;

    // Build Command Buffers
    vkResult = BuildCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  BuildCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    =>  BuildCommandBuffers() successful\n");
    }

    // Initialization is Completed
    bInitialized = true;

    __android_log_print(ANDROID_LOG_INFO, "MST:", "Initialize()    -->  function completed\n");
    return (vkResult);
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
    VkResult CreateCommandBuffers(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    >>>  function started\n");

    if (height < 0)
        height = 1;

    // Check bInitialized
    if (false == bInitialized)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  Initialization yet not completed or failed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // As recreation of swapchain is needed we are going to repeat many steps of initialize again hence set bInitialized to false again
    bInitialized = false;

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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDeviceWaitIdle() is done\n");
    }

    // Check presence of swapchain
    if (VK_NULL_HANDLE == vkSwapchainKHR)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  swapchain is NULL, cannot proceed further \n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Destoy Frame Buffer
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[le], NULL);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDestroyFramebuffer() successful for %d\n", le);
    }

    if (vkFramebuffer_array)
    {
        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkFramebuffer_array freed successfully\n");
    }

    // Destroy RenderPass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDestroyRenderPass() successful\n");
    }

    // Destroy Pipeline
    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDestroyPipeline() successful\n");
    }

    // Destroy Pipeline Layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDestroyPipelineLayout() successful\n");
    }

    // Destroy Command Buffers
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[le]);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkFreeCommandBuffers() successful for %d\n", le);
    }

    if (vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkCommandBuffer_array freed successfully\n");
    }

    // Destroy Swapchain Images and Image Views

    // Free swapchain image views
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[le], NULL);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDestroyImageView() successful for %d\n", le);
    }

    if (swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  swapchainImageView_array freed successfully\n");
    }

    // Free swapchain images
    // for(uint32_t le = 0; le < swapchainImageCount; ++le)
    // {
    //     vkDestroyImage(vkDevice, swapchainImage_array[le], NULL);
    //     __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyImage() successful for %d\n", le);
    //     fclose(gpLogFile);
    // }

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  swapchainImage_array freed successfully\n");
    }

    // Destroy Swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()  =>  vkDestroySwapchainKHR() successful\n");
    }

    /****** RECREATE FOR RESIZE ******/

    // Create Swapchain

    // Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreateSwapchain() failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Create Swapchain and ImageViews

    // Swapchain Images and ImageViews
    vkResult = CreateImagesAndImageViews();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreateImagesAndImageViews() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Command Buffers
    vkResult = CreateCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreateCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Pipeline Layout
    vkResult = CreatePipelineLayout();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreatePipelineLayout() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create RenderPass
    vkResult = CreateRenderPass();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreateRenderPass() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Pipeline
    vkResult = CreatePipeline();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreatePipeline() failed %d\n", vkResult);
        return (vkResult);
    }

    // Create Frame Buffer
    vkResult = CreateFrameBuffers();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  CreateFrameBuffers() failed %d\n", vkResult);
        return (vkResult);
    }

    // Build Command Buffers
    vkResult = BuildCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    =>  BuildCommandBuffers() failed %d\n", vkResult);
        return (vkResult);
    }

    bInitialized = true;
    __android_log_print(ANDROID_LOG_INFO, "MST:", "Resize()    -->  function completed\n");
    return (vkResult);
}

VkResult Display(void)
{
    // 11 June 2025 (Wednesday) 02:08:24 AM === MI DISPLAY MADHYE AATA AALO FIRST TIME IN VULKAN

    // Function Declaration
    VkResult UpdateUniformBuffer(void);

    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code

    // ifcontrol comes here before initialization is completed then return
    if (false == bInitialized)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => Initialization not completed\n");
        return ((VkResult)VK_FALSE);
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
    if (VK_SUCCESS != vkResult && VK_SUBOPTIMAL_KHR != vkResult)
    {
        if( VK_ERROR_OUT_OF_DATE_KHR == vkResult )
        {
            Resize(winWidth, winHeight);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => vkAcquireNextImageKHR() failed\n");
        return (vkResult);
        }
    }

    // Use Fence to allow host to wait of completion of execution of previous command buffer
    vkResult = vkWaitForFences(vkDevice,
                               1,
                               &vkFence_array[currentImageIndex],
                               VK_TRUE,
                               UINT64_MAX);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => vkWaitForFences() failed\n");
        return (vkResult);
    }

    // Now ready the fences for execution of next command buffer
    vkResult = vkResetFences(vkDevice,
                             1,
                             &vkFence_array[currentImageIndex]);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => vkWaitForFences() failed\n");
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => vkQueueSubmit() failed\n");
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
    if ( VK_SUCCESS != vkResult && VK_SUBOPTIMAL_KHR != vkResult )
    {
        if( VK_ERROR_OUT_OF_DATE_KHR == vkResult )
        {
            Resize(winWidth, winHeight);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => vkAcquireNextImageKHR() failed\n");
        return (vkResult);
        }
    }

    vkResult = UpdateUniformBuffer();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Display()   => UpdateUniformBuffer() failed\n");
        return (vkResult);
    }

    vkDeviceWaitIdle(vkDevice); //  VALIDATION ||> CASE 01
    return (vkResult);
}

void Update(void)
{
    // Code
    
    rotateAngle += 0.1f;
    if ( 360.0f <= rotateAngle )
    {
        rotateAngle -= 360.0f;
    }

}

void Uninitialize(void)
{
    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  >>>  function started\n");

    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDeviceWaitIdle() is done\n");
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyFence(vkDevice, vkFence_array[le], NULL);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyFence() successful for %d\n", le);
    }

    if (vkFence_array)
    {
        free(vkFence_array);
        vkFence_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFence_array freed successfully\n");
    }

    if (vkSemaphore_rendercomplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_rendercomplete, NULL);
        vkSemaphore_rendercomplete = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroySemaphore() successful for vkSemaphore_rendercomplete\n");
    }

    if (vkSemaphore_backbuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backbuffer, NULL);
        vkSemaphore_backbuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroySemaphore() successful for vkSemaphore_backbuffer\n");
    }

    // Frame Buffer
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[le], NULL);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyFramebuffer() successful for %d\n", le);
    }

    if (vkFramebuffer_array)
    {
        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFramebuffer_array freed successfully\n");
    }

    // Render Pass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyRenderPass() successful\n");
    }

    // Descriptor Pool
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        // Note: Descriptor Sets are automatically freed when pool is destroyed
        vkDescriptorSet_triangle = VK_NULL_HANDLE; // Resetting to NULL as it is not
        vkDescriptorSet_rectangle = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyDescriptorPool() successful for vkDescriptorPool and vkDescriptorSet_triangle & vkDescriptorSet_rectangle\n");
    }

    // Pipeline Layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyPipelineLayout() successful\n");
    }

    if (vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyDescriptorSetLayout() successful\n");
    }

    // Pipeline
    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyPipeline() successful\n");
    }

    // Destroy Shader Fragment

    // Fragment Shader
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyShaderModule() successful for vkShaderModule_fragment_shader\n");
    }

    // Vertex Shader
    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyShaderModule() successful for vkShaderModule_vertex_shader\n");
    }

    // Vertex Buffer
    if (vertexData_position_triangle.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_triangle.vkDeviceMemory, NULL);
        vertexData_position_triangle.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeMemory() successful for vertexData_position_triangle.vkDeviceMemory\n");
    }

    if (vertexData_position_triangle.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_triangle.vkBuffer, NULL);
        vertexData_position_triangle.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_position_triangle.vkBuffer\n");
    }

    // Vertex Color Buffer
    if ( vertexData_color_triangle.vkDeviceMemory )
    {
        vkFreeMemory(vkDevice, vertexData_color_triangle.vkDeviceMemory, NULL);
        vertexData_color_triangle.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeMemory() successful for vertexData_color_triangle.vkDeviceMemory\n");
    }
    
    if ( vertexData_color_triangle.vkBuffer )
    {
        vkDestroyBuffer(vkDevice, vertexData_color_triangle.vkBuffer, NULL);
        vertexData_color_triangle.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_color_triangle.vkBuffer\n");
    }

    // Vertex Buffer
    if (vertexData_position_rectangle.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position_rectangle.vkDeviceMemory, NULL);
        vertexData_position_rectangle.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeMemory() successful for vertexData_position_rectangle.vkDeviceMemory\n");
    }

    if (vertexData_position_rectangle.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position_rectangle.vkBuffer, NULL);
        vertexData_position_rectangle.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_position_rectangle.vkBuffer\n");
    }

    // Vertex Color Buffer
    if ( vertexData_color_rectangle.vkDeviceMemory )
    {
        vkFreeMemory(vkDevice, vertexData_color_rectangle.vkDeviceMemory, NULL);
        vertexData_color_rectangle.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeMemory() successful for vertexData_color_rectangle.vkDeviceMemory\n");
    }
    
    if ( vertexData_color_rectangle.vkBuffer )
    {
        vkDestroyBuffer(vkDevice, vertexData_color_rectangle.vkBuffer, NULL);
        vertexData_color_rectangle.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyBuffer() successful for vertexData_color_rectangle.vkBuffer\n");
    }

    // Uniform Buffer
    if (uniformData_triangle.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData_triangle.vkDeviceMemory, NULL);
        uniformData_triangle.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeMemory() successful for uniformData_triangle.vkDeviceMemory\n");
    }

    if (uniformData_triangle.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData_triangle.vkBuffer, NULL);
        uniformData_triangle.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyBuffer() successful for uniformData_triangle.vkBuffer\n");
    }

    // Uniform Buffer
    if (uniformData_rectangle.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData_rectangle.vkDeviceMemory, NULL);
        uniformData_rectangle.vkDeviceMemory = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeMemory() successful for uniformData_rectangle.vkDeviceMemory\n");
    }

    if (uniformData_rectangle.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData_rectangle.vkBuffer, NULL);
        uniformData_rectangle.vkBuffer = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyBuffer() successful for uniformData_rectangle.vkBuffer\n");
    }

    // Descriptor Pool (Note: Descriptor Sets are automatically freed when pool is destroyed)
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyDescriptorPool() successful\n");
    }

    // Command Buffers
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[le]);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkFreeCommandBuffers() successful for %d\n", le);
    }

    if (vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkCommandBuffer_array freed successfully\n");
    }

    // Destroy Command Pool
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyCommandPool() successful\n");
    }

    // Free swapchain image views
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[le], NULL);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyImageView() successful for %d\n", le);
    }

    if (swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  swapchainImageView_array freed successfully\n");
    }

    // Free swapchain images
    // for(uint32_t le = 0; le < swapchainImageCount; ++le)
    // {
    //     vkDestroyImage(vkDevice, swapchainImage_array[le], NULL);
    //     __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyImage() successful for %d\n", le);
    //     fclose(gpLogFile);
    // }

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  swapchainImage_array freed successfully\n");
    }

    // Destroy Swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroySwapchainKHR() successful\n");
    }

    // No need to Destroy/Uninitialize Device Queue

    // Destroy Vulkan Device
    if (vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroy() successful \n");
    }

    // No Need to Destroy Selected Physical Device as it gets destroy with the Logical Device

    // Destroy Surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL); //  the vkDestroySurfaceKHR() is generic and not platform specific
        vkSurfaceKHR = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroySurfaceKHR() successful\n");
    }

    if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyDebugReportCallbackEXT_fnptr() successful\n");
    }

    // Destroy Vulkan Instance
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Uninitialize()  =>  vkDestroyInstance() successful\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()    >>>  function started\n");

    memset((void *)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
    memset((void *)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

    // Step 01 Fill and Initialize required extension names & count global variables
    vkResult = FillInstanceExtensionNames();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  FillInstanceExtensionNames() failed\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  FillInstanceExtensionNames() successful\n");
    }

    // Fill Validation Layer
    if (true == bValidation)
    {
        vkResult = FillValidationLayerNames();
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  FillValidationLayerNames() failed\n");
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  FillValidationLayerNames() successful\n");
        }
    }

    // Step 02 Initialize struct VkApplicationInfo
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_3; // Latest for Android CURRENTLY

    // Step 03  Initialize struct VkInstanceCreateInfo by using information from Step 01 and Step 02

    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    if (true == bValidation)
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> incompatible driver(%d)\n", vkResult);
        return (vkResult);
    }
    else if (VK_ERROR_EXTENSION_NOT_PRESENT == vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> extension not present(%d)\n", vkResult);
        return (vkResult);
    }
    else if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  vkCreateInstance() failed due to -> unknown reason(%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  vkCreateInstance() successful\n");
    }

    // DO FOR VALIDATION CALLBACKS
    if (true == bValidation)
    {
        vkResult = CreateValidationCallbackFunction();
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  CreateValidationCallbackFunction() failed\n");
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()   =>  CreateValidationCallbackFunction() successful\n");
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanInstance()    -->  function completed\n");

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
    VkBool32 androidSurfaceExtensionFound = VK_FALSE;
    VkBool32 debugReportExtensionFound = VK_FALSE;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()  >>>  function started\n");

    // Step 01: Find how many instance extensions are supported by the Vulkan Driver for this version & keep the count in the local variable

    // First Vulkan API
    vkResult = vkEnumerateInstanceExtensionProperties(NULL,                    // Layer Name : कोणत्या layer ची extensions हवी आहे NULL -> सर्वच हवे आहे
                                                      &instanceExtensionCount, // Count : ExtensionCount
                                                      NULL);                   // Extension च्या properties चा array (Parameterized Return Value)
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() failed - first call\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() successful - first call\n");
    }

    //  Step 02: Allocate & Fill struct VkExtensionProperties array corresponding to above count;
    vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkExtensionProperties_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  Out of memory while allocating vkExtensionProperties_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL,
                                                      &instanceExtensionCount,
                                                      vkExtensionProperties_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() failed - second call\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  vkEnumerateInstanceExtensionProperties() successful - second call\n");
    }

    //  Step 03: Fill & Display a local string array of extension names obtained from VkExtensionProperties array
    instanceExtensionNames_array = (char **)malloc(sizeof(char *) * instanceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == instanceExtensionNames_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  Out of memory while allocating instanceExtensionNames_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    //  Allocate memory for each string member
    for (uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        instanceExtensionNames_array[le] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == instanceExtensionNames_array[le])
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()   =>  Out of memory while allocating instanceExtensionNames_array[%d]\n", le);
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        memcpy(instanceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    =>  vulkan instance extension name = %s\n", instanceExtensionNames_array[le]);
    }

    //  Step 04: As not required henceforth, free the allocated VkExtensionProperties Array;
    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    //  Step 05: Find whether above extension names contained our required 2 extensions viz, VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
    for (uint32_t le = 0; le < instanceExtensionCount; ++le)
    {
        // VK_KHR_SURFACE_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_KHR_SURFACE_EXTENSION_NAME))
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        // VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
        {
            androidSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
        }

        // VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        if (0 == strcmp(instanceExtensionNames_array[le], VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            debugReportExtensionFound = VK_TRUE;
            if (true == bValidation)
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    =>  VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    =>  VK_KHR_SURFACE_EXTENSION_NAME found successfully\n");
    }

    if (VK_FALSE == androidSurfaceExtensionFound)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    =>  VK_KHR_ANDROID_SURFACE_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    =>  VK_KHR_ANDROID_SURFACE_EXTENSION_NAME found successfully\n");
    }

    if (VK_FALSE == debugReportExtensionFound)
    {
        if (true == bValidation)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED; //  Return hardcoded failure
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    ||>  validation is on but VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found\n");
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    ||>  vailidation is off and VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found\n");
        }
    }
    else
    {
        if (true == bValidation)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    ||>  validation is on and VK_EXT_DEBUG_REPORT_EXTENSION_NAME found successfully\n");
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    ||>  validation is off and VK_EXT_DEBUG_REPORT_EXTENSION_NAME found successfully\n");
        }
    }

    //  Step 08: Print only enabled extension names
    for (uint32_t le = 0; le < enabledInstanceExtensionCount; ++le)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    =>  Enabled vulkan instance extension name = %s\n", enabledInstanceExtensionNames_array[le]);
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "FillInstanceExtensionNames()    -->  function completed\n");

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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()    >>>  function started\n");

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() failed - first call\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() successful - first call\n");
    }

    vkLayerProperties_array = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * validationLayerCount);
    // FOR THE SAKE OF BREVITY

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() failed - second call\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()   =>  vkEnumerateInstanceLayerProperties() successful - second call\n");
    }

    validationLayerNames_array = (char **)malloc(sizeof(char *) * validationLayerCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == validationLayerNames_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()   =>  Out of memory while allocating validationLayerNames_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    //  Allocate memory for each string member
    for (uint32_t le = 0; le < validationLayerCount; ++le)
    {
        validationLayerNames_array[le] = (char *)malloc(sizeof(char) * (strlen(vkLayerProperties_array[le].layerName) + 1));
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == validationLayerNames_array[le])
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()   =>  Out of memory while allocating validationLayerNames_array[%d]\n", le);
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        memcpy(validationLayerNames_array[le], vkLayerProperties_array[le].layerName, strlen(vkLayerProperties_array[le].layerName) + 1);

        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()    =>  vulkan instance layer name = %s\n", validationLayerNames_array[le]);
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()    =>  VK_LAYER_KHRONOS_validation not found\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()    =>  VK_LAYER_KHRONOS_validation found successfully\n");
    }

    if (true == bValidation)
    {
        for (uint32_t le = 0; le < enabledValidationLayerCount; ++le)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()    ||>  Enabled vulkan instance layer name = %s\n", enabledValidationLayerName_array[le]);
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "FillValidationLayerNames()    -->  function completed\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()    >>>  function started\n");

    // Get the required function pointers
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
    if (NULL == vkCreateDebugReportCallbackEXT_fnptr)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() failed to get function pointer for vkCreateDebugReportCallbackEXT\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() successful to get function pointer for vkCreateDebugReportCallbackEXT\n");
    }

    vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    if (NULL == vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() failed to get function pointer for vkDestroyDebugReportCallbackEXT\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()    ==>  vkGetInstanceProcAddr() successful to get function pointer for vkDestroyDebugReportCallbackEXT\n");
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()   =>  vkCreateDebugReportCallbackEXT_fnptr() failed\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()   =>  vkCreateDebugReportCallbackEXT_fnptr() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateValidationCallbackFunction()    -->  function completed\n");
    return (vkResult);
}

//  STEP 04
VkResult GetSupportedSurface(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkAndroidSurfaceCreateInfoKHR vkAndroidSurfaceCreateInfoKHR;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetSupportedSurface()    >>>  function started\n");

    memset((void *)&vkAndroidSurfaceCreateInfoKHR, 0, sizeof(VkAndroidSurfaceCreateInfoKHR));

    vkAndroidSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    vkAndroidSurfaceCreateInfoKHR.pNext = NULL;
    vkAndroidSurfaceCreateInfoKHR.flags = 0;
    vkAndroidSurfaceCreateInfoKHR.window = androidNativeWindow;

    vkResult = vkCreateAndroidSurfaceKHR(vkInstance, &vkAndroidSurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetSupportedSurface()   =>  vkCreateAndroidSurfaceKHR() failed\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetSupportedSurface()   =>  vkCreateAndroidSurfaceKHR() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetSupportedSurface()    -->  function completed\n");

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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() >>>  function started\n");

    // Step 02: call vkEnumeratePhysicalDevices() function to get Physical device count.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() failed - first call - %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == physicalDeviceCount)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()     =>  physicalDeviceCount is 0\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() successful - first call\n");
    }

    // Step 03: Allocate VkPhysicalDevice array according to above count
    vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkPhysicalDevice_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  Out of memory while allocating vkPhysicalDevice_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    // Step 04: Call vkEnumeratePhysicalDevice() to fill above array
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() failed - second call - %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  vkEnumeratePhysicalDevices() successful - second call\n");
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
            __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  Out of memory while allocating vkQueueFamilyProperties_array\n");
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
            __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()   =>  Out of memory while allocating isQueueSurfaceSupported_array\n");
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
            __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  isQueueSurfaceSupported_array successfully freed\n");
        }

        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  vkQueueFamilyProperties_array successful freed\n");
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  successful in getting Physical Device with graphics enabled\n");
    }
    else
    {
        if (vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
            __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  vkPhysicalDevice_array successfully freed\n");
        }

        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() => failed to select PhysicalDevice \n");
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  selected physical device support Tessellation Shader\n");
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  selected physical device does not support Tessellation Shader\n");
    }

    // Step 11: By using geometryShader member of above structure, check ifselected devices supportsd GeometryShader;
    if (vkPhysicalDeviceFeatures.geometryShader)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  selected physical device support Geometry Shader\n");
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice() =>  selected physical device does not support Geometry Shader\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevice()    -->  function completed\n");
    return (vkResult);
}

VkResult PrintVkInfo(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "PrintVkInfo()    >>>  function started\n");

    NEWLINEINFILE;
    __android_log_print(ANDROID_LOG_INFO, "MST:", "**************************************************************************\n");
    __android_log_print(ANDROID_LOG_INFO, "MST:", "                     Vulkan Info\n");
    __android_log_print(ANDROID_LOG_INFO, "MST:", "**************************************************************************\n");
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "API Version = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

        // Device Name
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Name = %s\n", vkPhysicalDeviceProperties.deviceName);

        // Device Type
        switch (vkPhysicalDeviceProperties.deviceType)
        {

        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Type = Integrated GPU(iGPU)\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Type = Discrete GPU(dGPU)\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Type = Virtual GPU(vGPU)\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Type = CPU\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Type = OTHER\n");
            break;

        default:
            __android_log_print(ANDROID_LOG_INFO, "MST:", "Device Type = UNKNOWN\n");
            break;
        }

        // Vendor ID
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Vendor ID = 0x%04X\n", vkPhysicalDeviceProperties.vendorID);

        // Device ID
        __android_log_print(ANDROID_LOG_INFO, "MST:", "Device ID = 0x%04X\n", vkPhysicalDeviceProperties.deviceID);
    }
    NEWLINEINFILE;
    __android_log_print(ANDROID_LOG_INFO, "MST:", "**************************************************************************\n");
    NEWLINEINFILE;

    // Free Physical Device
    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "PrintVkInfo() =>  vkPhysicalDevice_array successfully freed\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "PrintVkInfo()    -->  function completed\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()  >>>  function started\n");

    // First Vulkan API
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    NULL);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() failed - first call\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() successful - first call\n");
    }

    vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkExtensionProperties_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  Out of memory while allocating vkExtensionProperties_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL,
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() failed - second call\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  vkEnumerateDeviceExtensionProperties() successful - second call\n");
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  number of found device extension count = %u\n", deviceExtensionCount);
    }

    deviceExtensionNames_array = (char **)malloc(sizeof(char *) * deviceExtensionCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == deviceExtensionNames_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  Out of memory while allocating deviceExtensionNames_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    //  Allocate memory for each string member
    for (uint32_t le = 0; le < deviceExtensionCount; ++le)
    {
        deviceExtensionNames_array[le] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[le].extensionName) + 1));
        //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
        if (NULL == deviceExtensionNames_array[le])
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()   =>  Out of memory while allocating deviceExtensionNames_array[%d]\n", le);
            return (VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        memcpy(deviceExtensionNames_array[le], vkExtensionProperties_array[le].extensionName, strlen(vkExtensionProperties_array[le].extensionName) + 1);

        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()    =>  vulkan device extension name = %s\n", deviceExtensionNames_array[le]);
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()    =>  VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()    =>  VK_KHR_SWAPCHAIN_EXTENSION_NAME found successfully\n");
    }

    for (uint32_t le = 0; le < enabledDeviceExtensionCount; ++le)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()    =>  Enabled vulkan device extension name = %s\n", enabledDeviceExtensionNames_array[le]);
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "FillDeviceExtensionNames()    -->  function completed\n");

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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanDevice()  >>>  function started\n");

    //  Step 07 - Device Extensions

    // Step 01 & Step 02
    vkResult = FillDeviceExtensionNames();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanDevice()    =>  FillDeviceExtensionNames() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanDevice()    =>  FillDeviceExtensionNames() successful\n");
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanDevice()    =>  vkCreateDevice() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanDevice()    =>  vkCreateDevice() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVulkanDevice()  -->  function completed\n");
    return (vkResult);
}

void GetDeviceQueue(void)
{
    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetDeviceQueue()  >>>  function started\n");

    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

    if (VK_NULL_HANDLE == vkQueue)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetDeviceQueue()  =>  vkGetDeviceQueue() resulted null for vkQueue\n");
        return;
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetDeviceQueue()  =>  vkGetDeviceQueue() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetDeviceQueue()  -->  function completed\n");
}

VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t formatCount = 0;
    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()  >>>  function started\n");

    // Get the count of supported surface color formats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed first call with %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == formatCount)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed first call %d that resulted in 0 formatCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() first call successful with %d format counts\n", formatCount);
    }

    vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkSurfaceFormatKHR_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  Out of memory while allocating vkSurfaceFormatKHR_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkSurfaceFormatKHR_array successfully allocated\n");
    }

    //  Filling the array
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() failed second call with %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  vkGetPhysicalDeviceSurfaceFormatsKHR() second call\n");
    }

    // MY LOOP TO JUST LOG AND SEE PRESENT MODE OF MY PHYSICAL DEVICE
    // for(uint32_t le = 0; le < formatCount; ++le)
    // {
    //     __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  found %d surface color format & %d color space\n", vkSurfaceFormatKHR_array[le].format, vkSurfaceFormatKHR_array[le].colorSpace);
    // }

    //  Decide the surface color format first
    if (1 == formatCount && VK_FORMAT_UNDEFINED == vkSurfaceFormatKHR_array[0].format)
    {
        vkFormat_color = VK_FORMAT_B8G8R8_UNORM;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  selected VK_FORMAT_B8G8R8_UNORM for surfaceColorFormat\n");
    }
    else
    {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  selected %d for surfaceColorFormat\n", vkFormat_color);
    }

    // Decide the color space
    vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()    =>  selected %d for color space\n", vkColorSpaceKHR);

    if (vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace() =>  vkSurfaceFormatKHR_array successfully freed\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDeviceSurfaceFormatAndColorSpace()  -->  function completed\n");
    return (vkResult);
}

VkResult GetPhysicalDevicePresentMode(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    uint32_t presentModeCount = 0;
    VkPresentModeKHR *vkPresentModeKHR_array = NULL;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()  >>>  function started\n");

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed first call with %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == presentModeCount)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed first call %d that resulted in 0 presentModeCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() first call successful with %d presentModeCount\n", presentModeCount);
    }

    vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkPresentModeKHR_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  Out of memory while allocating vkPresentModeKHR_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() failed second call with %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  vkGetPhysicalDeviceSurfacePresentModesKHR() second call successful with %d presentModeCount\n", presentModeCount);
    }

    // MY LOOP TO JUST LOG AND SEE PRESENT MODE OF MY PHYSICAL DEVICE
    for (uint32_t le = 0; le < presentModeCount; ++le)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  found %d present mode\n", vkPresentModeKHR_array[le]);
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  selected VK_PRESENT_MODE_FIFO_KHR for present mode\n");
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()    =>  selected VK_PRESENT_MODE_MAILBOX_KHR for present mode\n");
    }

    if (vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode() =>  vkPresentModeKHR_array successfully freed\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "GetPhysicalDevicePresentMode()  -->  function completed\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()  >>>  function started\n");

    // ColorFormat and ColorSpace
    vkResult = GetPhysicalDeviceSurfaceFormatAndColorSpace();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  GetPhysicalDeviceSurfaceFormatAndColorSpace() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  GetPhysicalDeviceSurfaceFormatAndColorSpace() successful\n");
    }

    // Get Physical Device Surface Capabilities
    memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceKHR));

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkGetPhysicalDeviceSurfaceCapabilitiesKHR() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  minImageCount %d\n", vkSurfaceCapabilitiesKHR.minImageCount);
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  maxImageCount %d\n", vkSurfaceCapabilitiesKHR.maxImageCount);

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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  swapchain image width x height = %d x %d\n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    else
    {
        // ifsurface size is already defined then swapchain image size must match withit
        VkExtent2D vkExtent2D;

        memset((void *)&vkExtent2D, 0, sizeof(vkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        // FOR MY LOGGING
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.minImageExtent.width %d\n", vkSurfaceCapabilitiesKHR.minImageExtent.width);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.maxImageExtent.width %d\n", vkSurfaceCapabilitiesKHR.maxImageExtent.width);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkExtent2D.width %d\n", vkExtent2D.width);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  selected width = %d\n", glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)));

        vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));

        // FOR MY LOGGING
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.minImageExtent.height %d\n", vkSurfaceCapabilitiesKHR.minImageExtent.height);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkSurfaceCapabilitiesKHR.maxImageExtent.height %d\n", vkSurfaceCapabilitiesKHR.maxImageExtent.height);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkExtent2D.height %d\n", vkExtent2D.height);
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  selected height = %d\n", glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)));

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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  in ifvkSurfaceTransformFlagBitsKHR = %d\n", vkSurfaceTransformFlagBitsKHR);
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
        // FOR MY LOGGING
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  in else vkSurfaceTransformFlagBitsKHR = %d\n", vkSurfaceTransformFlagBitsKHR);
    }

    // Present Mode
    vkResult = GetPhysicalDevicePresentMode();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  GetPhysicalDevicePresentMode() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  GetPhysicalDevicePresentMode() successful\n");
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
    // vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkCreateSwapchainKHR() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()    =>  vkCreateSwapchainKHR() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSwapchain()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateImagesAndImageViews(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkImageViewCreateInfo vkImageViewCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()  >>>  function started\n");

    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed first call with %d\n", vkResult);
        return (vkResult);
    }
    else if (0 == swapchainImageCount)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed first call %d that resulted in 0 swapchainImageCount\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() first call successful with %d swapchainImageCount\n", swapchainImageCount);
    }

    swapchainImage_array = (VkImage *)malloc(sizeof(VkImage) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == swapchainImage_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  malloc failed for swapchainImage_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() failed second call with %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkGetSwapchainImagesKHR() second successful with %d swapchainImageCount\n", swapchainImageCount);
    }

    swapchainImageView_array = (VkImageView *)malloc(sizeof(VkImageView) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == swapchainImageView_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  malloc failed for swapchainImageView_array\n");
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
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkCreateImageViews() failed for iteration %d. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()    =>  vkCreateImageViews() successful for iteration %d \n", le);
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateImagesAndImageViews()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateCommandPool(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandPool()  >>>  function started\n");

    memset((void *)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

    vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandPool()    =>  vkCreateCommandPool() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandPool()    =>  vkCreateCommandPool() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandPool()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateCommandBuffers(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandBuffers()  >>>  function started\n");

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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandBuffers()    =>  malloc failed for vkCommandBuffer_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandBuffers()    =>  vkAllocateCommandBuffers() failed for iteration %d. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandBuffers()    =>  vkAllocateCommandBuffers() successful for iteration %d \n", le);
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateCommandBuffers()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateVertexBuffer(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    float triangle_position[] = {
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    float triangle_color[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    float rectangle_position[] = {
        // Triangle 01
        1.0f, 1.0f, 0.0f,   // Top Right Vertex
        -1.0f, 1.0f, 0.0f,  // Top Left Vertex
        -1.0f, -1.0f, 0.0f, // Bottom Left Vertex

        // Triangle 02
        -1.0f, -1.0f, 0.0f, // Bottom Left Vertex,
        1.0f, -1.0f, 0.0f,  // Bottom Right Vertex
        1.0f, 1.0f, 0.0f    // Top Right Vertex
    };

    float rectangle_color[] = {
        0.0f, 0.0f, 1.0f, // Blue Color
        0.0f, 0.0f, 1.0f, // Blue Color
        0.0f, 0.0f, 1.0f, // Blue Color

        0.0f, 0.0f, 1.0f, // Blue Color
        0.0f, 0.0f, 1.0f, // Blue Color
        0.0f, 0.0f, 1.0f  // Blue Color
    };

    VkBufferCreateInfo vkBufferCreateInfo;
    VkMemoryRequirements vkMemoryRequirements;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    void *data = NULL;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()  >>>  function started\n");

    // Triangle
    {

        // Position
        memset((void *)&vertexData_position_triangle, 0, sizeof(VertexData));
        memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
        vkBufferCreateInfo.size = sizeof(rectangle_position);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position_triangle.vkBuffer);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() successful \n");
        }

        memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, vertexData_position_triangle.vkBuffer, &vkMemoryRequirements);

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

        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_triangle.vkDeviceMemory);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() successful \n");
        }

        vkResult = vkBindBufferMemory(vkDevice, vertexData_position_triangle.vkBuffer, vertexData_position_triangle.vkDeviceMemory, 0);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() successful \n");
        }

        vkResult = vkMapMemory(vkDevice, vertexData_position_triangle.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() successful \n");
        }

        memcpy(data, triangle_position, sizeof(triangle_position));

        vkUnmapMemory(vkDevice, vertexData_position_triangle.vkDeviceMemory);

        // Color

        memset((void *)&vertexData_color_triangle, 0, sizeof(VertexData));
        memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
        vkBufferCreateInfo.size = sizeof(rectangle_color);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_color_triangle.vkBuffer);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() successful \n");
        }

        memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, vertexData_color_triangle.vkBuffer, &vkMemoryRequirements);

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

        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_color_triangle.vkDeviceMemory);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() successful \n");
        }

        vkResult = vkBindBufferMemory(vkDevice, vertexData_color_triangle.vkBuffer, vertexData_color_triangle.vkDeviceMemory, 0);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() successful \n");
        }

        vkResult = vkMapMemory(vkDevice, vertexData_color_triangle.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() successful \n");
        }

        memcpy(data, triangle_color, sizeof(triangle_color));

        vkUnmapMemory(vkDevice, vertexData_color_triangle.vkDeviceMemory);
    }

    // Rectangle
    {
        
        // Position
        memset((void *)&vertexData_position_rectangle, 0, sizeof(VertexData));
        memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
        vkBufferCreateInfo.size = sizeof(rectangle_position);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position_rectangle.vkBuffer);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() successful \n");
        }

        memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, vertexData_position_rectangle.vkBuffer, &vkMemoryRequirements);

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

        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_rectangle.vkDeviceMemory);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() successful \n");
        }

        vkResult = vkBindBufferMemory(vkDevice, vertexData_position_rectangle.vkBuffer, vertexData_position_rectangle.vkDeviceMemory, 0);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() successful \n");
        }

        vkResult = vkMapMemory(vkDevice, vertexData_position_rectangle.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() successful \n");
        }

        memcpy(data, rectangle_position, sizeof(rectangle_position));

        vkUnmapMemory(vkDevice, vertexData_position_rectangle.vkDeviceMemory);

        // Color

        memset((void *)&vertexData_color_rectangle, 0, sizeof(VertexData));
        memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0; // Valid Flags are use in scattered/sparse buffer
        vkBufferCreateInfo.size = sizeof(rectangle_color);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_color_rectangle.vkBuffer);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkCreateBuffer() successful \n");
        }

        memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, vertexData_color_rectangle.vkBuffer, &vkMemoryRequirements);

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

        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_color_rectangle.vkDeviceMemory);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkAllocateMemory() successful \n");
        }

        vkResult = vkBindBufferMemory(vkDevice, vertexData_color_rectangle.vkBuffer, vertexData_color_rectangle.vkDeviceMemory, 0);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkBindBufferMemory() successful \n");
        }

        vkResult = vkMapMemory(vkDevice, vertexData_color_rectangle.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()    =>  vkMapMemory() successful \n");
        }

        memcpy(data, rectangle_color, sizeof(rectangle_color));

        vkUnmapMemory(vkDevice, vertexData_color_rectangle.vkDeviceMemory);
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateVertexBuffer()  -->  function completed\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()  >>>  function started\n");

    // Create Uniform Buffer
    memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // Triangle
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()  => For Triangle\n");
    memset((void *)&uniformData_triangle, 0, sizeof(UniformData));
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData_triangle.vkBuffer);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkCreateBuffer() successful \n");
    }

    // Get Buffer Memory Requirements
    vkGetBufferMemoryRequirements(vkDevice, uniformData_triangle.vkBuffer, &vkMemoryRequirements);

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

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_triangle.vkDeviceMemory);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkAllocateMemory() successful \n");
    }

    // Bind Buffer to Memory
    vkResult = vkBindBufferMemory(vkDevice, uniformData_triangle.vkBuffer, uniformData_triangle.vkDeviceMemory, 0);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkBindBufferMemory() successful \n");
    }
    
    // Rectangle
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()  => For Rectangle\n");
    memset((void *)&uniformData_rectangle, 0, sizeof(UniformData));
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData_rectangle.vkBuffer);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkCreateBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkCreateBuffer() successful \n");
    }

    // Get Buffer Memory Requirements
    vkGetBufferMemoryRequirements(vkDevice, uniformData_rectangle.vkBuffer, &vkMemoryRequirements);

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

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_rectangle.vkDeviceMemory);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkAllocateMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkAllocateMemory() successful \n");
    }

    // Bind Buffer to Memory
    vkResult = vkBindBufferMemory(vkDevice, uniformData_rectangle.vkBuffer, uniformData_rectangle.vkDeviceMemory, 0);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkBindBufferMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  vkBindBufferMemory() successful \n");
    }

    // Update Uniform Buffer
    vkResult = UpdateUniformBuffer();
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  UpdateUniformBuffer() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()    =>  UpdateUniformBuffer() successful \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateUniformBuffer()  -->  function completed\n");
    return (vkResult);
}

VkResult UpdateUniformBuffer(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    void *data = NULL;
    MyUniformData myUniformData;
    glm::mat4 translateMatrix = glm::mat4(1.0f);
    glm::mat4 rotateMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);


    // Code

    // Initialize Uniform Data

    // Triangle
    memset((void *)&myUniformData, 0, sizeof(MyUniformData));

    translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
    rotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 6.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at position
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    myUniformData.modelMatrix = translateMatrix * rotateMatrix;
    myUniformData.viewMatrix = viewMatrix;

    // Create Perspective Projection Matrix
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    // Map Memory and Copy Data
    vkResult = vkMapMemory(vkDevice, uniformData_triangle.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "UpdateUniformBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(MyUniformData));

    vkUnmapMemory(vkDevice, uniformData_triangle.vkDeviceMemory);

    // Rectangle
    vkResult = VK_SUCCESS;
    data = NULL;
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    perspectiveProjectionMatrix = glm::mat4(1.0f);
    memset((void *)&myUniformData, 0, sizeof(MyUniformData));

    translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
    rotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));

    viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 6.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at position
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    myUniformData.modelMatrix = translateMatrix * rotateMatrix;
    myUniformData.viewMatrix = viewMatrix;

    // Create Perspective Projection Matrix
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    // Map Memory and Copy Data
    vkResult = vkMapMemory(vkDevice, uniformData_rectangle.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "UpdateUniformBuffer()    =>  vkMapMemory() failed (%d)\n", vkResult);
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(MyUniformData));

    vkUnmapMemory(vkDevice, uniformData_rectangle.vkDeviceMemory);

    return (vkResult);
}

VkResult CreateShaders(void)
{
    // Variables Definition
    VkResult vkResult = VK_SUCCESS;

    // For Vertex Shader
    const char *szFileName = "shaders/shader.vert.spv";
    // FILE *fp = NULL;
    size_t size = 0;
    char *shaderData = NULL;
    int retVal = 0;
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    AAsset *asset = NULL;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  >>>  function started\n");

    // For Vertex Shader
    asset = AAssetManager_open(androidAssetManager, szFileName, AASSET_MODE_BUFFER);
    if (NULL == asset)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  failed to open vertex shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  successful to open vertex shader spv file\n");
    }

    size = AAsset_getLength(asset);
    if (0 == size)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  size is 0 of vertex shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    shaderData = (char *)malloc(sizeof(char) * size);

    retVal = AAsset_read(asset, shaderData, size);
    if (retVal != size)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  failed to read vertex shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  successful to read vertex shader spv file\n");
    }

    AAsset_close(asset);

    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //  RESERVED so must be 0
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  vkCreateShaderModule() failed for vertex shader (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  vkCreateShaderModule() successful for vertex shader \n");
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  vertex shader data successfully freed \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  vertex shader module successfully created \n");

    // For fragment shader
    szFileName = "shaders/shader.frag.spv";

    asset = NULL;
    
    asset = AAssetManager_open(androidAssetManager, szFileName, AASSET_MODE_BUFFER);
    if (NULL == asset)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  failed to open fragment shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  successful to open fragment shader spv file\n");
    }

    size = AAsset_getLength(asset);
    if (0 == size)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  size is 0 of fragment shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    shaderData = NULL;
    shaderData = (char *)malloc(sizeof(char) * size);

    retVal = 0;
    retVal = AAsset_read(asset, shaderData, size);
    if (retVal != size)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  failed to read fragment shader spv file\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  successful to read fragment shader spv file\n");
    }

    AAsset_close(asset);

    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; //  RESERVED so must be 0
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  vkCreateShaderModule() failed for fragment shader (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  vkCreateShaderModule() successful for fragment shader \n");
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  fragment shader data successfully freed \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()    =>  fragment shader module successfully created \n");

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateShaders()  -->  function completed\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSetLayout()  >>>  function started\n");

    memset((void *)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));

    vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //  This is the type of descriptor we are going to use
    vkDescriptorSetLayoutBinding.binding = 0;                                        //  This is actually the 0 binding point of shader
    vkDescriptorSetLayoutBinding.descriptorCount = 1;
    vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSetLayout()    =>  vkCreateDescriptorSetLayout() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSetLayout()    =>  vkCreateDescriptorSetLayout() successful \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSetLayout()  -->  function completed\n");
    return (vkResult);
}

VkResult CreatePipelineLayout(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipelineLayout()  >>>  function started\n");

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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipelineLayout()    =>  vkCreatePipelineLayout() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipelineLayout()    =>  vkCreatePipelineLayout() successful \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipelineLayout()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateDescriptorPool(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkDescriptorPoolSize vkDescriptorPoolSize;
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorPool()  >>>  function started\n");

    // Before Creating Actual Descriptor Pool. Vulkan Expects Descriptor pool size.
    memset((void *)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));

    vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize.descriptorCount = 2;

    // Create The Pool
    memset((void *)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = 1;
    vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
    vkDescriptorPoolCreateInfo.maxSets = 2;

    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorPool()    =>  vkCreateDescriptorPool() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorPool()    =>  vkCreateDescriptorPool() successful \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorPool()  -->  function completed\n");
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
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()  >>>  function started\n");

    // Triangle
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()  >>>  For Triangle\n");
    memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_triangle);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() successful \n");
    }

    // Describe if we want Buffer as Uniform or Image as Uniform

    // Specify Buffer Info
    memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_triangle.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);

    // Now connect the descriptor set to the actual buffer using write descriptor set
    memset((void *)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));

    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.pNext = NULL;
    vkWriteDescriptorSet.dstSet = vkDescriptorSet_triangle;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet.pImageInfo = NULL;
    vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet.pTexelBufferView = NULL;
    vkWriteDescriptorSet.dstBinding = 0;

    // ISKA KATA HUA PAANI BHI NAHI MANGTA HAI
    vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

    // Rectangle
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()  >>>  For Rectangle\n");
    memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_rectangle);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()    =>  vkAllocateDescriptorSets() successful \n");
    }

    // Describe if we want Buffer as Uniform or Image as Uniform

    // Specify Buffer Info
    memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_rectangle.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);

    // Now connect the descriptor set to the actual buffer using write descriptor set
    memset((void *)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));

    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.pNext = NULL;
    vkWriteDescriptorSet.dstSet = vkDescriptorSet_rectangle;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet.pImageInfo = NULL;
    vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet.pTexelBufferView = NULL;
    vkWriteDescriptorSet.dstBinding = 0;

    // ISKA KATA HUA PAANI BHI NAHI MANGTA HAI
    vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateDescriptorSet()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateRenderPass(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkAttachmentDescription vkAttachmentDescription_array[1];
    VkAttachmentReference vkAttachmentReference;
    VkSubpassDescription vkSubpassDescription;
    VkRenderPassCreateInfo vkRenderPassCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateRenderPass()  >>>  function started\n");

    memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    memset((void *)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference.attachment = 0; //  index of attachment of above array
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = NULL;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;

    memset((void *)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;

    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateRenderPass()    =>  vkCreateRenderPass() failed (%d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateRenderPass()    =>  vkCreateRenderPass() successful \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateRenderPass()  -->  function completed\n");
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
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()  >>>  function started\n");

    // VkVertexInputBindingDescription
    memset((void *)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    // For Position
    vkVertexInputBindingDescription_array[0].binding = 0;
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // For Color
    vkVertexInputBindingDescription_array[1].binding = 1;
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // VkVertexInputAttributeDescription
    memset((void *)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    // For Position
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;

    // For Color
    vkVertexInputAttributeDescription_array[1].binding = 1;
    vkVertexInputAttributeDescription_array[1].location = 1;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;

    // Vertex Input State
    memset((void *)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
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
    memset((void *)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; //  NO VALIDATION | NO ERROR | NO OUTPUT =

    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;

    memset((void *)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

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
    vkViewport.maxDepth = 0.0f;

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
    memset((void *)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()    =>  vkCreatePipelineCache() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()    =>  vkCreatePipelineCache() successful \n");
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
    vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
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
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()  -->  vkDestroyPipelineCache() successful\n");

        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()    =>  vkCreateGraphicsPipelines() failed %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()    =>  vkCreateGraphicsPipelines() successful \n");
    }

    // We are done using vkPipelineCache so destroy it
    if (vkPipelineCache)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()  -->  vkDestroyPipelineCache() successful\n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreatePipeline()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateFrameBuffers(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkImageView vkImageView_attachment_array[1];
    VkFramebufferCreateInfo vkFramebufferCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFrameBuffers()  >>>  function started\n");

    memset((void *)vkImageView_attachment_array, 0, _ARRAYSIZE(vkImageView_attachment_array));

    memset((void *)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.pNext = NULL;
    vkFramebufferCreateInfo.flags = 0;
    vkFramebufferCreateInfo.renderPass = vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
    vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
    vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
    vkFramebufferCreateInfo.layers = 1; //  VALIDATION ||> CASE 02

    vkFramebuffer_array = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkFramebuffer_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFrameBuffers()    =>  malloc failed for vkFramebuffer_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkImageView_attachment_array[0] = swapchainImageView_array[le];

        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFrameBuffers()    =>  vkCreateFramebuffer() failed for iteration %d. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFrameBuffers()    =>  vkCreateFramebuffer() successful for iteration %d \n", le);
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFrameBuffers()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateSemaphores(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSemaphores()  >>>  function started\n");

    memset((void *)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;
    vkSemaphoreCreateInfo.flags = 0; //  MUST BE 0, BECAUSE OF RESERVED

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backbuffer);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSemaphores()    =>  vkCreateSemaphore() failed for vkSemaphore_backbuffer %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSemaphores()    =>  vkCreateSemaphore() successful for vkSemaphore_backbuffer \n");
    }

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_rendercomplete);
    if (VK_SUCCESS != vkResult)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSemaphores()    =>  vkCreateSemaphore() failed for vkSemaphore_rendercomplete %d\n", vkResult);
        return (vkResult);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSemaphores()    =>  vkCreateSemaphore() successful for vkSemaphore_rendercomplete \n");
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateSemaphores()  -->  function completed\n");
    return (vkResult);
}

VkResult CreateFences(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;
    VkFenceCreateInfo vkFenceCreateInfo;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFences()  >>>  function started\n");

    memset((void *)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkFence_array = (VkFence *)malloc(sizeof(VkFence) * swapchainImageCount);
    //  FOR THE SAKE OF BREVITY WE AVOID ERROR CHECKIG BUT HERE THERE SHOULD BE ERROR CHECKING FOR MALLOC
    if (NULL == vkFence_array)
    {
        __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFences()    =>  malloc failed for vkFence_array\n");
        return (VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[le]);
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFences()    =>  vkCreateFence() failed for iteration %d (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFences()    =>  vkCreateFence() successful for iteration %d \n", le);
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "CreateFences()  -->  function completed\n");
    return (vkResult);
}

VkResult BuildCommandBuffers(void)
{
    // Variable Definition
    VkResult vkResult = VK_SUCCESS;

    // Code
    __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()  >>>  function started\n");

    // Loop Per Swapchain Image
    for (uint32_t le = 0; le < swapchainImageCount; ++le)
    {
        // Reset Command Buffers
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[le], 0); // 0 means -> Dont release the resources created by command pool for this command buffers
        if (VK_SUCCESS != vkResult)
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()    =>  vkResetCommandBuffer() failed for %d buffer. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()    =>  vkResetCommandBuffer() successful for %d buffer\n", le);
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
            __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()    =>  vkBeginCommandBuffer() failed for %d buffer. (%d)\n", le, vkResult);
            return (vkResult);
        }
        else
        {
            __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()    =>  vkBeginCommandBuffer() successful for %d buffer\n", le);
        }

        // Set Clear Values
        VkClearValue vkClearValue_array[1];
        memset((void *)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

        vkClearValue_array[0].color = vkClearColorValue;

        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void *)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[le];

        vkCmdBeginRenderPass(vkCommandBuffer_array[le], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // Bind with the pipeline
            vkCmdBindPipeline(vkCommandBuffer_array[le], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
            VkDeviceSize vkDeviceSize_offset_position[1];
            VkDeviceSize vkDeviceSize_offset_color[1];

            // Triangle
            {
                // Bind Descriptor Set to Pipeline
                vkCmdBindDescriptorSets(vkCommandBuffer_array[le],
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        vkPipelineLayout,
                                        0, // firstSet
                                        1, // descriptorSetCount
                                        &vkDescriptorSet_triangle,
                                        0,     // dynamicOffsetCount
                                        NULL); // pDynamicOffsets

                // Bind with the Vertex Buffer
                memset((void *)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 0, 1, &vertexData_position_triangle.vkBuffer, vkDeviceSize_offset_position);
                
                memset((void *)vkDeviceSize_offset_color, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_color));
                vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 1, 1, &vertexData_color_triangle.vkBuffer, vkDeviceSize_offset_color);

                //******************************************************************************************************
                // HERE WE SHOULD CALL VULKAN DRAWING FUNCTION
                //******************************************************************************************************

                /*********** BEGIN DRAWING ***********/

                vkCmdDraw(vkCommandBuffer_array[le], 6, 1, 0, 0);

                /************ END DRAWING ************/
            }

            // Rectangle
            {
                // Bind Descriptor Set to Pipeline
                vkCmdBindDescriptorSets(vkCommandBuffer_array[le],
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        vkPipelineLayout,
                                        0, // firstSet
                                        1, // descriptorSetCount
                                        &vkDescriptorSet_rectangle,
                                        0,     // dynamicOffsetCount
                                        NULL); // pDynamicOffsets

                // Bind with the Vertex Buffer
                memset((void *)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
                vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 0, 1, &vertexData_position_rectangle.vkBuffer, vkDeviceSize_offset_position);
                
                memset((void *)vkDeviceSize_offset_color, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_color));
                vkCmdBindVertexBuffers(vkCommandBuffer_array[le], 1, 1, &vertexData_color_rectangle.vkBuffer, vkDeviceSize_offset_color);

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
            __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()    =>  vkEndCommandBuffer() failed for %d buffer. (%d)\n", le, vkResult);
            return (vkResult);
        }
        // else
        // {
        //     __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()    =>  vkEndCommandBuffer() successful for %d buffer\n", le);
        // }
    }

    __android_log_print(ANDROID_LOG_INFO, "MST:", "BuildCommandBuffers()  -->  function completed\n");
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
    // __android_log_print(ANDROID_LOG_INFO, "MST:", "DebugReportCallback()  ||>  function started\n");

    __android_log_print(ANDROID_LOG_INFO, "MST:", "MST_VALIDATION : DebugReportCallback()  ||>     %s %d = %s\n", pLayerPrefix, messageCode, pMessage);

    // __android_log_print(ANDROID_LOG_INFO, "MST:", "DebugReportCallback()  ||>  function completed\n");

    return (VK_FALSE);
}

/***********************************************************************************\
\***********************************************************************************/
