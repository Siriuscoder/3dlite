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
    
#pragma pack(push, 1)
typedef struct vertexPod
{
    float x,y,z,s,t;
} vertexPod;
#pragma pack(pop)

class DynamicCude : public Sample
{
public:
    
    DynamicCude() : 
        mBox(NULL),
        mWireftameView(false),
        mDepthTest(true),
        mKof(1)
    {}

    void createScene() override
    {
        Scene *scene = getMain().getResourceManager()->queryResource<Scene>("BoxScene",
            "samples:scenes/scene_rtt_box.json");

        mBoxTexture = getMain().getResourceManager()->queryResource<TextureImage>("color512x512.texture");
        setMainCamera(scene->getCamera("MyCamera"));
        mBox = scene->getObject("Box");
        mBoxMesh = getMain().getResourceManager()->queryResource<Mesh>("box.mesh");

        updateTextureData();
    }
    
    void timerTick(lite3d_timer *timerid) override
    {
        Sample::timerTick(timerid);
        if(timerid == getMain().getFixedUpdateTimer())
        {
            mBox->getRoot()->rotateAngle(KM_VEC3_NEG_Z, 0.01f);
        }
    }

    void processEvent(SDL_Event *e) override
    {
        Sample::processEvent(e);
        if (e->type == SDL_KEYDOWN)
        {
            if (e->key.keysym.sym == SDLK_v)
            {
                mWireftameView = !mWireftameView;
                getMainCamera().showWireframe(mWireftameView);
                getMainCamera().cullBackFaces(!mWireftameView);
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
        lite3dpp::BufferScopedMapper vmap = mBoxMesh->vertexBuffer().map(LITE3D_VBO_MAP_READ_WRITE);
        
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
    
    SceneObject *mBox;
    TextureImage *mBoxTexture;
    Mesh *mBoxMesh;

    bool mWireftameView;
    bool mDepthTest;
    int8_t mKof;    
};

}}

int main(int agrc, char *args[])
{
    lite3dpp::samples::DynamicCude sample;
    return sample.start("samples/config/config.json");
}


