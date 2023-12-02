#include "storm/geometry/geometry_loading.hpp"

#include "../../geom.h"
#include "../../geometry_r.h"

extern GEOM_SERVICE_R GSR;

namespace storm {

std::unique_ptr<GEOS> LoadGeometry(const istring_view &file_name)
{
    if (file_name.ends_with(".gltf")) {
        // TODO: Load gltf
    }
    else {
        std::string file_name_str(file_name.begin(), file_name.end());
        if (!file_name.ends_with(".gm")) {
            file_name_str += ".gm";
        }
        return std::make_unique<GEOM>(file_name_str.c_str(), nullptr, GSR, 0);
    }

    return {};
}

} // namespace storm
