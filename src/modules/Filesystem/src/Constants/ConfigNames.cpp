#include "Constants/ConfigNames.hpp"

#include "Filesystem/Constants/Paths.hpp"

using namespace Storm::Filesystem::Constants;

namespace {
    constexpr std::string_view config_file_ext{".toml"};

    constexpr std::string_view engine_config_name{"engine"};
    constexpr std::string_view animals_config_name{"animals"};
    constexpr std::string_view rigging_config_name{"rigging"};
    constexpr std::string_view particles_config_name{"particles"};
    constexpr std::string_view loclighter_config_name{"loclighter"};
    constexpr std::string_view dialog_config_name{"dialog"};
    constexpr std::string_view lights_config_name{"lights"};
    constexpr std::string_view texture_sequence_config_name{"TextureSequence"};
    constexpr std::string_view helpchooser_config_name{"helpchooser"};
    constexpr std::string_view pictures_config_name{"pictures"};
    constexpr std::string_view defaultnode_config_name{"defaultnode"};
    constexpr std::string_view interfaces_config_name{"interfaces"};
    constexpr std::string_view sound_scheme_config_name{"sound_scheme"};
    constexpr std::string_view mast_config_name{"mast"};
    constexpr std::string_view sailors_editor_config_name{"SailorsEditor"};
    constexpr std::string_view language_config_name{"language"};
}

std::filesystem::path ConfigNames::engine() noexcept {
    return Paths::root() / (std::string(engine_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::animals() noexcept {
    return Paths::ini() / (std::string(animals_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::language() noexcept {
    return Paths::texts() / (std::string(language_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::sailors_editor() noexcept {
    return Paths::root() / (std::string(sailors_editor_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::sound_scheme() noexcept {
    return Paths::ini() / (std::string(sound_scheme_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::helpchooser() noexcept {
    return Paths::ini() / (std::string(helpchooser_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::interfaces() noexcept {
    return Paths::interfaces() / (std::string(interfaces_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::rigging() noexcept {
    return Paths::ini() / (std::string(rigging_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::particles() noexcept {
    return Paths::ini() / (std::string(particles_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::loclighter() noexcept {
    return Paths::ini() / (std::string(loclighter_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::dialog() noexcept {
    return Paths::ini() / (std::string(dialog_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::pictures() noexcept {
    return Paths::interfaces() / (std::string(pictures_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::lights() noexcept {
    return Paths::ini() / (std::string(lights_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::defaultnode() noexcept {
    return Paths::interfaces() / (std::string(defaultnode_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::mast() noexcept {
    return Paths::ini() / (std::string(mast_config_name) + std::string(config_file_ext));
}

std::filesystem::path ConfigNames::texture_sequence() noexcept {
    return Paths::ini() / (std::string(texture_sequence_config_name) + std::string(config_file_ext));
}
