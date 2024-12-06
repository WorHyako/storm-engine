cmake_minimum_required(VERSION 3.19)

# -------- #
#   Ship   #
# -------- #
add_library(ship)
add_library(storm::ship ALIAS ship)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(ship
        PRIVATE ${Sources})

target_include_directories(ship
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ship
        PUBLIC
        storm::island
        storm::location
        storm::particles)
