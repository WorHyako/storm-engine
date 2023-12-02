#pragma once

#include "geos.h"

#include <string_view>

namespace storm {

std::unique_ptr<GEOS> LoadGltfModel(const std::string_view &path);

} // namespace storm
