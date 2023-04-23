#pragma once

#include <os_window.hpp>
#include <storm/renderer/renderer.hpp>

#include <memory>

namespace storm
{

class Dx11RendererImpl;

class Dx11Renderer final : public Renderer
{
  public:
    explicit Dx11Renderer(std::shared_ptr<OSWindow> window);
    ~Dx11Renderer() override;

    void Init() override;

    void Render(const Scene& scene) override;

    TextureHandle LoadTexture(const std::string_view &path) override;

  private:
    std::unique_ptr<Dx11RendererImpl> impl_;
};

} // namespace storm