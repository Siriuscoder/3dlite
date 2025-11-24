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
#include <lite3dpp_pipeline/lite3dpp_IBL_multiprobe.h>

#include <algorithm>
#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

IBLMultiProbe::IBLMultiProbe(Main& mMain, const String& pipelineName, const String& shaderPackage) : 
    mMain(mMain),
    mPipelineName(pipelineName),
    mShaderPackage(shaderPackage)
{
    mMain.addObserver(this);
}

IBLMultiProbe::~IBLMultiProbe()
{
    for (auto it = mResourcesList.rbegin(); it != mResourcesList.rend(); ++it)
    {
        mMain.getResourceManager().releaseResource(*it);
    }

    mMain.removeObserver(this);
}

void IBLMultiProbe::initialize(const ConfigurationReader &pipelineConfig)
{
    ConfigurationReader config = pipelineConfig.getObject(L"GI");
    mProbeCount = config.getInt(L"MaxCount", 1);
    mzNear = config.getDouble(L"ProbeZNearClip", 0.0f);
    mzFar = config.getDouble(L"ProbeZFarClip", 1.0f);

    calculateProbeBatchCount();
    integrateGGX();

    mProbesBuffer = createBuffer("_EnvProbesData", sizeof(ProbeRawEntity) * mProbeCount);
    // Массивы в std140 всегда выравниваются по границе 16 байт, даже если тип данных (например, int) 
    // сам занимает меньше места (4 байта).
    mProbesIndexBuffer = createBuffer("_EnvProbesIndex", sizeof(ProbeIndexRawEntity) * (mMaxProbeBatchCount + 1));
    createProbePass(config);
    createPrefilterShader(config);
}

void IBLMultiProbe::integrateGGX()
{
    const int32_t LUTSize = 512;
    ConfigurationWriter IntergratedGGXLUTConfig;
    IntergratedGGXLUTConfig.set(L"TextureType", "2D")
        .set(L"Filtering", "Linear")
        .set(L"Width", LUTSize)
        .set(L"Height", LUTSize)
        .set(L"Depth", 1)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RG")
        .set(L"InternalFormat", "RG16F");

    mBrdfLUT = mMain.getResourceManager().queryResourceFromJson<TextureImage>("IntergratedGGXLUT.texture", 
        IntergratedGGXLUTConfig.write());

    ConfigurationWriter shaderParams;
    shaderParams.set(L"Program", ConfigurationWriter()
        .set(L"Name", "IntergrateGGX.program")
        .set(L"Path", mShaderPackage + ":shaders/json/integrate_ggx.json"))
        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Type", "imageStore")
            .set(L"Name", "BrdfLUT")
            .set(L"Direction", "output")
            .set(L"TextureName", mBrdfLUT->getName())
    });

    auto intergrateGGXShader = mMain.getResourceManager().queryResourceFromJson<ComputeShader>("IntergrateGGX.comp",
        shaderParams.write());

    // 16 нитей в группе
    const uint32_t groupsCount = (LUTSize + 16 - 1) / 16;
    intergrateGGXShader->dispatch(groupsCount, groupsCount, 1);

    // Освобождаем ресурсы, расчет GGX делается только однажды, более шейдер не требуется
    mMain.getResourceManager().releaseResource(intergrateGGXShader->getProgram()->getName());
    mMain.getResourceManager().releaseResource(intergrateGGXShader->getName());
}

void IBLMultiProbe::calculateProbeBatchCount()
{
    int maxGeometryOutputVertices, maxGeometryTotalOutputComponents, UBOMaxSize;
    lite3d_shader_program_get_limitations(&maxGeometryOutputVertices, nullptr, &maxGeometryTotalOutputComponents);
    lite3d_vbo_get_limitations(&UBOMaxSize, nullptr, nullptr);

    // Число компонент на одну вершину в геометрическом шейдере для рендера пробников
    // Константа связана с кодом шейдера!!!
    const uint32_t probeComponentsByVertex = 13; // pos + iuv + iwv + iwn + drawID
    uint32_t a = maxGeometryTotalOutputComponents / (probeComponentsByVertex * 3 * 6);
    uint32_t b = maxGeometryOutputVertices / (3 * 6);
    mMaxProbeBatchCount = std::min(a, b);

    uint32_t maxPossibleProbeCount = UBOMaxSize / sizeof(ProbeRawEntity);
    uint32_t maxProbeCount = std::min(maxPossibleProbeCount, MaxProbeCount);

    if (mProbeCount > maxProbeCount)
    {
        LITE3D_THROW("Too much probe count(" << mProbeCount << ") are requested, "
            "max hardware posible limit is " << maxProbeCount);
    }

    ShaderProgram::addGlobalDefinition("LITE3D_ENV_PROBE_GS_MAX_VERTICES", std::to_string(mMaxProbeBatchCount * 3 * 6));
    ShaderProgram::addGlobalDefinition("LITE3D_ENV_PROBE_MAX", std::to_string(maxProbeCount));
}

VBOResource* IBLMultiProbe::createBuffer(const String& bufferName, size_t size)
{
    auto buffer = mMain.getResourceManager().queryResourceFromJson<UBO>(mPipelineName + bufferName,
        "{\"Dynamic\": true}");
    
    mResourcesList.emplace_back(buffer->getName());
    // Выделяем место под 6 * N матриц, нужны будут для рендера сторон массива кубических текстур
    buffer->extendBufferBytes(size);
    return buffer;
}

void IBLMultiProbe::createProbePass(const ConfigurationReader &config)
{
    auto resolution = config.getInt(L"ProbeResolution", DefaultProbeResolution);
    // Создание массива кубических текстур для пробирования окружающего освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE_ARRAY")
        .set(L"Filtering", "Trilinear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Depth", mProbeCount)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB16F");

    mEnvironmentProbe = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentMultiProbe.texture", mapConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbe->getName());

    // Создание массива кубических текстур глубины для рендера сцены
    mapConfig.set(L"Filtering", "None")
        .set(L"TextureFormat", "Depth")
        .set(L"InternalFormat", "");

    mEnvironmentDepth = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentMultiProbeDepth.texture", mapConfig.write());
    mResourcesList.emplace_back(mEnvironmentDepth->getName());

    ConfigurationWriter passConfig;
    passConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<uint32_t>(RenderPassPriority::GIProbe))
        .set(L"CleanColorBuf", false)
        .set(L"CleanDepthBuf", true)
        .set(L"CleanStencilBuf", false)
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"LayeredFramebuffer", true)
        .set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", stl<ConfigurationWriter>::vector {
                ConfigurationWriter().set(L"TextureName", mEnvironmentProbe->getName())
            }))
        .set(L"DepthAttachments", ConfigurationWriter()
            .set(L"TextureName", mEnvironmentDepth->getName()));

    mEnvironmentProbePass = mMain.getResourceManager().queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_EnvironmentMultiProbePass", passConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbePass->getName());
    mEnvironmentProbePass->addObserver(this);
}

void IBLMultiProbe::createPrefilterShader(const ConfigurationReader &config)
{
    auto resolution = config.getInt(L"ProbeResolution", DefaultProbeResolution);
    // Создание массива кубических текстур для пробирования окружающего освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE_ARRAY")
        .set(L"Filtering", "Trilinear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Depth", mProbeCount)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGBA16F");

    mPrefilteredEnvironment = mMain.getResourceManager().queryResourceFromJson<TextureImage>(
        mPipelineName + "_PrefilteredEnvironmentMultiProbe.texture", mapConfig.write());
    mResourcesList.emplace_back(mPrefilteredEnvironment->getName());

    ConfigurationWriter shaderParams;
    shaderParams.set(L"Program", ConfigurationWriter()
        .set(L"Name", "PrefilterEnvironment.program")
        .set(L"Path", mShaderPackage + ":shaders/json/prefilter_environment.json"))
        .set(L"Uniforms", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Type", "imageStore")
            .set(L"Name", "PrefilteredEnvironment")
            .set(L"Direction", "output")
            .set(L"TextureName", mPrefilteredEnvironment->getName()),
        ConfigurationWriter().set(L"Type", "sampler")
            .set(L"Name", "EnvironmentProbe")
            .set(L"TextureName", mEnvironmentProbe->getName()),
        ConfigurationWriter().set(L"Type", "float")
            .set(L"Name", "Roughness")
            .set(L"Value", 1.0f),
        ConfigurationWriter().set(L"Type", "UBO")
            .set(L"Name", "EnvProbesIndex")
            .set(L"UBOName", getProbeIndexBufferName())
    });

    mPrefilterEnvironmentShader = mMain.getResourceManager().queryResourceFromJson<ComputeShader>(
        mPipelineName + "_PrefilterEnvironment.comp", shaderParams.write());
    mResourcesList.emplace_back(mPrefilterEnvironmentShader->getName());
}

void IBLMultiProbe::rebuild()
{
    for (auto &probe : mProbes)
    {
        probe.invalidate();
    }
}

bool IBLMultiProbe::beginUpdate(RenderTarget *rt)
{
    // Nothing to do
    if (std::find_if(mProbes.begin(), mProbes.end(), [](const EnvProbe &e) { return e.invalidated(); }) == mProbes.end())
    {
        return false;
    }

    SDL_assert((mProbes.size() * sizeof(ProbeRawEntity)) <= mProbesBuffer->bufferSizeBytes());
    mProbesIndex.resize(1);
    
    {
        auto lock = mProbesBuffer->map(BufferScopedMapper::BufferScopedMapperLockType::LockTypeWrite);
        auto probeBlock = lock.getPtr<ProbeRawEntity>(); 

        for (size_t i = 0; i < mProbes.size(); ++i)
        {
            if (mProbes[i].invalidated())
            {
                mProbes[i].rebuildMatrix();
                mProbes[i].writeProbe(&probeBlock[i]);
                mProbesIndex.emplace_back(i);
            }

            if (mProbesIndex.size() == (mMaxProbeBatchCount + 1))
            {
                break;
            }
        }
    }
    
    mProbesIndex[0].index[0] = static_cast<int32_t>(mProbesIndex.size() - 1);
    mProbesIndexBuffer->setData(&mProbesIndex[0], 0, sizeof(ProbeIndexRawEntity) * mProbesIndex.size());

    return true;
}

void IBLMultiProbe::postUpdate(RenderTarget *rt)
{
    // nothing to do
    if (mProbesIndex.size() == 0 || mProbesIndex[0].index[0] == 0)
    {
        return;
    }

    auto totalLevels = mPrefilteredEnvironment->getTotalLevels();
    auto levelSize = mPrefilteredEnvironment->getWidth();
    for (int8_t mip = 0; mip < totalLevels; ++mip)
    {
        // Calculate a roughness value in the range [0, 1] based on the
        // current mip level and the total number of mip levels.
        const auto roughness = static_cast<float>(mip) / static_cast<float>(totalLevels);
        mPrefilterEnvironmentShader->getShaderParameters().setFloatParameter("Roughness", roughness);
        // Bind the current mip level of the environment probe texture
        mPrefilterEnvironmentShader->getShaderParameters().getParameter("PrefilteredEnvironment", 
            LITE3D_SHADER_PARAMETER_IMAGE_STORE)->imageMipLevel = mip;

        const uint32_t groupsCount = (levelSize + 16 - 1) / 16;
        mPrefilterEnvironmentShader->dispatch(groupsCount, groupsCount, mProbesIndex[0].index[0] * 6);

        // Shift to the next mip level
        levelSize >>= 1;
    }
}

size_t IBLMultiProbe::addProbe(const kmVec3 &position, EnvProbeFlags flags)
{
    if (mProbes.size() == mProbeCount)
    {
        LITE3D_THROW("The maximum probes count limit is reached: " << mProbeCount);
    }

    mProbes.emplace_back(&mMain, mzNear, mzFar, flags);
    mProbes.back().setPosition(position);
    return mProbes.size() - 1;
}

void IBLMultiProbe::updateProbe(size_t index, const kmVec3 &position)
{
    if (index >= mProbes.size())
    {
        LITE3D_THROW("Index out of range: " << index << " of " << mProbes.size());
    }

    mProbes[index].setPosition(position);
}

IBLMultiProbe::EnvProbe::EnvProbe(Main *main, float zNear, float zFar, EnvProbeFlags flags) : 
    mProbeCamera(std::make_shared<Camera>("", main)),
    mFlags(flags)
{
    mProbeCamera->setupPerspective(zNear, zFar, 90.0f, 1.0f);
}

void IBLMultiProbe::EnvProbe::rebuildMatrix()
{
    mProbeCamera->computeCubeProjView(mViewProjMatrices);
    mInvalidated = false;
}

void IBLMultiProbe::EnvProbe::setPosition(const kmVec3 &pos)
{
    mProbeCamera->setPosition(pos);
    invalidate();
}

void IBLMultiProbe::EnvProbe::writeProbe(IBLMultiProbe::ProbeRawEntity *probe) const
{
    SDL_assert(mViewProjMatrices.size() == 6);
    SDL_assert(probe);

    memset(probe, 0, sizeof(IBLMultiProbe::ProbeRawEntity));

    probe->position.x = mProbeCamera->getPosition().x;
    probe->position.y = mProbeCamera->getPosition().y;
    probe->position.z = mProbeCamera->getPosition().z;
    probe->position.w = 0.0f;
    
    std::copy(mViewProjMatrices.begin(), mViewProjMatrices.end(), probe->viewProjMatrices);
    probe->flags[0] = static_cast<uint32_t>(mFlags);
}

}}
