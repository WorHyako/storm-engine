cmake_minimum_required(VERSION 3.19)

# --------- #
#   Utils   #
# --------- #
add_library(utils)
add_library(storm::utils ALIAS utils)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(utils
        PRIVATE ${Sources})

target_include_directories(utils
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dialog
        ${CMAKE_CURRENT_SOURCE_DIR}/include/platform)

target_link_libraries(utils
        PUBLIC SDL2::SDL2main)
