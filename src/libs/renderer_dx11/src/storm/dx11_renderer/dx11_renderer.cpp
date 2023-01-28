#include "storm/dx11_renderer/dx11_renderer.hpp"

#include <d3d11.h>

namespace storm
{

class Dx11RendererImpl
{
  public:
    Dx11RendererImpl(std::shared_ptr<OSWindow> window) : window_(window)
    {
    }

    ~Dx11RendererImpl() noexcept;

    void Init();

    [[nodiscard]] HWND GetHwnd() const
    {
        return reinterpret_cast<HWND>(window_->OSHandle());
    }

  private:
    std::shared_ptr<OSWindow> window_;

    ID3D11Device *device_ = nullptr;
    ID3D11DeviceContext *context_ = nullptr;
    IDXGISwapChain *swap_chain_ = nullptr;
};

Dx11Renderer::Dx11Renderer(std::shared_ptr<OSWindow> window)
    : impl_(std::make_unique<Dx11RendererImpl>(std::move(window)))
{
}

Dx11Renderer::~Dx11Renderer() = default;

Dx11RendererImpl::~Dx11RendererImpl() noexcept
{
    swap_chain_->Release();
    context_->Release();
    device_->Release();
}

void Dx11Renderer::Init()
{
    impl_->Init();
}

void Dx11RendererImpl::Init()
{
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = GetHwnd();
    swap_chain_desc.SampleDesc.Count = 4;
    swap_chain_desc.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &swap_chain_desc,
                                  &swap_chain_, &device_, NULL, &context_);
}

} // namespace storm
