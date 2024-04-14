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
#include <lite3dpp_physics/lite3dpp_physics_scene_node.h>

#include <SDL_assert.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_mesh.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    PhysicsCollisionShapeSceneNode::PhysicsCollisionShapeSceneNode(const ConfigurationReader &conf, SceneNode *parent, 
        Scene *scene, Main *main) : 
        SceneNode(conf, parent, scene, main)
    {
        ConfigurationReader collisionShapeConf = conf.getObject(L"CollisionShape");
        auto typeString = collisionShapeConf.getUpperString(L"Type");

        if (typeString == "BOX")
        {
            mCollisionShapeType = Box;
            setupBoxCollisionShape(collisionShapeConf);
        }
        else if (typeString == "SPHERE")
        {
            mCollisionShapeType = Sphere;
            setupSphereCollisionShape(collisionShapeConf);
        }
        else if (typeString == "STATICPLANE")
        {
            mCollisionShapeType = StaticPlane;
            setupStaticPlaneCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CYLINDER")
        {
            mCollisionShapeType = Cylinder;
            setupCylinderCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CAPSULE")
        {
            mCollisionShapeType = Capsule;
            setupCapsuleCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CONE")
        {
            mCollisionShapeType = Cone;
            setupConeCollisionShape(collisionShapeConf);
        }
        else if (typeString == "CONVEXHULL")
        {
            mCollisionShapeType = ConvexHull;
            setupConvexHullCollisionShape(collisionShapeConf);
        }
        else if (typeString == "STATICTRIANGLEMESH")
        {
            mCollisionShapeType = StaticTriangleMesh;
            setupStaticTriangleMeshCollisionShape(collisionShapeConf);
        }
        else if (typeString == "GIMPACTTRIANGLEMESH")
        {
            mCollisionShapeType = GimpactTriangleMesh;
            setupGimpactTriangleMeshCollisionShape(collisionShapeConf);
        }
        else
        {
            LITE3D_THROW("CollisionShapeSceneNode: '" << getName() << "' Unknown collision shape type");
        }

        mNodeMass = collisionShapeConf.getDouble(L"Mass");
        mCollisionShape->setMargin(collisionShapeConf.getDouble(L"Margin", 0.5));
        mCollisionShape->setUserPointer(this);
    }

    void PhysicsCollisionShapeSceneNode::setupBoxCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 halfExtents = BulletUtils::convert(conf.getVec3(L"HalfExtents", KM_VEC3_ONE));
        mCollisionShape = std::make_unique<btBoxShape>(halfExtents);
    }

    void PhysicsCollisionShapeSceneNode::setupStaticPlaneCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 normal = BulletUtils::convert(conf.getVec3(L"PlaneNormal", KM_VEC3_POS_Z));
        btScalar planeConstant = conf.getDouble(L"PlaneConstant");
        mCollisionShape = std::make_unique<btStaticPlaneShape>(normal, planeConstant);
    }

    void PhysicsCollisionShapeSceneNode::setupSphereCollisionShape(const ConfigurationReader& conf)
    {
        btScalar sphereRadius = conf.getDouble(L"Radius");
        mCollisionShape = std::make_unique<btSphereShape>(sphereRadius);
    }

    void PhysicsCollisionShapeSceneNode::setupConeCollisionShape(const ConfigurationReader& conf)
    {
        btScalar coneRadius = conf.getDouble(L"Radius");
        btScalar coneHeight = conf.getDouble(L"Height");
        mCollisionShape = std::make_unique<btConeShapeZ>(coneRadius, coneHeight);
    }

    void PhysicsCollisionShapeSceneNode::setupCapsuleCollisionShape(const ConfigurationReader& conf)
    {
        btScalar coneRadius = conf.getDouble(L"Radius");
        btScalar coneHeight = conf.getDouble(L"Height");
        mCollisionShape = std::make_unique<btCapsuleShapeZ>(coneRadius, coneHeight);
    }

    void PhysicsCollisionShapeSceneNode::setupCylinderCollisionShape(const ConfigurationReader& conf)
    {
        btVector3 halfExtents = BulletUtils::convert(conf.getVec3(L"HalfExtents", KM_VEC3_ONE));
        mCollisionShape = std::make_unique<btCylinderShapeZ>(halfExtents);
    }

    size_t PhysicsCollisionShapeSceneNode::validateChunkAndCalcVertexOffset(const lite3d_mesh_chunk *chunk)
    {
        size_t offset = 0;
        if (!chunk->hasIndexes)
        {
            LITE3D_THROW("validateChunkAndCalcVertexOffset: '" << getName() << "' only indexed meshes supported");
        }

        for (uint32_t i = 0; i < chunk->layoutEntriesCount; ++i)
        {
            if (chunk->layout[i].binding != LITE3D_BUFFER_BINDING_VERTEX)
            {
                offset += chunk->layout[i].count * sizeof(float);
            }
            else
            {
                return chunk->vao.verticesOffset + offset;
            }
        }

        LITE3D_THROW("validateChunkAndCalcVertexOffset: '" << getName() << "' vertex data not found..");
        return 0;
    }
    
    void PhysicsCollisionShapeSceneNode::setupTriangleMeshArray(const ConfigurationReader& conf)
    {
        /* load collision mesh into GPU memory */
        auto rawMesh = mMain->getResourceManager()->queryResource<Mesh>(conf.getObject(L"CollisionMesh").getString(L"Name"),
            conf.getObject(L"CollisionMesh").getString(L"Mesh"));

        if (rawMesh->getPtr()->chunks.size == 0)
        {
            LITE3D_THROW("setupTriangleMeshArray: '" << getName() << "' collision mesh is empty");
        }

        mCollisionMeshInfo = std::make_unique<btTriangleIndexVertexArray>();

        /* copy vertex and index GPU buffers to host memory */
        rawMesh->vertexBuffer().getData(mCollisionMeshVertexData, 0, rawMesh->vertexBuffer().bufferSizeBytes());
        rawMesh->indexBuffer().getData(mCollisionMeshIndexData, 0, rawMesh->indexBuffer().bufferSizeBytes());

        lite3d_mesh_chunk *chunk = static_cast<lite3d_mesh_chunk *>(rawMesh->getPtr()->chunks.data);
        for (size_t i = 0; i < rawMesh->getPtr()->chunks.size; ++i)
        {
            /* calc initial vertex data offset */
            auto vertexOffset = validateChunkAndCalcVertexOffset(chunk + i);

            btIndexedMesh triangleArrayChunk;
            triangleArrayChunk.m_indexType = PHY_ScalarType::PHY_INTEGER;
            triangleArrayChunk.m_numTriangles = chunk[i].vao.elementsCount;
            triangleArrayChunk.m_triangleIndexBase = &mCollisionMeshIndexData[chunk[i].vao.indexesOffset];
            triangleArrayChunk.m_triangleIndexStride = sizeof(int32_t) * 3;
            triangleArrayChunk.m_numVertices = chunk[i].vao.verticesCount;
            triangleArrayChunk.m_vertexBase = &mCollisionMeshVertexData[vertexOffset];
            triangleArrayChunk.m_vertexStride = chunk[i].vertexStride;
            mCollisionMeshInfo->addIndexedMesh(triangleArrayChunk);
        }
    }

    void PhysicsCollisionShapeSceneNode::setupConvexHullCollisionShape(const ConfigurationReader& conf)
    {
        setupTriangleMeshArray(conf);

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

    void PhysicsCollisionShapeSceneNode::setupStaticTriangleMeshCollisionShape(const ConfigurationReader& conf)
    {
        setupTriangleMeshArray(conf);
        SDL_assert(mCollisionMeshInfo);

        mCollisionShape = std::make_unique<btBvhTriangleMeshShape>(mCollisionMeshInfo.get(), true, true);
    }

    void PhysicsCollisionShapeSceneNode::setupGimpactTriangleMeshCollisionShape(const ConfigurationReader& conf)
    {
        setupTriangleMeshArray(conf);
        SDL_assert(mCollisionMeshInfo);
        
        mCollisionShape = std::make_unique<btGImpactMeshShape>(mCollisionMeshInfo.get());
    }

    void PhysicsCollisionShapeSceneNode::purgeMeshData()
    {
        BufferData vertexEmpty;
        BufferData indexEmpty;
        mCollisionMeshVertexData.swap(vertexEmpty);
        mCollisionMeshIndexData.swap(indexEmpty);
    }
}}
