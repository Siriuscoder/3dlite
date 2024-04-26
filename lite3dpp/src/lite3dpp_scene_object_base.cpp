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
#include <lite3dpp/lite3dpp_scene_object_base.h>

#include <algorithm>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp
{
    SceneObjectBase::SceneObjectBase(const String &name, Scene *scene, Main *main, SceneObjectBase *parent, 
        const kmVec3 &initialPosition, const kmQuaternion &initialRotation, const kmVec3 &initialScale) : 
        mName(name),
        mScene(scene),
        mMain(main),
        mParent(parent),
        mInitialPosition(initialPosition),
        mInitialRotation(initialRotation),
        mInitialScale(initialScale)
    {}

    const Main &SceneObjectBase::getMain() const
    {
        SDL_assert(mMain);
        return *mMain; 
    }

    Main &SceneObjectBase::getMain()
    { 
        SDL_assert(mMain);
        return *mMain;
    }

    Scene *SceneObjectBase::getScene()
    {
        if (getRoot() && getRoot()->getScene())
        {
            if (mScene != getRoot()->getScene())
            {
                mScene = getRoot()->getScene();
            }
        }

        return mScene;
    }

    void SceneObjectBase::rebase(SceneObjectBase *parent)
    {
        if (getRoot() && parent->getRoot() && getScene())
        {
            if (!lite3d_scene_rebase_node(getScene()->getPtr(), getRoot()->getPtr(), parent->getRoot()->getPtr()))
            {
                LITE3D_THROW("Fail to rebase node '" << getRoot()->getName() << "', parent object '" << getParent()->getName() 
                    << "' was attached to another scene '" << getParent()->getScene()->getName() << "'");
            }

            setParent(parent);
        }
    }

    void SceneObjectBase::loadFromTemplateFromFile(const String &templatePath)
    {
        size_t fileSize = 0;
        const void *fileData = getMain().getResourceManager()->loadFileToMemory(templatePath, &fileSize);
        ConfigurationReader conf(static_cast<const char *>(fileData), fileSize);
        loadFromTemplate(conf);
    }

    bool SceneObjectBase::isEnabled() const
    {
        SDL_assert(getRoot());
        return getRoot()->getPtr()->enabled == LITE3D_TRUE;
    }

    void SceneObjectBase::enable()
    {
        SDL_assert(getRoot());
        getRoot()->getPtr()->enabled = LITE3D_TRUE;
    }

    void SceneObjectBase::disable()
    {
        SDL_assert(getRoot());
        getRoot()->getPtr()->enabled = LITE3D_FALSE;
    }
    
    const kmVec3& SceneObjectBase::getPosition() const
    {
        SDL_assert(getRoot());
        return getRoot()->getPosition();
    }

    const kmQuaternion& SceneObjectBase::getRotation() const
    {
        SDL_assert(getRoot());
        return getRoot()->getRotation();
    }

    kmVec3 SceneObjectBase::getWorldPosition() const
    {
        SDL_assert(getRoot());
        return getRoot()->getWorldPosition();
    }

    kmQuaternion SceneObjectBase::getWorldRotation() const
    {
        SDL_assert(getRoot());
        return getRoot()->getWorldRotation();
    }

    void SceneObjectBase::setPosition(const kmVec3 &position)
    {
        SDL_assert(getRoot());
        getRoot()->setPosition(position);
    }

    void SceneObjectBase::move(const kmVec3 &position)
    {
        SDL_assert(getRoot());
        getRoot()->move(position);
    }

    void SceneObjectBase::moveRelative(const kmVec3 &offset)
    {
        SDL_assert(getRoot());
        getRoot()->moveRelative(offset);
    }

    void SceneObjectBase::setRotation(const kmQuaternion &quat)
    {
        SDL_assert(getRoot());
        getRoot()->setRotation(quat);
    }

    void SceneObjectBase::rotate(const kmQuaternion &quat)
    {
        SDL_assert(getRoot());
        getRoot()->rotate(quat);
    }

    void SceneObjectBase::rotateAngle(const kmVec3 &axis, float angle)
    {
        SDL_assert(getRoot());
        getRoot()->rotateAngle(axis, angle);
    }

    void SceneObjectBase::scale(const kmVec3 &scale)
    {
        SDL_assert(getRoot());
        getRoot()->scale(scale);
    }

    void SceneObjectBase::rotateY(float angleDelta)
    {
        SDL_assert(getRoot());
        getRoot()->rotateY(angleDelta);
    }

    void SceneObjectBase::rotateX(float angleDelta)
    {
        SDL_assert(getRoot());
        getRoot()->rotateX(angleDelta);
    }

    void SceneObjectBase::rotateZ(float angleDelta)
    {
        SDL_assert(getRoot());
        getRoot()->rotateZ(angleDelta);
    }
}

