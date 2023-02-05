#include "storm/dx11_renderer/dx11_renderer.hpp"

#include <d3d11.h>
#include <d3dx9math.h>

namespace storm
{

namespace {

template<class T>
void Release(T* resource) {
    if (resource != nullptr)
    {
        resource->Release();
    }
}

} // namespace

class Dx11RendererImpl
{
  public:
    Dx11RendererImpl(std::shared_ptr<OSWindow> window) : window_(window)
    {
    }

    ~Dx11RendererImpl() noexcept;

    void Init();

    void Render(const Scene& scene);

    [[nodiscard]] HWND GetHwnd() const
    {
        return reinterpret_cast<HWND>(window_->OSHandle());
    }

  private:
    std::shared_ptr<OSWindow> window_;

    ID3D11Device *device_ = nullptr;
    ID3D11DeviceContext *context_ = nullptr;
    ID3D11RenderTargetView *backBuffer_ = nullptr;
    IDXGISwapChain *swap_chain_ = nullptr;
};

Dx11Renderer::Dx11Renderer(std::shared_ptr<OSWindow> window)
    : impl_(std::make_unique<Dx11RendererImpl>(std::move(window)))
{
}

Dx11Renderer::~Dx11Renderer() = default;

Dx11RendererImpl::~Dx11RendererImpl() noexcept
{
    Release(swap_chain_);
    Release(backBuffer_);
    Release(context_);
    Release(device_);
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

    ID3D11Texture2D *back_buffer = nullptr;
    swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&back_buffer));
    device_->CreateRenderTargetView(back_buffer, NULL, &backBuffer_);
    back_buffer->Release();

    context_->OMSetRenderTargets(1, &backBuffer_, NULL);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;
    context_->RSSetViewports(1, &viewport);
}

void Dx11Renderer::Render(const Scene& scene)
{
    impl_->Render(scene);
}

void Dx11RendererImpl::Render(const Scene& scene)
{
    context_->ClearRenderTargetView(backBuffer_, D3DXCOLOR(scene.background));

    // Do actual rendering

    swap_chain_->Present(0, 0);
}

} // namespace storm
