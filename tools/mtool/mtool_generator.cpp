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
#include <mtool_generator.h>
#include <mtool_utils.h>

Generator::Generator(const lite3dpp::String &outputFolder,
    const lite3dpp::String &objectName,
    const lite3dpp::String &packageName) :
    mOutputFolder(outputFolder),
    mObjectName(objectName),
    mPackageName(packageName)
{
    if(!mPackageName.empty())
        mPackageName.append(":");
}

NullGenerator::NullGenerator() :
    Generator("", "", "")
{}

void NullGenerator::generateNode(const lite3d_mesh *mesh, const lite3dpp::String &name, const kmMat4 *transform,
    bool meshExist)
{}

void NullGenerator::pushNodeTree()
{}

void NullGenerator::popNodeTree()
{}

void NullGenerator::generateMaterial(const lite3dpp::String &matName, 
    uint32_t matIdx,
    const kmVec4 *ambient,
    const kmVec4 *diffuse,
    const kmVec4 *specular,
    const kmVec4 *emissive,
    const kmVec4 *reflective,
    const kmVec4 *transparent,
    const char *diffuseTextureFile,
    const char *normalTextureFile,
    const char *reflectionTextureFile)
{}

JsonGenerator::JsonGenerator(const lite3dpp::String &outputFolder,
    const lite3dpp::String &objectName, const lite3dpp::String &packageName) : 
    Generator(outputFolder, objectName, packageName)
{}

void JsonGenerator::generateNode(const lite3d_mesh *mesh, const lite3dpp::String &name, const kmMat4 *transform,
    bool meshExist)
{
    lite3dpp::String relativeMeshPath = Utils::makeRelativePath("models/meshes/", name, "m");
    lite3dpp::String relativeMeshConfigPath = Utils::makeRelativePath("models/json/", name, "json");
    lite3dpp::String fullMeshConfigPath = Utils::makeFullPath(mOutputFolder, relativeMeshConfigPath);

    lite3dpp::ConfigurationWriter nodeConfig;
    nodeConfig.set(L"Name", name + ".node");

    if(meshExist)
    {
        /* configure mesh ison */
        lite3dpp::ConfigurationWriter meshConfig;
        meshConfig.set(L"Codec", "m");
        meshConfig.set(L"Model", mPackageName + relativeMeshPath);

        lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector matMapping;

        {
            const lite3d_mesh_chunk *meshChunk;
            const lite3d_list_node *chunkNode;
            for (chunkNode = mesh->chunks.l.next; chunkNode != &mesh->chunks.l;
                chunkNode = chunkNode->next)
            {
                meshChunk = LITE3D_MEMBERCAST(lite3d_mesh_chunk, chunkNode, node);
                lite3dpp::ConfigurationWriter material;
                material.set(L"MaterialIndex", (int32_t)meshChunk->materialIndex);

                lite3dpp::ConfigurationWriter materialDesc;
                materialDesc.set(L"Name", mMaterials[meshChunk->materialIndex] + ".material");
                materialDesc.set(L"Material", mPackageName + Utils::makeRelativePath("materials/", mMaterials[meshChunk->materialIndex], "json"));
                material.set(L"Material", materialDesc);
                matMapping.push_back(material);
            }
        }
        meshConfig.set(L"MaterialMapping", matMapping);

        lite3dpp::ConfigurationWriter nodeMeshConfig;
        nodeMeshConfig.set(L"Name", name + ".mesh");
        nodeMeshConfig.set(L"Mesh", mPackageName + relativeMeshConfigPath);
        nodeConfig.set(L"Mesh", nodeMeshConfig);

        Utils::saveTextFile(meshConfig.write(), fullMeshConfigPath);
        meshConfig.clear();
    }

    nodeConfig.set(L"Transform", *transform);
    mNodesStack.top().push_back(nodeConfig);
}

void JsonGenerator::pushNodeTree()
{
    mNodesStack.push(lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector());
}

void JsonGenerator::popNodeTree()
{
    if(mNodesStack.size() > 0)
    {
        lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector nodes = mNodesStack.top();
        mNodesStack.pop();

        if(mNodesStack.size() > 0)
        {
            if(nodes.size() > 0)
                mNodesStack.top().back().set(L"Nodes", nodes);
        }
        else
        {
            /* mean all nodes imported.. save hole tree as one object */
            lite3dpp::ConfigurationWriter rootObject;
            rootObject.set(L"Name", mObjectName + ".root");
            rootObject.set(L"Nodes", nodes);

            lite3dpp::ConfigurationWriter objectConfig;
            objectConfig.set(L"Root", rootObject);

            Utils::saveTextFile(objectConfig.write(),
                Utils::makeFullPath(mOutputFolder, Utils::makeRelativePath("objects/", mObjectName, "json")));
            objectConfig.clear();
        }
    }
}

void JsonGenerator::generateMaterial(const lite3dpp::String &matName, 
    uint32_t matIdx,
    const kmVec4 *ambient,
    const kmVec4 *diffuse,
    const kmVec4 *specular,
    const kmVec4 *emissive,
    const kmVec4 *reflective,
    const kmVec4 *transparent,
    const char *diffuseTextureFile,
    const char *normalTextureFile,
    const char *reflectionTextureFile)
{
    lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector uniforms;
    lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector passes;

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "projectionMatrix");
        uniforms.push_back(param);
    }

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "modelviewMatrix");
        uniforms.push_back(param);
    }

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "viewMatrix");
        uniforms.push_back(param);
    }

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "modelMatrix");
        uniforms.push_back(param);
    }

    generateUniformVec4(uniforms, "ambient", ambient);
    generateUniformVec4(uniforms, "diffuse", diffuse);
    generateUniformVec4(uniforms, "specular", specular);
    generateUniformVec4(uniforms, "emissive", emissive);
    generateUniformVec4(uniforms, "reflective", reflective);
    generateUniformVec4(uniforms, "transparent", transparent);

    generateUniformSampler(uniforms, diffuseTextureFile);
    generateUniformSampler(uniforms, normalTextureFile);
    generateUniformSampler(uniforms, reflectionTextureFile);

    {
        lite3dpp::ConfigurationWriter pass1;
        pass1.set(L"Pass", 1);

        lite3dpp::ConfigurationWriter program;
        program.set(L"Name", "Stub.program");
        program.set(L"Path", mPackageName + "shaders/json/stub.json");
        pass1.set(L"Program", program);
        passes.push_back(pass1);
    }

    lite3dpp::String matFull = Utils::makeFullPath(mOutputFolder, Utils::makeRelativePath("materials/", matName, "json"));
    lite3dpp::ConfigurationWriter material;

    material.set(L"Uniforms", uniforms);
    material.set(L"Passes", passes);
    Utils::saveTextFile(material.write(), matFull);
    material.clear();

    mMaterials.insert(std::make_pair(matIdx, matName));
}

void JsonGenerator::generateUniformSampler(lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector &uniforms, const char *fileName)
{
    if(!fileName)
        return;

    lite3dpp::ConfigurationWriter param;
    lite3dpp::String texRel = Utils::makeRelativePath("textures/json/", Utils::getFileNameWithoutExt(fileName), "json");
    lite3dpp::String texFull = Utils::makeFullPath(mOutputFolder, texRel);
    
    param.set(L"Name", "diffuseSampler");
    param.set(L"Type", "sampler");
    param.set(L"TextureName", Utils::getFileNameWithoutExt(fileName) + ".texture");
    param.set(L"TexturePath", mPackageName + texRel);
    uniforms.push_back(param);
    
    {
        lite3dpp::ConfigurationWriter texture;
        texture.set(L"TextureType", "2D");
        texture.set(L"Filtering", "Trilinear");
        texture.set(L"Wrapping", "ClampToEdge");
        texture.set(L"Image", mPackageName + "textures/images/" + fileName);
        texture.set(L"ImageFormat", Utils::getFileExt(fileName));
    
        Utils::saveTextFile(texture.write(), texFull);
        texture.clear();
    }
}

void JsonGenerator::generateUniformVec4(lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector &uniforms, const lite3dpp::String &paramName, const kmVec4 *val)
{
    if(!val)
        return;

    lite3dpp::ConfigurationWriter param;
    param.set(L"Name", paramName);
    param.set(L"Type", "v4");
    param.set(L"Value", *val);
    uniforms.push_back(param);
}