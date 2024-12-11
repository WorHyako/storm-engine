#pragma once

#include <string>

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR ""
#endif

namespace Storm::Filesystem::Paths {
    [[nodiscard]]
    constexpr std::string root() noexcept {
        return {PROJECT_ROOT_DIR};
    }

    [[nodiscard]]
    constexpr std::string stash() noexcept {
        return root() + "/Stash";
    }

    [[nodiscard]]
    constexpr std::string resources() noexcept {
        return root() + "/Resources";
    }

    [[nodiscard]]
    constexpr std::string sentry_db() noexcept {
        return root() + "/sentry-db";
    }

    [[nodiscard]]
    constexpr std::string logs() noexcept {
        return root() + "/Logs";
    }

    [[nodiscard]]
    constexpr std::string screenshots() noexcept {
        return root() + "/Screenshots";
    }

    [[nodiscard]]
    constexpr std::string script_cache() noexcept {
        return root() + "/Cache";
    }

    [[nodiscard]]
    constexpr std::string save_data() noexcept {
        return root() + "/SaveData";
    }

    [[nodiscard]]
    constexpr std::string program() noexcept {
        return root() + "/Program";
    }
}
