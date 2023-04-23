#include "storm/image_loader.hpp"

#include <FreeImage.h>

namespace storm
{

class ImageLoaderImpl
{
  public:
    ImageLoaderImpl();
    ~ImageLoaderImpl() noexcept;
};

ImageLoader::ImageLoader(): impl_(std::make_unique<ImageLoaderImpl>())

{
}

ImageLoader::~ImageLoader() = default;

ImageLoaderImpl::ImageLoaderImpl()
{
    FreeImage_Initialise();
}

ImageLoaderImpl::~ImageLoaderImpl() noexcept
{
    FreeImage_DeInitialise();
}

std::unique_ptr<Image> ImageLoader::LoadImageFromFile(const std::string_view &path)
{
    const auto path_str = "resource/textures/" + static_cast<std::string>(path);

    const auto image_type = FreeImage_GetFileType(path_str.c_str(), 0);

    if (image_type != FIF_UNKNOWN)
    {
        FIBITMAP *image = FreeImage_Load(image_type, path_str.c_str(), 0);
        auto result = std::make_unique<Image>();
        result->width = FreeImage_GetWidth(image);
        result->height = FreeImage_GetHeight(image);
        result->pitch = FreeImage_GetPitch(image);
        result->type = FreeImage_GetImageType(image);
        result->bpp = FreeImage_GetBPP(image);
        result->data = FreeImage_GetBits(image);
        return result;
    }
    else {
        return nullptr;
    }
}

void Image::CopyToBuffer(uint8_t *destination)
{
    if (bpp == 24) {
        for(size_t y = height; y > 0; y--) {
            BYTE *pixel = data + (y - 1) * pitch;
            for(size_t x = 0; x < width; x++) {
                destination[0] = pixel[FI_RGBA_BLUE];
                destination[1] = pixel[FI_RGBA_GREEN];
                destination[2] = pixel[FI_RGBA_RED];
                destination[3] = 255;
                destination += 4;
                pixel += 3;
            }
        }
    }
    else {
        for(size_t y = height; y > 0; y--) {
            BYTE *pixel = data + (y - 1) * pitch;
            for(size_t x = 0; x < width; x++) {
                destination[0] = pixel[FI_RGBA_BLUE];
                destination[1] = pixel[FI_RGBA_GREEN];
                destination[2] = pixel[FI_RGBA_RED];
                destination[3] = pixel[FI_RGBA_ALPHA];
                destination += 4;
                pixel += 4;
            }
        }
    }
}

} // namespace storm
