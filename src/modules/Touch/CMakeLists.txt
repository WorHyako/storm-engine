cmake_minimum_required(VERSION 3.19)

# --------- #
#   Touch   #
# --------- #
add_library(touch)
add_library(storm::touch ALIAS touch)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(touch
        PRIVATE ${Sources})

target_link_libraries(touch
        PUBLIC
        storm::island
        storm::ship
        storm::location)
