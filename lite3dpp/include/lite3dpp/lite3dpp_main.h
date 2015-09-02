/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#pragma once

#include <lite3d/lite3d_main.h>
#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_resource_manager.h>
#include <lite3dpp/lite3dpp_script_dispatcher.h>
#include <lite3dpp/lite3dpp_json_helper.h>
#include <lite3dpp/lite3dpp_scene.h>
#include <lite3dpp/lite3dpp_render_target.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Main
    {
    public:

        class LITE3DPP_EXPORT LifecycleListener
        {
        public:

            virtual ~LifecycleListener();

            virtual void init(Main *main) = 0;
            virtual void shut(Main *main) = 0;
            virtual void frameBegin(Main *main) = 0;
            virtual void frameEnd(Main *main) = 0;
            virtual void timerTick(Main *main, lite3d_timer *timerid) = 0;
            virtual void processEvent(Main *main, SDL_Event *e) = 0;
        };

    public:

        Main();
        ~Main();

        void initFromConfig(const char *config);
        const lite3d_global_settings &getSettings() const;

        void setResourceLocation(const String &name, 
            const String &location,
            size_t fileCacheMaxSize);

        inline ResourceManager *getResourceManager()
        { return &mResourceManager; }
        inline ScriptDispatcher *getScriptDispatcher()
        { return &mScriptDispatcher; }
        inline void registerLifecycleListener(LifecycleListener *listener)
        { mLifeCycleListener = listener; }
        WindowRenderTarget *window();

        void run();
        void stop();
        
    private:

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
        ScriptDispatcher mScriptDispatcher;
        std::unique_ptr<JsonHelper> mConfig;
        lite3d_global_settings mSettings;
        lite3d_timer *mFixedUpdatesTimer;
        LifecycleListener *mLifeCycleListener;
    };
}