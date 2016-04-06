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

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_script_dispatcher.h>

namespace lite3dpp
{
    ScriptDispatcher::ScriptDispatcher(Main *main) :
        mMain(main)
    {}

    ScriptDispatcher::~ScriptDispatcher()
    {}

    void ScriptDispatcher::registerScript(Script *script)
    {
        if(mScripts.find(script->getName()) != mScripts.end())
            throw std::runtime_error(script->getName() + " script already registered");

        mPendingScripts.insert(std::make_pair(script->getName(), script));
    }

    void ScriptDispatcher::unregisterScript(Script *script)
    {
        Scripts::iterator it = mPendingScripts.find(script->getName());
        if(it != mPendingScripts.end())
            mPendingScripts.erase(it);

        it = mScripts.find(script->getName());
        if(it != mScripts.end())
            mScripts.erase(it);
    }

    /* script callers */
    void ScriptDispatcher::performFrameBegin()
    {
        flushPendingScripts();
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performFrameBegin();
        }
    }

    void ScriptDispatcher::performFrameEnd()
    {
        flushPendingScripts();
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performFrameEnd();
        }
    }

    void ScriptDispatcher::performFixedUpdate()
    {
        flushPendingScripts();
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performFixedUpdate();
        }
    }

    void ScriptDispatcher::performProcessEvent(SDL_Event *e)
    {
        flushPendingScripts();
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performProcessEvent(e);
        }
    }

    void ScriptDispatcher::flushPendingScripts()
    {
        if(mPendingScripts.size() > 0)
        {
            std::for_each(mPendingScripts.begin(), mPendingScripts.end(), [this](Scripts::value_type val)
            {
                mScripts.insert(val);
            });

            mPendingScripts.clear();
        }
    }
}