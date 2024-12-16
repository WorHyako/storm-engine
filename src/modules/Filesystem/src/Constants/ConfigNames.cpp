#include "Constants/ConfigNames.hpp"

#include "Filesystem/Constants/Paths.hpp"

using namespace Storm::Filesystem::Constants;

namespace {
    constexpr std::string_view config_file_ext = ".toml";

    constexpr std::string_view engine_config_name = "engine";
    constexpr std::string_view animals_config_name = "animals";
    constexpr std::string_view rigging_config_name = "rigging";
    constexpr std::string_view particles_config_name = "particles";
    constexpr std::string_view loclighter_config_name = "loclighter";
    constexpr std::string_view dialog_config_name = "dialog";
}

std::filesystem::path ConfigNames::engine() noexcept {
    return Paths::root() / std::filesystem::path(std::string(engine_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::animals() noexcept {
    return Paths::ini() / std::filesystem::path(std::string(animals_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::rigging() noexcept {
    return Paths::ini() / std::filesystem::path(std::string(rigging_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::particles() noexcept {
    return Paths::ini() / std::filesystem::path(std::string(particles_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::loclighter() noexcept {
    return Paths::ini() / std::filesystem::path(std::string(loclighter_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::dialog() noexcept {
    return Paths::ini() / std::filesystem::path(std::string(dialog_config_name) + std::string(config_file_ext));
}
