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
#   nlohmann-json   #
# ----------------- #
FetchContent_Declare(nlohmann-json
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

FetchContent_MakeAvailable(spdlog tomlplusplus nlohmann-json)
