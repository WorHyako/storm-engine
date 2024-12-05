cmake_minimum_required(VERSION 3.19)

# ------------- #
#   Animation   #
# ------------- #
add_library(animation)
add_library(storm::animation ALIAS animation)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(animation
        PRIVATE ${Sources})

target_include_directories(animation
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(animation
        PUBLIC
        storm::core
        storm::utils)