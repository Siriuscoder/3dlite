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

static lite3d_resource_pack *mFileSysPack = NULL;
static lite3d_texture_unit mRifleTextureA;
static lite3d_texture_unit mRifleTextureB;
static lite3d_texture_unit mBattTexture;
static lite3d_camera mCamera01;
static lite3d_vbo mRifle;
static lite3d_vbo mRifleBatt;
static lite3d_shader_parameter mRifleTextureUnitA;
static lite3d_shader_parameter mRifleTextureUnitB;
static lite3d_shader_parameter mBattTextureUnit;
static lite3d_material mRifleMaterialA;
static lite3d_material mRifleMaterialB;
static lite3d_material mBattMaterial;
static lite3d_shader_program mProgram;

static lite3d_mesh_node mRifleNode;
static lite3d_mesh_node mBattNode;
static lite3d_scene mScene;

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
    lite3d_resource_file *file1;
    lite3d_material_pass *matPass;
    lite3d_shader shaders[2];

    if (!(file1 = lite3d_resource_pack_file_load(mFileSysPack, "pack/plasmagun/plasmarif02a.dds")))
        return LITE3D_FALSE;
    if (!lite3d_texture_unit_from_resource(&mRifleTextureA, file1, LITE3D_IMAGE_DDS,
        LITE3D_TEXTURE_2D, LITE3D_TEXTURE_QL_NICEST))
        return LITE3D_FALSE;
    if (!(file1 = lite3d_resource_pack_file_load(mFileSysPack, "pack/plasmagun/plasmarif02b.dds")))
        return LITE3D_FALSE;
    if (!lite3d_texture_unit_from_resource(&mRifleTextureB, file1, LITE3D_IMAGE_DDS,
        LITE3D_TEXTURE_2D, LITE3D_TEXTURE_QL_NICEST))
        return LITE3D_FALSE;
    if (!(file1 = lite3d_resource_pack_file_load(mFileSysPack, "pack/plasmagun/plasmarif02c.dds")))
        return LITE3D_FALSE;
    if (!lite3d_texture_unit_from_resource(&mBattTexture, file1, LITE3D_IMAGE_DDS,
        LITE3D_TEXTURE_2D, LITE3D_TEXTURE_QL_NICEST))
        return LITE3D_FALSE;

    /* init parameter with texture */
    lite3d_shader_parameter_init(&mRifleTextureUnitA);
    strcpy(mRifleTextureUnitA.name, "diffuse");
    mRifleTextureUnitA.persist = LITE3D_FALSE;
    mRifleTextureUnitA.type = LITE3D_SHADER_PARAMETER_SAMPLER;
    mRifleTextureUnitA.parameter.valsampler.texture = &mRifleTextureA;
    /* init parameter with texture */
    lite3d_shader_parameter_init(&mRifleTextureUnitB);
    strcpy(mRifleTextureUnitB.name, "diffuse");
    mRifleTextureUnitB.persist = LITE3D_FALSE;
    mRifleTextureUnitB.type = LITE3D_SHADER_PARAMETER_SAMPLER;
    mRifleTextureUnitB.parameter.valsampler.texture = &mRifleTextureB;
    /* init parameter with texture */
    lite3d_shader_parameter_init(&mBattTextureUnit);
    strcpy(mBattTextureUnit.name, "diffuse");
    mBattTextureUnit.persist = LITE3D_FALSE;
    mBattTextureUnit.type = LITE3D_SHADER_PARAMETER_SAMPLER;
    mBattTextureUnit.parameter.valsampler.texture = &mBattTexture;

    /* try to compile material shaders */
    lite3d_shader_init(&shaders[0], LITE3D_SHADER_TYPE_VERTEX);
    if (!lite3d_shader_compile(&shaders[0],
        "in vec4 vertexAttr; "
        "in vec3 normalAttr; "
        "in vec2 texCoordAttr; "
        "uniform mat4 projectionMatrix; "
        "uniform mat4 modelviewMatrix; "
        "varying vec2 vTexCoord; "
        "void main() "
        "{"
        "   vTexCoord = texCoordAttr; "
        "   gl_Position = projectionMatrix * modelviewMatrix * vertexAttr; "
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
    /* layout[1] - normals */
    /* layout[2] - tex coords */
    lite3d_shader_program_attribute_index(&mProgram, "vertexAttr", 0);
    lite3d_shader_program_attribute_index(&mProgram, "normalAttr", 1);
    lite3d_shader_program_attribute_index(&mProgram, "texCoordAttr", 2);
    
    if (!lite3d_shader_program_link(&mProgram, shaders, 2))
        return LITE3D_FALSE;

    lite3d_shader_purge(&shaders[0]);
    lite3d_shader_purge(&shaders[1]);

    /* create material A */
    lite3d_material_init(&mRifleMaterialA);
    matPass = lite3d_material_add_pass(&mRifleMaterialA);
    /* set default params */
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->projectionMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->modelviewMatrix);
    /* set sampler */
    lite3d_material_pass_add_parameter(matPass, &mRifleTextureUnitA);
    matPass->program = &mProgram;

    /* create material B */
    lite3d_material_init(&mRifleMaterialB);
    matPass = lite3d_material_add_pass(&mRifleMaterialB);
    /* set default params */
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->projectionMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->modelviewMatrix);
    /* set sampler */
    lite3d_material_pass_add_parameter(matPass, &mRifleTextureUnitB);
    matPass->program = &mProgram;

    /* create material Battery */
    lite3d_material_init(&mBattMaterial);
    matPass = lite3d_material_add_pass(&mBattMaterial);
    /* set default params */
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->projectionMatrix);
    lite3d_material_pass_add_parameter(matPass, &lite3d_shader_global_parameters()->modelviewMatrix);
    /* set sampler */
    lite3d_material_pass_add_parameter(matPass, &mBattTextureUnit);
    matPass->program = &mProgram;

    return LITE3D_TRUE;
}

static int initModel(void)
{
    lite3d_resource_file *file1;
    if (!(file1 = lite3d_resource_pack_file_load(mFileSysPack, "pack/plasmagun/plasmarif.3ds")))
        return LITE3D_FALSE;

    if (!lite3d_vbo_init(&mRifle))
        return LITE3D_FALSE;
    if (!lite3d_vbo_load(&mRifle, file1, "PlasmaRifl", GL_STATIC_DRAW, LITE3D_FLIP_UV_FLAG | LITE3D_OPTIMIZE_MESH_FLAG))
        return LITE3D_FALSE;
    /* fix material indexes to 0..maxVao */
    /* it is right way if you know how submeshes attached to real materials */
    lite3d_vbo_order_mat_indexes(&mRifle);

    if (!lite3d_vbo_init(&mRifleBatt))
        return LITE3D_FALSE;
    if (!lite3d_vbo_load(&mRifleBatt, file1, "Battery", GL_STATIC_DRAW, LITE3D_FLIP_UV_FLAG))
        return LITE3D_FALSE;

    return LITE3D_TRUE;
}

static int init(void)
{
    kmVec3 cameraInitPos = {
        40.0f, 40.0f, 40.0f
    };
    kmVec3 viewPos = {
        0.0f, 0.0f, 0.0f
    };

    if (!(mFileSysPack = lite3d_resource_pack_open("tests/", LITE3D_FALSE, 7000000)))
        return LITE3D_FALSE;

    if (!initModel())
        return LITE3D_FALSE;
    if (!initMaterials())
        return LITE3D_FALSE;

    lite3d_camera_init(&mCamera01);

    lite3d_camera_perspective(&mCamera01, 1.0f, 1000.0f, 45.0f, (float) DEFAULT_WIDTH / (float) DEFAULT_HEIGHT);
    lite3d_camera_set_position(&mCamera01, &cameraInitPos);

    lite3d_scene_mesh_init(&mScene);
    lite3d_mesh_node_init(&mRifleNode, &mRifle);
    lite3d_mesh_node_init(&mBattNode, &mRifleBatt);

    lite3d_scene_mesh_node_add(&mScene, &mRifleNode, NULL);
    lite3d_scene_mesh_node_add(&mScene, &mBattNode, &mRifleNode.sceneNode);

    lite3d_mesh_node_attach_material(&mRifleNode, &mRifleMaterialA, 0);
    lite3d_mesh_node_attach_material(&mRifleNode, &mRifleMaterialB, 1);
    lite3d_mesh_node_attach_material(&mRifleNode, &mRifleMaterialB, 2);
    lite3d_mesh_node_attach_material(&mRifleNode, &mRifleMaterialB, 3);
    lite3d_mesh_node_attach_material(&mRifleNode, &mRifleMaterialB, 4);
    lite3d_mesh_node_attach_material(&mBattNode, &mBattMaterial, 0);

    lite3d_scene_node_add(&mScene, &mCamera01.cameraNode, NULL);
    lite3d_render_target_screen_attach_camera(&mCamera01);
    lite3d_camera_lookAt(&mCamera01, &viewPos);

    return LITE3D_TRUE;
}

static int shutdown(void)
{
    /* release resources */
    lite3d_vbo_purge(&mRifle);
    lite3d_vbo_purge(&mRifleBatt);

    lite3d_material_purge(&mRifleMaterialA);
    lite3d_material_purge(&mRifleMaterialB);
    lite3d_material_purge(&mBattMaterial);

    lite3d_shader_program_purge(&mProgram);
    lite3d_scene_mesh_purge(&mScene);

    lite3d_texture_unit_purge(&mRifleTextureA);
    lite3d_texture_unit_purge(&mRifleTextureB);
    lite3d_texture_unit_purge(&mBattTexture);

    lite3d_resource_pack_close(mFileSysPack);

    return LITE3D_TRUE;
}

static int pre_frame(void)
{
    lite3d_scene_node_rotate_angle(&mRifleNode.sceneNode, &KM_VEC3_POS_Z, 0.001f);
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
    settings.renderLisneters.preFrame = pre_frame;
    settings.renderLisneters.postRender = shutdown;

    return !lite3d_main(&settings);
}