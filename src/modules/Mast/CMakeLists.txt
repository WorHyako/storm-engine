cmake_minimum_required(VERSION 3.19)

# -------- #
#   Mast   #
# -------- #
add_library(mast)
add_library(storm::mast ALIAS mast)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(mast
        PRIVATE ${Sources})

target_link_libraries(mast
        PUBLIC
        storm::ship)
