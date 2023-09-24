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
#include <iostream>
#include <string>

#include "lite3dpp_vault_lightpass.h"

namespace lite3dpp {
namespace samples {

class SampleVault111 : public Sample, public SceneObserver
{
public:

    SampleVault111() : 
        mLightPassScene(getMain())
    {}

    void createScene() override
    {
        mVaultScene = getMain().getResourceManager()->queryResource<Scene>("Vault_111", "vault_111:scenes/vault_111.json");
        mVaultScene->addObserver(this);
        setMainCamera(getMain().getCamera("MyCamera"));

        mLightPassScene.createScene();

        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("Vault_111_Postprocess",
            "vault_111:scenes/postprocess.json");
        
        // optimize: window clean not needed, because all pixels in last render target always be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
        getMain().window()->depthTestFunc(RenderTarget::TestFuncLEqual);

        kmVec3 resolution = { (float)getMain().window()->width(), (float)getMain().window()->height(), 0 };
        Material::setFloatv3GlobalParameter("screenResolution", resolution);
    }

    void initLightsPass()
    {
        if (!mLightPassScene.isInited())
        {
            for (const auto &light : mVaultScene->getLights())
            {
                mLightPassScene.addLightPass(light.first, light.second);
            }
        }
    }

    void endSceneRender(Scene *scene, Camera *camera) override
    {
        initLightsPass();
    }

    void mainCameraChanged() override
    {
        Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
    }

private:

    Vault111LightPass mLightPassScene;
    Scene* mVaultScene = nullptr;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVault111 sample;
    return sample.start("vault_111/config/config.json");
}

