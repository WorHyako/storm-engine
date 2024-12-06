cmake_minimum_required(VERSION 3.19)

# ------------ #
#   Location   #
# ------------ #
add_library(location)
add_library(storm::location ALIAS location)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(location
        PRIVATE ${Sources})

target_compile_definitions(location
        PRIVATE NOMINMAX)

target_include_directories(location
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(location
        PUBLIC
        storm::blade
        storm::sea
        storm::sound_service)
