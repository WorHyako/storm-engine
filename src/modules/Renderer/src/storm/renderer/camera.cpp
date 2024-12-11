#include "storm/renderer/camera.hpp"

#include "core.h"

namespace storm {

VDX9RENDER &Camera::GetRenderer()
{
    if (renderer_ == nullptr)
    {
        renderer_ = static_cast<VDX9RENDER *>(core.GetService("dx9render"));
        if (renderer_ == nullptr)
        {
            throw std::runtime_error("No service: dx9render");
        }
    }

    return *renderer_;
}

void Camera::ProcessStage(Stage stage, uint32_t delta)
{
    // Camera is managed through Scene
}

} // namespace storm
