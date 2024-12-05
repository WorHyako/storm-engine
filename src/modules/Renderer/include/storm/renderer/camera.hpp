#pragma once

#include "../../dx9render.h"

#include <entity.h>

namespace storm
{

class Camera : public Entity
{
  public:
    ~Camera() override = default;

    bool IsActive () const
    {
        return active_;
    }

    Camera &SetActive(bool active)
    {
        active_ = active;
        return *this;
    }

    virtual void Execute(uint32_t real_delta) {};

    void ProcessStage(Stage stage, uint32_t delta) override;

  protected:
    VDX9RENDER& GetRenderer();

  private:
    VDX9RENDER* renderer_ = nullptr;
    bool active_ = false;
};

} // namespace storm
