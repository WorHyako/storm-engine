cmake_minimum_required(VERSION 3.19)

# ------------ #
#   WorldMap   #
# ------------ #
add_library(worldmap)
add_library(storm::worldmap ALIAS worldmap)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(worldmap
        PRIVATE ${Sources})

target_compile_definitions(worldmap
        PRIVATE NOMINMAX)

target_link_libraries(worldmap
        PUBLIC storm::battle_interface)
