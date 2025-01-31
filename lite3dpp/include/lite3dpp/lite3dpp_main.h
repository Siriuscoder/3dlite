/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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

#include <lite3d/lite3d_main.h>
#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_resource_manager.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_scene.h>
#include <lite3dpp/lite3dpp_render_target.h>
#include <lite3dpp/lite3dpp_texture_render_target.h>
#include <lite3dpp/lite3dpp_config_writer.h>
#include <lite3dpp/lite3dpp_observer.h>
#include <lite3dpp/lite3dpp_material_factory.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Main : public Observable<LifecycleObserver>, public Noncopiable
    {
    public:
        typedef stl<String, lite3d_timer *>::unordered_map Timers;
        typedef stl<String, Camera>::unordered_map Cameras;

        static const char fixedUpdateTimerName[];

    public:

        Main();
        ~Main() = default;

        void initFromConfig(const std::string_view &config);
        void initFromConfigString(const std::string_view &config);
        const lite3d_global_settings &getSettings() const;

        void setResourceLocation(const String &name, 
            const String &location,
            size_t fileCacheMaxSize);

        lite3d_timer *addTimer(const String &name, int32_t millisec);
        lite3d_timer *getTimer(const String &name);

        inline ResourceManager *getResourceManager()
        { return &mResourceManager; }
        inline lite3d_timer *getFixedUpdateTimer()
        { return mFixedUpdatesTimer; }
        WindowRenderTarget *window();
        inline const lite3d_render_stats *getRenderStats() const
        { return lite3d_render_stats_get(); }
        void showSystemCursor(bool ok);
        void processEvents();
        void renderFrame();

        Camera *addCamera(const String &name);
        Camera *getCamera(const String &name);
        void removeAllCameras();
        void removeCamera(const String &name);

        void run();
        void stop();

        template<class T>
        inline void initMaterialFactory()
        {
            mMaterialFactory = std::make_unique<T>(*this);
        }

        const MaterialFactory &getMaterialFactory() const;

    private:

        void parseConfig();

        static int onInit(void *userdata);
        static int onShutdown(void *userdata);
        static int onFrameBegin(void *userdata);
        static int onFrameEnd(void *userdata);
        static void onTimerTick(lite3d_timer *timer);
        static int onProcessEvent(SDL_Event *e, void *userdata);

        void initResourceLocations();
        void init();
        void shut();

    private:

        ResourceManager mResourceManager;
        std::unique_ptr<ConfigurationReader> mConfig;
        lite3d_global_settings mSettings;
        Timers mTimers;
        lite3d_timer *mFixedUpdatesTimer;
        Cameras mCameras;
        std::unique_ptr<MaterialFactory> mMaterialFactory;
    };
}