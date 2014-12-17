/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL.h>

#include <3dlite/3dlite_video.h>

#ifdef PLATFORM_Windows
#include <3dlite/GL/wglew.h>
#elif defined PLATFORM_Linux
#include <3dlite/GL/glxew.h>
#endif

static lite3d_video_settings gVideoSettings;

static struct
{
    int32_t format;
    char descr[35];
} gBuiltInScreenPixelFormats[] = {
    {SDL_PIXELFORMAT_UNKNOWN, "SDL_PIXELFORMAT_UNKNOWN"},
    {SDL_PIXELFORMAT_INDEX1LSB, "SDL_PIXELFORMAT_INDEX1LSB"},
    {SDL_PIXELFORMAT_INDEX1MSB, "SDL_PIXELFORMAT_INDEX1MSB"},
    {SDL_PIXELFORMAT_INDEX4LSB, "SDL_PIXELFORMAT_INDEX4LSB"},
    {SDL_PIXELFORMAT_INDEX4MSB, "SDL_PIXELFORMAT_INDEX4MSB"},
    {SDL_PIXELFORMAT_INDEX8, "SDL_PIXELFORMAT_INDEX8"},
    {SDL_PIXELFORMAT_RGB332, "SDL_PIXELFORMAT_RGB332"},
    {SDL_PIXELFORMAT_RGB444, "SDL_PIXELFORMAT_RGB444"},
    {SDL_PIXELFORMAT_RGB555, "SDL_PIXELFORMAT_RGB555"},
    {SDL_PIXELFORMAT_BGR555, "SDL_PIXELFORMAT_BGR555"},
    {SDL_PIXELFORMAT_ARGB4444, "SDL_PIXELFORMAT_ARGB4444"},
    {SDL_PIXELFORMAT_RGBA4444, "SDL_PIXELFORMAT_RGBA4444"},
    {SDL_PIXELFORMAT_ABGR4444, "SDL_PIXELFORMAT_ABGR4444"},
    {SDL_PIXELFORMAT_BGRA4444, "SDL_PIXELFORMAT_BGRA4444"},
    {SDL_PIXELFORMAT_ARGB1555, "SDL_PIXELFORMAT_ARGB1555"},
    {SDL_PIXELFORMAT_RGBA5551, "SDL_PIXELFORMAT_RGBA5551"},
    {SDL_PIXELFORMAT_ABGR1555, "SDL_PIXELFORMAT_ABGR1555"},
    {SDL_PIXELFORMAT_BGRA5551, "SDL_PIXELFORMAT_BGRA5551"},
    {SDL_PIXELFORMAT_RGB565, "SDL_PIXELFORMAT_RGB565"},
    {SDL_PIXELFORMAT_RGB565, "SDL_PIXELFORMAT_RGB565"},
    {SDL_PIXELFORMAT_BGR565, "SDL_PIXELFORMAT_BGR565"},
    {SDL_PIXELFORMAT_RGB24, "SDL_PIXELFORMAT_RGB24"},
    {SDL_PIXELFORMAT_BGR24, "SDL_PIXELFORMAT_BGR24"},
    {SDL_PIXELFORMAT_RGB888, "SDL_PIXELFORMAT_RGB888"},
    {SDL_PIXELFORMAT_RGBX8888, "SDL_PIXELFORMAT_RGBX8888"},
    {SDL_PIXELFORMAT_BGR888, "SDL_PIXELFORMAT_BGR888"},
    {SDL_PIXELFORMAT_BGRX8888, "SDL_PIXELFORMAT_BGRX8888"},
    {SDL_PIXELFORMAT_ARGB8888, "SDL_PIXELFORMAT_ARGB8888"},
    {SDL_PIXELFORMAT_RGBA8888, "SDL_PIXELFORMAT_RGBA8888"},
    {SDL_PIXELFORMAT_ABGR8888, "SDL_PIXELFORMAT_ABGR8888"},
    {SDL_PIXELFORMAT_BGRA8888, "SDL_PIXELFORMAT_BGRA8888"},
    {SDL_PIXELFORMAT_ARGB2101010, "SDL_PIXELFORMAT_ARGB2101010"},
    {0, ""}
};

static SDL_Window *gRenderWindow = NULL;
static SDL_GLContext gGLContext = NULL;

static int init_platform_gl_extensions(void)
{
#ifdef PLATFORM_Windows

#elif defined PLATFORM_Linux

#endif

    return LITE3D_TRUE;
}

static int init_gl_extensions(void)
{

    return init_platform_gl_extensions();
}

int lite3d_setup_video(const lite3d_video_settings *settings)
{
    uint32_t windowFlags;
    SDL_DisplayMode displayMode;
    int i;

    gVideoSettings = *settings;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, gVideoSettings.colorBits > 24 ? 8 : 0);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 16);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    if (gVideoSettings.FSAA > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, gVideoSettings.FSAA);
    }

    windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    if (gVideoSettings.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }

    /* setup render window */
    gRenderWindow = SDL_CreateWindow(
        gVideoSettings.caption,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        gVideoSettings.screenWidth,
        gVideoSettings.screenHeight,
        windowFlags);

    if (!gRenderWindow)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: SDL_CreateWindow failed..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: render window created %d/%d (%s)",
        __FUNCTION__,
        gVideoSettings.screenWidth,
        gVideoSettings.screenHeight,
        gVideoSettings.fullscreen ? "fullscreen" : "windowed");

    /* Create an OpenGL context associated with the window. */
    gGLContext = SDL_GL_CreateContext(gRenderWindow);
    if (!gGLContext)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL Context create failed..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    /* set gl context */
    SDL_GL_MakeCurrent(gRenderWindow, gGLContext);

    SDL_GetWindowDisplayMode(gRenderWindow, &displayMode);
    for (i = 0; gBuiltInScreenPixelFormats[i].format != 0; ++i)
    {
        if (displayMode.format == gBuiltInScreenPixelFormats[i].format)
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "%s: closest pixel format is %s", __FUNCTION__,
                gBuiltInScreenPixelFormats[i].descr);
            break;
        }
    }

    SDL_GL_SetSwapInterval(gVideoSettings.vsync ? 1 : 0);

    /* enable multisample buffers */
    if (gVideoSettings.FSAA > 1 && GLEW_ARB_multisample)
        glEnable(GL_MULTISAMPLE_ARB);

    if (!init_gl_extensions())
    {
        lite3d_close_video();
        return LITE3D_FALSE;
    }

    SDL_ShowWindow(gRenderWindow);
    return LITE3D_TRUE;
}

const lite3d_video_settings *lite3d_get_video_settings(void)
{
    return &gVideoSettings;
}

int lite3d_close_video(void)
{

    return LITE3D_TRUE;
}
