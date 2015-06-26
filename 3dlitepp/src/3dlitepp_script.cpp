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

#include <3dlitepp/as_helpers/scriptbuilder.h>
#include <3dlitepp/3dlitepp_main.h>


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

    Script::Script(const lite3dpp_string &name, 
        const lite3dpp_string &path, Main *main) :
        AbstractResource(name, path, main),
        mScriptEngine(main->getScriptDispatcher()->mAsEngine),
        mInitFunction(NULL),
        mShutFunction(NULL),
        mFrameBeginFunction(NULL),
        mFrameEndFunction(NULL),
        mContext(NULL)
    {
        mType = AbstractResource::SCRIPT;
    }

    Script::~Script()
    {
    }

    void Script::scriptCompile(const char *data, size_t size)
    {
        SDL_assert(data);
        
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
            "Compiling %s ...", mPath.c_str());

        /*  The CScriptBuilder helper is an add-on that loads the file,
            performs a pre-processing pass if necessary, and then tells
            the engine to build a script module. */
        CScriptBuilder builder;
        builder.SetIncludeCallback(scriptBuilderSearchInclude, mMain);
        SDL_assert_release(builder.StartNewModule(mScriptEngine, mName.c_str()) >= 0);

        /* load code sections */
        if (builder.AddSectionFromMemory(mName.c_str(), data, size, 0) < 0)
            throw std::runtime_error("Script load error");

        /* compile script */
        if (builder.BuildModule() < 0)
            throw std::runtime_error("Script compile error");

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

        /* register script in event dispatcher */
        mMain->getScriptDispatcher()->registerScript(this);
    }

    void Script::checkScriptExec(int code)
    {
        if (code != asEXECUTION_FINISHED)
        {
            if (code == asEXECUTION_EXCEPTION)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                             "Script %s unexpected broken: %s", 
                             mName.c_str(), mContext->GetExceptionString());
                
                throw std::runtime_error(mContext->GetExceptionString());
            }
            else if (code == asEXECUTION_ERROR)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                             "Script %s execution error..",
                             mName.c_str());
                
                throw std::runtime_error(lite3dpp_string("Script ") +
                                         mName + " execution error..");
            }
        }
    }

    void Script::scriptRelease()
    {
        /* unregister script and going to release script object */
        mMain->getScriptDispatcher()->unregisterScript(this);

        if (mShutFunction)
        {
            mContext->Prepare(mShutFunction);
            checkScriptExec(mContext->Execute());
        }

        if (mContext)
            mContext->Release();
        mContext = NULL;
    }

    void Script::performFrameBegin()
    {
        if (mFrameBeginFunction)
        {
            mContext->Prepare(mFrameBeginFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void Script::performFrameEnd()
    {
        if (mFrameEndFunction)
        {
            mContext->Prepare(mFrameEndFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void Script::performFixedUpdate()
    {
        if (mFixedUpdateFunction)
        {
            mContext->Prepare(mFixedUpdateFunction);
            checkScriptExec(mContext->Execute());
        }
    }

    void Script::loadImpl(const void *buffer, size_t size)
    {
        scriptCompile(static_cast<const char *>(buffer), size);
    }

    void Script::unloadImpl()
    {
        scriptRelease();
    }

    void Script::reloadImpl()
    {}
}