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
    ConfigurationReader config = pipelineConfig.getObject(L"GI");
    mProbeMaxCount = config.getInt(L"MaxCount", 1);
    mzNear = config.getDouble(L"ProbeZNearClip", 0.0f);
    mzFar = config.getDouble(L"ProbeZFarClip", 1.0f);

    mProbesBuffer = createBuffer("_EnvProbesData", sizeof(ProbeRawEntity) * mProbeMaxCount);
    // Массивы в std140 всегда выравниваются по границе 16 байт, даже если тип данных (например, int) 
    // сам занимает меньше места (4 байта).
    mProbesIndexBuffer = createBuffer("_EnvProbesIndex", sizeof(ProbeIndexRawEntity) * (MaxProbeCountInBatch + 1));
    createProbePass(config);
}

VBOResource* IBLMultiProbe::createBuffer(const String& bufferName, size_t size)
{
    auto buffer = mMain.getResourceManager()->queryResourceFromJson<UBO>(mPipelineName + bufferName,
        "{\"Dynamic\": true}");
    
    mResourcesList.emplace_back(buffer->getName());
    // Выделяем место под 6 * N матриц, нужны будут для рендера сторон массива кубических текстур
    buffer->extendBufferBytes(size);
    return buffer;
}

void IBLMultiProbe::createProbePass(const ConfigurationReader &config)
{
    auto resolution = config.getInt(L"ProbeResolution", 256);
    // Создание массива кубических текстур для пробирования окружающего освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE_ARRAY")
        .set(L"Filtering", "Trilinear")
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

    mEnvironmentProbePass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_EnvironmentMultiProbePass", passConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbePass->getName());
    mEnvironmentProbePass->addObserver(this);
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

            if (mProbesIndex.size() == (MaxProbeCountInBatch + 1))
            {
                break;
            }
        }
    }
    
    mProbesIndex[0].index[0] = static_cast<int32_t>(mProbesIndex.size() - 1);
    mProbesIndexBuffer->setData(&mProbesIndex[0], 0, sizeof(ProbeIndexRawEntity) * mProbesIndex.size());

    return true;
}

size_t IBLMultiProbe::addProbe(const kmVec3 &position)
{
    if (mProbes.size() >= mProbeMaxCount)
    {
        LITE3D_THROW("Max probes count is reached: " << mProbeMaxCount << " probes");
    }

    mProbes.emplace_back(&mMain, mzNear, mzFar);
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

IBLMultiProbe::EnvProbe::EnvProbe(Main *main, float zNear, float zFar) : 
    mProbeCamera(std::make_shared<Camera>("", main))
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

    probe->position.x = mProbeCamera->getPosition().x;
    probe->position.y = mProbeCamera->getPosition().y;
    probe->position.z = mProbeCamera->getPosition().z;
    probe->position.w = 0.0f;
    
    std::copy(mViewProjMatrices.begin(), mViewProjMatrices.end(), probe->viewProjMatrices);
}

}}
