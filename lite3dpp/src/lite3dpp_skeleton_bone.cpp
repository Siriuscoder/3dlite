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
#include <lite3dpp/lite3dpp_skeleton_bone.h>

namespace lite3dpp
{
    SkeletonBone::SkeletonBone(const String &name, 
        SkeletonBone *parent, 
        kmMat4 *matrixBuffer, 
        const kmVec3 &head, 
        float length,
        const kmVec3 &restPosePosition,
        const kmQuaternion &restPoseRotation) : 
        mName(name),
        mParent(parent),
        mTransformFromRest(matrixBuffer),
        mHead(head),
        mLength(length),
        mRestPosePosition(restPosePosition),
        mRestPoseRotation(restPoseRotation)
    {
        // Caclulate bone rest pose local transform
        kmMat4 translate;
        kmMat4Translation(&translate, restPosePosition.x, restPosePosition.y, restPosePosition.z);
        kmMat4RotationQuaternion(&mRestPoseTransform, &restPoseRotation);
        kmMat4Multiply(&mRestPoseTransform, &translate, &mRestPoseTransform);
        // Caclulate bone rest pose skeleton space transform
        if (parent)
        {
            kmMat4Multiply(&mRestPoseTransform, &parent->mRestPoseTransform, &mRestPoseTransform);
        }
        // Caclulate bone rest pose skeleton space inverse transform
        if (!kmMat4Inverse(&mRestPoseTransformInverse, &mRestPoseTransform))
        {
            mRestPoseTransformInverse = mRestPoseTransform;
        }

        resetToRestPose();
    }

    void SkeletonBone::setPosition(const kmVec3 &position)
    {
        mPosition = position;
        mNeedRecalc = true;
    }

    const kmVec3& SkeletonBone::getPosition() const
    {
        return mPosition;
    }

    void SkeletonBone::setRotation(const kmQuaternion &rotation)
    {
        mRotation = rotation;
        mNeedRecalc = true;
    }

    const kmQuaternion& SkeletonBone::getRotation() const
    {
        return mRotation;
    }

    void SkeletonBone::setScale(const kmVec3 &scale)
    {
        mScale = scale;
        mNeedRecalc = true;
    }

    const kmVec3 &SkeletonBone::getScale() const
    {
        return mScale;
    }

    void SkeletonBone::addChildBone(SkeletonBone *bone)
    {
        mChildBones.emplace_back(bone);
    }

    void SkeletonBone::recalculateRecursive(bool force)
    {
        if (mNeedRecalc || force)
        {
            // Caclulate bone local transform
            kmMat4 translate;
            kmMat4TranslationScale(&translate, &mPosition, &mScale);
            kmMat4RotationQuaternion(&mTransform, &mRotation);
            kmMat4Multiply(&mTransform, &translate, &mTransform);
         
            // Caclulate bone skeleton space transform
            if (mParent)
            {
                kmMat4Multiply(&mTransform, &mParent->mTransform, &mTransform);
            }

            if (mTransformFromRest)
            {
                // Final bone transform matrix, will be loaded to GPU
                kmMat4Multiply(mTransformFromRest, &mTransform, &mRestPoseTransformInverse);
            }

            mNeedRecalc = false;
            force = true;
        }

        for (SkeletonBone *bone : mChildBones)
        {
            bone->recalculateRecursive(force);
        }
    }

    void SkeletonBone::resetToRestPose()
    {
        setPosition(mRestPosePosition);
        setRotation(mRestPoseRotation);
        setScale(KM_VEC3_ONE);

        mTransform = mRestPoseTransform;
        if (mTransformFromRest)
        {
            kmMat4Identity(mTransformFromRest);
        }
    }
}
