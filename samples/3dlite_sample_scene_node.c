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
#include <3dlite/3dlite_scene.h>

#define DEFAULT_WIDTH           800
#define DEFAULT_HEIGHT          600

static lite3d_resource_pack *mFileSysPack = NULL;
static lite3d_resource_pack *m7zPack = NULL;
static lite3d_texture_unit mNormandy, mMinigun;
static lite3d_camera mCamera01;
static kmVec3 cameraInitPos = {
    0.0f, 5.0f, 5.0f
};

static kmVec3 nodePos[] = {
    { -2.2f, 0.0f, 0.0f},
    { 0.0f, 0.0f, 0.0f},
    { 2.2f, 0.0f, 0.0f}
};

static kmVec3 nodeScale[] = {
    { 0.3f, 0.3f, 0.3f},
    { 0.5f, 0.5f, 0.5f},
    { 0.7f, 0.7f, 0.7f}
};

static kmVec3 rotAxisZ = {
    0.0f, 0.0f, 1.0f
};

static lite3d_scene_node mSceneNode[3];
static lite3d_scene_node mSceneNodeInherited[3];

static lite3d_scene mScene;

static void draw_box(struct lite3d_scene_node *node);

static int process_events(SDL_Event *levent)
{
    if (levent->type == SDL_KEYDOWN)
    {
        /* exit */
        if (levent->key.keysym.sym == SDLK_ESCAPE)
            return LITE3D_FALSE;
            /* print render stats */
        else if (levent->key.keysym.sym == SDLK_F1)
        {
            lite3d_render_stats *stats = lite3d_render_get_stats();
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
            lite3d_scene_node_rotate_angle(&mSceneNode[2], &rotAxisZ, kmDegreesToRadians(5));
        }
        else if (levent->key.keysym.sym == SDLK_UP)
        {
            lite3d_camera_pitch(&mCamera01, kmDegreesToRadians(-5));
        }
        else if (levent->key.keysym.sym == SDLK_DOWN)
        {
            lite3d_camera_pitch(&mCamera01, kmDegreesToRadians(5));
        }
        else if (levent->key.keysym.sym == SDLK_LEFT)
        {
            lite3d_camera_yaw(&mCamera01, kmDegreesToRadians(-5));
        }
        else if (levent->key.keysym.sym == SDLK_RIGHT)
        {
            lite3d_camera_yaw(&mCamera01, kmDegreesToRadians(5));
        }
        else if (levent->key.keysym.sym == SDLK_w)
        {
            lite3d_camera_move_relative(&mCamera01, &KM_VEC3_POS_Z);
        }
        else if (levent->key.keysym.sym == SDLK_s)
        {
            lite3d_camera_move_relative(&mCamera01, &KM_VEC3_NEG_Z);
        }
        else if (levent->key.keysym.sym == SDLK_a)
        {
            lite3d_camera_move_relative(&mCamera01, &KM_VEC3_POS_X);
        }
        else if (levent->key.keysym.sym == SDLK_d)
        {
            lite3d_camera_move_relative(&mCamera01, &KM_VEC3_NEG_X);
        }
        else if (levent->key.keysym.sym == SDLK_q)
        {
            lite3d_camera_roll(&mCamera01, kmDegreesToRadians(5));
        }
    }

    return LITE3D_TRUE;
}

static int init(void)
{
    int i = 0;
    lite3d_resource_file *file1, *file2;

    if (!(mFileSysPack = lite3d_resource_pack_open("tests/", LITE3D_FALSE, 700000)))
        return LITE3D_FALSE;
    if (!(m7zPack = lite3d_resource_pack_open("tests/pack.1", LITE3D_TRUE, 700000)))
        return LITE3D_FALSE;
    if (!(file1 = lite3d_resource_pack_file_load(mFileSysPack, "pack/minigun.dds")))
        return LITE3D_FALSE;
    if (!(file2 = lite3d_resource_pack_file_load(m7zPack, "pack/normandy/t1.jpg")))
        return LITE3D_FALSE;

    if (!lite3d_texture_unit_from_resource(&mMinigun, file1, LITE3D_IMAGE_DDS, 
        LITE3D_TEXTURE_2D, LITE3D_TEXTURE_QL_NICEST))
        return LITE3D_FALSE;

    if (!lite3d_texture_unit_from_resource(&mNormandy, file2, LITE3D_IMAGE_JPG, 
        LITE3D_TEXTURE_2D, LITE3D_TEXTURE_QL_NICEST))
        return LITE3D_FALSE;


    lite3d_camera_init(&mCamera01);

    lite3d_render_target_add(1, 10,
        10, 0, NULL)->enabled = LITE3D_FALSE;

    lite3d_camera_perspective(&mCamera01, 0.1f, 100.0f, 45.0f, (float) DEFAULT_WIDTH / (float) DEFAULT_HEIGHT);
    lite3d_camera_set_position(&mCamera01, &cameraInitPos);
    //lite3d_camera_pitch(&mCamera01, kmDegreesToRadians(90));
    lite3d_camera_lookAt(&mCamera01, &nodePos[0]);
    //lite3d_scene_node_rotate_angle(&mCamera01.cameraNode, &rotAxis, kmDegreesToRadians(90));
    //lite3d_scene_node_rotate_angle(&mCamera01.cameraNode, &rotAxisY, kmDegreesToRadians(90));

    lite3d_scene_init(&mScene);
    for (; i < sizeof (mSceneNode) / sizeof (lite3d_scene_node); ++i)
    {
        kmVec3 tmp = {
            0.0f, 0.0f, 2.0f
        };

        lite3d_scene_node_init(&mSceneNode[i]);
        lite3d_scene_node_init(&mSceneNodeInherited[i]);

        lite3d_scene_node_set_position(&mSceneNode[i], &nodePos[i]);

        lite3d_scene_node_set_position(&mSceneNodeInherited[i], &tmp);
        lite3d_scene_node_scale(&mSceneNodeInherited[i], &nodeScale[i]);

        lite3d_scene_node_add(&mScene, &mSceneNode[i], NULL);
        lite3d_scene_node_add(&mScene, &mSceneNodeInherited[i], &mSceneNode[i]);

        mSceneNode[i].doRenderNode = mSceneNodeInherited[i].doRenderNode = draw_box;
    }

    lite3d_scene_node_add(&mScene, &mCamera01.cameraNode, NULL);
    lite3d_render_target_root_attach_camera(&mCamera01);
    //lite3d_camera_link_to(&mCamera01, &mSceneNode[2], LITE3D_CAMERA_LINK_ORIENTATION);
    //lite3d_camera_tracking(&mCamera01, &mSceneNode[2]);

    return LITE3D_TRUE;
}

static int shutdown(void)
{
    lite3d_texture_unit_purge(&mNormandy);
    lite3d_texture_unit_purge(&mMinigun);
    lite3d_resource_pack_close(mFileSysPack);
    lite3d_resource_pack_close(m7zPack);

    return LITE3D_TRUE;
}

static void draw_box(struct lite3d_scene_node *node)
{
    lite3d_texture_unit_bind(&mMinigun);

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

    lite3d_texture_unit_unbind(&mMinigun);
    lite3d_texture_unit_bind(&mNormandy);

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

    lite3d_texture_unit_unbind(&mNormandy);
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

    return !lite3d_main(&settings);
}