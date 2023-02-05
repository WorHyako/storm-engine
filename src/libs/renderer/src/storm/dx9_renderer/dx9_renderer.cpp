#include "storm/dx9_renderer/dx9_renderer.hpp"

#include "core.h"
#include "dx9render.h"
#include "string_compare.hpp"

#ifdef _WIN32
#include <DxErr.h>
#include <corecrt_io.h>
#else
#include <unistd.h>
#endif

namespace storm
{

namespace
{

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

} // namespace

class Dx9RendererImpl
{
  public:
    Dx9RendererImpl(std::shared_ptr<OSWindow> window) : window_(window)
    {
    }

    ~Dx9RendererImpl() noexcept;

    void Init();

    void Render(const Scene &scene);

    [[nodiscard]] HWND GetHwnd() const
    {
        return reinterpret_cast<HWND>(window_->OSHandle());
    }

    bool DX9Clear(int32_t type, uint32_t color);

    std::shared_ptr<OSWindow> window_;

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
}

void Dx9Renderer::Render(const Scene &scene)
{
    impl_->Render(scene);
}

void Dx9RendererImpl::Render(const Scene &scene)
{
    DX9Clear(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | ((stencilFormat_ == D3DFMT_D24S8) ? D3DCLEAR_STENCIL : 0),
             scene.background);
}

[[nodiscard]] IDirect3DDevice9 *Dx9Renderer::GetDevice() const
{
    return impl_->device_;
}

bool Dx9RendererImpl::DX9Clear(const int32_t type, const uint32_t color)
{
    if (CHECKD3DERR(device_->Clear(0L, NULL, type, color, 1.0f, 0L)) == true)
        return false;
    return true;
}

} // namespace storm
