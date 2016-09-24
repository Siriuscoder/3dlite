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
#include <lite3dpp/as_helpers/scriptstdstring.h>
#include <lite3dpp/as_helpers/scriptarray.h>
#include <lite3dpp/as_helpers/scriptmath.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_script_binding.h>
#include <lite3dpp/lite3dpp_script_as.h>

asIScriptEngine *lite3dpp::AsScript::mScriptEngine = NULL;

namespace lite3dpp
{

    static int scriptBuilderSearchInclude(const char *include, const char *from,
                                          CScriptBuilder *builder, void *userParam)
    {
        Main *mainObj = reinterpret_cast<Main *> (userParam);

        size_t scriptFileSize;
        const char *scriptFile = static_cast<const char *>(
            mainObj->getResourceManager()->loadFileToMemory(include, &scriptFileSize));

        return builder->AddSectionFromMemory(include, (char *)scriptFile,
                                             scriptFileSize, 0);
    }

    static void asMessageListener(const asSMessageInfo *msg, void *param)
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

    AsScript::AsScript(const String &name, 
        const String &path, Main *main) :
        Script(name, path, main),
        mInitFunction(NULL),
        mShutFunction(NULL),
        mFrameBeginFunction(NULL),
        mFrameEndFunction(NULL),
        mContext(NULL)
    {}

    AsScript::~AsScript()
    {}

    void AsScript::engineInit()
    {
        // Create the script engine
        if((mScriptEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION)) == NULL)
            LITE3D_THROW("Angelscript engine startup error..");

        // Set the message callback to receive information on errors in human readable form.
        mScriptEngine->SetMessageCallback(asFUNCTION(asMessageListener), 0, asCALL_CDECL);

        /* register common types */
        RegisterStdString(mScriptEngine);
        RegisterScriptArray(mScriptEngine, true);
        RegisterStdStringUtils(mScriptEngine);
        RegisterScriptMath(mScriptEngine);

        /* register engine types */
        RegisterScriptTypes(mScriptEngine);
        RegisterScriptVec3(mScriptEngine);
        RegisterScriptVec4(mScriptEngine);
        RegisterScriptMat4(mScriptEngine);
        RegisterScriptMat3(mScriptEngine);
        RegisterScriptQuaternion(mScriptEngine);
        RegisterScriptLogger(mScriptEngine);

        /* register functions */
        SDL_assert(mScriptEngine->RegisterGlobalFunction("void breakRender()",
            asFUNCTION(lite3d_render_stop), asCALL_CDECL) >= 0);
        SDL_assert(mScriptEngine->RegisterGlobalFunction("void suspendRender()",
            asFUNCTION(lite3d_render_suspend), asCALL_CDECL) >= 0);
        SDL_assert(mScriptEngine->RegisterGlobalFunction("void resumeRender()",
            asFUNCTION(lite3d_render_resume), asCALL_CDECL) >= 0);
    }

    void AsScript::engineShut()
    {
        if (mScriptEngine)
        {
            mScriptEngine->ShutDownAndRelease();
            mScriptEngine = NULL;
        }
    }

    void AsScript::scriptCompile(const String &text)
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "Compiling %s ...", getPath().c_str());

        /*  The CScriptBuilder helper is an add-on that loads the file,
            performs a pre-processing pass if necessary, and then tells
            the engine to build a script module. */
        CScriptBuilder builder;
        builder.SetIncludeCallback(scriptBuilderSearchInclude, mMain);
        SDL_assert_release(builder.StartNewModule(mScriptEngine, getName().c_str()) >= 0);

        /* load code sections */
        if (builder.AddSectionFromMemory(getName().c_str(), text.data(), text.size(), 0) < 0)
            LITE3D_THROW("AsScript load error");

        /* compile script */
        if (builder.BuildModule() < 0)
            LITE3D_THROW("AsScript compile error");

        asIScriptModule *mod = builder.GetModule();
        mInitFunction = mod->GetFunctionByDecl("void init()");
        mShutFunction = mod->GetFunctionByDecl("void shut()");
        mFrameBeginFunction = mod->GetFunctionByDecl("void frameBegin()");
        mFrameEndFunction = mod->GetFunctionByDecl("void frameEnd()");
        mFixedUpdateFunction = mod->GetFunctionByDecl("void fixedUpdate()");

        mContext = mScriptEngine->CreateContext();
        SDL_assert_release(mContext);
        if (mInitFunction)
        {
            mContext->Prepare(mInitFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void AsScript::checkScriptExec(int code)
    {
        if (code != asEXECUTION_FINISHED)
        {
            if (code == asEXECUTION_EXCEPTION)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                             "AsScript %s unexpected broken: %s", 
                             getName().c_str(), mContext->GetExceptionString());
                
                LITE3D_THROW(mContext->GetExceptionString());
            }
            else if (code == asEXECUTION_ERROR)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                             "AsScript %s execution error..",
                             getName().c_str());
                
                LITE3D_THROW("AsScript " << getName() << " execution error..");
            }
        }
    }

    void AsScript::scriptRelease()
    {
        if (mShutFunction)
        {
            mContext->Prepare(mShutFunction);
            checkScriptExec(mContext->Execute());
        }

        if (mContext)
            mContext->Release();
        mContext = NULL;
    }

    void AsScript::performFrameBegin()
    {
        if (mFrameBeginFunction)
        {
            mContext->Prepare(mFrameBeginFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void AsScript::performFrameEnd()
    {
        if (mFrameEndFunction)
        {
            mContext->Prepare(mFrameEndFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void AsScript::performFixedUpdate()
    {
        if (mFixedUpdateFunction)
        {
            mContext->Prepare(mFixedUpdateFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void AsScript::performProcessEvent(SDL_Event *e)
    {

    }
}