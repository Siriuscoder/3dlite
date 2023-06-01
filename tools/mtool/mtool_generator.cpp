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
#include <mtool/mtool_generator.h>
#include <mtool/mtool_utils.h>

const GeneratorOptions GeneratorOptions::NullOptions;

GeneratorOptions::GeneratorOptions() : 
    objectName("noname"),
    useDifTexNameAsMatName(false),
    nodeUniqName(false)
{}

Generator::Generator(const GeneratorOptions &options) :
    mOptions(options)
{
    if(mOptions.texPackname.empty())
        mOptions.texPackname = mOptions.packname;
    if(mOptions.imgPackname.empty())
        mOptions.imgPackname = mOptions.packname;
    if(mOptions.matPackname.empty())
        mOptions.matPackname = mOptions.packname;
    if(mOptions.nodePackname.empty())
        mOptions.nodePackname = mOptions.packname;
    if(mOptions.meshPackname.empty())
        mOptions.meshPackname = mOptions.packname;
    
    mOptions.texPackname.append(":");
    mOptions.imgPackname.append(":");
    mOptions.matPackname.append(":");
    mOptions.nodePackname.append(":");
    mOptions.meshPackname.append(":");
}

NullGenerator::NullGenerator() :
    Generator(GeneratorOptions::NullOptions)
{}

void NullGenerator::generateNode(const lite3d_mesh *mesh, const lite3dpp::String &name, const kmMat4 *transform,
    bool meshExist)
{}

void NullGenerator::pushNodeTree()
{}

void NullGenerator::popNodeTree()
{}

void NullGenerator::generateMaterial(const lite3dpp::String &name, 
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

void NullGenerator::generateLight(const lite3dpp::String &lightName,
    const lite3d_light_params *params)
{}

JsonGenerator::JsonGenerator(const GeneratorOptions &options) :
    Generator(options),
    mNodeCounter(0)
{}

void JsonGenerator::generateNode(const lite3d_mesh *mesh, const lite3dpp::String &name, const kmMat4 *transform,
    bool meshExist)
{
    lite3dpp::String relativeMeshPath = Utils::makeRelativePath("models/meshes/", Utils::extractMeshName(name), "m");
    lite3dpp::String relativeMeshConfigPath = Utils::makeRelativePath("models/json/", Utils::extractMeshName(name), "json");
    lite3dpp::String fullMeshConfigPath = Utils::makeFullPath(mOptions.outputFolder, relativeMeshConfigPath);

    lite3dpp::ConfigurationWriter nodeConfig;
    nodeConfig.set(L"Name", name + (meshExist && mOptions.nodeUniqName ? std::to_string(++mNodeCounter) : "") + ".node");

    if(meshExist)
    {
        /* configure mesh ison */
        lite3dpp::ConfigurationWriter meshConfig;
        meshConfig.set(L"Codec", "m");
        meshConfig.set(L"Model", mOptions.meshPackname + relativeMeshPath);

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
                materialDesc.set(L"Material", mOptions.matPackname + Utils::makeRelativePath("materials/", mMaterials[meshChunk->materialIndex], "json"));
                material.set(L"Material", materialDesc);
                matMapping.push_back(material);
            }
        }
        meshConfig.set(L"MaterialMapping", matMapping);

        lite3dpp::ConfigurationWriter nodeMeshConfig;
        nodeMeshConfig.set(L"Name", Utils::extractMeshName(name) + ".mesh");
        nodeMeshConfig.set(L"Mesh", mOptions.meshPackname + relativeMeshConfigPath);
        nodeConfig.set(L"Mesh", nodeMeshConfig);

        Utils::saveTextFile(meshConfig.write(), fullMeshConfigPath);
    }

    generatePositionRotation(nodeConfig, transform);
    mNodesStack.top().push_back(nodeConfig);
}

void JsonGenerator::generateLight(const lite3dpp::String &lightName,
    const lite3d_light_params *params)
{
    if (mNodesStack.size() > 0)
    {
        if (mNodesStack.top().size() > 0)
        {
            auto &lastNode = mNodesStack.top().back();
            lite3dpp::ConfigurationWriter liConfig;
            
            if (params->block1.x == LITE3D_LIGHT_POINT)
                liConfig.set(L"Type", "Point");
            else if (params->block1.x == LITE3D_LIGHT_DIRECTIONAL)
                liConfig.set(L"Type", "Directional");
            else if (params->block1.x == LITE3D_LIGHT_SPOT)
                liConfig.set(L"Type", "Spot");
            else
                liConfig.set(L"Type", "Undefined");

            liConfig.set(L"Name", lightName);
            liConfig.set(L"Ambient", *((kmVec3 *)&params->block2.z));
            liConfig.set(L"Diffuse", *((kmVec3 *)&params->block3.y));
            liConfig.set(L"Specular", *((kmVec3 *)&params->block4.x));
            kmVec4 att = { params->block6.x, params->block6.y, params->block6.z,
                params->block1.z };
            liConfig.set(L"Attenuation", att);
            liConfig.set(L"Position", *((kmVec3 *)&params->block1.w));
            liConfig.set(L"SpotDirection", *((kmVec3 *)&params->block4.w));
            liConfig.set(L"SpotFactor", *((kmVec3 *)&params->block5.z));

            lastNode.set(L"Light", liConfig);
        }
    }
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
            rootObject.set(L"Name", mOptions.nodePackname + ".root");
            rootObject.set(L"Nodes", nodes);

            lite3dpp::ConfigurationWriter objectConfig;
            objectConfig.set(L"Root", rootObject);

            Utils::saveTextFile(objectConfig.write(),
                Utils::makeFullPath(mOptions.outputFolder, Utils::makeRelativePath("objects/", mOptions.objectName, "json")));
        }
    }
}

void JsonGenerator::generateMaterial(const lite3dpp::String &name, 
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
    lite3dpp::String matName = name;

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "projectionMatrix");
        uniforms.push_back(param);
    }

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "viewMatrix");
        uniforms.push_back(param);
    }

    {
        lite3dpp::ConfigurationWriter param;
        param.set(L"Name", "normalMatrix");
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
        program.set(L"Path", mOptions.matPackname + "shaders/json/stub.json");
        pass1.set(L"Program", program);
        passes.push_back(pass1);
    }
    
    if (mOptions.useDifTexNameAsMatName && diffuseTextureFile)
    {
        matName = Utils::getFileNameWithoutExt(diffuseTextureFile);
    }

    lite3dpp::String matFull = Utils::makeFullPath(mOptions.outputFolder, Utils::makeRelativePath("materials/", matName, "json"));
    lite3dpp::ConfigurationWriter material;

    material.set(L"Uniforms", uniforms);
    material.set(L"Passes", passes);
    Utils::saveTextFile(material.write(), matFull);

    mMaterials.emplace(matIdx, matName);
}

void JsonGenerator::generateUniformSampler(lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector &uniforms, const char *fileName)
{
    if(!fileName)
        return;

    lite3dpp::ConfigurationWriter param;
    lite3dpp::String texRel = Utils::makeRelativePath("textures/json/", Utils::getFileNameWithoutExt(fileName), "json");
    lite3dpp::String texFull = Utils::makeFullPath(mOptions.outputFolder, texRel);
    
    param.set(L"Name", "diffuseSampler");
    param.set(L"Type", "sampler");
    param.set(L"TextureName", Utils::getFileNameWithoutExt(fileName) + ".texture");
    param.set(L"TexturePath", mOptions.texPackname + texRel);
    uniforms.push_back(param);
    
    {
        lite3dpp::ConfigurationWriter texture;
        texture.set(L"TextureType", "2D");
        texture.set(L"Filtering", "Trilinear");
        texture.set(L"Wrapping", "Repeat");
        texture.set(L"Image", mOptions.imgPackname + Utils::makeRelativePath("textures/images/", Utils::getFileNameWithoutExt(fileName), Utils::getFileExt(fileName)));
        texture.set(L"ImageFormat", Utils::getFileExt(fileName));
    
        Utils::saveTextFile(texture.write(), texFull);
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

void JsonGenerator::generatePositionRotation(lite3dpp::ConfigurationWriter &writer, const kmMat4 *transform)
{
    kmMat4 transformt;
    kmVec3 position;
    kmMat3 rotation;
    kmQuaternion rotationq;
    
    kmMat4Transpose(&transformt, transform);
    writer.set(L"Transform", transformt);
    
    /* extract position and rotation from transform matrix */
    kmMat4ExtractPosition(&position, &transformt);
    kmMat4ExtractRotation(&rotation, &transformt);
    /* create rotation quaternion from rotation matrix */
    kmQuaternionRotationMatrix(&rotationq, &rotation);
    /* save results */
    writer.set(L"Position", position);
    writer.set(L"Rotation", rotationq);
}
