cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Lighter   #
# ----------- #
add_library(lighter)
add_library(storm::lighter ALIAS lighter)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(lighter
        PRIVATE ${Sources})

target_compile_definitions(lighter
        PRIVATE NOMINMAX)

target_link_libraries(lighter
        PUBLIC storm::model)
