#include "xservice_impl.hpp"

#include <catch2/catch.hpp>

namespace {

class MockTexturePool
{
  public:
    TextureHandle TextureCreate(const char *str) {
        return textures_++;
    }
    bool TextureRelease(TextureHandle id) {
        return true;
    }

    int textures_ = 0;
};

} // namespace

TEST_CASE("Get picture position", "[xinterface][xservice]")
{
    MockTexturePool texture_pool;

    storm::Data config;
    config["POTC_LOGO_ENGLISH"] = {
        {"sTextureName", "potc_logo.tga"},
        {"wTextureWidth", 512 },
        {"wTextureHeight", 128 },
        {"picture", "seadogslogo,0,0,512,128" },
    };
    config["POTC_LOGO_RUSSIAN"] = {
        {"sTextureName", "potc_logo_rus.tga"},
        {"wTextureWidth", "512" },
        {"wTextureHeight", "128" },
        {"picture", "seadogslogo,0,0,512,128" },
    };
    config["MARK"] = {
        {"sTextureName", "red_mark.tga"},
        {"wTextureWidth", 32 },
        {"wTextureHeight", 32 },
        {"picture", "mark,0,0,32,32" },
    };
    config["SHIPS16"] = {
        {"sTextureName", "ships-16.tga"},
        {"wTextureWidth", 1024 },
        {"wTextureHeight", 512 },
        {"picture", storm::Data::array({
                        "Galeon1,0,0,128,128",
                        "Frigate1,128,0,256,128",
                        "Fleut1,256,0,384,128",
                    })},
    };

    XSERVICE<MockTexturePool> xservice(texture_pool, config);

    CHECK(xservice.FindGroup("POTC_LOGO_ENGLISH") == 1);
    CHECK(xservice.GetImageNum("POTC_LOGO_ENGLISH", "seadogslogo") == 1);
    CHECK(xservice.FindGroup("POTC_LOGO_RUSSIAN") == 2);
    CHECK(xservice.GetImageNum("mark", "MARK") == 0);
    CHECK(xservice.GetImageNum("MARK", "mark") == 0);

    CHECK(xservice.GetImageNum("SHIPS16", "Galeon1") == 3);
    CHECK(xservice.GetImageNum("SHIPS16", "Frigate1") == 4);
    CHECK(xservice.GetImageNum("SHIPS16", "Fleut1") == 5);

    FXYRECT rect;
    xservice.GetTexturePos(0, rect);
    CHECK(rect.left == 0.5f/32);
    CHECK(rect.top == 0.5f/32);
    CHECK(rect.right == 31.5f/32);
    CHECK(rect.bottom == 31.5f/32);

    const FXYPOINT top_left(0.5, 0.5);
    const XYPOINT point_size(8, 8);
    xservice.GetTextureCutForSize("mark", top_left, point_size, 32, 32, rect);
    CHECK(rect.left == 0.5f);
    CHECK(rect.top == 0.5f);
    CHECK(rect.right == 0.75f);
    CHECK(rect.bottom == 0.75f);
}
