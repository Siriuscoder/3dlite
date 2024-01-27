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
#include <ctime>
#include <iostream>
#include <algorithm>
#include <SDL_log.h>
#include <SDL_assert.h>

#include "lite3dpp_common.h"

#define VELOCITY_MAX    200.0f
#define ACCEL           40.0f
#define ACCEL_RESIST    20.0f

namespace lite3dpp {
namespace samples {

Sample::Sample() : 
    mMainCamera(nullptr),
    mGuiCamera(nullptr),
    mGuiScene(nullptr),
    mMainWindow(nullptr),
    mStatTexture(nullptr),
    mStatTimer(nullptr),
    mWCenter(KM_VEC2_ZERO),
    mCamAngles(KM_VEC2_ZERO),
    mSensitivity(0.001f),
    mVelocity(KM_VEC2_ZERO),
    mAccel(KM_VEC2_ZERO)
{
    mMain.addObserver(this);
}

void Sample::initGui()
{
    /* preload font texture */
    mStatTexture = mMain.getResourceManager()->
        queryResource<lite3dpp_font::FontTexture>("arial256x128.texture",
        "samples:textures/json/arial256x128.json");
    
    mGuiScene = mMain.getResourceManager()->queryResource<Scene>("GUI",
        "samples:scenes/gui.json");
    
    mGuiCamera = getMain().getCamera("GuiCamera");
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
    mMain.showSystemCursor(false);
}

void Sample::frameBegin()
{
    moveCamera();
}

void Sample::fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard)
{}

void Sample::timerTick(lite3d_timer *timerid)
{
    if (timerid == mMain.getFixedUpdateTimer())
    {
        const Uint8 *kstate = SDL_GetKeyboardState(NULL);
        if (kstate[SDL_SCANCODE_W])
            mAccel.x = -ACCEL;
        if (kstate[SDL_SCANCODE_S])
            mAccel.x = ACCEL;
        if (kstate[SDL_SCANCODE_A])
            mAccel.y = -ACCEL;
        if (kstate[SDL_SCANCODE_D])
            mAccel.y = ACCEL;
        
        if (!kstate[SDL_SCANCODE_W] && !kstate[SDL_SCANCODE_S])
            mAccel.x = 0.0f;
        if (!kstate[SDL_SCANCODE_A] && !kstate[SDL_SCANCODE_D])
            mAccel.y = 0.0f;
        
        kmVec2Add(&mVelocity, &mVelocity, &mAccel);
        if (mVelocity.x > 0)
            mVelocity.x -= ACCEL_RESIST;
        if (mVelocity.x < 0)
            mVelocity.x += ACCEL_RESIST;
        if (mVelocity.y > 0)
            mVelocity.y -= ACCEL_RESIST;
        if (mVelocity.y < 0)
            mVelocity.y += ACCEL_RESIST;
        
        mVelocity.x = std::max(-VELOCITY_MAX, std::min(mVelocity.x, VELOCITY_MAX));
        mVelocity.y = std::max(-VELOCITY_MAX, std::min(mVelocity.y, VELOCITY_MAX));

        // Считаем запаздывание таймера как отношение фактического времени к интервалу
        float deltaRetard = static_cast<float>(static_cast<double>(timerid->deltaMcs) / (timerid->interval * 1000.0));
        fixedUpdateTimerTick(timerid->firedPerRound, timerid->deltaMcs, deltaRetard);
    }
    else if (timerid == mStatTimer)
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
        else if (e->key.keysym.sym == SDLK_4)
        {
            static bool showStats = true;
            showStats = !showStats;
            showStats ? mGuiCamera->enable() : mGuiCamera->disable();
        }
        else if (mMainWindow && mMainCamera && e->key.keysym.sym == SDLK_f)
        {
            static bool scRes = false;
            if(scRes)
            {
                resizeMainWindow(lite3d_get_global_settings()->videoSettings.screenWidth, 
                    lite3d_get_global_settings()->videoSettings.screenHeight);
                mMainWindow->fullscreen(false);
            }
            else
            {
                resizeMainWindow(0, 0);
                mMainWindow->fullscreen(true);
            }

            scRes = !scRes;
            mainCameraChanged();
        }
    }
    else if(mMainCamera && e->type == SDL_MOUSEMOTION)
    {
        mCamAngles.x += (e->motion.x - mWCenter.x) * mSensitivity;
        mCamAngles.y += (e->motion.y - mWCenter.y) * mSensitivity;
        
        // angles restrictions
        mCamAngles.y = std::max((float)-M_PI, std::min(mCamAngles.y, 0.0f));
        if (mCamAngles.x > M_PI * 2)
            mCamAngles.x = 0;
        
        kmQuaternion camZQuat, camPQuat, sumQuat;
        kmQuaternionRotationAxisAngle(&camZQuat, &KM_VEC3_POS_Z, mCamAngles.x);
        kmQuaternionRotationAxisAngle(&camPQuat, &KM_VEC3_POS_X, mCamAngles.y);
        kmQuaternionMultiply(&sumQuat, &camPQuat, &camZQuat);

        //mMainCamera->rotateZ(e->motion.xrel * 0.003f);
        //mMainCamera->pitch(e->motion.yrel * 0.003f);
        lite3d_video_set_mouse_pos(mWCenter.x, mWCenter.y);
        mMainCamera->setRotation(sumQuat);
        mainCameraChanged();
    }
}

void Sample::setGuiSize(int32_t width, int32_t height)
{
    SDL_assert(mGuiCamera);
    SDL_assert(mGuiScene);

    mGuiCamera->setupOrtho(0, 20, 0, mMainWindow->width(),
        0, mMainWindow->height());
    
    kmVec3 cameraPos = { 0, 0, 10 };
    mGuiCamera->setPosition(cameraPos);
    mGuiCamera->lookAt(KM_VEC3_ZERO);
    
    SceneObject *sOverlay = mGuiScene->getObject("StatOverlay");
    kmVec3 sOverlayPos = { float(mMainWindow->width()-270), float(mMainWindow->height())-14, 0 };
    sOverlay->getRoot()->setPosition(sOverlayPos);
}

void Sample::adjustMainCamera(int32_t width, int32_t height)
{
    SDL_assert(mMainCamera);
    mWCenter.x = width >> 1;
    mWCenter.y = height >> 1;
    lite3d_video_set_mouse_pos(mWCenter.x, mWCenter.y);
    //lite3d_video_relative_mouse_mode(LITE3D_TRUE);
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
        "\n==== Render statistics ========\n"
        "Fames: %ld\n"
        "last FPS\tavr FPS\t\tbest FPS\tworst FPS\n"
        "%d\t\t%d\t\t%d\t\t%d\n"
        "last frame ms\tavr frame ms\tbest frame ms\tworst frame ms\n"
        "%f\t%f\t%f\t%f\n"
        "nodes total\tbatch total\tbatch called\tbatch instanced\tfaces\n"
        "%d\t\t%d\t\t%d\t\t%d\t\t\t%d\n",
        stats->framesCount, stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
        stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs,
        stats->nodesTotal, stats->batchTotal, stats->batchCalled, stats->batchInstancedCalled,
        stats->trianglesByFrame);
}

void Sample::printMemoryStats()
{
    ResourceManager::ResourceManagerStats memStats = mMain.getResourceManager()->getStats();
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "\n==== Memory statistics ========\n"
        "Video memory:\t%d kB\n"
        "Total objects:\t%d\n"
        "Textures:\t%d/%d\n"
        "Materials:\t%d/%d\n"
        "Scenes:\t\t%d/%d\n"
        "Scripts:\t%d/%d\n"
        "Meshes:\t\t%d/%d\n"
        "Shaders:\t%d/%d\n"
        "Render targets:\t%d/%d\n"
        "SSBO:\t\t%d/%d\n"
        "File cache:\t%d kB in %d files\n",
        static_cast<uint32_t>(memStats.usedVideoMem / 1024),
        memStats.totalObjectsCount,
        memStats.texturesLoadedCount, memStats.texturesCount,
        memStats.materialsLoadedCount, memStats.materialsCount,
        memStats.scenesLoadedCount, memStats.scenesCount,
        memStats.scriptsLoadedCount, memStats.scriptsCount,
        memStats.meshesLoadedCount, memStats.meshesCount,
        memStats.shaderProgramsLoadedCount, memStats.shaderProgramsCount,
        memStats.renderTargetsLoadedCount, memStats.renderTargetsCount,
        memStats.ssboLoadedCount, memStats.ssboCount,
        static_cast<uint32_t>(memStats.totalCachedFilesMemSize / 1024), memStats.fileCachesCount);
}

void Sample::updateGuiStats()
{
    SDL_assert(mStatTexture);
    const lite3d_render_stats *stats = mMain.getRenderStats();
    
    char strbuf[150];
    kmVec2 textPos = {24, 25};
    kmVec4 textColor = {0.3f, 0.7f, 0.8f, 1.0f};

    sprintf(strbuf, "FPS: %d\nFrame time: %.2f ms\nBatching: %d/%d\nFaces: %d",
        stats->lastFPS, stats->lastFrameMs, stats->batchCalled, 
        stats->batchTotal, stats->trianglesByFrame);
    
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
    strftime(filename, sizeof(filename), "screen-%Y-%m-%d-%H-%M-%S.png", timeinfo);
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

void Sample::mainCameraChanged()
{}

void Sample::moveCamera()
{    
    if (mVelocity.x != 0.0f || mVelocity.y != 0.0f)
    {
        lite3d_render_stats *stats = lite3d_render_stats_get();    
        kmVec3 step;
        
        step.x = mVelocity.y * (stats->lastFrameMs / 1000);
        step.y = 0.0f;
        step.z = mVelocity.x * (stats->lastFrameMs / 1000);
        mMainCamera->moveRelative(step);
        mainCameraChanged();
    }
}

}}
