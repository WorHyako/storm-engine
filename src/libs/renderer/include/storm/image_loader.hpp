#pragma once

#include <memory>
#include <string_view>

namespace storm
{

class Image;
class ImageLoaderImpl;

class ImageLoader final
{
  public:
    ImageLoader();
    ~ImageLoader();

    std::unique_ptr<Image> LoadImageFromFile(const std::string_view &path);

  private:
    std::unique_ptr<ImageLoaderImpl> impl_;
};

class Image {
public:
  size_t width{};
  size_t height{};
  size_t pitch{};
  uint32_t type{};
  uint32_t bpp{};
  uint8_t *data{};

  void CopyToBuffer(uint8_t *destination);
};

} // namespace storm
