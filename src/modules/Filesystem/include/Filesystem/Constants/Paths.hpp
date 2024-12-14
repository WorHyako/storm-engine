#pragma once

#include <shlobj_core.h>
#include <filesystem>

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR ""
#endif

namespace Storm::Filesystem::Constants::Paths {
    [[nodiscard]]
    inline std::filesystem::path root() noexcept {
        return {PROJECT_ROOT_DIR};
    }

    [[nodiscard]]
    inline std::filesystem::path stash() noexcept {
#ifdef _WIN32 // SHGetKnownFolderPath
        wchar_t *str = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_SIMPLE_IDLIST, nullptr, &str) != S_OK
            || str == nullptr) {
            return {};
        }
        std::filesystem::path path{std::filesystem::path(str) / "My Games" / "Sea Dogs"};
        CoTaskMemFree(str);
#else
            char *pref_path = nullptr;
            pref_path = SDL_GetPrefPath("Akella", "Sea Dogs");
            if (pref_path == nullptr) {
                return {};
            }
            path = pref_path;
#endif
        return path;
    }

    [[nodiscard]]
    inline std::filesystem::path resources() noexcept {
        return {root() / "RESOURCE"};
    }

    [[nodiscard]]
    inline std::filesystem::path ini() noexcept {
        return {resources() / "INI"};
    }

    [[nodiscard]]
    inline std::filesystem::path sentry_db() noexcept {
        return {stash() / "sentry-db"};
    }

    [[nodiscard]]
    inline std::filesystem::path logs() noexcept {
        return {stash() / "Logs"};
    }

    [[nodiscard]]
    inline std::filesystem::path screenshots() noexcept {
        return {stash() / "Screenshots"};
    }

    [[nodiscard]]
    inline std::filesystem::path script_cache() noexcept {
        return {stash() / "Cache"};
    }

    [[nodiscard]]
    inline std::filesystem::path save_data() noexcept {
        return {stash() / "SaveData"};
    }

    [[nodiscard]]
    inline std::filesystem::path program() noexcept {
        return {root() / "Program"};
    }
}
