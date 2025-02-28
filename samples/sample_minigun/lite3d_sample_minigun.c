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
 *******************************************************************************/
#include <string.h>

#include <SDL_log.h>
#include <lite3d/lite3d_main.h>
#include <sample_common/lite3d_builtin_shaders.h>

#define DEFAULT_WIDTH           800
#define DEFAULT_HEIGHT          600

static lite3d_pack *mFileSysPack = NULL;
static lite3d_texture_unit mMinigunTexture;
static lite3d_camera mCamera01;
static lite3d_mesh mModel;
static lite3d_shader_parameter mMinigunTextureUnit;
static lite3d_material mMinigunMaterial;
static lite3d_shader_program mProgram;

static lite3d_scene_node mSceneNode;
static lite3d_scene mScene;

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
            lite3d_render_stats *stats = lite3d_render_stats_get();
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

static int initMaterials(void)
{
    lite3d_file *file1;
    lite3d_material_pass *matPass;
    lite3d_shader shaders[2];

    if (!(file1 = lite3d_pack_file_load(mFileSysPack, "textures/images/minigun.dds")))
        return LITE3D_FALSE;

    memset(&mMinigunTexture, 0, sizeof(lite3d_texture_unit));
    if (!lite3d_texture_unit_from_resource(&mMinigunTexture, file1, LITE3D_IMAGE_DDS,
        LITE3D_TEXTURE_2D, LITE3D_FALSE, LITE3D_TEXTURE_FILTER_TRILINEAR, LITE3D_TEXTURE_REPEAT, 0))
        return LITE3D_FALSE;

    /* init parameter with texture */
    lite3d_shader_parameter_init(&mMinigunTextureUnit);
    strcpy(mMinigunTextureUnit.name, "diffuse");
    mMinigunTextureUnit.type = LITE3D_SHADER_PARAMETER_SAMPLER;
    mMinigunTextureUnit.parameter.texture = &mMinigunTexture;

    /* try to compile material shaders */
    lite3d_shader_init(&shaders[0], LITE3D_SHADER_TYPE_VERTEX);
    if (!lite3d_shader_compile(&shaders[0], 1, &vs_builtin, 0))
        return LITE3D_FALSE;

    lite3d_shader_init(&shaders[1], LITE3D_SHADER_TYPE_FRAGMENT);
    if (!lite3d_shader_compile(&shaders[1], 1, &fs_builtin, 0))
        return LITE3D_FALSE;

    lite3d_shader_program_init(&mProgram);
    /* setup attributes indexes like layout in VBO */
    /* layout[0] - vertex */
    /* layout[1] - normals */
    /* layout[2] - tex coords */
    lite3d_shader_program_attribute_index(&mProgram, "vertexAttr", 0);
    lite3d_shader_program_attribute_index(&mProgram, "normalAttr", 1);
    lite3d_shader_program_attribute_index(&mProgram, "texCoordAttr", 2);
    
    if (!lite3d_shader_program_link(&mProgram, shaders, 2))
        return LITE3D_FALSE;

    lite3d_shader_purge(&shaders[0]);
    lite3d_shader_purge(&shaders[1]);

    /* create material for owr box */
    lite3d_material_init(&mMinigunMaterial);
    matPass = lite3d_material_add_pass(&mMinigunMaterial, 1);
    /* set default params */
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->projectionMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->modelMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->viewMatrix);

    /* set sampler */
    lite3d_material_pass_add_parameter(matPass, &mMinigunTextureUnit);
    matPass->program = &mProgram;

    return LITE3D_TRUE;
}

static int initModel(void)
{
    lite3d_file *file1;
    if (!(file1 = lite3d_pack_file_load(mFileSysPack, "models/meshes/minigun.m")))
        return LITE3D_FALSE;

    if (!lite3d_mesh_init(&mModel, LITE3D_VBO_STATIC_DRAW))
        return LITE3D_FALSE;
    if (!lite3d_mesh_load_from_m_file(&mModel, file1))
        return LITE3D_FALSE;

    return LITE3D_TRUE;
}

static int init(void *userdata)
{
    kmVec3 cameraInitPos = {
        80.0f, 80.0f, 80.0f
    };
    kmVec3 viewPos = {
        0.0f, 0.0f, 0.0f
    };

    if (!(mFileSysPack = lite3d_pack_open("samples/", LITE3D_FALSE, 7000000)))
        return LITE3D_FALSE;

    if (!initModel())
        return LITE3D_FALSE;
    if (!initMaterials())
        return LITE3D_FALSE;

    lite3d_camera_init(&mCamera01);

    lite3d_camera_perspective(&mCamera01, 1.0f, 1000.0f, 45.0f, (float) DEFAULT_WIDTH / (float) DEFAULT_HEIGHT);
    lite3d_camera_set_position(&mCamera01, &cameraInitPos);

    lite3d_scene_init(&mScene, 0);
    lite3d_scene_node_init(&mSceneNode);
    lite3d_scene_add_node(&mScene, &mSceneNode, NULL);
    lite3d_scene_node_touch_material(&mSceneNode, lite3d_mesh_chunk_get_by_material_index(&mModel, 0), NULL, &mMinigunMaterial, 1);

    //lite3d_scene_add_node(&mScene, &mCamera01.cameraNode, NULL);
    lite3d_render_target_screen_attach_camera(&mCamera01, &mScene, 1, 0, LITE3D_RENDER_DEFAULT);
    lite3d_camera_lookAt(&mCamera01, &viewPos);

    return LITE3D_TRUE;
}

static int shutdown(void *userdata)
{
    /* release resources */
    lite3d_mesh_purge(&mModel);
    lite3d_material_purge(&mMinigunMaterial);
    lite3d_shader_program_purge(&mProgram);
    lite3d_scene_purge(&mScene);
    lite3d_texture_unit_purge(&mMinigunTexture);
    lite3d_pack_close(mFileSysPack);

    return LITE3D_TRUE;
}

static int pre_frame(void *userdata)
{
    lite3d_scene_node_rotate_angle(&mSceneNode, &KM_VEC3_POS_Z, 0.001f);
    return LITE3D_TRUE;
}

int main(int argc, char *args[])
{
    lite3d_global_settings settings;
    memset(&settings, 0, sizeof (settings));

    settings.logLevel = LITE3D_LOGLEVEL_VERBOSE;
    settings.textureSettings.anisotropy = 8;
    settings.textureSettings.useGLCompression = LITE3D_TRUE;
    settings.videoSettings.MSAA = 4;
    strcpy(settings.videoSettings.caption, "TEST window");
    settings.videoSettings.colorBits = 32;
    settings.videoSettings.fullscreen = LITE3D_FALSE;
    settings.videoSettings.screenWidth = DEFAULT_WIDTH;
    settings.videoSettings.screenHeight = DEFAULT_HEIGHT;
    settings.videoSettings.vsync = LITE3D_TRUE;
    settings.videoSettings.glProfile = LITE3D_GL_PROFILE_CORE;
    settings.videoSettings.glVersionMajor = 3;
    settings.videoSettings.glVersionMinor = 3;
    settings.renderLisneters.processEvent = process_events;
    settings.renderLisneters.preRender = init;
    settings.renderLisneters.preFrame = pre_frame;
    settings.renderLisneters.postRender = shutdown;

    return !lite3d_main(&settings);
}
