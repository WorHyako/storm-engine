cmake_minimum_required(VERSION 3.19)

# ----------- #
#   Collide   #
# ----------- #
add_library(blade)
add_library(storm::blade ALIAS blade)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(blade
        PRIVATE ${Sources})

target_include_directories(blade
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(blade
        PUBLIC
        storm::animation
        storm::model)