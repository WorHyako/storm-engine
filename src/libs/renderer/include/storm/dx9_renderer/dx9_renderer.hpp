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

    void Render() override;

    // DX9 specific methods
    // _________________________________________________________________________
    [[nodiscard]] IDirect3DDevice9* GetDevice() const;

  private:
    std::unique_ptr<Dx9RendererImpl> impl_;
};

} // namespace storm