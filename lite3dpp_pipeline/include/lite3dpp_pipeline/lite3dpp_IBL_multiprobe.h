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

class LITE3DPP_PIPELINE_EXPORT IBLMultiProbe : public RenderTargetObserver, public LifecycleObserver, public Noncopiable
{
public:

    static constexpr const size_t MaxProbeCountInBatch = 4;

#pragma pack(push, 16)
    struct ProbeEntity
    {
        kmVec4 position;
        kmMat4 viewProjMatrices[6];
    };

    struct ProbeIndexEntity
    {
        ProbeIndexEntity() = default;

        template<class T>
        ProbeIndexEntity(T i)
        {
            index[0] = static_cast<int32_t>(i);
        }

        int32_t index[4] = {0};
    };
#pragma pack(pop)



    class EnvProbe
    {
    public:

        EnvProbe(Main *main, float zNear, float zFar);
        
        void setPosition(const kmVec3 &pos);
        void writeProbe(ProbeEntity *probe) const;

        void rebuildMatrix();
        inline bool invalidated() const { return mInvalidated; }
        inline void invalidate() { mInvalidated = true; }

    private:

        std::shared_ptr<Camera> mProbeCamera;
        stl<kmMat4>::vector mViewProjMatrices;
        bool mInvalidated = true;
    };

    IBLMultiProbe(Main& main, const String& pipelineName);
    virtual ~IBLMultiProbe();

    void initialize(const ConfigurationReader &pipelineConfig);
    inline RenderTarget* getPass() { return mEnvironmentProbePass; }
    inline const std::string &getProbeBufferName() const { return mProbesBuffer->getName(); }
    inline const std::string &getProbeIndexBufferName() const { return mProbesIndexBuffer->getName(); }
    inline Texture *getEnvProbeTexture() { return mEnvironmentProbe; }
    void rebuild();
    size_t addProbe(const kmVec3 &position);
    void updateProbe(size_t index, const kmVec3 &position);

protected:

    void postUpdate(RenderTarget *rt) override;
    bool beginUpdate(RenderTarget *rt) override;

    void createProbePass(const ConfigurationReader &config);
    VBOResource* createBuffer(const String& bufferName, size_t size);

protected:

    Main& mMain;
    uint32_t mProbeMaxCount = 1;
    String mPipelineName;
    VBOResource *mProbesBuffer = nullptr;
    VBOResource *mProbesIndexBuffer = nullptr;
    RenderTarget *mEnvironmentProbePass = nullptr;
    Texture *mEnvironmentProbe = nullptr;
    Texture *mEnvironmentDepth = nullptr;
    stl<EnvProbe>::vector mProbes;
    stl<ProbeIndexEntity>::vector mProbesIndex;
    stl<String>::list mResourcesList;
    float mzNear = 0.0, mzFar = 1.0;

};

}}