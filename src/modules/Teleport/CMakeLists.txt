cmake_minimum_required(VERSION 3.19)

# ------------ #
#   Teleport   #
# ------------ #
add_library(teleport)
add_library(storm::teleport ALIAS teleport)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(teleport
        PRIVATE ${Sources})

target_include_directories(teleport PUBLIC src)
target_link_libraries(teleport
        PUBLIC
        storm::pcs_controls
        storm::renderer)
