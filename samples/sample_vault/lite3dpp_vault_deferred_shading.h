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

#include "lite3dpp_vault_base.h"

namespace lite3dpp {
namespace samples {

class VaultDF : public VaultBase, public SceneObserver
{
public:
    
    VaultDF(const std::string_view &helpString) : 
        VaultBase(helpString)
    {}

    void createPipeline() override
    {
        // load main scene as precompute step
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault",
            "vaultmat:scenes/prepass.json");
        // load intermediate light compute scene
        getMain().getResourceManager()->queryResource<Scene>("VaultLightComputeStep",
            "vaultmat:scenes/lightpass.json")->addObserver(this);
        mLightComputeStep = getMain().getResourceManager()->queryResource<TextureRenderTarget>("LightComputeStep");
        // Scene that combines lightmap from previous step and textures and draw all transparent objects at end of step.
        getMain().getResourceManager()->queryResource<Scene>("VaultCombineStep",
            "vaultmat:scenes/combine.json");
        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("VaultPostprocessStep",
            "vaultmat:scenes/postprocess.json");
        
        // optimize: window clean not needed, because all pixels in last render target always be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
    }

    // setup lighting at once before light compute scene begin rendering first time
    bool beginSceneRender(Scene *scene, Camera *camera) override
    {
        SDL_assert(mVaultScene);
        /* check scene already fullup */
        if (scene->getObjects().size() == 0)
            setupLightPassScene(mVaultScene, scene);
        
        setLightLayersVisibility(mVaultScene, scene);
        
        return true;
    }

    void endSceneRender(Scene *scene, Camera *camera) override 
    {
        /* disable lightpass after lightmap recalc complete */
        SDL_assert(mLightComputeStep);

        if (!mMinigun->getRoot()->isVisible() && 
            !mGatling->getRoot()->isVisible() &&
            !mLazer->getRoot()->isVisible())
            mLightComputeStep->disable();
    }
    
    void setLightLayersVisibility(Scene *prepass, Scene *scene)
    {
        for (const auto &light : prepass->getLights())
        {
            SceneObject *lo = scene->getObject(light->getName());
            SDL_assert(lo);
            
            if (light->isVisible())
                lo->enable();
            else
                lo->disable();
        }       
    }
    
    void setupLightPassScene(Scene *prepass, Scene *scene)
    {
        for (const auto &light : prepass->getLights())
        {
            /* load per light big triangle */
            SceneObject *lo = scene->addObject(light->getName(), "vault:objects/lightpass_tri.json");
            /* load per light big triangle material and setup light properties as uniform parameters */
            Material *material = getMain().getResourceManager()->queryResource<Material>(light->getName() + ".material",
                "vaultmat:materials/lightpass.json");

            MeshSceneNode *mnode = (MeshSceneNode *)lo->getRoot();
            SDL_assert(mnode);


            light->translateToWorld();
            const auto &lightSource = *light->getLight();
            material->setIntParameter(1, "light.enabled", lightSource.enabled() ? 1 : 0, false);
            material->setIntParameter(1, "light.type", lightSource.getType(), false);
            material->setFloatv3Parameter(1, "light.position", lightSource.getWorldPosition(), false);
            material->setFloatv3Parameter(1, "light.diffuse", lightSource.getDiffuse(), false);
            material->setFloatv3Parameter(1, "light.direction", lightSource.getWorldDirection(), false);
            material->setFloatParameter(1, "light.influenceDistance", lightSource.getInfluenceDistance(), false);
            material->setFloatParameter(1, "light.attenuationContant", lightSource.getAttenuationConstant(), false);
            material->setFloatParameter(1, "light.attenuationLinear", lightSource.getAttenuationLinear(), false);
            material->setFloatParameter(1, "light.attenuationQuadratic", lightSource.getAttenuationQuadratic(), false);
            material->setFloatParameter(1, "light.innercone", lightSource.getAngleInnerCone(), false);
            material->setFloatParameter(1, "light.outercone", lightSource.getAngleOuterCone(), false);

            mnode->frustumTest(false);
            mnode->setName(light->getName());
            mnode->replaceMaterial(0, material);mnode->replaceMaterial(0, material);
        }
    }

    /* enable lightpass then the main camera state changed to recalc lightmap */
    void mainCameraChanged() override
    {
        VaultBase::mainCameraChanged();
        SDL_assert(mLightComputeStep);
        mLightComputeStep->enable();
    }
    
private:
    
    RenderTarget *mLightComputeStep = nullptr;
};

}}

