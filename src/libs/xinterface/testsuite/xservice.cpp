#include "xservice_impl.hpp"

#include <catch2/catch.hpp>

namespace {

class MockTexturePool
{
  public:
    int TextureCreate(const char *str) {
        return textures_++;
    }
    bool TextureRelease(int id) {
        return true;
    }

    int textures_ = 0;
};

} // namespace

TEST_CASE("Create XService", "[xinterface]")
{
    MockTexturePool texture_pool;
    XSERVICE<MockTexturePool> xservice(texture_pool);
}
