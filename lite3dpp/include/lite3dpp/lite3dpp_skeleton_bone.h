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
    class LITE3DPP_EXPORT SkeletonBone
    {
    public:

        using ChildBones = stl<SkeletonBone *>::vector;

        SkeletonBone(const String &name, 
            SkeletonBone *parent,
            kmMat4 *matrixBuffer, 
            const kmVec3 &restPosePosition,
            const kmQuaternion &restPoseRotation);

        inline const String &getName() const 
        { return mName; }
        void setPosition(const kmVec3 &position);
        const kmVec3& getPosition() const;
        void setRotation(const kmQuaternion &rotation);
        const kmQuaternion& getRotation() const;
        void setScale(const kmVec3 &scale);
        const kmVec3 &getScale() const;
        inline bool isEmptyParent() const
        { return !mParent; }

        void addChildBone(SkeletonBone *bone);
        void recalculateRecursive(bool force = false);
        void resetToRestPose();

    private:

        String mName;
        SkeletonBone *mParent;
        kmMat4 *mTransformFromRest;
        kmMat4 mTransform;
        kmMat4 mRestPoseTransform;
        kmMat4 mRestPoseTransformInverse;
        kmVec3 mPosition;
        kmVec3 mRestPosePosition;
        kmQuaternion mRotation;
        kmQuaternion mRestPoseRotation;
        kmVec3 mScale = KM_VEC3_ONE;
        ChildBones mChildBones;
        bool mNeedRecalc = true;
    };
}
