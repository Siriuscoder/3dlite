#include <lite3dpp_pipeline/lite3dpp_IBL.h>

#include <SDL_assert.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

IBLDiffuseIrradiance::IBLDiffuseIrradiance(Main& mMain, const String& pipelineName, const String& shaderPackage) : 
    mMain(mMain),
    mPipelineName(pipelineName),
    mShaderPackage(shaderPackage)
{
    mMain.addObserver(this);
}

IBLDiffuseIrradiance::~IBLDiffuseIrradiance()
{
    for (auto it = mResourcesList.rbegin(); it != mResourcesList.rend(); ++it)
    {
        mMain.getResourceManager()->releaseResource(*it);
    }

    mMain.removeObserver(this);
}

void IBLDiffuseIrradiance::initialize(const ConfigurationReader &pipelineConfig)
{
    mViewCubeMatricesGPUBuffer = createMatrixBuffer("_CubeTransform");

    ConfigurationReader iblConfig = pipelineConfig.getObject(L"IBL");
    createDiffuse(iblConfig);
    createDiffuseIrradiance(iblConfig);

    mEnvConvolutionCamera = mMain.getCamera(mPipelineName + "_DiffuseIrradiance_Camera");
    if (iblConfig.has(L"FixedProbePosition"))
    {
        mEnvProbeFixedPos = iblConfig.getVec3(L"FixedProbePosition");
    }
}

VBOResource* IBLDiffuseIrradiance::createMatrixBuffer(const String& bufferName)
{
    auto matrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>(mPipelineName + bufferName,
        "{\"Dynamic\": true}");
    
    mResourcesList.emplace_back(matrixBuffer->getName());
    // Выделяем место под 6 матриц, нужны будут для рендера 6ти сторон кубической текстуры
    matrixBuffer->extendBufferBytes(sizeof(kmMat4) * 6);
    return matrixBuffer;
}

void IBLDiffuseIrradiance::createDiffuse(const ConfigurationReader &iblConfig)
{
    auto resolution = iblConfig.getInt(L"environmentProbeResolution", 256);
    // Создание кубической текстуры окружающего диффузного освещения  
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE")
        .set(L"Filtering", "Trilinear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB32F");

    mEnvironmentProbe = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvironmentProbe.texture", mapConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbe->getName());

    // Создание кубической текстуры глубины для рендера сцены
    mapConfig.set(L"Filtering", "None")
        .set(L"TextureFormat", "Depth")
        .set(L"InternalFormat", "");

    mDepthCubeMap = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvCubeDepth.texture", mapConfig.write());
    mResourcesList.emplace_back(mDepthCubeMap->getName());

    ConfigurationWriter passConfig;
    passConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<int>(RenderPassPriority::EnvironmentProbe))
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
            .set(L"TextureName", mDepthCubeMap->getName()));

    mEnvironmentProbePass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_EnvironmentProbePass", passConfig.write());
    mResourcesList.emplace_back(mEnvironmentProbePass->getName());
    mEnvironmentProbePass->addObserver(this);
}

void IBLDiffuseIrradiance::createDiffuseIrradiance(const ConfigurationReader &iblConfig)
{
    auto iresolution = iblConfig.getInt(L"IrradianceProbeResolution", 64);
    // Создание кубической текстуры сожержащей свертку окружающего диффузного освещения
    ConfigurationWriter mapConfig;
    mapConfig.set(L"TextureType", "CUBE")
        .set(L"Filtering", "Linear")
        .set(L"Width", iresolution)
        .set(L"Height", iresolution)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB32F");

    mIrradianceProbe = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_IrradianceProbe.texture", mapConfig.write());
    mResourcesList.emplace_back(mIrradianceProbe->getName());

    ConfigurationWriter passConfig;
    passConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<int>(RenderPassPriority::IrradianceProbe))
        .set(L"CleanColorBuf", true)
        .set(L"CleanDepthBuf", false)
        .set(L"CleanStencilBuf", false)
        .set(L"Width", iresolution)
        .set(L"Height", iresolution)
        .set(L"LayeredFramebuffer", true)
        .set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", stl<ConfigurationWriter>::vector {
                ConfigurationWriter().set(L"TextureName", mIrradianceProbe->getName())
            }));

    mIrradianceProbePass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_diffuseIrradiancePass", passConfig.write());
    mResourcesList.emplace_back(mIrradianceProbePass->getName());
    mIrradianceProbePass->addObserver(this);

    SceneGenerator stageGenerator(mPipelineName + "_DiffuseIrradiance");
    stageGenerator.addCamera("Camera", ConfigurationWriter()
        .set(L"Position", kmVec3 { 0.0f, 0.0f, 0.0f })
        .set(L"LookAt", KM_VEC3_NEG_Z)
        .set(L"Perspective", ConfigurationWriter()
            .set(L"Znear", 0.0f)
            .set(L"Zfar", 1.0f)
            .set(L"Fov", 90.0f)));

    stageGenerator.addRenderTarget("Camera", mIrradianceProbePass->getName(), ConfigurationWriter()
        .set(L"Priority", static_cast<int>(RenderPassStagePriority::SkyBoxStage))
        .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
        .set(L"DepthTest", false)
        .set(L"ColorOutput", true)
        .set(L"DepthOutput", false));

    auto mIrradianceProbeScene = mMain.getResourceManager()->queryResourceFromJson<Scene>(
        mPipelineName + "_IrradianceProbeStage", stageGenerator.generate().write());
    mResourcesList.emplace_back(mIrradianceProbeScene->getName());

    SDL_assert(mEnvironmentProbe);

    ConfigurationWriter IrradianceComputeShaderConfig;
    IrradianceComputeShaderConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"Program", ConfigurationWriter()
                .set(L"Name", "IrradianceProbe.program")
                .set(L"Path", mShaderPackage + ":shaders/json/irradiance_probe.json"))
            .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                ConfigurationWriter()
                    .set(L"Name", "CubeTransform")
                    .set(L"Type", "UBO")
                    .set(L"UBOName", mViewCubeMatricesGPUBuffer->getName()),
                ConfigurationWriter()
                    .set(L"Name", "EnvironmentProbe")
                    .set(L"Type", "sampler")
                    .set(L"TextureName", mEnvironmentProbe->getName())
            })
    });

        // Создаем шейдер skybox
    auto IrradianceCumputeShader = mMain.getResourceManager()->queryResourceFromJson<Material>(
        mPipelineName + "_IrradianceCompute.material", IrradianceComputeShaderConfig.write());
    mResourcesList.emplace_back(IrradianceCumputeShader->getName());

        // Добавляем шейдер skybox
    mIrradianceProbeScene->addObject("SkyBox", SkyBoxObjectGenerator(IrradianceCumputeShader->getName()).generate());
}

void IBLDiffuseIrradiance::rebuildEnvironmentProbe()
{
    mEnvironmentProbePass->enable();
}

void IBLDiffuseIrradiance::rebuildIrradianceProbe()
{
    // Включаем перересовку по таймеру, каждый кадр обнолвять нет смысла
    mIrradianceProbePass->enable();
}

bool IBLDiffuseIrradiance::beginUpdate(RenderTarget *rt)
{
    if (!mMainCamera)
    {
        return false;
    }

    if (rt == mEnvironmentProbePass)
    {
        if (mEnvProbeFixedPos)
        {
            mMainCamera->computeCubeProjView(*mEnvProbeFixedPos, mMatricesHostBuffer);
        }
        else 
        {
            mMainCamera->computeCubeProjView(mMatricesHostBuffer);
        }
    }
    else if (rt == mIrradianceProbePass)
    {
        SDL_assert(mEnvConvolutionCamera);
        mEnvConvolutionCamera->computeCubeProjView(mMatricesHostBuffer);
    }

    mViewCubeMatricesGPUBuffer->setData(&mMatricesHostBuffer[0], 0, mMatricesHostBuffer.size() * sizeof(kmMat4));

    return true;
}

void IBLDiffuseIrradiance::postUpdate(RenderTarget *rt)
{
    // После обработка соответствующего прохода, отключим его для дальнейшего включения по внешнему событию 
    rt->disable();
}

}}
