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
#include <iostream>
#include <memory>

#include <lite3d/lite3d_main.h>
#include <mtool_converter.h>
#include <mtool_m_info.h>
#include <mtool_create_dirs.h>

static void print_help_and_exit()
{
    printf("Usage: \n");
    printf("View m file content:  \n\t-p -i[input file] \n");
    printf("Convert file:  \n\t-c -i[input file] -o[output folder] -O[optimize mesh] -F[flip UVs] -j[generate json]\n");
    printf("Create directories:  \n\t-d -o[output folder]\n\n");
    exit(1);
}

int main(int argc, char *args[])
{
    int i = 0;
    std::unique_ptr<Command> command;

    printf("Lite3d conversion utility.\n");
    printf("Conversion from formats supported by Assimp to internal lite3d format (m).\n");
    printf("Engine version %s\n\n", LITE3D_VERSION_STRING);

    if (argc < 3)
        print_help_and_exit();

    for (i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-p") == 0)
            command.reset(new MeshInfoCommand());
        else if (strcmp(args[i], "-c") == 0)
            command.reset(new ConverterCommand());
        else if (strcmp(args[i], "-d") == 0)
            command.reset(new CreateDirsCommand());
    }

    if (!command)
        print_help_and_exit();

    try
    {
        command->run(argc, args);
    }
    catch(std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        print_help_and_exit();
    }

    return 0;
}