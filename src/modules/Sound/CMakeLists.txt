cmake_minimum_required(VERSION 3.19)

# --------- #
#   Sound   #
# --------- #
add_library(sound)
add_library(storm::sound ALIAS sound)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sound
        PRIVATE ${Sources})

target_link_libraries(sound
        PUBLIC storm::sound_service)
