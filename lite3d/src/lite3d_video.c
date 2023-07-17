/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <SDL_log.h>
#include <SDL_syswm.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_video.h>


static SDL_Window *gRenderWindow = NULL;
static SDL_GLContext gGLContext = NULL;

#ifndef GLES
static void print_extensions_string(const char *label, const char *extensionString)
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

static int init_platform_gl_extensions(lite3d_video_settings *settings)
{
    SDL_SysWMinfo wminfo;

    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(gRenderWindow, &wminfo))
    {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_GetWindowWMInfo failed: %s",
            SDL_GetError());

        return LITE3D_TRUE;
    }

#ifdef PLATFORM_Windows

    if (!WGLEW_ARB_extensions_string)
    {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "WGLEW_ARB_extensions_string is not supported");

        return LITE3D_TRUE;
    }

    print_extensions_string("WGL", wglGetExtensionsStringARB(GetDC(wminfo.info.win.window)));

#elif defined PLATFORM_Linux

    if (!GLXEW_VERSION_1_3)
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLX v1.3 not supported..",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Client %s",
        LITE3D_CURRENT_FUNCTION,
        glXGetClientString(wminfo.info.x11.display, 1));

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Server %s",
        LITE3D_CURRENT_FUNCTION,
        glXQueryServerString(wminfo.info.x11.display, 0, 1));

    print_extensions_string("GLX", glXQueryExtensionsString(wminfo.info.x11.display, 0));

#endif

    return LITE3D_TRUE;
}
#endif

static int init_gl_extensions(lite3d_video_settings *settings)
{
    if (!lite3d_init_gl_extensions_binding())
    {
        SDL_LogDebug(
          SDL_LOG_CATEGORY_APPLICATION,
          "%s: lite3d_init_gl_extensions_binding failed",
          LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    if (!lite3d_check_gl_version())
    {
        SDL_LogDebug(
          SDL_LOG_CATEGORY_APPLICATION,
          "%s: lite3d_check_gl_version failed",
          LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Version: %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_VERSION));

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Vendor: %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_VENDOR));

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Renderer: %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_RENDERER));

    SDL_LogDebug(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Shading Lang %s",
        LITE3D_CURRENT_FUNCTION,
        (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

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
    if (settings->FSAA > 1 && GLEW_ARB_multisample)
        glEnable(GL_MULTISAMPLE_ARB);

    return init_platform_gl_extensions(settings);

#else
    return LITE3D_TRUE;
#endif
}

void set_opengl_version(lite3d_video_settings *settings)
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
    uint32_t windowFlags;
    SDL_DisplayMode displayMode;

    SDL_assert(settings);

#ifdef PLATFORM_Windows
    if (hideConsole)
    {
        FreeConsole();
    }
#endif

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, settings->colorBits > 24 ? 8 : 0);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 16);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifndef GLES
    /* Specify openGL context */
    if (settings->FSAA > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, settings->FSAA);
    }

    if (settings->glProfile == LITE3D_GL_PROFILE_CORE)
    {
      SDL_LogInfo(
          SDL_LOG_CATEGORY_APPLICATION,
          "Setting Core OpenGL Profile");

      SDL_GL_SetAttribute(
          SDL_GL_CONTEXT_PROFILE_MASK,
          SDL_GL_CONTEXT_PROFILE_CORE);

      set_opengl_version(settings);
    }
    else if (settings->glProfile == LITE3D_GL_PROFILE_COMPATIBILITY)
    {
      SDL_LogInfo(
          SDL_LOG_CATEGORY_APPLICATION,
          "Setting Compatibility OpenGL Profile");

      SDL_GL_SetAttribute(
          SDL_GL_CONTEXT_PROFILE_MASK,
          SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

      set_opengl_version(settings);
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

    windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    if (settings->fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }

    if (settings->screenWidth == 0 || settings->screenHeight == 0)
    {
        if (!lite3d_video_get_display_size(
              &settings->screenWidth,
              &settings->screenHeight))
        {
          SDL_LogWarn(
              SDL_LOG_CATEGORY_APPLICATION,
              "lite3d_video_get_display_size failed");

            return LITE3D_FALSE;
        }
    }

    /* setup render window */
    gRenderWindow = SDL_CreateWindow(
        settings->caption,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        settings->screenWidth,
        settings->screenHeight,
        windowFlags);

    if (!gRenderWindow)
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: SDL_CreateWindow failed..",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: render window created %dx%d (%s)",
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
            "%s: GL Context create failed..",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    /* set gl context */
    SDL_GL_MakeCurrent(gRenderWindow, gGLContext);

    SDL_GetWindowDisplayMode(gRenderWindow, &displayMode);
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "%s: selected pixel format: %d bpp, %s",
        LITE3D_CURRENT_FUNCTION,
        SDL_BITSPERPIXEL(displayMode.format),
        SDL_GetPixelFormatName(displayMode.format));

    SDL_GL_SetSwapInterval(settings->vsync ? 1 : 0);

    if (!init_gl_extensions(settings))
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
    SDL_GL_DeleteContext(gGLContext);
    SDL_DestroyWindow(gRenderWindow);
    return LITE3D_TRUE;
}

void lite3d_video_swap_buffers(void)
{
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
    SDL_DisplayMode displayMode;

    if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: SDL_GetDesktopDisplayMode failed..",
            LITE3D_CURRENT_FUNCTION);

        return LITE3D_FALSE;
    }

    *width = displayMode.w;
    *height = displayMode.h;
    return LITE3D_TRUE;
}

void lite3d_video_view_system_cursor(int8_t flag)
{
    SDL_ShowCursor(flag == LITE3D_TRUE ? 1 : 0);
}
