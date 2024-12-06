cmake_minimum_required(VERSION 3.19)

# ------------- #
#   Particles   #
# ------------- #
add_library(particles)
add_library(storm::particles ALIAS particles)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(particles
        PRIVATE ${Sources})

target_include_directories(particles
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(particles
        PUBLIC storm::geometry)
