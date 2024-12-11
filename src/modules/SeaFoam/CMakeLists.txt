cmake_minimum_required(VERSION 3.19)

# ----------- #
#   SeaFoam   #
# ----------- #
add_library(sea_foam)
add_library(storm::sea_foam ALIAS sea_foam)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea_foam
        PRIVATE ${Sources})

target_include_directories(sea_foam PUBLIC src)
target_link_libraries(sea_foam
        PUBLIC
        storm::sound_service
        storm::sea
        storm::sea_ai)
