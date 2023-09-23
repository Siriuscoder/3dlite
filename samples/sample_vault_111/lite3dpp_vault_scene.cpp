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
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class SampleVault111 : public Sample
{
public:

    void createScene() override
    {
        getMain().getResourceManager()->queryResource<Scene>("Vault_111", "vault_111:scenes/vault_111.json");
        setMainCamera(getMain().getCamera("MyCamera"));

        // load intermediate light compute scene
        auto lightCompute = getMain().getResourceManager()->queryResource<Scene>("Vault_111_LightCompute",
            "vault_111:scenes/lightpass.json");
        // postprocess step, fxaa, gamma correcion, draw directly info window. 
        getMain().getResourceManager()->queryResource<Scene>("Vault_111_Postprocess",
            "vault_111:scenes/postprocess.json");

        SceneObject *ambientLayer = lightCompute->addObject("lightpass_ambient_layer", "vault_111:objects/lightpass.json", NULL);
        Material *materialAmbientLayer = getMain().getResourceManager()->queryResource<Material>("lightpass_ambient.material",
            "vault_111:materials/bsdf_lightpass_ambient.json");

        auto mnode = dynamic_cast<MeshSceneNode *>(ambientLayer->getRoot());
        mnode->setName("lightpass_ambient_layer_node");
        mnode->replaceMaterial(0, materialAmbientLayer);
        
        // optimize: window clean not needed, because all pixels in last render target always be updated
        getMain().window()->setBuffersCleanBit(false, false, false);
        getMain().window()->depthTestFunc(RenderTarget::TestFuncLEqual);

        getMain().getResourceManager()->releaseFileCache();
    }

    void mainCameraChanged() override
    {
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
    }

private:

    std::string mSceneName;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleVault111 sample;
    return sample.start("vault_111/config/config.json");
}

