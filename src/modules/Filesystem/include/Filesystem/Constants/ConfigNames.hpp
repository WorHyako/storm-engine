#pragma once

#include <string>

#include "Filesystem/Constants/Paths.hpp"

constexpr std::string_view config_file_ext = ".toml";

namespace Storm::Filesystem::Constants::ConfigNames {
    [[nodiscard]]
    constexpr std::string engine() noexcept {
        return "engine" + std::string(config_file_ext);
    }

    [[nodiscard]]
    constexpr std::string animals() noexcept {
        return Paths::resources() + "animals" + std::string(config_file_ext);
    }
}
