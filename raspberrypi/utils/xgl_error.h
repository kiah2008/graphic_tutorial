#include <EGL/egl.h>
#include <GL/gl.h>

const char *egl_get_error_str()
{
    switch (eglGetError())
    {
    case EGL_SUCCESS:
        return "The last function succeeded without error.";
        break;
    case EGL_NOT_INITIALIZED:
        return "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
        break;
    case EGL_BAD_ACCESS:
        return "EGL cannot access a requested resource (for example a context is bound in another thread).";
        break;
    case EGL_BAD_ALLOC:
        return "EGL failed to allocate resources for the requested operation.";
        break;
    case EGL_BAD_ATTRIBUTE:
        return "An unrecognized attribute or attribute value was passed in the attribute list.";
        break;
    case EGL_BAD_CONTEXT:
        return "An EGLContext argument does not name a valid EGL rendering context.";
        break;
    case EGL_BAD_CONFIG:
        return "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
        break;
    case EGL_BAD_CURRENT_SURFACE:
        return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
        break;
    case EGL_BAD_DISPLAY:
        return "An EGLDisplay argument does not name a valid EGL display connection.";
        break;
    case EGL_BAD_SURFACE:
        return "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
        break;
    case EGL_BAD_MATCH:
        return "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface).";
        break;
    case EGL_BAD_PARAMETER:
        return "One or more argument values are invalid.";
        break;
    case EGL_BAD_NATIVE_PIXMAP:
        return "A NativePixmapType argument does not refer to a valid native pixmap.";
        break;
    case EGL_BAD_NATIVE_WINDOW:
        return "A NativeWindowType argument does not refer to a valid native window.";
        break;
    case EGL_CONTEXT_LOST:
        return "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering. ";
        break;
    default:
        return "Unknown error";
        break;
    }
}



#define GL_CHECK_ERROR() do {\
for(int i = glGetError(); i != GL_NO_ERROR; i = glGetError()) { \
        fprintf(stderr, "gl error %s:%d %d",__FUNCTION__, __LINE__, i); \
    } \
} while (0);
