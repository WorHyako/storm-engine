#pragma once

#include <storm/handle.hpp>

namespace storm
{

DEFINE_HANDLE(TextureHandle)

class Renderer
{
  public:
    virtual ~Renderer() = default;

    virtual void Init() = 0;
};


} // namespace storm