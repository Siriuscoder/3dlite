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
    ConfigurationReader iblConfig = pipelineConfig.getObject(L"IBL");

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

    auto iresolution = iblConfig.getInt(L"IrradianceResolution", 64);
    // Создание кубической текстуры сожержащей свертку окружающего диффузного освещения
    surroundTextureConfig.set(L"Width", iresolution)
        .set(L"Height", iresolution)
        .set(L"Filtering", "Linear");

    mSurroundIrradianceTexture = mMain.getResourceManager()->queryResourceFromJson<TextureImage>(
        pipelineName + "_surroundIrradiance.texture", surroundTextureConfig.write());
    mResourcesList.emplace_back(mSurroundIrradianceTexture->getName());

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
            .set(L"Renderbuffer", true));

    mSurroundDiffusePass = mMain.getResourceManager()->queryResourceFromJson<TextureRenderTarget>(
        pipelineName + "_surroundDiffuse_Pass", surroundDiffusePassConfig.write());
    mResourcesList.emplace_back(mSurroundDiffusePass->getName());

    mSurroundDiffusePass->addObserver(this);

  //  auto cubeViewMatrices = mMain.getResourceManager()->queryResourceFromJson<UBO>(pipelineName + "_CubeViewMatrices",
  //      "{\"Dynamic\": false}");
}

void IBLDiffuseIrradiance::timerTick(lite3d_timer *timerid)
{
    // Включаем перересовку по таймеру, каждый кадр обнолвять нет смысла
    mSurroundDiffusePass->enable();
    mSurroundIrradiancePass->enable();
}

bool IBLDiffuseIrradiance::beginUpdate(RenderTarget *rt)
{
    if (rt == mSurroundDiffusePass)
    {
        // TODO Обновить матрицы вида

    }

    return true;
}

void IBLDiffuseIrradiance::postUpdate(RenderTarget *rt)
{
    // После обработка соответствующего прохода, отключим его для дальнейшего включения по таймеру 
    rt->disable();
}


}}
