cmake_minimum_required(VERSION 3.19)

# ------- #
#   Sea   #
# ------- #
add_library(sea)
add_library(storm::sea ALIAS sea)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea
        PRIVATE ${Sources})

target_include_directories(sea
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sea
        PUBLIC
        storm::core
        storm::collide
        storm::renderer)