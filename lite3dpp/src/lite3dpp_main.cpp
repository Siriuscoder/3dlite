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
#include <algorithm>

#include <SDL_rwops.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_dbg.h>

#include <lite3dpp/json/JSON.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    const char Main::fixedUpdateTimerName[] = "fixedTimer";

    Main::Main() :
        mResourceManager(this)
    {
        /* init memory model first
         * json parser used lite3d allocator model,
         * and if we do not do it - application will 
         * be crash on config parse step 
         */
        lite3d_memory_init(NULL);
    }

    void Main::initFromConfig(const std::string_view &config)
    {           
        mConfig.reset(new ConfigurationReader(config));
        parseConfig();
    }

    void Main::initFromConfigString(const std::string_view &config)
    {           
        mConfig.reset(new ConfigurationReader(config.data(), config.size()));
        parseConfig();
    }
    
    void Main::parseConfig()
    {
        memset(&mSettings, 0, sizeof (mSettings));
        mSettings.maxFileCacheSize = 0x100000;

        mSettings.logLevel = mConfig->getInt(L"LogLevel", LITE3D_LOGLEVEL_ERROR);
        mSettings.logMuteStd = mConfig->getBool(L"logMuteStd", false) ? LITE3D_TRUE : LITE3D_FALSE;
        mSettings.logFlushAlways = mConfig->getBool(L"LogFlushAlways", false) ? LITE3D_TRUE : LITE3D_FALSE;
        mConfig->getString(L"LogFile").copy(mSettings.logFile, sizeof(mSettings.logFile)-1);

        if (mConfig->getBool(L"Minidump", false))
            lite3d_dbg_enable_coredump();

        ConfigurationReader textureSettings = mConfig->getObject(L"TextureSettings");
        ConfigurationReader videoSettings = mConfig->getObject(L"VideoSettings");

        mSettings.textureSettings.anisotropy = textureSettings.getInt(L"Anisotropy", 2);
        mSettings.textureSettings.useGLCompression =
            textureSettings.getBool(L"Compression", false) ? LITE3D_TRUE : LITE3D_FALSE;
        mSettings.textureSettings.useCompressedDataOnLoad =
            textureSettings.getBool(L"CompressedDataOnLoad", false) ? LITE3D_TRUE : LITE3D_FALSE;

        auto depthFormat = textureSettings.getString(L"DepthFormat", "Default");
        mSettings.textureSettings.useDepthFormat =
            depthFormat == "Depth32" ? LITE3D_TEXTURE_IFORMAT_DEPTH_32 : (
            depthFormat == "Depth32F" ? LITE3D_TEXTURE_IFORMAT_DEPTH_32F : LITE3D_TEXTURE_IFORMAT_DEPTH_DEFAULT);

        mSettings.videoSettings.screenWidth = videoSettings.getInt(L"Width", 0);
        mSettings.videoSettings.screenHeight = videoSettings.getInt(L"Height", 0);
        mSettings.videoSettings.colorBits = videoSettings.getInt(L"ColorBits", 24);
        mSettings.videoSettings.MSAA = videoSettings.getInt(L"MSAA", 1);
        mSettings.videoSettings.vsync =
            videoSettings.getBool(L"VSync", true) ? LITE3D_TRUE : LITE3D_FALSE;
        mSettings.videoSettings.fullscreen =
            videoSettings.getBool(L"Fullscreen", false) ? LITE3D_TRUE : LITE3D_FALSE;
        videoSettings.getString(L"Caption", "TEST window").copy(mSettings.videoSettings.caption,
            sizeof(mSettings.videoSettings.caption)-1);
        mSettings.videoSettings.hidden = videoSettings.getBool(L"Hidden", false) ? LITE3D_TRUE : LITE3D_FALSE;

        // GL Profile
        {
            auto glProfile = videoSettings.getUpperString(L"GLProfile", "");

            if (glProfile == "CORE")
            {
                mSettings.videoSettings.glProfile = LITE3D_GL_PROFILE_CORE;
            }
            else if (glProfile == "COMPATIBILITY")
            {
                mSettings.videoSettings.glProfile = LITE3D_GL_PROFILE_COMPATIBILITY;
            }
            else
            {
                mSettings.videoSettings.glProfile = LITE3D_GL_PROFILE_DEFAULT;
            }
        }

        // GL Version
        mSettings.videoSettings.glVersionMajor = \
            videoSettings.getInt(L"GLVersionMajor", 0);

        mSettings.videoSettings.glVersionMinor = \
            videoSettings.getInt(L"GLVersionMinor", 0);

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

    void Main::showSystemCursor(bool ok)
    {
        lite3d_video_view_system_cursor(ok ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void Main::init()
    {
        if(!mConfig)
            LITE3D_THROW("Bad configuration");
        
        /* basic initialization */
        initResourceLocations();

        /* create main window render target */
        /* it is fake and used only as label indicating render to screen */
        mResourceManager.queryResourceFromJson<WindowRenderTarget>("MainWindow", ConfigurableResource::emptyJson);

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
        removeAllCameras();
    }

    WindowRenderTarget *Main::window()
    {
        /* query main windows from resources */
        return mResourceManager.queryResource<WindowRenderTarget>("MainWindow");
    }

    void Main::renderFrame()
    {
        lite3d_render_frame();
    }

    void Main::processEvents()
    {
        while (lite3d_render_loop_pump_event());
    }

    Camera *Main::addCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if (it != mCameras.end())
            LITE3D_THROW("Camera \"" << name << "\" already exists..");

        auto emplaced = mCameras.try_emplace(name, name, this);
        return &emplaced.first->second;
    }

    Camera *Main::getCamera(const String &name)
    {
        auto it = mCameras.find(name);
        if (it != mCameras.end())
            return &it->second;

        return nullptr;
    }

    void Main::removeAllCameras()
    {
        mCameras.clear();
    }

    void Main::removeCamera(const String &name)
    {
        Cameras::iterator it = mCameras.find(name);
        if (it != mCameras.end())
        {
            mCameras.erase(it);
        }
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
            LITE3D_EXT_OBSERVER_NOTIFY(mainObj, shut);
            mainObj->shut();
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
        mTimers.emplace(name, timer);
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
