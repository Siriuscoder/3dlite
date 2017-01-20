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
#include <ctime>
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
    mStatTexture(NULL),
    mStatTimer(NULL),
    mCenterXPos(0),
    mCenterYPos(0)
{
    mMain.addObserver(this);
}

void Sample::initGui()
{
    /* preload font texture */
    mStatTexture = mMain.getResourceManager()->
        queryResource<lite3dpp_font::FontTexture>("arial256x128.texture",
        "samples:textures/json/arial256x128.json");
    
    Scene *gui = mMain.getResourceManager()->queryResource<Scene>("GUI",
        "samples:scenes/gui.json");
    gui->addObserver(this);
    
    mGuiCamera = gui->getCamera("GuiCamera");
    //mGuiCamera->cullBackFaces(false);
    setGuiSize(mMainWindow->width(), mMainWindow->height());
    
    mStatTimer = mMain.addTimer("StatTimer", 500);
}

void Sample::init()
{
    mMainWindow = mMain.window();
    initGui();
    createScene();
    updateGuiStats();
    
    adjustMainCamera(mMainWindow->width(), mMainWindow->height());
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
    else if(timerid == mStatTimer)
        updateGuiStats();
}

void Sample::processEvent(SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN)
    {
        /* exit */
        if (e->key.keysym.sym == SDLK_ESCAPE)
            mMain.stop();
        else if (e->key.keysym.sym == SDLK_1)
        {
            printRenderStats();
        }
        else if (e->key.keysym.sym == SDLK_2)
        {
            printMemoryStats();
        }
        else if (e->key.keysym.sym == SDLK_3)
        {
            saveScreenshot();
        }
        else if (mMainWindow && mMainCamera && e->key.keysym.sym == SDLK_f)
        {
            static bool scRes = false;
            if(scRes)
            {
                resizeMainWindow(1024, 768);
                mMainWindow->fullscreen(false);
            }
            else
            {
                resizeMainWindow(0, 0);
                mMainWindow->fullscreen(true);
            }

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

void Sample::setGuiSize(int32_t width, int32_t height)
{
    SDL_assert(mGuiCamera);
    mGuiCamera->setupOrtho(0, 20, 0, mMainWindow->width(),
        0, mMainWindow->height());
    
    kmVec3 cameraPos = { 0, 0, 10 };
    mGuiCamera->setPosition(cameraPos);
    mGuiCamera->lookAt(KM_VEC3_ZERO);
    
    SceneObject *sOverlay = mGuiCamera->getScene().getObject("StatOverlay");
    kmVec3 sOverlayPos = { float(mMainWindow->width()-270), float(mMainWindow->height())-14, 0 };
    sOverlay->getRoot()->setPosition(sOverlayPos);
}

void Sample::adjustMainCamera(int32_t width, int32_t height)
{
    SDL_assert(mMainCamera);
    mCenterXPos = mMainWindow->width() >> 1;
    mCenterYPos = mMainWindow->height() >> 1;
    lite3d_video_set_mouse_pos(mCenterXPos, mCenterYPos);
    mMainCamera->setAspect(mMainWindow->computeCameraAspect());
}

void Sample::resizeMainWindow(int32_t width, int32_t height)
{
    SDL_assert(mMainWindow);

    mMainWindow->resize(width, height);
    setGuiSize(mMainWindow->width(), mMainWindow->height());
    adjustMainCamera(mMainWindow->width(), mMainWindow->height());
}

void Sample::printRenderStats()
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
        stats->nodesTotal, stats->batchesTotal, stats->batchedByFrame, stats->trianglesByFrame);
}

void Sample::printMemoryStats()
{
    ResourceManager::ResourceManagerStats memStats = mMain.getResourceManager()->getStats();
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "==== Memory statistics ========\n"
        "Video memory: %lu bytes\n"
        "Total objects: %d\n"
        "Textures: %d/%d\n"
        "Materials: %d/%d\n"
        "Scenes: %d/%d\n"
        "Scripts: %d/%d\n"
        "Meshes: %d/%d\n"
        "Shaders: %d/%d\n"
        "Render targets: %d/%d\n"
        "File cache: %d/%lu bytes\n",
        memStats.usedVideoMem,
        memStats.totalObjectsCount,
        memStats.texturesLoadedCount, memStats.texturesCount,
        memStats.materialsLoadedCount, memStats.materialsCount,
        memStats.scenesLoadedCount, memStats.scenesCount,
        memStats.scriptsLoadedCount, memStats.scriptsCount,
        memStats.meshesLoadedCount, memStats.meshesCount,
        memStats.shaderProgramsLoadedCount, memStats.shaderProgramsCount,
        memStats.renderTargetsLoadedCount, memStats.renderTargetsCount,
        memStats.fileCachesCount, memStats.totalCachedFilesMemSize);
}

void Sample::updateGuiStats()
{
    SDL_assert(mStatTexture);
    const lite3d_render_stats *stats = mMain.getRenderStats();
    
    char strbuf[150];
    kmVec2 textPos = {15, 20};
    kmVec4 textColor = {0.3f, 0.7f, 0.8f, 1.0f};

    sprintf(strbuf, "FPS: %d\nFrame time: %f\nBatches: %d/%d\nFaces: %d",
        stats->lastFPS, stats->lastFrameMs, stats->batchedByFrame, 
        stats->batchesTotal, stats->trianglesByFrame);
    
    mStatTexture->clean();
    mStatTexture->drawText(strbuf, textPos, textColor);
    mStatTexture->uploadChanges();
}

void Sample::saveScreenshot()
{
    time_t rawtime;
    struct tm * timeinfo;
    char filename[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(filename, sizeof(filename), "screen-%F-%H-%M-%S.png", timeinfo);
    mMainWindow->saveScreenshot(filename);
}

WindowRenderTarget &Sample::getMainWindow()
{
    SDL_assert(mMainWindow);
    return *mMainWindow;
}

Camera &Sample::getMainCamera()
{
    SDL_assert(mMainCamera);
    return *mMainCamera;    
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
