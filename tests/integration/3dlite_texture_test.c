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
#include <3dlite/3dlite_camera.h>

#define DEFAULT_WIDTH           800
#define DEFAULT_HEIGHT          600

static lite3d_resource_pack *mFileSysPack = NULL;
static lite3d_resource_pack *m7zPack = NULL;
static lite3d_texture_unit *mNormandy = NULL, *mMinigun = NULL;
static lite3d_camera mCamera01;
static kmVec3 cameraInitPos = {
    5.0f, 5.0f, 5.0f
};

static kmVec3 cameraLookAt = {
    0.0f, 0.0f, 0.0f
};

static kmVec3 nodePos[] = {
    { -2.2f, 0.0f, 0.0f},
    { 0.0f, 0.0f, 0.0f},
    { 2.2f, 0.0f, 0.0f}
};

static kmVec3 rotAxis = {
    0.0f, .0f, 1.0f
};

static kmVec3 rotAxisY = {
    0.0f, 1.0f, 0.0f
};

static kmVec3 mVecMove = {
    0.0f, 0.0f, 0.001f
};

static lite3d_scene_node mSceneNode[3];
static lite3d_scene_node mSceneNodeInherited[3];

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
        else if (levent->key.keysym.sym == SDLK_F2)
        {
            lite3d_scene_node_rotate_angle(&mSceneNode[2], &rotAxis, kmDegreesToRadians(5));
            mSceneNodeInherited[2].recalc = LITE3D_TRUE;
        }
    }

    return LITE3D_TRUE;
}

static int init(void *userdata)
{
    int i = 0;

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

    lite3d_camera_init(&mCamera01, NULL);

    lite3d_add_render_target(1, 10,
        10, 0, NULL)->enabled = LITE3D_FALSE;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    lite3d_camera_perspective(&mCamera01, 0.1f, 100.0f, 45.0f, (float) DEFAULT_WIDTH / (float) DEFAULT_HEIGHT);
    lite3d_scene_node_set_position(&mCamera01.cameraNode, &cameraInitPos);
    //lite3d_camera_lookAt(&mCamera01, &nodePos[2]);
    lite3d_scene_node_rotate_angle(&mCamera01.cameraNode, &rotAxis, kmDegreesToRadians(90));
    lite3d_scene_node_rotate_angle(&mCamera01.cameraNode, &rotAxisY, kmDegreesToRadians(90));
    
    for (; i < sizeof (mSceneNode) / sizeof (lite3d_scene_node); ++i)
    {
        kmVec3 tmp = {
            0.0f, 0.0f, 2.0f
        };

        lite3d_scene_node_init(&mSceneNode[i], NULL);
        lite3d_scene_node_init(&mSceneNodeInherited[i], &mSceneNode[i]);
        lite3d_scene_node_set_position(&mSceneNode[i], &nodePos[i]);
        lite3d_scene_node_set_position(&mSceneNodeInherited[i], &tmp);
    }

    lite3d_camera_link_to(&mCamera01, &mSceneNode[0], LITE3D_CAMERA_LINK_POSITION);

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

static void draw_box(void)
{
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
}

static int frame(void *userdata)
{
    int i = 0;
    lite3d_camera_update_view(&mCamera01);

    for (; i < sizeof (mSceneNode) / sizeof (lite3d_scene_node); ++i)
    {
        mCamera01.cameraNode.recalc = LITE3D_TRUE;
        lite3d_camera_to_node(&mCamera01, &mSceneNode[i]);
        draw_box();
    }

    lite3d_scene_node_move(&mSceneNode[0], &mVecMove);
    lite3d_scene_node_rotate_angle(&mSceneNode[0], &rotAxis, 0.001f);
    if (mSceneNode[0].position.z > 10.0f || mSceneNode[0].position.z < -10.0f)
        mVecMove.z *= -1;

    lite3d_camera_lookAt(&mCamera01, &mSceneNode[2].position);

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
    settings.videoSettings.vsync = LITE3D_FALSE;
    settings.renderLisneters.processEvent = process_events;
    settings.renderLisneters.preRender = init;
    settings.renderLisneters.postRender = shutdown;
    settings.renderLisneters.renderFrame = frame;

    return !lite3d_main(&settings);
}