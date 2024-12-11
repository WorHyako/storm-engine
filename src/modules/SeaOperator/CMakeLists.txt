cmake_minimum_required(VERSION 3.19)

# --------------- #
#   SeaOperator   #
# --------------- #
add_library(sea_operator)
add_library(storm::sea_operator ALIAS sea_operator)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sea_operator
        PRIVATE ${Sources})

target_include_directories(sea_operator PUBLIC src)
target_link_libraries(sea_operator
        PUBLIC storm::ship)
