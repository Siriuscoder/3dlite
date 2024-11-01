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
    class LITE3DPP_EXPORT SceneNodeBase : public Manageable, public Noncopiable
    {
    public:

        SceneNodeBase(lite3d_scene_node *node);
        virtual ~SceneNodeBase() = default;

        inline void setName(const String &name)
        { mName = name; }
        inline String getName() const
        { return mName; }

        inline lite3d_scene_node *getPtr()
        { return mNodePtr; }
        inline const lite3d_scene_node *getPtr() const
        { return mNodePtr; }

        SceneNodeBase *getParent();
        const SceneNodeBase *getParent() const;

        Main *getMain();
        Scene *getScene();

        void frustumTest(bool flag);
        const bool frustumTest() const;

        void setPosition(const kmVec3 &position);
        const kmVec3& getPosition() const;
        kmVec3 getWorldPosition() const;
        void move(const kmVec3 &position);
        void moveRelative(const kmVec3 &p);
        
        void setRotation(const kmQuaternion &quat);
        const kmQuaternion& getRotation() const;
        kmQuaternion getWorldRotation() const;
        void rotate(const kmQuaternion &quat);
        void rotateAngle(const kmVec3 &axis, float angle);
        void rotateY(float angleDelta);
        void rotateX(float angleDelta);
        void rotateZ(float angleDelta);
        
        void scale(const kmVec3 &scale);

        void setVisible(bool flag);
        bool isVisible() const;

        void setRenderable(bool flag);
        bool isRenderable() const;

    private:

        String mName;
        lite3d_scene_node *mNodePtr = nullptr;
    };
}
