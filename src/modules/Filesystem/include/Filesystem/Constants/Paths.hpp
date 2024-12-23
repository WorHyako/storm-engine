#pragma once

#include <filesystem>

namespace Storm::Filesystem::Constants::Paths {
    [[nodiscard]]
    std::filesystem::path root() noexcept;

    [[nodiscard]]
    std::filesystem::path stash() noexcept;

    [[nodiscard]]
    std::filesystem::path interfaces() noexcept;

    [[nodiscard]]
    std::filesystem::path resources() noexcept;

    [[nodiscard]]
    std::filesystem::path ini() noexcept;

    [[nodiscard]]
    std::filesystem::path sentry_db() noexcept;

    [[nodiscard]]
    std::filesystem::path logs() noexcept;

    [[nodiscard]]
    std::filesystem::path screenshots() noexcept;

    [[nodiscard]]
    std::filesystem::path script_cache() noexcept;

    [[nodiscard]]
    std::filesystem::path save_data() noexcept;

    [[nodiscard]]
    std::filesystem::path locations() noexcept;

    [[nodiscard]]
    std::filesystem::path foam() noexcept;

    [[nodiscard]]
    std::filesystem::path aliases() noexcept;

    [[nodiscard]]
    std::filesystem::path program() noexcept;
}
