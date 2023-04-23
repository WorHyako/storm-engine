#pragma once

#include "renderer.hpp"

#include <istring.hpp>

#include <string>
#include <vector>

namespace storm::renderer {

class TexturePool {
  public:
    TextureHandle GetTexture(const istring_view &name);
    TexturePool& ReleaseTexture(TextureHandle);

    TextureHandle CreateTexture(const istring_view &name, void *handle);

    [[nodiscard]] void *GetHandle(TextureHandle handle) const;

  private:
    struct TextureInfo {
        istring name;
        void *handle{};
        size_t refCount{};
    };

    std::vector<TextureInfo> textures_;
};

} // namespace storm::renderer
