#include "storm/renderer/texture_pool.hpp"

namespace storm::renderer {

TextureHandle TexturePool::GetTexture(const istring_view &name)
{
    for (size_t i = 0; i < textures_.size(); ++i)
    {
        auto &tex_info = textures_[i];
        if (tex_info.name == name)
        {
            ++tex_info.refCount;
            return TextureHandle{i};
        }
    }
    return {};
}

TexturePool &TexturePool::ReleaseTexture(TextureHandle handle)
{
    const auto refCount = textures_[handle.Index()].refCount--;
    if (refCount == 0) {
        // TODO: Release texture resources
    }
    return *this;
}

TextureHandle TexturePool::CreateTexture(const istring_view &name, void *handle)
{
    textures_.emplace_back(TextureInfo{istring(name), handle, 1});
    return TextureHandle{textures_.size() - 1};
}

void *TexturePool::GetHandle(TextureHandle handle) const
{
    return textures_[handle.Index()].handle;
}

} // namespace storm::renderer
