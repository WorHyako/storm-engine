#pragma once

#include <string>

#include "Filesystem/Constants/Paths.hpp"

constexpr std::string_view config_file_ext = ".toml";

namespace Storm::Filesystem::Constants::ConfigNames {
    [[nodiscard]]
    inline std::filesystem::path engine() noexcept {
        return Paths::root() / std::filesystem::path("engine" + std::string(config_file_ext));
    }

    [[nodiscard]]
    inline std::filesystem::path animals() noexcept {
        return Paths::ini() / std::filesystem::path("animals" + std::string(config_file_ext));
    }
}
