cmake_minimum_required(VERSION 3.19)

# -------------- #
#   Filesystem   #
# -------------- #
add_library(filesystem)
add_library(storm::filesystem ALIAS filesystem)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(filesystem
        PRIVATE ${Sources})

target_include_directories(filesystem
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
