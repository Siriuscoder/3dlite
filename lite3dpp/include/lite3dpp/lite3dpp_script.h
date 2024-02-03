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
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ScriptActing
    {
    public:

        virtual void performFrameBegin() = 0;
        virtual void performFrameEnd() = 0;
        virtual void performFixedUpdate() = 0;
        virtual void performProcessEvent(SDL_Event *e) = 0;
    };

    class LITE3DPP_EXPORT Script : public AbstractResource, public ScriptActing, public Noncopiable
    {
    public:

        Script(const String &name, 
            const String &path, Main *main);
        ~Script();

    protected:

        virtual void scriptCompile(const String &text) = 0;
        virtual void scriptRelease() = 0;

        virtual void loadImpl(const void *buffer, size_t size) override;
        virtual void reloadImpl() override;
        virtual void unloadImpl() override;

    private:

        String mScriptText;
    };
}

