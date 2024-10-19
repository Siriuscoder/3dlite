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
#pragma once

#include <lite3dpp_pipeline/lite3dpp_generator.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

enum class EnvProbeFlags : int32_t
{
    Irradiance = 1 << 0,
    Specular = 1 << 1
};

// Перегрузка оператора | для использования побитовых операций
inline EnvProbeFlags operator|(EnvProbeFlags a, EnvProbeFlags b)
{
    return static_cast<EnvProbeFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
}

// Перегрузка оператора & для побитовых операций (например, для проверок)
inline EnvProbeFlags operator&(EnvProbeFlags a, EnvProbeFlags b)
{
    return static_cast<EnvProbeFlags>(static_cast<int32_t>(a) & static_cast<int32_t>(b));
}

class LITE3DPP_PIPELINE_EXPORT IBLMultiProbe : public RenderTargetObserver, public LifecycleObserver, public Noncopiable
{
public:

    static constexpr const uint32_t MaxProbeCount = 256;

#pragma pack(push, 16)
    struct ProbeRawEntity
    {
        kmVec4 position;
        kmMat4 viewProjMatrices[6];
        int32_t flags[4];
    };

    struct ProbeIndexRawEntity
    {
        ProbeIndexRawEntity() = default;

        template<class T>
        ProbeIndexRawEntity(T i)
        {
            index[0] = static_cast<int32_t>(i);
        }

        int32_t index[4] = {0};
    };
#pragma pack(pop)

    class EnvProbe
    {
    public:

        EnvProbe(Main *main, float zNear, float zFar, EnvProbeFlags flags);
        
        void setPosition(const kmVec3 &pos);
        void writeProbe(ProbeRawEntity *probe) const;

        void rebuildMatrix();
        inline bool invalidated() const { return mInvalidated; }
        inline void invalidate() { mInvalidated = true; }

    private:

        std::shared_ptr<Camera> mProbeCamera;
        stl<kmMat4>::vector mViewProjMatrices;
        bool mInvalidated = true;
        EnvProbeFlags mFlags;
    };

    IBLMultiProbe(Main& main, const String& pipelineName);
    virtual ~IBLMultiProbe();

    void initialize(const ConfigurationReader &pipelineConfig);
    inline RenderTarget* getPass() { return mEnvironmentProbePass; }
    inline const std::string &getProbeBufferName() const { return mProbesBuffer->getName(); }
    inline const std::string &getProbeIndexBufferName() const { return mProbesIndexBuffer->getName(); }
    inline Texture *getEnvProbeTexture() { return mEnvironmentProbe; }
    void rebuild();
    size_t addProbe(const kmVec3 &position, EnvProbeFlags flags = EnvProbeFlags::Irradiance | EnvProbeFlags::Specular);
    void updateProbe(size_t index, const kmVec3 &position);

protected:

    bool beginUpdate(RenderTarget *rt) override;

    void createProbePass(const ConfigurationReader &config);
    VBOResource* createBuffer(const String& bufferName, size_t size);
    void calculateProbeBatchCount();

protected:

    Main& mMain;
    uint32_t mProbeCount = 1;
    uint32_t mMaxProbeCount = 0;
    uint32_t mMaxProbeBatchCount = 0;
    String mPipelineName;
    VBOResource *mProbesBuffer = nullptr;
    VBOResource *mProbesIndexBuffer = nullptr;
    RenderTarget *mEnvironmentProbePass = nullptr;
    Texture *mEnvironmentProbe = nullptr;
    Texture *mEnvironmentDepth = nullptr;
    stl<EnvProbe>::vector mProbes;
    stl<ProbeIndexRawEntity>::vector mProbesIndex;
    stl<String>::list mResourcesList;
    float mzNear = 0.0, mzFar = 1.0;
};

}}
