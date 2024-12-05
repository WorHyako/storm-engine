cmake_minimum_required(VERSION 3.19)

# ----------- #
#   SeaFoam   #
# ----------- #
add_library(sea_foam)
add_library(storm::sea_foam ALIAS sea_foam)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea_foam
        PRIVATE ${Sources})

target_link_libraries(sea_foam
        PUBLIC
        storm::particles
        storm::core
        storm::geometry
        storm::ship
        storm::model
        storm::sound_service
        storm::renderer
        storm::sea
        storm::sea_ai)
