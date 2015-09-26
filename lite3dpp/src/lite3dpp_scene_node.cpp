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
#include <SDL_assert.h>

#include <SDL_log.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene_node.h>

namespace lite3dpp
{
    SceneNode::SceneNode() : 
        mMesh(NULL), 
        mBaseNode(NULL)
    {

    }

    SceneNode::SceneNode(const JsonHelper &json, SceneNode *base, Main *main) : 
        mMesh(NULL),
        mBaseNode(base)
    {
        SDL_assert(main);

        lite3d_scene_node_init(&mNode);

        mName = json.getString(L"Name");
        if(mName.size() == 0)
            throw std::runtime_error("Node must have a name..");

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Parsing node %s ...", mName.c_str());

        auto meshHelper = json.getObject(L"Mesh");
        if(!meshHelper.isEmpty())
        {
            setMesh(main->getResourceManager()->queryResource<Mesh>(
                meshHelper.getString(L"Name"),
                meshHelper.getString(L"Mesh")));

            for(auto &matMap : meshHelper.getObjects(L"MaterialMapping"))
            {
                replaceMaterial(matMap.getInt(L"MaterialIndex"),
                    main->getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material")));
            }
        }

        setPosition(json.getVec3(L"Position"));
        setRotation(json.getQuaternion(L"Rotation"));
        scale(json.getVec3(L"Scale", KM_VEC3_ONE));
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

    void SceneNode::replaceMaterial(int chunkNo, Material *material)
    {
        mMaterialMappingReplacement[chunkNo] = material;
    }

    void SceneNode::setPosition(const kmVec3 &position)
    {
        lite3d_scene_node_set_position(&mNode, &position);
    }

    void SceneNode::move(const kmVec3 &position)
    {
        lite3d_scene_node_move(&mNode, &position);
    }

    void SceneNode::setRotation(const kmQuaternion &quat)
    {
        lite3d_scene_node_set_rotation(&mNode, &quat);
    }

    void SceneNode::rotate(const kmQuaternion &quat)
    {
        lite3d_scene_node_rotate(&mNode, &quat);
    }

    void SceneNode::rotateAngle(const kmVec3 &axis, float angle)
    {
        lite3d_scene_node_rotate_angle(&mNode, &axis, angle);
    }

    void SceneNode::scale(const kmVec3 &scale)
    {
        lite3d_scene_node_scale(&mNode, &scale);
    }

    void SceneNode::addToScene(Scene *scene)
    {
        /* attach node to scene */
        if(!lite3d_scene_add_node(scene->getPtr(), &mNode, mBaseNode ? &mBaseNode->mNode : NULL))
            throw std::runtime_error("Attaching node failed..");

        if(!mMesh)
        {
            mNode.renderable = 0;
            return;
        }

        /* touch material and mesh chunk to node */ 
        for(auto &material : mMaterialMappingReplacement)
        {
            if(!lite3d_scene_node_touch_material(&mNode, 
                lite3d_mesh_chunk_get_by_index(mMesh->getPtr(), material.first), material.second->getPtr()))
                throw std::runtime_error("Linking node failed..");
        }
    }

    void SceneNode::removeFromScene(Scene *scene)
    {
        lite3d_scene_remove_node(scene->getPtr(), &mNode);
    }
}

