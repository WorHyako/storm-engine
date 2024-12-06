cmake_minimum_required(VERSION 3.19)

# -------------- #
#   SinkEffect   #
# -------------- #
add_library(sink_effect)
add_library(storm::sink_effect ALIAS sink_effect)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(sink_effect
        PRIVATE ${Sources})

target_link_libraries(sink_effect
        PUBLIC
        storm::sea
        storm::ship
        storm::sound_service)
