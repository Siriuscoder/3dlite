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
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3dpp/as_helpers/scriptbuilder.h>
#include <lite3dpp/lite3dpp_main.h>


namespace lite3dpp
{
    Script::Script(const String &name, 
        const String &path, Main *main) :
        AbstractResource(name, path, main, AbstractResource::SCRIPT)
    {}

    Script::~Script()
    {}

    void Script::loadImpl(const void *buffer, size_t size)
    {
        mScriptText.assign((const char *)buffer, size);
        scriptCompile(mScriptText);
        /* register script in event dispatcher */
        mMain->getScriptDispatcher()->registerScript(this);
    }

    void Script::unloadImpl()
    {
        scriptRelease();
        /* unregister script and going to release script object */
        mMain->getScriptDispatcher()->unregisterScript(this);
    }

    void Script::reloadImpl()
    {
        scriptCompile(mScriptText);
        /* register script in event dispatcher */
        mMain->getScriptDispatcher()->registerScript(this);
    }
}