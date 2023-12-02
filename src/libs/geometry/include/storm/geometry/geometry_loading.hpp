#pragma once

#include "../../geos.h"

#include <istring.hpp>

#include <string_view>

namespace storm {

std::unique_ptr<GEOS> LoadGeometry(const istring_view &file_name);

} // namespace storm
