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

#include <3dlitepp/3dlitepp_main.h>
#include <3dlitepp/3dlitepp_script_manager.h>

namespace lite3dpp
{
    ScriptManager::ScriptManager(Main &main) :
        mMain(main)
    {

    }

    ScriptManager::~ScriptManager()
    {

    }

    Script *ScriptManager::loadResourceFromFile(const lite3dpp_string &fileName)
    {
        lite3d_resource_file *resource;
        try
        {
            resource = 
                mMain.getResourcePackManager().loadResourceFromFile(fileName);
        }
        catch(std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Script load error: %s",
                ex.what());
        }
    }

    void ScriptManager::unloadResource(Script *resource)
    {

    }

    void ScriptManager::unloadResource(const lite3dpp_string &resourceName)
    {

    }

    void ScriptManager::unloadAllResources()
    {

    }

    size_t ScriptManager::loadedResourcesSize() const
    {
    }

    /* event initiators */
    void ScriptManager::performInit()
    {

    }

    void ScriptManager::performShut()
    {

    }

    void ScriptManager::performFrameBegin()
    {

    }

    void ScriptManager::performFrameEnd()
    {

    }
}