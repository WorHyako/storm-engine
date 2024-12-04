
cmake_minimum_required(VERSION 3.19)

# ----------------- #
#   ScriptLibrary   #
# ----------------- #
add_library(script_library INTERFACE)
add_library(storm::script_library ALIAS script_library)

#file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
#target_sources(script_library
#        PRIVATE ${Sources})

target_include_directories(script_library
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(script_library
        INTERFACE storm::filesystem)

