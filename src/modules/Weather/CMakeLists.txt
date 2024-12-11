cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Weather   #
# ----------- #
add_library(weather)
add_library(storm::weather ALIAS weather)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(weather
        PRIVATE ${Sources})

target_include_directories(weather
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_link_libraries(weather
        PUBLIC storm::ship)
