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
 *******************************************************************************/
#include <SDL_log.h>
#include <SDL_syswm.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_video.h>

static SDL_Window *gRenderWindow = NULL;
static SDL_GLContext gGLContext = NULL;

static int init_platform_gl_extensions(lite3d_video_settings *settings)
{
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(gRenderWindow, &wminfo))
        return LITE3D_FALSE;

#ifndef GLES
#   ifdef PLATFORM_Windows
    if (!WGLEW_ARB_extensions_string)
        return LITE3D_FALSE;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: WGL Extensions %s", LITE3D_CURRENT_FUNCTION,
        (char *) wglGetExtensionsStringARB(GetDC(wminfo.info.win.window)));

#   elif defined PLATFORM_Linux
    if (!GLXEW_VERSION_1_3)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLX v1.3 not supported..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Client %s", LITE3D_CURRENT_FUNCTION,
        (char *) glXGetClientString(wminfo.info.x11.display, 1));
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Server %s", LITE3D_CURRENT_FUNCTION,
        (char *) glXQueryServerString(wminfo.info.x11.display, 0, 1));
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Extensions %s", LITE3D_CURRENT_FUNCTION,
        (char *) glXQueryExtensionsString(wminfo.info.x11.display, 0));
#   endif
#endif

    return LITE3D_TRUE;
}

static int init_gl_extensions(lite3d_video_settings *settings)
{
    const char *extensionsStr;
    int32_t extensionsStrLength;

    if (!lite3d_init_gl_extensions_binding())
        return LITE3D_FALSE;

    if (!lite3d_check_gl_version())
        return LITE3D_FALSE;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Version: %s", LITE3D_CURRENT_FUNCTION, (const char *) glGetString(GL_VERSION));
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Vendor: %s", LITE3D_CURRENT_FUNCTION, (const char *) glGetString(GL_VENDOR));
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Renderer: %s", LITE3D_CURRENT_FUNCTION, (const char *) glGetString(GL_RENDERER));
    
    extensionsStr = (const char *) glGetString(GL_EXTENSIONS);
    extensionsStrLength = (int32_t)strlen(extensionsStr);

    while (extensionsStrLength >= 0)
    {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL Extensions: %s", LITE3D_CURRENT_FUNCTION, extensionsStr);
        extensionsStr += SDL_MAX_LOG_MESSAGE;
        extensionsStrLength -= SDL_MAX_LOG_MESSAGE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Shading Lang %s", LITE3D_CURRENT_FUNCTION, (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

    /* enable multisample buffers */
#ifndef GLES
    if (settings->FSAA > 1 && GLEW_ARB_multisample)
        glEnable(GL_MULTISAMPLE_ARB);
#endif

    return init_platform_gl_extensions(settings);
}

int lite3d_video_open(lite3d_video_settings *settings, int hideConsole)
{
    uint32_t windowFlags;
    SDL_DisplayMode displayMode;

    SDL_assert(settings);

#ifdef PLATFORM_Windows
    if (hideConsole)
        FreeConsole();
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
    
    /* Specify openGL context */
#ifndef GLES
    if (settings->FSAA > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, settings->FSAA);
    }

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

#else
#ifdef WITH_GLES2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif WITH_GLES3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
#endif
    
    windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    if (settings->fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }

    if (settings->screenWidth == 0 || settings->screenHeight == 0)
    {
        if (!lite3d_video_get_display_size(&settings->screenWidth,
            &settings->screenHeight))
            return LITE3D_FALSE;
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
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: SDL_CreateWindow failed..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: render window created %dx%d (%s)",
        LITE3D_CURRENT_FUNCTION,
        settings->screenWidth,
        settings->screenHeight,
        settings->fullscreen ? "fullscreen" : "windowed");

    /* Create an OpenGL context associated with the window. */
    gGLContext = SDL_GL_CreateContext(gRenderWindow);
    if (!gGLContext)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL Context create failed..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    /* set gl context */
    SDL_GL_MakeCurrent(gRenderWindow, gGLContext);

    SDL_GetWindowDisplayMode(gRenderWindow, &displayMode);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: selected pixel format: %d bpp, %s", LITE3D_CURRENT_FUNCTION,
        SDL_BITSPERPIXEL(displayMode.format), SDL_GetPixelFormatName(displayMode.format));

    SDL_GL_SetSwapInterval(settings->vsync ? 1 : 0);

    if (!init_gl_extensions(settings))
    {
        lite3d_video_close();
        return LITE3D_FALSE;
    }

    if (!settings->hidden)
        SDL_ShowWindow(gRenderWindow);
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
                return;
        }

        SDL_SetWindowSize(gRenderWindow, width, height);
    }
}

void lite3d_video_set_fullscreen(int8_t flag)
{
    if (gRenderWindow)
    {
        SDL_SetWindowFullscreen(gRenderWindow, flag ? SDL_WINDOW_FULLSCREEN : 0);
    }
}

int lite3d_video_get_display_size(int32_t *width, int32_t *height)
{
    SDL_DisplayMode displayMode;
    if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: SDL_GetDesktopDisplayMode failed..", LITE3D_CURRENT_FUNCTION);
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