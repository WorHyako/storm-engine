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

target_include_directories(lighter
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(lighter
        PUBLIC
        storm::core
        storm::geometry
        storm::model
        storm::utils
        storm::renderer)
