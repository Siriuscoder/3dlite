#include <lite3dpp_pipeline/lite3dpp_IBL.h>

namespace lite3dpp {
namespace lite3dpp_pipeline {

IBLDiffuseIrradiance::IBLDiffuseIrradiance(Main& mMain) : 
    mMain(mMain)
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

void IBLDiffuseIrradiance::initialize(const String& pipelineName, const ConfigurationReader &pipelineConfig)
{
    mPositionViewCubeMatrices = createMatrixBuffer(pipelineName, "_CubeTransform");
    mCenteredViewCubeMatrices = createMatrixBuffer(pipelineName, "_StaticCubeTransform");

    ConfigurationReader iblConfig = pipelineConfig.getObject(L"IBL");
    createDiffuse(pipelineName, iblConfig);
    //createDiffuseIrradiance(pipelineName, iblConfig);

    // Таймер перерисовки диффузной кубической карты
    mUpdateTimer = mMain.addTimer("EnvironmentUpdateTimer", 1000);
}

VBOResource* IBLDiffuseIrradiance::createMatrixBuffer(const String& pipelineName, const String& bufferName)
{
    auto matrixBuffer = mMain.getResourceManager()->queryResourceFromJson<UBO>(pipelineName + bufferName,
        "{\"Dynamic\": true}");
    
    mResourcesList.emplace_back(matrixBuffer->getName());
    // Выделяем место под 6 матриц, нужны будут для рендера 6ти сторон кубической текстуры
    matrixBuffer->extendBufferBytes(sizeof(kmMat4) * 6);
    return matrixBuffer;
}

void IBLDiffuseIrradiance::createDiffuse(const String& pipelineName, const ConfigurationReader &iblConfig)
{
    auto resolution = iblConfig.getInt(L"Resolution", 256);
    // Создание кубической текстуры окружающего диффузного освещения  
    ConfigurationWriter surroundTextureConfig;
    surroundTextureConfig.set(L"TextureType", "CUBE")
        .set(L"Filtering", "Trilinear")
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB32F");

    mSurroundDiffuseTexture = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        pipelineName + "_surroundDiffuse.texture", surroundTextureConfig.write());
    mResourcesList.emplace_back(mSurroundDiffuseTexture->getName());

    // Создание кубической текстуры глубины для рендера сцены
    surroundTextureConfig.set(L"Filtering", "None")
        .set(L"TextureFormat", "Depth")
        .set(L"InternalFormat", "");

    mSurroundDepthTexture = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        pipelineName + "_surroundDepth.texture", surroundTextureConfig.write());
    mResourcesList.emplace_back(mSurroundDepthTexture->getName());

    ConfigurationWriter surroundDiffusePassConfig;
    surroundDiffusePassConfig.set(L"BackgroundColor", kmVec4 { 0.0f, 0.0f, 0.0f, 1.0f })
        .set(L"Priority", static_cast<int>(RenderPassPriority::SurroundDiffuseMap))
        .set(L"CleanColorBuf", true)
        .set(L"CleanDepthBuf", true)
        .set(L"CleanStencilBuf", false)
        .set(L"Width", resolution)
        .set(L"Height", resolution)
        .set(L"LayeredFramebuffer", true)
        .set(L"ColorAttachments", ConfigurationWriter()
            .set(L"Attachments", stl<ConfigurationWriter>::vector {
                ConfigurationWriter().set(L"TextureName", mSurroundDiffuseTexture->getName())
            }))
        .set(L"DepthAttachments", ConfigurationWriter()
            .set(L"TextureName", mSurroundDepthTexture->getName()));

    mSurroundDiffusePass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        pipelineName + "_surroundDiffuse_Pass", surroundDiffusePassConfig.write());
    mResourcesList.emplace_back(mSurroundDiffusePass->getName());
    mSurroundDiffusePass->addObserver(this);
}

void IBLDiffuseIrradiance::createDiffuseIrradiance(const String& pipelineName, const ConfigurationReader &iblConfig)
{
    auto iresolution = iblConfig.getInt(L"IrradianceResolution", 64);
    // Создание кубической текстуры сожержащей свертку окружающего диффузного освещения
    ConfigurationWriter surroundTextureConfig;
    surroundTextureConfig.set(L"TextureType", "CUBE")
        .set(L"Filtering", "Linear")
        .set(L"Width", iresolution)
        .set(L"Height", iresolution)
        .set(L"Wrapping", "ClampToEdge")
        .set(L"Compression", false)
        .set(L"TextureFormat", "RGB")
        .set(L"InternalFormat", "RGB32F");

    mSurroundIrradianceTexture = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        pipelineName + "_surroundIrradiance.texture", surroundTextureConfig.write());
    mResourcesList.emplace_back(mSurroundIrradianceTexture->getName());
}

void IBLDiffuseIrradiance::timerTick(lite3d_timer *timerid)
{
    if (timerid == mUpdateTimer)
    {
        // Включаем перересовку по таймеру, каждый кадр обнолвять нет смысла
        mSurroundDiffusePass->enable();
        //mSurroundIrradiancePass->enable();
    }
}

bool IBLDiffuseIrradiance::beginUpdate(RenderTarget *rt)
{
    if (!mMainCamera)
    {
        return false;
    }

    if (rt == mSurroundDiffusePass)
    {
        // TODO Обновить матрицы вида
        stl<kmMat4>::vector matrices;
        kmVec3 pos = {0.0f, 0.0f, 3.0f};
        mMainCamera->computeCubeProjView(pos, matrices);
        mPositionViewCubeMatrices->setData(&matrices[0], 0, matrices.size() * sizeof(kmMat4));
    }

    return true;
}

void IBLDiffuseIrradiance::postUpdate(RenderTarget *rt)
{
    // После обработка соответствующего прохода, отключим его для дальнейшего включения по таймеру 
    rt->disable();
}


}}
