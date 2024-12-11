#include "steam_api.hpp"

steamapi::SteamApi &steamapi::SteamApi::getInstance(const bool mock)
{
    using namespace steamapi;
    static std::unique_ptr<SteamApi> steam_api = detail::factory(mock);
    return *steam_api;
}
