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
#pragma once

#include <3dlite/3dlite_scene.h>

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_json_helper.h>
#include <3dlitepp/3dlitepp_mesh.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneNode : public Manageable, public NoncopiableResource
    {
    public:

        SceneNode();
        SceneNode(const JsonHelper &json, SceneNode *base, Main *main);
        ~SceneNode();

        inline void setName(const lite3dpp_string &name)
        { mName = name; }

        inline lite3dpp_string getName() const
        { return mName; }

        void setMesh(Mesh *mesh);
        inline Mesh *getMesh()
        { return mMesh; }

        inline lite3d_scene_node *getPtr()
        { return &mNode; }
        inline const lite3d_scene_node *getPtr() const
        { return &mNode; }

        void setPosition(const kmVec3 *position);
        void move(const kmVec3 *position);
        void setRotation(const kmQuaternion *quat);
        void rotate(const kmQuaternion *quat);
        void rotateAngle(const kmVec3 *axis, float angle);
        void scale(const kmVec3 *scale);

        void replaceMaterial(int chunkNo, Material *material);

        void addToScene(Scene *scene);
        void removeFromScene(Scene *scene);

    private:

        lite3d_scene_node mNode;
        Mesh::MaterialMapping mMaterialMappingReplacement;
        Mesh *mMesh;
        lite3dpp_string mName;
        SceneNode *mBaseNode;
    };
}

