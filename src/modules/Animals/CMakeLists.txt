cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Animals   #
# ----------- #
add_library(animals)
add_library(storm::animals ALIAS animals)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(animals
        PRIVATE ${Sources})

target_include_directories(animals
        PUBLIC
        src)

target_link_libraries(animals
        PUBLIC
        storm::animation
        storm::ship
        storm::sound_service)
