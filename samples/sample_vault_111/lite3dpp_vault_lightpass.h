/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2016  Sirius (Korolev Nikita)
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

#include <SDL_assert.h>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class Vault111LightPass : public SceneObserver
{
public:

    class LightSourcePass
    {
    public:

        LightSourcePass(Main& render, const String& name, LightSceneNode *node) : 
            mRender(render),
            mName(name),
            mLightNode(node)
        {}

        void initPass(Scene* scene)
        {
            /* load per light big triangle */
            mLightPassShape = scene->addObject(mName, "vault_111:objects/lightpass.json", NULL);
            /* load per light big triangle material and setup light properties as uniform parameters */
            mPassMat = mRender.getResourceManager()->queryResource<Material>(mName + "_lightpass.material",
                "vault_111:materials/bsdf_lightpass.json");

            MeshSceneNode *mnode = dynamic_cast<MeshSceneNode *>(mLightPassShape->getRoot());
            SDL_assert(mnode);
            SDL_assert(mPassMat);
            SDL_assert(mLightNode);

            mLightNode->translateToWorld();
            const auto lightSource = mLightNode->getLight();
            mPassMat->setIntParameter(1, "Light.enabled", lightSource->enabled() ? 1 : 0, false);
            mPassMat->setIntParameter(1, "Light.type", lightSource->getType(), false);
            mPassMat->setFloatv3Parameter(1, "Light.position", lightSource->getPositionWorld(), false);
            mPassMat->setFloatv3Parameter(1, "Light.diffuse", lightSource->getDiffuse(), false);
            mPassMat->setFloatv3Parameter(1, "Light.direction", lightSource->getDirectionWorld(), false);
            mPassMat->setFloatParameter(1, "Light.influenceDistance", lightSource->getInfluenceDistance(), false);
            mPassMat->setFloatParameter(1, "Light.attenuationContant", lightSource->getAttenuationConstant(), false);
            mPassMat->setFloatParameter(1, "Light.attenuationLinear", lightSource->getAttenuationLinear(), false);
            mPassMat->setFloatParameter(1, "Light.attenuationQuadratic", lightSource->getAttenuationQuadratic(), false);
            mPassMat->setFloatParameter(1, "Light.innercone", lightSource->getAngleInnerCone(), false);
            mPassMat->setFloatParameter(1, "Light.outercone", lightSource->getAngleOuterCone(), false);
            mPassMat->setFloatParameter(1, "Light.radiance", lightSource->getRadiance(), false);
            
            mnode->frustumTest(false);
            mnode->setName(mName);
            mnode->replaceMaterial(0, mPassMat);
        }

        void validate()
        {
            SDL_assert(mLightNode);
            SDL_assert(mLightPassShape);

            if (mLightNode->isVisible())
                mLightPassShape->enable();
            else
                mLightPassShape->disable();
        }

    private:

        Main& mRender;
        String mName;
        Material* mPassMat = nullptr;
        LightSceneNode* mLightNode;
        SceneObject* mLightPassShape = nullptr;
    };

    Vault111LightPass(Main& render) :
        mRender(render)
    {}

    void createScene()
    {
        // load intermediate light compute scene
        mLightComputeScene = mRender.getResourceManager()->queryResource<Scene>("Vault_111_LightCompute",
            "vault_111:scenes/lightpass.json");
    }

    void addLightPass(const String& name, LightSceneNode *node)
    {
        mLightsPasses.emplace_back(mRender, name, node);
        mLightsPasses.back().initPass(mLightComputeScene);
    }

    // validate visibility of light sources before light compute step
    bool beginSceneRender(Scene *scene, Camera *camera) override
    {
        for (auto& pass : mLightsPasses)
        {
            pass.validate();
        }

        return true;
    }

    bool isInited() const
    {
        return mLightsPasses.size() > 0;
    }

private:

    Main& mRender;
    Scene* mLightComputeScene = nullptr;
    SceneObject* mAmbientLayer = nullptr;
    stl<LightSourcePass>::vector mLightsPasses;
};

}}
