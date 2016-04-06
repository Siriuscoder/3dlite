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
#include <mtool_create_dirs.h>

void CreateDirsCommand::runImpl()
{
    makeFolders(mOutputFolder);
}

void CreateDirsCommand::parseCommandLineImpl(int argc, char *args[])
{
    Command::parseCommandLineImpl(argc, args);

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-o") == 0)
        {
            if ((i + 1) < argc && args[i + 1][0] != '-')
                mOutputFolder.assign(args[i + 1]);
            else
                throw std::runtime_error("Missing output folder");
        }
    }
}