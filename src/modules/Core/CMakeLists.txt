cmake_minimum_required(VERSION 3.19)

# -------- #
#   Core   #
# -------- #
add_library(core)
add_library(storm::core ALIAS core)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(core
        PRIVATE ${Sources})

target_include_directories(core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(core
        PUBLIC
        storm::diagnostics
        storm::math
        fmt::fmt)
