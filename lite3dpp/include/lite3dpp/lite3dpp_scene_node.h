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

#include <lite3d/lite3d_scene.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneNode : public Manageable, public Noncopiable
    {
    public:
        
        using Ptr = std::shared_ptr<SceneNode>;

        SceneNode(const ConfigurationReader &json, SceneNode *parent, Scene *scene, Main *main);
        virtual ~SceneNode() = default;

        inline void setName(const String &name)
        { mName = name; }

        inline String getName() const
        { return mName; }

        inline lite3d_scene_node *getPtr()
        { return &mNode; }
        inline const lite3d_scene_node *getPtr() const
        { return &mNode; }
        inline void frustumTest(bool flag)
        { mNode.frustumTest = flag ? LITE3D_TRUE : LITE3D_FALSE; }
        inline const bool frustumTest() const
        { return mNode.frustumTest == LITE3D_TRUE; }
        

        void setPosition(const kmVec3 &position);
        const kmVec3& getPosition() const;
        void move(const kmVec3 &position);
        void moveRelative(const kmVec3 &p);
        void setRotation(const kmQuaternion &quat);
        const kmQuaternion& getRotation() const;
        void rotate(const kmQuaternion &quat);
        void rotateAngle(const kmVec3 &axis, float angle);
        void scale(const kmVec3 &scale);
        void setVisible(bool flag);
        bool isVisible() const;

        inline SceneNode *getParent()
        { return mParentNode; }
        inline const SceneNode *getParent() const
        { return mParentNode; }

        virtual void detachNode();

    protected:

        lite3d_scene_node mNode;
        SceneNode *mParentNode = nullptr;
        Scene *mScene = nullptr;
        Main *mMain = nullptr;

    private:

        String mName;
    };
}
