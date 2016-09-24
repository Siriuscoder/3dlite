/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_mesh_codec.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

#include <mtool/mtool_converter.h>
#include <mtool/mtool_utils.h>

void ConverterCommand::entry_on_mesh(lite3d_mesh *mesh, const kmMat4 *transform, const char *name, void *userdata)
{
    SDL_assert(userdata);
    static_cast<ConverterCommand *>(userdata)->processMesh(mesh, transform, name);
}

lite3d_mesh *ConverterCommand::entry_alloc_mesh(void *userdata)
{
    SDL_assert(userdata);
    lite3d_mesh *mesh = &static_cast<ConverterCommand *>(userdata)->mMesh;
    if (!lite3d_mesh_init(mesh))
        return NULL;

    return mesh;
}

void ConverterCommand::entry_level_push(void *userdata)
{
    SDL_assert(userdata);
    static_cast<ConverterCommand *>(userdata)->mGenerator->pushNodeTree();
}

void ConverterCommand::entry_level_pop(void *userdata)
{
    SDL_assert(userdata);
    ConverterCommand *command = static_cast<ConverterCommand *>(userdata);
    command->mGenerator->popNodeTree();
}

void ConverterCommand::entry_on_material(const char *matName, 
        uint32_t matIndex,
        const kmVec4 *ambient,
        const kmVec4 *diffuse,
        const kmVec4 *specular,
        const kmVec4 *emissive,
        const kmVec4 *reflective,
        const kmVec4 *transparent,
        const char *diffuseTextureFile,
        const char *normalTextureFile,
        const char *reflectionTextureFile,
        void *userdata)
{
    SDL_assert(userdata);
    ConverterCommand *command = static_cast<ConverterCommand *>(userdata);
    command->mGenerator->generateMaterial(matName, matIndex, ambient, diffuse, specular,
        emissive, reflective, transparent, diffuseTextureFile, normalTextureFile,
        reflectionTextureFile);
}

ConverterCommand::ConverterCommand() : 
    mOptimizeMesh(false),
    mFlipUV(false),
    mGenerateJson(false)
{}

#ifdef INCLUDE_ASSIMP
void ConverterCommand::runImpl()
{
    uint32_t loadFlags = 0;
    makeFolders(mGenOptions.outputFolder);

    if (mOptimizeMesh)
        loadFlags |= LITE3D_OPTIMIZE_MESH_FLAG;
    if (mFlipUV)
        loadFlags |= LITE3D_FLIP_UV_FLAG;

    lite3d_assimp_loader_ctx ctx;
    ctx.onAllocMesh = entry_alloc_mesh;
    ctx.onMesh = entry_on_mesh;
    ctx.onLevelPush = entry_level_push;
    ctx.onLevelPop = entry_level_pop;
    ctx.onMaterial = entry_on_material;
    ctx.userdata = this;

    if(mGenerateJson)
        mGenerator.reset(new JsonGenerator(mGenOptions));
    else 
        mGenerator.reset(new NullGenerator());

    if(!lite3d_assimp_mesh_load_recursive(
        mMain.getResourceManager()->loadFileToMemory(mInputFilePath), ctx,
        LITE3D_VBO_STATIC_READ, loadFlags))
        LITE3D_THROW("Unable to import input file.. possible bad format..");
}
#else
void ConverterCommand::runImpl()
{
    LITE3D_THROW("If you want to use converter, please, recompile with Assimp support!");
}
#endif

void ConverterCommand::parseCommandLineImpl(int argc, char *args[])
{
    Command::parseCommandLineImpl(argc, args);

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-i") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
            {
                mInputFilePath.assign("filesystem:");
                mInputFilePath.append(args[i + 1]);
            }
            else
                LITE3D_THROW("Missing input file");
        }
        else if (strcmp(args[i], "-o") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.outputFolder.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing output folder");
        }
        else if (strcmp(args[i], "-oname") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.objectName.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing object name");
        }
        else if (strcmp(args[i], "-O") == 0)
        {
            mOptimizeMesh = true;
        }
        else if (strcmp(args[i], "-F") == 0)
        {
            mFlipUV = true;
        }
        else if (strcmp(args[i], "-j") == 0)
        {
            mGenerateJson = true;
        }
        else if (strcmp(args[i], "-pkg") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.packname.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing package name");
        }
        else if (strcmp(args[i], "-texpkg") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.texPackname.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing package name");
        }
        else if (strcmp(args[i], "-imgpkg") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.imgPackname.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing package name");
        }
        else if (strcmp(args[i], "-meshpkg") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.meshPackname.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing package name");
        }
        else if (strcmp(args[i], "-matpkg") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.matPackname.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing package name");
        }
        else if (strcmp(args[i], "-nodepkg") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mGenOptions.nodePackname.assign(args[i + 1]);
            else
                LITE3D_THROW("Missing package name");
        }
        else if (strcmp(args[i], "-matastex") == 0)
        {
            mGenOptions.useDifTexNameAsMatName = true;
        }
        else if (strcmp(args[i], "-nodeuniq") == 0)
        {
            mGenOptions.nodeUniqName = true;
        }
    }
}

void ConverterCommand::convertMesh(lite3d_mesh *mesh, const lite3dpp::String &savePath)
{
    if(!mesh)
        return;

    size_t encodeBufferSize = lite3d_mesh_m_encode_size(mesh);
    void *encodeBuffer = lite3d_malloc(encodeBufferSize);
    if (!lite3d_mesh_m_encode(mesh, encodeBuffer, encodeBufferSize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: encode failed..",
            LITE3D_CURRENT_FUNCTION);
    }
    else
    {
        Utils::saveFile(encodeBuffer, encodeBufferSize, savePath);
    }

    lite3d_free(encodeBuffer);
    lite3d_mesh_purge(mesh);
}

void ConverterCommand::processMesh(lite3d_mesh *mesh, const kmMat4 *transform, const lite3dpp::String &name)
{
    lite3dpp::String relativeMeshPath = Utils::makeRelativePath("models/meshes/", name, "m");
    lite3dpp::String fullMeshPath = Utils::makeFullPath(mGenOptions.outputFolder, relativeMeshPath);
    lite3dpp::String relativeJsonPath = Utils::makeRelativePath("models/json/", name, "json");
    lite3dpp::String fullJsonPath = Utils::makeFullPath(mGenOptions.outputFolder, relativeJsonPath);
    
    mGenerator->generateNode(mesh, name, transform, mesh != NULL);
    convertMesh(mesh, fullMeshPath);
}