cmake_minimum_required(VERSION 3.15)

include(FetchContent)

# ---------- #
#   Toml++   #
# ---------- #
FetchContent_Declare(tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0)

# ---------- #
#   spdlog   #
# ---------- #
FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)

# ----------------- #
#   sentry-native   #
# ----------------- #
FetchContent_Declare(sentry_native
        GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
        GIT_TAG 0.7.15)

# -------------- #
#   fast_float   #
# -------------- #
FetchContent_Declare(fast_float
        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
        GIT_TAG v7.0.0)

# ----------------- #
#   nlohmann-json   #
# ----------------- #
FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3)

# -------- #
#   SDL2   #
# -------- #
# TODO: swap to gitmodule
find_package(SDL2 REQUIRED GLOBAL)

# Temp solution
set(SDL2_LIBRARIES "${SDL2_LIBRARIES}" CACHE INTERNAL "SDL2_LIBRARIES")
set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIRS}" CACHE INTERNAL "SDL2_INCLUDE_DIRS")

FetchContent_MakeAvailable(
        spdlog
        tomlplusplus
        fast_float
        nlohmann_json
        sentry_native)
