cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Collide   #
# ----------- #
add_library(blot)
add_library(storm::blot ALIAS blot)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(blot
        PRIVATE ${Sources})

target_link_libraries(blot
        PUBLIC storm::model)
