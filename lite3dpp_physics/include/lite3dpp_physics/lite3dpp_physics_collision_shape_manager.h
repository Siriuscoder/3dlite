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
#include <lite3dpp/lite3dpp_mesh.h>
#include <lite3dpp_physics/lite3dpp_physics_common.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    class PhysicsTriangleCollisionShape : public Noncopiable
    {
    public:

        using Ptr = std::shared_ptr<PhysicsTriangleCollisionShape>;

        enum TriangleCollisionShapeType
        {
            ConvexHull = 0,
            StaticTriangleMesh = 1,
            GimpactTriangleMesh = 2,
            CollisionShapeTypeMaxCount
        };

        PhysicsTriangleCollisionShape(Main &main, const std::string_view& name, TriangleCollisionShapeType type);
        ~PhysicsTriangleCollisionShape();

        btCollisionShape *getCollisionShape();
        inline const String &getName() const
        { return mName; }

        void loadShape(const ConfigurationReader& conf);
        /* GimpactTriangleMesh поддерживает изменение геомерии на лету. 
           Для этого предлагается получить буферы с верщшинами и индексами вызывав 
           getCollisionMeshVertexData и getCollisionMeshIndexData, провести 
           там необходимые изменения и вызывать updateBounds */ 
        void updateBounds();

    private:

        void setupConvexHullCollisionShape(const ConfigurationReader& conf);
        void setupStaticTriangleMeshCollisionShape(const ConfigurationReader& conf);
        void setupGimpactTriangleMeshCollisionShape(const ConfigurationReader& conf);
        void setupTriangleMeshArray(const ConfigurationReader& conf);
        size_t validateChunkAndCalcVertexOffset(const lite3d_mesh_chunk *chunk);
        void purgeMeshData();

    private:

        Main &mMain;
        String mName;
        TriangleCollisionShapeType mCollisionShapeType;
        std::unique_ptr<btCollisionShape> mCollisionShape;
        std::unique_ptr<btTriangleIndexVertexArray> mCollisionMeshInfo;
        stl<BufferData>::vector mCollisionMeshVertexData;
        stl<BufferData>::vector mCollisionMeshIndexData;
    };

    class PhysicsCollisionShapeManager : public Noncopiable
    {
    public:

        using CollisionsShapeCache = stl<String, PhysicsTriangleCollisionShape::Ptr>::unordered_map;
        using CollisionsShapeCacheByType = stl<CollisionsShapeCache>::vector;

        PhysicsCollisionShapeManager(Main &main);
        ~PhysicsCollisionShapeManager() = default;

        btCollisionShape *getCollisionShape(PhysicsTriangleCollisionShape::TriangleCollisionShapeType type,
            const ConfigurationReader& collisionShapeConf);

        PhysicsTriangleCollisionShape *getCollisionShapeItem(PhysicsTriangleCollisionShape::TriangleCollisionShapeType type,
            const String& name);

        void clearCache();
        void removeCollisionShapeItem(PhysicsTriangleCollisionShape::TriangleCollisionShapeType type,
            const String& name);

    private:

        Main &mMain;
        CollisionsShapeCacheByType mCache;
    };
}}