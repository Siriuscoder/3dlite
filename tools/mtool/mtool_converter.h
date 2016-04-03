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
#pragma once

#include <mtool_command.h>
#include <mtool_generator.h>

class ConverterCommand : public Command
{
public:

    ConverterCommand();

protected:

    virtual void runImpl() override;
    virtual void parseCommandLineImpl(int argc, char *args[]) override;

private:

    static void entry_on_mesh(lite3d_mesh *mesh, const kmMat4 *transform, const char *name, void *userdata);
    static lite3d_mesh *entry_alloc_mesh(void *userdata);
    static void entry_level_push(void *userdata);
    static void entry_level_pop(void *userdata);
    static void entry_on_material(const char *matName, 
        uint32_t matIndex,
        const kmVec4 *ambient,
        const kmVec4 *diffuse,
        const kmVec4 *specular,
        const kmVec4 *emissive,
        const kmVec4 *reflective,
        const kmVec4 *transparent,
        const char *diffuseTextureFile,
        const char *normalTextureFile,
        const char *reflectionTextureFile,
        void *userdata);

    void processMesh(lite3d_mesh *mesh, const kmMat4 *transform, const lite3dpp::String &name);
    void convertMesh(lite3d_mesh *mesh, const lite3dpp::String &savePath);

private:

    lite3dpp::String mInputFilePath;
    bool mOptimizeMesh;
    bool mFlipUV;
    bool mGenerateJson;
    lite3d_mesh mMesh;
    std::unique_ptr<Generator> mGenerator;
    GeneratorOptions mGenOptions;
};
