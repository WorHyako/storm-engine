cmake_minimum_required(VERSION 3.19)

# -------------- #
#   WaterRings   #
# -------------- #
add_library(water_rings)
add_library(storm::water_rings ALIAS water_rings)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(water_rings
        PRIVATE ${Sources})

target_include_directories(water_rings PUBLIC src)
target_link_libraries(water_rings
        PUBLIC
        storm::model
        storm::sea)
