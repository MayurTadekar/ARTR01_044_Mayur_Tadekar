

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

// Macros
#define     WIN_WIDTH   800
#define     WIN_HEIGHT  600
#define     fprintf(stream, ...) mylog((stream), __VA_ARGS__)

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
const char* gpszWindowTitle = "MST: Linux - Window Stub"; // Window title

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

// Entry point function
int main(int argc, char* argv[])
{
    // Function declarations
    void ToggleFullscreen(void);
    Bool isWindowMinimized(void);
    int initialize(void);
    void resize(int, int);
    void display(void);
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
    gpLogFile = fopen("Log.txt", "w");
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
    int iResult = initialize();
    if (0 != iResult)
    {
        fprintf(gpLogFile, "main() -> initialize() failed with error code: %d. Exiting...\n", iResult);
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
                        resize(giWindowWidth, giWindowHeight);
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
            if (False == gbWindowMinimized)
            {
                update();
                display();
            }
            
            if(True == gbEscapeKeyPressed)
            {
                bDone = True;
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
    // Atom wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    // Atom wm_state_hidden = XInternAtom(gpDisplay, "_NET_WM_STATE_HIDDEN", False);
    // Atom actual_type;
    // int actual_format;
    // unsigned long nitems, bytes_after;
    // unsigned char *prop = NULL;

    // if (XGetWindowProperty(gpDisplay, gWindow, wm_state, 0, (~0L), False, AnyPropertyType,
    //                        &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success) 
    // {
    //     if (prop != NULL) 
    //     {
    //         for (unsigned long i = 0; i < nitems; i++) 
    //         {
    //             Atom state_atom = ((Atom*)prop)[i];
    //             if (state_atom == wm_state_hidden) 
    //             {
    //                 XFree(prop);
    //                 return True; // Window is minimized
    //             }
    //         }
    //         XFree(prop);
    //     }
    // }
    // return False; // Window is not minimized

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

int initialize(void)
{
    // Code

    return(0);
}

void resize(int width, int height)
{
    // Code
}

void display(void)
{
    // Code
}

void update(void)
{
    // Code
}

void uninitialize(void)
{
    // Function declarations
    void ToggleFullscreen(void);

    // Code

    // restore window if it is in fullscreen mode
    if (True == gbFullscreen)
    {
        ToggleFullscreen();
        gbFullscreen = False;
    }

    if(gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
        gWindow = 0;
    }

    if(gColormap)
    {
        XFreeColormap(gpDisplay, gColormap);
        gColormap = 0;
    }

    if(gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }

    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }

    if(gpLogFile)
    {
        fprintf(gpLogFile, "uninitialize() -> Log file closed successfully.\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
}
