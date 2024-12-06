cmake_minimum_required(VERSION 3.19)

# -------------- #
#   SeaCameras   #
# -------------- #
add_library(sea_cameras)
add_library(storm::sea_cameras ALIAS sea_cameras)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea_cameras
        PRIVATE ${Sources})

target_compile_definitions(sea_cameras
        PRIVATE NOMINMAX)

target_link_libraries(sea_cameras
        PUBLIC
        storm::island
        storm::sea_ai)
