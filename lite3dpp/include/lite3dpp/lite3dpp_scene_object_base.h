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

#include <lite3dpp/lite3dpp_scene_node_base.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneObjectBase : public Manageable, public Noncopiable
    {
    public:

        SceneObjectBase(const String &name, Scene *scene, Main *main, SceneObjectBase *parent, const kmVec3 &initialPosition, 
            const kmQuaternion &initialRotation, const kmVec3 &initialScale);
        virtual ~SceneObjectBase() = default;

        inline SceneNodeBase *getRoot()
        { return mObjectRoot; }
        inline const SceneNodeBase *getRoot() const
        { return mObjectRoot; }
        inline SceneObjectBase *getParent()
        { return mParent; }
        inline const SceneObjectBase *getParent() const
        { return mParent; }
        inline void setParent(SceneObjectBase *parent)
        { mParent = parent; }
        inline const String &getName() const 
        { return mName; }
        const Main &getMain() const;
        Main &getMain();

        Scene *getScene();
        
        virtual const kmVec3& getPosition() const;
        virtual kmVec3 getWorldPosition() const;
        virtual void setPosition(const kmVec3 &position);
        virtual void move(const kmVec3 &position);
        virtual void moveRelative(const kmVec3 &p);
        virtual void scale(const kmVec3 &scale);
        
        virtual const kmQuaternion& getRotation() const;
        virtual kmQuaternion getWorldRotation() const;
        virtual void setRotation(const kmQuaternion &quat);
        virtual void rotate(const kmQuaternion &quat);
        virtual void rotateAngle(const kmVec3 &axis, float angle);
        virtual void rotateY(float angleDelta);
        virtual void rotateX(float angleDelta);
        virtual void rotateZ(float angleDelta);

        void loadFromTemplateFromFile(const String &templateJsonPath);
        virtual void loadFromTemplate(const ConfigurationReader& conf) = 0;

        virtual void rebase(SceneObjectBase *parent);

        bool isEnabled() const;
        virtual void disable();
        virtual void enable();

    protected:

        inline void setRoot(SceneNodeBase *root)
        { mObjectRoot = root; }

    private:

        String mName;
        Scene *mScene = nullptr;
        Main *mMain = nullptr;
        SceneNodeBase *mObjectRoot = nullptr;
        SceneObjectBase *mParent = nullptr;
        
    protected:

        kmVec3 mInitialPosition;
        kmQuaternion mInitialRotation;
        kmVec3 mInitialScale;
    };
}

