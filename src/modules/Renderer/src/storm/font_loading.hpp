#pragma once

#include "../font.h"

namespace storm
{

std::unique_ptr<VFont> LoadFont(const std::string_view &font_name, const std::string_view &ini_file_name,
                                VDX9RENDER &renderer, IDirect3DDevice9 &device);

} // namespace storm
