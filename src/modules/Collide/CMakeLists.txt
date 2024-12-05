cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Collide   #
# ----------- #
add_library(collide)
add_library(storm::collide ALIAS collide)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(collide
        PRIVATE ${Sources})

target_include_directories(collide
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(collide
        PUBLIC
        storm::core
        storm::script_library)