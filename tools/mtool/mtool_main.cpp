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

static void print_help_and_exit()
{
    printf("Lite3d conversion utility.\n");
    printf("Conversion from formats supported by Assimp to internal lite3d format (m).\n");
    printf("Engine version %s\n\n", LITE3D_VERSION_STRING);

    printf("Usage: -p[view file] -i[input] file -o[output] folder -O[optimize mesh] -F[flip UVs]\n\n");
    exit(1);
}

int main(int argc, char *args[])
{
    int i = 0;
    std::unique_ptr<Command> command;

    if (argc < 3)
        print_help_and_exit();

    for (i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-p") == 0)
        {
            command.reset(new MeshInfoCommand());
            if ((i + 1) < argc)
                command->setInputFilePath(args[i + 1]);
            else
                print_help_and_exit();
        }
        else if (strcmp(args[i], "-i") == 0)
        {
            command.reset(new ConverterCommand());
            if ((i + 1) < argc)
                command->setInputFilePath(args[i + 1]);
            else
                print_help_and_exit();
        }
        else if (strcmp(args[i], "-o") == 0)
        {
            if ((i + 1) < argc)
                static_cast<ConverterCommand *>(command.get())->setOutputFolder(args[i + 1]);
            else
                print_help_and_exit();
        }
        else if (strcmp(args[i], "-O") == 0)
        {
            static_cast<ConverterCommand *>(command.get())->enableOptimize();
        }
        else if (strcmp(args[i], "-F") == 0)
        {
            static_cast<ConverterCommand *>(command.get())->enableFlipUV();
        }
        else if (strcmp(args[i], "-j") == 0)
        {
            static_cast<ConverterCommand *>(command.get())->enableGenerateJson();
        }
    }

    if (!command)
        print_help_and_exit();

    try
    {
        command->run();
    }
    catch(std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    return 0;
}