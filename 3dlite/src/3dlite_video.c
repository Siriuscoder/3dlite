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
#include <SDL_syswm.h>



#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_render.h>
#include <3dlite/3dlite_video.h>

#ifdef PLATFORM_Windows
#include <3dlite/GL/wglew.h>
#elif defined PLATFORM_Linux
#include <3dlite/GL/glxew.h>
#endif

static lite3d_video_settings gVideoSettings;

static SDL_Window *gRenderWindow = NULL;
static SDL_GLContext gGLContext = NULL;

static int init_platform_gl_extensions(void)
{
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(gRenderWindow, &wminfo))
        return LITE3D_FALSE;

#ifdef PLATFORM_Windows
    if (!WGLEW_ARB_extensions_string)
        return LITE3D_FALSE;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: WGL Extensions %s", __FUNCTION__,
        (char *) wglGetExtensionsStringARB(GetDC(wminfo.info.win.window)));

#elif defined PLATFORM_Linux
    if (!GLXEW_VERSION_1_3)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GLX v1.3 not supported..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Client %s", __FUNCTION__,
        (char *) glXGetClientString(wminfo.info.x11.display, 1));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Server %s", __FUNCTION__,
        (char *) glXQueryServerString(wminfo.info.x11.display, 0, 1));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GLX Extensions %s", __FUNCTION__,
        (char *) glXQueryExtensionsString(wminfo.info.x11.display, 0));
#endif

    return LITE3D_TRUE;
}

static int init_gl_extensions(void)
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Glew failed.. %s\n", __FUNCTION__, glewGetErrorString(err));
    }
    
    if (!GL_VERSION_3_0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: GL v3.0 minimum required..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Version %s", __FUNCTION__, (char *) glGetString(GL_VERSION));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Vendor %s", __FUNCTION__, (char *) glGetString(GL_VENDOR));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Renderer %s", __FUNCTION__, (char *) glGetString(GL_RENDERER));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Extensions %s", __FUNCTION__, (char *) glGetString(GL_EXTENSIONS));
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: GL Shading Lang %s", __FUNCTION__, (char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

    /* enable multisample buffers */
    if (gVideoSettings.FSAA > 1 && GLEW_ARB_multisample)
        glEnable(GL_MULTISAMPLE_ARB);


    return init_platform_gl_extensions();
}

int lite3d_video_setup(const lite3d_video_settings *settings)
{
    uint32_t windowFlags;
    SDL_DisplayMode displayMode;

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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "%s: selected pixel format: %d bpp, %s", __FUNCTION__,
        SDL_BITSPERPIXEL(displayMode.format), SDL_GetPixelFormatName(displayMode.format));

    SDL_GL_SetSwapInterval(gVideoSettings.vsync ? 1 : 0);

    if (!init_gl_extensions())
    {
        lite3d_video_close();
        return LITE3D_FALSE;
    }

    if(!gVideoSettings.hidden)
        SDL_ShowWindow(gRenderWindow);
    return LITE3D_TRUE;
}

const lite3d_video_settings *lite3d_video_get_settings(void)
{
    return &gVideoSettings;
}

int lite3d_video_close(void)
{
    SDL_GL_DeleteContext(gGLContext);
    SDL_DestroyWindow(gRenderWindow);
    return LITE3D_TRUE;
}

void lite3d_video_register_root_render_target(void)
{
    lite3d_render_target_add(0, gVideoSettings.screenWidth, 
        gVideoSettings.screenHeight, LITE3D_TRUE, NULL);
}

void lite3d_video_swap_buffers(void)
{
    SDL_GL_SwapWindow(gRenderWindow);
}