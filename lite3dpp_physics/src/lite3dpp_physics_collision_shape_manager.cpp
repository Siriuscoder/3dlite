/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#include <lite3dpp_physics/lite3dpp_physics_collision_shape_manager.h>

#include <SDL_assert.h>
#include <lite3dpp_physics/lite3dpp_physics_bullet.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsCollisionShapeManager::PhysicsCollisionShapeManager(Main &main) : 
        mMain(main)
    {
        mCache.resize(PhysicsTriangleCollisionShape::CollisionShapeTypeMaxCount);
    }

    btCollisionShape *PhysicsCollisionShapeManager::getCollisionShape(
        PhysicsTriangleCollisionShape::TriangleCollisionShapeType type,
        const ConfigurationReader& collisionShapeConf)
    {
        if (!collisionShapeConf.has(L"CollisionMesh"))
        {
            LITE3D_THROW("CollisionMesh is empty");
        }

        auto name = collisionShapeConf.getObject(L"CollisionMesh").getString(L"Name");
        {
            auto shapeItemPtr = getCollisionShapeItem(type, name);
            if (shapeItemPtr)
            {
                return shapeItemPtr->getCollisionShape();
            }
        }

        auto shapeItem = std::make_shared<PhysicsTriangleCollisionShape>(mMain, name, type);
        shapeItem->loadShape(collisionShapeConf);

        mCache[type].emplace(name, shapeItem);
        return shapeItem->getCollisionShape();
    }

    PhysicsTriangleCollisionShape *PhysicsCollisionShapeManager::getCollisionShapeItem(
        PhysicsTriangleCollisionShape::TriangleCollisionShapeType type,
        const String& name)
    {
        if (type >= PhysicsTriangleCollisionShape::CollisionShapeTypeMaxCount)
        {
            LITE3D_THROW("Unknown collision mesh type '" << type << "' for mesh '" << name << "'");
        }

        auto cacheIt = mCache[type].find(name);
        if (cacheIt != mCache[type].end())
        {
            return cacheIt->second.get();
        }

        return nullptr;
    }

    void PhysicsCollisionShapeManager::clearCache()
    {
        for (auto &itemType : mCache)
        {
            itemType.clear();
        }
    }

    void PhysicsCollisionShapeManager::removeCollisionShapeItem(PhysicsTriangleCollisionShape::TriangleCollisionShapeType type,
        const String& name)
    {
        if (type >= PhysicsTriangleCollisionShape::CollisionShapeTypeMaxCount)
        {
            LITE3D_THROW("Unknown collision mesh type '" << type << "' for mesh '" << name << "'");
        }

        auto cacheIt = mCache[type].find(name);
        if (cacheIt != mCache[type].end())
        {
            mCache[type].erase(cacheIt);
        }
    }

    PhysicsTriangleCollisionShape::PhysicsTriangleCollisionShape(Main &main, const std::string_view& name, 
        TriangleCollisionShapeType type) : 
        mMain(main),
        mName(name),
        mCollisionShapeType(type)
    {}

    PhysicsTriangleCollisionShape::~PhysicsTriangleCollisionShape()
    {
        mCollisionShape.reset();
        purgeMeshData();
    }

    btCollisionShape *PhysicsTriangleCollisionShape::getCollisionShape()
    {
        SDL_assert(mCollisionShape);
        return mCollisionShape.get();
    }

    void PhysicsTriangleCollisionShape::loadShape(const ConfigurationReader& conf)
    {
        setupTriangleMeshArray(conf);
        switch (mCollisionShapeType)
        {
            case ConvexHull:
                setupConvexHullCollisionShape(conf);
                break;
            case StaticTriangleMesh:
                setupStaticTriangleMeshCollisionShape(conf);
                break;
            case GimpactTriangleMesh:
                setupGimpactTriangleMeshCollisionShape(conf);
                break;
            default:
            {
                SDL_assert(false);
                break;
            }
        }
    }

    size_t PhysicsTriangleCollisionShape::validateChunkAndCalcVertexOffset(const lite3d_mesh_chunk *chunk)
    {
        size_t offset = 0;
        if (!chunk->hasIndexes)
        {
            LITE3D_THROW("Not indexed mesh '" << getName() << "', only indexed meshes supported");
        }

        const lite3d_vao_layout *layout = static_cast<const lite3d_vao_layout *>(chunk->layout.data);
        for (uint32_t i = 0; i < chunk->layout.size; ++i)
        {
            if (layout[i].binding != LITE3D_BUFFER_BINDING_VERTEX)
            {
                offset += layout[i].count * sizeof(float);
            }
            else
            {
                return offset;
            }
        }

        LITE3D_THROW("Vertex data not found for mesh '" << getName() << "'...");
        return 0;
    }
    
    void PhysicsTriangleCollisionShape::setupTriangleMeshArray(const ConfigurationReader& conf)
    {
        /* load collision mesh into GPU memory */
        auto rawMesh = mMain.getResourceManager()->queryResource<Mesh>(getName(),
            conf.getObject(L"CollisionMesh").getString(L"Mesh"));

        if (rawMesh->chunksCount() == 0)
        {
            LITE3D_THROW("Collision mesh '" << getName() << "' is empty...");
        }

        mCollisionMeshInfo = std::make_unique<btTriangleIndexVertexArray>();
        mCollisionMeshVertexData.resize(rawMesh->chunksCount());
        mCollisionMeshIndexData.resize(rawMesh->chunksCount());

        for (size_t i = 0; i < rawMesh->chunksCount(); ++i)
        {
            auto chunkEntity = rawMesh->getChunk(i);

            /* load chunk from GPU to host memory */
            rawMesh->getPartition()->vertexBuffer().getDataBuffer(mCollisionMeshVertexData[i], 
                chunkEntity.chunk->vao.verticesOffset,
                chunkEntity.chunk->vao.verticesSize);
                
            rawMesh->getPartition()->indexBuffer().getDataBuffer(mCollisionMeshIndexData[i], 
                chunkEntity.chunk->vao.indexesOffset,
                chunkEntity.chunk->vao.indexesSize);

            /* calc vertices offset inside chunk */
            auto vertexOffset = validateChunkAndCalcVertexOffset(chunkEntity.chunk);

            btIndexedMesh triangleArrayChunk;
            triangleArrayChunk.m_indexType = PHY_ScalarType::PHY_INTEGER;
            triangleArrayChunk.m_numTriangles = chunkEntity.chunk->vao.elementsCount;
            triangleArrayChunk.m_triangleIndexBase = &mCollisionMeshIndexData[i][0];
            triangleArrayChunk.m_triangleIndexStride = sizeof(int32_t) * 3;
            triangleArrayChunk.m_numVertices = chunkEntity.chunk->vao.verticesCount;
            triangleArrayChunk.m_vertexBase = &mCollisionMeshVertexData[i][vertexOffset];
            triangleArrayChunk.m_vertexStride = chunkEntity.chunk->vertexStride;
            mCollisionMeshInfo->addIndexedMesh(triangleArrayChunk);
        }
    }

    void PhysicsTriangleCollisionShape::setupConvexHullCollisionShape(const ConfigurationReader& conf)
    {
        SDL_assert(mCollisionMeshInfo);
        /* build optimize hull */
        btConvexTriangleMeshShape convexTriangleShape(mCollisionMeshInfo.get());
        btShapeHull hullOptimizer(&convexTriangleShape);
        hullOptimizer.buildHull(conf.getDouble(L"Margin", 0.5));

        /* load hull to shape */
        auto convexHullShape = std::make_unique<btConvexHullShape>();
        for (int i = 0; i < hullOptimizer.numIndices(); ++i)
        {
            convexHullShape->addPoint(hullOptimizer.getVertexPointer()[hullOptimizer.getIndexPointer()[i]]);
        }

        mCollisionShape = std::move(convexHullShape);
        /* vertex data not needed yet */
        purgeMeshData();
    }

    void PhysicsTriangleCollisionShape::setupStaticTriangleMeshCollisionShape(const ConfigurationReader& conf)
    {
        SDL_assert(mCollisionMeshInfo);
        mCollisionShape = std::make_unique<btBvhTriangleMeshShape>(mCollisionMeshInfo.get(), true, true);
    }

    void PhysicsTriangleCollisionShape::setupGimpactTriangleMeshCollisionShape(const ConfigurationReader& conf)
    {
        SDL_assert(mCollisionMeshInfo);
        mCollisionShape = std::make_unique<btGImpactMeshShape>(mCollisionMeshInfo.get());
        updateBounds();
    }

    void PhysicsTriangleCollisionShape::purgeMeshData()
    {
        mCollisionMeshVertexData.clear();
        mCollisionMeshIndexData.clear();
    }

    void PhysicsTriangleCollisionShape::updateBounds()
    {
        SDL_assert(mCollisionShape);
        if (mCollisionShapeType == GimpactTriangleMesh)
        {
            btGImpactMeshShape *shape = static_cast<btGImpactMeshShape *>(mCollisionShape.get());
            shape->updateBound();
            shape->postUpdate();
        }
    }
}}