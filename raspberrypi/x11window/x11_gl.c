#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <bcm_host.h>
#include <gbm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>

#include <utils/xgl_error.h>

#define WIDTH 800
#define HEIGHT 600

void create_window(Display **display, Window *window)
{
    // Open X11 display and create window
    Display *x11_display = XOpenDisplay(NULL);
    int screen = DefaultScreen(x11_display);
    Window x11_window = XCreateSimpleWindow(x11_display, RootWindow(x11_display, screen), 10, 10, 640, 480, 1,
                                            BlackPixel(x11_display, screen), WhitePixel(x11_display, screen));
    XStoreName(x11_display, x11_window, "x11 gles");
    XMapWindow(x11_display, x11_window);

    // Return
    *display = x11_display;
    *window = x11_window;
}


static int matchConfigToVisual(EGLDisplay display, EGLint visualId, EGLConfig *configs, int count)
{
    EGLint id;
    for (int i = 0; i < count; ++i)
    {
        if (!eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID, &id)) {
            fprintf(stderr, "fail to query visual id  %d\n", i);
            continue;
        }
        printf("visual id %d, while expect %d\n", visualId, id);
        if (id == visualId)
            return i;
    }
    return -1;
}

void initialize_egl(Display *x11_display, Window x11_window, EGLDisplay *egl_display, EGLContext *egl_context, EGLSurface *egl_surface)
{
    // get an EGL display connection
    EGLDisplay display = eglGetDisplay(x11_display);
    assert(display != EGL_NO_DISPLAY);

    // initialize the EGL display connection
    int major, minor;
    eglInitialize(display, &major, &minor);

    // Set OpenGL rendering API
    eglBindAPI(EGL_OPENGL_API);

    printf("Initialized EGL version: %d.%d\n", major, minor);

    // get an appropriate EGL frame buffer configuration
    EGLint count;

    EGLConfig *configs = malloc(count * sizeof(EGLConfig));
    EGLint numConfigs;
    EGLint const attribute_list_config[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE};

    eglGetConfigs(display, NULL, 0, &count);
    printf("egl get config %d\n", count);

    if (!eglChooseConfig(display, attribute_list_config, configs, 1, &numConfigs))
    {
        fprintf(stderr, "Failed to get EGL configs! Error: %s\n",
                egl_get_error_str());
        eglTerminate(display);
        return;
    }

    // int configIndex = matchConfigToVisual(display, GBM_FORMAT_XRGB8888, configs, numConfigs);
    // if (configIndex < 0)
    // {
    //     fprintf(stderr, "Failed to find matching EGL config! Error: %s\n",
    //             egl_get_error_str());
    //     eglTerminate(display);
    //     return EXIT_FAILURE;
    // }
    int configIndex = 0;
    // create an EGL rendering context
    EGLint const attrib_list[] = {
        // EGL_CONTEXT_MAJOR_VERSION, 3,
        // EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE};
    EGLContext context = eglCreateContext(display, configs[configIndex], EGL_NO_CONTEXT, attrib_list);
    if (context == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "Failed to create EGL context! Error: %s\n",
                egl_get_error_str());
        eglTerminate(display);
        return;
    }
    // create an EGL window surface
    EGLSurface surface = eglCreateWindowSurface(display, configs[configIndex], x11_window, NULL);

    // connect the context to the surface
    eglMakeCurrent(display, surface, surface, context);
    
    char* glVersion = (char*)glGetString(GL_VERSION);
    printf("GL Version: %s\n", glVersion);

    // Return
    *egl_display = display;
    *egl_context = context;
    *egl_surface = surface;
}

static void draw(float progress)
{
    GL_CHECK_ERROR();
    glClearColor(1.0f - progress, progress, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    printf("draw %2.f\n", progress * 100);
}

int main()
{
    // Create X11 window
    Display *display;
    Window window;
    create_window(&display, &window);

    // Initialize EGL
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;
    initialize_egl(display, window, &egl_display, &egl_context, &egl_surface);

    if (egl_context == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "Unable to create GL context\n");
        fprintf(stderr, "%d\n", glGetError());
        exit(1);
    }

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

    XMapWindow(display, window);

    bool quit = false;
    while (!quit)
    {
        while (XPending(display) > 0)
        {
            XEvent event = {0};
            XNextEvent(display, &event);
            if (event.type == KeyPress)
            {
                KeySym keysym = XLookupKeysym(&event.xkey, 0);
                if (keysym == XK_Escape)
                {
                    quit = true;
                }
            }
        }

        glClearColor(1.0, 0.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < 100; i++)
        {
            draw(i / 100.0f);
            eglSwapBuffers(egl_display, egl_surface);
            usleep(33*1000);//us
        }
        break;
    }

    XCloseDisplay(display);
}