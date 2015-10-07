/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Script : public AbstractResource, public NoncopiableResource
    {
    public:

        Script(const String &name, 
            const String &path, Main *main);
        ~Script();

        void performFrameBegin();
        void performFrameEnd();
        void performFixedUpdate();
        void performProcessEvent(SDL_Event *e);

    protected:

        void scriptCompile(const char *data, size_t size);
        void scriptRelease();

        virtual void loadImpl(const void *buffer, size_t size) override;
        virtual void reloadImpl() override;
        virtual void unloadImpl() override;

    private:
        
        void checkScriptExec(int ret);
        
    private:

        asIScriptEngine *mScriptEngine;
        asIScriptFunction *mInitFunction;
        asIScriptFunction *mShutFunction;
        asIScriptFunction *mFrameBeginFunction;
        asIScriptFunction *mFrameEndFunction;
        asIScriptFunction *mFixedUpdateFunction;
        asIScriptContext *mContext;
    };
}

