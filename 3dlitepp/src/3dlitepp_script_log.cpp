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
#include <SDL_assert.h>
#include <SDL_log.h>

#include <3dlitepp/as/angelscript.h>

#include <3dlitepp/3dlitepp_script_binding.h>

namespace lite3dpp
{
    static void printToLogInfo(const lite3dpp::lite3dpp_string &str)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, str.c_str());
    }

    static void printToLogDebug(const lite3dpp::lite3dpp_string &str)
    {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, str.c_str());
    }

    static void printToLogError(const lite3dpp::lite3dpp_string &str)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, str.c_str());
    }

    void RegisterScriptLogger(asIScriptEngine *engine)
    {
        SDL_assert(engine->RegisterGlobalFunction("void logInfo(const string &in)",
            asFUNCTION(printToLogInfo), asCALL_CDECL) >= 0);
        SDL_assert(engine->RegisterGlobalFunction("void logDebug(const string &in)",
            asFUNCTION(printToLogDebug), asCALL_CDECL) >= 0);
        SDL_assert(engine->RegisterGlobalFunction("void logError(const string &in)",
            asFUNCTION(printToLogError), asCALL_CDECL) >= 0);
    }
}