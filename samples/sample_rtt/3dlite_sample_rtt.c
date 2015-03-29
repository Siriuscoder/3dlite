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
#define RENDER_TEXTURE_WIDTH    512
#define RENDER_TEXTURE_HEIGHT   512


static lite3d_resource_pack *mFileSysPack = NULL;
static lite3d_texture_unit mRenderTextureUnit, mBoxUnit;
static lite3d_shader_parameter mRenderTextureSampler;
static lite3d_shader_parameter mBoxTextureSampler;
static lite3d_material mRenderTextureMaterial;
static lite3d_material mBoxMaterial;
static lite3d_shader_program mProgram;
static lite3d_camera mCamera01;
static lite3d_camera mCamera02;
static lite3d_vbo mCubeVbo;
static lite3d_render_target mRTT;

static kmVec3 cameraInitPos = {
    3.0f, 3.0f, 3.0f
};

static kmVec3 cameraInitPos2 = {
    0.0f, 3.0f, 3.0f
};

static kmVec4 renderTextureClearColor = {
    0.0f, 0.0f, 0.0f, 1.0f
};

static kmVec3 nodePos[] = {
    { -2.2f, 0.0f, 0.0f},
    { 0.0f, 0.0f, 0.0f},
    { 2.2f, 0.0f, 0.0f}
};

static lite3d_mesh_node mSceneNode[2];

static lite3d_scene mScene;
static lite3d_scene mSceneMain;

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
        else if (levent->key.keysym.sym == SDLK_UP)
        {
            lite3d_camera_pitch(&mCamera02, kmDegreesToRadians(-1));
        }
        else if (levent->key.keysym.sym == SDLK_DOWN)
        {
            lite3d_camera_pitch(&mCamera02, kmDegreesToRadians(1));
        }
        else if (levent->key.keysym.sym == SDLK_LEFT)
        {
            lite3d_camera_yaw(&mCamera02, kmDegreesToRadians(-1));
        }
        else if (levent->key.keysym.sym == SDLK_RIGHT)
        {
            lite3d_camera_yaw(&mCamera02, kmDegreesToRadians(1));
        }
        else if (levent->key.keysym.sym == SDLK_w)
        {
            lite3d_camera_move_relative(&mCamera02, &KM_VEC3_POS_Z);
        }
        else if (levent->key.keysym.sym == SDLK_s)
        {
            lite3d_camera_move_relative(&mCamera02, &KM_VEC3_NEG_Z);
        }
        else if (levent->key.keysym.sym == SDLK_a)
        {
            lite3d_camera_move_relative(&mCamera02, &KM_VEC3_POS_X);
        }
        else if (levent->key.keysym.sym == SDLK_d)
        {
            lite3d_camera_move_relative(&mCamera02, &KM_VEC3_NEG_X);
        }
        else if (levent->key.keysym.sym == SDLK_q)
        {
            lite3d_camera_roll(&mCamera02, kmDegreesToRadians(5));
        }
    }

    return LITE3D_TRUE;
}

static int initMaterials(void)
{
    lite3d_material_pass *matPass;
    lite3d_shader shaders[2];

    /* init parameter with texture */
    lite3d_shader_parameter_init(&mRenderTextureSampler);
    strcpy(mRenderTextureSampler.name, "diffuse");
    mRenderTextureSampler.persist = LITE3D_FALSE;
    mRenderTextureSampler.type = LITE3D_SHADER_PARAMETER_SAMPLER;
    mRenderTextureSampler.parameter.valsampler.texture = &mRenderTextureUnit;
    /* init parameter with texture */
    lite3d_shader_parameter_init(&mBoxTextureSampler);
    strcpy(mBoxTextureSampler.name, "diffuse");
    mBoxTextureSampler.persist = LITE3D_FALSE;
    mBoxTextureSampler.type = LITE3D_SHADER_PARAMETER_SAMPLER;
    mBoxTextureSampler.parameter.valsampler.texture = &mBoxUnit;

    /* try to compile material shaders */
    lite3d_shader_init(&shaders[0], LITE3D_SHADER_TYPE_VERTEX);
    if (!lite3d_shader_compile(&shaders[0],
        "in vec3 vertexAttr; "
        "in vec2 texCoordAttr; "
        "uniform mat4 projectionMatrix; "
        "uniform mat4 modelviewMatrix; "
        "varying vec2 vTexCoord; "
        "void main() "
        "{"
        "   vTexCoord = texCoordAttr; "
        "   vec4 vertex = vec4(vertexAttr, 1.0); "
        "   gl_Position = projectionMatrix * modelviewMatrix * vertex; "
        "}"))
        return LITE3D_FALSE;
    lite3d_shader_init(&shaders[1], LITE3D_SHADER_TYPE_FRAGMENT);
    if (!lite3d_shader_compile(&shaders[1],
        "uniform sampler2D diffuse; "
        "varying vec2 vTexCoord; "
        "void main() "
        "{"
        "   gl_FragColor = texture2D(diffuse, vTexCoord.st); "
        "}"))
        return LITE3D_FALSE;

    lite3d_shader_program_init(&mProgram);
    /* setup attributes indexes like layout in VBO */
    /* layout[0] - vertex */
    /* layout[1] - tex coords */
    lite3d_shader_program_attribute_index(&mProgram, "vertexAttr", 0);
    lite3d_shader_program_attribute_index(&mProgram, "texCoordAttr", 1);
    if (!lite3d_shader_program_link(&mProgram, shaders, 2))
        return LITE3D_FALSE;

    lite3d_shader_purge(&shaders[0]);
    lite3d_shader_purge(&shaders[1]);

    /* create material for owr box */
    lite3d_material_init(&mRenderTextureMaterial);
    matPass = lite3d_material_add_pass(&mRenderTextureMaterial);
    /* set default params */
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->projectionMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->modelviewMatrix);
    /* set sampler */
    lite3d_material_pass_add_parameter(matPass, &mRenderTextureSampler);
    matPass->program = &mProgram;

    /* create material for owr box */
    lite3d_material_init(&mBoxMaterial);
    matPass = lite3d_material_add_pass(&mBoxMaterial);
    /* set default params */
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->projectionMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->modelviewMatrix);
    /* set sampler */
    lite3d_material_pass_add_parameter(matPass, &mBoxTextureSampler);
    matPass->program = &mProgram;

    return LITE3D_TRUE;
}

static int initCube(void)
{
    const float cubeVertices[] = {
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,

        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f
    };

    const uint8_t cubeIndices[] = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };

    const lite3d_vbo_layout layout[] = {
        { LITE3D_BUFFER_BINDING_ATTRIBUTE, 3},
        { LITE3D_BUFFER_BINDING_ATTRIBUTE, 2}
    };

    if (!lite3d_vbo_init(&mCubeVbo))
        return LITE3D_FALSE;
    if (!lite3d_vbo_load_from_memory(&mCubeVbo, cubeVertices, 24, layout, 2, cubeIndices, 12, 3, GL_STATIC_DRAW))
        return LITE3D_FALSE;

    return LITE3D_TRUE;
}

static int init(void *userdata)
{
    lite3d_resource_file *file1;

    if (!(mFileSysPack = lite3d_resource_pack_open("tests/", LITE3D_FALSE, 700000)))
        return LITE3D_FALSE;
    if (!(file1 = lite3d_resource_pack_file_load(mFileSysPack, "pack/box1.jpg")))
        return LITE3D_FALSE;

    if (!lite3d_texture_unit_from_resource(&mBoxUnit, file1, LITE3D_IMAGE_JPG,
        LITE3D_TEXTURE_2D, LITE3D_TEXTURE_QL_NICEST, LITE3D_TEXTURE_REPEAT))
        return LITE3D_FALSE;

    /* allocate texture for render */
    /* framebuffers not support texture compression, we mast turn off it */
    lite3d_texture_unit_compression(LITE3D_FALSE);
    if (!lite3d_texture_unit_allocate(&mRenderTextureUnit, LITE3D_TEXTURE_2D,
        LITE3D_TEXTURE_QL_MEDIUM, LITE3D_TEXTURE_REPEAT, LITE3D_TEXTURE_FORMAT_RGBA,
        RENDER_TEXTURE_WIDTH, RENDER_TEXTURE_HEIGHT, 1))
        return LITE3D_FALSE;
    /* then texture allocated we can enable it */
    lite3d_texture_unit_compression(LITE3D_TRUE);

    if (!initCube())
        return LITE3D_FALSE;
    if (!initMaterials())
        return LITE3D_FALSE;


    /* init cameras */
    lite3d_camera_init(&mCamera01);
    lite3d_camera_perspective(&mCamera01, 0.1f, 100.0f, 45.0f, (float) DEFAULT_WIDTH / (float) DEFAULT_HEIGHT);
    lite3d_camera_set_position(&mCamera01, &cameraInitPos);
    lite3d_camera_lookAt(&mCamera01, &nodePos[1]);

    lite3d_camera_init(&mCamera02);
    lite3d_camera_perspective(&mCamera02, 0.1f, 100.0f, 45.0f, (float) DEFAULT_WIDTH / (float) DEFAULT_HEIGHT);
    lite3d_camera_set_position(&mCamera02, &cameraInitPos2);
    lite3d_camera_lookAt(&mCamera02, &nodePos[1]);

    lite3d_scene_mesh_init(&mScene);
    lite3d_scene_mesh_init(&mSceneMain);


    lite3d_mesh_node_init(&mSceneNode[0], &mCubeVbo);
    lite3d_mesh_node_init(&mSceneNode[1], &mCubeVbo);

    lite3d_scene_node_set_position(&mSceneNode[0].sceneNode, &nodePos[1]);
    lite3d_scene_node_set_position(&mSceneNode[1].sceneNode, &nodePos[1]);


    lite3d_scene_mesh_node_add(&mScene, &mSceneNode[0], NULL);
    lite3d_mesh_node_attach_material(&mSceneNode[0], &mBoxMaterial, 0);

    lite3d_scene_mesh_node_add(&mSceneMain, &mSceneNode[1], NULL);
    lite3d_mesh_node_attach_material(&mSceneNode[1], &mRenderTextureMaterial, 0);


    lite3d_scene_node_add(&mScene, &mCamera01.cameraNode, NULL);
    lite3d_scene_node_add(&mSceneMain, &mCamera02.cameraNode, NULL);

    /* setup render targets */
    lite3d_render_target_screen_attach_camera(&mCamera02);

    /* create new render target and attach cam1 to it */
    lite3d_render_target_init(&mRTT, RENDER_TEXTURE_WIDTH, RENDER_TEXTURE_HEIGHT);
    lite3d_render_target_attach_camera(&mRTT, &mCamera01);
    /* setup render target framebuffer */
    if (!lite3d_framebuffer_setup(&mRTT.fb, &mRenderTextureUnit, 1,
        LITE3D_FALSE, NULL, LITE3D_TRUE, LITE3D_FALSE))
        return LITE3D_FALSE;

    /* setup clear color to render texture */
    mRTT.cleanColor = renderTextureClearColor;
    /* attach render target to render system, now it will be render ! */
    lite3d_render_target_add(&mRTT);

    return LITE3D_TRUE;
}

static int shutdown(void *userdata)
{
    lite3d_vbo_purge(&mCubeVbo);
    lite3d_material_purge(&mRenderTextureMaterial);
    lite3d_material_purge(&mBoxMaterial);

    lite3d_shader_program_purge(&mProgram);

    lite3d_scene_mesh_purge(&mScene);
    lite3d_scene_mesh_purge(&mSceneMain);
    lite3d_texture_unit_purge(&mRenderTextureUnit);
    lite3d_texture_unit_purge(&mBoxUnit);

    lite3d_render_target_purge(&mRTT);
    lite3d_resource_pack_close(mFileSysPack);

    return LITE3D_TRUE;
}

static int pre_frame(void *userdata)
{
    lite3d_scene_node_rotate_angle(&mSceneNode[0].sceneNode, &KM_VEC3_POS_Z, 0.005f);
    lite3d_scene_node_rotate_angle(&mSceneNode[1].sceneNode, &KM_VEC3_POS_Z, 0.001f);
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
    settings.renderLisneters.preFrame = pre_frame;

    return !lite3d_main(&settings);
}