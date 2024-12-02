
cmake_minimum_required(VERSION 3.19)

# ----------------- #
#   ScriptLibrary   #
# ----------------- #
add_library(script_library)
add_library(storm::script_library ALIAS script_library)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(script_library
        PRIVATE ${Sources})

target_include_directories(script_library
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(script_library
        PUBLIC storm::filesystem)

