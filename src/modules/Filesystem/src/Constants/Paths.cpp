#include "Constants/Paths.hpp"

#include <shlobj_core.h>

using namespace Storm::Filesystem::Constants;

std::filesystem::path Paths::root() noexcept {
    return std::filesystem::current_path();
}

std::filesystem::path Paths::stash() noexcept {
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

std::filesystem::path Paths::resources() noexcept {
    return {root() / "RESOURCE"};
}

std::filesystem::path Paths::ini() noexcept {
    return {resources() / "INI"};
}

std::filesystem::path Paths::sentry_db() noexcept {
    return {stash() / "sentry-db"};
}

std::filesystem::path Paths::logs() noexcept {
    return {stash() / "Logs"};
}

std::filesystem::path Paths::screenshots() noexcept {
    return {stash() / "Screenshots"};
}

std::filesystem::path Paths::script_cache() noexcept {
    return {stash() / "Cache"};
}

std::filesystem::path Paths::save_data() noexcept {
    return {stash() / "SaveData"};
}

std::filesystem::path Paths::program() noexcept {
    return {root() / "Program"};
}