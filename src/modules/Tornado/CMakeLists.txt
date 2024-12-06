cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Tornado   #
# ----------- #
add_library(tornado)
add_library(storm::tornado ALIAS tornado)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(tornado
        PRIVATE ${Sources})

target_link_libraries(tornado
        PUBLIC
        storm::sea
        storm::ship
        storm::sound_service)
