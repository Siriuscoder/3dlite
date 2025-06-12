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
#include <lite3dpp/lite3dpp_skeleton.h>
#include <lite3dpp/lite3dpp_main.h>

#include <SDL_assert.h>
#include <SDL_log.h>

namespace lite3dpp
{
    Skeleton::Skeleton(MeshSceneNode &node) : 
        mNode(node)
    {}

    Skeleton::~Skeleton()
    {
        mNode.getMain()->getSkeletonBuffer().unregisterSceneNode(&mNode);
    }

    void Skeleton::loadVertexGroups(const ConfigurationReader& conf)
    {
        for (const auto &groupCfg : conf.getObjects(L"VertexGroups"))
        {
            mVertexGroups[groupCfg.getString(L"Name")] = groupCfg.getInt(L"Index");
        }

        mBonesTransformData.resize(mVertexGroups.size());
    }

    void Skeleton::loadBone(SkeletonBone *parent, const ConfigurationReader& conf)
    {
        String boneName = conf.getString(L"Name");
        auto indexIt = mVertexGroups.find(boneName);
        if (indexIt == mVertexGroups.end())
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Vertex group for bone '%s' is not found. Mesh '%s'",
                boneName.c_str(), mNode.getName().c_str());
        }

        auto inserted = mBones.try_emplace(boneName, 
            boneName,
            parent,
            indexIt == mVertexGroups.end() ? nullptr : &mBonesTransformData[indexIt->second],
            conf.getVec3(L"Head"),
            conf.getDouble(L"Length"),
            conf.getQuaternion(L"Rotation"));

        if (!inserted.second)
        {
            LITE3D_THROW("Duplicate bone name '" << boneName << "'. Mesh '" << mNode.getName() << "'");
        }

        for (const auto &boneCfg : conf.getObjects(L"Bones"))
        {
            loadBone(&inserted.first->second, boneCfg);
        }
    }

    void Skeleton::loadFromJson(const ConfigurationReader& conf)
    {
        SDL_assert(mNode.getMain());
        
        loadVertexGroups(conf);
        for (const auto &boneCfg : conf.getObjects(L"Skeleton"))
        {
            loadBone(nullptr, boneCfg);
        }

        mNode.getMain()->getSkeletonBuffer().registerSceneNode(&mNode);
        LITE3D_EXT_OBSERVER_NOTIFY_1(&mNode, updateSkeletonPose, &mNode);
    }

    void Skeleton::resetToRestPose()
    {
        for (auto &bone : mBones)
        {
            bone.second.resetToRestPose();
        }

        mNode.getMain()->getSkeletonBuffer().updateData(mBufferIndex, getTransformData());
        LITE3D_EXT_OBSERVER_NOTIFY_1(&mNode, updateSkeletonPose, &mNode);
    }

    void Skeleton::recalculate()
    {
        for (auto &bone : mBones)
        {
            if (bone.second.isEmptyParent())
            {
                bone.second.recalculateRecursive();
            }
        }

        mNode.getMain()->getSkeletonBuffer().updateData(mBufferIndex, getTransformData());
        LITE3D_EXT_OBSERVER_NOTIFY_1(&mNode, updateSkeletonPose, &mNode);
    }
}
