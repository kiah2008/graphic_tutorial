// #include <xf86drm.h>
// #include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

/*
 This is based on the triangle.c example from https://github.com/matusnovak/rpi-opengl-without-x

 This requires OpenGL ES 3.1:

	1. Install libraries:
		sudo apt-get install libegl1-mesa-dev libgbm-dev libgles2-mesa-dev libdrm-dev -y
	2. Compile:
		gcc -o rpi4_compute rpi4_compute.c -ldrm -lgbm -lEGL -lGLESv2 -I/usr/include/libdrm -I/usr/include/GLES31
	3. Run:
		./rpi4_compute
*/

// The following code related to DRM/GBM was adapted from the following sources:
// https://github.com/eyelash/tutorials/blob/master/drm-gbm.c
// and
// https://www.raspberrypi.org/forums/viewtopic.php?t=243707#p1499181
//
// I am not the original author of this code, I have only modified it.
static int device;
static struct gbm_device* gbmDevice;
//struct gbm_surface *gbmSurface;

static int getDisplay(EGLDisplay* display)
{
	drmModeRes* resources = drmModeGetResources(device);
	if (resources == NULL)
	{
		fprintf(stderr, "Unable to get DRM resources\n");
		return -1;
	}

	drmModeFreeResources(resources);
	gbmDevice = gbm_create_device(device);
	//gbmSurface = gbm_surface_create(gbmDevice, mode.hdisplay, mode.vdisplay, GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	*display = eglGetDisplay(gbmDevice);
	return 0;
}

static int matchConfigToVisual(EGLDisplay display, EGLint visualId, EGLConfig* configs, int count)
{
	EGLint id;
	for (int i = 0; i < count; ++i)
	{
		if (!eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID, &id))
			continue;
		if (id == visualId)
			return i;
	}
	return -1;
}

static void gbmClean()
{
	//gbm_surface_destroy(gbmSurface);
	gbm_device_destroy(gbmDevice);
}

// The following code was adopted from
// https://github.com/matusnovak/rpi-opengl-without-x/blob/master/triangle.c
// and is licensed under the Unlicense.
static const EGLint configAttribs[] = {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_DEPTH_SIZE, 8,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
	EGL_NONE };

static const EGLint contextAttribs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 3,
	EGL_NONE };

// The following are GLSL shaders for rendering a triangle on the screen
#define STRINGIFY(x) #x

static const char* computeShaderCode =
"#version 310 es\n"
"    // layout (std140, binding = 0) buffer inputBuffer {\n"
"    //     float data[]\n"
"    // } inBuffer;\n"
"    // layout (std140, binding = 1) buffer destBuffer {\n"
"    //     float data[]\n"
"    // } outBuffer;\n"
"    void main() {\n"
"        ivec2 coord = ivec2(gl_GlobalInvocationID.xy);\n"
"        //outBuffer[coord] = inputBuf[coord];\n"
"    }\n";

// Get the EGL error back as a string. Useful for debugging.
static const char* eglGetErrorStr()
{
	switch (eglGetError())
	{
	case EGL_SUCCESS:
		return "The last function succeeded without error.";
	case EGL_NOT_INITIALIZED:
		return "EGL is not initialized, or could not be initialized, for the "
			"specified EGL display connection.";
	case EGL_BAD_ACCESS:
		return "EGL cannot access a requested resource (for example a context "
			"is bound in another thread).";
	case EGL_BAD_ALLOC:
		return "EGL failed to allocate resources for the requested operation.";
	case EGL_BAD_ATTRIBUTE:
		return "An unrecognized attribute or attribute value was passed in the "
			"attribute list.";
	case EGL_BAD_CONTEXT:
		return "An EGLContext argument does not name a valid EGL rendering "
			"context.";
	case EGL_BAD_CONFIG:
		return "An EGLConfig argument does not name a valid EGL frame buffer "
			"configuration.";
	case EGL_BAD_CURRENT_SURFACE:
		return "The current surface of the calling thread is a window, pixel "
			"buffer or pixmap that is no longer valid.";
	case EGL_BAD_DISPLAY:
		return "An EGLDisplay argument does not name a valid EGL display "
			"connection.";
	case EGL_BAD_SURFACE:
		return "An EGLSurface argument does not name a valid surface (window, "
			"pixel buffer or pixmap) configured for GL rendering.";
	case EGL_BAD_MATCH:
		return "Arguments are inconsistent (for example, a valid context "
			"requires buffers not supplied by a valid surface).";
	case EGL_BAD_PARAMETER:
		return "One or more argument values are invalid.";
	case EGL_BAD_NATIVE_PIXMAP:
		return "A NativePixmapType argument does not refer to a valid native "
			"pixmap.";
	case EGL_BAD_NATIVE_WINDOW:
		return "A NativeWindowType argument does not refer to a valid native "
			"window.";
	case EGL_CONTEXT_LOST:
		return "A power management event has occurred. The application must "
			"destroy all contexts and reinitialise OpenGL ES state and "
			"objects to continue rendering.";
	default:
		break;
	}
	return "Unknown error!";
}

int main()
{
	EGLDisplay display;
	// You can try chaning this to "card0" if "card1" does not work.
	device = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (getDisplay(&display) != 0)
	{
		fprintf(stderr, "Unable to get EGL display\n");
		close(device);
		return -1;
	}

	// Other variables we will need further down the code.
	int major, minor;
	GLuint program, cs, vbo;
	GLint posLoc, colorLoc, result;

	if (eglInitialize(display, &major, &minor) == EGL_FALSE)
	{
		fprintf(stderr, "Failed to get EGL version! Error: %s\n",
			eglGetErrorStr());
		eglTerminate(display);
		gbmClean();
		return EXIT_FAILURE;
	}

	// Make sure that we can use OpenGL in this EGL app.
	eglBindAPI(EGL_OPENGL_API);

	printf("Initialized EGL version: %d.%d\n", major, minor);

	EGLint count;
	EGLint numConfigs;
	eglGetConfigs(display, NULL, 0, &count);
	EGLConfig* configs = malloc(count * sizeof(configs));

	if (!eglChooseConfig(display, configAttribs, configs, count, &numConfigs))
	{
		fprintf(stderr, "Failed to get EGL configs! Error: %s\n",
			eglGetErrorStr());
		eglTerminate(display);
		gbmClean();
		return EXIT_FAILURE;
	}

	// I am not exactly sure why the EGL config must match the GBM format.
	// But it works!
	int configIndex = matchConfigToVisual(display, GBM_FORMAT_XRGB8888, configs, numConfigs);
	if (configIndex < 0)
	{
		fprintf(stderr, "Failed to find matching EGL config! Error: %s\n",
			eglGetErrorStr());
		eglTerminate(display);
		//gbm_surface_destroy(gbmSurface);
		gbm_device_destroy(gbmDevice);
		return EXIT_FAILURE;
	}

	EGLContext context =
		eglCreateContext(display, configs[configIndex], EGL_NO_CONTEXT, contextAttribs);
	if (context == EGL_NO_CONTEXT)
	{
		fprintf(stderr, "Failed to create EGL context! Error: %s\n",
			eglGetErrorStr());
		eglTerminate(display);
		gbmClean();
		return EXIT_FAILURE;
	}

	// No rendering, no surface, just compute
	EGLSurface surface = EGL_NO_SURFACE;

	free(configs);

	eglMakeCurrent(display, surface, surface, context);

	printf("OpenGL information:\n");
	printf("  version: \"%s\"\n", glGetString(GL_VERSION));
	printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
	printf("  extensions: \"%s\"\n", glGetString(GL_EXTENSIONS));
	printf("===================================\n");

	program = glCreateProgram();
	if (!program)
	{
		fprintf(stderr, "Failed to create GL program. Error: %s\n",
			eglGetErrorStr());
		eglTerminate(display);
		gbmClean();
		return EXIT_FAILURE;
	}
	cs = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(cs, 1, &computeShaderCode, NULL);
	glCompileShader(cs);
	int rvalue;
	glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(cs, 10239, &length, log);
		fprintf(stderr, "Compiler log:\n%s\n", log);
		exit(40);
	}
	glAttachShader(program, cs);
	glLinkProgram(program);
	glGetShaderiv(program, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(program, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
		exit(41);
	}
	glUseProgram(program);
	// GLint inputLoc = glGetUniformLocation(program, "inputBuf");
	// GLint outputLoc = glGetUniformLocation(program, "outputBuf");
	// printf("input loc: %i, output loc: %i \n", inputLoc, outputLoc);
	// glUniform2f(inputLoc, 2.5, 3.4);
	glDispatchCompute(1, 1, 1);


	// // Get vertex attribute and uniform locations
	// posLoc = glGetAttribLocation(program, "pos");
	// colorLoc = glGetUniformLocation(program, "color");

	// // Set the desired color of the triangle to pink
	// // 100% red, 0% green, 50% blue, 100% alpha
	// glUniform4f(colorLoc, 1.0, 0.0f, 0.5, 1.0);

	// // Set our vertex data
	// glEnableVertexAttribArray(posLoc);
	// glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
	//                       (void *)0);


	// Cleanup
	eglDestroyContext(display, context);
	//eglDestroySurface(display, surface);
	eglTerminate(display);
	gbmClean();

	close(device);
	return EXIT_SUCCESS;
}