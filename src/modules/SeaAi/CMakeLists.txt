cmake_minimum_required(VERSION 3.19)

# --------- #
#   SeaAi   #
# --------- #
add_library(sea_ai)
add_library(storm::sea_ai ALIAS sea_ai)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea_ai
        PRIVATE ${Sources})

target_include_directories(sea_ai
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sea_ai
        PUBLIC
        storm::core
        storm::collide
        storm::geometry
        storm::island
        storm::ai
        storm::location
        storm::model
        storm::particles
        storm::renderer
        storm::sea
        storm::ship)
