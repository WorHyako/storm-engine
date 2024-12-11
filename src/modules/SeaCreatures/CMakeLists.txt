cmake_minimum_required(VERSION 3.19)

# ---------------- #
#   SeaCreatures   #
# ---------------- #
add_library(sea_creatures)
add_library(storm::sea_creatures ALIAS sea_creatures)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea_creatures
        PRIVATE ${Sources})

target_include_directories(sea_creatures PUBLIC src)
target_link_libraries(sea_creatures
        PUBLIC
        storm::island
        storm::ship)
