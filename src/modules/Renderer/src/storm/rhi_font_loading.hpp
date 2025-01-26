#pragma once

#include "../rhi_font.h"

namespace storm
{

std::unique_ptr<VFont> LoadFont(const std::string_view &font_name, const std::string_view &ini_file_name,
                                RENDER &renderer, RHI::DeviceHandle &device);

} // namespace storm
