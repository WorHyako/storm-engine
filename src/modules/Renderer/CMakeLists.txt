cmake_minimum_required(VERSION 3.19)

# ------------ #
#   Renderer   #
# ------------ #
add_library(renderer)
add_library(storm::renderer ALIAS renderer)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(renderer
        PRIVATE ${Sources})

target_include_directories(renderer
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(renderer
        PRIVATE
        UNICODE
        NOMINMAX)

target_link_libraries(renderer
        PUBLIC
        storm::d3dx9
        storm::core)
