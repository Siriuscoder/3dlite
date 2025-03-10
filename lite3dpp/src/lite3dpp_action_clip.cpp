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
#include <lite3dpp/lite3dpp_action_clip.h>
#include <lite3dpp/lite3dpp_scene_node_base.h>
#include <lite3dpp/lite3dpp_action.h>
#include <lite3dpp/lite3dpp_main.h>

#include <SDL_assert.h>

namespace lite3dpp
{
    ActionClip::ActionClip(const Action &action, Main &main, SceneNodeBase *node) : 
        mAction(action),
        mMain(main),
        mNode(node)
    {
        SDL_assert(node);
        mMain.addObserver(this);

        mInitialPosition = node->getPosition();
        mInitialRotation = node->getRotation();
        mInitialScale = node->getScale();
    }

    ActionClip::~ActionClip()
    {
        mMain.removeObserver(this);
        reset();
    }

    template<class Node, class KeyFrame>
    kmVec3 ActionClip::consolidatePosition(Node *node, const KeyFrame *keyFrame, bool &updated)
    {
        auto position = node->getPosition();
        updated = false;

        for (const auto &channel : keyFrame->channels)
        {
            switch (std::get<0>(channel))
            {
                case Action::KeyFrame::Channel::LocationX:
                    position.x = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::LocationY:
                    position.y = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::LocationZ:
                    position.z = std::get<1>(channel);
                    updated = true;
                    break;
                default:
                    break;
            }
        }

        return position;
    }

    template<class Node, class KeyFrame>
    kmQuaternion ActionClip::consolidateRotation(Node *node, const KeyFrame *keyFrame, bool &updated)
    {
        auto rotation = node->getRotation();
        updated = false;

        for (const auto &channel : keyFrame->channels)
        {
            switch (std::get<0>(channel))
            {
                case Action::KeyFrame::Channel::RotationQX:
                    rotation.x = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::RotationQY:
                    rotation.y = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::RotationQZ:
                    rotation.z = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::RotationQW:
                    rotation.w = std::get<1>(channel);
                    updated = true;
                    break;
                default:
                    break;
            }
        }

        return rotation;
    }

    template<class Node, class KeyFrame>
    kmVec3 ActionClip::consolidateScale(Node *node, const KeyFrame *keyFrame, bool &updated)
    {
        auto scale = node->getScale();
        updated = false;

        for (const auto &channel : keyFrame->channels)
        {
            switch (std::get<0>(channel))
            {
                case Action::KeyFrame::Channel::ScaleX:
                    scale.x = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::ScaleY:
                    scale.y = std::get<1>(channel);
                    updated = true;
                    break;
                case Action::KeyFrame::Channel::ScaleZ:
                    scale.z = std::get<1>(channel);
                    updated = true;
                    break;
                default:
                    break;
            }
        }

        return scale;
    }

    template<class Node, class LeftRightFrame>
    void ActionClip::interpolate(Node *node, const LeftRightFrame &leftRightFrame)
    {
        // Еще не дошли до первого ключевого кадра
        if (!leftRightFrame.first && leftRightFrame.second)
        {
            bool updated = false;
            node->setPosition(consolidatePosition(node, leftRightFrame.second, updated));
            node->setRotation(consolidateRotation(node, leftRightFrame.second, updated));
            node->setScale(consolidateScale(node, leftRightFrame.second, updated));
        }
        // Последний ключевой кадр и дальше
        else if (leftRightFrame.first && !leftRightFrame.second)
        {
            bool updated = false;
            node->setPosition(consolidatePosition(node, leftRightFrame.first, updated));
            node->setRotation(consolidateRotation(node, leftRightFrame.first, updated));
            node->setScale(consolidateScale(node, leftRightFrame.first, updated));
        }
        // Между 2мя кадрами 
        else
        {
            float k = (mTime - leftRightFrame.first->frameNo) / 
                (leftRightFrame.second->frameNo - leftRightFrame.first->frameNo);

            bool updatedLeft = false, updatedRight = false;
            auto positionLeft = consolidatePosition(node, leftRightFrame.first, updatedLeft);
            auto positionRight = consolidatePosition(node, leftRightFrame.second, updatedRight);
            if (updatedLeft || updatedRight)
            {
                kmVec3 interpolated;
                kmVec3Lerp(&interpolated, &positionLeft, &positionRight, k);
                node->setPosition(interpolated);
            }

            auto rotationLeft = consolidateRotation(node, leftRightFrame.first, updatedLeft);
            auto rotationRight = consolidateRotation(node, leftRightFrame.second, updatedRight);
            if (updatedLeft || updatedRight)
            {
                kmQuaternion interpolated;
                kmQuaternionSlerp(&interpolated, &rotationLeft, &rotationRight, k);
                node->setRotation(interpolated);
            }

            auto scaleLeft = consolidateScale(node, leftRightFrame.first, updatedLeft);
            auto scaleRight = consolidateScale(node, leftRightFrame.second, updatedRight);
            if (updatedLeft || updatedRight)
            {
                kmVec3 interpolated;
                kmVec3Lerp(&interpolated, &scaleLeft, &scaleRight, k);
                node->setScale(interpolated);
            }
        }
    }

    void ActionClip::timerTick(lite3d_timer *timer)
    {
        if (!mPlaying)
            return;

        if (mMain.getFixedUpdateTimer() == timer)
        {
            if (mTime >= mAction.getMaxFrame())
            {
                if (!mCycle)
                {
                    return;
                }

                mTime = 0.0;
            }

            // Сколько реальных кадров прошло с прошлого вызова таймера 
            mTime += static_cast<float>(static_cast<double>(timer->deltaMcs) / (timer->interval * 1000.0));
            interpolate(mNode, mAction.getLeftRightFrameByTime(mTime));
        }
    }

    void ActionClip::play()
    {
        mTime = 0.0f;
        mPlaying = true;
    }

    void ActionClip::pause()
    {
        mPlaying = false;
    }

    void ActionClip::resume()
    {
        mPlaying = true;
    }

    void ActionClip::reset()
    {
        mTime = 0.0f;
        mPlaying = false;
        mNode->setPosition(mInitialPosition);
        mNode->setRotation(mInitialRotation);
        mNode->setScale(mInitialScale);
    }

    bool ActionClip::completed() const
    {
        return !mCycle && mTime >= mAction.getMaxFrame();
    }
}
