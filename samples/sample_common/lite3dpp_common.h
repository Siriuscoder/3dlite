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
#pragma once

#include <iostream>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp_font/lite3dpp_font_texture.h>

namespace lite3dpp {
namespace samples {

class Sample : public LifecycleObserver,
    public SceneObserver
{
public:

    Sample();

    void init() override;
    void timerTick(lite3d_timer *timerid) override;
    void processEvent(SDL_Event *e) override;
    void beginSceneRender(Scene *scene, Camera *camera) override;

    int start(const char *config);

    inline void setMainCamera(Camera *camera)
    { mMainCamera = camera; }
    inline Main &getMain()
    { return mMain; }
    WindowRenderTarget &getMainWindow();
    Camera &getMainCamera();


    virtual void createScene() = 0;

protected:

    void initGui();
    void printStats();
    void updateGuiStats();

private:

    Main mMain;
    Camera *mMainCamera;
    Camera *mGuiCamera;
    WindowRenderTarget *mMainWindow;
    lite3dpp_font::FontTexture *mStatTexture;
    lite3d_timer *mStatTimer;
    int mCenterXPos;
    int mCenterYPos;
};

}}
