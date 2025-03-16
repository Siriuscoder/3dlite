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

#include <algorithm>
#include <functional>

namespace lite3dpp
{
    Action::Action(const String &name, const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::ACTION)
    {}

    std::unique_ptr<ActionClip> Action::playAction(SceneNodeBase *node, bool cycle)
    {
        auto clip = std::make_unique<ActionClip>(*this, getMain(), node);
        clip->setCycle(cycle);
        clip->play();
        return clip;
    }

    void Action::loadFromConfigImpl(const ConfigurationReader &config)
    {
        mMinFrame = config.getDouble(L"MinFrame");
        mMaxFrame = config.getDouble(L"MaxFrame");
        loadKeyFrames(config.getObject(L"Frames"), mKeyFrames);

        config.getObject(L"SkeletonFrames").enumerateObjects([this](const WString &name, const ConfigurationReader &boneCfg)
        {
            stl<KeyFrame>::vector boneKeyFrames;
            loadKeyFrames(boneCfg, boneKeyFrames);

            if (boneKeyFrames.size() > 0)
            {
                String boneName(name.begin(), name.end());
                mSkeletonKeyFrames[boneName] = boneKeyFrames;
            }
        });
    }

    void Action::unloadImpl()
    {}

    void Action::loadKeyFrames(const ConfigurationReader &config, stl<KeyFrame>::vector &keyFrames)
    {
        config.enumerateObjects([&keyFrames](const WString &name, const ConfigurationReader &keyFrameCfg)
        {
            KeyFrame keyFrame;
            keyFrame.frameNo = std::stof(name);

            if (keyFrameCfg.has(L"location"))
            {
                KeyFrame::ChannelValue value = {0};
                value.position = keyFrameCfg.getVec3(L"location");
                keyFrame.channels.emplace_back(std::make_tuple(KeyFrame::Channel::Location, value));
            }
            
            if (keyFrameCfg.has(L"rotation_quaternion"))
            {
                KeyFrame::ChannelValue value = {0};
                value.rotation = keyFrameCfg.getQuaternion(L"rotation_quaternion");
                keyFrame.channels.emplace_back(std::make_tuple(KeyFrame::Channel::Rotation, value));
            }
            
            if (keyFrameCfg.has(L"scale"))
            {
                KeyFrame::ChannelValue value = {0};
                value.scale = keyFrameCfg.getVec3(L"scale");
                keyFrame.channels.emplace_back(std::make_tuple(KeyFrame::Channel::Scale, value));
            }

            keyFrames.emplace_back(keyFrame);
        });

        // В экспорте кадры отсортированы, но сделаем еще раз на всякий случай
        std::sort(keyFrames.begin(), keyFrames.end(), [](const KeyFrame &a, const KeyFrame &b)
        {
            return a.frameNo < b.frameNo;
        });
    }

    Action::LeftRightFrame Action::getLeftRightFrameByTime(float time, const stl<KeyFrame>::vector &frames) const
    {
        // Кадров вообще нет, не должно быть такого
        if (frames.size() == 0)
        {
            return std::make_pair(nullptr, nullptr); 
        }

        // ищем ближайший следующий кадр
        KeyFrame marker = {time}; 
        auto frameIt = std::upper_bound(frames.begin(), frames.end(), marker, [](const KeyFrame &a, const KeyFrame &b) 
        {
            return a.frameNo < b.frameNo;
        });

        // Время вышло за пределы анимации
        if (frameIt == frames.end())
        {
            return std::make_pair(&frames.back(), nullptr); 
        }
        // Анимация еще не наступила
        else if (frameIt == frames.begin())
        {
            return std::make_pair(nullptr, &frames.front()); 
        }

        return std::make_pair(&(*std::prev(frameIt)), &(*frameIt));
    }

    Action::LeftRightFrame Action::getLeftRightFrameByTime(float time) const
    {
        return getLeftRightFrameByTime(time, mKeyFrames);
    }

    Action::LeftRightFrame Action::getLeftRightFrameBoneByTime(float time, const String &boneName) const
    {
        const auto &boneKeyFrames = mSkeletonKeyFrames.find(boneName);
        if (boneKeyFrames != mSkeletonKeyFrames.end())
        {
            return getLeftRightFrameByTime(time, boneKeyFrames->second);
        }

        return std::make_pair(nullptr, nullptr); 
    }
}
