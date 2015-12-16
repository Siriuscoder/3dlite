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
    const lite3dpp::String &objectName) :
    mOutputFolder(outputFolder),
    mObjectName(objectName)
{}

NullGenerator::NullGenerator() :
    Generator("", "")
{}

void NullGenerator::generateNode(const lite3dpp::String &name, const kmMat4 *transform,
    bool meshExist)
{}

void NullGenerator::pushNodeTree()
{}

void NullGenerator::popNodeTree()
{}

JsonGenerator::JsonGenerator(const lite3dpp::String &outputFolder,
    const lite3dpp::String &objectName) : 
    Generator(outputFolder, objectName)
{}

void JsonGenerator::generateNode(const lite3dpp::String &name, const kmMat4 *transform,
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
        meshConfig.set(L"Model", relativeMeshPath);

        lite3dpp::ConfigurationWriter nodeMeshConfig;
        nodeMeshConfig.set(L"Name", name + ".mesh");
        nodeMeshConfig.set(L"Mesh", relativeMeshConfigPath);
        nodeConfig.set(L"Mesh", nodeMeshConfig);

        lite3dpp::String jsonData = meshConfig.write();
        Utils::saveFile(jsonData.data(), jsonData.size(), fullMeshConfigPath);
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

            lite3dpp::String jsonData = objectConfig.write();
            Utils::saveFile(jsonData.data(), jsonData.size(), 
                Utils::makeFullPath(mOutputFolder, Utils::makeRelativePath("objects/", mObjectName, "json")));
            objectConfig.clear();
        }
    }
}