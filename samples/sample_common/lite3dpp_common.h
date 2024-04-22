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
#pragma once

#include <iostream>
#include <chrono>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp_font/lite3dpp_font_texture.h>

namespace lite3dpp {
namespace samples {

#define CAMERA_DEFAULT_SENSITIVITY      0.001f
#define CAMERA_DEFAULT_VELOCITY_MAX     8.0f
#define CAMERA_DEFAULT_ACCEL            2.0f
#define CAMERA_DEFAULT_ACCEL_RESIST     1.0f

class Sample : public LifecycleObserver
{
public:

    Sample();

    void init() override;
    void timerTick(lite3d_timer *timerid) override;
    void processEvent(SDL_Event *e) override;
    void frameEnd() override;

    int start(const char *config);

    inline void setMainCamera(Camera *camera)
    { mMainCamera = camera; }
    inline Main &getMain()
    { return mMain; }
    WindowRenderTarget &getMainWindow();
    Camera &getMainCamera();
    inline void setCameraSensitivity(float v)
    { mCameraSensitivity = v; }
    inline void setCameraVelocityMax(float v)
    { mCameraVelocityMax = v; }
    inline void setCameraAcceleration(float v)
    { mCameraAccel = v; }
    inline void setCameraResistance(float v)
    { mCameraAccelResistance = v; }

    void resizeMainWindow(int32_t width, int32_t height);
    void saveScreenshot();

    virtual void createScene() = 0;
    virtual void fixedUpdateTimerTick(int32_t firedPerRound, uint64_t deltaMcs, float deltaRetard);
    virtual void updateCameraVelocity(const kmVec3& velocity);

protected:

    void initGui();
    void printRenderStats();
    void printMemoryStats();
    void moveCamera();

    void updateGuiStats();

    void setGuiSize(int32_t width, int32_t height);
    void adjustMainCamera(int32_t width, int32_t height);
    virtual void mainCameraChanged();

private:

    Main mMain;
    Camera *mMainCamera = nullptr;
    Camera *mGuiCamera = nullptr;
    Scene *mGuiScene = nullptr;
    WindowRenderTarget *mMainWindow = nullptr;
    lite3dpp_font::FontTexture *mStatTexture = nullptr;
    lite3d_timer *mStatTimer = nullptr;
    kmVec2 mWCenter = KM_VEC2_ZERO;
    std::optional<kmVec2> mCameraAngles;
    float mCameraSensitivity = CAMERA_DEFAULT_SENSITIVITY;
    kmVec2 mCameraVelocityVector = KM_VEC2_ZERO;
    float mCameraVelocityMax = CAMERA_DEFAULT_VELOCITY_MAX;
    float mCameraAccel = CAMERA_DEFAULT_ACCEL;
    float mCameraAccelResistance = CAMERA_DEFAULT_ACCEL_RESIST;
    std::chrono::steady_clock::time_point mLastFrameTime;
};

}}
