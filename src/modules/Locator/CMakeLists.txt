cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Locator   #
# ----------- #
add_library(locator)
add_library(storm::locator ALIAS locator)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(locator
        PRIVATE ${Sources})
target_include_directories(locator PUBLIC src)
target_link_libraries(locator
        PUBLIC storm::sea_ai)
