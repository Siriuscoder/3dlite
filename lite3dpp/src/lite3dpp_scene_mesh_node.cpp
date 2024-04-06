/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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

#include <lite3dpp/lite3dpp_scene_mesh_node.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    MeshSceneNode::MeshSceneNode(const ConfigurationReader &json, SceneNode *parent, Scene *scene, Main *main) : 
        SceneNode(json, parent, scene, main)
    {
        instances(json.getInt(L"Instances", 1));

        auto meshHelper = json.getObject(L"Mesh");
        if(!meshHelper.isEmpty())
        {
            setMesh(mMain->getResourceManager()->queryResource<Mesh>(
                meshHelper.getString(L"Name"),
                meshHelper.getString(L"Mesh")));

            for (auto &matMap : meshHelper.getObjects(L"MaterialMapping"))
            {
                mMaterialMappingReplacement[matMap.getInt(L"MaterialIndex")] = 
                    mMain->getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material"));
            }

            for (auto &material : mMaterialMappingReplacement)
            {
                applyMaterial(material.first, material.second);
            }
        }
    }
    
    void MeshSceneNode::setMesh(Mesh *mesh)
    {
        SDL_assert(mesh);
        mMaterialMappingReplacement = mesh->getMaterialMapping();
        mMesh = mesh;
    }

    void MeshSceneNode::replaceMaterial(int chunkNo, Material *material)
    {
        mMaterialMappingReplacement[chunkNo] = material;
        applyMaterial(chunkNo, material);
    }

    void MeshSceneNode::applyMaterial(int chunkNo, Material *material)
    {
        if (mMesh) /* check node is attached to scene */
        {
            getPtr()->renderable = LITE3D_TRUE;
            /* touch material and mesh chunk to node */ 
            lite3d_mesh_chunk *meshChunk = lite3d_mesh_chunk_get_by_index(mMesh->getPtr(), chunkNo);
                lite3d_mesh_chunk *bbMeshChunk = mMesh->getBBPtr() ? 
                lite3d_mesh_chunk_get_by_index(mMesh->getBBPtr(), chunkNo) : nullptr;

                if(!lite3d_scene_node_touch_material(getPtr(), 
                meshChunk, bbMeshChunk, material->getPtr(), mInstances))
                {
                    LITE3D_THROW("Linking node failed..");
            }
        }
    }
}
