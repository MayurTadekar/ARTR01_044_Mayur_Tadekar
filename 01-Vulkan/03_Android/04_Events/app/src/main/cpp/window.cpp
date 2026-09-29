/***************************************************************************
 * S.G.M.P
 * @file: window.cpp
 * @brief:
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: Saturday 2026-09-19 10:11:25 AM
 ***************************************************************************/

#include <android_native_app_glue.h> // Everything related with pure native activities needs this file, mainly entry point function android_main() and struct android_app
#include <android/log.h>			 // For android_log_print() -> Android che printf();

#include <memory.h> // for memset()
#include <math.h>

typedef struct
{
	struct android_app *app;
	bool bActive;
} Engine;

ANativeWindow *androidNativeWindow = NULL;

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

// Global Callback Function Declarations
void engine_handle_cmd(struct android_app *app, int32_t cmd);
int32_t engine_handle_input(struct android_app *app, AInputEvent *event);

void android_main(struct android_app *state)
{

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
	}
}

void engine_handle_cmd(struct android_app *app, int32_t cmd)
{
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
			__android_log_print(ANDROID_LOG_INFO, "MST:", "Window initialized\n");
		
			// Draw Background Color With Pixel ByPixel Coloring By CPU
			ANativeWindow_Buffer buffer;
			uint32_t *pixels = NULL;
			uint32_t color;

			// Set the buffer geometry and format
			ANativeWindow_setBuffersGeometry(androidNativeWindow, 0, 0, WINDOW_FORMAT_RGBA_8888);
			if (0 == ANativeWindow_lock(androidNativeWindow, &buffer, NULL))
			{
				pixels = (uint32_t *)buffer.bits;
				color = 0xFFFF00FF; // Magenta color - For Little Endian Format - ABGR
				
				for (int y = 0; y < buffer.height; y++)
				{
					for (int x = 0; x < buffer.width; x++)
					{
						pixels[y * buffer.stride + x] = color;
					}
				}
				ANativeWindow_unlockAndPost(androidNativeWindow);
			}
			engine->bActive = true;
		}
		else
		{
			androidNativeWindow = NULL;
			__android_log_print(ANDROID_LOG_INFO, "MST:", "Failed to initialize window\n");
		}
		break;

	case APP_CMD_TERM_WINDOW: // WM_DESTROY
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
