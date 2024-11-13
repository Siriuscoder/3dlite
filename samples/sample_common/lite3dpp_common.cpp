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

static const char *commonHelpString = 
    "============= LITE3D v%s ===================\n"
    "Help:\n"
    "Press '1' to show/hide this menu\n"
    "Press '2' to go to render and memory stats\n"
    "Press '3' to show/hide stats\n"
    "Press '4' to make screenshot\n"
    "Press 'f' to fullscreen\n"
    "Press 'w','s','a','d' to move\n"
    "%s";

static const char *resourceStatsString = 
    "============= Resource statistics =============\n"
    "Estimated video memory:         %d kB\n"
    "Total resources in cache:       %d\n"
    "Pipelines:                      %d\n"
    "Textures:                       %d/%d\n"
    "Materials:                      %d/%d\n"
    "Scenes:                         %d/%d\n"
    "Meshes:                         %d/%d\n"
    "Mesh partitions:                %d/%d\n"
    "Shaders:                        %d/%d\n"
    "Render targets:                 %d/%d\n"
    "SSBO:                           %d/%d\n"
    "UBO:                            %d/%d\n"
    "VBO:                            %d\n"
    "IBO:                            %d\n"
    "VAO:                            %d\n"
    "QUERIES:                        %d\n\n"
    "File cache: %d kB in %d files\n";

static const char *renderStatsString = 
    "============= Render statistics ===============\n"
    "Frames:                         %ld\n"
    "Last FPS:                       %d\n"
    "Average FPS:                    %d\n"
    "Best FPS:                       %d\n"
    "Worst FPS:                      %d\n"
    "Last frame time:                %f ms\n"
    "Average frame time:             %f ms\n"
    "Best frame time:                %f ms\n"
    "Worst frame time:               %f ms\n"
    "Total Nodes:                    %d\n"
    "Batches total:                  %d\n"
    "Batches draw called:            %d\n"
    "Batches draw instanced:         %d\n"
    "Batches draw occluded:          %d\n"
    "Draw faces:                     %d\n";

Sample::Sample(const std::string_view &sampleHelpString) : 
   mSampleHelpString(sampleHelpString)
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
        "samples:textures/json/arial512x512.json");
    
    mGuiScene = mMain.getResourceManager()->queryResource<Scene>("GUI",
        "samples:scenes/gui.json");
    
    mGuiCamera = getMain().getCamera("GuiCamera");
    mStatOverlay = mGuiScene->getObject("StatOverlay");
    mHelpOverlay = mGuiScene->getObject("HelpOverlay");
    setGuiSize(mMainWindow->width(), mMainWindow->height());
    
    mStatTimer = mMain.addTimer("StatTimer", 500);
}

void Sample::init()
{
    mMainWindow = mMain.window();
    initGui();
    createScene();
    updateGui();
    
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
        updateGui();
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
            SDL_assert(mHelpOverlay);
            mHelpOverlay->isEnabled() ? mHelpOverlay->disable() : mHelpOverlay->enable();
        }
        else if (e->key.keysym.sym == SDLK_2)
        {
            mHelpState = mHelpState == SHOW_HELP ? SHOW_RENDER : 
                (mHelpState == SHOW_RENDER ? SHOW_RESOURCES : SHOW_HELP);
            updateGui();
        }
        else if (e->key.keysym.sym == SDLK_3)
        {
            SDL_assert(mGuiCamera);
            mGuiCamera->isEnabled() ? mGuiCamera->disable() : mGuiCamera->enable();
        }
        else if (e->key.keysym.sym == SDLK_4)
        {
            saveScreenshot();
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

void Sample::updateGui()
{
    SDL_assert(mStatTexture);
    SDL_assert(mHelpTexture);
    const lite3d_render_stats *renderStats = mMain.getRenderStats();
    
    char strbuf[1024];
    const kmVec2 textPos = {24, 25};
    
    if (mStatOverlay->isEnabled())
    {
        const kmVec4 textColor = {0.3f, 0.7f, 0.8f, 1.0f};

        sprintf(strbuf, "FPS: %d\nFrame time: %.2f ms\nBatching: %d/%d\nFaces: %d",
            renderStats->lastFPS, renderStats->lastFrameMs, renderStats->batchCalled, 
            renderStats->batchTotal, renderStats->trianglesByFrame);

        mStatTexture->clean();
        mStatTexture->drawText(strbuf, textPos, textColor);
        mStatTexture->uploadChanges();
    }

    if (mHelpOverlay->isEnabled())
    {
        const kmVec4 textColor = {0.812f, 0.796f, 0.086f, 1.0f};
        if (mHelpState == SHOW_HELP)
        {
            sprintf(strbuf, commonHelpString, LITE3D_VERSION_STRING, mSampleHelpString.c_str());
        }
        else if (mHelpState == SHOW_RESOURCES)
        {
            ResourceManager::ResourceManagerStats memStats = mMain.getResourceManager()->getStats();
            sprintf(strbuf, resourceStatsString, 
                static_cast<uint32_t>(memStats.usedVideoMem / 1024),
                memStats.totalObjectsCount, memStats.pipelinesCount,
                memStats.texturesLoadedCount, memStats.texturesCount,
                memStats.materialsLoadedCount, memStats.materialsCount,
                memStats.scenesLoadedCount, memStats.scenesCount,
                memStats.meshesLoadedCount, memStats.meshesCount,
                memStats.meshPartitionsCount, memStats.meshPartitionsLoadedCount,
                memStats.shaderProgramsLoadedCount, memStats.shaderProgramsCount,
                memStats.renderTargetsLoadedCount, memStats.renderTargetsCount,
                memStats.ssboLoadedCount, memStats.ssboCount,
                memStats.uboLoadedCount, memStats.uboCount,
                renderStats->vboCount, renderStats->iboCount, renderStats->vaoCount, renderStats->queryCount,
                static_cast<uint32_t>(memStats.totalCachedFilesMemSize / 1024), memStats.fileCachesCount);
        }
        else
        {
            sprintf(strbuf, renderStatsString, 
                renderStats->framesCount, renderStats->lastFPS, renderStats->avrFPS, renderStats->bestFPS, renderStats->worstFPS,
                renderStats->lastFrameMs, renderStats->avrFrameMs, renderStats->bestFrameMs, renderStats->worstFrameMs,
                renderStats->nodesTotal, renderStats->batchTotal, renderStats->batchCalled, 
                renderStats->batchInstancedCalled - renderStats->batchCalled, renderStats->batchOccluded, 
                renderStats->trianglesByFrame);
        }

        mHelpTexture->clean();
        mHelpTexture->drawText(strbuf, textPos, textColor);
        mHelpTexture->uploadChanges();
    }
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

    updateCameraVelocity(kmVec3 { mCameraVelocityVector.y, 0.0f, mCameraVelocityVector.x}, deltaRetard);
    mainCameraChanged();
}

void Sample::frameEnd()
{
    moveCamera();
}

void Sample::updateCameraVelocity(const kmVec3& velocity, float deltaRetard)
{
    kmVec3 offsetPosition;
    kmVec3Scale(&offsetPosition, &velocity, deltaRetard);
    mMainCamera->moveRelative(offsetPosition);
}

}}
