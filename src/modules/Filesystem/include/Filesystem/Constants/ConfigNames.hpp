#pragma once

#include <filesystem>

namespace Storm::Filesystem::Constants::ConfigNames {
    [[nodiscard]]
    std::filesystem::path engine() noexcept;

    [[nodiscard]]
    std::filesystem::path animals() noexcept;

    [[nodiscard]]
    std::filesystem::path rigging() noexcept;

    [[nodiscard]]
    std::filesystem::path particles() noexcept;

    [[nodiscard]]
    std::filesystem::path loclighter() noexcept;

    [[nodiscard]]
    std::filesystem::path dialog() noexcept;
}
