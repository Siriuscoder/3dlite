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
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_skeleton.h>
#include <lite3dpp/lite3dpp_action_clip.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Action : public ConfigurableResource, public Noncopiable
    {
    public:

        struct KeyFrame
        {
            enum class Channel
            {
                Location,
                Rotation,
                Scale
            };

            union ChannelValue
            {
                kmVec3 position;
                kmQuaternion rotation;
                kmVec3 scale;
            };

            friend bool operator<(const KeyFrame& l, const KeyFrame& r)
            {
                return l.frameNo < r.frameNo;
            }

            float frameNo;
            stl<std::tuple<Channel, ChannelValue>>::vector channels;
        };

        using FramePair = std::pair<const KeyFrame *, const KeyFrame *>;

        Action(const String &name, const String &path, Main &main);
        virtual ~Action() = default;

        inline float getMinFrame() const
        { return mMinFrame; }
        inline float getMaxFrame() const
        { return mMaxFrame; }
        inline void setSkeleton(Skeleton *skeleton)
        { mSkeleton = skeleton; }

        virtual std::unique_ptr<ActionClip> playAction(SceneNodeBase &node, bool cycle);
        FramePair getFramePairByTime(float time) const;
        FramePair getFramePairBoneByTime(float time, const String &boneName) const;

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &config) override;
        virtual void unloadImpl() override;

        static void loadKeyFrames(const ConfigurationReader &config, stl<KeyFrame>::vector &keyFrames);
        FramePair getFramePairByTime(float time, const stl<KeyFrame>::vector &frames) const;

    protected:

        stl<KeyFrame>::vector mKeyFrames;
        stl<String, stl<KeyFrame>::vector>::unordered_map mSkeletonKeyFrames;
        float mMinFrame = 0;
        float mMaxFrame = 0;
        Skeleton *mSkeleton = nullptr;
    };
}
