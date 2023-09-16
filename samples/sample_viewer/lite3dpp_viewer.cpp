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

class SampleViewer : public Sample
{
public:

    SampleViewer(const std::string &scene) : 
        mSceneName(scene)
    {}

    void createScene() override
    {
        getMain().getResourceManager()->queryResource<Scene>("ViewScene", mSceneName.c_str());
        setMainCamera(getMain().getCamera("MyCamera"));
    }

private:

    std::string mSceneName;
};

}}

int main(int agrc, char *args[])
{
    if (agrc != 2)
    {
        std::cerr << "Wrong options, path to scene file must be specifed" << std::endl;
        return 1;
    }

    lite3dpp::samples::SampleViewer sample(std::string("samples:") + args[1]);
    return sample.start("samples/config/config.json");
}

