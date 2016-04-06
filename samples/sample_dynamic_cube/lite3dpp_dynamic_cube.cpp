/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <SDL_log.h>

#include <lite3dpp/lite3dpp_main.h>

#pragma pack(push, 1)
typedef struct vertexPod
{
    float x,y,z,s,t;
} vertexPod;
#pragma pack(pop)

class SampleLifecycleListener : public lite3dpp::Main::LifecycleListener
{
public:

    SampleLifecycleListener(lite3dpp::Main *main) :
        mMain(main),
        mBox(NULL),
        mWireftameView(false),
        mDepthTest(true),
        mKof(1)
    {}

    void init() override
    {
        lite3dpp::Scene *scene = mMain->getResourceManager()->queryResource<lite3dpp::Scene>("BoxScene",
            "samples:scenes/scene_rtt_box.json");

        mBoxTexture = mMain->getResourceManager()->queryResource<lite3dpp::Texture>("color512x512.texture");
        mCamera = scene->getCamera("MyCamera");
        mBox = scene->getObject("Box");
        mBoxMesh = mMain->getResourceManager()->queryResource<lite3dpp::Mesh>("box.mesh");

        updateTextureData();
    }

    void shut() override
    {}

    void frameBegin() override
    {}

    void frameEnd() override
    {}

    void timerTick(lite3d_timer *timerid) override
    {
        mBox->getRoot()->rotateAngle(KM_VEC3_NEG_Z, 0.01f);
    }

    void processEvent(SDL_Event *e) override
    {
        if (e->type == SDL_KEYDOWN)
        {
            /* exit */
            if (e->key.keysym.sym == SDLK_ESCAPE)
                mMain->stop();
        }
        else if (e->key.keysym.sym == SDLK_F1)
        {
            lite3d_render_stats *stats = lite3d_render_stats_get();
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "==== Render statistics ========\n"
                "last FPS\tavr FPS\t\tbest FPS\tworst FPS\n"
                "%d\t\t%d\t\t%d\t\t%d\n"
                "last frame ms\tavr frame ms\tbest frame ms\tworst frame ms\n"
                "%f\t%f\t%f\t%f",
                stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
                stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs);
        }
        else if (e->key.keysym.sym == SDLK_v)
        {
            mWireftameView = !mWireftameView;
            mCamera->showWireframe(mWireftameView);
            mCamera->cullBackFaces(!mWireftameView);
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

    void updateTextureData()
    {
        for(int8_t i = 0; i <= mBoxTexture->getLevelsNum(); ++i)
        {
            lite3dpp::Texture::PixelsData pixels;
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
        lite3dpp::BufferMapper vmap = mBoxMesh->mapVertexBuffer(LITE3D_VBO_MAP_READ_WRITE);
        
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

    lite3dpp::Main *mMain;
    lite3dpp::SceneObject *mBox;
    lite3dpp::Texture *mBoxTexture;
    lite3dpp::Camera *mCamera;
    lite3dpp::Mesh *mBoxMesh;

    bool mWireftameView;
    bool mDepthTest;
    int8_t mKof;
};

int main(int agrc, char *args[])
{
    try
    {
        lite3dpp::Main mainObj;
        SampleLifecycleListener lifecycleListener(&mainObj);

        mainObj.registerLifecycleListener(&lifecycleListener);
        mainObj.initFromConfig("samples/config/config.json");
        mainObj.run();
    }
    catch (std::exception &ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
