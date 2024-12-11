cmake_minimum_required(VERSION 3.19)

# ------------ #
#   Geometry   #
# ------------ #
add_library(geometry)
add_library(storm::geometry ALIAS geometry)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(geometry
        PRIVATE ${Sources})

target_include_directories(geometry
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_link_libraries(geometry
        PUBLIC
        storm::renderer
        storm::core)
