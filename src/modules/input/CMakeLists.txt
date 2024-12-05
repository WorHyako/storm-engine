cmake_minimum_required(VERSION 3.19)

# --------- #
#   Input   #
# --------- #
add_library(input)
add_library(storm::input ALIAS input)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(input
        PRIVATE ${Sources})

target_include_directories(input
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(input
        PUBLIC
        storm::utils
        storm::shared_headers)
