/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#pragma once

#include <lite3dpp_pipeline/lite3dpp_pipeline_base.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    class LITE3DPP_PIPELINE_EXPORT PipelineForward : public PipelineBase
    {
    public:

        PipelineForward(const String &name, const String &path, Main *main);

    protected:
        
        void constructCameraPipeline(const ConfigurationReader &pipelineConfig, const String &cameraName,
            SceneGenerator &sceneGenerator) override;

        virtual void constructCombinedPass(const ConfigurationReader &pipelineConfig, const String &cameraName,
            SceneGenerator &sceneGenerator);

    protected:

    };
}}
