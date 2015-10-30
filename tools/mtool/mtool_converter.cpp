/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#define MAKE_PATH(name) (mOutputFolder + "/" + name).c_str()

#ifdef PLATFORM_Windows
#   include <Windows.h>
#   define MKDIR(name) \
    if(CreateDirectory(MAKE_PATH(name), NULL)) \
        printf("Make directory %s\n", MAKE_PATH(name));
#elif PLATFORM_Linux
#   include <sys/stat.h>
#   define MKDIR(name) \
    if(mkdir(MAKE_PATH(name), 755) == 0) \
        printf("Make directory %s\n", MAKE_PATH(name));
#endif

#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <lite3d/lite3d_mesh_codec.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

#include <mtool_converter.h>

void ConverterCommand::stub_mesh_loaded(lite3d_mesh *mesh, const kmMat4 *transform, const char *name, void *userdata)
{
    SDL_assert(userdata);
    static_cast<ConverterCommand *>(userdata)->processMesh(mesh, transform, name);
}

lite3d_mesh *ConverterCommand::stub_mesh_init(void *userdata)
{
    SDL_assert(userdata);

    lite3d_mesh *mesh = &static_cast<ConverterCommand *>(userdata)->mMesh;
    if (!lite3d_mesh_init(mesh))
        return NULL;

    return mesh;
}

void ConverterCommand::stub_level_push(void *userdata)
{
    static_cast<ConverterCommand *>(userdata)->nodeLevelPush();
}

void ConverterCommand::stub_level_pop(void *userdata)
{
    static_cast<ConverterCommand *>(userdata)->nodeLevelPop();
}

ConverterCommand::ConverterCommand() : 
    mOptimizeMesh(false),
    mFlipUV(false),
    mNonameCounter(0)
{}

#ifdef INCLUDE_ASSIMP
void ConverterCommand::runImpl()
{
    uint32_t loadFlags = 0;
    makeFolders();

    if (mOptimizeMesh)
        loadFlags |= LITE3D_OPTIMIZE_MESH_FLAG;
    if (mFlipUV)
        loadFlags |= LITE3D_FLIP_UV_FLAG;

    lite3d_assimp_loader_ctx ctx;
    ctx.onNewMesh = stub_mesh_init;
    ctx.onLoaded = stub_mesh_loaded;
    ctx.onLevelPush = mGenerateJson ? stub_level_push : NULL;
    ctx.onLevelPop = mGenerateJson ? stub_level_pop : NULL;
    ctx.userdata = this;

    if(!lite3d_assimp_mesh_load_recursive(
        mMain.getResourceManager()->loadFileToMemory(mInputFilePath), ctx,
        LITE3D_VBO_STATIC_READ, loadFlags))
        throw std::runtime_error("Unable to import inmput file.. possible bad format..");
}
#else
void ConverterCommand::runImpl()
{
    throw std::runtime_error("If you want to use converter, please, recompile with Assimp support!");
}
#endif

void ConverterCommand::makeFolders()
{
    MKDIR("materials");
    MKDIR("models");
    MKDIR("models/json");
    MKDIR("models/meshes");
    MKDIR("objects");
    MKDIR("scenes");
    MKDIR("scripts");
    MKDIR("shaders");
    MKDIR("shaders/json");
    MKDIR("shaders/sources");
    MKDIR("targets");
    MKDIR("textures");
    MKDIR("textures/json");
    MKDIR("textures/images");
}

void ConverterCommand::saveFile(const void *buffer, size_t size, const lite3dpp::String &path)
{
    SDL_RWops *descr;
    printf("Writing %s ... ", path.c_str());
    fflush(stdout);

    descr = SDL_RWFromFile(path.c_str(), "wb");
    if (!descr)
        throw std::runtime_error(lite3dpp::String("Unable to open file " + path));

    if (SDL_RWwrite(descr, buffer, size, 1) != 1)
    {
        SDL_RWclose(descr);
        throw std::runtime_error(lite3dpp::String("IO error.. ") + path);
    }

    SDL_RWclose(descr);

    printf("done\n");
    fflush(stdout);
}

lite3dpp::String ConverterCommand::makeFullPath(const lite3dpp::String &relative)
{
    char path[1024] = {0};
    sprintf(path, "%s%s%s", mOutputFolder.c_str(), mOutputFolder.size() == 0 ? "" : "/", relative.c_str());

    return path;
}

lite3dpp::String ConverterCommand::makeRelativePath(const lite3dpp::String &inpath, const lite3dpp::String &name,
    const lite3dpp::String &ext)
{
    char path[1024] = {0};

    if (name.size() == 0)
        sprintf(path, "%snoname%d.%s", inpath.c_str(), ++mNonameCounter, ext.c_str());
    else
        sprintf(path, "%s%s.%s", inpath.c_str(), name.c_str(), ext.c_str());

    return path;
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
            __FUNCTION__);
    }
    else
    {
        saveFile(encodeBuffer, encodeBufferSize, savePath);
    }

    lite3d_free(encodeBuffer);
    lite3d_mesh_purge(mesh);
}

void ConverterCommand::processMesh(lite3d_mesh *mesh, const kmMat4 *transform, const lite3dpp::String &name)
{
    lite3dpp::String relativeMeshPath = makeRelativePath("models/meshes/", name, "m");
    lite3dpp::String fullMeshPath = makeFullPath(relativeMeshPath);
    lite3dpp::String relativeJsonPath = makeRelativePath("models/json/", name, "json");
    lite3dpp::String fullJsonPath = makeFullPath(relativeJsonPath);
    
    convertMesh(mesh, fullMeshPath);

    if(mGenerateJson)
    {
        /* configure node */
        lite3dpp::ConfigurationWriter nodeWriter;
        nodeWriter.set(L"Name", name + ".node");

        if(mesh)
        {
            /* configure mesh ison */
            lite3dpp::ConfigurationWriter writer;
            writer.set(L"Codec", "m");
            writer.set(L"Model", relativeMeshPath);

            lite3dpp::ConfigurationWriter nodeMeshWriter;
            nodeMeshWriter.set(L"Name", name + ".mesh");
            nodeMeshWriter.set(L"Mesh", relativeJsonPath);
            nodeWriter.set(L"Mesh", nodeMeshWriter);

            lite3dpp::String jsonData = writer.write();
            saveFile(jsonData.data(), jsonData.size(), fullJsonPath);
            writer.clear();
        }

        nodeWriter.set(L"Transform", *transform);
        mNodesLevels.top().push_back(nodeWriter);
    }
}

void ConverterCommand::nodeLevelPush()
{
    mNodesLevels.push(lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector());
}

void ConverterCommand::nodeLevelPop()
{
    lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector nodes = mNodesLevels.top();
    mNodesLevels.pop();

    if(mNodesLevels.size() > 0)
    {
        if(nodes.size() > 0)
            mNodesLevels.top().back().set(L"Nodes", nodes);
        return;
    }

    /* mean all nodes imported.. save hole tree as one object */
    lite3dpp::ConfigurationWriter rootObject;
    rootObject.set(L"Name", mObjectName.size() > 0 ? mObjectName : "noname.root");
    rootObject.set(L"Nodes", nodes);

    lite3dpp::ConfigurationWriter finalWriter;
    finalWriter.set(L"Root", rootObject);

    lite3dpp::String jsonData = finalWriter.write();
    saveFile(jsonData.data(), jsonData.size(), 
        makeFullPath(makeRelativePath("objects/", mObjectName, "json")));
    finalWriter.clear();
}
