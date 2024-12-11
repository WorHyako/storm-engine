cmake_minimum_required(VERSION 3.19)

# --------------- #
#   Diagnostics   #
# --------------- #
add_library(diagnostics)
add_library(storm::diagnostics ALIAS diagnostics)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(diagnostics
        PRIVATE ${Sources})

target_include_directories(diagnostics
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_link_libraries(diagnostics
        PUBLIC
        storm::filesystem
        sentry::sentry)
