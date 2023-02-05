#pragma once

#include "scene.hpp"

#include <storm/handle.hpp>

namespace storm
{

DEFINE_HANDLE(TextureHandle)

class Renderer
{
  public:
    virtual ~Renderer() = default;

    virtual void Init() = 0;

    virtual void Render(const Scene& scene) = 0;
};


} // namespace storm