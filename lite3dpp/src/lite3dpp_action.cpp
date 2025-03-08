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
#include <lite3dpp/lite3dpp_action.h>
#include <lite3dpp/lite3dpp_scene_node_base.h>

namespace lite3dpp
{
    Action::Action(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::ACTION)
    {}

    std::unique_ptr<ActionClip> Action::playAction(SceneNodeBase *node, bool cycle)
    {
        //auto skeleton = node->getSkeleton();
        auto clip = std::make_unique<ActionClip>(*this, getMain(), node);
        clip->setCycle(cycle);
        clip->play();
        return clip;
    }

    void Action::loadFromConfigImpl(const ConfigurationReader &config)
    {

    }

    void Action::unloadImpl()
    {

    }
}
