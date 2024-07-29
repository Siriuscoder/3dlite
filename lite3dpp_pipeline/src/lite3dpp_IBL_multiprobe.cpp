#include <lite3dpp_pipeline/lite3dpp_IBL_multiprobe.h>

#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

IBLMultiProbe::IBLMultiProbe(Main& mMain, const String& pipelineName) : 
    mMain(mMain),
    mPipelineName(pipelineName)
{
    mMain.addObserver(this);
}

IBLMultiProbe::~IBLMultiProbe()
{
    for (auto it = mResourcesList.rbegin(); it != mResourcesList.rend(); ++it)
    {
        mMain.getResourceManager()->releaseResource(*it);
    }

    mMain.removeObserver(this);
}

void IBLMultiProbe::initialize(const ConfigurationReader &pipelineConfig)
{
    ConfigurationReader config = pipelineConfig.getObject(L"IBLMultiProbe");
    mProbeMaxCount = config.getInt(L"MaxCount", 1);
    mzNear = config.getDouble(L"ProbeZNearClip", 0.0f);
    mzFar = config.getDouble(L"ProbeZFarClip", 1.0f);

    mViewMatricesGPUBuffer = createMatrixBuffer("_LightProbes");
    createProbePass(config);
}

VBOResource* IBLMultiProbe::createMatrixBuffer(const String& bufferName)
{
    auto matrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>(mPipelineName + bufferName,
        "{\"Dynamic\": true}");
    
    mResourcesList.emplace_back(matrixBuffer->getName());
    // Выделяем место под 6 * N матриц, нужны будут для рендера сторон массива кубических текстур
    matrixBuffer->extendBufferBytes(sizeof(EnvProbeEntity) * mProbeMaxCount);
    return matrixBuffer;
}

void IBLMultiProbe::createProbePass(const ConfigurationReader &config)
{
    auto resolution = config.getInt(L"environmentProbeResolution", 256);
    // Создание массива кубических текстур для пробирования окружающего освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE_ARRAY")
        .set(L"Filtering", "Linear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Depth", mProbeMaxCount)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB16F");

    mEnvironmentProbe = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentMultiProbe.texture", mapConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbe->getName());

    // Создание массива кубических текстур глубины для рендера сцены
    mapConfig.set(L"Filtering", "None")
        .set(L"TextureFormat", "Depth")
        .set(L"InternalFormat", "");

    mEnvironmentDepth = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentMultiProbeDepth.texture", mapConfig.write());
    mResourcesList.emplace_back(mEnvironmentDepth->getName());

    ConfigurationWriter passConfig;
    passConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<uint32_t>(RenderPassPriority::EnvironmentMultiProbe))
        .set(L"CleanColorBuf", true)
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

    mEnvironmentProbePass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_EnvironmentMultiProbePass", passConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbePass->getName());
    mEnvironmentProbePass->addObserver(this);
}

void IBLMultiProbe::rebuild()
{
    mEnvironmentProbePass->enable();
}

bool IBLMultiProbe::beginUpdate(RenderTarget *rt)
{
    if (mRecalcProbes)
    {
        SDL_assert((mProbes.size() * sizeof(EnvProbeEntity)) <= mViewMatricesGPUBuffer->bufferSizeBytes());
        auto lock = mViewMatricesGPUBuffer->map(BufferScopedMapper::BufferScopedMapperLockType::LockTypeWrite);
        
        auto probeBlock = lock.getPtr<EnvProbeEntity>(); 
        for (auto &probe : mProbes)
        {
            probe.rebuildMatrix();
            probe.writeProbe(probeBlock++);
        }

        mRecalcProbes = false;
    }

    return true;
}

void IBLMultiProbe::addProbe(const kmVec3 &position)
{
    if (mProbes.size() >= mProbeMaxCount)
    {
        LITE3D_THROW("Max probes count is reached: " << mProbeMaxCount << " probes");
    }

    EnvProbe probe(&mMain, mzNear, mzFar);
    probe.setPosition(position);

    mProbes.emplace_back(probe);
    mRecalcProbes = true;
}

void IBLMultiProbe::postUpdate(RenderTarget *rt)
{
    // После обработка соответствующего прохода, отключим его для дальнейшего включения по внешнему событию 
    rt->disable();
}

IBLMultiProbe::EnvProbe::EnvProbe(Main *main, float zNear, float zFar) : 
    mProbeCamera(std::make_shared<Camera>("", main))
{
    mProbeCamera->setupPerspective(zNear, zFar, 90.0f, 1.0f);
}

void IBLMultiProbe::EnvProbe::rebuildMatrix()
{
    mProbeCamera->computeCubeProjView(mViewProjMatrices);
}

void IBLMultiProbe::EnvProbe::setPosition(const kmVec3 &pos)
{
    mProbeCamera->setPosition(pos);
}

const void IBLMultiProbe::EnvProbe::writeProbe(IBLMultiProbe::EnvProbeEntity *probe)
{
    SDL_assert(mViewProjMatrices.size() == 6);
    SDL_assert(probe);

    probe->position.x = mProbeCamera->getPosition().x;
    probe->position.y = mProbeCamera->getPosition().y;
    probe->position.z = mProbeCamera->getPosition().z;
    probe->position.w = 0.0f;
    
    std::copy(mViewProjMatrices.begin(), mViewProjMatrices.end(), probe->viewProjMatrices);
}

}}
