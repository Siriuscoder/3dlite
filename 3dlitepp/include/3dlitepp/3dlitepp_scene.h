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

#include <3dlite/3dlite_scene.h>

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_resource.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Scene : public AbstractResource, public NoncopiableResource
    {
    public:
        Scene(const lite3dpp_string &name, 
            const lite3dpp_string &path, Main *main);
        ~Scene();

        inline lite3d_scene *getPtr()
        { return &mScene; }

    protected:

        virtual void loadImpl(const void *buffer, size_t size);
        virtual void unloadImpl();
        virtual void reloadImpl();

    private:

        lite3d_scene mScene;
    };
}

