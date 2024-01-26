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
#include <iostream>
#include <string>
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

class SampleReflectionSky : public Sample
{
public:

    void createScene() override
    {
        getMain().getResourceManager()->queryResource<Scene>("ReflectionSky", "samples:scenes/reflections.json");
        // Основной cubemap уже загружен сценой, просто надо получить
        mSkyCube = getMain().getResourceManager()->queryResource<TextureImage>("skybox.texture");
        // Загружаем альтернативный cubemap
        mCryoCube = getMain().getResourceManager()->queryResource<TextureImage>("cryo_cube.texture", "samples:textures/json/skybox_cryo.json");
        // Прихраниваем материалы для дальнейшей смены cubemap
        mSkyMaterial = getMain().getResourceManager()->queryResource<Material>("skycube.material");
        mReflectionMaterial = getMain().getResourceManager()->queryResource<Material>("reflection.material");
        // Установка главной камеры (для перемещения)
        setMainCamera(getMain().getCamera("MyCamera"));
        // Важно для отрисовки sky cube
        RenderTarget::depthTestFunc(RenderTarget::TestFuncLEqual);
        // Позиция наблюдателя
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
    }

    void mainCameraChanged() override
    {
        Sample::mainCameraChanged();
        lite3dpp::Material::setFloatv3GlobalParameter("eye", getMainCamera().getPosition());
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_e)
            {
                static bool cubeSwitch = true;
                cubeSwitch = !cubeSwitch;
                if (cubeSwitch)
                {
                    mSkyMaterial->setSamplerParameter(1, "skybox", *mSkyCube);
                    mReflectionMaterial->setSamplerParameter(1, "cube", *mSkyCube);
                }
                else
                {
                    mSkyMaterial->setSamplerParameter(1, "skybox", *mCryoCube);
                    mReflectionMaterial->setSamplerParameter(1, "cube", *mCryoCube);
                }
            }
        }
    }

private:

    Texture *mSkyCube = nullptr;
    Texture *mCryoCube = nullptr;
    Material *mSkyMaterial = nullptr;
    Material *mReflectionMaterial = nullptr;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::SampleReflectionSky sample;
    return sample.start("samples/config/config.json");
}
