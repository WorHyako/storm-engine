#include <renderer.h>
#include <RHIModuleWrapper.hpp>

#include <core.h>
#include <SDL_vulkan.h>
#include <sdl_window.hpp>
#include <c_vector.h>
#include <math_inlines.h>
#include <matrix.h>
#include <texture.h>
#include <vma.hpp>

#include "vector4.h"
#include "string_compare.hpp"

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/Paths.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

#include <fmt/chrono.h>

#define POST_PROCESS_FVF (VertexFVFBits::XYZ | VertexFVFBits::UV4)

namespace
{
    constexpr auto kKeyTakeScreenshot = "TakeScreenshot";

#ifdef _WIN32 // Screenshot
    ImageFileFormat GetScreenshotFormat(const std::string& fmt)
    {
        if (fmt == "bmp")
        {
            return ImageFileFormat::IFF_BMP;
        }
        if (fmt == "jpg")
        {
            return ImageFileFormat::IFF_JPG;
        }
        if (fmt == "tga")
        {
            return ImageFileFormat::IFF_TGA;
        }
        if (fmt == "png")
        {
            return ImageFileFormat::IFF_PNG;
        }
        if (fmt == "dds")
        {
            return ImageFileFormat::IFF_DDS;
        }
        if (fmt == "ppm")
        {
            return ImageFileFormat::IFF_PPM;
        }
        if (fmt == "dib")
        {
            return ImageFileFormat::IFF_DIB;
        }
        if (fmt == "hdr")
        {
            return ImageFileFormat::IFF_HDR;
        }
        if (fmt == "pfm")
        {
            return ImageFileFormat::IFF_PFM;
        }

        return ImageFileFormat::IFF_FORCE_DWORD;
    }
#endif

    void InvokeEntitiesLostRender()
    {
        core.ForEachEntity([](entptr_t entity_ptr) { entity_ptr->ProcessStage(Entity::Stage::lost_render); });
    }
    void InvokeEntitiesRestoreRender()
    {
        core.ForEachEntity([](entptr_t entity_ptr) { entity_ptr->ProcessStage(Entity::Stage::restore_render); });
    }

} // namespace

std::uint32_t dwTotalSize = 0;

struct SphereVertex
{
    CVECTOR v;
    std::uint32_t c;
};

std::uint32_t sphereNumTrgs;
SphereVertex* sphereVertex = nullptr;

void CreateSphere()
{
#define CalcKColor(ind)                                                                                                \
    {                                                                                                                  \
        kColor = light | !CVECTOR(sphereVertex[t * 3 + ind].v.x, sphereVertex[t * 3 + ind].v.y,                  \
                                  sphereVertex[t * 3 + ind].v.z);                                                   \
        if (kColor < 0.0f)                                                                                             \
            kColor = 0.0f;                                                                                             \
    }
#define CLerp(c, min) (std::uint32_t(c * (kColor * (1.0f - min) + min)))
#define Color1                                                                                                         \
    ((CLerp(255.0f, 0.5f) << 24) | (CLerp(255.0f, 0.7f) << 16) | (CLerp(255.0f, 0.7f) << 8) |                          \
     (CLerp(255.0f, 0.7f) << 0));

    if (sphereVertex)
        return;

    const float myPI = 3.1415926535897932f;
    const std::int32_t a1 = 32;
    const std::int32_t a2 = (a1 / 2);

    sphereNumTrgs = a1 * a2 * 2;
    sphereVertex = new SphereVertex[sphereNumTrgs * 6];

    const CVECTOR light = !CVECTOR(0.0f, 0.0f, 1.0f);
    float kColor;
    // fill in the vertices
    for (std::int32_t i = 0, t = 0; i < a2; i++)
    {
        const float r1 = sinf(myPI * i / static_cast<float>(a2));
        const float y1 = cosf(myPI * i / static_cast<float>(a2));
        const float r2 = sinf(myPI * (i + 1) / static_cast<float>(a2));
        const float y2 = cosf(myPI * (i + 1) / static_cast<float>(a2));
        for (std::int32_t j = 0; j < a1; j++)
        {
            const float x1 = sinf(2.0f * myPI * j / static_cast<float>(a1));
            const float z1 = cosf(2.0f * myPI * j / static_cast<float>(a1));
            const float x2 = sinf(2.0f * myPI * (j + 1) / static_cast<float>(a1));
            const float z2 = cosf(2.0f * myPI * (j + 1) / static_cast<float>(a1));
            // 0
            sphereVertex[t * 3 + 0].v.x = r1 * x1;
            sphereVertex[t * 3 + 0].v.y = y1;
            sphereVertex[t * 3 + 0].v.z = r1 * z1;
            CalcKColor(0);
            sphereVertex[t * 3 + 0].c = Color1;
            // 1
            sphereVertex[t * 3 + 1].v.x = r2 * x1;
            sphereVertex[t * 3 + 1].v.y = y2;
            sphereVertex[t * 3 + 1].v.z = r2 * z1;
            CalcKColor(1);
            sphereVertex[t * 3 + 1].c = Color1;
            // 2
            sphereVertex[t * 3 + 2].v.x = r1 * x2;
            sphereVertex[t * 3 + 2].v.y = y1;
            sphereVertex[t * 3 + 2].v.z = r1 * z2;
            CalcKColor(2);
            sphereVertex[t * 3 + 2].c = Color1;
            // 3 = 2
            sphereVertex[t * 3 + 3] = sphereVertex[t * 3 + 2];
            // 4 = 1
            sphereVertex[t * 3 + 4] = sphereVertex[t * 3 + 1];
            // 5
            sphereVertex[t * 3 + 5].v.x = r2 * x2;
            sphereVertex[t * 3 + 5].v.y = y2;
            sphereVertex[t * 3 + 5].v.z = r2 * z2;
            CalcKColor(5);
            sphereVertex[t * 3 + 5].c = Color1;
            // added 2 triangles
            t += 2;
        }
    }
}

struct SD_TEXTURE_FORMAT
{
    TX_FORMAT txFormat;
    RHI::Format format;
    bool isSwizzled;
    const char* formatTxt;
};

SD_TEXTURE_FORMAT textureFormats[] = {
    {TXF_DXT1, RHI::Format::BC1_UNORM, true, "BC1_UNORM"},
    {TXF_DXT3, RHI::Format::BC2_UNORM, true, "BC2_UNORM"},
    {TXF_DXT5, RHI::Format::BC3_UNORM, true, "BC3_UNORM"},
    {TXF_A8R8G8B8, RHI::Format::RGBA8_UNORM, false, "RGBA8_UNORM"},
    {TXF_X8R8G8B8, RHI::Format::RGBA8_UNORM, false, "RGBA8_UNORM"},
    {TXF_R5G6B5, RHI::Format::B5G6R5_UNORM, false, "B5G6R5_UNORM"},
    {TXF_A1R5G5B5, RHI::Format::B5G5R5A1_UNORM, false, "B5G5R5A1_UNORM"},
};

#define CHECKERR(expr) ErrorHandler(expr, __FILE__, __LINE__, __func__, #expr)

inline bool ErrorHandler(std::uint32_t hr, const char* file, unsigned line, const char* func, const char* expr)
{
    if (hr != 0)
    {
        core.Trace("[%s:%s:%d] (%s)", file, func, line, expr);
        return true;
    }

    return false;
}

//################################################################################
RENDER::RENDER()
{
    rectsVBuffer = nullptr;

    bPostProcessEnabled = true;
    bPostProcessError = false;
    iBlurPasses = 4;
    GlowIntensity = 200;
    fBlurSize = 0.8f;

    pPostProcessTexture = nullptr;
    pSmallPostProcessTexture = nullptr;
    pSmallPostProcessTexture2 = nullptr;

    pOriginalScreenSurface = nullptr;
    pOriginalDepthTexture = nullptr;

    pRS = this;
    device = nullptr;
    commandList = nullptr;
    aniVBuffer = nullptr;
    numAniVerteces = 0;
    pVTL = nullptr;
    idFontCurrent = 0;

    bLoadTextureEnabled = true;

    bSeaEffect = false;
    fSeaEffectSize = 0.0f;
    fSeaEffectSpeed = 0.0f;
    dwBackColor = 0x0;

    bTrace = true;
    iSetupPath = 0;

    bDropVideoConveyor = false;
    pDropConveyorVBuffer = nullptr;

    aspectRatio = -1.0f;
    viewport = RHI::Viewport{};

    bMakeShoot = false;
    bShowFps = false;
    bShowExInfo = false;
    bInsideScene = false;

    m_fHeightDeformator = 1.f;

    progressImage = nullptr;
    progressImageSize = 0;
    progressTexture = -1;
    progressBackImage = nullptr;
    progressBackImageSize = 0;
    progressTipsImage = nullptr;
    progressTipsImageSize = 0;
    progressTipsTexture = -1;
    loadFrame = 0;
    backTexture = -1;
    back0Texture = -1;
    progressSafeCounter = 0;
    isInPViewProcess = false;
    progressUpdateTime = 0;
    progressFramesPosX = 0.85f;
    progressFramesPosY = 0.8f;
    progressFramesWidth = 64;
    progressFramesHeight = 64;
    progressFramesCountX = 8;
    progressFramesCountY = 8;

    vViewRelationPos = CVECTOR(0.f, 0.f, 0.f);
    vWordRelationPos = -vViewRelationPos;
    bUseLargeBackBuffer = false;
}

static bool texLog = false;
static float fSin = 0.0f;

bool RENDER::Init() {
    if (auto* sentinelService = core.GetService("LostDeviceSentinel"); !sentinelService)
    {
        throw std::runtime_error("Cannot create LostDeviceSentinel! Abort");
    }

    for (std::int32_t i = 0; i < MAX_STEXTURES; i++)
        Textures[i].tex = nullptr;

    device = nullptr;

    std::filesystem::create_directories(Storm::Filesystem::Constants::Paths::screenshots());

    auto config = Storm::Filesystem::Config::Load(Storm::Filesystem::Constants::ConfigNames::engine());
    std::ignore = config.SelectSection("Main");

    bPostProcessEnabled = config.Get<std::int64_t>("PostProcess", 0) == 1;

    // screenshot format and extension
    screenshotExt = config.Get<std::string>("screenshot_format", "jpg");
    screenshotFormat = GetScreenshotFormat(screenshotExt);
    std::ranges::transform(screenshotExt, screenshotExt.begin(),
        [](const unsigned char c) { return std::tolower(c); });
#ifdef _WIN32 // Screenshot
    if (screenshotFormat == ImageFileFormat::IFF_FORCE_DWORD)
    {
        screenshotExt = "jpg";
        screenshotFormat = ImageFileFormat::IFF_JPG;
    }
#endif

    bShowFps = config.Get<std::int64_t>("show_fps", 0) == 1;
    bShowExInfo = config.Get<std::int64_t>("show_exinfo", 0) == 1;
    bSafeRendering = config.Get<std::int64_t>("safe_render", 0) == 0;
    bDropVideoConveyor = config.Get<std::int64_t>("DropVideoConveyor", 0) != 0;
    texLog = config.Get<std::int64_t>("texture_log", 0) == 1;
    bUseLargeBackBuffer = config.Get<std::int64_t>("UseLargeBackBuffer", 0) != 0;
    bWindow = config.Get<std::int64_t>("full_screen", 1) == 0;
    nTextureDegradation = config.Get<std::int64_t>("texture_degradation", 0);
    FovMultiplier = config.Get<double>("fov_multiplier", 1.0f);
    screen_size.x = config.Get<std::int64_t>("screen_x", 1024);
    screen_size.y = config.Get<std::int64_t>("screen_y", 768);
    fNearClipPlane = config.Get<double>("NearClipPlane", 0.1f);
    fFarClipPlane = config.Get<double>("FarClipPlane", 4000.0f);
    bBackBufferCanLock = config.Get<std::int64_t>("lockable_back_buffer", 0) != 0;

    auto back_buffer = config.Get<std::string>("screen_bpp", "FMT_R5G6B5");

    screen_bpp = RHI::Format::B5G6R5_UNORM;// FMT_R5G6B5;
    stencil_format = RHI::Format::D16;// FMT_D16;
    if (storm::iEquals(back_buffer, "FMT_A8R8G8B8"))
    {
        screen_bpp = RHI::Format::RGBA8_SNORM;// FMT_A8R8G8B8;
        stencil_format = RHI::Format::D24S8; // FMT_D24S8;
    }
    /*if (storm::iEquals(back_buffer, "FMT_X8R8G8B8"))
    {
        screen_bpp = FMT_X8R8G8B8;
        stencil_format = FMT_D24S8;
    }*/
    if (storm::iEquals(back_buffer, "FMT_R5G6B5"))
    {
        screen_bpp = RHI::Format::B5G6R5_UNORM;// FMT_R5G6B5;
        stencil_format = RHI::Format::D16;// FMT_D16;
    }

    // new renderer settings
    vSyncEnabled = config.Get<std::int64_t>("vsync", 0);

    msaa = config.Get<std::int64_t>("msaa", 0);

    if (msaa != 0)
    {
        if (msaa < 2 || msaa > 16)
        {
            msaa = 16;
        }
    }

    m_GraphicsAPI = RHI::GraphicsAPI::VULKAN;
    m_RhiModule = RHI::InitializeModuleRHI(m_GraphicsAPI);

    if (m_RhiModule)
    {
        RHI::DeviceParams deviceParams = {};
        deviceParams.useGraphicsQueue = true;
        deviceParams.useComputeQueue = true;
        deviceParams.useTransferQueue = true;
        deviceParams.usePresentQueue = true;
        deviceParams.backBufferWidth = screen_size.x;
        deviceParams.backBufferHeight = screen_size.y;
        deviceParams.vSyncEnabled = vSyncEnabled;

        storm::SDLWindow* window = dynamic_cast<storm::SDLWindow*>(core.GetWindow());
        if (m_GraphicsAPI == RHI::GraphicsAPI::VULKAN)
        {
            if (window)
            {
                //window->GetRequiredExtension(deviceParams.requiredVulkanInstanceExtensions);
                std::uint32_t extensionsCount = 0;
                const char** sdlExtensions;
                if (SDL_Vulkan_GetInstanceExtensions(window->SDLHandle(), &extensionsCount, sdlExtensions))
                {
                    for (std::uint32_t i = 0; i < extensionsCount; i++) {
                        deviceParams.requiredVulkanInstanceExtensions.push_back(sdlExtensions[i]);
                    }
                }
            }
        }

        m_DynamicRHI = m_RhiModule->createRHI(deviceParams);

        if (m_GraphicsAPI == RHI::GraphicsAPI::VULKAN)
        {
            if (RHI::Vulkan::VulkanDynamicRHI* VulkanDynamicRHI = dynamic_cast<RHI::Vulkan::VulkanDynamicRHI*>(m_DynamicRHI))
            {
                if (window)
                {
                    VkSurfaceKHR surface;
                    SDL_Vulkan_CreateSurface(window->SDLHandle(), VulkanDynamicRHI->getVulkanInstance().instance, &surface);
                    VulkanDynamicRHI->setWindowSurface(surface);
                }
            }
        }

        m_DynamicRHI->CreateDevice();
    }

    stencil_format = RHI::Format::D24S8;
    if (!InitDevice(bWindow, screen_size.x, screen_size.y))
        return false;

#ifdef _WIN32 // Effects
    RecompileEffects();
#else
    pTechnique = std::make_unique<CTechnique>(this);
    pTechnique->DecodeFiles();
#endif

    fontIniFileName = config.Get<std::string>("startFontIniFile", "resource\\ini\\fonts.ini");
    // get start ini file for fonts

    if (LoadFont(config.Get<std::string>("font", "normal")) == -1L)
        core.Trace("can not init start font");
    idFontCurrent = 0L;

    std::ignore = config.SelectSection("ProgressImage");
    // Progress image parameters
    progressFramesPosX = config.Get<double>("RelativePosX", 0.85f);
    progressFramesPosY = config.Get<double>("RelativePosY", 0.8f);

    progressFramesWidth = config.Get<double>("RelativeWidth", 0.0625f);
    progressFramesWidth = std::clamp(progressFramesWidth, 0.0f, 10.0f);

    progressFramesHeight = config.Get<double>("RelativeHeight", 0.0625f);
    progressFramesHeight = std::clamp(progressFramesHeight, 0.0f, 10.0f);

    progressFramesCountX = config.Get<std::int64_t>("HorisontalFramesCount", 8);
    progressFramesCountX = std::clamp(progressFramesCountX, 1, 64);

    progressFramesCountY = config.Get<std::int64_t>("VerticalFramesCount", 8);
    progressFramesCountY = std::clamp(progressFramesCountY, 1, 64);

    CreateSphere();
    {
        RHI::BufferDesc desc = {};
        desc.setSize(sphereNumTrgs * 6 * sizeof(SphereVertex))
            .setIsTransferDst(true)
            .setIsVertexBuffer(true)
            .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
        sphereVertexBuffer = device->createBuffer(desc);

        commandList->writeBuffer(sphereVertexBuffer.get(), desc.size, sphereVertex);
    }

    auto* pScriptRender = static_cast<VDATA*>(core.GetScriptVariable("Render"));
    ATTRIBUTES* pARender = pScriptRender->GetAClass();

    pARender->SetAttributeUseDword("full_screen", !bWindow);
    pARender->SetAttributeUseDword("screen_x", screen_size.x);
    pARender->SetAttributeUseDword("screen_y", screen_size.y);

    pDropConveyorVBuffer = nullptr;
    rectsVBuffer = nullptr;
    RHI::BufferDesc bufferDesc = {};
    bufferDesc.setSize(2 * sizeof(CVECTOR))
        .setIsTransferDst(true)
        .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
    pDropConveyorVBuffer = device->createBuffer(bufferDesc);

    bufferDesc.setSize(2 * sizeof(CVECTOR));
    rectsVBuffer = device->createBuffer(bufferDesc);

    if (!pDropConveyorVBuffer || !rectsVBuffer)
    {
        return false;
    }

    uint16_t* pI = &SeaEffectQuadIndices[0];
    // setup ibuffer
    for (std::int32_t y = 0; y < 31; y++)
    {
        for (std::int32_t x = 0; x < 31; x++)
        {
            *pI++ = static_cast<uint16_t>((y + 0) * 32 + x + 1);
            *pI++ = static_cast<uint16_t>((y + 1) * 32 + x);
            *pI++ = static_cast<uint16_t>((y + 0) * 32 + x);

            *pI++ = static_cast<uint16_t>((y + 0) * 32 + x + 1);
            *pI++ = static_cast<uint16_t>((y + 1) * 32 + x + 1);
            *pI++ = static_cast<uint16_t>((y + 1) * 32 + x);
        }
    }

    const auto ctrl = core.Controls->CreateControl(kKeyTakeScreenshot);
    core.Controls->MapControl(ctrl, VK_F8);

    // UNGUARD
    return true;
}

//################################################################################
RENDER::~RENDER()
{
    pOriginalScreenSurface = nullptr;
    pOriginalDepthTexture = nullptr;

    pSmallPostProcessTexture2 = nullptr;
    pPostProcessTexture = nullptr;
    pSmallPostProcessTexture = nullptr;

    rectsVBuffer = nullptr;

    delete progressImage;
    progressImage = nullptr;
    delete progressBackImage;
    progressBackImage = nullptr;
    delete progressTipsImage;
    progressTipsImage = nullptr;

    STORM_DELETE(sphereVertex);
    ReleaseDevice();
}

bool RENDER::InitDevice(bool windowed, std::uint32_t width, std::uint32_t height)
{
    aniVBuffer = nullptr;
    numAniVerteces = 0;

    bWindow = windowed;

    core.Trace("Initializing Render Device");

#ifdef _WIN32 // Effects
    effects_.setDevice(device);
#endif

    if (core.IsEditorEnabled())
    {
        core.InitializeEditor(device);
    }

    // Create render targets for POST PROCESS effects
    //

    fSmallWidth = 128;
    fSmallHeight = 128;

    if (bPostProcessEnabled)
    {
        RHI::TextureDesc desc = {};
        desc.setWidth(width)
            .setHeight(height)
            .setFormat(RHI::Format::RGBA8_UNORM)
            .setIsUAV(true)
            .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
        pPostProcessTexture = device->createImage(desc);

        desc.setWidth(static_cast<int>(fSmallWidth))
            .setHeight(static_cast<int>(fSmallHeight));
        pSmallPostProcessTexture = device->createImage(desc);

        desc.setWidth(static_cast<int>(fSmallWidth * 2))
            .setHeight(static_cast<int>(fSmallHeight * 2));
        pSmallPostProcessTexture2 = device->createImage(desc);
    }

    if (!pPostProcessTexture || !pSmallPostProcessTexture || !pSmallPostProcessTexture2)
    {
        bPostProcessEnabled = false;
        bPostProcessError = true;
    }

    if (!bPostProcessError)
    {
        ClearPostProcessTexture(pPostProcessTexture);
        ClearPostProcessTexture(pSmallPostProcessTexture);
        ClearPostProcessTexture(pSmallPostProcessTexture2);
    }

    ClearPostProcessTexture(pOriginalScreenSurface);

    OriginalViewPort = RHI::Viewport(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), 0.0f, 1.0f);

    for (std::int32_t b = 0; b < MAX_BUFFERS; b++)
    {
        VertexBuffers[b].buff = nullptr;
        IndexBuffers[b].buff = nullptr;
    }

    SetCamera(CVECTOR(0.0f, 0.0f, 0.0f), CVECTOR(0.0f, 0.0f, 0.0f), 1.0f);

    Light light{};
    light.Type = LightType::LIGHT_POINT;
    light.Range = 100.0f;
    light.Attenuation0 = 1.0f;

    for (int i = 0; i < 8; i++)
    {
        lights[i] = light;
        enabledLightsBitMask[i] = false;
    }

    SetCommonStates();

    m_fHeightDeformator = (static_cast<float>(height) * 4.0f) / (static_cast<float>(width) * 3.0f);

    return true;
}

//################################################################################
bool RENDER::ReleaseDevice()
{
    aniVBuffer = nullptr;
    numAniVerteces = 0;
    for (std::int32_t b = 0; b < MAX_BUFFERS; b++)
    {
        VertexBuffers[b].buff = nullptr;
        IndexBuffers[b].buff = nullptr;
    }

    bool res = true;
    for (std::int32_t t = 0; t < MAX_STEXTURES; t++)
    {
        Textures[t].tex = nullptr;
    }

    device = nullptr;
    commandList = nullptr;
    return res;
}

//################################################################################
bool RENDER::Clear(ClearBits type)
{
    if (CHECKERR(d3d9->Clear(0L, NULL, type, dwBackColor, 1.0f, 0L)) == true)
        return false;
    // if(CHECKERR(d3d9->Clear(0L, NULL, type, 0x0, 1.0f, 0L))==true)    return false;
    return true;
}

//################################################################################
RHI::BufferHandle RENDER::CreateRenderQuad(float fWidth, float fHeight, float fSrcWidth, float fSrcHeight, float fMulU,
    float fMulV)
{
    const float StartX = -0.5f;
    const float StartY = -0.5f;
    fWidth -= 0.5f;
    fHeight -= 0.5f;
    PostProcessQuad[0].vPos = Vector4(StartX, fHeight, 0.0f, 1.0f);
    PostProcessQuad[1].vPos = Vector4(StartX, StartY, 0.0f, 1.0f);
    PostProcessQuad[2].vPos = Vector4(fWidth, fHeight, 0.0f, 1.0f);
    PostProcessQuad[3].vPos = Vector4(fWidth, StartY, 0.0f, 1.0f);

    const float fTexelU = 1.0f / fSrcWidth;
    const float fTexelV = 1.0f / fSrcHeight;

    const float fNearV = fTexelV * 0.5f;
    const float fFarV = 1.0f - (fTexelV * 0.5f);

    const float fNearU = fTexelU * 0.5f;
    const float fFarU = 1.0f - (fTexelU * 0.5f);

    PostProcessQuad[0].v0 = fFarV;
    PostProcessQuad[0].u0 = fNearU;
    PostProcessQuad[1].v0 = fNearV;
    PostProcessQuad[1].u0 = fNearU;
    PostProcessQuad[2].v0 = fFarV;
    PostProcessQuad[2].u0 = fFarU;
    PostProcessQuad[3].v0 = fNearV;
    PostProcessQuad[3].u0 = fFarU;

    for (int i = 0; i < 4; i++)
    {
        float u = PostProcessQuad[i].u0;
        float v = PostProcessQuad[i].v0;

        u = u * fMulU;
        v = v * fMulV;

        PostProcessQuad[i].u0 = u - (fTexelU * fBlurSize);
        PostProcessQuad[i].v0 = v - (fTexelV * fBlurSize);

        PostProcessQuad[i].u1 = u + (fTexelU * fBlurSize);
        PostProcessQuad[i].v1 = v - (fTexelV * fBlurSize);

        PostProcessQuad[i].u2 = u - (fTexelU * fBlurSize);
        PostProcessQuad[i].v2 = v + (fTexelV * fBlurSize);

        PostProcessQuad[i].u3 = u + (fTexelU * fBlurSize);
        PostProcessQuad[i].v3 = v + (fTexelV * fBlurSize);
    }

    RHI::BufferDesc desc = {};
    desc.setSize(4 * sizeof(QuadVertex))
        .setIsTransferDst(true)
        .setIsVertexBuffer(true)
        .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
    RHI::BufferHandle buffer = device->createBuffer(desc);
    commandList->writeBuffer(buffer.get(), desc.size, PostProcessQuad);

    return buffer;
}

void RENDER::BlurGlowTexture()
{
    // Render everything to a small texture
    RHI::BufferHandle postProcessVertexBuffer = CreateRenderQuad(fSmallWidth * 2.0f, fSmallHeight * 2.0f, 1024.0f, 1024.0f);

    SetTexture(0, pPostProcessTexture);
    SetTexture(1, pPostProcessTexture);
    SetTexture(2, pPostProcessTexture);
    SetTexture(3, pPostProcessTexture);
    SetRenderTarget(pSmallPostProcessTexture2, nullptr);
    DrawPrimitive(RHI::PrimitiveType::TriangleStrip, POST_PROCESS_FVF, 4, 1, 0, postProcessVertexBuffer, sizeof(QuadVertex), "PostProcessBlur");

    // pre-blur iBlurPasses times
    for (int i = 0; i < iBlurPasses; i++)
    {
        RHI::BufferHandle postProcessBlurVertexBuffer1 = CreateRenderQuad(fSmallWidth, fSmallHeight, fSmallWidth * 2.0f, fSmallHeight * 2.0f);

        SetTexture(0, pSmallPostProcessTexture2);
        SetTexture(1, pSmallPostProcessTexture2);
        SetTexture(2, pSmallPostProcessTexture2);
        SetTexture(3, pSmallPostProcessTexture2);
        SetRenderTarget(pSmallPostProcessTexture, nullptr);
        DrawPrimitive(RHI::PrimitiveType::TriangleStrip, POST_PROCESS_FVF, 4, 1, 0, postProcessBlurVertexBuffer1, sizeof(QuadVertex),
            "PostProcessBlur");

        RHI::BufferHandle postProcessBlurVertexBuffer2 = CreateRenderQuad(fSmallWidth * 2.0f, fSmallHeight * 2.0f, fSmallWidth, fSmallHeight);

        SetTexture(0, pSmallPostProcessTexture);
        SetTexture(1, pSmallPostProcessTexture);
        SetTexture(2, pSmallPostProcessTexture);
        SetTexture(3, pSmallPostProcessTexture);
        SetRenderTarget(pSmallPostProcessTexture2, nullptr);
        DrawPrimitive(RHI::PrimitiveType::TriangleStrip, POST_PROCESS_FVF, 4, 1, 0, postProcessBlurVertexBuffer2, sizeof(QuadVertex),
            "PostProcessBlur");
    }

    RHI::BufferHandle postProcessBlurVertexBuffer = CreateRenderQuad(fSmallWidth, fSmallHeight, fSmallWidth * 2.0f, fSmallHeight * 2.0f);
    SetTexture(0, pSmallPostProcessTexture2);
    SetTexture(1, pSmallPostProcessTexture2);
    SetTexture(2, pSmallPostProcessTexture2);
    SetTexture(3, pSmallPostProcessTexture2);
    SetRenderTarget(pSmallPostProcessTexture, nullptr);
    DrawPrimitive(RHI::PrimitiveType::TriangleStrip, POST_PROCESS_FVF, 4, 1, 0, postProcessBlurVertexBuffer, sizeof(QuadVertex), "PostProcessBlur");
}

void RENDER::CopyGlowToScreen()
{
    const auto sx = static_cast<float>(screen_size.x);
    const auto sy = static_cast<float>(screen_size.y);
    // Render to screen
    PostProcessQuad[0].vPos = Vector4(0, sy, 0.0f, 1.0f);
    PostProcessQuad[1].vPos = Vector4(0, 0, 0.0f, 1.0f);
    PostProcessQuad[2].vPos = Vector4(sx, sy, 0.0f, 1.0f);
    PostProcessQuad[3].vPos = Vector4(sx, 0, 0.0f, 1.0f);

    PostProcessQuad[0].v0 = 1.0f;
    PostProcessQuad[0].u0 = 0.0f;
    PostProcessQuad[1].v0 = 0.0f;
    PostProcessQuad[1].u0 = 0.0f;
    PostProcessQuad[2].v0 = 1.0f;
    PostProcessQuad[2].u0 = 1.0f;
    PostProcessQuad[3].v0 = 0.0f;
    PostProcessQuad[3].u0 = 1.0f;

    RHI::BufferDesc desc = {};
    desc.setSize(4 * sizeof(QuadVertex))
        .setIsTransferDst(true)
        .setIsVertexBuffer(true)
        .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
    RHI::BufferHandle postProcessQuadBuffer = device->createBuffer(desc);
    commandList->writeBuffer(postProcessQuadBuffer.get(), desc.size, PostProcessQuad);

    SetRenderTarget(pOriginalScreenSurface, pOriginalDepthTexture);

    if (GlowIntensity < 0)
        GlowIntensity = 0;
    if (GlowIntensity > 255)
        GlowIntensity = 255;
    const uint8_t bGLOW = static_cast<uint8_t>(GlowIntensity);
    const std::uint32_t dwTFactor = (bGLOW << 24) | (bGLOW << 16) | (bGLOW << 8) | bGLOW;

    // Draw the GLOW screen
    textureFactorColor = dwTFactor;

    SetTexture(0, pSmallPostProcessTexture);
    SetTexture(1, pSmallPostProcessTexture);
    SetTexture(2, pSmallPostProcessTexture);
    SetTexture(3, pSmallPostProcessTexture);

    DrawPrimitive(RHI::PrimitiveType::TriangleStrip, POST_PROCESS_FVF, 4, 1, 0, postProcessQuadBuffer, sizeof(QuadVertex), "PostProcessGlow");
}

void RENDER::CopyPostProcessToScreen()
{
    const auto sx = static_cast<float>(screen_size.x);
    const auto sy = static_cast<float>(screen_size.y);
    PostProcessQuad[0].vPos = Vector4(0, sy, 0.0f, 1.0f);
    PostProcessQuad[1].vPos = Vector4(0, 0, 0.0f, 1.0f);
    PostProcessQuad[2].vPos = Vector4(sx, sy, 0.0f, 1.0f);
    PostProcessQuad[3].vPos = Vector4(sx, 0, 0.0f, 1.0f);

    PostProcessQuad[0].v0 = 1.0f;
    PostProcessQuad[0].u0 = 0.0f;
    PostProcessQuad[1].v0 = 0.0f;
    PostProcessQuad[1].u0 = 0.0f;
    PostProcessQuad[2].v0 = 1.0f;
    PostProcessQuad[2].u0 = 1.0f;
    PostProcessQuad[3].v0 = 0.0f;
    PostProcessQuad[3].u0 = 1.0f;

    RHI::BufferDesc desc = {};
    desc.setSize(4 * sizeof(QuadVertex))
        .setIsTransferDst(true)
        .setIsVertexBuffer(true)
        .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
    RHI::BufferHandle postProcessQuadBuffer = device->createBuffer(desc);
    commandList->writeBuffer(postProcessQuadBuffer.get(), desc.size, PostProcessQuad);

    SetRenderTarget(pOriginalScreenSurface, pOriginalDepthTexture);

    // Draw the original screen
    SetTexture(0, pPostProcessTexture);
    SetTexture(1, pPostProcessTexture);
    SetTexture(2, pPostProcessTexture);
    SetTexture(3, pPostProcessTexture);

    if (bSeaEffect)
    {
        vertexFormat = POST_PROCESS_FVF;
        DrawIndexedPrimitiveUP(RHI::PrimitiveType::TriangleList, 0, 32 * 32, 31 * 31 * 2, SeaEffectQuadIndices, RHI::Format::R16_UINT, SeaEffectQuadVertices, sizeof(QuadVertex),
            "PostProcess");
    }
    else
    {
        DrawPrimitive(RHI::PrimitiveType::TriangleStrip, POST_PROCESS_FVF, 4, 1, 0, postProcessQuadBuffer, sizeof(QuadVertex), "PostProcess");
    }
}

void RENDER::ClearPostProcessTexture(RHI::TextureHandle texture)
{
    commandList->clearColorTexture(texture.get(), RHI::TextureSubresourse(), RHI::Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void RENDER::SetScreenAsRenderTarget()
{
    SetRenderTarget(pOriginalScreenSurface, pOriginalDepthTexture);
    SetViewport(OriginalViewPort);
}

void RENDER::SetPostProcessTextureAsRenderTarget()
{
    SetRenderTarget(pPostProcessTexture, pOriginalDepthTexture);
    SetViewport(OriginalViewPort);
}

void RENDER::MakePostProcess()
{
    if (bPostProcessError)
        return;
    if (!bSeaEffect && !bPostProcessEnabled)
        return;
    if (!bNeedCopyToScreen)
        return;

    commandList->beginSingleTimeCommands();

    bNeedCopyToScreen = false;

    CopyPostProcessToScreen();
    BlurGlowTexture();
    CopyGlowToScreen();

    commandList->endSingleTimeCommands();

    SetScreenAsRenderTarget();
    /*
    Print (0, 0, "Blur = %3.2f",    fBlurSize);
    Print (0, 16, "Intensity = %d",    GlowIntensity);
    Print (0, 32, "BlurPasses = %d",    iBlurPasses);
    */
}

//################################################################################
bool RENDER::BeginScene()
{
    return true;
}

bool RENDER::EndScene()
{
    if (bShowFps)
    {
        Print(screen_size.x - 100, screen_size.y - 50, "FPS %d", core.EngineFps());
    }

    if (bShowExInfo)
    {
        std::uint32_t dwTotalTexSize = 0;
        std::uint32_t dwTotalTexNum = 0, dwTotalVB = 0, dwTotalIB = 0, dwTotalVBSize = 0, dwTotalIBSize = 0;

        std::int32_t t;
        for (t = 0; t < MAX_STEXTURES; t++)
            if (Textures[t].tex != nullptr)
            {
                dwTotalTexSize += Textures[t].tex->getDesc().width;
                dwTotalTexNum++;
            }

        for (t = 0; t < MAX_BUFFERS; t++)
        {
            if (VertexBuffers[t].buff != nullptr)
            {
                dwTotalVBSize += VertexBuffers[t].size;
                dwTotalVB++;
            }
            if (IndexBuffers[t].buff != nullptr)
            {
                dwTotalIBSize += IndexBuffers[t].size;
                dwTotalIB++;
            }
        }

        mView.Transposition();
        Print(80, 50, "Cam: %.3f, %.3f, %.3f", mView.Pos().x, mView.Pos().y, mView.Pos().z);
        Print(80, 70, "t : %d, %.3f Mb", dwTotalTexNum, float(dwTotalTexSize) / (1024.0f * 1024.0f));
        Print(80, 90, "v : %d, %.3f Mb", dwTotalVB, float(dwTotalVBSize) / (1024.0f * 1024.0f));
        Print(80, 110, "i : %d, %.3f Mb", dwTotalIB, float(dwTotalIBSize) / (1024.0f * 1024.0f));
        Print(80, 130, "d : %d, lv: %d, li: %d", dwNumDrawPrimitive, dwNumLV, dwNumLI);
        Print(80, 150, "s : %d, %.3f, %.3f", dwSoundBuffersCount, dwSoundBytes / 1024.f, dwSoundBytesCached / 1024.f);
    }

    // Try to drop video conveyor
    if (bDropVideoConveyor && pDropConveyorVBuffer)
    {
        CVECTOR pV[3];
        for (std::int32_t i = 0; i < 2; i++)
            pV[i] = CVECTOR(1e6f, 1e6f, 1e6f);
        commandList->writeBuffer(pDropConveyorVBuffer.get(), sizeof(pV), &pV[0]);
        vertexFormat = VertexFVFBits::XYZ;
        DrawPrimitive(RHI::PrimitiveType::LineList, vertexFormat, 2, 1, 0, pDropConveyorVBuffer, sizeof(CVECTOR));
    }

    if (EndScene())
        return false;

    if (auto* editor = core.GetEditor(); editor != nullptr) {
        editor->EndFrame();
    }

    if (bMakeShoot)
        MakeScreenShot();

    const HRESULT hRes = d3d9->Present(nullptr, nullptr, nullptr, nullptr);

    if (hRes == D3DERR_DEVICELOST)
    {
        LostRender();
    }

    return true;
}

//################################################################################
static int totSize = 0;

std::int32_t RENDER::TextureCreate(const char* fname)
{
    // start add texture path
    if ((uintptr_t)fname == -1)
    {
        iSetupPath = 1;
        return -1;
    }

    // continue add texture path process
    if (iSetupPath)
    {
        if (iSetupPath == 1)
        {
            dwSetupNumber = (uintptr_t)(fname);
            iSetupPath = 2;
            return -1;
        }

        TexPaths[dwSetupNumber].str[0] = 0;
        if (fname && fname[0])
            strcpy_s(TexPaths[dwSetupNumber].str, fname);
        iSetupPath = 0;
        return -1;
    }

    if (fname == nullptr)
    {
        core.Trace("Can't create texture with null name");
        return -1L;
    }

    if (!bLoadTextureEnabled)
        return -1;

    for (std::int32_t i = 3; i >= -1; i--)
    {
        char _fname[256];

        if (i >= 0 && TexPaths[i].str[0] == 0)
            continue;
        if (i >= 0)
        {
            const std::uint32_t dwLen = strlen(fname);

            std::int32_t j;
            for (j = dwLen - 1; j >= 0; j--)
                if (fname[j] == '\\')
                    break;

            _fname[0] = 0;
            strncpy_s(_fname, fname, j + 1);
            _fname[j + 1] = 0;
            strcat_s(_fname, TexPaths[i].str);
            strcat_s(_fname, &fname[j + 1]);
            bTrace = false;
        }
        else
        {
            bTrace = true;
            strcpy_s(_fname, fname);
        }

        std::ranges::for_each(_fname, [](char& c) { c = std::toupper(c); });

        const std::uint32_t hf = MakeHashValue(_fname);

        std::int32_t t;
        for (t = 0; t < MAX_STEXTURES; t++)
            if (Textures[t].ref != 0)
                if (Textures[t].name)
                    if (Textures[t].hash == hf && storm::iEquals(Textures[t].name, _fname))
                    {
                        Textures[t].ref++;
                        return t;
                    }

        for (t = 0; t < MAX_STEXTURES; t++)
            if (Textures[t].ref == 0)
                break;

        Textures[t].hash = hf;

        const auto len = strlen(_fname) + 1;
        if ((Textures[t].name = new char[len]) == nullptr)
            throw std::runtime_error("allocate memory error");
        strcpy_s(Textures[t].name, len, _fname);
        Textures[t].isCubeMap = false;
        Textures[t].loaded = false;
        Textures[t].ref++;
        if (TextureLoad(t))
            return t;
        Textures[t].ref--;
        STORM_DELETE(Textures[t].name);
    }
    return -1;
}

std::int32_t RENDER::TextureCreate(std::uint32_t width, std::uint32_t height, std::uint32_t levels, std::uint32_t usage, RHI::Format format, RHI::MemoryPropertiesBits pool)
{
    std::int32_t t;
    for (t = 0; t < MAX_STEXTURES; t++)
    {
        if (Textures[t].tex == nullptr)
        {
            break;
        }
    }

    RHI::TextureDesc desc = {};
    desc.setWidth(width)
        .setHeight(height)
        .setMipLevels(levels)
        .setFormat(format)
        .setMemoryProperties(pool);

    Textures[t].tex = device->createImage(desc);
    Textures[t].name = nullptr;
    Textures[t].hash = 0;
    Textures[t].ref = 1;
    Textures[t].dwSize = width * height * 4; // Assuming 32-bit pixels
    Textures[t].isCubeMap = false;
    Textures[t].loaded = true;

    return t;
}

bool RENDER::TextureIncReference(std::int32_t texid)
{
    ++Textures[texid].ref;
    return true;
}

bool RENDER::TextureLoad(std::int32_t t)
{
    using namespace std::literals;

    ProgressView();
    // Form the path to the texture
    char fn[_MAX_FNAME];
    Textures[t].dwSize = 0;
    if (Textures[t].name == nullptr)
    {
        return false;
    }

    auto lTexture = std::string(Textures[t].name);
    std::transform(lTexture.begin(), lTexture.end(), lTexture.begin(), [](unsigned char c) { return std::tolower(c); });
    auto has_resource_prefix = starts_with(lTexture, "resource\\textures\\");
    auto has_tx_postfix = ends_with(lTexture, ".tx");

    sprintf_s(fn, "%s%s%s", has_resource_prefix ? "" : "resource\\textures\\", Textures[t].name, has_tx_postfix ? "" : ".tx");

    for (std::int32_t s = 0, d = 0; fn[d]; s++)
    {
        if (d > 0 && (fn[d - 1] == PATH_SEP || fn[d - 1] == WRONG_PATH_SEP) &&
            (fn[s] == PATH_SEP || fn[s] == WRONG_PATH_SEP))
        {
            continue;
        }
        fn[d++] = fn[s];
    }
    // Opening the file
    auto fileS = fio->_CreateFile(fn, std::ios::binary | std::ios::in);
    if (!fileS.is_open())
    {
        // try to load without '.tx' (e.g. raw Targa)
        std::filesystem::path path_to_tex{ fn };
        path_to_tex.replace_extension();
        if (exists(path_to_tex))
        {
            return TextureLoadUsingD3DX(path_to_tex.string().c_str(), t);
        }
        if (bTrace)
        {
            core.Trace("Can't load texture %s", fn);
        }
        delete Textures[t].name;
        Textures[t].name = nullptr;
        return false;
    }
    // Reading the header
    TX_FILE_HEADER head;
    if (!fio->_ReadFile(fileS, &head, sizeof(head)))
    {
        if (bTrace)
        {
            core.Trace("Can't load texture %s", fn);
        }
        delete Textures[t].name;
        Textures[t].name = nullptr;
        fio->_CloseFile(fileS);
        return false;
    }
    // Analyzing the format
    RHI::Format format = RHI::Format::UNKNOWN;
    std::int32_t textureFI;
    for (textureFI = 0; textureFI < sizeof(textureFormats) / sizeof(SD_TEXTURE_FORMAT); textureFI++)
    {
        if (textureFormats[textureFI].txFormat == head.format)
        {
            break;
        }
    }
    if (textureFI == sizeof(textureFormats) / sizeof(SD_TEXTURE_FORMAT) || head.flags & TX_FLAGS_PALLETTE)
    {
        if (bTrace)
        {
            core.Trace("Invalidate texture format %s, not loading it.", fn);
        }
        delete Textures[t].name;
        Textures[t].name = nullptr;
        fio->_CloseFile(fileS);
        return false;
    }
    format = textureFormats[textureFI].format;
    bool isSwizzled = textureFormats[textureFI].isSwizzled;
    const char* formatTxt = textureFormats[textureFI].formatTxt;
    // Skipping mips
    std::uint32_t seekposition = 0;
    for (std::int32_t nTD = nTextureDegradation; nTD > 0; nTD--)
    {
        if (head.nmips <= 1 || head.width <= 32 || head.height <= 32)
        {
            break; // degradation limit
        }
        seekposition += head.mip_size;
        head.nmips--;
        head.width /= 2;
        head.height /= 2;
        head.mip_size /= 4;
    }
    // Loading the texture
    if (!(head.flags & TX_FLAGS_CUBEMAP))
    {
        // Loading a regular texture
        // Position in file
        if (seekposition)
        {
            fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
        }
        // create the texture
        RHI::TextureDesc texDesc{};
        texDesc.setWidth(head.width)
            .setHeight(head.height)
            .setMipLevels(head.nmips)
            .setFormat(format);
        RHI::TextureHandle tex {device->createImage(texDesc)};
        if (!tex)
        {
            if (bTrace)
            {
                core.Trace(
                    "Texture %s is not created (width: %i, height: %i, num mips: %i, format: %s), not loading it.", fn,
                    head.width, head.height, head.nmips, formatTxt);
            }
            delete Textures[t].name;
            Textures[t].name = nullptr;
            fio->_CloseFile(fileS);
            return false;
        }
        // Filling the levels
        for (std::int32_t m = 0; m < head.nmips; m++)
        {
            // take into account the size of the mip
            Textures[t].dwSize += head.mip_size;
            // Getting the mip surface
            bool isError = false;
            RHI::TextureHandle surface = nullptr;
            if (CHECKERR(tex->GetSurfaceLevel(m, &surface)) == true || !surface)
            {
                isError = true;
            }
            else
            {
                // read the mip
                isError = !LoadTextureSurface(fileS, surface, head.mip_size, head.width, head.height, isSwizzled);
            }
            // If there was an error, then interrupt the download
            if (isError)
            {
                if (bTrace)
                {
                    core.Trace("Can't loading mip %i, texture %s is not created (width: %i, height: %i, num mips: %i, "
                        "format: %s), not loading it.",
                        m, fn, head.width, head.height, head.nmips, formatTxt);
                }
                delete Textures[t].name;
                Textures[t].name = nullptr;
                fio->_CloseFile(fileS);
                tex = nullptr;
                return false;
            }
            // recalculate the dimensions for the next mip
            head.width /= 2;
            head.height /= 2;
            head.mip_size /= 4;
        }
        Textures[t].tex = tex;
        Textures[t].isCubeMap = false;
    }
    else
    {
        // Download cubemap
        if (head.width != head.height)
        {
            if (bTrace)
            {
                core.Trace("Cube map texture can't has not squared sides %s, not loading it.", fn);
            }
            delete Textures[t].name;
            Textures[t].name = nullptr;
            fio->_CloseFile(fileS);
            return false;
        }

        // create the texture
        RHI::TextureDesc desc = {};
        desc.setWidth(head.width)
            .setHeight(head.width)
            .setMipLevels(head.nmips)
            .setFormat(format)
            .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT) // RHI::MemoryPropertiesBits::HOST_VISIBLE_BIT | RHI::MemoryPropertiesBits::HOST_COHERENT_BIT
            .setDimension(RHI::TextureDimension::TextureCube);
        RHI::TextureHandle tex = device->createImage(desc);
        if (!tex)
        {
            if (bTrace)
            {
                core.Trace("Cube map texture %s is not created (size: %i, num mips: %i, format: %s), not loading it.",
                    fn, head.width, head.nmips, formatTxt);
            }
            delete Textures[t].name;
            Textures[t].name = nullptr;
            fio->_CloseFile(fileS);
            return false;
        }
        // Loading the sides
        bool isError = false;
        if (seekposition)
        {
            fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
        }
        std::uint32_t sz =
            LoadCubmapSide(fileS, tex, CubemapFaces::CUBEMAP_FACE_POSITIVE_Z, head.nmips, head.mip_size, head.width, isSwizzled);
        if (sz)
        {
            Textures[t].dwSize += sz;
            if (seekposition)
            {
                fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
            }
            sz = LoadCubmapSide(fileS, tex, CubemapFaces::CUBEMAP_FACE_POSITIVE_X, head.nmips, head.mip_size, head.width,
                isSwizzled);
            if (sz)
            {
                Textures[t].dwSize += sz;
                if (seekposition)
                {
                    fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
                }
                sz = LoadCubmapSide(fileS, tex, CubemapFaces::CUBEMAP_FACE_NEGATIVE_Z, head.nmips, head.mip_size, head.width,
                    isSwizzled);
                if (sz)
                {
                    Textures[t].dwSize += sz;
                    if (seekposition)
                    {
                        fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
                    }
                    sz = LoadCubmapSide(fileS, tex, CubemapFaces::CUBEMAP_FACE_NEGATIVE_X, head.nmips, head.mip_size, head.width,
                        isSwizzled);
                    if (sz)
                    {
                        Textures[t].dwSize += sz;
                        if (seekposition)
                        {
                            fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
                        }
                        sz = LoadCubmapSide(fileS, tex, CubemapFaces::CUBEMAP_FACE_POSITIVE_Y, head.nmips, head.mip_size,
                            head.width, isSwizzled);
                        if (sz)
                        {
                            Textures[t].dwSize += sz;
                            if (seekposition)
                            {
                                fio->_SetFilePointer(fileS, seekposition, std::ios::cur);
                            }
                            sz = LoadCubmapSide(fileS, tex, CubemapFaces::CUBEMAP_FACE_NEGATIVE_Y, head.nmips, head.mip_size,
                                head.width, isSwizzled);
                            if (!sz)
                            {
                                isError = true;
                            }
                            Textures[t].dwSize += sz;
                        }
                        else
                        {
                            isError = true;
                        }
                    }
                    else
                    {
                        isError = true;
                    }
                }
                else
                {
                    isError = true;
                }
            }
            else
            {
                isError = true;
            }
        }
        else
        {
            isError = true;
        }

        if (isError)
        {
            if (bTrace)
            {
                core.Trace("Cube map texture %s can't loading (size: %i, num mips: %i, format: %s), not loading it.",
                    fn, head.width, head.nmips, formatTxt);
            }
            delete Textures[t].name;
            Textures[t].name = nullptr;
            fio->_CloseFile(fileS);
            tex = nullptr;
            return false;
        }
        Textures[t].tex = tex;
        Textures[t].isCubeMap = true;
    }

    //---------------------------------------------------------------
    // print statistics
    //---------------------------------------------------------------
    if (texLog)
    {
        char s[256];
        if (totSize == 0)
        {
            fio->_DeleteFile("texLoad.txt");
        }
        auto fileS2 = fio->_CreateFile("texLoad.txt", std::ios::binary | std::ios::out | std::ios::app);
        totSize += Textures[t].dwSize;
        sprintf_s(s, "%.2f, size: %d, %d * %d, %s\n", totSize / 1024.0f / 1024.0f, Textures[t].dwSize, head.width,
            head.height, Textures[t].name);
        fio->_WriteFile(fileS2, s, strlen(s));
        fio->_FlushFileBuffers(fileS2);
        fio->_CloseFile(fileS2);
    }
    dwTotalSize += Textures[t].dwSize;
    //---------------------------------------------------------------
    Textures[t].loaded = true;
    // Close the file
    fio->_CloseFile(fileS);
    return true;
}

std::uint32_t RENDER::LoadCubmapSide(std::fstream& fileS, RHI::TextureHandle tex, CubemapFaces face,
    std::uint32_t numMips, std::uint32_t mipSize, std::uint32_t size, bool isSwizzled)
{
    std::uint32_t texsize = 0;
    // Filling the levels
    for (std::uint32_t m = 0; m < numMips; m++)
    {
        // take into account the size of the mip
        texsize += mipSize;
        // Getting the mip surface
        bool isError = false;
        RHI::TextureHandle surface = nullptr;
        if (CHECKERR(tex->GetCubeMapSurface(face, m, &surface)) == true || !surface)
        {
            isError = true;
        }
        else
        {
            // read the mip
            isError = !LoadTextureSurface(fileS, surface, mipSize, size, size, isSwizzled);
        }
        // If there was an error, then interrupt the download
        if (isError)
        {
            if (bTrace)
            {
                core.Trace("Can't loading cubemap mip %i (side: %i), not loading it.", m, face);
            }
            return 0;
        }
        // recalculate the dimensions for the next mip
        size /= 2;
        mipSize /= 4;
    }
    return texsize;
}

//################################################################################
bool RENDER::TextureSet(std::uint32_t textureIndex, std::uint32_t textureBindingIndex, RHI::SamplerHandle sampler, RHI::DescriptorSetInfo& dsInfos)
{
    if (textureIndex >= Textures.size())
    {
        return false;
    }

    SetTexture(Textures[textureIndex].tex, sampler, textureBindingIndex, dsInfos);

    return true;
}

//################################################################################
bool RENDER::TextureRelease(std::int32_t texid)
{
    if (texid == -1)
    {
        return true;
    }
    Textures[texid].ref--;
    if (Textures[texid].ref != 0)
    {
        return false;
    }
    if (Textures[texid].name != nullptr)
    {
        if (texLog)
        {
            auto fileS = fio->_CreateFile("texLoad.txt", std::ios::binary | std::ios::in | std::ios::out);
            const int bytes = fio->_GetFileSize("texLoad.txt");
            auto buf = new char[bytes + 1];
            fio->_ReadFile(fileS, buf, bytes);
            buf[bytes] = 0;

            char* str = strstr(buf, Textures[texid].name);
            if (str != nullptr)
            {
                fio->_SetFilePointer(fileS, str - buf, std::ios::beg);
                auto s = "*";
                fio->_WriteFile(fileS, s, 1);
            }
            delete[] buf;
            fio->_FlushFileBuffers(fileS);
            fio->_CloseFile(fileS);
        }

        delete Textures[texid].name;
        Textures[texid].name = nullptr;
    }
    if (Textures[texid].loaded == false)
    {
        return false;
    }

    if (Textures[texid].tex)
    {
        Textures[texid].tex = nullptr;
    }
    Textures[texid].tex = nullptr;
    dwTotalSize -= Textures[texid].dwSize;

    return true;
}

//################################################################################
bool RENDER::SetCamera(const CVECTOR& pos, const CVECTOR& ang, float fov)
{
    if (!SetCamera(pos, ang))
        return false;
    if (!SetPerspective(fov, aspectRatio))
        return false;

    ProcessScriptPosAng(pos, ang);
    return true;
}

bool RENDER::SetCamera(const CVECTOR& pos, const CVECTOR& ang)
{
    CMatrix mtx;
    const CVECTOR vOldWordRelationPos = vWordRelationPos;
    {
        mtx.BuildMatrix(ang);
        mtx.Transposition3X3();
        // mtx.SetInversePosition(pos->x, pos->y, pos->z);
        Pos = pos;
        Ang = ang;
        vWordRelationPos = -Pos;
    }

    mView = mtx;
    vViewRelationPos = -(mtx * vWordRelationPos);
    mtx = mWorld;
    mtx.Pos() += vWordRelationPos - vOldWordRelationPos;
    mWorld = mtx;

    FindPlanes();
    ProcessScriptPosAng(pos, ang);
    return true;
}

bool RENDER::SetCamera(CVECTOR lookFrom, CVECTOR lookTo, CVECTOR up)
{
    CMatrix mtx;
    if (!mtx.BuildViewMatrix(lookFrom, lookTo, up))
        return false;
    mView = mtx;
    Pos = lookFrom;

    // calculate the angle of the camera
    // Ang = 0.0f;
    const CVECTOR vNorm = !(lookTo - lookFrom);
    Ang.y = atan2f(vNorm.x, vNorm.z);
    Ang.x = atan2f(-vNorm.y, sqrtf(vNorm.x * vNorm.x + vNorm.z * vNorm.z));
    Ang.z = 0.f;

    FindPlanes();
    ProcessScriptPosAng(lookFrom, Ang);
    return true;
}

void RENDER::ProcessScriptPosAng(const CVECTOR& vPos, const CVECTOR& vAng)
{
    core.Event("CameraPosAng", "ffffff", vPos.x, vPos.y, vPos.z, vAng.x, vAng.y, vAng.z);
}

void RENDER::GetNearFarPlane(float& fNear, float& fFar)
{
    fNear = fNearClipPlane;
    fFar = fFarClipPlane;
}

void RENDER::SetNearFarPlane(float fNear, float fFar)
{
    fNearClipPlane = fNear;
    fFarClipPlane = fFar;

    SetPerspective(Fov, aspectRatio);
}

bool RENDER::SetPerspective(float perspective, float fAspectRatio)
{
    perspective *= FovMultiplier;

    const float near_plane = fNearClipPlane; // Distance to near clipping
    const float far_plane = fFarClipPlane;   // Distance to far clipping
    const float fov_horiz = perspective;     // Horizontal field of view  angle, in radians
    if (fAspectRatio < 0)
    {
        fAspectRatio = static_cast<float>(screen_size.y) / screen_size.x;
    }
    aspectRatio = fAspectRatio;
    const float fov_vert = 2.f * atanf(tanf(perspective / 2.f) * fAspectRatio); // Vertical field of view  angle, in radians

    const float w = 1.0f / tanf(fov_horiz * 0.5f);
    const float h = 1.0f / tanf(fov_vert * 0.5f);
    const float Q = far_plane / (far_plane - near_plane);

    CMatrix mtx{};

    mtx.m[0][0] = w;
    mtx.m[1][1] = h;
    mtx.m[2][2] = Q;
    mtx.m[3][2] = -Q * near_plane;
    mtx.m[2][3] = 1.0f;

    mProjection = mtx;
    Fov = perspective;
    FindPlanes();
    return true;
}

//################################################################################
bool RENDER::SetCurrentMatrix(D3DMATRIX* mtx)
{
    mWorld = mtx;
    return true;
}

bool RENDER::SetMaterial(Material& m)
{
    material = m;
    return true;
}

bool RENDER::SetLight(std::uint32_t dwIndex, const Light* pLight)
{
    // Set the property information for the first light.
    Light tmpLight = *pLight;
    tmpLight.Position.x += vWordRelationPos.x;
    tmpLight.Position.y += vWordRelationPos.y;
    tmpLight.Position.z += vWordRelationPos.z;
    if (dwIndex < lights.size())
    {
        lights[dwIndex] = tmpLight;
        return true;
    }

    return false;
}

bool RENDER::LightEnable(std::uint32_t dwIndex, bool bOn)
{
    if(dwIndex < lights.size())
    {
        enabledLightsBitMask[dwIndex] = bOn ? true : false;
    	return true;
    }

    return false;
}

bool RENDER::GetLightEnable(std::uint32_t dwIndex, bool* pEnable) const
{
    if(dwIndex < lights.size())
    {
        *pEnable = enabledLightsBitMask[dwIndex];
        return true;
    }

    return false;
}

bool RENDER::GetLight(std::uint32_t dwIndex, Light* pLight) const
{
    if (dwIndex < lights.size())
    {
        *pLight = lights[dwIndex];
        pLight->Position.x -= vWordRelationPos.x;
        pLight->Position.y -= vWordRelationPos.y;
        pLight->Position.z -= vWordRelationPos.z;
        return true;
    }
    
    return false;
}

//################################################################################
std::int32_t RENDER::CreateVertexBuffer(size_t size, std::uint32_t type, RHI::MemoryPropertiesBits memoryProperties)
{
    if (size <= 0)
        return -1; // fix

    std::int32_t b;
    for (b = 0; b < MAX_BUFFERS; b++)
        if (VertexBuffers[b].buff == nullptr)
            break;

    if (b == MAX_BUFFERS)
        return -1;

    RHI::BufferDesc vertexBufferDesc = {};
    vertexBufferDesc
        .setSize(size)
        .setIsVertexBuffer(true)
        .setIsTransferDst(true)
        .setMemoryProperties(memoryProperties);
    VertexBuffers[b].buff = device->createBuffer(vertexBufferDesc);
    VertexBuffers[b].type = type;

    return b;
}

RHI::BufferHandle RENDER::GetVertexBuffer(std::int32_t id)
{
    if (id < 0 || id >= MAX_BUFFERS)
        return nullptr;
    return VertexBuffers[id].buff;
}

//################################################################################
std::int32_t RENDER::CreateIndexBuffer(size_t size, std::uint32_t usage)
{
    std::int32_t b;
    for (b = 0; b < MAX_BUFFERS; b++)
        if (IndexBuffers[b].buff == nullptr)
            break;

    if (b == MAX_BUFFERS)
        return -1;

    RHI::BufferDesc indexBufferDesc = {};
    indexBufferDesc
        .setSize(size)
		.setFormat(RHI::Format::R16_UINT)
        .setIsIndexBuffer(true)
        .setIsTransferDst(true);
    IndexBuffers[b].buff = device->createBuffer(indexBufferDesc);
    IndexBuffers[b].size = size;
    IndexBuffers[b].dwUsage = usage;

    return b;
}

//################################################################################

void RENDER::CreateInputLayout(const VertexFVFBits vertexBindingsFormat, RHI::IInputLayout* inputLayout)
{
    std::uint32_t attributeOffset = 0;
    std::uint32_t location = 0;
    std::uint32_t stride = 0;
    std::vector<RHI::VertexInputAttributeDesc> attributes;
    if ((vertexBindingsFormat | VertexFVFBits::XYZ) != 0)
    {
        RHI::VertexInputAttributeDesc attributeDesc = RHI::VertexInputAttributeDesc{};
        attributeDesc.setFormat(RHI::Format::RGB32_FLOAT)
            .setOffset(attributeOffset)
            .setBinding(0)
            .setLocation(location);
        attributes.push_back(attributeDesc);
        attributeOffset += 3 * sizeof(float);
        location++;
        stride += 3 * sizeof(float);
    }
    if ((vertexBindingsFormat | VertexFVFBits::Color) != 0)
    {
        RHI::VertexInputAttributeDesc attributeDesc = RHI::VertexInputAttributeDesc{};
        attributeDesc.setFormat(RHI::Format::RGB32_FLOAT)
            .setOffset(attributeOffset)
            .setBinding(0)
            .setLocation(location);
        attributes.push_back(attributeDesc);
        attributeOffset += 3 * sizeof(float);
        location++;
        stride += 3 * sizeof(float);
    }
    if ((vertexBindingsFormat | VertexFVFBits::UV1) != 0)
    {
        RHI::VertexInputAttributeDesc attributeDesc = RHI::VertexInputAttributeDesc{};
        attributeDesc.setFormat(RHI::Format::RG32_FLOAT)
            .setOffset(attributeOffset)
            .setBinding(0)
            .setLocation(location);
        attributes.push_back(attributeDesc);
        attributeOffset += 2 * sizeof(float);
        location++;
        stride += 2 * sizeof(float);
    }

    RHI::VertexInputBindingDesc bindings[] =
    {
    RHI::VertexInputBindingDesc()
    .setBinding(0)
    .setStride(stride)
    };

    inputLayout = device->createInputLayout(attributes.data(), bindings);
}

void RENDER::MakeVertexBindings(RHI::BufferHandle vertexBuffer, const VertexFVFBits vertexBindingsFormat, std::vector<RHI::VertexBufferBinding>& vertexBufferBindings)
{
    std::uint32_t bindingSlotIndex = 0;
    std::uint32_t bindingSlotOffset = 0;
	if((vertexBindingsFormat | VertexFVFBits::XYZ) != 0)
	{
        vertexBufferBindings.push_back({ vertexBuffer.get(), bindingSlotIndex, bindingSlotOffset });
        bindingSlotIndex++;
        bindingSlotOffset += 3 * sizeof(float);
	}
    if ((vertexBindingsFormat | VertexFVFBits::Color) != 0)
    {
        vertexBufferBindings.push_back({ vertexBuffer.get(), bindingSlotIndex, bindingSlotOffset });
        bindingSlotIndex++;
        bindingSlotOffset += 3 * sizeof(float);
    }
    if ((vertexBindingsFormat | VertexFVFBits::UV1) != 0)
    {
        vertexBufferBindings.push_back({ vertexBuffer.get(), bindingSlotIndex, bindingSlotOffset });
        bindingSlotIndex++;
        bindingSlotOffset += 2 * sizeof(float);
    }
}

void RENDER::CreateGraphicsPipeline(RHI::ShaderHandle vertexShader, RHI::ShaderHandle pixelShader,
    RHI::IInputLayout* inputLayout, RHI::IBindingLayout* bindingLayout,
    RHI::FramebufferHandle framebuffer, RHI::GraphicsPipelineHandle pipeline)
{
    RHI::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.VS = vertexShader;
    pipelineDesc.PS = pixelShader;
    pipelineDesc.inputLayout = inputLayout;
    pipelineDesc.bindingLayouts = { bindingLayout };
    pipelineDesc.primType = RHI::PrimitiveType::TriangleList;
    pipelineDesc.renderState.depthStencilState.depthTestEnable = true;

    pipeline = device->createGraphicsPipeline(pipelineDesc, framebuffer.get());
}

RHI::GraphicsState RENDER::CreateGraphicsState(RHI::GraphicsPipelineHandle pipeline, RHI::FramebufferHandle framebuffer,
    RHI::IBindingSet* bindingSet, RHI::BufferHandle vertexBuffer, RHI::BufferHandle indexBuffer)
{
    RHI::GraphicsState state{};
    // Pick the right binding set for this view.
    state.bindingSets = { bindingSet };
    state.indexBufferBinding = { indexBuffer.get(), 0, 0 };
    // Bind the vertex buffers in reverse order to test the RHI implementation of binding slots
    MakeVertexBindings(vertexBuffer, vertexFormat, state.vertexBufferBindings);
    state.pipeline = pipeline.get();
    state.framebuffer = framebuffer.get();

    return state;
}

void RENDER::DrawBuffer(RHI::PrimitiveType primitiveType, std::uint32_t vertexBufferIndex, std::int32_t iStride, size_t vertexCount,
    size_t instanceCount, size_t startVertexLocation, const char* cBlockName)
{
    bool bDraw = true;

    if (VertexBuffers.size() < vertexBufferIndex)
		vertexFormat = static_cast<VertexFVFBits>(VertexBuffers[vertexBufferIndex].type);

    RHI::GraphicsState state = CreateGraphicsState(, , , VertexBuffers[vertexBufferIndex].buff);

    // Update the pipeline, bindings, and other state.
    commandList->setGraphicsState(state);

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);

    if (bDraw)
        do
        {
            dwNumDrawPrimitive++;

            // Draw the model.
            RHI::DrawArguments drawArgs = {};
            drawArgs.vertexCount = vertexCount;
            drawArgs.instanceCount = instanceCount;
            drawArgs.startVertexLocation = startVertexLocation;
            commandList->drawIndexed(drawArgs);
            //CHECKERR(d3d9->DrawIndexedPrimitive(RHI::PrimitiveType::TriangleList, minv, 0, numv, startidx, numtrg));
        } while (cBlockName && cBlockName[0] && TechniqueExecuteNext());
}

void RENDER::DrawIndexedBuffer(RHI::PrimitiveType primitiveType, std::uint32_t vertexBufferIndex, std::uint32_t indexBufferIndex, size_t vertexCount, size_t instanceCount,
    size_t startIndexLocation, size_t startVertexLocation, const char* cBlockName)
{
    bool bDraw = true;

    if (VertexBuffers.size() < vertexBufferIndex)
        vertexFormat = static_cast<VertexFVFBits>(VertexBuffers[vertexBufferIndex].type);
    // else VertexBuffer already set

    RHI::GraphicsState state = CreateGraphicsState(, , , VertexBuffers[vertexBufferIndex].buff, IndexBuffers[indexBufferIndex].buff);

    // Update the pipeline, bindings, and other state.
    commandList->setGraphicsState(state);

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);

    if (bDraw)
        do
        {
            dwNumDrawPrimitive++;

            // Draw the model.
            RHI::DrawArguments drawArgs = {};
            drawArgs.vertexCount = vertexCount;
            drawArgs.instanceCount = instanceCount;
            drawArgs.startIndexLocation = startIndexLocation;
            drawArgs.startVertexLocation = startVertexLocation;
            commandList->drawIndexed(drawArgs);
            //CHECKERR(d3d9->DrawIndexedPrimitive(RHI::PrimitiveType::TriangleList, minv, 0, numv, startidx, numtrg));
        } while (cBlockName && cBlockName[0] && TechniqueExecuteNext());
}

void RENDER::DrawIndexedBufferNoVShader(RHI::PrimitiveType primitiveType, std::uint32_t vertexBufferIndex, std::uint32_t indexBufferIndex, size_t vertexCount, size_t instanceCount,
    size_t startIndexLocation, size_t startVertexLocation, const char* cBlockName)
{
    bool bDraw = true;

    vertexFormat = VertexFVFBits::XYZ;

    RHI::GraphicsState state = CreateGraphicsState(, , , VertexBuffers[vertexBufferIndex].buff, IndexBuffers[indexBufferIndex].buff);

    // Update the pipeline, bindings, and other state.
    commandList->setGraphicsState(state);

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);

    if (bDraw)
        do
        {
            dwNumDrawPrimitive++;

            // Draw the model.
            RHI::DrawArguments drawArgs = {};
            drawArgs.vertexCount = vertexCount;
            drawArgs.instanceCount = instanceCount;
            drawArgs.startIndexLocation = startIndexLocation;
            drawArgs.startVertexLocation = startVertexLocation;
            commandList->drawIndexed(drawArgs);
        } while (cBlockName && cBlockName[0] && TechniqueExecuteNext());
}

// TODO: Use as dx09->DrawPrimitiveUP analog with correct vertex buffer or Model matrix
void RENDER::DrawPrimitive(RHI::PrimitiveType primitiveType, VertexFVFBits vertexBufferFormat, size_t vertexCount, size_t instanceCount,
    size_t startVertexLocation, RHI::BufferHandle vertexBuffer, std::uint32_t stride, const char* cBlockName)
{
    bool bDraw = true;

    vertexFormat = vertexBufferFormat;

    RHI::GraphicsState state = CreateGraphicsState(, , , vertexBuffer);

    // Update the pipeline, bindings, and other state.
    commandList->setGraphicsState(state);

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);

    if (bDraw)
        do
        {
            dwNumDrawPrimitive++;

            // Draw the model.
            RHI::DrawArguments drawArgs = {};
            drawArgs.vertexCount = vertexCount;
            drawArgs.instanceCount = instanceCount;
            drawArgs.startVertexLocation = startVertexLocation;
            commandList->draw(drawArgs);
        } while (cBlockName && cBlockName[0] && TechniqueExecuteNext());
}

// TODO: Use as dx09->DrawIndexedPrimitiveUP analog with correct vertex buffer or Model matrix
void RENDER::DrawIndexedPrimitive(RHI::PrimitiveType primitiveType, RHI::BufferHandle vertexBuffer, VertexFVFBits vertexDataFormat,
    RHI::BufferHandle indexBuffer, RHI::Format indexDataFormat, size_t vertexCount, size_t instanceCount,
    size_t startIndexLocation, size_t startVertexLocation, const char* cBlockName)
{
    bool bDraw = true;

    vertexFormat = vertexDataFormat;

    RHI::GraphicsState state = CreateGraphicsState(, , , vertexBuffer, indexBuffer);

    // Update the pipeline, bindings, and other state.
    commandList->setGraphicsState(state);

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);

    if (bDraw)
        do
        {
            dwNumDrawPrimitive++;

            // Draw the model.
            RHI::DrawArguments drawArgs = {};
            drawArgs.vertexCount = vertexCount;
            drawArgs.instanceCount = instanceCount;
            drawArgs.startIndexLocation = startIndexLocation;
            drawArgs.startVertexLocation = startVertexLocation;
            commandList->drawIndexed(drawArgs);
        } while (cBlockName && cBlockName[0] && TechniqueExecuteNext());
}

//################################################################################
void RENDER::RenderAnimation(std::int32_t ib, void* src, std::int32_t numVrts, std::int32_t minv, std::int32_t numv, std::int32_t startidx, std::int32_t numtrg,
    bool isUpdateVB)
{
    if (numVrts <= 0 || !src || ib < 0)
        return;
    vertexFormat = VertexFVFBits::XYZ | VertexFVFBits::Normal | VertexFVFBits::UV1;
    const std::int32_t size = numVrts * sizeof(FVF_VERTEX);
    if (isUpdateVB || numVrts > numAniVerteces || !aniVBuffer)
    {
        // Create vertex buffer
        if (numVrts > numAniVerteces)
        {
            if (aniVBuffer)
                aniVBuffer = nullptr;
            aniVBuffer = nullptr;
            numAniVerteces = 0;

            RHI::BufferDesc vertexBufferDesc = {};
            vertexBufferDesc
                .setSize(size)
                .setIsVertexBuffer(true)
                .setIsTransferDst(true)
                .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
            aniVBuffer = device->createBuffer(vertexBufferDesc);
            numAniVerteces = numVrts;
        }
        // Copy vertices
        commandList->writeBuffer(aniVBuffer.get(), size, src);
        dwNumLV++;
    }
    // Render
    DrawIndexedPrimitive(
        RHI::PrimitiveType::TriangleList,
        aniVBuffer,
        vertexFormat,
        IndexBuffers[ib].buff,
        RHI::Format::R16_UINT,
        numv, 1,
        startidx, minv);

    dwNumDrawPrimitive++;
}

//################################################################################
std::int32_t RENDER::GetVertexBufferSize(std::int32_t id)
{
    return VertexBuffers[id].size;
}


//################################################################################
void RENDER::ReleaseVertexBuffer(std::int32_t id)
{
    VertexBuffers[id].buff = nullptr;
}

//################################################################################
void RENDER::ReleaseIndexBuffer(std::int32_t id)
{
    IndexBuffers[id].buff = nullptr;
}

void RENDER::SetTransform(std::int32_t type, const CMatrix& mtx)
{
    CMatrix m = mtx;
    if (type == TSType::TS_VIEW)
    {
        vViewRelationPos.x = -mtx.m[3][0];
        vViewRelationPos.y = -mtx.m[3][1];
        vViewRelationPos.z = -mtx.m[3][2];

        CVECTOR vDeltaWorld = vWordRelationPos;
        m.MulToInvNorm(-vViewRelationPos, vWordRelationPos);
        vDeltaWorld -= vWordRelationPos;
        CMatrix mw = mWorld;
        mw.Pos() -= vDeltaWorld;
        mWorld = mw;

        m.Pos() += vViewRelationPos;
        mView = m;
    }
    else if (type == TSType::TS_WORLD)
    {
        m.Pos() += vWordRelationPos;
        mWorld = m;
    }
    else if(type == TSType::TS_PROJECTION)
    {
        mProjection = m;
    }
}

void RENDER::GetTransform(std::int32_t type, CMatrix* mtx)
{
    if (type == TSType::TS_VIEW)
    {
        *mtx = mView;
        mtx->m[3][0] -= vViewRelationPos.x;
        mtx->m[3][1] -= vViewRelationPos.y;
        mtx->m[3][2] -= vViewRelationPos.z;
    }
    else if (type == TSType::TS_WORLD)
    {
        mtx->m[3][0] -= vWordRelationPos.x;
        mtx->m[3][1] -= vWordRelationPos.y;
        mtx->m[3][2] -= vWordRelationPos.z;
    }
    else if(type == TSType::TS_PROJECTION)
    {
        *mtx = mProjection;
    }
}

void RENDER::LostRender()
{
    if (resourcesReleased)
    {
        return;
    }

    InvokeEntitiesLostRender();

    pOriginalScreenSurface = nullptr;
    pOriginalDepthTexture = nullptr;
    rectsVBuffer = nullptr;
    for (std::int32_t b = 0; b < MAX_BUFFERS; b++)
    {
        if (VertexBuffers[b].buff)
        {
            VertexBuffers[b].buff = nullptr;
        }
        if (IndexBuffers[b].buff)
        {
            IndexBuffers[b].buff = nullptr;
        }
    }

    resourcesReleased = true;
}

void RENDER::RestoreRender()
{
    pOriginalScreenSurface = m_DynamicRHI->GetBackBuffer(m_DynamicRHI->GetCurrentBackBufferIndex());
    pOriginalDepthTexture = m_DynamicRHI->GetDepthBuffer();
    fSmallWidth = 128;
    fSmallHeight = 128;
    if (bPostProcessEnabled)
    {
        RHI::TextureDesc desc = {};
        desc.setWidth(screen_size.x)
            .setHeight(screen_size.y)
            .setFormat(RHI::Format::RGBA8_UNORM)
            .setIsUAV(true)
            .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);
        pPostProcessTexture = device->createImage(desc);

        desc.setWidth(static_cast<int>(fSmallWidth))
            .setHeight(static_cast<int>(fSmallHeight));
        pSmallPostProcessTexture = device->createImage(desc);

        desc.setWidth(static_cast<int>(fSmallWidth * 2))
            .setHeight(static_cast<int>(fSmallHeight * 2));
        pSmallPostProcessTexture2 = device->createImage(desc);
    }
    if (!pPostProcessTexture || !pSmallPostProcessTexture || !pSmallPostProcessTexture2)
    {
        bPostProcessEnabled = false;
        bPostProcessError = true;
    }

    if (!bPostProcessError)
    {
        ClearPostProcessTexture(pPostProcessTexture);
        ClearPostProcessTexture(pSmallPostProcessTexture);
        ClearPostProcessTexture(pSmallPostProcessTexture2);
    }
    ClearPostProcessTexture(pOriginalScreenSurface);

    RHI::BufferDesc rectsVBufferDesc = {};
    rectsVBufferDesc
        .setSize(rectsVBuffer_SizeInRects * 6 * sizeof(RECT_VERTEX))
        .setIsVertexBuffer(true)
        .setIsTransferDst(true)
		.setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT);// RHI::MemoryPropertiesBits::HOST_VISIBLE_BIT | RHI::MemoryPropertiesBits::HOST_COHERENT_BIT
    rectsVBuffer = device->createBuffer(rectsVBufferDesc);

    for (std::int32_t b = 0; b < MAX_BUFFERS; b++)
    {
        if (VertexBuffers[b].buff)
        {
            CHECKERR(CreateVertexBuffer(VertexBuffers[b].size, VertexBuffers[b].type, RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT));
        }
        if (IndexBuffers[b].buff)
        {
            CHECKERR(CreateIndexBuffer(IndexBuffers[b].size, IndexBuffers[b].dwUsage));
        }
    }
    std::int32_t num_stages;
    num_stages = 8;
    for (std::int32_t s = 0; s < num_stages; s++)
    {
        // texture operation
        // TODO: remake texture operations in shader
        //SetTextureStageState(s, D3DTSS_COLORARG1, D3DTA_CURRENT);
        //SetTextureStageState(s, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        //SetTextureStageState(s, D3DTSS_COLOROP, D3DTOP_DISABLE);

        // texture coord
        // TODO: remake texture operations in shader
        //SetTextureStageState(s, D3DTSS_TEXCOORDINDEX, s);

        // texture filtering
        defaultSamplerDesc.magFilter = RHI::SamplerFilter::LINEAR;
        defaultSamplerDesc.minFilter = RHI::SamplerFilter::LINEAR;
        defaultSamplerDesc.mipFilter = RHI::SamplerFilter::LINEAR;
    }
    // set base texture and diffuse+specular lighting
    // TODO: remake texture operations in shader
    //SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    //SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
    Light light{};
    light.Type = LightType::LIGHT_POINT;
    light.Range = 100.0f;
    light.Attenuation0 = 1.0f;

    for (int i = 0; i < 8; i++)
    {
        lights[i] = light;
        enabledLightsBitMask[i] = false;
    }
    SetCommonStates();

#ifdef _WIN32 // Effects
    RecompileEffects();
#else
    pTechnique = std::make_unique<CTechnique>(this);
    pTechnique->DecodeFiles();
#endif

    InvokeEntitiesRestoreRender();

    resourcesReleased = false;
}

void RENDER::RecompileEffects()
{
#ifdef _WIN32 // Effects
    effects_.release();

    std::filesystem::path cur_path = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::u8path(fio->_GetExecutableDirectory()));
    for (const auto& p : std::filesystem::recursive_directory_iterator("resource/techniques"))
        if (is_regular_file(p) && p.path().extension() == ".fx")
        {
            auto s = p.path().string(); // hug microsoft
            effects_.compile(s.c_str());
        }
    std::filesystem::current_path(cur_path);
#endif
}

bool RENDER::ResetDevice()
{
    LostRender();

    //device->Reset();

    RestoreRender();

    return true;
}

void RENDER::SetGLOWParams(float _fBlurBrushSize, std::int32_t _GlowIntensity, std::int32_t _GlowPasses)
{
    fBlurSize = _fBlurBrushSize;
    GlowIntensity = _GlowIntensity;
    iBlurPasses = _GlowPasses;
}

void RENDER::RunStart()
{
    auto* pScriptRender = static_cast<VDATA*>(core.GetScriptVariable("Render"));
    ATTRIBUTES* pARender = pScriptRender->GetAClass();

    bSeaEffect = pARender->GetAttributeAsDword("SeaEffect", 0) != 0;
    fSeaEffectSize = pARender->GetAttributeAsFloat("SeaEffectSize", 0.0f);
    fSeaEffectSpeed = pARender->GetAttributeAsFloat("SeaEffectSpeed", 0.0f);
    dwBackColor = pARender->GetAttributeAsDword("BackColor", 0);

    if (bSeaEffect)
    {
        fSin += static_cast<float>(core.GetRDeltaTime()) * 0.001f * fSeaEffectSpeed;

        const auto sx = static_cast<float>(screen_size.x);
        const auto sy = static_cast<float>(screen_size.y);

        const auto fDX = static_cast<float>(static_cast<std::int32_t>(sx * fSeaEffectSize));
        const float fDY = static_cast<float>(static_cast<std::int32_t>(sy * fSeaEffectSize));

        const float sx2 = sx + fDX * 2.0f;
        const float sy2 = sy + fDY * 2.0f;

        for (std::int32_t y = 0; y < 32; y++)
        {
            for (std::int32_t x = 0; x < 32; x++)
            {
                SeaEffectQuadVertices[x + y * 32].vPos =
                    Vector4(sx * static_cast<float>(x) / 31.0f, sy * static_cast<float>(y) / 31.0f, 0.0f, 1.0f);

                SeaEffectQuadVertices[x + y * 32].u0 = (sx * static_cast<float>(x) / 31.0f +
                    sinf(fSin + static_cast<float>(x) / 31.0f * PI * 16.0f) * fDX + fDX) /
                    sx2;
                SeaEffectQuadVertices[x + y * 32].v0 = (sy * static_cast<float>(y) / 31.0f +
                    sinf(fSin + static_cast<float>(y) / 31.0f * PI * 16.0f) * fDY + fDY) /
                    sy2;
            }
        }
    }

    bNeedCopyToScreen = true;
    //------------------------------------------
    if ((bPostProcessEnabled || bSeaEffect) && (!bPostProcessError))
    {
        SetPostProcessTextureAsRenderTarget();
    }
    else
    {
        SetScreenAsRenderTarget();
    }

    Clear(ClearBits::Color | ClearBits::Depth | ((stencil_format == RHI::Format::D24S8) ? ClearBits::Stencil : 0));

    dwNumDrawPrimitive = 0;
    dwNumLV = 0;
    dwNumLI = 0;
    //BeginScene();

    auto* editor = core.GetEditor();

    if (editor != nullptr) {
        editor->StartFrame();
    }

    //------------------------------------------
    bInsideScene = true;

    // execute default technique for set default render/texture states
    // if (TechniqueExecuteStart("default")) do{} while (TechniqueExecuteNext());

    // boal del_cheat
    if (core.Controls->GetDebugAsyncKeyState(VK_SHIFT) < 0 && core.Controls->GetDebugAsyncKeyState(VK_F11) < 0)
    {
        InvokeEntitiesLostRender();
#ifdef _WIN32 // Effects
        RecompileEffects();
#else
        pTechnique = std::make_unique<CTechnique>(this);
        pTechnique->DecodeFiles();
#endif
        InvokeEntitiesRestoreRender();
    }

    renderState.polygonFillMode = (editor != nullptr && editor->IsDebugFlagEnabled(storm::editor::DebugFlag::RenderWireframe)) ? RHI::FillMode::LINE : RHI::FillMode::FILL;

    PlayToTexture();

    // make screenshot
    if (CONTROL_STATE cs; core.Controls->GetControlState(kKeyTakeScreenshot, cs)
        && cs.state == CST_ACTIVATED)
    {
        SaveShoot();
    }
}

void RENDER::RunEnd()
{
    // MakePostProcess();

    bInsideScene = false;
    EndScene();
    if (progressTexture >= 0)
    {
        const std::int32_t oldCnt = progressSafeCounter;
        ProgressView();
        progressSafeCounter = oldCnt;
        if (progressSafeCounter >= 50)
        {
            EndProgressView();
        }
        else
            progressSafeCounter++;
    }
}

char Buff_4k[4096];

std::int32_t RENDER::Print(std::int32_t x, std::int32_t y, const char* format, ...)
{
    // GUARD(DX9RENDER::Print)
    if (idFontCurrent < 0 || idFontCurrent >= FontList.size())
        return 0;
    if (FontList[idFontCurrent].font == nullptr || FontList[idFontCurrent].ref == 0)
        return 0;

    va_list args;
    va_start(args, format);
    vsnprintf(Buff_4k, sizeof(Buff_4k), format, args);
    va_end(args);

    return FontList[idFontCurrent].font->Print(static_cast<float>(x), static_cast<float>(y), Buff_4k).value_or(0);
    // UNGUARD
}

std::int32_t RENDER::Print(std::int32_t nFontNum, std::uint32_t color, std::int32_t x, std::int32_t y, const char* format, ...)
{
    // GUARD(DX9RENDER::Print)
    if (nFontNum < 0 || nFontNum >= FontList.size())
        return 0;
    if (FontList[nFontNum].font == nullptr || FontList[nFontNum].ref == 0)
        return 0;

    va_list args;
    va_start(args, format);
    vsnprintf(Buff_4k, sizeof(Buff_4k), format, args);
    va_end(args);

    const std::int32_t retVal = FontList[nFontNum].font->Print(static_cast<float>(x), static_cast<float>(y), Buff_4k, { .color = color }).value_or(0);
    return retVal;
    // UNGUARD
}

std::int32_t RENDER::StringWidth(const char* string, std::int32_t nFontNum, float fScale, std::int32_t scrWidth)
{
    if (string == nullptr)
    {
        return 0;
    }
    return StringWidth(std::string_view(string), nFontNum, fScale, scrWidth);
}

std::int32_t RENDER::StringWidth(const std::string_view& string, std::int32_t nFontNum, float fScale, std::int32_t scrWidth)
{
    if (nFontNum < 0 || nFontNum >= FontList.size())
        return 0;
    const auto& pFont = FontList[nFontNum].font;
    if (FontList[nFontNum].ref == 0 || pFont == nullptr)
        return 0;


    const std::int32_t xs = screen_size.x;
    if (scrWidth == 0)
        scrWidth = xs;
    if (xs != scrWidth)
        fScale *= static_cast<float>(xs) / scrWidth;
    const std::int32_t retVal = pFont->GetStringWidth(string, { .scale = fScale });
    return retVal;
}

std::int32_t RENDER::CharWidth(utf8::u8_char ch, std::int32_t nFontNum, float fScale, std::int32_t scrWidth)
{
    std::string str(ch.b, ch.l);
    return StringWidth(str.c_str(), nFontNum, fScale, scrWidth);
}

std::int32_t RENDER::CharHeight(std::int32_t fontID)
{
    if (fontID < 0 || fontID >= FontList.size())
        return 0;
    if (FontList[fontID].ref == 0 || FontList[fontID].font == nullptr)
        return 0;

    return FontList[fontID].font->GetHeight();
}

std::int32_t RENDER::ExtPrint(std::int32_t nFontNum, std::uint32_t foreColor, std::uint32_t backColor, int wAlign, bool bShadow, float fScale,
    std::int32_t scrWidth, std::int32_t scrHeight, std::int32_t x, std::int32_t y, const char* format, ...)
{
    // GUARD(DX9RENDER::ExtPrint)

    if (nFontNum < 0 || nFontNum >= FontList.size())
        return 0;
    const auto& pFont = FontList[nFontNum].font;
    if (FontList[nFontNum].ref == 0 || pFont == nullptr)
        return 0;

    va_list args;
    va_start(args, format);
    vsnprintf(Buff_4k, sizeof(Buff_4k), format, args);
    va_end(args);

    RHI::FramebufferHandle BackBuffer = m_DynamicRHI->GetFramebuffer(m_DynamicRHI->GetCurrentBackBufferIndex());
    const std::int32_t xs = static_cast<std::int32_t>(BackBuffer->framebufferWidth);
    const std::int32_t ys = static_cast<std::int32_t>(BackBuffer->framebufferHeight);
    if (scrWidth == 0)
        scrWidth = xs;
    if (scrHeight == 0)
        scrHeight = ys;
    if (xs != scrWidth)
    {
        const float fHorzScale = static_cast<float>(xs) / scrWidth;
        x = static_cast<std::int32_t>(x * fHorzScale);
        fScale *= fHorzScale;
    }
    if (ys != scrHeight)
        y = static_cast<std::int32_t>(y * (static_cast<float>(ys) / scrHeight));

    switch (wAlign)
    {
    case Align::CENTER:
        x -= static_cast<std::int32_t>(pFont->GetStringWidth(Buff_4k, { .scale = fScale }) / 2);
        break;
    case Align::RIGHT:
        x -= static_cast<std::int32_t>(pFont->GetStringWidth(Buff_4k, { .scale = fScale }));
        break;
    }

    const std::int32_t retVal = pFont->Print(static_cast<float>(x), static_cast<float>(y), Buff_4k,
        {
            .scale = fScale,
            .color = foreColor,
            .shadow = bShadow,
        }).value_or(0);
    return retVal;
    // UNGUARD
}

std::int32_t RENDER::LoadFont(const std::string_view& fontName)
{
    const auto sDup = std::string(fontName);

    const std::uint32_t hashVal = MakeHashValue(fontName);

    std::int32_t i;

    auto existing_font = std::find_if(std::begin(FontList), std::end(FontList), [&](FONTEntity& font) {
        return font.hash == hashVal && storm::iEquals(font.name, fontName);
        });
    if (existing_font != std::end(FontList))
    {
        if (existing_font->ref > 0)
            existing_font->ref++;
        else
        {
            existing_font->ref = 1;
            existing_font->font->RepeatInit();
        }
        return std::distance(std::begin(FontList), existing_font);
    }
    else {
        if (FontList.size() >= MAX_FONTS) {
            throw std::runtime_error("maximal font quantity exceeded");
        }

        auto font = storm::LoadFont(fontName, fontIniFileName, *this, *d3d9);
        if (font == nullptr) {
            core.Trace("Can't load font %s", sDup.c_str());
            return -1L;
        }
        FontList.emplace_back(FONTEntity{
            sDup,
            hashVal,
            std::move(font),
            1,
            });
        return FontList.size() - 1;
    }
}

bool RENDER::UnloadFont(const char* fontName)
{
    if (fontName == nullptr)
        return false;
    char sDup[256];
    if (strlen(fontName) < sizeof(sDup) - 1)
        strcpy_s(sDup, fontName);
    else
    {
        strncpy_s(sDup, fontName, sizeof(sDup) - 1);
        sDup[sizeof(sDup) - 1] = 0;
    }
    std::ranges::for_each(sDup, [](char& c) { c = std::toupper(c); });
    fontName = sDup;
    const std::uint32_t hashVal = MakeHashValue(fontName);

    for (int i = 0; i < FontList.size(); i++)
        if (FontList[i].hash == hashVal && storm::iEquals(FontList[i].name, fontName))
            return UnloadFont(i);
    core.Trace("Font name \"%s\" is not containing", fontName);
    return false;
}

bool RENDER::UnloadFont(std::int32_t fontID)
{
    if (fontID < 0 || fontID >= FontList.size())
        return false;

    if (FontList[fontID].ref > 0)
    {
        FontList[fontID].ref--;
        if (FontList[fontID].ref == 0)
        {
            FontList[fontID].font->TempUnload();
            // if (idFontCurrent == fontID)
            idFontCurrent = 0; // FONT_DEFAULT;
        }
        else
            return false;
    }

    return true;
}

bool RENDER::IncRefCounter(std::int32_t fontID)
{
    if (fontID < 0 || fontID >= FontList.size())
        return false;

    FontList[fontID].ref++;

    return true;
}

bool RENDER::SetCurFont(const char* fontName)
{
    if (fontName == nullptr)
        return false;
    char sDup[256];
    if (strlen(fontName) < sizeof(sDup) - 1)
        strcpy_s(sDup, fontName);
    else
    {
        strncpy_s(sDup, fontName, sizeof(sDup) - 1);
        sDup[sizeof(sDup) - 1] = 0;
    }
    std::ranges::for_each(sDup, [](char& c) { c = std::toupper(c); });
    fontName = sDup;
    const std::uint32_t hashVal = MakeHashValue(fontName);

    for (int i = 0; i < FontList.size(); i++)
        if (FontList[i].hash == hashVal)
        {
            idFontCurrent = i;
            return true;
        }
    core.Trace("Font name \"%s\" is not containing", fontName);
    return false;
}

bool RENDER::SetCurFont(std::int32_t fontID)
{
    if (fontID < 0 || fontID >= FontList.size())
        return false;
    idFontCurrent = fontID;
    return true;
}

std::int32_t RENDER::GetCurFont()
{
    if (idFontCurrent >= 0 && idFontCurrent < FontList.size())
        return idFontCurrent;
    return -1L;
}

char* RENDER::GetFontIniFileName()
{
    return fontIniFileName.data();
}

bool RENDER::SetFontIniFileName(const char* iniName)
{
    if (!fontIniFileName.empty() && iniName != nullptr && storm::iEquals(fontIniFileName, iniName))
        return true;

    if (iniName == nullptr)
    {
        return false;
    }
    fontIniFileName = std::string(iniName);

    for (int n = 0; n < FontList.size(); n++)
    {
        const std::string font_name = FontList[n].name;
        FontList[n].font = storm::LoadFont(font_name, fontIniFileName, *this, *d3d9);
        if (FontList[n].font == nullptr)
        {
            core.Trace("Can't reload font %s", font_name.c_str());
            return false;
        }
        if (FontList[n].ref == 0)
            FontList[n].font->TempUnload();
    }

    return true;
}

void RENDER::SetCommonStates()
{
    // dither enable
    //----------Z---------
    // depth enable
    // depth write enable
    // depth func lessequal
    renderState.depthStencilState.depthTestEnable = true;
    renderState.depthStencilState.depthWriteEnable = true;
    renderState.depthStencilState.depthCompareOp = RHI::CompareOp::LESS_OR_EQUAL;

    // enable fog exponent
    //SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);

    // *****************************************************************88
    // texture filtering
    defaultSamplerDesc.magFilter = RHI::SamplerFilter::LINEAR;
    defaultSamplerDesc.minFilter = RHI::SamplerFilter::LINEAR;
    defaultSamplerDesc.mipFilter = RHI::SamplerFilter::LINEAR;
    defaultSamplerDesc.anisotropyEnable = true;
    defaultSamplerDesc.maxAnisotropy = 3.0f;

    // unchanged texture stage states - both for base and detal texture
    // TODO: remake texture operations in shader
    //SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    //SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    //SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    //SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    //SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
    //SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    //SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    //SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    //SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

    // general
    renderState.CCWCullMode = true;
    renderState.blendEnable = true;
    renderState.srcColorBlendFactor = RHI::BlendState::SRC_ALPHA;
    renderState.dstColorBlendFactor = RHI::BlendState::ONE_MINUS_SRC_ALPHA;

    defaultAlphaTest.alphaTestEnable = true;
    defaultAlphaTest.alphaRefValue = 0xa0;
    defaultAlphaTest.alphaCompareOp = RHI::CompareOp::GREATER;

    // lighting effects
    defaultShadingState.ambientColor = Color{ 64.0f / 255.0f, 64.0f / 255.0f, 64.0f / 255.0f, 1.0f };
    defaultShadingState.ambientMaterialSource = 1;
    defaultShadingState.enableLighting = false;
    defaultShadingState.vertexColoring = true;
    defaultShadingState.specularHighlights = false;
    defaultShadingState.localViewerSpecularHighlights = false;

    // AA
    renderState.multisampleAA = true;
}

RHI::Viewport& RENDER::GetViewport()
{
    return viewport;
}

void RENDER::SetViewport(const RHI::Viewport& pViewport)
{
    viewport = pViewport;
}

// TODO: make RHI analog
//std::uint32_t RENDER::GetSamplerState(std::uint32_t Sampler, D3DSAMPLERSTATETYPE Type, std::uint32_t* pValue)
//{
//    return CHECKERR(d3d9->GetSamplerState(Sampler, Type, (DWORD*)pValue));
//}
//
//std::uint32_t RENDER::SetSamplerState(std::uint32_t Sampler, D3DSAMPLERSTATETYPE Type, std::uint32_t Value)
//{
//    return CHECKERR(d3d9->SetSamplerState(Sampler, Type, Value));
//}
//
//std::uint32_t RENDER::SetTextureStageState(std::uint32_t Stage, std::uint32_t Type, std::uint32_t Value)
//{
//    return CHECKERR(d3d9->SetTextureStageState(Stage, static_cast<D3DTEXTURESTAGESTATETYPE>(Type), Value));
//}
//
//std::uint32_t RENDER::GetTextureStageState(std::uint32_t Stage, std::uint32_t Type, std::uint32_t* pValue)
//{
//    return CHECKERR(d3d9->GetTextureStageState(Stage, static_cast<D3DTEXTURESTAGESTATETYPE>(Type), (DWORD*)pValue));
//}

void RENDER::GetCamera(CVECTOR& pos, CVECTOR& ang, float& perspective)
{
    pos = Pos;
    ang = Ang;
    perspective = Fov;
}

using TGA_H = struct tagTGA_H
{
    uint8_t byte1; // = 0
    uint8_t byte2; // = 0
    uint8_t type;
    uint8_t byte4_9[9]; // = 0
    uint16_t width;
    uint16_t height;
    uint8_t bpp; // bit per pixel
    uint8_t attr : 4;
    uint8_t rez : 1;
    uint8_t origin : 1;
    uint8_t storage : 2;
};

// WORD Temp[1600*4];

void RENDER::SaveShoot()
{
    bMakeShoot = true;
}

void RENDER::MakeScreenShot()
{
    bMakeShoot = false;

    RHI::TextureHandle BackBuffer{ m_DynamicRHI->GetBackBuffer(m_DynamicRHI->GetCurrentBackBufferIndex()) };
    if (BackBuffer.get() == nullptr)
    {
        core.Trace("Failed to make screenshot");
        return;
    }

    /** Offscreen rendering helpers */
    RHI::TextureDesc destDesc = {};
    destDesc.setWidth(screen_size.x)
        .setHeight(screen_size.y)
        .setFormat(RHI::Format::RGBA8_SNORM)
        .setIsTransferSrc(true)
        .setIsTransferDst(true)
        .setIsShaderResource(true)
        .setIsRenderTarget(true);
    destDesc.memoryProperties = RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT;
    RHI::TextureHandle destTexture{ device->createImage(destDesc) };
    if (destTexture.get() == nullptr)
    {
        core.Trace("Failed to make screenshot");
        return;
    }

    commandList->copyTexture(BackBuffer.get(), RHI::TextureSubresourse{}, destTexture.get(), RHI::TextureSubresourse{});

    const auto screenshot_base_filename = fmt::format("{:%Y-%m-%d_%H-%M-%S}", fmt::localtime(std::time(nullptr)));
    auto screenshot_path = Storm::Filesystem::Constants::Paths::screenshots() / screenshot_base_filename;
    screenshot_path.replace_extension(screenshotExt);
    for (size_t i = 0; std::filesystem::exists(screenshot_path); ++i)
    {
        screenshot_path.replace_filename(screenshot_base_filename + "_" + std::to_string(i));
        screenshot_path.replace_extension(screenshotExt);
    }
#ifdef _WIN32 // Screenshot
    D3DXSaveSurfaceToFile(screenshot_path.c_str(), screenshotFormat, surface, nullptr, nullptr);
#endif
}

Plane* RENDER::GetPlanes()
{
    FindPlanes();
    return viewplane;
}

void RENDER::FindPlanes()
{
    CMatrix m = mProjection;
    CVECTOR v[4];
    // left
    v[0].x = m.m[0][0];
    v[0].y = 0.0f;
    v[0].z = 1.0f;
    // right
    v[1].x = -m.m[0][0];
    v[1].y = 0.0f;
    v[1].z = 1.0f;
    // top
    v[2].x = 0.0f;
    v[2].y = -m.m[1][1];
    v[2].z = 1.0f;
    // bottom
    v[3].x = 0.0f;
    v[3].y = m.m[1][1];
    v[3].z = 1.0f;
    v[0] = !v[0];
    v[1] = !v[1];
    v[2] = !v[2];
    v[3] = !v[3];

    m = mView;
    CVECTOR pos;

    pos.x = -m.m[3][0] * m.m[0][0] - m.m[3][1] * m.m[0][1] - m.m[3][2] * m.m[0][2];
    pos.y = -m.m[3][0] * m.m[1][0] - m.m[3][1] * m.m[1][1] - m.m[3][2] * m.m[1][2];
    pos.z = -m.m[3][0] * m.m[2][0] - m.m[3][1] * m.m[2][1] - m.m[3][2] * m.m[2][2];

    viewplane[0].Nx = v[0].x * m.m[0][0] + v[0].y * m.m[0][1] + v[0].z * m.m[0][2];
    viewplane[0].Ny = v[0].x * m.m[1][0] + v[0].y * m.m[1][1] + v[0].z * m.m[1][2];
    viewplane[0].Nz = v[0].x * m.m[2][0] + v[0].y * m.m[2][1] + v[0].z * m.m[2][2];

    viewplane[1].Nx = v[1].x * m.m[0][0] + v[1].y * m.m[0][1] + v[1].z * m.m[0][2];
    viewplane[1].Ny = v[1].x * m.m[1][0] + v[1].y * m.m[1][1] + v[1].z * m.m[1][2];
    viewplane[1].Nz = v[1].x * m.m[2][0] + v[1].y * m.m[2][1] + v[1].z * m.m[2][2];

    viewplane[2].Nx = v[2].x * m.m[0][0] + v[2].y * m.m[0][1] + v[2].z * m.m[0][2];
    viewplane[2].Ny = v[2].x * m.m[1][0] + v[2].y * m.m[1][1] + v[2].z * m.m[1][2];
    viewplane[2].Nz = v[2].x * m.m[2][0] + v[2].y * m.m[2][1] + v[2].z * m.m[2][2];

    viewplane[3].Nx = v[3].x * m.m[0][0] + v[3].y * m.m[0][1] + v[3].z * m.m[0][2];
    viewplane[3].Ny = v[3].x * m.m[1][0] + v[3].y * m.m[1][1] + v[3].z * m.m[1][2];
    viewplane[3].Nz = v[3].x * m.m[2][0] + v[3].y * m.m[2][1] + v[3].z * m.m[2][2];

    viewplane[0].D = (pos.x * viewplane[0].Nx + pos.y * viewplane[0].Ny + pos.z * viewplane[0].Nz);
    viewplane[1].D = (pos.x * viewplane[1].Nx + pos.y * viewplane[1].Ny + pos.z * viewplane[1].Nz);
    viewplane[2].D = (pos.x * viewplane[2].Nx + pos.y * viewplane[2].Ny + pos.z * viewplane[2].Nz);
    viewplane[3].D = (pos.x * viewplane[3].Nx + pos.y * viewplane[3].Ny + pos.z * viewplane[3].Nz);
}

#ifdef _WIN32 // Effects
bool RENDER::TechniqueExecuteStart(const char* cBlockName)
{
    if (!cBlockName)
        return false;
    return effects_.begin(cBlockName);
}
#else
bool DX9RENDER::TechniqueExecuteStart(const char* cBlockName)
{
    if (!cBlockName)
        return false;
    pTechnique->SetCurrentBlock(cBlockName, 0, nullptr);
    return pTechnique->ExecutePassStart();
}
#endif

bool RENDER::TechniqueExecuteNext()
{
#ifdef _WIN32 // Effects
    return effects_.next();
#else
    return pTechnique->ExecutePassNext();
#endif
}

void RENDER::DrawRects(RS_RECT* pRSR, std::uint32_t dwRectsNum, const char* cBlockName, std::uint32_t dwSubTexturesX,
    std::uint32_t dwSubTexturesY, float fScaleX, float fScaleY)
{
    if (!pRSR || dwRectsNum == 0 || !rectsVBuffer)
        return;

    bool bDraw = true;

    static CMatrix camMtx, oldWorldMatrix, IMatrix;
    camMtx = mView;
    oldWorldMatrix = mWorld;

    fScaleY *= GetHeightDeformator();

    float du, dv;
    bool bUseSubTextures = false;
    if (dwSubTexturesX > 1)
    {
        bUseSubTextures = true;
        du = 1.0f / dwSubTexturesX;
    }
    else
    {
        dwSubTexturesX = 1;
        du = 1.0f;
    }
    if (dwSubTexturesY > 1)
    {
        bUseSubTextures = true;
        dv = 1.0f / dwSubTexturesY;
    }
    else
    {
        dwSubTexturesY = 1;
        dv = 1.0f;
    }

    mView = IMatrix;
    mWorld = IMatrix;

    for (std::uint32_t cnt = 0; cnt < dwRectsNum;)
    {
        // Number of rectangles to draw at a time
        std::uint32_t drawCount = dwRectsNum - cnt;
        if (drawCount > rectsVBuffer_SizeInRects)
            drawCount = rectsVBuffer_SizeInRects;
        // Buffer
        RECT_VERTEX* rectData = new RECT_VERTEX[drawCount * 6];
        if (!rectData)
            return;
        // Filling the buffer
        for (std::uint32_t i = 0; i < drawCount && cnt < dwRectsNum; i++)
        {
            // Local array of a particle
            RECT_VERTEX* buffer = &rectData[i * 6];
            RS_RECT& rect = pRSR[cnt++];
            CVECTOR pos = camMtx * (rect.vPos + vWordRelationPos);
            const float sizex = rect.fSize * fScaleX;
            const float sizey = rect.fSize * fScaleY;
            const float sn = sinf(rect.fAngle);
            const float cs = cosf(rect.fAngle);
            const std::int32_t color = rect.dwColor;
            float u1, v1, u2, v2;
            if (!bUseSubTextures)
            {
                v1 = u1 = 0.0f;
                v2 = u2 = 1.0f;
            }
            else
            {
                u1 = (rect.dwSubTexture % dwSubTexturesX) * du;
                v1 = ((rect.dwSubTexture / dwSubTexturesX) % dwSubTexturesY) * dv;
                u2 = u1 + du;
                v2 = v1 + dv;
            }
            // Filling the particle buffer
            buffer[0].pos = pos + CVECTOR(sizex * (-cs - sn), sizey * (sn - cs), 0.0f);
            buffer[0].color = color;
            buffer[0].u = u1;
            buffer[0].v = v2;
            buffer[1].pos = pos + CVECTOR(sizex * (-cs + sn), sizey * (sn + cs), 0.0f);
            buffer[1].color = color;
            buffer[1].u = u1;
            buffer[1].v = v1;
            buffer[2].pos = pos + CVECTOR(sizex * (cs + sn), sizey * (-sn + cs), 0.0f);
            buffer[2].color = color;
            buffer[2].u = u2;
            buffer[2].v = v1;
            buffer[3].pos = buffer[0].pos;
            buffer[3].color = color;
            buffer[3].u = u1;
            buffer[3].v = v2;
            buffer[4].pos = buffer[2].pos;
            buffer[4].color = color;
            buffer[4].u = u2;
            buffer[4].v = v1;
            buffer[5].pos = pos + CVECTOR(sizex * (cs - sn), sizey * (-sn - cs), 0.0f);
            buffer[5].color = color;
            buffer[5].u = u2;
            buffer[5].v = v2;
        }
        // Draw a buffer
        // Setup format and copy staging buffer
        VertexFVFBits vertexFVF = VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1;

        commandList->writeBuffer(rectsVBuffer.get(), sizeof(rectData), rectData);

        delete[] rectData;

        if (cBlockName && cBlockName[0])
            bDraw = TechniqueExecuteStart(cBlockName);
        if (bDraw)
            do
            {
                // Setup vertex buffer + draw vertex buffer as triangle list
                // TODO: fix value for stride param
                /*CHECKERR*/(DrawPrimitive(RHI::PrimitiveType::TriangleList, vertexFVF, 6, 1, 0, rectsVBuffer, 24));
            } while (cBlockName && TechniqueExecuteNext());
    }

    mView = camMtx;
    mWorld = oldWorldMatrix;
}

void RENDER::DrawSprites(RS_SPRITE* pRSS, std::uint32_t dwSpritesNum, const char* cBlockName)
{
    std::uint32_t i;
    if (dwSpritesNum == 0)
        return;

    auto* pIndices = new uint16_t[dwSpritesNum * 6];
    const std::uint32_t indicesArraySize = dwSpritesNum * 6 * sizeof(uint16_t);

    for (i = 0; i < dwSpritesNum; i++)
    {
        pIndices[i * 6 + 0] = static_cast<uint16_t>(i * 4 + 0);
        pIndices[i * 6 + 1] = static_cast<uint16_t>(i * 4 + 3);
        pIndices[i * 6 + 2] = static_cast<uint16_t>(i * 4 + 2);
        pIndices[i * 6 + 3] = static_cast<uint16_t>(i * 4 + 0);
        pIndices[i * 6 + 4] = static_cast<uint16_t>(i * 4 + 2);
        pIndices[i * 6 + 5] = static_cast<uint16_t>(i * 4 + 1);
    }

    RHI::BufferDesc indexBufferDesc = {};
    indexBufferDesc.setSize(indicesArraySize)
        .setFormat(RHI::Format::R16_UINT)
        .setIsTransferDst(true)
        .setIsIndexBuffer(true);
    RHI::BufferHandle indexBuffer = device->createBuffer(indexBufferDesc);

    commandList->writeBuffer(indexBuffer.get(), indicesArraySize, pIndices);

    RHI::BufferDesc vertexBufferDesc = {};
    vertexBufferDesc.setSize(dwSpritesNum * sizeof(RS_SPRITE) * 4)
        .setIsTransferDst(true)
        .setIsVertexBuffer(true);

    RHI::BufferHandle vertexBuffer = device->createBuffer(vertexBufferDesc);

    commandList->writeBuffer(vertexBuffer.get(), vertexBufferDesc.size, pRSS);

    vertexFormat = VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1;

    bool bDraw = true;
    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);
    if (bDraw)
        do
        {
            DrawIndexedPrimitive(RHI::PrimitiveType::TriangleList, vertexBuffer, vertexFormat, indexBuffer, RHI::Format::R16_UINT, dwSpritesNum * 4, 1, 0, 0);
        } while (cBlockName && TechniqueExecuteNext());
    delete[] pIndices;
}

void RENDER::DrawLines(RS_LINE* pRSL, std::uint32_t dwLinesNum, const char* cBlockName)
{
    if (!pRSL || dwLinesNum == 0)
        return;

    VertexFVFBits vertexFVF = VertexFVFBits::XYZ | VertexFVFBits::Color;

    RHI::BufferDesc vertexBufferDesc = {};
    vertexBufferDesc.setSize(dwLinesNum * sizeof(RS_LINE) * 2)
        .setIsTransferDst(true)
        .setIsVertexBuffer(true);

    RHI::BufferHandle vertexBuffer = device->createBuffer(vertexBufferDesc);

    commandList->writeBuffer(vertexBuffer.get(), vertexBufferDesc.size, pRSL);

    bool bDraw = true;

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);
    if (bDraw)
        do
        {
            DrawPrimitive(RHI::PrimitiveType::LineList, vertexFVF, dwLinesNum * 2, 1, 0, vertexBuffer, sizeof(RS_LINE));
        } while (cBlockName && TechniqueExecuteNext());
}

void RENDER::DrawLines2D(RS_LINE2D* pRSL2D, size_t dwLinesNum, const char* cBlockName)
{
    if (!pRSL2D || dwLinesNum == 0)
        return;

    VertexFVFBits vertexFVF = VertexFVFBits::XYZ | VertexFVFBits::Color;

    RHI::BufferDesc vertexBufferDesc = {};
    vertexBufferDesc.setSize(dwLinesNum * sizeof(RS_LINE2D) * 2)
        .setIsTransferDst(true)
        .setIsVertexBuffer(true);

    RHI::BufferHandle vertexBuffer = device->createBuffer(vertexBufferDesc);

    bool bDraw = true;

    if (cBlockName && cBlockName[0])
        bDraw = TechniqueExecuteStart(cBlockName);
    if (bDraw)
        do
        {
            DrawPrimitive(RHI::PrimitiveType::LineList, vertexFVF, dwLinesNum * 2, 1, 0, vertexBuffer, sizeof(RS_LINE2D));
        } while (cBlockName && TechniqueExecuteNext());
}

//-----------------------
RHI::BufferHandle RENDER::CreateVertexBufferAndUpload(size_t vertexBufferSize, const void* pVertexData)
{
    RHI::BufferDesc vertexBufferDesc = {};
    vertexBufferDesc
        .setSize(vertexBufferSize)
        .setIsVertexBuffer(true)
        .setIsTransferDst(true);
    RHI::BufferHandle vertexBuffer{ device->createBuffer(vertexBufferDesc) };

    commandList->writeBuffer(vertexBuffer.get(), vertexBufferSize, pVertexData);

    return vertexBuffer;
}

std::int32_t RENDER::Release(IUnknown* pObject)
{
    if (pObject)
    {
        const auto result = pObject->Release();
        pObject = nullptr;
        return result;
    }

    return 0;
}

std::int32_t RENDER::GetRenderTarget(RHI::TextureHandle pRenderTarget)
{
    pRenderTarget = currentRenderTarget.pRenderTarget;
    return 0;
}

std::int32_t RENDER::GetDepthStencilSurface(RHI::TextureHandle pZStencilSurface)
{
    pZStencilSurface = currentRenderTarget.pDepthSurface;
    return 0;
}

std::int32_t RENDER::GetCubeMapSurface(RHI::TextureHandle pCubeTexture, CubemapFaces FaceType, std::uint32_t Level,
    RHI::TextureHandle pCubeMapSurface)
{
    return ppCubeTexture->GetCubeMapSurface(FaceType, Level, ppCubeMapSurface);
}

std::int32_t RENDER::SetRenderTarget(RHI::TextureHandle pRenderTarget, RHI::TextureHandle pNewZStencil)
{
    currentRenderTarget.pRenderTarget = pRenderTarget;
    currentRenderTarget.pDepthSurface = pNewZStencil;
    return 0;
}

std::int32_t RENDER::Clear(std::uint32_t Count, const D3DRECT* pRects, std::uint32_t Flags, Color Color, float Z,
    std::uint32_t Stencil)
{
    return CHECKERR(d3d9->Clear(Count, pRects, Flags, Color, Z, Stencil));
}

std::int32_t RENDER::CreateTexture(std::uint32_t Width, std::uint32_t Height, std::uint32_t Levels, std::uint32_t Usage, RHI::Format Format, RHI::MemoryPropertiesBits Pool,
    RHI::TextureHandle pTexture)
{
    RHI::TextureDesc desc = {};
    desc.setWidth(Width)
        .setHeight(Height)
        .setMipLevels(Levels)
        .setFormat(Format)
        .setMemoryProperties(Pool);
    pTexture = device->createImage(desc);

    return 0;
}

std::int32_t RENDER::CreateCubeTexture(std::uint32_t EdgeLength, std::uint32_t Levels, std::uint32_t Usage, RHI::Format Format, RHI::MemoryPropertiesBits Pool,
    RHI::TextureHandle pCubeTexture)
{
    RHI::TextureDesc desc = {};
    desc.setWidth(EdgeLength)
        .setHeight(EdgeLength)
        .setMipLevels(Levels)
        .setFormat(Format)
        .setMemoryProperties(Pool)
        .setDimension(RHI::TextureDimension::TextureCube);
    pCubeTexture = device->createImage(desc);

    return 0;
}

std::int32_t RENDER::CreateOffscreenPlainSurface(std::uint32_t Width, std::uint32_t Height, RHI::Format Format, RHI::TextureHandle pSurface)
{
    RHI::TextureDesc desc = {};
    desc.setWidth(Width)
        .setHeight(Height)
        .setFormat(Format)
        .setMemoryProperties(RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT) //RHI::MemoryPropertiesBits::HOST_VISIBLE_BIT | RHI::MemoryPropertiesBits::HOST_COHERENT_BIT
        .setIsTransferSrc(true)
        .setIsTransferDst(true)
        .setIsShaderResource(true)
        .setIsRenderTarget(true);
    pSurface = device->createImage(desc);

    return 0;
}

std::uint32_t RENDER::CreateDepthStencilSurface(std::uint32_t Width, std::uint32_t Height, RHI::Format Format, std::uint32_t msaaSamples,
    RHI::TextureHandle pSurface)
{
    RHI::TextureDesc desc{};
    desc.setWidth(Width)
        .setHeight(Height)
        .setFormat(Format)
        .setIsRenderTarget(true);
    pSurface = device->createImage(desc);

    return 0;
}


#ifdef _WIN32 // Effects
ID3DXEffect* RENDER::GetEffectPointer(const char* techniqueName)
{
    return effects_.getEffectPointer(techniqueName);
}
#endif

std::int32_t RENDER::SetTexture(RHI::TextureHandle pTexture, RHI::SamplerHandle sampler, std::uint32_t textureBindingIndex, RHI::DescriptorSetInfo& dsInfos)
{
    RHI::TextureAttachment textureAttachment = {};
    textureAttachment
        .setDescriptorInfo(RHI::DescriptorInfo{ RHI::DescriptorType::COMBINED_IMAGE_SAMPLER, RHI::ShaderStageFlagBits::FRAGMENT_BIT })
        .setTexture(pTexture.get())
        .setSampler(sampler.get());

    if (dsInfos.textures.size() <= textureBindingIndex)
    {
        dsInfos.textures[textureBindingIndex] = textureAttachment;
    }
    else
    {
        dsInfos.textures.push_back(textureAttachment);
    }
}

std::int32_t RENDER::GetLevelDesc(RHI::TextureHandle pTexture, std::uint32_t Level, D3DSURFACE_DESC* pDesc)
{
    return CHECKERR(pTexture->GetLevelDesc(Level, pDesc));
}

std::int32_t RENDER::GetLevelDesc(RHI::TextureHandle pCubeTexture, std::uint32_t Level, D3DSURFACE_DESC* pDesc)
{
    return CHECKERR(pCubeTexture->GetLevelDesc(Level, pDesc));
}

// TODO: use RHI updateTextureImage() instead rect lock/unlock or create analog for it 
//HRESULT RENDER::LockRect(IDirect3DCubeTexture9* ppCubeTexture, D3DCUBEMAP_FACES FaceType, UINT Level,
//    D3DLOCKED_RECT* pLockedRect, const RECT* pRect, std::uint32_t Flags)
//{
//    return CHECKERR(ppCubeTexture->LockRect(FaceType, Level, pLockedRect, pRect, Flags));
//}
//
//HRESULT RENDER::LockRect(IDirect3DTexture9* ppTexture, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect,
//    std::uint32_t Flags)
//{
//    return CHECKERR(ppTexture->LockRect(Level, pLockedRect, pRect, Flags));
//}
//
//HRESULT RENDER::UnlockRect(IDirect3DCubeTexture9* pCubeTexture, D3DCUBEMAP_FACES FaceType, UINT Level)
//{
//    return CHECKERR(pCubeTexture->UnlockRect(FaceType, Level));
//}
//
//HRESULT RENDER::UnlockRect(IDirect3DTexture9* pTexture, UINT Level)
//{
//    return CHECKERR(pTexture->UnlockRect(Level));
//}

std::int32_t RENDER::GetSurfaceLevel(RHI::TextureHandle pTexture, std::uint32_t Level, IDirect3DSurface9** ppSurfaceLevel)
{
    return CHECKERR(pTexture->GetSurfaceLevel(Level, ppSurfaceLevel));
}

std::int32_t RENDER::UpdateSurface(RHI::TextureHandle pSourceSurface, RHI::TextureHandle pDestinationSurface)
{
    commandList->copyTexture(pSourceSurface.get(), RHI::TextureSubresourse{}, pDestinationSurface.get(), RHI::TextureSubresourse{});
    return 0;
}

std::int32_t RENDER::StretchRect(RHI::TextureHandle pSourceSurface, RHI::TextureHandle pDestinationSurface)
{
    commandList->resolveTexture(pSourceSurface.get(), RHI::TextureSubresourse{}, pDestinationSurface.get(), RHI::TextureSubresourse{});
    return 0;
}

std::int32_t RENDER::GetRenderTargetData(RHI::TextureHandle pRenderTarget, RHI::TextureHandle pDestSurface)
{
    const RHI::TextureDesc& RTDesc = pRenderTarget->getDesc();

    commandList->transitionImageLayout(pRenderTarget.get(), RHI::ImageLayout::COLOR_ATTACHMENT_OPTIMAL, RHI::ImageLayout::TRANSFER_SRC_OPTIMAL);
    commandList->transitionImageLayout(pDestSurface.get(), RHI::ImageLayout::UNDEFINED, RHI::ImageLayout::TRANSFER_DST_OPTIMAL);

    if (RTDesc.sampleCount != 1)
    {
        const RHI::TextureDesc& DestDesc = pDestSurface->getDesc();

        commandList->resolveTexture(pRenderTarget.get(), RHI::TextureSubresourse{}, pDestSurface.get(), RHI::TextureSubresourse{});
    }
    else
    {
        commandList->copyTexture(pRenderTarget.get(), RHI::TextureSubresourse{}, pDestSurface.get(), RHI::TextureSubresourse{});
    }

    commandList->transitionImageLayout(pRenderTarget.get(), RHI::ImageLayout::TRANSFER_SRC_OPTIMAL, RHI::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
    commandList->transitionImageLayout(pDestSurface.get(), RHI::ImageLayout::TRANSFER_DST_OPTIMAL, RHI::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

    return 0;
}

RHI::DeviceParams& RENDER::GetDeviceParams() const
{
    return m_DynamicRHI->getDeviceParams();
}

CVideoTexture* RENDER::GetVideoTexture(const char* sVideoName)
{
    if (sVideoName == nullptr)
        return nullptr;
    CVideoTexture* retVal = nullptr;
    VideoTextureEntity* pVTLcur = pVTL;

    // check already loaded
    const std::uint32_t newHash = MakeHashValue(sVideoName);
    while (pVTLcur != nullptr)
    {
        if (pVTLcur->hash == newHash && storm::iEquals(pVTLcur->name, sVideoName))
        {
            if (core.GetEntityPointer(pVTLcur->videoTexture_id))
            {
                pVTLcur->ref++;
                return pVTLcur->VideoTexture;
            }
            VideoTextureEntity* pVTmp = pVTLcur;
            pVTLcur = pVTLcur->next;
            delete pVTmp->name;
            delete pVTmp;
            continue;
        }
        pVTLcur = pVTLcur->next;
    }

    // create new video texture
    pVTLcur = new VideoTextureEntity;
    if (pVTLcur == nullptr)
        throw std::runtime_error("memory allocate error");
    pVTLcur->next = pVTL;
    pVTLcur->VideoTexture = nullptr;
    pVTLcur->hash = newHash;
    pVTLcur->ref = 1;
    const auto len = strlen(sVideoName) + 1;
    if ((pVTLcur->name = new char[len]) == nullptr)
        throw std::runtime_error("memory allocate error");
    strcpy_s(pVTLcur->name, len, sVideoName);
    const entid_t ei = core.CreateEntity("TextureSequence");
    pVTLcur->VideoTexture = static_cast<CVideoTexture*>(core.GetEntityPointer(ei));
    if (pVTLcur->VideoTexture != nullptr)
    {
        pVTLcur->videoTexture_id = ei;
        if (pVTLcur->VideoTexture->Initialize(this, sVideoName, true) == nullptr)
        {
            delete pVTLcur;
            core.EraseEntity(ei);
        }
        else
        {
            pVTL = pVTLcur;
            retVal = pVTLcur->VideoTexture;
        }
    }
    else
    {
        delete pVTLcur;
        core.EraseEntity(ei);
    }

    return retVal;
}

void RENDER::ReleaseVideoTexture(CVideoTexture* pVTexture)
{
    VideoTextureEntity* cur = pVTL;
    VideoTextureEntity* prev = nullptr;

    if (cur != nullptr)
        do
        {
            if (cur->VideoTexture == pVTexture)
            {
                cur->ref--;
                if (cur->ref > 0)
                    return;
                if (prev == nullptr)
                    pVTL = cur->next;
                else
                    prev->next = cur->next;
                core.EraseEntity(cur->videoTexture_id);
                delete cur->name;
                delete cur;
                break;
            }
            prev = cur;
        } while ((cur = cur->next) != nullptr);
}

void RENDER::PlayToTexture()
{
    VideoTextureEntity* cur = pVTL;
    while (cur != nullptr)
    {
        if (core.GetEntityPointer(pVTL->videoTexture_id))
        {
            cur->VideoTexture->FrameUpdate();
            cur = cur->next;
        }
        else
        {
            core.Trace("ERROR: void DX9RENDER::PlayToTexture()");
            delete cur->name;
            VideoTextureEntity* pcur = cur;
            cur = cur->next;
            if (pVTL == pcur)
                pVTL = cur;
            delete pcur;
        }
    }
}

std::int32_t RENDER::ImageBlt(std::int32_t TextureID, Rect* pDstRect, Rect* pSrcRect)
{
    struct F3DVERTEX
    {
        float x, y, z, rhw;
        float tu, tv;
    };
    Rect dr;
    F3DVERTEX v[6];
    std::int32_t res;

    if (pDstRect)
    {
        dr = *pDstRect;
    }
    else
    {
        dr.left = 0;
        dr.top = 0;
        dr.right = screen_size.x - 1;
        dr.bottom = screen_size.y - 1;
    }

    for (std::uint32_t n = 0; n < 6; n++)
    {
        v[n].rhw = 1.0f;
        v[n].z = 0.5f;
    }

    v[0].x = static_cast<float>(dr.left);
    v[0].y = static_cast<float>(dr.top);
    v[0].tu = 0.0f;
    v[0].tv = 0.0f;

    v[1].x = static_cast<float>(dr.left);
    v[1].y = static_cast<float>(dr.bottom);
    v[1].tu = 0.0f;
    v[1].tv = 1.0f;

    v[2].x = static_cast<float>(dr.right);
    v[2].y = static_cast<float>(dr.top);
    v[2].tu = 1.0f;
    v[2].tv = 0.0f;

    v[3].x = static_cast<float>(dr.right);
    v[3].y = static_cast<float>(dr.top);
    v[3].tu = 1.0f;
    v[3].tv = 0.0f;

    v[4].x = static_cast<float>(dr.left);
    v[4].y = static_cast<float>(dr.bottom);
    v[4].tu = 0.0f;
    v[4].tv = 1.0f;

    v[5].x = static_cast<float>(dr.right);
    v[5].y = static_cast<float>(dr.bottom);
    v[5].tu = 1.0f;
    v[5].tv = 1.0f;

    TextureSet(TextureID, 0, ,);

    const bool bDraw = TechniqueExecuteStart("texturedialogfon");
    if (bDraw)
        do
        {
            vertexFormat = VertexFVFBits::XYZ | VertexFVFBits::UV1;
            RHI::DrawArguments drawArgs = {};
            drawArgs.instanceCount = 2;
            drawArgs.vertexCount = 6;
            commandList->draw(drawArgs);
            //res = d3d9->DrawPrimitiveUP(RHI::PrimitiveType::TriangleList, 2, &v, sizeof(F3DVERTEX));
            dwNumDrawPrimitive++;
        } while (TechniqueExecuteNext());

    return res;
}

std::int32_t RENDER::ImageBlt(const char* pName, Rect* pDstRect, Rect* pSrcRect)
{
    std::int32_t TextureID;
    TextureID = TextureCreate(pName);
    const std::int32_t res = ImageBlt(TextureID, pDstRect, pSrcRect);
    TextureRelease(TextureID);
    SetProgressImage(pName);
    return res;
}

void RENDER::SetProgressImage(const char* image)
{
    if (!image || !image[0])
    {
        if (progressImageSize > 0 && progressImage)
            progressImage[0] = 0;
        return;
    }
    const std::int32_t s = strlen(image) + 1;
    if (s > progressImageSize)
    {
        progressImageSize = s;
        delete progressImage;
        progressImage = new char[progressImageSize];
    }
    strcpy_s(progressImage, s, image);
}

void RENDER::SetProgressBackImage(const char* image)
{
    if (!image || !image[0])
    {
        if (progressBackImageSize > 0 && progressBackImage)
            progressBackImage[0] = 0;
        return;
    }
    const std::int32_t s = strlen(image) + 1;
    if (s > progressBackImageSize)
    {
        progressBackImageSize = s;
        delete progressBackImage;
        progressBackImage = new char[progressBackImageSize];
    }
    strcpy_s(progressBackImage, s, image);
}

void RENDER::SetTipsImage(const char* image)
{
    if (!image || !image[0])
    {
        if (progressTipsImageSize > 0 && progressTipsImage)
            progressTipsImage[0] = 0;
        return;
    }
    const std::int32_t s = strlen(image) + 1;
    if (s > progressTipsImageSize)
    {
        progressTipsImageSize = s;
        delete progressTipsImage;
        progressTipsImage = new char[progressTipsImageSize];
    }
    memcpy(progressTipsImage, image, s);
}

char* RENDER::GetTipsImage()
{
    return progressTipsImage;
}

void RENDER::StartProgressView()
{
    progressSafeCounter = 0;
    if (progressTexture < 0)
    {
        // Loading the texture
        loadFrame = 0;
        isInPViewProcess = true;
        const std::int32_t t = TextureCreate("Loading\\progress.tga");
        isInPViewProcess = false;
        if (t < 0)
        {
            core.Trace("Progress error!");
            return;
        }
        progressTexture = t;
    }
    else
        return;
    // Loading an unscaled background image
    if (progressBackImage && progressBackImage[0])
    {
        isInPViewProcess = true;
        if (back0Texture >= 0)
            TextureRelease(back0Texture);
        back0Texture = -1;
        back0Texture = TextureCreate(progressBackImage);
        isInPViewProcess = false;
    }
    else
    {
        back0Texture = -1;
    }
    // Loading a scalable background image
    if (progressImage && progressImage[0])
    {
        isInPViewProcess = true;
        if (backTexture >= 0)
            TextureRelease(backTexture);
        backTexture = -1;
        backTexture = TextureCreate(progressImage);
        isInPViewProcess = false;
    }
    else
    {
        backTexture = -1;
    }
    // Loading tips
    if (progressTipsImage && progressTipsImage[0])
    {
        isInPViewProcess = true;
        if (progressTipsTexture >= 0)
            TextureRelease(progressTipsTexture);
        progressTipsTexture = -1;
        progressTipsTexture = TextureCreate(progressTipsImage);
        isInPViewProcess = false;
    }
    progressUpdateTime = SDL_GetTicks() - 1000;
}

void RENDER::ProgressView()
{
    // get the texture
    if (progressTexture < 0)
        return;
    if (isInPViewProcess)
        return;
    // Analyzing time
    const std::uint32_t time = SDL_GetTicks();
    if (abs(static_cast<std::int32_t>(progressUpdateTime - time)) < 50)
        return;
    progressUpdateTime = time;
    isInPViewProcess = true;
    progressSafeCounter = 0;
    // Drawing mode
    BeginScene();
    // Filling the vertices of the texture
    struct LoadVertex
    {
        float x, y, z, rhw;
        std::uint32_t color;
        float u, v;
    } v[4];
    std::uint32_t i;
    for (i = 0; i < 4; i++)
    {
        v[i].z = 0.5;
        v[i].rhw = 2.0;
        v[i].color = 0xffffffff;
    }
    // Calculate the rectangle in which to draw
    RHI::Viewport vp = GetViewport();
    float vpWidth = vp.getWidth();
    float vpHeight = vp.getHeight();

    v[0].x = 0.0f;
    v[0].y = 0.0f;
    v[1].x = vpWidth;
    v[1].y = 0.0f;
    v[2].x = 0.0f;
    v[2].y = vpHeight;
    v[3].x = vpWidth;
    v[3].y = vpHeight;
    v[0].u = 0.0f;
    v[0].v = 0.0f;
    v[1].u = 1.0f;
    v[1].v = 0.0f;
    v[2].u = 0.0f;
    v[2].v = 1.0f;
    v[3].u = 1.0f;
    v[3].v = 1.0f;
    TextureSet(back0Texture, 0, );
    if (back0Texture < 0)
        for (i = 0; i < 4; i++)
            v[i].color = 0;
    if (back0Texture >= 0)
    {
        DrawPrimitive(RHI::PrimitiveType::TriangleStrip, VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1, 4, 1, 0, , 24,
            "ProgressBackTech");
    }

    float dy = 0.0f;
    float dx = ((float(vpWidth) - (4.0f * float(vpHeight) / 3.0f)) / 2.0f);
    if (dx < 10.0f)
        dx = 0.0f;
    else
    {
        dy = 25.0f;
        dx = ((float(vpWidth) - (4.0f * (float(vpHeight) - 2.0f * dy) / 3.0f)) / 2.0f);
    }

    v[0].x = 0.0f + dx;
    v[0].y = 0.0f + dy;
    v[1].x = float(vpWidth) - dx;
    v[1].y = 0.0f + dy;
    v[2].x = 0.0f + dx;
    v[2].y = float(vpHeight) - dy;
    v[3].x = float(vpWidth) - dx;
    v[3].y = float(vpHeight) - dy;
    v[0].u = 0.0f;
    v[0].v = 0.0f;
    v[1].u = 1.0f;
    v[1].v = 0.0f;
    v[2].u = 0.0f;
    v[2].v = 1.0f;
    v[3].u = 1.0f;
    v[3].v = 1.0f;

    TextureSet(backTexture, 0, );
    if (backTexture < 0)
    {
        for (i = 0; i < 4; i++)
            v[i].color = 0;
    }
    else
    {
        if (progressTipsTexture >= 0)
        {
            TextureSet(progressTipsTexture, 1, );
            DrawPrimitive(RHI::PrimitiveType::TriangleStrip, VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1, 4, 1, 0, v, sizeof(v[0]),
                "ProgressBackTechWithTips");
        }
        else
        {
            DrawPrimitive(RHI::PrimitiveType::TriangleStrip, VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1, 4, 1, 0, v, sizeof(v[0]),
                "ProgressBackTech");
        }
    }
    if (backTexture < 0)
        for (i = 0; i < 4; i++)
            v[i].color = 0xffffffff;
    // Animated object
    m_fHeightDeformator = ((float)vpHeight * 4.0f) / ((float)vpWidth * 3.0f);
    // core.Trace(" size_x %f", (vp.Width - dx * 2.0f)*progressFramesWidth);
    CVECTOR pos((vpWidth - dx * 2.0f) * progressFramesPosX + dx, (vpHeight - dy * 2.0f) * progressFramesPosY + dy,
        0.0f);
    CVECTOR size((vpWidth - dx * 2.0f) * progressFramesWidth,
        (vpHeight - dy * 2.0f) * progressFramesHeight * 4.0f / 3.0f, 0.0f);
    v[0].x = pos.x;
    v[0].y = pos.y;
    v[1].x = pos.x + size.x + 0.5f;
    v[1].y = pos.y;
    v[2].x = pos.x;
    v[2].y = pos.y + size.y + 0.5f;
    v[3].x = pos.x + size.x + 0.5f;
    v[3].y = pos.y + size.y + 0.5f;
    v[3].y = pos.y + size.y + 0.5f;
    // Frame grid size
    std::int32_t sizeX = progressFramesCountX;
    std::int32_t sizeY = progressFramesCountY;
    // Position of the current frame
    std::int32_t fx = loadFrame % sizeX;
    std::int32_t fy = loadFrame / sizeY;
    v[0].u = fx / float(sizeX);
    v[0].v = fy / float(sizeY);
    v[1].u = (fx + 1.0f) / float(sizeX);
    v[1].v = fy / float(sizeY);
    v[2].u = fx / float(sizeX);
    v[2].v = (fy + 1.0f) / float(sizeY);
    v[3].u = (fx + 1.0f) / float(sizeX);
    v[3].v = (fy + 1.0f) / float(sizeY);
    // Draw
    TextureSet(progressTexture, 0);
    DrawPrimitive(RHI::PrimitiveType::TriangleStrip, VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1, 4, 1, 0, v, sizeof(v[0]),
        "ProgressTech");
    EndScene();
    d3d9->Present(nullptr, nullptr, nullptr, nullptr);
    BeginScene();
    // Next frame
    loadFrame++;
    if (loadFrame >= sizeX * sizeY)
        loadFrame = 0;
    // leave
    isInPViewProcess = false;
}

void RENDER::EndProgressView()
{
    if (progressTexture >= 0)
        TextureRelease(progressTexture);
    progressTexture = -1;
    if (backTexture >= 0)
        TextureRelease(backTexture);
    backTexture = -1;
    if (back0Texture >= 0)
        TextureRelease(back0Texture);
    back0Texture = -1;
    if (progressTipsTexture >= 0)
        TextureRelease(progressTipsTexture);
    progressTipsTexture = -1;
    if (progressImage && progressImageSize > 0)
        progressImage[0] = 0;
    if (progressBackImage && progressBackImageSize > 0)
        progressBackImage[0] = 0;
    if (progressTipsImage && progressTipsImageSize > 0)
        progressTipsImage[0] = 0;
}

void RENDER::SetColorParameters(float fGamma, float fBrightness, float fContrast)
{
    uint16_t rgb[256];
    for (std::uint32_t i = 0; i < 256; i++)
    {
        float fRamp = std::clamp(fContrast * 255.0f * 256.0f * powf(static_cast<float>(i / 255.0f), 1.0f / fGamma) +
            fBrightness * 256.0f,
            0.0f, 65535.0f);
        rgb[i] = static_cast<uint16_t>(fRamp);
    }
    core.GetWindow()->SetGamma(rgb, rgb, rgb);
}

void RENDER::MakeDrawVector(RS_LINE* pLines, std::uint32_t dwNumSubLines, const CMatrix& mMatrix, CVECTOR vUp, CVECTOR v1,
    CVECTOR v2, float fScale, std::uint32_t dwColor)
{
    std::uint32_t i;
    std::uint32_t k;

    // for (i=0; i<dwNumSubLines * 2 + 2; i++) pLines[i].dwColor = dwColor;
    k = dwNumSubLines * 2 + 2; // boal optimization if the for loop runs the calculations every iteration
    for (i = 0; i < k; i++)
    {
        pLines[i].dwColor = dwColor;
    }
    pLines[0].vPos = v1;
    pLines[1].vPos = v1 + (fScale * v2);

    const float fRadius = 0.03f * fScale;
    const float fDist = 0.85f * fScale * sqrtf(~v2);

    for (i = 0; i < dwNumSubLines; i++)
    {
        const float fAng = PIm2 * static_cast<float>(i) / static_cast<float>(dwNumSubLines);

        const float x = fRadius * sinf(fAng);
        const float z = fRadius * cosf(fAng);

        CVECTOR vRes;

        if (fabsf(vUp.x) < 1e-5f)
            vRes = CVECTOR(fDist, x, z);
        if (fabsf(vUp.y) < 1e-5f)
            vRes = CVECTOR(x, fDist, z);
        if (fabsf(vUp.z) < 1e-5f)
            vRes = CVECTOR(x, z, fDist);
        vRes = (CMatrix&)mMatrix * vRes;
        pLines[2 + i * 2 + 0].vPos = vRes;
        pLines[2 + i * 2 + 1].vPos = pLines[1].vPos;
    }
}

void RENDER::DrawVector(const CVECTOR& v1, const CVECTOR& v2, std::uint32_t dwColor, const char* pTechniqueName)
{
    RS_LINE lines[51 * 2];
    CMatrix mView;

    const float fScale = sqrtf(~(v2 - v1));
    if (!mView.BuildViewMatrix(v1, v2, CVECTOR(0.0f, 1.0f, 0.0f)))
        if (!mView.BuildViewMatrix(v1, v2, CVECTOR(1.0f, 0.0f, 0.0f)))
            return;

    mView.Transposition();

    MakeDrawVector(&lines[0], 50, mView, CVECTOR(1.0f, 1.0f, 0.0f), mView.Pos(), mView.Vz(), fScale, dwColor);

    CMatrix mWorldSave;
    GetTransform(TSType::TS_WORLD, &mWorldSave);
    SetTransform(TSType::TS_WORLD, CMatrix());
    DrawLines(lines, 51, pTechniqueName);
    SetTransform(TSType::TS_WORLD, mWorldSave);
}

void RENDER::DrawSphere(const CVECTOR& vPos, float fRadius, std::uint32_t dwColor)
{
    CMatrix m;
    m.BuildPosition(vPos.x, vPos.y, vPos.z);
    m.m[0][0] = fRadius;
    m.m[1][1] = fRadius;
    m.m[2][2] = fRadius;

    textureFactorColor = dwColor;
    SetTransform(TSType::TS_WORLD, m);
    DrawPrimitive(RHI::PrimitiveType::TriangleList, VertexFVFBits::XYZ | VertexFVFBits::Color, sphereNumTrgs * 6, 1, 0,
        sphereVertexBuffer, sizeof(SphereVertex), "DXSphere");
}


void RENDER::DrawEllipsoid(const CVECTOR& vPos, float a, float b, float c, float ay, std::uint32_t dwColor)
{
    CMatrix trans, scale, rot;
    trans.BuildPosition(vPos.x, vPos.y, vPos.z);
    scale.BuildScale(a, b, c);
    rot.BuildRotateY(ay);

    CMatrix mWorldRes = scale * rot * trans;
    SetTransform(TSType::TS_WORLD, mWorldRes);
    textureFactorColor = dwColor;
    DrawPrimitive(RHI::PrimitiveType::TriangleList, VertexFVFBits::XYZ | VertexFVFBits::Color, sphereNumTrgs * 6, 1, 0,
        sphereVertexBuffer, sizeof(SphereVertex), "DXEllipsoid");
}

void RENDER::SetLoadTextureEnable(bool bEnable)
{
    bLoadTextureEnabled = bEnable;
}

RHI::TextureHandle RENDER::CreateVolumeTexture(std::uint32_t Width, std::uint32_t Height, std::uint32_t Depth, std::uint32_t Levels,
    std::uint32_t Usage, RHI::Format Format, RHI::MemoryPropertiesBits Pool)
{
    RHI::TextureDesc desc = {};
    desc.setWidth(Width)
        .setHeight(Height)
        .setDepth(Depth)
        .setMipLevels(Levels)
        .setIsTransferDst(true)
        .setFormat(Format)
        .setMemoryProperties(Pool);
    return device->createImage(desc);
}

bool RENDER::PushRenderTarget()
{
    RenderTarget renderTarget{};
    GetRenderTarget(renderTarget.pRenderTarget);
    renderTarget.pDepthSurface = m_DynamicRHI->GetDepthBuffer();
    renderTarget.ViewPort = GetViewport();
    stRenderTarget.push(renderTarget);

    return true;
}

bool RENDER::PopRenderTarget()
{
    if (stRenderTarget.empty())
    {
        core.Trace("DX9Error: Try to pop RenderTarget, but RenderTarget stack is empty");
        return false;
    }

    auto top = stRenderTarget.top();
    SetRenderTarget(top.pRenderTarget, top.pDepthSurface);
    SetViewport(top.ViewPort);
    top.pRenderTarget = nullptr;
    top.pDepthSurface = nullptr;
    stRenderTarget.pop();

    return true;
}

bool RENDER::SetRenderTarget(RHI::TextureHandle pCubeRenderTarget, std::uint32_t faceType, std::uint32_t mipLevel,
    RHI::TextureHandle pZStencil)
{
    currentRenderTarget.pRenderTarget = pCubeRenderTarget;
    currentRenderTarget.pDepthSurface = pZStencil;
    currentRenderTarget.CubeFace = faceType;
    return true;
}

void RENDER::SetView(const CMatrix& mView)
{
    this->mView = mView;
}

void RENDER::SetWorld(const CMatrix& mWorld)
{
    this->mWorld = mWorld;
}

void RENDER::SetProjection(const CMatrix& mProjection)
{
    this->mProjection = mProjection;
}

const CMatrix& RENDER::GetView() const
{
    return mView;
}

const CMatrix& RENDER::GetWorld() const
{
    return mWorld;
}

const CMatrix& RENDER::GetProjection() const
{
    return mProjection;
}

RHI::TextureHandle RENDER::GetTextureFromID(std::int32_t nTextureID)
{
    if (nTextureID < 0)
        return nullptr;
    return Textures[nTextureID].tex;
}

bool RENDER::GetRenderTargetAsTexture(RHI::TextureHandle tex)
{
    tex = m_DynamicRHI->GetBackBuffer(m_DynamicRHI->GetCurrentBackBufferIndex());

    // Maybe need to copy image

    return true;
}
