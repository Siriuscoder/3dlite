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

#include <lite3dpp_pipeline/lite3dpp_pipeline_deffered.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

    template<class PipelineImpl, class SceneImpl>
    class CustomScenePipeline : public PipelineImpl
    {
    public:

        using SceneType = typename SceneImpl;
        using PipelineType = typename PipelineImpl;

        CustomScenePipeline(const String &name, const String &path, Main *main) : 
            SceneType(name, path, main)
        {}

        void createMainScene(const std::String& name, const std::String& sceneConfig) override
        {
            mMainScene = getMain().getResourceManager()->queryResourceFromJson<SceneType>(name, sceneConfig);
        }

        SceneImpl &getMainScene()
        {
            return static_cast<SceneType&>(PipelineType::getMainScene());
        }
    };
}}
