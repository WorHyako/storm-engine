cmake_minimum_required(VERSION 3.15)

include(FetchContent)

# -------------- #
#   dear imgui   #
# -------------- #
FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.5)

# ---------- #
#   Toml++   #
# ---------- #
FetchContent_Declare(tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0)

# ---------- #
#   fmtlib   #
# ---------- #
FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.0.2)

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

add_library(sdl2 INTERFACE)
add_library(sdl2::sdl2 ALIAS sdl2)

target_include_directories(sdl2
        INTERFACE ${SDL2_INCLUDE_DIRS})

target_link_libraries(sdl2
        INTERFACE ${SDL2_LIBRARIES})

FetchContent_MakeAvailable(
        spdlog
        tomlplusplus
        fast_float
        nlohmann_json
        sentry_native
        fmt
        imgui)
