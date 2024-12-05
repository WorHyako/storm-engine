cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Rigging   #
# ----------- #
add_library(rigging)
add_library(storm::rigging ALIAS rigging)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(rigging
        PRIVATE ${Sources})

target_link_libraries(rigging
        PUBLIC
        storm::geometry
        storm::core
        storm::model
        storm::renderer
        storm::sea_ai
        storm::ship
        storm::weather)
