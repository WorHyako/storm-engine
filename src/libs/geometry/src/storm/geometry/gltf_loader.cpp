#include "gltf_loader.hpp"

#include "gltf_geometry.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

namespace storm {

std::unique_ptr<GEOS> LoadGltfModel(const std::string_view &path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string error;
    std::string warning;

    std::string path_str(path.begin(), path.end());

    bool success = loader.LoadASCIIFromFile(&model, &error, &warning, path_str);

    if (success)
    {
        return std::make_unique<GltfGeometry>(std::move(model));
    }
    else
    {
        return {};
    }
}

} // namespace storm
