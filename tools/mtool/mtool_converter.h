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
#include <lite3dpp/lite3dpp_config_writer.h>

class ConverterCommand : public Command
{
public:

    ConverterCommand();

    inline void setOutputFolder(const lite3dpp::String &path)
    { mOutputFolder = path; }
    inline void enableOptimize()
    { mOptimizeMesh = true; }
    inline void enableFlipUV()
    { mFlipUV = true; }
    inline void enableGenerateJson()
    { mGenerateJson = true; }
    inline void setObjectName(const lite3dpp::String &objName)
    { mObjectName = objName; }

protected:

    virtual void runImpl() override;

    void processMesh(lite3d_mesh *mesh, const kmMat4 *transform, const lite3dpp::String &name);
    void makeFolders();
    void saveFile(const void *buffer, size_t size, const lite3dpp::String &path);
    void nodeLevelPush();
    void nodeLevelPop();
    void convertMesh(lite3d_mesh *mesh, const lite3dpp::String &savePath);

    lite3dpp::String makeFullPath(const lite3dpp::String &relative);
    lite3dpp::String makeRelativePath(const lite3dpp::String &inpath, 
        const lite3dpp::String &name, const lite3dpp::String &ext);
private:

    static void stub_mesh_loaded(lite3d_mesh *mesh, const kmMat4 *transform, const char *name, void *userdata);
    static lite3d_mesh *stub_mesh_init(void *userdata);
    static void stub_level_push(void *userdata);
    static void stub_level_pop(void *userdata);

    lite3dpp::String mOutputFolder;
    lite3dpp::String mObjectName;
    bool mOptimizeMesh;
    bool mFlipUV;
    bool mGenerateJson;
    int mNonameCounter;
    lite3d_mesh mMesh;
    lite3dpp::stl<lite3dpp::stl<lite3dpp::ConfigurationWriter>::vector>::stack mNodesLevels;
};
