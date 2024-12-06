cmake_minimum_required(VERSION 3.19)

# ---------- #
#   Shadow   #
# ---------- #
add_library(shadow)
add_library(storm::shadow ALIAS shadow)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(shadow
        PRIVATE ${Sources})

target_compile_definitions(shadow
        PRIVATE NOMINMAX)

target_link_libraries(shadow
        PUBLIC storm::model)
