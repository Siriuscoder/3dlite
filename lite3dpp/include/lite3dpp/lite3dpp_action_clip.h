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
#include <lite3dpp/lite3dpp_observer.h>
#include <lite3dpp/lite3dpp_skeleton.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ActionClip : public LifecycleObserver, public Noncopiable
    {
    public:

        ActionClip(const Action &action, Main &main, SceneNodeBase *node);
        virtual ~ActionClip();

        void play();
        void pause();
        void resume();
        void reset();
        // Will never be completed if cycle = true
        bool completed() const;
        void setCycle(bool cycle)
        { mCycle = cycle; }

    protected:

        virtual void timerTick(lite3d_timer *timerid) override;
        // Interpolate between two frames, node may be SceneNode, Bone or something else with 
        // get/set Position Rotation Scale
        template<class Node, class LeftRightFrame>
        void interpolate(Node *node, const LeftRightFrame& leftRightFrame);
        template<class Node, class KeyFrame>
        kmVec3 consolidatePosition(Node *node, const KeyFrame *keyFrame, bool &updated);
        template<class Node, class KeyFrame>
        kmQuaternion consolidateRotation(Node *node, const KeyFrame *keyFrame, bool &updated);
        template<class Node, class KeyFrame>
        kmVec3 consolidateScale(Node *node, const KeyFrame *keyFrame, bool &updated);

    protected:

        const Action &mAction;
        Main &mMain;
        SceneNodeBase *mNode;
        bool mCycle = false;
        kmVec3 mInitialPosition;
        kmQuaternion mInitialRotation;
        kmVec3 mInitialScale;
        float mTime = 0.0f;
        bool mPlaying = false;
    };
}
