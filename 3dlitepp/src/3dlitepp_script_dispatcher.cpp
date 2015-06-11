/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <3dlitepp/as/angelscript.h>
#include <3dlitepp/as_helpers/scriptstdstring.h>
#include <3dlitepp/as_helpers/scriptarray.h>
#include <3dlitepp/as_helpers/scriptmath.h>

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_script_dispatcher.h>
#include <algorithm>

namespace lite3dpp
{

    void asMessageListener(const asSMessageInfo *msg, void *param)
    {
        if (msg->type == asMSGTYPE_WARNING)
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Script: %s, (%d/%d) %s",
                        msg->section, msg->row, msg->col, msg->message);
        else if (msg->type == asMSGTYPE_ERROR)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Script: %s, (%d/%d) %s",
                         msg->section, msg->row, msg->col, msg->message);
        else if (msg->type == asMSGTYPE_INFORMATION)
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Script: %s, (%d/%d) %s",
                        msg->section, msg->row, msg->col, msg->message);
    }

    ScriptDispatcher::ScriptDispatcher(Main *main) :
        mMain(main), mAsEngine(NULL)
    {
        // Create the script engine
        mAsEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        // Set the message callback to receive information on errors in human readable form.
        mAsEngine->SetMessageCallback(asFUNCTION(asMessageListener), 0, asCALL_CDECL);
    }

    ScriptDispatcher::~ScriptDispatcher()
    {
        if (mAsEngine)
        {
            mAsEngine->ShutDownAndRelease();
            mAsEngine = NULL;
        }
    }

    void ScriptDispatcher::registerScript(Script *script)
    {
        mScripts.insert(std::make_pair(script->getName(), script));
    }

    void ScriptDispatcher::unregisterScript(Script *script)
    {
        Scripts::iterator it = mScripts.find(script->getName());
        if(it != mScripts.end())
            mScripts.erase(it);
    }

    /* script callers */
    void ScriptDispatcher::performFrameBegin()
    {
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performFrameBegin();
        }
    }

    void ScriptDispatcher::performFrameEnd()
    {
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performFrameEnd();
        }
    }

    void ScriptDispatcher::performFixedUpdate()
    {
        Scripts::const_iterator it = mScripts.begin();
        for (; it != mScripts.end(); ++it)
        {
            it->second->performFixedUpdate();
        }
    }

    void ScriptDispatcher::registerGlobals()
    {
        RegisterStdString(mAsEngine);
        RegisterScriptArray(mAsEngine, true);
        RegisterStdStringUtils(mAsEngine);
        RegisterScriptMath(mAsEngine);

        SDL_assert(mAsEngine->RegisterGlobalFunction("void breakRender()",
            asFUNCTION(lite3d_render_stop), asCALL_CDECL) >= 0);
    }
}