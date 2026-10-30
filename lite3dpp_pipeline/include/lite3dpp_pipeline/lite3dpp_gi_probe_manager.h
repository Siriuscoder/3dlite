/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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

#include <lite3dpp/lite3dpp_compute_shader.h>
#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

enum class EnvProbeFlags : uint32_t
{
    Irradiance = 1 << 0,
    Specular = 1 << 1,
    Reusable = 1 << 2 // Inactive slot available for reuse.
};

LITE3D_DECLARE_ENUM_OPERATORS(EnvProbeFlags);

class LITE3DPP_PIPELINE_EXPORT GIProbeManager : public RenderTargetObserver, public LifecycleObserver, 
    public SceneObserver, public Noncopiable
{
public:

    static constexpr const uint32_t MaxProbeCount = 256;
    static constexpr const int32_t DefaultProbeResolution = 256;
    static constexpr const char GGXLUTName[] = "IntergratedGGXLUT.texture";

#pragma pack(push, 16)
    struct ProbeRawEntity
    {
        kmVec4 position;
        kmMat4 viewProjMatrices[6];
        kmVec4 sh[9];
        uint32_t flags[4];
    };
#pragma pack(pop)

    class EnvProbe;

    class LITE3DPP_PIPELINE_EXPORT HintNode : public SceneNodeObserver, public Noncopiable
    {
    public:

        HintNode(EnvProbe &probe, SceneNodeBase *node, bool recursive);
        ~HintNode();

    private:

        void updatePosition(SceneNodeBase *node) override;
        void updateRotation(SceneNodeBase *node) override;
        void updateScale(SceneNodeBase *node) override;
        void updateSkeletonPose(SceneNodeBase *node) override;

        EnvProbe &mProbe;
        stl<SceneNodeBase*>::vector mNodes;
    };

    class LITE3DPP_PIPELINE_EXPORT EnvProbe
    {
    public:

        using HintCache = stl<SceneNodeBase*, std::shared_ptr<HintNode>>::unordered_map;

        EnvProbe(Main *main, float zNear, float zFar, EnvProbeFlags flags);
        
        void setPosition(const kmVec3 &pos);
        void writeProbe(ProbeRawEntity *probe) const;

        void rebuildMatrix();
        inline bool invalidated() const { return mInvalidated && !isReusable(); }
        inline void invalidate() { mInvalidated = true; }
        inline EnvProbeFlags getFlags() const { return mFlags; }
        inline void setFlags(EnvProbeFlags flags) { mFlags = flags; }
        inline bool isReusable() const { return (mFlags & EnvProbeFlags::Reusable) == EnvProbeFlags::Reusable; }
        // Unregister hints before destroying their scene nodes.
        HintNode *registerHintNode(SceneNodeBase *node, bool recursive = false);
        void unregisterHintNode(SceneNodeBase *node);
        void unregisterAllHintNodes();

    private:

        std::shared_ptr<Camera> mProbeCamera;
        stl<kmMat4>::vector mViewProjMatrices;
        bool mInvalidated = true;
        EnvProbeFlags mFlags;
        HintCache mHintNodes;
    };

    GIProbeManager(Main &main, PipelineBase &pipeline);
    virtual ~GIProbeManager();

    void initialize();
    inline RenderTarget* getPass() { return mEnvironmentProbeColorPass; }
    inline const std::string &getProbeBufferName() const { return mProbesBuffer->getName(); }
    inline const std::string &getProbeIndexBufferName() const { return mProbesIndexBuffer->getName(); }
    inline const std::string &getBrdfLUTName() const { return mBrdfLUT->getName(); }
    inline Texture *getEnvProbeTexture() { return mPrefilteredEnvironment; }
    void rebuild();
    // Pointers remain stable until manager destruction; removed slots may be reused by addProbe.
    EnvProbe *addProbe(const kmVec3 &position, EnvProbeFlags flags = EnvProbeFlags::Irradiance | EnvProbeFlags::Specular);
    // Keep the slot and upload its Reusable flag without clearing captured data.
    void removeProbe(EnvProbe *probe);
    void prepareAffectedProbes();

protected:

    bool beginUpdate(RenderTarget *rt) override;
    void postUpdate(RenderTarget *rt) override;
    bool customFrustumCheck(Scene *scene, SceneNodeBase *node, lite3d_mesh_chunk *meshChunk, Material *material,
        lite3d_bounding_vol *boundingVol, Camera *camera, const lite3d_scene_render_params *params) override;

    void createProbePass(const ConfigurationReader &config);
    void createPrefilterShader(const ConfigurationReader &config);
    void createDiffuseRadianceShader();
    void calculateProbeBatchCount();
    void integrateGGX();

protected:

    Main& mMain;
    PipelineBase &mPipeline;
    uint32_t mProbeCount = 1;
    uint32_t mMaxProbeBatchCount = 0;
    String mPipelineName;
    String mShaderPackage;
    VBOResource *mProbesBuffer = nullptr;
    VBOResource *mProbesIndexBuffer = nullptr;
    RenderTarget *mEnvironmentProbeColorPass = nullptr;
    TextureImage *mEnvironmentProbeColor = nullptr;
    TextureImage *mEnvironmentProbeDepth = nullptr;
    Texture *mBrdfLUT = nullptr;
    ComputeShader *mPrefilterEnvironmentShader = nullptr;
    ComputeShader *mDiffuseRadianceSHShader = nullptr;
    TextureImage *mPrefilteredEnvironment = nullptr;
    stl<EnvProbe>::vector mProbes;
    stl<int32_t>::vector mProbesIndex;
    float mzNear = 0.0, mzFar = 1.0;
    int32_t mSHMaxThreads = 256;
};

}}
