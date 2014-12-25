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
#include <string.h>

#include <SDL_log.h>
#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_main.h>

#define DEFAULT_WIDTH           800
#define DEFAULT_HEIGHT          600

static GLfloat xrot = 0;
static GLfloat yrot = 0;
static GLfloat zrot = 0;

static lite3d_resource_pack *mFileSysPack = NULL;
static lite3d_resource_pack *m7zPack = NULL;
static lite3d_texture_unit *mNormandy = NULL, *mMinigun = NULL;

static int process_events(SDL_Event *levent, void *userdata)
{
    if (levent->type == SDL_KEYDOWN)
    {
        /* exit */
        if (levent->key.keysym.sym == SDLK_ESCAPE)
            return LITE3D_FALSE;
            /* print render stats */
        else if (levent->key.keysym.sym == SDLK_F1)
        {
            lite3d_render_stats *stats = lite3d_get_render_stats();
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "==== Render statistics ========\n"
                "last FPS\tavr FPS\t\tbest FPS\tworst FPS\n"
                "%d\t\t%d\t\t%d\t\t%d\n"
                "last frame ms\tavr frame ms\tbest frame ms\tworst frame ms\n"
                "%f\t%f\t%f\t%f",
                stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
                stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs);
        }
    }

    return LITE3D_TRUE;
}

static int init(void *userdata)
{
    if (!(mFileSysPack = lite3d_open_pack("tests/", LITE3D_FALSE, 700000)))
        return LITE3D_FALSE;
    if (!(m7zPack = lite3d_open_pack("tests/pack.1", LITE3D_TRUE, 700000)))
        return LITE3D_FALSE;

    if (!(mNormandy = lite3d_texture_unit_from_resource_pack(m7zPack,
        "pack/normandy/t1.jpg", LITE3D_IMAGE_JPG, LITE3D_TEXTURE_2D,
        LITE3D_TEXTURE_QL_NICEST)))
        return LITE3D_FALSE;
    if (!(mMinigun = lite3d_texture_unit_from_resource_pack(mFileSysPack,
        "pack/minigun.dds", LITE3D_IMAGE_DDS, LITE3D_TEXTURE_2D,
        LITE3D_TEXTURE_QL_NICEST)))
        return LITE3D_FALSE;

    lite3d_add_render_target(1, 10, 
        10, 0, NULL)->enabled = LITE3D_FALSE;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glViewport(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat) DEFAULT_WIDTH / (GLfloat) DEFAULT_HEIGHT, 0.1f, 100.0f);

    return LITE3D_TRUE;
}

static int shutdown(void *userdata)
{
    lite3d_texture_unit_purge(mNormandy);
    lite3d_texture_unit_purge(mMinigun);
    lite3d_close_pack(mFileSysPack);
    lite3d_close_pack(m7zPack);

    return LITE3D_TRUE;
}

static int frame(void *userdata)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);

    lite3d_texture_unit_bind(mMinigun);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glEnd();

    lite3d_texture_unit_unbind(mMinigun);
    lite3d_texture_unit_bind(mNormandy);

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glEnd();

    lite3d_texture_unit_unbind(mNormandy);

    xrot += 0.3f;
    yrot += 0.2f;
    zrot += 0.4f;

    return LITE3D_TRUE;
}

int main(int argc, char *args[])
{
    lite3d_global_settings settings;
    memset(&settings, 0, sizeof (settings));

    settings.logLevel = LITE3D_LOGLEVEL_VERBOSE;
    settings.textureSettings.anisotropy = 8;
    settings.textureSettings.useGLCompression = LITE3D_TRUE;
    settings.videoSettings.FSAA = 4;
    strcpy(settings.videoSettings.caption, "TEST window");
    settings.videoSettings.colorBits = 32;
    settings.videoSettings.fullscreen = LITE3D_FALSE;
    settings.videoSettings.screenWidth = DEFAULT_WIDTH;
    settings.videoSettings.screenHeight = DEFAULT_HEIGHT;
    settings.videoSettings.vsync = LITE3D_TRUE;
    settings.renderLisneters.processEvent = process_events;
    settings.renderLisneters.preRender = init;
    settings.renderLisneters.postRender = shutdown;
    settings.renderLisneters.renderFrame = frame;

    return !lite3d_main(&settings);
}