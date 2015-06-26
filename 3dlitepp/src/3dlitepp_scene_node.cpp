/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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
#include <SDL_assert.h>

#include <3dlitepp/3dlitepp_scene_node.h>

namespace lite3dpp
{
    SceneNode::SceneNode() : 
        mMesh(NULL)
    {

    }

    SceneNode::SceneNode(const JsonHelper &json) : 
        mMesh(NULL)
    {

    }

    SceneNode::~SceneNode()
    {}

    void SceneNode::setMesh(Mesh *mesh)
    {
        SDL_assert(mesh);
        if(mMesh == NULL)
        {
            mMaterialMappingReplacement = mesh->getMaterialMapping();
            mMesh = mesh;
        }
    }

    void SceneNode::replaceMaterial(int unit, Material *material)
    {
        mMaterialMappingReplacement[unit] = material;
    }
}

