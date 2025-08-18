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
#include <lite3dpp/lite3dpp_skeleton_bone.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Skeleton
    {
    public:

        using BonesTransformData = stl<kmMat4>::vector; // bones gtransform matrices buffer CPU side 
        using VertexGroups = stl<String, int32_t>::map;
        using Bones = stl<String, SkeletonBone>::map;

        Skeleton(MeshSceneNode &node);
        ~Skeleton();

        void loadFromJson(const ConfigurationReader& conf);
        void resetToRestPose();
        void recalculate();

        inline size_t getBonesCount() const
        { return mBones.size(); }
        inline Bones &getBones() 
        { return mBones; }
        inline const BonesTransformData &getTransformData() const
        { return mBonesTransformData; }
        inline void setBufferIndex(int32_t index)
        { mBufferIndex = index; }
    

    private:

        void loadVertexGroups(const ConfigurationReader& conf);
        void loadBone(SkeletonBone *parent, const ConfigurationReader& conf);

    private:

        MeshSceneNode &mNode;
        int32_t mBufferIndex = 0;
        BonesTransformData mBonesTransformData;
        VertexGroups mVertexGroups;
        Bones mBones;
    };
}
