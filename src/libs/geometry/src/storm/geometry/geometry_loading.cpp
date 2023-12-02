#include "storm/geometry/geometry_loading.hpp"

#include "../../geom.h"
#include "../../geometry_r.h"
#include "core.h"
#include "gltf_geometry.hpp"

#include "gltf_loader.hpp"

extern GEOM_SERVICE_R GSR;

namespace storm {

std::unique_ptr<GEOS> LoadGeometry(const istring_view &file_name, const istring_view &light_name)
{
    try
    {
        std::string file_name_str(file_name.begin(), file_name.end());
        if (!file_name.starts_with("resource/models/"))
        {
            file_name_str = "resource/models/" + file_name_str;
        }
        if (file_name.ends_with(".gltf")) {
            auto result = LoadGltfModel(file_name_str);
            std::string light_name_str = std::format("resource/models/{}_{}.col", file_name.substr(0, file_name.size() - 5), light_name);
            result->LoadColorData(light_name_str);
            return result;
        }
        else {
            if (!(file_name.ends_with(".gm") || file_name.ends_with(".col") ) ) {
                file_name_str += ".gm";
            }
            std::string light_name_str = std::format("resource/models/{}_{}.col", file_name, light_name);
            return std::make_unique<GEOM>(file_name_str.c_str(), light_name_str.empty() ? nullptr : light_name_str.c_str(), GSR, 0);
        }
    }
    catch (const std::exception &e)
    {
        core.Trace("%s: %s", file_name, e.what());
        return nullptr;
    }
    catch (...)
    {
        core.Trace("Invalid model: %s", file_name);
        return nullptr;
    }

    return {};
}

} // namespace storm
