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

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Skeleton
    {
    public:

        using BonesTransformData = stl<kmMat4>::vector;

        Skeleton(MeshSceneNode &node);
        ~Skeleton();

        void loadFromJson(const ConfigurationReader& conf);

        size_t getBonesCount() const;
        inline void setBufferIndex(int32_t index)
        { mBufferIndex = index; }

    private:

        MeshSceneNode &mNode;
        int32_t mBufferIndex = 0;
        BonesTransformData mBonesTransformData;

    };
}
