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
#pragma once

#include <lite3dpp_pipeline/lite3dpp_pipeline_deffered.h>
#include <lite3dpp_pipeline/lite3dpp_pipeline_forward.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    template<class PipelineImpl, class SceneImpl>
    class CustomScenePipeline : public PipelineImpl
    {
    public:

        using SceneType = SceneImpl;
        using PipelineType = PipelineImpl;

        CustomScenePipeline(const String &name, const String &path, Main *main) : 
            PipelineType(name, path, main)
        {}

        SceneImpl &getMainScene()
        {
            return static_cast<SceneType&>(PipelineType::getMainScene());
        }

    protected:

        void createMainScene(const String& name, const String& sceneConfig) override
        {
            PipelineType::mMainScene = PipelineType::getMain().getResourceManager()->
                template queryResourceFromJson<SceneType>(name, sceneConfig);
        }
    };
}}
