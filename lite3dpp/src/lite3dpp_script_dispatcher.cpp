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
#include <algorithm>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3dpp/as/angelscript.h>
#include <lite3dpp/as_helpers/scriptstdstring.h>
#include <lite3dpp/as_helpers/scriptarray.h>
#include <lite3dpp/as_helpers/scriptmath.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_script_binding.h>
#include <lite3dpp/lite3dpp_script_dispatcher.h>

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

    void ScriptDispatcher::performEvent(SDL_Event *e)
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

    void ScriptDispatcher::registerGlobals()
    {
        /* register common types */
        RegisterStdString(mAsEngine);
        RegisterScriptArray(mAsEngine, true);
        RegisterStdStringUtils(mAsEngine);
        RegisterScriptMath(mAsEngine);

        /* register engine types */
        RegisterScriptTypes(mAsEngine);
        RegisterScriptVec3(mAsEngine);
        RegisterScriptVec4(mAsEngine);
        RegisterScriptMat4(mAsEngine);
        RegisterScriptMat3(mAsEngine);
        RegisterScriptQuaternion(mAsEngine);
        RegisterScriptLogger(mAsEngine);

        /* register functions */
        SDL_assert(mAsEngine->RegisterGlobalFunction("void breakRender()",
            asFUNCTION(lite3d_render_stop), asCALL_CDECL) >= 0);
        SDL_assert(mAsEngine->RegisterGlobalFunction("void suspendRender()",
            asFUNCTION(lite3d_render_suspend), asCALL_CDECL) >= 0);
        SDL_assert(mAsEngine->RegisterGlobalFunction("void resumeRender()",
            asFUNCTION(lite3d_render_resume), asCALL_CDECL) >= 0);
    }
}