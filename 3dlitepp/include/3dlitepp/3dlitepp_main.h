/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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

#include <3dlite/3dlite_main.h>
#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_manageable.h>
#include <3dlitepp/3dlitepp_resource_pack_manager.h>

namespace lite3dpp
{

    class LITE3DPP_EXPORT Main
    {
    public:

        Main();
        ~Main();

        bool initFromConfig(const char *config);
        const lite3d_global_settings &getSettings() const;

        void setResourceLocation(const lite3dpp_string &location);

        bool run();
        void stop();

        inline ResourcePackManager &getResourcePackManager()
        { return mResourcePackManager; }

    private:

        static int engineInit(void *userdata);
        static int engineLeave(void *userdata);
        static int engineFrameBegin(void *userdata);
        static int engineFrameEnd(void *userdata);

    private:

        ResourcePackManager mResourcePackManager;
        stl<lite3dpp_string>::set mResourceLocations;
        lite3d_global_settings mSettings;
    };
}