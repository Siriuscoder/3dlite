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
#include <SDL_assert.h>

#include <sample_common/lite3dpp_common.h>

namespace lite3dpp {
namespace samples {

Sample::Sample() : 
    mMainCamera(NULL),
    mGuiCamera(NULL),
    mMainWindow(NULL),
    mCenterXPos(0),
    mCenterYPos(0)
{
    mMain.addObserver(this);
}

void Sample::initGui()
{
    lite3dpp::Scene *gui = mMain.getResourceManager()->queryResource<Scene>("GUI",
        "samples:scenes/gui.json");
    gui->addObserver(this);
    
    mGuiCamera = gui->getCamera("GuiCamera");
    //mGuiCamera->cullBackFaces(false);
    
    mGuiCamera->setupOrtho(0, 20, 0, mMainWindow->width(),
        0, mMainWindow->height());
    
    kmVec3 cameraPos = { 0, 0, 10 };
    mGuiCamera->setPosition(cameraPos);
    mGuiCamera->lookAt(KM_VEC3_ZERO);
    
    SceneObject *sOverlay = gui->getObject("StatOverlay");
    kmVec3 sOverlayPos = { mMainWindow->width()-270, mMainWindow->height()-14, 0 };
    sOverlay->getRoot()->setPosition(sOverlayPos);
}

void Sample::init()
{
    mMainWindow = mMain.window();
    initGui();
    createScene();
    
    mCenterXPos = mMainWindow->width() >> 1;
    mCenterYPos = mMainWindow->height() >> 1;
    lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);
    mMain.getResourceManager()->releaseFileCache();
}

void Sample::beginSceneRender(Scene *scene, Camera *camera) 
{
    if(mMainWindow && scene->getName() == "GUI")
    {
        /* clean depth berore gui render should begin */
        mMainWindow->clear(false, true, false);
    }
}

void Sample::timerTick(lite3d_timer *timerid)
{
    if(mMainCamera && timerid == mMain.getFixedUpdateTimer())
    {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_W])
        {
            kmVec3 vec3 = {0, 0, -6};
            mMainCamera->moveRelative(vec3);
        }
        
        if(state[SDL_SCANCODE_S])
        {
            kmVec3 vec3 = {0, 0, 6};
            mMainCamera->moveRelative(vec3);
        }
        
        if(state[SDL_SCANCODE_A])
        {
            kmVec3 vec3 = {-6, 0, 0};
            mMainCamera->moveRelative(vec3);
        }
        
        if(state[SDL_SCANCODE_D])
        {
            kmVec3 vec3 = {6, 0, 0};
            mMainCamera->moveRelative(vec3);
        }
    }
}

void Sample::processEvent(SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN)
    {
        /* exit */
        if (e->key.keysym.sym == SDLK_ESCAPE)
            mMain.stop();
        else if (e->key.keysym.sym == SDLK_F1)
        {
            printStats();
        }
        else if (mMainWindow && mMainCamera && e->key.keysym.sym == SDLK_f)
        {
            static bool scRes = true;
            if(scRes)
            {
                mMainWindow->resize(1024, 768);
                mMainWindow->fullscreen(false);
            }
            else
            {
                mMainWindow->resize(0, 0);
                mMainWindow->fullscreen(true);
            }

            mCenterXPos = mMainWindow->width() >> 1;
            mCenterYPos = mMainWindow->height() >> 1;
            lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);
            mMainCamera->setAspect(mMainWindow->computeCameraAspect());
            scRes = !scRes;
        }
    }
    else if(mMainCamera && e->type == SDL_MOUSEMOTION)
    {
        mMainCamera->rotateZ((e->motion.x - mCenterXPos) * 0.003f);
        mMainCamera->pitch((e->motion.y - mCenterYPos) * 0.003f);
        lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);
    }
}

void Sample::printStats()
{
    lite3d_render_stats *stats = lite3d_render_stats_get();
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "==== Render statistics ========\n"
        "last FPS\tavr FPS\t\tbest FPS\tworst FPS\n"
        "%d\t\t%d\t\t%d\t\t%d\n"
        "last frame ms\tavr frame ms\tbest frame ms\tworst frame ms\n"
        "%f\t%f\t%f\t%f\n"
        "nodes total\tbatches total\tbatches called\tfaces\n"
        "%d\t\t%d\t\t%d\t\t%d\n",
        stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
        stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs,
        stats->nodesTotal, stats->batchesTotal, stats->batchedByFrame, stats->verticesByFrame);
}

WindowRenderTarget &Sample::getMainWindow()
{
    SDL_assert(mMainWindow);
    return *mMainWindow;
}


int Sample::start(const char *config)
{
    try
    {
        mMain.initFromConfig(config);
        mMain.run();
    }
    catch (std::exception &ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}

}}
