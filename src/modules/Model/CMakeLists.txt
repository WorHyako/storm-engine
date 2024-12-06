cmake_minimum_required(VERSION 3.19)

# --------- #
#   Model   #
# --------- #
add_library(model)
add_library(storm::model ALIAS model)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(model
        PRIVATE ${Sources})

target_include_directories(model
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(model
        PUBLIC
        storm::animation
        storm::collide
        storm::geometry
        storm::core)