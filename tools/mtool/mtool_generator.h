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

#include <lite3dpp/lite3dpp_config_writer.h>

class Generator
{
public:

    Generator(const lite3dpp::String &outputFolder,
        const lite3dpp::String &objectName);

    virtual void generateNode(const lite3dpp::String &name, const kmMat4 *transform,
        bool meshExist) = 0;
    /* make child node and go to it */
    virtual void pushNodeTree() = 0;
    /* go to parent node */
    virtual void popNodeTree() = 0;
    /* genegate material and textures */
    virtual void generateMaterial(const lite3dpp::String &matName, 
        const kmVec4 *ambient,
        const kmVec4 *diffuse,
        const kmVec4 *specular,
        const kmVec4 *emissive,
        const kmVec4 *reflective,
        const kmVec4 *transparent,
        const lite3dpp::String &diffuseTextureFile,
        const lite3dpp::String &normalTextureFile,
        const lite3dpp::String &reflectionTextureFile) = 0;

protected:

    lite3dpp::String mOutputFolder;
    lite3dpp::String mObjectName;
};

class NullGenerator : public Generator
{
public:

    NullGenerator();

    virtual void generateNode(const lite3dpp::String &name, const kmMat4 *transform,
        bool meshExist) override;
    /* make child node and go to it */
    virtual void pushNodeTree() override;
    /* go to parent node */
    virtual void popNodeTree() override;
    /* genegate material and textures */
    virtual void generateMaterial(const lite3dpp::String &matName, 
        const kmVec4 *ambient,
        const kmVec4 *diffuse,
        const kmVec4 *specular,
        const kmVec4 *emissive,
        const kmVec4 *reflective,
        const kmVec4 *transparent,
        const lite3dpp::String &diffuseTextureFile,
        const lite3dpp::String &normalTextureFile,
        const lite3dpp::String &reflectionTextureFile) override;
};

class JsonGenerator : public Generator
{
public:

    JsonGenerator(const lite3dpp::String &outputFolder,
        const lite3dpp::String &objectName);

    virtual void generateNode(const lite3dpp::String &name, const kmMat4 *transform,
        bool meshExist) override;
    /* make child node and go to it */
    virtual void pushNodeTree() override;
    /* go to parent node */
    virtual void popNodeTree() override;
    /* genegate material and textures */
    virtual void generateMaterial(const lite3dpp::String &matName, 
        const kmVec4 *ambient,
        const kmVec4 *diffuse,
        const kmVec4 *specular,
        const kmVec4 *emissive,
        const kmVec4 *reflective,
        const kmVec4 *transparent,
        const lite3dpp::String &diffuseTextureFile,
        const lite3dpp::String &normalTextureFile,
        const lite3dpp::String &reflectionTextureFile) override;

private:

    lite3dpp::stl<lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector>::stack mNodesStack;
};