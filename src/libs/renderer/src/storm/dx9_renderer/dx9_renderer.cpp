#include "storm/dx9_renderer/dx9_renderer.hpp"

#include "storm/image_loader.hpp"
#include "storm/renderer/texture_pool.hpp"
#include "../../effects.h"

#include "core.h"
#include "dx9render.h"
#include "string_compare.hpp"

#ifdef _WIN32
#include <DxErr.h>
#include <corecrt_io.h>

#include <utility>
#else
#include <unistd.h>
#endif

namespace storm
{

namespace
{

constexpr auto SPRITE_VERTEX_FORMAT = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2;

struct SpriteVertex
{
    CVECTOR pos;
    float tu, tv;
};

#define CHECKD3DERR(expr) ErrorHandler(expr, __FILE__, __LINE__, __func__, #expr)

inline bool ErrorHandler(HRESULT hr, const char *file, unsigned line, const char *func, const char *expr)
{
    if (hr != D3D_OK)
    {
#ifdef _WIN32
        core.Trace("[%s:%s:%d] %s: %s (%s)", file, func, line, DXGetErrorStringA(hr), DXGetErrorDescriptionA(hr), expr);
#else
        core.Trace("[%s:%s:%d] (%s)", file, func, line, expr);
#endif
        return true;
    }

    return false;
}

template <class T> void Release(T *resource)
{
    if (resource != nullptr)
    {
        CHECKD3DERR(resource->Release());
    }
}

struct RenderState {
    uint32_t isFogEnabled{};
    uint32_t textureFactor{};
    CMatrix worldMatrix{};
    CMatrix viewMatrix{};
    CMatrix projectionMatrix{};
};

} // namespace

class Dx9RendererImpl
{
  public:
    explicit Dx9RendererImpl(std::shared_ptr<OSWindow> window)
        : window_(std::move(window)), imageLoader_(new ImageLoader())
    {
    }

    ~Dx9RendererImpl() noexcept;

    void Init();

    TextureHandle LoadTexture(const std::string_view &path);

    [[nodiscard]] HWND GetHwnd() const
    {
        return reinterpret_cast<HWND>(window_->OSHandle());
    }

    RenderState SaveState();
    void RestoreState(const RenderState &state);

    bool DX9Clear(int32_t type, uint32_t color);
    uint32_t DX9GetRenderState(D3DRENDERSTATETYPE state);
    CMatrix DX9GetTransform(D3DTRANSFORMSTATETYPE state);
    void DX9SetRenderState(D3DRENDERSTATETYPE state, uint32_t value);
    void DX9SetTransform(D3DTRANSFORMSTATETYPE state, const CMatrix &matrix);

    void RecompileEffects();

    std::shared_ptr<OSWindow> window_;
    std::unique_ptr<ImageLoader> imageLoader_;
    renderer::TexturePool defaultTexturePool_;

    Effects effects_;

    IDirect3D9 *d3d_ = nullptr;
    IDirect3DDevice9 *device_ = nullptr;

    D3DFORMAT stencilFormat_{};

    uint32_t backColor_ = 0;
};

Dx9Renderer::Dx9Renderer(std::shared_ptr<OSWindow> window) : impl_(std::make_unique<Dx9RendererImpl>(std::move(window)))
{
}

Dx9Renderer::~Dx9Renderer() = default;

Dx9RendererImpl::~Dx9RendererImpl() noexcept
{
    Release(device_);
    Release(d3d_);
}

void Dx9Renderer::Init()
{
    impl_->Init();
}

void Dx9RendererImpl::Init()
{
    auto ini = fio->OpenIniFile(core.EngineIniFileName());
    bool use_large_back_buffer = false;
    bool back_buffer_can_lock = false;
    bool enable_vsync = false;
    bool windowed_mode = false;
    int msaa = D3DMULTISAMPLE_NONE;
    int adapter_index = 0;
    POINT screen_size{};
    D3DFORMAT screen_bpp{};
    if (ini)
    {
        use_large_back_buffer = ini->GetInt(nullptr, "UseLargeBackBuffer", 0) != 0;
        back_buffer_can_lock = ini->GetInt(nullptr, "lockable_back_buffer", 0) != 0;
        enable_vsync = ini->GetInt(nullptr, "vsync", 0);

        windowed_mode = ini->GetInt(nullptr, "full_screen", 1) == 0;
        screen_size.x = ini->GetInt(nullptr, "screen_x", 1024);
        screen_size.y = ini->GetInt(nullptr, "screen_y", 768);

        const auto str = ini->GetString(nullptr, "screen_bpp").value_or("D3DFMT_R5G6B5");
        if (storm::iEquals(str, "D3DFMT_A8R8G8B8"))
        {
            screen_bpp = D3DFMT_A8R8G8B8;
            stencilFormat_ = D3DFMT_D24S8;
        }
        if (storm::iEquals(str, "D3DFMT_X8R8G8B8"))
        {
            screen_bpp = D3DFMT_X8R8G8B8;
            stencilFormat_ = D3DFMT_D24S8;
        }
        if (storm::iEquals(str, "D3DFMT_R5G6B5"))
        {
            screen_bpp = D3DFMT_R5G6B5;
            stencilFormat_ = D3DFMT_D16;
        }

        msaa = ini->GetInt(nullptr, "msaa", D3DMULTISAMPLE_NONE);
        if (msaa != D3DMULTISAMPLE_NONE)
        {
            if (msaa < D3DMULTISAMPLE_2_SAMPLES || msaa > D3DMULTISAMPLE_16_SAMPLES)
            {
                msaa = D3DMULTISAMPLE_16_SAMPLES;
            }
        }

        adapter_index = ini->GetInt(nullptr, "adapter", std::numeric_limits<int32_t>::max());
    }

    core.Trace("Initializing DirectX 9");
    d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d_ == nullptr)
    {
        // MessageBox(hwnd, "Direct3DCreate9 error", "InitDevice::Direct3DCreate9", MB_OK);
        core.Trace("Direct3DCreate9 error : InitDevice::Direct3DCreate9");
        return;
    }

    const auto hwnd = GetHwnd();

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.BackBufferWidth = screen_size.x;
    d3dpp.BackBufferHeight = screen_size.y;
    d3dpp.BackBufferFormat = screen_bpp;
    d3dpp.BackBufferCount = 1;
    d3dpp.hDeviceWindow = hwnd;
    d3dpp.Windowed = windowed_mode;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = stencilFormat_;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

    if (windowed_mode)
    {
        D3DDISPLAYMODE d3ddm;
        if (FAILED(d3d_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
            throw std::runtime_error("failed to GetAdapterDisplayMode");
        d3dpp.BackBufferFormat = d3ddm.Format;
        if (d3dpp.BackBufferFormat == D3DFMT_R5G6B5)
            d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        else
            d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
        stencilFormat_ = d3dpp.AutoDepthStencilFormat;
        if (!use_large_back_buffer)
            if (d3ddm.Width < static_cast<uint32_t>(screen_size.x) ||
                d3ddm.Height < static_cast<uint32_t>(screen_size.y))
            {
                d3dpp.BackBufferWidth = d3ddm.Width;
                d3dpp.BackBufferHeight = d3ddm.Height;
                screen_size.x = d3ddm.Width;
                screen_size.y = d3ddm.Height;
            }
    }

    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    for (auto samples = msaa; msaa > D3DMULTISAMPLE_2_SAMPLES; samples--)
    {
        if (SUCCEEDED(d3d_->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat,
                                                       false, static_cast<D3DMULTISAMPLE_TYPE>(samples), nullptr)))
        {
            d3dpp.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(samples);
            break;
        }
    }

    if (back_buffer_can_lock)
        d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    else
        d3dpp.Flags = 0;
    // d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    // d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
    // if(windowed) d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//FLIP;
    // else d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;

    if (enable_vsync)
    {
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }
    else
    {
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }

    // Choose desired video adapter
    auto adapters_num = d3d_->GetAdapterCount();
    if (adapter_index > adapters_num - 1)
        adapter_index = 0U;

    spdlog::info("Querying available DirectX 9 adapters... detected {}:", adapters_num);
    for (UINT i = 0; i != adapters_num; ++i)
    {
        D3DCAPS9 caps;
        d3d_->GetDeviceCaps(i, D3DDEVTYPE_HAL, &caps);
        if (caps.DeviceType == D3DDEVTYPE_HAL)
        {
            D3DADAPTER_IDENTIFIER9 id;
            d3d_->GetAdapterIdentifier(i, 0, &id);
            spdlog::info("{}: {} ({}) ", i, id.Description, id.DeviceName);
        }
    }
    spdlog::info("Using adapter with index {} (configurable by setting adapter=<index> inside engine.ini)",
                 adapter_index);

    // Create device
    if (CHECKD3DERR(d3d_->CreateDevice(adapter_index, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp,
                                       &device_)))
    {
        if (CHECKD3DERR(d3d_->CreateDevice(adapter_index, D3DDEVTYPE_HAL, hwnd, D3DCREATE_MIXED_VERTEXPROCESSING,
                                           &d3dpp, &device_)))
        {
            if (CHECKD3DERR(d3d_->CreateDevice(adapter_index, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                               &d3dpp, &device_)))
            {
                return;
            }
        }
    }

    effects_.setDevice(device_);
}

void Dx9Renderer::Render(const Scene &scene)
{
    impl_->DX9Clear(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | ((impl_->stencilFormat_ == D3DFMT_D24S8) ? D3DCLEAR_STENCIL : 0),
             scene.background);

    const auto old_state = impl_->SaveState();

    // Render sprites
    SpriteVertex vertices[4]{};
    for (int i = 0; i < 4; i++)
        vertices[i].pos.z = 1.f;
    vertices[0].tu = vertices[1].tu = 0.f;
    vertices[2].tu = vertices[3].tu = 1.f;
    vertices[0].tv = vertices[2].tv = 0.f;
    vertices[1].tv = vertices[3].tv = 1.f;

    // Render scene
    for (const auto &node : scene.nodes_) {
        if (node->IsVisible() ) {
            if (const auto *sprite_node = dynamic_cast<SpriteNode *>(node); sprite_node != nullptr) {
                const TextureHandle texture_handle = sprite_node->GetTexture();
                if (texture_handle.IsValid())
                {
                    auto *texture =
                        static_cast<IDirect3DTexture9 *>(impl_->defaultTexturePool_.GetHandle(texture_handle));
                    impl_->device_->SetTexture(0, texture);
                }
                else {
                    impl_->device_->SetTexture(0, nullptr);
                }

                const auto &position = sprite_node->GetPosition();
                const size_t width = sprite_node->GetWidth();
                const size_t height = sprite_node->GetHeight();
                vertices[0].pos.x = vertices[1].pos.x = static_cast<float>(position.x - static_cast<int32_t>(width) / 2);
                vertices[2].pos.x = vertices[3].pos.x = static_cast<float>(position.x + static_cast<int32_t>(width) / 2);
                vertices[0].pos.y = vertices[2].pos.y = static_cast<float>(position.y - static_cast<int32_t>(height) / 2);
                vertices[1].pos.y = vertices[3].pos.y = static_cast<float>(position.y + static_cast<int32_t>(height) / 2);

                if (impl_->effects_.begin("iMouseCurShow")) {
                    do {
                        impl_->device_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(SpriteVertex));
                    } while(impl_->effects_.next());
                }
            }
        }
    }

    impl_->RestoreState(old_state);
}

TextureHandle Dx9Renderer::LoadTexture(const std::string_view &path)
{
    return impl_->LoadTexture(path);
}

TextureHandle Dx9RendererImpl::LoadTexture(const std::string_view &path)
{
    const auto image = imageLoader_->LoadImageFromFile(path);

    if (image != nullptr)
    {
        IDirect3DTexture9 *texture = nullptr;
        if (CHECKD3DERR(device_->CreateTexture(image->width, image->height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
                                               &texture, nullptr)))
            return {};

        const auto path_str = istring(traits_cast<ichar_traits<char>>(path));
        const auto result = defaultTexturePool_.CreateTexture(path_str, texture);

        IDirect3DSurface9 *surface = nullptr;
        texture->GetSurfaceLevel(0, &surface);
        D3DLOCKED_RECT rect;
        if (CHECKD3DERR(surface->LockRect(&rect, nullptr, 0))) {
            defaultTexturePool_.ReleaseTexture(result);
            return {};
        }
        image->CopyToBuffer(static_cast<uint8_t *>(rect.pBits));
        surface->UnlockRect();
        surface->Release();

        return result;
    }
    else
    {
        return {};
    }
}

[[nodiscard]] IDirect3DDevice9 *Dx9Renderer::GetDevice() const
{
    return impl_->device_;
}

void Dx9Renderer::RecompileEffects()
{
    impl_->RecompileEffects();
}

renderer::TexturePool &Dx9Renderer::GetDefaultTexturePool() const
{
    return impl_->defaultTexturePool_;
}

RenderState Dx9RendererImpl::SaveState()
{
    RenderState result;

    result.textureFactor = DX9GetRenderState(D3DRS_TEXTUREFACTOR);
    result.isFogEnabled = DX9GetRenderState(D3DRS_FOGENABLE);
    result.viewMatrix = DX9GetTransform(D3DTS_VIEW);
    result.projectionMatrix = DX9GetTransform(D3DTS_PROJECTION);
    result.worldMatrix = DX9GetTransform(D3DTS_WORLD);

    return result;
}

void Dx9RendererImpl::RestoreState(const RenderState &state)
{
    DX9SetRenderState(D3DRS_TEXTUREFACTOR, state.textureFactor);
    DX9SetRenderState(D3DRS_FOGENABLE, state.isFogEnabled);
    DX9SetTransform(D3DTS_VIEW, state.viewMatrix);
    DX9SetTransform(D3DTS_PROJECTION, state.projectionMatrix);
    DX9SetTransform(D3DTS_WORLD, state.worldMatrix);
}

bool Dx9RendererImpl::DX9Clear(const int32_t type, const uint32_t color)
{
    if (CHECKD3DERR(device_->Clear(0L, NULL, type, color, 1.0f, 0L)) == true)
        return false;
    return true;
}

uint32_t Dx9RendererImpl::DX9GetRenderState(D3DRENDERSTATETYPE state)
{
    uint32_t value{};
    CHECKD3DERR(device_->GetRenderState(state, (DWORD *)&value));
    return value;
}

CMatrix Dx9RendererImpl::DX9GetTransform(D3DTRANSFORMSTATETYPE state)
{
    CMatrix m{};
    CHECKD3DERR(device_->GetTransform(state, (D3DMATRIX *)&m));
    return m;
}

void Dx9RendererImpl::DX9SetRenderState(D3DRENDERSTATETYPE state, uint32_t value)
{
    CHECKD3DERR(device_->SetRenderState(state, value));
}

void Dx9RendererImpl::DX9SetTransform(D3DTRANSFORMSTATETYPE state, const CMatrix &matrix)
{
    CHECKD3DERR(device_->SetTransform(state, (D3DMATRIX *)&matrix));
}

void Dx9RendererImpl::RecompileEffects()
{
#ifdef _WIN32 // Effects
    effects_.release();

    std::filesystem::path cur_path = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::u8path(fio->_GetExecutableDirectory()));
    for (const auto &p : std::filesystem::recursive_directory_iterator("resource/techniques"))
        if (is_regular_file(p) && p.path().extension() == ".fx")
        {
            auto s = p.path().string(); // hug microsoft
            effects_.compile(s.c_str());
        }
    std::filesystem::current_path(cur_path);
#endif
}

} // namespace storm
