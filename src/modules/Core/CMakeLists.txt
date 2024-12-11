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
        ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_compile_definitions(core
        PRIVATE
        UNICODE
        NOMINMAX)

target_link_libraries(core
        PUBLIC
        comctl32
        storm::window
        storm::script_library
        storm::input
        storm::shadow
        storm::imgui
        storm::diagnostics
        storm::math
        storm::plf_stack
        fmt::fmt-header-only
        storm::steamworksSdk
        zlib
        FastFloat::fast_float)
