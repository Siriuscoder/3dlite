/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <lite3d/lite3d_video.h>

#include <SDL_log.h>
#include <SDL_video.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_render.h>


static SDL_Window *gRenderWindow = NULL;
static SDL_GLContext gGLContext = NULL;
static char gVideoVendor[256] = {0};

#ifndef GLES

static void lite3d_print_gl_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, 
    const GLchar *message, const void *userParam) 
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "DebugContext: %s: %s(%d): %s: %s",
        source == GL_DEBUG_SOURCE_API ? "API" : (
        source == GL_DEBUG_SOURCE_WINDOW_SYSTEM ? "Window System" : (
        source == GL_DEBUG_SOURCE_SHADER_COMPILER ? "Shader Compiler" : (
        source == GL_DEBUG_SOURCE_THIRD_PARTY ? "Third Party" : (
        source == GL_DEBUG_SOURCE_APPLICATION ? "Application" : (
        source == GL_DEBUG_SOURCE_OTHER ? "Other" : "Unknown"))))),

        type == GL_DEBUG_TYPE_ERROR ? "Error" : (
        type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "Deprecated Behavior" : (
        type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ? "Undefined Behavior" : (
        type == GL_DEBUG_TYPE_PORTABILITY ? "Portability" : (
        type == GL_DEBUG_TYPE_PERFORMANCE ? "Performance" : (
        type == GL_DEBUG_TYPE_OTHER ? "Other" : "Unknown"))))),

        id,

        severity == GL_DEBUG_SEVERITY_HIGH ? "High" : (
        severity == GL_DEBUG_SEVERITY_MEDIUM ? "Medium" : (
        severity == GL_DEBUG_SEVERITY_LOW ? "Low" : (
        severity == GL_DEBUG_SEVERITY_NOTIFICATION ? "Notification" : "Unknown"))),

        message);
}

static void lite3d_setup_gl_debug_context(void)
{
    if (lite3d_check_debug_context())
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(lite3d_print_gl_debug_message, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DebugContext: OpenGL debug context has been enabled");
    }
}

static void lite3d_print_extensions_string(const char *label, const char *extensionString)
{
    if (extensionString)
    {
        char *extensionStringCopy = lite3d_strdup(extensionString);
        char *extension = strtok(extensionStringCopy, " ");

        while (extension) 
        {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s: %s Extension %s", 
                LITE3D_CURRENT_FUNCTION, label, extension);
            extension = strtok(NULL, " ");
        }

        lite3d_free(extensionStringCopy);
    }
    else
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: %s extension string is null", 
            LITE3D_CURRENT_FUNCTION, label);
    }
}

static int lite3d_init_platform_gl_extensions(lite3d_video_settings *settings)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(gRenderWindow);
    if (!props) 
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: falied to get window properties: %s",
            LITE3D_CURRENT_FUNCTION,
            SDL_GetError());

        return LITE3D_FALSE;
    }

#ifdef PLATFORM_Windows

    if (!WGLEW_ARB_extensions_string)
    {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: WGLEW_ARB_extensions_string is not supported",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_TRUE;
    }
    else
    {
        HDC hdc = (HDC)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HDC_POINTER, NULL);
        if (hdc)
        {
            lite3d_print_extensions_string("WGL", wglGetExtensionsStringARB(hdc));
        }
        else
        {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_APPLICATION,
                "%s: failed to get window HDC",
                LITE3D_CURRENT_FUNCTION);
        }
    }

#elif defined(PLATFORM_Linux)

    if (!GLXEW_VERSION_1_3)
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLX v1.3 is not supported..",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }
    else
    {
        Display *display = (Display *)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        int screen = (int)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_SCREEN_NUMBER, 0);

        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLX Client %s",
            LITE3D_CURRENT_FUNCTION,
            glXGetClientString(display, 1));

        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLX Server %s",
            LITE3D_CURRENT_FUNCTION,
            glXQueryServerString(display, screen, 1));

        lite3d_print_extensions_string("GLX", glXQueryExtensionsString(display, 0));
    }
#endif

    return LITE3D_TRUE;
}
#endif

static int lite3d_init_gl_extensions(lite3d_video_settings *settings)
{
    if (!lite3d_init_gl_extensions_binding())
    {
        SDL_LogCritical(
          SDL_LOG_CATEGORY_APPLICATION,
          "%s: lite3d_init_gl_extensions_binding failed",
          LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    if (!lite3d_check_gl_version())
    {
        SDL_LogCritical(
          SDL_LOG_CATEGORY_APPLICATION,
          "%s: lite3d_check_gl_version failed",
          LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Version: %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_VERSION));

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Vendor: %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_VENDOR));

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Renderer: %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_RENDERER));

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Shading Lang %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

    strncpy(gVideoVendor, (const char *) glGetString(GL_VENDOR), sizeof(gVideoVendor)-1);

#ifdef WITH_GLES2
    const char *extensionsStr = (const char *) glGetString(GL_EXTENSIONS);
    if (extensionsStr)
    {
        int32_t extensionsStrLength = (int32_t)strlen(extensionsStr);
        while (extensionsStrLength >= 0)
        {
            SDL_LogDebug(
                SDL_LOG_CATEGORY_APPLICATION,
                "%s: GL Extensions: %s",
                LITE3D_CURRENT_FUNCTION,
                extensionsStr);

            extensionsStr += SDL_MAX_LOG_MESSAGE;
            extensionsStrLength -= SDL_MAX_LOG_MESSAGE;
        }
    }
#else
    GLint ext_id = 0, numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (ext_id = 0; ext_id < numExtensions; ++ext_id)
    {
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL Extension (%u): %s",
            LITE3D_CURRENT_FUNCTION, ext_id,
            glGetStringi(GL_EXTENSIONS, ext_id));
    }
#endif

#ifndef GLES
    /* enable multisample buffers */
    if (settings->MSAA > 1 && GLEW_ARB_multisample)
    {
        glEnable(GL_MULTISAMPLE_ARB);
    }

    if (settings->debug)
    {
        lite3d_setup_gl_debug_context();
    }

    return lite3d_init_platform_gl_extensions(settings);

#else
    return LITE3D_TRUE;
#endif
}

static void lite3d_set_opengl_version(lite3d_video_settings *settings)
{
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "Setting OpenGL Version: %d.%d",
        settings->glVersionMajor,
        settings->glVersionMinor);

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_MAJOR_VERSION, settings->glVersionMajor);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_MINOR_VERSION, settings->glVersionMinor);
}

int lite3d_video_open(lite3d_video_settings *settings, int hideConsole)
{
    int32_t contexFlags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    SDL_PropertiesID windowProps = SDL_CreateProperties();
    const SDL_DisplayMode *displayMode = NULL;

    SDL_assert(settings);

    if (windowProps == 0) 
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: failed to crate SDL window properties: %s",
            LITE3D_CURRENT_FUNCTION, SDL_GetError());

        return LITE3D_FALSE;
    }

#ifdef PLATFORM_Windows
    if (hideConsole)
    {
        FreeConsole();
    }
#endif

    if (settings->debug)
    {
        contexFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, settings->colorBits > 24 ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contexFlags);

#ifndef GLES
    /* Specify openGL context */
    if (settings->MSAA > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, settings->MSAA);
    }

    if (settings->glProfile == LITE3D_GL_PROFILE_CORE)
    {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "Setting Core OpenGL Profile");

        SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);

        lite3d_set_opengl_version(settings);
    }
    else if (settings->glProfile == LITE3D_GL_PROFILE_COMPATIBILITY)
    {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "Setting Compatibility OpenGL Profile");

        SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

        lite3d_set_opengl_version(settings);
    }
    else
    {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "Using Default OpenGL Profile");

        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "Using Default OpenGL Version");
    }
#endif

#ifdef WITH_GLES2
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#elif WITH_GLES3
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    /* setup render window */
    SDL_SetStringProperty(windowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, settings->caption);
    SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, false);
    SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);

    if (settings->fullscreen)
    {
        SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
        SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
    }

    if (settings->screenWidth == 0 || settings->screenHeight == 0)
    {
        if (!lite3d_video_get_display_size(
            &settings->screenWidth,
            &settings->screenHeight))
        {
            return LITE3D_FALSE;
        }
    }

    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, settings->screenWidth);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, settings->screenHeight);

    /* create render window */
    if (!(gRenderWindow = SDL_CreateWindowWithProperties(windowProps)))
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: falied to create render window: %s",
            LITE3D_CURRENT_FUNCTION,
            SDL_GetError());

        return LITE3D_FALSE;
    }

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: render window has been created %dx%d (%s)",
        LITE3D_CURRENT_FUNCTION,
        settings->screenWidth,
        settings->screenHeight,
        settings->fullscreen ? "fullscreen" : "windowed");

    /* Create an OpenGL context associated with the window. */
    gGLContext = SDL_GL_CreateContext(gRenderWindow);
    if (!gGLContext)
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL Context create failed: %s",
            LITE3D_CURRENT_FUNCTION,
            SDL_GetError());

        return LITE3D_FALSE;
    }

    /* set gl context */
    SDL_GL_MakeCurrent(gRenderWindow, gGLContext);

    displayMode = SDL_GetDesktopDisplayMode(SDL_GetDisplayForWindow(gRenderWindow));
    if (displayMode)
    {
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: display mode: %d bpp, %s, density %.2f, refresh rate %.2f Hz",
            LITE3D_CURRENT_FUNCTION,
            SDL_BITSPERPIXEL(displayMode->format),
            SDL_GetPixelFormatName(displayMode->format),
            displayMode->pixel_density,
            displayMode->refresh_rate);
    }

    SDL_GL_SetSwapInterval(settings->vsync ? 1 : 0);

    if (!lite3d_init_gl_extensions(settings))
    {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "init_gl_extensions failed");

        lite3d_video_close();

        return LITE3D_FALSE;
    }

    if (!settings->hidden)
    {
        SDL_ShowWindow(gRenderWindow);
    }

    return LITE3D_TRUE;
}

int lite3d_video_close(void)
{
    SDL_assert(gRenderWindow);
    SDL_assert(gGLContext);

    // Finish opengl queue
    lite3d_video_wait_async_complete();
    // Destroy opengl context
    SDL_GL_MakeCurrent(gRenderWindow, NULL);
    SDL_GL_DestroyContext(gGLContext);
    // Pump events before destroying window due to SDL3 buggy stuck behaviour of SDL_DestroyWindow
    SDL_PumpEvents();
    SDL_DestroyWindow(gRenderWindow);
    return LITE3D_TRUE;
}

void lite3d_video_swap_buffers(void)
{
    SDL_assert(gRenderWindow);
    SDL_GL_SwapWindow(gRenderWindow);
}

void lite3d_video_set_mouse_pos(int32_t x, int32_t y)
{
    SDL_WarpMouseInWindow(gRenderWindow, x, y);
}

void lite3d_video_resize(int32_t width, int32_t height)
{
    if (gRenderWindow)
    {
        if (width == 0 || height == 0)
        {
            if (!lite3d_video_get_display_size(&width, &height))
            {
                return;
            }
        }

        SDL_SetWindowSize(gRenderWindow, width, height);
    }
}

void lite3d_video_set_fullscreen(int8_t flag)
{
    if (gRenderWindow)
    {
        SDL_SetWindowFullscreen(
            gRenderWindow,
            flag ? SDL_WINDOW_FULLSCREEN : 0);
    }
}

int lite3d_video_get_display_size(int32_t *width, int32_t *height)
{
    const SDL_DisplayMode *displayMode = NULL;
    SDL_DisplayID displayId;
    // Render window is not created yet, first call 
    if (!gRenderWindow)
    {
        displayId = SDL_GetPrimaryDisplay();
    }
    else
    {
        displayId = SDL_GetDisplayForWindow(gRenderWindow);
    }

    if (!(displayMode = SDL_GetDesktopDisplayMode(displayId)))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: failed to get display info: %s",
            LITE3D_CURRENT_FUNCTION, SDL_GetError());

        return LITE3D_FALSE;
    }

    *width = displayMode->w;
    *height = displayMode->h;
    return LITE3D_TRUE;
}

void lite3d_video_view_system_cursor(int8_t flag)
{
    flag ? SDL_ShowCursor() : SDL_HideCursor();
}

void lite3d_video_wait_async_complete(void)
{
    glFinish();
}

const char *lite3d_video_get_vendor(void)
{
    return gVideoVendor;
}
