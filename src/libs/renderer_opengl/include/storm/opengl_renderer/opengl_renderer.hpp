#pragma once

#include <os_window.hpp>
#include <storm/renderer/renderer.hpp>

#include <memory>

namespace storm
{

class OpenGlRenderer final : public Renderer
{
  public:
    explicit OpenGlRenderer(std::shared_ptr<OSWindow> window);
    ~OpenGlRenderer() override;

    void Init() override;

    void Render(const Scene& scene) override;

    void Present() override;

    TextureHandle LoadTexture(const std::string_view &path) override;

  private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

} // namespace storm
