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
    XSERVICE<MockTexturePool> xservice(texture_pool);

    CHECK(xservice.FindGroup("POTC_LOGO_ENGLISH") == 0);
    CHECK(xservice.GetImageNum("POTC_LOGO_ENGLISH", "seadogslogo") == 0);
    CHECK(xservice.FindGroup("POTC_LOGO_RUSSIAN") == 1);
    CHECK(xservice.GetImageNum("mark", "MARK") == 2);
    CHECK(xservice.GetImageNum("MARK", "mark") == 2);

    CHECK(xservice.GetImageNum("SHIPS16", "Galeon1") == 4);
    CHECK(xservice.GetImageNum("SHIPS16", "Frigate1") == 5);
    CHECK(xservice.GetImageNum("SHIPS16", "Fleut1") == 6);

    FXYRECT rect;
    xservice.GetTexturePos(0, rect);
    CHECK(rect.left == 0.5f/512);
    CHECK(rect.top == 0.5f/128);
    CHECK(rect.right == 511.5f/512);
    CHECK(rect.bottom == 127.5f/128);

    const FXYPOINT top_left(0.5, 0.5);
    const XYPOINT point_size(8, 8);
    xservice.GetTextureCutForSize("mark", top_left, point_size, 32, 32, rect);
    CHECK(rect.left == 0.5f);
    CHECK(rect.top == 0.5f);
    CHECK(rect.right == 0.75f);
    CHECK(rect.bottom == 0.75f);
}
