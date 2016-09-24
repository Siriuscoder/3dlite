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
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class Vault : public Sample
{
public:

    void createScene() override
    {
        Scene *scene = getMain().getResourceManager()->queryResource<Scene>("Vault",
            "vaultmat:scenes/vault.json");
        setMainCamera(scene->getCamera("MyCamera"));
        Material::setFloatGlobalParameter("mode", 2);
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_m)
            {
                static int mode = 2;
                if (mode == 1)
                    mode = 2;
                else
                    mode = 1;
                
                lite3dpp::Material::setFloatGlobalParameter("mode", (float)mode);
            }
        }
    }
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::Vault sample;
    return sample.start("vault/config/config_vault.json");
}

