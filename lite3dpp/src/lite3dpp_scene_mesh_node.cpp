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
#include <lite3dpp/lite3dpp_scene_mesh_node.h>

#include <SDL_assert.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    MeshSceneNode::MeshSceneNode(const ConfigurationReader &json, SceneNodeBase *parent, Scene *scene) : 
        SceneNode(json, parent, scene)
    {
        instances(json.getInt(L"Instances", 1));
        SDL_assert(getMain());

        auto meshHelper = json.getObject(L"Mesh");
        if (!meshHelper.isEmpty())
        {
            setMesh(getMain()->getResourceManager()->queryResource<Mesh>(
                meshHelper.getString(L"Name"),
                meshHelper.getString(L"Mesh")));

            stl<uint32_t, Material *>::unordered_map materials;
            for (auto &matMap : meshHelper.getObjects(L"MaterialMapping"))
            {
                materials[matMap.getInt(L"MaterialIndex")] = 
                    getMain()->getResourceManager()->queryResource<Material>(
                    matMap.getObject(L"Material").getString(L"Name"),
                    matMap.getObject(L"Material").getString(L"Material"));
            }

            for (size_t i = 0; i < mMesh->chunksCount(); ++i)
            {
                auto chunk = mMesh->getChunk(i);
                auto materialReplacedIt = materials.find(chunk.chunk->materialIndex);
                Material *material = materialReplacedIt != materials.end() ? materialReplacedIt->second : chunk.material;
                if (material)
                {
                    applyMaterial(chunk, material);
                }
            }
        }
    }
    
    void MeshSceneNode::setMesh(Mesh *mesh)
    {
        SDL_assert(mesh);
        mMesh = mesh;
    }
    
    void MeshSceneNode::applyMaterial(uint32_t materialID, Material *material)
    {
        SDL_assert(material);
        if (!mMesh)
            return;

        for (size_t i = 0; i < mMesh->chunksCount(); ++i)
        {
            auto chunk = mMesh->getChunk(i);
            if (chunk.chunk->materialIndex == materialID)
            {
                applyMaterial(chunk, material);
            }
        }
    }

    void MeshSceneNode::applyMaterial(const Mesh::ChunkEntity &entity, Material *material)
    {
        SDL_assert(material);
        // Берем сгенерированный boundig box если он есть (может и не быть)
        lite3d_mesh_chunk *boundigBoxChunk = entity.boudingBoxChunkIndex ? 
            mMesh->getChunkBoudingBox(entity.boudingBoxChunkIndex.value()).chunk : nullptr;

        if (!lite3d_scene_node_touch_material(getPtr(), entity.chunk, boundigBoxChunk, material->getPtr(), mInstances))
        {
            LITE3D_THROW(getName() << ": Failed to attach node to scene '" << 
                (getScene() ? getScene()->getName() : "none") << "'");
        }
    }
}
