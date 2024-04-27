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

namespace lite3dpp {
namespace samples {

Sample::Sample()
{
    mMain.addObserver(this);
}

void Sample::initGui()
{
    /* preload font textures */
    mStatTexture = mMain.getResourceManager()->
        queryResource<lite3dpp_font::FontTexture>("arial256x128.texture",
        "samples:textures/json/arial256x128.json");
    mHelpTexture = mMain.getResourceManager()->
        queryResource<lite3dpp_font::FontTexture>("arial512x512.texture",
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

void Sample::fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard)
{}

void Sample::timerTick(lite3d_timer *timerid)
{
    if (timerid == mMain.getFixedUpdateTimer())
    {
        // Считаем запаздывание таймера как отношение фактического времени к интервалу
        float deltaRetard = static_cast<float>(static_cast<double>(timerid->deltaMcs) / (timerid->interval * 1000.0));
        // Обработка плавного движения камеры
        const Uint8 *kstate = SDL_GetKeyboardState(NULL);
        kmVec2 accel = KM_VEC2_ZERO;

        if (kstate[SDL_SCANCODE_W])
            accel.x = -mCameraAccel * deltaRetard;
        if (kstate[SDL_SCANCODE_S])
            accel.x = mCameraAccel * deltaRetard;
        if (kstate[SDL_SCANCODE_A])
            accel.y = -mCameraAccel * deltaRetard;
        if (kstate[SDL_SCANCODE_D])
            accel.y = mCameraAccel * deltaRetard;

        kmVec2Add(&mCameraVelocityVector, &mCameraVelocityVector, &accel);
        mCameraVelocityVector.x = std::max(-mCameraVelocityMax, std::min(mCameraVelocityVector.x, mCameraVelocityMax));
        mCameraVelocityVector.y = std::max(-mCameraVelocityMax, std::min(mCameraVelocityVector.y, mCameraVelocityMax));

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
        if (e->motion.x != mWCenter.x || e->motion.y != mWCenter.y)
        {
            if (!mCameraAngles)
            {
                mCameraAngles = kmVec2 {
                    -mMainCamera->getZW(),
                    -mMainCamera->getXW()
                };

                lite3d_video_set_mouse_pos(mWCenter.x, mWCenter.y);
                return;
            }

            mCameraAngles->x += (e->motion.x - mWCenter.x) * mCameraSensitivity;
            mCameraAngles->y += (e->motion.y - mWCenter.y) * mCameraSensitivity;

            // angles restrictions
            mCameraAngles->y = std::max((float)-M_PI, std::min(mCameraAngles->y, 0.0f));
            if (mCameraAngles->x >= M_PI * 2 || mCameraAngles->x <= -M_PI * 2)
                mCameraAngles->x = 0;

            mMainCamera->setOrientationAngles(-mCameraAngles->x, -mCameraAngles->y);
            lite3d_video_set_mouse_pos(mWCenter.x, mWCenter.y);
            mainCameraChanged();
        }
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
    mGuiCamera->lookAtLocal(KM_VEC3_ZERO);
    
    SceneObject *sOverlay = mGuiScene->getObject("StatOverlay");
    sOverlay->setPosition(kmVec3 { mMainWindow->width()-270.0f, mMainWindow->height()-14.0f, 0.0f });

    sOverlay = mGuiScene->getObject("HelpOverlay");
    sOverlay->setPosition(kmVec3 { 14.0f, mMainWindow->height()-14.0f, 0.0f });
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
        "nodes total\tbatches total\tbatches called\tbatches instanced\tbatches occluded\tfaces\n"
        "%d\t\t%d\t\t%d\t\t%d\t\t\t%d\t\t\t%d\n",
        stats->framesCount, stats->lastFPS, stats->avrFPS, stats->bestFPS, stats->worstFPS,
        stats->lastFrameMs, stats->avrFrameMs, stats->bestFrameMs, stats->worstFrameMs,
        stats->nodesTotal, stats->batchTotal, stats->batchCalled, stats->batchInstancedCalled,
        stats->batchOccluded, stats->trianglesByFrame);
}

void Sample::printMemoryStats()
{
    ResourceManager::ResourceManagerStats memStats = mMain.getResourceManager()->getStats();
    lite3d_render_stats *renderStats = lite3d_render_stats_get();

    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
        "\n==== Memory statistics ========\n"
        "Video memory:\t%d kB\n"
        "Total objects:\t%d\n"
        "Textures:\t%d/%d\n"
        "Materials:\t%d/%d\n"
        "Scenes:\t\t%d/%d\n"
        "Meshes:\t\t%d/%d\n"
        "Shaders:\t%d/%d\n"
        "Render targets:\t%d/%d\n"
        "SSBO:\t\t%d/%d\n"
        "UBO:\t\t%d/%d\n"
        "VBO:\t\t%d\n"
        "IBO:\t\t%d\n"
        "VAO:\t\t%d\n"
        "QUERIES:\t%d\n"
        "File cache:\t%d kB in %d files\n",
        static_cast<uint32_t>(memStats.usedVideoMem / 1024),
        memStats.totalObjectsCount,
        memStats.texturesLoadedCount, memStats.texturesCount,
        memStats.materialsLoadedCount, memStats.materialsCount,
        memStats.scenesLoadedCount, memStats.scenesCount,
        memStats.meshesLoadedCount, memStats.meshesCount,
        memStats.shaderProgramsLoadedCount, memStats.shaderProgramsCount,
        memStats.renderTargetsLoadedCount, memStats.renderTargetsCount,
        memStats.ssboLoadedCount, memStats.ssboCount,
        memStats.uboLoadedCount, memStats.uboCount,
        renderStats->vboCount, renderStats->iboCount, renderStats->vaoCount, renderStats->queryCount,
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

int Sample::start(const std::string_view &config)
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
    auto timeNow = std::chrono::steady_clock::now();
    float elapsedMilliSec = std::chrono::duration_cast<std::chrono::microseconds>(timeNow - mLastFrameTime).count() / 1000.0f;
    mLastFrameTime = timeNow;

    // Отношение прошедшего времени с прошлого кадра к интервалу таймера
    float deltaRetard = elapsedMilliSec / mMain.getFixedUpdateTimer()->interval;

    if (mCameraVelocityVector.x > 0)
        mCameraVelocityVector.x -= std::min(mCameraAccelResistance * deltaRetard, mCameraVelocityVector.x);
    if (mCameraVelocityVector.x < 0)
        mCameraVelocityVector.x += std::min(mCameraAccelResistance * deltaRetard, std::abs(mCameraVelocityVector.x));
    if (mCameraVelocityVector.y > 0)
        mCameraVelocityVector.y -= std::min(mCameraAccelResistance * deltaRetard, mCameraVelocityVector.y);
    if (mCameraVelocityVector.y < 0)
        mCameraVelocityVector.y += std::min(mCameraAccelResistance * deltaRetard, std::abs(mCameraVelocityVector.y));
    
    if (near(mCameraVelocityVector.x, 0.0f) && near(mCameraVelocityVector.y, 0.0f))
    {
        return;
    }

    updateCameraVelocity(kmVec3 { mCameraVelocityVector.y * deltaRetard, 0.0f, mCameraVelocityVector.x * deltaRetard});
    mainCameraChanged();
}

void Sample::frameEnd()
{
    moveCamera();
}

void Sample::updateCameraVelocity(const kmVec3& velocity)
{
    mMainCamera->moveRelative(velocity);
}

}}
