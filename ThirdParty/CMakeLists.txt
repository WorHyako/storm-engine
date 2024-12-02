cmake_minimum_required(VERSION 3.15)

# -------- #
#   SDL2   #
# -------- #
# TODO: swap to gitmodule
find_package(SDL2 REQUIRED GLOBAL)

# Temp solution
set(SDL2_LIBRARIES  "${SDL2_LIBRARIES}" CACHE INTERNAL "SDL2_LIBRARIES")
set(SDL2_INCLUDE_DIRS  "${SDL2_INCLUDE_DIRS}" CACHE INTERNAL "SDL2_INCLUDE_DIRS")

# ---------- #
#   spdlog   #
# ---------- #
add_subdirectory(spdlog)
