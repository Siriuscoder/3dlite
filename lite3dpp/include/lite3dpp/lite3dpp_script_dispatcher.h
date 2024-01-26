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

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>
#include <lite3dpp/lite3dpp_resource_manager.h>
#include <lite3dpp/lite3dpp_script.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ScriptDispatcher : public ScriptActing, public Manageable
    {
        friend class AsScript;
    public:

        typedef stl<String, Script *>::unordered_map Scripts;

        ScriptDispatcher(Main *main);
        virtual ~ScriptDispatcher();

        /* script callers */
        virtual void performFrameBegin() override;
        virtual void performFrameEnd() override;
        virtual void performFixedUpdate() override;
        virtual void performProcessEvent(SDL_Event *e) override;

        void registerScript(Script *script);
        void unregisterScript(Script *script);

    private:

        void flushPendingScripts();

        Main *mMain;
        Scripts mScripts;
        Scripts mPendingScripts;
    };
}

