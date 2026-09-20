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

typedef struct
{
	struct android_app *app;
	bool bActive;
} Engine;

ANativeWindow *androidNativeWindow = NULL;

// Global Callback Function Declarations
void engine_handle_cmd(struct android_app *app, int32_t cmd);
int32_t engine_handle_input(struct android_app *app, AInputEvent *event);

void android_main(struct android_app *state)
{
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
		}
		else
		{
			androidNativeWindow = NULL;
			__android_log_print(ANDROID_LOG_INFO, "MST:", "Failed to initialize window\n");
		}
		engine->bActive = true;
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

	return (0);
}
