cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Sailors   #
# ----------- #
add_library(sailors)
add_library(storm::sailors ALIAS sailors)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sailors
        PRIVATE ${Sources})

target_link_libraries(sailors
        PUBLIC
        storm::sea
        storm::ship)
