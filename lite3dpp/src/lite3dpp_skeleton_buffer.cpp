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
#include <lite3dpp/lite3dpp_skeleton_buffer.h>
#include <lite3dpp/lite3dpp_main.h>

#include <SDL_assert.h>

namespace lite3dpp
{
    SkeletonBuffer::SkeletonBuffer(Main &main) : 
        mMain(main)
    {}

    SSBO &SkeletonBuffer::getBuffer()
    {
        if (!mGlobalSkeletonBuffer)
        {
            mGlobalSkeletonBuffer = mMain.getResourceManager().
                queryResourceFromJson<SSBO>("GlobalSkeletonBuffer", "{\"Dynamic\": true}");
        }

        SDL_assert(mGlobalSkeletonBuffer);
        return *mGlobalSkeletonBuffer;
    }

    void SkeletonBuffer::registerSceneNode(MeshSceneNode *node)
    {
        SDL_assert(node);

        if (!node->getSkeleton())
        {
            return;
        }

        auto it = mNodes.emplace(node);
        if (it.second)
        {
            getBuffer();

            size_t bufferIndex = mUsedBytes / sizeof(Skeleton::BonesTransformData::value_type);
            size_t sizeBytes = node->getSkeleton()->getTransformData().size() * 
                sizeof(Skeleton::BonesTransformData::value_type);

            if ((mUsedBytes + sizeBytes) > mGlobalSkeletonBuffer->bufferSizeBytes())
            {
                mGlobalSkeletonBuffer->extendBufferBytes(sizeBytes);
                mUsedBytes += sizeBytes;
            }

            node->setSkeletonBufferIndex(static_cast<int32_t>(bufferIndex));
            updateData(bufferIndex, node->getSkeleton()->getTransformData());
        }
    }

    void SkeletonBuffer::unregisterSceneNode(MeshSceneNode *node)
    {
        SDL_assert(node);

        if (!node->getSkeleton())
        {
            return;
        }

        if (mNodes.erase(node) > 0)
        {
            size_t sizeBytes = node->getSkeleton()->getBonesCount() * 
                sizeof(Skeleton::BonesTransformData::value_type);
            mPendingRemoveBytes += sizeBytes;
        }
    }

    void SkeletonBuffer::unregisterAll()
    {
        if (mGlobalSkeletonBuffer)
        {
            mMain.getResourceManager().releaseResource(mGlobalSkeletonBuffer->getName());
            mGlobalSkeletonBuffer = nullptr;
        }

        mNodes.clear();
    }

    void SkeletonBuffer::updateData(size_t index, const Skeleton::BonesTransformData &data)
    {
        size_t offset = index * sizeof(Skeleton::BonesTransformData::value_type);
        size_t dataSize = data.size() * sizeof(Skeleton::BonesTransformData::value_type);

        SDL_assert(mGlobalSkeletonBuffer);
        SDL_assert((offset + dataSize) <= mGlobalSkeletonBuffer->bufferSizeBytes());

        mGlobalSkeletonBuffer->setData(&data[0], offset, dataSize);
    }
}
