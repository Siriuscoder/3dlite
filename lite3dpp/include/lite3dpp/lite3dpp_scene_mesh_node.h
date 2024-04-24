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
#pragma once

#include <lite3dpp/lite3dpp_scene_node.h>
#include <lite3dpp/lite3dpp_mesh.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT MeshSceneNode : public SceneNode
    {
    public:

        using Ptr = std::shared_ptr<MeshSceneNode>;
        
        MeshSceneNode(const ConfigurationReader &json, SceneNodeBase *parent, Scene *scene);
        
        inline Mesh *getMesh()
        { return mMesh; }
        inline const Mesh *getMesh() const
        { return mMesh; }
        
        inline void instances(uint32_t count)
        { mInstances = count; }
        
        void replaceMaterial(int chunkNo, Material *material);
            
    protected:

        void setMesh(Mesh *mesh);
        void applyMaterial(int chunkNo, Material *material);
        
    private:
        
        Mesh::MaterialMapping mMaterialMappingReplacement;
        Mesh *mMesh = nullptr;
        uint32_t mInstances = 1;
    };
}
