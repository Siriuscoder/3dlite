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

class Cellwallerkiller : public Sample
{
public:

    void createScene() override
    {
        Scene *scene = getMain().getResourceManager()->queryResource<lite3dpp::Scene>("InstancedRobots",
            "samples:scenes/robots.json");
        setMainCamera(scene->getCamera("MyCamera"));

        kmVec4 bColor = { 0.05, 0.38, 0.45, 1.0 };
        getMainWindow().setBackgroundColor(bColor);
        Material::setFloatv4GlobalParameter("fogColor", bColor);
    }
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::Cellwallerkiller sample;
    return sample.start("samples/config/config.json");
}
