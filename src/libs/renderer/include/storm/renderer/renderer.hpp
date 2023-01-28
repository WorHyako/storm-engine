#pragma once

namespace storm
{

class Renderer
{
  public:
    virtual ~Renderer() = default;

    virtual void Init() = 0;
};


} // namespace storm