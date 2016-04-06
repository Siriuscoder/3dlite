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
#pragma once

#include <lite3d/lite3d_scene.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_mesh.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneNode : public Manageable, public Noncopiable
    {
    public:

        SceneNode();
        SceneNode(const ConfigurationReader &json, SceneNode *base, Main *main);
        ~SceneNode();

        inline void setName(const String &name)
        { mName = name; }

        inline String getName() const
        { return mName; }

        void setMesh(Mesh *mesh);
        inline Mesh *getMesh()
        { return mMesh; }

        inline lite3d_scene_node *getPtr()
        { return &mNode; }
        inline const lite3d_scene_node *getPtr() const
        { return &mNode; }
        inline void frustumTest(bool flag)
        { mNode.frustumTest = flag ? LITE3D_TRUE : LITE3D_FALSE; }
        inline void instances(size_t count)
        { mInstances = count; }
        

        void setPosition(const kmVec3 &position);
        void move(const kmVec3 &position);
        void setRotation(const kmQuaternion &quat);
        void rotate(const kmQuaternion &quat);
        void rotateAngle(const kmVec3 &axis, float angle);
        void scale(const kmVec3 &scale);
        void setTransform(const kmMat4 &mat);

        void replaceMaterial(int chunkNo, Material *material);

        void addToScene(Scene *scene);
        void removeFromScene(Scene *scene);

    private:

        lite3d_scene_node mNode;
        Mesh::MaterialMapping mMaterialMappingReplacement;
        Mesh *mMesh;
        String mName;
        SceneNode *mBaseNode;
        size_t mInstances;
    };
}

