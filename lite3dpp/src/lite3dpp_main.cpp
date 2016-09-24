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
#include <algorithm>

#include <SDL_rwops.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>

#include <lite3dpp/json/JSON.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    const char Main::fixedUpdateTimerName[] = "fixedTimer";

    Main::Main() :
        mResourceManager(this),
        mScriptDispatcher(this)
    {
        /* init memory model first
         * json parser used lite3d allocator model,
         * and if we do not do it - application will 
         * be crash on config parse step 
         */
        lite3d_memory_init(NULL);
    }

    void Main::initFromConfig(const char *config)
    {           
        mConfig.reset(new ConfigurationReader(config));
        parseConfig();
    }

    void Main::initFromConfigString(const char *config)
    {           
        mConfig.reset(new ConfigurationReader(config, strlen(config)));
        parseConfig();
    }
    
    void Main::parseConfig()
    {
        memset(&mSettings, 0, sizeof (mSettings));
        mSettings.maxFileCacheSize = 0x100000;

        mSettings.logLevel = mConfig->getInt(L"LogLevel", LITE3D_LOGLEVEL_ERROR);
        mSettings.logFlushAlways = mConfig->getBool(L"LogFlushAlways", false) ? LITE3D_TRUE : LITE3D_FALSE;
        mConfig->getString(L"LogFile").copy(mSettings.logFile, sizeof(mSettings.logFile)-1);

        ConfigurationReader textureSettings = mConfig->getObject(L"TextureSettings");
        ConfigurationReader videoSettings = mConfig->getObject(L"VideoSettings");

        mSettings.textureSettings.anisotropy = textureSettings.getInt(L"Anisotropy", 2);
        mSettings.textureSettings.useGLCompression =
            textureSettings.getBool(L"Compression", false) ? LITE3D_TRUE : LITE3D_FALSE;

        mSettings.videoSettings.screenWidth = videoSettings.getInt(L"Width", 0);
        mSettings.videoSettings.screenHeight = videoSettings.getInt(L"Height", 0);
        mSettings.videoSettings.colorBits = videoSettings.getInt(L"ColorBits", 24);
        mSettings.videoSettings.FSAA = videoSettings.getInt(L"FSAA", 1);
        mSettings.videoSettings.vsync =
            videoSettings.getBool(L"VSync", true) ? LITE3D_TRUE : LITE3D_FALSE;
        mSettings.videoSettings.fullscreen =
            videoSettings.getBool(L"Fullscreen", false) ? LITE3D_TRUE : LITE3D_FALSE;
        videoSettings.getString(L"Caption", "TEST window").copy(mSettings.videoSettings.caption,
            sizeof(mSettings.videoSettings.caption)-1);
        mSettings.videoSettings.hidden = videoSettings.getBool(L"Hidden", false) ? LITE3D_TRUE : LITE3D_FALSE;

        mSettings.renderLisneters.userdata = reinterpret_cast<void *> (this);
        mSettings.renderLisneters.preRender = Main::onInit;
        mSettings.renderLisneters.postRender = Main::onShutdown;
        mSettings.renderLisneters.preFrame = Main::onFrameBegin;
        mSettings.renderLisneters.postFrame = Main::onFrameEnd;
        mSettings.renderLisneters.processEvent = Main::onProcessEvent;

    }

    const lite3d_global_settings &Main::getSettings() const
    {
        return *lite3d_get_global_settings();
    }

    void Main::setResourceLocation(const String &name, 
        const String &location,
        size_t fileCacheMaxSize)
    {
        mResourceManager.addResourceLocation(name,
            location,
            fileCacheMaxSize);
    }

    Main::~Main()
    {

    }

    void Main::run()
    {
        if (!lite3d_main(&mSettings))
            LITE3D_THROW("Engine startup failed..");
    }

    void Main::stop()
    {
        lite3d_render_stop();
    }

    void Main::initResourceLocations()
    {
        for (auto &location : mConfig->getObjects(L"ResourceLocations"))
        {           
            setResourceLocation(location.getString(L"Name"), 
                location.getString(L"Path"),
                location.getInt(L"FileCacheMaxSize"));
        }
    }

    void Main::init()
    {
        if(!mConfig)
            LITE3D_THROW("Bad configuration");
        
        /* init scripting */
        AsScript::engineInit();
        /* basic initialization */
        initResourceLocations();

        /* create main window render target */
        /* it is fake and used only as label indicating render to screen */
        mResourceManager.queryResource<WindowRenderTarget>(LITE3D_EMPTY_NAMED_RESOURCE("MainWindow"));

        /* perform fixed update timer */
        mFixedUpdatesTimer = addTimer(fixedUpdateTimerName, mConfig->getInt(L"FixedUpdatesInterval", 200));
    }

    void Main::shut()
    {
        std::for_each(mTimers.begin(), mTimers.end(), [](Timers::value_type &it)
        {
            lite3d_timer_purge(it.second);
        });

        mResourceManager.releaseAllResources();
        mResourceManager.releaseFileCache();
        AsScript::engineShut();
    }

    WindowRenderTarget *Main::window()
    {
        /* query main windows from resources */
        return mResourceManager.queryResource<WindowRenderTarget>("MainWindow");
    }

    /* callbackes */
    int Main::onInit(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->init();

            LITE3D_EXT_OBSERVER_NOTIFY(mainObj, init);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    int Main::onShutdown(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->shut();

            LITE3D_EXT_OBSERVER_NOTIFY(mainObj, shut);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    int Main::onFrameBegin(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            LITE3D_EXT_OBSERVER_NOTIFY(mainObj, frameBegin);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    int Main::onFrameEnd(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            LITE3D_EXT_OBSERVER_NOTIFY(mainObj, frameEnd);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    void Main::onTimerTick(lite3d_timer *timer)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (timer->userdata);
            LITE3D_EXT_OBSERVER_NOTIFY_1(mainObj, timerTick, timer);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    int Main::onProcessEvent(SDL_Event *e, void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            LITE3D_EXT_OBSERVER_NOTIFY_1(mainObj, processEvent, e);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    lite3d_timer *Main::addTimer(const String &name, int32_t millisec)
    {
        lite3d_timer *timer;
        if((timer = getTimer(name)) != NULL)
            return timer;

        timer = lite3d_timer_add(millisec, onTimerTick, this);
        mTimers.insert(std::make_pair(name, timer));
        return timer;
    }

    lite3d_timer *Main::getTimer(const String &name)
    {
        auto it = mTimers.find(name);
        if(it != mTimers.end())
            return it->second;

        return NULL;
    }
}
