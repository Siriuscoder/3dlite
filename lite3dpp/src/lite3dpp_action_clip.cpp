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
#include <SDL_log.h>

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
        updated = false;
        for (const auto &channel : keyFrame->channels)
        {
            switch (std::get<0>(channel))
            {
                case Action::KeyFrame::Channel::Location:
                    updated = true;
                    return std::get<1>(channel).position;
                default:
                    break;
            }
        }

        return node->getPosition();
    }

    template<class Node, class KeyFrame>
    kmQuaternion ActionClip::consolidateRotation(Node *node, const KeyFrame *keyFrame, bool &updated)
    {
        updated = false;
        for (const auto &channel : keyFrame->channels)
        {
            switch (std::get<0>(channel))
            {
                case Action::KeyFrame::Channel::Rotation:
                    updated = true;
                    return std::get<1>(channel).rotation;
                default:
                    break;
            }
        }

        return node->getRotation();;
    }

    template<class Node, class KeyFrame>
    kmVec3 ActionClip::consolidateScale(Node *node, const KeyFrame *keyFrame, bool &updated)
    {
        updated = false;
        for (const auto &channel : keyFrame->channels)
        {
            switch (std::get<0>(channel))
            {
                case Action::KeyFrame::Channel::Scale:
                    updated = true;
                    return std::get<1>(channel).scale;
                default:
                    break;
            }
        }

        return node->getScale();
    }

    template<class Node, class FramePair>
    void ActionClip::interpolate(Node *node, const FramePair &framePair)
    {
        // Еще не дошли до первого ключевого кадра
        if (!framePair.first && framePair.second)
        {
            bool updated = false;
            node->setPosition(consolidatePosition(node, framePair.second, updated));
            node->setRotation(consolidateRotation(node, framePair.second, updated));
            node->setScale(consolidateScale(node, framePair.second, updated));
        }
        // Последний ключевой кадр и дальше
        else if (framePair.first && !framePair.second)
        {
            bool updated = false;
            node->setPosition(consolidatePosition(node, framePair.first, updated));
            node->setRotation(consolidateRotation(node, framePair.first, updated));
            node->setScale(consolidateScale(node, framePair.first, updated));
        }
        // Между 2мя кадрами 
        else
        {
            float k = (mTime - framePair.first->frameNo) / 
                (framePair.second->frameNo - framePair.first->frameNo);

            bool updatedLeft = false, updatedRight = false;
            auto positionLeft = consolidatePosition(node, framePair.first, updatedLeft);
            auto positionRight = consolidatePosition(node, framePair.second, updatedRight);
            if (updatedLeft || updatedRight)
            {
                kmVec3 interpolated;
                kmVec3Lerp(&interpolated, &positionLeft, &positionRight, k);
                node->setPosition(interpolated);
            }

            auto rotationLeft = consolidateRotation(node, framePair.first, updatedLeft);
            auto rotationRight = consolidateRotation(node, framePair.second, updatedRight);
            if (updatedLeft || updatedRight)
            {
                kmQuaternion interpolated;
                kmQuaternionSlerp(&interpolated, &rotationLeft, &rotationRight, k);
                node->setRotation(interpolated);
            }

            auto scaleLeft = consolidateScale(node, framePair.first, updatedLeft);
            auto scaleRight = consolidateScale(node, framePair.second, updatedRight);
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
        if (mState != ActionClipState::PLAYING)
            return;

        if (mMain.getFixedUpdateTimer() == timer)
        {
            if (mTime >= mAction.getMaxFrame())
            {
                if (!mCycle)
                {
                    mState = ActionClipState::COMPLETED;
                    return;
                }

                mTime = 0.0;
            }

            // Сколько реальных кадров прошло с прошлого вызова таймера 
            mTime += static_cast<float>(static_cast<double>(timer->deltaMcs) / (timer->interval * 1000.0));
            //SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "mTime %f delta", mTime);
            interpolate(mNode, mAction.getFramePairByTime(mTime));
        }
    }

    void ActionClip::play()
    {
        mTime = 0.0f;
        mState = ActionClipState::PLAYING;
    }

    void ActionClip::pause()
    {
        mState = ActionClipState::PAUSED;
    }

    void ActionClip::resume()
    {
        mState = ActionClipState::PLAYING;
    }

    void ActionClip::reset()
    {
        mTime = 0.0f;
        mState = ActionClipState::STOPPED;
        mNode->setPosition(mInitialPosition);
        mNode->setRotation(mInitialRotation);
        mNode->setScale(mInitialScale);
    }
}
