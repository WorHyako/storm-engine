#pragma once

#include <string>

constexpr std::string_view config_file_ext = ".ini";

namespace Storm::Filesystem::ConfigNames {
    [[nodiscard]]
    constexpr std::string engine() noexcept {
        return "engine" + std::string(config_file_ext);
    }
}
