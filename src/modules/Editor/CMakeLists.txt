cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Editor   #
# ---------- #
add_library(editor)
add_library(storm::editor ALIAS editor)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(editor
        PRIVATE ${Sources})

target_include_directories(editor
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(editor
        PUBLIC
        fmt::fmt
        storm::imgui)
