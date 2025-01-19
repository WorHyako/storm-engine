#pragma once 

#include <bitset>
#include <service.h>
#include <matrix.h>
#include <RHICommon.hpp>

#include "rhi_video_texture.h"
#include "script_libriary.h"
#include "utf8.h"

#include <stack>
#include <vector>

#define MAX_STEXTURES 10240
#define MAX_BUFFERS 10240
constexpr size_t MAX_FONTS = 256;

struct RS_RECT
{
    CVECTOR vPos;
    float fSize;
    float fAngle;
    std::uint32_t dwColor;
    std::uint32_t dwSubTexture;
};

struct RS_LINE
{
    CVECTOR vPos;
    std::uint32_t dwColor;
};

struct RS_SPRITE
{
    CVECTOR vPos;
    float rhw;
    std::uint32_t dwColor;
    float tu, tv;
};

struct RS_LINE2D
{
    CVECTOR vPos;
    float rhw;
    std::uint32_t dwColor;
};

typedef enum
{
	TS_WORLD = 1,
    TS_VIEW = 2,
    TS_PROJECTION = 3,
    TS_NONE = 0
} TSType;

typedef enum {
    LIGHT_POINT = 1,
    LIGHT_SPOT = 2,
    LIGHT_DIRECTIONAL = 3,
    LIGHT_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
} LightType;

typedef struct {
    float r;
    float g;
    float b;
    float a;
} Color;

typedef struct {
    LightType       Type;            /* Type of light source */
    Color           Diffuse;         /* Diffuse color of light */
    Color           Specular;        /* Specular color of light */
    Color           Ambient;         /* Ambient color of light */
    CVECTOR         Position;        /* Position in world space */
    CVECTOR         Direction;       /* Direction in world space */
    float           Range;           /* Cutoff range */
    float           Falloff;         /* Falloff */
    float           Attenuation0;    /* Constant attenuation */
    float           Attenuation1;    /* Linear attenuation */
    float           Attenuation2;    /* Quadratic attenuation */
    float           Theta;           /* Inner angle of spotlight cone */
    float           Phi;             /* Outer angle of spotlight cone */
} Light;

typedef struct {
    Color           Diffuse;         /* Diffuse color RGBA */
    Color           Ambient;         /* Ambient color RGB */
    Color           Specular;        /* Specular 'shininess' */
    Color           Emissive;        /* Emissive color RGB */
    float           Power;           /* Sharpness if specular highlight */
} Material;

typedef struct
{
    std::uint32_t x;
    std::uint32_t y;
} Point;

typedef struct
{
    std::uint32_t    left;
    std::uint32_t    top;
    std::uint32_t    right;
    std::uint32_t    bottom;
} Rect;

typedef struct
{
    float Nx;
    float Ny;
    float Nz;
    float D;
} Plane;

struct FVF_VERTEX
{
    float x, y, z;
    float Nx, Ny, Nz;
    float tu, tv;
};

/* CubeMap Face identifiers */
typedef enum 
{
    CUBEMAP_FACE_POSITIVE_X = 0,
    CUBEMAP_FACE_NEGATIVE_X = 1,
    CUBEMAP_FACE_POSITIVE_Y = 2,
    CUBEMAP_FACE_NEGATIVE_Y = 3,
    CUBEMAP_FACE_POSITIVE_Z = 4,
    CUBEMAP_FACE_NEGATIVE_Z = 5,

    CUBEMAP_FACE_FORCE_DWORD = 0x7fffffff
} CubemapFaces;

class LostDeviceSentinel : public SERVICE
{
    void RunStart() override;
};

struct D3DERRORS
{
    HRESULT err;
    char* descrition;
};

struct texpaths_t
{
    char str[MAX_PATH];
};

struct STEXTURE
{
    RHI::TextureHandle tex;
    char* name;
    std::uint32_t hash;
    std::int32_t ref;
    std::uint32_t dwSize;
    bool isCubeMap;
    bool loaded;
};

//-----------buffers-----------
struct VERTEX_BUFFER
{
    std::uint32_t dwNumLocks;
    std::int32_t type;
    std::int32_t size;
    RHI::BufferHandle buff;
};

struct INDEX_BUFFER
{
    std::uint32_t dwNumLocks;
    std::uint32_t dwUsage;
    std::int32_t size;
    RHI::BufferHandle buff;
};

struct FONTEntity
{
    std::string name;
    std::uint32_t hash;
    std::unique_ptr<storm::VFont> font;
    std::int32_t ref;
};

struct VideoTextureEntity
{
    char* name;
    std::uint32_t hash;
    std::int32_t ref;
    uint64_t videoTexture_id;
    CVideoTexture* VideoTexture;
    VideoTextureEntity* next;
};

enum Align
{
    LEFT = 0,
	RIGHT = 1,
	CENTER = 2
};

typedef enum
{
    IFF_BMP = 0,
    IFF_JPG = 1,
    IFF_TGA = 2,
    IFF_PNG = 3,
    IFF_DDS = 4,
    IFF_PPM = 5,
    IFF_DIB = 6,
    IFF_HDR = 7,       //high dynamic range formats
    IFF_PFM = 8,       //
    IFF_FORCE_DWORD = 0x7fffffff

} ImageFileFormat;

struct AlphaTestState
{
    bool alphaTestEnable = true;
    float alphaRefValue = 0.0f;
    RHI::CompareOp alphaCompareOp = RHI::CompareOp::GREATER;
};

enum class VertexFVFBits : uint8_t
{
    XYZ     =   0x00000001,
    Color   =   0x00000002,
    Normal  =   0x00000004,
    UV1     =   0x00000008,
    UV2     =   0x00000016,
    UV3     =   0x00000032,
    UV4     =   0x00000064
};

ENUM_CLASS_FLAG_OPERATORS(VertexFVFBits)

enum class ClearBits : uint8_t
{
    Color = 0x00000001,
    Depth = 0x00000002,
    Stencil = 0x00000004,
};

ENUM_CLASS_FLAG_OPERATORS(ClearBits)

struct ShadingState
{
    bool enableLighting = false;
    Color ambientColor ;
    std::uint32_t ambientMaterialSource = 1; // First color in vertex declaration
    bool vertexColoring = true;
    bool specularHighlights = false;
    bool localViewerSpecularHighlights = false;
};

// ----- Sound statistics -----
extern std::uint32_t dwSoundBuffersCount;
extern std::uint32_t dwSoundBytes;
extern std::uint32_t dwSoundBytesCached;

class RENDER_SCRIPT_LIBRIARY : public SCRIPT_LIBRIARY
{
public:
    RENDER_SCRIPT_LIBRIARY() {};

    ~RENDER_SCRIPT_LIBRIARY() override {};
    bool Init() override;
};

//-----------SDEVICE-----------
class RENDER : public SERVICE
{
public:
    static RENDER* pRS;

    RENDER();
    ~RENDER() override;

    bool InitDevice(bool windowed, std::uint32_t width, std::uint32_t height);
    bool ReleaseDevice();

    // Render: Animation
    void RenderAnimation(std::int32_t ib, void* src, std::int32_t numVrts, std::int32_t minv, std::int32_t numv, std::int32_t startidx, std::int32_t numtrg,
        bool isUpdateVB);

    // Render: Return device
    virtual std::shared_ptr<RHI::IDevice> GetDevice()
    {
        return device;
    };

    // Render: Render Target/Begin/End/Clear
    bool Clear(ClearBits type) override; // D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
    bool BeginScene();
    bool EndScene() override;

    // Render: Materials/Lights Section
    bool SetLight(std::uint32_t dwIndex, const Light* pLight) override;
    bool LightEnable(std::uint32_t dwIndex, bool bOn) override;
    bool SetMaterial(Material& material) override;
    bool GetLightEnable(std::uint32_t dwIndex, bool* pEnable) const;
    bool GetLight(std::uint32_t dwIndex, Light* pLight) const;

    // Render: Screenshot Section
    void SaveShoot() override;

    // Render: Clip Planes Section
    Plane* GetPlanes() override;

    // Render: Camera Section
    void SetTransform(std::int32_t type, const CMatrix& mtx) override;
    void GetTransform(std::int32_t type, CMatrix* mtx) override;

    bool SetCamera(const CVECTOR& pos, const CVECTOR& ang, float perspective) override;
    bool SetCamera(const CVECTOR& pos, const CVECTOR& ang) override;
    bool SetCamera(CVECTOR lookFrom, CVECTOR lookTo, CVECTOR up) override;
    bool SetPerspective(float perspective, float fAspectRatio = -1.0f) override;
    void GetCamera(CVECTOR& pos, CVECTOR& ang, float& perspective) override;

    bool SetCurrentMatrix(D3DMATRIX* mtx) override;

    // Render: Textures Section
    std::int32_t TextureCreate(const char* fname) override;
    std::int32_t TextureCreate(std::uint32_t width, std::uint32_t height, std::uint32_t levels, std::uint32_t usage, RHI::Format format, RHI::MemoryPropertiesBits pool) override;
    bool TextureSet(std::uint32_t textureIndex, std::uint32_t textureBindingIndex, RHI::SamplerHandle sampler, RHI::DescriptorSetInfo& dsInfos);
    bool TextureRelease(std::int32_t texid) override;
    bool TextureIncReference(std::int32_t texid) override;

    // Render: Fonts Section
    std::int32_t Print(std::int32_t x, std::int32_t y, const char* format, ...) override;
    std::int32_t Print(std::int32_t nFontNum, std::uint32_t color, std::int32_t x, std::int32_t y, const char* format, ...) override;
    std::int32_t ExtPrint(std::int32_t nFontNum, std::uint32_t foreColor, std::uint32_t backColor, int wAlignment, bool bShadow, float fScale,
        std::int32_t scrWidth, std::int32_t scrHeight, std::int32_t x, std::int32_t y, const char* format, ...) override;
    std::int32_t StringWidth(const char* string, std::int32_t nFontNum = 0, float fScale = 1.f, std::int32_t scrWidth = 0) override;
    std::int32_t StringWidth(const std::string_view& string, std::int32_t nFontNum = 0, float fScale = 1.f, std::int32_t scrWidth = 0) override;
    std::int32_t CharWidth(utf8::u8_char, std::int32_t nFontNum = 0, float fScale = 1.f, std::int32_t scrWidth = 0) override;
    std::int32_t CharHeight(std::int32_t fontID) override;
    std::int32_t LoadFont(const std::string_view& fontName) override;   // returns the number \ font id, or -1 on error
    bool UnloadFont(const char* fontName) override; // returns true if the font is still in use
    bool UnloadFont(std::int32_t fontID) override;          // returns true if the font is still in use
    bool IncRefCounter(std::int32_t fontID) override;       // increase reference counter if object is being copied
    bool SetCurFont(const char* fontName) override; // returns true if the given font is installed
    bool SetCurFont(std::int32_t fontID) override;          // returns true if the given font is installed
    std::int32_t GetCurFont() override;
    char* GetFontIniFileName() override;
    bool SetFontIniFileName(const char* iniName) override;

    // DX9Render: Techniques Section
    bool TechniqueExecuteStart(const char* cBlockName) override;
    bool TechniqueExecuteNext() override;

    // DX9Render: Draw Section
    void DrawRects(RS_RECT* pRSR, std::uint32_t dwRectsNum, const char* cBlockName = nullptr, std::uint32_t dwSubTexturesX = 1,
        std::uint32_t dwSubTexturesY = 1, float fScaleX = 1.0f, float fScaleY = 1.0f) override;
    void DrawSprites(RS_SPRITE* pRSS, std::uint32_t dwSpritesNum, const char* cBlockName = nullptr) override;
    void DrawLines(RS_LINE* pRSL, std::uint32_t dwLinesNum, const char* cBlockName = nullptr) override;
    void DrawLines2D(RS_LINE2D* pRSL2D, size_t dwLinesNum, const char* cBlockName = nullptr) override;

    void DrawBuffer(RHI::PrimitiveType primitiveType, std::uint32_t vertexBufferIndex, std::int32_t iStride, size_t vertexCount,
        size_t instanceCount, size_t startVertexLocation, const char* cBlockName = nullptr) override;
    void DrawIndexedBuffer(RHI::PrimitiveType primitiveType, std::uint32_t vertexBufferIndex, std::uint32_t indexBufferIndex, size_t vertexCount, size_t instanceCount,
        size_t startIndexLocation, size_t startVertexLocation, const char* cBlockName = nullptr) override;
    void DrawIndexedBufferNoVShader(RHI::PrimitiveType primitiveType, std::uint32_t vertexBufferIndex, std::uint32_t indexBufferIndex, size_t vertexCount, size_t instanceCount,
        size_t startIndexLocation, size_t startVertexLocation, const char* cBlockName = nullptr) override;
    void DrawPrimitive(RHI::PrimitiveType primitiveType, VertexFVFBits vertexBufferFormat, size_t vertexCount, size_t instanceCount,
        size_t startVertexLocation, RHI::BufferHandle vertexBuffer, std::uint32_t stride, const char* cBlockName = nullptr)
    void DrawIndexedPrimitive(RHI::PrimitiveType primitiveType, RHI::BufferHandle vertexBuffer, VertexFVFBits vertexDataFormat,
        RHI::BufferHandle indexBuffer, RHI::Format indexDataFormat, size_t vertexCount, size_t instanceCount,
        size_t startIndexLocation, size_t startVertexLocation, const char* cBlockName = nullptr) override;

    // Render: Video Section
    void PlayToTexture() override;
    CVideoTexture* GetVideoTexture(const char* sVideoName) override;
    void ReleaseVideoTexture(CVideoTexture* pVTexture) override;

    // Render: Vertex/Index Buffers Section
    std::int32_t CreateVertexBuffer(size_t size, std::uint32_t type, RHI::MemoryPropertiesBits memoryProperties = RHI::MemoryPropertiesBits::DEVICE_LOCAL_BIT) override;
    std::int32_t CreateIndexBuffer(size_t size, std::uint32_t usage) override;

    RHI::BufferHandle GetVertexBuffer(std::int32_t id);
    std::int32_t GetVertexBufferSize(std::int32_t id) override;
    void ReleaseVertexBuffer(std::int32_t id) override;
    void ReleaseIndexBuffer(std::int32_t id) override;

    // Render: Render/Texture States Section
    /*std::uint32_t GetSamplerState(std::uint32_t Sampler, D3DSAMPLERSTATETYPE Type, std::uint32_t* pValue);
    std::uint32_t SetSamplerState(std::uint32_t Sampler, D3DSAMPLERSTATETYPE Type, std::uint32_t Value);
    std::uint32_t SetTextureStageState(std::uint32_t Stage, std::uint32_t Type, std::uint32_t Value);
    std::uint32_t GetTextureStageState(std::uint32_t Stage, std::uint32_t Type, std::uint32_t* pValue);*/

    // aspect ratio section
    float GetHeightDeformator() const
    {
        return m_fHeightDeformator;
    }

    Point GetScreenSize() const
    {
        return screen_size;
    }

    // ===============================================================================================
    // --------------------===================== D3D SECTION =====================--------------------
    // ===============================================================================================

    // D3D Device/Viewport Section
    RHI::Viewport& GetViewport();
    void SetViewport(const RHI::Viewport& pViewport);
    RHI::DeviceParams& GetDeviceParams() const;

    // D3D
    std::int32_t Release(IUnknown* pSurface) override;

    // Vertex/Index Buffers Section
    RHI::BufferHandle CreateVertexBufferAndUpload(size_t vertexBufferSize, const void* pVertexData);

    // D3D Textures/Surfaces Section
    std::int32_t GetDepthStencilSurface(RHI::TextureHandle pZStencilSurface) override;
    std::int32_t GetCubeMapSurface(RHI::TextureHandle pCubeTexture, CubemapFaces FaceType, std::uint32_t Level,
        RHI::TextureHandle pCubeMapSurface) override;
    std::int32_t CreateTexture(std::uint32_t Width, std::uint32_t Height, std::uint32_t Levels, std::uint32_t Usage, RHI::Format Format, RHI::MemoryPropertiesBits Pool,
        RHI::TextureHandle pTexture) override;
    std::int32_t CreateCubeTexture(std::uint32_t EdgeLength, std::uint32_t Levels, std::uint32_t Usage, RHI::Format Format, RHI::MemoryPropertiesBits Pool,
        RHI::TextureHandle pCubeTexture) override;
    std::int32_t CreateOffscreenPlainSurface(std::uint32_t Width, std::uint32_t Height, RHI::Format Format,
        RHI::TextureHandle pSurface) override;
    std::uint32_t CreateDepthStencilSurface(std::uint32_t Width, std::uint32_t Height, RHI::Format Format, std::uint32_t msaaSamples,
        RHI::TextureHandle pSurface) override;
    std::int32_t SetTexture(RHI::TextureHandle pTexture, RHI::SamplerHandle sampler, std::uint32_t textureBindingIndex, RHI::DescriptorSetInfo& dsInfos) override;
    std::int32_t GetLevelDesc(RHI::TextureHandle pTexture, std::uint32_t Level, D3DSURFACE_DESC* pDesc) override;
    std::int32_t GetLevelDesc(RHI::TextureHandle pCubeTexture, std::uint32_t Level, D3DSURFACE_DESC* pDesc) override;
    std::int32_t GetSurfaceLevel(RHI::TextureHandle pTexture, std::uint32_t Level, IDirect3DSurface9** ppSurfaceLevel) override;
    std::int32_t UpdateSurface(RHI::TextureHandle pSourceSurface, RHI::TextureHandle pDestinationSurface) override;
    std::int32_t StretchRect(RHI::TextureHandle pSourceSurface, RHI::TextureHandle pDestinationSurface) override;
    std::int32_t GetRenderTargetData(RHI::TextureHandle pRenderTarget, RHI::TextureHandle pDestSurface) override;

    // Pixel/Vertex Shaders Section
#ifdef _WIN32 // Effects
    ID3DXEffect* GetEffectPointer(const char* techniqueName) override;
#endif

    // D3D Render Target/Begin/End/Clear
    std::int32_t GetRenderTarget(RHI::TextureHandle pRenderTarget) override;
    std::int32_t SetRenderTarget(RHI::TextureHandle pRenderTarget, RHI::TextureHandle pNewZStencil) override;
    std::int32_t Clear(std::uint32_t Count, const D3DRECT* pRects, std::uint32_t Flags, Color Color, float Z,
        std::uint32_t Stencil) override;

    std::int32_t ImageBlt(const char* pName, Rect* pDstRect, Rect* pSrcRect);
    std::int32_t ImageBlt(std::int32_t nTextureId, Rect* pDstRect, Rect* pSrcRect) override;

    void MakeScreenShot();
    std::uint32_t LoadCubmapSide(std::fstream& fileS, RHI::TextureHandle tex, CubemapFaces face, std::uint32_t numMips,
        std::uint32_t mipSize, std::uint32_t size, bool isSwizzled);

    // core interface
    bool Init() override;
    void RunStart() override;
    void RunEnd() override;

    std::uint32_t RunSection() override
    {
        return SECTION_REALIZE;
    };

    void ProcessScriptPosAng(const CVECTOR& vPos, const CVECTOR& vAng);
    void FindPlanes();

    void SetCommonStates();
    void SetProgressImage(const char* image) override;
    void SetProgressBackImage(const char* image) override;
    void SetTipsImage(const char* image) override;
    void StartProgressView() override;
    void ProgressView() override;
    void EndProgressView() override;

    static const std::uint32_t rectsVBuffer_SizeInRects;
    RHI::BufferHandle rectsVBuffer;

    char* progressImage;
    std::int32_t progressImageSize;
    std::int32_t backTexture;
    char* progressBackImage;
    std::int32_t progressBackImageSize;
    std::int32_t back0Texture;
    std::int32_t progressTexture;
    char* progressTipsImage;
    std::int32_t progressTipsImageSize;
    std::int32_t progressTipsTexture;

    std::int32_t loadFrame;
    std::int32_t progressSafeCounter;
    bool isInPViewProcess;
    std::uint32_t progressUpdateTime;
    float progressFramesPosX;
    float progressFramesPosY;
    float progressFramesWidth;
    float progressFramesHeight;
    std::int32_t progressFramesCountX;
    std::int32_t progressFramesCountY;

    // new renderer settings
    bool vSyncEnabled;
    std::uint32_t msaa;
    std::uint32_t videoAdapterIndex;

    CMatrix mView, mWorld, mProjection;

    CVECTOR vWordRelationPos;
    CVECTOR vViewRelationPos;

    bool bUseLargeBackBuffer;

    bool resourcesReleased = false;

    bool IsInsideScene() override
    {
        return bInsideScene;
    }

    char* GetTipsImage() override;

    void SetColorParameters(float fGamma, float fBrightness, float fContrast) override;
    void DrawSphere(const CVECTOR& vPos, float fRadius, std::uint32_t dwColor) override;
    void DrawEllipsoid(const CVECTOR& vPos, float a, float b, float c, float ay, std::uint32_t dwColor) override;

    void GetNearFarPlane(float& fNear, float& fFar) override;
    void SetNearFarPlane(float fNear, float fFar) override;

    void SetLoadTextureEnable(bool bEnable = true) override;
    bool ResetDevice();

    void MakeDrawVector(RS_LINE* pLines, std::uint32_t dwNumSubLines, const CMatrix& mMatrix, CVECTOR vUp, CVECTOR v1,
        CVECTOR v2, float fScale, std::uint32_t dwColor);
    void DrawVector(const CVECTOR& v1, const CVECTOR& v2, std::uint32_t dwColor,
        const char* pTechniqueName = "DXVector") override;

    bool PushRenderTarget() override;
    bool PopRenderTarget() override;
    bool SetRenderTarget(RHI::TextureHandle pCubeRenderTarget, std::uint32_t faceType, std::uint32_t mipLevel,
        RHI::TextureHandle pNewZStencil) override;
    void SetView(const CMatrix& mView);
    void SetWorld(const CMatrix& mView);
    void SetProjection(const CMatrix& mView) override;
    const CMatrix& GetView() const;
    const CMatrix& GetWorld() const;
    const CMatrix& GetProjection() const;

    RHI::TextureHandle CreateVolumeTexture(std::uint32_t Width, std::uint32_t Height, std::uint32_t Depth, std::uint32_t Levels,
        std::uint32_t Usage, RHI::Format Format, RHI::MemoryPropertiesBits Pool) override;

    void MakePostProcess() override;
    void SetGLOWParams(float _fBlurBrushSize, std::int32_t _GlowIntensity, std::int32_t _GlowPasses) override;

    RHI::TextureHandle GetTextureFromID(std::int32_t nTextureID) override;

    bool GetRenderTargetAsTexture(RHI::TextureHandle tex) override;

    void LostRender();
    void RestoreRender();

    void RecompileEffects();

private:
    struct RECT_VERTEX
    {
        CVECTOR pos;
        std::uint32_t color;
        float u, v;
    };

    struct RenderTarget
    {
        RHI::TextureHandle pRenderTarget;
        RHI::TextureHandle pDepthSurface;
        RHI::Viewport ViewPort;
        std::uint32_t CubeFace = 0u;
    };

    std::shared_ptr<RHI::IDevice> device;
    std::shared_ptr<RHI::IRHICommandList> commandList;

    CVECTOR Pos, Ang;
    float Fov;

    float FovMultiplier{ 1.0f };

#ifdef _WIN32 // Effects
    Effects effects_;
#else
    std::unique_ptr<CTechnique> pTechnique;
#endif

    std::string fontIniFileName;
    std::vector<FONTEntity> FontList{};
    std::int32_t idFontCurrent;

    VideoTextureEntity* pVTL;

    std::int32_t nTextureDegradation;
    float aspectRatio;
    float m_fHeightDeformator;

    bool bSafeRendering;
    bool bShowFps, bShowExInfo;
    bool bInsideScene;

    Plane viewplane[4];

    std::array<STEXTURE, MAX_STEXTURES> Textures{};
    std::array<INDEX_BUFFER, MAX_BUFFERS> IndexBuffers{};
    std::array<VERTEX_BUFFER, MAX_BUFFERS> VertexBuffers{};

    //-------- post process

    struct QuadVertex
    {
        Vector4 vPos;

        float u0;
        float v0;

        float u1;
        float v1;

        float u2;
        float v2;

        float u3;
        float v3;
    };

    QuadVertex PostProcessQuad[4];
    QuadVertex SeaEffectQuadVertices[32 * 32];
    uint16_t SeaEffectQuadIndices[31 * 31 * 2 * 3];

    float fSmallWidth;
    float fSmallHeight;
    RHI::TextureHandle pPostProcessTexture;

    RHI::TextureHandle pSmallPostProcessTexture;

    RHI::TextureHandle pSmallPostProcessTexture2;

    RHI::TextureHandle pOriginalScreenSurface;
    RHI::TextureHandle pOriginalDepthTexture;

    RHI::Viewport OriginalViewPort;

    RHI::BufferHandle CreateRenderQuad(float fWidth, float fHeight, float fSrcWidth, float fSrcHeight, float fMulU = 1.0f,
        float fMulV = 1.0f);

    void ClearPostProcessTexture(RHI::TextureHandle texture);
    void BlurGlowTexture();
    void CopyGlowToScreen();
    void CopyPostProcessToScreen();

    void SetPostProcessTextureAsRenderTarget();
    void SetScreenAsRenderTarget();

    float fBlurSize;
    int GlowIntensity;
    int iBlurPasses;

    bool bNeedCopyToScreen;

    bool bPostProcessEnabled;
    bool bPostProcessError;

    bool bSeaEffect;
    float fSeaEffectSize;
    float fSeaEffectSpeed;
    std::uint32_t dwBackColor;

    //-------- post process

    // state save/load ex
    Point screen_size;
    RHI::Format screen_bpp;
    RHI::Format stencil_format;

    bool bMakeShoot;
    bool bWindow;
    bool bBackBufferCanLock;

    RHI::BufferHandle aniVBuffer;
    std::int32_t numAniVerteces;

    RHI::BufferHandle pDropConveyorVBuffer;

    std::uint32_t dwNumDrawPrimitive, dwNumLV, dwNumLI;
    float fG, fB, fC;

    float fNearClipPlane, fFarClipPlane;

    bool bLoadTextureEnabled;

    bool bTrace;
    std::int32_t iSetupPath;
    uint64_t dwSetupNumber;
    texpaths_t TexPaths[4]{};

    bool bDropVideoConveyor;

    std::stack<RenderTarget> stRenderTarget;

#ifdef _WIN32 // Screenshot
    ImageFileFormat screenshotFormat;
#endif
    std::string screenshotExt;

    bool TextureLoad(std::int32_t texid);

    RHI::BufferHandle sphereVertexBuffer;

    RHI::GraphicsAPI m_GraphicsAPI = RHI::GraphicsAPI::VULKAN;
    RHI::IRHIModule* m_RhiModule;
    RHI::IDynamicRHI* m_DynamicRHI;

    RHI::Viewport viewport;
    RHI::RenderState renderState;
    AlphaTestState defaultAlphaTest = {};
    ShadingState defaultShadingState = {};
    RHI::SamplerDesc defaultSamplerDesc = {};

    RenderTarget currentRenderTarget;
    std::array<Light, 8> lights;
    std::bitset<8> enabledLightsBitMask;
    Material material;
    VertexFVFBits vertexFormat = 0;
    std::uint32_t textureFactorColor = 0;

    void MakeVertexBindings(RHI::BufferHandle vertexBuffer, const VertexFVFBits vertexBindingsFormat, std::vector<RHI::VertexBufferBinding>& vertexBufferBindings);
    void CreateInputLayout(const VertexFVFBits vertexBindingsFormat, RHI::IInputLayout* inputLayout);
    void CreateGraphicsPipeline(RHI::ShaderHandle vertexShader, RHI::ShaderHandle pixelShader,
        RHI::IInputLayout* inputLayout, RHI::IBindingLayout* bindingLayout,
        RHI::FramebufferHandle framebuffer, RHI::GraphicsPipelineHandle pipeline);
    RHI::GraphicsState CreateGraphicsState(RHI::GraphicsPipelineHandle pipeline, RHI::FramebufferHandle framebuffer,
        RHI::IBindingSet* bindingSet, RHI::BufferHandle vertexBuffer = nullptr, RHI::BufferHandle indexBuffer = nullptr);
};