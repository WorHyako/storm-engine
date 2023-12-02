#pragma once

#include "geos.h"

#include <string_view>

namespace storm {

class GltfGeometry;

std::unique_ptr<GltfGeometry> LoadGltfModel(const std::string_view &path);

} // namespace storm
