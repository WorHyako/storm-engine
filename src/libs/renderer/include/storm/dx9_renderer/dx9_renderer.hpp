#pragma once

#include <os_window.hpp>
#include <storm/renderer/renderer.hpp>

#include <memory>

struct IDirect3DDevice9;

namespace storm
{

class Dx9RendererImpl;

class Dx9Renderer final : public Renderer
{
  public:
    explicit Dx9Renderer(std::shared_ptr<OSWindow> window);
    ~Dx9Renderer() override;

    void Init() override;

    void Render(const Scene& scene) override;

    TextureHandle LoadTexture(const std::string_view &path) override;

    // DX9 specific methods
    // _________________________________________________________________________
    [[nodiscard]] IDirect3DDevice9* GetDevice() const;

    renderer::TexturePool &GetDefaultTexturePool() const;

  private:
    std::unique_ptr<Dx9RendererImpl> impl_;
};

} // namespace storm