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
    mPositionViewCubeMatrices = createMatrixBuffer("_CubeTransform");
    mCenteredViewCubeMatrices = createMatrixBuffer("_StaticCubeTransform");

    ConfigurationReader iblConfig = pipelineConfig.getObject(L"IBL");
    createDiffuse(iblConfig);
    createDiffuseIrradiance(iblConfig);

    // Таймер перерисовки диффузной кубической карты
    mUpdateTimer = mMain.addTimer("EnvironmentUpdateTimer", 1000);
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
    auto resolution = iblConfig.getInt(L"SpecularMapResolution", 256);
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

    mSpecularMap = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_SpecularMap.texture", mapConfig.write());
    mResourcesList.emplace_back(mSpecularMap->getName());

    // Создание кубической текстуры глубины для рендера сцены
    mapConfig.set(L"Filtering", "None")
        .set(L"TextureFormat", "Depth")
        .set(L"InternalFormat", "");

    mDepthCubeMap = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_EnvCubeDepth.texture", mapConfig.write());
    mResourcesList.emplace_back(mDepthCubeMap->getName());

    ConfigurationWriter passConfig;
    passConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<int>(RenderPassPriority::SpecularMap))
        .set(L"CleanColorBuf", true)
        .set(L"CleanDepthBuf", true)
        .set(L"CleanStencilBuf", false)
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"LayeredFramebuffer", true)
        .set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", stl<ConfigurationWriter>::vector {
                ConfigurationWriter().set(L"TextureName", mSpecularMap->getName())
            }))
        .set(L"DepthAttachments", ConfigurationWriter()
            .set(L"TextureName", mDepthCubeMap->getName()));

    mSpecularMapPass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_specularMapPass", passConfig.write());
    mResourcesList.emplace_back(mSpecularMapPass->getName());
    mSpecularMapPass->addObserver(this);
}

void IBLDiffuseIrradiance::createDiffuseIrradiance(const ConfigurationReader &iblConfig)
{
    auto iresolution = iblConfig.getInt(L"IrradianceMapResolution", 64);
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

    mIrradianceMap = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        mPipelineName + "_IrradianceMap.texture", mapConfig.write());
    mResourcesList.emplace_back(mIrradianceMap->getName());

    ConfigurationWriter passConfig;
    passConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<int>(RenderPassPriority::IrradianceMap))
        .set(L"CleanColorBuf", true)
        .set(L"CleanDepthBuf", false)
        .set(L"CleanStencilBuf", false)
        .set(L"Width", iresolution)
        .set(L"Height", iresolution)
        .set(L"LayeredFramebuffer", true)
        .set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", stl<ConfigurationWriter>::vector {
                ConfigurationWriter().set(L"TextureName", mIrradianceMap->getName())
            }));

    mIrradianceMapPass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        mPipelineName + "_diffuseIrradiancePass", passConfig.write());
    mResourcesList.emplace_back(mIrradianceMapPass->getName());
    mIrradianceMapPass->addObserver(this);

    // Нужно чтобы название камеры вышло такое же как на главной сцене (так как камера должна быть общая)
    SceneGenerator stageGenerator(mPipelineName + "_DiffuseIrradiance");
    stageGenerator.addCamera("Camera", ConfigurationWriter()
        .set(L"Position", kmVec3 { 0.0f, 0.0f, 0.0f })
        .set(L"LookAt", KM_VEC3_NEG_Z)
        .set(L"Perspective", ConfigurationWriter()
            .set(L"Znear", 0.0f)
            .set(L"Zfar", 1.0f)
            .set(L"Fov", 90.0f)));

    stageGenerator.addRenderTarget("Camera", mIrradianceMapPass->getName(), ConfigurationWriter()
        .set(L"Priority", static_cast<int>(RenderPassStagePriority::SkyBoxStage))
        .set(L"TexturePass", static_cast<int>(TexturePassTypes::RenderPass))
        .set(L"DepthTest", false)
        .set(L"ColorOutput", true)
        .set(L"DepthOutput", false));

    auto mIrradianceMapScene = mMain.getResourceManager()->queryResourceFromJson<Scene>(
        mPipelineName + "_IrradianceMapStage", stageGenerator.generate().write());
    mResourcesList.emplace_back(mIrradianceMapScene->getName());

    SDL_assert(mSpecularMap);

    ConfigurationWriter IrradianceComputeShaderConfig;
    IrradianceComputeShaderConfig.set(L"Passes", stl<ConfigurationWriter>::vector {
        ConfigurationWriter().set(L"Pass", static_cast<int>(TexturePassTypes::RenderPass))
            .set(L"Program", ConfigurationWriter()
                .set(L"Name", "IrradianceMap.program")
                .set(L"Path", mShaderPackage + ":shaders/json/irradiance_probe.json"))
            .set(L"Uniforms", stl<ConfigurationWriter>::vector {
                ConfigurationWriter()
                    .set(L"Name", "CubeTransform")
                    .set(L"Type", "UBO")
                    .set(L"UBOName", mCenteredViewCubeMatrices->getName()),
                ConfigurationWriter()
                    .set(L"Name", "SpecularCubeMap")
                    .set(L"Type", "sampler")
                    .set(L"TextureName", mSpecularMap->getName())
            })
    });

        // Создаем шейдер skybox
    auto IrradianceCumputeShader = mMain.getResourceManager()->queryResourceFromJson<Material>(
        mPipelineName + "_IrradianceCompute.material", IrradianceComputeShaderConfig.write());
    mResourcesList.emplace_back(IrradianceCumputeShader->getName());

        // Добавляем шейдер skybox
    mIrradianceMapScene->addObject("SkyBox", SkyBoxObjectGenerator(IrradianceCumputeShader->getName()).generate());
}

void IBLDiffuseIrradiance::timerTick(lite3d_timer *timerid)
{
    if (timerid == mUpdateTimer)
    {
        // Включаем перересовку по таймеру, каждый кадр обнолвять нет смысла
        mSpecularMapPass->enable();
        mIrradianceMapPass->enable();
    }
}

bool IBLDiffuseIrradiance::beginUpdate(RenderTarget *rt)
{
    if (!mMainCamera)
    {
        return false;
    }

    if (rt == mSpecularMapPass)
    {
        // TODO Обновить матрицы вида
        stl<kmMat4>::vector matrices;
        kmVec3 pos = {0.0f, 0.0f, 3.0f};
        mMainCamera->computeCubeProjView(pos, matrices);
        mPositionViewCubeMatrices->setData(&matrices[0], 0, matrices.size() * sizeof(kmMat4));
    }
    else if (rt == mIrradianceMapPass)
    {
        stl<kmMat4>::vector matrices;
        auto camera = mMain.getCamera(mPipelineName + "_DiffuseIrradiance_Camera");
        camera->computeCubeProjView(matrices);
        mCenteredViewCubeMatrices->setData(&matrices[0], 0, matrices.size() * sizeof(kmMat4));
    }

    return true;
}

void IBLDiffuseIrradiance::postUpdate(RenderTarget *rt)
{
    // После обработка соответствующего прохода, отключим его для дальнейшего включения по таймеру 
    rt->disable();
}


}}
