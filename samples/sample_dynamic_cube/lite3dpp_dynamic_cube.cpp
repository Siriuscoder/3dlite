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
#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {
    
#pragma pack(push, 1)
typedef struct vertexPod
{
    float x,y,z,s,t;
} vertexPod;
#pragma pack(pop)

static const char *helpString = 
    "Press 'v' to set wireframe view\n"
    "Press 'r' to reload box texture pixels\n"
    "Press 'a' to deform box (hold ctrl to reverse)\n";

class DynamicCude : public Sample
{
public:

    DynamicCude() : 
        Sample(helpString)
    {}

    void createScene() override
    {
        Scene *scene = getMain().getResourceManager()->queryResource<Scene>("BoxScene",
            "samples:scenes/scene_rtt_box.json");

        mBoxTexture = getMain().getResourceManager()->queryResource<TextureImage>("color512x512.texture");
        setMainCamera(getMain().getCamera("MyCamera"));
        mBox = scene->getObject("Box");
        mBoxMesh = getMain().getResourceManager()->queryResource<Mesh>("box.mesh");

        updateTextureData();
    }
    
    void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard) override
    {
        mBox->rotateAngle(KM_VEC3_NEG_Z, 0.01f * deltaRetard);
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_v)
            {
                mWireframeView = !mWireframeView;
                getMainCamera().setPolygonMode(mWireframeView ? Camera::PolygonLine : Camera::PolygonFill);
                getMainCamera().setCullFaceMode(mWireframeView ? Camera::CullFaceNever : Camera::CullFaceBack);
            }
            else if (e->key.keysym.sym == SDLK_r)
            {
                updateTextureData();
            }
            else if (e->key.keysym.sym == SDLK_a)
            {
                if (e->key.keysym.mod & KMOD_LCTRL)
                    mKof = -1;
                else
                    mKof = 1;
            
                updateMesh();
            }
        }
    }
    
    void updateTextureData()
    {
        lite3dpp::PixelsData pixels;
        for(int8_t i = 0; i <= mBoxTexture->getLevelsNum(); ++i)
        {
            mBoxTexture->getPixels(i, pixels);

            /* random color */
            unsigned char r = (unsigned char)(rand() % 255);
            unsigned char g = (unsigned char)(rand() % 255);
            unsigned char b = (unsigned char)(rand() % 255);

            /* modify pixels */
            for(unsigned j = 0; j < pixels.size();)
            {
                pixels[j++] = r;
                pixels[j++] = g;
                pixels[j++] = b;
                pixels[j++] = 255;
            }

            mBoxTexture->setPixels(i, pixels);
        }
    }

    void updateMesh()
    {
        vertexPod *vertices;
        lite3dpp::BufferScopedMapper vmap = mBoxMesh->vertexBuffer().map(BufferScopedMapper::LockTypeReadWrite);
        
        vertices = vmap.getPtr<vertexPod>();
        for(uint32_t i = 0; i < vmap.getSize(); i += sizeof(vertexPod), ++vertices)
        {
            if(vertices->z >= 0.0f)
            {
                vertices->z += mKof * 0.1f;
                if(vertices->z < 0.0f)
                    vertices->z = 0.0f;
            }
        }
    }
    
private:
    
    SceneObject *mBox = nullptr;
    TextureImage *mBoxTexture = nullptr;
    Mesh *mBoxMesh = nullptr;

    bool mWireframeView = false;
    int8_t mKof = 1;
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::DynamicCude sample;
    return sample.start("samples/config/config.json");
}


