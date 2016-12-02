/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <functional>

#include <lite3d/lite3d_main.h>
#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    template<class T>
    class Observable
    {
    public:

        typedef typename stl<T *>::list Observers;

        inline Observers &getObservers()
        { return mObservers; }

        void addObserver(T *observer)
        {
            auto it = mObservers.begin();
            for (; it != mObservers.end(); ++it)
            {
                if (observer == *it)
                    LITE3D_THROW("Observer already exist");
            }

            mObservers.push_back(observer);
        }

        void removeObserver(T *observer)
        {
            auto it = mObservers.begin();
            for (; it != mObservers.end(); ++it)
            {
                if (observer == *it)
                {
                    mObservers.erase(it);
                    return;
                }
            }
        }

    private:

        Observers mObservers;
    };

    class LifecycleObserver
    {
    public:

        virtual ~LifecycleObserver() {}
        virtual void init() {}
        virtual void shut() {}
        virtual void frameBegin() {}
        virtual void frameEnd() {}
        virtual void timerTick(lite3d_timer *timerid) {}
        virtual void processEvent(SDL_Event *e) {}
    };

    class SceneObserver
    {
    public:

        virtual ~SceneObserver() {}
        
        virtual void beginDrawBatch(Scene *scene, 
            SceneNode *node, lite3d_mesh_chunk *meshChunk, Material *material) {}

        virtual void nodeInFrustum(Scene *scene, 
            SceneNode *node, lite3d_mesh_chunk *meshChunk, 
            Material *material, lite3d_bouding_vol *boudingVol, 
            Camera *camera) {}

        virtual void nodeOutOfFrustum(Scene *scene, 
            SceneNode *node, lite3d_mesh_chunk *meshChunk, 
            Material *material, lite3d_bouding_vol *boudingVol,
            Camera *camera) {}

        virtual void beginSceneRender(Scene *scene, Camera *camera) {}
        virtual void endSceneRender(Scene *scene, Camera *camera) {}
        virtual void beginFirstStageRender(Scene *scene, Camera *camera) {}
        virtual void beginSecondStageRender(Scene *scene, Camera *camera) {}
    };

#define LITE3D_EXT_OBSERVER_NOTIFY(obj, func)                               { for(auto o : (obj)->getObservers()) o->func(); }
#define LITE3D_EXT_OBSERVER_NOTIFY_1(obj, func, p1)                         { for(auto o : (obj)->getObservers()) o->func(p1); }
#define LITE3D_EXT_OBSERVER_NOTIFY_2(obj, func, p1, p2)                     { for(auto o : (obj)->getObservers()) o->func(p1, p2); }
#define LITE3D_EXT_OBSERVER_NOTIFY_3(obj, func, p1, p2, p3)                 { for(auto o : (obj)->getObservers()) o->func(p1, p2, p3); }
#define LITE3D_EXT_OBSERVER_NOTIFY_4(obj, func, p1, p2, p3, p4)             { for(auto o : (obj)->getObservers()) o->func(p1, p2, p3, p4); }
#define LITE3D_EXT_OBSERVER_NOTIFY_5(obj, func, p1, p2, p3, p4, p5)         { for(auto o : (obj)->getObservers()) o->func(p1, p2, p3, p4, p5); }
#define LITE3D_EXT_OBSERVER_NOTIFY_6(obj, func, p1, p2, p3, p4, p5, p6)     { for(auto o : (obj)->getObservers()) o->func(p1, p2, p3, p4, p5, p6); }

#define LITE3D_OBSERVER_NOTIFY(func)                                        LITE3D_EXT_OBSERVER_NOTIFY(this, func)
#define LITE3D_OBSERVER_NOTIFY_1(func, p1)                                  LITE3D_EXT_OBSERVER_NOTIFY_1(this, func, p1)
#define LITE3D_OBSERVER_NOTIFY_2(func, p1, p2)                              LITE3D_EXT_OBSERVER_NOTIFY_2(this, func, p1, p2)
#define LITE3D_OBSERVER_NOTIFY_3(func, p1, p2, p3)                          LITE3D_EXT_OBSERVER_NOTIFY_3(this, func, p1, p2, p3)
#define LITE3D_OBSERVER_NOTIFY_4(func, p1, p2, p3, p4)                      LITE3D_EXT_OBSERVER_NOTIFY_4(this, func, p1, p2, p3, p4)
#define LITE3D_OBSERVER_NOTIFY_5(func, p1, p2, p3, p4, p5)                  LITE3D_EXT_OBSERVER_NOTIFY_5(this, func, p1, p2, p3, p4, p5)
#define LITE3D_OBSERVER_NOTIFY_6(func, p1, p2, p3, p4, p5, p6)              LITE3D_EXT_OBSERVER_NOTIFY_6(this, func, p1, p2, p3, p4, p5, p6)

}

