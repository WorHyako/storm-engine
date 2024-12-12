#pragma once

#include <shlobj_core.h>
#include <string>
#include <filesystem>

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

namespace Storm::Filesystem::Constants::Paths {
    [[nodiscard]]
    constexpr std::string root() noexcept {
        return std::string(PROJECT_ROOT_DIR);
    }

    [[nodiscard]]
    inline std::string stash() noexcept {
        std::filesystem::path path;
        if (path.empty()) {
#ifdef _WIN32 // SHGetKnownFolderPath
            wchar_t *str = nullptr;
            if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_SIMPLE_IDLIST, nullptr, &str) != S_OK || str ==
                nullptr) {
                return {};
            }
            path = std::filesystem::path(str) / "My Games" / "Sea Dogs";
            CoTaskMemFree(str);
#else
            char *pref_path = nullptr;
            pref_path = SDL_GetPrefPath("Akella", "Sea Dogs");
            if (pref_path == nullptr) {
                return {};
            }
            path = pref_path;
#endif
        }
        return path.string();
    }

    [[nodiscard]]
    constexpr std::string resources() noexcept {
        return root() + "/Resources";
    }

    [[nodiscard]]
    constexpr std::string sentry_db() noexcept {
        return stash() + "/sentry-db";
    }

    [[nodiscard]]
    constexpr std::string logs() noexcept {
        return stash() + "\\Logs";
    }

    [[nodiscard]]
    constexpr std::string screenshots() noexcept {
        return stash() + "/Screenshots";
    }

    [[nodiscard]]
    constexpr std::string script_cache() noexcept {
        return stash() + "/Cache";
    }

    [[nodiscard]]
    constexpr std::string save_data() noexcept {
        return stash() + "/SaveData";
    }

    [[nodiscard]]
    constexpr std::string program() noexcept {
        return root() + "/Program";
    }
}
