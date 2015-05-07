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
#include <3dlitepp/3dlitepp_script_manager.h>
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

    ScriptManager::ScriptManager(Main *main) :
    mMain(main), mAsEngine(NULL)
    {

    }

    ScriptManager::~ScriptManager()
    {

    }

    Script *ScriptManager::loadResourceFromFile(const lite3dpp_string &fileName)
    {
        ManagedScripts::iterator it = mManagedScripts.find(fileName);
        if (it != mManagedScripts.end())
            return it->second;

        lite3d_resource_file *resource =
            mMain->getResourcePackManager().loadResourceFromFile(fileName);

        Script *scriptObj = new Script(fileName, mMain, mAsEngine);
        
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Compiling script %s ...",
                    fileName.c_str());
        scriptObj->scriptCompile((char *) resource->fileBuff, resource->fileSize);
        mManagedScripts.insert(std::make_pair(fileName, scriptObj));

        return scriptObj;
    }

    void ScriptManager::unloadResource(Script *resource)
    {
        ManagedScripts::iterator it = mManagedScripts.begin();
        for (; it != mManagedScripts.end(); ++it)
        {
            if (it->second == resource)
            {
                it->second->scriptRelease();
                delete it->second;
                mManagedScripts.erase(it);
                break;
            }
        }
    }

    void ScriptManager::unloadResource(const lite3dpp_string &resourceName)
    {
        ManagedScripts::iterator it = mManagedScripts.find(resourceName);
        if (it != mManagedScripts.end())
        {
            it->second->scriptRelease();
            delete it->second;
            mManagedScripts.erase(it);
        }
    }

    void ScriptManager::unloadAllResources()
    {
        ManagedScripts::iterator it = mManagedScripts.begin();
        for (; it != mManagedScripts.end(); ++it)
        {
            it->second->scriptRelease();
            delete it->second;
        }

        mManagedScripts.clear();
    }

    size_t ScriptManager::loadedResourcesSize() const
    {
        size_t result = 0;
        ManagedScripts::const_iterator it = mManagedScripts.begin();
        for (; it != mManagedScripts.end(); ++it)
        {
            result += it->second->getScriptSize();
        }

        return result;
    }

    /* script callers */
    void ScriptManager::performFrameBegin()
    {
        ManagedScripts::const_iterator it = mManagedScripts.begin();
        for (; it != mManagedScripts.end(); ++it)
        {
            it->second->performFrameBegin();
        }
    }

    void ScriptManager::performFrameEnd()
    {
        ManagedScripts::const_iterator it = mManagedScripts.begin();
        for (; it != mManagedScripts.end(); ++it)
        {
            it->second->performFrameEnd();
        }
    }

    void ScriptManager::performFixedUpdate()
    {
        ManagedScripts::const_iterator it = mManagedScripts.begin();
        for (; it != mManagedScripts.end(); ++it)
        {
            it->second->performFixedUpdate();
        }
    }

    void ScriptManager::registerGlobals()
    {
        RegisterStdString(mAsEngine);
        RegisterScriptArray(mAsEngine, true);
        RegisterStdStringUtils(mAsEngine);
        RegisterScriptMath(mAsEngine);

        SDL_assert(mAsEngine->RegisterGlobalFunction("void breakRender()",
            asFUNCTION(lite3d_render_stop), asCALL_CDECL) >= 0);
    }

    void ScriptManager::init()
    {
        // Create the script engine
        mAsEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        // Set the message callback to receive information on errors in human readable form.
        mAsEngine->SetMessageCallback(asFUNCTION(asMessageListener), 0, asCALL_CDECL);

        registerGlobals();
    }

    void ScriptManager::shut()
    {
        /* destroy all scripts */
        unloadAllResources();

        if (mAsEngine)
        {
            mAsEngine->ShutDownAndRelease();
            mAsEngine = NULL;
        }
    }
}