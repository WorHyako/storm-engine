#pragma once

#include <filesystem>

namespace Storm::Filesystem::Constants::ConfigNames {
    [[nodiscard]]
    std::filesystem::path engine() noexcept;

    [[nodiscard]]
    std::filesystem::path animals() noexcept;

    [[nodiscard]]
    std::filesystem::path language() noexcept;

    [[nodiscard]]
    std::filesystem::path sailors_editor() noexcept;

    [[nodiscard]]
    std::filesystem::path sound_scheme() noexcept;

    [[nodiscard]]
    std::filesystem::path helpchooser() noexcept;

    [[nodiscard]]
    std::filesystem::path rigging() noexcept;

    [[nodiscard]]
    std::filesystem::path particles() noexcept;

    [[nodiscard]]
    std::filesystem::path interfaces() noexcept;

    [[nodiscard]]
    std::filesystem::path loclighter() noexcept;

    [[nodiscard]]
    std::filesystem::path dialog() noexcept;

    [[nodiscard]]
    std::filesystem::path pictures() noexcept;

    [[nodiscard]]
    std::filesystem::path texture_sequence() noexcept;

    [[nodiscard]]
    std::filesystem::path lights() noexcept;

    [[nodiscard]]
    std::filesystem::path defaultnode() noexcept;

    [[nodiscard]]
    std::filesystem::path mast() noexcept;
}
