/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#include <iostream>
#include <memory>

#include <lite3d/lite3d_main.h>
#include <mtool/mtool_converter.h>
#include <mtool/mtool_m_info.h>
#include <mtool/mtool_create_dirs.h>

static void print_help_and_exit()
{
    printf("Usage: \n");
    printf("\n\t-p\tview m file content \n\t-i\tinput file \n");
    printf("\n\t-c\tconvert file \n\t-i\tinput file \n\t-o\toutput folder \n\t-O\toptimize mesh \n\t-F\tflip UVs \n\t-j\tgenerate json \n\t-oname\tobject name \n\t-[img|mesh|tex|mat|node]pkg \n\t-matastex \n");
    printf("\n\t-d\tcreate directories \n\t-o\toutput folder\n\n");
    exit(1);
}

int main(int argc, char *args[])
{
    int i = 0;
    std::unique_ptr<Command> command;

    printf("Lite3d scene objests conversion utility.\n");
    printf("Conversion from formats supports by Assimp to internal lite3d format (m).\n");
    printf("Engine version: %s\n\n", LITE3D_VERSION_STRING);

    if (argc < 3)
        print_help_and_exit();

    for (i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-p") == 0)
        {
            command.reset(new MeshInfoCommand());
            break;
        }
        else if (strcmp(args[i], "-c") == 0)
        {
            command.reset(new ConverterCommand());
            break;
        }
        else if (strcmp(args[i], "-d") == 0)
        {
            command.reset(new CreateDirsCommand());
            break;
        }
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