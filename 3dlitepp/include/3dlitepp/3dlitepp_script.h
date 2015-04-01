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
#pragma once

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_manageable.h>

namespace lite3dpp
{

    class LITE3DPP_EXPORT ScriptExecuteByEvents : public Manageable
    {
    public:

        virtual void performInit() = 0;
        virtual void performShut() = 0;
        virtual void performFrameBegin() = 0;
        virtual void performFrameEnd() = 0;
    };

    class LITE3DPP_EXPORT Script : public ScriptExecuteByEvents
    {
    public:

        Script(Main *main);
        virtual ~Script();

        lite3dpp_string getName();

        void scriptCompile(const char *data, size_t size);
        void scriptRelease();

        inline size_t getScriptSize() const
        { return mScriptSize; }

        virtual void performInit();
        virtual void performShut();
        virtual void performFrameBegin();
        virtual void performFrameEnd();

    private:

        Main *mMain;
        size_t mScriptSize;
    };
}

