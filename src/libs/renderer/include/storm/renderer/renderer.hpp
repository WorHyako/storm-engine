#pragma once

#include "scene.hpp"
#include "texture_handle.hpp"

#include <string_view>

namespace storm
{

namespace renderer {

class TexturePool;

} // namespace renderer

class Renderer
{
  public:
    virtual ~Renderer() = default;

    virtual void Init() = 0;

    virtual void Render(const Scene& scene) = 0;

    virtual TextureHandle LoadTexture(const std::string_view &path) = 0;
};


} // namespace storm