cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Config   #
# ---------- #
add_library(config)
add_library(storm::config ALIAS config)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(config
        PRIVATE ${Sources})

target_link_libraries(config
        PUBLIC storm::core)
